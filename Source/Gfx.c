/* *************************************
 * 	Includes
 * *************************************/

#include "Gfx.h"

/* *************************************
 * 	Defines
 * *************************************/

#define PRIMITIVE_LIST_SIZE 0x1000
#define DOUBLE_BUFFERING_SWAP_Y	256
#define UPLOAD_IMAGE_FLAG 1
#define MAX_LUMINANCE 0xFF
#define ROTATE_BIT_SHIFT 12
#define GPUSTAT (*(unsigned int*)0x1F801814)
#define D2_CHCR (*(unsigned int*)0x1F8010A8)

/* *************************************
 * 	Structs and enums
 * *************************************/

enum
{
	BUTTON_SIZE = 16,
	
	BUTTON_CROSS_U = 48,
	BUTTON_CROSS_V = 0,
	
	BUTTON_SQUARE_U = 0,
	BUTTON_SQUARE_V = 0,
	
	BUTTON_TRIANGLE_U = 32,
	BUTTON_TRIANGLE_V = 0,
	
	BUTTON_CIRCLE_U = 16,
	BUTTON_CIRCLE_V = 0,

	BUTTON_DIRECTION_U = 64,
	BUTTON_DIRECTION_V = 0,
	
	BUTTON_LR_U = 80,
	BUTTON_LR_V = 0,
	BUTTON_LR_SIZE = 24,

	LETTER_SIZE = 8,

	LETTER_L1_U = 104,
	LETTER_L1_V = 0,

	LETTER_L2_U = 112,
	LETTER_L2_V = 0,

	LETTER_R1_U = 104,
	LETTER_R1_V = 8,

	LETTER_R2_U = 112,
	LETTER_R2_V = 8,

	LETTER_OFFSET_INSIDE_BUTTON_LR_X = 8,
	LETTER_OFFSET_INSIDE_BUTTON_LR_Y = 6
	
};

enum
{
	GFX_SECOND_DISPLAY_X = 384,
	GFX_SECOND_DISPLAY_Y = 256,
	GFX_SECOND_DISPLAY_TPAGE = 22
};

/* *************************************
 * 	Global Variables
 * *************************************/

GsSprite PSXButtons;

/* *************************************
 * 	Local Prototypes
 * *************************************/



/* *************************************
 * 	Local Variables
 * *************************************/

//Drawing environment
static GsDrawEnv DrawEnv;
//Display environment
static GsDispEnv DispEnv;
//Primitive list (it contains all the graphical data for the GPU)
static unsigned int prim_list[PRIMITIVE_LIST_SIZE];
//Tells other modules whether data is being loaded to GPU
static volatile bool gfx_busy;
//Dictates (R,G,B) brigthness to all sprites silently
static uint8_t global_lum;

static bool five_hundred_ms_show;
static bool one_second_show;

void GfxSwapBuffers(void)
{
	// Consistency check
#if PSXSDK_DEBUG

	if(GsListPos() >= PRIMITIVE_LIST_SIZE)
	{
		dprintf("Linked list iterator overflow!\n");
		while(1);
	}
	
	if( (DrawEnv.h != Y_SCREEN_RESOLUTION)
					||
		( (DrawEnv.w != X_SCREEN_RESOLUTION)
						&&
		  (DrawEnv.w != X_SCREEN_RESOLUTION >> 1) )
					||
		( (DispEnv.y != DOUBLE_BUFFERING_SWAP_Y)
						&&
		  (DispEnv.y != 0) )		)
	{
		dprintf("What the hell is happening?\n");
		DEBUG_PRINT_VAR(DispEnv.x);
		DEBUG_PRINT_VAR(DispEnv.y);
		DEBUG_PRINT_VAR(DrawEnv.x);
		DEBUG_PRINT_VAR(DrawEnv.y);

		while(1);
	}
#endif // PSXSDK_DEBUG

	if(DrawEnv.h == Y_SCREEN_RESOLUTION)
	{
		if(DispEnv.y == 0)
		{
			DispEnv.y = DOUBLE_BUFFERING_SWAP_Y;
			DrawEnv.y = 0;
		}
		else if(DispEnv.y == DOUBLE_BUFFERING_SWAP_Y)
		{
			DispEnv.y = 0;
			DrawEnv.y = DOUBLE_BUFFERING_SWAP_Y;
		}
			
		GsSetDispEnv(&DispEnv);
		GsSetDrawEnv(&DrawEnv);
	}
}


void GfxInitDrawEnv(void)
{
	DrawEnv.x = 0;
	DrawEnv.y = 0;
	DrawEnv.draw_on_display = false;
	DrawEnv.w = X_SCREEN_RESOLUTION;
	DrawEnv.h = Y_SCREEN_RESOLUTION;
	
	GsSetDrawEnv(&DrawEnv);
}

void GfxInitDispEnv(void)
{
	DispEnv.x = 0;
	DispEnv.y = 0;
	
	GsSetDispEnv(&DispEnv);
}

void GfxSetPrimitiveList(void)
{
	GsSetList(prim_list);
}

void GfxDrawScene_Fast(void)
{
	if(System1SecondTick() == true)
	{
		one_second_show = one_second_show? false:true;
	}

	if(System500msTick() == true)
	{
		five_hundred_ms_show = five_hundred_ms_show? false:true;
	}
	
	GfxSwapBuffers();
	FontCyclic();
	GsDrawList();
}

bool GfxReadyForDMATransfer(void)
{
	return ( (GPUSTAT & 1<<28) && !(D2_CHCR & 1<<24) );
}

void GfxDrawScene(void)
{
	while(	(SystemRefreshNeeded() == false) 
				||
			(GfxIsGPUBusy() == true)		);
			
	GfxDrawScene_Fast();
	
	SystemCyclicHandler();
}

void GfxDrawScene_Slow(void)
{
	GfxDrawScene();
	while(GfxIsGPUBusy() == true);
}

void GfxSortSprite(GsSprite * spr)
{
	uint8_t aux_r = spr->r;
	uint8_t aux_g = spr->g;
	uint8_t aux_b = spr->b;
	unsigned char aux_tpage = spr->tpage;
	short aux_w = spr->w;
	short aux_x = spr->x;
	bool has_1hz_flash = spr->attribute & GFX_1HZ_FLASH;
	bool has_2hz_flash = spr->attribute & GFX_2HZ_FLASH;
	
	if(	(spr->w <= 0) || (spr->h <= 0) )
	{
		// Invalid width or heigth
		return;
	}
	
	if(GfxIsSpriteInsideScreenArea(spr) == false)
	{
		return;
	}
	else if(has_2hz_flash && Gfx2HzFlash() == false)
	{
		return;
	}
	else if(has_1hz_flash && Gfx1HzFlash() == false)
	{
		return;
	}
	
	if(global_lum != NORMAL_LUMINANCE)
	{
		if(spr->r < NORMAL_LUMINANCE - global_lum)
		{
			spr->r = 0;
		}
		else
		{
			spr->r -= NORMAL_LUMINANCE - global_lum;
		}
		
		if(spr->g < NORMAL_LUMINANCE - global_lum)
		{
			spr->g = 0;
		}
		else
		{
			spr->g -= NORMAL_LUMINANCE - global_lum;
		}
		
		if(spr->b < NORMAL_LUMINANCE - global_lum)
		{
			spr->b = 0;
		}
		else
		{
			spr->b -= NORMAL_LUMINANCE - global_lum;
		}
	}
	
	if(has_1hz_flash == true)
	{
		spr->attribute &= ~(GFX_1HZ_FLASH);
	}
	
	if(spr->w > MAX_SIZE_FOR_GSSPRITE)
	{
		// GsSprites can't be bigger than 256x256, so since display
		// resolution is 384x240, it must be split into two primitives.
		
		spr->w = MAX_SIZE_FOR_GSSPRITE;
		GsSortSprite(spr);

		spr->x += MAX_SIZE_FOR_GSSPRITE;
		spr->w = X_SCREEN_RESOLUTION - MAX_SIZE_FOR_GSSPRITE;
		spr->tpage += MAX_SIZE_FOR_GSSPRITE / GFX_TPAGE_WIDTH;
		GsSortSprite(spr);
		
		// Restore original values after sorting
		spr->w = aux_w;
		spr->tpage = aux_tpage;
		spr->x = aux_x;
	}
	else
	{
		GsSortSprite(spr);
	}
	
	if(has_1hz_flash == true)
	{
		spr->attribute |= GFX_1HZ_FLASH;
	}
	
	spr->r = aux_r;
	spr->g = aux_g;
	spr->b = aux_b;
}

uint8_t GfxGetGlobalLuminance(void)
{
	return global_lum;
}

void GfxSetGlobalLuminance(uint8_t value)
{
	global_lum = value;
}

void GfxIncreaseGlobalLuminance(int8_t step)
{	
	if( (	(global_lum + step) < MAX_LUMINANCE )
			&&
		(	(global_lum + step) > 0	)			)
	{
		global_lum += step;
	}
	else
	{
		global_lum = MAX_LUMINANCE;
	}
}

int GfxRotateFromDegrees(int deg)
{
	return deg << ROTATE_BIT_SHIFT;
}

bool GfxIsGPUBusy(void)
{
	return (GsIsDrawing() || gfx_busy || (GfxReadyForDMATransfer() == false) );
}

bool GfxSpriteFromFile(char* fname, GsSprite * spr)
{
	GsImage gsi;
	
	if(SystemLoadFile(fname) == false)
	{
		return false;
	}
	
	while(GfxIsGPUBusy() == true);
	
	gfx_busy = true;
		
	GsImageFromTim(&gsi,SystemGetBufferAddress() );
	
	GsSpriteFromImage(spr,&gsi,UPLOAD_IMAGE_FLAG);
	gfx_busy = false;

    DEBUG_PRINT_VAR(spr->tpage);
    DEBUG_PRINT_VAR(spr->u);
    DEBUG_PRINT_VAR(spr->v);
    DEBUG_PRINT_VAR(spr->w);
    DEBUG_PRINT_VAR(spr->h);
	
	return true;
}

bool GfxCLUTFromFile(char* fname)
{
	GsImage gsi;

	if(SystemLoadFile(fname) == false)
	{
		return false;
	}
	
	while(GfxIsGPUBusy() == true);
	
	gfx_busy = true;
		
	GsImageFromTim(&gsi,SystemGetBufferAddress() );
	
	GsUploadCLUT(&gsi);
	
	gfx_busy = false;
	
	return true;
}

bool GfxIsInsideScreenArea(short x, short y, short w, short h)
{
	if( ( (x + w) >= 0) 
			&&
		(x < DrawEnv.w)
			&&
		( (y + h) >= 0)
			&&
		(y < DrawEnv.h)	)
	{
		return true;
	}
		
	return false;
}

bool GfxIsSpriteInsideScreenArea(GsSprite * spr)
{
	return GfxIsInsideScreenArea(spr->x, spr->y, spr->w, spr->h);
}

void GfxButtonSetFlags(uint8_t flags)
{
	PSXButtons.attribute |= flags;
}

void GfxButtonRemoveFlags(uint8_t flags)
{
	PSXButtons.attribute &= ~flags;
}

void GfxDrawButton(short x, short y, unsigned short btn)
{
	static bool first_entered = true;
	static short orig_u;
	static short orig_v;
	
	if(first_entered == true)
	{
		first_entered = false;
		orig_u = PSXButtons.u;
		orig_v = PSXButtons.v;
	}
	
	PSXButtons.w = BUTTON_SIZE;
	PSXButtons.h = BUTTON_SIZE;
	
	PSXButtons.r = NORMAL_LUMINANCE;
	PSXButtons.g = NORMAL_LUMINANCE;
	PSXButtons.b = NORMAL_LUMINANCE;
	
	PSXButtons.x = x;
	PSXButtons.y = y;
	PSXButtons.mx = PSXButtons.w >> 1;
	PSXButtons.my = PSXButtons.h >> 1;
	
	switch(btn)
	{
		case PAD_CROSS:
			PSXButtons.u = BUTTON_CROSS_U;
			PSXButtons.v = BUTTON_CROSS_V;
		break;
		
		case PAD_SQUARE:
			PSXButtons.u = BUTTON_SQUARE_U;
			PSXButtons.v = BUTTON_SQUARE_V;
		break;
		
		case PAD_TRIANGLE:
			PSXButtons.u = BUTTON_TRIANGLE_U;
			PSXButtons.v = BUTTON_TRIANGLE_V;
		break;
		
		case PAD_CIRCLE:
			PSXButtons.u = BUTTON_CIRCLE_U;
			PSXButtons.v = BUTTON_CIRCLE_V;
		break;

		case PAD_RIGHT:
			PSXButtons.u = BUTTON_DIRECTION_U;
			PSXButtons.v = BUTTON_DIRECTION_V;
		break;
		
		case PAD_UP:
			PSXButtons.u = BUTTON_DIRECTION_U;
			PSXButtons.v = BUTTON_DIRECTION_V;
			PSXButtons.rotate = 90 << ROTATE_BIT_SHIFT;
		break;
		
		case PAD_DOWN:
			PSXButtons.u = BUTTON_DIRECTION_U;
			PSXButtons.v = BUTTON_DIRECTION_V;
			PSXButtons.rotate = 270 << ROTATE_BIT_SHIFT;
		break;

		case PAD_LEFT:
			PSXButtons.u = BUTTON_DIRECTION_U;
			PSXButtons.v = BUTTON_DIRECTION_V;
			PSXButtons.attribute |= H_FLIP;
		break;
		
		case PAD_L1:
			// Fall through
		case PAD_L2:
			// Fall through
		case PAD_R1:
			// Fall through
		case PAD_R2:
			PSXButtons.u = BUTTON_LR_U;
			PSXButtons.v = BUTTON_LR_V;
			PSXButtons.w = BUTTON_LR_SIZE;
		break;

		case PAD_SELECT:
			// Fall through
		case PAD_START:
			// Fall through
		default:
			// Set null width and height so that sprite doesn't get sorted
			PSXButtons.w = 0;
			PSXButtons.h = 0;
		break;
	}
	
	PSXButtons.u += orig_u;
	PSXButtons.v += orig_v;
	
	GfxSortSprite(&PSXButtons);

	PSXButtons.attribute &= ~H_FLIP;
	PSXButtons.rotate = 0;
}

void GfxSaveDisplayData(GsSprite *spr)
{
	while(GfxIsGPUBusy() == true);
	
	MoveImage(	DispEnv.x,
				DispEnv.y,
				GFX_SECOND_DISPLAY_X,
				GFX_SECOND_DISPLAY_Y,
				X_SCREEN_RESOLUTION,
				Y_SCREEN_RESOLUTION);
				
	spr->x = 0;
	spr->y = 0;
	spr->tpage = GFX_SECOND_DISPLAY_TPAGE;
	spr->attribute |= COLORMODE(COLORMODE_16BPP);
	spr->w = X_SCREEN_RESOLUTION;
	spr->h = Y_SCREEN_RESOLUTION;
	spr->u = 0;
	spr->v = 0;
	spr->r = NORMAL_LUMINANCE;
	spr->g = NORMAL_LUMINANCE;
	spr->b = NORMAL_LUMINANCE;

	while(GfxIsGPUBusy() == true);
}

bool Gfx1HzFlash(void)
{
	return one_second_show;
}

bool Gfx2HzFlash(void)
{
	return five_hundred_ms_show;
}

bool GfxTPageOffsetFromVRAMPosition(GsSprite * spr, short x, short y)
{
	if(	(x >= VRAM_W) || (x < 0) || (y >= VRAM_H) || (y < 0) )
	{
		return false;
	}
	
	spr->tpage = x / GFX_TPAGE_WIDTH;
	spr->tpage += (short)(VRAM_W / GFX_TPAGE_WIDTH) * (short)(y / GFX_TPAGE_HEIGHT);
	
	spr->u = (x % GFX_TPAGE_WIDTH);
	
	if(spr->attribute & COLORMODE(COLORMODE_8BPP))
	{
		// On 8bpp images, it looks like U offset needs to be multiplied by 2.
		spr->u <<= 1;
	}
	
	spr->v = (y % GFX_TPAGE_HEIGHT);
	
	//dprintf("Sprite:\n\tTPAGE: %d\n\tU=%d\n\tV=%d\n",spr->tpage,spr->u, spr->v);
	
	return false;
}

TYPE_CARTESIAN_POS GfxIsometricToCartesian(TYPE_ISOMETRIC_POS * ptrIsoPos)
{
	TYPE_CARTESIAN_POS retCartPos;
	
	retCartPos.x = ptrIsoPos->x - (ptrIsoPos->x >> 1);
	retCartPos.x -= ptrIsoPos->y >> 1;
	
	retCartPos.y = ptrIsoPos->y >> 2;
	retCartPos.y += ptrIsoPos->x >> 2;
	retCartPos.y -= ptrIsoPos->z;
	
	return retCartPos;
}

void GfxDrawScene_NoSwap(void)
{
	GsDrawList();
}

TYPE_CARTESIAN_POS GfxIsometricFix16ToCartesian(TYPE_ISOMETRIC_FIX16_POS * ptrIso16Pos)
{
	TYPE_ISOMETRIC_POS IsoPos;
	
	IsoPos.x = (short)fix16_to_int(ptrIso16Pos->x);
	IsoPos.y = (short)fix16_to_int(ptrIso16Pos->y);
	IsoPos.z = (short)fix16_to_int(ptrIso16Pos->z);
	
	return GfxIsometricToCartesian(&IsoPos);
}

TYPE_ISOMETRIC_POS GfxCartesianToIsometric(TYPE_CARTESIAN_POS * ptrCartPos)
{
	TYPE_ISOMETRIC_POS IsoPos;
	
	/*isoX = cartX - cartY;
	isoY = (cartX + cartY) / 2;*/
	IsoPos.x = ptrCartPos->x + (ptrCartPos->y << 1);
	IsoPos.y = (ptrCartPos->y << 1) - ptrCartPos->x;
	
	// Explicitly suppose z = 0
	IsoPos.z = 0;
	
	return IsoPos;
}

void GfxSetSplitScreen(uint8_t playerIndex)
{
	switch(playerIndex)
	{
		case PLAYER_ONE:
			DrawEnv.x = 0;
			DrawEnv.w = X_SCREEN_RESOLUTION >> 1;
		break;
		
		case PLAYER_TWO:
			DrawEnv.x = X_SCREEN_RESOLUTION >> 1;
			DrawEnv.w = X_SCREEN_RESOLUTION >> 1;
		break;
		
		default:
		break;
	}
	
	GsSetDrawEnv_DMA(&DrawEnv);
}

void GfxDisableSplitScreen(void)
{
	DrawEnv.x = 0;
	DrawEnv.w = X_SCREEN_RESOLUTION;
	
	GsSetDrawEnv_DMA(&DrawEnv);
}

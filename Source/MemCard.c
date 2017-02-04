/* *************************************
 * 	Includes
 * *************************************/

#include "MemCard.h"

/* *************************************
 * 	Defines
 * *************************************/
 
#define MEMCARD_SECTOR_SIZE 128
#define MEMCARD_SECTORS_PER_BLOCK 64
#define MEMCARD_SECTORS_PER_BLOCK_BITSHIFT 6
#define MEMCARD_BLOCK_MAX_ICONS 3
#define MEMCARD_ICON_INDEX_TIME 4

#define MEMCARD_MAXIMUM_SECTOR 511

#define MEMCARD_INVALID_CHECKSUM 	0x4E
#define MEMCARD_CORRECT_RW			0x47
#define MEMCARD_BAD_SECTOR			0xFF

/* *************************************
 * 	Structs and enums
 * *************************************/
 
typedef enum t_Sectors
{
	TITLE_FRAME = 0,
	ICON_FRAME_1,
	ICON_FRAME_2,
	ICON_FRAME_3,
	DATA_FRAME,	
}MEMCARD_FILE_SECTORS;

enum
{
	MEMCARD_BLOCK_IMAGE_X = 768	,
	MEMCARD_BLOCK_IMAGE_Y = 352	,
	MEMCARD_BLOCK_CLUT_X = 960	,
	MEMCARD_BLOCK_CLUT_Y = 352	,
	MEMCARD_BLOCK_CLUT_W = 16	,
	MEMCARD_BLOCK_CLUT_H = 1	,
	MEMCARD_BLOCK_IMAGE_W = 16	,
	MEMCARD_BLOCK_IMAGE_H = 16	,
	MEMCARD_BLOCK_IMAGE_W_BITSHIFT = 2,
	MEMCARD_LOAD_DATA_TEXT_X = 96,
	MEMCARD_LOAD_DATA_TEXT_Y = 192,
};

enum
{
	MEMCARD_DIALOG_X = 64,
	MEMCARD_DIALOG_Y = 28,
	MEMCARD_DIALOG_W = 256,
	MEMCARD_DIALOG_H = 184,
	
	MEMCARD_DIALOG_R = 0,
	MEMCARD_DIALOG_G = 128,
	MEMCARD_DIALOG_B = 64,
	
	MEMCARD_DIALOG_GAP_X = 24,
	MEMCARD_DIALOG_GAP_SLOT = 128
};

enum
{
	MEMCARD_BG_X = 64,
	MEMCARD_BG_Y = 28,
	MEMCARD_BG_W = 256,
	MEMCARD_BG_H = 184,
	
	MEMCARD_BG_R0 = 0,
	MEMCARD_BG_R1 = MEMCARD_BG_R0,
	MEMCARD_BG_R2 = MEMCARD_BG_R0,
	MEMCARD_BG_R3 = MEMCARD_BG_R0,
	
	MEMCARD_BG_G0 = 0,
	MEMCARD_BG_G1 = MEMCARD_BG_G0,
	MEMCARD_BG_G2 = NORMAL_LUMINANCE,
	MEMCARD_BG_G3 = MEMCARD_BG_G2,
	
	MEMCARD_BG_B0 = 0,
	MEMCARD_BG_B1 = MEMCARD_BG_B0,
	MEMCARD_BG_B2 = NORMAL_LUMINANCE >> 1,
	MEMCARD_BG_B3 = MEMCARD_BG_B2
};

enum
{
	MEMCARD_PROGRESS_BAR_X = 86,
	MEMCARD_PROGRESS_BAR_Y = 148,
	MEMCARD_PROGRESS_BAR_W = 226,
	MEMCARD_PROGRESS_BAR_H = 16,
	
	MEMCARD_PROGRESS_BAR_N_LINES = 4,
	
	MEMCARD_PROGRESS_BAR_R = NORMAL_LUMINANCE,
	MEMCARD_PROGRESS_BAR_G = NORMAL_LUMINANCE,
	MEMCARD_PROGRESS_BAR_B = NORMAL_LUMINANCE
};

typedef enum t_MemcardProcess
{
	MEMCARD_PROCESS_GET_FILENAME = 0,
	MEMCARD_PROCESS_GET_INITIAL_FRAME,
	MEMCARD_PROCESS_GET_ICON_FRAME,
	MEMCARD_PROCESS_UPLOAD_TO_GPU
}MEMCARD_PROCESS;

typedef struct t_MemCardErrors
{
	unsigned char ErrorByte;
	MEMCARD_SLOTS Slot;
	MEMCARD_BLOCKS Block;
	MEMCARD_PROCESS Process;
}TYPE_MEMCARD_ERRORS;
 
/* *************************************
 * 	Local Prototypes
 * *************************************/
 
static bool MemCardGetInitialFrameInfo(TYPE_BLOCK_DATA * ptrBlockData);
static bool MemCardGetIconFrameInfo(TYPE_BLOCK_DATA * ptrBlockData);
static bool MemCardGetBlockStateFileName(TYPE_BLOCK_DATA * ptrBlockData);
static void ISR_MemCardDataHandling(void);
static bool MemCardReadSector(TYPE_BLOCK_DATA * ptrBlockData, int sector);
static void MemCardIconIndexHandler(void);

/* *************************************
 * 	Local Variables
 * *************************************/
 
static uint8_t DataBuffer[MEMCARD_SECTOR_SIZE];
static TYPE_MEMCARD_ERRORS MemCardErrors;
static GsSprite SecondDisplay;
static GsGPoly4 MemCardRect;
static GsRectangle MemCardProgressBar;
static GsLine MemCardProgressBarLines[MEMCARD_PROGRESS_BAR_N_LINES];
static uint8_t IconIndex;
static MEMCARD_STATUS MemCardStatus[MEMCARD_NUMBER_OF_SLOTS];

// Local variables used to communicate between functions and ISR.
// Names are pretty self-explanatory.
static volatile uint8_t TotalBlocks;
static volatile uint8_t CurrentReadBlock;
static volatile short ProgressBarXOffset;

/* *************************************
 * 	Global Variables
 * *************************************/

TYPE_BLOCK_DATA MemCardData[MEMCARD_BLOCKS_PER_CARD][MEMCARD_NUMBER_OF_SLOTS];

void MemCardInit(void)
{
	TYPE_BLOCK_DATA * ptrBlockData;
	uint8_t i;
	uint8_t j;
	
	for(j = SLOT_ONE; j <= SLOT_TWO; j++)
	{	
		for(i = BLOCK_1; i <= BLOCK_15; i++)
		{
			ptrBlockData = &MemCardData[i - BLOCK_1][j];
			
			ptrBlockData->IconTPoly.r = NORMAL_LUMINANCE;
			ptrBlockData->IconTPoly.g = NORMAL_LUMINANCE;
			ptrBlockData->IconTPoly.b = NORMAL_LUMINANCE;
		}
	}
	
	bzero((TYPE_MEMCARD_ERRORS*)&MemCardErrors, sizeof(TYPE_MEMCARD_ERRORS) );
}

void ISR_MemCardDataHandling(void)
{
	
	uint8_t i;
	
	if(	(GfxIsGPUBusy() == true) || (SystemIsBusy() == true) )
	{
		return;
	}
	
	// Dim background
	SecondDisplay.r = NORMAL_LUMINANCE >> 1;
	SecondDisplay.g = NORMAL_LUMINANCE >> 1;
	SecondDisplay.b = NORMAL_LUMINANCE >> 1;
	
	MemCardRect.x[0] = MEMCARD_BG_X;
	MemCardRect.x[1] = MEMCARD_BG_X + MEMCARD_BG_W;
	MemCardRect.x[2] = MEMCARD_BG_X;
	MemCardRect.x[3] = MEMCARD_BG_X + MEMCARD_BG_W;
	
	MemCardRect.y[0] = MEMCARD_BG_Y;
	MemCardRect.y[1] = MEMCARD_BG_Y;
	MemCardRect.y[2] = MEMCARD_BG_Y + MEMCARD_BG_H;
	MemCardRect.y[3] = MEMCARD_BG_Y + MEMCARD_BG_H;
	
	MemCardRect.r[0] = MEMCARD_BG_R0;
	MemCardRect.r[1] = MEMCARD_BG_R1;
	MemCardRect.r[2] = MEMCARD_BG_R2;
	MemCardRect.r[3] = MEMCARD_BG_R3;
	
	MemCardRect.g[0] = MEMCARD_BG_G0;
	MemCardRect.g[1] = MEMCARD_BG_G1;
	MemCardRect.g[2] = MEMCARD_BG_G2;
	MemCardRect.g[3] = MEMCARD_BG_G3;
	
	MemCardRect.b[0] = MEMCARD_BG_B0;
	MemCardRect.b[1] = MEMCARD_BG_B1;
	MemCardRect.b[2] = MEMCARD_BG_B2;
	MemCardRect.b[3] = MEMCARD_BG_B3;
	
	MemCardRect.attribute |= ENABLE_TRANS | TRANS_MODE(0);
	
	// "Loading" bar line 0 (up left - up right)
	
	MemCardProgressBarLines[0].x[0] = MEMCARD_PROGRESS_BAR_X;
	MemCardProgressBarLines[0].x[1] = MEMCARD_PROGRESS_BAR_X + MEMCARD_PROGRESS_BAR_W;
	
	MemCardProgressBarLines[0].y[0] = MEMCARD_PROGRESS_BAR_Y;
	MemCardProgressBarLines[0].y[1] = MEMCARD_PROGRESS_BAR_Y;
	
	// "Loading" bar line 1 (up left - down left)
	
	MemCardProgressBarLines[1].x[0] = MEMCARD_PROGRESS_BAR_X;
	MemCardProgressBarLines[1].x[1] = MEMCARD_PROGRESS_BAR_X;
	
	MemCardProgressBarLines[1].y[0] = MEMCARD_PROGRESS_BAR_Y;
	MemCardProgressBarLines[1].y[1] = MEMCARD_PROGRESS_BAR_Y + MEMCARD_PROGRESS_BAR_H;
	
	// "Loading" bar line 2 (down left - down right)
	
	MemCardProgressBarLines[2].x[0] = MEMCARD_PROGRESS_BAR_X;
	MemCardProgressBarLines[2].x[1] = MEMCARD_PROGRESS_BAR_X + MEMCARD_PROGRESS_BAR_W;
	
	MemCardProgressBarLines[2].y[0] = MEMCARD_PROGRESS_BAR_Y + MEMCARD_PROGRESS_BAR_H;
	MemCardProgressBarLines[2].y[1] = MEMCARD_PROGRESS_BAR_Y + MEMCARD_PROGRESS_BAR_H;
	
	// "Loading" bar line 3 (up right - down right)
	
	MemCardProgressBarLines[3].x[0] = MEMCARD_PROGRESS_BAR_X + MEMCARD_PROGRESS_BAR_W;
	MemCardProgressBarLines[3].x[1] = MEMCARD_PROGRESS_BAR_X + MEMCARD_PROGRESS_BAR_W;
	
	MemCardProgressBarLines[3].y[0] = MEMCARD_PROGRESS_BAR_Y;
	MemCardProgressBarLines[3].y[1] = MEMCARD_PROGRESS_BAR_Y + MEMCARD_PROGRESS_BAR_H;
	
	for(i = 0; i < MEMCARD_PROGRESS_BAR_N_LINES; i++)
	{
		MemCardProgressBarLines[i].r = NORMAL_LUMINANCE;
		MemCardProgressBarLines[i].g = NORMAL_LUMINANCE;
		MemCardProgressBarLines[i].b = NORMAL_LUMINANCE;
	}
	
	// Set progress bar attributes
	
	MemCardProgressBar.x = MEMCARD_PROGRESS_BAR_X;
	MemCardProgressBar.y = MEMCARD_PROGRESS_BAR_Y;
	
	MemCardProgressBar.w = ProgressBarXOffset;
	MemCardProgressBar.h = MEMCARD_PROGRESS_BAR_H;
	
	MemCardProgressBar.r = MEMCARD_PROGRESS_BAR_R;
	MemCardProgressBar.g = MEMCARD_PROGRESS_BAR_G;
	MemCardProgressBar.b = MEMCARD_PROGRESS_BAR_B;
	
	GfxSortSprite(&SecondDisplay);
	GsSortGPoly4(&MemCardRect);
	GsSortRectangle(&MemCardProgressBar);
	
	for(i = 0; i < MEMCARD_PROGRESS_BAR_N_LINES; i++)
	{
		GsSortLine(&MemCardProgressBarLines[i]);
	}
	
	FontSetFlags(&SmallFont, FONT_BLEND_EFFECT);
	
	FontPrintText(&SmallFont, MEMCARD_LOAD_DATA_TEXT_X, MEMCARD_LOAD_DATA_TEXT_Y, "Loading memory card data...");
	
	FontSetFlags(&SmallFont, FONT_NOFLAGS);
	
	GfxDrawScene_Fast();
	
}
 
void MemCardResetBlockData(TYPE_BLOCK_DATA * ptrBlockData)
{	
	bzero((TYPE_BLOCK_DATA*)ptrBlockData, sizeof(TYPE_BLOCK_DATA));
	
	ptrBlockData->BlockCount = FIRST_OR_ONLY_BLOCK;
	
	IconIndex = 0;	
}

bool MemCardGetBlockInfo(	TYPE_BLOCK_DATA * ptrBlockData,
							MEMCARD_SLOTS slot,
							MEMCARD_BLOCKS blockNumber	)
{
	MemCardResetBlockData(ptrBlockData);
	
	ptrBlockData->Slot = slot;
	ptrBlockData->Block = blockNumber;
	
	dprintf("MemCardGetBlockStateFileName...\n");
	
	if(MemCardGetBlockStateFileName(ptrBlockData) == false)
	{
		return false;
	}
	
	if(ptrBlockData->BlockCount == EMPTY_BLOCK)
	{
		// Stop looking for any other data.
		return true;
	}
	
	dprintf("MemCardGetInitialFrameInfo...\n");
	
	if(MemCardGetInitialFrameInfo(ptrBlockData) == false)
	{
		return false;
	}
	
	dprintf("MemCardGetIconFrameInfo...\n");
	
	if(MemCardGetIconFrameInfo(ptrBlockData) == false)
	{
		return false;
	}
	
	// We will not get any block data information, we are only interested
	// in basic info.
	
	dprintf("MemCardUploadToGPU...\n");
	
	if(MemCardUploadToGPU(ptrBlockData) == false)
	{
		return false;
	}
		
	return true;
}

bool MemCardGetBlockStateFileName(TYPE_BLOCK_DATA * ptrBlockData)
{
	int sector = ptrBlockData->Block;
	
	MemCardErrors.Block = ptrBlockData->Block;
	MemCardErrors.Slot = ptrBlockData->Slot;
	MemCardErrors.Process = MEMCARD_PROCESS_GET_FILENAME;
	
	memset(DataBuffer, 0, MEMCARD_SECTOR_SIZE);
	
	if(MemCardReadSector(ptrBlockData, sector) == false)
	{
		return false;
	}
	
	// 00h-03h Block Allocation State
	dprintf("Block %d, slot %d, allocation state: 0x%02X.\n",
			ptrBlockData->Block,
			ptrBlockData->Slot,
			DataBuffer[0]	);
	
	/*	00000051h - In use ;first-or-only block of a file
	*	00000052h - In use ;middle block of a file (if 3 or more blocks)
	*	00000053h - In use ;last block of a file   (if 2 or more blocks) 
	* 	000000A0h - Free   ;freshly formatted
	*	000000A1h - Free   ;deleted (first-or-only block of file)
	*	000000A2h - Free   ;deleted (middle block of file)
	*	000000A3h - Free   ;deleted (last block of file)		*/
	
	// Always take into account memory card data is little-endian,
	// so if using a hex editor, you will read 51000000h.
	
	switch(DataBuffer[0])
	{
		case 0x51:
			ptrBlockData->BlockCount = FIRST_OR_ONLY_BLOCK;
		break;
		
		case 0x52:
			ptrBlockData->BlockCount = INTERMEDIATE_BLOCK;
		break;
		
		case 0x53:
			ptrBlockData->BlockCount = LAST_BLOCK;
		break;
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
			ptrBlockData->BlockCount = EMPTY_BLOCK;
			return true;
		default:
			printf("Invalid block allocation state!\n");
			return false;
	}
	
	// 0Ah-1Eh Filename in ASCII, terminated by 00h (max 20 chars, plus ending 00h)
	// File name is only defined on first block of group (allocation state == 0x51)
	
	if(ptrBlockData->BlockCount == FIRST_OR_ONLY_BLOCK)
	{
		memset(ptrBlockData->FileName, 0 , MEMCARD_FILENAME_SIZE);
		
		memcpy(ptrBlockData->FileName, &DataBuffer[0x0A], MEMCARD_FILENAME_SIZE);
		
		dprintf("File name: %s\n", ptrBlockData->FileName);
	}
	
	return true;
}

bool MemCardGetInitialFrameInfo(TYPE_BLOCK_DATA * ptrBlockData)
{
	unsigned int i;
	int sector = ptrBlockData->Block << MEMCARD_SECTORS_PER_BLOCK_BITSHIFT;
	
	MemCardErrors.Block = ptrBlockData->Block;
	MemCardErrors.Slot = ptrBlockData->Slot;
	MemCardErrors.Process = MEMCARD_PROCESS_GET_FILENAME;
	
	if(ptrBlockData->BlockCount != FIRST_OR_ONLY_BLOCK)
	{
		// Icon data is only stored on first block (if game takes more
		// than one block. Skip this step otherwise.
		
		// When dealing with intermediate or last blocks of a file,
		// we use a static pointer which points to first block of a file,
		// and then image data is copied into other blocks.
		return true;
	}
	
	// Pretty silly operation (TITLE_FRAME = 0), but used for
	// conceptual purposes and better understanding.
	sector += TITLE_FRAME;
	
	memset(DataBuffer, 0, MEMCARD_SECTOR_SIZE);
	
	if(MemCardReadSector(ptrBlockData, sector) == false)
	{
		return false;
	}
	
	dprintf("Magic number: '%c' '%c'\n",DataBuffer[0], DataBuffer[1]);
	
	if(DataBuffer[0] != 'S' || DataBuffer[1] != 'C')
	{
		// Invalid magic number.
		dprintf("Invalid magic number extracted from slot %d, block %d.\n",
				ptrBlockData->Slot,
				ptrBlockData->Block);

		return false;
	}
	
	/* 02h	Icon Display Flag
				11h...Icon has 1 frame  (static) (same image shown forever)
				12h...Icon has 2 frames (animated) (changes every 16 PAL frames)
				13h...Icon has 3 frames (animated) (changes every 11 PAL frames)
	* */
	
	switch(DataBuffer[2])
	{
		case 0x11:
			ptrBlockData->IconNumber = 1;
		break;
		
		case 0x12:
			ptrBlockData->IconNumber = 2;
		break;
		
		case 0x13:
			ptrBlockData->IconNumber = 3;
		break;
		
		default:
			// Invalid icon display flag! We can't know how many icons
			// are used.
			return false;
	}
	
	dprintf("Number of icons: %d\n", ptrBlockData->IconNumber);
	
	// 60h-7Fh  Icon 16 Color Palette Data (each entry is 16bit CLUT)
	
	for(i = 0; i < ptrBlockData->IconNumber; i++)
	{
		memcpy(ptrBlockData->CLUT[i],&DataBuffer[0x60], MEMCARD_CLUT_SIZE);
	}
	
	return true;
}

bool MemCardGetIconFrameInfo(TYPE_BLOCK_DATA * ptrBlockData)
{
	int initial_sector = ptrBlockData->Block << MEMCARD_SECTORS_PER_BLOCK_BITSHIFT;
	int sector;
	unsigned int i;
	unsigned int j;
	uint8_t buffer_contents;
	static TYPE_BLOCK_DATA * ptrReferenceBlock = NULL;
	
	switch(ptrBlockData->BlockCount)
	{
		case EMPTY_BLOCK:
		// Empty blocks are not of interest. Skip.
		return true;
		
		case INTERMEDIATE_BLOCK:
		case LAST_BLOCK:
		
			if(ptrReferenceBlock == NULL)
			{
				dprintf("No reference memory card block found yet!\n");
				return false;
			}
			
			ptrBlockData->IconNumber = ptrReferenceBlock->IconNumber;
			
			for(i = 0; i < MEMCARD_NUMBER_OF_ICONS; i++)
			{
				memcpy(ptrBlockData->CLUT[i], ptrReferenceBlock->CLUT[i], MEMCARD_CLUT_SIZE);
				memcpy(ptrBlockData->Icons[i], ptrReferenceBlock->Icons[i], MEMCARD_ICON_SIZE);
			}
			
			if(ptrBlockData->BlockCount == LAST_BLOCK)
			{
				// Dereference pointer
				ptrReferenceBlock = NULL;
			}
			
		return true;
		case FIRST_OR_ONLY_BLOCK:
		
			// Icon Frame(s) (Block 1..15, Frame 1..3) (in first block of file only)
			for(i = ICON_FRAME_1; i <= ptrBlockData->IconNumber; i++)
			{
				dprintf("\tIcon %d out of %d\n",i, ptrBlockData->IconNumber);
				buffer_contents = 0;
				sector = initial_sector + i;
				memset(DataBuffer, 0, MEMCARD_SECTOR_SIZE * sizeof(uint8_t) );
				
				if(MemCardReadSector(ptrBlockData, sector) == false)
				{
					dprintf("Could not read memory sector!\n");
					return false;
				}
				
				memcpy(ptrBlockData->Icons[i - 1 /* ICON_FRAME_# - 1 */], DataBuffer, MEMCARD_SECTOR_SIZE);
				
				for(j = 0; j < MEMCARD_SECTOR_SIZE; j++)
				{
					buffer_contents |= ptrBlockData->Icons[i - 1][j];
				}
				
				if(buffer_contents == 0)
				{
					// Icon buffer is empty!
					dprintf("Invalid icon buffer for slot %d, block %d.\n",
							ptrBlockData->Slot,
							ptrBlockData->Block);

					return false;
				}
			}
			
		// Use current block as reference if file contains more than one block.
		ptrReferenceBlock = ptrBlockData;
		
		return true;
	}
	
	dprintf("Unknown error from MemCardGetIconFrameInfo()!\n");
	
	return false;
}

bool MemCardUploadToGPU(TYPE_BLOCK_DATA * ptrBlockData)
{
	uint8_t i;
	short x_clut_offset;
	short y_clut_offset;
	short x_block_offset;
	GsImage gs;
	
	if(	(ptrBlockData->IconNumber < 1)
					||
		(ptrBlockData->IconNumber > MEMCARD_NUMBER_OF_ICONS) )
	{
		dprintf("Invalid number of icons.\n");
		return false;
	}
	
	for(i = 0; i < ptrBlockData->IconNumber; i++)
	{
		gs.pmode = COLORMODE_4BPP;
		gs.has_clut = 1;
		
		x_clut_offset = i << 4;
		y_clut_offset = ptrBlockData->Block - 1;
		
		gs.clut_x = MEMCARD_BLOCK_CLUT_X + x_clut_offset;
		gs.clut_y = MEMCARD_BLOCK_CLUT_Y + y_clut_offset + (ptrBlockData->Slot << 4);
		gs.clut_w = MEMCARD_BLOCK_CLUT_W;
		gs.clut_h = MEMCARD_BLOCK_CLUT_H;
		
		dprintf("Gs Clut = {%d,%d,%d,%d}\n",
				gs.clut_x,
				gs.clut_y,
				gs.clut_w,
				gs.clut_h	);
		
		x_block_offset = MEMCARD_BLOCK_IMAGE_W * (ptrBlockData->Block - 1);
		x_block_offset *= MEMCARD_BLOCK_MAX_ICONS;
		x_block_offset >>= MEMCARD_BLOCK_IMAGE_W_BITSHIFT;
		x_block_offset += i<<MEMCARD_BLOCK_IMAGE_W_BITSHIFT;
		
		gs.x = MEMCARD_BLOCK_IMAGE_X + x_block_offset;			
		gs.y = MEMCARD_BLOCK_IMAGE_Y + (MEMCARD_BLOCK_IMAGE_H * ptrBlockData->Slot);
		
		// Dimensions are 16x16 px, but since 4bpp is used, it actually
		// takes 4x16 px on the framebuffer.
		gs.w = MEMCARD_BLOCK_IMAGE_W >> MEMCARD_BLOCK_IMAGE_W_BITSHIFT;
		gs.h = MEMCARD_BLOCK_IMAGE_H;
		
		gs.clut_data = (uint8_t*)ptrBlockData->CLUT[i];
		gs.data = (uint8_t*)ptrBlockData->Icons[i];
		
		GsUploadImage(&gs);
		
		if(i == 0)
		{
			ptrBlockData->IconTPoly.attribute = COLORMODE(COLORMODE_4BPP);
			ptrBlockData->IconTPoly.tpage = (gs.x / 64) + ((gs.y/256)*16);
			
			dprintf("\tTPAGE = %d\n", ptrBlockData->IconTPoly.tpage);
			
			x_block_offset = MEMCARD_BLOCK_IMAGE_W * (ptrBlockData->Block - 1);
			x_block_offset *= MEMCARD_BLOCK_MAX_ICONS;
			
			ptrBlockData->IconTPoly.u[0] = MEMCARD_BLOCK_IMAGE_X + x_block_offset;
			ptrBlockData->IconTPoly.u[1] = ptrBlockData->IconTPoly.u[0] + (gs.w << 2);
			ptrBlockData->IconTPoly.u[2] = ptrBlockData->IconTPoly.u[0];
			ptrBlockData->IconTPoly.u[3] = ptrBlockData->IconTPoly.u[1];
			
			dprintf("\tu = {%d, %d, %d, %d}\n",
					ptrBlockData->IconTPoly.u[0],
					ptrBlockData->IconTPoly.u[1],
					ptrBlockData->IconTPoly.u[2],
					ptrBlockData->IconTPoly.u[3]);
			
			ptrBlockData->IconTPoly.v[0] = gs.y % 256;
			ptrBlockData->IconTPoly.v[1] = ptrBlockData->IconTPoly.v[0];
			ptrBlockData->IconTPoly.v[2] = (gs.y % 256) + (gs.h);
			ptrBlockData->IconTPoly.v[3] = ptrBlockData->IconTPoly.v[2];
			
			dprintf("\tu = {%d, %d, %d, %d}\n",
					ptrBlockData->IconTPoly.v[0],
					ptrBlockData->IconTPoly.v[1],
					ptrBlockData->IconTPoly.v[2],
					ptrBlockData->IconTPoly.v[3]);
			
			ptrBlockData->IconTPoly.r = NORMAL_LUMINANCE;
			ptrBlockData->IconTPoly.g = NORMAL_LUMINANCE;
			ptrBlockData->IconTPoly.b = NORMAL_LUMINANCE;
			
			ptrBlockData->IconTPoly.cx = gs.clut_x;
			ptrBlockData->IconTPoly.cy = gs.clut_y;
			
			dprintf("\tclut = {%d, %d}\n",
					ptrBlockData->IconTPoly.cx,
					ptrBlockData->IconTPoly.cy);
		}
	}
	
	return true;
}

bool MemCardReadSector(TYPE_BLOCK_DATA * ptrBlockData, int sector)
{
	uint8_t result;
	
	MemCardErrors.Block = ptrBlockData->Block;
	MemCardErrors.Slot = ptrBlockData->Slot;
	
	if(	(ptrBlockData->Slot != 0)
				&&
		(ptrBlockData->Slot != 1)	)
	{
		MemCardErrors.ErrorByte = 'S';
		
		dprintf("Incorrect slot %d! Block %d?\n",
				ptrBlockData->Slot,
				ptrBlockData->Block);
				
		return false;
	}
	
	if((sector < 0) || (sector > MEMCARD_MAXIMUM_SECTOR))
	{
		MemCardErrors.ErrorByte = 'T';
		
		dprintf("Invalid memory card sector %d. Only values between"
				" 0 and 511 are allowed!\n", sector);
		return false;
	}
	
	result = McReadSector(ptrBlockData->Slot, sector, DataBuffer);
	
	// Fill char "MemCardErrors" for further error description.
	MemCardErrors.ErrorByte = result;
	
	switch(result)
	{
		case '1':
		case '2':
		case 'L':
		case 'M':
			return false;

		case MEMCARD_INVALID_CHECKSUM:
			dprintf("Invalid checksum for memory card sector %d"
					" from block %d, slot %d",
					sector,
					ptrBlockData->Block,
					ptrBlockData->Slot		);
			return false;
			
		case MEMCARD_BAD_SECTOR:
			dprintf("Invalid memory card sector %d. Only values between"
				" 0 and 511 are allowed!\n", sector);
			return false;
			
		case MEMCARD_CORRECT_RW:
			return true;
		
		default:
		return false;
	}
	
	return true;
}

bool MemCardGetAllData(void)
{
	uint8_t i;
	uint8_t j;
	
	PadClearData();
	
	CurrentReadBlock = 0;
	
	TotalBlocks = MEMCARD_BLOCKS_PER_CARD * MEMCARD_NUMBER_OF_SLOTS;
	
	SmallFont.spr.r = 0;
	SmallFont.spr.g = 0;
	SmallFont.spr.b = 0;
	
	GfxSaveDisplayData(&SecondDisplay);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	// ISR_MemCardDataHandling draws a rectangle on top to show
	// memory card loading progress.
	
	SetVBlankHandler(&ISR_MemCardDataHandling);
	
	for(j = SLOT_ONE; j <= SLOT_TWO; j++)
	{
		MemCardStatus[j] = McGetStatus(j);
		
		if(MemCardStatus[j] == MEMCARD_STATUS_UNKNOWN)
		{
			// Memcard not connected and/or formatted.
			continue;
		}
	
		for(i = BLOCK_1; i <= BLOCK_15; i++)
		{
			ProgressBarXOffset = (short)(CurrentReadBlock * 
								(MEMCARD_PROGRESS_BAR_W /
								(MEMCARD_BLOCKS_PER_CARD * MEMCARD_NUMBER_OF_SLOTS) ) );
			
			if(MemCardGetBlockInfo(&MemCardData[i - BLOCK_1][j], j, i) == false)
			{
				// Return to normal behaviour if anything fails
				SetVBlankHandler(&ISR_SystemDefaultVBlank);
				return false;
			}
			
			CurrentReadBlock++;
		}
	}
	
	SetVBlankHandler(&ISR_SystemDefaultVBlank);
	
	CurrentReadBlock = 0;
	
	return true;
}

void MemCardHandler(void)
{
	MemCardIconIndexHandler();
}

void MemCardIconIndexHandler(void)
{
	static uint8_t iconTimer = 0;
	
	if(System100msTick() == true)
	{
		if(++iconTimer >= MEMCARD_ICON_INDEX_TIME)
		{
			iconTimer = 0;
			
			if(++IconIndex >= MEMCARD_NUMBER_OF_ICONS)
			{
				IconIndex = 0;
			}
		}
	}
}

void MemCardDrawIcon(TYPE_BLOCK_DATA * ptrBlockData, short x, short y)
{
	uint8_t i;
	// Auxiliar variable to keep original data
	short orig_u[4];
	short orig_clut_x;
	static bool first_access = true;
	
	if(ptrBlockData->BlockCount == EMPTY_BLOCK)
	{
		return;
	}
	
	ptrBlockData->IconTPoly.x[0] = x;
	ptrBlockData->IconTPoly.x[1] = x + MEMCARD_BLOCK_IMAGE_W;
	ptrBlockData->IconTPoly.x[2] = x;
	ptrBlockData->IconTPoly.x[3] = ptrBlockData->IconTPoly.x[1];

	ptrBlockData->IconTPoly.y[0] = y;
	ptrBlockData->IconTPoly.y[1] = ptrBlockData->IconTPoly.y[0];
	ptrBlockData->IconTPoly.y[2] = y + MEMCARD_BLOCK_IMAGE_H;
	ptrBlockData->IconTPoly.y[3] = ptrBlockData->IconTPoly.y[2];

	for(i = 0; i < 4; i++)
	{
		orig_u[i] = ptrBlockData->IconTPoly.u[i];
	}
	
	orig_clut_x = ptrBlockData->IconTPoly.cx;
	
	if(ptrBlockData->IconNumber >= IconIndex)
	{
		ptrBlockData->IconTPoly.u[0] += MEMCARD_BLOCK_IMAGE_W * IconIndex;
		ptrBlockData->IconTPoly.u[1] = ptrBlockData->IconTPoly.u[0] + MEMCARD_BLOCK_IMAGE_W;
		ptrBlockData->IconTPoly.u[2] = ptrBlockData->IconTPoly.u[0];
		ptrBlockData->IconTPoly.u[3] = ptrBlockData->IconTPoly.u[1];
		
		ptrBlockData->IconTPoly.cx += IconIndex * MEMCARD_BLOCK_CLUT_W;
	}
	
	if(first_access == true)
	{
		if(IconIndex == 0)
		{
			first_access = false;
			
			dprintf("Icon index: %d\n",IconIndex);
		
			dprintf("\tU = {%d,%d,%d,%d}\n",
					ptrBlockData->IconTPoly.u[0],
					ptrBlockData->IconTPoly.u[1],
					ptrBlockData->IconTPoly.u[2],
					ptrBlockData->IconTPoly.u[3]);
					
					
			dprintf("\tV = {%d,%d,%d,%d}\n",
					ptrBlockData->IconTPoly.v[0],
					ptrBlockData->IconTPoly.v[1],
					ptrBlockData->IconTPoly.v[2],
					ptrBlockData->IconTPoly.v[3]);
					
			dprintf("\tBlock number: %d\n",ptrBlockData->Block);
			
			dprintf("\tBlock count: %d\n",ptrBlockData->BlockCount);
			
			dprintf("\tX = {%d,%d,%d,%d}\n",
					ptrBlockData->IconTPoly.x[0],
					ptrBlockData->IconTPoly.x[1],
					ptrBlockData->IconTPoly.x[2],
					ptrBlockData->IconTPoly.x[3]);
					
			dprintf("\tY = {%d,%d,%d,%d}\n",
					ptrBlockData->IconTPoly.y[0],
					ptrBlockData->IconTPoly.y[1],
					ptrBlockData->IconTPoly.y[2],
					ptrBlockData->IconTPoly.y[3]);
					
			dprintf("\tTPAGE = %d\n", ptrBlockData->IconTPoly.tpage);
			dprintf("\tCLUT = {%d, %d}\n",
					ptrBlockData->IconTPoly.cx,
					ptrBlockData->IconTPoly.cy);
		}
	}
	
	GsSortTPoly4(&ptrBlockData->IconTPoly);
	
	for(i = 0; i < 4; i++)
	{
		ptrBlockData->IconTPoly.u[i] = orig_u[i]; // Restore data
	}
	
	ptrBlockData->IconTPoly.cx = orig_clut_x;
}

TYPE_BLOCK_DATA * MemCardShowMap(void)
{
	uint8_t i;
	uint8_t j;
	uint8_t selectedBlock = BLOCK_1;
	uint8_t selectedSlot = SLOT_ONE;
	TYPE_BLOCK_DATA * ptrBlockData;
	GsRectangle emptyBlockRect;
	short x;
	short y;
	unsigned char orig_r;
	unsigned char orig_g;
	unsigned char orig_b;
	GsRectangle MemCardMapDialog;
	
	if(MemCardGetAllData() == false)
	{
		return false;
	}
	
	bzero((GsRectangle*)&MemCardMapDialog, sizeof(GsRectangle));
	bzero((GsRectangle*)&emptyBlockRect, sizeof(GsRectangle));
	
	emptyBlockRect.attribute |= ENABLE_TRANS | TRANS_MODE(1);
	
	MemCardMapDialog.x = MEMCARD_DIALOG_X;
	MemCardMapDialog.y = MEMCARD_DIALOG_Y;
	MemCardMapDialog.w = MEMCARD_DIALOG_W;
	MemCardMapDialog.h = MEMCARD_DIALOG_H;
	
	MemCardMapDialog.r = MEMCARD_DIALOG_R;
	MemCardMapDialog.g = MEMCARD_DIALOG_G;
	MemCardMapDialog.b = MEMCARD_DIALOG_B;
	
	MemCardMapDialog.attribute |= ENABLE_TRANS | TRANS_MODE(0);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	while(1)
	{	
		if(PadOneKeyReleased(PAD_TRIANGLE) == true)
		{
			break;
		}
		else if(PadOneKeyReleased(PAD_CROSS) == true)
		{
			return &MemCardData[selectedBlock - BLOCK_1][selectedSlot];
		}
		else if(PadOneKeyReleased(PAD_LEFT) == true)
		{
			if(selectedSlot == SLOT_TWO)
			{
				selectedSlot = SLOT_ONE;
			}
		}
		else if(PadOneKeyReleased(PAD_RIGHT) == true)
		{
			if(selectedSlot == SLOT_ONE)
			{
				selectedSlot = SLOT_TWO;
			}
		}
		else if(PadOneKeyReleased(PAD_UP) == true)
		{
			if(selectedBlock > BLOCK_1)
			{
				selectedBlock--;
			}
		}
		else if(PadOneKeyReleased(PAD_DOWN) == true)
		{
			if(selectedBlock < BLOCK_15)
			{
				selectedBlock++;
			}
		}
		
		// Dim background
		SecondDisplay.r = NORMAL_LUMINANCE >> 1;
		SecondDisplay.g = NORMAL_LUMINANCE >> 1;
		SecondDisplay.b = NORMAL_LUMINANCE >> 1;
		
		GfxSortSprite(&SecondDisplay);
		
		GsSortRectangle(&MemCardMapDialog);
		
		for(j = SLOT_ONE; j <= SLOT_TWO; j++)
		{
			if(MemCardStatus[j] == MEMCARD_STATUS_UNKNOWN)
			{
				FontSetFlags(&SmallFont, FONT_NOFLAGS);
				
				x = MEMCARD_DIALOG_X;
				x += MEMCARD_DIALOG_GAP_SLOT * j;
				
				y = MEMCARD_DIALOG_Y;
				y += (short)(MEMCARD_DIALOG_GAP_X << 1);
				y += MEMCARD_DIALOG_GAP_X;
				
				x += MEMCARD_DIALOG_GAP_X;
					
				FontPrintText(	&SmallFont,
								x,
								y,
								"Disconnected"			);
				continue;
			}
			
			for(i = BLOCK_1; i <= BLOCK_15; i++)
			{				
				ptrBlockData = &MemCardData[i - BLOCK_1][j];
				
				x = MEMCARD_DIALOG_X;
				x += MEMCARD_DIALOG_GAP_SLOT * j;
				x += ( (i - BLOCK_1) % 3) * MEMCARD_DIALOG_GAP_X;
				x += MEMCARD_DIALOG_GAP_X;
				
				y = MEMCARD_DIALOG_Y;
				y += (short)(MEMCARD_DIALOG_GAP_X * ((i - BLOCK_1) / 3));
				y += MEMCARD_DIALOG_GAP_X;
				
				if(ptrBlockData->BlockCount == EMPTY_BLOCK)
				{
					emptyBlockRect.x = x;
					emptyBlockRect.y = y;
					emptyBlockRect.w = MEMCARD_BLOCK_IMAGE_W;
					emptyBlockRect.h = MEMCARD_BLOCK_IMAGE_H;
					
					if(	(i == selectedBlock) && (j == selectedSlot) )
					{
						emptyBlockRect.r = FULL_LUMINANCE;
						emptyBlockRect.g = FULL_LUMINANCE;
						emptyBlockRect.b = FULL_LUMINANCE;
						
						FontSetFlags(&SmallFont, FONT_NOFLAGS);
					
						FontPrintText(	&SmallFont,
										MEMCARD_LOAD_DATA_TEXT_X,
										MEMCARD_LOAD_DATA_TEXT_Y,
										"Empty block"			);	
					}
					else
					{
						emptyBlockRect.r = NORMAL_LUMINANCE >> 1;
						emptyBlockRect.g = NORMAL_LUMINANCE >> 1;
						emptyBlockRect.b = NORMAL_LUMINANCE >> 1;
					}
					
					GsSortRectangle(&emptyBlockRect);
				
					continue;
				}
				
				orig_r = ptrBlockData->IconTPoly.r;
				orig_g = ptrBlockData->IconTPoly.g;
				orig_b = ptrBlockData->IconTPoly.b;
				
				if(	(i == selectedBlock) && (j == selectedSlot) )
				{
					ptrBlockData->IconTPoly.r = FULL_LUMINANCE;
					ptrBlockData->IconTPoly.g = FULL_LUMINANCE;
					ptrBlockData->IconTPoly.b = FULL_LUMINANCE;
					
					if(ptrBlockData->BlockCount == FIRST_OR_ONLY_BLOCK)
					{
						FontPrintText(	&SmallFont,
										MEMCARD_LOAD_DATA_TEXT_X,
										MEMCARD_LOAD_DATA_TEXT_Y,
										(char*)ptrBlockData->FileName	);	
					}
					else if(ptrBlockData->BlockCount == INTERMEDIATE_BLOCK)
					{
						FontPrintText(	&SmallFont,
										MEMCARD_LOAD_DATA_TEXT_X,
										MEMCARD_LOAD_DATA_TEXT_Y,
										"Intermediate block"	);	
					}
					else if(ptrBlockData->BlockCount == LAST_BLOCK)
					{
						FontPrintText(	&SmallFont,
										MEMCARD_LOAD_DATA_TEXT_X,
										MEMCARD_LOAD_DATA_TEXT_Y,
										"Last block"			);	
					}
				}
				else
				{
					ptrBlockData->IconTPoly.r = NORMAL_LUMINANCE >> 1;
					ptrBlockData->IconTPoly.g = NORMAL_LUMINANCE >> 1;
					ptrBlockData->IconTPoly.b = NORMAL_LUMINANCE >> 1;
				}
				
				MemCardDrawIcon(ptrBlockData, x, y);
				
				ptrBlockData->IconTPoly.r = orig_r;
				ptrBlockData->IconTPoly.g = orig_g;
				ptrBlockData->IconTPoly.b = orig_b;
			}
		}
		
		GfxDrawScene_Slow();
	}
	
	return NULL;
}

bool MemCardSaveData(TYPE_BLOCK_DATA * ptrBlockData)
{
	uint32_t i;
	uint32_t sz;
	int sector = (ptrBlockData->Block << MEMCARD_SECTORS_PER_BLOCK_BITSHIFT) + DATA_FRAME;
	
	// Always check whether current block is empty or not
	
	if(ptrBlockData->BlockCount != EMPTY_BLOCK)
	{
		if(strncmp((char*)ptrBlockData->FileName, MEMCARD_GAME_FILENAME, MEMCARD_FILENAME_SIZE) != 0)
		{
			// Only our own blocks can be overwritten. NEVER overwrite other game blocks!
			dprintf("I cannot erase blocks from other games!\n");
			return false;
		}
	}
	else if(ptrBlockData->BlockCount != FIRST_OR_ONLY_BLOCK)
	{
		dprintf("Please select first block of block array.\n");
		return false;
	}
	else if(ptrBlockData->Data == NULL)
	{
		dprintf("No data on current block!\n");
		return false;
	}
	else if(ptrBlockData->Block == DIRECTORY_BLOCK)
	{
		dprintf("Invalid block selected!\n");
		return false;
	}
	
	// After all these checks, now we can save data!
	
	sz = MEMCARD_FIRST_OR_LAST_DATA_SIZE;
	
	for(i = 0; i < sz; i++)
	{
		McWriteSector(ptrBlockData->Slot, sector + i, &ptrBlockData->Data[i << 7 /* 128 */]);
	}
	
	return true;
}

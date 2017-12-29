/* **************************************
 * 	Includes							*
 * *************************************/

#include "LoadMenu.h"
#include "Gfx.h"
#include "System.h"
#include "PltParser.h"
#include "Font.h"
#include "Sfx.h"

/* **************************************
 * 	Defines								*
 * *************************************/

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	SMALL_FONT_SIZE = 8,
    SMALL_FONT_SPACING = 6
};

enum
{
	BG_BLUE_TARGET_VALUE = 0xC0,
	BG_WHITE_TARGET_VALUE = /*0x40*/ 0,
	BG_INCREASE_STEP = 0x10
};

enum
{
	LOADING_BAR_X = 64,
	LOADING_BAR_Y = 200,
	LOADING_BAR_N_LINES = 4,

	LOADING_BAR_WIDTH = 256,
	LOADING_BAR_HEIGHT = 16,

	LOADING_BAR_LUMINANCE_TARGET = NORMAL_LUMINANCE,
	LOADING_BAR_LUMINANCE_STEP = 10
};

enum
{
	LOADING_TITLE_CLUT_X = 384,
	LOADING_TITLE_CLUT_Y = 496,
	LOADING_TITLE_X = 128,
	LOADING_TITLE_Y = 32,

	LOADING_TITLE_U = 0,
	LOADING_TITLE_V = 0,

	LOADING_TITLE_LUMINANCE_STEP = 10,
	LOADING_TITLE_LUMINANCE_TARGET = NORMAL_LUMINANCE
};

enum
{
	PLANE_START_X = 56,
	PLANE_START_Y = 200,

	PLANE_U = 0,
	PLANE_V = 32,
	PLANE_SIZE = 16,

	PLANE_LUMINANCE_STEP = 0x10,
	PLANE_LUMINANCE_TARGET_VALUE = NORMAL_LUMINANCE
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void LoadMenuInit(void);
static void ISR_LoadMenuVBlank(void);
static bool LoadMenuISRHasEnded(void);
static bool LoadMenuISRHasStarted(void);
static void LoadMenuLoadFileList(	const char* fileList[], 	void* dest[],
									uint8_t szFileList, uint8_t szDestList);

/* *************************************
 * 	Local Variables
 * *************************************/

static GsGPoly4 loadMenuBg;
static GsSprite LoadMenuPlaneSpr;
static GsSprite LoadMenuTitleSpr;
static GsLine LoadMenuBarLines[LOADING_BAR_N_LINES];
static GsRectangle LoadMenuBarRect;

static const char* LoadMenuFiles[] = {	"cdrom:\\DATA\\SPRITES\\PLANE.TIM;1",
                                        "cdrom:\\DATA\\SPRITES\\LOADING.TIM;1",
                                        "cdrom:\\DATA\\FONTS\\FONT_2.FNT;1"	};

static void* LoadMenuDest[] = { (GsSprite*)&LoadMenuPlaneSpr,
								(GsSprite*)&LoadMenuTitleSpr,
								(TYPE_FONT*)&SmallFont		};

static char* strCurrentFile;

// Flags to communicate with ISR state
// 	*	startup_flag: background fades in from black to blue.
// 	*	end_flag: tells the background to fade out to black.
//	*	isr_ended: background has totally faded out to black.
//	*	isr_started: tells the ISR has finished starting up.
static volatile bool startup_flag;
static volatile bool isr_started;
static volatile bool end_flag;
static volatile bool isr_ended;
// Set to true when LoadMenuInit() has been called, and set to false
// once LoadMenuEnd() is called.
// It's used when multiple modules call LoadMenu() at the same time,
// so load menu does not have to be initialised each time;
static bool load_menu_running;

void LoadMenuInit(void)
{
	int i;
	static bool first_load = false;

	if (first_load == false)
	{
		first_load = true;
		LoadMenuLoadFileList(	LoadMenuFiles,
								LoadMenuDest,
								sizeof(LoadMenuFiles) / sizeof(char*),
								sizeof(LoadMenuDest)	/ sizeof(void*));
	}

	FontSetSize(&SmallFont, SMALL_FONT_SIZE);
    FontSetSpacing(&SmallFont, SMALL_FONT_SPACING);

	LoadMenuPlaneSpr.r = 0;
	LoadMenuPlaneSpr.g = 0;
	LoadMenuPlaneSpr.b = 0;

	LoadMenuPlaneSpr.x = PLANE_START_X;
	LoadMenuPlaneSpr.y = PLANE_START_Y;

	// "Loading..." title init

	LoadMenuTitleSpr.r = 0;
	LoadMenuTitleSpr.g = 0;
	LoadMenuTitleSpr.b = 0;

	LoadMenuTitleSpr.x = LOADING_TITLE_X;
	LoadMenuTitleSpr.y = LOADING_TITLE_Y;

	LoadMenuTitleSpr.cx = LOADING_TITLE_CLUT_X;
	LoadMenuTitleSpr.cy = LOADING_TITLE_CLUT_Y;
	LoadMenuTitleSpr.u = LOADING_TITLE_U;
	LoadMenuTitleSpr.v = LOADING_TITLE_V;

	startup_flag = true;
	isr_started = false;
	end_flag = false;
	isr_ended = false;

	// Background init

	loadMenuBg.x[0] = 0;
	loadMenuBg.x[1] = X_SCREEN_RESOLUTION;
	loadMenuBg.x[2] = 0;
	loadMenuBg.x[3] = X_SCREEN_RESOLUTION;

	loadMenuBg.y[0] = 0;
	loadMenuBg.y[1] = 0;
	loadMenuBg.y[2] = Y_SCREEN_RESOLUTION;
	loadMenuBg.y[3] = Y_SCREEN_RESOLUTION;

	// Colour components adjustment (default to zero)
	for (i = 0; i < 4 ; i++)
	{
		loadMenuBg.r[i] = 0;
		loadMenuBg.g[i] = 0;
		loadMenuBg.b[i] = 0;
	}

	// "Loading" bar line 0 (up left - up right)

	LoadMenuBarLines[0].x[0] = LOADING_BAR_X;
	LoadMenuBarLines[0].x[1] = LOADING_BAR_X + LOADING_BAR_WIDTH;

	LoadMenuBarLines[0].y[0] = LOADING_BAR_Y;
	LoadMenuBarLines[0].y[1] = LOADING_BAR_Y;

	// "Loading" bar line 1 (up left - down left)

	LoadMenuBarLines[1].x[0] = LOADING_BAR_X;
	LoadMenuBarLines[1].x[1] = LOADING_BAR_X;

	LoadMenuBarLines[1].y[0] = LOADING_BAR_Y;
	LoadMenuBarLines[1].y[1] = LOADING_BAR_Y + LOADING_BAR_HEIGHT;

	// "Loading" bar line 2 (down left - down right)

	LoadMenuBarLines[2].x[0] = LOADING_BAR_X;
	LoadMenuBarLines[2].x[1] = LOADING_BAR_X + LOADING_BAR_WIDTH;

	LoadMenuBarLines[2].y[0] = LOADING_BAR_Y + LOADING_BAR_HEIGHT;
	LoadMenuBarLines[2].y[1] = LOADING_BAR_Y + LOADING_BAR_HEIGHT;

	// "Loading" bar line 3 (up right - down right)

	LoadMenuBarLines[3].x[0] = LOADING_BAR_X + LOADING_BAR_WIDTH;
	LoadMenuBarLines[3].x[1] = LOADING_BAR_X + LOADING_BAR_WIDTH;

	LoadMenuBarLines[3].y[0] = LOADING_BAR_Y;
	LoadMenuBarLines[3].y[1] = LOADING_BAR_Y + LOADING_BAR_HEIGHT;

	for (i = 0; i < LOADING_BAR_N_LINES ; i++)
	{
		LoadMenuBarLines[i].r = 0;
		LoadMenuBarLines[i].g = 0;
		LoadMenuBarLines[i].b = 0;
	}

	LoadMenuBarRect.r = 0;
	LoadMenuBarRect.g = 0;
	LoadMenuBarRect.b = 0;

	// LoadMenuBarRect.attribute |= ENABLE_TRANS | TRANS_MODE(0);

	LoadMenuBarRect.x = LOADING_BAR_X;
	LoadMenuBarRect.y = LOADING_BAR_Y;
	LoadMenuBarRect.w = 0;
	LoadMenuBarRect.h = LOADING_BAR_HEIGHT;

	LoadMenuBarRect.attribute |= ENABLE_TRANS | TRANS_MODE(0);

	load_menu_running = true;

	SmallFont.spr.r = 0;
	SmallFont.spr.g = 0;
	SmallFont.spr.b = 0;

	GfxSetGlobalLuminance(0);

    Serial_printf("I_MASK = 0x%08X\n", (*(unsigned int*)0x1F801074));

	SetVBlankHandler(&ISR_LoadMenuVBlank);
}

void LoadMenuEnd(void)
{
	end_flag = true;
	load_menu_running = false;

	while (LoadMenuISRHasEnded() == false);
	Serial_printf("Set default VBlank handler.\n");
	SetVBlankHandler(&ISR_SystemDefaultVBlank);

	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
}

void ISR_LoadMenuVBlank(void)
{
	uint8_t i;

    SystemIncreaseGlobalTimer();

    if (    (SystemIsBusy() != false)
                    ||
            (GfxIsGPUBusy() != false)
                    ||
            (SerialIsBusy() != false)   )
    {
        return;
    }

	/*if ( (SystemIsBusy() != false) || (GfxIsGPUBusy() != false) || (SerialIsBusy() != false) )
	{
		return;
	}*/

	if (startup_flag != false)
	{
		// "Loading..." text
		if (LoadMenuTitleSpr.r < LOADING_TITLE_LUMINANCE_TARGET)
		{
			LoadMenuTitleSpr.r += LOADING_TITLE_LUMINANCE_STEP;
			LoadMenuTitleSpr.g += LOADING_TITLE_LUMINANCE_STEP;
			LoadMenuTitleSpr.b += LOADING_TITLE_LUMINANCE_STEP;
		}

		if (loadMenuBg.g[0] < BG_WHITE_TARGET_VALUE)
		{
			loadMenuBg.r[0] += BG_INCREASE_STEP;
			loadMenuBg.r[1] += BG_INCREASE_STEP;

			loadMenuBg.g[0] += BG_INCREASE_STEP;
			loadMenuBg.g[1] += BG_INCREASE_STEP;

			loadMenuBg.b[0] += BG_INCREASE_STEP;
			loadMenuBg.b[1] += BG_INCREASE_STEP;
		}
		// Blue background
		if (loadMenuBg.b[2] < BG_BLUE_TARGET_VALUE)
		{
			loadMenuBg.b[2] += BG_INCREASE_STEP;
			loadMenuBg.b[3] += BG_INCREASE_STEP;
		}

		if (LoadMenuBarRect.r < LOADING_BAR_LUMINANCE_TARGET)
		{
			LoadMenuBarRect.r += LOADING_BAR_LUMINANCE_STEP;
			LoadMenuBarRect.g += LOADING_BAR_LUMINANCE_STEP;
			LoadMenuBarRect.b += LOADING_BAR_LUMINANCE_STEP;
		}
		else
		{
			startup_flag = false;
			isr_started = true;
		}

		for (i = 0;i < LOADING_BAR_N_LINES ; i++)
		{
			if (LoadMenuBarLines[i].r < LOADING_BAR_LUMINANCE_TARGET)
			{
				LoadMenuBarLines[i].r += LOADING_BAR_LUMINANCE_STEP;
				LoadMenuBarLines[i].g += LOADING_BAR_LUMINANCE_STEP;
				LoadMenuBarLines[i].b += LOADING_BAR_LUMINANCE_STEP;
			}
		}

		if (LoadMenuPlaneSpr.r < PLANE_LUMINANCE_TARGET_VALUE)
		{
			LoadMenuPlaneSpr.r += PLANE_LUMINANCE_STEP;
			LoadMenuPlaneSpr.g += PLANE_LUMINANCE_STEP;
			LoadMenuPlaneSpr.b += PLANE_LUMINANCE_STEP;
		}

	}
	else if (end_flag != false)
	{
		LoadMenuTitleSpr.r -= LOADING_TITLE_LUMINANCE_STEP;
		LoadMenuTitleSpr.g -= LOADING_TITLE_LUMINANCE_STEP;
		LoadMenuTitleSpr.b -= LOADING_TITLE_LUMINANCE_STEP;

		if (loadMenuBg.g[0] > 0)
		{
			loadMenuBg.r[0] -= BG_INCREASE_STEP;
			loadMenuBg.r[1] -= BG_INCREASE_STEP;

			loadMenuBg.g[0] -= BG_INCREASE_STEP;
			loadMenuBg.g[1] -= BG_INCREASE_STEP;

			loadMenuBg.b[0] -= BG_INCREASE_STEP;
			loadMenuBg.b[1] -= BG_INCREASE_STEP;
		}

		if (loadMenuBg.b[2] > 0)
		{
			loadMenuBg.b[2] -= BG_INCREASE_STEP;
			loadMenuBg.b[3] -= BG_INCREASE_STEP;
		}

		if (loadMenuBg.b[2] == 0)
		{
			end_flag = false;
			isr_ended = true;
		}

		if (LoadMenuPlaneSpr.r > 0)
		{
			LoadMenuPlaneSpr.r -= PLANE_LUMINANCE_STEP;
			LoadMenuPlaneSpr.g -= PLANE_LUMINANCE_STEP;
			LoadMenuPlaneSpr.b -= PLANE_LUMINANCE_STEP;
		}

		LoadMenuPlaneSpr.x = (PLANE_START_X + LOADING_BAR_WIDTH);
		LoadMenuPlaneSpr.y = PLANE_START_Y;

		LoadMenuBarRect.w = LOADING_BAR_WIDTH;

		if (LoadMenuBarRect.r > 0)
		{
			LoadMenuBarRect.r -= LOADING_BAR_LUMINANCE_STEP;
			LoadMenuBarRect.g -= LOADING_BAR_LUMINANCE_STEP;
			LoadMenuBarRect.b -= LOADING_BAR_LUMINANCE_STEP;
		}

		for (i = 0;i < LOADING_BAR_N_LINES ; i++)
		{
			if (LoadMenuBarLines[i].r > 0)
			{
				LoadMenuBarLines[i].r -= LOADING_BAR_LUMINANCE_STEP;
				LoadMenuBarLines[i].g -= LOADING_BAR_LUMINANCE_STEP;
				LoadMenuBarLines[i].b -= LOADING_BAR_LUMINANCE_STEP;
			}
		}
	}

	GsSortGPoly4(&loadMenuBg);

	GsSortRectangle(&LoadMenuBarRect);

	for (i = 0 ; i < LOADING_BAR_N_LINES ; i++)
	{
		GsSortLine(&LoadMenuBarLines[i]);
	}

	GsSortSprite(&LoadMenuTitleSpr);

	LoadMenuPlaneSpr.w = PLANE_SIZE;
	LoadMenuPlaneSpr.h = PLANE_SIZE;

	GsSortSprite(&LoadMenuPlaneSpr);

	FontSetFlags(&SmallFont, FONT_BLEND_EFFECT);

	FontPrintText(	&SmallFont,
					LOADING_BAR_X,
					LOADING_BAR_Y + LOADING_BAR_HEIGHT + 8,
					strCurrentFile	);

	GfxDrawScene_Fast();
}

bool LoadMenuISRHasEnded(void)
{
	return isr_ended;
}

bool LoadMenuISRHasStarted(void)
{
	return isr_started;
}

void LoadMenu(	const char*	fileList[],
				void* dest[],
				uint8_t szFileList	, uint8_t szDestList)
{

	if (load_menu_running == false)
	{
		LoadMenuInit();

		while (LoadMenuISRHasStarted() == false);
	}

	LoadMenuLoadFileList(fileList, dest, szFileList, szDestList);
}

void LoadMenuLoadFileList(	const char* fileList[], 	void* dest[],
							uint8_t szFileList, uint8_t szDestList)
{
	char aux_file_name[100];
	char* extension;
	short x_increment;
	uint8_t fileLoadedCount;

	if (szFileList != szDestList)
	{
		Serial_printf("File list size different from dest list size! %d vs %d\n",
				szFileList, szDestList);
		return;
	}

	for (fileLoadedCount = 0; fileLoadedCount < szFileList ; fileLoadedCount++)
	{
        strCurrentFile = (char*)fileList[fileLoadedCount];

		if (strCurrentFile == NULL)
		{
			continue;
		}

		x_increment = (short)(LOADING_BAR_WIDTH / szFileList);

		// Calculate new X position for loading menu plane sprite.
		// This is not calculated on ISR as to avoid longer ISR time.
		LoadMenuPlaneSpr.x = (PLANE_START_X + (fileLoadedCount* x_increment) );

		LoadMenuBarRect.w = fileLoadedCount* x_increment;

		//Serial_printf("Files %d / %d loaded. New plane X = %d.\n",fileLoadedCount,szFileList,LoadMenuPlaneSpr.x);

		// Backup original file path
		strncpy(aux_file_name, strCurrentFile, 100);

		//We want to get file extension, so split into tokens
		strtok(strCurrentFile, ".;");
		extension = strtok(NULL, ".;");

		Serial_printf("File extension: .%s\n", extension);
		//Restore original file path in order to load file
		strncpy(strCurrentFile, aux_file_name, 100);

		if (strncmp(extension,"TIM",3) == 0)
		{
			if (GfxSpriteFromFile(strCurrentFile, dest[fileLoadedCount]) == false)
			{
				Serial_printf("Could not load image file \"%s\"!\n", strCurrentFile);
			}
		}
		else if (strncmp(extension,"CLT",3) == 0)
		{
			if (dest[fileLoadedCount] != NULL)
			{
				Serial_printf("WARNING: File %s linked to non-NULL destination pointer!\n", dest[fileLoadedCount]);
			}

			if (GfxCLUTFromFile(strCurrentFile) == false)
			{
				Serial_printf("Could not load CLUT file \"%s\"!\n", strCurrentFile);
			}
		}
		else if (strncmp(extension,"VAG",3) == 0)
		{
			if (SfxUploadSound(strCurrentFile, dest[fileLoadedCount]) == false)
			{
				Serial_printf("Could not load sound file \"%s\"!\n", strCurrentFile);
			}
		}
		else if (strncmp(extension,"FNT",3) == 0)
		{
			if (FontLoadImage(strCurrentFile, dest[fileLoadedCount]) == false)
			{
				Serial_printf("Could not load font file \"%s\"!\n", strCurrentFile);
			}
		}
		else if (strncmp(extension,"PLT",3) == 0)
		{
			if (PltParserLoadFile(strCurrentFile, dest[fileLoadedCount]) == false)
			{
				Serial_printf("Could not load pilots file \"%s\"!\n", strCurrentFile);
			}
		}
		else
		{
			Serial_printf("LoadMenu does not recognize following extension: %s\n",extension);
		}
	}
}

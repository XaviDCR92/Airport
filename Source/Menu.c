/* **************************************
 * 	Includes							*
 * *************************************/

#include "Menu.h"
#include "Global_Inc.h"
#include "System.h"
#include "Gfx.h"
#include "Sfx.h"
#include "MainMenuBtnAni.h"
#include "LoadMenu.h"
#include "Game.h"
#include "EndAnimation.h"
#include "PSXSDKIntro.h"
#include "MemCard.h"
#include "Serial.h"
#include "Pad.h"

/* **************************************
 * 	Defines								*
 * *************************************/

#define MAIN_MENU_BUTTON_SIZE 64
#define SELECTED_BUTTON_LUMINANCE 0xC0

/* **************************************
 * 	Structs and enums					*
 * *************************************/

typedef enum t_levelId
{
    LEVEL1 = 0,
    LEVEL2,
    MAX_LEVELS
}LEVEL_ID;

typedef struct t_lvlpltdata
{
    LEVEL_ID levelID;
    const char** fileNames[];
}TYPE_LVL_PLT_DATA;

typedef enum
{
	PLAY_OPTIONS_LEVEL,
	ONE_TWO_PLAYERS_LEVEL,
    LEVEL_LIST_LEVEL,
	OPTIONS_LEVEL
}MainMenuLevel;

enum
{
	MAIN_MENU_PLAY_OPTIONS_LEVEL_BUTTONS = 2,
	MAIN_MENU_ONE_TWO_PLAYERS_LEVEL_BUTTONS = 2,
	MAIN_MENU_ONE_TWO_PLAYERS_LEVEL_BUTTONS_PAD_TWO_DISCONNECTED = 1,
	MAIN_MENU_OPTIONS_LEVEL_BUTTONS = 1
};

typedef enum
{
	PLAY_BUTTON_INDEX,
	OPTIONS_BUTTON_INDEX,
	ONE_PLAYER_BUTTON_INDEX,
	TWO_PLAYER_BUTTON_INDEX,

	MAIN_MENU_BUTTONS_MAX
}MMBtn_Index;

enum
{
	MAIN_MENU_PLAY_BUTTON_X = 92,
	MAIN_MENU_PLAY_BUTTON_Y = 92,

	MAIN_MENU_OPTIONS_BUTTON_X = 225,
	MAIN_MENU_OPTIONS_BUTTON_Y = 92,

	MAIN_MENU_ONE_PLAYER_BUTTON_X = 92,
	MAIN_MENU_ONE_PLAYER_BUTTON_Y = 92,

	MAIN_MENU_TWO_PLAYER_BUTTON_X = 225,
	MAIN_MENU_TWO_PLAYER_BUTTON_Y = 92
};

typedef enum
{
	PLAY_BUTTON_U_OFFSET = 0,
	PLAY_BUTTON_Y_OFFSET = 64,

	ONE_PLAYER_BUTTON_U_OFFSET = 128,
	ONE_PLAYER_BUTTON_Y_OFFSET = 64,

	OPTIONS_BUTTON_U_OFFSET = 64,
	OPTIONS_BUTTON_Y_OFFSET = 64,

	TWO_PLAYER_BUTTON_U_OFFSET = 192,
	TWO_PLAYER_BUTTON_Y_OFFSET = 64,

	DEFAULT_BUTTON_U_OFFSET = 0,
	DEFAULT_BUTTON_V_OFFSET = 128

}MMBtn_Offset;

typedef struct
{
	MMBtn_Offset offset_u;
	MMBtn_Offset offset_v;

	// Timer for absolute, sine-like animation
	short timer;

	// Pointer to function executed by pressing the button
	void (*f)();

	MMBtn_Index i;

	bool selected;
	bool was_selected;

}TYPE_MMBtn;

/* **************************************
 * 	Local prototypes					*
 * **************************************/

static void MenuCheatInit(void);
static void MainMenuDrawButton(TYPE_MMBtn * btn);
static void PlayMenu(void);
static void OptionsMenu(void);
static void OnePlayerMenu(void);
static void TwoPlayerMenu(void);
static void MainMenuButtonHandler(void);
static void MainMenuRestoreInitValues(void);
static void MenuTestCheat(void);
static void MainMenuRenderLevelList(void);

/* **************************************
 * 	Local variables						*
 * **************************************/

static GsSprite MenuSpr;
static GsSprite MenuStarSpr;
static SsVag BellSnd;
static SsVag AcceptSnd;
static TYPE_CHEAT TestCheat;
static TYPE_CHEAT StackCheckCheat;
static TYPE_CHEAT DevMenuCheat;
static TYPE_CHEAT SerialCheat;
static volatile bool BcnGWSpr_set;
static LEVEL_ID SelectedLevel;
static uint8_t SelectedPlt;
static bool isLevelSelected;

static const char* MainMenuFiles[] = {	"cdrom:\\DATA\\SPRITES\\MAINMENU.TIM;1"	,
                                        "cdrom:\\DATA\\SOUNDS\\BELL.VAG;1"		,
                                        "cdrom:\\DATA\\SOUNDS\\ACCEPT.VAG;1"	,
                                        "cdrom:\\DATA\\SPRITES\\BUTTONS.TIM;1"	,
                                        "cdrom:\\DATA\\SPRITES\\MENUSTAR.TIM;1"	,
#ifndef NO_INTRO
                                        "cdrom:\\DATA\\SPRITES\\PSXDISK.TIM;1"	,
                                        "cdrom:\\DATA\\FONTS\\INTROFNT.TIM;1"	,
                                        "cdrom:\\DATA\\SPRITES\\GPL.TIM;1"		,
                                        "cdrom:\\DATA\\SPRITES\\OPENSRC.TIM;1"	,
                                        "cdrom:\\DATA\\SOUNDS\\TRAYCL.VAG;1"	,
                                        "cdrom:\\DATA\\SOUNDS\\SPINDISK.VAG;1"
#endif // NO_INTRO
                                                                                };

static const char* MainMenuLevelList[] = {  [LEVEL1] = "cdrom:\\DATA\\LEVELS\\LEVEL1.LVL;1"	,
                                            [LEVEL2] = "cdrom:\\DATA\\LEVELS\\LEVEL2.LVL;1"	};

static const char* MainMenuLevel1Plt[] = {	"cdrom:\\DATA\\LEVELS\\TUTORIA1.PLT;1",
											"cdrom:\\DATA\\LEVELS\\LEVEL1.PLT;1",
											"cdrom:\\DATA\\LEVELS\\EASY.PLT;1",
											NULL};

static const char* MainMenuLevel2Plt[] = {"cdrom:\\DATA\\LEVELS\\LEVEL2.PLT;1", NULL};

static const char** MainMenuPltList[] = {[LEVEL1] = MainMenuLevel1Plt, [LEVEL2] = MainMenuLevel2Plt};

static TYPE_GAME_CONFIGURATION GameCfg;

static void* MainMenuDest[] = {     (GsSprite*)&MenuSpr			,
									(SsVag*)&BellSnd			,
									(SsVag*)&AcceptSnd			,
									(GsSprite*)&PSXButtons		,
									(GsSprite*)&MenuStarSpr		,
#ifndef NO_INTRO
                                    (GsSprite*)&PsxDisk			,
                                    (GsSprite*)&PSXSDKIntroFont	,
									(GsSprite*)&GPL_Logo		,
									(GsSprite*)&OpenSource_Logo	,
									(SsVag*)&TrayClSnd			,
									(SsVag*)&SpinDiskSnd
#endif // NO_INTRO
									};

static TYPE_MMBtn MainMenuBtn[MAIN_MENU_BUTTONS_MAX];
static MainMenuLevel menuLevel;
static MMBtn_Index MainMenuMinimumBtn;

void PlayMenu(void)
{
	menuLevel = ONE_TWO_PLAYERS_LEVEL;
	MainMenuMinimumBtn = ONE_PLAYER_BUTTON_INDEX;

	MainMenuBtn[PLAY_BUTTON_INDEX].selected = false;
	MainMenuBtn[PLAY_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[PLAY_BUTTON_INDEX].timer = 0;

	MainMenuBtn[OPTIONS_BUTTON_INDEX].selected = false;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].timer = 0;

	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].selected = true;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].timer = 0;

	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].selected = false;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].timer = 0;
}

void OptionsMenu(void)
{
	Serial_printf("OptionsMenu(void)!\n");
}

void OnePlayerMenu(void)
{
    menuLevel = LEVEL_LIST_LEVEL;
    GameCfg.TwoPlayers = false;
	//EndAnimation();
	//Game(false /* One Player Only */);
}

void TwoPlayerMenu(void)
{
    menuLevel = LEVEL_LIST_LEVEL;
    GameCfg.TwoPlayers = true;
	//EndAnimation();
	//Game(true /* Two players */);
}


void MainMenuInit(void)
{
    enum
    {
        MENU_STAR_X = 32,
        MENU_STAR_Y = Y_SCREEN_RESOLUTION - 32,
    };

	LoadMenu(	MainMenuFiles,
                MainMenuDest,
				sizeof (MainMenuFiles) / sizeof (char*) ,
				sizeof (MainMenuDest) / sizeof (void*) );

	MainMenuBtn[PLAY_BUTTON_INDEX].offset_u = PLAY_BUTTON_U_OFFSET;
	MainMenuBtn[PLAY_BUTTON_INDEX].offset_v = PLAY_BUTTON_Y_OFFSET;
	MainMenuBtn[PLAY_BUTTON_INDEX].timer = 0;
	MainMenuBtn[PLAY_BUTTON_INDEX].f = &PlayMenu;
	MainMenuBtn[PLAY_BUTTON_INDEX].i = PLAY_BUTTON_INDEX;

	MainMenuBtn[OPTIONS_BUTTON_INDEX].offset_u = OPTIONS_BUTTON_U_OFFSET;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].offset_v = OPTIONS_BUTTON_Y_OFFSET;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].timer = 0;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].f = &OptionsMenu;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].i = OPTIONS_BUTTON_INDEX;

	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].offset_u = ONE_PLAYER_BUTTON_U_OFFSET;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].offset_v = ONE_PLAYER_BUTTON_Y_OFFSET;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].timer = 0;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].f = &OnePlayerMenu;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].i = ONE_PLAYER_BUTTON_INDEX;

	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].offset_u = TWO_PLAYER_BUTTON_U_OFFSET;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].offset_v = TWO_PLAYER_BUTTON_Y_OFFSET;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].timer = 0;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].f = &TwoPlayerMenu;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].i = TWO_PLAYER_BUTTON_INDEX;

	menuLevel = PLAY_OPTIONS_LEVEL;

	MainMenuMinimumBtn = PLAY_BUTTON_INDEX;

    MenuStarSpr.x = MENU_STAR_X;
    MenuStarSpr.y = MENU_STAR_Y;
    MenuStarSpr.mx = MenuStarSpr.w >> 1;
    MenuStarSpr.my = MenuStarSpr.h >> 1;
    MenuStarSpr.rotate = 0;

    SelectedLevel = LEVEL1;

    MenuCheatInit();

	LoadMenuEnd();
}

void MenuCheatInit(void)
{
    TestCheat.Callback = &MenuTestCheat;
	memset(TestCheat.Combination,0,CHEAT_ARRAY_SIZE);
	//memcpy(myarray, (int [5]){a,b,c,d,e}, 5*sizeof (int));

	memcpy(	TestCheat.Combination,
			(unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_CIRCLE, PAD_CIRCLE, PAD_CROSS, PAD_TRIANGLE,
				PAD_TRIANGLE, PAD_TRIANGLE, 0 , 0 ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

	PadAddCheat(&TestCheat);

	StackCheckCheat.Callback = &SystemPrintStackPointerAddress;
	memset(StackCheckCheat.Combination, 0, CHEAT_ARRAY_SIZE);

	memcpy(	StackCheckCheat.Combination,
			(unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_TRIANGLE, PAD_TRIANGLE, PAD_CROSS, PAD_TRIANGLE,
				PAD_L1, PAD_R1, 0 , 0 ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

	PadAddCheat(&StackCheckCheat);

    DevMenuCheat.Callback = &SystemDevMenuToggle;
    memset(DevMenuCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memcpy( DevMenuCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_SQUARE, PAD_SQUARE, PAD_CROSS, PAD_CROSS,
				PAD_CIRCLE, PAD_CIRCLE, PAD_TRIANGLE , PAD_TRIANGLE ,
				PAD_L1, PAD_L1, PAD_R1, PAD_R1,
				PAD_L2, PAD_L2, PAD_R2, PAD_R2	} ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&DevMenuCheat);

    SerialCheat.Callback = &SystemReturnToLoader;
    memset(SerialCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memcpy( SerialCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE,
				PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE , PAD_CIRCLE ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&SerialCheat);
}

void MainMenu(void)
{
	MainMenuInit();

#ifndef NO_INTRO
	PSXSDKIntro();
#endif //PSXSDK_DEBUG

	GfxSetGlobalLuminance(NORMAL_LUMINANCE);

	while (1)
	{
        enum
        {
            MAIN_MENU_BG_R = 0,
            MAIN_MENU_BG_G = 0,
            MAIN_MENU_BG_B = 40
        };

		MainMenuButtonHandler();

        GsSortCls(MAIN_MENU_BG_R, MAIN_MENU_BG_G, MAIN_MENU_BG_B);

        MenuStarSpr.rotate += ROTATE_ONE;

		switch(menuLevel)
		{
			case PLAY_OPTIONS_LEVEL:
				MainMenuDrawButton(&MainMenuBtn[PLAY_BUTTON_INDEX]);
				MainMenuDrawButton(&MainMenuBtn[OPTIONS_BUTTON_INDEX]);
			break;

			case ONE_TWO_PLAYERS_LEVEL:
				MainMenuDrawButton(&MainMenuBtn[ONE_PLAYER_BUTTON_INDEX]);
				MainMenuDrawButton(&MainMenuBtn[TWO_PLAYER_BUTTON_INDEX]);
			break;

            case LEVEL_LIST_LEVEL:
                MainMenuRenderLevelList();
            break;

			default:
			break;
		}

        GfxDrawScene_Slow();
	}
}

void MainMenuRenderLevelList(void)
{
    enum
    {
        LEVEL_LIST_RECT_X = 16,
        LEVEL_LIST_RECT_Y = 16,
        LEVEL_LIST_RECT_W = 128,
        LEVEL_LIST_RECT_H = 128,

        LEVEL_LIST_RECT_B0 = 48,
        LEVEL_LIST_RECT_B1 = LEVEL_LIST_RECT_B0,
        LEVEL_LIST_RECT_B2 = 128,
        LEVEL_LIST_RECT_B3 = LEVEL_LIST_RECT_B2,
    };

    enum
    {
        LEVEL_LIST_PLT_RECT_X = LEVEL_LIST_RECT_X,
        LEVEL_LIST_PLT_RECT_Y = LEVEL_LIST_RECT_Y + LEVEL_LIST_RECT_H + 16,
        LEVEL_LIST_PLT_RECT_H = 64,
        LEVEL_LIST_PLT_RECT_W = LEVEL_LIST_RECT_W,
    };

    enum
    {
        LEVEL_LIST_TEXT_X = LEVEL_LIST_RECT_X + 8,
        LEVEL_LIST_TEXT_Y = LEVEL_LIST_RECT_Y + 8,

        LEVEL_LIST_PLT_TEXT_X = LEVEL_LIST_PLT_RECT_X + 8,
        LEVEL_LIST_PLT_TEXT_Y = LEVEL_LIST_PLT_RECT_Y + 8,
    };

    enum
    {
        LEVEL_LIST_SELECTION_RECT_X = LEVEL_LIST_TEXT_X,
        LEVEL_LIST_SELECTION_RECT_Y = LEVEL_LIST_TEXT_Y,
        LEVEL_LIST_SELECTION_RECT_W = LEVEL_LIST_RECT_W - 16,
        LEVEL_LIST_SELECTION_RECT_H = 8,
    };

    GsGPoly4 levelListRect = {0};
    GsGPoly4 levelListSelectionRect = {0};
    LEVEL_ID i;

    levelListRect.x[0] = LEVEL_LIST_RECT_X;
    levelListRect.x[1] = LEVEL_LIST_RECT_X + LEVEL_LIST_RECT_W;
    levelListRect.x[2] = levelListRect.x[0];
    levelListRect.x[3] = levelListRect.x[1];

    levelListRect.y[0] = LEVEL_LIST_RECT_Y;
    levelListRect.y[1] = levelListRect.y[0];
    levelListRect.y[2] = LEVEL_LIST_RECT_Y + LEVEL_LIST_RECT_H;
    levelListRect.y[3] = levelListRect.y[2];

    levelListRect.b[0] = LEVEL_LIST_RECT_B0;
    levelListRect.b[1] = LEVEL_LIST_RECT_B1;
    levelListRect.b[2] = LEVEL_LIST_RECT_B2;
    levelListRect.b[3] = LEVEL_LIST_RECT_B3;

    GsSortGPoly4(&levelListRect);

    levelListRect.x[0] = LEVEL_LIST_PLT_RECT_X;
    levelListRect.x[1] = LEVEL_LIST_PLT_RECT_X + LEVEL_LIST_PLT_RECT_W;
    levelListRect.x[2] = levelListRect.x[0];
    levelListRect.x[3] = levelListRect.x[1];

    levelListRect.y[0] = LEVEL_LIST_PLT_RECT_Y;
    levelListRect.y[1] = levelListRect.y[0];
    levelListRect.y[2] = LEVEL_LIST_PLT_RECT_Y + LEVEL_LIST_PLT_RECT_H;
    levelListRect.y[3] = levelListRect.y[2];

    levelListRect.b[0] = LEVEL_LIST_RECT_B0;
    levelListRect.b[1] = LEVEL_LIST_RECT_B1;
    levelListRect.b[2] = LEVEL_LIST_RECT_B2;
    levelListRect.b[3] = LEVEL_LIST_RECT_B3;

    GsSortGPoly4(&levelListRect);

    levelListSelectionRect.x[0] = LEVEL_LIST_SELECTION_RECT_X;
    levelListSelectionRect.x[1] = LEVEL_LIST_SELECTION_RECT_X + LEVEL_LIST_SELECTION_RECT_W;
    levelListSelectionRect.x[2] = levelListSelectionRect.x[0];
    levelListSelectionRect.x[3] = levelListSelectionRect.x[1];

    levelListSelectionRect.y[0] = LEVEL_LIST_SELECTION_RECT_Y + (short)(SelectedLevel << 3);
    levelListSelectionRect.y[1] = levelListSelectionRect.y[0];
    levelListSelectionRect.y[2] = LEVEL_LIST_SELECTION_RECT_Y + LEVEL_LIST_SELECTION_RECT_H + (short)(SelectedLevel << 3);
    levelListSelectionRect.y[3] = levelListSelectionRect.y[2];

    levelListSelectionRect.r[0] = LEVEL_LIST_RECT_B0;
    levelListSelectionRect.r[1] = LEVEL_LIST_RECT_B1;
    levelListSelectionRect.r[2] = LEVEL_LIST_RECT_B2;
    levelListSelectionRect.r[3] = LEVEL_LIST_RECT_B3;

    levelListSelectionRect.g[0] = LEVEL_LIST_RECT_B0;
    levelListSelectionRect.g[1] = LEVEL_LIST_RECT_B1;
    levelListSelectionRect.g[2] = LEVEL_LIST_RECT_B2;
    levelListSelectionRect.g[3] = LEVEL_LIST_RECT_B3;

    levelListSelectionRect.b[0] = LEVEL_LIST_RECT_B0;
    levelListSelectionRect.b[1] = LEVEL_LIST_RECT_B1;
    levelListSelectionRect.b[2] = LEVEL_LIST_RECT_B2;
    levelListSelectionRect.b[3] = LEVEL_LIST_RECT_B3;

    levelListSelectionRect.attribute |= ENABLE_TRANS | TRANS_MODE(0);

    for (i = LEVEL1; i < MAX_LEVELS; i++)
    {
        char baseName[32];
        const size_t maxLength = sizeof (baseName) / sizeof (baseName[0]);

        // Update "baseName" with file name + extension.
        SystemGetFileBasename(MainMenuLevelList[i], baseName, maxLength);

        FontPrintText(&SmallFont, LEVEL_LIST_TEXT_X, LEVEL_LIST_TEXT_Y + (i << 3), baseName);

        if (i == SelectedLevel)
        {
			const char** ptrPltLevelArray;
			size_t j;

            for (ptrPltLevelArray = MainMenuPltList[i], j = 0; *ptrPltLevelArray != NULL; ptrPltLevelArray++, j++)
            {
				const char* strPltLevel = *ptrPltLevelArray;

                if (strPltLevel != NULL)
                {
                    // Update "baseName" with file name + extension.
                    SystemGetFileBasename(strPltLevel, baseName, maxLength);

                    FontPrintText(	&SmallFont,
									LEVEL_LIST_PLT_TEXT_X,
									LEVEL_LIST_PLT_TEXT_Y + (j << 3),
									baseName	);
                }
            }
        }
    }

    if (isLevelSelected == false)
    {
        GsSortGPoly4(&levelListSelectionRect);
    }
    else
    {
        levelListSelectionRect.y[0] = LEVEL_LIST_PLT_TEXT_Y + (short)(SelectedPlt << 3);
        levelListSelectionRect.y[1] = levelListSelectionRect.y[0];
        levelListSelectionRect.y[2] = LEVEL_LIST_PLT_TEXT_Y + LEVEL_LIST_SELECTION_RECT_H + (short)(SelectedPlt << 3);
        levelListSelectionRect.y[3] = levelListSelectionRect.y[2];

        GsSortGPoly4(&levelListSelectionRect);
    }
}

void MainMenuRestoreInitValues(void)
{
    uint8_t i;

	menuLevel = PLAY_OPTIONS_LEVEL;
	MainMenuMinimumBtn = PLAY_BUTTON_INDEX;

	MainMenuBtn[PLAY_BUTTON_INDEX].selected = true;
	MainMenuBtn[PLAY_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[PLAY_BUTTON_INDEX].timer = 0;

	MainMenuBtn[OPTIONS_BUTTON_INDEX].selected = false;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[OPTIONS_BUTTON_INDEX].timer = 0;

	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].selected = false;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[ONE_PLAYER_BUTTON_INDEX].timer = 0;

	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].selected = false;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].was_selected = false;
	MainMenuBtn[TWO_PLAYER_BUTTON_INDEX].timer = 0;

    SelectedLevel = LEVEL1;
    SelectedPlt = 0;

    memset(&GameCfg, 0, sizeof (TYPE_GAME_CONFIGURATION));

    for (i = 0; i < MAIN_MENU_BUTTONS_MAX; i++)
    {
        MainMenuBtn[i].was_selected = false;
    }

	GfxSetGlobalLuminance(NORMAL_LUMINANCE);

    SfxPlayTrack(INTRO_TRACK);
}

void MainMenuButtonHandler(void)
{
	static uint8_t btn_selected = PLAY_BUTTON_INDEX;
	static uint8_t previous_btn_selected = 0;
	uint8_t max_buttons = 0;

	if (PadOneAnyKeyPressed() != false)
	{
		if (SystemIsRandSeedSet() == false)
		{
			SystemSetRandSeed();
		}
	}

	if (	(PadOneKeySinglePress(PAD_CROSS) != false)
				||
            (PadOneKeySinglePress(PAD_TRIANGLE) != false)	)
	{
		SfxPlaySound(&AcceptSnd);
	}

	switch(menuLevel)
	{
		case PLAY_OPTIONS_LEVEL:
			max_buttons = MAIN_MENU_PLAY_OPTIONS_LEVEL_BUTTONS;
		break;

		case ONE_TWO_PLAYERS_LEVEL:

            if ( (btn_selected == TWO_PLAYER_BUTTON_INDEX)
                            &&
                (PadTwoConnected() == false)                )
            {
                max_buttons = MAIN_MENU_ONE_TWO_PLAYERS_LEVEL_BUTTONS_PAD_TWO_DISCONNECTED;
            }
            else
            {
                max_buttons = MAIN_MENU_ONE_TWO_PLAYERS_LEVEL_BUTTONS;
            }

			if (PadOneKeySinglePress(PAD_TRIANGLE) != false)
			{
				menuLevel = PLAY_OPTIONS_LEVEL;
				MainMenuMinimumBtn = PLAY_BUTTON_INDEX;
				btn_selected = PLAY_BUTTON_INDEX;
			}

		break;

        case LEVEL_LIST_LEVEL:
            if (PadOneKeySinglePress(PAD_UP) != false)
            {
                if (isLevelSelected == false)
                {
                    if (SelectedLevel > 0)
                    {
                        SelectedLevel--;
                        SelectedPlt = 0;
                    }
                }
                else
                {
                    if (SelectedPlt > 0)
                    {
                        SelectedPlt--;
                    }
                }
            }
            else if (PadOneKeySinglePress(PAD_DOWN) != false)
            {
                if (isLevelSelected == false)
                {
                    if (SelectedLevel < (MAX_LEVELS - 1))
                    {
                        SelectedLevel++;
                        SelectedPlt = 0;
                    }
                }
                else
                {
                    if (MainMenuPltList[SelectedLevel][SelectedPlt + 1] != NULL)
                    {
                        SelectedPlt++;
                    }
                }
            }
            else if (PadOneKeySinglePress(PAD_TRIANGLE) != false)
            {
                if (isLevelSelected == true)
                {
                    isLevelSelected = false;
                }
                else
                {
                    MainMenuRestoreInitValues();
                }
            }
        break;

		default:
			max_buttons = 0;
		break;
	}

    //DEBUG_PRINT_VAR(btn_selected);

	MainMenuBtn[previous_btn_selected].was_selected = MainMenuBtn[previous_btn_selected].selected;
	MainMenuBtn[btn_selected].was_selected = MainMenuBtn[btn_selected].selected;

	if (PadOneKeySinglePress(PAD_LEFT)	&& (btn_selected > 0) )
	{
		MainMenuBtn[btn_selected].selected = false;
		previous_btn_selected = btn_selected;
		btn_selected--;
		SfxPlaySound(&BellSnd);
	}
	else if (PadOneKeySinglePress(PAD_RIGHT)
				&&
			(btn_selected < (max_buttons - 1 + MainMenuMinimumBtn) ) )
	{
		MainMenuBtn[btn_selected].selected = false;
		previous_btn_selected = btn_selected;
		btn_selected++;
		SfxPlaySound(&BellSnd);
	}

	if (btn_selected < MainMenuMinimumBtn)
	{
		btn_selected = MainMenuMinimumBtn;
	}

	if (btn_selected > (max_buttons - 1 + MainMenuMinimumBtn) )
	{
		// Avoid overflow when going back in menu navigation
		btn_selected = (max_buttons - 1 + MainMenuMinimumBtn);
	}

	if (PadOneKeySinglePress(PAD_CROSS) )
	{
        if (menuLevel == LEVEL_LIST_LEVEL)
		{
            if (isLevelSelected == false)
            {
                isLevelSelected = true;
            }
            else
            {
                GameCfg.LVLPath = MainMenuLevelList[SelectedLevel];
                GameCfg.PLTPath = MainMenuPltList[SelectedLevel][SelectedPlt];

                EndAnimation();

                // Start gameplay!
                Game(&GameCfg);

                MainMenuRestoreInitValues();
                btn_selected = PLAY_BUTTON_INDEX;
                isLevelSelected = false;
            }
		}
        else
        {
            MainMenuBtn[btn_selected].f();
        }
	}

	MainMenuBtn[btn_selected].selected = true;
}

void MainMenuDrawButton(TYPE_MMBtn * btn)
{
	MenuSpr.w = MAIN_MENU_BUTTON_SIZE;
	MenuSpr.h = MAIN_MENU_BUTTON_SIZE;

	if ( (btn->timer) < (MainMenuBtnAni_sz - 1) )
	{
		btn->timer++;
	}

	if (btn->selected != false)
	{
		if (btn->was_selected == false)
		{
			btn->timer = 0;
		}

		MenuSpr.r = SELECTED_BUTTON_LUMINANCE;
		MenuSpr.g = SELECTED_BUTTON_LUMINANCE;
		MenuSpr.b = SELECTED_BUTTON_LUMINANCE;
	}
	else
	{
		MenuSpr.r = NORMAL_LUMINANCE;
		MenuSpr.g = NORMAL_LUMINANCE;
		MenuSpr.b = NORMAL_LUMINANCE;
	}

	MenuSpr.u = DEFAULT_BUTTON_U_OFFSET;
	MenuSpr.v = DEFAULT_BUTTON_V_OFFSET;

	switch(btn->i)
	{
		case PLAY_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_PLAY_BUTTON_X;
			MenuSpr.y = MAIN_MENU_PLAY_BUTTON_Y;
		break;

		case OPTIONS_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_OPTIONS_BUTTON_X;
			MenuSpr.y = MAIN_MENU_OPTIONS_BUTTON_Y;
		break;

		case ONE_PLAYER_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_ONE_PLAYER_BUTTON_X;
			MenuSpr.y = MAIN_MENU_ONE_PLAYER_BUTTON_Y;
		break;

		case TWO_PLAYER_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_TWO_PLAYER_BUTTON_X;
			MenuSpr.y = MAIN_MENU_TWO_PLAYER_BUTTON_Y;

            // Exception: turn option dimmer if second player pad isn't connected

            if (PadTwoConnected() == false)
            {
                MenuSpr.r = NORMAL_LUMINANCE >> 1;
                MenuSpr.g = NORMAL_LUMINANCE >> 1;
                MenuSpr.b = NORMAL_LUMINANCE >> 1;
            }

		break;

		default:
		break;
	}

    MenuSpr.u += btn->offset_u;
    MenuSpr.v += btn->offset_v;
    MenuSpr.y -= MainMenuBtnAni[btn->timer];
    GsSortSprite(&MenuSpr);
}

void MenuTestCheat(void)
{
	if (MemCardShowMap() == false)
	{
		Serial_printf("MemCardShowMap() failed!\n");
		return;
	}
}

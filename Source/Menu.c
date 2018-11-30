/* **************************************
 *  Includes                            *
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
 *  Defines                             *
 * *************************************/

#define MAIN_MENU_BUTTON_SIZE 64
#define SELECTED_BUTTON_LUMINANCE 0xC0

/* **************************************
 *  Structs and enums                   *
 * *************************************/

typedef enum t_levelId
{
    LEVEL1 = 0,
    LEVEL2,
    LEVEL3,
    LEVEL4,
    LEVEL5,

    MAX_LEVELS
}LEVEL_ID;

typedef struct t_lvlpltdata
{
    LEVEL_ID levelID;
    const char* const* fileNames[];
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
 *  Local prototypes                    *
 * **************************************/

static void MainMenuInit(void);
static void MenuCheatInit(void);
static void MainMenuDrawButton(TYPE_MMBtn* btn);
static void PlayMenu(void);
static void OptionsMenu(void);
static void OnePlayerMenu(void);
static void TwoPlayerMenu(void);
static void MainMenuButtonHandler(void);
static void MainMenuRestoreInitValues(void);
static void MenuTestCheat(void);
static void MainMenuRenderLevelList(void);

/* **************************************
 *  Local variables                     *
 * **************************************/

static GsSprite MenuSpr;
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

static const char* MainMenuLevelList[] =
{
    [LEVEL1] = "DATA\\LEVELS\\LEVEL1.LVL",
    [LEVEL2] = "DATA\\LEVELS\\LEVEL2.LVL",
    [LEVEL3] = "DATA\\LEVELS\\LEVEL3.LVL",
    [LEVEL4] = "DATA\\LEVELS\\XAMI.LVL",
    [LEVEL5] = "DATA\\LEVELS\\LEVEL18.LVL"
};

static const char** MainMenuPltList[] =
{
    [LEVEL1] = (const char*[])
    {
        "DATA\\LEVELS\\TUTORIA1.PLT",
        "DATA\\LEVELS\\LEVEL1.PLT",
        "DATA\\LEVELS\\EASY.PLT",
        NULL
    },

    [LEVEL2] = (const char*[])
    {
        "DATA\\LEVELS\\LEVEL2.PLT",
        NULL
    },

    [LEVEL3] = (const char*[])
    {
        "DATA\\LEVELS\\LEVEL3.PLT",
        NULL
    },

    [LEVEL4]  = (const char*[])
    {
        "DATA\\LEVELS\\XAMI.PLT",
        NULL
    },

    [LEVEL5]  = (const char*[])
    {
        "DATA\\LEVELS\\LEVEL18.PLT",
        NULL
    }
};

static TYPE_GAME_CONFIGURATION GameCfg;

static TYPE_MMBtn MainMenuBtn[MAIN_MENU_BUTTONS_MAX];
static MainMenuLevel menuLevel;
static MMBtn_Index MainMenuMinimumBtn;

static void PlayMenu(void)
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

static void OptionsMenu(void)
{
    Serial_printf("OptionsMenu(void)!\n");
}

static void OnePlayerMenu(void)
{
    menuLevel = LEVEL_LIST_LEVEL;
    GameCfg.TwoPlayers = false;
    //EndAnimation();
    //Game(false /* One Player Only */);
}

static void TwoPlayerMenu(void)
{
    menuLevel = LEVEL_LIST_LEVEL;
    GameCfg.TwoPlayers = true;
    //EndAnimation();
    //Game(true /* Two players */);
}


static void MainMenuInit(void)
{
    static const char* const MainMenuFiles[] =
    {
        "DATA\\SPRITES\\MAINMENU.TIM",
        "DATA\\SOUNDS\\BELL.VAG",
        "DATA\\SOUNDS\\ACCEPT.VAG",
        "DATA\\SPRITES\\BUTTONS.TIM",
#ifndef NO_INTRO
        "DATA\\SPRITES\\PSXDISK.TIM",
        "DATA\\FONTS\\INTROFNT.TIM",
        "DATA\\SPRITES\\GPL.TIM",
        "DATA\\SPRITES\\OPENSRC.TIM",
        "DATA\\SOUNDS\\TRAYCL.VAG",
        "DATA\\SOUNDS\\SPINDISK.VAG"
#endif // NO_INTRO
    };

    static void* const MainMenuDest[] =
    {
        &MenuSpr,
        &BellSnd,
        &AcceptSnd,
        &PSXButtons,
#ifndef NO_INTRO
        &PsxDisk,
        &PSXSDKIntroFont,
        &GPL_Logo,
        &OpenSource_Logo,
        &TrayClSnd,
        &SpinDiskSnd
#endif // NO_INTRO
    };

    LoadMenu(   MainMenuFiles,
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

    SelectedLevel = LEVEL1;

    MenuCheatInit();

    LoadMenuEnd();
}

static void MenuCheatInit(void)
{
    TestCheat.Callback = &MenuTestCheat;
    memset(TestCheat.Combination,0,CHEAT_ARRAY_SIZE);
    //memmove(myarray, (int [5]){a,b,c,d,e}, 5*sizeof (int));

    memmove( TestCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
            {   PAD_CIRCLE, PAD_CIRCLE, PAD_CROSS, PAD_TRIANGLE,
                PAD_TRIANGLE, PAD_TRIANGLE, 0 , 0 ,
                0, 0, 0, 0,
                0, 0, 0, 0  } ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&TestCheat);

    StackCheckCheat.Callback = &SystemPrintStackPointerAddress;
    memset(StackCheckCheat.Combination, 0, CHEAT_ARRAY_SIZE);

    memmove( StackCheckCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
            {   PAD_TRIANGLE, PAD_TRIANGLE, PAD_CROSS, PAD_TRIANGLE,
                PAD_L1, PAD_R1, 0 , 0 ,
                0, 0, 0, 0,
                0, 0, 0, 0  } ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&StackCheckCheat);

    DevMenuCheat.Callback = &SystemDevMenuToggle;
    memset(DevMenuCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memmove( DevMenuCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
            {   PAD_SQUARE, PAD_SQUARE, PAD_CROSS, PAD_CROSS,
                PAD_CIRCLE, PAD_CIRCLE, PAD_TRIANGLE , PAD_TRIANGLE ,
                PAD_L1, PAD_L1, PAD_R1, PAD_R1,
                PAD_L2, PAD_L2, PAD_R2, PAD_R2  } ,
            sizeof (unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&DevMenuCheat);

    SerialCheat.Callback = &SystemReturnToLoader;
    memset(SerialCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memmove( SerialCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
            {   PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE,
                PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE , PAD_CIRCLE ,
                0, 0, 0, 0,
                0, 0, 0, 0  } ,
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

    while (GfxIsGPUBusy());

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

static void MainMenuRenderLevelList(void)
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

    static GsGPoly4 levelListSelectionRect =
    {
        .x[0] = LEVEL_LIST_SELECTION_RECT_X,
        .x[1] = LEVEL_LIST_SELECTION_RECT_X + LEVEL_LIST_SELECTION_RECT_W,
        .x[2] = LEVEL_LIST_SELECTION_RECT_X,
        .x[3] = LEVEL_LIST_SELECTION_RECT_X + LEVEL_LIST_SELECTION_RECT_W,

        .r[0] = LEVEL_LIST_RECT_B0,
        .r[1] = LEVEL_LIST_RECT_B1,
        .r[2] = LEVEL_LIST_RECT_B2,
        .r[3] = LEVEL_LIST_RECT_B3,

        .g[0] = LEVEL_LIST_RECT_B0,
        .g[1] = LEVEL_LIST_RECT_B1,
        .g[2] = LEVEL_LIST_RECT_B2,
        .g[3] = LEVEL_LIST_RECT_B3,

        .b[0] = LEVEL_LIST_RECT_B0,
        .b[1] = LEVEL_LIST_RECT_B1,
        .b[2] = LEVEL_LIST_RECT_B2,
        .b[3] = LEVEL_LIST_RECT_B3,

        .attribute = ENABLE_TRANS | TRANS_MODE(0)
    };

    GsSortGPoly4(
        &(GsGPoly4)
        {
            .x[0] = LEVEL_LIST_RECT_X,
            .x[1] = LEVEL_LIST_RECT_X + LEVEL_LIST_RECT_W,
            .x[2] = LEVEL_LIST_RECT_X,
            .x[3] = LEVEL_LIST_RECT_X + LEVEL_LIST_RECT_W,

            .y[0] = LEVEL_LIST_RECT_Y,
            .y[1] = LEVEL_LIST_RECT_Y,
            .y[2] = LEVEL_LIST_RECT_Y + LEVEL_LIST_RECT_H,
            .y[3] = LEVEL_LIST_RECT_Y + LEVEL_LIST_RECT_H,

            .b[0] = LEVEL_LIST_RECT_B0,
            .b[1] = LEVEL_LIST_RECT_B1,
            .b[2] = LEVEL_LIST_RECT_B2,
            .b[3] = LEVEL_LIST_RECT_B3
        });

    GsSortGPoly4(
        &(GsGPoly4)
        {
            .x[0] = LEVEL_LIST_PLT_RECT_X,
            .x[1] = LEVEL_LIST_PLT_RECT_X + LEVEL_LIST_PLT_RECT_W,
            .x[2] = LEVEL_LIST_RECT_X,
            .x[3] = LEVEL_LIST_PLT_RECT_X + LEVEL_LIST_PLT_RECT_W,

            .y[0] = LEVEL_LIST_PLT_RECT_Y,
            .y[1] = LEVEL_LIST_PLT_RECT_Y,
            .y[2] = LEVEL_LIST_PLT_RECT_Y + LEVEL_LIST_PLT_RECT_H,
            .y[3] = LEVEL_LIST_PLT_RECT_Y + LEVEL_LIST_PLT_RECT_H,

            .b[0] = LEVEL_LIST_RECT_B0,
            .b[1] = LEVEL_LIST_RECT_B1,
            .b[2] = LEVEL_LIST_RECT_B2,
            .b[3] = LEVEL_LIST_RECT_B3
        });

    levelListSelectionRect.y[0] = LEVEL_LIST_SELECTION_RECT_Y + (short)(SelectedLevel << 3);
    levelListSelectionRect.y[1] = levelListSelectionRect.y[0];
    levelListSelectionRect.y[2] = LEVEL_LIST_SELECTION_RECT_Y + LEVEL_LIST_SELECTION_RECT_H + (short)(SelectedLevel << 3);
    levelListSelectionRect.y[3] = levelListSelectionRect.y[2];

    {
        LEVEL_ID i;

        for (i = LEVEL1; i < MAX_LEVELS; i++)
        {
            char baseName[32];
            const size_t maxLength = ARRAY_SIZE(baseName);

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

                        FontPrintText(  &SmallFont,
                                        LEVEL_LIST_PLT_TEXT_X,
                                        LEVEL_LIST_PLT_TEXT_Y + (j << 3),
                                        baseName    );
                    }
                }
            }
        }
    }

    if (isLevelSelected)
    {
        levelListSelectionRect.y[0] = LEVEL_LIST_PLT_TEXT_Y + (short)(SelectedPlt << 3);
        levelListSelectionRect.y[1] = levelListSelectionRect.y[0];
        levelListSelectionRect.y[2] = LEVEL_LIST_PLT_TEXT_Y + LEVEL_LIST_SELECTION_RECT_H + (short)(SelectedPlt << 3);
        levelListSelectionRect.y[3] = levelListSelectionRect.y[2];
    }

    GsSortGPoly4(&levelListSelectionRect);
}

static void MainMenuRestoreInitValues(void)
{
    uint8_t i;

    menuLevel = PLAY_OPTIONS_LEVEL;
    MainMenuMinimumBtn = PLAY_BUTTON_INDEX;

    MainMenuBtn[PLAY_BUTTON_INDEX].selected = false;
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

static void MainMenuButtonHandler(void)
{
    static uint8_t btn_selected = PLAY_BUTTON_INDEX;
    static uint8_t previous_btn_selected = 0;
    uint8_t max_buttons = 0;

    if (PadOneAnyKeyPressed())
    {
        if (SystemIsRandSeedSet() == false)
        {
            SystemSetRandSeed();
        }
    }

    if (    (PadOneKeySinglePress(PAD_CROSS))
                ||
            (PadOneKeySinglePress(PAD_TRIANGLE))   )
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

            if (PadOneKeySinglePress(PAD_TRIANGLE))
            {
                menuLevel = PLAY_OPTIONS_LEVEL;
                MainMenuMinimumBtn = PLAY_BUTTON_INDEX;
                btn_selected = PLAY_BUTTON_INDEX;
            }

        break;

        case LEVEL_LIST_LEVEL:
            if (PadOneKeySinglePress(PAD_UP))
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
            else if (PadOneKeySinglePress(PAD_DOWN))
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
            else if (PadOneKeySinglePress(PAD_TRIANGLE))
            {
                if (isLevelSelected == true)
                {
                    isLevelSelected = false;
                }
                else
                {
                    MainMenuRestoreInitValues();
                    btn_selected = PLAY_BUTTON_INDEX;
                    isLevelSelected = false;
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

    if (PadOneKeySinglePress(PAD_LEFT)  && (btn_selected > 0) )
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

static void MainMenuDrawButton(TYPE_MMBtn* btn)
{
    MenuSpr.w = MAIN_MENU_BUTTON_SIZE;
    MenuSpr.h = MAIN_MENU_BUTTON_SIZE;

    if ( (btn->timer) < (MainMenuBtnAni_sz - 1) )
    {
        btn->timer++;
    }

    if (btn->selected)
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

static void MenuTestCheat(void)
{
    if (MemCardShowMap() == false)
    {
        Serial_printf("MemCardShowMap() failed!\n");
        return;
    }
}

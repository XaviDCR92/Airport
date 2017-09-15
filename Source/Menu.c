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

#define BUTTON_SIZE 64
#define SELECTED_BUTTON_LUMINANCE 0xC0

/* **************************************
 * 	Structs and enums					*
 * *************************************/

typedef enum
{
	PLAY_OPTIONS_LEVEL = 0,
	ONE_TWO_PLAYERS_LEVEL,
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
	PLAY_BUTTON_INDEX = 0,
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

static void MainMenuDrawButton(TYPE_MMBtn * btn);
static void PlayMenu(void);
static void OptionsMenu(void);
static void OnePlayerMenu(void);
static void TwoPlayerMenu(void);
static void MainMenuButtonHandler(void);
static void MainMenuRestoreInitValues(void);
static void MenuTestCheat(void);

/* **************************************
 * 	Local variables						*
 * **************************************/

static GsSprite MenuSpr;
static SsVag BellSnd;
static SsVag AcceptSnd;
static TYPE_CHEAT TestCheat;
static TYPE_CHEAT StackCheckCheat;
static TYPE_CHEAT DevMenuCheat;
static TYPE_CHEAT SerialCheat;

static char* MainMenuFiles[] = {	"cdrom:\\DATA\\SPRITES\\MAINMENU.TIM;1"	,
									"cdrom:\\DATA\\SOUNDS\\BELL.VAG;1"		,
									"cdrom:\\DATA\\SOUNDS\\ACCEPT.VAG;1"	,
									"cdrom:\\DATA\\SPRITES\\BUTTONS.TIM;1"	,
#ifndef NO_INTRO
                                    "cdrom:\\DATA\\SPRITES\\PSXDISK.TIM;1"	,
                                    "cdrom:\\DATA\\SPRITES\\INTROFNT.TIM;1"	,
									"cdrom:\\DATA\\SPRITES\\GPL.TIM;1"		,
									"cdrom:\\DATA\\SPRITES\\OPENSRC.TIM;1"	,
									"cdrom:\\DATA\\SOUNDS\\TRAYCL.VAG;1"	,
									"cdrom:\\DATA\\SOUNDS\\SPINDISK.VAG;1"	
#endif // NO_INTRO
									};
									
static void* MainMenuDest[] = {     (GsSprite*)&MenuSpr			,
									(SsVag*)&BellSnd			,
									(SsVag*)&AcceptSnd			,
									(GsSprite*)&PSXButtons		,
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
	EndAnimation();
	Game(false /* One Player Only */);
}

void TwoPlayerMenu(void)
{
	EndAnimation();
	Game(true /* Two players */);
}


void MainMenuInit(void)
{
	LoadMenu(	MainMenuFiles,MainMenuDest,
				sizeof(MainMenuFiles) / sizeof(char*) ,
				sizeof(MainMenuDest) / sizeof(void*) );
	
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
	
	TestCheat.Callback = &MenuTestCheat;
	memset(TestCheat.Combination,0,CHEAT_ARRAY_SIZE);
	//memcpy(myarray, (int [5]){a,b,c,d,e}, 5*sizeof(int));
	
	memcpy(	TestCheat.Combination,
			(unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_CIRCLE, PAD_CIRCLE, PAD_CROSS, PAD_TRIANGLE,
				PAD_TRIANGLE, PAD_TRIANGLE, 0 , 0 ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof(unsigned short) * CHEAT_ARRAY_SIZE);
				
	PadAddCheat(&TestCheat);
	
	StackCheckCheat.Callback = &SystemPrintStackPointerAddress;
	memset(StackCheckCheat.Combination, 0, CHEAT_ARRAY_SIZE);
	
	memcpy(	StackCheckCheat.Combination,
			(unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_TRIANGLE, PAD_TRIANGLE, PAD_CROSS, PAD_TRIANGLE,
				PAD_L1, PAD_R1, 0 , 0 ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof(unsigned short) * CHEAT_ARRAY_SIZE);
				
	PadAddCheat(&StackCheckCheat);

    DevMenuCheat.Callback = &SystemDevMenuToggle;
    memset(DevMenuCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memcpy( DevMenuCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_SQUARE, PAD_SQUARE, PAD_CROSS, PAD_CROSS,
				PAD_CIRCLE, PAD_CIRCLE, PAD_TRIANGLE , PAD_TRIANGLE ,
				PAD_L1, PAD_L1, PAD_R1, PAD_R1,
				PAD_L2, PAD_L2, PAD_R2, PAD_R2	} ,
            sizeof(unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&DevMenuCheat);

    SerialCheat.Callback = &SystemReturnToLoader;
    memset(SerialCheat.Combination, 0 , CHEAT_ARRAY_SIZE);

    memcpy( SerialCheat.Combination,
            (unsigned short[CHEAT_ARRAY_SIZE])
			{	PAD_SQUARE, PAD_SQUARE, PAD_SQUARE, PAD_SQUARE,
				PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE , PAD_CIRCLE ,
				0, 0, 0, 0,
				0, 0, 0, 0	} ,
            sizeof(unsigned short) * CHEAT_ARRAY_SIZE);

    PadAddCheat(&SerialCheat);

	LoadMenuEnd();
}

void MainMenu(void)
{
	unsigned short* cheat_array;
	uint8_t i;
	MainMenuInit();
	
	#ifndef NO_INTRO
	PSXSDKIntro();
	#endif //PSXSDK_DEBUG

	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	while (1)
	{	
		MainMenuButtonHandler();

		while (GfxIsGPUBusy() == true);
		
		switch(menuLevel)
		{
			case PLAY_OPTIONS_LEVEL:
				
				GsSortCls(0,0,40);				
				MainMenuDrawButton(&MainMenuBtn[PLAY_BUTTON_INDEX]);
				MainMenuDrawButton(&MainMenuBtn[OPTIONS_BUTTON_INDEX]);
				
				for (cheat_array = PadGetPlayerOneCheatArray(), i = 0; *cheat_array != 0; cheat_array++, i += 16)
				{
					GfxDrawButton(i, 220, *cheat_array);
				}
			
				GfxDrawScene();
			break;
			
			case ONE_TWO_PLAYERS_LEVEL:
				
				GsSortCls(0,0,40);
				MainMenuDrawButton(&MainMenuBtn[ONE_PLAYER_BUTTON_INDEX]);
				MainMenuDrawButton(&MainMenuBtn[TWO_PLAYER_BUTTON_INDEX]);
				
				for (cheat_array = PadGetPlayerOneCheatArray(), i = 0; *cheat_array != 0; cheat_array++, i += 16)
				{
					GfxDrawButton(i, 220, *cheat_array);
				}
				
				GfxDrawScene();
			break;
			
			default:
			break;
		}
	}

}

void MainMenuRestoreInitValues(void)
{
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
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);

    SfxPlayTrack(INTRO_TRACK);
}

void MainMenuButtonHandler(void)
{
	static uint8_t btn_selected = PLAY_BUTTON_INDEX;
	static uint8_t previous_btn_selected = 0;
	uint8_t max_buttons;
	
	if (PadOneAnyKeyPressed() == true)
	{
		if (SystemIsRandSeedSet() == false)
		{
			SystemSetRandSeed();
		}
	}
	
	if (	(PadOneKeySinglePress(PAD_CROSS) == true)
				||
		(PadOneKeySinglePress(PAD_TRIANGLE) == true)	)
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

			if (PadOneKeySinglePress(PAD_TRIANGLE) == true)
			{
				menuLevel = PLAY_OPTIONS_LEVEL;
				MainMenuMinimumBtn = PLAY_BUTTON_INDEX;
				btn_selected = PLAY_BUTTON_INDEX;
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
		if (menuLevel == ONE_TWO_PLAYERS_LEVEL)
		{
            // Start gameplay!
            MainMenuBtn[btn_selected].f();
            // Once gameplay has finished, turn back to first level
            MainMenuRestoreInitValues();
            btn_selected = PLAY_BUTTON_INDEX;
		}
		else
		{
			MainMenuBtn[btn_selected].f();
		}
		
		
		
		if (menuLevel == ONE_TWO_PLAYERS_LEVEL)
		{
			btn_selected = PLAY_BUTTON_INDEX;
		}
		
	}
	
	MainMenuBtn[btn_selected].selected = true;

	
}

void MainMenuDrawButton(TYPE_MMBtn * btn)
{		
	MenuSpr.w = BUTTON_SIZE;
	MenuSpr.h = BUTTON_SIZE;
	
	if (btn->timer < MainMenuBtnAni_sz)
	{
		btn->timer++;
	}
	
	if (btn->selected == true)
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
			MenuSpr.y = MAIN_MENU_PLAY_BUTTON_Y - MainMenuBtnAni[btn->timer];
			MenuSpr.u += btn->offset_u;
			MenuSpr.v += btn->offset_v;
					
			GsSortSprite(&MenuSpr);
		break;
		
		case OPTIONS_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_OPTIONS_BUTTON_X;
			MenuSpr.y = MAIN_MENU_OPTIONS_BUTTON_Y - MainMenuBtnAni[btn->timer];
			MenuSpr.u += btn->offset_u;
			MenuSpr.v += btn->offset_v;
					
			GsSortSprite(&MenuSpr);
		break;
		
		case ONE_PLAYER_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_ONE_PLAYER_BUTTON_X;
			MenuSpr.y = MAIN_MENU_ONE_PLAYER_BUTTON_Y - MainMenuBtnAni[btn->timer];
			MenuSpr.u += btn->offset_u;
			MenuSpr.v += btn->offset_v;
					
			GsSortSprite(&MenuSpr);
		break;
		
		case TWO_PLAYER_BUTTON_INDEX:
			MenuSpr.x = MAIN_MENU_TWO_PLAYER_BUTTON_X;
			MenuSpr.y = MAIN_MENU_TWO_PLAYER_BUTTON_Y - MainMenuBtnAni[btn->timer];
			MenuSpr.u += btn->offset_u;
			MenuSpr.v += btn->offset_v;

            // Exception: turn option dimmer if second player pad isn't connected

            if (PadTwoConnected() == false)
            {
                MenuSpr.r = NORMAL_LUMINANCE >> 1;
                MenuSpr.g = NORMAL_LUMINANCE >> 1;
                MenuSpr.b = NORMAL_LUMINANCE >> 1;
            }
					
			GsSortSprite(&MenuSpr);
		break;
		
		default:
		break;
	}

}

void MenuTestCheat(void)
{
	if (MemCardShowMap() == false)
	{
		Serial_printf("MemCardShowMap() failed!\n");
		return;
	}
}

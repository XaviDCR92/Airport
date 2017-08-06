/* *************************************
 * 	Includes
 * *************************************/

#include "GameGui.h"
#include "System.h"
#include "Gfx.h"
#include "Game.h"
#include "LoadMenu.h"

/* *************************************
 * 	Defines
 * *************************************/
 
#define GAME_GUI_AIRCRAFT_DATA_MAX_PAGE 4

#define SLOW_SCORE_LOW_SPEED_MARGIN		100
#define SLOW_SCORE_LOW_SPEED			5
#define SLOW_SCORE_HIGH_SPEED			10

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	BUBBLE_SPRITE_X = 298,
	BUBBLE_SPRITE_Y = 32,
	BUBBLE_SPRITE_RAND_MIN = -3,
	BUBBLE_SPRITE_RAND_MAX = 3,
	
	NOTIFICATION_BUTTON_X = BUBBLE_SPRITE_X + 24,
	NOTIFICATION_BUTTON_Y = BUBBLE_SPRITE_Y + 16
};

enum
{
	AIRCRAFT_DATA_GSGPOLY4_R0 = 0,
	AIRCRAFT_DATA_GSGPOLY4_R1 = AIRCRAFT_DATA_GSGPOLY4_R0,
	AIRCRAFT_DATA_GSGPOLY4_R2 = 0,
	AIRCRAFT_DATA_GSGPOLY4_R3 = AIRCRAFT_DATA_GSGPOLY4_R2,
	
	AIRCRAFT_DATA_GSGPOLY4_G0 = NORMAL_LUMINANCE,
	AIRCRAFT_DATA_GSGPOLY4_G1 = AIRCRAFT_DATA_GSGPOLY4_G0,
	AIRCRAFT_DATA_GSGPOLY4_G2 = 0,
	AIRCRAFT_DATA_GSGPOLY4_G3 = AIRCRAFT_DATA_GSGPOLY4_G2,
	
	AIRCRAFT_DATA_GSGPOLY4_B0 = 40,
	AIRCRAFT_DATA_GSGPOLY4_B1 = AIRCRAFT_DATA_GSGPOLY4_B0,
	AIRCRAFT_DATA_GSGPOLY4_B2 = 0,
	AIRCRAFT_DATA_GSGPOLY4_B3 = AIRCRAFT_DATA_GSGPOLY4_B2,
	
	AIRCRAFT_DATA_GSGPOLY4_X0 = (X_SCREEN_RESOLUTION >> 1) - 88,
	AIRCRAFT_DATA_GSGPOLY4_X1 = (X_SCREEN_RESOLUTION >> 1) + 88,
	AIRCRAFT_DATA_GSGPOLY4_X2 = AIRCRAFT_DATA_GSGPOLY4_X0,
	AIRCRAFT_DATA_GSGPOLY4_X3 = AIRCRAFT_DATA_GSGPOLY4_X1,
	
	AIRCRAFT_DATA_GSGPOLY4_Y0 = (Y_SCREEN_RESOLUTION >> 1) - 96,
	AIRCRAFT_DATA_GSGPOLY4_Y1 = AIRCRAFT_DATA_GSGPOLY4_Y0,
	AIRCRAFT_DATA_GSGPOLY4_Y2 = (Y_SCREEN_RESOLUTION >> 1) + 96,
	AIRCRAFT_DATA_GSGPOLY4_Y3 = AIRCRAFT_DATA_GSGPOLY4_Y2,
	
	AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER = 24,
	AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER = 168,
	AIRCRAFT_DATA_GSGPOLY4_X2_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER,
	AIRCRAFT_DATA_GSGPOLY4_X3_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER,
	
	AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER = 24,
	AIRCRAFT_DATA_GSGPOLY4_Y1_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER,
	AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER = 208,
	AIRCRAFT_DATA_GSGPOLY4_Y3_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER,
};

enum
{
	AIRCRAFT_LOCK_TARGET_X = 32,
	AIRCRAFT_LOCK_TARGET_TEXT_X = AIRCRAFT_LOCK_TARGET_X + 16,
	AIRCRAFT_LOCK_TARGET_Y = 224,
	AIRCRAFT_LOCK_TARGET_TEXT_Y = AIRCRAFT_LOCK_TARGET_Y + 4,
};

enum
{
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R0 = NORMAL_LUMINANCE,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R1 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R2 = 0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R3 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R2,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G0 = NORMAL_LUMINANCE,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G1 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G2 = 0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G3 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G2,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B0 = NORMAL_LUMINANCE,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B1 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B2 = 0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B3 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B2,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP = 8,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0 = AIRCRAFT_DATA_GSGPOLY4_X0 + 24,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1 = AIRCRAFT_DATA_GSGPOLY4_X1 - 24,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X2 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X3 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H = 42,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0 = AIRCRAFT_DATA_GSGPOLY4_Y0 + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y1 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0 + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y3 = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER - AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X2_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0_2PLAYER,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X3_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1_2PLAYER,
	
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y1_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0_2PLAYER,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H,
	AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y3_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2_2PLAYER
};

enum
{
	AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0 + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0 + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	
	AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y_2PLAYER = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	
	AIRCRAFT_DATA_DIRECTION_X = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X,
	AIRCRAFT_DATA_DIRECTION_Y = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	
	AIRCRAFT_DATA_DIRECTION_X_2PLAYER = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X_2PLAYER,
	AIRCRAFT_DATA_DIRECTION_Y_2PLAYER = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
	
	AIRCRAFT_DATA_PASSENGERS_X = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X + 64,
	AIRCRAFT_DATA_PASSENGERS_Y = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y,
	
	AIRCRAFT_DATA_PASSENGERS_X_2PLAYER = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X_2PLAYER + 64,
	AIRCRAFT_DATA_PASSENGERS_Y_2PLAYER = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y_2PLAYER,

	AIRCRAFT_DATA_REMAINING_TIME_X = AIRCRAFT_DATA_DIRECTION_X + 24,
	AIRCRAFT_DATA_REMAINING_TIME_Y = AIRCRAFT_DATA_DIRECTION_Y + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,

	AIRCRAFT_DATA_REMAINING_TIME_X_2PLAYER = AIRCRAFT_DATA_DIRECTION_X_2PLAYER + 24,
	AIRCRAFT_DATA_REMAINING_TIME_Y_2PLAYER = AIRCRAFT_DATA_DIRECTION_Y_2PLAYER + AIRCRAFT_DATA_FLIGHT_GSGPOLY4_GAP,
};

enum
{
	AIRCRAFT_DEPARTURE_ARRIVAL_ICON_SIZE = 16,
	AIRCRAFT_ARRIVAL_ICON_U = 0,
	AIRCRAFT_DEPARTURE_ICON_U = 16
};

enum
{
	AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_X = 112,
	AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y = 112,

	AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_X_2PLAYER = 48,
	AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y_2PLAYER = 112,
	
	AIRCRAFT_DATA_FLIGHT_PAGE_UP_X = 260,
	AIRCRAFT_DATA_FLIGHT_PAGE_UP_Y = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y,

	AIRCRAFT_DATA_FLIGHT_PAGE_UP_X_2PLAYER = 128,
	AIRCRAFT_DATA_FLIGHT_PAGE_UP_Y_2PLAYER = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y_2PLAYER,

	AIRCRAFT_DATA_FLIGHT_PAGE_UP_U = 16,
	AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_U = 0,

	AIRCRAFT_DATA_FLIGHT_PAGEUPDN_SIZE = 16
};

enum
{
	GAME_GUI_SECOND_DISPLAY_X = 384,
	GAME_GUI_SECOND_DISPLAY_Y = 256,
	GAME_GUI_SECOND_DISPLAY_TPAGE = 22,
};

/* **************************************
 * 	Local prototypes					*
 * *************************************/

static void GameGuiShowAircraftData(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameGuiClearPassengersLeft(void);
static void GameGuiBubbleStop(void);
static void GameGuiBubbleStopVibration(void);

/* **************************************
 * 	Local variables						*
 * *************************************/

static GsSprite BubbleSpr;
static GsGPoly4 AircraftDataGPoly4;
static GsGPoly4 SelectedAircraftGPoly4;
static GsGPoly4 PauseRect;
static GsSprite SecondDisplay;
static GsSprite DepArrSpr;
static GsSprite PageUpDownSpr;
static TYPE_TIMER* ShowAircraftPassengersTimer;
static bool GameGuiClearPassengersLeft_Flag;
static bool GameGuiBubbleShowFlag;
static bool GameGuiBubbleVibrationFlag;

static char* GameFileList[] = {	"cdrom:\\DATA\\SPRITES\\BUBBLE.TIM;1"	,
								"cdrom:\\DATA\\FONTS\\FONT_1.FNT;1"		,
								"cdrom:\\DATA\\SPRITES\\DEPARR.TIM;1"	,
								"cdrom:\\DATA\\SPRITES\\PAGEUPDN.TIM;1"	};
								
static void* GameFileDest[] = {(GsSprite*)&BubbleSpr		,
								(TYPE_FONT*)&RadioFont		,
								(GsSprite*)&DepArrSpr		,
								(GsSprite*)&PageUpDownSpr	};

static uint32_t slowScore; // It will update slowly to actual score value

void GameGuiInit(void)
{
	enum
	{
		PAUSE_DIALOG_X = 92,
		PAUSE_DIALOG_Y = 28,
		PAUSE_DIALOG_W = 200,
		PAUSE_DIALOG_H = 184,
		
		PAUSE_DIALOG_R0 = 0,
		PAUSE_DIALOG_R1 = PAUSE_DIALOG_R0,
		PAUSE_DIALOG_R2 = 0,
		PAUSE_DIALOG_R3 = PAUSE_DIALOG_R2,
		
		PAUSE_DIALOG_G0 = NORMAL_LUMINANCE,
		PAUSE_DIALOG_G1 = PAUSE_DIALOG_G0,
		PAUSE_DIALOG_G2 = 0,
		PAUSE_DIALOG_G3 = PAUSE_DIALOG_G2,
		
		PAUSE_DIALOG_B0 = 40,
		PAUSE_DIALOG_B1 = PAUSE_DIALOG_B0,
		PAUSE_DIALOG_B2 = 0,
		PAUSE_DIALOG_B3 = PAUSE_DIALOG_B2,
	};

    enum
    {
        RADIO_FONT_SPACING = 12
    };

    static bool firstLoad = true;

    if(firstLoad == true)
    {
        firstLoad = false;

        LoadMenu(	GameFileList,
                    GameFileDest,
                    sizeof(GameFileList) / sizeof(char*),
                    sizeof(GameFileDest) /sizeof(void*)	);
    }
				
	PauseRect.x[0] = PAUSE_DIALOG_X;
	PauseRect.x[1] = PAUSE_DIALOG_X + PAUSE_DIALOG_W;
	PauseRect.x[2] = PAUSE_DIALOG_X;
	PauseRect.x[3] = PAUSE_DIALOG_X + PAUSE_DIALOG_W;
	
	PauseRect.y[0] = PAUSE_DIALOG_Y;
	PauseRect.y[1] = PAUSE_DIALOG_Y;
	PauseRect.y[2] = PAUSE_DIALOG_Y + PAUSE_DIALOG_H;
	PauseRect.y[3] = PAUSE_DIALOG_Y + PAUSE_DIALOG_H;
	
	PauseRect.r[0] = PAUSE_DIALOG_R0;
	PauseRect.r[1] = PAUSE_DIALOG_R1;
	PauseRect.r[2] = PAUSE_DIALOG_R2;
	PauseRect.r[3] = PAUSE_DIALOG_R3;
	
	PauseRect.b[0] = PAUSE_DIALOG_B0;
	PauseRect.b[1] = PAUSE_DIALOG_B1;
	PauseRect.b[2] = PAUSE_DIALOG_B2;
	PauseRect.b[3] = PAUSE_DIALOG_B3;
	
	PauseRect.g[0] = PAUSE_DIALOG_G0;
	PauseRect.g[1] = PAUSE_DIALOG_G1;
	PauseRect.g[2] = PAUSE_DIALOG_G2;
	PauseRect.g[3] = PAUSE_DIALOG_G3;
	
	PauseRect.attribute |= ENABLE_TRANS | TRANS_MODE(0);

	PageUpDownSpr.w = AIRCRAFT_DATA_FLIGHT_PAGEUPDN_SIZE;

	ShowAircraftPassengersTimer = SystemCreateTimer(20, true, GameGuiClearPassengersLeft);

	slowScore = 0;

	GameGuiBubbleShowFlag = false;

    FontSetSpacing(&RadioFont, RADIO_FONT_SPACING);
}

bool GameGuiPauseDialog(TYPE_PLAYER* ptrPlayer)
{	
	GfxSaveDisplayData(&SecondDisplay);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
				
	//DrawFBRect(0, 0, X_SCREEN_RESOLUTION, VRAM_H, 0, 0, 0);
				
	while(GfxIsGPUBusy() == true);
	
	do
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			return true;
		}
		
		GfxSortSprite(&SecondDisplay);
		
		GsSortGPoly4(&PauseRect);
		
		GfxDrawScene_Slow();
		
	}while(ptrPlayer->PadKeySinglePress_Callback(PAD_START) == false);
	
	return false;
}

void GameGuiActiveAircraftPage(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	if(ptrPlayer->ActiveAircraft != 0)
	{
		while(ptrPlayer->ActiveAircraft <= ptrPlayer->SelectedAircraft)
		{
			ptrPlayer->SelectedAircraft--;
		}
	}
	
	while(ptrPlayer->ActiveAircraft < (uint8_t)(GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * ptrPlayer->FlightDataPage) )
	{
		ptrPlayer->FlightDataPage--;
	}
	
	if(ptrPlayer->ShowAircraftData == true)
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_DOWN) == true)
		{
			if( ( (ptrPlayer->SelectedAircraft + 1) < ptrPlayer->ActiveAircraft)
												&&
				( (ptrPlayer->SelectedAircraft + 1) < ( (ptrPlayer->FlightDataPage + 1) * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE) ) )
			{
				ptrPlayer->SelectedAircraft++;
			}
		}
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_UP) == true)
		{
			if(ptrPlayer->SelectedAircraft > ( (ptrPlayer->FlightDataPage) * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE) )
			{
				ptrPlayer->SelectedAircraft--;
			}
		}
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_RIGHT) == true)
		{
			if(ptrPlayer->ActiveAircraft > (GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * (ptrPlayer->FlightDataPage + 1) ) )
			{
				ptrPlayer->FlightDataPage++;
				ptrPlayer->SelectedAircraft = ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE;
				Serial_printf("Selected Aircraft = %d\n",ptrPlayer->SelectedAircraft);
			}
		}
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_LEFT) == true)
		{
			if(ptrPlayer->FlightDataPage != 0)
			{
				ptrPlayer->FlightDataPage--;
				ptrPlayer->SelectedAircraft = ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE;
				Serial_printf("Selected Aircraft = %d\n",ptrPlayer->SelectedAircraft);
			}
		}
	}
}

void GameGuiAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	short y_offset;
	uint8_t page_aircraft;
	short orig_pageupdn_u;

	enum
	{
		GAME_GUI_REMAINING_AIRCRAFT_X = AIRCRAFT_DATA_GSGPOLY4_X0 + 16,
		GAME_GUI_REMAINING_AIRCRAFT_Y = AIRCRAFT_DATA_GSGPOLY4_Y2 - 16,
		GAME_GUI_REMAINING_AIRCRAFT_X_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER + 16,
		GAME_GUI_REMAINING_AIRCRAFT_Y_2PLAYER = AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER - 16
	};
	
	if(ptrPlayer->ShowAircraftData == true)
	{
		// Prepare RGB data and (X,Y) coordinates for aircraft
		// data list request rectangle.
		AircraftDataGPoly4.r[0] = AIRCRAFT_DATA_GSGPOLY4_R0;
		AircraftDataGPoly4.r[1] = AIRCRAFT_DATA_GSGPOLY4_R1;
		AircraftDataGPoly4.r[2] = AIRCRAFT_DATA_GSGPOLY4_R2;
		AircraftDataGPoly4.r[3] = AIRCRAFT_DATA_GSGPOLY4_R3;
		
		AircraftDataGPoly4.g[0] = AIRCRAFT_DATA_GSGPOLY4_G0;
		AircraftDataGPoly4.g[1] = AIRCRAFT_DATA_GSGPOLY4_G1;
		AircraftDataGPoly4.g[2] = AIRCRAFT_DATA_GSGPOLY4_G2;
		AircraftDataGPoly4.g[3] = AIRCRAFT_DATA_GSGPOLY4_G3;
		
		AircraftDataGPoly4.b[0] = AIRCRAFT_DATA_GSGPOLY4_B0;
		AircraftDataGPoly4.b[1] = AIRCRAFT_DATA_GSGPOLY4_B1;
		AircraftDataGPoly4.b[2] = AIRCRAFT_DATA_GSGPOLY4_B2;
		AircraftDataGPoly4.b[3] = AIRCRAFT_DATA_GSGPOLY4_B3;
		
		AircraftDataGPoly4.attribute |= ENABLE_TRANS | TRANS_MODE(0);
			
		if(GameTwoPlayersActive() == true)
		{
			AircraftDataGPoly4.x[0] = AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER;
			AircraftDataGPoly4.x[1] = AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER;
			AircraftDataGPoly4.x[2] = AIRCRAFT_DATA_GSGPOLY4_X2_2PLAYER;
			AircraftDataGPoly4.x[3] = AIRCRAFT_DATA_GSGPOLY4_X3_2PLAYER;
			
			AircraftDataGPoly4.y[0] = AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER;
			AircraftDataGPoly4.y[1] = AIRCRAFT_DATA_GSGPOLY4_Y1_2PLAYER;
			AircraftDataGPoly4.y[2] = AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER;
			AircraftDataGPoly4.y[3] = AIRCRAFT_DATA_GSGPOLY4_Y3_2PLAYER;
		}
		else
		{
			AircraftDataGPoly4.x[0] = AIRCRAFT_DATA_GSGPOLY4_X0;
			AircraftDataGPoly4.x[1] = AIRCRAFT_DATA_GSGPOLY4_X1;
			AircraftDataGPoly4.x[2] = AIRCRAFT_DATA_GSGPOLY4_X2;
			AircraftDataGPoly4.x[3] = AIRCRAFT_DATA_GSGPOLY4_X3;
			
			AircraftDataGPoly4.y[0] = AIRCRAFT_DATA_GSGPOLY4_Y0;
			AircraftDataGPoly4.y[1] = AIRCRAFT_DATA_GSGPOLY4_Y1;
			AircraftDataGPoly4.y[2] = AIRCRAFT_DATA_GSGPOLY4_Y2;
			AircraftDataGPoly4.y[3] = AIRCRAFT_DATA_GSGPOLY4_Y3;
		}
		
		GsSortGPoly4(&AircraftDataGPoly4);

		if(GameTwoPlayersActive() == true)
		{
			FontPrintText(	&SmallFont,
							GAME_GUI_REMAINING_AIRCRAFT_X_2PLAYER,
							GAME_GUI_REMAINING_AIRCRAFT_Y_2PLAYER,
							"Rem. aircraft: %d",
							ptrFlightData->nRemainingAircraft		);
		}
		else
		{
			FontPrintText(	&SmallFont,
							GAME_GUI_REMAINING_AIRCRAFT_X,
							GAME_GUI_REMAINING_AIRCRAFT_Y,
							"Remaining aircraft: %d",
							ptrFlightData->nRemainingAircraft		);
		}
			
		if(ptrPlayer->ActiveAircraft != 0)
		{
			SelectedAircraftGPoly4.r[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R0;
			SelectedAircraftGPoly4.r[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R1;
			SelectedAircraftGPoly4.r[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R2;
			SelectedAircraftGPoly4.r[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_R3;
			
			SelectedAircraftGPoly4.g[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G0;
			SelectedAircraftGPoly4.g[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G1;
			SelectedAircraftGPoly4.g[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G2;
			SelectedAircraftGPoly4.g[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_G3;
			
			SelectedAircraftGPoly4.b[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B0;
			SelectedAircraftGPoly4.b[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B1;
			SelectedAircraftGPoly4.b[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B2;
			SelectedAircraftGPoly4.b[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_B3;
			
			SelectedAircraftGPoly4.attribute |= ENABLE_TRANS | TRANS_MODE(0);
			
			if(GameTwoPlayersActive() == true)
			{
				SelectedAircraftGPoly4.x[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0_2PLAYER;
				SelectedAircraftGPoly4.x[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1_2PLAYER;
				SelectedAircraftGPoly4.x[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X2_2PLAYER;
				SelectedAircraftGPoly4.x[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X3_2PLAYER;
			}
			else
			{				
				SelectedAircraftGPoly4.x[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X0;
				SelectedAircraftGPoly4.x[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X1;
				SelectedAircraftGPoly4.x[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X2;
				SelectedAircraftGPoly4.x[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_X3;
			}
			
			page_aircraft = (ptrPlayer->SelectedAircraft) - (ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE);
			
			y_offset = 	(short)(page_aircraft * AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H);
									
			/*Serial_printf("ptrPlayer->ActiveAircraft = %d\n",ptrPlayer->ActiveAircraft);
			Serial_printf("ptrPlayer->SelectedAircraft = %d\n",ptrPlayer->SelectedAircraft);
			Serial_printf("ptrPlayer->FlightDataPage = %d\n",ptrPlayer->FlightDataPage);
			Serial_printf("y_offset = %d\n",y_offset);*/
			
			if(GameTwoPlayersActive() == true)
			{
				SelectedAircraftGPoly4.y[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0_2PLAYER;
				SelectedAircraftGPoly4.y[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y1_2PLAYER;
				SelectedAircraftGPoly4.y[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2_2PLAYER;
				SelectedAircraftGPoly4.y[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y3_2PLAYER;
			}
			else
			{
				SelectedAircraftGPoly4.y[0] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y0;
				SelectedAircraftGPoly4.y[1] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y1;
				SelectedAircraftGPoly4.y[2] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y2;
				SelectedAircraftGPoly4.y[3] = AIRCRAFT_DATA_FLIGHT_GSGPOLY4_Y3;
			}
			
			SelectedAircraftGPoly4.y[0] += y_offset;
			SelectedAircraftGPoly4.y[1] += y_offset;	
			SelectedAircraftGPoly4.y[2] += y_offset;
			SelectedAircraftGPoly4.y[3] += y_offset;
			
			GsSortGPoly4(&SelectedAircraftGPoly4);

			PageUpDownSpr.attribute |= GFX_1HZ_FLASH;
			
			if(ptrPlayer->ActiveAircraft > (GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * (ptrPlayer->FlightDataPage + 1) ) )
			{
				orig_pageupdn_u = PageUpDownSpr.u;
				
				PageUpDownSpr.u = orig_pageupdn_u + AIRCRAFT_DATA_FLIGHT_PAGE_UP_U;
				
				if(GameTwoPlayersActive() == true)
				{
					PageUpDownSpr.x = AIRCRAFT_DATA_FLIGHT_PAGE_UP_X_2PLAYER;
					PageUpDownSpr.y = AIRCRAFT_DATA_FLIGHT_PAGE_UP_Y_2PLAYER;
				}
				else
				{
					PageUpDownSpr.x = AIRCRAFT_DATA_FLIGHT_PAGE_UP_X;
					PageUpDownSpr.y = AIRCRAFT_DATA_FLIGHT_PAGE_UP_Y;
				}

				GfxSortSprite(&PageUpDownSpr);

				PageUpDownSpr.u = orig_pageupdn_u;
			}
			
			if(ptrPlayer->FlightDataPage != 0)
			{
				orig_pageupdn_u = PageUpDownSpr.u;
				
				PageUpDownSpr.u = orig_pageupdn_u + AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_U;
				
				if(GameTwoPlayersActive() == true)
				{
					PageUpDownSpr.x = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_X_2PLAYER;
					PageUpDownSpr.y = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y_2PLAYER;
				}
				else
				{
					PageUpDownSpr.x = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_X;
					PageUpDownSpr.y = AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_Y;
				}
				
				GfxSortSprite(&PageUpDownSpr);

				PageUpDownSpr.u = orig_pageupdn_u;
			}
			
			GameGuiShowAircraftData(ptrPlayer, ptrFlightData);
			
			GfxDrawButton(AIRCRAFT_LOCK_TARGET_X, AIRCRAFT_LOCK_TARGET_Y, PAD_SQUARE);
			
			if(ptrPlayer->LockTarget == true)
			{
				FontPrintText(&SmallFont, AIRCRAFT_LOCK_TARGET_TEXT_X, AIRCRAFT_LOCK_TARGET_TEXT_Y, "Unlock target");
			}
			else
			{
				FontPrintText(&SmallFont, AIRCRAFT_LOCK_TARGET_TEXT_X, AIRCRAFT_LOCK_TARGET_TEXT_Y, "Lock target");
			}
		}
		else
		{
			if(GameTwoPlayersActive() == true)
			{
				FontPrintText( 	&SmallFont,
								AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER + 
								( (AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER - AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER) >> 2),
								AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER +
								( (AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER - AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER) >> 1),
								"No flights!"	);
			}
			else
			{
				FontPrintText( 	&SmallFont,
								AIRCRAFT_DATA_GSGPOLY4_X0 + 
								( (AIRCRAFT_DATA_GSGPOLY4_X1 - AIRCRAFT_DATA_GSGPOLY4_X0) >> 2),
								AIRCRAFT_DATA_GSGPOLY4_Y0 +
								( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
								"No flights!"	);
			}
		}
	}

}

void GameGuiBubbleShow(void)
{
	static TYPE_TIMER* GameGuiBubbleTimer = NULL;

	if(GameGuiBubbleTimer == NULL)
	{
		Serial_printf("Started GameGuiBubbleTimer...\n");
		GameGuiBubbleTimer = SystemCreateTimer(50, false, &GameGuiBubbleStop);
	}
	else
	{
		SystemTimerRestart(GameGuiBubbleTimer);
	}

	GameGuiBubbleShowFlag = true;
	GameGuiBubbleVibrationFlag = true;
}

void GameGuiBubble(TYPE_FLIGHT_DATA* ptrFlightData)
{
	static bool GameGuiBubbleShowFlagOld;

	if(GameGuiBubbleShowFlag == true)
	{
		static TYPE_TIMER* GameGuiBubbleVibrationTimer = NULL;

		if(GameGuiBubbleShowFlagOld == false)
		{
			if(GameGuiBubbleVibrationTimer == NULL)
			{
				Serial_printf("Started GameGuiBubbleVibrationTimer...\n");
				GameGuiBubbleVibrationTimer = SystemCreateTimer(20, false, &GameGuiBubbleStopVibration);
			}
			else
			{
				SystemTimerRestart(GameGuiBubbleVibrationTimer);
			}
		}
		
		BubbleSpr.x = BUBBLE_SPRITE_X;
		BubbleSpr.y = BUBBLE_SPRITE_Y;
		
		if(GameGuiBubbleVibrationFlag == true)
		{
			BubbleSpr.x += SystemRand(BUBBLE_SPRITE_RAND_MIN,BUBBLE_SPRITE_RAND_MAX);
			BubbleSpr.y += SystemRand(BUBBLE_SPRITE_RAND_MIN,BUBBLE_SPRITE_RAND_MAX);
		}

		GfxSortSprite(&BubbleSpr);
		FontSetFlags(&SmallFont, FONT_CENTERED);
		FontPrintText(&SmallFont,BubbleSpr.x + 8 , BubbleSpr.y + 2, "%d", ptrFlightData->ActiveAircraft);
		
		GfxDrawButton(NOTIFICATION_BUTTON_X, NOTIFICATION_BUTTON_Y, PAD_CIRCLE);
	}

	GameGuiBubbleShowFlagOld = GameGuiBubbleShowFlag;
}

void GameGuiClock(uint8_t hour, uint8_t min)
{
	enum
	{
		CLOCK_X = 16,
		CLOCK_Y = 4
	};

	static char strClock[6]; // HH:MM + \0 (6 characters needed)
	
	if(GameStartupFlag || System1SecondTick() == true)
	{
		memset(strClock, 0, 6);
		snprintf(strClock,6,"%02d:%02d",hour, min);
	}
    
	RadioFont.max_ch_wrap = 0;
    FontSetFlags(&RadioFont, FONT_NOFLAGS);
	FontPrintText(&RadioFont,CLOCK_X,CLOCK_Y,strClock);
}

void GameGuiShowPassengersLeft(TYPE_PLAYER* ptrPlayer)
{
	if(GameGuiClearPassengersLeft_Flag == true)
	{
		// Reset flag
		GameGuiClearPassengersLeft_Flag = false;
		ptrPlayer->PassengersLeftSelectedAircraft = 0;
	}

	if(ptrPlayer->PassengersLeftSelectedAircraft != 0)
	{
		if(GameTwoPlayersActive() == true)
		{
			FontPrintText(&SmallFont, 48, Y_SCREEN_RESOLUTION - 64, "%d left", ptrPlayer->PassengersLeftSelectedAircraft);
		}
		else
		{
			FontPrintText(&SmallFont, 128, Y_SCREEN_RESOLUTION - 64, "%d left", ptrPlayer->PassengersLeftSelectedAircraft);
		}
	}
}

void GameGuiShowAircraftData(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t init_flight = ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE;
	uint8_t i;
	uint8_t j;
	short AircraftDataDirection_X;
	short AircraftDataDirection_Y;
	short AircraftDataFlightNumber_X;
	short AircraftDataFlightNumber_Y;
	//short AircraftDataPassengers_X;
	//short AircraftDataPassengers_Y;
	short AircraftDataState_X_Offset;
	short AircraftDataRemainingTime_X;
	short AircraftDataRemainingTime_Y;
	short orig_DepArr_u = DepArrSpr.u;
	
	if(GameTwoPlayersActive() == true)
	{
		AircraftDataDirection_X = AIRCRAFT_DATA_DIRECTION_X_2PLAYER;
		AircraftDataDirection_Y = AIRCRAFT_DATA_DIRECTION_Y_2PLAYER;
		AircraftDataFlightNumber_X = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X_2PLAYER;
		AircraftDataFlightNumber_Y = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y_2PLAYER;
		//AircraftDataPassengers_X = AIRCRAFT_DATA_PASSENGERS_X_2PLAYER;
		//AircraftDataPassengers_Y = AIRCRAFT_DATA_PASSENGERS_Y_2PLAYER;
		AircraftDataState_X_Offset = 24;
		AircraftDataRemainingTime_X = AIRCRAFT_DATA_REMAINING_TIME_X_2PLAYER;
		AircraftDataRemainingTime_Y = AIRCRAFT_DATA_REMAINING_TIME_Y_2PLAYER;
	}
	else
	{
		AircraftDataDirection_X = AIRCRAFT_DATA_DIRECTION_X;
		AircraftDataDirection_Y = AIRCRAFT_DATA_DIRECTION_Y;
		AircraftDataFlightNumber_X = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_X;
		AircraftDataFlightNumber_Y = AIRCRAFT_DATA_FLIGHT_NUMBER_TEXT_Y;
		//AircraftDataPassengers_X = AIRCRAFT_DATA_PASSENGERS_X;
		//AircraftDataPassengers_Y = AIRCRAFT_DATA_PASSENGERS_Y;
		AircraftDataState_X_Offset = 24;
		AircraftDataRemainingTime_X = AIRCRAFT_DATA_REMAINING_TIME_X;
		AircraftDataRemainingTime_Y = AIRCRAFT_DATA_REMAINING_TIME_Y;
	}
	
	FontSetFlags(&SmallFont,FONT_NOFLAGS);
	
	for(i = init_flight ; i < ptrPlayer->ActiveAircraft ; i++)
	{
		j = i - init_flight;
		
		if(j >= GAME_GUI_AIRCRAFT_DATA_MAX_PAGE)
		{
			break;
		}
		
		FontPrintText(	&SmallFont,
						AircraftDataFlightNumber_X,
						AircraftDataFlightNumber_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
						ptrFlightData->strFlightNumber[ptrPlayer->ActiveAircraftList[i]]	);

		DepArrSpr.x = AircraftDataDirection_X;
		DepArrSpr.y = AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j);
		DepArrSpr.w = AIRCRAFT_DEPARTURE_ARRIVAL_ICON_SIZE;
					
		switch(ptrFlightData->FlightDirection[ptrPlayer->ActiveAircraftList[i]])
		{
			case ARRIVAL:
				DepArrSpr.u = orig_DepArr_u + AIRCRAFT_ARRIVAL_ICON_U;
			break;
			case DEPARTURE:
				DepArrSpr.u = orig_DepArr_u + AIRCRAFT_DEPARTURE_ICON_U;
			break;

			default:
			break;
		}

		GfxSortSprite(&DepArrSpr);

		DepArrSpr.u = orig_DepArr_u;
		
		FontSetFlags(&SmallFont, FONT_2HZ_FLASH);
		
		switch(ptrFlightData->State[ptrPlayer->ActiveAircraftList[i]])
		{
			case STATE_APPROACH:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Approach"	);
			break;
				
			case STATE_READY_FOR_TAKEOFF:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Takeoff"	);
			break;
			
			case STATE_LANDED:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Landed"	);
			break;
			
			case STATE_PARKED:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Parked"	);
			break;
			
			case STATE_UNBOARDING:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Unboard"	);
			break;

			case STATE_HOLDING_RWY:
				FontPrintText(	&SmallFont,
								AircraftDataDirection_X + AircraftDataState_X_Offset,
								AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
								"Holding"	);
			break;
			
			default:
			break;
		}
		
		FontSetFlags(&SmallFont, FONT_NOFLAGS);
		
		/*FontPrintText(	&SmallFont,
						AircraftDataPassengers_X,
						AircraftDataPassengers_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
						"%d pax.",
						ptrFlightData->Passengers[ptrPlayer->ActiveAircraftList[i]]	);*/

		FontPrintText(	&SmallFont,
						AircraftDataRemainingTime_X,
						AircraftDataRemainingTime_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
						"%ds",
						ptrFlightData->RemainingTime[ptrPlayer->ActiveAircraftList[i]] );
	}
}

bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER* ptrPlayer)
{
	// Aircraft list data cannot be shown under these conditions.
	
	if(	(ptrPlayer->SelectRunway == true)
						||
		(ptrPlayer->SelectTaxiwayParking == true)
						||
		(ptrPlayer->SelectTaxiwayRunway == true)	)
	{
		return true;
	}
	
	return false;
}

void GameGuiCalculateSlowScore(void)
{
	uint32_t currentScore = GameGetScore();
	uint32_t scoreSpeed;

	if(abs(slowScore - currentScore) < SLOW_SCORE_LOW_SPEED_MARGIN)
	{
		scoreSpeed = SLOW_SCORE_LOW_SPEED;

		if(abs(slowScore - currentScore) < SLOW_SCORE_LOW_SPEED)
		{
			slowScore = currentScore;
			return;
		}
	}
	else
	{
		scoreSpeed = SLOW_SCORE_HIGH_SPEED;
	}

	slowScore = (slowScore > currentScore)? (slowScore - scoreSpeed) : (slowScore + scoreSpeed);
}

void GameGuiShowScore(void)
{
	enum
	{
		SCORE_X = (X_SCREEN_RESOLUTION >> 1) - 64,
		SCORE_Y = 4,
	};

	FontPrintText(	&RadioFont,
					SCORE_X,
					SCORE_Y,
					"Score:%d", slowScore );
}

void GameGuiDrawUnboardingSequence(TYPE_PLAYER* ptrPlayer)
{
	uint8_t i;

	if(ptrPlayer->Unboarding == true)
	{
		for(i = ptrPlayer->UnboardingSequenceIdx; i < GAME_MAX_SEQUENCE_KEYS; i++)
		{
			if(ptrPlayer->UnboardingSequence[i] == 0)
			{
				break;
			}

			// TODO: Draw above the plane
			if(GameTwoPlayersActive() == true)
			{
				GfxDrawButton(48 + (16*i), Y_SCREEN_RESOLUTION - 32, ptrPlayer->UnboardingSequence[i]);
			}
			else
			{
				GfxDrawButton(128 + (16*i), Y_SCREEN_RESOLUTION - 32, ptrPlayer->UnboardingSequence[i]);
			}
		}
	}
}

void GameGuiClearPassengersLeft(void)
{
	GameGuiClearPassengersLeft_Flag = true;
}

bool GameGuiFinishedDialog(TYPE_PLAYER* ptrPlayer)
{
	GfxSaveDisplayData(&SecondDisplay);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	do
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			return true;
		}
		
		GfxSortSprite(&SecondDisplay);
		
		GsSortGPoly4(&PauseRect);

		FontPrintText( 	&SmallFont,
								AIRCRAFT_DATA_GSGPOLY4_X0 + 
								( (AIRCRAFT_DATA_GSGPOLY4_X1 - AIRCRAFT_DATA_GSGPOLY4_X0) >> 2),
								AIRCRAFT_DATA_GSGPOLY4_Y0 +
								( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
								"Level finished!"	);
		
		GfxDrawScene_Slow();
		
	}while(ptrPlayer->PadKeySinglePress_Callback(PAD_START) == false);
	
	return false;
}

void GameGuiAircraftCollision(TYPE_PLAYER* ptrPlayer)
{
	GfxSaveDisplayData(&SecondDisplay);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	do
	{		
		GfxSortSprite(&SecondDisplay);
		
		GsSortGPoly4(&PauseRect);

		FontPrintText( 	&SmallFont,
								AIRCRAFT_DATA_GSGPOLY4_X0 + 8,
								AIRCRAFT_DATA_GSGPOLY4_Y0 +
								( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
								"Collision between aircraft!"	);
		
		GfxDrawScene_Slow();
		
	}while(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == false);
}

void GameGuiBubbleStop(void)
{
	Serial_printf("GameGuiBubbleStop\n");
	GameGuiBubbleShowFlag = false;
}

void GameGuiBubbleStopVibration(void)
{
	Serial_printf("GameGuiBubbleStopVibration\n");
	GameGuiBubbleVibrationFlag = false;
}

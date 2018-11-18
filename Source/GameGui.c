/* *************************************
 *  Includes
 * *************************************/

#include "GameGui.h"
#include "System.h"
#include "Gfx.h"
#include "Game.h"
#include "LoadMenu.h"
#include "Timer.h"

/* *************************************
 *  Defines
 * *************************************/

#define GAME_GUI_AIRCRAFT_DATA_MAX_PAGE 4

#define SLOW_SCORE_LOW_SPEED_MARGIN     100
#define SLOW_SCORE_LOW_SPEED            5
#define SLOW_SCORE_HIGH_SPEED           10

/* **************************************
 *  Structs and enums                   *
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
 *  Local prototypes                    *
 * *************************************/

static void GameGuiShowAircraftData(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameGuiClearPassengersLeft(void);
static void GameGuiBubbleStop(void);
static void GameGuiBubbleStopVibration(void);

/* **************************************
 *  Local variables                     *
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

static const char* GameFileList[] = {   "DATA\\SPRITES\\BUBBLE.TIM"   ,
                                        "DATA\\FONTS\\FONT_1.FNT"     ,
                                        "DATA\\SPRITES\\DEPARR.TIM"   ,
                                        "DATA\\SPRITES\\PAGEUPDN.TIM" };

static void* GameFileDest[] = {	(GsSprite*)&BubbleSpr     	,
                                (TYPE_FONT*)&RadioFont      ,
                                (GsSprite*)&DepArrSpr       ,
                                (GsSprite*)&PageUpDownSpr   };

static uint32_t slowScore; // It will update slowly to actual score value

/* ***************************************************************************************
 *
 * @name: void GameGuiInit(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Reportedly, GameGui module initialization.
 *
 * ***************************************************************************************/
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

    if (firstLoad != false)
    {
        firstLoad = false;

        LoadMenu(   GameFileList,
                    GameFileDest,
                    sizeof (GameFileList) / sizeof (char*),
                    sizeof (GameFileDest) /sizeof (void*) );
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

    ShowAircraftPassengersTimer = TimerCreate( 20, true, GameGuiClearPassengersLeft);

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

    slowScore = 0;

    GameGuiBubbleShowFlag = false;

    FontSetSpacing(&RadioFont, RADIO_FONT_SPACING);
}

/* ***************************************************************************************
 *
 * @name: bool GameGuiPauseDialog(const TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Checks for pause event (player presses START button). If true, it traps the game
 *  on a while loop, waiting for user action.
 *
 * @remarks:
 *  Blocking function!
 *
 * ***************************************************************************************/
bool GameGuiPauseDialog(const TYPE_PLAYER* const ptrPlayer)
{
    GfxSaveDisplayData(&SecondDisplay);

    GfxSetGlobalLuminance(NORMAL_LUMINANCE);

    //DrawFBRect(0, 0, X_SCREEN_RESOLUTION, VRAM_H, 0, 0, 0);

    while (GfxIsGPUBusy() != false);

    do
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) != false)
        {
            return true;
        }

        GfxSortSprite(&SecondDisplay);

        GsSortGPoly4(&PauseRect);

        GfxDrawScene_Slow();

    }while (ptrPlayer->PadKeySinglePress_Callback(PAD_START) == false);

    return false;
}


/* ******************************************************************************************************
 *
 * @name: void GameGuiCalculateNextAircraftTime(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Updates ptrPlayer->NextAircraftTime with next aircraft remaining time.
 *
 * ******************************************************************************************************/
void GameGuiCalculateNextAircraftTime(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
    uint8_t i;
    uint16_t minRemainingTime = 0;

    for (i = 0; i < GAME_MAX_AIRCRAFT; i++)
    {
        if (    (ptrFlightData->State[i] == STATE_IDLE)
                                &&
                (   (ptrPlayer->FlightDirection & ptrFlightData->FlightDirection[i]) != 0)
                                &&
                (   (ptrFlightData->Hours[i] != 0)
                                    ||
                    (ptrFlightData->Minutes[i] != 0)    )   )
        {
            uint16_t seconds = (ptrFlightData->Hours[i] * 60) + (ptrFlightData->Minutes[i]);

            if (    (minRemainingTime == 0)
                                ||
                    (seconds < minRemainingTime)    )
            {
                minRemainingTime = seconds;
            }
        }
    }

    ptrPlayer->NextAircraftTime = minRemainingTime;
}

/* **********************************************************************************************
 *
 * @name: void GameGuiActiveAircraftPage(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Updates ptrPlayer->SelectedAircraft and ptrPlayer->FlightDataPage when new aircraft are
 *  added/removed. On the other hand, it also updates ptrPlayer->SelectedAircraft depending on
 *  user interaction.
 *
 * **********************************************************************************************/
void GameGuiActiveAircraftPage(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
    if (ptrPlayer->ActiveAircraft != 0)
    {
        while (ptrPlayer->ActiveAircraft <= ptrPlayer->SelectedAircraft)
        {
            ptrPlayer->SelectedAircraft--;
        }
    }

    if (ptrPlayer->ActiveAircraft != 0)
    {
        while (ptrPlayer->ActiveAircraft <= (uint8_t)(GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * ptrPlayer->FlightDataPage) )
        {
            ptrPlayer->FlightDataPage--;
        }
    }

    while (ptrPlayer->SelectedAircraft >= (uint8_t)(GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * (ptrPlayer->FlightDataPage + 1)) )
    {
        ptrPlayer->FlightDataPage++;
    }

    if (ptrPlayer->ShowAircraftData != false)
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_DOWN) != false)
        {
            if ( ( (ptrPlayer->SelectedAircraft + 1) < ptrPlayer->ActiveAircraft)
                                                &&
                ( (ptrPlayer->SelectedAircraft + 1) < ( (ptrPlayer->FlightDataPage + 1) * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE) ) )
            {
                ptrPlayer->SelectedAircraft++;
            }
        }

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_UP) != false)
        {
            if (ptrPlayer->SelectedAircraft > ( (ptrPlayer->FlightDataPage) * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE) )
            {
                ptrPlayer->SelectedAircraft--;
            }
        }

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_RIGHT) != false)
        {
            if (ptrPlayer->ActiveAircraft > (GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * (ptrPlayer->FlightDataPage + 1) ) )
            {
                ptrPlayer->FlightDataPage++;
                ptrPlayer->SelectedAircraft = ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE;
                Serial_printf("Selected Aircraft = %d\n",ptrPlayer->SelectedAircraft);
            }
        }

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_LEFT) != false)
        {
            if (ptrPlayer->FlightDataPage != 0)
            {
                ptrPlayer->FlightDataPage--;
                ptrPlayer->SelectedAircraft = ptrPlayer->FlightDataPage * GAME_GUI_AIRCRAFT_DATA_MAX_PAGE;
                Serial_printf("Selected Aircraft = %d\n",ptrPlayer->SelectedAircraft);
            }
        }
    }
}

/* **********************************************************************************************
 *
 * @name: void GameGuiAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Draws aircraft list for current player when ptrPlayer->ShowAircraftData state is active.
 *
 * **********************************************************************************************/
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

    enum
    {
        GAME_GUI_NEXT_AIRCRAFT_TIME_X = GAME_GUI_REMAINING_AIRCRAFT_X,
        GAME_GUI_NEXT_AIRCRAFT_TIME_Y = GAME_GUI_REMAINING_AIRCRAFT_Y + 8,
        GAME_GUI_NEXT_AIRCRAFT_TIME_X_2PLAYER = GAME_GUI_REMAINING_AIRCRAFT_X_2PLAYER - 6,
        GAME_GUI_NEXT_AIRCRAFT_TIME_Y_2PLAYER = GAME_GUI_REMAINING_AIRCRAFT_Y_2PLAYER + 8,
    };

    enum
    {
        AIRCRAFT_STOP_X = 128,
        AIRCRAFT_STOP_TEXT_X = AIRCRAFT_STOP_X + 32,
        AIRCRAFT_STOP_Y = AIRCRAFT_LOCK_TARGET_Y,
        AIRCRAFT_STOP_TEXT_Y = AIRCRAFT_STOP_Y + 4
    };

    if (ptrPlayer->ShowAircraftData != false)
    {
        AircraftDataGPoly4.attribute |= ENABLE_TRANS | TRANS_MODE(0);

        if (GameTwoPlayersActive() != false)
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

        if (GameTwoPlayersActive() != false)
        {
            FontPrintText(  &SmallFont,
                            GAME_GUI_REMAINING_AIRCRAFT_X_2PLAYER,
                            GAME_GUI_REMAINING_AIRCRAFT_Y_2PLAYER,
                            "Rem. aircraft: %d",
                            ptrFlightData->nRemainingAircraft       );

            FontPrintText(  &SmallFont,
                            GAME_GUI_NEXT_AIRCRAFT_TIME_X_2PLAYER,
                            GAME_GUI_NEXT_AIRCRAFT_TIME_Y_2PLAYER,
                            "Next aircraft: %d sec",
                            ptrPlayer->NextAircraftTime             );
        }
        else
        {
            FontPrintText(  &SmallFont,
                            GAME_GUI_REMAINING_AIRCRAFT_X,
                            GAME_GUI_REMAINING_AIRCRAFT_Y,
                            "Remaining aircraft: %d",
                            ptrFlightData->nRemainingAircraft       );

            FontPrintText(  &SmallFont,
                            GAME_GUI_NEXT_AIRCRAFT_TIME_X,
                            GAME_GUI_NEXT_AIRCRAFT_TIME_Y,
                            "Next aircraft: %d sec",
                            ptrPlayer->NextAircraftTime             );
        }

        if (ptrPlayer->ActiveAircraft != 0)
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

            if (GameTwoPlayersActive() != false)
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

            y_offset =  (short)(page_aircraft * AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H);

            if (GameTwoPlayersActive() != false)
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

            PageUpDownSpr.attribute |= GFX_2HZ_FLASH;

            if (ptrPlayer->ActiveAircraft > (GAME_GUI_AIRCRAFT_DATA_MAX_PAGE * (ptrPlayer->FlightDataPage + 1) ) )
            {
                orig_pageupdn_u = PageUpDownSpr.u;

                PageUpDownSpr.u = orig_pageupdn_u + AIRCRAFT_DATA_FLIGHT_PAGE_UP_U;

                if (GameTwoPlayersActive() != false)
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

            if (ptrPlayer->FlightDataPage != 0)
            {
                orig_pageupdn_u = PageUpDownSpr.u;

                PageUpDownSpr.u = orig_pageupdn_u + AIRCRAFT_DATA_FLIGHT_PAGE_DOWN_U;

                if (GameTwoPlayersActive() != false)
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

            if (ptrPlayer->LockTarget != false)
            {
                FontPrintText(&SmallFont, AIRCRAFT_LOCK_TARGET_TEXT_X, AIRCRAFT_LOCK_TARGET_TEXT_Y, "Unlock target");
            }
            else
            {
                FontPrintText(&SmallFont, AIRCRAFT_LOCK_TARGET_TEXT_X, AIRCRAFT_LOCK_TARGET_TEXT_Y, "Lock target");
            }

            if (ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_USER_STOPPED)
            {
                GfxDrawButton(AIRCRAFT_STOP_X, AIRCRAFT_STOP_Y, PAD_L1);
                FontPrintText(&SmallFont, AIRCRAFT_STOP_TEXT_X, AIRCRAFT_STOP_TEXT_Y, "Resume taxiing");
            }
            else if (ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_TAXIING)
            {
                GfxDrawButton(AIRCRAFT_STOP_X, AIRCRAFT_STOP_Y, PAD_L1);
                FontPrintText(&SmallFont, AIRCRAFT_STOP_TEXT_X, AIRCRAFT_STOP_TEXT_Y, "Stop immediately");
            }
        }
        else
        {
            if (GameTwoPlayersActive() != false)
            {
                FontPrintText(  &SmallFont,
                                AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER +
                                ( (AIRCRAFT_DATA_GSGPOLY4_X1_2PLAYER - AIRCRAFT_DATA_GSGPOLY4_X0_2PLAYER) >> 2),
                                AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER +
                                ( (AIRCRAFT_DATA_GSGPOLY4_Y2_2PLAYER - AIRCRAFT_DATA_GSGPOLY4_Y0_2PLAYER) >> 1),
                                "No flights!"   );
            }
            else
            {
                FontPrintText(  &SmallFont,
                                AIRCRAFT_DATA_GSGPOLY4_X0 +
                                ( (AIRCRAFT_DATA_GSGPOLY4_X1 - AIRCRAFT_DATA_GSGPOLY4_X0) >> 2),
                                AIRCRAFT_DATA_GSGPOLY4_Y0 +
                                ( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
                                "No flights!"   );
            }
        }
    }

}

/* **********************************************************************************************
 *
 * @name: void GameGuiBubbleShow(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Handler for GameGuiBubbleTimer.
 *
 * **********************************************************************************************/
void GameGuiBubbleShow(void)
{
    static TYPE_TIMER* GameGuiBubbleTimer = NULL;

    if (GameGuiBubbleTimer == NULL)
    {
        Serial_printf("Started GameGuiBubbleTimer...\n");
        GameGuiBubbleTimer = TimerCreate(50, false, &GameGuiBubbleStop);
    }
    else
    {
        TimerRestart(GameGuiBubbleTimer);
    }

    GameGuiBubbleShowFlag = true;
    GameGuiBubbleVibrationFlag = true;
}

/* **********************************************************************************************
 *
 * @name: void GameGuiBubble(TYPE_FLIGHT_DATA* ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  When a new aircraft is spawned, a bubble is temporarily shown on screen to notify players.
 *
 * **********************************************************************************************/
void GameGuiBubble(TYPE_FLIGHT_DATA* ptrFlightData)
{
    static bool GameGuiBubbleShowFlagOld;

    if (GameGuiBubbleShowFlag != false)
    {
        static TYPE_TIMER* GameGuiBubbleVibrationTimer = NULL;

        if (GameGuiBubbleShowFlagOld == false)
        {
            if (GameGuiBubbleVibrationTimer == NULL)
            {
                Serial_printf("Started GameGuiBubbleVibrationTimer...\n");
                GameGuiBubbleVibrationTimer = TimerCreate(20, false, &GameGuiBubbleStopVibration);
            }
            else
            {
                TimerRestart(GameGuiBubbleVibrationTimer);
            }
        }

        BubbleSpr.x = BUBBLE_SPRITE_X;
        BubbleSpr.y = BUBBLE_SPRITE_Y;

        if (GameGuiBubbleVibrationFlag != false)
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

/* **********************************************************************************************
 *
 * @name: void GameGuiClock(uint8_t hour, uint8_t min)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Shows clock time on screen.
 *
 * **********************************************************************************************/
void GameGuiClock(uint8_t hour, uint8_t min)
{
    enum
    {
        CLOCK_X = 16,
        CLOCK_Y = 4
    };

    static char strClock[6]; // HH:MM + \0 (6 characters needed)

    if (GameStartupFlag || System1SecondTick() != false)
    {
        memset(strClock, 0, 6);
        snprintf(strClock,6,"%02d:%02d",hour, min);
    }

    RadioFont.max_ch_wrap = 0;
    FontSetFlags(&RadioFont, FONT_NOFLAGS);
    FontPrintText(&RadioFont,CLOCK_X,CLOCK_Y,strClock);
}

/* **********************************************************************************************
 *
 * @name: void GameGuiShowPassengersLeft(TYPE_PLAYER* ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  TODO
 *
 * **********************************************************************************************/
void GameGuiShowPassengersLeft(TYPE_PLAYER* ptrPlayer)
{
    if (GameGuiClearPassengersLeft_Flag != false)
    {
        // Reset flag
        GameGuiClearPassengersLeft_Flag = false;
        ptrPlayer->PassengersLeftSelectedAircraft = 0;
    }

    if (ptrPlayer->PassengersLeftSelectedAircraft != 0)
    {
        if (GameTwoPlayersActive() != false)
        {
            FontPrintText(&SmallFont, 48, Y_SCREEN_RESOLUTION - 64, "%d passengers left", ptrPlayer->PassengersLeftSelectedAircraft);
        }
        else
        {
            FontPrintText(&SmallFont, 128, Y_SCREEN_RESOLUTION - 64, "%d pax. left", ptrPlayer->PassengersLeftSelectedAircraft);
        }
    }
}

/* **********************************************************************************************
 *
 * @name: void GameGuiShowAircraftData(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Reportedly, renders all aircraft data from a list on screen for a given TYPE_PLAYER and
 *  TYPE_FLIGHT_DATA instances.
 *
 * **********************************************************************************************/
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

    if (GameTwoPlayersActive() != false)
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

    for (i = init_flight ; i < ptrPlayer->ActiveAircraft ; i++)
    {
		const char* const strAircraftArray[MAX_STATES] = {	[STATE_APPROACH]			=	"Approach",
															[STATE_READY_FOR_TAKEOFF]	=	"Takeoff",
															[STATE_LANDED]				=	"Landed",
															[STATE_PARKED]				=	"Parked",
															[STATE_UNBOARDING]			=	"Unboard",
															[STATE_HOLDING_RWY]			=	"Holding",
															[STATE_USER_STOPPED]		=	"Stopped",
															[STATE_AUTO_STOPPED]		=	"Stopped"	};

		FL_STATE aircraftState;
		const char* strState;

        j = i - init_flight;

        if (j >= GAME_GUI_AIRCRAFT_DATA_MAX_PAGE)
        {
            break;
        }

        FontPrintText(  &SmallFont,
                        AircraftDataFlightNumber_X,
                        AircraftDataFlightNumber_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
                        ptrFlightData->strFlightNumber[ptrPlayer->ActiveAircraftList[i]]    );

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

        aircraftState = ptrFlightData->State[ptrPlayer->ActiveAircraftList[i]];

        strState = strAircraftArray[aircraftState];

        if (strState != NULL)
        {
			FontPrintText(  &SmallFont,
							AircraftDataDirection_X + AircraftDataState_X_Offset,
							AircraftDataDirection_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
							(char*)strState	);
		}

        FontSetFlags(&SmallFont, FONT_NOFLAGS);

        FontPrintText(  &SmallFont,
                        AircraftDataRemainingTime_X,
                        AircraftDataRemainingTime_Y + (AIRCRAFT_DATA_FLIGHT_GSGPOLY4_H * j),
                        "%ds",
                        ptrFlightData->RemainingTime[ptrPlayer->ActiveAircraftList[i]] );
    }
}

/* **********************************************************************************************
 *
 * @name: bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER* ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Tells under what conditions aircraft list data should not be shown.
 *
 * @return:
 *  true if we are inside a special condition which must NOT allow showing aircraft data.
 *  false otherwise.
 *
 * **********************************************************************************************/
bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER* ptrPlayer)
{
    // Aircraft list data cannot be shown under these conditions.

    if (    (ptrPlayer->SelectRunway != false)
                        ||
            (ptrPlayer->SelectTaxiwayParking != false)
                        ||
            (ptrPlayer->SelectTaxiwayRunway != false)   )
    {
        return true;
    }

    return false;
}

/* **********************************************************************************************
 *
 * @name: void GameGuiCalculateSlowScore(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Reportedly, it calculates a fake score which increments or decrements depending on actual
 *  score, which is immediately calculated. This "slow score" is then rendered on screen by
 *  calling GameGuiShowScore().
 *
 * **********************************************************************************************/
void GameGuiCalculateSlowScore(void)
{
    uint32_t currentScore = GameGetScore();
    uint32_t scoreSpeed;

    if (abs(slowScore - currentScore) < SLOW_SCORE_LOW_SPEED_MARGIN)
    {
        scoreSpeed = SLOW_SCORE_LOW_SPEED;

        if (abs(slowScore - currentScore) < SLOW_SCORE_LOW_SPEED)
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

/* **********************************************************************************************
 *
 * @name: void GameGuiShowScore(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Reportedly, it draws current score on screen.
 *
 * @remarks:
 *  "slowScore", which increments or decrements depending on actual score, is shown.
 *
 * **********************************************************************************************/
void GameGuiShowScore(void)
{
    enum
    {
        SCORE_X = (X_SCREEN_RESOLUTION >> 1) - 64,
        SCORE_Y = 4,
    };

    FontPrintText(  &RadioFont,
                    SCORE_X,
                    SCORE_Y,
                    "Score:%d", slowScore );
}

/* **********************************************************************************************
 *
 * @name: void GameGuiDrawUnboardingSequence(TYPE_PLAYER* ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  When player is unboarding passengers from an aircraft, this function renders unboarding
 *  button sequence on screen.
 *
 * **********************************************************************************************/
void GameGuiDrawUnboardingSequence(TYPE_PLAYER* ptrPlayer)
{
    uint8_t i;

    if (ptrPlayer->Unboarding != false)
    {
        for (i = ptrPlayer->UnboardingSequenceIdx; i < GAME_MAX_SEQUENCE_KEYS; i++)
        {
            if (ptrPlayer->UnboardingSequence[i] == 0)
            {
                break;
            }

            // TODO: Draw above the plane
            if (GameTwoPlayersActive() != false)
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

/* **********************************************************************************************
 *
 * @name: bool GameGuiFinishedDialog(TYPE_PLAYER* ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Executed when all aircraft has been dispatched and level has been finished.
 *
 * @return:
 *  true if player has pressed PAD_CROSS so gameplay is exited.
 *
 * **********************************************************************************************/
bool GameGuiFinishedDialog(TYPE_PLAYER* ptrPlayer)
{
    GfxSaveDisplayData(&SecondDisplay);

    GfxSetGlobalLuminance(NORMAL_LUMINANCE);

    do
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) != false)
        {
            return true;
        }

        GfxSortSprite(&SecondDisplay);

        GsSortGPoly4(&PauseRect);

        FontPrintText(  &SmallFont,
                        AIRCRAFT_DATA_GSGPOLY4_X0 +
                        ( (AIRCRAFT_DATA_GSGPOLY4_X1 - AIRCRAFT_DATA_GSGPOLY4_X0) >> 2),
                        AIRCRAFT_DATA_GSGPOLY4_Y0 +
                        ( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
                        "Level finished!"   );

        GfxDrawScene_Slow();

    }while (ptrPlayer->PadKeySinglePress_Callback(PAD_START) == false);

    return false;
}

/* **********************************************************************************************
 *
 * @name: void GameGuiAircraftCollision(TYPE_PLAYER* ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  When two or more aircraft collide, this function is called to show a "Game over"-like
 *  message on screen. Gameplay is finished at this point.
 *
 * @remarks:
 *  Blocking function.
 *
 * **********************************************************************************************/
void GameGuiAircraftCollision(TYPE_PLAYER* ptrPlayer)
{
    GfxSaveDisplayData(&SecondDisplay);

    GfxSetGlobalLuminance(NORMAL_LUMINANCE);

    do
    {
        GfxSortSprite(&SecondDisplay);

        GsSortGPoly4(&PauseRect);

        FontPrintText(  &SmallFont,
                                AIRCRAFT_DATA_GSGPOLY4_X0 + 8,
                                AIRCRAFT_DATA_GSGPOLY4_Y0 +
                                ( (AIRCRAFT_DATA_GSGPOLY4_Y2 - AIRCRAFT_DATA_GSGPOLY4_Y0) >> 1),
                                "Collision between aircraft!"   );

        GfxDrawScene_Slow();

    }while (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == false);
}

/* **********************************************************************************************
 *
 * @name: void GameGuiBubbleStop(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Callback executed when bubble must not be shown because of bubble timer timeout.
 *
 * **********************************************************************************************/
void GameGuiBubbleStop(void)
{
    Serial_printf("GameGuiBubbleStop\n");
    GameGuiBubbleShowFlag = false;
}

/* **********************************************************************************************
 *
 * @name: void GameGuiBubbleStopVibration(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Callback executed when bubble vibration must be stopped.
 *
 * **********************************************************************************************/
void GameGuiBubbleStopVibration(void)
{
    Serial_printf("GameGuiBubbleStopVibration\n");
    GameGuiBubbleVibrationFlag = false;
}

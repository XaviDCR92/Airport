/* *************************************
 *  Includes
 * *************************************/

#include "Game.h"
#include "Timer.h"
#include "LoadMenu.h"
#include "System.h"
#include "Camera.h"
#include "Aircraft.h"
#include "GameGui.h"
#include "EndAnimation.h"
#include "Sfx.h"
#include "Pad.h"
#include "Message.h"

/* *************************************
 *  Defines
 * *************************************/

#define GAME_MAX_RUNWAYS 16
#define GAME_MAX_AIRCRAFT_PER_TILE 4
#define FLIGHT_DATA_INVALID_IDX 0xFF

#define MIN_MAP_COLUMNS 8
#define MAX_MAP_COLUMNS 32

#define LEVEL_HEADER_SIZE 64
#define COLUMNS_PER_TILESET 4
#define ROWS_PER_TILESET COLUMNS_PER_TILESET
#define LEVEL_MAGIC_NUMBER_SIZE 3
#define LEVEL_MAGIC_NUMBER_STRING "ATC"
#define LEVEL_TITLE_SIZE 24
#define TILE_MIRROR_FLAG (0x80)

#define GAME_INVALID_TILE_SELECTION ( (uint16_t)0xFFFF )

#define GAME_MINIMUM_PARKING_SPAWN_TIME (2 * TIMER_PRESCALER_1_SECOND) // 2 seconds

/* **************************************
 *  Structs and enums                   *
 * *************************************/

typedef struct t_rwyentrydata
{
    DIRECTION Direction;
    uint16_t rwyEntryTile;
    int8_t rwyStep;
    uint16_t rwyHeader;
}TYPE_RWY_ENTRY_DATA;

typedef struct t_GameLevelBuffer_UVData
{
    short u;
    short v;
}TYPE_TILE_UV_DATA;

enum
{
    MOUSE_W = 8,
    MOUSE_H = 8,
    MOUSE_X = (X_SCREEN_RESOLUTION >> 1),
    MOUSE_Y = (Y_SCREEN_RESOLUTION >> 1),
    MOUSE_X_2PLAYER = (X_SCREEN_RESOLUTION >> 2),
    MOUSE_Y_2PLAYER = (Y_SCREEN_RESOLUTION >> 1)
};

enum
{
    LOST_FLIGHT_PENALTY = 4000,
    SCORE_REWARD_TAXIING = 200,
    SCORE_REWARD_FINAL = 400,
    SCORE_REWARD_UNLOADING = 300,
    SCORE_REWARD_TAKEOFF = 200,
    SCORE_REWARD_FINISH_FLIGHT = 1000
};

enum
{
    UNBOARDING_KEY_SEQUENCE_EASY = 4,
    UNBOARDING_KEY_SEQUENCE_MEDIUM = 6,
    UNBOARDING_KEY_SEQUENCE_HARD = GAME_MAX_SEQUENCE_KEYS,
    UNBOARDING_PASSENGERS_PER_SEQUENCE = 100
};

enum
{
    TILE_GRASS,
    TILE_ASPHALT_WITH_BORDERS,
    TILE_WATER,
    TILE_ASPHALT,

    TILE_RWY_MID,
    TILE_RWY_START_1,
    TILE_RWY_START_2,
    TILE_PARKING,

    TILE_PARKING_2,
    TILE_TAXIWAY_INTERSECT_GRASS,
    TILE_TAXIWAY_GRASS,
    TILE_TAXIWAY_CORNER_GRASS,

    TILE_HALF_WATER_1,
    TILE_HALF_WATER_2,
    TILE_RWY_HOLDING_POINT,
    TILE_RWY_HOLDING_POINT_2,

    TILE_RWY_EXIT,
    TILE_TAXIWAY_CORNER_GRASS_2,
    TILE_TAXIWAY_4WAY_CROSSING,
    TILE_RWY_EXIT_2,

    LAST_TILE_TILESET1 = TILE_RWY_EXIT_2,

    TILE_UNUSED_1,
    TILE_TAXIWAY_CORNER_GRASS_3,

    FIRST_TILE_TILESET2 = TILE_UNUSED_1,
    LAST_TILE_TILESET2 = TILE_TAXIWAY_CORNER_GRASS_3
};

enum
{
    SOUND_M1_INDEX,
    SOUND_W1_INDEX,

    MAX_RADIO_CHATTER_SOUNDS
}RADIO_CHATTER_VOICE_NUMBERS;

/* *************************************
 *  Local Prototypes
 * *************************************/

static void GameInit(const TYPE_GAME_CONFIGURATION* const pGameCfg);
static void GameInitTileUVTable(void);
static bool GameExit(void);
static void GameLoadLevel(const char* path);
static bool GamePause(void);
static void GameFinished(const uint8_t i);
static void GameEmergencyMode(void);
static void GameCalculations(void);
static void GamePlayerHandler(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GamePlayerAddWaypoint(TYPE_PLAYER* const ptrPlayer);
static void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* const ptrPlayer, uint16_t tile);
static void GameGraphics(void);
static void GameRenderTerrainPrecalculations(TYPE_PLAYER* const ptrPlayer, const TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameRenderTerrain(TYPE_PLAYER* const ptrPlayer);
static void GameClock(void);
static void GameClockFlights(const uint8_t i);
static void GameAircraftState(const uint8_t i);
static void GameActiveAircraft(const uint8_t i);
static void GameStateShowAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameSelectAircraftFromList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameStateSelectRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameStateSelectTaxiwayRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameStateSelectTaxiwayParking(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameStateLockTarget(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* const ptrPlayer);
static void GameSelectAircraftWaypoint(TYPE_PLAYER* const ptrPlayer);
static void GameGetRunwayArray(void);
static void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz);
static void GameAssignRunwaytoAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static bool GamePathToTile(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameDrawMouse(TYPE_PLAYER* const ptrPlayer);
static void GameStateUnboarding(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameGenerateUnboardingSequence(TYPE_PLAYER* const ptrPlayer);
static void GameCreateTakeoffWaypoints(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData, uint8_t aircraftIdx);
static void GameGetRunwayEntryTile(uint8_t aircraftIdx, TYPE_RWY_ENTRY_DATA* ptrRwyEntry);
static void GameActiveAircraftList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
static void GameRemainingAircraft(const uint8_t i);
static void GameMinimumSpawnTimeout(void);
static void GameRenderBuildingAircraft(TYPE_PLAYER* const ptrPlayer);
static void GameGetAircraftTilemap(const uint8_t i);
static bool GameWaypointCheckExisting(TYPE_PLAYER* const ptrPlayer, uint16_t temp_tile);
static void GameDrawBackground(void);
static DIRECTION GameGetRunwayDirection(uint16_t rwyHeader);
static DIRECTION GameGetParkingDirection(uint16_t parkingTile);

/* *************************************
 *  Global Variables
 * *************************************/

bool GameStartupFlag;
uint32_t GameScore;

/* *************************************
 *  Local Variables
 * *************************************/

// Sprites
static GsSprite GameTilesetSpr;
static GsSprite GameTileset2Spr;
static GsSprite GamePlaneSpr;
static GsSprite GameMouseSpr;
static GsSprite GameBuildingSpr;

static uint16_t GameRwy[GAME_MAX_RUNWAYS];
static TYPE_FLIGHT_DATA FlightData;
static uint16_t GameUsedRwy[GAME_MAX_RUNWAYS];
static uint16_t GameSelectedTile;
static TYPE_TIMER* GameSpawnMinTime;
static bool spawnMinTimeFlag;
static bool aircraftCreated;
static bool GameAircraftCollisionFlag;
static uint8_t GameAircraftCollisionIdx;
static uint8_t GameAircraftTilemap[GAME_MAX_MAP_SIZE][GAME_MAX_AIRCRAFT_PER_TILE];

static TYPE_TILE_UV_DATA GameLevelBuffer_UVData[GAME_MAX_MAP_SIZE];

// Radio chatter
static SsVag ApproachSnds[MAX_RADIO_CHATTER_SOUNDS];
static SsVag TowerFinalSnds[MAX_RADIO_CHATTER_SOUNDS];

// Takeoff sounds
static SsVag TakeoffSnd;

// Beep sounds (taxiway/parking accept)
static SsVag BeepSnd;

// Instances for player-specific data
static TYPE_PLAYER PlayerData[MAX_PLAYERS] =
{
    [PLAYER_ONE] =
    {
        .PadKeyPressed_Callback = &PadOneKeyPressed,
        .PadKeyReleased_Callback = &PadOneKeyReleased,
        .PadKeySinglePress_Callback = &PadOneKeySinglePress,
        .PadDirectionKeyPressed_Callback = &PadOneDirectionKeyPressed,
        .PadLastKeySinglePressed_Callback = &PadOneGetLastKeySinglePressed
    },

    [PLAYER_TWO] =
    {
        .PadKeyPressed_Callback = &PadTwoKeyPressed,
        .PadKeyReleased_Callback = &PadTwoKeyReleased,
        .PadDirectionKeyPressed_Callback = &PadTwoDirectionKeyPressed,
        .PadKeySinglePress_Callback = &PadTwoKeySinglePress,
        .PadLastKeySinglePressed_Callback = &PadTwoGetLastKeySinglePressed
    }

};

static void* GamePltDest[] = {(TYPE_FLIGHT_DATA*)&FlightData    };

static uint16_t levelBuffer[GAME_MAX_MAP_SIZE];

static uint8_t GameLevelColumns;
static uint16_t GameLevelSize;

static char GameLevelTitle[LEVEL_TITLE_SIZE];

//Game local time
static uint8_t GameHour;
static uint8_t GameMinutes;

//Local flag for two-player game mode. Obtained from Menu
static bool twoPlayers;

// Determines whether game has finished or not.
bool levelFinished;

/* ***************************************************************************************
 *
 * @name: void Game(TYPE_GAME_CONFIGURATION* pGameCfg)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Game main loop. Called by "Menu" module.
 *
 * @remarks:
 *
 * ***************************************************************************************/
void Game(const TYPE_GAME_CONFIGURATION* const pGameCfg)
{
    twoPlayers = pGameCfg->TwoPlayers;
    GameInit(pGameCfg);

    while (1)
    {
        if (GameExit())
        {
            break;
        }

        GameEmergencyMode();

        GameCalculations();

        GameGraphics();

        if (GameStartupFlag)
        {
            GameStartupFlag = false;
        }
    }

    GfxDisableSplitScreen();

    EndAnimation();
}

/* ***************************************************************************************
 *
 * @name: bool GameExit(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Evaluates special conditions which end current game and return to main menu.
 *
 * @returns:
 *  True if game has to be exitted, false otherwise.
 *
 * ***************************************************************************************/
static bool GameExit(void)
{
    if (levelFinished)
    {
        // Exit game on level finished.
        if (GameGuiFinishedDialog(&PlayerData[PLAYER_ONE]))
        {
            return true;
        }
    }

    if (GamePause())
    {
        // Exit game if player desires to exit.
        return true;
    }

    if (GameAircraftCollisionFlag)
    {
        GameGuiAircraftCollision(&PlayerData[PLAYER_ONE]);
        return true;
    }

    return false;
}

/* ***************************************************************************************
 *
 * @name: bool GamePause(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  When PAD_START is pressed, it draws a rectangle on top of the screen and the game halts.
 *
 * @remarks:
 *
 * ***************************************************************************************/
static bool GamePause(void)
{
    uint8_t i;

    if (GameStartupFlag)
    {
        return false;
    }

    for (i = 0 ; i < MAX_PLAYERS ; i++)
    {
        const TYPE_PLAYER* const ptrPlayer = &PlayerData[i];

        // Run player-specific functions for each player
        if (ptrPlayer->Active)
        {
            //Serial_printf("Released callback = 0x%08X\n", ptrPlayer->PadKeySinglePress_Callback);
            if (ptrPlayer->PadKeySinglePress_Callback(PAD_START))
            {
                Serial_printf("Player %d set pause_flag to true!\n",i);

                // Blocking function:
                //  * Returns true if player pointed to by ptrPlayer wants to exit game
                //  * Returns false if player pointed to by ptrPlayer wants to resume game
                return GameGuiPauseDialog(ptrPlayer);
            }
        }
    }

    return false;
}

/* ***************************************************************************************
 *
 * @name: void GameInit(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Game basic parameters initialization.
 *
 * @remarks:
 *  Tilesets and buildings are only loaded on first game. Then, only PLT is loaded.
 *
 * ***************************************************************************************/
void GameInit(const TYPE_GAME_CONFIGURATION* const pGameCfg)
{
    static const char* const GameFileList[] =
    {
        "DATA\\SPRITES\\TILESET1.TIM",
        "DATA\\SPRITES\\TILESET2.TIM",
        "DATA\\SPRITES\\GAMEPLN.TIM",
        "DATA\\SPRITES\\MOUSE.TIM",
        "DATA\\SPRITES\\BLDNGS1.TIM",
        "DATA\\SOUNDS\\RCPW1A1.VAG",
        "DATA\\SOUNDS\\RCPM1A1.VAG",
        "DATA\\SOUNDS\\RCTM1F1.VAG",
        "DATA\\SOUNDS\\TAKEOFF1.VAG",
        "DATA\\SOUNDS\\BEEP.VAG"
    };

    static void* GameFileDest[] =
    {
        &GameTilesetSpr,
        &GameTileset2Spr,
        &GamePlaneSpr,
        &GameMouseSpr,
        &GameBuildingSpr,
        &ApproachSnds[SOUND_M1_INDEX],
        &ApproachSnds[SOUND_W1_INDEX],
        &TowerFinalSnds[SOUND_M1_INDEX],
        &TakeoffSnd,
        &BeepSnd
    };

    uint8_t i;
    static bool loaded;

    GameStartupFlag = true;

    // Has to be initialized before loading *.PLT files inside LoadMenu().
    MessageInit();

    if (loaded == false)
    {
        loaded = true;

        LOAD_FILES(GameFileList, GameFileDest);

        GameSpawnMinTime = TimerCreate(GAME_MINIMUM_PARKING_SPAWN_TIME, false, GameMinimumSpawnTimeout);
    }

    LoadMenu(   &pGameCfg->PLTPath,
                GamePltDest,
                sizeof (char),
                ARRAY_SIZE(GamePltDest));

    GameLoadLevel(pGameCfg->LVLPath);

    GameGuiInit();

    memset(GameRwy, 0, GAME_MAX_RUNWAYS * sizeof (uint16_t) );

    memset(GameUsedRwy, 0, GAME_MAX_RUNWAYS * sizeof (uint16_t) );

    PlayerData[PLAYER_ONE].Active = true;
    PlayerData[PLAYER_ONE].FlightDataPage = 0;
    PlayerData[PLAYER_ONE].UnboardingSequenceIdx = 0;

    PlayerData[PLAYER_ONE].ShowAircraftData = false;
    PlayerData[PLAYER_ONE].SelectRunway = false;
    PlayerData[PLAYER_ONE].SelectTaxiwayRunway = false;
    PlayerData[PLAYER_ONE].SelectTaxiwayParking = false;
    PlayerData[PLAYER_ONE].InvalidPath = false;
    PlayerData[PLAYER_ONE].LockTarget = false;
    PlayerData[PLAYER_ONE].Unboarding = false;

    memset(PlayerData[PLAYER_ONE].UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS * sizeof (unsigned short) );
    memset(PlayerData[PLAYER_ONE].TileData, 0, GAME_MAX_MAP_SIZE * sizeof (TYPE_TILE_DATA));

    PlayerData[PLAYER_TWO].Active = twoPlayers? true : false;

    if (PlayerData[PLAYER_TWO].Active)
    {
        PlayerData[PLAYER_TWO].FlightDataPage = 0;
        PlayerData[PLAYER_TWO].UnboardingSequenceIdx = 0;

        PlayerData[PLAYER_TWO].ShowAircraftData = false;
        PlayerData[PLAYER_TWO].SelectRunway = false;
        PlayerData[PLAYER_TWO].SelectTaxiwayRunway = false;
        PlayerData[PLAYER_TWO].SelectTaxiwayParking = false;
        PlayerData[PLAYER_TWO].InvalidPath = false;
        PlayerData[PLAYER_TWO].LockTarget = false;
        PlayerData[PLAYER_TWO].Unboarding = false;

        memset(PlayerData[PLAYER_TWO].UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS * sizeof (unsigned short) );
        memset(PlayerData[PLAYER_TWO].TileData, 0, GAME_MAX_MAP_SIZE * sizeof (TYPE_TILE_DATA));

        // On 2-player mode, one player controls departure flights and
        // other player controls arrival flights.
        PlayerData[PLAYER_ONE].FlightDirection = DEPARTURE;
        PlayerData[PLAYER_TWO].FlightDirection = ARRIVAL;
    }
    else
    {
        PlayerData[PLAYER_ONE].FlightDirection = DEPARTURE | ARRIVAL;
    }

    for (i = 0; i < MAX_PLAYERS ; i++)
    {
        CameraInit(&PlayerData[i]);
        PlayerData[i].ShowAircraftData = false;
        PlayerData[i].SelectRunway = false;
        PlayerData[i].SelectTaxiwayRunway = false;
        PlayerData[i].LockTarget = false;
        PlayerData[i].SelectedAircraft = 0;
        PlayerData[i].FlightDataPage = 0;
        memset(&PlayerData[i].Waypoints, 0, sizeof (uint16_t) * PLAYER_MAX_WAYPOINTS);
        PlayerData[i].WaypointIdx = 0;
        PlayerData[i].LastWaypointIdx = 0;
        PlayerData[i].RemainingAircraft = 0;
    }

    aircraftCreated = false;
    GameAircraftCollisionFlag = false;
    GameAircraftCollisionIdx = 0;

    if (GameTwoPlayersActive())
    {
        GameMouseSpr.x = MOUSE_X_2PLAYER;
        GameMouseSpr.y = MOUSE_Y_2PLAYER;
    }
    else
    {
        GameMouseSpr.x = MOUSE_X;
        GameMouseSpr.y = MOUSE_Y;
    }

    GameMouseSpr.w = MOUSE_W;
    GameMouseSpr.h = MOUSE_H;
    GameMouseSpr.attribute = COLORMODE(COLORMODE_16BPP);
    GameMouseSpr.r = NORMAL_LUMINANCE;
    GameMouseSpr.g = NORMAL_LUMINANCE;
    GameMouseSpr.b = NORMAL_LUMINANCE;

    spawnMinTimeFlag = false;

    GameScore = 0;

    GameGetRunwayArray();

    GameSelectedTile = 0;

    levelFinished = false;

    GameInitTileUVTable();

    AircraftInit();

    LoadMenuEnd();

    GfxSetGlobalLuminance(0);

    {
        const uint32_t track = SystemRand(GAMEPLAY_FIRST_TRACK, GAMEPLAY_LAST_TRACK);

        SfxPlayTrack(track);
    }
}

/* ***************************************************************************************
 *
 * @name: void GameEmergencyMode(void)
 *
 * @author: Xavier Del Campo
 *
 *
 * @brief:
 *  Draws a blue rectangle on top of the screen whenever one of the two active controllers
 *  (e.g.: pad1 on single player mode, pad1 || pad2 on two player mode) is disconnected.
 *
 * @remarks:
 *  See PSX_PollPad(), defined on psx.h, and Pad module for further information.
 *
 * ***************************************************************************************/
void GameEmergencyMode(void)
{
    uint8_t i;
    uint8_t disconnected_players = 0x00;
    static bool (*const PadXConnected[MAX_PLAYERS])(void) =
    {
        [PLAYER_ONE] = &PadOneConnected,
        [PLAYER_TWO] = &PadTwoConnected
    };

    enum
    {
        PAD_DISCONNECTED_TEXT_X = 48,
        PAD_DISCONNECTED_TEXT_Y = 48,
        PAD_DISCONNECTED_TEXT_Y_OFFSET_BITSHIFT = 5
    };

    do
    {
        bool enabled = false;

        if (SystemGetEmergencyMode())
        {
            enum
            {
                ERROR_RECT_X = 32,
                ERROR_RECT_W = X_SCREEN_RESOLUTION - (ERROR_RECT_X << 1),

                ERROR_RECT_Y = 16,
                ERROR_RECT_H = Y_SCREEN_RESOLUTION - (ERROR_RECT_Y << 1),

                ERROR_RECT_R = 0,
                ERROR_RECT_G = 32,
                ERROR_RECT_B = NORMAL_LUMINANCE
            };

            static const GsRectangle errorRct =
            {
                .x = ERROR_RECT_X,
                .w = ERROR_RECT_W,
                .y = ERROR_RECT_Y,
                .h = ERROR_RECT_H,
                .r = ERROR_RECT_R,
                .g = ERROR_RECT_G,
                .b = ERROR_RECT_B
            };

            // One of the pads has been disconnected during gameplay
            // Show an error screen until it is disconnected again.

            GsSortCls(0,0,0);
            GsSortRectangle((GsRectangle*)&errorRct);

            for (i = 0; i < MAX_PLAYERS; i++)
            {
                if (disconnected_players & (1 << i) )
                {
                    FontPrintText(  &SmallFont,
                                    PAD_DISCONNECTED_TEXT_X,
                                    PAD_DISCONNECTED_TEXT_Y + (i << PAD_DISCONNECTED_TEXT_Y_OFFSET_BITSHIFT),
                                    "Pad %s disconnected", i? "right" : "left"    );
                }
            }

            GfxDrawScene_Slow();
        }

        for (i = 0; i < MAX_PLAYERS; i++)
        {
            TYPE_PLAYER* const ptrPlayer = &PlayerData[i];

            if (ptrPlayer->Active)
            {
                if (PadXConnected[i]() == false)
                {
                    enabled = true;
                    disconnected_players |= 1 << i;
                }
                else
                {
                    disconnected_players &= ~(1 << i);
                }
            }
        }

        SystemSetEmergencyMode(enabled);

    } while (SystemGetEmergencyMode());
}

/* ***************************************************************************************
 *
 * @name: void GameGetAircraftTilemap(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint8_t i:
 *      Index for FlightData table.
 *
 * @brief:
 *  On each cycle, it creates a 2-dimensional array relating aircraft indexes against
 *  tile numbers.
 *
 * @remarks:
 *
 * ***************************************************************************************/
static void GameGetAircraftTilemap(const uint8_t i)
{
    if (i == 0)
    {
        memset(GameAircraftTilemap, FLIGHT_DATA_INVALID_IDX, sizeof (GameAircraftTilemap) );
    }

    if (FlightData.State[i] != STATE_IDLE)
    {
        const uint16_t tileNr = AircraftGetTileFromFlightDataIndex(i);

        uint8_t j;

        for (j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
        {
            if (GameAircraftTilemap[tileNr][j] == FLIGHT_DATA_INVALID_IDX)
            {
                break;
            }
        }

        GameAircraftTilemap[tileNr][j] = i;
    }
}

/* ***************************************************************************************
 *
 * @name: void GameCalculations(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  First half of game execution. Executed when GPU is still drawing previous frame.
 *  Calculates all new states and values.
 *
 * @remarks:
 *  Since the GPU takes a long time to draw a frame, GameCalculations() should be used
 *  for all CPU-intensive tasks.
 *
 * ***************************************************************************************/
void GameCalculations(void)
{
    uint8_t i;

    GameClock();

    // Set level finished flag. It will
    // reset if at least one flight is still pending.
    levelFinished = true;

    for (i = 0; i < FlightData.nAircraft; i++)
    {
        GameFinished(i);
        GameClockFlights(i);
        GameAircraftState(i);
        GameActiveAircraft(i);
        GameRemainingAircraft(i);
        GameGetAircraftTilemap(i);
    }

    MessageHandler();
    AircraftHandler();
    GameGuiCalculateSlowScore();

    for (i = 0 ; i < MAX_PLAYERS ; i++)
    {
        // Run player-specific functions for each player
        if (PlayerData[i].Active)
        {
            GamePlayerHandler(&PlayerData[i], &FlightData);
        }
    }
}

/* ***************************************************************************************
 *
 * @name: void GamePlayerHandler(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Calls all routines attached to a player.
 *
 * @remarks:
 *
 * ***************************************************************************************/
void GamePlayerHandler(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    ptrPlayer->SelectedTile = 0;    // Reset selected tile if no states
                                    // which use this are currently active.
    ptrPlayer->InvalidPath = false; // Do the same thing for "InvalidPath".

    // Recalculate ptrPlayer->SelectedAircraft. In case new aircraft appear, we may be pointing
    // to a incorrect instance.
    GameActiveAircraftList(ptrPlayer, ptrFlightData);

    if (GameAircraftCollisionFlag)
    {
        TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(GameAircraftCollisionIdx);
        CameraMoveToIsoPos(ptrPlayer, IsoPos);
    }

    if (System1SecondTick())
    {
        GameGuiCalculateNextAircraftTime(ptrPlayer, ptrFlightData);
    }

    GameStateUnboarding(ptrPlayer, ptrFlightData);
    GameStateLockTarget(ptrPlayer, ptrFlightData);
    GameStateSelectRunway(ptrPlayer, ptrFlightData);
    GameStateSelectTaxiwayRunway(ptrPlayer, ptrFlightData);
    GameStateSelectTaxiwayParking(ptrPlayer, ptrFlightData);
    GameStateShowAircraft(ptrPlayer, ptrFlightData);
    CameraHandler(ptrPlayer);
    GameRenderTerrainPrecalculations(ptrPlayer, ptrFlightData);
    GameGuiActiveAircraftPage(ptrPlayer, ptrFlightData);
    GameSelectAircraftFromList(ptrPlayer, ptrFlightData);
}

/* *******************************************************************
 *
 * @name: void GameClock(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Handles game clock later rendered on GameGui.
 *
 * @remarks:
 *
 * *******************************************************************/

void GameClock(void)
{
    if (System1SecondTick())
    {
        if (++GameMinutes >= 60)
        {
            GameMinutes = 0;

            if (++GameHour >= 24)
            {
                GameHour = 0;
            }
        }
    }
}

/* *******************************************************************
 *
 * @name: void GameClockFlights(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint8_t i:
 *      Index for FlightData table.
 *
 * @brief:
 *  Handles hours/minutes values for all active aircraft.
 *
 * @remarks:
 *
 * *******************************************************************/

static void GameClockFlights(const uint8_t i)
{
    if (System1SecondTick())
    {
        if (FlightData.State[i] != STATE_IDLE)
        {
            if (FlightData.RemainingTime[i] != 0)
            {
                FlightData.RemainingTime[i]--;
            }
        }
        else
        {
            if ((FlightData.Minutes[i] == 0)
                        &&
                FlightData.Hours[i])
            {
                FlightData.Minutes[i] = 60;
                FlightData.Hours[i]--;
            }

            if (FlightData.Minutes[i])
            {
                FlightData.Minutes[i]--;
            }
        }
    }
}

/* *******************************************************************
 *
 * @name: void GameGraphics(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Second half of game execution. Once GameCalculations() has ended,
 *  states and new values have been calculated and all primitives are
 *  rendered depending on the obtained results.
 *
 * @remarks:
 *  It is advisable to keep CPU usage here, as once this function is
 *  entered, GPU is waiting for primitive data. Always try to move
 *  CPU-intensive operations to GameCalculations().
 *
 * *******************************************************************/

void GameGraphics(void)
{
    uint8_t i;
    bool split_screen = false;

    // Caution: blocking function!
    MessageRender();

    if (twoPlayers)
    {
        split_screen = true;
    }

    if (GfxGetGlobalLuminance() < NORMAL_LUMINANCE)
    {
        // Fading from black effect on startup.
        GfxIncreaseGlobalLuminance(1);
    }

    for (i = 0; i < MAX_PLAYERS ; i++)
    {
        TYPE_PLAYER* const ptrPlayer = &PlayerData[i];

        if (ptrPlayer->Active)
        {
            if (split_screen)
            {
                GfxSetSplitScreen(i);
            }

            // Draw half split screen for each player
            // only if 2-player mode is active. Else, render
            // the whole screen as usual.

            // Render background first.
            GameDrawBackground();

            // Then ground tiles must be rendered.

            GameRenderTerrain(ptrPlayer);

            // Ground tiles are now rendered. Now, depending on building/aircraft
            // positions, determine in what order they should be rendered.

            GameRenderBuildingAircraft(ptrPlayer);

            GameGuiAircraftList(ptrPlayer, &FlightData);

            GameGuiShowPassengersLeft(ptrPlayer);

            GameDrawMouse(ptrPlayer);

            GameGuiDrawUnboardingSequence(ptrPlayer);

            if (split_screen)
            {
                GfxDrawScene_NoSwap();
                while (GsIsDrawing());
            }
        }
    }

    // Avoid changing drawing environment twice on 1-player mode
    // as it doesn't make any sense.
    if (split_screen)
    {
        GfxDisableSplitScreen();
    }

    // Draw common elements for both players (messages, clock...)

    GameGuiBubble(&FlightData);

    GameGuiClock(GameHour,GameMinutes);

    GameGuiShowScore();

    if (split_screen)
    {
        GfxDrawScene_NoSwap();
    }

    GfxDrawScene();
}

/* *******************************************************************
 *
 * @name: void GameDrawBackground(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Draws the background used for main gameplay.
 *
 * @remarks:
 *  Must be called before rendering anything else on screen!
 *
 * *******************************************************************/

static void GameDrawBackground(void)
{
    enum
    {
        BG_POLY4_R0 = 0,
        BG_POLY4_G0 = 0,
        BG_POLY4_B0 = 16,

        BG_POLY4_R1 = 0,
        BG_POLY4_G1 = 0,
        BG_POLY4_B1 = 16,

        BG_POLY4_R2 = 0,
        BG_POLY4_G2 = 0,
        BG_POLY4_B2 = 80,

        BG_POLY4_R3 = 0,
        BG_POLY4_G3 = 0,
        BG_POLY4_B3 = 80
    };

    static GsGPoly4 BgPoly4 =
    {
        .x[0] = 0,
        .x[2] = 0,

        .y[0] = 0,
        .y[1] = 0,

        .r[0] = BG_POLY4_R0,
        .g[0] = BG_POLY4_G0,
        .b[0] = BG_POLY4_B0,

        .r[1] = BG_POLY4_R1,
        .g[1] = BG_POLY4_G1,
        .b[1] = BG_POLY4_B1,

        .r[2] = BG_POLY4_R2,
        .g[2] = BG_POLY4_G2,
        .b[2] = BG_POLY4_B2,

        .r[3] = BG_POLY4_R3,
        .g[3] = BG_POLY4_G3,
        .b[3] = BG_POLY4_B3
    };

    BgPoly4.x[1] = GfxGetDrawEnvWidth();
    BgPoly4.x[3] = BgPoly4.x[1];

    BgPoly4.y[2] = GfxGetDrawEnvHeight();
    BgPoly4.y[3] = BgPoly4.y[2];

    GsSortGPoly4((GsGPoly4*)&BgPoly4);
}

/* *******************************************************************
 *
 * @name: void GameRenderBuildingAircraft(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to player data structure.
 *
 * @brief:
 *  Determines rendering order depending on building/aircraft
 *  isometric position data.
 *
 * @remarks:
 *
 * *******************************************************************/

void GameRenderBuildingAircraft(TYPE_PLAYER* const ptrPlayer)
{
    enum
    {
        BUILDING_NONE,
        BUILDING_HANGAR,
        BUILDING_ILS,
        BUILDING_ATC_TOWER,
        BUILDING_ATC_LOC,
        BUILDING_TERMINAL,
        BUILDING_TERMINAL_2,
        BUILDING_GATE,

        LAST_BUILDING = BUILDING_GATE,
        MAX_BUILDING_ID
    };

    enum
    {
        BUILDING_ATC_LOC_OFFSET_X = TILE_SIZE >> 1,
        BUILDING_ATC_LOC_OFFSET_Y = TILE_SIZE >> 1,

        BUILDING_ILS_OFFSET_X = 0,
        BUILDING_ILS_OFFSET_Y = 0,

        BUILDING_GATE_OFFSET_X = (TILE_SIZE >> 1) - 4,
        BUILDING_GATE_OFFSET_Y = 0,

        BUILDING_HANGAR_OFFSET_X = 4,
        BUILDING_HANGAR_OFFSET_Y = TILE_SIZE >> 1,

        BUILDING_TERMINAL_OFFSET_X = 0,
        BUILDING_TERMINAL_OFFSET_Y = TILE_SIZE >> 1,

        BUILDING_TERMINAL_2_OFFSET_X = BUILDING_TERMINAL_OFFSET_X,
        BUILDING_TERMINAL_2_OFFSET_Y = BUILDING_TERMINAL_OFFSET_Y,

        BUILDING_ATC_TOWER_OFFSET_X = TILE_SIZE >> 2,
        BUILDING_ATC_TOWER_OFFSET_Y = TILE_SIZE >> 1,
    };

    enum
    {
        BUILDING_ILS_U = 34,
        BUILDING_ILS_V = 0,
        BUILDING_ILS_W = 24,
        BUILDING_ILS_H = 34,

        BUILDING_GATE_U = 0,
        BUILDING_GATE_V = 70,
        BUILDING_GATE_W = 28,
        BUILDING_GATE_H = 25,

        BUILDING_HANGAR_U = 0,
        BUILDING_HANGAR_V = 0,
        BUILDING_HANGAR_W = 34,
        BUILDING_HANGAR_H = 28,

        BUILDING_TERMINAL_U = 0,
        BUILDING_TERMINAL_V = 34,
        BUILDING_TERMINAL_W = 51,
        BUILDING_TERMINAL_H = 36,

        BUILDING_TERMINAL_2_U = 51,
        BUILDING_TERMINAL_2_V = BUILDING_TERMINAL_V,
        BUILDING_TERMINAL_2_W = BUILDING_TERMINAL_W,
        BUILDING_TERMINAL_2_H = BUILDING_TERMINAL_H,

        BUILDING_ATC_TOWER_U = 58,
        BUILDING_ATC_TOWER_V = 0,
        BUILDING_ATC_TOWER_W = 29,
        BUILDING_ATC_TOWER_H = 34,

        BUILDING_ATC_LOC_U = 87,
        BUILDING_ATC_LOC_V = 0,
        BUILDING_ATC_LOC_W = 10,
        BUILDING_ATC_LOC_H = 34
    };

    enum
    {
        BUILDING_ILS_ORIGIN_X = 10,
        BUILDING_ILS_ORIGIN_Y = 22,

        BUILDING_GATE_ORIGIN_X = 20,
        BUILDING_GATE_ORIGIN_Y = 8,

        BUILDING_TERMINAL_ORIGIN_X = 20,
        BUILDING_TERMINAL_ORIGIN_Y = 11,

        BUILDING_TERMINAL_2_ORIGIN_X = BUILDING_TERMINAL_ORIGIN_X,
        BUILDING_TERMINAL_2_ORIGIN_Y = BUILDING_TERMINAL_ORIGIN_Y,

        BUILDING_HANGAR_ORIGIN_X = 16,
        BUILDING_HANGAR_ORIGIN_Y = 12,

        BUILDING_ATC_TOWER_ORIGIN_X = 12,
        BUILDING_ATC_TOWER_ORIGIN_Y = 20,

        BUILDING_ATC_LOC_ORIGIN_X = 6,
        BUILDING_ATC_LOC_ORIGIN_Y = 32
    };

    static const struct
    {
        TYPE_ISOMETRIC_POS IsoPos;  // Offset inside tile
        short orig_x;               // Coordinate X origin inside building sprite
        short orig_y;               // Coordinate Y origin inside building sprite
        short w;                    // Building width
        short h;                    // Building height
        short u;                    // Building X offset inside texture page
        short v;                    // Building Y offset inside texture page
    } GameBuildingData[MAX_BUILDING_ID] =
    {
        [BUILDING_GATE] =
        {
            .IsoPos.x = BUILDING_GATE_OFFSET_X,
            .IsoPos.y = BUILDING_GATE_OFFSET_Y,
            .orig_x = BUILDING_GATE_ORIGIN_X,
            .orig_y = BUILDING_GATE_ORIGIN_Y,
            .u = BUILDING_GATE_U,
            .v = BUILDING_GATE_V,
            .w = BUILDING_GATE_W,
            .h = BUILDING_GATE_H,
            // z coordinate set to 0 by default.
        },

        [BUILDING_ATC_LOC] =
        {
            .IsoPos.x = BUILDING_ATC_LOC_OFFSET_X,
            .IsoPos.y = BUILDING_ATC_LOC_OFFSET_Y,
            .orig_x = BUILDING_ATC_LOC_ORIGIN_X,
            .orig_y = BUILDING_ATC_LOC_ORIGIN_Y,
            .u = BUILDING_ATC_LOC_U,
            .v = BUILDING_ATC_LOC_V,
            .w = BUILDING_ATC_LOC_W,
            .h = BUILDING_ATC_LOC_H,
            // z coordinate set to 0 by default.
        },

        [BUILDING_ILS] =
        {
            .IsoPos.x = BUILDING_ILS_OFFSET_X,
            .IsoPos.y = BUILDING_ILS_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_ILS_ORIGIN_X,
            .orig_y = BUILDING_ILS_ORIGIN_Y,
            .u = BUILDING_ILS_U,
            .v = BUILDING_ILS_V,
            .w = BUILDING_ILS_W,
            .h = BUILDING_ILS_H,
        },

        [BUILDING_HANGAR] =
        {
            // BUILDING_HANGAR coordinates inside tile.
            .IsoPos.x = BUILDING_HANGAR_OFFSET_X,
            .IsoPos.y = BUILDING_HANGAR_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_HANGAR_ORIGIN_X,
            .orig_y = BUILDING_HANGAR_ORIGIN_Y,
            .u = BUILDING_HANGAR_U,
            .v = BUILDING_HANGAR_V,
            .w = BUILDING_HANGAR_W,
            .h = BUILDING_HANGAR_H,
        },

        [BUILDING_TERMINAL] =
        {
            // BUILDING_TERMINAL coordinates inside tile.
            .IsoPos.x = BUILDING_TERMINAL_OFFSET_X,
            .IsoPos.y = BUILDING_TERMINAL_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_TERMINAL_ORIGIN_X,
            .orig_y = BUILDING_TERMINAL_ORIGIN_Y,
            .u = BUILDING_TERMINAL_U,
            .v = BUILDING_TERMINAL_V,
            .w = BUILDING_TERMINAL_W,
            .h = BUILDING_TERMINAL_H,
        },

        [BUILDING_TERMINAL_2] =
        {
            // BUILDING_TERMINAL_2 coordinates inside tile.
            .IsoPos.x = BUILDING_TERMINAL_2_OFFSET_X,
            .IsoPos.y = BUILDING_TERMINAL_2_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_TERMINAL_2_ORIGIN_X,
            .orig_y = BUILDING_TERMINAL_2_ORIGIN_Y,
            .u = BUILDING_TERMINAL_2_U,
            .v = BUILDING_TERMINAL_2_V,
            .w = BUILDING_TERMINAL_2_W,
            .h = BUILDING_TERMINAL_2_H,
        },

        [BUILDING_ATC_TOWER] =
        {
            // BUILDING_ATC_TOWER coordinates inside tile.
            .IsoPos.x = BUILDING_ATC_TOWER_OFFSET_X,
            .IsoPos.y = BUILDING_ATC_TOWER_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_ATC_TOWER_ORIGIN_X,
            .orig_y = BUILDING_ATC_TOWER_ORIGIN_Y,
            .u = BUILDING_ATC_TOWER_U,
            .v = BUILDING_ATC_TOWER_V,
            .w = BUILDING_ATC_TOWER_W,
            .h = BUILDING_ATC_TOWER_H,
        },

        [BUILDING_GATE] =
        {
            // BUILDING_GATE coordinates inside tile.
            .IsoPos.x = BUILDING_GATE_OFFSET_X,
            .IsoPos.y = BUILDING_GATE_OFFSET_Y,
            // z coordinate set to 0 by default.
            .orig_x = BUILDING_GATE_ORIGIN_X,
            .orig_y = BUILDING_GATE_ORIGIN_Y,
            .u = BUILDING_GATE_U,
            .v = BUILDING_GATE_V,
            .w = BUILDING_GATE_W,
            .h = BUILDING_GATE_H,
        },
    };

    uint16_t tileNr;
    uint8_t rows = 0;
    uint8_t columns = 0;

    for (tileNr = 0; tileNr < GameLevelSize; tileNr++)
    {
        // Building data is stored in levelBuffer MSB. LSB is dedicated to tile data.
        uint8_t CurrentBuilding = (uint8_t)(levelBuffer[tileNr] >> 8);
        uint8_t j;
        uint8_t k;
        uint8_t AircraftRenderOrder[GAME_MAX_AIRCRAFT_PER_TILE];
        short Aircraft_Y_Data[GAME_MAX_AIRCRAFT_PER_TILE];

        memset(AircraftRenderOrder, FLIGHT_DATA_INVALID_IDX, sizeof (AircraftRenderOrder) );

        for (j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
        {
            // Fill with 0x7FFF (maximum 16-bit positive value).
            Aircraft_Y_Data[j] = 0x7FFF;
        }

        for (j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
        {
            uint8_t AircraftIdx = GameAircraftTilemap[tileNr][j];

            TYPE_ISOMETRIC_POS aircraftIsoPos = AircraftGetIsoPos(AircraftIdx);

            if (AircraftIdx == FLIGHT_DATA_INVALID_IDX)
            {
                // No more aircraft on this tile.
                break;
            }

            //DEBUG_PRINT_VAR(aircraftIsoPos.y);

            for (k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                if (aircraftIsoPos.y < Aircraft_Y_Data[k])
                {
                    uint8_t idx;

                    for (idx = k; idx < (GAME_MAX_AIRCRAFT_PER_TILE - 1); idx++)
                    {
                        // Move previous Y values to the right.
                        Aircraft_Y_Data[idx + 1] = Aircraft_Y_Data[idx];
                        AircraftRenderOrder[idx + 1] = AircraftRenderOrder[idx];
                    }

                    Aircraft_Y_Data[k] = aircraftIsoPos.y;
                    AircraftRenderOrder[k] = AircraftIdx;

                    break;
                }
            }
        }

        if (CurrentBuilding == BUILDING_NONE)
        {
            for (k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                AircraftRender(ptrPlayer, AircraftRenderOrder[k]);
            }
        }
        else
        {
            // Determine rendering order depending on Y value.
            short x_bldg_offset = GameBuildingData[CurrentBuilding].IsoPos.x;
            short y_bldg_offset = GameBuildingData[CurrentBuilding].IsoPos.y;
            short z_bldg_offset = GameBuildingData[CurrentBuilding].IsoPos.z;
            short orig_u = GameBuildingSpr.u;
            short orig_v = GameBuildingSpr.v;

            TYPE_ISOMETRIC_POS buildingIsoPos =
            {
                .x = (columns << (TILE_SIZE_BIT_SHIFT)) + x_bldg_offset,
                .y = (rows << (TILE_SIZE_BIT_SHIFT)) + y_bldg_offset,
                .z = z_bldg_offset
            };

            // Isometric -> Cartesian conversion
            TYPE_CARTESIAN_POS buildingCartPos = GfxIsometricToCartesian(&buildingIsoPos);
            bool buildingDrawn = false;

            // Define new coordinates for building.
            GameBuildingSpr.x = buildingCartPos.x - GameBuildingData[CurrentBuilding].orig_x;
            GameBuildingSpr.y = buildingCartPos.y - GameBuildingData[CurrentBuilding].orig_y;

            GameBuildingSpr.u = orig_u + GameBuildingData[CurrentBuilding].u;
            GameBuildingSpr.v = orig_v + GameBuildingData[CurrentBuilding].v;
            GameBuildingSpr.w = GameBuildingData[CurrentBuilding].w;
            GameBuildingSpr.h = GameBuildingData[CurrentBuilding].h;

            CameraApplyCoordinatesToSprite(ptrPlayer, &GameBuildingSpr);

            for (k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                if (AircraftRenderOrder[k] == FLIGHT_DATA_INVALID_IDX)
                {
                    if (buildingDrawn == false)
                    {
                        GfxSortSprite(&GameBuildingSpr);

                        GameBuildingSpr.u = orig_u;
                        GameBuildingSpr.v = orig_v;

                        buildingDrawn = true;
                    }

                    break;
                }

                if (Aircraft_Y_Data[k] < buildingIsoPos.y)
                {
                    AircraftRender(ptrPlayer, AircraftRenderOrder[k]);
                }
                else
                {
                    if (buildingDrawn == false)
                    {
                        GfxSortSprite(&GameBuildingSpr);

                        GameBuildingSpr.u = orig_u;
                        GameBuildingSpr.v = orig_v;

                        buildingDrawn = true;
                    }

                    AircraftRender(ptrPlayer, AircraftRenderOrder[k]);
                }
            }
        }

        if (columns < (GameLevelColumns - 1) )
        {
            columns++;
        }
        else
        {
            rows++;
            columns = 0;
        }
    }
}

/* *******************************************************************
 *
 * @name: void GameLoadLevel(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Loads and parses *.LVL data.
 *
 *
 * @remarks:
 *  Filepath for *.LVL is given by GameLevelList[0]. Do NOT ever move
 *  it from there to avoid problems!
 *
 * *******************************************************************/

static void GameLoadLevel(const char* path)
{
    uint8_t i = 0;
    uint8_t* ptrBuffer;
    char LevelHeader[LEVEL_MAGIC_NUMBER_SIZE + 1];

    /* TODO - Very important */
    // Map contents (that means, without header) should be copied to levelBuffer
    // Header treatment (magic number, map size, map title...) should be done
    // using System's file buffer.

    if (SystemLoadFile((char*)path) == false)
    {
        return;
    }

    ptrBuffer = SystemGetBufferAddress();

    //SystemLoadFileToBuffer(GameLevelList[0],levelBuffer,GAME_MAX_MAP_SIZE);

    memset(LevelHeader,0, LEVEL_MAGIC_NUMBER_SIZE + 1);

    memmove(LevelHeader,ptrBuffer,LEVEL_MAGIC_NUMBER_SIZE);

    LevelHeader[LEVEL_MAGIC_NUMBER_SIZE] = '\0';

    Serial_printf("Level header: %s\n",LevelHeader);

    if (strncmp(LevelHeader,LEVEL_MAGIC_NUMBER_STRING,LEVEL_MAGIC_NUMBER_SIZE) != 0)
    {
        Serial_printf("Invalid level header! Read \"%s\" instead of " LEVEL_MAGIC_NUMBER_STRING "\n", LevelHeader);
        return;
    }

    i += LEVEL_MAGIC_NUMBER_SIZE;

    GameLevelColumns = ptrBuffer[i++];

    Serial_printf("Level size: %d\n",GameLevelColumns);

    if (    (GameLevelColumns < MIN_MAP_COLUMNS)
                ||
            (GameLevelColumns > MAX_MAP_COLUMNS)    )
    {
        Serial_printf("Invalid map size! Value: %d\n",GameLevelColumns);
        return;
    }

    GameLevelSize = GameLevelColumns * GameLevelColumns;

    memset(GameLevelTitle,0,LEVEL_TITLE_SIZE);

    memmove(GameLevelTitle,&ptrBuffer[i],LEVEL_TITLE_SIZE);

    Serial_printf("Game level title: %s\n",GameLevelTitle);

    i += LEVEL_TITLE_SIZE;

    memset(levelBuffer, 0, GAME_MAX_MAP_SIZE);

    i = LEVEL_HEADER_SIZE;

    {
        size_t j;
        size_t k;

        for (j = LEVEL_HEADER_SIZE, k = 0; k < GameLevelSize; j += sizeof (uint16_t), k++)
        {
            levelBuffer[k] = ptrBuffer[j + 1];
            levelBuffer[k] |= (ptrBuffer[j] << 8);
        }
    }
}

/* ******************************************************************************************
 *
 * @name: void GameAircraftState(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint8_t i:
 *      Index for FlightData table.
 *
 * @brief:
 *  It determines what state should be applied to aircraft when spawn timer expires.
 *
 * @remarks:
 *  This is where TYPE_FLIGHT_DATA is transferred to TYPE_AIRCRAFT on departure.
 *
 * ******************************************************************************************/

static void GameAircraftState(const uint8_t i)
{
    if (FlightData.Finished[i] == false)
    {
        if ((FlightData.Hours[i] == 0)
                    &&
            (FlightData.Minutes[i] == 0)
                    &&
            (FlightData.State[i] == STATE_IDLE)
                    &&
            (FlightData.RemainingTime[i] != 0))
        {
            if (spawnMinTimeFlag == false)
            {
                if ((FlightData.FlightDirection[i] == DEPARTURE)
                                    &&
                     FlightData.Parking[i])
                {
                    uint8_t j;
                    bool bParkingBusy = false;

                    for (j = 0; j < FlightData.nAircraft; j++)
                    {
                        TYPE_AIRCRAFT_DATA* ptrAircraft = AircraftFromFlightDataIndex(j);

                        if (ptrAircraft != NULL)
                        {
                            if (ptrAircraft->State != STATE_IDLE)
                            {
                                const uint16_t* const targets = AircraftGetTargets(j);

                                if (targets != NULL)
                                {
                                    const uint16_t tile = AircraftGetTileFromFlightDataIndex(j);

                                    if (tile == FlightData.Parking[i])
                                    {
                                        bParkingBusy = true;
                                    }
                                    else if (SystemContains_u16(FlightData.Parking[i], targets, AIRCRAFT_MAX_TARGETS))
                                    {
                                        bParkingBusy = true;
                                    }
                                }
                            }
                        }
                    }

                    if (bParkingBusy == false)
                    {
                        uint16_t target[2] = {0};
                        // Arrays are copied to AircraftAddNew, so we create a first and only
                        // target which is the parking tile itself, and the second element
                        // is just the NULL character.
                        // Not an ideal solution, but the best one currently available.

                        FlightData.State[i] = STATE_PARKED;

                        aircraftCreated = true;

                        // Create notification request for incoming aircraft
                        GameGuiBubbleShow();

                        target[0] = FlightData.Parking[i];

                        if (AircraftAddNew(&FlightData, i, target, GameGetParkingDirection(levelBuffer[target[0]])) == false)
                        {
                            Serial_printf("Exceeded maximum aircraft number!\n");
                            return;
                        }
                    }
                }
                else if (FlightData.FlightDirection[i] == ARRIVAL)
                {
                    const uint32_t idx = SystemRand(SOUND_M1_INDEX, MAX_RADIO_CHATTER_SOUNDS - 1);

                    FlightData.State[i] = STATE_APPROACH;
                    aircraftCreated = true;

                    // Play chatter sound.
                    SfxPlaySound(&ApproachSnds[idx]);

                    // Create notification request for incoming aircraft
                    GameGuiBubbleShow();
                }
            }
        }
        else if ( (FlightData.State[i] != STATE_IDLE)
                            &&
                  (FlightData.RemainingTime[i] == 0))
        {
            // Player(s) lost a flight!
            GameRemoveFlight(i, false);
        }
    }
}

static void GameInitTileUVTable(void)
{
    uint16_t i;

    memset(GameLevelBuffer_UVData, 0, sizeof (GameLevelBuffer_UVData));

    for (i = 0 ; i < GameLevelSize; i++)
    {
        uint8_t CurrentTile = (uint8_t)(levelBuffer[i] & 0x007F);   // Remove building data
                                                                    // and mirror flag.

        if (CurrentTile >= FIRST_TILE_TILESET2)
        {
            CurrentTile -= FIRST_TILE_TILESET2;
        }

        GameLevelBuffer_UVData[i].u = (short)(CurrentTile % COLUMNS_PER_TILESET) << TILE_SIZE_BIT_SHIFT;
        GameLevelBuffer_UVData[i].v = (short)(CurrentTile / COLUMNS_PER_TILESET) * TILE_SIZE_H;
    }
}

/* ******************************************************************************************
 *
 * @name: void GameRenderTerrainPrecalculations(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Reads current player states, precalculates RGB/XY/visibilty data and saves it into
 *  lookup tables which will be then used on GameRenderTerrain().
 *
 * @remarks:
 *  Tiles are usually rendered with normal RGB values unless parking/runway is busy
 *  or ptrPlayer->InvalidPath.
 *
 * ******************************************************************************************/
static void GameRenderTerrainPrecalculations(TYPE_PLAYER* const ptrPlayer, const TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint16_t i;
    uint8_t rows = 0;
    uint8_t columns = 0;
    unsigned char rwy_sine = SystemGetSineValue();
    bool used_rwy = SystemContains_u16(ptrPlayer->RwyArray[0], GameUsedRwy, GAME_MAX_RUNWAYS);

    for (i = 0 ; i < GameLevelSize; i++)
    {
        TYPE_ISOMETRIC_POS tileIsoPos;

        // levelBuffer bits explanation:
        // X X X X  X X X X     X X X X     X X X X
        // | | | |  | | | |     | | | |     | | | |
        // | | | |  | | | |     | | | |     | | | V
        // | | | |  | | | |     | | | |     | | V Tile, bit 0
        // | | | |  | | | |     | | | |     | V Tile, bit 1
        // | | | |  | | | |     | | | |     V Tile, bit 2
        // | | | |  | | | |     | | | V     Tile, bit 3
        // | | | |  | | | |     | | V Tile, bit 4
        // | | | |  | | | |     | V Tile, bit 5
        // | | | |  | | | |     V Tile, bit 6
        // | | | |  | | | |     Tile mirror flag
        // | | | |  | | | V
        // | | | |  | | V Building, bit 0
        // | | | |  | V Building, bit 1
        // | | | |  V Building, bit 2
        // | | | V  Building, bit 3
        // | | V Building, bit 4
        // | V Building, bit 5
        // V Building, bit 6
        // Building, bit 7
        uint8_t CurrentTile = (uint8_t)(levelBuffer[i] & 0x007F);   // Remove building data
                                                                    // and mirror flag.

        // Isometric -> Cartesian conversion
        tileIsoPos.x = columns << (TILE_SIZE_BIT_SHIFT);
        tileIsoPos.y = rows << (TILE_SIZE_BIT_SHIFT);
        tileIsoPos.z = 0;

        TYPE_TILE_DATA* const tileData = &ptrPlayer->TileData[i];

        tileData->CartPos = GfxIsometricToCartesian(&tileIsoPos);

        if (columns < (GameLevelColumns - 1) )
        {
            columns++;
        }
        else
        {
            rows++;
            columns = 0;
        }

        // Set coordinate origin to left upper corner.
        tileData->CartPos.x -= TILE_SIZE >> 1;

        CameraApplyCoordinatesToCartesianPos(ptrPlayer, &tileData->CartPos);

        if (GfxIsInsideScreenArea(  tileData->CartPos.x,
                                    tileData->CartPos.y,
                                    TILE_SIZE,
                                    TILE_SIZE_H ))
        {
            tileData->ShowTile = true;

            tileData->r = NORMAL_LUMINANCE;
            tileData->g = NORMAL_LUMINANCE;
            tileData->b = NORMAL_LUMINANCE;

            if (i != 0)
            {
                if (ptrPlayer->SelectRunway)
                {
                    if (SystemContains_u16(i, ptrPlayer->RwyArray, GAME_MAX_RWY_LENGTH))
                    {
                        if (used_rwy)
                        {
                            tileData->r = rwy_sine;
                            tileData->b = NORMAL_LUMINANCE >> 2;
                            tileData->g = NORMAL_LUMINANCE >> 2;
                        }
                        else
                        {
                            tileData->r = NORMAL_LUMINANCE >> 2;
                            tileData->g = NORMAL_LUMINANCE >> 2;
                            tileData->b = rwy_sine;
                        }
                    }
                }
                else if (   (ptrPlayer->SelectTaxiwayParking)
                                                ||
                            (ptrPlayer->SelectTaxiwayRunway)   )
                {
                    if ((   (SystemContains_u16(i, ptrPlayer->Waypoints, ptrPlayer->WaypointIdx))
                                        ||
                            (i == ptrPlayer->SelectedTile)  )
                                        &&
                            (ptrPlayer->SelectedTile != GAME_INVALID_TILE_SELECTION)    )
                    {
                        if (ptrPlayer->InvalidPath)
                        {
                            tileData->r = rwy_sine;
                            tileData->b = NORMAL_LUMINANCE >> 2;
                            tileData->g = NORMAL_LUMINANCE >> 2;
                        }
                        else
                        {
                            tileData->r = NORMAL_LUMINANCE >> 2;
                            tileData->g = NORMAL_LUMINANCE >> 2;
                            tileData->b = rwy_sine;
                        }
                    }
                    else if (   (ptrPlayer->SelectTaxiwayRunway)
                                            &&
                                (   (CurrentTile == TILE_RWY_HOLDING_POINT)
                                            ||
                                    (CurrentTile == TILE_RWY_HOLDING_POINT_2)   )   )
                    {
                        tileData->r = NORMAL_LUMINANCE >> 2;
                        tileData->g = rwy_sine;
                        tileData->b = NORMAL_LUMINANCE >> 2;
                    }
                    else if (   (ptrPlayer->SelectTaxiwayParking)
                                            &&
                                (   (CurrentTile == TILE_PARKING)
                                            ||
                                    (CurrentTile == TILE_PARKING_2) )   )
                    {
                        bool parkingBusy = false;

                        uint8_t aircraftIndex;

                        for (aircraftIndex = 0; aircraftIndex < GAME_MAX_AIRCRAFT; aircraftIndex++)
                        {
                            const TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(aircraftIndex);

                            if (ptrAircraft->State == STATE_PARKED)
                            {
                                const uint16_t tile = AircraftGetTileFromFlightDataIndex(aircraftIndex);

                                if (i == tile)
                                {
                                    parkingBusy = true;
                                    break;
                                }
                            }
                        }

                        if (parkingBusy)
                        {
                            tileData->r = rwy_sine;
                            tileData->g = NORMAL_LUMINANCE >> 2;
                            tileData->b = NORMAL_LUMINANCE >> 2;
                        }
                        else
                        {
                            tileData->r = NORMAL_LUMINANCE >> 2;
                            tileData->g = rwy_sine;
                            tileData->b = NORMAL_LUMINANCE >> 2;
                        }
                    }
                }
                else if (ptrPlayer->ShowAircraftData)
                {
                    const uint8_t aircraftIndex = ptrPlayer->FlightDataSelectedAircraft;

                    switch (ptrFlightData->State[aircraftIndex])
                    {
                        case STATE_TAXIING:
                            // Fall through.
                        case STATE_USER_STOPPED:
                            // Fall through.
                        case STATE_AUTO_STOPPED:
                            // Fall through.
                        {
                            const uint16_t* const targets = AircraftGetTargets(aircraftIndex);

                            if (targets != NULL)
                            {
                                if (SystemContains_u16(i, targets, AIRCRAFT_MAX_TARGETS))
                                {
                                    if (SystemIndexOf_U16(i, targets, AIRCRAFT_MAX_TARGETS) >=
                                        AircraftGetTargetIdx(aircraftIndex))
                                    {
                                        tileData->r = NORMAL_LUMINANCE >> 2;
                                        tileData->g = NORMAL_LUMINANCE >> 2;
                                        tileData->b = rwy_sine;
                                    }
                                }
                            }
                        }
                        break;

                        default:
                        break;
                    }
                }
            }
        }
    }
}

/* ******************************************************************************************
 *
 * @name: void GameRenderTerrain(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *
 * @brief:
 *  Draws all tiles depending on levelBuffer configuration.
 *
 * @remarks:
 *  Tiles are usually rendered with normal RGB values unless parking/runway is busy
 *  or ptrPlayer->InvalidPath.
 *
 * ******************************************************************************************/
void GameRenderTerrain(TYPE_PLAYER* const ptrPlayer)
{
    uint16_t i;

    for (i = 0 ; i < GameLevelSize; i++)
    {
        if (ptrPlayer->TileData[i].ShowTile)
        {
            bool flip_id;
            GsSprite* ptrTileset;
            uint8_t aux_id;
            uint8_t CurrentTile = (uint8_t)(levelBuffer[i] & 0x00FF);

            // Flipped tiles have bit 7 set.
            if (CurrentTile & TILE_MIRROR_FLAG)
            {
                flip_id = true;
                aux_id = CurrentTile;
                CurrentTile &= ~(TILE_MIRROR_FLAG);
            }
            else
            {
                flip_id = false;
            }

            if (CurrentTile <= LAST_TILE_TILESET1)
            {
                // Draw using GameTilesetSpr
                ptrTileset = &GameTilesetSpr;
            }
            else if (   (CurrentTile > LAST_TILE_TILESET1)
                            &&
                        (CurrentTile <= LAST_TILE_TILESET2) )
            {
                // Draw using GameTileset2Spr
                ptrTileset = &GameTileset2Spr;
            }
            else
            {
                ptrTileset = NULL;
                continue;
            }

            // Apply {X, Y} data from precalculated lookup tables.
            ptrTileset->x = ptrPlayer->TileData[i].CartPos.x;
            ptrTileset->y = ptrPlayer->TileData[i].CartPos.y;

            // Apply RGB data from precalculated lookup tables.
            ptrTileset->r = ptrPlayer->TileData[i].r;
            ptrTileset->g = ptrPlayer->TileData[i].g;
            ptrTileset->b = ptrPlayer->TileData[i].b;

            if (flip_id)
            {
                ptrTileset->attribute |= H_FLIP;
            }

            ptrTileset->w = TILE_SIZE;
            ptrTileset->h = TILE_SIZE_H;

            ptrTileset->u = GameLevelBuffer_UVData[i].u;
            ptrTileset->v = GameLevelBuffer_UVData[i].v;

            ptrTileset->mx = ptrTileset->u + (TILE_SIZE >> 1);
            ptrTileset->my = ptrTileset->v + (TILE_SIZE_H >> 1);

            if (flip_id)
            {
                flip_id = false;
                CurrentTile = aux_id;
            }

            GfxSortSprite(ptrTileset);

            if (ptrTileset->attribute & H_FLIP)
            {
                ptrTileset->attribute &= ~(H_FLIP);
            }
        }
    }
}

/* *******************************************************************
 *
 * @name: void GameSetTime(uint8_t hour, uint8_t minutes)
 *
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Reportedly, it sets game time to specified hour and minutes.
 *
 *
 * @remarks:
 *  To be used on GameInit() after PLT file parsing.
 *
 * *******************************************************************/
void GameSetTime(uint8_t hour, uint8_t minutes)
{
    GameHour = hour;
    GameMinutes = minutes;
}

/* *******************************************************************
 *
 * @name: void GameActiveAircraft(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint8_t i:
 *      Index from FlightData array.
 *
 * @brief:
 *  On each game cycle, FlightData.ActiveAircraft is set to 0 and
 *  number of active aircraft is recalculated.
 *
 * @remarks:
 *  Called ciclically from GameCalculations(). This function is
 *  executed GAME_MAX_AIRCRAFT times on each cycle.
 *
 * *******************************************************************/
static void GameActiveAircraft(const uint8_t i)
{
    // Reset iterator when i == 0.

    if (i == 0)
    {
        FlightData.ActiveAircraft = 0;
    }

    if (FlightData.State[i] != STATE_IDLE)
    {
        FlightData.ActiveAircraft++;
    }
}

/* ******************************************************************************************
 *
 * @name: void GameStateShowAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handles ptrPlayer->ShowAircraftData state.
 *
 *
 * @remarks:
 *  Called ciclically from GamePlayerHandler().
 *
 * ******************************************************************************************/

static void GameStateShowAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    if (ptrPlayer->ShowAircraftData)
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE))
        {
            ptrPlayer->ShowAircraftData = false;
        }
    }

    if (ptrPlayer->PadKeySinglePress_Callback(PAD_CIRCLE))
    {
        if (GameGuiShowAircraftDataSpecialConditions(ptrPlayer) == false)
        {
            //Invert ptrPlayer->ShowAircraftData value
            ptrPlayer->ShowAircraftData = ptrPlayer->ShowAircraftData ? false : true;
        }
    }
}

/* ******************************************************************************************
 *
 * @name: void GameStateLockTarget(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handles ptrPlayer->LockTarget state.
 *
 *
 * @remarks:
 *  Called ciclically from GamePlayerHandler().
 *
 ******************************************************************************************/

static void GameStateLockTarget(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;

    if (ptrPlayer->LockTarget)
    {
        if ((ptrPlayer->LockedAircraft != FLIGHT_DATA_INVALID_IDX)
                            &&
            (ptrPlayer->LockedAircraft <= ptrPlayer->FlightDataSelectedAircraft))
        {
            CameraMoveToIsoPos(ptrPlayer, AircraftGetIsoPos(ptrPlayer->LockedAircraft) );
        }
    }

    if (ptrPlayer->ShowAircraftData)
    {
        if ( (ptrFlightData->State[AircraftIdx] != STATE_IDLE)
                                        &&
            (ptrFlightData->State[AircraftIdx] != STATE_APPROACH) )
        {
            ptrPlayer->LockTarget = true;
            ptrPlayer->LockedAircraft = AircraftIdx;
        }
    }
    else
    {
        ptrPlayer->LockTarget = false;
        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
    }
}

/* ******************************************************************************************
 *
 * @name: void GameStateSelectTaxiwayRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handler for ptrPlayer->SelectTaxiwayRunway.
 *
 *
 * @remarks:
 *  Called ciclically from GamePlayerHandler().
 *
 * ******************************************************************************************/

static void GameStateSelectTaxiwayRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t i;
    uint16_t target_tile;

    /*Serial_printf("Camera is pointing to {%d,%d}\n",IsoPos.x, IsoPos.y);*/

    if (ptrPlayer->SelectTaxiwayRunway)
    {
        // Under this mode, always reset locking target.
        ptrPlayer->LockTarget = false;
        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;

        if (GamePathToTile(ptrPlayer, ptrFlightData) == false)
        {
            ptrPlayer->InvalidPath = true;
        }

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE))
        {
            // State exit.
            ptrPlayer->SelectTaxiwayRunway = false;
            // Clear waypoints array.
            memset(ptrPlayer->Waypoints, 0, sizeof (uint16_t) * PLAYER_MAX_WAYPOINTS);
            ptrPlayer->WaypointIdx = 0;
            ptrPlayer->LastWaypointIdx = 0;
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS))
        {
            if (ptrPlayer->InvalidPath == false)
            {
                for (i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
                {
                    if (ptrPlayer->Waypoints[i] == 0)
                    {
                        break;
                    }

                    ptrPlayer->LastWaypointIdx = i;
                }

                target_tile = levelBuffer[ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx]];

                SfxPlaySound(&BeepSnd);

                switch(target_tile)
                {
                    case TILE_RWY_HOLDING_POINT:
                        // Fall through
                    case TILE_RWY_HOLDING_POINT | TILE_MIRROR_FLAG:
                        // Fall through
                    case TILE_RWY_HOLDING_POINT_2:
                        // Fall through
                    case TILE_RWY_HOLDING_POINT_2 | TILE_MIRROR_FLAG:
                        AircraftFromFlightDataIndexAddTargets(ptrPlayer->FlightDataSelectedAircraft, ptrPlayer->Waypoints);
                        Serial_printf("Added these targets to aircraft %d:\n", ptrPlayer->FlightDataSelectedAircraft);

                        for (i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
                        {
                            Serial_printf("%d ",ptrPlayer->Waypoints[i]);
                        }

                        Serial_printf("\n");

                        // Clear waypoints array.
                        memset(ptrPlayer->Waypoints, 0, sizeof (uint16_t) * PLAYER_MAX_WAYPOINTS);

                        // Reset state and auxiliar variables
                        ptrPlayer->WaypointIdx = 0;
                        ptrPlayer->LastWaypointIdx = 0;
                        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
                        ptrPlayer->LockTarget = false;
                        ptrPlayer->SelectTaxiwayRunway = false;

                        ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] = STATE_TAXIING;
                        GameScore += SCORE_REWARD_TAXIING;
                    break;

                    default:
                    break;
                }
            }
        }
    }
}

/* **************************************************************************************************
 *
 * @name: void GameStateSelectTaxiwayParking(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handler for ptrPlayer->SelectTaxiwayParking.
 *
 * @remarks:
 *
 * **************************************************************************************************/
static void GameStateSelectTaxiwayParking(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t i;
    uint16_t target_tile;

    if (ptrPlayer->SelectTaxiwayParking)
    {
        // Under this mode, always reset locking target.
        ptrPlayer->LockTarget = false;
        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;

        if (GamePathToTile(ptrPlayer, ptrFlightData) == false)
        {
            ptrPlayer->InvalidPath = true;
        }

        for (i = 0; i < GAME_MAX_AIRCRAFT; i++)
        {
            if (ptrPlayer->InvalidPath == false)
            {
                const TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(i);

                if (ptrAircraft != NULL)
                {
                    if (ptrAircraft->State == STATE_PARKED)
                    {
                        const uint16_t tile = AircraftGetTileFromFlightDataIndex(i);

                        if (ptrPlayer->SelectedTile == tile)
                        {
                            ptrPlayer->InvalidPath = true;
                            break;
                        }
                    }
                }
            }
        }

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE))
        {
            // State exit.
            ptrPlayer->SelectTaxiwayParking = false;
            // Clear waypoints array.
            memset(ptrPlayer->Waypoints, 0, sizeof (uint16_t) * PLAYER_MAX_WAYPOINTS);
            ptrPlayer->WaypointIdx = 0;
            ptrPlayer->LastWaypointIdx = 0;
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS))
        {
            if (ptrPlayer->InvalidPath == false)
            {
                for (i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
                {
                    if (ptrPlayer->Waypoints[i] == 0)
                    {
                        break;
                    }

                    ptrPlayer->LastWaypointIdx = i;
                }

                target_tile = levelBuffer[ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx]] & ~(TILE_MIRROR_FLAG);

                SfxPlaySound(&BeepSnd);

                if (    (target_tile == TILE_PARKING)
                                ||
                        (target_tile == TILE_PARKING_2) )
                {
                    // TODO: Assign path to aircraft
                    AircraftFromFlightDataIndexAddTargets(ptrPlayer->FlightDataSelectedAircraft, ptrPlayer->Waypoints);

                    Serial_printf("Added these targets to aircraft %d:\n", ptrPlayer->FlightDataSelectedAircraft);

                    for (i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
                    {
                        Serial_printf("%d ",ptrPlayer->Waypoints[i]);
                    }

                    Serial_printf("\n");

                    ptrPlayer->SelectTaxiwayParking = false;
                    // Clear waypoints array.
                    memset(ptrPlayer->Waypoints, 0, sizeof (uint16_t) * PLAYER_MAX_WAYPOINTS);
                    ptrPlayer->WaypointIdx = 0;
                    ptrPlayer->LastWaypointIdx = 0;

                    ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] = STATE_TAXIING;
                    GameScore += SCORE_REWARD_TAXIING;
                }
                else
                {
                    Serial_printf("Tile %d cannot be used as end point.\n", target_tile);
                }
            }
        }
    }
}

/* **************************************************************************************************
 *
 * @name: void GameStateSelectRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handler for ptrPlayer->SelectRunway.
 *
 * @remarks:
 *
 * **************************************************************************************************/
static void GameStateSelectRunway(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t i;
    TYPE_ISOMETRIC_POS IsoPos = {   GameGetXFromTile_short(GameRwy[ptrPlayer->SelectedRunway]),
                                    GameGetYFromTile_short(GameRwy[ptrPlayer->SelectedRunway]),
                                    0   };

    if (ptrPlayer->SelectRunway)
    {
        // Under this mode, always reset locking target.
        ptrPlayer->LockTarget = false;
        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;

        GameGetSelectedRunwayArray(GameRwy[ptrPlayer->SelectedRunway], ptrPlayer->RwyArray, sizeof (ptrPlayer->RwyArray));

        CameraMoveToIsoPos(ptrPlayer, IsoPos);

        if (ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE))
        {
            ptrPlayer->SelectRunway = false;
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS))
        {
            bool success = false;

            if (SystemContains_u16(GameRwy[ptrPlayer->SelectedRunway], GameUsedRwy, GAME_MAX_RUNWAYS) == false)
            {
                ptrPlayer->SelectRunway = false;
                Serial_printf("Player selected runway %d!\n",GameRwy[ptrPlayer->SelectedRunway]);

                for (i = 0; i < GAME_MAX_RUNWAYS; i++)
                {
                    if (GameUsedRwy[i] == 0)
                    {
                        GameAssignRunwaytoAircraft(ptrPlayer, ptrFlightData);
                        success = true;
                        GameUsedRwy[i] = GameRwy[ptrPlayer->SelectedRunway];
                        break;
                    }
                }

                if (success == false)
                {
                    Serial_printf("No available runways!\n");
                }
            }
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_LEFT))
        {
            if (ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_APPROACH)
            {
                if (ptrPlayer->SelectedRunway != 0)
                {
                    ptrPlayer->SelectedRunway--;
                }
            }
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_RIGHT))
        {
            if (ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_APPROACH)
            {
                if (ptrPlayer->SelectedRunway < (GAME_MAX_RUNWAYS - 1))
                {
                    if (GameRwy[ptrPlayer->SelectedRunway + 1] != 0)
                    {
                        ptrPlayer->SelectedRunway++;
                    }
                }
            }
        }
    }
}

/* **************************************************************************************************
 *
 * @name: void GameGetRunwayArray(void)
 *
 * @author: Xavier Del Campo
 *
 *
 * @brief:
 *  On startup, an array of runway headers is created from levelBuffer once *.LVL is parsed.
 *
 * @remarks:
 *  Do not confuse GameRwy with GameRwyArray, which are used for completely different purposes.
 *
 * **************************************************************************************************/
void GameGetRunwayArray(void)
{
    uint16_t i;
    uint8_t j = 0;

    for (i = 0; i < GameLevelSize; i++)
    {
        uint8_t tileNr = levelBuffer[i] & ~TILE_MIRROR_FLAG;

        if (tileNr == TILE_RWY_START_1)
        {
            if (SystemContains_u16(i, levelBuffer, GAME_MAX_RUNWAYS) == false)
            {
                GameRwy[j++] = i;
            }
        }
    }

    Serial_printf("GameRwy = ");

    for (i = 0; i < GAME_MAX_RUNWAYS; i++)
    {
        if (GameRwy[i] == 0)
        {
            break;
        }

        Serial_printf("%d ", GameRwy[i]);
    }

    Serial_printf("\n");
}

/* **************************************************************************************************
 *
 * @name: void GameSelectAircraftFromList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Actions for ptrPlayer->ShowAircraftData.
 *
 * @remarks:
 *
 * **************************************************************************************************/

static void GameSelectAircraftFromList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
    FL_STATE aircraftState = ptrFlightData->State[AircraftIdx];

    if (ptrPlayer->ShowAircraftData)
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS))
        {
            if (ptrPlayer->ActiveAircraft != 0)
            {
                ptrPlayer->ShowAircraftData = false;

                switch(aircraftState)
                {
                    case STATE_APPROACH:
                        ptrPlayer->SelectRunway = true;
                    break;

                    case STATE_PARKED:
                        ptrPlayer->SelectTaxiwayRunway = true;
                        // Move camera to selected aircraft and add first waypoint.
                        GameSelectAircraftWaypoint(ptrPlayer);
                    break;

                    case STATE_LANDED:
                        ptrPlayer->SelectTaxiwayParking = true;
                        // Move camera to selected aircraft and add first waypoint.
                        GameSelectAircraftWaypoint(ptrPlayer);
                    break;

                    case STATE_UNBOARDING:
                        ptrPlayer->Unboarding = true;
                        // Move camera to selected aircraft.
                        GameSelectAircraft(ptrPlayer);
                        // Generate first unboarding key sequence
                        GameGenerateUnboardingSequence(ptrPlayer);
                    break;

                    case STATE_READY_FOR_TAKEOFF:
                        ptrFlightData->State[AircraftIdx] = STATE_TAKEOFF;
                        GameCreateTakeoffWaypoints(ptrPlayer, ptrFlightData, AircraftIdx);
                        SfxPlaySound(&TakeoffSnd);
                    break;

                    case STATE_HOLDING_RWY:
                    {
                        TYPE_RWY_ENTRY_DATA rwyEntryData = {0};
                        uint8_t i;

                        ptrPlayer->SelectRunway = true;
                        GameGetRunwayEntryTile(AircraftIdx, &rwyEntryData);

                        for (i = 0; GameRwy[i] != 0 && (i < (sizeof (GameRwy) / sizeof (GameRwy[0]))); i++)
                        {
                            if (GameRwy[i] == rwyEntryData.rwyHeader)
                            {
                                break;
                            }
                        }

                        ptrPlayer->SelectedRunway = i;
                    }
                    break;

                    default:
                        Serial_printf("Incompatible state %d!\n",aircraftState);
                        // States remain unchanged
                        ptrPlayer->SelectRunway = false;
                        ptrPlayer->SelectTaxiwayRunway = false;
                        ptrPlayer->ShowAircraftData = true;
                        ptrPlayer->Unboarding = false;
                    break;
                }
            }
        }
        else if (ptrPlayer->PadKeySinglePress_Callback(PAD_L1))
        {
            FL_STATE* const ptrAircraftState = &FlightData.State[ptrPlayer->FlightDataSelectedAircraft];

            switch (*ptrAircraftState)
            {
                case STATE_TAXIING:
                    *ptrAircraftState = STATE_USER_STOPPED;
                break;

                case STATE_USER_STOPPED:
                    // Fall through.
                case STATE_AUTO_STOPPED:
                    *ptrAircraftState = STATE_TAXIING;
                break;

                default:
                break;
            }
        }
    }
}

/* **************************************************************************************************
 *
 * @name: DIRECTION GameGetParkingDirection(uint16_t parkingTile)
 *
 * @author: Xavier Del Campo
 *
 * @return:
 *  Depending on tile number, parking direction is returned.
 *
 * **************************************************************************************************/
DIRECTION GameGetParkingDirection(uint16_t parkingTile)
{
    switch (parkingTile)
    {
        case TILE_PARKING:
        return DIR_WEST;

        case TILE_PARKING | TILE_MIRROR_FLAG:
        return DIR_NORTH;

        case TILE_PARKING_2:
        return DIR_EAST;

        case TILE_PARKING_2 | TILE_MIRROR_FLAG:
        return DIR_SOUTH;

        default:
        break;
    }

    return NO_DIRECTION;
}

/* **************************************************************************************************
 *
 * @name: DIRECTION GameGetRunwayDirection(uint16_t rwyHeader)
 *
 * @author: Xavier Del Campo
 *
 * @return:
 *  Depending on tile number, runway direction is returned.
 *
 * **************************************************************************************************/
DIRECTION GameGetRunwayDirection(uint16_t rwyHeader)
{
    switch(levelBuffer[rwyHeader])
    {
        case TILE_RWY_START_1:
            return DIR_EAST;

        case TILE_RWY_START_2:
            return DIR_WEST;

        case TILE_RWY_START_1 | TILE_MIRROR_FLAG:
            return DIR_SOUTH;

        case TILE_RWY_START_2 | TILE_MIRROR_FLAG:
            return DIR_NORTH;

        default:
            Serial_printf("Unknown direction for tile %d\n",rwyHeader);
            break;
    }

    return NO_DIRECTION;
}

/* **************************************************************************************************
 *
 * @name: void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t rwyHeader:
 *      Level tile number (located inside levelBuffer) pointing to runway header.
 *      Only TILE_RWY_START_1 and TILE_RWY_START_2 (with or without TILE_MIRROR_FLAG)
 *      can be used for runway headers!
 *
 *  uint16_t* rwyArray:
 *      Pointer to an array which will be filled with all the tiles belonging to a runway
 *      with header pointed to by rwyHeader.
 *
 *  size_t sz:
 *      Maximum size of the array.
 *
 * @brief:
 *  Fills rwyArray with all the tile numbers (included in levelBuffer) belonging to a
 *  runway with header pointed to by rwyHeader.
 *
 * @remarks:
 *
 * **************************************************************************************************/
void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz)
{
    static uint16_t last_tile = 0;
    static uint8_t i = 0;
    static DIRECTION dir;

    if (sz != (GAME_MAX_RWY_LENGTH * sizeof (uint16_t) ))
    {
        Serial_printf(  "GameGetSelectedRunwayArray: size %d is different"
                        " than expected (%d bytes). Returning...\n",
                        sz,
                        (GAME_MAX_RWY_LENGTH * sizeof (uint16_t) ) );
        return;
    }

    if (rwyHeader != 0)
    {
        // This function is called recursively.
        // Since 0 is not a valid value (it's not allowed to place
        // a runway header on first tile), it is used to determine
        // when to start creating the array.

        // Part one: determine runway direction and call the function again with rwyHeader == 0.

        memset(rwyArray, 0, sz);
        last_tile = rwyHeader;
        i = 0;

        dir = GameGetRunwayDirection(rwyHeader);

        if (dir == NO_DIRECTION)
        {
            Serial_printf("rwyHeader = %d returned NO_DIRECTION\n", rwyHeader);
            return;
        }
    }
    else
    {
        // Part two: append tiles to array until runway end is found.

        if (    (levelBuffer[last_tile] == TILE_RWY_START_1)
                            ||
                (levelBuffer[last_tile] == TILE_RWY_START_2)
                            ||
                (levelBuffer[last_tile] == (TILE_RWY_START_1 | TILE_MIRROR_FLAG) )
                            ||
                (levelBuffer[last_tile] == (TILE_RWY_START_2 | TILE_MIRROR_FLAG) )  )
        {
            // Runway end found
            rwyArray[i++] = last_tile;
            return;
        }
    }

    rwyArray[i++] = last_tile;

    switch(dir)
    {
        case DIR_EAST:
            last_tile++;
        break;

        case DIR_WEST:
            last_tile--;
        break;

        case DIR_NORTH:
            last_tile -= GameLevelColumns;
        break;

        case DIR_SOUTH:
            last_tile += GameLevelColumns;
        break;

        case NO_DIRECTION:
            // Fall through
        default:
            Serial_printf("Invalid runway direction.\n");
        return;
    }

    GameGetSelectedRunwayArray(0, rwyArray, sz);
}

/* **************************************************************************************************
 *
 * @name: void GameAssignRunwaytoAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Assigns a runway to an incoming aircraft (FlightDirection == ARRIVAL) depending on
 *  player selection.
 *
 * @remarks:
 *
 * **************************************************************************************************/

void GameAssignRunwaytoAircraft(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint16_t assignedRwy = GameRwy[ptrPlayer->SelectedRunway];
    uint8_t aircraftIndex = ptrPlayer->FlightDataSelectedAircraft;
    uint16_t rwyExit = 0;
    uint8_t i;
    uint16_t targets[AIRCRAFT_MAX_TARGETS] = {0};
    uint8_t rwyTiles[GAME_MAX_RWY_LENGTH] = {0};

    // Remember that ptrPlayer->SelectedAircraft contains an index to
    // be used with ptrFlightData.

    if (ptrFlightData->State[aircraftIndex] == STATE_APPROACH)
    {
        uint8_t j;
        bool firstEntryPointFound = false;
        uint16_t rwyArray[GAME_MAX_RWY_LENGTH];

        // TODO: Algorithm is not correct. If TILE_RWY_EXIT is placed further,
        // but returns a match earlier than other rwyExitTiles[], invalid targets
        // are returned to aircraft. We should check this before proceeding.
        static const uint8_t rwyExitTiles[] =
        {
            TILE_RWY_EXIT,
            TILE_RWY_EXIT | TILE_MIRROR_FLAG,
            TILE_RWY_EXIT_2,
            TILE_RWY_EXIT_2 | TILE_MIRROR_FLAG
        };

        ptrFlightData->State[aircraftIndex] = STATE_FINAL;
        GameScore += SCORE_REWARD_FINAL;

        GameGetSelectedRunwayArray(assignedRwy, rwyArray, sizeof (rwyArray));

        for (i = 0; i < GAME_MAX_RWY_LENGTH; i++)
        {
            rwyTiles[i] = levelBuffer[rwyArray[i]];
        }

        for (i = 0; (i < GAME_MAX_RWY_LENGTH) && (rwyExit == 0); i++)
        {
            for (j = 0; j < ARRAY_SIZE(rwyExitTiles); j++)
            {
                if (rwyTiles[i] == rwyExitTiles[j])
                {
                    if (firstEntryPointFound == false)
                    {
                        firstEntryPointFound = true;
                    }
                    else
                    {
                        rwyExit = rwyArray[i];
                    }

                    break;
                }
            }
        }

        if (rwyExit == 0)
        {
            Serial_printf("ERROR: Could not find TILE_RWY_EXIT or TILE_RWY_EXIT_2 for runway header %d.\n", assignedRwy);
            return;
        }

        // Create two new targets for the recently created aircraft.
        targets[0] = assignedRwy;
        targets[1] = rwyExit;

        if (AircraftAddNew( ptrFlightData,
                            aircraftIndex,
                            targets,
                            GameGetRunwayDirection(assignedRwy) ) == false)
        {
            Serial_printf("Exceeded maximum aircraft number!\n");
            return;
        }

        SfxPlaySound(&TowerFinalSnds[SystemRand(SOUND_M1_INDEX, MAX_RADIO_CHATTER_SOUNDS - 1)]);
    }
    else if (ptrFlightData->State[aircraftIndex] == STATE_HOLDING_RWY)
    {
        TYPE_RWY_ENTRY_DATA rwyEntryData = {0};

        GameGetRunwayEntryTile(aircraftIndex, &rwyEntryData);

        targets[0] = rwyEntryData.rwyEntryTile;
        targets[1] = targets[0] + rwyEntryData.rwyStep;

        AircraftAddTargets(AircraftFromFlightDataIndex(aircraftIndex), targets);

        ptrFlightData->State[aircraftIndex] = STATE_ENTERING_RWY;
    }
}

/* *******************************************************************
 *
 * @name: short GameGetXFromTile_short(uint16_t tile)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @return:
 *  Returns relative X position (no fixed-point arithmetic) given
 *  a tile number from levelBuffer.
 *
 * @remarks:
 *
 * *******************************************************************/

short GameGetXFromTile_short(uint16_t tile)
{
    short retVal;

    tile %= GameLevelColumns;

    retVal = (tile << TILE_SIZE_BIT_SHIFT);

    // Always point to tile center
    retVal += TILE_SIZE >> 1;

    return retVal;
}

/* *******************************************************************
 *
 * @name: short GameGetYFromTile_short(uint16_t tile)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @return:
 *  Returns relative Y position (no fixed-point arithmetic) given
 *  a tile number from levelBuffer.
 *
 * @remarks:
 *
 * *******************************************************************/

short GameGetYFromTile_short(uint16_t tile)
{
    short retVal;

    tile /= GameLevelColumns;

    retVal = (tile << TILE_SIZE_BIT_SHIFT);

    // Always point to tile center
    retVal += TILE_SIZE >> 1;

    return retVal;
}

/* *******************************************************************
 *
 * @name: fix16_t GameGetXFromTile(uint16_t tile)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @return:
 *  Returns relative X position in 16.16 (fix16_t) fixed-point format
 *  given a tile number from levelBuffer.
 *
 * @remarks:
 *
 * *******************************************************************/

fix16_t GameGetXFromTile(uint16_t tile)
{
    return fix16_from_int(GameGetXFromTile_short(tile));
}

/* *******************************************************************
 *
 * @name: fix16_t GameGetYFromTile(uint16_t tile)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @return:
 *  Returns relative Y position in 16.16 (fix16_t) fixed-point format
 *  given a tile number from levelBuffer.
 *
 * @remarks:
 *
 * *******************************************************************/

fix16_t GameGetYFromTile(uint16_t tile)
{
    return fix16_from_int(GameGetYFromTile_short(tile));
}

/* ****************************************************************************
 *
 * @name: FL_STATE GameTargetsReached(uint16_t firstTarget, uint8_t index)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t firstTarget:
 *      First waypoint of TYPE_AIRCRAFT instance.
 *
 *  uint8_t index:
 *      Index of FlightData.
 *
 * @brief:
 *  Calculates new state for aircraft once all waypoints have been reached.
 *
 * @return:
 *  New state for aircraft once waypoints have been reached.
 *
 * @remarks:
 *
 * ****************************************************************************/

FL_STATE GameTargetsReached(uint16_t firstTarget, uint8_t index)
{
    FL_STATE retState = STATE_IDLE;
    uint8_t i;

    switch(FlightData.State[index])
    {
        case STATE_FINAL:
            FlightData.State[index] = STATE_LANDED;

            for (i = 0; i < GAME_MAX_RUNWAYS; i++)
            {
                if (GameUsedRwy[i] == firstTarget)
                {
                    GameUsedRwy[i] = 0;
                }
            }
        break;

        case STATE_TAXIING:
            if (FlightData.FlightDirection[index] == DEPARTURE)
            {
                FlightData.State[index] = STATE_HOLDING_RWY;
            }
            else if (FlightData.FlightDirection[index] == ARRIVAL)
            {
                FlightData.State[index] = STATE_UNBOARDING;
            }
        break;

        case STATE_TAKEOFF:
            FlightData.State[index] = STATE_CLIMBING;
        break;

        case STATE_ENTERING_RWY:
            FlightData.State[index] = STATE_READY_FOR_TAKEOFF;
        break;

        default:
        break;
    }

    return retState;
}

/* ****************************************************************************
 *
 * @name: uint16_t GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS* IsoPos)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_ISOMETRIC_POS* IsoPos:
 *      (x, y, z) position data.
 *
 * @brief:
 *  Calculates new state for aircraft once all waypoints have been reached.
 *
 * @return:
 *  Tile number to be used against levelBuffer.
 *
 * @remarks:
 *  GameLevelColumns is used to determine tile number.
 *
 * ****************************************************************************/

uint16_t GameGetTileFromIsoPosition(const TYPE_ISOMETRIC_POS* const IsoPos)
{
    uint16_t tile;

    if (IsoPos == NULL)
    {
        return 0;
    }

    if (    (IsoPos->x < 0) || (IsoPos->y < 0) )
    {
        return GAME_INVALID_TILE_SELECTION; // Invalid XYZ position
    }

    tile = IsoPos->x >> TILE_SIZE_BIT_SHIFT;
    tile += (IsoPos->y >> TILE_SIZE_BIT_SHIFT) * GameLevelColumns;

    /*Serial_printf("Returning tile %d from position {%d, %d, %d}\n",
            tile,
            IsoPos->x,
            IsoPos->y,
            IsoPos->z   );*/

    return tile;
}

/* ****************************************************************************
 *
 * @name: uint8_t GameGetLevelColumns(void)
 *
 * @author: Xavier Del Campo
 *
 * @return:
 *  GameLevelColumns. Used for other modules without declaring GameLevelColumns
 *  as a global variable.
 *
 * ****************************************************************************/

uint8_t GameGetLevelColumns(void)
{
    return GameLevelColumns;
}

/* ****************************************************************************
 *
 * @name: void GamePlayerAddWaypoint(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Wrapper for GamePlayerAddWaypoint_Ex().
 *
 * ****************************************************************************/

void GamePlayerAddWaypoint(TYPE_PLAYER* const ptrPlayer)
{
    GamePlayerAddWaypoint_Ex(ptrPlayer, ptrPlayer->SelectedTile);
}

/* ****************************************************************************
 *
 * @name: void GamePlayerAddWaypoint(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure.
 *
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @brief:
 *  It allows adding a tile number to ptrPlayer.
 *
 * @remark:
 *  To be used together with GamePathToTile().
 *
 * ****************************************************************************/

void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* const ptrPlayer, uint16_t tile)
{
    // "_Ex" function allow selecting a certain tile, whereas the other one
    // is a particulare case of "_Ex" for tile = ptrPlayer->SelectedTIle.

    if (ptrPlayer->WaypointIdx >= PLAYER_MAX_WAYPOINTS)
    {
        Serial_printf("No available waypoints for this player!\n");
        return;
    }

    /*Serial_printf("Added tile %d to ptrPlayer->Waypoints[%d]\n",
            tile,
            ptrPlayer->WaypointIdx);*/

    ptrPlayer->Waypoints[ptrPlayer->WaypointIdx++] = tile;
}

/* **************************************************************************************
 *
 * @name: bool GameWaypointCheckExisting(TYPE_PLAYER* const ptrPlayer, uint16_t temp_tile)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure.
 *
 *  uint16_t tile:
 *      Tile number from levelBuffer.
 *
 * @brief:
 *  Checks if tile number temp_tile is already included on player's waypoint list.
 *
 * @return:
 *  True if waypoint is already included on waypoint list, false otherwise.
 *
 * **************************************************************************************/

bool GameWaypointCheckExisting(TYPE_PLAYER* const ptrPlayer, uint16_t temp_tile)
{
    if (SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
    {
        /*for (i = 0; i < FlightData.nAircraft; i++)
        {
            if ( (ptrFlightData->State[i] != STATE_IDLE)
                            &&
                (AircraftMoving(i) == false)            )
            {
                if (temp_tile == AircraftGetTileFromFlightDataIndex(i))
                {
                    return false;   // Check pending!
                }
            }
        }*/

        GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);

        return false;
    }

    // temp_tile is already included on ptrPlayer->Waypoints!
    return true;
}

/* ****************************************************************************************
 *
 * @name: bool GamePathToTile(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Given an input TYPE_PLAYER structure and a selected tile,
 *  it updates current Waypoints array with all tiles between two points.
 *  If one of these tiles do not belong to desired tiles (i.e.: grass,
 *  water, buildings...), then false is returned.
 *
 * @return:
 *  Returns false on invalid path or invalid tile number selected. True otherwise.
 *
 * ****************************************************************************************/

bool GamePathToTile(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    static const uint8_t AcceptedTiles[] =
    {
        TILE_ASPHALT_WITH_BORDERS,
        TILE_PARKING,
        TILE_RWY_MID,
        TILE_RWY_EXIT,
        TILE_RWY_EXIT_2,
        TILE_TAXIWAY_CORNER_GRASS,
        TILE_TAXIWAY_CORNER_GRASS_2,
        TILE_TAXIWAY_GRASS,
        TILE_TAXIWAY_INTERSECT_GRASS,
        TILE_TAXIWAY_4WAY_CROSSING,
        TILE_PARKING_2,
        TILE_RWY_HOLDING_POINT,
        TILE_RWY_HOLDING_POINT_2,
        TILE_TAXIWAY_CORNER_GRASS_3
    };

    uint8_t i;

    uint16_t x_diff;
    uint16_t y_diff;
    uint16_t temp_tile;

    const TYPE_ISOMETRIC_POS IsoPos = CameraGetIsoPos(ptrPlayer);

    ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);

    if (ptrPlayer->SelectedTile == GAME_INVALID_TILE_SELECTION)
    {
        return false;
    }

    for (i = (ptrPlayer->LastWaypointIdx + 1); i < PLAYER_MAX_WAYPOINTS; i++)
    {
        ptrPlayer->Waypoints[i] = 0;
    }

    ptrPlayer->WaypointIdx = ptrPlayer->LastWaypointIdx + 1;

    x_diff = abs((ptrPlayer->SelectedTile % GameLevelColumns) -
                 (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns));

    y_diff = abs((ptrPlayer->SelectedTile / GameLevelColumns) -
                 (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns));

    // At this point, we have to update current waypoints list.
    // ptrPlayer->Waypoints[ptrPlayer->WaypointIdx - 1] points to the last inserted point,
    // so now we have to determine how many points need to be created.

    temp_tile = ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx];

    if (x_diff >= y_diff)
    {
        while ( (x_diff--) > 0)
        {
            if ( (ptrPlayer->SelectedTile % GameLevelColumns) >
                (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns)   )
            {
                temp_tile++;
            }
            else
            {
                temp_tile--;
            }

            if (GameWaypointCheckExisting(ptrPlayer, temp_tile))
            {
                return false;   // Tile is already included in the list of temporary tiles?
            }
        }

        while ( (y_diff--) > 0)
        {
            if ( (ptrPlayer->SelectedTile / GameLevelColumns) >
                (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns)   )
            {
                temp_tile += GameLevelColumns;
            }
            else
            {
                temp_tile -= GameLevelColumns;
            }

            if (GameWaypointCheckExisting(ptrPlayer, temp_tile))
            {
                return false;   // Tile is already included in the list of temporary tiles?
            }
        }
    }
    else
    {
        while ( (y_diff--) > 0)
        {
            if ( (ptrPlayer->SelectedTile / GameLevelColumns) >
                (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns)   )
            {
                temp_tile += GameLevelColumns;
            }
            else
            {
                temp_tile -= GameLevelColumns;
            }

            if (GameWaypointCheckExisting(ptrPlayer, temp_tile))
            {
                return false;   // Tile is already included in the list of temporary tiles?
            }
        }

        while ( (x_diff--) > 0)
        {
            if ( (ptrPlayer->SelectedTile % GameLevelColumns) >
                (ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns)   )
            {
                temp_tile++;
            }
            else
            {
                temp_tile--;
            }

            if (GameWaypointCheckExisting(ptrPlayer, temp_tile))
            {
                return false;   // Tile is already included in the list of temporary tiles?
            }
        }
    }

    // Now at this point, we have prepared our array.

    for (i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
    {
        if (ptrPlayer->Waypoints[i] == 0)
        {
            // We have found empty waypoints. Exit loop
            break;
        }

        if (SystemContains_u8(  levelBuffer[ptrPlayer->Waypoints[i]],
                                AcceptedTiles,
                                sizeof (AcceptedTiles) ) == false)
        {
            // Now try again with mirrored tiles, just in case!
            static const uint8_t AcceptedMirroredTiles[] =
            {
                TILE_ASPHALT_WITH_BORDERS | TILE_MIRROR_FLAG,
                TILE_PARKING | TILE_MIRROR_FLAG,
                TILE_RWY_MID | TILE_MIRROR_FLAG,
                TILE_RWY_EXIT | TILE_MIRROR_FLAG,
                TILE_RWY_EXIT_2 | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_CORNER_GRASS | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_CORNER_GRASS_2 | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_GRASS | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_INTERSECT_GRASS | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_4WAY_CROSSING | TILE_MIRROR_FLAG,
                TILE_PARKING_2 | TILE_MIRROR_FLAG,
                TILE_RWY_HOLDING_POINT | TILE_MIRROR_FLAG,
                TILE_RWY_HOLDING_POINT_2 | TILE_MIRROR_FLAG,
                TILE_TAXIWAY_CORNER_GRASS_3 | TILE_MIRROR_FLAG
            };

            if (SystemContains_u8(  levelBuffer[ptrPlayer->Waypoints[i]],
                                    AcceptedMirroredTiles,
                                    sizeof (AcceptedTiles) ) == false)
            {
                // Both cases have failed. Return from function.
                return false;
            }
        }
    }

    return true;
}

/* ****************************************************************************************
 *
 * @name: TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Moves player camera position to selected aircraft.
 *
 * @return:
 *  Isometric position of selected aircraft.
 *
 * ****************************************************************************************/

static TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* const ptrPlayer)
{
    const uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
    const TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(AircraftIdx);

    CameraMoveToIsoPos(ptrPlayer, IsoPos);

    return IsoPos;
}

/* ********************************************************************************
 *
 * @name: void GameSelectAircraftWaypoint(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Moves player camera to selected aircraft and adds first waypoint.
 *
 * ********************************************************************************/

void GameSelectAircraftWaypoint(TYPE_PLAYER* const ptrPlayer)
{
    TYPE_ISOMETRIC_POS IsoPos = GameSelectAircraft(ptrPlayer);

    ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);

    GamePlayerAddWaypoint(ptrPlayer);
}

/* ********************************************************************************
 *
 * @name: bool GameTwoPlayersActive(void)
 *
 * @author: Xavier Del Campo
 *
 * @return:
 *  Returns if a second player is active. To be used with other modules without
 *  declaring twoPlayers as a global variable.
 *
 * ********************************************************************************/

bool GameTwoPlayersActive(void)
{
    return twoPlayers;
}

/* *****************************************************************
 *
 * @name: void GameDrawMouse(TYPE_PLAYER* const ptrPlayer)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Draws GameMouseSpr under determined player states.
 *
 * *****************************************************************/

static void GameDrawMouse(TYPE_PLAYER* const ptrPlayer)
{
    if ((ptrPlayer->SelectTaxiwayParking)
                        ||
        (ptrPlayer->SelectTaxiwayRunway)   )
    {
        GfxSortSprite(&GameMouseSpr);
    }
}

/* ********************************************************************************
 *
 * @name: FL_STATE GameGetFlightDataStateFromIdx(uint8_t FlightDataIdx)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint8_t FlightDataIdx:
 *      Index from FlightData.
 *
 * @return:
 *  Returns .State variable given offset from FlightData.
 *
 * ********************************************************************************/

FL_STATE GameGetFlightDataStateFromIdx(uint8_t FlightDataIdx)
{
    if (FlightDataIdx >= FlightData.nAircraft)
    {
        return STATE_IDLE; // Error: could cause buffer overrun
    }

    return FlightData.State[FlightDataIdx];
}

/* ********************************************************************************
 *
 * @name: uint32_t GameGetScore(void)
 *
 * @author: Xavier Del Campo
 *
 * @return:
 *  Returns game score to other modules.
 *
 * ********************************************************************************/

uint32_t GameGetScore(void)
{
    return GameScore;
}

/* *******************************************************************************************
 *
 * @name: void GameStateUnboarding(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Handler for StateUnboarding.
 *
 * *******************************************************************************************/

static void GameStateUnboarding(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    if (ptrPlayer->Unboarding)
    {
        if (ptrPlayer->PadKeySinglePress_Callback(PAD_CIRCLE))
        {
            ptrPlayer->Unboarding = false;
            ptrPlayer->UnboardingSequenceIdx = 0;   // Player will need to repeat sequence
                                                    // if he/she decides to leave without finishing
        }

        ptrPlayer->LockTarget = true;
        ptrPlayer->LockedAircraft = ptrPlayer->FlightDataSelectedAircraft;

        if (ptrPlayer->PadLastKeySinglePressed_Callback() == ptrPlayer->UnboardingSequence[ptrPlayer->UnboardingSequenceIdx])
        {
            if (++ptrPlayer->UnboardingSequenceIdx >= UNBOARDING_KEY_SEQUENCE_MEDIUM)
            {
                if (ptrFlightData->Passengers[ptrPlayer->FlightDataSelectedAircraft] > UNBOARDING_PASSENGERS_PER_SEQUENCE)
                {
                    // Player has entered correct sequence. Unboard UNBOARDING_PASSENGERS_PER_SEQUENCE passengers.

                    ptrFlightData->Passengers[ptrPlayer->FlightDataSelectedAircraft] -= UNBOARDING_PASSENGERS_PER_SEQUENCE;
                    GameScore += SCORE_REWARD_UNLOADING;

                    ptrPlayer->PassengersLeftSelectedAircraft = ptrFlightData->Passengers[ptrPlayer->FlightDataSelectedAircraft];

                    GameGenerateUnboardingSequence(ptrPlayer);
                }
                else
                {
                    // Flight has finished. Remove aircraft and set finished flag
                    ptrPlayer->Unboarding = false;
                    GameRemoveFlight(ptrPlayer->FlightDataSelectedAircraft, true);
                }

                ptrPlayer->UnboardingSequenceIdx = 0;
            }

            Serial_printf("ptrPlayer->UnboardingSequenceIdx = %d\n", ptrPlayer->UnboardingSequenceIdx);

            SfxPlaySound(&BeepSnd);
        }
        else if (ptrPlayer->PadLastKeySinglePressed_Callback() != 0)
        {
            ptrPlayer->UnboardingSequenceIdx = 0; // Player has committed a mistake while entering the sequence. Repeat it!
        }
    }
}

/* *******************************************************************************************
 *
 * @name: void GameStateUnboarding(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Modifies ptrPlayer->UnboardingSequence creating a random sequence of numbers
 *  using SystemRand().
 *
 * @todo:
 *  Maximum number of sequence keys should be adjusted according to difficulty.
 *
 * *******************************************************************************************/

static void GameGenerateUnboardingSequence(TYPE_PLAYER* const ptrPlayer)
{
    uint8_t i;
    unsigned short keyTable[] = { PAD_CROSS, PAD_SQUARE, PAD_TRIANGLE };

    memset(ptrPlayer->UnboardingSequence, 0, sizeof (ptrPlayer->UnboardingSequence) );

    ptrPlayer->UnboardingSequenceIdx = 0;

    Serial_printf("Key sequence generated: ");

    // Only medium level implemented. TODO: Implement other levels
    for (i = 0; i < UNBOARDING_KEY_SEQUENCE_MEDIUM; i++)
    {
        uint8_t randIdx = SystemRand(0, (sizeof (keyTable) / sizeof (keyTable[0])) - 1);

        ptrPlayer->UnboardingSequence[i] = keyTable[randIdx];

        Serial_printf("idx = %d, 0x%04X ", randIdx, ptrPlayer->UnboardingSequence[i]);
    }

    Serial_printf("\n");
}

/* *********************************************************************************************************************
 *
 * @name: void GameCreateTakeoffWaypoints(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData, uint8_t aircraftIdx)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 *  uint8_t aircraftIdx:
 *      Index from FlightData.
 *
 * @brief:
 *  Given input aircraft from FlightData, it automatically looks for selected runway and creates an array
 *  of waypoints to be then executed by corresponding TYPE_AIRCRAFT_DATA instance.
 *
 * *********************************************************************************************************************/

static void GameCreateTakeoffWaypoints(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData, uint8_t aircraftIdx)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(aircraftIdx);

    if (ptrAircraft != NULL)
    {
        // Look for aircraft direction by searching TILE_RWY_EXIT
        //uint16_t currentTile = AircraftGetTileFromFlightDataIndex(aircraftIdx);
        //uint8_t targetsIdx = 0;
        DIRECTION aircraftDir = AircraftGetDirection(ptrAircraft);
        int8_t rwyStep = 0;
        uint16_t currentTile = 0;
        uint16_t targets[AIRCRAFT_MAX_TARGETS] = {0};
        uint8_t i;

        switch(aircraftDir)
        {
            case DIR_EAST:
                rwyStep = 1;
            break;

            case DIR_WEST:
                rwyStep = -1;
            break;

            case DIR_NORTH:
                rwyStep = -GameLevelColumns;
            break;

            case DIR_SOUTH:
                rwyStep = GameLevelColumns;
            break;

            default:
            return;
        }

        DEBUG_PRINT_VAR(AircraftGetTileFromFlightDataIndex(aircraftIdx));

        for (currentTile = (AircraftGetTileFromFlightDataIndex(aircraftIdx) + rwyStep);
            ((levelBuffer[currentTile] & ~(TILE_MIRROR_FLAG)) != TILE_RWY_START_1)
                                &&
            ((levelBuffer[currentTile] & ~(TILE_MIRROR_FLAG)) != TILE_RWY_START_2);
            currentTile -= rwyStep  )
        {
            // Calculate new currentTile value until conditions are invalid.
        }

        for (i = 0; i < GAME_MAX_RUNWAYS; i++)
        {
            if (GameUsedRwy[i] == currentTile)
            {
                GameUsedRwy[i] = 0;
                break;
            }
        }

        for (   currentTile = (AircraftGetTileFromFlightDataIndex(aircraftIdx) + rwyStep);
                ((levelBuffer[currentTile] & ~(TILE_MIRROR_FLAG)) != TILE_RWY_EXIT)
                            &&
                ((levelBuffer[currentTile] & ~(TILE_MIRROR_FLAG)) != TILE_RWY_EXIT_2);
                currentTile += rwyStep  )
        {

        }

        targets[0] = currentTile;
        AircraftAddTargets(AircraftFromFlightDataIndex(aircraftIdx), targets);
    }
}

/* *******************************************************************************************
 *
 * @name: void GameGetRunwayEntryTile(uint8_t aircraftIdx, TYPE_RWY_ENTRY_DATA* ptrRwyEntry)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t aircraftIdx:
 *      Index from FlightData. Used to determine target tile.
 *
 *  TYPE_RWY_ENTRY_DATA* ptrRwyEntry:
 *      Instance to be filled with runway data.
 *
 * @brief:
 *  Fills a TYPE_RWY_ENTRY_DATA instance with information about runway.
 *
 * *******************************************************************************************/

static void GameGetRunwayEntryTile(uint8_t aircraftIdx, TYPE_RWY_ENTRY_DATA* ptrRwyEntry)
{
    // Look for aircraft direction by searching TILE_RWY_EXIT
    const uint16_t currentTile = AircraftGetTileFromFlightDataIndex(aircraftIdx);
    int16_t step = 0;
    uint16_t i;

    if ( (currentTile >= GameLevelColumns)
                        &&
        ( (currentTile + GameLevelColumns) < GameLevelSize) )
    {
        if (    ((levelBuffer[currentTile + 1] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT)
                                ||
                ((levelBuffer[currentTile + 1] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT_2)   )
        {
            ptrRwyEntry->Direction = DIR_EAST;
            ptrRwyEntry->rwyStep = GameLevelColumns;
            step = 1;
        }
        else if (   ((levelBuffer[currentTile - 1] & ~(TILE_MIRROR_FLAG) ) == TILE_RWY_EXIT)
                                    ||
                    ((levelBuffer[currentTile - 1] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT_2)   )
        {
            ptrRwyEntry->Direction = DIR_WEST;
            ptrRwyEntry->rwyStep = GameLevelColumns;
            step = -1;
        }
        else if (   ((levelBuffer[currentTile + GameLevelColumns] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT)
                                                ||
                    ((levelBuffer[currentTile + GameLevelColumns] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT_2)    )
        {
            ptrRwyEntry->Direction = DIR_SOUTH;
            ptrRwyEntry->rwyStep = 1;
            step = GameLevelColumns;
        }
        else if (   ((levelBuffer[currentTile - GameLevelColumns] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT)
                                                ||
                    ((levelBuffer[currentTile - GameLevelColumns] & ~(TILE_MIRROR_FLAG)) == TILE_RWY_EXIT_2)    )
        {
            ptrRwyEntry->Direction = DIR_NORTH;
            ptrRwyEntry->rwyStep = 1;
            step = -GameLevelColumns;
        }
        else
        {
            ptrRwyEntry->rwyEntryTile = 0;
            ptrRwyEntry->Direction = NO_DIRECTION;
            ptrRwyEntry->rwyStep = 0;
            return;
        }

        ptrRwyEntry->rwyEntryTile = currentTile + step;

        i = ptrRwyEntry->rwyEntryTile;

        while ( ((levelBuffer[i] & ~TILE_MIRROR_FLAG) != TILE_RWY_START_1)
                                &&
                ((levelBuffer[i] & ~TILE_MIRROR_FLAG) != TILE_RWY_START_2)
                                &&
                (i > ptrRwyEntry->rwyStep)
                                &&
                ((i - ptrRwyEntry->rwyStep) < GameLevelSize ) )
        {
            i -= ptrRwyEntry->rwyStep;
        }

        ptrRwyEntry->rwyHeader = i;
    }
    else
    {
        Serial_printf("GameGetRunwayEntryTile(): Invalid index for tile.\n");
    }
}

/* *******************************************************************************************
 *
 * @name: bool GameInsideLevelFromIsoPos(TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos:
 *      (x, y, z) coordinate data in an isometric system.
 *
 * @return:
 *  Returns true if a (x, y, z) coordinate is inside level coordinates. False otherwise.
 *
 * *******************************************************************************************/

bool GameInsideLevelFromIsoPos(TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos)
{
    short x = (short)fix16_to_int(ptrIsoPos->x);
    short y = (short)fix16_to_int(ptrIsoPos->y);

    if (x < 0)
    {
        return false;
    }

    if (x > (GameLevelColumns << TILE_SIZE_BIT_SHIFT))
    {
        return false;
    }

    if (y < 0)
    {
        return false;
    }

    if (y > (GameLevelColumns << TILE_SIZE_BIT_SHIFT) )
    {
        return false;
    }

    return true;
}

/* *******************************************************************************************
 *
 * @name: void GameRemoveFlight(uint8_t idx, bool successful)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t idx:
 *      Index from FlightData.
 *
 *  bool successful:
 *      False if flight was lost on timeout, true otherwise.
 *
 * @brief:
 *  Actions to be performed when a flight ends, both successfully or not (lost flight).
 *
 * @remarks:
 *  GameScore is updated here depending on player actions.
 *
 * *******************************************************************************************/

void GameRemoveFlight(const uint8_t idx, const bool successful)
{
    uint8_t i;

    for (i = PLAYER_ONE; i < MAX_PLAYERS; i++)
    {
        TYPE_PLAYER* const ptrPlayer = &PlayerData[i];
        uint8_t j;

        if (ptrPlayer->Active == false)
        {
            continue;
        }

        if (idx >= FlightData.nAircraft)
        {
            Serial_printf("GameRemoveFlight: index %d exceeds max index %d!\n", idx, FlightData.nAircraft);
            return;
        }

        if ((FlightData.FlightDirection[idx] & ptrPlayer->FlightDirection) == 0)
        {
            continue;
        }

        for (j = 0; j < ptrPlayer->ActiveAircraft; j++)
        {
            if (ptrPlayer->ActiveAircraftList[j] == idx)
            {
                if (FlightData.State[idx] != STATE_IDLE)
                {
                    uint8_t k;

                    for (k = 0; k < GAME_MAX_RUNWAYS; k++)
                    {
                        const uint16_t* const targets = AircraftGetTargets(idx);
                        uint16_t rwyArray[GAME_MAX_RWY_LENGTH] = {0};

                        if (SystemContains_u16(GameUsedRwy[k], targets, AIRCRAFT_MAX_TARGETS))
                        {
                            GameUsedRwy[k] = 0;
                        }
                        else
                        {
                            // GameRwyArray is filled with runway tiles.
                            GameGetSelectedRunwayArray(GameUsedRwy[k], rwyArray, GAME_MAX_RWY_LENGTH * sizeof (uint16_t) );

                            if (SystemContains_u16(  AircraftGetTileFromFlightDataIndex(idx),
                                                    rwyArray,
                                                    sizeof (rwyArray) / sizeof (rwyArray[0]) ))
                            {
                                GameUsedRwy[k] = 0;
                            }
                        }
                    }

                    if (FlightData.State[idx] != STATE_APPROACH)
                    {
                        if (FlightData.State[idx] == STATE_UNBOARDING)
                        {
                            memset(ptrPlayer->UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS);
                            ptrPlayer->UnboardingSequenceIdx = 0;
                            ptrPlayer->Unboarding = false;
                            ptrPlayer->LockTarget = false;
                            ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
                        }

                        if (AircraftRemove(idx) == false)
                        {
                            Serial_printf("Something went wrong when removing aircraft!\n");
                            return;
                        }
                    }
                    else
                    {
                        // STATE_APPROACH is the only state which is not linked to a TYPE_AIRCRAFT_DATA instance.
                    }

                    if (ptrPlayer->LockedAircraft == idx)
                    {
                        ptrPlayer->LockTarget = false;
                        ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
                    }

                    if (successful)
                    {
                        GameScore += SCORE_REWARD_FINISH_FLIGHT;

                        // Add punctuation
                        GameScore += FlightData.RemainingTime[idx] << 1;
                    }
                    else
                    {
                        GameScore = (GameScore < LOST_FLIGHT_PENALTY)? 0 : (GameScore - LOST_FLIGHT_PENALTY);
                    }

                    if (ptrPlayer->SelectedAircraft != 0)
                    {
                        if (ptrPlayer->SelectedAircraft >= j)
                        {
                            ptrPlayer->SelectedAircraft--;
                        }
                    }

                    FlightData.Passengers[idx] = 0;
                    FlightData.State[idx] = STATE_IDLE;
                    FlightData.Finished[idx] = true;

                    spawnMinTimeFlag = true;
                    TimerRestart(GameSpawnMinTime);

                    return;
                }
            }
        }

        // Usually called in PlayerHandler(), but now
        // force active aircraft list update.
        GameActiveAircraftList(ptrPlayer, &FlightData);
    }
}

/* *******************************************************************************************
 *
 * @name: void GameActiveAircraftList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  TYPE_PLAYER* const ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* const ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *
 * @brief:
 *  Rebuilds flight data arrray for a specific player.
 *
 * *******************************************************************************************/

void GameActiveAircraftList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData)
{
    uint8_t i;
    uint8_t j = 0;

    uint8_t currentFlightDataIdx;
    const uint8_t lastFlightDataIdx = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

    // Clear all pointers for aircraft data first.
    // Then, rebuild aircraft list for current player.
    memset(ptrPlayer->ActiveAircraftList, 0, GAME_MAX_AIRCRAFT);
    ptrPlayer->ActiveAircraft = 0;
    ptrPlayer->RemainingAircraft = 0;

    for (i = 0; i < FlightData.nAircraft; i++)
    {
        if ((ptrFlightData->State[i] != STATE_IDLE)
                            &&
            (ptrFlightData->FlightDirection[i] & ptrPlayer->FlightDirection) )
        {
            ptrPlayer->ActiveAircraftList[j++] = i;
            ptrPlayer->ActiveAircraft++;
        }
        else if (ptrFlightData->State[i] == STATE_IDLE)
        {
            ptrPlayer->RemainingAircraft++;
        }
    }

    currentFlightDataIdx = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

    if (aircraftCreated)
    {
        aircraftCreated = false;

        if (ptrPlayer->ActiveAircraft > 1)
        {
            if (currentFlightDataIdx != lastFlightDataIdx)
            {
                for (ptrPlayer->SelectedAircraft = 0; ptrPlayer->SelectedAircraft < FlightData.nAircraft; ptrPlayer->SelectedAircraft++)
                {
                    if (ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft] == lastFlightDataIdx)
                    {
                        break;
                    }
                }
            }
        }
    }

    ptrPlayer->FlightDataSelectedAircraft = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];
}

/* *******************************************************************************************
 *
 * @name: void GameRemainingAircraft(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t i:
 *      Index from FlightData.
 *
 * @brief:
 *  Reportedly, it updates FlightData.nRemainingAircraft depending on game status.
 *
 * @remarks:
 *  This function is called nActiveAircraft times. See loop inside GameCalculations()
 *  for further reference.
 *
 * *******************************************************************************************/

static void GameRemainingAircraft(const uint8_t i)
{
    // Reset iterator when starting from first element.

    if (i == 0)
    {
        FlightData.nRemainingAircraft = FlightData.nAircraft;
    }

    if ((FlightData.State[i] != STATE_IDLE)
            ||
        FlightData.Finished[i])
    {
        FlightData.nRemainingAircraft--;
    }
}

/* *******************************************************************************************
 *
 * @name: void GameFinished(uint8_t i)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t i:
 *      Index from FlightData.
 *
 * @brief:
 *  Sets levelFinished if there are no more active aircraft.
 *
 * @remarks:
 *  This function is called nActiveAircraft times. See loop inside GameCalculations()
 *  for further reference.
 *
 * *******************************************************************************************/

static void GameFinished(const uint8_t i)
{
    if (FlightData.Finished[i] == false)
    {
        levelFinished = false;
    }
}

/* *******************************************************************************************
 *
 * @name: void GameMinimumSpawnTimeout(void)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t i:
 *      Index from FlightData.
 *
 * @brief:
 *  Callback automatically executed on GameSpawnMinTime expired. spawnMinTimeFlag is used
 *  to set a minimum time between flight ended and flight spawn.
 *
 * *******************************************************************************************/

void GameMinimumSpawnTimeout(void)
{
    spawnMinTimeFlag = false;
}

/* *******************************************************************************************
 *
 * @name: void GameAircraftCollision(uint8_t AircraftIdx)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t AircraftIdx:
 *      Index from FlightData.
 *
 * @brief:
 *  Sets GameAircraftCollisionFlag when two or more aircraft collide. This flag is then
 *  checked by Game().
 *
 * *******************************************************************************************/

void GameAircraftCollision(uint8_t AircraftIdx)
{
    GameAircraftCollisionFlag = true;
    GameAircraftCollisionIdx = AircraftIdx;
}

/* *******************************************************************************************
 *
 * @name: void GameAircraftCollision(uint8_t AircraftIdx)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t AircraftIdx:
 *      Index from FlightData.
 *
 * @brief:
 *  Event triggered by Aircraft when aircraft A is getting close to non-moving aircraft B
 *  and a security stop must be done in order to avoid collision.
 *
 * *******************************************************************************************/

void GameStopFlight(uint8_t AircraftIdx)
{
    FL_STATE* ptrState = &FlightData.State[AircraftIdx];

    if (*ptrState == STATE_TAXIING)
    {
        // Only allow auto stop under taxi
        *ptrState = STATE_AUTO_STOPPED;
    }
}

/* *******************************************************************************************
 *
 * @name: void GameAircraftCollision(uint8_t AircraftIdx)
 *
 * @author: Xavier Del Campo
 *
 * @param:
 *
 *  uint8_t AircraftIdx:
 *      Index from FlightData.
 *
 * @brief:
 *  Event triggered by Aircraft when aircraft A is no longer getting close to aircraft B, so
 *  that taxiing can be resumed.
 *
 * *******************************************************************************************/

void GameResumeFlightFromAutoStop(uint8_t AircraftIdx)
{
    FL_STATE* ptrState = &FlightData.State[AircraftIdx];

    if (*ptrState == STATE_AUTO_STOPPED)
    {
        // Only recovery to STATE_TAXIING is allowed.
        *ptrState = STATE_TAXIING;
    }
}

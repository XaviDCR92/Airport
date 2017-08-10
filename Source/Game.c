/* *************************************
 * 	Includes
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

/* *************************************
 * 	Defines
 * *************************************/

#define GAME_MAX_MAP_SIZE 0x400
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
#define TILE_MIRROR_FLAG ( (uint8_t)0x80 )

#define GAME_INVALID_TILE_SELECTION ( (uint16_t)0xFFFF )

#define GAME_MINIMUM_PARKING_SPAWN_TIME (2 * TIMER_PRESCALER_1_SECOND) // 2 seconds

/* **************************************
 * 	Structs and enums					*
 * *************************************/

typedef struct t_rwyentrydata
{
	AIRCRAFT_DIRECTION Direction;
	uint16_t rwyEntryTile;
	int8_t rwyStep;
	uint16_t rwyHeader;
}TYPE_RWY_ENTRY_DATA;

typedef struct t_buildingdata
{
    TYPE_ISOMETRIC_POS IsoPos;  // Offset inside tile
    short orig_x;               // Coordinate X origin inside building sprite
    short orig_y;               // Coordinate Y origin inside building sprite
    short w;                    // Building width
    short h;                    // Building height
    short u;                    // Building X offset inside texture page
    short v;                    // Building Y offset inside texture page
}TYPE_BUILDING_DATA;

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
    BUILDING_NONE = 0,
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
    TILE_GRASS = 0,
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
    
    LAST_TILE_TILESET1 = TILE_TAXIWAY_CORNER_GRASS_2
};

enum
{		
    LAST_TILE_TILESET2 = LAST_TILE_TILESET1
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void GameInit(void);
static void GameLoadLevel(void);
static bool GamePause(void);
static void GameFinished(uint8_t i);
static void GameEmergencyMode(void);
static void GameCalculations(void);
static void GamePlayerHandler(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer);
static void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* ptrPlayer, uint16_t tile);
static void GameGraphics(void);
static void GameRenderLevel(TYPE_PLAYER* ptrPlayer);
//static void GameLoadPilots(char* strPath);
static void GameClock(void);
static void GameClockFlights(uint8_t i);
static void GameAircraftState(uint8_t i);
static void GameActiveAircraft(uint8_t i);
static void GameStateShowAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameSelectAircraftFromList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameStateSelectRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameStateSelectTaxiwayRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameStateSelectTaxiwayParking(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameStateLockTarget(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* ptrPlayer);
static void GameSelectAircraftWaypoint(TYPE_PLAYER* ptrPlayer);
static void GameGetRunwayArray(void);
static void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz);
static void GameAssignRunwaytoAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static bool GamePathToTile(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameDrawMouse(TYPE_PLAYER* ptrPlayer);
static void GameStateUnboarding(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameGenerateUnboardingSequence(TYPE_PLAYER* ptrPlayer);
static void GameCreateTakeoffWaypoints(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData, uint8_t aircraftIdx);
static void GameGetRunwayEntryTile(uint8_t aircraftIdx, TYPE_RWY_ENTRY_DATA* ptrRwyEntry);
static void GameActiveAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameRemainingAircraft(uint8_t i);
static void GameMinimumSpawnTimeout(void);
static void GameRenderBuildingAircraft(TYPE_PLAYER* ptrPlayer);
static void GameBuildingsInit(void);
static void GameGetAircraftTilemap(uint8_t i);
static bool GameWaypointCheckExisting(TYPE_PLAYER* ptrPlayer, uint16_t temp_tile);

/* *************************************
 * 	Global Variables
 * *************************************/

bool GameStartupFlag;
uint32_t GameScore;

/* *************************************
 * 	Local Variables
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
static bool firstLevelRender; // Used to avoid reentrance issues on GameRenderLevel()
static TYPE_TIMER* GameSpawnMinTime;
static bool spawnMinTimeFlag;
static bool GameAircraftCreatedFlag;
static bool GameAircraftCollisionFlag;
static uint8_t GameAircraftCollisionIdx;
static TYPE_BUILDING_DATA GameBuildingData[MAX_BUILDING_ID];
static uint8_t GameAircraftTilemap[GAME_MAX_MAP_SIZE][GAME_MAX_AIRCRAFT_PER_TILE];

// Instances for player-specific data
TYPE_PLAYER PlayerData[MAX_PLAYERS];

static char* GameFileList[] = {	"cdrom:\\DATA\\SPRITES\\TILESET1.TIM;1"	,
                                "cdrom:\\DATA\\SPRITES\\GAMEPLN.TIM;1"	,
                                "cdrom:\\DATA\\SPRITES\\PLNBLUE.CLT;1"	,
                                "cdrom:\\DATA\\SPRITES\\MOUSE.TIM;1"	,
                                "cdrom:\\DATA\\SPRITES\\BLDNGS1.TIM;1"	};
									
static void* GameFileDest[] = { (GsSprite*)&GameTilesetSpr		,
                                (GsSprite*)&GamePlaneSpr		,
                                NULL,
                                (GsSprite*)&GameMouseSpr		,
                                (GsSprite*)&GameBuildingSpr		};

static char* GamePlt[] = { "cdrom:\\DATA\\LEVELS\\LEVEL1.PLT;1"	};
static void* GamePltDest[] = {(TYPE_FLIGHT_DATA*)&FlightData	,};
									
static char* GameLevelList[] = {	"cdrom:\\DATA\\LEVELS\\LEVEL1.LVL;1"};
static uint16_t GameLevelBuffer[GAME_MAX_MAP_SIZE];
									
static uint8_t GameLevelColumns;
static uint8_t GameLevelSize;

static char GameLevelTitle[LEVEL_TITLE_SIZE];

//Game local time
static uint8_t GameHour;
static uint8_t GameMinutes;

//Local flag for two-player game mode. Obtained from Menu
static bool TwoPlayersActive;

// Determines whether game has finished or not.
bool GameFinishedFlag;

/* ***************************************************************************************
 * 
 * @name: void Game(bool two_players)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:
 *  Game main loop. Called by "Menu" module.
 *	
 * @remarks:
 * 
 * ***************************************************************************************/

void Game(bool two_players)
{	
	TwoPlayersActive = two_players;
	GameInit();
	
	while(1)
	{
		if(GameFinishedFlag == true)
		{
			// Exit game on level finished.
            if(GameGuiFinishedDialog(&PlayerData[PLAYER_ONE]) == true)
            {
                break;
            }
		}

		if(GamePause() == true)
		{
			// Exit game if player desires to exit.
			break;
		}

		if(GameAircraftCollisionFlag == true)
		{
			GameGuiAircraftCollision(&PlayerData[PLAYER_ONE]);
			break;
		}
		
		GameEmergencyMode();
		
		GameCalculations();
		
		GameGraphics();
		
		if(GameStartupFlag == true)
		{
			GameStartupFlag = false;
		}
	}
	
	GfxDisableSplitScreen();
	
	EndAnimation();
	
	SfxPlayTrack(INTRO_TRACK);
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

bool GamePause(void)
{
	TYPE_PLAYER* ptrPlayer;
	uint8_t i;
	bool pause_flag = false;
	
	if(GameStartupFlag == true)
	{
		return false;
	}
	
	for(i = 0 ; i < MAX_PLAYERS ; i++)
	{
		ptrPlayer = &PlayerData[i];
		// Run player-specific functions for each player
		if(ptrPlayer->Active == true)
		{
			//Serial_printf("Released callback = 0x%08X\n", ptrPlayer->PadKeySinglePress_Callback);
			if(ptrPlayer->PadKeySinglePress_Callback(PAD_START) == true)
			{
				Serial_printf("Player %d set pause_flag to true!\n",i);
				pause_flag = true;
				break;
			}
		}
	}
	
	if(pause_flag == true)
	{	
		// Blocking function:
		// 	* Returns true if player pointed to by ptrPlayer wants to exit game
		//	* Returns false if player pointed to by ptrPlayer wants to resume game
		return GameGuiPauseDialog(ptrPlayer);
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

void GameInit(void)
{
	uint8_t i;
	uint32_t track;
    static bool firstLoad = true;
	
	GameStartupFlag = true;

    if(firstLoad == true)
    {
        firstLoad = false;

        LoadMenu(	GameFileList,
                    GameFileDest,
                    sizeof(GameFileList) / sizeof(char*),
                    sizeof(GameFileDest) /sizeof(void*)	);
    }

    LoadMenu(   GamePlt,
                GamePltDest,
                sizeof(GamePlt) / sizeof(GamePlt[0]),
                sizeof(GamePltDest) / sizeof(GamePltDest[0]) );
	
	GameLoadLevel();
	
	GameGuiInit();

    GameBuildingsInit();
	
	memset(GameRwy,0,GAME_MAX_RUNWAYS * sizeof(uint16_t) );
	
	memset(GameUsedRwy,0,GAME_MAX_RUNWAYS * sizeof(uint16_t) );
	
	PlayerData[PLAYER_ONE].Active = true;
	PlayerData[PLAYER_ONE].PadKeyPressed_Callback = &PadOneKeyPressed;
	PlayerData[PLAYER_ONE].PadKeyReleased_Callback = &PadOneKeyReleased;
	PlayerData[PLAYER_ONE].PadKeySinglePress_Callback = &PadOneKeySinglePress;
	PlayerData[PLAYER_ONE].PadDirectionKeyPressed_Callback = &PadOneDirectionKeyPressed;
	PlayerData[PLAYER_ONE].PadLastKeySinglePressed_Callback = &PadOneGetLastKeySinglePressed;
	PlayerData[PLAYER_ONE].FlightDataPage = 0;
	PlayerData[PLAYER_ONE].UnboardingSequenceIdx = 0;
	
	memset(PlayerData[PLAYER_ONE].UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS * sizeof(unsigned short) );
	
	PlayerData[PLAYER_TWO].Active = TwoPlayersActive? true : false;
	
	if(PlayerData[PLAYER_TWO].Active == true)
	{
		PlayerData[PLAYER_TWO].PadKeyPressed_Callback = &PadTwoKeyPressed;
		PlayerData[PLAYER_TWO].PadKeyReleased_Callback = &PadTwoKeyReleased;
		PlayerData[PLAYER_TWO].PadDirectionKeyPressed_Callback = &PadTwoDirectionKeyPressed;
		PlayerData[PLAYER_TWO].FlightDataPage = 0;
		PlayerData[PLAYER_TWO].PadKeySinglePress_Callback = &PadTwoKeySinglePress;
		PlayerData[PLAYER_TWO].PadLastKeySinglePressed_Callback = &PadTwoGetLastKeySinglePressed;
		PlayerData[PLAYER_TWO].UnboardingSequenceIdx = 0;
		
		memset(PlayerData[PLAYER_TWO].UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS * sizeof(unsigned short) );
		
		// On 2-player mode, one player controls departure flights and
		// other player controls arrival flights.
		PlayerData[PLAYER_ONE].FlightDirection = DEPARTURE;
		PlayerData[PLAYER_TWO].FlightDirection = ARRIVAL;
	}
	else
	{
		PlayerData[PLAYER_ONE].FlightDirection = DEPARTURE | ARRIVAL;
	}
	
	for(i = 0; i < MAX_PLAYERS ; i++)
	{
		CameraInit(&PlayerData[i]);
		PlayerData[i].ShowAircraftData = false;
		PlayerData[i].SelectRunway = false;
		PlayerData[i].SelectTaxiwayRunway = false;
		PlayerData[i].LockTarget = false;
		PlayerData[i].SelectedAircraft = 0;
		PlayerData[i].FlightDataPage = 0;
		memset(&PlayerData[i].Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
		PlayerData[i].WaypointIdx = 0;
		PlayerData[i].LastWaypointIdx = 0;
	}
	
	GameAircraftCreatedFlag = false;
	GameAircraftCollisionFlag = false;
	GameAircraftCollisionIdx = 0;
	
	if(GameTwoPlayersActive() == true)
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

	GameSpawnMinTime = TimerCreate(GAME_MINIMUM_PARKING_SPAWN_TIME, false, &GameMinimumSpawnTimeout);

	spawnMinTimeFlag = false;

	GameScore = 0;
	
	GameGetRunwayArray();
	
	GameSelectedTile = 0;

    GameFinishedFlag = false;
	
	AircraftInit();
				
	LoadMenuEnd();
	
	//GfxSetGlobalLuminance(0);
	
	track = SystemRand(GAMEPLAY_FIRST_TRACK, GAMEPLAY_LAST_TRACK);
	
	SfxPlayTrack(track);	
}

/* ***************************************************************************************
 * 
 * @name: void GameBuildingsInit(void)
 * 
 * @author: Xavier Del Campo
 *
 * 
 * @brief:
 *  Reportedly, it initializes coordinate/size data for each building instance.
 *	
 * @remarks:
 *
 * 
 * ***************************************************************************************/

void GameBuildingsInit(void)
{
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
        BUILDING_HANGAR_V = 34,
        BUILDING_HANGAR_W = 51,
        BUILDING_HANGAR_H = 36,

        BUILDING_ATC_TOWER_U = 58,
        BUILDING_ATC_TOWER_V = 0,
        BUILDING_ATC_TOWER_W = 29,
        BUILDING_ATC_TOWER_H = 34,
    };

    enum
    {
        BUILDING_ILS_ORIGIN_X = 10,
        BUILDING_ILS_ORIGIN_Y = 22,

        BUILDING_GATE_ORIGIN_X = 20,
        BUILDING_GATE_ORIGIN_Y = 8,

        BUILDING_HANGAR_ORIGIN_X = 20,
        BUILDING_HANGAR_ORIGIN_Y = 11,
        
        BUILDING_ATC_TOWER_ORIGIN_X = 12,
        BUILDING_ATC_TOWER_ORIGIN_Y = 20,
    };

    memset(GameBuildingData, 0, sizeof(TYPE_BUILDING_DATA) );

    GameBuildingData[BUILDING_GATE].IsoPos.x = BUILDING_GATE_OFFSET_X;
    GameBuildingData[BUILDING_GATE].IsoPos.y = BUILDING_GATE_OFFSET_Y;
    // z coordinate set to 0 by default.

    // BUILDING_ATC_LOC coordinates inside tile.
    GameBuildingData[BUILDING_ATC_LOC].IsoPos.x = BUILDING_ATC_LOC_OFFSET_X;
    GameBuildingData[BUILDING_ATC_LOC].IsoPos.y = BUILDING_ATC_LOC_OFFSET_Y;
    // z coordinate set to 0 by default.
    GameBuildingData[BUILDING_GATE].orig_x = BUILDING_GATE_ORIGIN_X;
    GameBuildingData[BUILDING_GATE].orig_y = BUILDING_GATE_ORIGIN_Y;
    GameBuildingData[BUILDING_GATE].u = BUILDING_GATE_U;
    GameBuildingData[BUILDING_GATE].v = BUILDING_GATE_V;
    GameBuildingData[BUILDING_GATE].w = BUILDING_GATE_W;
    GameBuildingData[BUILDING_GATE].h = BUILDING_GATE_H;

    // BUILDING_ILS coordinates inside tile.
    GameBuildingData[BUILDING_ILS].IsoPos.x = BUILDING_ILS_OFFSET_X;
    GameBuildingData[BUILDING_ILS].IsoPos.y = BUILDING_ILS_OFFSET_Y;
    // z coordinate set to 0 by default.
    GameBuildingData[BUILDING_ILS].orig_x = BUILDING_ILS_ORIGIN_X;
    GameBuildingData[BUILDING_ILS].orig_y = BUILDING_ILS_ORIGIN_Y;
    GameBuildingData[BUILDING_ILS].u = BUILDING_ILS_U;
    GameBuildingData[BUILDING_ILS].v = BUILDING_ILS_V;
    GameBuildingData[BUILDING_ILS].w = BUILDING_ILS_W;
    GameBuildingData[BUILDING_ILS].h = BUILDING_ILS_H;

    // BUILDING_HANGAR coordinates inside tile.
    GameBuildingData[BUILDING_HANGAR].IsoPos.x = BUILDING_HANGAR_OFFSET_X;
    GameBuildingData[BUILDING_HANGAR].IsoPos.y = BUILDING_HANGAR_OFFSET_Y;
    // z coordinate set to 0 by default.
    GameBuildingData[BUILDING_HANGAR].orig_x = BUILDING_HANGAR_ORIGIN_X;
    GameBuildingData[BUILDING_HANGAR].orig_y = BUILDING_HANGAR_ORIGIN_Y;
    GameBuildingData[BUILDING_HANGAR].u = BUILDING_HANGAR_U;
    GameBuildingData[BUILDING_HANGAR].v = BUILDING_HANGAR_V;
    GameBuildingData[BUILDING_HANGAR].w = BUILDING_HANGAR_W;
    GameBuildingData[BUILDING_HANGAR].h = BUILDING_HANGAR_H;

    // BUILDING_ATC_TOWER coordinates inside tile.
    GameBuildingData[BUILDING_ATC_TOWER].IsoPos.x = BUILDING_ATC_TOWER_OFFSET_X;
    GameBuildingData[BUILDING_ATC_TOWER].IsoPos.y = BUILDING_ATC_TOWER_OFFSET_Y;
    // z coordinate set to 0 by default.
    GameBuildingData[BUILDING_ATC_TOWER].orig_x = BUILDING_ATC_TOWER_ORIGIN_X;
    GameBuildingData[BUILDING_ATC_TOWER].orig_y = BUILDING_ATC_TOWER_ORIGIN_Y;
    GameBuildingData[BUILDING_ATC_TOWER].u = BUILDING_ATC_TOWER_U;
    GameBuildingData[BUILDING_ATC_TOWER].v = BUILDING_ATC_TOWER_V;
    GameBuildingData[BUILDING_ATC_TOWER].w = BUILDING_ATC_TOWER_W;
    GameBuildingData[BUILDING_ATC_TOWER].h = BUILDING_ATC_TOWER_H;

    // BUILDING_GATE coordinates inside tile.
    GameBuildingData[BUILDING_GATE].IsoPos.x = BUILDING_GATE_OFFSET_X;
    GameBuildingData[BUILDING_GATE].IsoPos.y = BUILDING_GATE_OFFSET_Y;
    // z coordinate set to 0 by default.
    
    /*BUILDING_ILS,
    BUILDING_ATC_TOWER,
    BUILDING_ATC_LOC,
    BUILDING_TERMINAL,
    BUILDING_TERMINAL_2,
    BUILDING_GATE,*/
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
    bool (*PadXConnected[MAX_PLAYERS])(void) = {    [PLAYER_ONE] = &PadOneConnected,
                                                    [PLAYER_TWO] = &PadTwoConnected };

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
	
	while(SystemGetEmergencyMode() == true)
	{
        GsRectangle errorRct = {.x = ERROR_RECT_X,
                                .w = ERROR_RECT_W,
                                .y = ERROR_RECT_Y,
                                .h = ERROR_RECT_H,
                                .r = ERROR_RECT_R,
                                .g = ERROR_RECT_G,
                                .b = ERROR_RECT_B   };

		// Pad one has been disconnected during gameplay
		// Show an error screen until it is disconnected again.
		
		GsSortCls(0,0,0);
		
		GsSortRectangle(&errorRct);

        for(i = 0; i < MAX_PLAYERS; i++)
        {
            TYPE_PLAYER* ptrPlayer = &PlayerData[i];

            if(ptrPlayer->Active == true)
            {
                if(PadXConnected[i]() == false)
                {
                    FontPrintText(&SmallFont, 48, 48 + (i << 5), "Pad %d disconnected", i);
                    SystemSetEmergencyMode(true);
                }
            }
        }

		GfxDrawScene_Slow();
	}
}

/* ***************************************************************************************
 * 
 * @name: void GameGetAircraftTilemap(uint8_t i)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 * 	uint8_t i:
 *      Index for FlightData table.
 * 
 * @brief:
 *  On each cycle, it creates a 2-dimensional array relating aircraft indexes against
 *  tile numbers.
 *	
 * @remarks:
 * 
 * ***************************************************************************************/

void GameGetAircraftTilemap(uint8_t i)
{
    uint16_t tileNr;
    uint8_t j;

    if(i == 0)
    {
        memset(GameAircraftTilemap, FLIGHT_DATA_INVALID_IDX, sizeof(GameAircraftTilemap) );
    }
    
    if(FlightData.State[i] == STATE_IDLE)
    {
        return;
    }

    tileNr = AircraftGetTileFromFlightDataIndex(i);

    for(j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
    {
        //DEBUG_PRINT_VAR(GameAircraftTilemap[tileNr][j]);
        if(GameAircraftTilemap[tileNr][j] == FLIGHT_DATA_INVALID_IDX)
        {
            break;
        }
    }
    
    GameAircraftTilemap[tileNr][j] = i;

    //Serial_printf("GameAircraftTileMap[%d][%d] = %d\n", tileNr, j, GameAircraftTilemap[tileNr][j]);
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

    // FlightData handling

    for(i = 0; i < FlightData.nAircraft; i++)
    {
        GameFinished(i);
        GameClockFlights(i);
        GameAircraftState(i);
        GameActiveAircraft(i);
        GameRemainingAircraft(i);
        GameGetAircraftTilemap(i);
    }
	
    AircraftHandler();
    GameGuiCalculateSlowScore();
	
	for(i = 0 ; i < MAX_PLAYERS ; i++)
	{
		// Run player-specific functions for each player
		if(PlayerData[i].Active == true)
		{
			GamePlayerHandler(&PlayerData[i], &FlightData);
		}
	}	
}

/* ***************************************************************************************
 * 
 * @name: void GamePlayerHandler(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 *  
 * @brief:
 *  Calls all routines attached to a player.
 *	
 * @remarks:
 * 
 * ***************************************************************************************/

void GamePlayerHandler(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	ptrPlayer->SelectedTile = 0;	// Reset selected tile if no states
									// which use this are currently active.
	ptrPlayer->InvalidPath = false; // Do the same thing for "InvalidPath".

	ptrPlayer->FlightDataSelectedAircraft = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

	if(GameAircraftCollisionFlag == true)
	{
		TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(GameAircraftCollisionIdx);
		CameraMoveToIsoPos(ptrPlayer, IsoPos);
	}

	GameActiveAircraftList(ptrPlayer, ptrFlightData);
	GameStateUnboarding(ptrPlayer, ptrFlightData);
	GameStateLockTarget(ptrPlayer, ptrFlightData);
	GameStateSelectRunway(ptrPlayer, ptrFlightData);
	GameStateSelectTaxiwayRunway(ptrPlayer, ptrFlightData);
	GameStateSelectTaxiwayParking(ptrPlayer, ptrFlightData);
	GameStateShowAircraft(ptrPlayer, ptrFlightData);
	CameraHandler(ptrPlayer);
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
	if(System1SecondTick() == true)
	{
		GameMinutes++;
		
		if(GameMinutes >= 60)
		{
			GameHour++;
			GameMinutes = 0;
		}
		
		if(GameHour >= 24)
		{
			GameHour = 0;
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
 * 	uint8_t i:
 *      Index for FlightData table.
 *  
 * @brief:
 *  Handles hours/minutes values for all active aircraft.
 *	
 * @remarks:
 * 
 * *******************************************************************/

void GameClockFlights(uint8_t i)
{	
    if(System1SecondTick() == true)
    {
        if(	(FlightData.Minutes[i] == 0)
                    &&
            (FlightData.Hours[i] > 0)	)
        {
            FlightData.Minutes[i] = 60;
            FlightData.Hours[i]--;
        }
        
        if(FlightData.Minutes[i] > 0)
        {
            FlightData.Minutes[i]--;
        }

        if( (FlightData.State[i] != STATE_IDLE)
                            &&
            (FlightData.RemainingTime[i] > 0) )
        {
            FlightData.RemainingTime[i]--;
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
 * 	Second half of game execution. Once GameCalculations() has ended,
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
	int i;
	bool split_screen = false;
	
	while( (SystemRefreshNeeded() == false) || (GfxIsGPUBusy() == true) );
			
	if(TwoPlayersActive == true)
	{
		split_screen = true;
	}
	
	if(GfxGetGlobalLuminance() < NORMAL_LUMINANCE)
	{
		// Fading from black effect on startup.
		GfxIncreaseGlobalLuminance(1);
	}

	firstLevelRender = true;
	
	for(i = 0; i < MAX_PLAYERS ; i++)
	{
        TYPE_PLAYER* ptrPlayer = &PlayerData[i];

		if(ptrPlayer->Active == true)
		{	
			if(split_screen == true)
			{
				GfxSetSplitScreen(i);
			}
			
			// Draw half split screen for each player
			// only if 2-player mode is active. Else, render
			// the whole screen as usual.
			
			GsSortCls(0,0,GfxGetGlobalLuminance() >> 1);

            // Ground tiles must be rendered first.

			GameRenderLevel(ptrPlayer);

            // Ground tiles are now rendered. Now, depending on building/aircraft
            // positions, determine in what order they should be rendered.

            GameRenderBuildingAircraft(ptrPlayer);

			GameGuiAircraftList(ptrPlayer, &FlightData);
			
			GameDrawMouse(ptrPlayer);

			GameGuiDrawUnboardingSequence(ptrPlayer);

            if(split_screen == true)
            {
                GfxDrawScene_NoSwap();
                while(GfxIsGPUBusy() == true);
            }
		}
	}
	
	// Avoid changing drawing environment twice on 1-player mode
	// as it doesn't make any sense.
	if(split_screen == true)
	{
		GfxDisableSplitScreen();
	}
	
	// Draw common elements for both players (messages, clock...)
	
	GameGuiBubble(&FlightData);
	
	GameGuiClock(GameHour,GameMinutes);

	GameGuiShowScore();

    if(split_screen == true)
    {
        GfxDrawScene_NoSwap();
    }

    GfxDrawScene();
}

/* *******************************************************************
 * 
 * @name: void GameGraphics(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 	Determines rendering order depending on building/aircraft
 *  isometric position data.
 *  
 * @remarks:
 * 
 * *******************************************************************/

void GameRenderBuildingAircraft(TYPE_PLAYER* ptrPlayer)
{
    uint8_t tileNr;
    uint8_t rows = 0;
    uint8_t columns = 0;
    uint8_t k;

    for(tileNr = 0; tileNr < GameLevelSize; tileNr++)
    {
        // Building data is stored in GameLevelBuffer MSB. LSB is dedicated to tile data.
        uint8_t CurrentBuilding = (uint8_t)(GameLevelBuffer[tileNr] >> 8);
        uint8_t j;
        uint8_t AircraftRenderOrder[GAME_MAX_AIRCRAFT_PER_TILE];
        short Aircraft_Y_Data[GAME_MAX_AIRCRAFT_PER_TILE];

        memset(AircraftRenderOrder, FLIGHT_DATA_INVALID_IDX, sizeof(AircraftRenderOrder) );

        for(j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
        {
            // Fill with 0x7FFF (maximum 16-bit positive value).
            Aircraft_Y_Data[j] = 0x7FFF;
        }

        //memset(Aircraft_Y_Data, 0x7F, GAME_MAX_AIRCRAFT_PER_TILE * sizeof(short));

        for(j = 0; j < GAME_MAX_AIRCRAFT_PER_TILE; j++)
        {
            uint8_t AircraftIdx = GameAircraftTilemap[tileNr][j];
            
            TYPE_ISOMETRIC_POS aircraftIsoPos = AircraftGetIsoPos(AircraftIdx);

            if(AircraftIdx == FLIGHT_DATA_INVALID_IDX)
            {
                // No more aircraft on this tile.
                break;
            }

            //DEBUG_PRINT_VAR(aircraftIsoPos.y);

            for(k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                if(aircraftIsoPos.y < Aircraft_Y_Data[k])
                {
                    uint8_t idx;

                    for(idx = k; idx < (GAME_MAX_AIRCRAFT_PER_TILE - 1); idx++)
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

            /*for(k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                Serial_printf("Aircraft_Y_Data[%d] = %d\n", k, Aircraft_Y_Data[k]);
                Serial_printf("AircraftRenderOrder[%d] = %d\n", k, AircraftRenderOrder[k]);
            }*/
        }

        if(CurrentBuilding == BUILDING_NONE)
        {
            for(k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
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

            TYPE_ISOMETRIC_POS buildingIsoPos = {   .x = (columns << (TILE_SIZE_BIT_SHIFT)) + x_bldg_offset,
                                                    .y = (rows << (TILE_SIZE_BIT_SHIFT)) + y_bldg_offset,
                                                    .z = z_bldg_offset  };

            // Isometric -> Cartesian conversion
            //buildingIsoPos.x = (columns << (TILE_SIZE_BIT_SHIFT)) + x_bldg_offset;
            //buildingIsoPos.y = (rows << (TILE_SIZE_BIT_SHIFT)) + y_bldg_offset;
            //buildingIsoPos.z = z_bldg_offset;

            TYPE_CARTESIAN_POS buildingCartPos = GfxIsometricToCartesian(&buildingIsoPos);
            bool buildingDrawn = false;

            // Define new coordinates for building.
    
            GameBuildingSpr.x = buildingCartPos.x - GameBuildingData[CurrentBuilding].orig_x;
            GameBuildingSpr.y = buildingCartPos.y - GameBuildingData[CurrentBuilding].orig_y;

            GameBuildingSpr.u = orig_u + GameBuildingData[CurrentBuilding].u;
            GameBuildingSpr.v = orig_v + GameBuildingData[CurrentBuilding].v;
            GameBuildingSpr.w = GameBuildingData[CurrentBuilding].w;
            GameBuildingSpr.h = GameBuildingData[CurrentBuilding].h;

            //DEBUG_PRINT_VAR(buildingIsoPos.x);
            //DEBUG_PRINT_VAR(buildingIsoPos.y);

            CameraApplyCoordinatesToSprite(ptrPlayer, &GameBuildingSpr);

            for(k = 0; k < GAME_MAX_AIRCRAFT_PER_TILE; k++)
            {
                if(AircraftRenderOrder[k] == FLIGHT_DATA_INVALID_IDX)
                {
                    if(buildingDrawn == false)
                    {
                        GfxSortSprite(&GameBuildingSpr);

                        GameBuildingSpr.u = orig_u;
                        GameBuildingSpr.v = orig_v;
                        
                        buildingDrawn = true;
                    }

                    break;
                }
            
                if(Aircraft_Y_Data[k] < buildingIsoPos.y)
                {
                    AircraftRender(ptrPlayer, AircraftRenderOrder[k]);
                }
                else
                {
                    if(buildingDrawn == false)
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

        if(columns < (GameLevelColumns - 1) )
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
 * 	Loads and parses *.LVL data.
 *	
 * 
 * @remarks:
 * 	Filepath for *.LVL is given by GameLevelList[0]. Do NOT ever move
 *  it from there to avoid problems!
 * 
 * *******************************************************************/

void GameLoadLevel(void)
{	
	uint8_t i = 0;
	uint8_t* ptrBuffer;
	char LevelHeader[LEVEL_MAGIC_NUMBER_SIZE + 1];
	
	/* TODO - Very important */
	// Map contents (that means, without header) should be copied to GameLevelBuffer
	// Header treatment (magic number, map size, map title...) should be done
	// using System's file buffer.
	
	if(SystemLoadFile(GameLevelList[0]) == false)
	{
		return;
	}
	
	ptrBuffer = SystemGetBufferAddress();
	
	//SystemLoadFileToBuffer(GameLevelList[0],GameLevelBuffer,GAME_MAX_MAP_SIZE);
	
	memset(LevelHeader,0, LEVEL_MAGIC_NUMBER_SIZE + 1);
	
	memcpy(LevelHeader,ptrBuffer,LEVEL_MAGIC_NUMBER_SIZE);
	
	LevelHeader[LEVEL_MAGIC_NUMBER_SIZE] = '\0';
	
	Serial_printf("Level header: %s\n",LevelHeader);
	
	if(strncmp(LevelHeader,LEVEL_MAGIC_NUMBER_STRING,LEVEL_MAGIC_NUMBER_SIZE) != 0)
	{
		Serial_printf("Invalid level header! Read \"%s\" instead of \"ATC\"\n",LevelHeader);
		return;
	}
	
	i += LEVEL_MAGIC_NUMBER_SIZE;
	
	GameLevelColumns = ptrBuffer[i++];
	
	Serial_printf("Level size: %d\n",GameLevelColumns);
	
	if(	(GameLevelColumns < MIN_MAP_COLUMNS)
			||
		(GameLevelColumns > MAX_MAP_COLUMNS)	)
	{
		Serial_printf("Invalid map size! Value: %d\n",GameLevelColumns);	
		return;
	}
	
	GameLevelSize = GameLevelColumns * GameLevelColumns;
	
	memset(GameLevelTitle,0,LEVEL_TITLE_SIZE);
	
	memcpy(GameLevelTitle,&ptrBuffer[i],LEVEL_TITLE_SIZE);
	
	Serial_printf("Game level title: %s\n",GameLevelTitle);
	
	i += LEVEL_TITLE_SIZE;
	
	memset(GameLevelBuffer,0,GAME_MAX_MAP_SIZE);
	
	i = LEVEL_HEADER_SIZE;
	
	memcpy(GameLevelBuffer, &ptrBuffer[i], GameLevelSize * sizeof(uint16_t)); // 2 bytes per tile
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
 * 	It determines what state should be applied to aircraft when spawn timer expires.
 *  
 * @remarks:
 * 	This is where TYPE_FLIGHT_DATA is transferred to TYPE_AIRCRAFT on departure.
 * 
 * ******************************************************************************************/
 
void GameAircraftState(uint8_t i)
{
	uint16_t target[2] = {0};
	// Arrays are copied to AircraftAddNew, so we create a first and only
	// target which is the parking tile itself, and the second element
	// is just the NULL character.
	// Not an ideal solution, but the best one currently available.
	
    if(FlightData.Finished[i] == false)
    {
        if(	(FlightData.Hours[i] == 0)
                    &&
            (FlightData.Minutes[i] == 0)
                    &&
            (FlightData.State[i] == STATE_IDLE)
                    &&
            (FlightData.RemainingTime[i] > 0)
                    &&
            (spawnMinTimeFlag == false)			)
        {
            if( (FlightData.FlightDirection[i] == DEPARTURE)
                                &&
                (FlightData.Parking[i] != 0)				)
            {
                uint8_t j;
                bool bParkingBusy = false;

                for(j = 0; j < FlightData.nAircraft; j++)
                {
                    if(AircraftFromFlightDataIndex(j)->State != STATE_IDLE)
                    {
                        uint16_t tile = AircraftGetTileFromFlightDataIndex(j);
                        uint16_t* targets = AircraftGetTargets(j);

                        if(tile == FlightData.Parking[i])
                        {
                            bParkingBusy = true;
                        }

                        if(SystemContains_u16(FlightData.Parking[i], targets, AIRCRAFT_MAX_TARGETS) == true)
                        {
                            bParkingBusy = true;
                        }
                    }
                }

                if(bParkingBusy == false)
                {
                    FlightData.State[i] = STATE_PARKED;

                    GameAircraftCreatedFlag = true;

                    // Create notification request for incoming aircraft
                    GameGuiBubbleShow();
                    
                    target[0] = FlightData.Parking[i];
                    
                    Serial_printf("Target assigned = %d\n", target[0]);

                    Serial_printf("2\n");
                    
                    if(AircraftAddNew(&FlightData, i, target) == false)
                    {
                        Serial_printf("Exceeded maximum aircraft number!\n");
                        return;
                    }
                }
            }
            else if(FlightData.FlightDirection[i] == ARRIVAL)
            {
                Serial_printf("Flight %d set to STATE_APPROACH.\n", i);
                FlightData.State[i] = STATE_APPROACH;
                GameAircraftCreatedFlag = true;

                // Create notification request for incoming aircraft
                GameGuiBubbleShow();
            }
        }

        if( (FlightData.State[i] != STATE_IDLE)
                            &&
            (FlightData.RemainingTime[i] == 0)	)
        {
            // Player(s) lost a flight!
            GameRemoveFlight(i, false);
        }
    }
}

/* ******************************************************************************************
 * 
 * @name: void GameRenderLevel(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 * 
 * @brief:
 * 	Draws all tiles depending on GameLevelBuffer configuration.
 *  
 * @remarks:
 * 	Tiles are usually rendered with normal RGB values unless parking/runway is busy
 *  or ptrPlayer->InvalidPath == true.
 * 
 * ******************************************************************************************/

void GameRenderLevel(TYPE_PLAYER* ptrPlayer)
{
	uint16_t i;
	uint8_t columns = 0;
	uint8_t rows = 0;
	bool flip_id;
	bool used_rwy = SystemContains_u16(ptrPlayer->RwyArray[0], GameUsedRwy, GAME_MAX_RUNWAYS);
	uint8_t aux_id;
	GsSprite * ptrTileset;
	const uint8_t rwy_sine_step = 24;
	static unsigned char rwy_sine = rwy_sine_step;
	static bool rwy_sine_decrease = false;
	TYPE_ISOMETRIC_POS tileIsoPos;
	TYPE_CARTESIAN_POS tileCartPos;

    //uint16_t init_timer_value = 0;
    //uint16_t end_timer_value = 0;
	
	// Prepare runway to be painted in blue if player is on runway selection mode
	if(ptrPlayer->SelectRunway == true)
	{
		
		/*Serial_printf("Runway array:\n");
		
		for(j = 0; j < GAME_MAX_RWY_LENGTH; j++)
		{
			Serial_printf("%d ",ptrPlayer->RwyArray[j]);
		}
		
		Serial_printf("\n");*/
	}

	if(firstLevelRender == true)
	{
		// Avoid re-entrance.

		firstLevelRender = false;

		if(rwy_sine_decrease == false)
		{
			if(rwy_sine < 240)
			{
				rwy_sine += rwy_sine_step;
			}
			else
			{
				rwy_sine_decrease = true;
			}
		}
		else
		{
			if(rwy_sine > rwy_sine_step)
			{
				rwy_sine -= rwy_sine_step;
			}
			else
			{
				rwy_sine_decrease = false;
			}
		}
	}
	
	for(i = 0 ; i < GameLevelSize; i++)
	{
		// GameLevelBuffer bits explanation:
		// X X X X  X X X X     X X X X     X X X X
		// | | | |  | | | |     | | | |     | | | |
		// | | | |  | | | |     | | | |     | | | V
		// | | | |  | | | |     | | | |     | | V Tile, bit 0
		// | | | |  | | | |     | | | |     | V Tile, bit 1
		// | | | |  | | | |     | | | |     V Tile, bit 2
		// | | | |  | | | |     | | | V     Tile, bit 3
		// | | | |  | | | |     | | V Tile, bit 4
		// | |            |     | V Tile, bit 5
		// |              |     V Tile, bit 6
        //                |     Tile mirror flag
                          
		uint8_t CurrentTile = (uint8_t)(GameLevelBuffer[i] & 0x00FF);

        // Isometric -> Cartesian conversion
		tileIsoPos.x = columns << (TILE_SIZE_BIT_SHIFT);
		tileIsoPos.y = rows << (TILE_SIZE_BIT_SHIFT);
		tileIsoPos.z = 0;
		
		tileCartPos = GfxIsometricToCartesian(&tileIsoPos);
		
		// Flipped tiles have bit 7 set.
		if(CurrentTile & TILE_MIRROR_FLAG)
		{
			flip_id = true;
			aux_id = CurrentTile;
			CurrentTile &= ~(TILE_MIRROR_FLAG);
		}
		else
		{
			flip_id = false;
		}
		
		if(CurrentTile <= LAST_TILE_TILESET1)
		{
			// Draw using GameTilesetSpr
			ptrTileset = &GameTilesetSpr;
		}
		else if(	(CurrentTile > LAST_TILE_TILESET1)
						&&
					(CurrentTile <= LAST_TILE_TILESET2)	)
		{
			// Draw using GameTileset2Spr
			ptrTileset = &GameTileset2Spr;
		}
		else
		{
			ptrTileset = NULL;

			continue;
			
//			if(flip_id == false)
//			{
//				continue;
//			}
		}
		
		ptrTileset->w = TILE_SIZE;
		ptrTileset->h = TILE_SIZE_H;

        ptrTileset->x = tileCartPos.x;
		ptrTileset->y = tileCartPos.y;
		
		// Set coordinate origin to left upper corner
		ptrTileset->x -= TILE_SIZE >> 1;
		//ptrTileset->y -= TILE_SIZE_H >> 2;

        CameraApplyCoordinatesToSprite(ptrPlayer, ptrTileset);

        if(columns < (GameLevelColumns - 1) )
		{
			columns++;
		}
		else
		{
			rows++;
			columns = 0;
		}

        if(GfxIsSpriteInsideScreenArea(ptrTileset) == false)
        {
            continue;
        }

		ptrTileset->r = NORMAL_LUMINANCE;
		ptrTileset->g = NORMAL_LUMINANCE;
		ptrTileset->b = NORMAL_LUMINANCE;
		
		if(	(ptrPlayer->SelectRunway == true)
						&&
			(i != 0)
						&&
			(SystemContains_u16(i, ptrPlayer->RwyArray, GAME_MAX_RWY_LENGTH) == true)	)
		{
			
			if(used_rwy == true)
			{
				ptrTileset->r = rwy_sine;
				ptrTileset->b = NORMAL_LUMINANCE >> 2;
				ptrTileset->g = NORMAL_LUMINANCE >> 2;
			}
			else
			{
				ptrTileset->r = NORMAL_LUMINANCE >> 2;
				ptrTileset->g = NORMAL_LUMINANCE >> 2;
				ptrTileset->b = rwy_sine;
			}
		}
		else if( (	(ptrPlayer->SelectTaxiwayParking == true)
										||
					(ptrPlayer->SelectTaxiwayRunway == true)	)
								&&
					(i != 0)		)
		{
			if((	(SystemContains_u16(i, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == true)
								||
					(i == ptrPlayer->SelectedTile)	)
								&&
					(ptrPlayer->SelectedTile != GAME_INVALID_TILE_SELECTION)	)
			{
				if(ptrPlayer->InvalidPath == true)
				{
					ptrTileset->r = rwy_sine;
					ptrTileset->b = NORMAL_LUMINANCE >> 2;
					ptrTileset->g = NORMAL_LUMINANCE >> 2;
				}
				else
				{
					ptrTileset->r = NORMAL_LUMINANCE >> 2;
					ptrTileset->g = NORMAL_LUMINANCE >> 2;
					ptrTileset->b = rwy_sine;
				}
			}
			else if(	(ptrPlayer->SelectTaxiwayRunway == true)
									&&
						(	(CurrentTile == TILE_RWY_HOLDING_POINT)
									||
							(CurrentTile == TILE_RWY_HOLDING_POINT_2)	)	)
			{
                ptrTileset->r = NORMAL_LUMINANCE >> 2;
                ptrTileset->g = rwy_sine;
                ptrTileset->b = NORMAL_LUMINANCE >> 2;
			}
			else if(	(ptrPlayer->SelectTaxiwayParking == true)
									&&
						(	(CurrentTile == TILE_PARKING)
									||
							(CurrentTile == TILE_PARKING_2)	)	)
			{
                //init_timer_value = GetRCnt(2);

                //end_timer_value = GetRCnt(2);
				
                ptrTileset->r = NORMAL_LUMINANCE >> 2;
                ptrTileset->g = rwy_sine;
                ptrTileset->b = NORMAL_LUMINANCE >> 2;
			}
		}
		
		if(ptrTileset != NULL)
		{
			if(flip_id == true)
			{
				ptrTileset->attribute |= H_FLIP;
			}
		}
		
		ptrTileset->u = (short)(CurrentTile % COLUMNS_PER_TILESET) << TILE_SIZE_BIT_SHIFT;
		ptrTileset->v = (short)(CurrentTile / COLUMNS_PER_TILESET) * TILE_SIZE_H;

		ptrTileset->mx = ptrTileset->u + (TILE_SIZE >> 1);
		ptrTileset->my = ptrTileset->v + (TILE_SIZE_H >> 1);
			
		if(flip_id == true)
		{
			flip_id = false;
			CurrentTile = aux_id;
		}

	//	Serial_printf("Tile %d, attribute 0x%X\n",i,ptrTileset->attribute);

		GfxSortSprite(ptrTileset);
				
		if(ptrTileset->attribute & H_FLIP)
		{
			ptrTileset->attribute &= ~(H_FLIP);
		}
	}

    /*dprintf("GameRenderLevel execution time = %d\t"
            "end_timer_value = 0x%04X\tinit_timer_value = 0x%04X\n",
            end_timer_value - init_timer_value,
            end_timer_value,
            init_timer_value                                );*/
}

/* *******************************************************************
 * 
 * @name: void GameSetTime(uint8_t hour, uint8_t minutes)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 	Reportedly, it sets game time to specified hour and minutes.
 *	
 * 
 * @remarks:
 * 	To be used on GameInit() after PLT file parsing.
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
 * 	On each game cycle, FlightData.ActiveAircraft is set to 0 and
 *  number of active aircraft is recalculated.
 * 
 * @remarks:
 * 	Called ciclically from GameCalculations(). This function is
 *  executed GAME_MAX_AIRCRAFT times on each cycle.
 * 
 * *******************************************************************/

void GameActiveAircraft(uint8_t i)
{
    // Reset iterator when i == 0.

    if(i == 0)
    {
        FlightData.ActiveAircraft = 0;
    }
	
    if(FlightData.State[i] != STATE_IDLE)
    {
        FlightData.ActiveAircraft++;
    }
}

/* ******************************************************************************************
 * 
 * @name: void GameStateShowAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Handles ptrPlayer->ShowAircraftData state.
 *  
 * 
 * @remarks:
 * 	Called ciclically from GamePlayerHandler().
 * 
 * ******************************************************************************************/

void GameStateShowAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	if(ptrPlayer->ShowAircraftData == true)
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE) == true)
		{
			ptrPlayer->ShowAircraftData = false;
		}
	}
	
	if(ptrPlayer->PadKeySinglePress_Callback(PAD_CIRCLE) == true)
	{
		if(GameGuiShowAircraftDataSpecialConditions(ptrPlayer) == false)
		{
			//Invert ptrPlayer->ShowAircraftData value
			ptrPlayer->ShowAircraftData = ptrPlayer->ShowAircraftData ? false : true;
		}
	}
}

/* ******************************************************************************************
 * 
 * @name: void GameStateLockTarget(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Handles ptrPlayer->LockTarget state.
 *  
 * 
 * @remarks:
 * 	Called ciclically from GamePlayerHandler().
 * 
 ******************************************************************************************/

void GameStateLockTarget(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
	
	if(ptrPlayer->LockTarget == true)
	{
        if(ptrPlayer->LockedAircraft != FLIGHT_DATA_INVALID_IDX)
        {
            CameraMoveToIsoPos(ptrPlayer, AircraftGetIsoPos(ptrPlayer->LockedAircraft) );
        }
	}
	
	if(ptrPlayer->PadKeySinglePress_Callback(PAD_SQUARE) == true)
	{
		if(ptrPlayer->LockTarget == false)
		{
			if(ptrPlayer->ShowAircraftData == true)
			{
				if( (ptrFlightData->State[AircraftIdx] != STATE_IDLE)
												&&
					(ptrFlightData->State[AircraftIdx] != STATE_APPROACH) )
				{
					ptrPlayer->LockTarget = true;
					ptrPlayer->LockedAircraft = AircraftIdx;
				}
			}
		}
		else
		{
			ptrPlayer->LockTarget = false;
			ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
		}
	}
	else if(ptrPlayer->PadDirectionKeyPressed_Callback() == true)
	{
		if( (ptrPlayer->LockTarget == true)
						&&
			(ptrPlayer->ShowAircraftData == false) )
		{
			ptrPlayer->LockTarget = false;
			ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
		}
	}
}

/* ******************************************************************************************
 * 
 * @name: void GameStateSelectTaxiwayRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Handler for ptrPlayer->SelectTaxiwayRunway.
 *  
 * 
 * @remarks:
 * 	Called ciclically from GamePlayerHandler().
 * 
 * ******************************************************************************************/

void GameStateSelectTaxiwayRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	TYPE_ISOMETRIC_POS IsoPos = CameraGetIsoPos(ptrPlayer);
	uint8_t i;
	uint16_t target_tile;
	
	/*Serial_printf("Camera is pointing to {%d,%d}\n",IsoPos.x, IsoPos.y);*/
	
	if(ptrPlayer->SelectTaxiwayRunway == true)
	{
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
		
		ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);
		
		if(GamePathToTile(ptrPlayer, ptrFlightData) == false)
		{
			ptrPlayer->InvalidPath = true;
		}
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE) == true)
		{
			// State exit.
			ptrPlayer->SelectTaxiwayRunway = false;
			// Clear waypoints array.
			memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
			ptrPlayer->WaypointIdx = 0;
			ptrPlayer->LastWaypointIdx = 0;
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			if(ptrPlayer->InvalidPath == false)
			{
				for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
				{
					if(ptrPlayer->Waypoints[i] == 0)
					{
						break;
					}
					
					ptrPlayer->LastWaypointIdx = i;
				}
				
				target_tile = GameLevelBuffer[ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx]];
				
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
						
						for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
						{
							Serial_printf("%d ",ptrPlayer->Waypoints[i]);
						}
						
						Serial_printf("\n");
						
						// Clear waypoints array.
						memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);

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
 * @name: void GameStateSelectTaxiwayParking(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Handler for ptrPlayer->SelectTaxiwayParking.
 *  
 * @remarks:
 * 
 * **************************************************************************************************/

void GameStateSelectTaxiwayParking(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	TYPE_ISOMETRIC_POS IsoPos = CameraGetIsoPos(ptrPlayer);
	uint8_t i;
	uint16_t target_tile;
	
	if(ptrPlayer->SelectTaxiwayParking == true)
	{
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;
		
		ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);
		
		if(GamePathToTile(ptrPlayer, ptrFlightData) == false)
		{
			ptrPlayer->InvalidPath = true;
		}
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE) == true)
		{
			// State exit.
			ptrPlayer->SelectTaxiwayParking = false;
			// Clear waypoints array.
			memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
			ptrPlayer->WaypointIdx = 0;
			ptrPlayer->LastWaypointIdx = 0;
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			if(ptrPlayer->InvalidPath == false)
			{
				for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
				{
					if(ptrPlayer->Waypoints[i] == 0)
					{
						break;
					}
					
					ptrPlayer->LastWaypointIdx = i;
				}
				
				target_tile = GameLevelBuffer[ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx]];
				
				Serial_printf("ptrPlayer->LastWaypointIdx = %d\n",
						ptrPlayer->LastWaypointIdx);
				
				Serial_printf("target_tile = %d, TILE_PARKING = %d\n",
						target_tile,
						TILE_PARKING);
				
				if(	(target_tile == TILE_PARKING)
								||
					(target_tile == (TILE_PARKING | TILE_MIRROR_FLAG))
								||
					(target_tile == TILE_PARKING_2)
								||
					(target_tile == (TILE_PARKING_2 | TILE_MIRROR_FLAG) ) )
				{
					// TODO: Assign path to aircraft
					AircraftFromFlightDataIndexAddTargets(ptrPlayer->FlightDataSelectedAircraft, ptrPlayer->Waypoints);
					
					Serial_printf("Added these targets to aircraft %d:\n", ptrPlayer->FlightDataSelectedAircraft);
					
					for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
					{
						Serial_printf("%d ",ptrPlayer->Waypoints[i]);
					}
					
					Serial_printf("\n");
					
					ptrPlayer->SelectTaxiwayParking = false;
					// Clear waypoints array.
					memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
					ptrPlayer->WaypointIdx = 0;
					ptrPlayer->LastWaypointIdx = 0;
					
					ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] = STATE_TAXIING;
					GameScore += SCORE_REWARD_TAXIING;
				}
			}
		}
	}
}

/* **************************************************************************************************
 * 
 * @name: void GameStateSelectRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Handler for ptrPlayer->SelectRunway.
 *  
 * @remarks:
 * 
 * **************************************************************************************************/

void GameStateSelectRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{	
	uint8_t i;
	bool success;
	TYPE_ISOMETRIC_POS IsoPos = {	GameGetXFromTile_short(GameRwy[ptrPlayer->SelectedRunway]),
									GameGetYFromTile_short(GameRwy[ptrPlayer->SelectedRunway]),
									0	};
	
	if(ptrPlayer->SelectRunway == true)
	{		
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;

        GameGetSelectedRunwayArray(GameRwy[ptrPlayer->SelectedRunway], ptrPlayer->RwyArray, sizeof(ptrPlayer->RwyArray));
		
		CameraMoveToIsoPos(ptrPlayer, IsoPos);
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE) == true)
		{
			ptrPlayer->SelectRunway = false;
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			ptrPlayer->SelectRunway = false;

			if(SystemContains_u16(GameRwy[ptrPlayer->SelectedRunway], GameUsedRwy, GAME_MAX_RUNWAYS) == false)
			{
				ptrPlayer->SelectRunway = false;
				Serial_printf("Player selected runway %d!\n",GameRwy[ptrPlayer->SelectedRunway]);
				
				success = false;
				
				for(i = 0; i < GAME_MAX_RUNWAYS; i++)
				{
					if(GameUsedRwy[i] == 0)
					{
						GameAssignRunwaytoAircraft(ptrPlayer, ptrFlightData);
						success = true;
						GameUsedRwy[i] = GameRwy[ptrPlayer->SelectedRunway];
						break;
					}
				}
				
				if(success == false)
				{
					Serial_printf("No available runways!\n");
				}
			}
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_LEFT) == true)
		{
			if(ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_APPROACH)
			{
				if(ptrPlayer->SelectedRunway != 0)
				{
					ptrPlayer->SelectedRunway--;
				}
			}
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_RIGHT) == true)
		{
			if(ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] == STATE_APPROACH)
			{
				if(ptrPlayer->SelectedRunway < (GAME_MAX_RUNWAYS - 1))
				{
					if(GameRwy[ptrPlayer->SelectedRunway + 1] != 0)
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
 * 	On startup, an array of runway headers is created from GameLevelBuffer once *.LVL is parsed.
 *  
 * @remarks:
 *  Do not confuse GameRwy with GameRwyArray, which are used for completely different purposes.
 * 
 * **************************************************************************************************/

void GameGetRunwayArray(void)
{
	uint8_t i;
	uint8_t j = 0;
	
	for(i = 0; i < GameLevelSize; i++)
	{
		if(GameLevelBuffer[i] == TILE_RWY_START_1)
		{
			if(SystemContains_u16(i, GameLevelBuffer, GAME_MAX_RUNWAYS) == false)
			{
				GameRwy[j++] = i;
			}
		}
	}
	
	Serial_printf("GameRwy = ");
	
	for(i = 0; i < GAME_MAX_RUNWAYS; i++)
	{
		if(GameRwy[i] == 0)
		{
			break;
		}
		
		Serial_printf("%d ", GameRwy[i]);
	}
	
	Serial_printf("\n");
}

/* **************************************************************************************************
 * 
 * @name: void GameSelectAircraftFromList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Determines actions for aircraft on PAD_CROSS pressed.
 *  
 * @remarks:
 * 
 * **************************************************************************************************/

void GameSelectAircraftFromList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
	FL_STATE aircraftState = ptrFlightData->State[AircraftIdx];
	
	if(ptrPlayer->ShowAircraftData == true)
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			if(ptrPlayer->ActiveAircraft != 0)
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
					break;

					case STATE_HOLDING_RWY:
					{
						TYPE_RWY_ENTRY_DATA rwyEntryData = {0};
						uint8_t i;

						ptrPlayer->SelectRunway = true;
						GameGetRunwayEntryTile(AircraftIdx, &rwyEntryData);

						for(i = 0; GameRwy[i] != 0 && (i < (sizeof(GameRwy) / sizeof(GameRwy[0]))); i++)
						{
							if(GameRwy[i] == rwyEntryData.rwyHeader)
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
			
			Serial_printf("aircraftState = %d\n", aircraftState);
			Serial_printf("AircraftIdx = %d\n", AircraftIdx);
		}
	}
}

/* **************************************************************************************************
 * 
 * @name: void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  uint16_t rwyHeader:
 *      Level tile number (located inside GameLevelBuffer) pointing to runway header.
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
 * 	Fills rwyArray with all the tile numbers (included in GameLevelBuffer) belonging to a
 *  runway with header pointed to by rwyHeader.
 *  
 * @remarks:
 * 
 * **************************************************************************************************/

void GameGetSelectedRunwayArray(uint16_t rwyHeader, uint16_t* rwyArray, size_t sz)
{
	typedef enum t_rwydir
	{
		RWY_DIR_EAST = 0,
		RWY_DIR_WEST,
		RWY_DIR_NORTH,
		RWY_DIR_SOUTH,
	}RWY_DIR;
	
	static uint16_t last_tile = 0;
	static uint8_t i = 0;
	static RWY_DIR dir;

    if(sz != (GAME_MAX_RWY_LENGTH * sizeof(uint16_t) ))
    {
        Serial_printf("GameGetSelectedRunwayArray: size %d is different"
                " than expected (%d bytes). Returning...\n",
                sz,
                (GAME_MAX_RWY_LENGTH * sizeof(uint16_t) ) );
        return;
    }
	
	if(rwyHeader != 0)
	{
		// This function is called recursively.
		// Since 0 is not a valid value (it's not allowed to place
		// a runway header on first tile), it is used to determine
		// when to start creating the array.

        // Part one: determine runway direction and call the function again with rwyHeader == 0.
        
		memset(rwyArray, 0, sz);
		last_tile = rwyHeader;
		i = 0;
		
		switch(GameLevelBuffer[rwyHeader])
		{
			case TILE_RWY_START_1:
				dir = RWY_DIR_EAST; 
				break;
			case TILE_RWY_START_2:
				dir = RWY_DIR_WEST; 
				break;
			case TILE_RWY_START_1 | TILE_MIRROR_FLAG:
				dir = RWY_DIR_SOUTH;
			case TILE_RWY_START_2 | TILE_MIRROR_FLAG:
				dir = RWY_DIR_NORTH;
			default:
				Serial_printf("Unknown direction for tile %d\n",rwyHeader);
				return;
			break;
		}
	}
	else
	{
        // Part two: append tiles to array until runway end is found.

		if(	(GameLevelBuffer[last_tile] == TILE_RWY_START_1)
						||
			(GameLevelBuffer[last_tile] == TILE_RWY_START_2)
						||
			(GameLevelBuffer[last_tile] == (TILE_RWY_START_1 | TILE_MIRROR_FLAG) )
						||
			(GameLevelBuffer[last_tile] == (TILE_RWY_START_2 | TILE_MIRROR_FLAG) )	)
		{
			// Runway end found
			rwyArray[i++] = last_tile;
			return;
		}
	}

    //DEBUG_PRINT_VAR(i);
	
	rwyArray[i++] = last_tile;

    //DEBUG_PRINT_VAR(rwyArray[i -1]);

	switch(dir)
	{
		case RWY_DIR_EAST:
			last_tile++;
			break;
		case RWY_DIR_WEST:
			last_tile--;
		case RWY_DIR_NORTH:
			last_tile -= GameLevelColumns;
		case RWY_DIR_SOUTH:
			last_tile += GameLevelColumns;
	}
	
	GameGetSelectedRunwayArray(0, rwyArray, sz);
}

/* **************************************************************************************************
 * 
 * @name: void GameAssignRunwaytoAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
 *      In the end, pointer to FlightData data table, which contains
 *      information about all available flights.
 * 
 * @brief:
 * 	Assigns a runway to an incoming aircraft (FlightDirection == ARRIVAL) depending on
 *  player selection.
 *  
 * @remarks:
 * 
 * **************************************************************************************************/

void GameAssignRunwaytoAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint16_t assignedRwy = GameRwy[ptrPlayer->SelectedRunway];
	uint8_t aircraftIndex = ptrPlayer->FlightDataSelectedAircraft;
	uint16_t rwyExit;
	uint32_t i;
	uint16_t targets[AIRCRAFT_MAX_TARGETS] = {0};
	uint8_t rwyTiles[GAME_MAX_RWY_LENGTH] = {0};
	
	// Remember that ptrPlayer->SelectedAircraft contains an index to
	// be used with ptrFlightData.
	
	/*typedef enum t_flstate
{
	STATE_IDLE = 0,
	STATE_PARKED,
	STATE_TAXIING,
	STATE_APPROACH,
	STATE_FINAL
	}FL_STATE;*/
	
	Serial_printf("aircraftIndex = %d\n",aircraftIndex);
	
	if(ptrFlightData->State[aircraftIndex] == STATE_APPROACH)
	{
        uint16_t rwyArray[GAME_MAX_RWY_LENGTH];

		ptrFlightData->State[aircraftIndex] = STATE_FINAL;
		GameScore += SCORE_REWARD_FINAL;
		
		GameGetSelectedRunwayArray(assignedRwy, rwyArray, sizeof(rwyArray));
		
		for(i = 0; i < GAME_MAX_RWY_LENGTH; i++)
		{
			rwyTiles[i] = GameLevelBuffer[rwyArray[i]];
		}
		
		i = SystemIndexOf_U8((uint8_t)TILE_RWY_EXIT, rwyTiles, 0, GAME_MAX_RWY_LENGTH);
		
		if(i == -1)
		{
			Serial_printf("ERROR: Could not find TILE_RWY_EXIT for runway header %d.\n", assignedRwy);
			return;
		}
		
		i = SystemIndexOf_U8((uint8_t)TILE_RWY_EXIT, rwyTiles, i + 1, GAME_MAX_RWY_LENGTH);
		
		if(i == -1)
		{
			Serial_printf("ERROR: Could not find second TILE_RWY_EXIT for runway header %d.\n", assignedRwy);
			return;
		} 
		
		rwyExit = rwyArray[i];
		
		targets[0] = assignedRwy;
		targets[1] = rwyExit;

		Serial_printf("1\n");
		
		if( AircraftAddNew(ptrFlightData,
							aircraftIndex,
							targets	) == false)
		{
			Serial_printf("Exceeded maximum aircraft number!\n");
			return;
		}
	}
	else if(ptrFlightData->State[aircraftIndex] == STATE_HOLDING_RWY)
	{
		TYPE_RWY_ENTRY_DATA rwyEntryData;
		uint8_t i;

		GameGetRunwayEntryTile(aircraftIndex, &rwyEntryData);
		
		targets[0] = rwyEntryData.rwyEntryTile;
		targets[1] = targets[0] + rwyEntryData.rwyStep;

		Serial_printf("Added the following targets = ");

		for(i = 0; i < (sizeof(targets) / sizeof(targets[0])); i++)
		{
			Serial_printf("%d ", targets[i]);
		}

		Serial_printf("\n");

		AircraftAddTargets(AircraftFromFlightDataIndex(aircraftIndex), targets);

		ptrFlightData->State[aircraftIndex] = STATE_ENTERING_RWY;
		
		/*uint16_t i;
		

		i = rwyEntryData.rwyEntryTile;

		DEBUG_PRINT_VAR(rwyEntryData.rwyEntryTile);
		DEBUG_PRINT_VAR(rwyEntryData.rwyStep);

		while(GameLevelBuffer[i] != TILE_RWY_START_1)
		{
			if(i > rwyEntryData.rwyStep)
			{
				i -= rwyEntryData.rwyStep;
			}
		}

		GameGetSelectedRunwayArray(i);*/
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
 *      Tile number from GameLevelBuffer.
 *
 * @return:
 *  Returns relative X position (no fixed-point arithmetic) given
 *  a tile number from GameLevelBuffer.
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
 *      Tile number from GameLevelBuffer.
 *
 * @return:
 *  Returns relative Y position (no fixed-point arithmetic) given
 *  a tile number from GameLevelBuffer.
 * 
 * @remarks:
 * 
 * *******************************************************************/

short GameGetYFromTile_short(uint16_t tile)
{
	short retVal;
	
	tile /= GameLevelColumns;
	
	//retVal = (fix16_t)(tile << TILE_SIZE_BIT_SHIFT);
	retVal = (tile << TILE_SIZE_BIT_SHIFT);
	
	// Always point to tile center
	//retVal += TILE_SIZE >> 1;
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
 *      Tile number from GameLevelBuffer.
 *
 * @return:
 *  Returns relative X position in 16.16 (fix16_t) fixed-point format
 *  given a tile number from GameLevelBuffer.
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
 *      Tile number from GameLevelBuffer.
 *
 * @return:
 *  Returns relative Y position in 16.16 (fix16_t) fixed-point format
 *  given a tile number from GameLevelBuffer.
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

			for(i = 0; i < GAME_MAX_RUNWAYS; i++)
			{
				if(GameUsedRwy[i] == firstTarget)
				{
					GameUsedRwy[i] = 0;
				}
			}
		break;
		
		case STATE_TAXIING:
			if(FlightData.FlightDirection[index] == DEPARTURE)
			{
				FlightData.State[index] = STATE_HOLDING_RWY;
			}
			else if(FlightData.FlightDirection[index] == ARRIVAL)
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
 *  Tile number to be used against GameLevelBuffer.
 * 
 * @remarks:
 *  GameLevelColumns is used to determine tile number.
 * 
 * ****************************************************************************/

uint16_t GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS* IsoPos)
{
	uint16_t tile;
	
	if(IsoPos == NULL)
	{
		return 0;
	}
	
	if(	(IsoPos->x < 0) || (IsoPos->y < 0) )
	{
		return GAME_INVALID_TILE_SELECTION; // Invalid XYZ position
	}
	
	tile = IsoPos->x >> TILE_SIZE_BIT_SHIFT;
	tile += (IsoPos->y >> TILE_SIZE_BIT_SHIFT) * GameLevelColumns;
	
	/*Serial_printf("Returning tile %d from position {%d, %d, %d}\n",
			tile,
			IsoPos->x,
			IsoPos->y,
			IsoPos->z	);*/
	
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
 * @name: void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Wrapper for GamePlayerAddWaypoint_Ex().
 * 
 * ****************************************************************************/

void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer)
{
	GamePlayerAddWaypoint_Ex(ptrPlayer, ptrPlayer->SelectedTile);
}

/* ****************************************************************************
 * 
 * @name: void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure.
 * 
 *  uint16_t tile:
 *      Tile number from GameLevelBuffer.
 *
 * @brief:
 *  It allows adding a tile number to ptrPlayer.
 *
 * @remark:
 *  To be used together with GamePathToTile().
 * 
 * ****************************************************************************/

void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* ptrPlayer, uint16_t tile)
{
	// "_Ex" function allow selecting a certain tile, whereas the other one
	// is a particulare case of "_Ex" for tile = ptrPlayer->SelectedTIle.
	
	if(ptrPlayer->WaypointIdx >= PLAYER_MAX_WAYPOINTS)
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
 * @name: bool GameWaypointCheckExisting(TYPE_PLAYER* ptrPlayer, uint16_t temp_tile)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure.
 * 
 *  uint16_t tile:
 *      Tile number from GameLevelBuffer.
 *
 * @brief:
 *  Checks if tile number temp_tile is already included on player's waypoint list.
 *
 * @return:
 *  True if waypoint is already included on waypoint list, false otherwise.
 * 
 * **************************************************************************************/

bool GameWaypointCheckExisting(TYPE_PLAYER* ptrPlayer, uint16_t temp_tile)
{
    if(SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
    {
        /*for(i = 0; i < FlightData.nAircraft; i++)
        {
            if( (ptrFlightData->State[i] != STATE_IDLE)
                            &&
                (AircraftMoving(i) == false)			)
            {
                if(temp_tile == AircraftGetTileFromFlightDataIndex(i))
                {
                    return false;	// Check pending!
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
 * @name: bool GamePathToTile(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 *  TYPE_FLIGHT_DATA* ptrFlightData:
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

bool GamePathToTile(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{	
	uint8_t AcceptedTiles[] = {	TILE_ASPHALT_WITH_BORDERS,
								TILE_PARKING, TILE_RWY_MID,
								TILE_RWY_EXIT, TILE_TAXIWAY_CORNER_GRASS,
								TILE_TAXIWAY_CORNER_GRASS_2, TILE_TAXIWAY_GRASS,
								TILE_TAXIWAY_INTERSECT_GRASS,
								TILE_RWY_HOLDING_POINT, TILE_RWY_HOLDING_POINT_2 };
								
	uint8_t i;
	uint8_t j;
	
	uint16_t x_diff;
	uint16_t y_diff;
	uint16_t temp_tile;
	
	if(ptrPlayer->SelectedTile == GAME_INVALID_TILE_SELECTION)
	{
		return false;
	}
	
	for(i = (ptrPlayer->LastWaypointIdx + 1); i < PLAYER_MAX_WAYPOINTS; i++)
	{
		ptrPlayer->Waypoints[i] = 0;
	}
	
	ptrPlayer->WaypointIdx = ptrPlayer->LastWaypointIdx + 1;
	
	x_diff = (uint16_t)abs(	(ptrPlayer->SelectedTile % GameLevelColumns) -
							(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns) );
	
	y_diff = (uint16_t)abs(	(ptrPlayer->SelectedTile / GameLevelColumns) -
							(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns) );
							
	/*Serial_printf("SelectedTile = %d, ptrPlayer->Waypoints[%d] = %d\n",
			ptrPlayer->SelectedTile,
			0,
			ptrPlayer->Waypoints[0] );

	Serial_printf("X = abs(%d - %d)\n",
			ptrPlayer->SelectedTile % GameLevelColumns,
			(ptrPlayer->Waypoints[0] % GameLevelColumns) );
			
	Serial_printf("Y = abs(%d - %d)\n",
			ptrPlayer->SelectedTile / GameLevelColumns,
			(ptrPlayer->Waypoints[0] / GameLevelColumns) );
			
	Serial_printf("Diff = {%d, %d}\n", x_diff, y_diff);*/
	
	// At this point, we have to update current waypoints list.
	// ptrPlayer->Waypoints[ptrPlayer->WaypointIdx - 1] points to the last inserted point,
	// so now we have to determine how many points need to be created.
	
	temp_tile = ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx];
	
	if(x_diff >= y_diff)
	{
		while( (x_diff--) > 0)
		{
			if( (ptrPlayer->SelectedTile % GameLevelColumns) > 
				(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns)	)
			{
				temp_tile++;
			}
			else
			{
				temp_tile--;
			}
			
			if(GameWaypointCheckExisting(ptrPlayer, temp_tile) == true)
            {
				return false;	// Tile is already included in the list of temporary tiles?
            }
		}
		
		while( (y_diff--) > 0)
		{
			if( (ptrPlayer->SelectedTile / GameLevelColumns) > 
				(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns)	)
			{
				temp_tile += GameLevelColumns;
			}
			else
			{
				temp_tile -= GameLevelColumns;
			}
			
			if(GameWaypointCheckExisting(ptrPlayer, temp_tile) == true)
            {
				return false;	// Tile is already included in the list of temporary tiles?
            }
		}
	}
	else
	{		
		while( (y_diff--) > 0)
		{
			if( (ptrPlayer->SelectedTile / GameLevelColumns) > 
				(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] / GameLevelColumns)	)
			{
				temp_tile += GameLevelColumns;
			}
			else
			{
				temp_tile -= GameLevelColumns;
			}

            if(GameWaypointCheckExisting(ptrPlayer, temp_tile) == true)
            {
				return false;	// Tile is already included in the list of temporary tiles?
            }
		}
		
		while( (x_diff--) > 0)
		{
			if( (ptrPlayer->SelectedTile % GameLevelColumns) > 
				(ptrPlayer->Waypoints[ptrPlayer->LastWaypointIdx] % GameLevelColumns)	)
			{
				temp_tile++;
			}
			else
			{
				temp_tile--;
			}
			
			if(GameWaypointCheckExisting(ptrPlayer, temp_tile) == true)
            {
				return false;	// Tile is already included in the list of temporary tiles?
            }
		}
	}
	
	// Now at this point, we have prepared our array.
	
	for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
	{
		if(ptrPlayer->Waypoints[i] == 0)
		{
			// We have found empty waypoints. Exit loop
			break;
		}
		
		if(SystemContains_u8(	GameLevelBuffer[ptrPlayer->Waypoints[i]],
								AcceptedTiles,
								sizeof(AcceptedTiles) ) == false)
		{
			// Now try again with mirrored tiles, just in case!
		
			for(j = 0; j < (sizeof(AcceptedTiles) * sizeof(uint8_t) ); j++)
			{
				AcceptedTiles[j] |= TILE_MIRROR_FLAG;
			}
			
			if(SystemContains_u8(	GameLevelBuffer[ptrPlayer->Waypoints[i]],
									AcceptedTiles,
									sizeof(AcceptedTiles) ) == false)
			{
				// Both cases have failed. Return from function.
				return false;
			}
			
			// Reverse mirror flag.
			
			for(j = 0; j < (sizeof(AcceptedTiles) * sizeof(uint8_t) ); j++)
			{
				AcceptedTiles[j] &= ~(TILE_MIRROR_FLAG);
			}
		}
	}
	
	return true;
}

/* ****************************************************************************************
 * 
 * @name: TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Moves player camera position to selected aircraft.
 * 
 * @return:
 *  Isometric position of selected aircraft.
 * 
 * ****************************************************************************************/

TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* ptrPlayer)
{
	uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
	TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(AircraftIdx);
	
	CameraMoveToIsoPos(ptrPlayer, IsoPos);
	
	return IsoPos;
}

/* ********************************************************************************
 * 
 * @name: void GameSelectAircraftWaypoint(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 *
 * @brief:
 *  Moves player camera to selected aircraft and adds first waypoint.
 * 
 * ********************************************************************************/

void GameSelectAircraftWaypoint(TYPE_PLAYER* ptrPlayer)
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
 *  declaring TwoPlayersActive as a global variable.
 * 
 * ********************************************************************************/

bool GameTwoPlayersActive(void)
{
	return TwoPlayersActive;
}

/* *****************************************************************
 * 
 * @name: void GameDrawMouse(TYPE_PLAYER* ptrPlayer)
 * 
 * @author: Xavier Del Campo
 *
 * @param:
 *  TYPE_PLAYER* ptrPlayer:
 *      Pointer to a player structure
 * 
 * @brief:
 *  Draws GameMouseSpr under determined player states.
 * 
 * *****************************************************************/

void GameDrawMouse(TYPE_PLAYER* ptrPlayer)
{
	if(	(ptrPlayer->SelectTaxiwayParking == true)
						||
		(ptrPlayer->SelectTaxiwayRunway == true) 	)
	{
		GfxSortSprite(&GameMouseSpr);
	}
}

FL_STATE GameGetFlightDataStateFromIdx(uint8_t FlightDataIdx)
{
	if(FlightDataIdx >= FlightData.nAircraft)
	{
		return STATE_IDLE; // Error: could cause buffer overrun
	}

	return FlightData.State[FlightDataIdx];
}

uint32_t GameGetScore(void)
{
	return GameScore;
}

void GameStateUnboarding(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{	
	if(ptrPlayer->Unboarding == true)
	{
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_CIRCLE) == true)
		{
			ptrPlayer->Unboarding = false;
			ptrPlayer->UnboardingSequenceIdx = 0;	// Player will need to repeat sequence
													// if he/she decides to leave without finishing
		}

		ptrPlayer->LockTarget = true;
		ptrPlayer->LockedAircraft = ptrPlayer->FlightDataSelectedAircraft;

		if(ptrPlayer->PadLastKeySinglePressed_Callback() == ptrPlayer->UnboardingSequence[ptrPlayer->UnboardingSequenceIdx])
		{
			if(++ptrPlayer->UnboardingSequenceIdx >= UNBOARDING_KEY_SEQUENCE_MEDIUM)
			{
				if(ptrFlightData->Passengers[ptrPlayer->FlightDataSelectedAircraft] > UNBOARDING_PASSENGERS_PER_SEQUENCE)
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
		}
		else if(ptrPlayer->PadLastKeySinglePressed_Callback() != 0)
		{
			ptrPlayer->UnboardingSequenceIdx = 0; // Player has committed a mistake while entering the sequence. Repeat it!
		}
	}
}

void GameGenerateUnboardingSequence(TYPE_PLAYER* ptrPlayer)
{
	uint8_t i;
	unsigned short keyTable[] = { PAD_CROSS, PAD_SQUARE, PAD_TRIANGLE };

	memset(ptrPlayer->UnboardingSequence, 0, sizeof(ptrPlayer->UnboardingSequence) );

	ptrPlayer->UnboardingSequenceIdx = 0;

	Serial_printf("Key sequence generated: ");
	
	// Only medium level implemented. TODO: Implement other levels
	for(i = 0; i < UNBOARDING_KEY_SEQUENCE_MEDIUM; i++)
	{
		uint8_t randIdx = SystemRand(0, (sizeof(keyTable) / sizeof(keyTable[0])) - 1);
		
		ptrPlayer->UnboardingSequence[i] = keyTable[randIdx];

		Serial_printf("idx = %d, 0x%04X ", randIdx, ptrPlayer->UnboardingSequence[i]);
	}

	Serial_printf("\n");
}

void GameCreateTakeoffWaypoints(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData, uint8_t aircraftIdx)
{
	// Look for aircraft direction by searching TILE_RWY_EXIT
	//uint16_t currentTile = AircraftGetTileFromFlightDataIndex(aircraftIdx);
	//uint8_t targetsIdx = 0;
	AIRCRAFT_DIRECTION aircraftDir = AircraftGetDirection(AircraftFromFlightDataIndex(aircraftIdx));
	int8_t rwyStep = 0;
	uint16_t currentTile = 0;
	uint16_t targets[AIRCRAFT_MAX_TARGETS] = {0};
	uint8_t i;

	switch(aircraftDir)
	{
		case AIRCRAFT_DIR_EAST:
			Serial_printf("EAST\n");
			rwyStep = 1;
		break;

		case AIRCRAFT_DIR_WEST:
			Serial_printf("WEST\n");
			rwyStep = -1;
		break;

		case AIRCRAFT_DIR_NORTH:
			Serial_printf("NORTH\n");
			rwyStep = -GameLevelColumns;
		break;

		case AIRCRAFT_DIR_SOUTH:
			Serial_printf("SOUTH\n");
			rwyStep = GameLevelColumns;
		break;

		default:
		return;
	}

	for(currentTile = (AircraftGetTileFromFlightDataIndex(aircraftIdx) + rwyStep);
		(GameLevelBuffer[currentTile] != TILE_RWY_START_1)
							&&
		(GameLevelBuffer[currentTile] != TILE_RWY_START_2);
		currentTile -= rwyStep	)
	{
		
	}

	for(i = 0; i < GAME_MAX_RUNWAYS; i++)
	{
		if(GameUsedRwy[i] == currentTile)
		{
			GameUsedRwy[i] = 0;
			break;
		}
	}

	for(currentTile = (AircraftGetTileFromFlightDataIndex(aircraftIdx) + rwyStep);
		GameLevelBuffer[currentTile] != TILE_RWY_EXIT;
		currentTile += rwyStep	)
	{
		
	}

	targets[0] = currentTile;

	AircraftAddTargets(AircraftFromFlightDataIndex(aircraftIdx), targets);
}

void GameGetRunwayEntryTile(uint8_t aircraftIdx, TYPE_RWY_ENTRY_DATA* ptrRwyEntry)
{
	// Look for aircraft direction by searching TILE_RWY_EXIT
	uint16_t currentTile = AircraftGetTileFromFlightDataIndex(aircraftIdx);
	int16_t step = 0;
	uint16_t i;

	if( (currentTile >= GameLevelColumns)
						&&
		( (currentTile + GameLevelColumns) < GameLevelSize) )
	{
		if(GameLevelBuffer[currentTile + 1] == TILE_RWY_EXIT)
		{
			ptrRwyEntry->Direction = AIRCRAFT_DIR_EAST;
			ptrRwyEntry->rwyStep = GameLevelColumns;
			step = -1;
		}
		else if(GameLevelBuffer[currentTile - 1] == TILE_RWY_EXIT)
		{
			ptrRwyEntry->Direction = AIRCRAFT_DIR_WEST;
			ptrRwyEntry->rwyStep = -GameLevelColumns;
			step = 1;
		}
		else if(GameLevelBuffer[currentTile + GameLevelColumns] == TILE_RWY_EXIT)
		{
			ptrRwyEntry->Direction = AIRCRAFT_DIR_SOUTH;
			ptrRwyEntry->rwyStep = 1;
			step = GameLevelColumns;
		}
		else if(GameLevelBuffer[currentTile - GameLevelColumns] == TILE_RWY_EXIT)
		{
			ptrRwyEntry->Direction = AIRCRAFT_DIR_NORTH;
			ptrRwyEntry->rwyStep = -1;
			step = -GameLevelColumns;
		}
		else
		{
			ptrRwyEntry->rwyEntryTile = 0;
			ptrRwyEntry->Direction = AIRCRAFT_DIR_NO_DIRECTION;
			ptrRwyEntry->rwyStep = 0;
			Serial_printf("GameCreateTakeoffWaypoints(): could not determine aircraft direction.\n");
			return;
		}

		ptrRwyEntry->rwyEntryTile = currentTile + step;

		i = ptrRwyEntry->rwyEntryTile;

		while(	(GameLevelBuffer[i] != TILE_RWY_START_1)
								&&
				(GameLevelBuffer[i] != TILE_RWY_START_2)
								&&
				(i > ptrRwyEntry->rwyStep)
								&&
				((i - ptrRwyEntry->rwyStep) < GameLevelSize ) )
		{
			i -= ptrRwyEntry->rwyStep;
		}

		ptrRwyEntry->rwyHeader = i;

		DEBUG_PRINT_VAR(ptrRwyEntry->rwyHeader);
	}
	else
	{
		Serial_printf("GameCreateTakeoffWaypoints(): Invalid index for tile.\n");
	}
}

bool GameInsideLevelFromIsoPos(TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos)
{
	short x = (short)fix16_to_int(ptrIsoPos->x);
	short y = (short)fix16_to_int(ptrIsoPos->y);

	if(x < 0)
	{
		return true;
	}

	if(x > (GameLevelColumns << TILE_SIZE_BIT_SHIFT))
	{
		return true;
	}

	if(y < 0)
	{
		return true;
	}

	if(y > (GameLevelColumns << TILE_SIZE_BIT_SHIFT) )
	{
		return true;
	}

	return false;
}

void GameRemoveFlight(uint8_t idx, bool successful)
{
	uint8_t i;

	for(i = PLAYER_ONE; i < MAX_PLAYERS; i++)
	{
		TYPE_PLAYER* ptrPlayer = &PlayerData[i];
		uint8_t j;

        if(ptrPlayer->Active == false)
        {
            continue;
        }

        if(idx >= FlightData.nAircraft)
        {
            Serial_printf("GameRemoveFlight: index %d exceeds max index %d!\n", idx, FlightData.nAircraft);
            return;
        }

        if((FlightData.FlightDirection[idx] & ptrPlayer->FlightDirection) == 0)
        {
            continue;
        }

		for(j = 0; j < ptrPlayer->ActiveAircraft; j++)
		{
			if(ptrPlayer->ActiveAircraftList[j] == idx)
            {
                if(FlightData.State[idx] != STATE_IDLE)
                {
                    uint8_t k;

					memset(ptrPlayer->UnboardingSequence, 0, GAME_MAX_SEQUENCE_KEYS);
					ptrPlayer->UnboardingSequenceIdx = 0;

					for(k = 0; k < GAME_MAX_RUNWAYS; k++)
					{
						uint16_t* targets = AircraftGetTargets(idx);
                        uint16_t rwyArray[GAME_MAX_RWY_LENGTH] = {0};

						if(SystemContains_u16(GameUsedRwy[k], targets, AIRCRAFT_MAX_TARGETS) == true)
						{
							GameUsedRwy[k] = 0;
						}
                        else
                        {
                            Serial_printf("1\n");

                            // GameRwyArray is filled with runway tiles.
                            GameGetSelectedRunwayArray(GameUsedRwy[k], rwyArray, GAME_MAX_RWY_LENGTH * sizeof(uint16_t) );

                            Serial_printf("2\n");

                            if(SystemContains_u16(  AircraftGetTileFromFlightDataIndex(idx),
                                                    rwyArray,
                                                    sizeof(rwyArray) / sizeof(rwyArray[0]) ) == true)
                            {
                                GameUsedRwy[k] = 0;
                            }
                        }
					}

					if(FlightData.State[idx] != STATE_APPROACH)
					{
						if(AircraftRemove(idx) == false)
						{
							Serial_printf("Something went wrong when removing aircraft!\n");
                            return;
						}
					}
                    else
                    {
                        // STATE_APPROACH is the only state which is not linked to a TYPE_AIRCRAFT_DATA instance.
                    }

					ptrPlayer->LockTarget = false;
					ptrPlayer->LockedAircraft = FLIGHT_DATA_INVALID_IDX;

					if(successful == true)
					{
						GameScore += SCORE_REWARD_FINISH_FLIGHT;
					}
					else
					{
						GameScore = (GameScore < LOST_FLIGHT_PENALTY)? 0 : (GameScore - LOST_FLIGHT_PENALTY);
					}

					if(ptrPlayer->SelectedAircraft >= j)
					{
						ptrPlayer->SelectedAircraft--;
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

void GameActiveAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t i;
	uint8_t j = 0;

	uint8_t currentFlightDataIdx;
	uint8_t lastFlightDataIdx;
	
	// Clear all pointers for aircraft data first.
	// Then, rebuild aircraft list for player.
	
	lastFlightDataIdx = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

	memset(ptrPlayer->ActiveAircraftList, 0, GAME_MAX_AIRCRAFT);
	ptrPlayer->ActiveAircraft = 0;
	
	for(i = 0; i < FlightData.nAircraft; i++)
	{
		if(	(ptrFlightData->State[i] != STATE_IDLE)
							&&
			(ptrFlightData->FlightDirection[i] & ptrPlayer->FlightDirection) )
		{
			ptrPlayer->ActiveAircraftList[j++] = i;
			ptrPlayer->ActiveAircraft++;
		}
	}

	currentFlightDataIdx = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

	if(GameAircraftCreatedFlag == true)
	{
		GameAircraftCreatedFlag = false;

		if(ptrPlayer->ActiveAircraft > 1)
		{
			if(currentFlightDataIdx != lastFlightDataIdx)
			{	
				for(ptrPlayer->SelectedAircraft = 0; ptrPlayer->SelectedAircraft < FlightData.nAircraft; ptrPlayer->SelectedAircraft++)
				{
					if(ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft] == lastFlightDataIdx)
					{
						break;
					}
				}
			}
		}
	}
}

void GameRemainingAircraft(uint8_t i)
{
    // Reset iterator when starting from first element.

    if(i == 0)
    {
        FlightData.nRemainingAircraft = FlightData.nAircraft;
    }
	
    if(FlightData.Finished[i] == true)
    {
        FlightData.nRemainingAircraft--;
    }
}

void GameFinished(uint8_t i)
{
	/*uint8_t i;

	for(i = 0; i < FlightData.nAircraft; i++)
	{
		if(FlightData.Finished[i] == false)
		{
			// At least one aircraft still not finished
			return false;
		}
	}*/

    if(i == 0)
    {
        GameFinishedFlag = true;
    }

    if(FlightData.Finished[i] == false)
    {
        GameFinishedFlag = false;
    }
}

void GameMinimumSpawnTimeout(void)
{
	spawnMinTimeFlag = false;
}

void GameAircraftCollision(uint8_t AircraftIdx)
{
	GameAircraftCollisionFlag = true;
	GameAircraftCollisionIdx = AircraftIdx;
}

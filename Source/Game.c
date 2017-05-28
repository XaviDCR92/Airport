/* *************************************
 * 	Includes
 * *************************************/

#include "Game.h"

/* *************************************
 * 	Defines
 * *************************************/

#define GAME_MAX_MAP_SIZE 0x400
#define GAME_MAX_RUNWAYS 16
#define GAME_MAX_RWY_LENGTH 16

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

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	TILE_GRASS = 0,
	TILE_ASPHALT_WITH_BORDERS,
	TILE_WATER,
	TILE_ASPHALT,
	TILE_HANGAR,
	TILE_ILS,
	TILE_ATC_TOWER,
	TILE_ATC_LOC,
	TILE_RWY_MID,
	TILE_RWY_START_1,
	TILE_RWY_START_2,
	TILE_PARKING,
	TILE_RESERVED_1,
	TILE_TAXIWAY_INTERSECT_GRASS,
	TILE_TAXIWAY_GRASS,
	TILE_TAXIWAY_CORNER_GRASS,
	
	LAST_TILE_TILESET1 = TILE_TAXIWAY_CORNER_GRASS
};

enum
{
	TILE_HALF_WATER_1 = LAST_TILE_TILESET1 + 1,
	TILE_HALF_WATER_2,
	TILE_RESERVED_2,
	TILE_RESERVED_3,
	TILE_AIRPORT_BUILDING,
	TILE_PLANE,
	TILE_RWY_HOLDING_POINT,
	TILE_RWY_HOLDING_POINT_2,
	TILE_RWY_EXIT,
	TILE_GATE,
	TILE_RESERVED_6,
	TILE_RESERVED_7,
	TILE_TAXIWAY_CORNER_GRASS_2,
	
	LAST_TILE_TILESET2 = TILE_TAXIWAY_CORNER_GRASS_2,
	
	TILE_NOTHING = 0xFF
};

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
	UNBOARDING_PASSENGERS_PER_SEQUENCE = 25
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void GameInit(void);
static void GameLoadLevel(void);
static bool GamePause(void);
static void GameEmergencyMode(void);
static void GameCalculations(void);
static void GamePlayerHandler(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer);
static void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* ptrPlayer, uint16_t tile);
static void GameGraphics(void);
static void GameRenderLevel(TYPE_PLAYER* ptrPlayer);
//static void GameLoadPilots(char * strPath);
static void GameClock(void);
static void GameClockFlights(void);
static void GameAircraftState(void);
static void GameActiveAircraft(void);
static void GameStateShowAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameFirstLastAircraftIndex(void);
static void GameSelectAircraftFromList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectTaxiwayRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectTaxiwayParking(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateLockTarget(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* ptrPlayer);
static void GameSelectAircraftWaypoint(TYPE_PLAYER* ptrPlayer);
static void GameGetRunwayArray(void);
static void GameGetSelectedRunwayArray(uint16_t rwyHeader);
static void GameAssignRunwaytoAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static bool GamePathToTile(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameDrawMouse(TYPE_PLAYER* ptrPlayer);
static void GameStateUnboarding(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData);
static void GameGenerateUnboardingSequence(TYPE_PLAYER* ptrPlayer);
static void GameCreateTakeoffWaypoints(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData, uint8_t aircraftIdx);

/* *************************************
 * 	Global Variables
 * *************************************/

bool GameStartupFlag;
uint32_t GameScore;

/* *************************************
 * 	Local Variables
 * *************************************/

static GsSprite GameTilesetSpr;
static GsSprite GameTileset2Spr;
static GsSprite GamePlaneSpr;
static GsSprite GameMouseSpr;
static uint16_t GameRwy[GAME_MAX_RUNWAYS];
static TYPE_FLIGHT_DATA FlightData;
static uint16_t GameRwyArray[GAME_MAX_RWY_LENGTH];
static uint16_t GameUsedRwy[GAME_MAX_RUNWAYS];
static uint16_t GameSelectedTile;
static bool firstLevelRender; // Used to avoid reentrance on GameRenderLevel()

// Instances for player-specific data
TYPE_PLAYER PlayerData[MAX_PLAYERS];

static char * GameFileList[] = {	"cdrom:\\DATA\\SPRITES\\TILESET1.TIM;1"	,
									"cdrom:\\DATA\\SPRITES\\TILESET2.TIM;1"	,
									"cdrom:\\DATA\\LEVELS\\LEVEL1.PLT;1"	,
									"cdrom:\\DATA\\SPRITES\\GAMEPLN.TIM;1"	,
									"cdrom:\\DATA\\SPRITES\\PLNBLUE.CLT;1"	,
									"cdrom:\\DATA\\SPRITES\\MOUSE.TIM;1"	};
									
static void * GameFileDest[] = {	(GsSprite*)&GameTilesetSpr		,
									(GsSprite*)&GameTileset2Spr		,
									(TYPE_FLIGHT_DATA*)&FlightData	,
									(GsSprite*)&GamePlaneSpr		,
									NULL							,
									(GsSprite*)&GameMouseSpr		};
									
static char * GameLevelList[] = {	"cdrom:\\DATA\\LEVELS\\LEVEL1.LVL;1"};
static uint8_t GameLevelBuffer[GAME_MAX_MAP_SIZE];
									
static uint8_t GameLevelColumns;
static uint8_t GameLevelSize;

static char GameLevelTitle[LEVEL_TITLE_SIZE];

//Game local time
static uint8_t GameHour;
static uint8_t GameMinutes;

//Local flag for two-player game mode. Obtained from Menu
static bool TwoPlayersActive;
//Index for first non-idle aircraft on list
static uint8_t firstActiveAircraft;
//Index for last non-idle aircraft on list
static uint8_t lastActiveAircraft;

void Game(bool two_players)
{	
	TwoPlayersActive = two_players;
	GameInit();
	
	while(1)
	{
		if(GamePause() == true)
		{
			// Exit game
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
			//dprintf("Released callback = 0x%08X\n", ptrPlayer->PadKeySinglePress_Callback);
			if(ptrPlayer->PadKeySinglePress_Callback(PAD_START) == true)
			{
				dprintf("Player %d set pause_flag to true!\n",i);
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

void GameInit(void)
{
	uint8_t i;
	uint32_t track;
	
	GameStartupFlag = true;
	
	LoadMenu(	GameFileList,
				GameFileDest,
				sizeof(GameFileList) / sizeof(char*),
				sizeof(GameFileDest) /sizeof(void*)	);	
	
	GameLoadLevel();
	
	GameGuiInit();
	
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
	
	firstActiveAircraft = 0;
	lastActiveAircraft = 0;
	
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

	GameScore = 0;
	
	GameGetRunwayArray();
	
	GameSelectedTile = 0;
	
	AircraftInit();
				
	LoadMenuEnd();
	
	GfxSetGlobalLuminance(0);
	
	track = SystemRand(GAMEPLAY_FIRST_TRACK,GAMEPLAY_LAST_TRACK);
	
	SfxPlayTrack(track);	
}

void GameEmergencyMode(void)
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
	
	GsRectangle errorRct;
	
	bzero((GsRectangle*)&errorRct, sizeof(GsRectangle));
	
	while(SystemGetEmergencyMode() == true)
	{
		// Pad one has been disconnected during gameplay
		// Show an error screen until it is disconnected again.
		
		GsSortCls(0,0,0);
		
		errorRct.x = ERROR_RECT_X;
		errorRct.w = ERROR_RECT_W;
		errorRct.y = ERROR_RECT_Y;
		errorRct.h = ERROR_RECT_H;
		
		errorRct.r = ERROR_RECT_R;
		errorRct.g = ERROR_RECT_G;
		errorRct.b = ERROR_RECT_B;
		
		GsSortRectangle(&errorRct);
		GfxDrawScene();
	}
}

void GameCalculations(void)
{
	uint8_t i;
	
	GameClock();
	GameAircraftState();
	GameActiveAircraft();
	GameFirstLastAircraftIndex();
	GameGuiCalculateSlowScore();
	AircraftHandler();
	
	for(i = 0 ; i < MAX_PLAYERS ; i++)
	{
		// Run player-specific functions for each player
		if(PlayerData[i].Active == true)
		{
			GamePlayerHandler(&PlayerData[i], &FlightData);
		}
	}
	
	/*if(PadOneKeyReleased(PAD_CIRCLE) == true)
	{
		for(i = 0; i < FlightData.nAircraft ; i++)
		{	
			dprintf("\n*****************\n");
			dprintf("\tAIRCRAFT %d\n",i);
			dprintf("*****************\n");
			
			if(FlightData.FlightDirection[i] == ARRIVAL)
			{
				dprintf("Direction: %s\n","Arrival");
			}
			else if(FlightData.FlightDirection[i] == DEPARTURE)
			{
				dprintf("Direction: %s\n","Arrival");
			}
			
			dprintf("Time: %d:%d\n",FlightData.Hours[i],FlightData.Minutes[i]);
			
			dprintf("State: ");
			
			switch(FlightData.State[i])
			{
				case STATE_APPROACH:
					dprintf("Approach");
				break;
				
				case STATE_TAXIING:
					dprintf("Taxiing");
				break;
				
				case STATE_FINAL:
					dprintf("Final");
				break;
				
				case STATE_IDLE:
					dprintf("Idle");
				break;
				
				case STATE_PARKED:
					dprintf("Parked");
				break;
				
				case STATE_LANDED:
					dprintf("Landed");
				break;
				
				case STATE_TAKEOFF:
					dprintf("Takeoff");
				break;
				
				default:
				break;
			}
			dprintf("\n");
		}
		
		dprintf("Active aircraft: %d\n",FlightData.ActiveAircraft);
	}*/
	
}

void GameFirstLastAircraftIndex(void)
{
	bool first_set = false;
	uint8_t i;
	
	for(i = 0; i < GAME_MAX_AIRCRAFT ; i++)
	{
		if(FlightData.State[i] != STATE_IDLE)
		{
			if(first_set == false)
			{
				firstActiveAircraft = i;
				first_set = true;
			}
			lastActiveAircraft = i;
		}
	}
}

uint8_t GameGetFirstActiveAircraft(void)
{
	return firstActiveAircraft;
}

uint8_t GameGetLastActiveAircraft(void)
{
	return lastActiveAircraft;
}

void GamePlayerHandler(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	ptrPlayer->SelectedTile = 0;	// Reset selected tile if no states
									// which use this are currently active.
	ptrPlayer->InvalidPath = false; // Do the same thing for "InvalidPath".

	ptrPlayer->FlightDataSelectedAircraft = ptrPlayer->ActiveAircraftList[ptrPlayer->SelectedAircraft];

	GameStateUnboarding(ptrPlayer, ptrFlightData);
	GameStateLockTarget(ptrPlayer, ptrFlightData);
	GameStateSelectRunway(ptrPlayer, ptrFlightData);
	GameStateSelectTaxiwayRunway(ptrPlayer, ptrFlightData);
	GameStateSelectTaxiwayParking(ptrPlayer, ptrFlightData);
	GameStateShowAircraft(ptrPlayer, ptrFlightData);
	CameraHandler(ptrPlayer);
	GameGuiActiveAircraftList(ptrPlayer, ptrFlightData);
	GameGuiActiveAircraftPage(ptrPlayer, ptrFlightData);
	GameSelectAircraftFromList(ptrPlayer, ptrFlightData);
}

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
	
	GameClockFlights();
}

void GameClockFlights(void)
{
	uint8_t i;
	
	for(i = 0; i < FlightData.nAircraft ; i++)
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
}

void GameGraphics(void)
{
	int i;
	bool split_screen = false;
	
	while(GfxIsGPUBusy() == true);
			
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
		if(PlayerData[i].Active == true)
		{	
			if(split_screen == true)
			{
				GfxSetSplitScreen(i);
			}
			
			// Draw half split screen for each player
			// only if 2-player mode is active. Else, render
			// the whole screen as usual.
			
			GsSortCls(0,0,GfxGetGlobalLuminance() >> 1);

			GameRenderLevel(&PlayerData[i]);

			AircraftRender(&PlayerData[i]);

			GameGuiAircraftList(&PlayerData[i], &FlightData);
			
			GameDrawMouse(&PlayerData[i]);

			GameGuiDrawUnboardingSequence(&PlayerData[i]);
		}
	}
	
	// Avoid changing drawing environment twice on 1-player mode
	// as it doesn't make any sense.
	if(split_screen == true)
	{
		GfxDisableSplitScreen();
	}
	
	// Draw common elements for both players (messages, clock...)
	
	GameGuiAircraftNotificationRequest(&FlightData);
	
	GameGuiBubble(&FlightData);
	
	GameGuiClock(GameHour,GameMinutes);

	GameGuiShowScore();
	
	GfxDrawScene();
}

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
	
	dprintf("Level header: %s\n",LevelHeader);
	
	if(strncmp(LevelHeader,LEVEL_MAGIC_NUMBER_STRING,LEVEL_MAGIC_NUMBER_SIZE) != 0)
	{
		dprintf("Invalid level header! Read \"%s\" instead of \"ATC\"\n",LevelHeader);
		return;
	}
	
	i += LEVEL_MAGIC_NUMBER_SIZE;
	
	GameLevelColumns = ptrBuffer[i++];
	
	dprintf("Level size: %d\n",GameLevelColumns);
	
	if(	(GameLevelColumns < MIN_MAP_COLUMNS)
			||
		(GameLevelColumns > MAX_MAP_COLUMNS)	)
	{
		dprintf("Invalid map size! Value: %d\n",GameLevelColumns);	
		return;
	}
	
	GameLevelSize = GameLevelColumns * GameLevelColumns;
	
	memset(GameLevelTitle,0,LEVEL_TITLE_SIZE);
	
	memcpy(GameLevelTitle,&ptrBuffer[i],LEVEL_TITLE_SIZE);
	
	dprintf("Game level title: %s\n",GameLevelTitle);
	
	i += LEVEL_TITLE_SIZE;
	
	memset(GameLevelBuffer,0,GAME_MAX_MAP_SIZE);
	
	i = LEVEL_HEADER_SIZE;
	
	memcpy(GameLevelBuffer,&ptrBuffer[i],GameLevelSize);
	
}

char* GetGameLevelTitle(void)
{
	return GameLevelTitle;
}

void GameAircraftState(void)
{
	uint8_t i;
	uint16_t target[2] = {0};
	// Arrays are copied to AircraftAddNew, so we create a first and only
	// target which is the parking tile itself, and the second element
	// is just the NULL character.
	// Not an ideal solution, but the best one currently available.
	
	for(i = 0; i < FlightData.nAircraft ; i++)
	{
		if(FlightData.Finished[i] == false)
		{
			if(	(FlightData.Hours[i] == 0)
						&&
				(FlightData.Minutes[i] == 0)
						&&
				(FlightData.State[i] == STATE_IDLE)
						&&
				(FlightData.RemainingTime[i] > 0)	)
			{
				if(FlightData.FlightDirection[i] == DEPARTURE)
				{
					FlightData.State[i] = STATE_PARKED;
					
					target[0] = FlightData.Parking[i];
					
					dprintf("Target assigned = %d", target[0]);
					
					if(AircraftAddNew(&FlightData, i, target) == false)
					{
						dprintf("Exceeded maximum aircraft number!\n");
						return;
					}
				}
				else if(FlightData.FlightDirection[i] == ARRIVAL)
				{
					FlightData.State[i] = STATE_APPROACH;
				}
				
				// Create notification request for incoming aircraft
				FlightData.NotificationRequest[i] = true;
			}

			if( (FlightData.State[i] != STATE_IDLE)
								&&
				(FlightData.RemainingTime[i] == 0)	)
			{
				// Player(s) lost a flight!
				FlightData.State[i] = STATE_IDLE;
				GameScore = (GameScore < LOST_FLIGHT_PENALTY)? 0 : (GameScore - LOST_FLIGHT_PENALTY);
			}
		}
	}
}

void GameRenderLevel(TYPE_PLAYER* ptrPlayer)
{
	uint16_t i;
	uint16_t j;
	uint8_t columns = 0;
	uint8_t rows = 0;
	bool flip_id;
	bool used_rwy;
	uint8_t aux_id;
	GsSprite * ptrTileset;
	const uint8_t rwy_sine_step = 24;
	static unsigned char rwy_sine = rwy_sine_step;
	static bool rwy_sine_decrease = false;
	TYPE_ISOMETRIC_POS tileIsoPos;
	TYPE_CARTESIAN_POS tileCartPos;
	
	// Prepare runway to be painted in blue if player is on runway selection mode
	if(ptrPlayer->SelectRunway == true)
	{
		GameGetSelectedRunwayArray(GameRwy[ptrPlayer->SelectedRunway]);
		/*dprintf("Runway array:\n");
		
		for(j = 0; j < GAME_MAX_RWY_LENGTH; j++)
		{
			dprintf("%d ",GameRwyArray[j]);
		}
		
		dprintf("\n");*/
	}

	if(firstLevelRender == true)
	{
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
		// Flipped tiles have bit 7 enabled
		if(GameLevelBuffer[i] & TILE_MIRROR_FLAG)
		{
			flip_id = true;
			aux_id = GameLevelBuffer[i];
			GameLevelBuffer[i] &= ~(TILE_MIRROR_FLAG);
		}
		else
		{
			flip_id = false;
		}
		
		if(GameLevelBuffer[i] == TILE_NOTHING)
		{
			// Skip empty tiles
			continue;
		}
		
		if(GameLevelBuffer[i] <= LAST_TILE_TILESET1)
		{
			// Draw using GameTilesetSpr
			ptrTileset = &GameTilesetSpr;
		}
		else if(	(GameLevelBuffer[i] > LAST_TILE_TILESET1)
						&&
					(GameLevelBuffer[i] <= LAST_TILE_TILESET2)	)
		{
			// Draw using GameTileset2Spr
			ptrTileset = &GameTileset2Spr;
		}
		else
		{
			ptrTileset = NULL;
			
			if(flip_id == false)
			{
				continue;
			}
		}
		
		ptrTileset->w = TILE_SIZE;
		ptrTileset->h = TILE_SIZE;
		
		used_rwy = false;
		
		if(	(ptrPlayer->SelectRunway == true)
						&&
			(i != 0)
						&&
			(SystemContains_u16(i, GameRwyArray, GAME_MAX_RWY_LENGTH) == true)	)
		{
			for(j = 0; j < GAME_MAX_RUNWAYS; j++)
			{
				if(GameUsedRwy[j] != 0)
				{
					if(SystemContains_u16(GameUsedRwy[j], GameRwyArray, GAME_MAX_RWY_LENGTH) == true)
					{
						used_rwy = true;
						break;
					}
				}
			}
			
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
				 (i != 0)
								&&
				 (	(SystemContains_u16(i, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == true)
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
		else
		{
			ptrTileset->r = NORMAL_LUMINANCE;
			ptrTileset->g = NORMAL_LUMINANCE;
			ptrTileset->b = NORMAL_LUMINANCE;
		}
		
		// TODO: Isometric -> Cartesian conversion
		tileIsoPos.x = columns << (TILE_SIZE_BIT_SHIFT);
		tileIsoPos.y = rows << (TILE_SIZE_BIT_SHIFT);
		tileIsoPos.z = 0;
		
		tileCartPos = GfxIsometricToCartesian(&tileIsoPos);
		
		ptrTileset->x = tileCartPos.x;
		ptrTileset->y = tileCartPos.y;
		
		// Set coordinate origin to left upper corner
		ptrTileset->x -= TILE_SIZE >> 1;
		ptrTileset->y -= TILE_SIZE >> 2;
		
		/*ptrTileset->x = columns << (TILE_SIZE_BIT_SHIFT - 1);
		ptrTileset->x -= rows << (TILE_SIZE_BIT_SHIFT - 1);
		
		ptrTileset->y = rows << (TILE_SIZE_BIT_SHIFT - 2);
		ptrTileset->y += columns << (TILE_SIZE_BIT_SHIFT - 2);*/
		
		if(columns < GameLevelColumns -1 )
		{
			columns++;
		}
		else
		{
			rows++;
			columns = 0;
		}
		
		if(ptrTileset != NULL)
		{
			if(flip_id == true)
			{
				ptrTileset->attribute |= H_FLIP;
			}
		}
		
		ptrTileset->u = (short)(GameLevelBuffer[i] % COLUMNS_PER_TILESET)<<TILE_SIZE_BIT_SHIFT;
		ptrTileset->v = (short)(GameLevelBuffer[i] / COLUMNS_PER_TILESET)<<TILE_SIZE_BIT_SHIFT;
			
		if(flip_id == true)
		{
			flip_id = false;
			GameLevelBuffer[i] = aux_id;
		}
		
	//	dprintf("Tile %d, attribute 0x%X\n",i,ptrTileset->attribute);
		
		CameraApplyCoordinatesToSprite(ptrPlayer, ptrTileset);
		GfxSortSprite(ptrTileset);
				
		if(ptrTileset->attribute & H_FLIP)
		{
			ptrTileset->attribute &= ~(H_FLIP);
		}
	}
	
	/*if(PadOneKeyReleased(PAD_CROSS) == true)
	{
		for(i = 0; i < GameLevelSize; i++)
		{
			dprintf("Tile number %d, ID: %d\n",i,GameLevelBuffer[i]);
		}
	}*/
}

void GameSetTime(uint8_t hour, uint8_t minutes)
{
	GameHour = hour;
	GameMinutes = minutes;
}

void GameActiveAircraft(void)
{
	uint8_t i;
	
	FlightData.ActiveAircraft = 0;
	
	for(i = 0 ; i < FlightData.nAircraft ; i++)
	{
		if(FlightData.State[i] != STATE_IDLE)
		{
			FlightData.ActiveAircraft++;
		}
	}
}

void GameStateShowAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
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

void GameStateLockTarget(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
	
	if(ptrPlayer->LockTarget == true)
	{
		CameraMoveToIsoPos(ptrPlayer, AircraftGetIsoPos(ptrPlayer->LockedAircraft) );
	}
	
	if(ptrPlayer->PadKeySinglePress_Callback(PAD_SQUARE) == true)
	{
		if(ptrPlayer->ShowAircraftData == true)
		{
			if(ptrPlayer->LockTarget == false)
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
		else if(ptrPlayer->LockTarget == true)
		{
			ptrPlayer->LockTarget = false;
			ptrPlayer->LockedAircraft = 0;
		}
	}
	else if(ptrPlayer->PadDirectionKeyPressed_Callback() == true)
	{
		if( (ptrPlayer->LockTarget == true)
						&&
			(ptrPlayer->ShowAircraftData == false) )
		{
			ptrPlayer->LockTarget = false;
			ptrPlayer->LockedAircraft = 0;
		}
	}
}

void GameStateSelectTaxiwayRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	TYPE_ISOMETRIC_POS IsoPos = CameraGetIsoPos(ptrPlayer);
	uint8_t i;
	uint16_t target_tile;
	
	/*dprintf("Camera is pointing to {%d,%d}\n",IsoPos.x, IsoPos.y);*/
	
	if(ptrPlayer->SelectTaxiwayRunway == true)
	{
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = 0;
		
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
						dprintf("Added these targets to aircraft %d:\n", ptrPlayer->FlightDataSelectedAircraft);
						
						for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
						{
							dprintf("%d ",ptrPlayer->Waypoints[i]);
						}
						
						dprintf("\n");
						
						// Clear waypoints array.
						memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);

						// Reset state and auxiliar variables
						ptrPlayer->WaypointIdx = 0;
						ptrPlayer->LastWaypointIdx = 0;
						ptrPlayer->LockedAircraft = 0;
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

void GameStateSelectTaxiwayParking(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	TYPE_ISOMETRIC_POS IsoPos = CameraGetIsoPos(ptrPlayer);
	uint8_t i;
	uint16_t target_tile;
	
	if(ptrPlayer->SelectTaxiwayParking == true)
	{
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = 0;
		
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
				
				dprintf("ptrPlayer->LastWaypointIdx = %d\n",
						ptrPlayer->LastWaypointIdx);
				
				dprintf("target_tile = %d, TILE_PARKING = %d\n",
						target_tile,
						TILE_PARKING);
				
				if(	(target_tile == TILE_PARKING)
								||
					(target_tile == (TILE_PARKING | TILE_MIRROR_FLAG) ) )
				{
					// TODO: Assign path to aircraft
					AircraftFromFlightDataIndexAddTargets(ptrPlayer->FlightDataSelectedAircraft, ptrPlayer->Waypoints);
					
					dprintf("Added these targets to aircraft %d:\n", ptrPlayer->FlightDataSelectedAircraft);
					
					for(i = 0; i < PLAYER_MAX_WAYPOINTS; i++)
					{
						dprintf("%d ",ptrPlayer->Waypoints[i]);
					}
					
					dprintf("\n");
					
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

void GameStateSelectRunway(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
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
		ptrPlayer->LockedAircraft = 0;
		
		CameraMoveToIsoPos(ptrPlayer, IsoPos);
		
		if(ptrPlayer->PadKeySinglePress_Callback(PAD_TRIANGLE) == true)
		{
			ptrPlayer->SelectRunway = false;
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_CROSS) == true)
		{
			ptrPlayer->SelectRunway = false;
			
			dprintf("ptrPlayer->SelectedRunway = %d\n", GameRwy[ptrPlayer->SelectedRunway]);
			if(SystemContains_u16(GameRwy[ptrPlayer->SelectedRunway], GameUsedRwy, GAME_MAX_RUNWAYS) == false)
			{
				ptrPlayer->SelectRunway = false;
				dprintf("Player selected runway %d!\n",GameRwy[ptrPlayer->SelectedRunway]);
				
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
					dprintf("No available runways!\n");
				}
			}
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_LEFT) == true)
		{
			if(ptrPlayer->SelectedRunway != 0)
			{
				ptrPlayer->SelectedRunway--;
			}
		}
		else if(ptrPlayer->PadKeySinglePress_Callback(PAD_RIGHT) == true)
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

void GameGetRunwayArray(void)
{
	uint8_t i;
	uint8_t j = 0;
	
	for(i = 0; i < GameLevelSize; i++)
	{
		if(GameLevelBuffer[i] == TILE_RWY_START_1)
		{
			if(SystemContains_u8(i, GameLevelBuffer, GAME_MAX_RUNWAYS) == false)
			{
				GameRwy[j++] = i;
			}
		}
	}
	
	dprintf("GameRwy = ");
	
	for(i = 0; i < GAME_MAX_RUNWAYS; i++)
	{
		if(GameRwy[i] == 0)
		{
			break;
		}
		
		dprintf("%d ", GameRwy[i]);
	}
	
	dprintf("\n");
}

void GameSelectAircraftFromList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
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
					
					default:
						dprintf("Incompatible state %d!\n",aircraftState);
						// States remain unchanged
						ptrPlayer->SelectRunway = false;
						ptrPlayer->SelectTaxiwayRunway = false;
						ptrPlayer->ShowAircraftData = true;
					break;
				}
			}
			
			dprintf("aircraftState = %d\n", aircraftState);
			dprintf("AircraftIdx = %d\n", AircraftIdx);
		}
	}
}

void GameGetSelectedRunwayArray(uint16_t rwyHeader)
{
	typedef enum t_rwydir
	{
		RWY_DIR_EAST	= 0,
		RWY_DIR_WEST,
		RWY_DIR_NORTH,
		RWY_DIR_SOUTH,
	}RWY_DIR;
	
	static uint16_t last_tile = 0;
	static uint8_t i = 0;
	static RWY_DIR dir;
	
	if(rwyHeader != 0)
	{
		// This function is called recursively.
		// Since 0 is not a valid value (it's not allowed to place
		// a runway header on first tile), it is used to determine
		// when to start creating the array.
		memset(GameRwyArray, 0, GAME_MAX_RWY_LENGTH * sizeof(uint16_t));
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
				dprintf("Unknown direction for tile %d\n",rwyHeader);
				return;
			break;
		}
	}
	else
	{
		if(	(GameLevelBuffer[last_tile] == TILE_RWY_START_1)
						||
			(GameLevelBuffer[last_tile] == TILE_RWY_START_2)
						||
			(GameLevelBuffer[last_tile] == (TILE_RWY_START_1 | TILE_MIRROR_FLAG) )
						||
			(GameLevelBuffer[last_tile] == (TILE_RWY_START_2 | TILE_MIRROR_FLAG) )	)
		{
			// Runway end found
			GameRwyArray[i++] = last_tile;
			return;
		}
	}
	
	GameRwyArray[i++] = last_tile;
	
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
	
	GameGetSelectedRunwayArray(0);
}

void GameAssignRunwaytoAircraft(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	uint16_t assignedRwy = GameRwy[ptrPlayer->SelectedRunway];
	uint8_t aircraftIndex = ptrPlayer->FlightDataSelectedAircraft;
	uint16_t rwyExit;
	uint32_t i;
	uint16_t targets[AIRCRAFT_MAX_TARGETS];
	uint8_t rwyTiles[GAME_MAX_RWY_LENGTH];
	
	memset(targets, 0, sizeof(uint16_t) * AIRCRAFT_MAX_TARGETS);
	
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
	
	dprintf("aircraftIndex = %d\n",aircraftIndex);
	
	if(ptrFlightData->State[aircraftIndex] == STATE_APPROACH)
	{		
		ptrFlightData->State[aircraftIndex] = STATE_FINAL;
		GameScore += SCORE_REWARD_FINAL;
		
		GameGetSelectedRunwayArray(assignedRwy);
		
		for(i = 0; i < GAME_MAX_RWY_LENGTH; i++)
		{
			rwyTiles[i] = GameLevelBuffer[GameRwyArray[i]];
		}
		
		i = SystemIndexOf_U8((uint8_t)TILE_RWY_EXIT, rwyTiles, 0, GAME_MAX_RWY_LENGTH);
		
		if(i == -1)
		{
			dprintf("ERROR: Could not find TILE_RWY_EXIT for runway header %d.\n", assignedRwy);
			return;
		}
		
		i = SystemIndexOf_U8((uint8_t)TILE_RWY_EXIT, rwyTiles, i + 1, GAME_MAX_RWY_LENGTH);
		
		if(i == -1)
		{
			dprintf("ERROR: Could not find second TILE_RWY_EXIT for runway header %d.\n", assignedRwy);
			return;
		} 
		
		rwyExit = GameRwyArray[i];
		
		targets[0] = assignedRwy;
		targets[1] = rwyExit;
		
		if( AircraftAddNew(ptrFlightData,
							aircraftIndex,
							targets	) == false)
		{
			dprintf("Exceeded maximum aircraft number!\n");
			return;
		}
	}
	else if(ptrFlightData->State[aircraftIndex] == STATE_READY_FOR_TAKEOFF)
	{
		AIRCRAFT_DIRECTION aircraftDir = AircraftGetDirection(AircraftFromFlightDataIndex(aircraftIndex));
		uint16_t rwyTile = 0;

		switch(aircraftDir)
		{
			case AIRCRAFT_DIR_EAST:
				rwyTile = AircraftGetTileFromFlightDataIndex(aircraftIndex) - GameLevelColumns;
			break;

			case AIRCRAFT_DIR_WEST:
				rwyTile = AircraftGetTileFromFlightDataIndex(aircraftIndex) + GameLevelColumns;
			break;

			case AIRCRAFT_DIR_NORTH:
				rwyTile = AircraftGetTileFromFlightDataIndex(aircraftIndex) + 1;
			break;

			case AIRCRAFT_DIR_SOUTH:
				rwyTile = AircraftGetTileFromFlightDataIndex(aircraftIndex) - 1;
			break;

			case AIRCRAFT_DIR_NO_DIRECTION:
				// Fall through
			default:
			break;

		}
		ptrFlightData->State[aircraftIndex] = STATE_TAKEOFF;
		GameScore += SCORE_REWARD_TAKEOFF;
	}
}

short GameGetXFromTile_short(uint16_t tile)
{
	short retVal;
	
	tile %= GameLevelColumns;
	
	retVal = (fix16_t)(tile << TILE_SIZE_BIT_SHIFT);
	
	// Always point to tile center
	retVal += TILE_SIZE >> 1;
	
	return retVal;
}

short GameGetYFromTile_short(uint16_t tile)
{
	short retVal;
	
	tile /= GameLevelColumns;
	
	retVal = (fix16_t)(tile << TILE_SIZE_BIT_SHIFT);
	
	// Always point to tile center
	retVal += TILE_SIZE >> 1;
	
	return retVal;
}

fix16_t GameGetXFromTile(uint16_t tile)
{
	return fix16_from_int(GameGetXFromTile_short(tile));
}

fix16_t GameGetYFromTile(uint16_t tile)
{
	return fix16_from_int(GameGetYFromTile_short(tile));
}

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
		
		default:
		break;
	}
	
	return retState;
}

uint16_t GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS * IsoPos)
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
	
	/*dprintf("Returning tile %d from position {%d, %d, %d}\n",
			tile,
			IsoPos->x,
			IsoPos->y,
			IsoPos->z	);*/
	
	return tile;
}

uint8_t GameGetLevelColumns(void)
{
	return GameLevelColumns;
}

void GamePlayerAddWaypoint(TYPE_PLAYER* ptrPlayer)
{
	GamePlayerAddWaypoint_Ex(ptrPlayer, ptrPlayer->SelectedTile);
}

void GamePlayerAddWaypoint_Ex(TYPE_PLAYER* ptrPlayer, uint16_t tile)
{
	// "_Ex" function allow selecting a certain tile, whereas the other one
	// is a particulare case of "_Ex" for tile = ptrPlayer->SelectedTIle.
	
	if(ptrPlayer->WaypointIdx >= PLAYER_MAX_WAYPOINTS)
	{
		dprintf("No available waypoints for this player!\n");
		return;
	}
	
	/*dprintf("Added tile %d to ptrPlayer->Waypoints[%d]\n",
			tile,
			ptrPlayer->WaypointIdx);*/
	
	ptrPlayer->Waypoints[ptrPlayer->WaypointIdx++] = tile;
}

bool GamePathToTile(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData)
{
	// Given an input TYPE_PLAYER structure and a selected tile,
	// it updates current Waypoints array with all tiles between two points.
	// If one of these tiles do not belong to desired tiles (i.e.: grass,
	// water, buildings...), then false is returned.
	
	uint8_t AcceptedTiles[] = {	TILE_ASPHALT, TILE_ASPHALT_WITH_BORDERS,
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
							
	/*dprintf("SelectedTile = %d, ptrPlayer->Waypoints[%d] = %d\n",
			ptrPlayer->SelectedTile,
			0,
			ptrPlayer->Waypoints[0] );

	dprintf("X = abs(%d - %d)\n",
			ptrPlayer->SelectedTile % GameLevelColumns,
			(ptrPlayer->Waypoints[0] % GameLevelColumns) );
			
	dprintf("Y = abs(%d - %d)\n",
			ptrPlayer->SelectedTile / GameLevelColumns,
			(ptrPlayer->Waypoints[0] / GameLevelColumns) );
			
	dprintf("Diff = {%d, %d}\n", x_diff, y_diff);*/
	
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
			
			if(SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
			{
				for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
				{
					if(ptrFlightData->State[i] != STATE_IDLE)
					{
						if(temp_tile == AircraftGetTileFromFlightDataIndex(i))
						{
							dprintf("i = %d, state = %d\n", i, ptrFlightData->State[i]);
							return false;	// Check pending!
						}
					}
				}
				
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
			}
			else
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
			
			if(SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
			{
				for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
				{
					if(ptrFlightData->State[i] != STATE_IDLE)
					{
						if(temp_tile == AircraftGetTileFromFlightDataIndex(i))
						{
							dprintf("i = %d, state = %d\n", i, ptrFlightData->State[i]);
							return false;	// Check pending!
						}
					}
				}
				
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
			}
			else
			{
				// TEST - Check pending!
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
			
			if(SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
			{
				for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
				{
					if(temp_tile == AircraftGetTileFromFlightDataIndex(i))
					{
						return false;	// Check pending!
					}
				}
				
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
			}
			else
			{
				// TEST - Check pending!
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
			
			if(SystemContains_u16(temp_tile, ptrPlayer->Waypoints, PLAYER_MAX_WAYPOINTS) == false)
			{
				for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
				{
					if(temp_tile == AircraftGetTileFromFlightDataIndex(i))
					{
						return false;	// Check pending!
					}
				}

				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
			}
			else
			{
				// TEST - Check pending!
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

TYPE_ISOMETRIC_POS GameSelectAircraft(TYPE_PLAYER* ptrPlayer)
{
	uint8_t AircraftIdx = ptrPlayer->FlightDataSelectedAircraft;
	TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(AircraftIdx);
	
	CameraMoveToIsoPos(ptrPlayer, IsoPos);
	
	return IsoPos;
}

void GameSelectAircraftWaypoint(TYPE_PLAYER* ptrPlayer)
{
	TYPE_ISOMETRIC_POS IsoPos = GameSelectAircraft(ptrPlayer);
	
	ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);
	
	GamePlayerAddWaypoint(ptrPlayer);
}

bool GameTwoPlayersActive(void)
{
	return TwoPlayersActive;
}

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
	if(FlightDataIdx >= GAME_MAX_AIRCRAFT)
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

					GameGenerateUnboardingSequence(ptrPlayer);
				}
				else
				{
					// Flight has finished. Remove aircraft and set finished flag

					ptrFlightData->Passengers[ptrPlayer->FlightDataSelectedAircraft] = 0;
					ptrFlightData->State[ptrPlayer->FlightDataSelectedAircraft] = STATE_IDLE;
					ptrPlayer->Unboarding = false;
					ptrFlightData->Finished[ptrPlayer->FlightDataSelectedAircraft] = true;
					
					if(AircraftRemove(ptrPlayer->FlightDataSelectedAircraft) == false)
					{
						dprintf("Something went wrong when removing aircraft!\n");
					}

					ptrPlayer->LockTarget = false;
					ptrPlayer->LockedAircraft = 0;
					
					GameScore += SCORE_REWARD_FINISH_FLIGHT;
				}
				
				ptrPlayer->UnboardingSequenceIdx = 0;
			}

			dprintf("ptrPlayer->UnboardingSequenceIdx = %d\n", ptrPlayer->UnboardingSequenceIdx);
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
	unsigned short keyTable[] = {	PAD_CROSS, PAD_SQUARE, PAD_TRIANGLE };

	memset(ptrPlayer->UnboardingSequence, 0, sizeof(ptrPlayer->UnboardingSequence) );

	ptrPlayer->UnboardingSequenceIdx = 0;

	dprintf("Key sequence generated: ");
	
	// Only medium level implemented. TODO: Implement other levels
	for(i = 0; i < UNBOARDING_KEY_SEQUENCE_MEDIUM; i++)
	{
		uint8_t randIdx = SystemRand(0, (sizeof(keyTable) / sizeof(keyTable[0])) - 1);
		
		ptrPlayer->UnboardingSequence[i] = keyTable[randIdx];

		dprintf("idx = %d, 0x%04X ", randIdx, ptrPlayer->UnboardingSequence[i]);
	}

	dprintf("\n");
}

void GameCreateTakeoffWaypoints(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA* ptrFlightData, uint8_t aircraftIdx)
{
	// Look for aircraft direction by searching TILE_RWY_EXIT
	AIRCRAFT_DIRECTION aircraftDir = AIRCRAFT_DIR_NO_DIRECTION;
	uint16_t currentTile = AircraftGetTileFromFlightDataIndex(aircraftIdx);
	uint16_t targets[AIRCRAFT_MAX_TARGETS] = {0};
	uint8_t targetsIdx = 0;
	uint16_t runwayTile = 0;
	uint16_t i;

	if( (currentTile > GameLevelColumns) && ( (currentTile + GameLevelColumns) < (sizeof(GameLevelBuffer) / sizeof(GameLevelBuffer[0]) ) ) )
	{
		if(GameLevelBuffer[currentTile + 1] == TILE_RWY_EXIT)
		{
			aircraftDir = AIRCRAFT_DIR_EAST;
			runwayTile = currentTile + 1;
		}
		else if(GameLevelBuffer[currentTile - 1] == TILE_RWY_EXIT)
		{
			aircraftDir = AIRCRAFT_DIR_WEST;
			runwayTile = currentTile - 1;
		}
		else if(GameLevelBuffer[currentTile + GameLevelColumns] == TILE_RWY_EXIT)
		{
			aircraftDir = AIRCRAFT_DIR_SOUTH;
			runwayTile = currentTile + GameLevelColumns;
		}
		else if(GameLevelBuffer[currentTile - GameLevelColumns] == TILE_RWY_EXIT)
		{
			aircraftDir = AIRCRAFT_DIR_NORTH;
			runwayTile = currentTile - GameLevelColumns;
		}
		else
		{
			dprintf("GameCreateTakeoffWaypoints(): could not determine aircraft direction.\n");
			return;
		}

		targets[targetsIdx++] = runwayTile;

		dprintf("Direction = ");

		switch(aircraftDir)
		{
			case AIRCRAFT_DIR_EAST:
				dprintf("EAST\n");
			break;

			case AIRCRAFT_DIR_WEST:
				dprintf("WEST\n");
			break;

			case AIRCRAFT_DIR_NORTH:
				dprintf("NORTH\n");
			break;

			case AIRCRAFT_DIR_SOUTH:
				dprintf("SOUTH\n");
				for(i = (runwayTile + 1); (i < (runwayTile + GameLevelColumns)) && GameLevelBuffer[i] != TILE_RWY_EXIT; i++)
				{
					targets[targetsIdx++] = i;
				}
			break;

			default:
			break;
		}

		dprintf("Waypoints for takeoff added = ");

		for(i = 0; i < AIRCRAFT_MAX_TARGETS; i++)
		{
			dprintf("%d ", targets[i]);
		}

		dprintf("\n");
		
		AircraftAddTargets(AircraftFromFlightDataIndex(aircraftIdx), targets);
		
	}
	else
	{
		dprintf("GameCreateTakeoffWaypoints(): Invalid index for tile.\n");
	}
}

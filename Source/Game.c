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
	TILE_RESERVED_4,
	TILE_RESERVED_5,
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
	MOUSE_X = X_SCREEN_RESOLUTION - (X_SCREEN_RESOLUTION >> 1),
	MOUSE_Y = Y_SCREEN_RESOLUTION - (Y_SCREEN_RESOLUTION >> 1),
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void GameInit(void);
static void GameLoadLevel(void);
static bool GamePause(void);
static void GameEmergencyMode(void);
static void GameCalculations(void);
static void GamePlayerHandler(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GamePlayerAddWaypoint(TYPE_PLAYER * ptrPlayer);
static void GamePlayerAddWaypoint_Ex(TYPE_PLAYER * ptrPlayer, uint16_t tile);
static void GameGraphics(void);
static void GameRenderLevel(TYPE_PLAYER * ptrPlayer);
//static void GameLoadPilots(char * strPath);
static void GameClock(void);
static void GameClockFlights(void);
static void GameAircraftState(void);
static void GameActiveAircraft(void);
static void GameStateShowAircraft(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameFirstLastAircraftIndex(void);
static void GameSelectAircraftFromList(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectRunway(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectTaxiwayRunway(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateSelectTaxiwayParking(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static void GameStateLockTarget(TYPE_PLAYER * ptrPlayer);
static void GameSelectAircraft(TYPE_PLAYER * ptrPlayer);
static void GameGetRunwayArray(void);
static void GameGetSelectedRunwayArray(uint16_t rwyHeader);
static void GameAssignRunwaytoAircraft(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
static bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER * ptrPlayer);
static uint16_t GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS * IsoPos);
static bool GamePathToTile(TYPE_PLAYER * ptrPlayer);

/* *************************************
 * 	Global Variables
 * *************************************/

bool GameStartupFlag;

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
	
	EndAnimation();
	
	SfxPlayTrack(INTRO_TRACK);
}

bool GamePause(void)
{
	TYPE_PLAYER * ptrPlayer;
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
			//dprintf("Released callback = 0x%08X\n", ptrPlayer->PadKeyReleased_Callback);
			if(ptrPlayer->PadKeyReleased_Callback(PAD_START) == true)
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
	PlayerData[PLAYER_ONE].PadDirectionKeyPressed_Callback = &PadOneDirectionKeyPressed;
	PlayerData[PLAYER_ONE].FlightDataPage = 0;
	
	PlayerData[PLAYER_TWO].Active = TwoPlayersActive? true : false;
	
	if(PlayerData[PLAYER_TWO].Active == true)
	{
		PlayerData[PLAYER_TWO].PadKeyPressed_Callback = &PadTwoKeyPressed;
		PlayerData[PLAYER_TWO].PadKeyReleased_Callback = &PadTwoKeyReleased;
		PlayerData[PLAYER_TWO].PadDirectionKeyPressed_Callback = &PadTwoDirectionKeyPressed;
		PlayerData[PLAYER_TWO].FlightDataPage = 0;
		
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
	
	GameMouseSpr.x = MOUSE_X;
	GameMouseSpr.y = MOUSE_Y;
	GameMouseSpr.w = MOUSE_W;
	GameMouseSpr.h = MOUSE_H;
	GameMouseSpr.attribute = COLORMODE(COLORMODE_16BPP);
	GameMouseSpr.r = NORMAL_LUMINANCE;
	GameMouseSpr.g = NORMAL_LUMINANCE;
	GameMouseSpr.b = NORMAL_LUMINANCE;
	
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
	AircraftHandler();
	
	for(i = 0 ; i < MAX_PLAYERS ; i++)
	{
		// Run player-specific functions for each player
		if(PlayerData[i].Active == true)
		{
			GamePlayerHandler(&PlayerData[i], &FlightData);
		}
	}
	
	if(PadOneKeyReleased(PAD_CIRCLE) == true)
	{
		for(i = 0; i < FlightData.nAircraft ; i++)
		{
			/*typedef struct
			{
				FL_DIR FlightDirection[GAME_MAX_AIRCRAFT];
				char strFlightNumber[GAME_MAX_AIRCRAFT][GAME_MAX_CHARACTERS];
				uint8_t Passengers[GAME_MAX_AIRCRAFT];
				uint8_t Hours[GAME_MAX_AIRCRAFT];
				uint8_t Minutes[GAME_MAX_AIRCRAFT];
				uint8_t Parking[GAME_MAX_AIRCRAFT];
			}TYPE_FLIGHT_DATA;*/
			
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
	}
	
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

void GamePlayerHandler(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	ptrPlayer->SelectedTile = 0;	// Reset selected tile if no states
									// which use this are currently active.
	ptrPlayer->InvalidPath = false; // Do the same thing for "InvalidPath".

	GameStateLockTarget(ptrPlayer);
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
		}
	}
}

void GameGraphics(void)
{
	int i;
	
	while(	(GfxIsGPUBusy() == true)
					||
			(SystemRefreshNeeded() == false)	);

	GsSortCls(0,0,NORMAL_LUMINANCE >> 1);
	
	if(GfxGetGlobalLuminance() < NORMAL_LUMINANCE)
	{
		GfxIncreaseGlobalLuminance(1);
	}
	
	for(i = 0; i < MAX_PLAYERS ; i++)
	{
		if(PlayerData[i].Active == true)
		{
			GameRenderLevel(&PlayerData[i]);
			AircraftRender(&PlayerData[i]);
		}
	}
	
	GameGuiAircraftNotificationRequest(&FlightData);
	
	GameGuiBubble(&FlightData);
	
	GameGuiClock(GameHour,GameMinutes);
	
	for(i = 0; i < MAX_PLAYERS ; i++)
	{
		GameGuiAircraftList(&PlayerData[i], &FlightData);
	}
	
	GfxDrawScene();	
}

void GameLoadLevel(void)
{	
	uint8_t i = 0;
	uint8_t * ptrBuffer;
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
	
	for(i = 0; i < FlightData.nAircraft ; i++)
	{
		if(	(FlightData.Hours[i] == 0)
					&&
			(FlightData.Minutes[i] == 0)
					&&
			(FlightData.State[i] == STATE_IDLE)	)
		{
			if(FlightData.FlightDirection[i] == DEPARTURE)
			{
				FlightData.State[i] = STATE_PARKED;
			}
			else if(FlightData.FlightDirection[i] == ARRIVAL)
			{
				FlightData.State[i] = STATE_APPROACH;
			}
			
			// Create notification request for incoming aircraft
			FlightData.NotificationRequest[i] = true;
		}
	}
}


void GameRenderLevel(TYPE_PLAYER * ptrPlayer)
{
	uint16_t i;
	uint16_t j;
	uint8_t columns = 0;
	uint8_t rows = 0;
	bool flip_id;
	bool used_rwy;
	uint8_t aux_id;
	GsSprite * ptrTileset;
	static unsigned char rwy_sine = 0;
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
		
		if(System100msTick() == true)
		{
			if(rwy_sine_decrease == false)
			{
				if(rwy_sine < 255)
				{
					rwy_sine++;
				}
				else
				{
					rwy_sine_decrease = true;
				}
			}
			else
			{
				if(rwy_sine > (NORMAL_LUMINANCE >> 2))
				{
					rwy_sine--;
				}
				else
				{
					rwy_sine_decrease = false;
				}
			}
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
	
	if(	(ptrPlayer->SelectTaxiwayParking == true)
						||
		(ptrPlayer->SelectTaxiwayRunway == true) 	)
	{
		GfxSortSprite(&GameMouseSpr);
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

void GameStateShowAircraft(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	if(ptrPlayer->ShowAircraftData == true)
	{
		if(ptrPlayer->PadKeyReleased_Callback(PAD_TRIANGLE) == true)
		{
			ptrPlayer->ShowAircraftData = false;
		}
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_SQUARE) == true)
		{
			dprintf("Aircraft state = %d. STATE_IDLE = %d\n",
					ptrFlightData->State[ptrPlayer->SelectedAircraft],
					STATE_IDLE);
					
			if(ptrFlightData->State[ptrPlayer->SelectedAircraft] != STATE_IDLE)
			{
				ptrPlayer->LockTarget = true;
				ptrPlayer->LockedAircraft = ptrPlayer->SelectedAircraft;
			}
		}
	}
	
	if(ptrPlayer->PadKeyReleased_Callback(PAD_CIRCLE) == true)
	{
		if(GameGuiShowAircraftDataSpecialConditions(ptrPlayer) == false)
		{
			//Invert ptrPlayer->ShowAircraftData value
			ptrPlayer->ShowAircraftData = ptrPlayer->ShowAircraftData ? false : true;
		}
	}
}

void GameStateLockTarget(TYPE_PLAYER * ptrPlayer)
{
	if(ptrPlayer->LockTarget == true)
	{
		CameraMoveToIsoPos(ptrPlayer, AircraftGetIsoPos(ptrPlayer->LockedAircraft) );
		
		if(ptrPlayer->PadKeyReleased_Callback(PAD_SQUARE) == true)
		{
			ptrPlayer->LockTarget = false;
			ptrPlayer->LockedAircraft = 0;
		}
	}
}

void GameStateSelectTaxiwayRunway(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
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
		
		if(GamePathToTile(ptrPlayer) == false)
		{
			ptrPlayer->InvalidPath = true;
		}
		
		if(ptrPlayer->PadKeyReleased_Callback(PAD_TRIANGLE) == true)
		{
			// State exit.
			ptrPlayer->SelectTaxiwayRunway = false;
			// Clear waypoints array.
			memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
			ptrPlayer->WaypointIdx = 0;
			ptrPlayer->LastWaypointIdx = 0;
		}
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_CROSS) == true)
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
				
				if(	(target_tile == TILE_RWY_START_1)
										||
					(target_tile == (TILE_RWY_START_1 | TILE_MIRROR_FLAG) )
										||
					(target_tile == TILE_RWY_START_2)
										||
					(target_tile == (TILE_RWY_START_2 | TILE_MIRROR_FLAG) ) )
				{
					// TODO: Assign path to aircraft
					AircraftFromFlightDataIndexAddTargets(ptrPlayer->LockedAircraft, ptrPlayer->Waypoints);
					dprintf("Added these targets to aircraft %d:\n", ptrPlayer->LockedAircraft);
					
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
					
					ptrFlightData->State[ptrPlayer->LockedAircraft] = STATE_TAXIING;
				}
			}
		}
	}
}

void GameStateSelectTaxiwayParking(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
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
		
		if(GamePathToTile(ptrPlayer) == false)
		{
			ptrPlayer->InvalidPath = true;
		}
		
		if(ptrPlayer->PadKeyReleased_Callback(PAD_TRIANGLE) == true)
		{
			// State exit.
			ptrPlayer->SelectTaxiwayParking = false;
			// Clear waypoints array.
			memset(ptrPlayer->Waypoints, 0, sizeof(uint16_t) * PLAYER_MAX_WAYPOINTS);
			ptrPlayer->WaypointIdx = 0;
			ptrPlayer->LastWaypointIdx = 0;
		}
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_CROSS) == true)
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
					AircraftFromFlightDataIndexAddTargets(ptrPlayer->LockedAircraft, ptrPlayer->Waypoints);
					
					dprintf("Added these targets to aircraft %d:\n", ptrPlayer->LockedAircraft);
					
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
					
					ptrFlightData->State[ptrPlayer->LockedAircraft] = STATE_TAXIING;
				}
			}
		}
	}
}

void GameStateSelectRunway(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{	
	uint8_t i;
	bool success;
	
	if(ptrPlayer->SelectRunway == true)
	{
		// Under this mode, always reset locking target.
		ptrPlayer->LockTarget = false;
		ptrPlayer->LockedAircraft = 0;
		
		if(ptrPlayer->PadKeyReleased_Callback(PAD_TRIANGLE) == true)
		{
			ptrPlayer->SelectRunway = false;
		}
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_CROSS) == true)
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
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_LEFT) == true)
		{
			if(ptrPlayer->SelectedRunway != 0)
			{
				ptrPlayer->SelectedRunway--;
			}
		}
		else if(ptrPlayer->PadKeyReleased_Callback(PAD_RIGHT) == true)
		{
			if(ptrPlayer->SelectedRunway < GAME_MAX_RUNWAYS)
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

void GameSelectAircraftFromList(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	FL_STATE aircraftState = ptrFlightData->State[ptrPlayer->SelectedAircraft];
	
	if(ptrPlayer->ShowAircraftData == true)
	{
		if(ptrPlayer->PadKeyReleased_Callback(PAD_CROSS) == true)
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
						GameSelectAircraft(ptrPlayer);
					break;
					
					case STATE_LANDED:
						ptrPlayer->SelectTaxiwayParking = true;
						// Move camera to selected aircraft and add first waypoint.
						GameSelectAircraft(ptrPlayer);
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

void GameAssignRunwaytoAircraft(TYPE_PLAYER * ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData)
{
	uint16_t assignedRwy = GameRwy[ptrPlayer->SelectedRunway];
	uint8_t aircraftIndex = ptrPlayer->SelectedAircraft;
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
}

fix16_t GameGetXFromTile(uint16_t tile)
{
	fix16_t retVal;
	
	tile %= GameLevelColumns;
	
	retVal = (fix16_t)(tile << TILE_SIZE_BIT_SHIFT);
	
	// Always point to tile center
	retVal += TILE_SIZE >> 1;
	
	retVal = fix16_from_int(retVal);
	
	return retVal;
}

fix16_t GameGetYFromTile(uint16_t tile)
{
	fix16_t retVal;
	
	tile /= GameLevelColumns;
	
	retVal = (fix16_t)(tile << TILE_SIZE_BIT_SHIFT);
	
	// Always point to tile center
	retVal += TILE_SIZE >> 1;
	
	retVal = fix16_from_int(retVal);
	
	return retVal;
}

void GameTargetsReached(uint8_t index)
{
	switch(FlightData.State[index])
	{
		case STATE_FINAL:
			FlightData.State[index] = STATE_LANDED;
		break;
		
		default:
		break;
	}
}

bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER * ptrPlayer)
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

uint16_t GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS * IsoPos)
{
	uint16_t tile;
	
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

void GamePlayerAddWaypoint(TYPE_PLAYER * ptrPlayer)
{
	GamePlayerAddWaypoint_Ex(ptrPlayer, ptrPlayer->SelectedTile);
}

void GamePlayerAddWaypoint_Ex(TYPE_PLAYER * ptrPlayer, uint16_t tile)
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

bool GamePathToTile(TYPE_PLAYER * ptrPlayer)
{
	// Given an input TYPE_PLAYER structure and a selected tile,
	// it updates current Waypoints array with all tiles between two points.
	// If one of these tiles do not belong to desired tiles (i.e.: grass,
	// water, buildings...), then false is returned.
	
	uint8_t AcceptedTiles[] = {	TILE_ASPHALT, TILE_ASPHALT_WITH_BORDERS,
								TILE_PARKING, TILE_RWY_START_1,
								TILE_RWY_START_2, TILE_RWY_MID,
								TILE_RWY_EXIT, TILE_TAXIWAY_CORNER_GRASS,
								TILE_TAXIWAY_CORNER_GRASS_2, TILE_TAXIWAY_GRASS,
								TILE_TAXIWAY_INTERSECT_GRASS};
								
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
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
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
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
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
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
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
				GamePlayerAddWaypoint_Ex(ptrPlayer, temp_tile);
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

void GameSelectAircraft(TYPE_PLAYER * ptrPlayer)
{
	TYPE_ISOMETRIC_POS IsoPos = AircraftGetIsoPos(ptrPlayer->SelectedAircraft);
	
	CameraMoveToIsoPos(ptrPlayer, IsoPos);
	
	ptrPlayer->SelectedTile = GameGetTileFromIsoPosition(&IsoPos);
	
	GamePlayerAddWaypoint(ptrPlayer);
}

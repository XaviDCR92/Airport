#ifndef GAME_STRUCTURES__HEADER__
#define GAME_STRUCTURES__HEADER__

/* *************************************
 * 	Defines
 * *************************************/

#define GAME_MAX_AIRCRAFT 32
#define GAME_MAX_CHARACTERS 8
#define GAME_MAX_PARKING 32
#define GAME_MAX_RWY_LENGTH 16
#define GAME_MAX_MAP_SIZE 0x400
#define CHEAT_ARRAY_SIZE 16
#define AIRCRAFT_MAX_TARGETS 48
#define PLAYER_MAX_WAYPOINTS AIRCRAFT_MAX_TARGETS
#define GAME_MAX_SEQUENCE_KEYS 12

/* *************************************
 * 	Structs and enums
 * *************************************/

typedef struct t_Camera
{
	int32_t X_Offset;
	int32_t Y_Offset;
	int8_t X_Speed;
	int8_t Y_Speed;
	uint8_t Speed_Timer;
}TYPE_CAMERA;

typedef enum t_fldir
{
	DEPARTURE = 0x01,
	ARRIVAL = 0x02
}FL_DIR;

typedef enum t_flstate
{
	STATE_IDLE = 0,
	STATE_PARKED,
	STATE_UNBOARDING,
	STATE_TAXIING,
	STATE_HOLDING_RWY,
	STATE_ENTERING_RWY,
	STATE_READY_FOR_TAKEOFF,
	STATE_TAKEOFF,
	STATE_CLIMBING,
	STATE_APPROACH,
	STATE_FINAL,
	STATE_USER_STOPPED,
    STATE_AUTO_STOPPED,
	STATE_LANDED,
	MAX_STATES
}FL_STATE;

typedef enum t_LevelDifficulty
{
	LEVEL_DIFFICULTY_EASY = 0,
	LEVEL_DIFFICULTY_MEDIUM,
	LEVEL_DIFFICULTY_HARD
}LEVEL_DIFFICULTY;

typedef struct t_isopos
{
	short x;
	short y;
	short z;
}TYPE_ISOMETRIC_POS;

typedef struct t_isofix16pos
{
	fix16_t x;
	fix16_t y;
	fix16_t z;
}TYPE_ISOMETRIC_FIX16_POS;

typedef struct t_cartpos
{
	short x;
	short y;
}TYPE_CARTESIAN_POS;

typedef struct t_tileData
{
	bool ShowTile;
    TYPE_CARTESIAN_POS CartPos;
    unsigned char r;
    unsigned char g;
    unsigned char b;
}TYPE_TILE_DATA;

typedef struct t_flightData
{
	FL_DIR FlightDirection[GAME_MAX_AIRCRAFT];
	char strFlightNumber[GAME_MAX_AIRCRAFT][GAME_MAX_CHARACTERS];
	uint8_t Passengers[GAME_MAX_AIRCRAFT];
	uint8_t Hours[GAME_MAX_AIRCRAFT];
	uint8_t Minutes[GAME_MAX_AIRCRAFT];
	uint8_t Parking[GAME_MAX_AIRCRAFT];
	uint16_t RemainingTime[GAME_MAX_AIRCRAFT];
	uint8_t nAircraft;
	uint8_t nRemainingAircraft;
	uint8_t ActiveAircraft;
	FL_STATE State[GAME_MAX_AIRCRAFT];
	bool Finished[GAME_MAX_AIRCRAFT];
}TYPE_FLIGHT_DATA;

typedef enum t_livery
{
	AIRCRAFT_LIVERY_UNKNOWN = 0,
	AIRCRAFT_LIVERY_0
}AIRCRAFT_LIVERY;

typedef enum t_direction
{
	NO_DIRECTION = 0,
	DIR_NORTH,
	DIR_SOUTH,
	DIR_EAST,
	DIR_WEST,
}DIRECTION;

typedef enum t_aircraftAttitude
{
	AIRCRAFT_STATE_NEUTRAL = 0,
	AIRCRAFT_STATE_UP_5_DEGREES,
	AIRCRAFT_STATE_UP_15_DEGREES,
	AIRCRAFT_STATE_DOWN_5_DEGREES,
}AIRCRAFT_ATTITUDE;

typedef struct t_aircraftData
{
	AIRCRAFT_LIVERY Livery;
	DIRECTION Direction;
	AIRCRAFT_ATTITUDE Attitude;
	FL_STATE State;
	// Target tile (used to calculate direction and movement)
	uint16_t Target[AIRCRAFT_MAX_TARGETS];
	uint8_t TargetIdx;
	// Used to relate TYPE_AIRCRAFT_DATA and TYPE_FLIGHT_DATA
	uint8_t FlightDataIdx;
	// Position data (real pos inside map)
	TYPE_ISOMETRIC_FIX16_POS IsoPos;
	fix16_t Speed;
	short TargetSpeed;
	bool TargetReached;
	// Used for target reached detection
	fix16_t XPos_Old;
	// Used for target reached detection
	fix16_t YPos_Old;
}TYPE_AIRCRAFT_DATA;

typedef struct
{
	// ## State flags ##
		// Player is on the game
		bool Active;
		// Player requests showing aircraft data menu
		bool ShowAircraftData;
		// Player on runway selection mode (when FL_STATE == APPROACH)
		bool SelectRunway;
		// Player on taxiway + runway selection mode (when FL_STATE == PARKED)
		bool SelectTaxiwayRunway;
		// Player on taxiway + parking selection mode (when FL_STATE == LANDED)
		bool SelectTaxiwayParking;
		// Tiles selected by player are not valid for establishing a new path
		bool InvalidPath;
		// Player has locked the camera at a determined aircraft
		bool LockTarget;
		// Player is unboarding passengers
		bool Unboarding;

	// Stores indexes for player-specific active aircraft
	// Used to relate SelectedAircraft agains FlightData index
	uint8_t	ActiveAircraftList[GAME_MAX_AIRCRAFT];
	// Flight direction to be managed by player (see 2-player mode)
	FL_DIR	FlightDirection;
	// Number of active aircraft for current player (could be not equal to total active aircraft!)
	uint8_t	ActiveAircraft;
	// When too many aircraft are active, they are separated into different pages
	uint8_t	FlightDataPage;
	// Selected aircraft on aircraft data list
	uint8_t	SelectedAircraft;
	// Aircraft being followed by camera
	uint8_t LockedAircraft;
	// Index for GameRwy buffer on "Select runway" mode
	uint16_t SelectedRunway;
	// Tile pointed to by cursor
	uint16_t SelectedTile;
	// Conversion between selected aircraft from data list and actual flight data index
	uint8_t FlightDataSelectedAircraft;
	// Waypoints added to list when player is tracing a path for an aircraft.
	// For example: when determining path from runway to parking.
	uint16_t Waypoints[PLAYER_MAX_WAYPOINTS];
	// Internal index for waypoint management.
	uint8_t WaypointIdx;
	// Another internal index to keep last desired selected point by user when defining a path.
	uint8_t LastWaypointIdx;
	// If player is unboarding passengers, then a sequence of keys is generated to make unboarding
	// process a bit more difficult and challenging.
	unsigned short UnboardingSequence[GAME_MAX_SEQUENCE_KEYS];
	// Index inside UnboardingSequence[]
	uint8_t UnboardingSequenceIdx;
	// Show passengers left
	uint8_t PassengersLeftSelectedAircraft;
    // Lookup tables defined on GameRenderTerrainPrecalculations() to be later used on
    // GameRenderTerrain().
    TYPE_TILE_DATA TileData[GAME_MAX_MAP_SIZE];
    // Player camera instance.
    TYPE_CAMERA Camera;
    // Array of tiles which will change their RGB values when displayed under certain player states.
    uint16_t RwyArray[GAME_MAX_RWY_LENGTH];
    // Remaining time for next aircraft (if any).
    uint16_t NextAircraftTime;

    // Pad callbacks.
	bool	(*PadKeyPressed_Callback)(unsigned short);
	bool	(*PadKeyReleased_Callback)(unsigned short);
	bool	(*PadKeySinglePress_Callback)(unsigned short);
	bool	(*PadDirectionKeyPressed_Callback)(void);
	unsigned short	(*PadLastKeySinglePressed_Callback)(void);
}TYPE_PLAYER;

typedef enum t_fontflags
{
	FONT_NOFLAGS		= 1 << 0,
	FONT_CENTERED		= 1 << 1,
	FONT_WRAP_LINE		= 1 << 2,
	FONT_BLEND_EFFECT	= 1 << 3,
	FONT_1HZ_FLASH		= 1 << 4,
	FONT_2HZ_FLASH		= 1 << 5
}FONT_FLAGS;

typedef struct t_Font
{
	GsSprite spr;
	short char_spacing;
	short char_w;
	short char_h;
	char init_ch;
	uint8_t char_per_row;
	uint8_t max_ch_wrap;
	volatile FONT_FLAGS flags;
	short spr_w;
	short spr_h;
	short spr_u;
	short spr_v;
}TYPE_FONT;

typedef struct t_Timer
{
	uint32_t time;
	uint32_t orig_time;
	bool repeat_flag;
	bool busy;
	void (*Timeout_Callback)(void);
}TYPE_TIMER;

typedef struct t_Cheat
{
	unsigned short Combination[CHEAT_ARRAY_SIZE];
	void (*Callback)(void);
}TYPE_CHEAT;

// TYPE_PLT_CONFIG is used for PLT file generation.
typedef struct t_PltConfig
{
	uint8_t MaxTime; // Minutes
	uint8_t nRunway;
	uint8_t ParkingArray[GAME_MAX_PARKING];
	LEVEL_DIFFICULTY Level;
	bool TwoPlayers;
	uint8_t maxLostFlights;
}TYPE_PLT_CONFIG;

#endif // GAME_STRUCTURES__HEADER__

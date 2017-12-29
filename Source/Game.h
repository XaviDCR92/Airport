#ifndef GAME_HEADER__
#define GAME_HEADER__

/* *************************************
 * 	Includes
 * *************************************/
#include "Global_Inc.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/
#define PLAYER_ONE 0
#define PLAYER_TWO 1
#define MAX_PLAYERS (PLAYER_TWO + 1)

#define TILE_SIZE 64
#define TILE_SIZE_H 48
#define TILE_SIZE_BIT_SHIFT 6

/* *************************************
 * 	Structs and enums
 * *************************************/
/* *************************************
 * 	Global variables
 * *************************************/
extern bool GameStartupFlag;

/* *************************************
 * 	Global prototypes
 * *************************************/
void 		Game(bool two_players);
void 		GameSetTime(uint8_t hour, uint8_t minutes);
bool		GameTwoPlayersActive(void);
uint8_t 	GameGetLevelColumns(void);
fix16_t 	GameGetXFromTile(uint16_t tile);
fix16_t 	GameGetYFromTile(uint16_t tile);
short		GameGetXFromTile_short(uint16_t tile);
short		GameGetYFromTile_short(uint16_t tile);
FL_STATE 	GameTargetsReached(uint16_t firstTarget, uint8_t index);
uint16_t	GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS* IsoPos);
FL_STATE	GameGetFlightDataStateFromIdx(uint8_t FlightDataIdx);
uint32_t	GameGetScore(void);
bool		GameInsideLevelFromIsoPos(TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos);
void		GameRemoveFlight(uint8_t idx, bool successful);
void		GameCalculateRemainingAircraft(void);
void		GameAircraftCollision(uint8_t AircraftIdx);
void        GameStopFlight(uint8_t AicraftIdx);
void        GameResumeFlightFromAutoStop(uint8_t AircraftIdx);

#endif //GAME_HEADER__

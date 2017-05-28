#ifndef __GAME_HEADER__
#define __GAME_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "GameStructures.h"
#include "LoadMenu.h"
#include "System.h"
#include "Camera.h"
#include "Aircraft.h"
#include "GameGui.h"

/* *************************************
 * 	Defines
 * *************************************/

#define PLAYER_ONE 0
#define PLAYER_TWO 1
#define MAX_PLAYERS (PLAYER_TWO + 1)

#define TILE_SIZE 64
#define TILE_SIZE_BIT_SHIFT 6

/* *************************************
 * 	Global variables
 * *************************************/

extern bool GameStartupFlag;
extern TYPE_PLAYER PlayerData[];

/* *************************************
 * 	Global prototypes
 * *************************************/

void 		Game(bool two_players);
char * 		GetGameLevelTitle(void);
void 		GameSetTime(uint8_t hour, uint8_t minutes);
bool		GameTwoPlayersActive(void);
uint8_t 	GameGetFirstActiveAircraft(void);
uint8_t 	GameGetLastActiveAircraft(void);
uint8_t 	GameGetLevelColumns(void);
fix16_t 	GameGetXFromTile(uint16_t tile);
fix16_t 	GameGetYFromTile(uint16_t tile);
short		GameGetXFromTile_short(uint16_t tile);
short		GameGetYFromTile_short(uint16_t tile);
FL_STATE 	GameTargetsReached(uint16_t firstTarget, uint8_t index);
uint16_t	GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS * IsoPos);
FL_STATE	GameGetFlightDataStateFromIdx(uint8_t FlightDataIdx);
uint32_t	GameGetScore(void);
bool		GameInsideLevelFromIsoPos(TYPE_ISOMETRIC_FIX16_POS* ptrIsoPos);
void		GameRemoveFlight(uint8_t idx);
#endif //__GAME_HEADER__

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
uint8_t 	GameGetFirstActiveAircraft(void);
uint8_t 	GameGetLastActiveAircraft(void);
uint8_t 	GameGetLevelColumns(void);
fix16_t 	GameGetXFromTile(uint16_t tile);
fix16_t 	GameGetYFromTile(uint16_t tile);
void 		GameTargetsReached(uint8_t index);
uint16_t	GameGetTileFromIsoPosition(TYPE_ISOMETRIC_POS * IsoPos);

#endif //__GAME_HEADER__

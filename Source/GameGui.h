#ifndef __GAME_GUI_HEADER__
#define __GAME_GUI_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "GameStructures.h"
#include "Gfx.h"
#include "Game.h"

/* *************************************
 * 	Defines
 * *************************************/

/* *************************************
 * 	Structs and enums
 * *************************************/

/* *************************************
 * 	Global variables
 * *************************************/

/* *************************************
 * 	Global prototypes
 * *************************************/

void GameGuiInit(void);
bool GameGuiPauseDialog(TYPE_PLAYER* ptrPlayer);
bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER* ptrPlayer);
void GameGuiAircraftNotificationRequest(TYPE_FLIGHT_DATA * ptrFlightData);
void GameGuiBubble(TYPE_FLIGHT_DATA * ptrFlightData);
void GameGuiAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
void GameGuiClock(uint8_t hour, uint8_t min);
void GameGuiActiveAircraftList(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
void GameGuiActiveAircraftPage(TYPE_PLAYER* ptrPlayer, TYPE_FLIGHT_DATA * ptrFlightData);
void GameGuiCalculateSlowScore(void);
void GameGuiShowScore(void);
void GameGuiDrawUnboardingSequence(TYPE_PLAYER* ptrPlayer);

#endif //__GAME_GUI_HEADER__

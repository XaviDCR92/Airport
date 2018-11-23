#ifndef GAME_GUI_HEADER__
#define GAME_GUI_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "GameStructures.h"

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
bool GameGuiPauseDialog(const TYPE_PLAYER* const ptrPlayer);
bool GameGuiShowAircraftDataSpecialConditions(TYPE_PLAYER* const ptrPlayer);
void GameGuiBubble(TYPE_FLIGHT_DATA* const ptrFlightData);
void GameGuiClock(uint8_t hour, uint8_t min);
void GameGuiActiveAircraftList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
void GameGuiActiveAircraftPage(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
void GameGuiCalculateSlowScore(void);
void GameGuiShowScore(void);
void GameGuiDrawUnboardingSequence(TYPE_PLAYER* const ptrPlayer);
void GameGuiAircraftList(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);
bool GameGuiFinishedDialog(TYPE_PLAYER* const ptrPlayer);
void GameGuiAircraftCollision(TYPE_PLAYER* const ptrPlayer);
void GameGuiBubbleShow(void);
void GameGuiShowPassengersLeft(TYPE_PLAYER* const ptrPlayer);
void GameGuiCalculateNextAircraftTime(TYPE_PLAYER* const ptrPlayer, TYPE_FLIGHT_DATA* const ptrFlightData);

#endif //GAME_GUI_HEADER__

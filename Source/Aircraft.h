#ifndef __AIRCRAFT_HEADER__
#define __AIRCRAFT_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "GameStructures.h"


/* *************************************
 * 	Global prototypes
 * *************************************/
 
void AircraftInit(void);
void AircraftHandler(void);
void AircraftRender(TYPE_PLAYER * ptrPlayer);
TYPE_AIRCRAFT_DATA* AircraftFromFlightDataIndex(uint8_t index);
void AircraftFromFlightDataIndexAddTargets(uint8_t index, uint16_t * targets);
void AircraftAddTargets(TYPE_AIRCRAFT_DATA* ptrAircraft, uint16_t * targets);
TYPE_ISOMETRIC_POS AircraftGetIsoPos(uint8_t FlightDataIdx);
uint16_t AircraftGetTileFromFlightDataIndex(uint8_t index);
bool AircraftAddNew(	TYPE_FLIGHT_DATA * ptrFlightData,
						uint8_t FlightDataIndex,
						uint16_t * targets		);

#endif //__AIRCRAFT_HEADER__

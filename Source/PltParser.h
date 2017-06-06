#ifndef __PLT_PARSER__
#define __PLT_PARSER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/

/* **************************************
 * 	Structs and enums					*
 * *************************************/

/* *************************************
 * 	Global prototypes
 * *************************************/

bool PltParserLoadFile(char* strPath, TYPE_FLIGHT_DATA* ptrFlightData);
uint8_t* PltParserGenerateFile(TYPE_PLT_CONFIG* ptrPltConfig);

/* *************************************
 * 	Global variables
 * *************************************/

#endif //__PLT_PARSER__

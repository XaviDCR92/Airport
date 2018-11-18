#ifndef PLT_PARSER__
#define PLT_PARSER__

/* *************************************
 * 	Includes
 * *************************************/
#include "Global_Inc.h"
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
bool PltParserLoadFile(const char* strPath, TYPE_FLIGHT_DATA* ptrFlightData);
uint8_t* PltParserGenerateFile(TYPE_PLT_CONFIG* ptrPltConfig);

/* *************************************
 * 	Global variables
 * *************************************/
#endif //PLT_PARSER__

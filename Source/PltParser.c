/* *************************************
 * 	Includes
 * *************************************/

#include "PltParser.h"
#include "System.h"
#include "Game.h"

/* *************************************
 * 	Defines
 * *************************************/
 
#define LINE_MAX_CHARACTERS 100

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	DEPARTURE_ARRIVAL_INDEX = 0,
	FLIGHT_NUMBER_INDEX,
	PASSENGERS_INDEX,
	HOURS_MINUTES_INDEX,
	PARKING_INDEX,
	REMAINING_TIME_INDEX
};

enum
{
	PLT_HOUR_MINUTE_CHARACTERS = 2,
	PLT_FIRST_LINE_CHARACTERS = 5,
	PLT_COLON_POSITION = 2
};

/* *************************************
 * 	Local Variables
 * *************************************/

/* *************************************
 * 	Local Prototypes
 * *************************************/
static void PltParserResetBuffers(TYPE_FLIGHT_DATA* ptrFlightData);

bool PltParserLoadFile(char* strPath, TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t i;
	uint8_t j;
	uint8_t aircraftIndex;
	bool first_line_read = false;
	char* buffer;
	char lineBuffer[LINE_MAX_CHARACTERS];
	char* lineBufferPtr;
	char* pltBufferSavePtr;
	char strHour[PLT_HOUR_MINUTE_CHARACTERS];
	char strMinutes[PLT_HOUR_MINUTE_CHARACTERS];
	uint8_t* strPltBuffer;

	if(SystemLoadFile(strPath) == false)
	{
		Serial_printf("Error loading file %s!\n",strPath);
		return false;
	}

	strPltBuffer = SystemGetBufferAddress();

    dprintf("%s\n", strPltBuffer);
	
	PltParserResetBuffers(ptrFlightData);
	
	// Now, buffer shall be read from line to line
	
	buffer = strtok_r((char*)strPltBuffer,"\n",&pltBufferSavePtr);
	
	aircraftIndex = 0;
	
	while(buffer != NULL)
	{
		if(buffer[0] == '#')
		{
			// Comment line
			buffer = strtok_r(NULL,"\n",&pltBufferSavePtr);
			continue;
		}
		
		if(first_line_read == false)
		{
			// First (non-comment) line should indicate level time
			// i.e.: 10:30, or 22:45
			first_line_read = true;
			
			if(strlen(buffer) != PLT_FIRST_LINE_CHARACTERS)
			{
				// Format should always be HH:MM (5 characters)
				// Treat any other combination as possible error
				return false;
			}
			
			if(buffer[PLT_COLON_POSITION] != ':')
			{
				// Check whether time format is HH:MM
				return false;
			}
			
			j = 0;
			
			for(i = 0; i < PLT_FIRST_LINE_CHARACTERS ; i++)
			{
				if(i == PLT_COLON_POSITION)
				{
					j = 0;
					buffer = strtok(NULL,"\n");
					continue;
				}
				else if(i < PLT_COLON_POSITION)
				{
					strHour[j++] = buffer[i];
				}
				else
				{
					strMinutes[j++] = buffer[i];
				}
			}

			GameSetTime((uint8_t)atoi(strHour),(uint8_t)atoi(strMinutes) );
			Serial_printf("Game time set to %.2d:%.2d.\n",(uint8_t)atoi(strHour),(uint8_t)atoi(strMinutes) );
		}
		else
		{
			// File header (initial game time) has already been read
			strncpy(lineBuffer, buffer, LINE_MAX_CHARACTERS);
			
			lineBufferPtr = strtok(lineBuffer,";");
			
			i = 0;
			
			Serial_printf("New line read: %s\n",buffer);
			
			while(lineBufferPtr != NULL)
			{
				/*
				 * enum
					{
						DEPARTURE_ARRIVAL_INDEX = 0,
						FLIGHT_NUMBER_INDEX,
						PASSENGERS_INDEX,
						HOURS_MINUTES_INDEX,
						PARKING_INDEX,
						REMAINING_TIME_INDEX
					};*/
				switch(i)
				{
					case DEPARTURE_ARRIVAL_INDEX:
					
						if(strncmp(lineBufferPtr,"DEPARTURE",strlen("DEPARTURE") ) == 0)
						{
							ptrFlightData->FlightDirection[aircraftIndex] = DEPARTURE;
							Serial_printf("Aircraft %d set to DEPARTURE.\n",aircraftIndex);
						}
						else if(strncmp(lineBufferPtr,"ARRIVAL",strlen("ARRIVAL") ) == 0)
						{
							ptrFlightData->FlightDirection[aircraftIndex] = ARRIVAL;
							Serial_printf("Aircraft %d set to ARRIVAL.\n",aircraftIndex);
						}
						else
						{
							Serial_printf("Flight direction is not correct!\n");
						}
					break;
					
					case FLIGHT_NUMBER_INDEX:
						strncpy(ptrFlightData->strFlightNumber[aircraftIndex],lineBufferPtr,GAME_MAX_CHARACTERS);
						ptrFlightData->strFlightNumber[aircraftIndex][GAME_MAX_CHARACTERS - 1] = '\0';
						Serial_printf("Aircraft %d flight number set to %s.\n",aircraftIndex,ptrFlightData->strFlightNumber[aircraftIndex]);
					break;
					
					case PASSENGERS_INDEX:
						ptrFlightData->Passengers[aircraftIndex] = atoi(lineBufferPtr);
						Serial_printf("Aircraft %d passengers set to %d.\n",aircraftIndex,ptrFlightData->Passengers[aircraftIndex]);
					break;
					
					case PARKING_INDEX:
						if(ptrFlightData->FlightDirection[aircraftIndex] == DEPARTURE)
						{
							ptrFlightData->Parking[aircraftIndex] = atoi(lineBufferPtr);
						}
						else
						{
							ptrFlightData->Parking[aircraftIndex] = 0;
						}
						Serial_printf("Aircraft %d parking set to %d.\n",aircraftIndex,ptrFlightData->Parking[aircraftIndex]);
					break;
					
					case HOURS_MINUTES_INDEX:
						if(	strlen(lineBufferPtr) != strlen("HH:MM") )
						{
							Serial_printf("Hour minute format is not correct!\n");
							break;
						}
						
						// Copy hour
						strHour[0] = lineBufferPtr[0];
						strHour[1] = lineBufferPtr[1];
						// Copy minutes
						strMinutes[0] = lineBufferPtr[3];
						strMinutes[1] = lineBufferPtr[4];
						
						ptrFlightData->Hours[aircraftIndex] = (uint8_t)atoi(strHour);
						ptrFlightData->Minutes[aircraftIndex] = (uint8_t)atoi(strMinutes);
						
						Serial_printf("Aircraft %d time set to %.2d:%.2d.\n",	aircraftIndex,
																		ptrFlightData->Hours[aircraftIndex],
																		ptrFlightData->Minutes[aircraftIndex]	);
					break;

					case REMAINING_TIME_INDEX:
						ptrFlightData->RemainingTime[aircraftIndex] = (uint8_t)atoi(lineBufferPtr);
						Serial_printf("ptrFlightData->RemainingTime[%d] = %d\n", aircraftIndex, ptrFlightData->RemainingTime[aircraftIndex]);
					break;
					
					
					default:
					break;
				}
				
				lineBufferPtr = strtok(NULL,";");
				i++;
			}
			
			ptrFlightData->State[aircraftIndex] = STATE_IDLE;
			aircraftIndex++;
		}
		
		buffer = strtok_r(NULL,"\n",&pltBufferSavePtr);
	}
	
	ptrFlightData->nAircraft = aircraftIndex;	//Set total number of aircraft used
	ptrFlightData->ActiveAircraft = 0;
	
	Serial_printf("Number of aircraft parsed: %d\n",ptrFlightData->nAircraft);
	
	return true;
}

void PltParserResetBuffers(TYPE_FLIGHT_DATA* ptrFlightData)
{
	uint8_t i;
	
	for(i = 0; i < GAME_MAX_AIRCRAFT ; i++)
	{
		memset(ptrFlightData->strFlightNumber[i],'\0',GAME_MAX_CHARACTERS);
	}
	
	memset(ptrFlightData->FlightDirection,DEPARTURE,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->Passengers,0,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->Hours,0,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->Minutes,0,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->State,STATE_IDLE,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->Parking,0,GAME_MAX_AIRCRAFT);
	memset(ptrFlightData->Finished,0,GAME_MAX_AIRCRAFT);
}

uint8_t* PltParserGenerateFile(TYPE_PLT_CONFIG* ptrPltConfig)
{
	enum
	{
		MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_EASY = 3,
		MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_EASY = 45,
		MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_MEDIUM = 20,
		MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_MEDIUM = 30,
		MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_HARD = 10,
		MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_HARD = 20,
	};

	enum
	{
		MIN_AIRCRAFT_EASY = 5,
		MAX_AIRCRAFT_EASY = 10,
		MIN_AIRCRAFT_MEDIUM = 10,
		MAX_AIRCRAFT_MEDIUM = GAME_MAX_AIRCRAFT >> 1,
		MIN_AIRCRAFT_HARD = 20,
		MAX_AIRCRAFT_HARD = GAME_MAX_AIRCRAFT,
	};

	typedef enum t_Hour
	{
		MIN_HOUR = 0,
		MAX_HOUR = 23
	}TYPE_HOUR;

	typedef enum t_Minute
	{
		MIN_MINUTE = 0,
		MAX_MINUTE = 59
	}TYPE_MINUTE;

	uint8_t* PltBuffer = SystemGetBufferAddress();
	uint8_t minAircraftTime;
	uint8_t maxAircraftTime;
	uint8_t nAircraft;
	uint8_t i;
	uint8_t j;
	TYPE_HOUR InitialHour;
	TYPE_MINUTE InitialMinutes;
	char auxBuffer[32] = {'\0'};

	switch(ptrPltConfig->Level)
	{
		case LEVEL_DIFFICULTY_EASY:
			minAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_EASY;
			maxAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_EASY;
			nAircraft = SystemRand(MIN_AIRCRAFT_EASY, MAX_AIRCRAFT_EASY);
		break;

		case LEVEL_DIFFICULTY_MEDIUM:
			minAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_MEDIUM;
			maxAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_MEDIUM;
			nAircraft = SystemRand(MIN_AIRCRAFT_MEDIUM, MAX_AIRCRAFT_MEDIUM);
		break;

		case LEVEL_DIFFICULTY_HARD:
			minAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MIN_SECONDS_HARD;
			maxAircraftTime = MIN_AIRCRAFT_TIMELAPSE_MAX_SECONDS_HARD;
			nAircraft = SystemRand(MIN_AIRCRAFT_HARD, MAX_AIRCRAFT_HARD);
		break;

		default:
			Serial_printf("PltParserGenerateFile(): Undefined level!\n");
		return NULL;
	}

	SystemClearBuffer();

	// At this point, PltBuffer is filled with zeros. Start generating PLT file.

	InitialHour = SystemRand(MIN_HOUR, MAX_HOUR);

	InitialMinutes = SystemRand(MIN_MINUTE, MAX_MINUTE);

	snprintf(auxBuffer, 32, "%d:%d\n", InitialHour, InitialMinutes);

	for(i = 0; auxBuffer[i] != '\0'; i++)
	{
		// Transfer contents generated from snprintf to main buffer.
		PltBuffer[i] = auxBuffer[i];
	}

	Serial_printf("%s\n", PltBuffer);
	DEBUG_PRINT_VAR(ptrPltConfig->Level);
	DEBUG_PRINT_VAR(nAircraft);
	DEBUG_PRINT_VAR(minAircraftTime);
	DEBUG_PRINT_VAR(maxAircraftTime);

	for(j = 0; j < nAircraft; j++)
	{
		uint8_t dep_arr_rand = SystemRand(0,100);
		
		if(dep_arr_rand < 50)
		{
			// Set departure flight

			memcpy(&PltBuffer[i], "DEPARTURE", sizeof("DEPARTURE") * sizeof(uint8_t) );
			i += strlen("DEPARTURE");
		}
		else
		{
			// Set arrival flight
			memcpy(&PltBuffer[i], "ARRIVAL", sizeof("ARRIVAL") * sizeof(uint8_t) );
			i += strlen("ARRIVAL");
		}
	}

	return PltBuffer;
}

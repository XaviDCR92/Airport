#ifndef SERIAL_HEADER__
#define SERIAL_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "Gfx.h"
#include "Font.h"

/* *************************************
 * 	Defines
 * *************************************/

#define SERIAL_DATA_PACKET_SIZE 8
#define ACK_BYTE_STRING "b"

/* *************************************
 * 	Global prototypes
 * *************************************/

void SerialInit(void);
bool SerialRead(uint8_t* ptrArray, size_t nBytes);
bool SerialWrite(void* ptrArray, size_t nBytes);
volatile bool SerialIsBusy(void);

#ifdef SERIAL_INTERFACE
void Serial_printf(const char* str, ...);
#endif // SERIAL_INTERFACE

#endif // SERIAL_HEADER__

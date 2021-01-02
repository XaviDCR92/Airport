#ifndef GLOBAL_INC__H__
#define GLOBAL_INC__H__

/* *************************************
 * 	Includes
 * *************************************/
#include <psx.h>
#include <stdio.h>
#include <psxsio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <fixmath.h>
#include <limits.h>
#include "Serial.h"

/* *************************************
 * 	Defines
 * *************************************/

#ifndef SERIAL_INTERFACE

#define Serial_printf printf

#endif // SERIAL_INTERFACE

#define REFRESH_FREQUENCY 50 //50 Hz PAL / 60 Hz NTSC
#define DEBUG_PRINT_VAR(var) Serial_printf(#var " = %d\n", var);

#endif // GLOBAL_INC__H__

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
#include "Serial.h"

/* *************************************
 * 	Defines
 * *************************************/

#ifndef SERIAL_INTERFACE

#define Serial_printf dprintf

#endif // SERIAL_INTERFACE

#define REFRESH_FREQUENCY 50 //50 Hz PAL / 60 Hz NTSC
#define DEBUG_PRINT_VAR(var) Serial_printf(#var " = %d\n", var);

#ifndef bool
typedef enum t_bool
{
    false = 0,
    true = 1
}bool;
#endif

#if (PSXSDK_VERSION != 0x0599)
#error "Wrong PSXSDK version! Please use version 0.5.99."
#endif

/* Test for GCC == 5.2.0 */
#if ( (__GNUC__ != 5) || (__GNUC_MINOR__ != 2) || (__GNUC_PATCHLEVEL__ != 0) )
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#pragma message (   "******** You are using GCC version number: " STR(__GNUC__)   \
                    "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__) " ********")
#error "Wrong GCC version! Please use version 5.2.0."
#endif

#endif // GLOBAL_INC__H__

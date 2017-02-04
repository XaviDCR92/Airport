#ifndef __GLOBAL_INC__H__
#define __GLOBAL_INC__H__

/* *************************************
 * 	Includes
 * *************************************/

#include <psx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <fixmath.h>

/* *************************************
 * 	Defines
 * *************************************/

#define REFRESH_FREQUENCY 50 //50 Hz PAL / 60 Hz NTSC

#ifndef bool
	typedef enum
	{
		false = 0,
		true = 1
	}bool;
#endif

#if (PSXSDK_VERSION != 0x0599)
#error "Wrong PSXSDK version! Please use version 0.5.99."
#endif

/* Test for GCC > 5.2.0 */
#if ( (__GNUC__ != 5) || (__GNUC_MINOR__ != 2) || (__GNUC_PATCHLEVEL__ != 0) )
#error "Wrong GCC version! Please use version 5.2.0."
#endif

#endif // __GLOBAL_INC__H__

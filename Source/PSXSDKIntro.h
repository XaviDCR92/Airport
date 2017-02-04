#ifndef __PSXSDK_INTRO_HEADER__
#define __PSXSDK_INTRO_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "Gfx.h"

/* *************************************
 * 	Defines
  *************************************/

/* *************************************
 * 	Global prototypes
 * *************************************/

void PSXSDKIntro(void);

/* *************************************
 * 	Global variables
 * *************************************/

extern GsSprite PsxDisk;
extern GsSprite PSXSDKIntroFont;
extern GsSprite GPL_Logo;
extern GsSprite OpenSource_Logo;
extern SsVag TrayClSnd;
extern SsVag SpinDiskSnd;

#endif //__PSXSDK_INTRO_HEADER__

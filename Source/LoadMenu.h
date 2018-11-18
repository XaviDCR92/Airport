#ifndef LOAD_MENU_HEADER__
#define LOAD_MENU_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"

/* *************************************
 * 	Defines
 * *************************************/

/* *************************************
 * 	Global prototypes
 * *************************************/

void LoadMenu(	const char*	const fileList[],
				void* const dest[],
				uint8_t szFileList	, uint8_t szDestList);

void LoadMenuEnd(void);

#endif //LOAD_MENU_HEADER__

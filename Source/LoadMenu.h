#ifndef __LOAD_MENU_HEADER__
#define __LOAD_MENU_HEADER__

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

void LoadMenu(	char*	fileList[], 
				void* dest[],
				uint8_t szFileList	, uint8_t szDestList);
				
void LoadMenuEnd(void);

#endif //__LOAD_MENU_HEADER__

#ifndef __FONT_HEADER__
#define __FONT_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/

#define FONT_DEFAULT_CHAR_SIZE 16
#define FONT_DEFAULT_INIT_CHAR '!'

/* **************************************
 * 	Structs and enums					*
 * *************************************/

/* *************************************
 * 	Global prototypes
 * *************************************/

bool FontLoadImage(char* strPath, TYPE_FONT * ptrFont);
void FontSetSize(TYPE_FONT * ptrFont, short size);
void FontPrintText(TYPE_FONT *ptrFont, short x, short y, char* str, ...);
void FontSetInitChar(TYPE_FONT * ptrFont, char c);
void FontSetFlags(TYPE_FONT * ptrFont, FONT_FLAGS flags);
void FontCyclic(void);
void FontSetSpacing(TYPE_FONT* ptrFont, short spacing);

/* *************************************
 * 	Global variables
 * *************************************/

TYPE_FONT RadioFont;
TYPE_FONT SmallFont;

#endif //__FONT_HEADER__

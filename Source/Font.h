#ifndef FONT_HEADER__
#define FONT_HEADER__

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

bool FontLoadImage(const char* strPath, TYPE_FONT * ptrFont);
void FontSetSize(TYPE_FONT * ptrFont, short size);
void FontPrintText(TYPE_FONT *ptrFont, short x, short y, const char* str, ...);
void FontSetInitChar(TYPE_FONT * ptrFont, char c);
void FontSetFlags(TYPE_FONT * ptrFont, FONT_FLAGS flags);
void FontCyclic(void);
void FontSetSpacing(TYPE_FONT* ptrFont, short spacing);
void FontSetMaxCharPerLine(TYPE_FONT* ptrFont, uint8_t max);

/* *************************************
 * 	Global variables
 * *************************************/

extern TYPE_FONT RadioFont, SmallFont;

#endif //FONT_HEADER__

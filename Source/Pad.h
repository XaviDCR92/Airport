#ifndef PAD_HEADER__
#define PAD_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/

#define PAD_ALWAYS_REPEAT 1

/* *************************************
 * 	Global prototypes
 * *************************************/

void PadInit(void);
void PadClearData(void);

bool PadOneConnected(void);
bool PadTwoConnected(void);

unsigned char PadOneGetType(void);
unsigned char PadTwoGetType(void);

unsigned char PadOneGetID(void);
unsigned char PadTwoGetID(void);

bool PadOneAnyKeyPressed(void);
bool PadTwoAnyKeyPressed(void);

bool PadOneKeyPressed(unsigned short key);
bool PadTwoKeyPressed(unsigned short key);

bool PadOneKeyRepeat(unsigned short key, uint8_t time);
bool PadTwoKeyRepeat(unsigned short key, uint8_t time);

bool PadOneKeyReleased(unsigned short key);
bool PadTwoKeyReleased(unsigned short key);

bool PadOneKeySinglePress(unsigned short key);
bool PadTwoKeySinglePress(unsigned short key);

unsigned short PadOneGetLastKeySinglePressed(void);
unsigned short PadTwoGetLastKeySinglePressed(void);

bool PadOneDirectionKeyPressed(void);
bool PadTwoDirectionKeyPressed(void);

bool PadOneDirectionKeyReleased(void);
bool PadTwoDirectionKeyReleased(void);

bool UpdatePads(void);
bool PadOneIsVibrationEnabled(void);
bool PadTwoIsVibrationEnabled(void);

unsigned short PadOneGetRawData(void);
unsigned short PadTwoGetRawData(void);

bool PadAddCheat(const TYPE_CHEAT* const cheat);

unsigned short* PadGetPlayerOneCheatArray(void);

// Experimental (to be removed)
unsigned short* PadOneGetAddress(void);

#endif //PAD_HEADER__

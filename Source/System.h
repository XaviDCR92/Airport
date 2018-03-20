#ifndef SYSTEM_HEADER__
#define SYSTEM_HEADER__

/* **************************************
 * 	Includes							*
 * **************************************/
#include "Global_Inc.h"
#include "GameStructures.h"

/* **************************************
 * 	Defines								*
 * **************************************/
#define TIMER_PRESCALER_1_SECOND    10
#define TIMER_PRESCALER_1_MINUTE    (TIMER_PRESCALER_1_SECOND * 60)

/* **************************************
 * 	Global Prototypes					*
 * **************************************/
// Calls PSXSDK init routines
void SystemInit(void);

// Sets default VSync (only sets VBlank flag)
void ISR_SystemDefaultVBlank(void);

// Calls srand() using current global_timer value as seed
void SystemSetRandSeed(void);

// Returns VSync flag value
bool SystemRefreshNeeded(void);

// Loads a file into system's internal buffer
bool SystemLoadFile(char*fname);

// Loads a file into desired buffer
bool SystemLoadFileToBuffer(char* fname, uint8_t* buffer, uint32_t szBuffer);

// Clears VSync flag after each frame
void SystemDisableScreenRefresh(void);

// Returns file buffer address
uint8_t* SystemGetBufferAddress(void);

// Tells whether srand() has been called using a pseudo-random value
bool SystemIsRandSeedSet(void);

// To be called from GfxDrawScene after each cycle
void SystemRunTimers(void);

// 1 cycle-length flag with a frequency of 1 Hz
bool System1SecondTick(void);

// 1 cycle-length flag with a frequency of 2 Hz
bool System500msTick(void);

// 1 cycle-length flag with a frequency of 10 Hz
bool System100msTick(void);

// Returns random value between given minimum and maximum values
uint32_t SystemRand(uint32_t min, uint32_t max);

// Increases global timer by 1 step
void SystemIncreaseGlobalTimer(void);

// Sets value to emergency mode flag
void SystemSetEmergencyMode(bool value);

// Returns emergency mode flag state
bool SystemGetEmergencyMode(void);

volatile uint64_t SystemGetGlobalTimer(void);

// Returns whether critical section of code is being entered
volatile bool SystemIsBusy(void);

// Returns whether indicated value is contained inside buffer
bool SystemContains_u8(uint8_t value, uint8_t* buffer, size_t sz);

// Overload for uint16_t
bool SystemContains_u16(uint16_t value, uint16_t* buffer, size_t sz);

// Compares two arrays of unsigned short type.
bool SystemArrayCompare(unsigned short* arr1, unsigned short* arr2, size_t sz);

// Prints stack pointer address using Serial_printf()
void SystemPrintStackPointerAddress(void);

// Checks if a 32-bit pattern set at the end of the stack has been
// accidentally modified by program flow.
void SystemCheckStack(void);

// Looks for string "str" inside a string array pointed to by "array".
// Returns index inside string array on success, -1 if not found.
int32_t SystemIndexOfStringArray(char* str, char** array);

// Function overload for uint16_t data type.
int32_t SystemIndexOf_U16(uint16_t value, uint16_t* array, uint32_t sz);

// Function overload for uint8_t data type.
int32_t SystemIndexOf_U8(uint8_t value, uint8_t* array, uint32_t from, uint32_t sz);

// Returns frames per second.
volatile uint8_t SystemGetFPS(void);

// Increase temp_fps in order to calculate frame rate.
void SystemAcknowledgeFrame(void);

void SystemCyclicHandler(void);

void SystemClearFileBuffer(void);

void SystemEnableVBlankInterrupt(void);

void SystemDisableVBlankInterrupt(void);

void SystemEnableRCnt2Interrupt(void);

void SystemDisableRCnt2Interrupt(void);

void SystemReturnToLoader(void);

void SystemDevMenuToggle(void);

void SystemDevMenu(void);

void SystemCalculateSine(void);

unsigned char SystemGetSineValue(void);

// Fills str with only file name and extension given input file path.
// For example: "FOLDER\\FOLDER\\MYFILE.EXT;1" -> "MYFILE.EXT"
void SystemGetFileBasename(const char* fileName, char* str, const size_t sz);

/* **************************************
 * 	Global Variables					*
 * **************************************/
#endif //SYSTEM_HEADER__

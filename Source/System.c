/* *************************************
 * 	Includes
 * *************************************/

#include "System.h"
#include "Pad.h"
#include "Menu.h"
#include "Gfx.h"
#include "MemCard.h"
#include "EndAnimation.h"
#include "Timer.h"

/* *************************************
 * 	Defines
 * *************************************/

#define FILE_BUFFER_SIZE 0x20014

#define END_STACK_PATTERN (uint32_t) 0x18022015
#define BEGIN_STACK_ADDRESS (uint32_t*) 0x801FFF00
#define STACK_SIZE 0x1000
#define I_MASK (*(volatile unsigned int*)0x1F801074)

/* *************************************
 * 	Local Prototypes
 * *************************************/
 
static void SystemCheckTimer(bool* timer, uint64_t* last_timer, uint8_t step);
static void SystemSetStackPattern(void);
static void ISR_RootCounter2(void);

/* *************************************
 * 	Local Variables
 * *************************************/

//Buffer to store any kind of files. It supports files up to 128 kB
static uint8_t file_buffer[FILE_BUFFER_SIZE];
//Global timer (called by interrupt)
static volatile uint64_t global_timer;
//Tells whether rand seed has been set
static bool rand_seed;
//Screen refresh flag (called by interrupt)
static volatile bool refresh_needed;
// Frames per second measurement
static volatile uint8_t fps;
static volatile uint8_t temp_fps;
//Timers
static bool one_second_timer;
static bool hundred_ms_timer;
static bool five_hundred_ms_timer;
//Emergency mode flag. Toggled on pad connected/disconnected
static bool emergency_mode;
//Critical section is entered (i.e.: when accessing fopen() or other BIOS functions
static volatile bool system_busy;
// When true, it draws a rectangle on top of all primitives with
// information for development purposes.
static bool devmenu_flag;
// Used for sine-like effect.
static unsigned char sine_counter;

/* *******************************************************************
 * 
 * @name: void SystemInit(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief: Calls main intialization routines.
 * 
 * @remarks: To be called before main loop.
 * 
 * *******************************************************************/

void SystemInit(void)
{
	//Reset global timer
	global_timer = 0;
	//Reset 1 second timer
	one_second_timer = 0;
    
	//PSXSDK init
#ifdef SERIAL_INTERFACE
    // PSX_INIT_SAVESTATE | PSX_INIT_CD flags are not needed
    // when coming from OpenSend.
    PSX_InitEx(0);
#else // SERIAL_INTERFACE
    PSX_InitEx(PSX_INIT_SAVESTATE | PSX_INIT_CD);
#endif // SERIAL_INTERFACE

	//Graphics init
	GsInit();
	//Clear VRAM
	GsClearMem();
	//Set Video Resolution
#ifdef _PAL_MODE_
	GsSetVideoMode(X_SCREEN_RESOLUTION, Y_SCREEN_RESOLUTION, VMODE_PAL);
#else
	GsSetVideoMode(X_SCREEN_RESOLUTION, Y_SCREEN_RESOLUTION, VMODE_NTSC);
#endif //_PAL_MODE_
	//SPU init
	SsInit();
	//Reset all user-handled timers
	TimerReset();
	//Pads init
	PadInit();
	//Set Drawing Environment
	GfxInitDrawEnv();
	//Set Display Environment
	GfxInitDispEnv();
	//Set VBlank Handler for screen refresh
	SetVBlankHandler(&ISR_SystemDefaultVBlank);
	//Set Primitive List
	GfxSetPrimitiveList();
	// Init memory card
	MemCardInit();
	//Initial value for system_busy
	system_busy = false;
    //Development menu flag
    devmenu_flag = false;
    //Emergency mode flag
    emergency_mode = false;
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	SystemSetStackPattern();
	
	//SetRCntHandler(&ISR_RootCounter2, 2, 0xA560);

    Serial_printf("Begin SetRCntHandler\n");
	SetRCntHandler(&ISR_RootCounter2, 2, 0xFFFF);
    Serial_printf("End SetRCntHandler\n");

    SystemEnableRCnt2Interrupt();
}

static volatile uint16_t u16_0_01seconds_cnt;

void ISR_RootCounter2(void)
{
    Serial_printf("YO\n");
    u16_0_01seconds_cnt++;
}

/* *******************************************************************
 * 
 * @name: void SystemSetRandSeed(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 	Calls srand() while avoiding multiple calls by setting internal
 *	variable rand_seed to true. Internal variable "global_timer" is
 *	used to generate the new seed.
 * 
 * @remarks:
 * 	It is recommended to call it once user has pressed any key.
 * 
 * *******************************************************************/

void SystemSetRandSeed(void)
{
	if(rand_seed == false)
	{
		rand_seed = true;
		//Set random seed using global timer as reference
		srand((unsigned int)global_timer);
		
		Serial_printf("Seed used: %d\n",(unsigned int)global_timer);
	}
}

/* *******************************************************************
 * 
 * @name: bool SystemIsRandSeedSet(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 	Reportedly, returns whether rand seed has already been set.
 * 
 * @remarks:
 *
 * @return:
 *	 Reportedly, returns whether rand seed has already been set.
 * 
 * *******************************************************************/

bool SystemIsRandSeedSet(void)
{
	return rand_seed;
}

/* *******************************************************************
 * 
 * @name: bool SystemRefreshNeeded(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 
 * @remarks:
 *
 * @return:
 *	 Returns whether VSync flag has been enabled.
 * 
 * *******************************************************************/

bool SystemRefreshNeeded(void)
{
	return refresh_needed;
}

/* *******************************************************************
 * 
 * @name: void ISR_SystemDefaultVBlank(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 
 * @remarks:
 * 	Called from VSync interrupt. Called 50 times a second in PAL mode,
 * 	60 times a second in NTSC mode.
 * 
 * *******************************************************************/

void ISR_SystemDefaultVBlank(void)
{
    if(System1SecondTick() == true)
    {
        fps = temp_fps;
        temp_fps = 0;
    }
    
	refresh_needed = true;
}

/* *******************************************************************
 * 
 * @name: void SystemAcknowledgeFrame(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 
 * @remarks:
 * 	Called by Game module in order to calculate frames per second.
 * 
 * *******************************************************************/

void SystemAcknowledgeFrame(void)
{
    temp_fps++;
}

/* *******************************************************************
 * 
 * @name: void SystemAcknowledgeFrame(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 *  Creates a sine-line (more exactly, a parabola-like) effect and
 *  stores its value into a variable.
 * 
 * @remarks:
 * 	To be called only once, preferibly on SystemCyclic().
 * 
 * *******************************************************************/

void SystemCalculateSine(void)
{
    enum
    {
        SINE_EFFECT_STEP = 24,
        SINE_EFFECT_MAX = 240
    };

	static bool sine_decrease = false;    

    if(sine_decrease == false)
    {
        if(sine_counter < SINE_EFFECT_MAX)
        {
            sine_counter += SINE_EFFECT_STEP;
        }
        else
        {
            sine_decrease = true;
        }
    }
    else
    {
        if(sine_counter > SINE_EFFECT_STEP)
        {
            sine_counter -= SINE_EFFECT_STEP;
        }
        else
        {
            sine_decrease = false;
        }
    }
}

/* *******************************************************************
 * 
 * @name: unsigned char SystemGetSineValue(void)
 * 
 * @author: Xavier Del Campo
 *
 * @return:
 *  Returns a value which oscillates like a sine (more exactly, like
 *  a parabola) function to be used wherever you want.
 * 
 * *******************************************************************/

unsigned char SystemGetSineValue(void)
{
    return sine_counter;
}

/* *******************************************************************
 * 
 * @name: void SystemIncreaseGlobalTimer(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:
 * 	Increases internal variable responsible for time handling.
 * 
 * @remarks:
 * 	Usually called from ISR_SystemDefaultVBlank().
 * 
 * *******************************************************************/

void SystemIncreaseGlobalTimer(void)
{
	global_timer++;
}

/* *******************************************************************
 * 
 * @name: uint64_t SystemGetGlobalTimer(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief: Returns internal global timer value.
 * 
 * *******************************************************************/

uint64_t SystemGetGlobalTimer(void)
{
	return global_timer;
}

/* *******************************************************************
 * 
 * @name: void SystemDisableScreenRefresh(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief: Resets VBlank IRQ flag.
 * 
 * *******************************************************************/

void SystemDisableScreenRefresh(void)
{
	refresh_needed = false;
}

/* *******************************************************************
 * 
 * @name: bool System1SecondTick(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @return: bool variable with a 1-cycle-length pulse that gets
 * 			set each second.
 * 
 * *******************************************************************/

bool System1SecondTick(void)
{
	return one_second_timer;
}

/* *******************************************************************
 * 
 * @name: bool System100msTick(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @return: bool variable with a 1-cycle-length pulse that gets
 * 			set every 100 milliseconds.
 * 
 * *******************************************************************/

bool System100msTick(void)
{
	return hundred_ms_timer;
}

/* *******************************************************************
 * 
 * @name	bool System500msTick(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @return: bool variable with a 1-cycle-length pulse that gets
 * 			set every 500 milliseconds.
 * 
 * *******************************************************************/

bool System500msTick(void)
{
	return five_hundred_ms_timer;
}

/* *******************************************************************
 * 
 * @name	void SystemRunTimers(void)	
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	general timer handler
 *
 * @remarks:	1 second, 500 ms and 100 ms ticks get updated here.
 * 
 * *******************************************************************/

void SystemRunTimers(void)
{
	static uint64_t last_one_second_tick;
	static uint64_t last_100_ms_tick;
	static uint64_t last_500_ms_tick;

	SystemCheckTimer(&one_second_timer, &last_one_second_tick, REFRESH_FREQUENCY);
	
#ifdef _PAL_MODE_
	SystemCheckTimer(&hundred_ms_timer, &last_100_ms_tick, 2 /* 2 * 50 ms = 100 ms */);
	SystemCheckTimer(&five_hundred_ms_timer, &last_500_ms_tick, 10 /* 10 * 50 ms = 500 ms */);
#else // _PAL_MODE_
	SystemCheckTimer(&hundred_ms_timer, &last_100_ms_tick, 3);
#endif // _PAL_MODE_
	
}

/* ********************************************************************************
 * 
 * @name	void SystemCheckTimer(bool* timer, uint64_t* last_timer, uint8_t step)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Checks if needed time step has been elapsed. If true, flag gets set.
 * 
 * *******************************************************************************/
 
void SystemCheckTimer(bool* timer, uint64_t* last_timer, uint8_t step)
{
	if(*timer == true)
	{
		*timer = false;
	}

	if(global_timer >= (*last_timer + step) )
	{
		*timer = true;
		*last_timer = global_timer;
	}
}

/* ****************************************************************************************
 * 
 * @name	bool SystemLoadFileToBuffer(char* fname, uint8_t* buffer, uint32_t szBuffer)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Given an input path, it fills a buffer pointed to by "buffer" with
 * 			maximum size "szBuffer" with data from CD-ROM.
 *
 * @return:	true if file has been loaded successfully, false otherwise.
 * 
 * ****************************************************************************************/

bool SystemLoadFileToBuffer(char* fname, uint8_t* buffer, uint32_t szBuffer)
{
#ifdef SERIAL_INTERFACE
	uint8_t fileSizeBuffer[sizeof(uint32_t)] = {0};
    uint32_t i;
#else // SERIAL_INTERFACE
    FILE *f;
#endif // SERIAL_INTERFACE
	int32_t size = 0;
	
	// Wait for possible previous operation from the GPU before entering this section.
	while( (SystemIsBusy() == true) || (GfxIsGPUBusy() == true) );
	
	if(fname == NULL)
	{
		Serial_printf("SystemLoadFile: NULL fname!\n");
		return false;
	}
	
	memset(buffer,0,szBuffer);

#ifdef SERIAL_INTERFACE
    Serial_printf("#%s@", fname);

    SerialRead(fileSizeBuffer, sizeof(uint32_t) );

    for(i = 0; i < sizeof(uint32_t); i++)
    {
        size |= fileSizeBuffer[i] << (i << 3); // (i << 3) == (i * 8)
    }

    SerialWrite(ACK_BYTE_STRING, 1);

    for(i = 0; i < size; i += SERIAL_DATA_PACKET_SIZE)
    {
        uint32_t bytes_to_read;

        // Read actual EXE data into proper RAM address.

        if( (i + SERIAL_DATA_PACKET_SIZE) >= size)
        {
            bytes_to_read = size - i;
        }
        else
        {
            bytes_to_read = SERIAL_DATA_PACKET_SIZE;
        }

        SerialRead(file_buffer + i, bytes_to_read);

        SerialWrite(ACK_BYTE_STRING, sizeof(uint8_t)); // Write ACK
    }
#else // SERIAL_INTERFACE

    system_busy = true;

    SystemDisableVBlankInterrupt();

    f = fopen(fname, "r");
	
	if(f == NULL)
	{
		Serial_printf("SystemLoadFile: file could not be found!\n");
		//File couldn't be found
		return false;
	}

	fseek(f, 0, SEEK_END);

	size = ftell(f);
	
	if(size > szBuffer)
	{
		Serial_printf("SystemLoadFile: Exceeds file buffer size (%d bytes)\n",size);
		//Bigger than 128 kB (buffer's max size)
		return false;
	}
	
	fseek(f, 0, SEEK_SET); //f->pos = 0;
	
	fread(buffer, sizeof(char), size, f);
	
	fclose(f);

    SystemEnableVBlankInterrupt();

    system_busy = false;

#endif // SERIAL_INTERFACE
	
	Serial_printf("File \"%s\" loaded successfully!\n",fname);
	
	return true;
}

/* ****************************************************************************************
 * 
 * @name	bool SystemLoadFile(char*fname)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Given an input file name, it loads its conents into internal buffer.
 *
 * @return:	true if file has been loaded successfully, false otherwise.
 * 
 * ****************************************************************************************/

bool SystemLoadFile(char*fname)
{
	return SystemLoadFileToBuffer(fname,file_buffer,sizeof(file_buffer));
}

/* ******************************************************************
 * 
 * @name	uint8_t* SystemGetBufferAddress(void)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	Reportedly, returns internal buffer initial address.
 * 
 * *****************************************************************/

uint8_t* SystemGetBufferAddress(void)
{
	return file_buffer;
}

/* ******************************************************************
 * 
 * @name	void SystemClearBuffer(void)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	Fills internal buffer with zeros
 * 
 * *****************************************************************/

void SystemClearBuffer(void)
{
	memset(file_buffer, 0, sizeof(file_buffer));
}

/* ******************************************************************
 * 
 * @name	void SystemWaitCycles(uint32_t cycles)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	halts program execution for n-"cycles"
 * 
 * *****************************************************************/

void SystemWaitCycles(uint32_t cycles)
{
	uint64_t currentTime = global_timer;
	
	while(global_timer < (currentTime + cycles) );
}

/* ******************************************************************
 * 
 * @name	uint32_t SystemRand(uint32_t min, uint32_t max)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	random number between "min" and "max".
 *
 * @remarks: rand seed must be set before using this function, or
 * 			 you will predictable values otherwise!
 * 
 * *****************************************************************/

uint32_t SystemRand(uint32_t min, uint32_t max)
{
    if(rand_seed == false)
    {
        Serial_printf("Warning: calling rand() before srand()\n");
    }
    
	return rand() % (max - min + 1) + min;
}

/* ***********************************************************************
 * 
 * @name	void SystemSetEmergencyMode(bool value)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	Sets emergency mode flag.
 *
 * @remarks: emergency mode is set once that a controller is unplugged.
 * 
 * ***********************************************************************/

void SystemSetEmergencyMode(bool value)
{
	emergency_mode = value;
}

/* ***********************************************************************
 * 
 * @name	bool SystemGetEmergencyMode(void)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	returns emergency mode flag.
 * 
 * ***********************************************************************/

bool SystemGetEmergencyMode(void)
{
	return emergency_mode;
}

/* ***********************************************************************
 * 
 * @name	volatile bool SystemIsBusy(void)
 * 
 * @author: Xavier Del Campo
 *
 * @return:	returns system busy flag.
 * 
 * ***********************************************************************/

volatile bool SystemIsBusy(void)
{
	return system_busy;
}

/* ****************************************************************************
 * 
 * @name	bool SystemContains_u8(uint8_t value, uint8_t* buffer, size_t sz)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	checks if a certain value is contained in a buffer with size "sz".
 * 
 * @return:	true if value is contained inside buffer, false otherwise.
 * 
 * ****************************************************************************/

bool SystemContains_u8(uint8_t value, uint8_t* buffer, size_t sz)
{
	size_t i = 0;
	
	for(i = 0; i < sz; i++)
	{
		if(buffer[i] == value)
		{
			return true;
		}
	}
	
	return false;
}

/* ****************************************************************************
 * 
 * @name	bool SystemContains_u16(uint16_t value, uint16_t* buffer, size_t sz)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	checks if a certain value is contained in a buffer with size "sz".
 * 			Variant for u16 variables.
 * 
 * @return:	true if value is contained inside buffer, false otherwise.
 * 
 * ****************************************************************************/

bool SystemContains_u16(uint16_t value, uint16_t* buffer, size_t sz)
{
	size_t i = 0;
	
	for(i = 0; i < sz; i++)
	{
		if(buffer[i] == value)
		{
			return true;
		}
	}
	
	return false;
}

/* ****************************************************************************************
 * 
 * @name	bool SystemArrayCompare(unsigned short* arr1, unsigned short* arr2, size_t sz)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Reportedly, it compares two arrays "arr1" and "arr2", with size "sz".
 *
 * @return: true if they are equal, false otherwise.
 * 
 * ****************************************************************************************/

bool SystemArrayCompare(unsigned short* arr1, unsigned short* arr2, size_t sz)
{
	size_t i;
	
	for(i = 0; i < sz; i++)
	{
		if(arr1[i] != arr2[i])
		{
			return false;
		}
	}
	
	return true;
}

/* ****************************************************************************************
 * 
 * @name	void SystemPrintStackPointerAddress(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Prints stack usage in percentage via dprintf calls.
 * 
 * ****************************************************************************************/

void SystemPrintStackPointerAddress(void)
{
#ifdef PSXSDK_DEBUG // Used to avoid unused variable warning
	void* ptr = NULL;
	fix16_t used_bytes = fix16_from_int((int)((void*)BEGIN_STACK_ADDRESS - (void*)&ptr));
	fix16_t stackPercent = fix16_sdiv(used_bytes,fix16_from_int((int)STACK_SIZE));
	
	stackPercent = fix16_smul(stackPercent, fix16_from_int((int)100));
	
	Serial_printf("stackPercent: %d\n", stackPercent);
	
	Serial_printf("Stack begin pointer: 0x%08X\n"
			"Stack pointer address: 0x%08X\n"
			"Used %d%% of stack size.\n"
			"\tUsed bytes: %d\n",
			(void*)BEGIN_STACK_ADDRESS,
			(void*)&ptr,
			fix16_to_int(stackPercent),
			fix16_to_int(used_bytes)	);
#endif // PSXSDK_DEBUG

}

/* ****************************************************************************************
 * 
 * @name	void SystemCheckStack(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Compares stack top with expected byte pattern. If does not match, a stack
 *          overflow has been caused, and application returns to a safe state.
 * 
 * ****************************************************************************************/

void SystemCheckStack(void)
{
	uint32_t * ptrStack = BEGIN_STACK_ADDRESS;
	uint32_t data;
	
	ptrStack -= STACK_SIZE;
	data = (*ptrStack);
	
	if(data != END_STACK_PATTERN)
	{
		Serial_printf("Stack overflow?\n");
		
		while(1);
	}
}


/* ****************************************************************************************
 * 
 * @name	void SystemSetStackPattern(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Sets a determined byte pattern on stack top to detect possible stack
 *          overflow during execution.
 * 
 * ****************************************************************************************/

void SystemSetStackPattern(void)
{
	uint32_t * ptrStack = BEGIN_STACK_ADDRESS;
	
	ptrStack -= STACK_SIZE;
	
	*ptrStack = END_STACK_PATTERN;
}

/* ****************************************************************************************
 * 
 * @name	int32_t SystemIndexOfStringArray(char* str, char** array)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Finds string "str" inside an array of strings "array".
 *
 * @return  Index for a string "str" inside "array". -1 if it could not be found.
 * 
 * ****************************************************************************************/

int32_t SystemIndexOfStringArray(char* str, char** array)
{
	int32_t i;
	
	for(i = 0; array[i] != NULL; i++)
	{
		Serial_printf("String to find: %s\nEntry: %s\n", str, array[i]);
		
		if(strcmp(str, array[i]) == 0)
		{
			Serial_printf("Match! Returning index %d...\n", i);
			return i;
		}
	}
	
	return -1;
}

/* ****************************************************************************************
 * 
 * @name	int32_t SystemIndexOf_U16(uint16_t value, uint16_t* array, uint32_t sz)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	For a uint16_t array, it returns index of a variable "value" inside an array.
 *
 * @return  Index for a variable "value" inside "array". -1 if it could not be found.
 * 
 * ****************************************************************************************/

int32_t SystemIndexOf_U16(uint16_t value, uint16_t* array, uint32_t sz)
{
	int32_t i;
	
	for(i = 0; i < sz; i++)
	{
		if(value == array[i])
		{
			return i;
		}
	}
	
	return -1;
}

/* ****************************************************************************************
 * 
 * @name	int32_t SystemIndexOf_U8(uint8_t value, uint8_t* array, uint32_t from, uint32_t sz)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	For a uint8_t array, it returns index of a variable "value" inside an array.
 *          "from" and "size_t" can be used to determine initial/ending positions.
 *
 * @return  Index for a variable "value" inside "array". -1 if it could not be found.
 * 
 * ****************************************************************************************/


int32_t SystemIndexOf_U8(uint8_t value, uint8_t* array, uint32_t from, uint32_t sz)
{
	int32_t i;
	
	for(i = from; i < sz; i++)
	{
		if(value == array[i])
		{
			return i;
		}
	}
	
	return -1;
}

/* ****************************************************************************************
 * 
 * @name	volatile uint8_t SystemGetFPS(void)
 * 
 * @author: Xavier Del Campo
 *          
 * @return: Frames per second
 * 
 * ****************************************************************************************/

volatile uint8_t SystemGetFPS(void)
{
    return fps;
}

/* ****************************************************************************************
 * 
 * @name	void SystemCyclicHandler(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	It calls system handlers once an execution cycle has finished.
 *          
 * 
 * ****************************************************************************************/

void SystemCyclicHandler(void)
{
    UpdatePads();

    SystemIncreaseGlobalTimer();
	
	SystemRunTimers();
	
	TimerHandler();
	
	SystemDisableScreenRefresh();
	
	MemCardHandler();

    SystemCalculateSine();
	
	SystemCheckStack();
}

/* ****************************************************************************************
 * 
 * @name	void SystemDisableVBlankInterrupt(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Reportedly, this routine enables VBLANK interrupt flag.
 *
 * @remark: Used when critical timing is needed or GPU activity is not desired
 *          e.g.: when reading files from CD-ROM.         
 * 
 * ****************************************************************************************/

void SystemDisableVBlankInterrupt(void)
{
	I_MASK &= ~(0x0001);
}

/* ****************************************************************************************
 * 
 * @name	void SystemEnableVBlankInterrupt(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Reportedly, this routine enables VBLANK interrupt flag.
 *          
 * 
 * ****************************************************************************************/

void SystemEnableVBlankInterrupt(void)
{
	I_MASK |= (0x0001);
}

/* ****************************************************************************************
 * 
 * @name	void SystemReturnToLoader(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Deinitializes PSXSDK library and returns to OpenSend loader,
 *          located at memory address 0x801A0000
 * 
 * ****************************************************************************************/

void SystemReturnToLoader(void)
{
    Serial_printf("Returning to loader...\n");

    EndAnimation();
    
    PSX_DeInit();

    __asm__("j 0x801A0000");
}

/* ****************************************************************************************
 * 
 * @name	void SystemDevMenuToggle(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	It toggles a flag called "devmenu_flag" which, if true, shows information on
 *          top of all drawn primitives for debugging/development purposes.
 * 
 * ****************************************************************************************/
 
void SystemDevMenuToggle(void)
{
    devmenu_flag = devmenu_flag? false: true;
}

/* ****************************************************************************************
 * 
 * @name	void SystemEnableRCnt2Interrupt(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Enables bit 6 from I_MASK (0x1F801074)/IRQ6 RCNT2 (System clock / 8)
 * 
 * ****************************************************************************************/

void SystemEnableRCnt2Interrupt(void)
{
    I_MASK |= 1<<6;
}


/* ****************************************************************************************
 * 
 * @name	void SystemDisableRCnt2Interrupt(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Disables bit 6 from I_MASK (0x1F801074)/IRQ6 RCNT2 (System clock / 8)
 * 
 * ****************************************************************************************/

void SystemDisableRCnt2Interrupt(void)
{
    I_MASK &= ~(1<<6);
}

/* ****************************************************************************************
 * 
 * @name	void SystemDevMenu(void)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Shows information on top of all drawn primitives for debugging/development purposes.
 * 
 * ****************************************************************************************/

void SystemDevMenu(void)
{
    enum
    {
        DEVMENU_BG_W = 256,
        DEVMENU_BG_X = (X_SCREEN_RESOLUTION >> 1) - (DEVMENU_BG_W >> 1),
        DEVMENU_BG_Y = 32,
        DEVMENU_BG_H = 128,
        
        DEVMENU_BG_R = 0,
        DEVMENU_BG_G = 128,
        DEVMENU_BG_B = 32,
    };

    enum
    {
        DEVMENU_TEXT_GAP = 8,

        DEVMENU_PAD1_STATUS_TEXT_X = DEVMENU_BG_X + DEVMENU_TEXT_GAP,
        DEVMENU_PAD1_STATUS_TEXT_Y = DEVMENU_BG_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD1_TYPE_TEXT_X = DEVMENU_PAD1_STATUS_TEXT_X,
        DEVMENU_PAD1_TYPE_TEXT_Y = DEVMENU_PAD1_STATUS_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD1_ID_TEXT_X = DEVMENU_PAD1_STATUS_TEXT_X,
        DEVMENU_PAD1_ID_TEXT_Y = DEVMENU_PAD1_TYPE_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD1_RAW_DATA_TEXT_X = DEVMENU_PAD1_ID_TEXT_X,
        DEVMENU_PAD1_RAW_DATA_TEXT_Y = DEVMENU_PAD1_ID_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD2_STATUS_TEXT_X = DEVMENU_PAD1_RAW_DATA_TEXT_X,
        DEVMENU_PAD2_STATUS_TEXT_Y = DEVMENU_PAD1_RAW_DATA_TEXT_Y + (DEVMENU_TEXT_GAP << 1), // Leave a bigger gap here

        DEVMENU_PAD2_TYPE_TEXT_X = DEVMENU_PAD2_STATUS_TEXT_X,
        DEVMENU_PAD2_TYPE_TEXT_Y = DEVMENU_PAD2_STATUS_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD2_ID_TEXT_X = DEVMENU_PAD2_TYPE_TEXT_X,
        DEVMENU_PAD2_ID_TEXT_Y = DEVMENU_PAD2_TYPE_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_PAD2_RAW_DATA_TEXT_X = DEVMENU_PAD2_ID_TEXT_X,
        DEVMENU_PAD2_RAW_DATA_TEXT_Y = DEVMENU_PAD2_ID_TEXT_Y + DEVMENU_TEXT_GAP,

        DEVMENU_ROOTCNT2_TEXT_X = DEVMENU_PAD2_RAW_DATA_TEXT_X,
        DEVMENU_ROOTCNT2_TEXT_Y = DEVMENU_PAD2_RAW_DATA_TEXT_Y + DEVMENU_TEXT_GAP,
    };

    if(devmenu_flag == true)
    {
        GsRectangle devMenuBg = {   .x = DEVMENU_BG_X,
                                    .y = DEVMENU_BG_Y,
                                    .w = DEVMENU_BG_W,
                                    .h = DEVMENU_BG_H,
                                    .r = DEVMENU_BG_R,
                                    .g = DEVMENU_BG_G,
                                    .b = DEVMENU_BG_B,
                                    .attribute = ENABLE_TRANS | TRANS_MODE(0) };

        GsSortRectangle(&devMenuBg);

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD1_STATUS_TEXT_X,
                        DEVMENU_PAD1_STATUS_TEXT_Y,
                        "Pad1 connected = %d",
                        PadOneConnected()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD1_TYPE_TEXT_X,
                        DEVMENU_PAD1_TYPE_TEXT_Y,
                        "Pad1 type = 0x%02X",
                        PadOneGetType()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD1_ID_TEXT_X,
                        DEVMENU_PAD1_ID_TEXT_Y,
                        "Pad1 ID = 0x%02X",
                        PadOneGetID()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD1_RAW_DATA_TEXT_X,
                        DEVMENU_PAD1_RAW_DATA_TEXT_Y,
                        "Pad1 raw data = 0x%04X",
                        PadOneGetRawData()   );

         FontPrintText( &SmallFont,
                        DEVMENU_PAD2_STATUS_TEXT_X,
                        DEVMENU_PAD2_STATUS_TEXT_Y,
                        "Pad2 connected = %d",
                        PadTwoConnected()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD2_TYPE_TEXT_X,
                        DEVMENU_PAD2_TYPE_TEXT_Y,
                        "Pad2 type = 0x%02X",
                        PadTwoGetType()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD2_ID_TEXT_X,
                        DEVMENU_PAD2_ID_TEXT_Y,
                        "Pad2 ID = 0x%02X",
                        PadTwoGetID()   );

        FontPrintText(  &SmallFont,
                        DEVMENU_PAD2_RAW_DATA_TEXT_X,
                        DEVMENU_PAD2_RAW_DATA_TEXT_Y,
                        "Pad2 raw data = 0x%04X",
                        PadTwoGetRawData()   );

        /*FontPrintText(  &SmallFont,
                        DEVMENU_ROOTCNT2_TEXT_X,
                        DEVMENU_ROOTCNT2_TEXT_Y,
                        "Timer2 = 0x%04X",
                        GetRCnt(2) );*/

        FontPrintText(  &SmallFont,
                        DEVMENU_ROOTCNT2_TEXT_X,
                        DEVMENU_ROOTCNT2_TEXT_Y,
                        "Timer2 = 0x%04X, timer2 = 0x%04X",
                        u16_0_01seconds_cnt, GetRCnt(2) );
    }
}

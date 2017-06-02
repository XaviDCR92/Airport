/* *************************************
 * 	Includes
 * *************************************/

#include "System.h"

/* *************************************
 * 	Defines
 * *************************************/

#define FILE_BUFFER_SIZE 0x20014
#define SYSTEM_MAX_TIMERS 16

#define END_STACK_PATTERN (uint32_t) 0x18022015
#define BEGIN_STACK_ADDRESS (uint32_t*) 0x801FFF00
#define STACK_SIZE 0x1000
#define I_MASK (*(unsigned int*)0x1F801074)

/* *************************************
 * 	Local Prototypes
 * *************************************/
 
static void SystemCheckTimer(bool* timer, uint64_t* last_timer, uint8_t step);
static void SystemSetStackPattern(void);
static void SystemEnableVBlankInterrupt(void);
static void SystemDisableVBlankInterrupt(void);

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
//Timers
static bool one_second_timer;
static bool hundred_ms_timer;
static bool five_hundred_ms_timer;
//Emergency mode flag. Toggled on pad connected/disconnected
static bool emergency_mode;
//Critical section is entered (i.e.: when accessing fopen() or other BIOS functions
static volatile bool system_busy;
//Timer array.
static TYPE_TIMER timer_array[SYSTEM_MAX_TIMERS];

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
	PSX_InitEx(PSX_INIT_SAVESTATE | PSX_INIT_CD);
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
	SystemResetTimers();
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
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	SystemSetStackPattern();
	
	SetRCnt(RCntCNT2,0xFFFF,RCntSC);
	StartRCnt(RCntCNT2);
}

/* *******************************************************************
 * 
 * @name: void SystemInit(void)
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
		
		dprintf("Seed used: %d\n",(unsigned int)global_timer);
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
	refresh_needed = true;
	SystemIncreaseGlobalTimer();
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
 * @name	bool SystemLoadFileToBuffer(char * fname, uint8_t* buffer, uint32_t szBuffer)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Given an input path, it fills a buffer pointed to by "buffer" with
 * 			maximum size "szBuffer" with data from CD-ROM.
 *
 * @return:	true if file has been loaded successfully, false otherwise.
 * 
 * ****************************************************************************************/

bool SystemLoadFileToBuffer(char * fname, uint8_t* buffer, uint32_t szBuffer)
{
	FILE *f;
	int32_t size;
	
	// Wait for possible previous operation from the GPU before entering this section.
	while( (SystemIsBusy() == true) || (GfxIsGPUBusy() == true) );
	
	if(fname == NULL)
	{
		dprintf("SystemLoadFile: NULL fname!\n");
		return false;
	}
	
	memset(buffer,0,szBuffer);
	
	system_busy = true;
	
	SystemDisableVBlankInterrupt();
	
	f = fopen(fname, "r");
	
	if(f == NULL)
	{
		dprintf("SystemLoadFile: file could not be found!\n");
		//File couldn't be found
		return false;
	}

	fseek(f, 0, SEEK_END);

	size = ftell(f);
	
	if(size > szBuffer)
	{
		dprintf("SystemLoadFile: Exceeds file buffer size (%d bytes)\n",size);
		//Bigger than 128 kB (buffer's max size)
		return false;
	}
	
	fseek(f, 0, SEEK_SET); //f->pos = 0;
	
	fread(buffer, sizeof(char), size, f);
	
	fclose(f);
	
	SystemEnableVBlankInterrupt();
	
	system_busy = false;
	
	dprintf("File \"%s\" loaded successfully!\n",fname);
	
	return true;
}

/* ****************************************************************************************
 * 
 * @name	bool SystemLoadFile(char *fname)
 * 
 * @author: Xavier Del Campo
 * 
 * @brief:	Given an input file name, it loads its conents into internal buffer.
 *
 * @return:	true if file has been loaded successfully, false otherwise.
 * 
 * ****************************************************************************************/

bool SystemLoadFile(char *fname)
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

/* ********************************************************************************************
 * 
 * @name	TYPE_TIMER* SystemCreateTimer(uint32_t t, bool rf, void (*timer_callback)(void) )
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	fills a TYPE_TIMER structure with input parameters
 *
 * @param:	uint32_t t:
 * 				Timeout value (1 unit = 10 ms)
 * 			bool rf:
 * 				Repeat flag
 * 			void (*timer_callback)(void)
 * 				Function to be called on timeout	
 * 
 * @return:	pointer to TYPE_TIMER structure if filled correctly, NULL pointer otherwise.
 * 
 * ********************************************************************************************/

TYPE_TIMER* SystemCreateTimer(uint32_t t, bool rf, void (*timer_callback)(void) )
{
	bool success = false;
	uint8_t i;
	
	if(t == 0)
	{
		dprintf("Cannot create timer with time == 0!\n");
		return NULL;
	}
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		if(timer_array[i].busy == false)
		{
			timer_array[i].Timeout_Callback = timer_callback;
			timer_array[i].time = t;
			timer_array[i].orig_time = t;
			timer_array[i].repeat_flag = rf;
			timer_array[i].busy = true;
			success = true;
			break;
		}
	}
	
	if(success == false)
	{
		dprintf("Could not find any free timer!\n");
		return NULL;
	}
	
	return &timer_array[i];
}

/* *******************************************
 * 
 * @name	void SystemResetTimers(void)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	reportedly, removes all timers.
 * 
 * *******************************************/

void SystemResetTimers(void)
{
	uint8_t i;
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		SystemTimerRemove(&timer_array[i]);
	}
}

/* *****************************************************
 * 
 * @name	void SystemUserTimersHandler(void)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	reportedly, handles all available timers.
 *
 * @remarks: calls callback on timeout.
 * 
 * *****************************************************/

void SystemUserTimersHandler(void)
{
	uint8_t i;
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		if(timer_array[i].busy == true)
		{
			if(System100msTick() == true)
			{
				timer_array[i].time--;
				
				if(timer_array[i].time == 0)
				{
					timer_array[i].Timeout_Callback();
					
					if(timer_array[i].repeat_flag == true)
					{
						timer_array[i].time = timer_array[i].orig_time;
					}
					else
					{
						// Clean timer data
						timer_array[i].busy = false;
						timer_array[i].orig_time = 0;
						timer_array[i].Timeout_Callback = NULL;
					}
				}
			}
		}
	}
}

/* *********************************************************************
 * 
 * @name	void SystemUserTimersHandler(void)
 * 
 * @author: Xavier Del Campo
 *
 * @brief:	sets time left for TYPE_TIMER instance to initial value.
 *
 * @remarks: specially used when TYPE_TIMER.rf is enabled.
 * 
 * *********************************************************************/

void SystemTimerRestart(TYPE_TIMER* timer)
{
	timer->time = timer->orig_time;
}

void SystemTimerRemove(TYPE_TIMER* timer)
{
	timer->time = 0;
	timer->orig_time = 0;
	timer->Timeout_Callback = NULL;
	timer->busy = false;
	timer->repeat_flag = false;
}

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

void SystemPrintStackPointerAddress(void)
{
#ifdef PSXSDK_DEBUG // Used to avoid unused variable warning
	void * ptr = NULL;
	fix16_t used_bytes = fix16_from_int((int)((void*)BEGIN_STACK_ADDRESS - (void*)&ptr));
	fix16_t stackPercent = fix16_sdiv(used_bytes,fix16_from_int((int)STACK_SIZE));
	
	stackPercent = fix16_smul(stackPercent, fix16_from_int((int)100));
	
	dprintf("stackPercent: %d\n", stackPercent);
	
	dprintf("Stack begin pointer: 0x%08X\n"
			"Stack pointer address: 0x%08X\n"
			"Used %d%% of stack size.\n"
			"\tUsed bytes: %d\n",
			(void*)BEGIN_STACK_ADDRESS,
			(void*)&ptr,
			fix16_to_int(stackPercent),
			fix16_to_int(used_bytes)	);
#endif // PSXSDK_DEBUG

}

void SystemCheckStack(void)
{
	uint32_t * ptrStack = BEGIN_STACK_ADDRESS;
	uint32_t data;
	
	ptrStack -= STACK_SIZE;
	data = (*ptrStack);
	
	if(data != END_STACK_PATTERN)
	{
		dprintf("Stack overflow?\n");
		
		while(1);
	}
}

void SystemSetStackPattern(void)
{
	uint32_t * ptrStack = BEGIN_STACK_ADDRESS;
	
	ptrStack -= STACK_SIZE;
	
	*ptrStack = END_STACK_PATTERN;
}

int32_t SystemIndexOfStringArray(char * str, char ** array)
{
	int32_t i;
	
	for(i = 0; array[i] != NULL; i++)
	{
		dprintf("String to find: %s\nEntry: %s\n", str, array[i]);
		
		if(strcmp(str, array[i]) == 0)
		{
			dprintf("Match! Returning index %d...\n", i);
			return i;
		}
	}
	
	return -1;
}

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

void SystemCyclicHandler(void)
{
	if(UpdatePads() == false)
	{
		SystemSetEmergencyMode(true);
	}
	else
	{
		SystemSetEmergencyMode(false);
	}
	
	SystemRunTimers();
	
	SystemUserTimersHandler();
	
	SystemDisableScreenRefresh();
	
	MemCardHandler();
	
	SystemCheckStack();
}

void SystemDisableVBlankInterrupt(void)
{
	I_MASK &= ~(0x0001);
}

void SystemEnableVBlankInterrupt(void)
{
	I_MASK |= (0x0001);
}

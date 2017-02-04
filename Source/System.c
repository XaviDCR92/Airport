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

/* *************************************
 * 	Local Prototypes
 * *************************************/
 
static void SystemCheckTimer(bool * timer, uint64_t * last_timer, uint8_t step);
static void SystemSetStackPattern(void);

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
static bool system_busy;
//Timer array.
static TYPE_TIMER timer_array[SYSTEM_MAX_TIMERS];

/* *************************************
 * @name: void SystemInit(void)
 * @date: 19/05/2016
 * @author: Xavier Del Campo
 * @brief:
 * *************************************/

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

bool SystemIsRandSeedSet(void)
{
	return rand_seed;
}

bool SystemDMAReady(void)
{
	return (*((unsigned int*)0x1F801814) & 1<<28);
}

bool SystemDMABusy(void)
{
	return !SystemDMAReady();
}

bool SystemRefreshNeeded(void)
{
	return refresh_needed;
}

void ISR_SystemDefaultVBlank(void)
{
	refresh_needed = true;
	SystemIncreaseGlobalTimer();
}

void SystemIncreaseGlobalTimer(void)
{
	global_timer++;
}

uint64_t SystemGetGlobalTimer(void)
{
	return global_timer;
}

void SystemDisableScreenRefresh(void)
{
	refresh_needed = false;
}

bool System1SecondTick(void)
{
	return one_second_timer;
}

bool System100msTick(void)
{
	return hundred_ms_timer;
}

bool System500msTick(void)
{
	return five_hundred_ms_timer;
}

void SystemRunTimers(void)
{
	static uint64_t last_one_second_tick;
	static uint64_t last_100_ms_tick;
	static uint64_t last_500_ms_tick;
	
	SystemCheckTimer(&one_second_timer, &last_one_second_tick, REFRESH_FREQUENCY);
	
#ifdef _PAL_MODE_
	SystemCheckTimer(&hundred_ms_timer, &last_100_ms_tick, 2 /* 2 * 50 ms = 100 ms */);
	SystemCheckTimer(&five_hundred_ms_timer, &last_500_ms_tick, 10 /* 10 * 50 ms = 500 ms */);
#else
	SystemCheckTimer(&hundred_ms_timer, &last_100_ms_tick, 3);
#endif //VMODE_PAL
	
}

void SystemCheckTimer(bool * timer, uint64_t * last_timer, uint8_t step)
{
	if(*timer == true)
	{
		*timer = false;
		*last_timer = global_timer;
	}

	if(global_timer >= (*last_timer + step) )
	{
		*timer = true;
	}	
}

bool SystemLoadFileToBuffer(char * fname, uint8_t * buffer, uint32_t szBuffer)
{
	FILE *f;
	int32_t size;
	
	if(fname == NULL)
	{
		dprintf("SystemLoadFile: NULL fname!\n");
		return false;
	}
	
	memset(buffer,0,szBuffer);
	
	system_busy = true;
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
	
	system_busy = false;
	
	dprintf("File \"%s\" loaded successfully!\n",fname);
	
	return true;
}

bool SystemLoadFile(char *fname)
{
	return SystemLoadFileToBuffer(fname,file_buffer,sizeof(file_buffer));
}

uint8_t * SystemGetBufferAddress(void)
{
	return file_buffer;
}

void SystemWaitCycles(uint32_t cycles)
{
	uint64_t currentTime = global_timer;
	
	while(global_timer < (currentTime + cycles) );
}

uint32_t SystemRand(uint32_t min, uint32_t max)
{
	return rand() % (max - min + 1) + min;
}

void SystemSetEmergencyMode(bool value)
{
	emergency_mode = value;
}

bool SystemGetEmergencyMode(void)
{
	return emergency_mode;
}

bool SystemIsBusy(void)
{
	return system_busy;
}

bool SystemContains_u8(uint8_t value, uint8_t * buffer, size_t sz)
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

bool SystemContains_u16(uint16_t value, uint16_t * buffer, size_t sz)
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

TYPE_TIMER * SystemCreateTimer(uint32_t seconds, bool rf, void (*timer_callback)(void) )
{
	bool success = false;
	uint8_t i;
	
	if(seconds == 0)
	{
		dprintf("Cannot create timer with time == 0!\n");
		return NULL;
	}
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		if(timer_array[i].busy == false)
		{
			timer_array[i].Timeout_Callback = timer_callback;
			timer_array[i].time = seconds;
			timer_array[i].orig_time = seconds;
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

void SystemResetTimers(void)
{
	uint8_t i;
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		timer_array[i].Timeout_Callback = NULL;
		timer_array[i].busy = false;
		timer_array[i].repeat_flag = false;
		timer_array[i].time = 0;
		timer_array[i].orig_time = 0;
	}
}

void SystemUserTimersHandler(void)
{
	uint8_t i;
	
	for(i = 0; i < SYSTEM_MAX_TIMERS; i++)
	{
		if(timer_array[i].busy == true)
		{
			if(System1SecondTick() == true)
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

void SystemTimerRestart(TYPE_TIMER * timer)
{
	timer->time = timer->orig_time;
}

void SystemTimerRemove(TYPE_TIMER * timer)
{
	timer->time = 0;
	timer->orig_time = 0;
	timer->Timeout_Callback = NULL;
	timer->busy = false;
	timer->repeat_flag = false;
}

bool SystemArrayCompare(unsigned short * arr1, unsigned short * arr2, size_t sz)
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

int32_t SystemIndexOf_U16(uint16_t value, uint16_t * array, uint32_t sz)
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

int32_t SystemIndexOf_U8(uint8_t value, uint8_t * array, uint32_t from, uint32_t sz)
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

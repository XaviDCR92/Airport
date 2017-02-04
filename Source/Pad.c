/* *************************************
 * 	Includes
 * *************************************/

#include "Pad.h"

/* *************************************
 * 	Defines
 * *************************************/

#define PAD_ONE 0
#define PAD_TWO 1
#define PAD_CHEAT_TIMEOUT 2
#define PAD_MAX_CHEATS 16

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	
	PAD_CROSS_INDEX = 0,
	PAD_SQUARE_INDEX,
	PAD_TRIANGLE_INDEX,
	PAD_CIRCLE_INDEX,
	
	PAD_DOWN_INDEX,
	PAD_LEFT_INDEX,
	PAD_UP_INDEX,
	PAD_RIGHT_INDEX,
	
	PAD_L1_INDEX,
	PAD_L2_INDEX,
	
	PAD_R1_INDEX,
	PAD_R2_INDEX,
	
	NUMBER_OF_KEYS
	
};
	

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void PadOneVibrationHandler(void);
static void PadTwoVibrationHandler(void);
static void PadCheatHandler(uint8_t n_pad);
static void PadOneCleanCheatArray(void);
static void PadTwoCleanCheatArray(void);
static psx_pad_state PadOneGetState(void);
uint8_t PadGetKeyIndex(unsigned short key);

/* *************************************
 * 	Local Variables
 * *************************************/

// Pad data
static unsigned short pad1;
static unsigned short pad2;

// Pad data from previous frame
static unsigned short previous_pad1;
static unsigned short previous_pad2;

// Vibration timers
static uint16_t pad1_vibration_timer;
static uint16_t pad2_vibration_timer;

// Vibration strenght data (big motor)
static uint8_t pad1_big_vibration_force;
static uint8_t pad2_big_vibration_force;

// Vibration strenght data (small motor)
static uint8_t pad1_small_vibration_force;
static uint8_t pad2_small_vibration_force;

// Timers for each key pressed (used for PadXXKeyRepeat() )
static uint8_t pad1_keys_repeat[NUMBER_OF_KEYS];
static uint8_t pad2_keys_repeat[NUMBER_OF_KEYS];
// These arrays include last 16 buttons pressed by user and keeps them
// for cheating purposes. They are cleaned if no keys are pressed during
// PAD_CHEAT_TIMEOUT seconds.
static unsigned short pad1_cheat_array[CHEAT_ARRAY_SIZE];
static unsigned short pad2_cheat_array[CHEAT_ARRAY_SIZE];

// Pointers to timers which clean padX_cheat_array.
static TYPE_TIMER * pad1_cheat_timer;
static TYPE_TIMER * pad2_cheat_timer;

static TYPE_CHEAT * cheatsArray[PAD_MAX_CHEATS];

psx_pad_state PadOneGetState(void)
{
	psx_pad_state PadOne;
	
	PSX_PollPad_Fast(PAD_ONE,&PadOne);
	
	return PadOne;
}

bool PadOneConnected(void)
{
	psx_pad_state PadOne = PadOneGetState();
	
	if(PadOne.status != PAD_STATUS_OK)
	{
		return false;
	}
	
	return true;
}

bool PadOneAnyKeyPressed(void)
{
	return (bool)pad1;
}

bool PadOneDirectionKeyPressed(void)
{
	return (	(PadOneKeyPressed(PAD_UP) == true)
						||
				(PadOneKeyPressed(PAD_LEFT) == true)
						||
				(PadOneKeyPressed(PAD_RIGHT) == true)
						||
				(PadOneKeyPressed(PAD_DOWN) == true)	);
}

bool PadOneDirectionKeyReleased(void)
{
	return (	(PadOneKeyReleased(PAD_UP) == true)
						||
				(PadOneKeyReleased(PAD_LEFT) == true)
						||
				(PadOneKeyReleased(PAD_RIGHT) == true)
						||
				(PadOneKeyReleased(PAD_DOWN) == true)	);
}

bool PadTwoDirectionKeyReleased(void)
{
	return (	(PadTwoKeyReleased(PAD_UP) == true)
						||
				(PadTwoKeyReleased(PAD_LEFT) == true)
						||
				(PadTwoKeyReleased(PAD_RIGHT) == true)
						||
				(PadTwoKeyReleased(PAD_DOWN) == true)	);
}

bool PadTwoDirectionKeyPressed(void)
{
	return (	(PadTwoKeyPressed(PAD_UP) == true)
						||
				(PadTwoKeyPressed(PAD_LEFT) == true)
						||
				(PadTwoKeyPressed(PAD_RIGHT) == true)
						||
				(PadTwoKeyPressed(PAD_DOWN) == true)	);
}

bool PadTwoAnyKeyPressed(void)
{
	return (bool)pad2;
}

bool PadOneKeyPressed(unsigned short key)
{
	return (bool)( pad1 & key );
}

bool PadTwoKeyPressed(unsigned short key)
{
	return (bool)( pad2 & key );
}

bool PadOneKeyRepeat(unsigned short key, uint8_t time)
{
	uint8_t key_index = PadGetKeyIndex(key);
	
	if(key_index == NUMBER_OF_KEYS)
	{
		return false;
	}
	
	pad1_keys_repeat[key_index]++;
	
	if(pad1_keys_repeat[key_index] >= time)
	{
		pad1_keys_repeat[key_index] = 0;
		return true;
	}

	return false;
}

bool PadTwoKeyRepeat(unsigned short key, uint8_t time)
{
	uint8_t key_index = PadGetKeyIndex(key);
	
	if(key_index == NUMBER_OF_KEYS)
	{
		return false;
	}
	
	pad2_keys_repeat[key_index]++;
	
	if(pad2_keys_repeat[key_index] >= time)
	{
		pad2_keys_repeat[key_index] = 0;
		return true;
	}
	
	return false;
}

void PadOneVibrationHandler(void)
{
	if(PadOneIsVibrationEnabled() == true)
	{
		pad_enable_vibration(PAD_ONE);
		pad_set_vibration(PAD_ONE,pad1_small_vibration_force,pad1_big_vibration_force);
		pad1_vibration_timer--;
	}
}

void PadTwoVibrationHandler(void)
{
	if(PadTwoIsVibrationEnabled() == true)
	{
		pad_enable_vibration(PAD_TWO);
		pad_set_vibration(PAD_TWO,pad2_small_vibration_force,pad2_big_vibration_force);
		pad2_vibration_timer--;
	}	
}

bool PadOneIsVibrationEnabled(void)
{
	return (pad1_vibration_timer & true);
}

bool PadTwoIsVibrationEnabled(void)
{
	return (pad2_vibration_timer & true);
}

bool UpdatePads(void)
{
	PadOneVibrationHandler();
	
	PadTwoVibrationHandler();
	
	PadCheatHandler(PAD_ONE);
	
	PadCheatHandler(PAD_TWO);
	
	// Get now-old pad data
	previous_pad1 = pad1;
	previous_pad2 = pad2;
	
	PSX_ReadPad(&pad1,&pad2);
	
	if(PadOneConnected() == false)
	{
		return false;
	}
	
	return true;
}

bool PadOneKeyReleased(unsigned short key)
{
	return ( !(pad1 & key) && (previous_pad1 & key) );
}

bool PadTwoKeyReleased(unsigned short key)
{
	return ( !(pad2 & key) && (previous_pad2 & key) );
}

uint8_t PadGetKeyIndex(unsigned short key)
{
	switch(key)
	{
		case PAD_CROSS:
			return PAD_CROSS_INDEX;
		break;
		
		case PAD_SQUARE:
			return PAD_SQUARE_INDEX;
		break;
		
		case PAD_TRIANGLE:
			return PAD_TRIANGLE_INDEX;
		break;
		
		case PAD_CIRCLE:
			return PAD_CIRCLE_INDEX;
		break;
		
		case PAD_DOWN:
			return PAD_DOWN_INDEX;
		break;
		
		case PAD_LEFT:
			return PAD_LEFT_INDEX;
		break;
		
		case PAD_UP:
			return PAD_UP_INDEX;
		break;
		
		case PAD_RIGHT:
			return PAD_RIGHT_INDEX;
		break;
		
		case PAD_L1:
			return PAD_L1_INDEX;
		break;
		
		case PAD_R1:
			return PAD_R1_INDEX;
		break;
		
		case PAD_L2:
			return PAD_L2_INDEX;
		break;
		
		case PAD_R2:
			return PAD_R2_INDEX;
		break;
		
		default:
			return NUMBER_OF_KEYS;
		break;
	}
}

unsigned short * PadOneGetAddress(void)
{
	return &pad1;
}

void PadClearData(void)
{
	pad1 = 0;
	pad2 = 0;
	
	previous_pad1 = 0;
	previous_pad2 = 0;
}

void PadInit(void)
{
	pad1_cheat_timer = SystemCreateTimer(PAD_CHEAT_TIMEOUT,true /* Repeat flag */,&PadOneCleanCheatArray);
	pad2_cheat_timer = SystemCreateTimer(PAD_CHEAT_TIMEOUT,true /* Repeat flag */,&PadTwoCleanCheatArray);
	memset(cheatsArray,0, sizeof(TYPE_CHEAT) * PAD_MAX_CHEATS);
}

void PadCheatHandler(uint8_t n_pad)
{
	unsigned short available_keys[12] = { 	PAD_LEFT, PAD_RIGHT, PAD_UP, PAD_DOWN,	
											PAD_L2, PAD_R2, PAD_L1, PAD_R1,
											PAD_TRIANGLE, PAD_CIRCLE, PAD_CROSS, PAD_SQUARE };
										
	uint8_t i;
	uint8_t keys_released = 0;
	unsigned short key;
	uint8_t j;
	bool (*released_callback)(unsigned short);
	void (*clean_callback)(void);
	bool success = false;
	unsigned short * cheat_array;
	TYPE_TIMER * timer;
	
	switch(n_pad)
	{
		case PAD_ONE:
			released_callback = &PadOneKeyReleased;
			cheat_array = pad1_cheat_array;
			clean_callback = &PadOneCleanCheatArray;
			timer = pad1_cheat_timer;
		break;
		
		case PAD_TWO:
			released_callback = &PadTwoKeyReleased;
			cheat_array = pad2_cheat_array;
			clean_callback = &PadTwoCleanCheatArray;
			timer = pad2_cheat_timer;
		break;
		
		default:
			dprintf("Invalid pad called for PadCheatHandler()!\n");
			return;
	}
	
	for(i = 0; i < PAD_MAX_CHEATS; i++)
	{
		if(cheatsArray[i] != NULL)
		{
			if(SystemArrayCompare(cheat_array, cheatsArray[i]->Combination, CHEAT_ARRAY_SIZE) == true)
			{
				if(cheatsArray[i]->Callback != NULL)
				{
					if(clean_callback != NULL)
					{
						clean_callback();
					}
					
					cheatsArray[i]->Callback();
					
					return;
				}
			}
		}
	}
	
	for(i = 0; i < sizeof(available_keys) / sizeof(unsigned short); i++)
	{
		if(released_callback(available_keys[i]) == true)
		{
			SystemTimerRestart(timer);
			key = available_keys[i];
			keys_released++;
		}
	}
	
	if(keys_released != 1)
	{
		return;
	}
	
	// Check for full array (return success = true if an empty array
	// element was found.
	for(j = 0; j < CHEAT_ARRAY_SIZE; j++)
	{
		if(cheat_array[j] == 0)
		{
			success = true;
			break;
		}
	}
		
	if(success == false)
	{
		if(clean_callback != NULL)
		{
			// Overrun
			clean_callback();
		}
	}
	
	cheat_array[j] = key;
}

bool PadAddCheat(TYPE_CHEAT * cheat)
{
	static uint8_t idx = 0;
	
	if(idx >= PAD_MAX_CHEATS)
	{
		dprintf("Maximum number of cheats exceeded!\n");
		return false;
	}
	
	cheatsArray[idx++] = cheat;
	
	return true;
}

void PadOneCleanCheatArray(void)
{
	memset(pad1_cheat_array,0,sizeof(unsigned short) * CHEAT_ARRAY_SIZE);
}

void PadTwoCleanCheatArray(void)
{	
	memset(pad2_cheat_array,0,sizeof(unsigned short) * CHEAT_ARRAY_SIZE);
}

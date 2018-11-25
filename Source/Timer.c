/* *************************************
 * 	Includes
 * *************************************/

#include "Timer.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/

#define MAX_TIMERS 16

/* *************************************
 * 	Local Variables
 * *************************************/

//Timer array.
static TYPE_TIMER timer_array[MAX_TIMERS];

/* *************************************
 * 	Local Prototypes
 * *************************************/

/* ********************************************************************************************
 *
 * @name	TYPE_TIMER* TimerCreate(uint32_t t, bool rf, void (*timer_callback)(void) )
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

TYPE_TIMER* TimerCreate(uint32_t t, bool rf, void (*timer_callback)(void) )
{
	bool success = false;
	uint8_t i;

	if (t == 0)
	{
		Serial_printf("Cannot create timer with time == 0!\n");
		return NULL;
	}

	for (i = 0; i < MAX_TIMERS; i++)
	{
        TYPE_TIMER* const ptrTimer = &timer_array[i];

		if (ptrTimer->busy == false)
		{
			ptrTimer->Timeout_Callback = timer_callback;
			ptrTimer->time = t;
			ptrTimer->orig_time = t;
			ptrTimer->repeat_flag = rf;
			ptrTimer->busy = true;
			success = true;
			break;
		}
	}

	if (success == false)
	{
		Serial_printf("Could not find any free timer!\n");
		return NULL;
	}

	return &timer_array[i];
}

/* *******************************************
 *
 * @name	void TimerReset(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:	reportedly, removes all timers.
 *
 * *******************************************/

void TimerReset(void)
{
	uint8_t i;

	for (i = 0; i < MAX_TIMERS; i++)
	{
		TimerRemove(&timer_array[i]);
	}
}

/* *****************************************************
 *
 * @name	void TimerHandler(void)
 *
 * @author: Xavier Del Campo
 *
 * @brief:	reportedly, handles all available timers.
 *
 * @remarks: calls callback on timeout.
 *
 * *****************************************************/

void TimerHandler(void)
{
	uint8_t i;

	for (i = 0; i < MAX_TIMERS; i++)
	{
		if (timer_array[i].busy)
		{
			if (System100msTick())
			{
				timer_array[i].time--;

				if (timer_array[i].time == 0)
				{
					timer_array[i].Timeout_Callback();

					if (timer_array[i].repeat_flag)
					{
						timer_array[i].time = timer_array[i].orig_time;
					}
				}
			}
		}
	}
}

/* *********************************************************************
 *
 * @name	void TimerRestart(TYPE_TIMER* timer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:	sets time left for TYPE_TIMER instance to initial value.
 *
 * @remarks: specially used when TYPE_TIMER.rf is enabled.
 *
 * *********************************************************************/

void TimerRestart(TYPE_TIMER* timer)
{
	timer->time = timer->orig_time;
}

/* *********************************************************************
 *
 * @name	void TimerRemove(TYPE_TIMER* timer)
 *
 * @author: Xavier Del Campo
 *
 * @brief:	Resets timer parameters to default values so timer instance
 * 			can be recycled.
 *
 * @remarks:
 *
 * *********************************************************************/

void TimerRemove(TYPE_TIMER* timer)
{
	timer->time = 0;
	timer->orig_time = 0;
	timer->Timeout_Callback = NULL;
	timer->busy = false;
	timer->repeat_flag = false;
}

#ifndef TIMER_HEADER__
#define TIMER_HEADER__

/* **************************************
 * 	Includes							*
 * **************************************/

#include "Global_Inc.h"
#include "GameStructures.h"

/* **************************************
 * 	Defines								*
 * **************************************/

/* **************************************
 * 	Global Prototypes					*
 * **************************************/

// Creates a timer instance wiht a determined value and associates it to a callback
// Once time expires, callback is automatically called right after GfxDrawScene().
// Time is expressed so that t = 100 ms e.g.: 2 seconds = 20.
TYPE_TIMER* TimerCreate(uint32_t t, bool rf, void (*timer_callback)(void) );

// Reportedly, sets all timer data to zero.
void TimerReset(void);

// To be called every cycle (i.e.: inside GfxDrawScene() ).
void TimerHandler(void);

// Sets timer remaining time to its initial value.
void TimerRestart(TYPE_TIMER* timer);

// Flushes a timer pointed to by timer.
void TimerRemove(TYPE_TIMER* timer);

#endif // TIMER_HEADER__

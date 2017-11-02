#ifndef _TIMER_APP_H_
#define _TIMER_APP_H_


// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "timer.h"
#include "prcm.h"
// Common interface includes
#include "timer_if.h"
// User include
#include "ring_buffer.h"
#include "common_include.h"


void Timer_init(void TimerBaseIntHandler (void));


#endif /* _TIMER_APP_H_ */

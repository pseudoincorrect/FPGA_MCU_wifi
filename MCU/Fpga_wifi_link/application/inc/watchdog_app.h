#ifndef _WATCHDOG_APP_H_
#define _WATCHDOG_APP_H_

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "wdt.h"
#include "wdt_if.h"
// Common interface includes
#include "timer_if.h"
// User include
#include "common_include.h"


/////////////////////////////////////////////////////////////////////////////////////////
void watchdog_init(void WatchdogIntHandler (void));

/////////////////////////////////////////////////////////////////////////////////////////
void watchDogAck (void);

#endif /* _WATCHDOG_APP_H_ */

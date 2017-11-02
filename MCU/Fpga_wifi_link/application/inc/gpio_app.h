#ifndef _GPIO_APP_H
#define _GPIO_APP_H

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "interrupt.h"

// Common interface includes
#include "dbg.h"
#include "gpio.h"
#include "pin.h"
#include "common_include.h"


/////////////////////////////////////////////////////////////////////////////////////////
void GPIOConfigureNIntEnable(unsigned int uiGPIOPort,
                                  unsigned char ucGPIOPin,
                                  unsigned int uiIntType,
                                  void (*pfnIntHandler)(void));

/////////////////////////////////////////////////////////////////////////////////////////
static unsigned char GetPeripheralIntNum(unsigned int uiGPIOPort);

/////////////////////////////////////////////////////////////////////////////////////////
void DebugPulse(int pin, int amount);

/////////////////////////////////////////////////////////////////////////////////////////
void DebugPinSet(int pin, int status);

/////////////////////////////////////////////////////////////////////////////////////////
void LedSet(int led_number, int status);

#endif /* _GPIO_APP_H */

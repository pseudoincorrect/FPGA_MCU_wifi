#ifndef _UART_APP_H_
#define _UART_APP_H_

// Driverlib includes
#include "hw_types.h"
#include "common.h"
#include "uart.h"
// Common interface includes
#ifndef  NOTERM
#include "uart_if.h"
#endif
// User include
#include "common_include.h"
#include "dbg.h"

/////////////////////////////////////////////////////////////////////////////////////////
void uart_init (void);

/////////////////////////////////////////////////////////////////////////////////////////
void BufferDisplay(char type, void *buffer, int size);

#endif /* _UART_APP_H_ */

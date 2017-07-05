#ifndef DBG_H_
#define DBG_H_

#include <string.h>
#include "uart_if.h"

// __file__

#ifdef NO_DEBUG

#define DBG_debug(M, ...)
#define DBG_log_err(M, ...)
#define DBG_log_warn(M, ...)
#define DBG_log_info(M, ...)
#define DBG_check(A, M, ...)
#define DBG_sentinel(M, ...)
#define DBG_check_mem(A)
#define DBG_check_debug(A, M, ...)

#else

#define DBG_debug(M, ...)     Report("[DEBUG] %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)

#define DBG_log_err(M, ...)   Report("[ERROR] %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)
#define DBG_log_warn(M, ...)  Report("[WARN]  %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)
#define DBG_log_info(M, ...)  Report("[INFO]  %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)

#define DBG_check(A, M, ...) if(!(A)) { DBG_log_err(M, ##__VA_ARGS__); goto error; }
#define DBG_warn_check(A, M, ...) if(!(A)) { DBG_log_warn(M, ##__VA_ARGS__);}

#define DBG_sentinel(M, ...) { DBG_log_err(M, ##__VA_ARGS__); goto error; }

#define DBG_check_mem(A) 	DBG_check((A), "Out of memory.")

#endif

// GPIOPinWrite(GPIOA2_BASE, 1 << 0x06, 1 << 0x06);
// GPIOPinWrite(GPIOA2_BASE, 1 << 0x06, 0x00);

#endif /* DBG_H_ */

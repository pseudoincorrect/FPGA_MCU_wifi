#ifndef DBG_H_
#define DBG_H_

#include <string.h>
#include "uart_if.h"

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

// function to realise a ASSERT type test, and output the function, line number and a 
// formated string on the debug terminal
#define DBG_debug(M, ...)     Report("[DEBUG] %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)

#define DBG_log_err(M, ...)   Report("[ERROR] %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)
#define DBG_log_warn(M, ...)  Report("[WARN]  %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)
#define DBG_log_info(M, ...)  Report("[INFO]  %s() %d : " M "\n\r",__func__,__LINE__, ##__VA_ARGS__)

#define DBG_check(A, M, ...) if(!(A)) { DBG_log_err(M, ##__VA_ARGS__); goto error; }
#define DBG_warn_check(A, M, ...) if(!(A)) { DBG_log_warn(M, ##__VA_ARGS__);}

#define DBG_sentinel(M, ...) { DBG_log_err(M, ##__VA_ARGS__); goto error; }

#define DBG_check_mem(A) 	DBG_check((A), "Out of memory.")

#endif

// __file__      __func__      __LINE__

#endif /* DBG_H_ */

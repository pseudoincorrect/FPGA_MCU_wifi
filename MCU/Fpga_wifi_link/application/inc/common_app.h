//*****************************************************************************
// Copyright 2017 Maxime Clement
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to
// do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//*****************************************************************************

#ifndef __COMMON_APP_H__
#define __COMMON_APP_H__

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "prcm.h"
#include "interrupt.h"
// Common interface includes
#include "gpio.h"
#include "pin.h"
// User include
#include "ring_buffer.h"
#include "common_include.h"
#include "dbg.h"

/*88888b.  8888888888  .d8888b.  888
888  "Y88b 888        d88P  Y88b 888
888    888 888        888    888 888
888    888 8888888    888        888
888    888 888        888        888
888    888 888        888    888 888
888  .d88P 888        Y88b  d88P 888      d8b
8888888P"  8888888888  "Y8888P"  88888888 Y*/


/////////////////////////////////////////////////////////////////////////////////////////
void common_init(void);

/////////////////////////////////////////////////////////////////////////////////////////
void PinMuxConfig(void);

/*88888888 Y88b   d88P 88888888888 8888888888 8888888b.  888b    888
888         Y88b d88P      888     888        888   Y88b 8888b   888
888          Y88o88P       888     888        888    888 88888b  888
8888888       Y888P        888     8888888    888   d88P 888Y88b 888
888           d888b        888     888        8888888P"  888 Y88b888
888          d88888b       888     888        888 T88b   888  Y88888
888         d88P Y88b      888     888        888  T88b  888   Y8888
8888888888 d88P   Y88b     888     8888888888 888   T88b 888    Y8*/


/////////////////////////////////////////////////////////////////////////////////////////
extern void (* const g_pfnVectors[])(void);





#endif

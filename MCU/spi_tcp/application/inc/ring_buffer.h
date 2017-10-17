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

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

/////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>
#include "dbg.h"
#include "uart_app.h"
/////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    char *data;

    int length;
    int length_unit;
    int write_index;
    int read_index;

}RingBuffer;


/////////////////////////////////////////////////////////////////////////////////////////
RingBuffer* RingBuffer_create        (int size_global, int size_unit_byte);

/////////////////////////////////////////////////////////////////////////////////////////
void        RingBuffer_destroy       (RingBuffer *buffer);

/////////////////////////////////////////////////////////////////////////////////////////
int         RingBuffer_availableData (RingBuffer *buffer);

/////////////////////////////////////////////////////////////////////////////////////////
int         RingBuffer_read          (RingBuffer *buffer, char *target, int amount);

/////////////////////////////////////////////////////////////////////////////////////////
void        RingBuffer_commitRead    (RingBuffer *buffer);

/////////////////////////////////////////////////////////////////////////////////////////
int         RingBuffer_write         (RingBuffer *buffer, char *target, int amount);

/////////////////////////////////////////////////////////////////////////////////////////
int        RingBuffer_commitWrite   (RingBuffer *buffer);

/////////////////////////////////////////////////////////////////////////////////////////
void BufferStatusDisplay(RingBuffer *buffer);

#endif

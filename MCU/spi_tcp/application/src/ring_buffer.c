
#include "ring_buffer.h"



 /*8888b.  8888888b.  8888888888        d8888 88888888888 8888888888
d88P  Y88b 888   Y88b 888              d88888     888     888
888    888 888    888 888             d88P888     888     888
888        888   d88P 8888888        d88P 888     888     8888888
888        8888888P"  888           d88P  888     888     888
888    888 888 T88b   888          d88P   888     888     888
Y88b  d88P 888  T88b  888         d8888888888     888     888
 "Y8888P"  888   T88b 8888888888 d88P     888     888     88888888*/

RingBuffer* RingBuffer_create(int size_global, int size_unit_byte)
{
    RingBuffer *buffer = calloc(1, sizeof(RingBuffer));

    buffer->data = calloc((size_global * size_unit_byte), 1);

    buffer->length      = size_global;
    buffer->length_unit = size_unit_byte;
    buffer->read_index  = 0;
    buffer->write_index = 0;

    return buffer;
}


/*88888b.  8888888888  .d8888b. 88888888888 8888888b.   .d88888b. Y88b   d88P
888  "Y88b 888        d88P  Y88b    888     888   Y88b d88P" "Y88b Y88b d88P
888    888 888        Y88b.         888     888    888 888     888  Y88o88P
888    888 8888888     "Y888b.      888     888   d88P 888     888   Y888P
888    888 888            "Y88b.    888     8888888P"  888     888    888
888    888 888              "888    888     888 T88b   888     888    888
888  .d88P 888        Y88b  d88P    888     888  T88b  Y88b. .d88P    888
8888888P"  8888888888  "Y8888P"     888     888   T88b  "Y88888P"     8*/

void RingBuffer_destroy(RingBuffer *buffer)
{
    if (buffer)
    {
        free(buffer->data);
        free(buffer);
    }
}


       /*888 888     888      d8888 8888888 888
      d88888 888     888     d88888   888   888
     d88P888 888     888    d88P888   888   888
    d88P 888 Y88b   d88P   d88P 888   888   888
   d88P  888  Y88b d88P   d88P  888   888   888
  d88P   888   Y88o88P   d88P   888   888   888
 d8888888888    Y888P   d8888888888   888   888
d88P     888     Y8P   d88P     888 8888888 888888*/

int RingBuffer_availableData(RingBuffer *buffer) 
{
    if ((buffer->read_index     == buffer->write_index)  ||
        (buffer->read_index + 1 == buffer->write_index)  || 
        (buffer->read_index     == buffer->write_index + 1) )
        return 0;

    else if (buffer->read_index < buffer->write_index)
    {
        DBG_debug("R_ind = %d , W_ind = %d", buffer->read_index, buffer->write_index );
        return (buffer->write_index - buffer->read_index);
    }
    else if (buffer->read_index > buffer->write_index)
    {
        DBG_debug("R_ind = %d , W_ind = %d", buffer->read_index, buffer->write_index );
        return (   buffer->write_index
                 + buffer->length
                 - buffer->read_index);
    }
    else
        return -1;
}


/*88888b.  8888888888        d8888 8888888b.
888   Y88b 888              d88888 888  "Y88b
888    888 888             d88P888 888    888
888   d88P 8888888        d88P 888 888    888
8888888P"  888           d88P  888 888    888
888 T88b   888          d88P   888 888    888
888  T88b  888         d8888888888 888  .d88P
888   T88b 8888888888 d88P     888 888888*/

int RingBuffer_read(RingBuffer *buffer, char *target, int amount)
{
   void *result;

   DBG_check(amount == buffer->length_unit, "Buffer: wrong read length");

   DBG_check(RingBuffer_availableData(buffer) > 0,
             "Not enough data in the buffer : %d",
             RingBuffer_availableData(buffer));

   result = memcpy( target,
                    buffer->data + buffer->read_index * buffer->length_unit,
                    amount);

   DBG_check(result != NULL, "Failed to write buffer into data.");

   return amount;

   error:
       return -1;
}


 /*8888b.          8888888b.  8888888888        d8888 8888888b.
d88P  Y88b         888   Y88b 888              d88888 888  "Y88b
888    888         888    888 888             d88P888 888    888
888                888   d88P 8888888        d88P 888 888    888
888                8888888P"  888           d88P  888 888    888
888    888         888 T88b   888          d88P   888 888    888
Y88b  d88P d8b     888  T88b  888         d8888888888 888  .d88P
 "Y8888P"  Y8P     888   T88b 8888888888 d88P     888 8888888*/

void RingBuffer_commitRead(RingBuffer *buffer)
{
    buffer->read_index++;

    if (buffer->read_index >= buffer->length)
        buffer->read_index = 0;
}


/*8       888 8888888b.  8888888 88888888888 8888888888
888   o   888 888   Y88b   888       888     888
888  d8b  888 888    888   888       888     888
888 d888b 888 888   d88P   888       888     8888888
888d88888b888 8888888P"    888       888     888
88888P Y88888 888 T88b     888       888     888
8888P   Y8888 888  T88b    888       888     888
888P     Y888 888   T88b 8888888     888     88888888*/

int RingBuffer_write(RingBuffer *buffer, char *target, int amount)
{
    void *result;

    DBG_check(amount == buffer->length_unit, "Buffer: wrong write length");

    result = memcpy( buffer->data + buffer->write_index * buffer->length_unit,
                     target,
                     amount);

   DBG_check(result != NULL, "Failed to write buffer into data.");

   return amount;

   error:
       return -1;
}


 /*8888b.          888       888 8888888b.  8888888 88888888888 8888888888
d88P  Y88b         888   o   888 888   Y88b   888       888     888
888    888         888  d8b  888 888    888   888       888     888
888                888 d888b 888 888   d88P   888       888     8888888
888                888d88888b888 8888888P"    888       888     888
888    888         88888P Y88888 888 T88b     888       888     888
Y88b  d88P d8b     8888P   Y8888 888  T88b    888       888     888
 "Y8888P"  Y8P     888P     Y888 888   T88b 8888888     888     88888888*/

void RingBuffer_commitWrite(RingBuffer *buffer)
{
    buffer->write_index++;

    if (buffer->write_index >= buffer->length)
        buffer->write_index = 0;

    DBG_check(buffer->write_index != buffer->read_index,
              "Buffer overflow");

    return;

    error:
        return;
}






























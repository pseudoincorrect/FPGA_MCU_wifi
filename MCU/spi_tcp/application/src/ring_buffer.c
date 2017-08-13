#include "ring_buffer.h"

//*********************************************************************
//
//                  RING BUFFER
//
//  This file contains the function used to work with the RingBuffers
//  Architecture of the ring buffer:
//  We can see this implementation of a Ring buffer as an array of arrays
//  It contain BUFFER_SIZE units of BUFFER_UNIT_SIZE elements.
//  Each element can be accessed (read from or written to the buffer)
//  However in this application we usuall process a whole unit at a time
//  instead of elements by element/
//  
//  { UNIT: 0 } =>  {element_0} ... {element_BUFFER_UNIT_SIZE - 1}
//  { UNIT: 1 } =>  {element_0} ... {element_BUFFER_UNIT_SIZE - 1}
//  
//   ° ° °
//  
//  { UNIT: (BUFFER_SIZE - 2) } => {element_0} ... {element_BUFFER_UNIT_SIZE - 1}
//  { UNIT: (BUFFER_SIZE - 1) } => {element_0} ... {element_BUFFER_UNIT_SIZE - 1}
//  
//  read_index and write_index are used to read/write in circle fashion.
//  For instance when the write index reach the "end" of the buffer, it will 
//  be reassigned to the beginning of the buffer for the next writting, 
//  giving the buffer its apparent circling memory use. The reading 
//  follow the same principle. 
//  
//  In this implementation, the write idex represent the memory space being
//  written and unavailable for read, until we "comit a write" in that case, 
//  the write index will be incremented and the memory space will precedently 
//  protected by the write index is available for read. 
//
//  Here, we can only read data if write_index is different from read index.
//  
//  An overflow occurs when the write index become equal to the read index,
//  meaning that we did a "circle turn" of the buffer without enough read,
//  and that all data of the buffer is lost. the buffer is reseted in that
//  case.
//  It means that the we couldn't read the data fast enough out of the buffer.
//  In this application, it can happen when the socket_send function is stuck
//  (hence we cannot send/read data out of the buffer) and that the SPI 
//  continue to receive data. This is why we use the function SpiBlock() to 
//  prevent that kind of overflow to happen.
//  
//
//*********************************************************************

 /*8888b.  8888888b.  8888888888        d8888 88888888888 8888888888
d88P  Y88b 888   Y88b 888              d88888     888     888
888    888 888    888 888             d88P888     888     888
888        888   d88P 8888888        d88P 888     888     8888888
888        8888888P"  888           d88P  888     888     888
888    888 888 T88b   888          d88P   888     888     888
Y88b  d88P 888  T88b  888         d8888888888     888     888
 "Y8888P"  888   T88b 8888888888 d88P     888     888     88888888*/

//****************************************************************************
//
//!    \brief This function create a RingBuffer by allocating memory on
//!     the heap
//!
//!   \param[in] Amount of unit in the buffer
//!   \param[in] Size of the unit
//!
//!   \return  A pointer to the buffer
//
//****************************************************************************
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

//****************************************************************************
//
//!    \brief This function handle the destruction of the RingBuffer
//!
//!   \param[in] RingBuffer pointer
//!
//!   \return  None
//
//****************************************************************************
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

//****************************************************************************
//
//!    \brief This function return the Amount of available data in the buffer
//!
//!   \param[in] RingBuffer pointer
//!
//!   \return The amount of data or an error code
//
//****************************************************************************
int RingBuffer_availableData(RingBuffer *buffer) 
{
    if (buffer->read_index == buffer->write_index )
    {
        return 0;
    }
    else if (buffer->read_index < buffer->write_index)
    {
        return (buffer->write_index - buffer->read_index);
    }
    else if (buffer->read_index > buffer->write_index)
    {
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

//****************************************************************************
//
//!   \brief this function copy a specified amount (in BYTES) of data
//!   (if available) from the Ringbuffer to a target buffer.
//!   It does not modify the RingBuffer data or pointers
//!   In this implementation we read from a buffer by unit length, thus
//!   the amount of data read from the Buffer must be equalt to a
//!   buffer unit lenght in bytes (=BUFFER_UNIT_SIZE)
//!
//!   \param[in] A RingBuffer pointer
//!   \param[in] A Buffer target (to receive the copied data)
//!   \param[in] The amount of data to copy (in term of RingBuffer BYTES )
//!
//!   \return  The amount of data read
//
//****************************************************************************
int RingBuffer_read(RingBuffer *buffer, char *target, int amount)
{
   void *result;

   DBG_check(amount == buffer->length_unit, "Buffer: wrong read length \r\n"
             "in this implementation amount must be equal to a buffer unit length");

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

//****************************************************************************
//
//!   \brief  This function, to use AFTER a read from a ringBuffer, commit 
//!   that data have been read. This function commit the read of ONE unit
//!
//!   \param[in] a RingBuffer pointer
//!
//!   \return  None
//
//****************************************************************************
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

//****************************************************************************
//
//!   \brief  This function Copy the data from a buffer to a RingBuffer, if
//!   there is enough place to stor it
//!   this function modify the data in the RingBuffer but not its pointers
//!   In this implementation we write to a buffer by unit length, thus
//!   the amount of data written to the Buffer must be equalt to a
//!   buffer unit lenght in bytes (=BUFFER_UNIT_SIZE)
//!
//!   \param[in] A RingBuffer pointer
//!   \param[in] A Buffer pointer (to copy data from it)
//!   \param[in] The amount of data to copy (in term of RingBuffer BYTES )
//!
//!   \return The amount of data copied
//
//****************************************************************************
int RingBuffer_write(RingBuffer *buffer, char *target, int amount)
{
    void *result;

    DBG_check(amount == buffer->length_unit, "Buffer: wrong write length \r\n"
              "in this implementation amount must be equal to a buffer unit length");

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

//****************************************************************************
//
//!   \brief  This function, to use AFTER a write to a Ringbuffer, commit 
//!   that data have been read. This function commit the write of ONE unit
//!
//!   \param[in] A RingBuffer pointer
//!
//!   \return Success of Faillure (in case of overflow)
//
//****************************************************************************
int RingBuffer_commitWrite(RingBuffer *buffer)
{
    buffer->write_index++;

    if (buffer->write_index >= buffer->length)
        buffer->write_index = 0;

    // Buffer overflow
    if (buffer->write_index == buffer->read_index)
        return -1;

    return 1;

}






























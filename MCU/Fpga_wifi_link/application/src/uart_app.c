#include "uart_app.h"


/*88888 888b    888 8888888 88888888888
  888   8888b   888   888       888
  888   88888b  888   888       888
  888   888Y88b 888   888       888
  888   888 Y88b888   888       888
  888   888  Y88888   888       888
  888   888   Y8888   888       888
8888888 888    Y888 8888888     8*/

//****************************************************************************
//
//!     \brief This function Initialize the uar peripheral
//!
//!     \return None
//
//****************************************************************************
void uart_init (void)
{
        // Initialising the Terminal.
        InitTerm();

        // Clearing the Terminal.
        ClearTerm();

        // Display the Banner
        UART_PRINT("********************************************\n\r");
        UART_PRINT("          SPI / WiFi Link  \n\r");
        UART_PRINT("********************************************\n\r\n\r");
}


/*8888b.   888     888 8888888888 8888888888 8888888888 8888888b.
888  "88b  888     888 888        888        888        888   Y88b
888  .88P  888     888 888        888        888        888    888
8888888K.  888     888 8888888    8888888    8888888    888   d88P
888  "Y88b 888     888 888        888        888        8888888P"
888    888 888     888 888        888        888        888 T88b
888   d88P Y88b. .d88P 888        888        888        888  T88b
8888888P"   "Y88888P"  888        888        8888888888 888   T8*/

//****************************************************************************
//
//!     \brief This function is used to display the content of a data buffer
//!     It can display the conternt of a buffer containing LONG, CHAR (either
//!     under the form of ASCII character of the integer representing them)
//!
//!     \param[in] Type of buffer (long, char (ASCII), char (Integer))
//!     \param[in] Pointer to the buffer
//!     \param[in] Ammount of data to display
//!
//!     \return None
//
//****************************************************************************
void BufferDisplay(char type, void *buffer, int size)
{
    int i;

    switch (type)
    {
        case 0: // unsigned long
        {
            unsigned long* ul_buffer = (unsigned long*) buffer;

            for (i = 0; i < size; i++)
            {
                if (i < 10)
                    UART_PRINT("%d   : %08x\n\r", i, ul_buffer[i]);
                else if (i < 100)
                    UART_PRINT("%d  : %08x\n\r", i, ul_buffer[i]);
                else
                    UART_PRINT("%d : %08x\n\r", i, ul_buffer[i]);
            }
            break;
        }

        case 1: // char number
        {
            char* ch_buffer = (char*) buffer;

            for (i = 0; i < size; i++)
            {
                if (i < 10) // alignment
                    UART_PRINT("%d   : %02x\n\r", i, ch_buffer[i]);
                else if (i < 100)
                    UART_PRINT("%d  : %02x\n\r", i, ch_buffer[i]);
                else
                    UART_PRINT("%d : %02x\n\r", i, ch_buffer[i]);
            }
            break;
        }

        case 2: // char ASCII
        {
            char* ch_buffer = (char*) buffer;

            for (i = 0; i < size; i++)
                    UART_PRINT("%c", ch_buffer[i]);

            break;
        }

        default:
        {
            UART_PRINT("wrong type declaration");
            break;
        }
    }
}




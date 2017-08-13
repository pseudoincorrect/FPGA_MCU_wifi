#include <common_app.h>

//*********************************************************************
//
//                          COMMON APP
//
//  This file contain most of the function used to to initialize 
//  the MCU hardware as well as the display and debug functions.
//  We see there the functions working with the real world interfaces such
//  as SpiBlock that output a signal to the FPGA (and that work on
//  the MCU peripheral register adresses such as the GPIOs)
//
//*********************************************************************

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
//!     \brief This function is used to initialize the MCU's clocks, GPIO port
//!     and UART debuging tool
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void init(void)
{
    IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);

    // Enable Processor
    IntMasterEnable();
    IntEnable(FAULT_SYSTICK);
    PRCMCC3200MCUInit();

    // init the pin function and clock enabling
    PinMuxConfig();

    // init GPIO int handler (for SPI nSS)
//    GPIOConfigureNIntEnable(GPIOA2_BASE,
//                            (1 << 1),
//                            GPIO_RISING_EDGE,
//                            GpioIntHandler);

    // Initialising the Terminal.
    InitTerm();

    // Clearing the Terminal.
    ClearTerm();

    // Reset the peripheral
    PRCMPeripheralReset(PRCM_GSPI);

    // Display the Banner
    Message("\t\t   ********************************************\n\r");
    Message("\t\t                   SPI / WiFi Link  \n\r");
    Message("\t\t   ********************************************\n\r\n\r");
}


 /*8888b.  8888888b. 8888888          .d8888b.   .d88888b.  888b    888 8888888888
d88P  Y88b 888   Y88b  888           d88P  Y88b d88P" "Y88b 8888b   888 888
Y88b.      888    888  888           888    888 888     888 88888b  888 888
 "Y888b.   888   d88P  888           888        888     888 888Y88b 888 8888888
    "Y88b. 8888888P"   888           888        888     888 888 Y88b888 888
      "888 888         888           888    888 888     888 888  Y88888 888
Y88b  d88P 888         888           Y88b  d88P Y88b. .d88P 888   Y8888 888
 "Y8888P"  888       8888888          "Y8888P"   "Y88888P"  888    Y888 8*/

//****************************************************************************
//
//!     \brief This function is used to Initialize the SPI hardware and 
//!     interupt
//!
//!     \param[in] pointer to a SPI handler
//!
//!     \return None
//
//****************************************************************************
void Spiconf(void spiIntHandler (void))
{
    SPIReset(GSPI_BASE);

    UDMAInit();

    // Configure SPI interface
    SPIConfigSetExpClk(GSPI_BASE,
                      PRCMPeripheralClockGet(PRCM_GSPI),
                      SPI_IF_BIT_RATE,
                      SPI_MODE_SLAVE,
                      SPI_SUB_MODE_1,
                          (SPI_HW_CTRL_CS   |
                           SPI_4PIN_MODE    |
                           SPI_TURBO_OFF    |
                           SPI_CS_ACTIVELOW |
                           SPI_WL_32 ));

    SPIIntRegister(GSPI_BASE, spiIntHandler);

    // SPIFIFOLevelSet(GSPI_BASE, 0x0, 0xF);

    // SPIFIFOEnable(GSPI_BASE, SPI_RX_FIFO);

    SPIDmaEnable(GSPI_BASE, SPI_RX_DMA);

    SPIIntEnable(GSPI_BASE, SPI_INT_DMARX);

    SPIDisable(GSPI_BASE);

    // SPIEnable(GSPI_BASE);
}


/*88888b. 8888888 888b    888 888b     d888 888     888 Y88b   d88P
888   Y88b  888   8888b   888 8888b   d8888 888     888  Y88b d88P
888    888  888   88888b  888 88888b.d88888 888     888   Y88o88P
888   d88P  888   888Y88b 888 888Y88888P888 888     888    Y888P
8888888P"   888   888 Y88b888 888 Y888P 888 888     888    d888b
888         888   888  Y88888 888  Y8P  888 888     888   d88888b
888         888   888   Y8888 888   "   888 Y88b. .d88P  d88P Y88b
888       8888888 888    Y888 888       888  "Y88888P"  d88P   Y8*/

//****************************************************************************
//
//!     \brief This function is used to configure the all GPIOs
//!     It first enable the GPIO's port clocks then set the pins' functions
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void PinMuxConfig(void)
{
    // Enable Peripheral Clocks 
    PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GSPI,   PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);

    // Configure PIN_55 for UART0 UART0_TX
    PinTypeUART(PIN_55, PIN_MODE_3);
    // Configure PIN_57 for UART0 UART0_RX
    PinTypeUART(PIN_57, PIN_MODE_3);

    // Configure PIN_05 for SPI0 GSPI_CLK
    PinTypeSPI(PIN_05, PIN_MODE_7);
    // Configure PIN_06 for SPI0 GSPI_MISO
    PinTypeSPI(PIN_06, PIN_MODE_7);
    // Configure PIN_07 for SPI0 GSPI_MOSI
    PinTypeSPI(PIN_07, PIN_MODE_7);

    // Configure PIN_08 for sofware spi nCSS
    PinTypeGPIO(PIN_08, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA2_BASE, (1 << 1), GPIO_DIR_MODE_IN);

    // Logic analyzer

    // GPIO of logic analyzer debug
    // Configure PIN_15 for GPIO_22
    PinTypeGPIO(PIN_15, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA2_BASE, (1 << 6), GPIO_DIR_MODE_OUT);

    // GPIO of logic analyzer debug
    // Configure PIN_60 for GPIO_05
    PinTypeGPIO(PIN_60, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, (1 << 5), GPIO_DIR_MODE_OUT);

    // GPIO of logic analyzer debug
    // Configure PIN_00 for GPIO_50
    PinTypeGPIO(PIN_50, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, (1 << 0), GPIO_DIR_MODE_OUT);


    // LEDs

    // Configure PIN_64 for GPIOOutput
    PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, (1 << 1), GPIO_DIR_MODE_OUT);
    // Configure PIN_01 for GPIOOutput
    PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, (1 << 2), GPIO_DIR_MODE_OUT);
    // Configure PIN_02 for GPIOOutput
    PinTypeGPIO(PIN_02, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA1_BASE, (1 << 3), GPIO_DIR_MODE_OUT);

    PRCMPeripheralReset(PRCM_GSPI);

}


 /*8888b.   .d88888b.  888b    888 8888888888     8888888 888b    888 88888888888
d88P  Y88b d88P" "Y88b 8888b   888 888              888   8888b   888     888
888    888 888     888 88888b  888 888              888   88888b  888     888
888        888     888 888Y88b 888 8888888          888   888Y88b 888     888
888        888     888 888 Y88b888 888              888   888 Y88b888     888
888    888 888     888 888  Y88888 888              888   888  Y88888     888
Y88b  d88P Y88b. .d88P 888   Y8888 888              888   888   Y8888     888
 "Y8888P"   "Y88888P"  888    Y888 888            8888888 888    Y888     88
 */

//****************************************************************************
//
//!     \brief Configure the interrupt for the GPIO (raising edge on a 
//!     specified Pin)
//!
//!     \param[in]  GPIO Port
//!     \param[in]  GPIO Pin
//!     \param[in]  Type of the interrupt (ex: raising elec edge)
//!     \param[in]  Pointer to a Interrupt handler
//!
//!     \return None
//
//****************************************************************************
void GPIOConfigureNIntEnable(unsigned int uiGPIOPort,
                                  unsigned char ucGPIOPin,
                                  unsigned int uiIntType,
                                  void (*pfnIntHandler)(void))
{
    // Set GPIO interrupt type
    GPIOIntTypeSet(uiGPIOPort,ucGPIOPin,uiIntType);

    // Register Interrupt handler
    IntPrioritySet(GetPeripheralIntNum(uiGPIOPort), INT_PRIORITY_LVL_1);
    GPIOIntRegister(uiGPIOPort,pfnIntHandler);

    // Enable Interrupt
    GPIOIntClear(uiGPIOPort,ucGPIOPin);
    GPIOIntEnable(uiGPIOPort,ucGPIOPin);
}


/*88888 888b    888 88888888888     888b    888 888     888 888b     d888
  888   8888b   888     888         8888b   888 888     888 8888b   d8888
  888   88888b  888     888         88888b  888 888     888 88888b.d88888
  888   888Y88b 888     888         888Y88b 888 888     888 888Y88888P888
  888   888 Y88b888     888         888 Y88b888 888     888 888 Y888P 888
  888   888  Y88888     888         888  Y88888 888     888 888  Y8P  888
  888   888   Y8888     888         888   Y8888 Y88b. .d88P 888   "   888
8888888 888    Y888     888         888    Y888  "Y88888P"  888       8*/

//****************************************************************************
//
//!     \brief Function used to determine on which port the GPIO interrupt 
//!     come from
//!
//!     \param[in] Base adress of the GPIOs Port
//!
//!     \return The GPIO port where the interrupt come frome
//
//****************************************************************************
static unsigned char
GetPeripheralIntNum(unsigned int uiGPIOPort)
{

    switch(uiGPIOPort)
    {
       case GPIOA0_BASE:
          return INT_GPIOA0;
       case GPIOA1_BASE:
          return INT_GPIOA1;
       case GPIOA2_BASE:
          return INT_GPIOA2;
       case GPIOA3_BASE:
          return INT_GPIOA3;
       default:
          return INT_GPIOA0;
    }

}

/*88888b. 8888888  .d8888b.  8888888b.  888             d8888 Y88b   d88P
888  "Y88b  888   d88P  Y88b 888   Y88b 888            d88888  Y88b d88P
888    888  888   Y88b.      888    888 888           d88P888   Y88o88P
888    888  888    "Y888b.   888   d88P 888          d88P 888    Y888P
888    888  888       "Y88b. 8888888P"  888         d88P  888     888
888    888  888         "888 888        888        d88P   888     888
888  .d88P  888   Y88b  d88P 888        888       d8888888888     888
8888888P" 8888888  "Y8888P"  888        88888888 d88P     888     8*/

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
                    Report("%d   : %08x\n\r", i, ul_buffer[i]);
                else if (i < 100)
                    Report("%d  : %08x\n\r", i, ul_buffer[i]);
                else
                    Report("%d : %08x\n\r", i, ul_buffer[i]);
            }
            break;
        }

        case 1: // char number
        {
            char* ch_buffer = (char*) buffer;

            for (i = 0; i < size; i++)
            {
                if (i < 10) // alignment
                    Report("%d   : %02x\n\r", i, ch_buffer[i]);
                else if (i < 100)
                    Report("%d  : %02x\n\r", i, ch_buffer[i]);
                else
                    Report("%d : %02x\n\r", i, ch_buffer[i]);
            }
            break;
        }

        case 2: // char ASCII
        {
            char* ch_buffer = (char*) buffer;

            for (i = 0; i < size; i++)
                    Report("%c", ch_buffer[i]);

            break;
        }

        default:
        {
            Report("wrong type declaration");
            break;
        }
    }
}


/* 8888b. 88888888888     d8888 88888888888     8888888b. 8888888  .d8888b.
d88P  Y88b    888        d88888     888         888  "Y88b  888   d88P  Y88b
Y88b.         888       d88P888     888         888    888  888   Y88b.
 "Y888b.      888      d88P 888     888         888    888  888    "Y888b.
    "Y88b.    888     d88P  888     888         888    888  888       "Y88b.
      "888    888    d88P   888     888         888    888  888         "888
Y88b  d88P    888   d8888888888     888         888  .d88P  888   Y88b  d88P
 "Y8888P"     888  d88P     888     888         8888888P" 8888888  "Y8888*/

//****************************************************************************
//
//!     \brief This function displays the status of a Ringbuffer
//!     it displays the read and write indexes, and the amount of data avail.
//!
//!     \param[in] Pointer to a RingBuffer
//!
//!     \return None
//
//****************************************************************************
void BufferStatusDisplay(RingBuffer *buffer)
{
    Report("\n\rRingBuffer Status\n\r");
    Report("RingBuffer available data = %d\n\r", RingBuffer_availableData(buffer));
    Report("RingBuffer read_index = %d\n\r",     buffer->read_index);
    Report("RingBuffer write_index = %d\n\r",    buffer->write_index);
}


/*88888b.  888     888 888      .d8888b.  8888888888
888   Y88b 888     888 888     d88P  Y88b 888
888    888 888     888 888     Y88b.      888
888   d88P 888     888 888      "Y888b.   8888888
8888888P"  888     888 888         "Y88b. 888
888        888     888 888           "888 888
888        Y88b. .d88P 888     Y88b  d88P 888
888         "Y88888P"  88888888 "Y8888P"  88888888*/

//****************************************************************************
//
//!     \brief Send a certain amount of Pulse on a given Pin of the MCU
//!
//!     \param[in] Pin to send the pulse
//!     \param[in] Number of pulse to send
//!
//!     \return None
//
//****************************************************************************
void DebugPulse(int pin, int amount)
{
    int i;

    switch (pin)
    {
        case PIN_15 :
            for (i = 0; i<amount; i++)
            {
                // Small pulse for logic analyser
                GPIOPinWrite(GPIOA2_BASE, (1 << 6), (1 << 6));  // PIN_15 high
                GPIOPinWrite(GPIOA2_BASE, (1 << 6), 0);         // PIN_15 low
            }
            break;

        case PIN_60 :
            for (i = 0; i<amount; i++)
            {
                // Small pulse for logic analyser
                GPIOPinWrite(GPIOA0_BASE, (1 << 5), (1 << 5));  // PIN_60 high
                GPIOPinWrite(GPIOA0_BASE, (1 << 5), 0);         // PIN_60 low
            }
            break;

        case PIN_50 :
            for (i = 0; i<amount; i++)
            {
                // Small pulse for logic analyser
                GPIOPinWrite(GPIOA0_BASE, (1 << 0), (1 << 0));  // PIN_50 high
                GPIOPinWrite(GPIOA0_BASE, (1 << 0), 0);     // PIN_50 low
            }
            break;

        default :
            DBG_log_warn("wrong pulse pin");
            break;
    }
}


/*8888b.  8888888888 88888888888
d88P  Y88b 888            888
Y88b.      888            888
 "Y888b.   8888888        888
    "Y88b. 888            888
      "888 888            888
Y88b  d88P 888            888
 "Y8888P"  8888888888     8*/

//****************************************************************************
//
//!     \brief This function is used to set a Debug pin High or Low
//!
//!     \param[in] Pin used
//!     \param[in] state of the pin to set (either High or Low)
//!
//!     \return None
//
//****************************************************************************
void DebugPinSet(int pin, int status)
{
    if (status)
    {
       switch (pin) // Set the pin HIGH
       {
            case PIN_60 : GPIOPinWrite(GPIOA0_BASE, (1 << 5), (1 << 5)); break;
            case PIN_50 : GPIOPinWrite(GPIOA0_BASE, (1 << 0), (1 << 0)); break;
            default : DBG_log_warn("wrong pulse pin"); break;
        }
     }
     else // Set it LOW
     {
         switch (pin)
         {
             case PIN_60 : GPIOPinWrite(GPIOA0_BASE, (1 << 5), 0); break;
             case PIN_50 : GPIOPinWrite(GPIOA0_BASE, (1 << 0), 0); break;
             default : DBG_log_warn("wrong pulse pin"); break;
        }
    }
}

/*8      8888888888 8888888b.
888      888        888  "Y88b
888      888        888    888
888      8888888    888    888
888      888        888    888
888      888        888    888
888      888        888  .d88P
88888888 8888888888 888888*/

//****************************************************************************
//
//!     \brief This function is used to set a LED pin High or Low
//!
//!     \param[in] LED used
//!     \param[in] state of the LED to set (either High or Low)
//!
//!     \return None
//
//****************************************************************************
void LedSet(int led_number, int status)
{
    if (status) // Set the Led ON
    {
        switch(led_number)
        {
            case 1: GPIOPinWrite(GPIOA1_BASE, 0x02 , 0x02); break;
            case 2: GPIOPinWrite(GPIOA1_BASE, 0x04, 0x04);  break;
            case 3: GPIOPinWrite(GPIOA1_BASE, 0x08, 0x08);  break;
        }
    }
    else // Set it OFF
    {
        switch(led_number)
        {
            case 1: GPIOPinWrite(GPIOA1_BASE, 0x02 , 0); break;
            case 2: GPIOPinWrite(GPIOA1_BASE, 0x04, 0);  break;
            case 3: GPIOPinWrite(GPIOA1_BASE, 0x08, 0);  break;
        }
    }
}


/*8888b.   888       .d88888b.   .d8888b.  888    d8P
888  "88b  888      d88P" "Y88b d88P  Y88b 888   d8P
888  .88P  888      888     888 888    888 888  d8P
8888888K.  888      888     888 888        888d88K
888  "Y88b 888      888     888 888        8888888b
888    888 888      888     888 888    888 888  Y88b
888   d88P 888      Y88b. .d88P Y88b  d88P 888   Y88b
8888888P"  88888888  "Y88888P"   "Y8888P"  888    Y8*/

//****************************************************************************
//
//!     \brief Function used to send a signal to the FPGA stating that the 
//!     socket send function is stuck and that it should stop sending data 
//!     through SPI to avoid overflowing the RingBuffer.
//!     This function set the pin liked to FPGA to a HIGH state
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void SpiBlock(void)
{
    // Set PIN_15 to 1
    GPIOPinWrite(GPIOA2_BASE, (1 << 6), (1 << 6));
}


/*8     888 888b    888 888888b.   888       .d88888b.   .d8888b.  888    d8P
888     888 8888b   888 888  "88b  888      d88P" "Y88b d88P  Y88b 888   d8P
888     888 88888b  888 888  .88P  888      888     888 888    888 888  d8P
888     888 888Y88b 888 8888888K.  888      888     888 888        888d88K
888     888 888 Y88b888 888  "Y88b 888      888     888 888        8888888b
888     888 888  Y88888 888    888 888      888     888 888    888 888  Y88b
Y88b. .d88P 888   Y8888 888   d88P 888      Y88b. .d88P Y88b  d88P 888   Y88b
 "Y88888P"  888    Y888 8888888P"  88888888  "Y88888P"   "Y8888P"  888    Y8*/

//****************************************************************************
//
//!     \brief Function used to send a signal to the FPGA stating that the 
//!     socket send function is NOT stuck anymore that the sending of data 
//!     through SPI can be resumed.
//!     This function set the pin liked to FPGA to a LOW state
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void SpiUnblock(void)
{
    // Set PIN_15 to 0
    GPIOPinWrite(GPIOA2_BASE, (1 << 6), 0);
}

/*8b    888  .d8888b.   .d8888b.      8888888b.  
8888b   888 d88P  Y88b d88P  Y88b     888   Y88b 
88888b  888 Y88b.      Y88b.          888    888 
888Y88b 888  "Y888b.    "Y888b.       888   d88P 
888 Y88b888     "Y88b.     "Y88b.     8888888P"  
888  Y88888       "888       "888     888 T88b   
888   Y8888 Y88b  d88P Y88b  d88P     888  T88b  
888    Y888  "Y8888P"   "Y8888P"      888   T8*/

//****************************************************************************
//
//!     \brief Read the SPI Nss pin and return its value
//!
//!     \param[in] None
//!
//!     \return valur of the Nss pin 
//
//****************************************************************************
int SpiNssRead(void)
{
    return GPIOPinRead(GPIOA2_BASE, (1 << 1));
}


/*888888888 8888888b.         d8888 888b    888  .d8888b.  8888888888
    888     888   Y88b       d88888 8888b   888 d88P  Y88b 888
    888     888    888      d88P888 88888b  888 Y88b.      888
    888     888   d88P     d88P 888 888Y88b 888  "Y888b.   8888888
    888     8888888P"     d88P  888 888 Y88b888     "Y88b. 888
    888     888 T88b     d88P   888 888  Y88888       "888 888
    888     888  T88b   d8888888888 888   Y8888 Y88b  d88P 888
    888     888   T88b d88P     888 888    Y888  "Y8888P"  8*/

//****************************************************************************
//
//!     \brief  This function is used to start and update the DMA controller
//!     by setting up the required buffers and transfert parameters
//!
//!     \param[in]
//!
//!     \return
//
//****************************************************************************
void SpiDmaTransfer(int primary, unsigned long *rx_buffer, int size_transfert)
{

    if (primary)
    {
        UDMASetupTransfer(  UDMA_CH30_GSPI_RX | UDMA_PRI_SELECT,   // ulChannel | primary
                            UDMA_MODE_PINGPONG,                    // ulMode
                            size_transfert,                        // ulItemCount
                            UDMA_SIZE_32,                          // ulItemSize
                            UDMA_ARB_1,                            // ulArbSize
                            (void *) (GSPI_BASE + MCSPI_O_RX0),    // pvSrcBuf
                            UDMA_SRC_INC_NONE,                     // ulSrcInc
                            rx_buffer,                             // pvDstBuf
                            UDMA_DST_INC_32);                      // ulDstInc
    }
    else
    {
        UDMASetupTransfer(  UDMA_CH30_GSPI_RX | UDMA_ALT_SELECT,  // ulChannel | alternative
                            UDMA_MODE_PINGPONG,                   // ulMode
                            size_transfert,                       // ulItemCount
                            UDMA_SIZE_32,                         // ulItemSize
                            UDMA_ARB_1,                           // ulArbSize
                            (void *) (GSPI_BASE + MCSPI_O_RX0),   // pvSrcBuf
                            UDMA_SRC_INC_NONE,                    // ulSrcInc
                            rx_buffer,                            // pvDstBuf
                            UDMA_DST_INC_32);                     // ulDstInc
    }
}


#if (FAKE_DATA)

/*888888888 8888888 888b     d888     8888888 888b    888 8888888 88888888888
    888       888   8888b   d8888       888   8888b   888   888       888
    888       888   88888b.d88888       888   88888b  888   888       888
    888       888   888Y88888P888       888   888Y88b 888   888       888
    888       888   888 Y888P 888       888   888 Y88b888   888       888
    888       888   888  Y8P  888       888   888  Y88888   888       888
    888       888   888   "   888       888   888   Y8888   888       888
    888     8888888 888       888     8888888 888    Y888 8888888     8*/

//****************************************************************************
//
//!     \brief This function Initialize the timer that will produce a
//!     periodic interrupt used to create fake data for testing purposes
//!
//!     \param[in] No   lne
//!
//!     \return None
//
//****************************************************************************
void Timer_init(void)
{
    ledToggle    = 0;
    g_ulFakeData = 0;

    // Base address for first timer
    BaseTimer = TIMERA0_BASE;

    // Configuring the timers
    Timer_IF_Init(PRCM_TIMERA0, BaseTimer, TIMER_CFG_PERIODIC, TIMER_A, 0);

    // Setup the interrupts for the timer timeouts.
    Timer_IF_IntSetup(BaseTimer, TIMER_A, TimerBaseIntHandler);

    // Turn on the timers feeding values in mSec
    Timer_IF_Start(BaseTimer, TIMER_A, 5);
}
#endif


/*8       888            d8888  .d8888b.  888    d8P
888   o   888           d88888 d88P  Y88b 888   d8P
888  d8b  888          d88P888 888    888 888  d8P
888 d888b 888         d88P 888 888        888d88K
888d88888b888        d88P  888 888        8888888b
88888P Y88888       d88P   888 888    888 888  Y88b
8888P   Y8888      d8888888888 Y88b  d88P 888   Y88b
888P     Y888     d88P     888  "Y8888P"  888    Y8*/

//****************************************************************************
//
//!     \brief clear the watchdog interrupt to prevent watchdog reset
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void watchDogAck (void)
{
    // Clear the watchdog interrupt.
    WatchdogIntClear(WDT_BASE);
}















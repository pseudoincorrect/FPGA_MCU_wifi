#include <common_app.h>

/*88888 888b    888 8888888 88888888888
  888   8888b   888   888       888
  888   88888b  888   888       888
  888   888Y88b 888   888       888
  888   888 Y88b888   888       888
  888   888  Y88888   888       888
  888   888   Y8888   888       888
8888888 888    Y888 8888888     8*/

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
    Message("\t\t                   SPI FIFO  \n\r");
    Message("\t\t   ********************************************\n\r");
}


 /*8888b.  8888888b. 8888888          .d8888b.   .d88888b.  888b    888 8888888888
d88P  Y88b 888   Y88b  888           d88P  Y88b d88P" "Y88b 8888b   888 888
Y88b.      888    888  888           888    888 888     888 88888b  888 888
 "Y888b.   888   d88P  888           888        888     888 888Y88b 888 8888888
    "Y88b. 8888888P"   888           888        888     888 888 Y88b888 888
      "888 888         888           888    888 888     888 888  Y88888 888
Y88b  d88P 888         888           Y88b  d88P Y88b. .d88P 888   Y8888 888
 "Y8888P"  888       8888888          "Y8888P"   "Y88888P"  888    Y888 8*/

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

    // GPIO of logic analyzer debug
    // Configure PIN_15 for GPIO
    PinTypeGPIO(PIN_15, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA2_BASE, (1 << 6), GPIO_DIR_MODE_OUT);

    // GPIO of logic analyzer debug
    // Configure PIN_60 for GPIO
    PinTypeGPIO(PIN_60, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA0_BASE, 0x20, GPIO_DIR_MODE_OUT);

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

//***********************************************************************
//***********************************************************************
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


//***********************************************************************
//***********************************************************************
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

void SendDebugPulse(int pin, int amount)
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
                GPIOPinWrite(GPIOA0_BASE, 0x20, 0x20);  // PIN_60 high
                GPIOPinWrite(GPIOA0_BASE, 0x20, 0);         // PIN_60 low
            }
            break;

        default :
            DBG_log_warn("wrong pulse pin");
            break;
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

void LedSet(int led_number, int status)
{
    if (status)
    {
        switch(led_number)
        {
            case 1:
                GPIOPinWrite(GPIOA1_BASE, 0x02 , 0x02);
                break;
            case 2:
                GPIOPinWrite(GPIOA1_BASE, 0x04, 0x04);
                break;
            case 3:
                GPIOPinWrite(GPIOA1_BASE, 0x08, 0x08);
                break;
        }
    }
    else
    {
        switch(led_number)
        {
            case 1:
                GPIOPinWrite(GPIOA1_BASE, 0x02 , 0);
                break;
            case 2:
                GPIOPinWrite(GPIOA1_BASE, 0x04, 0);
                break;
            case 3:
                GPIOPinWrite(GPIOA1_BASE, 0x08, 0);
                break;
        }
    }
}

// // Example small blink
//LedSet(1,1);    LedSet(2,1);    LedSet(3,1);
//UtilsDelay(500);
//LedSet(1,0);    LedSet(2,0);    LedSet(3,0);
//UtilsDelay(2500000); // 100 ms (maybe)






























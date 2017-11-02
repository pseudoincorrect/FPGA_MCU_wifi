

#include <common_app.h>

//*********************************************************************
//
//                          COMMON APP
//
//  This file contains most of the function used to to initialize
//  peripherals' clock and GPIO port accoring to the pin description
//  of the CC3200
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
void common_init(void)
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

    // Reset the peripheral
//    PRCMPeripheralReset(PRCM_GSPI);
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

//    PRCMPeripheralReset(PRCM_GSPI);

}


















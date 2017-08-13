//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - SPI TCP
// Application Overview:
// This application is used to transmit data from a
// FPGA to a PC through wifi (via a wifi capable MCU, the CC3200 from TI). 
//
// General: 
// A FPGA receive data from several sensor, store them in memory and 
// transmit them to a Microcontroller (MCU) through a SPI connexion. 
// The MCU first connect itself to an Access point (wifi) and then to a 
// receiving server (in our case a python app).
// Afterward, the MCU start accepting the SPI data sent from the FPGA and store
// them in a Ring Buffer. Once the ring buffer is full enough, the MCU send 
// data to a computer through the TCP/IP protocol. Meanwhile, it still receives
// data from the SPI. These parallel processes (SPI reception, TCP transmission)
// repeat endlessly as long as the (socket) connexion between the MCU and the 
// PC stay alive.
// 
// Connexions: 
// A FPGA is connectd to the CC3200 chip. 4 wires are dedicated 
// for the SPI (Sclk, Miso, Mosi, Nss), 1 wire for the SpiBlock signal, 
// and 3 wires to connect the ground of the two boards (essential to maintain 
// signal stability).
// 
//
//*****************************************************************************

// Standard includes
#include <common_app.h>
#include <stdlib.h>
#include <string.h>

// simplelink includes 
#include "simplelink.h"
#include "wlan.h"

// driverlib includes 
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "wdt.h"

// common interface includes 
#include "udma_if.h"
#include "common.h"
#ifndef  NOTERM
#include "uart_if.h"
#include "dbg.h"
#endif
#include "timer_if.h"
#include "wdt_if.h"

// application includes
#include "common_app.h"
#include "net_app.h"


#undef  USER_INPUT_ENABLE
#define APPLICATION_NAME        "TCP Socket"
#define APPLICATION_VERSION     "1.1.1"


#define BUF_SIZE_RX         64
#define RECV_LOOP_DIVIDER   50
#if (FAKE_DATA)
#define END_OF_FRAME        0x34120000 // for data created by timer (for tests)
#else
#define END_OF_FRAME        0x00001234 // for SPI data
#endif
#define INTERNET 1
#define FAKE_DATA 0

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
static int  TransmissionTCP(void);
static void SpiInterruptHandler(void);
void GpioIntHandler(void);
static void SpiDmaInitTransfer();
static void WatchdogIntHandler(void);
void SpiDmaInitTransfer(void);
void ReinitializeDMA(void);
void DeinitDmaSpi(void);
static void InitApp(void);
#if (FAKE_DATA)
static void TimerBaseIntHandler(void)
static void Timer_init(void);
#endif

//*****************************************************************************
//                  STATIC VARIABLES
//*****************************************************************************
volatile tBoolean       g_bFeedWatchdog = true;
static unsigned long    ulRecvData_a [DMA_SIZE_WORD];
static unsigned long    ulRecvData_b [DMA_SIZE_WORD];
static volatile int     semaphore_counter;
static volatile int     spi_block_semaphore;
static char             pre_Tx_buff  [MAX_TCP_SIZE_BYTE];
static char             tx_Buff      [MAX_TCP_SIZE_BYTE];
static RingBuffer       *R_buffer;
#if (FAKE_DATA)
static volatile unsigned long g_ulFakeData;
static volatile unsigned long BaseTimer;
static char ledToggle;
#endif


//*****************************************************************************
//                  INTERRUPT VECTOR TABLE
//*****************************************************************************
// Compiler dependent
extern void (* const g_pfnVectors[])(void);


/*8b     d888        d8888 8888888 888b    888
8888b   d8888       d88888   888   8888b   888
88888b.d88888      d88P888   888   88888b  888
888Y88888P888     d88P 888   888   888Y88b 888
888 Y888P 888    d88P  888   888   888 Y88b888
888  Y8P  888   d88P   888   888   888  Y88888
888   "   888  d8888888888   888   888   Y8888
888       888 d88P     888 8888888 888    Y8*/

void main()

{
    long lRetVal = -1;

    strcpy(pre_Tx_buff, "TEST !\n\r");

    // Initialize all variables
    InitializeNetAppVariables();

    // Board Initialization
    init();

    // init buffer and variables
    InitApp();

    // Configuring UART
    InitTerm();

    watchDogAck(); // Few more seconds before reset if the watchdog is not fed

    // configure the simplelink device to default state
    lRetVal = ConfigureSimpleLinkToDefaultState();
    watchDogAck(); // Few more seconds before reset if the watchdog is not fed

    DBG_check(lRetVal >= 0, "Failed to configure the device in its default state \r\n"
              "\r\n      *** Please RESET the Device ***\r\n");

    UART_PRINT("Device is configured in default state \n\r");

    // Assumption is that the device is configured default state
    lRetVal = sl_Start(0, 0, 0);
    watchDogAck(); // Few more seconds before reset if the watchdog is not fed

    DBG_check(lRetVal >= 0, "Failed to start the device ");

    UART_PRINT("Device started as STATION \n\r");
    UART_PRINT("Connecting to AP..\r\n");

    // Connecting to WLAN AP - Set with static parameters defined at common.h
    // After this call we will be connected and have IP address
    lRetVal = WlanConnect();
    watchDogAck(); // Few more seconds before reset if the watchdog is not fed

    DBG_check(lRetVal >= 0, "Connection to AP failed ");


    while (1)
    {
        lRetVal = TransmissionTCP();
        DBG_log_warn("TCP loop stopped, retrying later..");
        UtilsDelay(20000000);
    }

    error:

    DBG_debug("Exiting Application ...\n\r");

    // power off the Network processor
    lRetVal = sl_Stop(SL_STOP_TIMEOUT);

    while (1)
    {
        _SlNonOsMainLoopTask();
    }
}


/*888888888  .d8888b.  8888888b.       .d8888b.  888      8888888
    888     d88P  Y88b 888   Y88b     d88P  Y88b 888        888
    888     888    888 888    888     888    888 888        888
    888     888        888   d88P     888        888        888
    888     888        8888888P"      888        888        888
    888     888    888 888            888    888 888        888
    888     Y88b  d88P 888            Y88b  d88P 888        888
    888      "Y8888P"  888             "Y8888P"  88888888 888888*/

//****************************************************************************
//
//!     \brief Sending routine (through TCP/IP)
//!     This is one of the two the main routines of this Application. 
//!     This function opens a TCP socket and tries to connect to a 
//!     Server IP_ADDR waiting on port PORT_NUM with net_app functions.
//!     It checks by polling if there are new data available on the RingBuffer 
//!     and send them in the positive case. Data are added to the ringBuffer 
//!     through the SPI interrupt handler. the semaphores are used to detect
//!     Wether the socket_send() function get stuck (due to wifi, TCP/IP protocols)
//!     Which happen often in a normal case.
//!
//!     \param[in]  None
//!
//!     \return    -1  when the TCP connection is interrupted.
//
//****************************************************************************
static int TransmissionTCP(void)
{
    int iStatus;
    short BuffFillLen;
    void *result;

    iStatus = TcpClientConnect();
    watchDogAck(); // Few more seconds before reset if the watchdog is not fed
    DBG_check(iStatus == 0, "connection error in TcpClientConnect");

    // create a message test string
    strcpy(pre_Tx_buff, "TEST !\n\r");
    iStatus = net_send_data(pre_Tx_buff, 8);
    DBG_check(iStatus >= 0, "SEND_ERROR (first transmission) error: %d", iStatus);

    // configure SPI DMA
    SpiDmaInitTransfer();
    SpiUnblock();

    UART_PRINT("Entering infinite sending loop\n\r");

    // init the timer interrupt for testing the system without a SPI peripheral
    //Timer_init();

    // loop that send data over TCP when the buffer is ready
    while (1)
    {
        // Check if data are available in the RingBuffer
        BuffFillLen = RingBuffer_availableData(R_buffer);
        watchDogAck(); // Few more seconds before reset if the watchdog is not fed

        // send data if there are
        if (BuffFillLen > 0)
        {
            RingBuffer_read(R_buffer, pre_Tx_buff, sizeof(pre_Tx_buff));

            RingBuffer_commitRead(R_buffer);

            result = memcpy(tx_Buff,
                            pre_Tx_buff,
                            MAX_TCP_SIZE_BYTE - 4);

            DBG_check(result != NULL, "Failed to write buffer into data.");

            // logic analyzer to measure and periodicity of net_send_data()
            DebugPinSet(PIN_60, 1);

            iStatus = net_send_data(tx_Buff, MAX_TCP_SIZE_BYTE - 4);
            watchDogAck(); // Few more seconds before reset if the watchdog is not fed

            DebugPinSet(PIN_60, 0);

            // Keep the SPI enable on the FPGA side since the send function worked
            SpiUnblock();

            // Used in Spi interrup handler to check weither the send funcion
            // got stuck due wifi TCP/IP protocol (data collisions)
            semaphore_counter++;

            // Check weither all data have been sent or weither an error occured
            DBG_check(iStatus == (MAX_TCP_SIZE_BYTE - 4), 
                      "SEND_ERROR (main transmission) error: %d",
                      iStatus);
        }
    }

    // We arrive in here usually when the Connection with the server
    // has been interrupted.
    error:

    DebugPulse(PIN_15, 100); // out pulse for logic analyzer
    UART_PRINT("DEinitialising SPI DMA\n\r");

    DeinitDmaSpi();

    UART_PRINT("Closing socket\n\r");

    iStatus = net_close_connection();

    //closing the socket after sending 1000 packets
    DBG_check(iStatus >= 0, "error while closing socket");

    return FAILURE;
}


/*8888b.  8888888b. 8888888     8888888 888b    888 88888888888
d88P  Y88b 888   Y88b  888         888   8888b   888     888
Y88b.      888    888  888         888   88888b  888     888
 "Y888b.   888   d88P  888         888   888Y88b 888     888
    "Y88b. 8888888P"   888         888   888 Y88b888     888
      "888 888         888         888   888  Y88888     888
Y88b  d88P 888         888         888   888   Y8888     888
 "Y8888P"  888       8888888     8888888 888    Y888     8*/

//****************************************************************************
//
//!     \brief  SPI Interrupt Handler
//!     This is one of the second main routines of this Application. 
//!     A SPI interrupt occurs when 256 bits has been received by the DMA
//!     controller to a desired buffer (here: ulRecvData_a and _b).
//!     When it occurs, we clear the interrupt, check if it is a DMA related
//!     one. 
//!     Then we check weither the socket_send() function is stuck, and in
//!     the positive case, we send a signal (SpiBlock) to the FPGA stating to 
//!     stop sending Data through SPI until the function get unstuck. 
//!     Then, we copy the data received by the SPI/DMA to the ringbuffer and 
//!     prepare the DMA peripheral for a new SPI data reception by updating 
//!     the receiving buffer.
//!     If this fucntion seems to have two "almost" identical parts, it is 
//!     because we are using the "PING-PONG DMA". Which means that the DMA 
//!     Have two buffers to work on. When one buffer is full, the second one
//!     takes the lead, which allows to use the DMA witout dead times.   
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
// Handler for the SPI interrupt
static void SpiInterruptHandler()
{
    static int spi_block_counter;
    static int prev_semaphore_counter;
    unsigned long ulStatus;
    unsigned long ulMode;

    ulStatus = SPIIntStatus(GSPI_BASE,true);

    SPIIntClear(GSPI_BASE, ulStatus);

    if (ulStatus == SPI_INT_DMARX)
    {

        DebugPinSet(PIN_50, 1);

        //Check if the PRIMARY_BUFFER transfer has been completed
        ulMode = uDMAChannelModeGet(UDMA_CH30_GSPI_RX | UDMA_PRI_SELECT);

        // Detection of the socket_send() fucntion blockage
        if (prev_semaphore_counter == semaphore_counter)
        {
            spi_block_counter++;
            // We block the SPI when we receive SPI_BLOCK_COUNTER spi frame
            // and that the send function is still stuck
            if (spi_block_counter > SPI_BLOCK_COUNTER)
            {
                spi_block_semaphore = 1;
                SpiBlock();
                LedSet(1, 1);
            }
        }
        else
        {
            spi_block_counter = 0;
            spi_block_semaphore = 0;
            SpiUnblock();
            LedSet(1, 0);
        }
        prev_semaphore_counter = semaphore_counter;

        if(ulMode == UDMA_MODE_STOP)
        {
            // At the end of each SPI transmission, the last word  of the spi frame
            // (position 256) transmitted by the FPGA is equal to (uint32_t) END_OF_FRAME 
            // To whether we are desync with the SPI we check that the last SPI word is 
            // equal to (uint32_t) END_OF_FRAME. 
            // If not, it means that our SPI clock is desychronised
            // It happens because of electrical interference in the linked wires.
            // We resynchronize the SPI by waiting until the end of the transmission
            // (until Nss become HIGH again) by restarting it with ReinitializeDMA()
            if ( ! (ulRecvData_a[sizeof(ulRecvData_a) / sizeof(uint32_t) - 1] 
                == END_OF_FRAME))
            {
                // Poll until we arrive to the end of a SPI buffer (to resync)
                while (! SpiNssRead())
                {;}

                ReinitializeDMA();

                // nSS low means that the "low time" of the spi is to short for the 
                // MCU to keep up with the data rate SPI, we need to lower it
                if (! SpiNssRead())
                    UART_PRINT("not enough time to resync, need to tune the spi\n\r");
            }
            else
            {
                RingBuffer_write(R_buffer,
                                 (char*) (ulRecvData_a),
                                 sizeof(ulRecvData_a));

                RingBuffer_commitWrite(R_buffer);
            }
            SpiDmaTransfer(PRIMARY_BUFFER, ulRecvData_a, DMA_SIZE_WORD);
        }

        //Check if the ALTERNATIVE_BUFFER transfer has been completed
        ulMode = uDMAChannelModeGet(UDMA_CH30_GSPI_RX | UDMA_ALT_SELECT);

        if(ulMode == UDMA_MODE_STOP)
        {
            //check whether we are desync with the SPI, nSS should be high
            if ( ! (ulRecvData_b[sizeof(ulRecvData_b) / sizeof(uint32_t)  - 1] 
                == END_OF_FRAME))
            {
                // Poll until we arrive to the end of a SPI buffer (to resync)
                while (! SpiNssRead())
                {;}

                ReinitializeDMA();

                // nSS low means that we "low time" of the spi is to short
                if (! SpiNssRead())
                    UART_PRINT("not enough time to resync, need to tune the spi\n\r");
            }
            else
            {
                RingBuffer_write(R_buffer,
                                 (char*) (ulRecvData_b),
                                 sizeof(ulRecvData_b));

                RingBuffer_commitWrite(R_buffer);
            }
            SpiDmaTransfer(ALTERNATIVE_BUFFER, ulRecvData_b, DMA_SIZE_WORD);
        }
            DebugPinSet(PIN_50, 0);
    }

    else // Unwanted interrupts
    {
        DBG_debug("Error: Unexpected SPI interrupt : %08x\n\r", ulStatus);
    }
}

#if (FAKE_DATA)

/*888{888888 8888888 888b     d888 8888888888 8888888b.
    888       888   8888b   d8888 888        888   Y88b
    888       888   88888b.d88888 888        888    888
    888       888   888Y88888P888 8888888    888   d88P
    888       888   888 Y888P 888 888        8888888P"
    888       888   888  Y8P  888 888        888 T88b
    888       888   888   "   888 888        888  T88b
    888     8888888 888       888 8888888888 888   T8*/

//****************************************************************************
//
//!     \brief Timer handler interrupt
//!     This function create fake cata and send it to the RingBuffer
//!     The data created are simply a increasing counter
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
// Handler for the Timer interrupt
static void TimerBaseIntHandler(void)
{
    int i;

    // Clear the timer interrupt.
    Timer_IF_InterruptClear(BaseTimer);

    for (i = 0; i < 255; i++)
        ulRecvData_a[i] = g_ulFakeData++;

    ulRecvData_a[255] = 0x12340000;

    RingBuffer_write(R_buffer,
                     (char*) (ulRecvData_a),
                     sizeof(ulRecvData_a));

    RingBuffer_commitWrite(R_buffer);

    if (ledToggle)
        LedSet(LED1, ON);
    else
        LedSet(LED1, OFF);

   ledToggle = ~ledToggle;
}
#endif


/*8888b.  8888888b. 8888888  .d88888b.
d88P  Y88b 888   Y88b  888   d88P" "Y88b
888    888 888    888  888   888     888
888        888   d88P  888   888     888
888  88888 8888888P"   888   888     888
888    888 888         888   888     888
Y88b  d88P 888         888   Y88b. .d88P
 "Y8888P88 888       8888888  "Y88888*/

//****************************************************************************
//
//!     \brief  GPIO interrupt handle
//!     This function is used to handle GPIO input event
//!     It was used to detect the end of SPI transmission
//!     Will be usefull to implement future feature, hence kept
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
// Handler for the GPIO interrupt
void GpioIntHandler()
{
    unsigned long ulStatus;
    ulStatus = GPIOIntStatus(GPIOA2_BASE,true);
    GPIOIntClear(GPIOA2_BASE, ulStatus);
}


/*8       888        d8888 88888888888  .d8888b.  888    888 8888888b.
888   o   888       d88888     888     d88P  Y88b 888    888 888  "Y88b
888  d8b  888      d88P888     888     888    888 888    888 888    888
888 d888b 888     d88P 888     888     888        8888888888 888    888
888d88888b888    d88P  888     888     888        888    888 888    888
88888P Y88888   d88P   888     888     888    888 888    888 888    888
8888P   Y8888  d8888888888     888     Y88b  d88P 888    888 888  .d88P
888P     Y888 d88P     888     888      "Y8888P"  888    888 8888888*/

//*****************************************************************************
//
//! \brief The interrupt handler for the watchdog timer
//!         A watchdog reset occurs when a watchdog interrupt before the previous
//!         watchdog interrupt has been cleared. We need to clear the interrupt
//!         constantly throughout the program to avoid undesirable resets
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void WatchdogIntHandler(void)
{
    // if we satisfy that condition: direct reset
    if(!g_bFeedWatchdog) { return; }

    //watchDogAck(); // Few more seconds before reset if the watchdog is not fed
}



/*8888b.  8888888b. 8888888     8888888 888b    888 8888888 88888888888
d88P  Y88b 888   Y88b  888         888   8888b   888   888       888
Y88b.      888    888  888         888   88888b  888   888       888
 "Y888b.   888   d88P  888         888   888Y88b 888   888       888
    "Y88b. 8888888P"   888         888   888 Y88b888   888       888
      "888 888         888         888   888  Y88888   888       888
Y88b  d88P 888         888         888   888   Y8888   888       888
 "Y8888P"  888       8888888     8888888 888    Y888 8888888     8*/

//****************************************************************************
//
//!     \brief SPI DMA Initialisation
//!     This function initialize the SPI and the DMA peripheral and start
//!     a first transfert to enable the DMA controller
//!
//!     \param[in] None
//!
//!     \return  None
//
//****************************************************************************
void SpiDmaInitTransfer(void)
{
    // Set up the SPI FIFO DMA
    Spiconf(SpiInterruptHandler);

    SpiDmaTransfer(PRIMARY_BUFFER, ulRecvData_a, DMA_SIZE_WORD);

    SpiDmaTransfer(ALTERNATIVE_BUFFER, ulRecvData_b, DMA_SIZE_WORD);

    SPIEnable(GSPI_BASE);
}


/*88888b.   .d8888b. 88888888888     8888888b.  888b     d888        d8888
888   Y88b d88P  Y88b    888         888  "Y88b 8888b   d8888       d88888
888    888 Y88b.         888         888    888 88888b.d88888      d88P888
888   d88P  "Y888b.      888         888    888 888Y88888P888     d88P 888
8888888P"      "Y88b.    888         888    888 888 Y888P 888    d88P  888
888 T88b         "888    888         888    888 888  Y8P  888   d88P   888
888  T88b  Y88b  d88P    888         888  .d88P 888   "   888  d8888888888
888   T88b  "Y8888P"     888         8888888P"  888       888 d88P     8*/

//****************************************************************************
//
//!     \brief  This function is used to reinit the DMA controller.
//!     It is usually  trigger when the TCP connection fail and that we need
//!     to restart fully the system
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
void ReinitializeDMA(void)
{
    UDMADeInit();
    SpiDmaInitTransfer();

    SPIEnable(GSPI_BASE);
}


/*88888b.  8888888888     8888888b.  888b     d888        d8888
888  "Y88b 888            888  "Y88b 8888b   d8888       d88888
888    888 888            888    888 88888b.d88888      d88P888
888    888 8888888        888    888 888Y88888P888     d88P 888
888    888 888            888    888 888 Y888P 888    d88P  888
888    888 888            888    888 888  Y8P  888   d88P   888
888  .d88P 888            888  .d88P 888   "   888  d8888888888
8888888P"  8888888888     8888888P"  888       888 d88P     8*/

//****************************************************************************
//
//!     \brief  This function is used to start Disable the DMA controller
//!     and its interrupts
//!
//!     \param[in]
//!
//!     \return
//
//****************************************************************************
void DeinitDmaSpi(void)
{
    SPIDisable(GSPI_BASE);
    SPIDmaDisable(GSPI_BASE, SPI_RX_DMA);

    SPIIntDisable(GSPI_BASE, SPI_INT_DMARX);
    SPIIntUnregister(GSPI_BASE);

    SPIReset(GSPI_BASE);
}


/*     d8888 8888888b.  8888888b.      888     888      d8888 8888888b.
      d88888 888   Y88b 888   Y88b     888     888     d88888 888   Y88b
     d88P888 888    888 888    888     888     888    d88P888 888    888
    d88P 888 888   d88P 888   d88P     Y88b   d88P   d88P 888 888   d88P
   d88P  888 8888888P"  8888888P"       Y88b d88P   d88P  888 8888888P"
  d88P   888 888        888              Y88o88P   d88P   888 888 T88b
 d8888888888 888        888               Y888P   d8888888888 888  T88b
d88P     888 888        888                Y8P   d88P     888 888   T8*/

//****************************************************************************
//
//!     \brief This function initialize all the variables of this Application
//!
//!     \param[in] None
//!
//!     \return None
//
//****************************************************************************
static void InitApp(void)
{
    int iCounter;
    semaphore_counter = 0;

    LedSet (LED1, OFF);
    LedSet (LED2, OFF);
    LedSet (LED3, OFF);

    // Init the watchdog
    WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(WD_PERIOD_MS));

    // init the Ring buffer
    R_buffer = RingBuffer_create(BUFFER_SIZE, DMA_SIZE_WORD * sizeof(uint32_t));

    // filling the buffers Tx and Rx
    for (iCounter = 0; iCounter < sizeof(pre_Tx_buff); iCounter++)
        pre_Tx_buff[iCounter] =  '\0';
}








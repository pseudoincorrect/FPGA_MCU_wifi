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
// Application Name     - TCP Socket
// Application Overview - This particular application illustrates how this
//                        device can be used as a client or server for TCP
//                        communication.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_TCP_Socket_Application
// or
// docs\examples\CC32xx_TCP_Socket_Application.pdf
//
//*****************************************************************************


//****************************************************************************
//
//! \addtogroup tcp_socket
//! @{
//
//****************************************************************************

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

// common interface includes 
#include "udma_if.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

// application includes
#include "common_app.h"
#include "dbg.h"


#undef  USER_INPUT_ENABLE
#define APPLICATION_NAME        "TCP Socket"
#define APPLICATION_VERSION     "1.1.1"

#define IP_ADDR             0xc0a80202
#define PORT_NUM            5001
#define BUF_SIZE_RX         64
#define TCP_PACKET_COUNT    20

#define RECV_LOOP_DIVIDER   50

//#define END_OF_FRAME        0x00001234
#define END_OF_FRAME        0x34120000
#define TCP 1

// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SOCKET_CREATE_ERROR = -0x7D0,
    BIND_ERROR = SOCKET_CREATE_ERROR - 1,
    LISTEN_ERROR = BIND_ERROR -1,
    SOCKET_OPT_ERROR = LISTEN_ERROR -1,
    CONNECT_ERROR = SOCKET_OPT_ERROR -1,
    ACCEPT_ERROR = CONNECT_ERROR - 1,
    SEND_ERROR = ACCEPT_ERROR -1,
    RECV_ERROR = SEND_ERROR -1,
    SOCKET_CLOSE_ERROR = RECV_ERROR -1,
    DEVICE_NOT_IN_STATION_MODE = SOCKET_CLOSE_ERROR - 1,
    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
#if (TCP)
static int  BsdTcpClient(unsigned short usPort);
#else
static int  BsdUdpClient(unsigned short usPort);
#endif

static long WlanConnect();
static long ConfigureSimpleLinkToDefaultState();
static void InitializeNetAppVariables();
static void SpiInterruptHandler();
static void SpiDmaTransfer(int primary, unsigned long *rx_buffer, int size_transfert);
static void SpiDmaInitTransfer();
void InitBuffers();
void ReinitializeDMA();
void DeinitDmaSpi(void);

//*****************************************************************************
//                  GLOBAL VARIABLES
//*****************************************************************************
volatile unsigned long  g_ulStatus = 0;//SimpleLink Status
volatile unsigned long  g_ulPacketCount = TCP_PACKET_COUNT;
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
unsigned long  g_ulDestinationIp = IP_ADDR;
unsigned int   g_uiPortNum = PORT_NUM;
unsigned char  g_ucConnectionStatus = 0;
unsigned char  g_ucSimplelinkstarted = 0;
unsigned long  g_ulIpAddr = 0;

char pre_Tx_buff  [DMA_SIZE_WORD * sizeof(uint32_t)];
char g_cBsdBuf_Tx [MAX_TCP_SIZE_BYTE];

//char pre_Tx_buff_2[DMA_SIZE_WORD * 4 * 2];
char g_cBsdBuf_Rx[BUF_SIZE_RX];

unsigned long ulRecvData_a[DMA_SIZE_WORD];
unsigned long ulRecvData_b[DMA_SIZE_WORD];

RingBuffer *R_buffer;

// sizeof(myText) = 753
char myText[]=
        "88888888888 8888888888  .d8888b. 88888888888 \n\r"
        "    888     888        d88P  Y88b    888     \n\r"
        "    888     888        Y88b.         888     \n\r"
        "    888     8888888      Y888b.      888     \n\r"
        "    888     888             Y88b.    888     \n\r"
        "    888     888               888    888     \n\r"
        "    888     888        Y88b  d88P    888     \n\r"
        "    888     8888888888   Y8888P      888     \n\r"
        "\n\r"
        "The Internet is the global system of interconnected computer networks that use the \n\r"
        "Internet protocol suite (TCP/IP) to link devices worldwide. It is a network of \n\r"
        "networks that consists of private, public, academic, business, and government \n\r"
        "networks of local to global scope, linked by a broad array of electronic, wireless, \n\r"
        "and optical networking technologies.\n\r"
        "\n\r"
        "\n\r";

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

    InitBuffers();

    strcpy(pre_Tx_buff, "TEST !\n\r");

    // Initialize all variables
    InitializeNetAppVariables();

    // Board Initialization
    init();

    // Configuring UART
    InitTerm();

    //    DBG_debug("sizeof(myText) = %d", sizeof(myText));

    // configure the simplelink device to default state
    lRetVal = ConfigureSimpleLinkToDefaultState();

    DBG_check(lRetVal >= 0, "Failed to configure the device in its default state ");

    UART_PRINT("Device is configured in default state \n\r");

    // Assumption is that the device is configured default state
    lRetVal = sl_Start(0, 0, 0);

    DBG_check(lRetVal >= 0, "Failed to start the device ");

    UART_PRINT("Device started as STATION \n\r");
    UART_PRINT("Connecting to AP: %s ...\r\n", SSID_NAME);

    // Connecting to WLAN AP - Set with static parameters defined at common.h
    // After this call we will be connected and have IP address
    lRetVal = WlanConnect();

    DBG_check(lRetVal >= 0, "Connection to AP failed ");

    UART_PRINT("Connected to AP: %s \n\r", SSID_NAME);
    UART_PRINT( "Device IP: %d.%d.%d.%d\n\r\n\r",
                SL_IPV4_BYTE(g_ulIpAddr,3),
                SL_IPV4_BYTE(g_ulIpAddr,2),
                SL_IPV4_BYTE(g_ulIpAddr,1),
                SL_IPV4_BYTE(g_ulIpAddr,0));
    UART_PRINT( "Default settings: SSID Name: %s, PORT = %d, \n\rPacket Count = %d, "
                "Destination IP: %d.%d.%d.%d\n\r",
                SSID_NAME, g_uiPortNum, g_ulPacketCount,
                SL_IPV4_BYTE(g_ulDestinationIp,3),
                SL_IPV4_BYTE(g_ulDestinationIp,2),
                SL_IPV4_BYTE(g_ulDestinationIp,1),
                SL_IPV4_BYTE(g_ulDestinationIp,0));

    while (1)
    {
#if (TCP)
        lRetVal = BsdTcpClient(PORT_NUM);
        DBG_log_warn("TCP loop stopped, retrying later");
#else
        lRetVal = BsdUdpClient(PORT_NUM);
        DBG_log_warn("UDP loop stopped, retrying later");
#endif

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


#if (TCP)

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
//! \brief Opening a TCP client side socket and sending data
//!
//! This function opens a TCP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    If the socket connection is successful then the function will send 1000
//! TCP packets to the server.
//!
//! \param[in]      port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
static int BsdTcpClient(unsigned short usPort)
{
    // declarations
    int iAddrSize;
    int iSockID;
    int iStatus;
    short BuffFillLen   = 0;
    SlSockAddrIn_t  sAddr;
    int i;
    void *result;

    // filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = htons((unsigned short)usPort);
    sAddr.sin_addr.s_addr = htonl((unsigned int)g_ulDestinationIp);

    iAddrSize = sizeof(SlSockAddrIn_t);

    UART_PRINT("TCP client function\n\r");

    // create a message test string
    strcpy(pre_Tx_buff, "TEST !\n\r");

    UART_PRINT("Socket creation\n\r");

    // creating a TCP socket
    iSockID = socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    DBG_check(iSockID >= 0, "SOCKET_CREATE_ERROR");

    UART_PRINT("Socket connection\n\r");
    iStatus = connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
    DBG_check(iStatus >= 0, "CONNECT ERROR");

    // send a first message
    UART_PRINT("Sending a test message\n\r");

    iStatus = send(iSockID, pre_Tx_buff, 8,  0);
    DBG_check(iStatus >= 0, "SEND_ERROR (first transmission) error: %d", iStatus);

    // configure SPI DMA
    UART_PRINT("SPI DMA Init\n\r");

    SpiDmaInitTransfer();

    UART_PRINT("entering infinite TCP loop\n\r");
    // loop that send data over TCP when the buffer is ready

    while (1)
    {
        BuffFillLen = RingBuffer_availableData(R_buffer);

        // send data if there are enough data to send
        if (BuffFillLen > 0)
        {
            SendDebugPulse(PIN_60, 3); // out pulse for logic analyzer

            RingBuffer_read(R_buffer, pre_Tx_buff, sizeof(pre_Tx_buff));

            RingBuffer_commitRead(R_buffer);

            SPIIntDisable(GSPI_BASE, SPI_INT_DMARX);

            // for (i = 0; i < DMA_SIZE_WORD / MAX_TCP_SIZE_WORD; i++)
            // {
                result = memcpy(g_cBsdBuf_Tx,
                                pre_Tx_buff, // + i * DMA_SIZE_WORD,
                                MAX_TCP_SIZE_BYTE - 4);

                DBG_check(result != NULL, "Failed to write buffer into data.");

//                 BufferDisplay(2,  g_cBsdBuf_Tx, MAX_TCP_SIZE_BYTE - 4);

                iStatus = sl_Send(  iSockID,
                                    g_cBsdBuf_Tx,
                                    MAX_TCP_SIZE_BYTE - 4,
                                    0);

                DBG_check(iStatus >= 0, "SEND_ERROR (main transmission) error: %d", iStatus);

                SendDebugPulse(PIN_60, 1); // out pulse for logic analyzer
            // }
            SPIIntEnable(GSPI_BASE, SPI_INT_DMARX);
        }
    }

    error:

    SendDebugPulse(PIN_15, 100); // out pulse for logic analyzer
    UART_PRINT("DEinitialising SPI DMA\n\r");

    DeinitDmaSpi();

    UART_PRINT("Closing socket\n\r");
    iStatus = sl_Close(iSockID);
    //closing the socket after sending 1000 packets
    DBG_check(iStatus >= 0, "error while closing socket");

    return FAILURE;
}

#else


/*8     888 8888888b.  8888888b.       .d8888b.  888      8888888
888     888 888  "Y88b 888   Y88b     d88P  Y88b 888        888
888     888 888    888 888    888     888    888 888        888
888     888 888    888 888   d88P     888        888        888
888     888 888    888 8888888P"      888        888        888
888     888 888    888 888            888    888 888        888
Y88b. .d88P 888  .d88P 888            Y88b  d88P 888        888
 "Y88888P"  8888888P"  888             "Y8888P"  88888888 88888*/

//****************************************************************************
//
//! \brief Opening a UDP client side socket and sending data
//!
//! This function opens a UDP socket and tries to connect to a Server IP_ADDR
//!    waiting on port PORT_NUM.
//!    Then the function will send 1000 UDP packets to the server.
//!
//! \param[in]  port number on which the server will be listening on
//!
//! \return    0 on success, -1 on Error.
//
//****************************************************************************
static int BsdUdpClient(unsigned short usPort)
{
    short   BuffFillLen;
    int     iAddrSize;
    int     iSockID;
    int     iStatus;
    int     i;

    SlSockAddrIn_t  sAddr;

    BuffFillLen  = 0;

    //filling the UDP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)usPort);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)g_ulDestinationIp);

    iAddrSize = sizeof(SlSockAddrIn_t);

    UART_PRINT(" UDP client function\n\r");

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    DBG_check(iSockID >= 0, "SOCKET_CREATE_ERROR");

    UART_PRINT("Socket set non blocking mode\n\r");

    // send a first message
    UART_PRINT("Sending a test message\n\r");
    iStatus = sl_SendTo(iSockID, pre_Tx_buff, 14, 0, (SlSockAddr_t *)&sAddr, iAddrSize);
    DBG_check(iStatus >= 0, "SEND_ERROR (first transmission)");

    // Configure SPI DMA
    UART_PRINT("SPI DMA Init\n\r");
    SpiDmaInitTransfer();

    UART_PRINT("entering infinite UDP loop\n\r");

    // loop that send data over UDP when data are available
    while (1)
    {
        //        SPIIntDisable(GSPI_BASE, SPI_INT_DMARX);
        BuffFillLen = RingBuffer_availableData(R_buffer);
        //        SPIIntEnable(GSPI_BASE, SPI_INT_DMARX);

        // send data if there are enough data to send
        if (BuffFillLen > 0)
        {
            SendDebugPulse(PIN_60, 3); // out pulse for logic analyzer

            RingBuffer_read(R_buffer, pre_Tx_buff, sizeof(pre_Tx_buff));

            SendDebugPulse(PIN_60, 2); // out pulse for logic analyzer

            RingBuffer_commitRead(R_buffer);

            SendDebugPulse(PIN_60, 2); // out pulse for logic analyzer

            for (i = 0; i < DMA_SIZE_WORD / MAX_TCP_SIZE_WORD; i++)
            {
                iStatus = sl_SendTo(iSockID,
                                    pre_Tx_buff + i * DMA_SIZE_WORD,
                                    MAX_TCP_SIZE_BYTE - 1* sizeof(uint32_t), //BuffFillLen,
                                    0,
                                    (SlSockAddr_t *)&sAddr,
                                    iAddrSize);

                DBG_check(iStatus >= 0, "SEND_ERROR continuoust transmission)");
                SendDebugPulse(PIN_60, 1); // out pulse for logic analyzer
            }
        }
    }

    error:

    iStatus = sl_Close(iSockID);
    DBG_check(iStatus >= 0, "error while closing socket");

    return FAILURE;
}

#endif



//*****************************************************************************
//                        Event Handlers -- Start
//*****************************************************************************

/*8888b.  8888888b. 8888888
d88P  Y88b 888   Y88b  888
Y88b.      888    888  888
 "Y888b.   888   d88P  888
    "Y88b. 8888888P"   888
      "888 888         888
Y88b  d88P 888         888
 "Y8888P"  888       888888*/

static void SpiInterruptHandler()
{
    unsigned long ulStatus;
    unsigned long ulMode;

    ulStatus = SPIIntStatus(GSPI_BASE,true);

    SPIIntClear(GSPI_BASE, ulStatus);

    if (ulStatus == SPI_INT_DMARX)
    {

//        SendDebugPulse(PIN_15, 2); // out pulse for logic analyzer

        //Check if the PRIMARY_BUFFER transfer has been completed
        ulMode = uDMAChannelModeGet(UDMA_CH30_GSPI_RX | UDMA_PRI_SELECT);

        if(ulMode == UDMA_MODE_STOP)
        {
            //check whether we are desync with the SPI, nSS should be high
            if ( ! (ulRecvData_a[sizeof(ulRecvData_a) / sizeof(uint32_t) - 1] == END_OF_FRAME))
            {
                // Poll until we arrive to the end of a SPI buffer (to resync)
                while (! GPIOPinRead(GPIOA2_BASE, (1 << 1)))
                {;}

                ReinitializeDMA();

                // nSS low means that we "low time" of the spi is to short
                if (! GPIOPinRead(GPIOA2_BASE, (1 << 1)))
                    UART_PRINT("not enough time to resync, need to tune the spi\n\r");

                SendDebugPulse(PIN_15, 10); // out pulse for logic analyzer
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
            if ( ! (ulRecvData_b[sizeof(ulRecvData_b) / sizeof(uint32_t)  - 1] == END_OF_FRAME))
            {
                // Poll until we arrive to the end of a SPI buffer (to resync)
                while (! GPIOPinRead(GPIOA2_BASE, (1 << 1)))
                {;}

                ReinitializeDMA();

                // nSS low means that we "low time" of the spi is to short
                if (! GPIOPinRead(GPIOA2_BASE, (1 << 1)))
                    UART_PRINT("not enough time to resync, need to tune the spi\n\r");

                SendDebugPulse(PIN_15, 10); // out pulse for logic analyzer
            }
            else
            {
                RingBuffer_write(R_buffer,
                                 (char*) (ulRecvData_b),
                                 sizeof(ulRecvData_b));
            }
            SpiDmaTransfer(ALTERNATIVE_BUFFER, ulRecvData_b, DMA_SIZE_WORD);
        }

//        SendDebugPulse(PIN_15, 2); // out pulse for logic analyzer
    }

    // Check for unwanted interrupts
    else
    {
        DBG_debug("Error: Unexpected SPI interrupt : %08x\n\r", ulStatus);
        SendDebugPulse(PIN_15, 1000); // out pulse for logic analyzer
        // while(1){};
    }
}


/*8888b.  8888888b. 8888888  .d88888b.
d88P  Y88b 888   Y88b  888   d88P" "Y88b
888    888 888    888  888   888     888
888        888   d88P  888   888     888
888  88888 8888888P"   888   888     888
888    888 888         888   888     888
Y88b  d88P 888         888   Y88b. .d88P
 "Y8888P88 888       8888888  "Y88888*/

void GpioIntHandler()
{
    //unsigned long ulStatus;
    //ulStatus = GPIOIntStatus(GPIOA2_BASE,true);
    //GPIOIntClear(GPIOA2_BASE, ulStatus);
    ;
}


/*8       888 888             d8888 888b    888
888   o   888 888            d88888 8888b   888
888  d8b  888 888           d88P888 88888b  888
888 d888b 888 888          d88P 888 888Y88b 888
888d88888b888 888         d88P  888 888 Y88b888
88888P Y88888 888        d88P   888 888  Y88888
8888P   Y8888 888       d8888888888 888   Y8888
888P     Y888 88888888 d88P     888 888    Y8*/

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(!pWlanEvent)
    {
        return;
    }

    switch(pWlanEvent->Event)
    {
    case SL_WLAN_CONNECT_EVENT:
    {
        SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

        //
        // Information about the connected AP (like name, MAC etc) will be
        // available in 'slWlanConnectAsyncResponse_t'-Applications
        // can use it if required
        //
        //  slWlanConnectAsyncResponse_t *pEventData = NULL;
        // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
        //

        // Copy new connection SSID and BSSID to global parameters
        memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
               STAandP2PModeWlanConnected.ssid_name,
               pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
        memcpy(g_ucConnectionBSSID,
               pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
               SL_BSSID_LENGTH);

        UART_PRINT( "[WLAN EVENT] STA Connected to the AP: %s ,"
                " BSSID: %x:%x:%x:%x:%x:%x\n\r",
                g_ucConnectionSSID,g_ucConnectionBSSID[0],
                g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                g_ucConnectionBSSID[5]);
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
        slWlanConnectAsyncResponse_t*  pEventData = NULL;

        CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

        pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

        // If the user has initiated 'Disconnect' request,
        //'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION
        if(SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
        {
            UART_PRINT( "[WLAN EVENT]Device disconnected from the AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                    g_ucConnectionSSID,g_ucConnectionBSSID[0],
                    g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                    g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                    g_ucConnectionBSSID[5]);
        }
        else
        {
            UART_PRINT( "[WLAN ERROR]Device disconnected from the AP AP: %s,"
                    "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                    g_ucConnectionSSID,g_ucConnectionBSSID[0],
                    g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                    g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                    g_ucConnectionBSSID[5]);
        }
        memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
        memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    }
    break;

    default:
    {
        UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                   pWlanEvent->Event);
    }
    break;
    }
}


/*8b    888 8888888888 88888888888     d8888 8888888b.  8888888b.
8888b   888 888            888        d88888 888   Y88b 888   Y88b
88888b  888 888            888       d88P888 888    888 888    888
888Y88b 888 8888888        888      d88P 888 888   d88P 888   d88P
888 Y88b888 888            888     d88P  888 8888888P"  8888888P"
888  Y88888 888            888    d88P   888 888        888
888   Y8888 888            888   d8888888888 888        888
888    Y888 8888888888     888  d88P     888 888        8*/

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(!pNetAppEvent)
    {
        return;
    }

    switch(pNetAppEvent->Event)
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
    {
        SlIpV4AcquiredAsync_t *pEventData = NULL;

        SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

        //Ip Acquired Event Data
        pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
        g_ulIpAddr = pEventData->ip;

        //Gateway IP address
        g_ulGatewayIP = pEventData->gateway;

        UART_PRINT( "[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
                "Gateway=%d.%d.%d.%d\n\r",
                SL_IPV4_BYTE(g_ulIpAddr,3),
                SL_IPV4_BYTE(g_ulIpAddr,2),
                SL_IPV4_BYTE(g_ulIpAddr,1),
                SL_IPV4_BYTE(g_ulIpAddr,0),
                SL_IPV4_BYTE(g_ulGatewayIP,3),
                SL_IPV4_BYTE(g_ulGatewayIP,2),
                SL_IPV4_BYTE(g_ulGatewayIP,1),
                SL_IPV4_BYTE(g_ulGatewayIP,0));
    }
    break;

    default:
    {
        UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                   pNetAppEvent->Event);
    }
    break;
    }
}


//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    // Unused in this application
}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(!pDevEvent)
    {
        return;
    }

    //
    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    //
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->EventData.deviceEvent.status,
               pDevEvent->EventData.deviceEvent.sender);
}


/*8888b.   .d88888b.   .d8888b.  888    d8P
d88P  Y88b d88P" "Y88b d88P  Y88b 888   d8P
Y88b.      888     888 888    888 888  d8P
 "Y888b.   888     888 888        888d88K
    "Y88b. 888     888 888        8888888b
      "888 888     888 888    888 888  Y88b
Y88b  d88P Y88b. .d88P Y88b  d88P 888   Y88b
 "Y8888P"   "Y88888P"   "Y8888P"  888    Y8*/

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    if(!pSock)
    {
        return;
    }

    switch( pSock->Event )
    {
    case SL_SOCKET_TX_FAILED_EVENT:
        switch( pSock->socketAsyncEvent.SockTxFailData.status)
        {
        case SL_ECLOSE:
            UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
                    "failed to transmit all queued packets\n\n",
                    pSock->socketAsyncEvent.SockTxFailData.sd);
            break;
        default:
            UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                    "(%d) \n\n",
                    pSock->socketAsyncEvent.SockTxFailData.sd,
                    pSock->socketAsyncEvent.SockTxFailData.status);
            break;
        }
        break;

        default:
            UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
            break;
    }

}

//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- End
//*****************************************************************************

/*88888 888b    888 8888888 88888888888     888     888      d8888 8888888b.
  888   8888b   888   888       888         888     888     d88888 888   Y88b
  888   88888b  888   888       888         888     888    d88P888 888    888
  888   888Y88b 888   888       888         Y88b   d88P   d88P 888 888   d88P
  888   888 Y88b888   888       888          Y88b d88P   d88P  888 8888888P"
  888   888  Y88888   888       888           Y88o88P   d88P   888 888 T88b
  888   888   Y8888   888       888            Y888P   d8888888888 888  T88b
8888888 888    Y888 8888888     888             Y8P   d88P     888 888   T8*/

//*****************************************************************************
//
//! This function initializes the application variables
//!
//! \param[in]    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeNetAppVariables()
{
    g_ulStatus = 0;
    g_ulGatewayIP = 0;
    memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    g_ulDestinationIp = IP_ADDR;
    g_uiPortNum = PORT_NUM;
    g_ulPacketCount = TCP_PACKET_COUNT;
}


/*8888b.   .d88888b.  888b    888 8888888888      .d8888b.  888
d88P  Y88b d88P" "Y88b 8888b   888 888            d88P  Y88b 888
888    888 888     888 88888b  888 888            Y88b.      888
888        888     888 888Y88b 888 8888888         "Y888b.   888
888        888     888 888 Y88b888 888                "Y88b. 888
888    888 888     888 888  Y88888 888                  "888 888
Y88b  d88P Y88b. .d88P 888   Y8888 888            Y88b  d88P 888
 "Y8888P"   "Y88888P"  888    Y888 888             "Y8888P"  888888*/

//*****************************************************************************
//! \brief This function puts the device in its default state. It:
//!           - Set the mode to STATION
//!           - Configures connection policy to Auto and AutoSmartConfig
//!           - Deletes all the stored profiles
//!           - Enables DHCP
//!           - Disables Scan policy
//!           - Sets Tx power to maximum
//!           - Sets power policy to normal
//!           - Unregister mDNS services
//!           - Remove all filters
//!
//! \param   none
//! \return  On success, zero is returned. On error, negative is returned
//
// remark:
// Following function configure the device to default state by cleaning
// the persistent settings stored in NVMEM (viz. connection profiles &
// policies, power policy etc)
//
// Applications may choose to skip this step if the developer is sure
// that the device is in its default state at start of applicaton
//
// Note that all profiles and persistent settings that were done on the
// device will be lost
//
//*****************************************************************************
static long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower = 0;

    long lRetVal = -1;
    long lMode = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode 
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event 
            // before doing anything 
            while(!IS_IP_ACQUIRED(g_ulStatus))
            {
#ifndef SL_PLATFORM_MULTI_THREADED
                _SlNonOsMainLoopTask();
#endif
            }
        }

        // Switch to STA role and restart 
        lRetVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Stop(0xFF);
        ASSERT_ON_ERROR(lRetVal);

        lRetVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(lRetVal);

        // Check if the device is in station again 
        if (ROLE_STA != lRetVal)
        {
            // We don't want to proceed if the device is not coming up in STA-mode 
            return DEVICE_NOT_IN_STATION_MODE;
        }
    }

    // Get the device's version-information
    ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    ucConfigLen = sizeof(ver);
    lRetVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt, 
                        &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
               ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
               ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
               ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
               ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
               ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig 
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, 
                               SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);



    //
    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore 
    // other return-codes
    //
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(g_ulStatus))
        {
#ifndef SL_PLATFORM_MULTI_THREADED
            _SlNonOsMainLoopTask();
#endif
        }
    }

    // Enable DHCP client
    lRetVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE,1,1,&ucVal);
    ASSERT_ON_ERROR(lRetVal);

    // Disable scan
    ucConfigOpt = SL_SCAN_POLICY(0);
    lRetVal = sl_WlanPolicySet(SL_POLICY_SCAN , ucConfigOpt, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Set Tx power level for station mode
    // Number between 0-15, as dB offset from max power - 0 will set max power
    ucPower = 0;
    lRetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, 
                         WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (unsigned char *)&ucPower);
    ASSERT_ON_ERROR(lRetVal);

    // Set PM policy to normal
    lRetVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Unregister mDNS services
    lRetVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove  all 64 filters (8*8)
    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    lRetVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                                 sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(lRetVal);

    lRetVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(lRetVal);

    InitializeNetAppVariables();

    return lRetVal; // Success
}


/*88888 8888888b.      8888888b.      d8888 8888888b.   .d8888b.  8888888888 8888888b.
  888   888   Y88b     888   Y88b    d88888 888   Y88b d88P  Y88b 888        888   Y88b
  888   888    888     888    888   d88P888 888    888 Y88b.      888        888    888
  888   888   d88P     888   d88P  d88P 888 888   d88P  "Y888b.   8888888    888   d88P
  888   8888888P"      8888888P"  d88P  888 8888888P"      "Y88b. 888        8888888P"
  888   888            888       d88P   888 888 T88b         "888 888        888 T88b
  888   888            888      d8888888888 888  T88b  Y88b  d88P 888        888  T88b
8888888 888            888     d88P     888 888   T88b  "Y8888P"  8888888888 888   T8*/

//****************************************************************************
//
//!    \brief Parse the input IP address from the user
//!
//!    \param[in]                     ucCMD (char pointer to input string)
//!
//!    \return                        0 : if correct IP, -1 : incorrect IP
//
//****************************************************************************
int IpAddressParser(char *ucCMD)
{
    volatile int i=0;
    unsigned int uiUserInputData;
    unsigned long ulUserIpAddress = 0;
    char *ucInpString;
    ucInpString = strtok(ucCMD, ".");
    uiUserInputData = (int)strtoul(ucInpString,0,10);
    while(i<4)
    {
        //
        // Check Whether IP is valid
        //
        if((ucInpString != NULL) && (uiUserInputData < 256))
        {
            ulUserIpAddress |= uiUserInputData;
            if(i < 3)
                ulUserIpAddress = ulUserIpAddress << 8;
            ucInpString=strtok(NULL,".");
            uiUserInputData = (int)strtoul(ucInpString,0,10);
            i++;
        }
        else
        {
            return -1;
        }
    }
    g_ulDestinationIp = ulUserIpAddress;
    return SUCCESS;
}


/*8       888 888             d8888 888b    888            d8888 8888888b.
888   o   888 888            d88888 8888b   888           d88888 888   Y88b
888  d8b  888 888           d88P888 88888b  888          d88P888 888    888
888 d888b 888 888          d88P 888 888Y88b 888         d88P 888 888   d88P
888d88888b888 888         d88P  888 888 Y88b888        d88P  888 8888888P"
88888P Y88888 888        d88P   888 888  Y88888       d88P   888 888
8888P   Y8888 888       d8888888888 888   Y8888      d8888888888 888
888P     Y888 88888888 d88P     888 888    Y888     d88P     888 8*/

//****************************************************************************
//
//!  \brief Connecting to a WLAN Accesspoint
//!
//!   This function connects to the required AP (SSID_NAME) with Security
//!   parameters specified in the form of macros at the top of this file
//!
//!   \param[in]              None
//!
//!   \return     Status value
//!
//!   \warning    If the WLAN connection fails or we don't acquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
static long WlanConnect()
{
    SlSecParams_t secParams = {0};
    long lRetVal = 0;

    secParams.Key = (signed char*)SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type = SECURITY_TYPE;

    lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    /* Wait */
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        // Wait for WLAN Event
#ifndef SL_PLATFORM_MULTI_THREADED
        _SlNonOsMainLoopTask();
#endif
    }

    return SUCCESS;
}


/*8888b.  8888888b. 8888888     8888888 888b    888 8888888 88888888888
d88P  Y88b 888   Y88b  888         888   8888b   888   888       888
Y88b.      888    888  888         888   88888b  888   888       888
 "Y888b.   888   d88P  888         888   888Y88b 888   888       888
    "Y88b. 8888888P"   888         888   888 Y88b888   888       888
      "888 888         888         888   888  Y88888   888       888
Y88b  d88P 888         888         888   888   Y8888   888       888
 "Y8888P"  888       8888888     8888888 888    Y888 8888888     8*/

static void SpiDmaInitTransfer(void)
{
    // Set up the SPI FIFO DMA
    Spiconf(SpiInterruptHandler);

    SpiDmaTransfer(PRIMARY_BUFFER, ulRecvData_a, DMA_SIZE_WORD);

    SpiDmaTransfer(ALTERNATIVE_BUFFER, ulRecvData_b, DMA_SIZE_WORD);

    SPIEnable(GSPI_BASE);
}



/*888888888 8888888b.         d8888 888b    888  .d8888b.  8888888888
    888     888   Y88b       d88888 8888b   888 d88P  Y88b 888
    888     888    888      d88P888 88888b  888 Y88b.      888
    888     888   d88P     d88P 888 888Y88b 888  "Y888b.   8888888
    888     8888888P"     d88P  888 888 Y88b888     "Y88b. 888
    888     888 T88b     d88P   888 888  Y88888       "888 888
    888     888  T88b   d8888888888 888   Y8888 Y88b  d88P 888
    888     888   T88b d88P     888 888    Y888  "Y8888P"  8*/

static void SpiDmaTransfer(int primary, unsigned long *rx_buffer, int size_transfert)
{

    if (primary)
    {
        UDMASetupTransfer(  UDMA_CH30_GSPI_RX | UDMA_PRI_SELECT,   // ulChannel | primary
                            UDMA_MODE_PINGPONG,                    // ulMode
                            size_transfert,                        // ulItemCount
                            UDMA_SIZE_32,                     // ulItemSize
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

void ReinitializeDMA(void)
{
    UDMADeInit();
    SpiDmaInitTransfer();

    SPIEnable(GSPI_BASE);
}

void DeinitDmaSpi(void)
{
    SPIDisable(GSPI_BASE);
    SPIDmaDisable(GSPI_BASE, SPI_RX_DMA);

    SPIIntDisable(GSPI_BASE, SPI_INT_DMARX);
    SPIIntUnregister(GSPI_BASE);

    SPIReset(GSPI_BASE);
}

void InitBuffers(void)
{
    int iCounter;

    // init the Ring buffer
    R_buffer = RingBuffer_create(BUFFER_SIZE, DMA_SIZE_WORD * sizeof(uint32_t));

    // filling the buffers Tx and Rx
    for (iCounter=0 ; iCounter<sizeof(pre_Tx_buff) ; iCounter++)
    {
        pre_Tx_buff[iCounter] =  '\0';
    }

    for (iCounter=0 ; iCounter<sizeof(g_cBsdBuf_Rx) ; iCounter++)
    {
        g_cBsdBuf_Rx[iCounter] = '\0';
    }
}





//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

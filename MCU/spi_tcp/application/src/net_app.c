#include "net_app.h"

//*********************************************************************
//
//                          NETWORK COMMON APP
//
//  This file contain the function used by the Simple link applications
//  that are working with wifi, and TCP/IP protocols
//
//*********************************************************************

//*****************************************************************************
//                  STATIC VARIABLES
//*****************************************************************************
static unsigned char ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
static unsigned char ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
static unsigned long ulStatus            = 0;          //SimpleLink Status
static unsigned long ulGatewayIP         = 0;          //Network Gateway IP address
static unsigned long ulIpAddr            = 0;
static unsigned long ulDestinationIp     = IP_ADDR;
static unsigned int uiPortNum            = PORT_NUM;
static int iSockID;


// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SOCKET_CREATE_ERROR        = -0x7D0,
    BIND_ERROR                 = SOCKET_CREATE_ERROR - 1,
    LISTEN_ERROR               = BIND_ERROR -1,
    SOCKET_OPT_ERROR           = LISTEN_ERROR -1,
    CONNECT_ERROR              = SOCKET_OPT_ERROR -1,
    ACCEPT_ERROR               = CONNECT_ERROR - 1,
    SEND_ERROR                 = ACCEPT_ERROR -1,
    RECV_ERROR                 = SEND_ERROR -1,
    SOCKET_CLOSE_ERROR         = RECV_ERROR -1,
    DEVICE_NOT_IN_STATION_MODE = SOCKET_CLOSE_ERROR - 1,
    STATUS_CODE_MAX            = -0xBB8
} e_AppStatusCodes;


/*8b    888 8888888888 88888888888     888     888      d8888 8888888b.
8888b   888 888            888         888     888     d88888 888   Y88b
88888b  888 888            888         888     888    d88P888 888    888
888Y88b 888 8888888        888         Y88b   d88P   d88P 888 888   d88P
888 Y88b888 888            888          Y88b d88P   d88P  888 8888888P"
888  Y88888 888            888           Y88o88P   d88P   888 888 T88b
888   Y8888 888            888            Y888P   d8888888888 888  T88b
888    Y888 8888888888     888             Y8P   d88P     888 888   T8*/

//*****************************************************************************
//
//!     \brief  This function initializes the Net application variables
//!
//!     \param[in] None
//!
//!     \return None
//!
//*****************************************************************************
void InitializeNetAppVariables(void)
{
    ulStatus        = 0;
    ulGatewayIP     = 0;
    memset(ucConnectionSSID,  0, sizeof(ucConnectionSSID));
    memset(ucConnectionBSSID, 0, sizeof(ucConnectionBSSID));
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
//!     \brief This Function Handles WLAN Events
//!
//!     \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//!     \return None
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
        SET_STATUS_BIT(ulStatus, STATUS_BIT_CONNECTION);

        // Information about the connected AP (like name, MAC etc) will be
        // available in 'slWlanConnectAsyncResponse_t'-Applications
        // can use it if required
        //
        //  slWlanConnectAsyncResponse_t *pEventData = NULL;
        // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;

        // Copy new connection SSID and BSSID to global parameters
        memcpy(ucConnectionSSID,pWlanEvent->EventData.
               STAandP2PModeWlanConnected.ssid_name,
               pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);

        memcpy(ucConnectionBSSID,
               pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
               SL_BSSID_LENGTH);

        UART_PRINT( "Connected \n\r");

        UART_PRINT( "AP SSID:            %s \n\r"
                    "BSSID:              %x:%x:%x:%x:%x:%x\n\r",
                    ucConnectionSSID,ucConnectionBSSID[0],
                    ucConnectionBSSID[1],ucConnectionBSSID[2],
                    ucConnectionBSSID[3],ucConnectionBSSID[4],
                    ucConnectionBSSID[5]);
    }
    break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
        slWlanConnectAsyncResponse_t*  pEventData = NULL;

        CLR_STATUS_BIT(ulStatus, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(ulStatus, STATUS_BIT_IP_AQUIRED);

        pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

        // If the user has initiated 'Disconnect' request,
        //'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION
        if(SL_WLAN_DISCONNECT_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
        {
            UART_PRINT( "[WLAN EVENT]Device disconnected from the AP: %s,"
                        "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                        ucConnectionSSID,ucConnectionBSSID[0],
                        ucConnectionBSSID[1],ucConnectionBSSID[2],
                        ucConnectionBSSID[3],ucConnectionBSSID[4],
                        ucConnectionBSSID[5]);
        }
        else
        {
            UART_PRINT( "[WLAN ERROR]Device disconnected from the AP AP: %s,"
                        "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                        ucConnectionSSID,ucConnectionBSSID[0],
                        ucConnectionBSSID[1],ucConnectionBSSID[2],
                        ucConnectionBSSID[3],ucConnectionBSSID[4],
                        ucConnectionBSSID[5]);
        }
        memset(ucConnectionSSID,0,sizeof(ucConnectionSSID));
        memset(ucConnectionBSSID,0,sizeof(ucConnectionBSSID));
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
//!     \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//!     \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//!     \return None
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

        SET_STATUS_BIT(ulStatus, STATUS_BIT_IP_AQUIRED);

        //Ip Acquired Event Data
        pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
        ulIpAddr = pEventData->ip;

        //Gateway IP address
        ulGatewayIP = pEventData->gateway;

        UART_PRINT( "Emitting device IP: %d.%d.%d.%d \n\r"
                    "Gateway:            %d.%d.%d.%d\n\r",
                    SL_IPV4_BYTE(ulIpAddr,3),
                    SL_IPV4_BYTE(ulIpAddr,2),
                    SL_IPV4_BYTE(ulIpAddr,1),
                    SL_IPV4_BYTE(ulIpAddr,0),
                    SL_IPV4_BYTE(ulGatewayIP,3),
                    SL_IPV4_BYTE(ulGatewayIP,2),
                    SL_IPV4_BYTE(ulGatewayIP,1),
                    SL_IPV4_BYTE(ulGatewayIP,0));
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


/*8    888 88888888888 88888888888 8888888b.
888    888     888         888     888   Y88b
888    888     888         888     888    888
8888888888     888         888     888   d88P
888    888     888         888     8888888P"
888    888     888         888     888
888    888     888         888     888
888    888     888         888     8*/

//*****************************************************************************
//
//!     \brief This function handles HTTP server events
//!
//!     \param[in]  pServerEvent - Contains the relevant event information
//!     \param[in]  pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//!     \return None
//!
//****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{
    // Unused in this application
}

/* 8888b.  8888888888 888     888 8888888888 888b    888 88888888888
d88P  Y88b 888        888     888 888        8888b   888     888
888    888 888        888     888 888        88888b  888     888
888        8888888    Y88b   d88P 8888888    888Y88b 888     888
888  88888 888         Y88b d88P  888        888 Y88b888     888
888    888 888          Y88o88P   888        888  Y88888     888
Y88b  d88P 888           Y888P    888        888   Y8888     888
 "Y8888P88 8888888888     Y8P     8888888888 888    Y888     8*/

//*****************************************************************************
//
//!     \brief This function handles General Events
//!
//!     \param[in]     pDevEvent - Pointer to General Event Info
//!
//!     \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(!pDevEvent)
    {
        return;
    }

    // Most of the general errors are not FATAL are are to be handled
    // appropriately by the application
    UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\r\n",
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
//!     \param[in]      pSock - Pointer to Socket Event Info
//!
//!     \return None
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
            UART_PRINT( "[SOCK ERROR] - close socket (%d) operation "
                        "failed to transmit all queued packets\r\n",
                        pSock->socketAsyncEvent.SockTxFailData.sd);
            break;
        default:
            UART_PRINT( "[SOCK ERROR] - TX FAILED  :  socket %d , reason "
                        "(%d) \r\n",
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



/*8888b.   .d88888b.  888b    888 8888888888      .d8888b.  888
d88P  Y88b d88P" "Y88b 8888b   888 888            d88P  Y88b 888
888    888 888     888 88888b  888 888            Y88b.      888
888        888     888 888Y88b 888 8888888         "Y888b.   888
888        888     888 888 Y88b888 888                "Y88b. 888
888    888 888     888 888  Y88888 888                  "888 888
Y88b  d88P Y88b. .d88P 888   Y8888 888            Y88b  d88P 888
 "Y8888P"   "Y88888P"  888    Y888 888             "Y8888P"  888888*/

//*****************************************************************************
//!     \brief This function puts the device in its default state. It:
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
//!     \param   none
//!     \return  On success, zero is returned. On error, negative is returned
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
long ConfigureSimpleLinkToDefaultState()
{
    SlVersionFull   ver = {0};
    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {0};

    unsigned char ucVal       = 1;
    unsigned char ucConfigOpt = 0;
    unsigned char ucConfigLen = 0;
    unsigned char ucPower     = 0;

    long lRetVal = -1;
    long lMode   = -1;

    lMode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(lMode);

    // If the device is not in station-mode, try configuring it in station-mode
    if (ROLE_STA != lMode)
    {
        if (ROLE_AP == lMode)
        {
            // If the device is in AP mode, we need to wait for this event
            // before doing anything
            while(!IS_IP_ACQUIRED(ulStatus))
            {
                _SlNonOsMainLoopTask();
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
    lRetVal     = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &ucConfigOpt,
                            &ucConfigLen, (unsigned char *)(&ver));
    ASSERT_ON_ERROR(lRetVal);

    // TI Drivers info
    // UART_PRINT("Host Driver Version: %s\n\r",SL_DRIVER_VERSION);
    // UART_PRINT("Build Version %d.%d.%d.%d.31.%d.%d.%d.%d.%d.%d.%d.%d\n\r",
    //            ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3],
    //            ver.ChipFwAndPhyVersion.FwVersion[0],ver.ChipFwAndPhyVersion.FwVersion[1],
    //            ver.ChipFwAndPhyVersion.FwVersion[2],ver.ChipFwAndPhyVersion.FwVersion[3],
    //            ver.ChipFwAndPhyVersion.PhyVersion[0],ver.ChipFwAndPhyVersion.PhyVersion[1],
    //            ver.ChipFwAndPhyVersion.PhyVersion[2],ver.ChipFwAndPhyVersion.PhyVersion[3]);

    // Set connection policy to Auto + SmartConfig
    //      (Device's default connection policy)
    lRetVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                               SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Remove all profiles
    lRetVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(lRetVal);

    // Device in station-mode. Disconnect previous connection if any
    // The function returns 0 if 'Disconnected done', negative number if already
    // disconnected Wait for 'disconnection' event if 0 is returned, Ignore
    // other return-codes
    lRetVal = sl_WlanDisconnect();
    if(0 == lRetVal)
    {
        // Wait
        while(IS_CONNECTED(ulStatus))
        {
            _SlNonOsMainLoopTask(); // We don't use RTOS in this app
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
//!     \brief Parse the input IP address from the user
//!
//!     \param[in] ucCMD (char pointer to input string)
//!
//!     \return 0 : if correct IP, -1 : incorrect IP
//
//****************************************************************************
int IpAddressParser(char *ucCMD)
{
    volatile int i = 0;
    unsigned int uiUserInputData;
    unsigned long ulUserIpAddress = 0;
    char *ucInpString;
    ucInpString     = strtok(ucCMD, ".");
    uiUserInputData = (int)strtoul(ucInpString,0,10);
    while(i<4)
    {
        // Check Whether IP is valid
        if((ucInpString != NULL) && (uiUserInputData < 256))
        {
            ulUserIpAddress |= uiUserInputData;
            if(i < 3)
                ulUserIpAddress = ulUserIpAddress << 8;
            ucInpString     = strtok(NULL,".");
            uiUserInputData = (int)strtoul(ucInpString,0,10);
            i++;
        }
        else
        {
            return -1;
        }
    }
    ulDestinationIp = ulUserIpAddress;
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
//!     \brief Connecting to a WLAN Accesspoint
//!
//!   This function connects to the required AP (SSID_NAME) with Security
//!   parameters specified in the form of macros at the top of this file
//!
//!     \param[in] None
//!
//!     \return Status value
//!
//!     \warning If the WLAN connection fails or we don't acquire an IP
//!            address, It will be stuck in this function forever.
//
//****************************************************************************
long WlanConnect()
{
    SlSecParams_t secParams = {0};
    long lRetVal            = 0;

    secParams.Key    = (signed char*)SECURITY_KEY;
    secParams.KeyLen = strlen(SECURITY_KEY);
    secParams.Type   = SECURITY_TYPE;

    lRetVal = sl_WlanConnect((signed char*)SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(lRetVal);

    // Wait
    while((!IS_CONNECTED(ulStatus)) || (!IS_IP_ACQUIRED(ulStatus)))
    {
        // Wait for WLAN Event
        _SlNonOsMainLoopTask(); // We don't use RTOS in this app
    }

    return SUCCESS;
}



/*888888888  .d8888b.  8888888b.       .d8888b.   .d88888b.  888b    888 888b    888
    888     d88P  Y88b 888   Y88b     d88P  Y88b d88P" "Y88b 8888b   888 8888b   888
    888     888    888 888    888     888    888 888     888 88888b  888 88888b  888
    888     888        888   d88P     888        888     888 888Y88b 888 888Y88b 888
    888     888        8888888P"      888        888     888 888 Y88b888 888 Y88b888
    888     888    888 888            888    888 888     888 888  Y88888 888  Y88888
    888     Y88b  d88P 888            Y88b  d88P Y88b. .d88P 888   Y8888 888   Y8888
    888      "Y8888P"  888             "Y8888P"   "Y88888P"  888    Y888 888    Y8*/

//****************************************************************************
//
//!     \brief TCP Connection Setup
//!     This function is used to set up the TCP connection with parametred
//!     IP and PORT number, then send a test message
//!
//!     \param[in] None
//!
//!     \return Success or Failure
//
//****************************************************************************
int TcpClientConnect(void)
{
    int iAddrSize;
    int iStatus;
    SlSockAddrIn_t  sAddr;

    // filling the TCP server socket address
    sAddr.sin_family      = SL_AF_INET;
    sAddr.sin_port        = htons((unsigned short)uiPortNum);
    sAddr.sin_addr.s_addr = htonl((unsigned int)ulDestinationIp);

    iAddrSize = sizeof(SlSockAddrIn_t);

    UART_PRINT("Connecting to Server..\n\r");

    // UART_PRINT("Socket creation\n\r");

    // creating a TCP socket
    iSockID = socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    DBG_check(iSockID >= 0, "SOCKET_CREATE_ERROR");

    // UART_PRINT("Socket connection\n\r");
    iStatus = connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
    DBG_check(iStatus >= 0, "CONNECT ERROR");

    UART_PRINT("Connected \n\r");

    UART_PRINT( "Destination IP:     %d.%d.%d.%d \n\r"
                "Destination PORT =  %d \n\r",
                SL_IPV4_BYTE(ulDestinationIp,3),
                SL_IPV4_BYTE(ulDestinationIp,2),
                SL_IPV4_BYTE(ulDestinationIp,1),
                SL_IPV4_BYTE(ulDestinationIp,0),
                uiPortNum  );


    return SUCCESS;

    error:
        return FAILURE;
}


/* 8888b.  8888888888 888b    888 8888888b.
d88P  Y88b 888        8888b   888 888  "Y88b
Y88b.      888        88888b  888 888    888
 "Y888b.   8888888    888Y88b 888 888    888
    "Y88b. 888        888 Y88b888 888    888
      "888 888        888  Y88888 888    888
Y88b  d88P 888        888   Y8888 888  .d88P
 "Y8888P"  8888888888 888    Y888 8888888*/

//****************************************************************************
//
//!     \brief TCP Socket send wrapper
//!
//!     \param[in] pointer to the buffer containing the data to be sent
//!     \param[in] amount of data to be sent
//!
//!     \return Socket error (or success) code
//
//****************************************************************************
int net_send_data(char* buffer, int amount)
{
    int iStatus;

    iStatus = sl_Send (
        iSockID,
        buffer,
        amount,
        0
    );

    return iStatus;
}


/* 8888b.  888       .d88888b.   .d8888b.  8888888888
d88P  Y88b 888      d88P" "Y88b d88P  Y88b 888
888    888 888      888     888 Y88b.      888
888        888      888     888  "Y888b.   8888888
888        888      888     888     "Y88b. 888
888    888 888      888     888       "888 888
Y88b  d88P 888      Y88b. .d88P Y88b  d88P 888
 "Y8888P"  88888888  "Y88888P"   "Y8888P"  88888888*/

//****************************************************************************
//
//!     \brief socket connection closing wrapper
//!
//!     \param[in] None
//!
//!     \return Socket error (or success) code
//
//****************************************************************************
int net_close_connection(void)
{
    int iStatus;

    iStatus = sl_Close(iSockID);

    return iStatus;
}

















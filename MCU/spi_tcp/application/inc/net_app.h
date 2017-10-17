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

// simplelink includes
#include "simplelink.h"
#include <stdlib.h>
#include "wlan.h"
#include "common.h"
#ifndef  NOTERM
#include "uart_if.h"
#include "dbg.h"
#endif

//
// Values for below macros shall be modified as per access-point(AP) properties
// SimpleLink device will connect to following AP when application is executed
//
#define SSID_NAME           "APcc3200"          /* AP SSID */
#define SECURITY_KEY        "sedefairedesonego" /* Password of the secured AP */
#define SECURITY_TYPE       SL_SEC_TYPE_WPA_WPA2/* Security type (OPEN or WEP or WPA*/
#define SSID_LEN_MAX        32
#define BSSID_LEN_MAX       6
#define PORT_NUM            5001  // Destination Port
#define TCP_PACKET_COUNT    20
#define IP_ADDR             0xc0a80202 // 192.168.2.2 Destination IP for APCC3200 Access point

void InitializeNetAppVariables(void);
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent);
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent);
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse);
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent);
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock);
long ConfigureSimpleLinkToDefaultState(void);
long WlanConnect(void);
int IpAddressParser(char *ucCMD);
int TcpClientConnect(void);
int net_send_data(char* buffer, int amount);
int net_close_connection(void);

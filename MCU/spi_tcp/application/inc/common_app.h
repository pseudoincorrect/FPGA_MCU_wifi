#ifndef __COMMON_APP_H__
#define __COMMON_APP_H__


// Standard includes
#include <string.h>
#include <stdint.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_mcspi.h"
#include "spi.h"
#include "hw_ints.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"
#include "wdt.h"

// Common interface includes
#include "udma_if.h"
#include "udma.h"
#include "gpio.h"
#include "pin.h"
#ifndef  NOTERM
#include "uart_if.h"
#include "dbg.h"
#endif

// User include
#include "ring_buffer.h"

/*88888b.  8888888888 8888888888 8888888 888b    888 8888888888
888  "Y88b 888        888          888   8888b   888 888
888    888 888        888          888   88888b  888 888
888    888 8888888    8888888      888   888Y88b 888 8888888
888    888 888        888          888   888 Y88b888 888
888    888 888        888          888   888  Y88888 888
888  .d88P 888        888          888   888   Y8888 888
8888888P"  8888888888 888        8888888 888    Y888 888888888*/

#define SPI_IF_BIT_RATE     12500000 // 12.5 MHz
#define TR_BUFF_SIZE        100

#define BUFFER_SIZE         30
#define DMA_SIZE_WORD       256
#define SPI_PACKET_SIZE     DMA_SIZE_WORD
#define MAX_TCP_SIZE_WORD   DMA_SIZE_WORD
#define BUFFER_UNIT_SIZE	(DMA_SIZE_WORD * sizeof(uint32_t))
#define MAX_TCP_SIZE_BYTE   (DMA_SIZE_WORD * sizeof(uint32_t))

#define SPI_BLOCK_COUNTER   5

#define PRIMARY_BUFFER      1
#define ALTERNATIVE_BUFFER  0

#define WD_PERIOD_MS        5000

#define LED1 1
#define LED2 2
#define LED3 3

#define ON   1
#define OFF  0


/*88888b.  8888888888  .d8888b.  888
888  "Y88b 888        d88P  Y88b 888
888    888 888        888    888 888
888    888 8888888    888        888
888    888 888        888        888
888    888 888        888    888 888
888  .d88P 888        Y88b  d88P 888      d8b
8888888P"  8888888888  "Y8888P"  88888888 Y*/

void init(void);

void Spiconf(void spiIntHandler (void));

void PinMuxConfig(void);

void BufferDisplay(char type, void *buffer, int size);

void BufferStatusDisplay(RingBuffer *buffer);

void DebugPulse(int pin, int amount);

void DebugPinSet(int pin, int status);

void LedSet(int led_number, int status);

void SpiBlock(void);

void SpiUnblock(void);

int SpiNssRead(void);

void watchDogAck (void);

void GPIOConfigureNIntEnable(unsigned int uiGPIOPort,
                                  unsigned char ucGPIOPin,
                                  unsigned int uiIntType,
                                  void (*pfnIntHandler)(void));

static unsigned char GetPeripheralIntNum(unsigned int uiGPIOPort);

void SpiDmaTransfer(int primary, unsigned long *rx_buffer, int size_transfert);

void ReinitializeDMA(void);

void DeinitDmaSpi(void);

#if (FAKE_DATA)
static void Timer_init(void)
#endif


/*88888888 Y88b   d88P 88888888888 8888888888 8888888b.  888b    888
888         Y88b d88P      888     888        888   Y88b 8888b   888
888          Y88o88P       888     888        888    888 88888b  888
8888888       Y888P        888     8888888    888   d88P 888Y88b 888
888           d888b        888     888        8888888P"  888 Y88b888
888          d88888b       888     888        888 T88b   888  Y88888
888         d88P Y88b      888     888        888  T88b  888   Y8888
8888888888 d88P   Y88b     888     8888888888 888   T88b 888    Y8*/

extern void (* const g_pfnVectors[])(void);

extern void GpioIntHandler(void);



#endif

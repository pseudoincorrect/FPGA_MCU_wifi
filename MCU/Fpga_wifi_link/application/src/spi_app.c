#include "spi_app.h"


/*888888 888b    888 8888888 88888888888
   888   8888b   888   888       888
   888   88888b  888   888       888
   888   888Y88b 888   888       888
   888   888 Y88b888   888       888
   888   888  Y88888   888       888
   888   888   Y8888   888       888
 8888888 888    Y888 8888888     8*/

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
void SpiDmaInitTransfer(void spiIntHandler (void),
                        unsigned long *ulRecvData_a,
                        unsigned long *ulRecvData_b)
{
    // Set up the SPI FIFO DMA
    Spiconf(spiIntHandler);

    SpiDmaTransfer(PRIMARY_BUFFER, ulRecvData_a, DMA_SIZE_WORD);

    SpiDmaTransfer(ALTERNATIVE_BUFFER, ulRecvData_b, DMA_SIZE_WORD);

    SPIEnable(GSPI_BASE);
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
    // Reset the peripheral
    PRCMPeripheralReset(PRCM_GSPI);

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
void ReinitializeSpiDma(void spiIntHandler (void),
                        unsigned long *ulRecvData_a,
                        unsigned long *ulRecvData_b)
{
    UDMADeInit();
    SpiDmaInitTransfer(spiIntHandler,
                       ulRecvData_a,
                       ulRecvData_b);

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



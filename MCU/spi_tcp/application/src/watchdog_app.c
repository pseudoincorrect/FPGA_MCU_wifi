#include "watchdog_app.h"


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
void watchdog_init(void WatchdogIntHandler (void))
{
    WDT_IF_Init(WatchdogIntHandler, MILLISECONDS_TO_TICKS(WD_PERIOD_MS));
}



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

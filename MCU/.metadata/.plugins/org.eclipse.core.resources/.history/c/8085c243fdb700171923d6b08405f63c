#include "timer_app.h"


#if (FAKE_DATA)

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
//!     \brief This function Initialize the timer that will produce a
//!     periodic interrupt used to create fake data for testing purposes
//!
//!     \param[in] No   lne
//!
//!     \return None
//
//****************************************************************************
void Timer_init(void TimerBaseIntHandler (void))
{
    unsigned long BaseTimer;

    // Base address for first timer
    BaseTimer = TIMERA0_BASE;

    // Configuring the timers
    Timer_IF_Init(PRCM_TIMERA0, BaseTimer, TIMER_CFG_PERIODIC, TIMER_A, 0);

    // Setup the interrupts for the timer timeouts.
    Timer_IF_IntSetup(BaseTimer, TIMER_A, TimerBaseIntHandler);

    // Turn on the timers feeding values in mSec
    Timer_IF_Start(BaseTimer, TIMER_A, 15);
}
#endif

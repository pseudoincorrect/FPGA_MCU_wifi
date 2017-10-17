#include "gpio_app.h"

/*8888b.   .d88888b.  888b    888 8888888888     8888888 888b    888 88888888888
d88P  Y88b d88P" "Y88b 8888b   888 888              888   8888b   888     888
888    888 888     888 88888b  888 888              888   88888b  888     888
888        888     888 888Y88b 888 8888888          888   888Y88b 888     888
888        888     888 888 Y88b888 888              888   888 Y88b888     888
888    888 888     888 888  Y88888 888              888   888  Y88888     888
Y88b  d88P Y88b. .d88P 888   Y8888 888              888   888   Y8888     888
 "Y8888P"   "Y88888P"  888    Y888 888            8888888 888    Y888     88
 */

//****************************************************************************
//
//!     \brief Configure the interrupt for the GPIO (raising edge on a
//!     specified Pin)
//!
//!     \param[in]  GPIO Port
//!     \param[in]  GPIO Pin
//!     \param[in]  Type of the interrupt (ex: raising elec edge)
//!     \param[in]  Pointer to a Interrupt handler
//!
//!     \return None
//
//****************************************************************************
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

//****************************************************************************
//
//!     \brief Function used to determine on which port the GPIO interrupt
//!     come from
//!
//!     \param[in] Base adress of the GPIOs Port
//!
//!     \return The GPIO port where the interrupt come frome
//
//****************************************************************************
static unsigned char GetPeripheralIntNum(unsigned int uiGPIOPort)
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



/*88888b.  888     888 888      .d8888b.  8888888888
888   Y88b 888     888 888     d88P  Y88b 888
888    888 888     888 888     Y88b.      888
888   d88P 888     888 888      "Y888b.   8888888
8888888P"  888     888 888         "Y88b. 888
888        888     888 888           "888 888
888        Y88b. .d88P 888     Y88b  d88P 888
888         "Y88888P"  88888888 "Y8888P"  88888888*/

//****************************************************************************
//
//!     \brief Send a certain amount of Pulse on a given Pin of the MCU
//!
//!     \param[in] Pin to send the pulse
//!     \param[in] Number of pulse to send
//!
//!     \return None
//
//****************************************************************************
void DebugPulse(int pin, int amount)
{
    int i;

    switch (pin)
    {
        case PIN_60 :
            for (i = 0; i<amount; i++)
            {
                // Small pulse for logic analyser
                GPIOPinWrite(GPIOA0_BASE, (1 << 5), (1 << 5));  // PIN_60 high
                GPIOPinWrite(GPIOA0_BASE, (1 << 5), 0);         // PIN_60 low
            }
            break;

        case PIN_50 :
            for (i = 0; i<amount; i++)
            {
                // Small pulse for logic analyser
                GPIOPinWrite(GPIOA0_BASE, (1 << 0), (1 << 0));  // PIN_50 high
                GPIOPinWrite(GPIOA0_BASE, (1 << 0), 0);     // PIN_50 low
            }
            break;

        default :
            DBG_log_warn("wrong pulse pin");
            break;
    }
}

#include "gpio_app.h"


/*8888b.  8888888888 88888888888
d88P  Y88b 888            888
Y88b.      888            888
 "Y888b.   8888888        888
    "Y88b. 888            888
      "888 888            888
Y88b  d88P 888            888
 "Y8888P"  8888888888     8*/

//****************************************************************************
//
//!     \brief This function is used to set a Debug pin High or Low
//!
//!     \param[in] Pin used
//!     \param[in] state of the pin to set (either High or Low)
//!
//!     \return None
//
//****************************************************************************
void DebugPinSet(int pin, int status)
{
    if (status)
    {
       switch (pin) // Set the pin HIGH
       {
            case PIN_60 : GPIOPinWrite(GPIOA0_BASE, (1 << 5), (1 << 5)); break;
            case PIN_50 : GPIOPinWrite(GPIOA0_BASE, (1 << 0), (1 << 0)); break;
            default : DBG_log_warn("wrong pulse pin"); break;
        }
     }
     else // Set it LOW
     {
         switch (pin)
         {
             case PIN_60 : GPIOPinWrite(GPIOA0_BASE, (1 << 5), 0); break;
             case PIN_50 : GPIOPinWrite(GPIOA0_BASE, (1 << 0), 0); break;
             default : DBG_log_warn("wrong pulse pin"); break;
        }
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

//****************************************************************************
//
//!     \brief This function is used to set a LED pin High or Low
//!
//!     \param[in] LED used
//!     \param[in] state of the LED to set (either High or Low)
//!
//!     \return None
//
//****************************************************************************
void LedSet(int led_number, int status)
{
    if (status) // Set the Led ON
    {
        switch(led_number)
        {
            case 1: GPIOPinWrite(GPIOA1_BASE, 0x02 , 0x02); break;
            case 2: GPIOPinWrite(GPIOA1_BASE, 0x04, 0x04);  break;
            case 3: GPIOPinWrite(GPIOA1_BASE, 0x08, 0x08);  break;
        }
    }
    else // Set it OFF
    {
        switch(led_number)
        {
            case 1: GPIOPinWrite(GPIOA1_BASE, 0x02 , 0); break;
            case 2: GPIOPinWrite(GPIOA1_BASE, 0x04, 0);  break;
            case 3: GPIOPinWrite(GPIOA1_BASE, 0x08, 0);  break;
        }
    }
}



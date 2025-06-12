/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio-hal.c
**
** $Id: gpio-hal.c 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Hardware abstraction layer module for GPIOs.
** This module handles the GPIOs. It provides functions to
** configure a GPIO as input or output and to get, set or toggle
** the pin-state of a GPIO.
** Before using the GPIOs the function gpioHAL_Init should be called to enable
** the clock for the GPIOs.
**
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

/* system includes */
#include "stm32f10x.h"

#include "xtypes.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* application includes */
#include "gpio-hal.h"
#include "gpio-hal_loc.h"
/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** gpioHAL_Init()
**
** Description:
** This function activates the controller peripheral clocks for port A to E.
** Datasheet: "When the peripheral clock is not active, the peripheral
** register values may not be readable by software and the returned value is
** always 0x0."
** This function should be called before using the GPIO registers.
**
**------------------------------------------------------------------------------
** Input:
**  void
**
** Return:
**  void
**
** Usage:
**  -
**
** Module Test: - NO -
**      Reason: only one Register setting
**------------------------------------------------------------------------------
*/
void gpioHAL_Init( void )
{

   /*
    * APB2 peripheral clock enable register (RCC_APB2ENR)
    */

   /*  IOP(x)EN: IO port (x) clock enable */
   RCC->APB2ENR |= GPIOHALCFG_APB2ENR_VALUE;

} /* end of gpioHAL_Init() */


/*------------------------------------------------------------------------------
**
** gpioHAL_ConfigureInput()
**
** Description:
** This function configures a GPIO as a input. An input can be configured for
** the following operating modes:
** - analog mode
** - floating input (this is the reset state)
** - input with pull-down
** - input with pull-up
**
**------------------------------------------------------------------------------
** Input:
**  sGpio    -  GPIO-Port, which shall be configured as input
**  ePin     -  GPIO-Pin, which shall be configured as input
**  eInCfg   -  input operating mode
**
** Return:
**  void
**
** Usage:
**  gpioHAL_IoPortClockEnable
**  gpioHAL_SetPin
**  gpioHAL_ClearPin
**  gpioHAL_SetConfigRegister
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
void gpioHAL_ConfigureInput ( GPIO_TypeDef*                sGpio,
                              CONST GPIO_PIN_ENUM          ePin,
                              CONST GPIO_INPUT_CONFIG_ENUM eInCfg )
{
   UINT8 u8IoConfig = ( UINT8 )( ~VALID_IO_CFG_MASK );/*lint !e835*//* pre-set with invalid value */
   /* Deactivate lint message 835 because the IO config contains zeros. */
   /* check parameters */
   gpioHAL_CheckPort(sGpio);

   GPIO_PIN_SAFETY_ASSERT( ePin );

   switch( eInCfg )
   {
      case IN_CONFIG_ANALOG:
      {
         u8IoConfig = VALUE_INPUT_ANALOG;
         break;
      }
      case IN_CONFIG_FLOATING:
      {
         u8IoConfig = VALUE_INPUT_FLOATING;
         break;
      }
      case IN_CONFIG_PULL_DOWN:
      {
         u8IoConfig = VALUE_INPUT_PULLUPDOWN;

         /* clear ODR bit for pull-down input */
         gpioHAL_ClearPin( sGpio, gpioHAL_GetPinMask( ePin ));
         break;
      }
      case IN_CONFIG_PULL_UP:
      {
         u8IoConfig = VALUE_INPUT_PULLUPDOWN;

         /* set ODR bit for pull-up input */
         gpioHAL_SetPin( sGpio, gpioHAL_GetPinMask( ePin ));
         break;
      }
      default:
      {
         GLOBFAIL_SAFETY_FAIL( GLOB_FAILCODE_VARIABLE_ERR );
         break;
      }
   }

   gpioHAL_SetConfigRegister( sGpio, ePin, u8IoConfig );

} /* end of gpioHAL_ConfigureInput() */


/*------------------------------------------------------------------------------
**
** gpioHAL_ConfigureOutput()
**
** Description:
** This function configures a GPIO as a output. An output can operate in the
** following modes:
**  - general purpose output push-pull
**  - general purpose output open drain
**  - alternate function output push-pull
**  - alternate function output open drain
**
** If the output is configured as an output, there can also be specified a
** maximum frequency.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio                   -  GPIO-Port, which shall be configured as output
**  ePin                    -  GPIO-Pin, which shall be configured as output
**  eOutCfg                 -  output configuration mode
**  GPIO_OUTPUT_MODE_ENUM   - maximum frequency
**
** Return:
**  void
**
** Usage:
**  gpioHAL_CheckPort
**  gpioHAL_SetConfigRegister
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
void gpioHAL_ConfigureOutput ( GPIO_TypeDef*                 sGpio,
                               CONST GPIO_PIN_ENUM           ePin,
                               CONST GPIO_OUTPUT_CONFIG_ENUM eOutCfg,
                               CONST GPIO_OUTPUT_MODE_ENUM   eOutMode )
{
   UINT8  u8IoConfig = ( UINT8 )( ~VALID_IO_CFG_MASK );/*lint !e835*//* pre-set with invalid value*/
   /* Deactivate lint message 835 because the IO config contains zeros. */

   /* check parameters */
   gpioHAL_CheckPort( sGpio );

   /* check position of port pin */
   GPIO_PIN_SAFETY_ASSERT( ePin );

   switch( eOutCfg )
   {
      case OUT_CONFIG_GPIO_PP:
      {
         u8IoConfig = VALUE_OUTPUT_GPIO_PP;
         break;
      }

      case OUT_CONFIG_GPIO_OD:
      {
         u8IoConfig = VALUE_OUTPUT_GPIO_OD;
         break;
      }

      case OUT_CONFIG_AF_PP:
      {
         u8IoConfig = VALUE_OUTPUT_AF_PP;
         break;
      }

      case OUT_CONFIG_AF_OD:
      {
         u8IoConfig = VALUE_OUTPUT_AF_OD;
         break;
      }

      default:
      {
         GLOBFAIL_SAFETY_FAIL( GLOB_FAILCODE_VARIABLE_ERR );
         break;
      }
   } /* switch( eOutCfg ) */


   switch( eOutMode )
   {
      case OUT_MODE_10MHZ:
      {
         u8IoConfig |= VALUE_IO_OUT10;
         break;
      }

      case OUT_MODE_2MHZ:
      {
         u8IoConfig |= VALUE_IO_OUT2;
         break;
      }

      case OUT_MODE_50MHZ:
      {
         u8IoConfig |= VALUE_IO_OUT50;
         break;
      }

      default:
      {
         GLOBFAIL_SAFETY_FAIL( GLOB_FAILCODE_VARIABLE_ERR );
         break;
      }
   } /* switch( eOutMode ) */

   gpioHAL_SetConfigRegister( sGpio, ePin, u8IoConfig );

} /* end of gpioHAL_ConfigureOutput() */


/*------------------------------------------------------------------------------
**
** gpioHAL_GetPin()
**
** Description:
** This function returns the input state of one or more pins
** of an IO port. The return value is IO_HIGH if all marked
** IO pins of the port are set, otherwise IO_LOW.
** If a single port pin state shall be read, a calling
** function has to ensure that solely the one corresponding
** bit is set in u16PinMask.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio       -  GPIO-Port, which input pin states shall be read
**  u16PinMask  -  specifies which pins shall be read
**
** Return:
**  GPIO_PINSTATE_ENUM   -  IO_HIGH, if all marked IO pins of the port are set
**                          IO_LOW, otherwise
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
GPIO_PINSTATE_ENUM gpioHAL_GetPin ( CONST GPIO_TypeDef* sGpio,
                                    CONST UINT16        u16PinMask )
{
   GPIO_PINSTATE_ENUM  eRet        = IO_LOW;
   UINT16              u16PinState;

   /* check parameters */
   gpioHAL_CheckPort(sGpio);

   GLOBFAIL_SAFETY_ASSERT_RET( 0u != u16PinMask,
                               GLOB_FAILCODE_INVALID_PARAM,
                               eRet );

   u16PinState = ( UINT16 )sGpio->IDR & u16PinMask;

   if( u16PinMask != u16PinState )
   {
      eRet = IO_LOW;
   }
   else
   {
      eRet = IO_HIGH;
   }

   return eRet;

} /* end of gpioHAL_GetPin() */


/*------------------------------------------------------------------------------
**
** gpioHAL_GetOuputPin()
**
** Description:
** This function returns the output state of one or more pins
** of an IO port. The return value is IO_HIGH if all marked
** IO pins of the port are set, otherwise IO_LOW.
** If a single port pin state shall be read, a calling
** function has to ensure that solely the one corresponding
** bit is set in u16PinMask.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio       -  GPIO-Port, which output pins states shall be read
**  u16PinMask  -  specifies which pins shall be read
**
** Return:
**  GPIO_PINSTATE_ENUM   -  IO_HIGH, if all marked output pin states of the
**                                   port are set
**                          IO_LOW, otherwise
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
GPIO_PINSTATE_ENUM gpioHAL_GetOuputPin ( CONST GPIO_TypeDef* sGpio,
                                         CONST UINT16        u16PinMask )
{
   GPIO_PINSTATE_ENUM  eRet        = IO_LOW;
   UINT16              u16PinState;

   /* check parameters */
   gpioHAL_CheckPort( sGpio );

   GLOBFAIL_SAFETY_ASSERT_RET( 0u != u16PinMask,
                               GLOB_FAILCODE_INVALID_PARAM,
                               eRet );

   u16PinState = ( UINT16 )sGpio->ODR & u16PinMask;

   if( u16PinMask != u16PinState )
   {
      eRet = IO_LOW;
   }
   else
   {
      eRet = IO_HIGH;
   }

   return eRet;

} /* end of gpioHAL_GetOuputPin()*/


/*------------------------------------------------------------------------------
**
** gpioHAL_SetPin()
**
** Description:
** This function is used to set one or more pins of an IO port.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio       -  GPIO-Port selection
**  u16PinMask  -  specifies which pins shall be set
**
** Return:
**  void
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
void gpioHAL_SetPin ( GPIO_TypeDef* sGpio,
                      CONST UINT16  u16PinMask)
{
   /* check parameters */
   gpioHAL_CheckPort(sGpio);

   GLOBFAIL_SAFETY_ASSERT( 0u != u16PinMask,
                           GLOB_FAILCODE_INVALID_PARAM );

   sGpio->ODR |= (UINT32)u16PinMask;

} /* end of gpioHAL_SetPin() */


/*------------------------------------------------------------------------------
**
** gpioHAL_ClearPin()
**
** Description:
** This function is used to reset (clear) one or more pins of
** an IO port.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio       -  GPIO-Port, the pins of which shall be cleared
**  u16PinMask  -  specifies which pins shall be cleared
**
** Return:
**  void
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
void gpioHAL_ClearPin ( GPIO_TypeDef* sGpio,
                        CONST UINT16  u16PinMask )
{
   /* check parameters */
   gpioHAL_CheckPort( sGpio );

   GLOBFAIL_SAFETY_ASSERT( 0u != u16PinMask,
                           GLOB_FAILCODE_INVALID_PARAM );

   sGpio->ODR &=  ~( ( UINT32 )u16PinMask );

} /* end of gpioHAL_ClearPin() */


/*------------------------------------------------------------------------------
**
** gpioHAL_TogglePin()
**
** Description:
** This function is used to toggle one or more pins of an IO port.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio       -  GPIO-Port, the pins of which shall be toggled
**  u16PinMask  -  specifies which pins shall be toggled
**
** Return:
**  void
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: NO
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
void gpioHAL_TogglePin ( GPIO_TypeDef* sGpio,
                         CONST UINT16  u16PinMask )
{
   /* check parameters */
   gpioHAL_CheckPort( sGpio );

   GLOBFAIL_SAFETY_ASSERT( 0u != u16PinMask,
                           GLOB_FAILCODE_INVALID_PARAM );

   sGpio->ODR ^= ( UINT32 )u16PinMask;

} /* end of gpioHAL_TogglePin() */


#ifdef __DEBUG__
/*------------------------------------------------------------------------------
**
** gpioHAL_InitSwTestPort()
**
** Description:
** This function initialized the SW test ports on GPIOE 2, 7 and as output
**
**------------------------------------------------------------------------------
** Input:
**  void
**
** Return:
**  void
**
** Usage:
**  gpioHAL_ConfigureOutput
**
** Module Test: NO
**      Reason: low complexity, only function calls
**------------------------------------------------------------------------------
*/
void gpioHAL_InitSwTestPort( void )
{

   /* SWtest(a): set as output push pull */
   gpioHAL_ConfigureOutput ( GPIOE_SW_TEST,
                             GPIO_PIN2_SW_TEST_A,
                             OUT_CONFIG_GPIO_PP,
                             OUT_MODE_50MHZ );

   /* SWtest(b): set as output push pull */
   gpioHAL_ConfigureOutput ( GPIOE_SW_TEST,
                             GPIO_PIN7_SW_TEST_B,
                             OUT_CONFIG_GPIO_PP,
                             OUT_MODE_50MHZ );

} /* end of gpioHAL_InitSwTestPort() */
#endif


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** gpioHAL_GetPinMask()
**
** Description:
** This function is a helper function to get the corresponding
** pin mask of a single IO pin.
**
**------------------------------------------------------------------------------
** Input:
**  enum ePin -  Selection of pin
**
** Return:
**  UINT16    -  pinmask of selected pin
**
** Usage:
**  -
**
** Module Test: - NO -
**      Reason: not complex
**------------------------------------------------------------------------------
*/
STATIC UINT16 gpioHAL_GetPinMask( CONST GPIO_PIN_ENUM ePin )
{
   UINT16 u16Ret;

   /* Check the value of ePin <= 15 */
   GPIO_PIN_SAFETY_ASSERT_RET( ePin, 0u );

   /* set the bit on position of ePin */
   u16Ret = (UINT16)  ( (UINT16)1u << (UINT16)ePin );/*lint !e701*/
   /* Info 701: Shift left of signed quantity (int) [MISRA 2004 Rule 10.5]
    * This statement contains no signed values. */

   return ( u16Ret );

} /* end of gpioHAL_GetPinMask() */


/*------------------------------------------------------------------------------
**
** gpioHAL_CheckPort()
**
** Description:
** This function checks whether a GPIO port is enclosed in the
** list of valid ports GPIOA to GPIOE or not. In the latter
** case a safety assert is triggered.
**
** With error handling if the GPIOx address is not correct.
**
**------------------------------------------------------------------------------
** Input:
**  sGpio  -  GPIO-Port, which shall be checked
**
** Return:
**  void
**
** Usage:
**  -
**
** Module Test: - NO -
**      Reason: not complex
**------------------------------------------------------------------------------
*/
STATIC void gpioHAL_CheckPort( CONST GPIO_TypeDef* sGpio )
{
   /* check the correct GPIOx address */
   if( (GPIOA != sGpio) &&
       (GPIOB != sGpio) &&
       (GPIOC != sGpio) &&
       (GPIOD != sGpio) &&
       (GPIOE != sGpio)   )
   {
      /* GPIO address is not correct, error */
      GLOBFAIL_SAFETY_FAIL( GLOB_FAILCODE_INVALID_PARAM );
   }

} /* end of gpioHAL_CheckPort() */



/*------------------------------------------------------------------------------
**
** gpioHAL_SetConfigRegister()
**
** Description:
** This function configures a pin of an IO port according to the passed
** configuration value.
**
** Each port has 16 pins and each pin is configured by 4 bits in the port
** configuration. Since 16*4 is twice the size of a register, the port pin
** configuration is subdivided into two registers: CRL for the configuration of
** the pins 0 to 7 and CRH for the pins 8 to 15.
**
** The configuration parameter of this function is a 4 bit value which must be
** shifted to the corresponding position in a 32 bit value; this value then
** can be set to the related CRL or CRH register after possible previous config
** bits at this position have been cleared.
** Example:
** ePin is GPIO_14.
** The corresponding register is CRH. The pin configuration bits for GPIO_14
** in this register are at
** IO15 IO14 IO13 IO12 IO11 IO10 IO9  IO8
** 0000 XXXX 0000 0000 0000 0000 0000 0000
**
** So u8CfgValue must be shifted (as a UINT32) for 24 bits to the left. This
** number of shifts can be calculated from ePin:
** (ePin - GPIO_8) * 4 = number of bits to shift (since ePin is > GPIO_7 we
** must subtract GPIO_8 from it.
**
** With error handling, if the ePin > 15
**
**------------------------------------------------------------------------------
** Input:
**  sGpio      - GPIO-Port, which shall be configured
**  ePin       - GPIO-Pin, which shall be configured
**  u8CfgValue - configuration value
**
** Return:
**  void
**
** Usage:
**  gpioHAL_CheckPort
**
** Module Test: - YES -
**      Reason: -
**------------------------------------------------------------------------------
*/
STATIC void gpioHAL_SetConfigRegister( GPIO_TypeDef*       sGpio,
                                       CONST GPIO_PIN_ENUM ePin,
                                       CONST UINT8         u8CfgValue )
{
   UINT32  u32IoConfig;    /* pin configuration bits to add to the IO config register */
   UINT32  u32ClrValue;    /* bits to clear before modifying the config register */
   UINT16  u16ShiftCount;  /* number of bits u8CfgValue must be shifted to the
                             left to form a 32 bit configuration value to be
                             written to the IO config register */

   /*check parameters, is it an exist port */
   gpioHAL_CheckPort( sGpio );

   /* error handling: check if ePin > 15 */
   GPIO_PIN_SAFETY_ASSERT( ePin );

   /* check the cfgValue that it is not on an invalid configuration, the only
    * invalid config is when Mode is set to Input and CNF to 0b11 or if the
    * high nibble is set to a value not zero. */
   if (  ( (VALID_IO_CFG_MASK | u8CfgValue) != VALID_IO_CFG_MASK )/*lint !e835*/
      || ( u8CfgValue == (VALUE_IO_IN + VALUE_OUTPUT_AF_OD) ))/*lint !e835 */
   /* Deactivate lint message 835: because the IO config contains zeros. */
   {
      /* error handling */
      GLOBFAIL_SAFETY_FAIL( GLOB_FAILCODE_INVALID_PARAM );
   }

   u32ClrValue = VALUE_CLEAR_REG;
   u32IoConfig = ( UINT32 )u8CfgValue;

   if ( ePin > GPIO_7 )
   {
      /* calculate the numbers of bits to shift (a multiple of 4) */
      u16ShiftCount = (UINT16)((UINT16)((UINT16)ePin - (UINT16)GPIO_8) << 2u);

      /* shift clear and config values */
      u32IoConfig <<= u16ShiftCount;
      u32ClrValue <<= u16ShiftCount;

      /* and assign to the corresponding CRH register */
      sGpio->CRH &= ( UINT32 )( ~u32ClrValue );
      sGpio->CRH |= u32IoConfig;
   }
   else /* GPIO_7 >= ePin */
   {
      /* calculate the numbers of bits to shift (a multiple of 4) */
      u16ShiftCount = ( UINT16 ) ( ( UINT16 ) ePin << 2u);

      /* shift clear and config values */
      u32IoConfig <<= u16ShiftCount;
      u32ClrValue <<= u16ShiftCount;

      /* and assign to the corresponding CRL register */
      sGpio->CRL &= ( UINT32 )( ~u32ClrValue );
      sGpio->CRL |= u32IoConfig;
   } /* end of if( ePin > GPIO_7 ) */


} /* end of gpioHAL_SetConfigRegister() */


/*******************************************************************************
**
** End of gpio-hal.c
**
********************************************************************************
*/

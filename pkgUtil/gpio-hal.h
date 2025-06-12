/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio-hal.h
**
** $Id: gpio-hal.h 2448 2017-03-27 13:45:16Z klan $
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
** Header of module gpio-hal.
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

#ifndef  GPIOHAL_H
#define  GPIOHAL_H

/*******************************************************************************
**
** Includes
**
********************************************************************************
*/


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! Bit-definitions of pinmasks */
#define GPIO_PINMASK_0    ((UINT16)0x0001u)  /*!< Pinmask for pin 0 */
#define GPIO_PINMASK_1    ((UINT16)0x0002u)  /*!< Pinmask for pin 1 */
#define GPIO_PINMASK_2    ((UINT16)0x0004u)  /*!< Pinmask for pin 2 */
#define GPIO_PINMASK_3    ((UINT16)0x0008u)  /*!< Pinmask for pin 3 */
#define GPIO_PINMASK_4    ((UINT16)0x0010u)  /*!< Pinmask for pin 4 */
#define GPIO_PINMASK_5    ((UINT16)0x0020u)  /*!< Pinmask for pin 5 */
#define GPIO_PINMASK_6    ((UINT16)0x0040u)  /*!< Pinmask for pin 6 */
#define GPIO_PINMASK_7    ((UINT16)0x0080u)  /*!< Pinmask for pin 7 */
#define GPIO_PINMASK_8    ((UINT16)0x0100u)  /*!< Pinmask for pin 8 */
#define GPIO_PINMASK_9    ((UINT16)0x0200u)  /*!< Pinmask for pin 9 */
#define GPIO_PINMASK_10   ((UINT16)0x0400u)  /*!< Pinmask for pin 10 */
#define GPIO_PINMASK_11   ((UINT16)0x0800u)  /*!< Pinmask for pin 11 */
#define GPIO_PINMASK_12   ((UINT16)0x1000u)  /*!< Pinmask for pin 12 */
#define GPIO_PINMASK_13   ((UINT16)0x2000u)  /*!< Pinmask for pin 13 */
#define GPIO_PINMASK_14   ((UINT16)0x4000u)  /*!< Pinmask for pin 14 */
#define GPIO_PINMASK_15   ((UINT16)0x8000u)  /*!< Pinmask for pin 15 */


/*
 * SWtest(a) on GPIOE2
 * SWtest(b) on GPIOE7
 */
#define GPIOE_SW_TEST                   GPIOE
#define GPIO_PIN2_SW_TEST_A             GPIO_2
#define GPIO_PINMASK2_SW_TEST_A         GPIO_PINMASK_2
#define GPIO_PIN7_SW_TEST_B             GPIO_7
#define GPIO_PINMASK7_SW_TEST_B         GPIO_PINMASK_7

/*
 * SC-CC-Select-C1
 *
 */
#define GPIO_SC_SELECT_C12             GPIOB
#define GPIO_PIN_SC_SELECT_C12         GPIO_9
#define GPIO_PINMASK_SC_SELECT_C12     GPIO_PINMASK_9


/*
 * This macro checks if the value of x is IO_LOW or IO_HIGH. If it is not on
 * this state, the Safety-Handler is called.
 * This macro is used in functions with no return value.
 * */
#define GPIO_PINSTATE_SAFETY_ASSERT(x) \
            GLOBFAIL_SAFETY_ASSERT((IO_LOW == x) || (IO_HIGH == x), \
                                   GLOB_FAILCODE_VARIABLE_ERR)

/*
 * This macro checks if the value of x is IO_LOW or IO_HIGH. If it is not on
 * this state, the Safety-Handler is called.
 * This macro is used in functions with a return value. This macro has a
 * mechanism for the unit test handling.
 * */
#define GPIO_PINSTATE_SAFETY_ASSERT_RET(x, ret) \
            GLOBFAIL_SAFETY_ASSERT_RET((IO_LOW == x) || (IO_HIGH == x), \
                                   GLOB_FAILCODE_VARIABLE_ERR, ret)

/*
 * This macro checks if the specified value of x in range or not. If it is
 * out of range the Safety-Handler is called.
 * This macro is used in functions with no return value.
 * */
#define GPIO_PIN_SAFETY_ASSERT(x) \
            GLOBFAIL_SAFETY_ASSERT(GPIO_15>= x, \
                                   GLOB_FAILCODE_VARIABLE_ERR)

/*
 * This macro checks if the specified value of x in range or not. If it is
 * out of range the Safety-Handler is called.
 * This macro is used in functions with a return value. This macro has a
 * mechanism for the unit test handling.
 * */
#define GPIO_PIN_SAFETY_ASSERT_RET(x, ret) \
            GLOBFAIL_SAFETY_ASSERT_RET(GPIO_15>= x, \
                                   GLOB_FAILCODE_VARIABLE_ERR, ret)

/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/
/*! Hamming distance here:
 *  This enum contains the available states of an IO Pin.
 * */
typedef enum
{
   IO_LOW                   = 0x3267u,   /*!< Identifier for low pin-state */
   IO_HIGH                  = 0x33f0u    /*!< Identifier for high pin-state */
} GPIO_PINSTATE_ENUM;


/*! Hamming distance here:
 *  This enum contains the available input configurations.
 * */
typedef enum
{
   IN_CONFIG_ANALOG         = 0x343bu,   /*!< Identifier for analog input mode */
   IN_CONFIG_FLOATING       = 0x35acu,   /*!< Identifier for floating input (reset state)  */
   IN_CONFIG_PULL_DOWN      = 0x3682u,   /*!< Identifier for input with pull-down */
   IN_CONFIG_PULL_UP        = 0x3715u    /*!< Identifier for input with pull-up */
} GPIO_INPUT_CONFIG_ENUM;

/*! Hamming distance here:
 *  This enum contains the available output configurations.
 * */
typedef enum
{
   OUT_CONFIG_GPIO_PP       = 0x3883u,   /*!< Identifier for general purpose output push-pull */
   OUT_CONFIG_GPIO_OD       = 0x3914u,   /*!< Identifier for general purpose output open drain */
   OUT_CONFIG_AF_PP         = 0x3a3au,   /*!< Identifier for alternate function output push-pull */
   OUT_CONFIG_AF_OD         = 0x3badu    /*!< Identifier for alternate function output open drain */
} GPIO_OUTPUT_CONFIG_ENUM;

/*! Hamming distance here:
 *  This enum contains the available operating modes for an output.
 * */
typedef enum
{
   OUT_MODE_10MHZ           = 0x3c66u,   /*!< Identifier for a maximum speed of 10 MHz */
   OUT_MODE_2MHZ            = 0x3df1u,   /*!< Identifier for a maximum speed of 2 MHz */
   OUT_MODE_50MHZ           = 0x3edfu    /*!< Identifier for a maximum speed of 50 MHz */
} GPIO_OUTPUT_MODE_ENUM;

/*! This enum contains identifiers for all IO Pins of one IO Port
 *  (no hamming distance here, enum value has to be equal to pin number) */
typedef enum
{
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
  GPIO_0  = 0u,         /*!< Identifier for IO Pin 0*/
  GPIO_1  = 1u,         /*!< Identifier for IO Pin 1*/
  GPIO_2  = 2u,         /*!< Identifier for IO Pin 2*/
  GPIO_3  = 3u,         /*!< Identifier for IO Pin 3*/
  GPIO_4  = 4u,         /*!< Identifier for IO Pin 4*/
  GPIO_5  = 5u,         /*!< Identifier for IO Pin 5*/
  GPIO_6  = 6u,         /*!< Identifier for IO Pin 6*/
  GPIO_7  = 7u,         /*!< Identifier for IO Pin 7*/
  GPIO_8  = 8u,         /*!< Identifier for IO Pin 8*/
  GPIO_9  = 9u,         /*!< Identifier for IO Pin 9*/
  GPIO_10 = 10u,        /*!< Identifier for IO Pin 10*/
  GPIO_11 = 11u,        /*!< Identifier for IO Pin 11*/
  GPIO_12 = 12u,        /*!< Identifier for IO Pin 12*/
  GPIO_13 = 13u,        /*!< Identifier for IO Pin 13*/
  GPIO_14 = 14u,        /*!< Identifier for IO Pin 14*/
  GPIO_15 = 15u         /*!< Identifier for IO Pin 15*/
/* RSM_IGNORE_QUALITY_END */
} GPIO_PIN_ENUM;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void gpioHAL_Init (void);

extern void gpioHAL_ConfigureInput( GPIO_TypeDef*                sGpio,
                                    CONST GPIO_PIN_ENUM          ePin,
                                    CONST GPIO_INPUT_CONFIG_ENUM eInCfg );

extern void gpioHAL_ConfigureOutput( GPIO_TypeDef*                 sGpio,
                                     CONST GPIO_PIN_ENUM           ePin,
                                     CONST GPIO_OUTPUT_CONFIG_ENUM eOutCfg,
                                     CONST GPIO_OUTPUT_MODE_ENUM   eOutMode );

extern GPIO_PINSTATE_ENUM gpioHAL_GetPin( CONST GPIO_TypeDef* sGpio,
                                          CONST UINT16        u16PinMask );

extern GPIO_PINSTATE_ENUM gpioHAL_GetOuputPin( CONST GPIO_TypeDef* sGpio,
                                               CONST UINT16        u16PinMask );

extern void gpioHAL_SetPin ( GPIO_TypeDef*      sGpio,
                             CONST UINT16       u16PinMask );

extern void gpioHAL_ClearPin ( GPIO_TypeDef*    sGpio,
                               CONST UINT16     u16PinMask );

extern void gpioHAL_TogglePin( GPIO_TypeDef* sGpio,
                               CONST UINT16 u16PinMask );

#ifdef __DEBUG__
extern void gpioHAL_InitSwTestPort( void );
#endif

#endif   /* inclusion lock */

/*******************************************************************************
**
** End of gpio-hal.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio-hal_loc.h
**
** $Id: gpio-hal_loc.h 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
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

#ifndef  GPIOHAL_LOC_H
#define  GPIOHAL_LOC_H


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
/*! This constants are used as bit-definitions for setting the configuration registers CRL and CRH*/
/*! Constant used to clear the registers*/
#define VALUE_CLEAR_REG         (UINT32)0x0000000Fu

/*-- Bit definitions for port configuration. --*/
/*! Bit-definition to configure IO Pin as analog input */
#define VALUE_INPUT_ANALOG      (UINT8)0u
/*! Bit-definition to configure IO Pin as floating input */
#define VALUE_INPUT_FLOATING    (UINT8)4u
/*! Bit-definition to configure IO Pin as pull-up or pull-down input */
#define VALUE_INPUT_PULLUPDOWN  (UINT8)8u
/*! Bit-definition to configure IO Pin as push-pull output */
#define VALUE_OUTPUT_GPIO_PP    (UINT8)0u
/*! Bit-definition to configure IO Pin as open-drain output */
#define VALUE_OUTPUT_GPIO_OD    (UINT8)4u
/*! Bit-definition to configure IO Pin as alternate function output push-pull */
#define VALUE_OUTPUT_AF_PP      (UINT8)8u
/*! Bit-definition to configure IO Pin as alternate function output open-drain */
#define VALUE_OUTPUT_AF_OD      (UINT8)12u

/*-- Bit definitions for port mode. --*/
/*! Bit-definition to configure the operating mode of an input */
#define VALUE_IO_IN             (UINT8)0u
/*! Bit-definition to configure the operating mode of an output with an maximum speed of 10 MHz */
#define VALUE_IO_OUT10          (UINT8)1u
/*! Bit-definition to configure the operating mode of an output with an maximum speed of 2 MHz */
#define VALUE_IO_OUT2           (UINT8)2u
/*! Bit-definition to configure the operating mode of an output with an maximum speed of 50 MHz */
#define VALUE_IO_OUT50          (UINT8)3u

/*! Bitmask used to evaluate if a bit-definition for configuration register is valid */
#define VALID_IO_CFG_MASK ( UINT8 ) ( VALUE_INPUT_ANALOG     | \
                                      VALUE_INPUT_FLOATING   | \
                                      VALUE_INPUT_PULLUPDOWN | \
                                      VALUE_OUTPUT_GPIO_PP   | \
                                      VALUE_OUTPUT_GPIO_OD   | \
                                      VALUE_OUTPUT_AF_PP     | \
                                      VALUE_OUTPUT_AF_OD     | \
                                      VALUE_IO_OUT10         | \
                                      VALUE_IO_OUT2          | \
                                      VALUE_IO_OUT50         )

/*
 * This constant contains the bit-definition for the APB2ENR-Register
 * to enable the clock for GPIOA to GPIOE
 */
#define GPIOHALCFG_APB2ENR_VALUE  (\
                                  /* enable the clock for GPIOA */\
                                  RCC_APB2ENR_IOPAEN |\
                                  /* enable the clock for GPIOB */\
                                  RCC_APB2ENR_IOPBEN |\
                                  /* enable the clock for GPIOC */\
                                  RCC_APB2ENR_IOPCEN | \
                                  /* enable the clock for GPIOD */\
                                  RCC_APB2ENR_IOPDEN | \
                                  /* enable the clock for GPIOE */\
                                  RCC_APB2ENR_IOPEEN   \
                                  )

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/



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
STATIC UINT16  gpioHAL_GetPinMask       ( CONST GPIO_PIN_ENUM ePin );
STATIC void    gpioHAL_CheckPort        ( CONST GPIO_TypeDef* sGpio );
STATIC void    gpioHAL_SetConfigRegister( GPIO_TypeDef*  sGpio,
                                          CONST GPIO_PIN_ENUM ePin,
                                          CONST UINT8         u8CfgValue );

#endif   /* GPIOHAL_LOC_H */

/*******************************************************************************
**
** End of gpio-hal_loc.h
**
********************************************************************************
*/

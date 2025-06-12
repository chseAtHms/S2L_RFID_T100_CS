/***************************************************************************************************
**    Copyright (C) 2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: i2c_DigitalComparator-hal_cfg.h
**     Summary: Configuration header of the I2C Digital Comparator module.
**              This is the configuration header. It contains all configurations which
**              are used in the module, e.g. constant definitions for array sizes.
**   $Revision: 3806 $
**       $Date: 2023-01-10 14:46:54 +0100 (Di, 10 Jan 2023) $
**      Author: A. Kramer
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: -
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef I2CHAL_CFG_H
#define I2CHAL_CFG_H


/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Base I2C address of the sensor */
/* usage of this macro constant depends on configuration. */
/*lint -esym(755, I2C_k_TMP102_SLAVE_ADDRESS_BASE) */
#define I2C_k_TMP102_SLAVE_ADDRESS_BASE     (UINT8)0x48U

/* Port Pins used for I2C */
#define I2C_k_PORT_SDA                      GPIOB
#define I2C_k_PIN_SDA                       GPIO_7
#define I2C_k_PINMASK_SDA                   GPIO_PINMASK_7

#define I2C_k_PORT_SCL                      GPIOB
#define I2C_k_PIN_SCL                       GPIO_6
#define I2C_k_PINMASK_SCL                   GPIO_PINMASK_6


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/

  
/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

#endif   /* <I2CHAL_CFG_H> */


/*** End Of File ***/


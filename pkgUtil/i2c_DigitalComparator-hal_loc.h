/***************************************************************************************************
**    Copyright (C) 2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: i2c_DigitalComparator-hal_cfg.h
**     Summary: Local header file of the I2C Digital Comparator module.
**              Contains the local constants and type definitions of the appendent c-file,
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
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef I2CHAL_LOC_H
#define I2CHAL_LOC_H


/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Addresses of the TMP102 registers */
#define I2C_k_TMP102_PTR_TEMPERATURE     (UINT8)0x00U
#define I2C_k_TMP102_PTR_CONFIGURATION   (UINT8)0x01U

/* This is the expected value we want to read from the TMP102's configuration register */
#define I2C_k_TMP102_EXPECTED_CONFIG     0x60A0U

/* The AL bit in the configuration register depends on the temperature. Thus we need to be able to
   mask it out. */
#define I2C_k_TMP102_CFG_AL              0x0020U

/* This value is returned between initialization and first real read of the temperature register */
#define I2C_k_INIT_TEMP_DC               -1   /* degrees celsius */
#define I2C_k_INIT_TEMP_ADCCOUNT         ((UINT16)((((I2C_k_INIT_TEMP_DC*10)+500)*4096)/2500))

/* Measurement ranges of the TMP102 in °C */
#define I2C_k_TMP102_MIN_VALUE_DC        -50
#define I2C_k_TMP102_MAX_VALUE_DC        126

/* When waiting for a flag in the I2C Controller Status register we wait this number of micro
   seconds until we declare a timeout (measurements have shown that the I2C Controller sets these
   flags in less than 3 micro seconds) */
#define I2C_k_REGISTER_WAITTIMEOUT_US    ((UINT32)15U)

/* Time for transmission of 8 bits on the I2C bus: 400kHz -> one bit is 2.5 micro seconds */
#define I2C_k_8_BIT_TIME_US              ((UINT32)20U)


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/

  
/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


#endif /* I2CHAL_LOC_H */


/*** End Of File ***/


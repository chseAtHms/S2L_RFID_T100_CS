/***************************************************************************************************
**    Copyright (C) 2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: i2c_DigitalComparator-hal.h
**     Summary: This module is the Hardware Abstraction Layer for the I2C.
**              It provides functions to initialize the I2C and to read a current value from the
**              TMP102 temperature sensor.
**   $Revision: 3806 $
**       $Date: 2023-01-10 14:46:54 +0100 (Di, 10 Jan 2023) $
**      Author: A. Kramer
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: i2cHAL_Init
**             i2cHAL_Read
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef I2CHAL_H
#define I2CHAL_H

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)


#include "i2c_DigitalComparator-hal_cfg.h"


/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Initialization function is called via this macro for fulfilling full code coverage.
   Integration/Acceptance testing will prove that the this macro calls the right function. */
#define TEMPERATURE_HAL_INIT()      i2cHAL_Init()


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/

  
/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    i2cHAL_Init

  Description:
    This function initializes the I2C according to the _cfg.h-File.
    More precisely it 
    - detects the pull-up resistors on the I2C lines
    - performs a reset of the TMP102 with a long reset on I2C clock
    - configures and starts the I2C master
    Attention: due to the TMP102 serial interface reset this function runs > 40ms.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
extern void i2cHAL_Init(void);


/***************************************************************************************************
  Function:
    i2cHAL_Read

  Description:
    This function runs the I2C reading state machine. In a round-robin principle it performs one
    step of:
    - writing the Pointer register (Configuration)
    - reading and comparing the Configuration register
    - writing the Pointer register (Temperature)
    - reading the Temperature register

  See also:
    -

  Parameters:
    -

  Return value:
    UINT16                 - temperature (converted to previous ADC units)

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
extern UINT16 i2cHAL_Read(void);


#endif  /* (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE) */

#endif /* I2CHAL_H */

/*** End Of File ***/


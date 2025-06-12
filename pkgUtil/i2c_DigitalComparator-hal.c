/***************************************************************************************************
**    Copyright (C) 2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: i2c_DigitalComparator-hal.c
**     Summary: This module is the Hardware Abstraction Layer for the I2C Temperature sensor.
**              This module handles the I2C Controller and external TMP102 temperature sensor
**              chip. It provides functions to initialize the I2C and to read a current value
**              from the external sensor.
**              Nomenclature is according to STM32F10x:
**              we use I2C1 at 0x40005400-0x400057FF with port pins PB6, PB7.
**   $Revision: 3972 $
**       $Date: 2023-03-24 15:06:49 +0100 (Fr, 24 Mrz 2023) $
**      Author: A. Kramer
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: i2cHAL_Init
**             i2cHAL_Read
**
**             Tmp102PresenceCheck
**             Tmp102WaitUntilStatusFlagSet
**             Tmp102WaitUntilControlBitCleared
**             Tmp102PointerSet
**             Tmp102Read16
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "xtypes.h"
#include "cfg_Config-sys.h"

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)


#include "stm32f10x.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "gpio-hal.h"
#include "timer-def.h"
#include "timer-hal.h"

/* Header-file of module */
#include "i2c_DigitalComparator-hal.h"
#include "i2c_DigitalComparator-hal_loc.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      38u

/* Our Controller's I2C address (not used here as we are always master) */
#define k_I2C_CONTROLLER_ADDRESS         0x003EU

/* When writing the address into the DR during a transfer then this flag determines whether it is a
   read(1) or write(0) operation */
#define k_I2C_READ_WRITE_FLAG            (UINT16)0x0001U

/* This is the clock frequency that drives the I2C peripheral */
#define k_APB1_CLOCK_MHZ                 36U

/* Duration of low level on SCL for reset of the I2C slave (micro seconds) */
#define k_I2C_RESET_SCL_TIMEOUT_US       41000U

/*! This enumeration defines the states for the I2C reading state machine.
 *  With a hamming distance >=4 */
typedef enum
{
   k_I2C_STATE_WR_PTR_CFG  = 0x000F,
   k_I2C_STATE_RD_CFG      = 0x00F0,
   k_I2C_STATE_WR_PTR_TMP  = 0x0F00,
   k_I2C_STATE_RD_TMP      = 0xF000
} t_I2C_SM_ENUM;

/* variable for the I2C reading state machine */
STATIC t_I2C_SM_ENUM e_I2cState;

/* latest temperature value read from the sensor */
STATIC RDS_UINT16 u16_CurrentTemperatureValue;


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC BOOL Tmp102PresenceCheck(void);
STATIC BOOL Tmp102WaitUntilStatusFlagSet(UINT16 u16_sr1mask,
                                         UINT32 u32_timeout_us);
STATIC BOOL Tmp102WaitUntilControlBitCleared(UINT16 u16_cr1mask,
                                             UINT32 u32_timeout_us);
STATIC BOOL Tmp102PointerSet(UINT8 u8_address);
STATIC UINT16 Tmp102Read16(void);


/***************************************************************************************************
**    global functions
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
void i2cHAL_Init(void)
{
  UINT32 u32_startTime_us;
  UINT32 u32_time_us;

  /* init state machine */
  e_I2cState = k_I2C_STATE_WR_PTR_CFG;

  /* set the value that is returned between the call of this function and until the i2cHAL_Read()
     has the acquired the first temperature value from the sensor */
  RDS_SET(u16_CurrentTemperatureValue, I2C_k_INIT_TEMP_ADCCOUNT);

  /* if we cannot find the TMP102 */
  if (Tmp102PresenceCheck() != TRUE)
  {
    /* this software runs on wrong or defective hardware */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_TMP102_PRESENCE, GLOBFAIL_ADDINFO_FILE(1u));
  }
  else  /* else: TMP102 was detected */
  {
    /* Before using a peripheral we must enable its clock in the RCC registers. */
    RCC->APB1ENR |= (UINT32)RCC_APB1ENR_I2C1EN;

    /* make sure the I2C controller is disabled and select I2C mode */
    I2C1->CR1   = 0U;

    /* Main pins for this peripheral are used. Thus no alternative function port pin remapping
       required. */

    /* Initialization sequence for a stuck I2C slave:
       There are (at least) 2 situations in which the I2C protocol can get stuck in a slave device:
         - master is reset (e.g. SW reset), but the slave cannot be reset
         - a spike caused by EMC influence interpreted as a clock signal
       For pure I2C devices a sequence of 16 clock pulses may advance the state machine of any
       blocked slave to a point where it accepts the next start condition again. The TMP102 is also
       capable of the SMBus protocol thus a low level longer than 40ms on the clock line will force
       a reset in its internal protocol state machine.
    */
    /* configure the I2C lines (SCL and SDA) as general purpose outputs */
    gpioHAL_ConfigureOutput(I2C_k_PORT_SCL, I2C_k_PIN_SCL, OUT_CONFIG_GPIO_OD, OUT_MODE_50MHZ);
    gpioHAL_ConfigureOutput(I2C_k_PORT_SDA, I2C_k_PIN_SDA, OUT_CONFIG_GPIO_OD, OUT_MODE_50MHZ);

    /* generate reset condition */
    gpioHAL_ClearPin(I2C_k_PORT_SCL, I2C_k_PINMASK_SCL);  /* I2C1_SCL */
    gpioHAL_SetPin(I2C_k_PORT_SDA, I2C_k_PINMASK_SDA);    /* I2C1_SDA */

    /* get the actual system time for timeout calculation */
    u32_startTime_us = timerHAL_GetSystemTime3();

    /* now wait until time expires */
    do
    {
      u32_time_us = timerHAL_GetSystemTime3();
      u32_time_us = u32_time_us - u32_startTime_us;
    } while (u32_time_us < k_I2C_RESET_SCL_TIMEOUT_US);

    /* put the I2C lines to idle state */
    gpioHAL_SetPin(I2C_k_PORT_SCL, I2C_k_PINMASK_SCL);    /* I2C1_SCL */
    gpioHAL_SetPin(I2C_k_PORT_SDA, I2C_k_PINMASK_SDA);    /* I2C1_SDA */

    /* configure the GPIO Pin for I2C1_SCL */
    gpioHAL_ConfigureOutput(I2C_k_PORT_SCL, I2C_k_PIN_SCL, OUT_CONFIG_AF_OD, OUT_MODE_50MHZ);
    /* configure the GPIO Pin for I2C1_SDA */
    gpioHAL_ConfigureOutput(I2C_k_PORT_SDA, I2C_k_PIN_SDA, OUT_CONFIG_AF_OD, OUT_MODE_50MHZ);

    /* our addresses in slave mode: not used, but set to a defined value */
    /* Bit14 should always be kept at 1 by software */
    I2C1->OAR1  = (UINT16)(0x4000U | k_I2C_CONTROLLER_ADDRESS);
    I2C1->OAR2  = 0x0000U;

    /* do not use DMA and interrupts, set peripheral clock frequency */
    I2C1->CR2   = k_APB1_CLOCK_MHZ & I2C_CR2_FREQ;

    /* Clock Control:
       I2C Fast Mode               => F/S  = 1
       Duty Cycle low/high = 2:1   => DUTY = 0
       I2C clock: 400kHz -> T = 2500ns = Thigh + Tlow = 833.3ns + 1666.7ns
       Thigh = CCR * Tpclk1   ;   Tlow = 2 * CCR * Tpclk1  ;  Tpclk1 = 1/36MHz = 27.8ns
       => CCR = Thigh/Tpclk1 = 833.3ns / 27.8ns = 30 */
    I2C1->CCR = I2C_CCR_FS | (UINT16)(30U & I2C_CCR_CCR);

    /* These bits must be programmed with the maximum SCL rise time (300ns) given
       in the I2C bus specification, incremented by 1
       36 MHz -> Period Tpclk1 = 27.8 ns -> (300ns / 27.8ns) + 1 = 11 */
    I2C1->TRISE = (11U & I2C_TRISE_TRISE);

    /* Enable the selected I2C peripheral */
    I2C1->CR1   |= I2C_CR1_PE;
  }
}


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
UINT16 i2cHAL_Read(void)
{
  UINT16 u16_registerValue;       /* value read from a register of the sensor */
  INT32 i32_temp;              /* temporary value for conversion to ADC units */
  UINT32 u32_temp;             /* temporary value for conversion to ADC units */
  UINT8  u8_temp;              /* temporary value for conversion to ADC units */
  INT8 i8_tmpDegree;                /* read the value from the TMP102 via I2C */
  UINT8 u8_decFract;                   /* decimal fraction of the temperature */

  /* actions executed in this function call depends on state */
  switch (e_I2cState)
  {
    case k_I2C_STATE_WR_PTR_CFG:
    {
      /* set Pointer register to Configuration register */
      if (Tmp102PointerSet(I2C_k_TMP102_PTR_CONFIGURATION) != TRUE)
      {
        /* error writing the Pointer register */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_PTR_WRITE_CFG, GLOBFAIL_ADDINFO_FILE(2u));
      }
      else
      {
        /* nothing to do */
      }
      /* advance to next state */
      e_I2cState = k_I2C_STATE_RD_CFG;
      break;
    }

    case k_I2C_STATE_RD_CFG:
    {
      /* read the configuration register */
      u16_registerValue = Tmp102Read16();

      /* if the read configuration register value differs from expectation (considering that the AL
         bit may be set or not - depending on temperature) */
      if ((u16_registerValue | I2C_k_TMP102_CFG_AL) != I2C_k_TMP102_EXPECTED_CONFIG)
      {
        /* error: configuration register has changed! */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_CONFIG_ERR, GLOBFAIL_ADDINFO_FILE(3u));
      }
      else  /* else: configuration register value is ok */
      {
        /* nothing to do */
      }
      /* advance to next state */
      e_I2cState = k_I2C_STATE_WR_PTR_TMP;
      break;
    }

    case k_I2C_STATE_WR_PTR_TMP:
    {
      /* set Pointer register to Temperature register */
      if (Tmp102PointerSet(I2C_k_TMP102_PTR_TEMPERATURE) != TRUE)
      {
        /* error writing the Pointer register */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_PTR_WRITE_TMP, GLOBFAIL_ADDINFO_FILE(4u));
      }
      else
      {
        /* nothing to do */
      }
      /* advance to next state */
      e_I2cState = k_I2C_STATE_RD_TMP;
      break;
    }

    case k_I2C_STATE_RD_TMP:
    {
      /* read the temperature register */
      u16_registerValue = Tmp102Read16();

      /* if reserved bits are set */
      if ((u16_registerValue & 0x000FU) != 0U)
      {
        /* temperature register format error */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_TEMP_FORM_ERR, GLOBFAIL_ADDINFO_FILE(5u));
      }
      else  /* else: format looks ok */
      {
        /* using temporary variable for conversion to avoid PC-Lint warning */
        u8_temp = (UINT8)(u16_registerValue >> 8);
        i8_tmpDegree = (INT8)u8_temp;
        u8_decFract = (UINT8)(u16_registerValue >> 4) & 0x0FU;

        /* if measurement result is not in the expected range */
        if (    (i8_tmpDegree < I2C_k_TMP102_MIN_VALUE_DC)
            || (i8_tmpDegree > I2C_k_TMP102_MAX_VALUE_DC)
          )
        {
          /* temperature measurement out of range error */
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_TEMP_RANGE_ERR, GLOBFAIL_ADDINFO_FILE(6u));
        }
        else  /* else: measurement resulted in a value in expected range */
        {
          /* Convert the TMP102 measurement result (in °C) to the units used on
            devices with analog sensor and ADC measurement:
              - Add the positions after the decimal point: 4 bits -> each count
                is the 16th part of a degree.
              - Due to the range check above we can be sure that the first
                line will always result in a correct positive value and after
                the final division the value will fit into an UINT16.
                At 126.9375°C the result is 2898.

            On the devices with analog sensor this formula applies:
                T[°C] = (((ADCvalue/4096)*2500mV)-500mV)/10mV/°C
            resolved to the ADCvalue that we want to return from this function:
                ADCvalue = (((T + 50) * 4096) / 250)
            Considering that the positions after the decimal point must be
            divided by 16 it is simpler to add this term right before the final
            division (thus instead of division by 16 and then multiplication by
            4096 only multiply by 256).
            The order of the operations is optimized for the most precise
            numerical result.
          */
          /* using temporary variable for conversion to avoid PC-Lint warning */
          i32_temp = (INT32)i8_tmpDegree + 50;
          u32_temp = (UINT32)(i32_temp);
          u32_temp = u32_temp * 4096U;
          u32_temp = u32_temp + ((UINT32)u8_decFract * 256U);
          u32_temp = u32_temp / 250U;

          /* transfer newly calculated value into static temperature variable */
          RDS_SET(u16_CurrentTemperatureValue, (UINT16)u32_temp);
        }
      }
      /* return to first state */
      e_I2cState = k_I2C_STATE_WR_PTR_CFG;
      break;
    }

    default:
    {
      /* invalid state */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_STATE_ERR, GLOBFAIL_ADDINFO_FILE(7u));
      break;
    }
  }

  return (RDS_GET(u16_CurrentTemperatureValue));
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    Tmp102PresenceCheck

  Description:
    This function configures the I2C pins as GPIO input pins and checks for the presence of the
    pull-up resistors on the SCL and SDA line.

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                 - external pull-ups were detected
    FALSE                - no external pull-ups detected

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
STATIC BOOL Tmp102PresenceCheck(void)
{
  BOOL o_ret = FALSE;

  /* configure the I2C lines (SCL and SDA) as general purpose inputs with weak pull-down */
  gpioHAL_ConfigureInput(I2C_k_PORT_SCL, I2C_k_PIN_SCL, IN_CONFIG_PULL_DOWN);
  gpioHAL_ConfigureInput(I2C_k_PORT_SDA, I2C_k_PIN_SDA, IN_CONFIG_PULL_DOWN);

  /* if SCL pin is low */
  if (gpioHAL_GetPin(I2C_k_PORT_SCL, I2C_k_PINMASK_SCL) == IO_LOW)
  {
    /* external pull-up not detected */
    /* return value already set */
  }
  /* else: if SDA pin is low */
  else if (gpioHAL_GetPin(I2C_k_PORT_SDA, I2C_k_PINMASK_SDA) == IO_LOW)
  {
    /* external pull-up not detected */
    /* return value already set */
  }
  else  /* else: SCL and SDA pin high */
  {
    /* both external pull-ups detected */
    o_ret = TRUE;
  }

  return (o_ret);
}


/***************************************************************************************************
  Function:
    Tmp102WaitUntilStatusFlagSet

  Description:
    This function polls the I2C Status Register 1 and waits until
      - (one of the bits in) u16_sr1mask is set
      - timeout (System Timer 3 is used for timeout checking)

  See also:

  Parameters:
    u16_sr1mask (IN)      - mask value that is compared to SR1
    u32_timeout_us (IN)   - timeout value in micro seconds

  Return value:
     TRUE                 - bit in SR1 is set
     FALSE                - timeout

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL Tmp102WaitUntilStatusFlagSet(UINT16 u16_sr1mask,
                                         UINT32 u32_timeout_us)
{
  UINT32 u32_startTime_us;
  UINT32 u32_time_us;
  BOOL o_retVal = TRUE;

  /* get the actual system time for timeout calculation */
  u32_startTime_us = timerHAL_GetSystemTime3();

  /* wait while status bit is not set and flag is set */
  while (((I2C1->SR1 & u16_sr1mask) == 0U) && (o_retVal == TRUE))
  {
    /* timeout calculation */
    u32_time_us = timerHAL_GetSystemTime3();
    u32_time_us = u32_time_us - u32_startTime_us;

    /* if timeout */
    if (u32_time_us >= u32_timeout_us)
    {
      /* stop waiting - break the loop */
      o_retVal = FALSE;
    }
    else  /* else: no timeout */
    {
      /* continue waiting */
    }
  }

  return (o_retVal);
}


/***************************************************************************************************
  Function:
    Tmp102WaitUntilControlBitCleared

  Description:
    This function polls the I2C Control Register 1 and waits until
      - (all of the bits in) u16_cr1mask is cleared
      - timeout (System Timer 3 is used for timeout checking)

  See also:

  Parameters:
    u16_cr1mask (IN)      - mask value that is compared to CR1
    u32_timeout_us (IN)   - timeout value in micro seconds

  Return value:
     TRUE                 - bit in CR1 is cleared
     FALSE                - timeout

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL Tmp102WaitUntilControlBitCleared(UINT16 u16_cr1mask,
                                             UINT32 u32_timeout_us)
{
  UINT32 u32_startTime_us;
  UINT32 u32_time_us;
  BOOL o_retVal = TRUE;

  /* get the actual system time for timeout calculation */
  u32_startTime_us = timerHAL_GetSystemTime3();

  /* wait while control bit is set and flag is set */
  while (((I2C1->CR1 & u16_cr1mask) != 0U) && (o_retVal == TRUE))
  {
    /* timeout calculation */
    u32_time_us = timerHAL_GetSystemTime3();
    u32_time_us = u32_time_us - u32_startTime_us;

    /* if timeout */
    if (u32_time_us >= u32_timeout_us)
    {
      /* stop waiting - break the loop */
      o_retVal = FALSE;
    }
    else  /* else: no timeout */
    {
      /* continue waiting */
    }
  }

  return (o_retVal);
}


/***************************************************************************************************
  Function:
    Tmp102Read16

  Description:
    This function initiates a read from the TMP102 sensor of a 16 bit register. This function
    doesn't care about register addressing. The address must already be set before in the pointer
    register.

  See also:

  Parameters:
    -

  Return value:
     UINT16               - value read from the currently addressed register

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC UINT16 Tmp102Read16(void)
{
  UINT16 u16_result = 0xFFFFU;
  UINT8  u8_b1;
  UINT8  u8_b2;
  /* mute lint warning "Previously assigned value to variable has not been used" */
  /*lint -esym(438, u16_r16Temp) */
  /*lint -esym(550, u16_r16Temp) */
  /*lint -esym(838, u16_r16Temp) */
  volatile UINT16 u16_r16Temp;

  /*
  ** Procedure according to STM32F103xx Reference Manual (RM0008) 26.3.3:
  ** I2C Master Mode - Master Receiver - Method 2 - Special case for 2 byte
  ** receive - Figure 276.
  */

  /* Enable POS
     ACK bit controls the (N)ACK of the next byte being received in the shift
     register. The PEC bit indicates that next byte in shift register is a
     PEC (Packet error checking).*/
  I2C1->CR1 = I2C1->CR1 | I2C_CR1_POS;

  /* Enable Acknowledge: our Acknowledge will be returned after a byte is
     received (matched address or data) */
  I2C1->CR1 = I2C1->CR1 | I2C_CR1_ACK;

  /* Generate Start = enter master mode */
  I2C1->CR1 = I2C1->CR1 | I2C_CR1_START;

  /* Wait until SB flag is set */
  if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_SB, I2C_k_REGISTER_WAITTIMEOUT_US) != TRUE)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT1, GLOBFAIL_ADDINFO_FILE(8u));
  }
  else
  {
    /* Send slave address for read operation:
       7-bit address in most significant bits, LSB is read(1) / write(0) flag */
    I2C1->DR = (UINT16)((UINT16)I2C_k_TMP102_SLAVE_ADDRESS_BASE << 1) | (k_I2C_READ_WRITE_FLAG);

    /* Wait until slave address byte has been transmitted and ACK has been received
       (transmission of 8 bits on I2C + setting of the flag in the I2C controller) */
    if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_ADDR,
                                     (I2C_k_8_BIT_TIME_US + I2C_k_REGISTER_WAITTIMEOUT_US)) != TRUE)
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT2, GLOBFAIL_ADDINFO_FILE(9u));
    }
    else
    {
      /* for 7-bit addressing, the ADDR bit is set after the master transmitted
         the address byte and received the ACK from the slave now check for
         failure */
      if ((I2C1->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR | I2C_SR1_OVR)))
      {
        /* slave did not respond with ACK */

         /* Generate Stop */
         I2C1->CR1 = I2C1->CR1 | I2C_CR1_STOP;
         /* Clear AF Flag */
         I2C1->SR1 = I2C1->SR1 & (UINT16)~(I2C_SR1_AF);
         /* return error */
      }
      else  /* else: acknowledge successful */
      {
        /* By disabling interrupts we make sure that clearing of the ACK bit happens right after
           clearing the ADDR flag - ensuring that clearing the ACK bit (Event EV6_1) happens before
           the Data is sent.
           References:
             - STM32 Errata (ES0340 - Rev17) section 2.9.1
             - Application Note AN2824 section 1.2.1 Note 2
        */
        __disable_irq();
        {
          /* Clear ADDR flag by reading both status registers */
          u16_r16Temp = I2C1->SR1;
          u16_r16Temp = I2C1->SR2;

          /* clear the ack bit */
          I2C1->CR1 = I2C1->CR1 & (UINT16)~(I2C_CR1_ACK);
        }
        __enable_irq();

        /* Wait until receive buffer is full 
          (reception of 16 bits on I2C + setting of the flag in the I2C controller) */
        if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_BTF,
            (UINT32)((2UL * I2C_k_8_BIT_TIME_US) + I2C_k_REGISTER_WAITTIMEOUT_US)) != TRUE)
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT3, GLOBFAIL_ADDINFO_FILE(10u));
        }
        else
        {
          /* To prevent data corruption we must make sure that the STOP condition on the bus is
             generated before reading the data register. This is done by disabling interrupts.
             References:
               - STM32 Errata (ES0340 - Rev17) section 2.9.2
               - Application Note AN2824 section 1.2.1 Note 1
          */
          __disable_irq();
          {
            /* Generate Stop */
            I2C1->CR1 = I2C1->CR1 | I2C_CR1_STOP;

            /* Read high byte of temperature */
            u8_b1 = (UINT8)I2C1->DR;
          }
          __enable_irq();

          /* Read low byte of temperature */
          u8_b2 = (UINT8)I2C1->DR;

          /* Wait until STOP bit is cleared by hardware */
          if (Tmp102WaitUntilControlBitCleared(I2C_CR1_STOP,
              I2C_k_REGISTER_WAITTIMEOUT_US) != TRUE)
          {
            GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT8, GLOBFAIL_ADDINFO_FILE(11u));
          }
          else
          {
            /* Clear POS and ACK */
            I2C1->CR1 = I2C1->CR1 & (UINT16)~(I2C_CR1_POS | I2C_CR1_ACK);

            /* combine the two bytes to a 16 bit value */
            u16_result = (UINT16)((UINT16)u8_b1 << 8);
            u16_result = (UINT16)(u16_result | u8_b2);
          }
        }
      }
    }
  }

  return (u16_result);
}


/***************************************************************************************************
  Function:
    Tmp102PointerSet

  Description:
    This function initiates a write to the TMP102 sensor's Pointer register. The address to be
    written into this register is passed as a parameter to this function.

  See also:

  Parameters:
    u8_address (IN)       - value to be written into the TMP102 pointer register

  Return value:
     TRUE                 - success
     FALSE                - failure

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL Tmp102PointerSet(UINT8 u8_address)
{
  BOOL            o_retVal = FALSE;
  /* mute lint warning "Previously assigned value to variable has not been used" */
  /*lint -esym(438, u16_pSetTemp) */
  /*lint -esym(550, u16_pSetTemp) */
  /*lint -esym(838, u16_pSetTemp) */
  volatile UINT16 u16_pSetTemp;

  /*
  ** Procedure according to STM32F103xx Reference Manual (RM0008) 26.3.3:
  ** I2C Master Mode - Master Transmitter - Figure 273.
  */

  /* Generate Start = enter master mode */
  I2C1->CR1 = I2C1->CR1 | I2C_CR1_START;

  /* Wait until SB flag is set (start condition generated) */
  if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_SB, I2C_k_REGISTER_WAITTIMEOUT_US) != TRUE)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT4, GLOBFAIL_ADDINFO_FILE(12u));
  }
  else
  {
    /* Send slave address for write operation:
       7-bit address in most significant bits, LSB is read(1) / write(0) flag */
    I2C1->DR = (UINT16)((UINT16)I2C_k_TMP102_SLAVE_ADDRESS_BASE << 1)
             & (UINT16)~(k_I2C_READ_WRITE_FLAG);

    /* Wait until slave address byte has been transmitted and ACK has been received
       (transmission of 8 bits on I2C + setting of the flag in the I2C controller) */
    if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_ADDR,
                                     (I2C_k_8_BIT_TIME_US + I2C_k_REGISTER_WAITTIMEOUT_US)) != TRUE)
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT5, GLOBFAIL_ADDINFO_FILE(13u));
    }
    else
    {
      /* for 7-bit addressing, the ADDR bit is set after the master transmitted
         the address byte and received the ACK from the slave now check for
         failures */
      if ((I2C1->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR | I2C_SR1_OVR)))
      {
        /* slave did not respond with ACK */

         /* Generate Stop */
         I2C1->CR1 = I2C1->CR1 | I2C_CR1_STOP;
         /* Clear AF Flag */
         I2C1->SR1 = I2C1->SR1 & (UINT16)~(I2C_SR1_AF);
         /* return error */
      }
      else  /* else: acknowledge successful */
      {
        /* Clear ADDR flag by reading both status registers */
        u16_pSetTemp = I2C1->SR1;
        u16_pSetTemp = I2C1->SR2;

        /* Wait until data register is empty */
        if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_TXE,
                                         I2C_k_REGISTER_WAITTIMEOUT_US) != TRUE)
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT6, GLOBFAIL_ADDINFO_FILE(14u));
        }
        else
        {
          /* write pointer register value */
          I2C1->DR = u8_address;

          /* Wait until data register is empty
             (transmission of 8 bits on I2C + setting of the flag in the I2C controller) */
          if (Tmp102WaitUntilStatusFlagSet(I2C_SR1_BTF,
                                           (I2C_k_8_BIT_TIME_US + I2C_k_REGISTER_WAITTIMEOUT_US))
              != TRUE)
          {
            GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT7, GLOBFAIL_ADDINFO_FILE(15u));
          }
          else
          {
            /* in case of failure */
            if ((I2C1->SR1 & (I2C_SR1_AF | I2C_SR1_ARLO | I2C_SR1_BERR | I2C_SR1_OVR)))
            {
              /* slave did not respond with ACK */

              /* Generate Stop */
              I2C1->CR1 = I2C1->CR1 | I2C_CR1_STOP;
              /* Clear AF Flag */
              I2C1->SR1 = I2C1->SR1 & (UINT16)~(I2C_SR1_AF);
              /* return error */
            }
            else  /* else: acknowledge successful */
            {
              /* Generate Stop */
              I2C1->CR1 = I2C1->CR1 | I2C_CR1_STOP;

              /* Wait until STOP bit is cleared by hardware */
              if (Tmp102WaitUntilControlBitCleared(I2C_CR1_STOP,
                  I2C_k_REGISTER_WAITTIMEOUT_US) != TRUE)
              {
                GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT9,
                                        GLOBFAIL_ADDINFO_FILE(16u));
              }
              else
              {
                /* success */
                o_retVal = TRUE;
              }
            }
          }
        }
      }
    }
  }

  return (o_retVal);
}


#endif  /* (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE) */

/*** End Of File ***/

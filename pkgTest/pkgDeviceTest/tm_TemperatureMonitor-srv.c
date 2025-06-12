/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** tm_TemperatureMonitor-srv.c
**
** $Id: tm_TemperatureMonitor-srv.c 4178 2024-01-10 15:08:06Z ankr $
** $Revision: 4178 $
** $Date: 2024-01-10 16:08:06 +0100 (Mi, 10 Jan 2024) $
** $Author: ankr $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Service module for temperature monitor.
** This module is used to crosswise monitor the temperature of the safety
** microcontrollers. It provides a function to test, if the read temperature is
** within the limits specified in the corresponding cfg-header.
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

/*******************************************************************************
**
** includes
**
********************************************************************************
*/
/* system includes */
#include "stm32f10x.h"

#include "xtypes.h"           /*!< embeX data types */
#include "xdefs.h"
#include "rds.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "cfg_Config-sys.h"
#include "gpio-hal.h"
#include "adc_AnalogComparator-hal.h"
#include "i2c_DigitalComparator-hal.h"
#include "ipcs_IpcSync-sys.h"
#include "ipcx_ExchangeData-sys.h"
#include "ipcx_ExchangeData-sys_def.h"

/* application includes */
#include "tm_TemperatureMonitor-srv.h"   /*!< own header file */
#include "tm_TemperatureMonitor-srv_loc.h"

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

/*! This variable holds the ADC raw value (12 Bit ADC resolution). */
STATIC UINT16 tmSRV_u16AdcRawValue = (UINT16)0u;

/*! This variable indicates the status of the first step of temperature test 
 * (own channel, before sync) */
STATIC TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_eTempTestStatusOwnChannel = TMSRV_TEMP_TEST_FAILED;

/*! This variable indicates the status of the second step of temperature test 
 * (other channel, after sync) */
STATIC TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_eTempTestStatusOtherChannel = TMSRV_TEMP_TEST_FAILED;

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** tmSRV_Init()
**
** Description:
**    This function initiates the module. More precisely, it configures the
**    GPIOs for temperature monitoring as analog inputs.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    gpioHAL_ConfigureInput();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void tmSRV_Init (void)
{
   /* configure IO for Sensor  */
   gpioHAL_ConfigureInput( TMSRV_SENSOR_PORT,
                           TMSRV_SENSOR_PIN,
                           IN_CONFIG_ANALOG );
}


/*------------------------------------------------------------------------------
**
** tmSRV_CheckTemperature()
**
** Description:
**    This function reads the current analog value of the temperature sensor
**    input and compares the current value with a reference value specified in
**    the cfg-header. If the temperature value is not equal to its expected
**    value, an error bit is set to tmSRV_u8TestResult respectively.
**    Upon each call of this function, the variable tmSRV_u8TestResult is
**    shifted 1 bit to the left. If there are old test values they are cleared
**    in that way that the bits are shifted out of the byte.
**    After reading the temperature value the result-variable is evaluated and
**    filtered according to definitions in the corresponding _cfg.h-file.
**    The test result is transmitted to the other controller to evaluate the
**    result of the test in the next step. The test result of this controller
**    is stored for the next step.
**
**    Attention:
**    For a successful temperature test execution the order of the function
**    calls must be correct. In the first step the function
**    tmSRV_CheckTemperature must be called. In the next cycle the function
**    tmSRV_SyncTemperature must be called. In the meantime of these functions
**    an IPC communication is done to exchange the test results. So the gap
**    should be large enough to fulfill an IPC.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    tmSRV_eTempTestStatusOwnChannel    - TMSRV_TEMP_TEST_PASSED
**                                       - TMSRV_TEMP_TEST_FAILED
**
** Usage:
**    adcHAL_Read();
**    tmSRV_EvaluateTestResult();
**    tmSRV_ExChangeTestResult
**
** Module Test:
**    - YES -
**------------------------------------------------------------------------------
*/
void tmSRV_CheckTemperature (void)
{
   /* RDS union initializations */
   LOCAL_STATIC(, RDS_UINT8,  tmSRV_u8TestResult, TMSRV_RESULT_BYTE_INIT); /*lint !e708 */
   GLOB_RETCODE_ENUM                   eFilteredResult;
   UINT8                               u8TestResult;

   /* get the RDS variables */
   u8TestResult = RDS_GET(tmSRV_u8TestResult);

   /* shift result byte one bit to the left (used to count errors, see [SRS_398]),
    * a shifting out of the value is wanted to clear the old test results */
   u8TestResult = (UINT8)(u8TestResult << 1u);

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
   /* MEASSURE TEMPERATURE / GET TEST RESULTS (from I2C, see [SRS_2249]) */
   /* Be aware that in case of the digital sensor:
      - a first range check of the measured value was already done in the I2C HAL
      - the value was converted to the same units as the analog sensor
      - the sensor only measures every 250ms - so the value is not updated every cycle
    */
   tmSRV_u16AdcRawValue = i2cHAL_Read();
#else
   /* MEASSURE TEMPERATURE / GET TEST RESULTS (from ADC, see [SRS_699]) */
   tmSRV_u16AdcRawValue = adcHAL_Read(TMSRV_SENSOR_ADCCH,
                          TMSRV_ADC_READ_CNT);
#endif

   /* check if the temperature is in range (see [SRS_267]) */
   /* lint warning 960 disabled because compiler stores calculated value and not float */
   if ((tmSRV_u16AdcRawValue <= TMSRV_TEMP_VAL_LOW_LEVEL) ||  /*lint !e960*/
       (tmSRV_u16AdcRawValue > TMSRV_TEMP_VAL_HIGH_LEVEL))    /*lint !e960*/
   {
      /* set error bit (bit used for counting errors, see [SRS_398]) */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      u8TestResult |= (UINT8)1u;
      /* RSM_IGNORE_QUALITY_END */
   }

   /* write back the RDS variables */
   RDS_SET(tmSRV_u8TestResult, u8TestResult);

   /*filter test results*/
   eFilteredResult = tmSRV_EvaluateTestResult(tmSRV_u8TestResult);

   /* If test of this controller was passed, then set status to
    * PASSED, else to FAILED. In each case, call function to initiate IPC
    * transmission and send the test result to the other controller
    * (see [SRS_397]).
    * In case of an illegal result value, the Safety Handler is called. */
   if (GLOB_RET_OK == eFilteredResult)
   {
      ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_TEMP_MESS, (UINT32)TMSRV_TEMP_TEST_PASSED);
      tmSRV_eTempTestStatusOwnChannel = TMSRV_TEMP_TEST_PASSED;
   }
   else if (GLOB_RET_TEMPERATURE_ERR == eFilteredResult)
   {
      ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_TEMP_MESS, (UINT32)TMSRV_TEMP_TEST_FAILED);
      tmSRV_eTempTestStatusOwnChannel = TMSRV_TEMP_TEST_FAILED;
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*------------------------------------------------------------------------------
**
** tmSRV_SyncTemperature()
**
** Description:
**    checks the temperature test result received from other channel/uC.
**    If both temperature checks (on both controllers) are OK, then the
**    identifier will be set to passed else to failed.
**
**    Attention:
**    For a successful temperature test execution the order of the function
**    calls must be correct. In the first step the function
**    tmSRV_CheckTemperature must be called. In the next cycle the function
**    tmSRV_SyncTemperature must be called. In the meantime of these functions
**    an IPC communication is done to exchange the test results. So the gap
**    should be large enough to fulfill an IPC.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    tmSRV_eTempTestStatus         - TMSRV_TEMP_TEST_PASSED
**                                  - TMSRV_TEMP_TEST_FAILED
**
** Usage:
**
** Module Test:
**    - NO -
**      Reason: low complexity, no arithmetic operation.
**------------------------------------------------------------------------------
*/
void tmSRV_SyncTemperature (void)
{
   UINT32 u32IpcRecValue;

   /* get temperature measurement from other channel (from IPC RX buffer),
    * see [SRS_397] */
   u32IpcRecValue = ipcxSYS_GetUINT32inclWait( IPCXSYS_IPC_ID_TEMP_MESS );

   if (u32IpcRecValue == (UINT32)TMSRV_TEMP_TEST_PASSED)
   {
      /* save state of other channel */
      tmSRV_eTempTestStatusOtherChannel = TMSRV_TEMP_TEST_PASSED;
   }
   else
   {
      /* save state of other channel to failed in all other cases */
      tmSRV_eTempTestStatusOtherChannel = TMSRV_TEMP_TEST_FAILED;
   }

   /* if both tests on both controllers were passed, then set status to
    * PASSED, else to FAILED. IN case of an illegal value the Safety Handler
    * is called. */
   if ( (tmSRV_eTempTestStatusOwnChannel == TMSRV_TEMP_TEST_PASSED) \
     && (tmSRV_eTempTestStatusOtherChannel == TMSRV_TEMP_TEST_PASSED))
   {
      /* temperature in range, no need for actions here */
   }
   /* If the result of the temperature check is failed at uC1 or uC2,
    * the software calls FS state (see [SRS_268]) */
   else if ( (tmSRV_eTempTestStatusOwnChannel == TMSRV_TEMP_TEST_FAILED) \
          || (tmSRV_eTempTestStatusOtherChannel == TMSRV_TEMP_TEST_FAILED))
   {
      GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_TEMPERATURE_ERR);
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*------------------------------------------------------------------------------
**
** tmSRV_GetTempTestStatusOwnChannel()
**
** Description:
**    This function returns the status of the temperature test of the own channel.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    tmSRV_eTempTestStatusOwnChannel    - TMSRV_TEMP_TEST_PASSED
**                                       - TMSRV_TEMP_TEST_FAILED
**
** Usage:
**
** Module Test:
**    - NO -
**    Reason: function returns only one value.
**------------------------------------------------------------------------------
*/
TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_GetTempTestStatusOwnChannel (void)
{
   return tmSRV_eTempTestStatusOwnChannel;
/* RSM_IGNORE_QUALITY_BEGIN Notice #2 - Function name length > 32 characters */
}
/* RSM_IGNORE_QUALITY_END */


/*------------------------------------------------------------------------------
**
** tmSRV_GetTempTestStatusOtherChannel()
**
** Description:
**    This function returns the status of the temperature test of the other channel.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    tmSRV_eTempTestStatusOtherChannel  - TMSRV_TEMP_TEST_PASSED
**                                       - TMSRV_TEMP_TEST_FAILED
**
** Usage:
**
** Module Test:
**    - NO -
**    Reason: function returns only one value.
**------------------------------------------------------------------------------
*/
TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_GetTempTestStatusOtherChannel (void)
{
   return tmSRV_eTempTestStatusOtherChannel;
/* RSM_IGNORE_QUALITY_BEGIN Notice #2 - Function name length > 32 characters */
}
/* RSM_IGNORE_QUALITY_END */


/*------------------------------------------------------------------------------
**
** tmSRV_GetAdcRawValue()
**
** Description:
**    This function returns the ADC raw value of the temperature.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    16 Bit raw value (12 Bit ADC resolution)
**
** Usage:
**
** Module Test:
**    - NO -
**    Reason: function returns only one value.
**------------------------------------------------------------------------------
*/
UINT16 tmSRV_GetAdcRawValue (void)
{
   return tmSRV_u16AdcRawValue;
}

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** tmSRV_EvaluateTestResult()
**
** Description:
**    This function evaluates the test result. In case of an error a fail
**    counter is incremented. Are there n errors in a row, the fail counter
**    reaches its threshold TMSRV_TEST_THRESHOLD and the function returns the
**    error GLOB_RET_TEMPERATURE_ERR.
**
**------------------------------------------------------------------------------
** Inputs:
**    u8RdsResultByte            - Result byte
**
** Outputs:
**    eRet                       - GLOB_RET_OK: Everything is OK
**                                 GLOB_RET_TEMPERATURE_ERR: fail counter
**                                                           reached limit
**
** Module Test:
**    - YES -
**------------------------------------------------------------------------------
*/
STATIC GLOB_RETCODE_ENUM tmSRV_EvaluateTestResult (RDS_UINT8 u8RdsResultByte)
{
   GLOB_RETCODE_ENUM eRet = GLOB_RET_OK;
   UINT8 u8ResultByte;
   UINT8 u8FailCounter = (UINT8)0u;
   UINT8 u8Loop;

   u8ResultByte = RDS_GET(u8RdsResultByte);

   /* EVALUATE TEST RESULTS, if there are more than "TMSRV_TEST_FILTER_SIZE"
    * in a row, the test result is failed (see [SRS_398]) */
   for (u8Loop = (UINT8)0u; u8Loop < TMSRV_TEST_FILTER_SIZE; u8Loop++)
   {
      if (((UINT8)(1u << u8Loop) & u8ResultByte) != 0u)
      {
         u8FailCounter++;
      }
      else
      {
         /*have to be failed in row, otherwise counter is reseted*/
         u8FailCounter = 0u;
      }

      if (TMSRV_TEST_THRESHOLD <= u8FailCounter)
      {
         eRet = GLOB_RET_TEMPERATURE_ERR;
         /* RSM_IGNORE_QUALITY_BEGIN Notice #44   - Keyword 'break' identified outside a 'switch' 
         ** structure */
         break; /* prevent further loop-execution for overwriting the detection of 3 fails in row */
         /* RSM_IGNORE_QUALITY_END */
      }
   }

   return eRet;
}


/*******************************************************************************
**
** End of tm_TemperatureMonitor-srv.c
**
********************************************************************************
*/

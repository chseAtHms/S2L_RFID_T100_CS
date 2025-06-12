/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doSetGetPin.c
**
** $Id: doSetGetPin.c 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** functions for Port-Pin access, like setting/getting diagnostic outputs/inputs
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

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "cfg_Config-sys.h"

#include "rds.h"

#include "gpio_cfg.h"
#include "diInput.h"

/* Module header */
#include "gpio-hal.h"

#include "diDoDiag.h"
#include "doCfg.h"
#include "doPortMap.h"
#include "doErrHdl.h"

/* Header-file of module */
#include "doSetGetPin.h"

/* generate compiler warning if discharge pin is de-activated (e.g. in Debug-Mode) */
#if (USE_DISCHARGE != TRUE)
   #warning "discharge pin deactivated for debug usage"
#endif


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

/*------------------------------------------------------------------------------
** wait cycle counter for wait states between DO and Discharge switching
**------------------------------------------------------------------------------
*/
STATIC RDS_UINT8 doSetGetPin_au8DoDisWait[GPIOCFG_NUM_DO_TOT];
STATIC RDS_UINT8 doSetGetPin_au8DisDoWait[GPIOCFG_NUM_DO_TOT];

/*------------------------------------------------------------------------------
** nominal value of Test-DOs (used to check if the DO pin has the expected value)
**------------------------------------------------------------------------------
*/
STATIC GPIO_STATE_ENUM doSetGetPin_eNomValueTestDo[GPIOCFG_NUM_DO_TOT];


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/
STATIC void doSetGetPin_SetDigOutPin(CONST UINT8 u8doNum, CONST GPIO_STATE_ENUM eVal);
STATIC void doSetGetPin_SwitchDoPinOnC1(CONST UINT8 u8doNum);
STATIC void doSetGetPin_SwitchDoPinOnC2(CONST UINT8 u8doNum);
STATIC void doSetGetPin_SwitchDoPinOffC1(CONST UINT8 u8doNum);
STATIC void doSetGetPin_SwitchDoPinOffC2(CONST UINT8 u8doNum);



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doSetGetPin_Init()
**
** Description:
**    initialize the variables/counters for the waittime between switching
**    DO output and Discahrge pin
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_Init(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_Init(void)
{
   UINT8 u8Index;

   /* do for all available DOs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      RDS_SET(doSetGetPin_au8DoDisWait[u8Index], 0u);
      RDS_SET(doSetGetPin_au8DisDoWait[u8Index], 0u);
      /* set nominal vale of Test-DOs, which is HIGH (inverted value of DO) */
      doSetGetPin_eNomValueTestDo[u8Index] = eGPIO_HIGH;
   }
}

/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOn()
**
** Description:
**    set a dedicated "safe" output pin to HIGH (TRUE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**    These timings are checked here by a local counter which bases on the
**    requirement to call this function every 200ms (calling less frequently
*     will not cause any damage, only change timing).
**    This function shall be the default one to call for standard DO setting
**    processes
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOn(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_SwitchDoPinOn(CONST UINT8 u8doNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if input parameter "u8doNum" (DO number) is valid, is done in called
    * functions, so there is no additional check necessary here */

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         doSetGetPin_SwitchDoPinOnC1(u8doNum);
         break;

      case SAFETY_CONTROLLER_2:
         doSetGetPin_SwitchDoPinOnC2(u8doNum);
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }

   /* if wait time to activate Discharge was reached, then the discharge pin
    * was set in the blocks above and the expected of Test-DO pin may be set
    * and used accordingly */
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ((RDS_GET(doSetGetPin_au8DoDisWait[u8doNum])) >= DOSETGETPIN_DO_HIGH_WAIT) /*lint !e948*/
   {
      /* expected value of Test-DO is LOW after both DO pin and discharge pin
       * were set to HIGH (Hint: DO and corresponding discharge located at
       * different uCs) */
      doSetGetPin_eNomValueTestDo[u8doNum] = eGPIO_LOW;
   }
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ((RDS_GET(doSetGetPin_au8DoDisWait[u8doNum])) < DOSETGETPIN_DO_HIGH_WAIT) /*lint !e948*/
   {
      RDS_INC(doSetGetPin_au8DoDisWait[u8doNum]);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOff()
**
** Description:
**    set a dedicated "safe" output pin to LOW (FALSE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**    These timings are checked here by a local counter which bases on the
**    requirement to call this function every 200ms (calling less frequently
*     will not cause any damage, only change timing).
**    This function shall be the default one to call for standard DO setting
**    processes.
**    "DO off" is used for Requirements: [SRS_75]
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOff(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_SwitchDoPinOff(CONST UINT8 u8doNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if input parameter "u8doNum" (DO number) is valid, is done in called
    * functions, so there is no additional check necessary here */

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         doSetGetPin_SwitchDoPinOffC1(u8doNum);
         break;

      case SAFETY_CONTROLLER_2:
         doSetGetPin_SwitchDoPinOffC2(u8doNum);
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }

   /* if wait time to activate Discharge was reached, then the discharge pin
    * was set in the blocks above and the expected of Test-DO pin may be set
    * and used accordingly */
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ((RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) >= DOSETGETPIN_DISC_LOW_WAIT) /*lint !e948*/
   {
      /* expected value of Test-DO is HIGH after both DO pin and discharge pin
       * were set to LOW (Hint: DO and corresponding discharge located at
       * different uCs) */
      doSetGetPin_eNomValueTestDo[u8doNum] = eGPIO_HIGH;
   }

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ((RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) 
           < (DOSETGETPIN_DISC_LOW_WAIT + DOSETGETPIN_DISC_LOW_TIME)) /*lint !e948*/
   {
      RDS_INC(doSetGetPin_au8DisDoWait[u8doNum]);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_CheckPinState()
**
** Description:
**    Checks if the DO output has the nominal/expected value [SRS_638]:
**    - If DO was set to "HIGH", the according TestDo should be "LOW"
**    - If DO was set to "LOW", the according TestDo should be "HIGH"
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_CheckPinState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_CheckPinState(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* check Test_DO pin value */

   if (doSetGetPin_eNomValueTestDo[u8doNum] != DI_INPUTVAL_TESTDO(u8doNum))
   {
      /* check failed, see [SRS_639] */
      doErrHdl_PinStateCheckFailed(u8doNum);
   }
   else
   {
      /* check OK, see [SRS_639] */
      doErrHdl_PinStateCheckOk(u8doNum);
   }
}

/*------------------------------------------------------------------------------
**
** doSetGetPin_DoPin()
**
** Description:
**    set a dedicated "safe" output pin to HIGH (TRUE) or LOW (FALSE)
**    ATTENTION: DO shall only be set to HIGH if corresponding discharge pin
**    is HIGH for at least 100us. Use function therefore with care!
**    For standard DO On/Off use function "doSetGetPin_SetDoPinSafe"
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**    - eVal: value (TRUE or FALSE)
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_DoPin(x, y);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_SetDoPin(CONST UINT8 u8doNum, CONST  GPIO_STATE_ENUM eVal)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* is diNum equal number, then its an input located at uC1 */
         if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
         {
            doSetGetPin_SetDigOutPin(u8doNum, eVal);
         }
         else
         {
            /* do not change pin in this case, because request for other uC */
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* is diNum equal number, then its an input located at uC1 */
         if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
         {
            /* do not change pin in this case, because request for other uC */
         }
         else
         {
            doSetGetPin_SetDigOutPin(u8doNum, eVal);
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_SetDisChrgPin()
**
** Description:
**    set discharge pin belonging to a "safe" output to HIGH (TRUE) or LOW (FALSE)
**    ATTENTION: Discharge pin may only be set to "LOW" if corresponding DO is
**    at least 400us "LOW"! Use function therefore with care!
**    For standard DO On/Off use function "doSetGetPin_SetDoPinSafe" (includes
**    setting of discharge pin)
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**    - eVal: value (TRUE or FALSE)
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SetDisChrgPin(x, y);
**
** Module Test:
**    - NO -
**    Reason: low complexity, calculation easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_SetDisChrgPin(CONST UINT8 u8doNum, CONST GPIO_STATE_ENUM eVal)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   if (eGPIO_HIGH == eVal)
   {
      gpioHAL_SetPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
   }
   else if (eGPIO_LOW == eVal )
   {
#if (USE_DISCHARGE == TRUE)
      gpioHAL_ClearPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
#endif
   }
   else
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_GetDisChrgPin()
**
** Description:
**    gets value of discharge output
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output which belongs to this discharge
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_GetDisChrgPin(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, calculation easy to understand
**------------------------------------------------------------------------------
*/
GPIO_STATE_ENUM doSetGetPin_GetDisChrgPin(CONST UINT8 u8doNum)
{
   GPIO_STATE_ENUM eRetPinState;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   if (IO_HIGH == 
        (gpioHAL_GetOuputPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask)))
   {
      eRetPinState = eGPIO_HIGH;
   }
   else
   {
      eRetPinState = eGPIO_LOW;
   }

   return eRetPinState;
}

/*------------------------------------------------------------------------------
**
** doSetGetPin_SetPin()
**
** Description:
**    set any output pin to HIGH (TRUE) or LOW (FALSE)
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**    - eVal: value (TRUE or FALSE)
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SetPin(x, y);
**
** Module Test:
**    - NO -
**    Reason: low complexity, calculation easy to understand
**------------------------------------------------------------------------------
*/
void doSetGetPin_SetPin(GPIO_TypeDef* sGpio, CONST UINT16 u16PinMask, CONST GPIO_STATE_ENUM eVal)
{
   if (eGPIO_HIGH == eVal)
   {
      gpioHAL_SetPin(sGpio, u16PinMask);
   }
   else if (eGPIO_LOW == eVal )
   {
      gpioHAL_ClearPin(sGpio, u16PinMask);
   }
   else
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_RstDisDo()
**
** Description:
**    resets the counter which are used to count the steps between switching
**    of DO and Discharge Pins.
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_RstDisDo(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**------------------------------------------------------------------------------
*/
void doSetGetPin_RstDisDo(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   RDS_SET(doSetGetPin_au8DoDisWait[u8doNum], 0u);
   RDS_SET(doSetGetPin_au8DisDoWait[u8doNum], 0u);
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doSetGetPin_digOutPin()
**
** Description:
**    set a "safe" output pin to HIGH (TRUE) or LOW (FALSE)
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**    - eVal: value (TRUE or FALSE)
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_digOutPin(x, y);
**
** Module Test:
**    - NO -
**    Reason: low complexity, calculation easy to understand
**------------------------------------------------------------------------------
*/
STATIC void doSetGetPin_SetDigOutPin(CONST UINT8 u8doNum, CONST GPIO_STATE_ENUM eVal)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   if (eGPIO_HIGH == eVal)
   {
      gpioHAL_SetPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
   }
   else if (eGPIO_LOW == eVal )
   {
      gpioHAL_ClearPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
   }
   else
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_INVALID_PARAM);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOnC1()
**
** Description:
**    set a dedicated "safe" output pin at uC1 to HIGH (TRUE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOnC1(x);
**
** Module Test:
**    - YES -
**    Reason: complexity, not easy to understand
**------------------------------------------------------------------------------
*/
STATIC void doSetGetPin_SwitchDoPinOnC1(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* reset wait time for next "DO switch off" command */
   RDS_SET(doSetGetPin_au8DisDoWait[u8doNum], 0u);

   /* DO0, DO2, DO4, ... => DOs located at uC1 (counting starting from 0 in SW) */
   if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
   {
      /* is wait time reached to set DO ON (other uC sets discharge pin when wait time starts) */
      /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
      if ((RDS_GET(doSetGetPin_au8DoDisWait[u8doNum])) >= DOSETGETPIN_DO_HIGH_WAIT) /*lint !e948*/
      {
         /* set DO pin On */
         gpioHAL_SetPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
      }
   }
   /* DO1, DO3, DO5, ... => DOs located at uC2 (counting starting from 0 in SW) */
   else
   {
      /* set discharge pin in first step (other uC waits to set corresponding DO later) */
      gpioHAL_SetPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
   }
}


/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOnC2()
**
** Description:
**    set a dedicated "safe" output pin at uC2 to HIGH (TRUE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOnC2(x);
**
** Module Test:
**    - YES -
**    Reason: complexity, not easy to understand
**------------------------------------------------------------------------------
*/
STATIC void doSetGetPin_SwitchDoPinOnC2(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* reset wait time for next "DO switch off" command */
   RDS_SET(doSetGetPin_au8DisDoWait[u8doNum], 0u);

   /* DO0, DO2, DO4, ... => DOs located at uC1 (counting starting from 0 in SW) */
   if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
   {
      /* set discharge pin in first step (other uC waits to set corresponding DO later) */
      gpioHAL_SetPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
   }
   /* DO1, DO3, DO5, ... => DOs located at uC2 (counting starting from 0 in SW) */
   else
   {
      /* is wait time reached to clear discharge pin (other uC sets corresponding DO pin when wait 
       * time starts) */
      /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
      if ((RDS_GET(doSetGetPin_au8DoDisWait[u8doNum])) >= DOSETGETPIN_DO_HIGH_WAIT) /*lint !e948*/
      {
         /* set DO pin On */
         gpioHAL_SetPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
      }
   }

}


/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOffC1()
**
** Description:
**    set a dedicated "safe" output pin at uC1 to LOW (FALSE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOffC1(x);
**
** Module Test:
**    - YES -
**    Reason: complexity, not easy to understand
**------------------------------------------------------------------------------
*/
STATIC void doSetGetPin_SwitchDoPinOffC1(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* reset wait time for next "DO switch on" command */
   RDS_SET(doSetGetPin_au8DoDisWait[u8doNum], 0u);

   /* DO0, DO2, DO4, ... => DOs located at uC1 (counting starting from 0 in SW) */
   if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
   {
      /* switch OFF DO pin */
      gpioHAL_ClearPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
   }
   /* DO1, DO3, DO5, ... => DOs located at uC2 (counting starting from 0 in SW) */
   else
   {
#if (USE_DISCHARGE == TRUE)
      /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
      if ( (RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) 
              >= (DOSETGETPIN_DISC_LOW_WAIT + DOSETGETPIN_DISC_LOW_TIME)) /*lint !e948*/
      {
         /* set discharge pin back to HIGH again, see [SRS_344] */
         gpioHAL_SetPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
      }
      else if ((RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) 
              >= DOSETGETPIN_DISC_LOW_WAIT) /*lint !e948*/
      {
         /* time window to activate Discharge between DOSETGETPIN_DISC_LOW_WAIT and
          * (DOSETGETPIN_DISC_LOW_WAIT + DOSETGETPIN_DISC_LOW_TIME), see [SRS_344] */
         gpioHAL_ClearPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
      }
      else
      {
         /* nothing to do... */
      }
#endif
   }

}



/*------------------------------------------------------------------------------
**
** doSetGetPin_SwitchDoPinOffC2()
**
** Description:
**    set a dedicated "safe" output pin at uC2 to LOW (FALSE) including
**    the handling of the discharge pin.
**    When setting DO to HIGH, Discharge pin must already be HIGH for at least
**    100us, when setting DO to LOW, Discharge pin may be set to LOW (discharge)
**    not earlier than 400us later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - u8doNum: number/id of output
**
** Outputs:
**    - None -
**
** Usage:
**    doSetGetPin_SwitchDoPinOffC2(x);
**
** Module Test:
**    - YES -
**    Reason: complexity, not easy to understand
**------------------------------------------------------------------------------
*/
STATIC void doSetGetPin_SwitchDoPinOffC2(CONST UINT8 u8doNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8doNum);

   /* reset wait time for next "DO switch on" command */
   RDS_SET(doSetGetPin_au8DoDisWait[u8doNum], 0u);

   /* DO0, DO2, DO4, ... => DOs located at uC1 (counting starting from 0 in SW) */
   if (DIDODIAG_IS_VAL_EQUAL(u8doNum))
   {
#if (USE_DISCHARGE == TRUE)
      /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
      if ( (RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) 
              >= (DOSETGETPIN_DISC_LOW_WAIT + DOSETGETPIN_DISC_LOW_TIME)) /*lint !e948*/
      {
         /* set discharge pin back to HIGH again, see [SRS_344] */
         gpioHAL_SetPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
      }
      else if ((RDS_GET(doSetGetPin_au8DisDoWait[u8doNum])) 
              >= DOSETGETPIN_DISC_LOW_WAIT) /*lint !e948*/
      {
         /* time window to activate Discharge between DOSETGETPIN_DISC_LOW_WAIT and
          * (DOSETGETPIN_DISC_LOW_WAIT + DOSETGETPIN_DISC_LOW_TIME), see [SRS_344] */
         gpioHAL_ClearPin(asDisChrgInit[u8doNum>>1u].port, asDisChrgInit[u8doNum>>1u].pinmask);
      }
      else
      {
         /* nothing to do... */
      }
#endif
   }
   /* DO1, DO3, DO5, ... => DOs located at uC2 (counting starting from 0 in SW) */
   else
   {
      /* switch OFF DO pin */
      gpioHAL_ClearPin(asOutputInit[u8doNum>>1u].port, asOutputInit[u8doNum>>1u].pinmask);
   }

}


/*******************************************************************************
**
** End of doSetGetPin.c
**
********************************************************************************
*/

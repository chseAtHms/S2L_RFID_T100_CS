/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiagVsup.c
**
** $Id: doDiagVsup.c 2448 2017-03-27 13:45:16Z klan $
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
** module handling the Voltage Supervision (VSUP) test states of the
** output section.
**
** Vsup-Test: Test of voltage supervision (HW-Block enabling/disabling the
**            ability to control the digital outputs). There is only one
**            voltage supervision block, so one test for the whole output HW.
**
** Principle of implementation:
** A counter is increased with every call of the module. If the counter reaches
** a certain value (define), then the diagnostic test has to be executed
** (execution mode of the module).
** If no other test is ongoing, test will switch to execution mode directly, if
** there is any other test ongoing, it will enter first an "request state" and
** enter execution mode when the other test was finished.
** During execution mode the same counter is used to execute the several test
** steps.
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

#include "rds.h"

/* Module header */
#include "cfg_Config-sys.h"
#include "gpio-hal.h"
#include "gpio_cfg.h"
#include "fiParam.h"
#include "doCfg.h"
#include "doPortMap.h"
#include "diInput.h"
#include "doSetGetPin.h"
#include "diDoDiag.h"
#include "doDiag.h"
#include "doErrHdl.h"
#include "doDiagVsup.h"
#include "doDiagVsup_priv.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing test state and counters for the different DOs,
** Voltage Supervision (VSUP) test
**------------------------------------------------------------------------------
*/
DO_DIAG_VSUP_STATE_STRUCT doDiagVsup_sState;


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/
/* Variable to identify if the VSUP-Test was executed */
STATIC TRUE_FALSE_ENUM doDiagVsup_eVsupTestExecuted = eFALSE;

/* Variable to indicate that at least one DO is active. */
STATIC TRUE_FALSE_ENUM doDiagVsup_eDosActive = eFALSE;

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doDiagVsup_Init()
**
** Description:
**    initialization of the VSUP-Test module of digital safe outputs
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doDiagVsup_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiagVsup_Init (void)
{
   /* VSUP-Test is initialized in a way that the test takes place as fast as
    * possible after the DI/DO processing starts */

   /* set VSUP test request directly to execute test once directly after startup */
   doDiagVsup_sState.eTestState = eDO_DIAG_VSUP_REQ;
   /* reset test counter: only reason is that then both 1st and cyclic test execution starts from 
    * value 0 and can use the same timing values during test execution */
   RDS_SET(doDiagVsup_sState.u32TestCnt, DO_DIAG_VSUP_CYC_TEST_WAITTIME);

   /* reset test execution identifier */
   doDiagVsup_RstTestExecutedVar();
}


/*------------------------------------------------------------------------------
**
** doDiagVsup_HandleTestState()
**
** Description:
**    State machine of the VSUP-Test.
**    Test counters are incremented with every function call, the calling
**    frequency of this function is the time base, because counters are
**    incremented with every function call (definitions for timing can be found
**    in "doDiagVsup_priv.h").
**    Possible States:
**    eDIAG_WAIT: time for test not yet reached, output in the wait/idle
**                    time before first test
**    eDIAG_REQ:  time for test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC: execution of test
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doDiagVsup_HandleTestState();
**
** Module Test:
**    - NO -
**    Reason: easy to understand, only function calls and counter increase
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiagVsup_HandleTestState (void)
{
   switch (doDiagVsup_sState.eTestState)
   {
      case eDO_DIAG_VSUP_WAIT:
         /* increase/handle test(-state) counters in wait mode, but not in requested mode */
         RDS_INC(doDiagVsup_sState.u32TestCnt);
         /* call function to check if cyclic test has to be executed,
          * hint: cyclic test has lower prio then first test, if another pin is already in test mode
          * then current pin will not change to cyclic test mode (order of calling changes prio) */
         doDiagVsup_CheckCycTestStart();
         break;

      case eDO_DIAG_VSUP_REQ:
         /* call function to check if cyclic test has to be executed,
          * hint: cyclic test has lower prio then first test, if another pin is already in test mode
          * then current pin will not change to cyclic test mode (order of calling changes prio) */
         doDiagVsup_CheckCycTestStart();
         /* Hint: No increment of test counter in "request" states (test pending
          *       for execution) */
         break;

      case eDO_DIAG_VSUP_EXEC:
         /* increase/handle test(-state) counters */
         RDS_INC(doDiagVsup_sState.u32TestCnt);
         /* call function to execute test */
         doDiagVsup_ExecTest();
         break;

      default:
         /* error: invalid state/value of variable */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;

   }
}


/*------------------------------------------------------------------------------
 **
 ** doDiagVsup_WasTestExecuted()
 **
 ** Description:
 **    This function returns the value of the Test-Execute-Variable (which shall
 **    be set whenever the VSUP-Test has been executed).
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    TRUE_FALSE_ENUM: value of the Test-Execute-Variable
 **
 ** Usage:
 **    x = doDiagVsup_WasTestExecuted()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM doDiagVsup_WasTestExecuted (void)
{
   return doDiagVsup_eVsupTestExecuted;
}


/*------------------------------------------------------------------------------
 **
 ** doDiagVsup_RstTestExecutedVar()
 **
 ** Description:
 **    Resets the Test-Execute-Variable to "eFALSE"
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiagVsup_RstTestExecutedVar()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: IRQ Scheduler
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void doDiagVsup_RstTestExecutedVar (void)
{
   doDiagVsup_eVsupTestExecuted = eFALSE;
}


/*------------------------------------------------------------------------------
 **
 ** doDiagVsup_ActivateVsupTest()
 **
 ** Description:
 ** This function sets the flag to TRUE to indicate the VSUP Test that at least
 ** one DO is active.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiagVsup_ActivateVsupTest()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: IRQ Scheduler
 **-----------------------------------------------------------------------------
 */
void doDiagVsup_ActivateVsupTest (void)
{
   doDiagVsup_eDosActive = eTRUE;
}


/*------------------------------------------------------------------------------
 **
 ** doDiagVsup_DeactivateVsupTest()
 **
 ** Description:
 ** This function sets the flag to FALSE to indicate the VSUP Test that all
 ** DOs are inactive.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiagVsup_DeactivateVsupTest()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: IRQ Scheduler
 **-----------------------------------------------------------------------------
 */
void doDiagVsup_DeactivateVsupTest (void)
{
   doDiagVsup_eDosActive = eFALSE;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doDiagVsup_CheckCycTestStart()
**
** Description:
**    checks if the voltage supply (VSUP) test has to be executed.
**    If the Test-Counter variable reaches a certain value, then the Test-State
**    is set to "execution of test" or (if this is not possible) to
**    "request test"
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_CheckCycTestStart();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_CheckCycTestStart (void)
{
   /* time for cyclic test? */
   if ( (RDS_GET(doDiagVsup_sState.u32TestCnt)) >= (DO_DIAG_VSUP_CYC_TEST_WAITTIME))/*lint !e948*/
      /* Lint message deactivated, problem in the Macro RDS_GET. The condition can be TRUE in case 
       * of an error. */
   {
      /* and no other test currently ongoing? */
      if (FALSE == doDiag_IsAnyTestOngoing())
      {
         /* set state "execution of test" */
         doDiagVsup_sState.eTestState = eDO_DIAG_VSUP_EXEC;
         /* reset test counter: only reason is that then both 1st and cyclic test execution starts 
          * from value 0
          * and can use the same timing values during test execution */
         RDS_SET(doDiagVsup_sState.u32TestCnt, DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      }
      else
      {
         /* execution mode currently not possible, because other test ongoing,
          * so set state "requesting test" */
         doDiagVsup_sState.eTestState = eDO_DIAG_VSUP_REQ; /* assign test state */
      }
   }
}


/*------------------------------------------------------------------------------
**
** doDiagVsup_ExecTest()
**
** Description:
**    executes the Voltage-Supervision-Test.
**    As both uCs are involved in a test, depending on the current controller
**    different actions are executed. Which action has to be executed by which
**    uC is toggled every second test for signal "TEST_3V3"
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_ExecTest();
**
** Module Test:
**    - YES -
**    Reason: complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_ExecTest (void)
{
   LOCAL_STATIC(, TRUE_FALSE_ENUM, eVsupToggle, eFALSE);

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* VSUP test execute function called => set variable */
   doDiagVsup_eVsupTestExecuted = eTRUE;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         if (eFALSE == eVsupToggle)
         {
            /* call test execution function for uC1, test step 1 */
            doDiagVsup_ExecTestStep1C1();
         }
         else if (eTRUE == eVsupToggle)
         {
            /* call test execution function for uC1, test step 2 */
            doDiagVsup_ExecTestStep2C1();
         }
         else
         {
            /* error: invalid value of variable */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         }
         break;

      case SAFETY_CONTROLLER_2:
         if (eFALSE == eVsupToggle)
         {
            /* call test execution function for uC2, test step 1 */
            doDiagVsup_ExecTestStep1C2();
         }
         else if (eTRUE == eVsupToggle)
         {
            /* call test execution function for uC2, test step 2 */
            doDiagVsup_ExecTestStep2C2();
         }
         else
         {
            /* error: invalid value of variable */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
         /* no break, fall through, in case of invalid value of controller ID */
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }


   /* if test was ended meanwhile, toggle mode */
   if (eDO_DIAG_VSUP_WAIT == doDiagVsup_sState.eTestState)
   {
      if (eFALSE == eVsupToggle)
      {
         eVsupToggle = eTRUE;
      }
      else if (eTRUE == eVsupToggle)
      {
         eVsupToggle = eFALSE;
      }
      else
      {
         /* error: invalid value of variable */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }

   }
}



/*------------------------------------------------------------------------------
**
** doDiagVsup_ExecTestStep1C1()
**
** Description:
**    executes the Voltage-Supervision-Test.
**    first part of VSUP-Test, on uC1, different steps, see [SRS_342]:
**    - DO_DIAG_VSUP_TEST_START
**       . set TEST_3V3 to LOW
**    - DO_DIAG_VSUP_TEST_DISCHARGE
**       . set all discharge pins to LOW
**    - DO_DIAG_VSUP_TEST_READVAL
**       . evaluate test results
**       . set discharge pins to HIGH
**       . set TEST_3V3 to HIGH
**    - DO_DIAG_VSUP_TEST_END
**       . reset of test states
**
**    If the test counter is larger than the last state DO_DIAG_VSUP_TEST_END,
**    the safety handler is called.
**    If the test counter is smaller than the last state and it is not equal
**    with another state there is no action.
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_ExecTestStep1C1();
**
** Module Test:
**    - NO -
**    Reason: function calls and port settings
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_ExecTestStep1C1 (void)
{
   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.
    *
    * Note 948: Lint message deactivated, problem in the Macro RDS_GET. The
    * condition can be TRUE in case of an error.
    *
    * Deactivated Misra Rule 10.1, the macro DO_DIAG_VSUP_TEST_READVAL
    * contains another macro with a shift operation that PC-Lint identify as
    * complex integer expression.
    * */

   UINT8 u8DoNum;
   UINT32 u32TestCntTmp;

   /* get RDS variable to temporary variable */
   u32TestCntTmp = RDS_GET(doDiagVsup_sState.u32TestCnt);

   /* check/execute different steps of test execution */
   if (DO_DIAG_VSUP_TEST_START == u32TestCntTmp) /*lint !e835 */
   {
      /* set Test_3v3 pin to LOW */
      doSetGetPin_SetPin(PORT_TEST_3V3, PINMASK_TEST_3V3, eGPIO_LOW);
   }
   else if (DO_DIAG_VSUP_TEST_DISCHARGE == u32TestCntTmp) /*lint !e835 */
   {
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted), to discharge output */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_LOW);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_READVAL == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* call function to evaluate current state of TEST_DO pin */
      doDiagVsup_EvalTestResStep1();
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted) to disable discharging of other uC
          * (needed to re-enable output) */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_HIGH);
      }
      /* set Test_3v3 pin to HIGH */
      doSetGetPin_SetPin(PORT_TEST_3V3, PINMASK_TEST_3V3, eGPIO_HIGH);
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* reset test state */
      doDiagVsup_RstTestState();
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END < u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}


/*------------------------------------------------------------------------------
**
** doDiagVsup_ExecTestStep2C1()
**
** Description:
**    executes the Voltage-Supervision-Test.
**    second part of VSUP-Test, on uC1, different steps, see [SRS_342]:
**    - DO_DIAG_VSUP_TEST_START
**       . nothing to do
**    - DO_DIAG_VSUP_TEST_DISCHARGE
**       . set all discharge pins to LOW
**    - DO_DIAG_VSUP_TEST_READVAL
**       . evaluate test results
**       . set discharge pins to HIGH
**    - DO_DIAG_VSUP_TEST_END
**       . reset of test states
**
**    If the test counter is larger than the last state DO_DIAG_VSUP_TEST_END,
**    the safety handler is called.
**    If the test counter is smaller than the last state and it is not equal
**    with another state there is no action.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_ExecTestStep2C1();
**
** Module Test:
**    - NO -
**    Reason: function calls and port settings
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_ExecTestStep2C1 (void)
{
   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.
    *
    * Note 948: Lint message deactivated, problem in the Macro RDS_GET. The
    * condition can be TRUE in case of an error.
    *
    * Deactivated Misra Rule 10.1, the macro DO_DIAG_VSUP_TEST_READVAL
    * contains another macro with a shift operation that PC-Lint identify as
    * complex integer expression.
    * */

   UINT8 u8DoNum;
   UINT32 u32TestCntTmp;

   /* get RDS variable to temporary variable */
   u32TestCntTmp = RDS_GET(doDiagVsup_sState.u32TestCnt);

   /* check/execute different steps of test execution */
   if (DO_DIAG_VSUP_TEST_START == u32TestCntTmp) /*lint !e835 */
   {
      /* nothing to do... */
   }
   else if (DO_DIAG_VSUP_TEST_DISCHARGE == u32TestCntTmp) /*lint !e835 */
   {  /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted), to discharge output */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_LOW);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_READVAL == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* call function to evaluate current state of TEST_DO pin */
      doDiagVsup_EvalTestResStep2();
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted) to disable discharging of other uC (needed to re-enable 
          * output) */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_HIGH);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* rest test state */
      doDiagVsup_RstTestState();
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END < u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}


/*------------------------------------------------------------------------------
**
** doDiagVsup_ExecTestStep1C2()
**
** Description:
**    executes the Voltage-Supervision-Test.
**    first part of VSUP-Test, on uC2, different steps, see [SRS_342]:
**    - DO_DIAG_VSUP_TEST_START
**       . set Life_C2 to LOW
**    - DO_DIAG_VSUP_TEST_DISCHARGE
**       . set all discharge pins to LOW
**    - DO_DIAG_VSUP_TEST_READVAL
**       . evaluate test results
**       . set discharge pins to HIGH
**    - DO_DIAG_VSUP_TEST_END
**       . reset of test states
**       . set Life_C2 to HIGH
**
**    If the test counter is larger than the last state DO_DIAG_VSUP_TEST_END,
**    the safety handler is called.
**    If the test counter is smaller than the last state and it is not equal
**    with another state there is no action.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_ExecVsupStep1C2();
**
** Module Test:
**    - NO -
**    Reason: function calls and port settings
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_ExecTestStep1C2 (void)
{
   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.
    *
    * Note 948: Lint message deactivated, problem in the Macro RDS_GET. The
    * condition can be TRUE in case of an error.
    *
    * Deactivated Misra Rule 10.1, the macro DO_DIAG_VSUP_TEST_READVAL
    * contains another macro with a shift operation that PC-Lint identify as
    * complex integer expression.
    * */

   UINT32 u32TestCntTmp;
   UINT8 u8DoNum;

   /* get RDS variable to temporary variable */
   u32TestCntTmp = RDS_GET(doDiagVsup_sState.u32TestCnt);

   /* check/execute different steps of test execution */
   if (DO_DIAG_VSUP_TEST_START == u32TestCntTmp) /*lint !e835 */
   {
      /* deactivate life signal Life_C2 */
      doSetGetPin_SetPin(PORT_LIFE_SIG, GPIO_PINMASK_8, eGPIO_LOW);
   }
   else if (DO_DIAG_VSUP_TEST_DISCHARGE == u32TestCntTmp) /*lint !e835 */
   {
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted), to discharge output */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_LOW);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_READVAL == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* call function to evaluate current state of TEST_DO pin */
      doDiagVsup_EvalTestResStep1();
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted) to disable discharging of other uC (needed to re-enable 
          * output) */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_HIGH);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* reset life signal, the life signal can be restarted 300us after reset of the TEST signals, 
       * but not earlier  */
      /* note: life signal is also handled by toggle function */
      doSetGetPin_SetPin(PORT_LIFE_SIG, PINMASK_LIFE_SIG, eGPIO_HIGH);

      /* reset test state */
      doDiagVsup_RstTestState();
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END < u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}

/*------------------------------------------------------------------------------
**
** doDiagVsup_ExecTestStep2C2()
**
** Description:
**    executes the Voltage-Supervision-Test.
**    second part of VSUP-Test, on uC2, different steps, see [SRS_342]:
**    - DO_DIAG_VSUP_TEST_START
**       . set Life_C2 to LOW
**       . set TEST_3V3 to HIGH
**    - DO_DIAG_VSUP_TEST_DISCHARGE
**       . set all discharge pins to LOW
**    - DO_DIAG_VSUP_TEST_READVAL
**       . evaluate test results
**       . set discharge pins to HIGH
**       . set TEST_3V3 to LOW
**    - DO_DIAG_VSUP_TEST_END
**       . reset of test states
**       . set Life_C2 to HIGH
**
**    If the test counter is larger than the last state DO_DIAG_VSUP_TEST_END,
**    the safety handler is called.
**    If the test counter is smaller than the last state and it is not equal
**    with another state there is no action.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_ExecTestStep2C2();
**
** Module Test:
**    - NO -
**    Reason: function calls and port settings
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_ExecTestStep2C2 (void)
{
   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.
    *
    * Note 948: Lint message deactivated, problem in the Macro RDS_GET. The
    * condition can be TRUE in case of an error.
    *
    * Deactivated Misra Rule 10.1, the macro DO_DIAG_VSUP_TEST_READVAL
    * contains another macro with a shift operation that PC-Lint identify as
    * complex integer expression.
    * */

   UINT8 u8DoNum;
   UINT32 u32TestCntTmp;

   /* get RDS variable to temporary variable */
   u32TestCntTmp = RDS_GET(doDiagVsup_sState.u32TestCnt);

   /* check/execute different steps of test execution */
   if (DO_DIAG_VSUP_TEST_START == u32TestCntTmp) /*lint !e835 */
   {
      /* deactivate life signal Life_C2 */
      doSetGetPin_SetPin(PORT_LIFE_SIG, PINMASK_LIFE_SIG, eGPIO_LOW);
      /* set Test_3v3 pin to high */
      doSetGetPin_SetPin(PORT_TEST_3V3, PINMASK_TEST_3V3, eGPIO_HIGH);
   }
   else if (DO_DIAG_VSUP_TEST_DISCHARGE == u32TestCntTmp) /*lint !e835 */
   {
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted), to discharge output */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_LOW);
      }
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_READVAL == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* call function to evaluate the results of the VSUP test step2 */
      doDiagVsup_EvalTestResStep2();
      /* for all digital outputs */
      for (u8DoNum = 0u; u8DoNum < GPIOCFG_NUM_DO_TOT; u8DoNum++)
      {
         /* set discharge Pin (inverted) to disable discharging of other uC (needed to re-enable 
          * output) */
         doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_HIGH);
      }
      /* reset default state of Test_3v3 pin */
      doSetGetPin_SetPin(PORT_TEST_3V3, PINMASK_TEST_3V3, eGPIO_LOW);
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END == u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* reset life signal, the life signal can be restarted 300us after reset of the TEST signals, 
       * but not earlier  */
      /* note: life signal is also handled by toggle function */
      doSetGetPin_SetPin(PORT_LIFE_SIG, PINMASK_LIFE_SIG, eGPIO_HIGH);
      doDiagVsup_RstTestState();
   }
   /*lint -esym(960, 10.1)*/
   else if (DO_DIAG_VSUP_TEST_END < u32TestCntTmp) /*lint !e835 */
   /*lint +esym(960, 10.1)*/
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}

/*------------------------------------------------------------------------------
**
** doDiagVsup_EvalTestResStep1()
**
** Description:
**    This function evaluates the results of the VSUP test step1 if at least
**    one DO is active. Both controller read their TEST_DO pin. If the signal
**    is LOW at one controller, the VSUP test is failed. Only if the signal is
**    HIGH at both controller the VSUP test is passed. The evaluation of the
**    TEST_DO pin is independent of the test execution step (step 1 or step 2).
**    But the error handler provides different functions for the test execution
**    steps.
**    If all DOs are inactive, the test results are not evaluated.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_EvalTestResStep1();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_EvalTestResStep1 (void)
{
   UINT8 u8DoNum;
   TRUE_FALSE_ENUM eBreak = eFALSE;

   /* check if there is a DO active */
   if (doDiagVsup_eDosActive == eTRUE)
   {
      /* for all digital outputs: check check value of all TESTDO */
      for (u8DoNum = 0u; (u8DoNum < GPIOCFG_NUM_DO_TOT) && (eBreak == eFALSE); u8DoNum++)
      {
         switch (DI_INPUTVAL_TESTDO(u8DoNum))
         {
            case eGPIO_LOW:
               /* error, VSUP did not switch off */
               doErrHdl_VsupTestStep1Failed();
               /* stop the check */
               eBreak = eTRUE;
               break;
            case eGPIO_HIGH:
               /* test passed for one TESTDO, VSUP switched off */
               if (u8DoNum == (GPIOCFG_NUM_DO_TOT - 1u))
               {
                  /* test passed, VSUP switched off */
                  doErrHdl_VsupTestStep1Ok();
               }
               break;
            default:
               /* invalid state/value of input variable */
               GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
               break;
         }
      }
   }
   else
   {
      /* check if the value of the variable is valid, if not go to the
       * safety handler */
      if (doDiagVsup_eDosActive != eFALSE)
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }
}

/*------------------------------------------------------------------------------
**
** doDiagVsup_EvalTestResStep2()
**
** Description:
**    This function evaluates the results of the VSUP test step2 if at least
**    one DO is active. Both controller read their TEST_DO pin. If the signal
**    is LOW at one controller, the VSUP test is failed. Only if the signal is
**    HIGH at both controller the VSUP test is passed. The evaluation of the
**    TEST_DO pin is independent of the test execution step (step 1 or step 2).
**    But the error handler provides different functions for the test execution
**    steps.
**    If all DOs are inactive, the test results are not evaluated.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_EvalTestResC2();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_EvalTestResStep2 (void)
{
   UINT8 u8DoNum;
   TRUE_FALSE_ENUM eBreak = eFALSE;

   if (doDiagVsup_eDosActive == eTRUE)
   {
      /* for all digital outputs: check check value of all TESTDO */
      for (u8DoNum = 0u; (u8DoNum < GPIOCFG_NUM_DO_TOT) && (eBreak == eFALSE); u8DoNum++)
      {
         switch (DI_INPUTVAL_TESTDO(u8DoNum))
         {
            case eGPIO_LOW:
               /* error, VSUP did not switch off */
               doErrHdl_VsupTestStep2Failed();
               /* stop the check */
               eBreak = eTRUE;
               break;
            case eGPIO_HIGH:
               /* test passed for one TESTDO, VSUP switched off */
               if (u8DoNum == (GPIOCFG_NUM_DO_TOT - 1u))
               {
                  /* test passed, VSUP switched off */
                  doErrHdl_VsupTestStep2Ok();
               }
               break;
            default:
               /* invalid state/value of input variable */
               GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
               break;
         }
      }
   }
   else
   {
      /* check if the value of the variable is valid, if not go to the
       * safety handler */
      if (doDiagVsup_eDosActive != eFALSE)
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }
}

/*------------------------------------------------------------------------------
**
** doDiagVsup_RstTestState()
**
** Description:
**    reset of test state and test counter
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagVsup_RstTestState();
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagVsup_RstTestState (void)
{
   doDiagVsup_sState.eTestState = eDO_DIAG_VSUP_WAIT;
   /* set default value for Voltage-Supply-test counter */
   RDS_SET(doDiagVsup_sState.u32TestCnt, DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
}


/*******************************************************************************
**
** End of doDiagVsup.c
**
********************************************************************************
*/

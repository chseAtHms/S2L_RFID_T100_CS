/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiagHs.c
**
** $Id: doDiagHs.c 2448 2017-03-27 13:45:16Z klan $
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
** Module handles the HS-Test states and execution of HS-Test of the
** safe output section.
**
** HS-Test: for active outputs, the test shall verify if the safe outputs still
**          can be switched off, HS is used as abbreviation for "High Side Switch".
**          Test is executed for each active output separately
**
** Principle of implementation:
** A counter is increased with every call of the module. If the counter reaches
** a certain value (define), then the diagnostic test has to be executed (execution
** mode of the module).
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
#include "gpio_cfg.h"
#include "doCfg.h"
#include "fiParam.h"
#include "diInput.h"
#include "doSetGetPin.h"
#include "diDoDiag.h"
#include "doDiag.h"
#include "doErrHdl.h"
#include "doDiagHs.h"
#include "doDiagHs_priv.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing test state and counters for the different DOs,
** Highside-Switch test
**------------------------------------------------------------------------------
*/
DIDODIAG_DO_DIAGSTATE_STRUCT doDiagHs_sState;


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
** doDiagHs_Init()
**
** Description:
**    initialization of the HS-Test module of digital safe outputs
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
**    doDiagHs_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiagHs_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      /* set default values for test-state */
      doDiagHs_RstTestState(u8Index);
   }
}


/*------------------------------------------------------------------------------
**
** doDiagHs_RstTestState()
**
** Description:
**    reset of HS-Test-Counter and HS-Test-State for a dedicated output
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output test to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_RstTestState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiagHs_RstTestState(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* set default values for test-state */
   doDiagHs_sState.eTestState[u8DoNum] = eDIAG_WAIT_1ST;
   /* set default values for test counter */
   RDS_SET(doDiagHs_sState.u32TestCnt[u8DoNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);

   doDiagHs_sState.eTestExecuted[u8DoNum] = eFALSE;
}


/*------------------------------------------------------------------------------
**
** doDiagHs_HandleTestState()
**
** Description:
**    State machine of the HS-Test.
**    Test counters are incremented with every function call, the calling
**    frequency of this function is the time base, because counters are
**    incremented with every function call (definitions for timing can be found
**    in "doDiagHs_priv.h").
**    Possible States:
**    eDIAG_WAIT_1ST: time for first test not yet reached, output in the wait/idle
**                    time before first test
**    eDIAG_REQ_1ST:  time for first test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_1ST: execution of first test
**    eDIAG_WAIT_CYC: time for cyclic test not yet reached, output in the wait/idle
**                    time before first test
**    eDIAG_REQ_CYC:  time for cyclic test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_CYC: execution of cyclic test
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doDiagHs_HandleTestState();
**
** Module Test:
**    - NO -
**    Reason: easy to understand, only function calls and counter increase
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiagHs_HandleTestState(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   switch(doDiagHs_sState.eTestState[u8DoNum])
   {
      case eDIAG_WAIT_1ST:
         /* increase/handle test(-state) counters in wait mode */
         RDS_INC(doDiagHs_sState.u32TestCnt[u8DoNum]);

         /* call function to check if 1st test after Pin state change has to be
          * executed,
          * Hint: first test has higher prio then cyclic test (order of calling
          *       would change prio)*/
         doDiagHs_Check1stTestStart(u8DoNum);
         break;

      case eDIAG_REQ_1ST:
         /* call function to check if 1st test after Pin state change has to be executed,
          * first test has higher prio then cyclic test (order of calling changes prio)*/
         doDiagHs_Check1stTestStart(u8DoNum);
         /* Hint: No increment of test counter in "request" states (test pending
          * for execution) */
         break;

      case eDIAG_WAIT_CYC:
         /* increase/handle test(-state) counters in wait mode, but not in requested mode */
         RDS_INC(doDiagHs_sState.u32TestCnt[u8DoNum]);

         /* call function to check if cyclic test of DI has to be executed,
          * Hint: cyclic test has lower prio then first test, if another pin is
          * already in test mode then current pin will not change to cyclic test
          * mode (order of calling changes prio) */
         doDiagHs_CheckCycTestStart(u8DoNum);
         break;

      case eDIAG_REQ_CYC:
         /* call function to check if cyclic test of DO has to be executed,
          * hint: cyclic test has lower prio then first test, if another pin is
          * already in test mode then current pin will not change to cyclic test
          * mode (order of calling changes prio) */
         doDiagHs_CheckCycTestStart(u8DoNum);
         /* Hint: No increment of test counter in "request" states (test pending
          *       for execution) */
         break;

      case eDIAG_EXEC_1ST:
         /* no break, fall through, because same test in both cases */
      case eDIAG_EXEC_CYC:
         /* increase/handle test(-state) counters */
         RDS_INC(doDiagHs_sState.u32TestCnt[u8DoNum]);

         /* call function to execute test */
         doDiagHs_ExecTest(u8DoNum);
         break;

      default:
         /* error: invalid state/value of variable */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;

   }
}


/*------------------------------------------------------------------------------
 **
 ** doDiagHs_WasTestExecuted()
 **
 ** Description:
 **    This function returns the value of the Test-Execute-Variable (which shall
 **    be set whenever the HS-Test has been executed) for a dedicated safe DO.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    TRUE_FALSE_ENUM: value of the Test-Execute-Variable
 **
 ** Usage:
 **    x = doDiagHs_WasTestExecuted()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM doDiagHs_WasTestExecuted (CONST UINT8 u8DoNum)
{
   return (doDiagHs_sState.eTestExecuted[u8DoNum]);
}


/*------------------------------------------------------------------------------
 **
 ** doDiagHs_RstTestExecutedVar()
 **
 ** Description:
 **    Resets the Test-Execute-Variable of a dedicated safe DO to "eFALSE"
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiagHs_RstTestExecutedVar(x)
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void doDiagHs_RstTestExecutedVar(CONST UINT8 u8DoNum)
{
   doDiagHs_sState.eTestExecuted[u8DoNum] = eFALSE;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doDiagHs_Check1stTestStart()
**
** Description:
**    Check if the first HS-Test has to be executed and assign
**    the test-state:
**    If the Test-Counter variable reaches a certain value, then the Test-State
**    is set to "execution of first test" or (if this is not possible) to
**    "request first test"
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_Check1stTestStart(x);
**
** Module Test:
**    - No -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_Check1stTestStart(CONST UINT8 u8DoNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   u32TmpTestCnt = (UINT32)(RDS_GET(doDiagHs_sState.u32TestCnt[u8DoNum])); /*lint !e948*/

   /* time for first test? */
   if (u32TmpTestCnt >= DO_DIAG_HS_1ST_TEST_WAITTIME)
   {
      /* and no other test currently ongoing? */
      if (FALSE == doDiag_IsAnyTestOngoing())
      {
         /* set state "execution of first test" */
         doDiagHs_sState.eTestState[u8DoNum] = eDIAG_EXEC_1ST;
         /* reset test counter: only reason is that then both 1st and cyclic test
          * execution starts from value 0 and can use the same timing values
          * during test execution */
         RDS_SET(doDiagHs_sState.u32TestCnt[u8DoNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      }
      else
      {
         /* execution mode currently not possible, because other test ongoing,
          * so set state "requesting first test" */
         doDiagHs_sState.eTestState [u8DoNum] = eDIAG_REQ_1ST;  /* assign test state */
      }
   }
}


/*------------------------------------------------------------------------------
**
** doDiagHs_CheckCycTestStart()
**
** Description:
**    Check if the cyclic HS-Test has to be executed and assign
**    the test-state:
**    If the Test-Counter variable reaches a certain value, then the Test-State
**    is set to "execution of cyclic test" or (if this is not possible) to
**    "request cyclic test"
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_CheckCycTestStart(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_CheckCycTestStart(CONST UINT8 u8DoNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   u32TmpTestCnt = (UINT32)(RDS_GET(doDiagHs_sState.u32TestCnt[u8DoNum])); /*lint !e948*/

   /* time for cyclic test? */
   if (u32TmpTestCnt >= DO_DIAG_HS_CYC_TEST_WAITTIME)
   {
      /* and no other test currently ongoing? */
      if (FALSE == doDiag_IsAnyTestOngoing())
      {
         /* set state "execution of first test" */
         doDiagHs_sState.eTestState[u8DoNum] = eDIAG_EXEC_CYC;
         /* reset test counter: only reason is that then both 1st and cyclic test
          * execution starts from value 0 and can use the same timing values during
          * test execution */
         RDS_SET(doDiagHs_sState.u32TestCnt[u8DoNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      }
      else
      {
         /* execution mode currently not possible, because other test ongoing,
          * so set state "requesting cyclic test" */
         doDiagHs_sState.eTestState [u8DoNum] = eDIAG_REQ_CYC;  /* assign test state */
      }
   }
 }


/*------------------------------------------------------------------------------
**
** doDiagHs_ExecTest()
**
** Description:
**    executes the HS-Test of a dedicated output.
**    One test is always done for one channel/one HW-Output, the two channels of
**    one dual channel are tested one after the other.
**    Depending on output number doNum the output of uC1 or uC2 is tested,
**    principle:
**    -------------------------------------------------------------------
**    |           test a          |  |           test b          |  ...
**    -------------------------------------------------------------------
**    |         dualChDI1         |  |         dualChDI2         |  ...
**    -------------------------------------------------------------------
**    | singleChDI1 | singleChDI2 |  | singleChDI1 | singleChDI2 |  ...
**    -------------------------------------------------------------------
**    |     uC1     |     uC2     |  |     uC1     |     uC2     |  ...
**    -------------------------------------------------------------------
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to test
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_ExecTest(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_ExecTest(CONST UINT8 u8DoNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* is diNum equal number, then its an input located at uC1 */
         if (DIDODIAG_IS_VAL_EQUAL(u8DoNum))
         {
            doDiagHs_ExecTestAtOwnCh(u8DoNum);
         }
         else
         {
            doDiagHs_ExecTestAtOtherCh(u8DoNum);
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* is diNum equal number, then its an input located at uC1 */
         if (DIDODIAG_IS_VAL_EQUAL(u8DoNum))
         {
            doDiagHs_ExecTestAtOtherCh(u8DoNum);
         }
         else
         {
            doDiagHs_ExecTestAtOwnCh(u8DoNum);
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
         /* no break, fall through, in case of invalid value of controller ID */
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** doDiagHs_ExecTestAtOwnCh()
**
** Description:
**    executes the actions for HS-Test of a dedicated output
**    located at the own channel (see [SRS_211]):
**    - deactivation of safe output pin
**    - evaluation (read-back) of output state
**    - reset test variables
**    Hint: Output will not be re-activated here, this is done in higher-level
**    part of the SW, reason is the wait time caused by using the discharge pin.
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_ExecTestAtOwnCh(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_ExecTestAtOwnCh (CONST UINT8 u8DoNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   u32TmpTestCnt = (UINT32)(RDS_GET(doDiagHs_sState.u32TestCnt[u8DoNum])); /*lint !e948*/

   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.
    * Deactivated Misra Rule 10.1, the macro DO_DIAG_HS_READVAL
    * contains a shift operation that PC-Lint identify as complex integer
    * expression.
    * */
   /*lint -esym(960, 10.1)*/

   /* actions to be taken during test at own channel */
   if (DO_DIAG_HS_START == u32TmpTestCnt)/*lint !e835*/
   {
      /* first action: switch output OFF (Highside-Switch) */
      doSetGetPin_SetDoPin(u8DoNum, eGPIO_LOW);
   }
   else if ((DO_DIAG_HS_READVAL(u8DoNum)) == u32TmpTestCnt)/*lint !e835*/
   {
      /* call function to evaluate current state of outputs */
      doDiagHs_EvalTestRes(u8DoNum);

      /* set default value for test counter */
      RDS_SET(doDiagHs_sState.u32TestCnt[u8DoNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);

      /* set default value for test state */
      doDiagHs_sState.eTestState[u8DoNum] = eDIAG_WAIT_CYC;

      /* set identifier that test was executed */
      doDiagHs_sState.eTestExecuted[u8DoNum] = eTRUE;
   }
   else if ((DO_DIAG_HS_READVAL(u8DoNum)) < u32TmpTestCnt)/*lint !e835*/
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
   /*lint +esym(960, 10.1)*/
}


/*------------------------------------------------------------------------------
**
** doDiagHs_ExecTestAtOtherCh()
**
** Description:
**    executes the actions for HS-Test of a dedicated output
**    located at the other channel (see [SRS_211]):
**    - set discharge pin
**    - evaluate/read output states
**    - reset test variables
**
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to test
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_ExecTestAtOtherCh(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_ExecTestAtOtherCh (CONST UINT8 u8DoNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   u32TmpTestCnt = (UINT32)(RDS_GET(doDiagHs_sState.u32TestCnt[u8DoNum])); /*lint !e948*/

   /* Info 835: A zero has been given as left argument to operator '+'
    * --> Reset value of the counter variable is 0, this is defined in the
    * constant DIDODIAG_DIDO_TEST_CNT_RESET_VAL which is used in the following
    * macros.    * Deactivated Misra Rule 10.1, the macro DO_DIAG_HS_READVAL
    * contains a shift operation that PC-Lint identify as complex integer
    * expression.
    * */
   /*lint -esym(960, 10.1)*/

   /* actions to be taken during test at other channel */
   if (DO_DIAG_HS_DISCHARGE == u32TmpTestCnt)/*lint !e835*/
   {
      /* set discharge Pin (inverted), to discharge output of other uC */
      doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_LOW);
   }
   else if ((DO_DIAG_HS_READVAL(u8DoNum)) == u32TmpTestCnt)/*lint !e835*/
   {
      /* call function to evaluate current state of outputs */
      doDiagHs_EvalTestRes(u8DoNum);

      /* set discharge Pin (inverted) to disable discharging of other uC (needed to re-enable 
       * output) */
      doSetGetPin_SetDisChrgPin(u8DoNum, eGPIO_HIGH);

      /* set default value for test counter */
      RDS_SET(doDiagHs_sState.u32TestCnt[u8DoNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);

      /* set default value for test state */
      doDiagHs_sState.eTestState[u8DoNum] = eDIAG_WAIT_CYC;

      /* set identifier that test was executed */
      doDiagHs_sState.eTestExecuted[u8DoNum] = eTRUE;
   }
   else if ((DO_DIAG_HS_READVAL(u8DoNum)) < u32TmpTestCnt)/*lint !e835*/
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
   /*lint +esym(960, 10.1)*/
}


/*------------------------------------------------------------------------------
**
** doDiagHs_EvalTestRes()
**
** Description:
**    evaluates the results of the HS-Test.
**    - test pulse shall be received at the channel where the test is ongoing
**    - test pulse shall not be received at other channel
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiagHs_EvalTestRes(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doDiagHs_EvalTestRes(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* check value of TESTDO ch1 */

   /* time to read out values reached */
   if (eGPIO_HIGH == DI_INPUTVAL_TESTDO(u8DoNum))
   {
      /* inform do error handler that HS test OK */
      doErrHdl_HsTestOk(u8DoNum);
   }
   else if (eGPIO_LOW == DI_INPUTVAL_TESTDO(u8DoNum))
   {
      /* error: output did not switch off (might be in HIGH state or invalid state, means neither 
       * HIGH or LOW)! */
      /* inform do error handler that HS test failed */
      doErrHdl_HsTestFailed(u8DoNum);
   }
   else
   {
      /* invalid state/value of input variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*******************************************************************************
**
** End of doDiagHs.c
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagLev.c
**
** $Id: diDiagLev.c 2448 2017-03-27 13:45:16Z klan $
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
** Module handles the Level-Test states of the digital "safe" inputs.
** Level-Test is trying to force an "active" input to "not active" state.
** If this is not possible, an error on input (short) has to be assumed.
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
#include "xtypes.h"
#include "xdefs.h"


#include "cfg_Config-sys.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

/* Module header */
#include "gpio_cfg.h"
#include "diCfg.h"
#include "diSetGetPin.h"
#include "diInput.h"
#include "diDoDiag.h"
#include "diDiag.h"
#include "diErrHdl.h"
#include "diDiagShortCheck.h"
#include "diDiagLev.h"
#include "diDiagLev_priv.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing level-test state and counters for the different DIs
** global used due to performance reasons
**-----------------------------------------------------------------------------*/
DIDODIAG_DI_DIAGSTATE_STRUCT diDiagLev_sState;

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
** diDiagLev_Init()
**
** Description:
**    initialization of the Level-Test module for digital safe inputs
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
**    diDiagLev_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagLev_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* set default values for test-state */
      diDiagLev_RstTestState(u8Index);
      /* set Identifier/Variable that test function was not executed */
      diDiagLev_sState.eTestExecuted[u8Index] = eFALSE;

      /* reset TI_LEVEL Pin, can be used within this loop, only disadvantage is
       * that every TI-Level pin is reseted two times */
      diSetGet_SetTiLevelPin(u8Index, DICFG_TILEVEL_INACTIVE);
   }
}


/*------------------------------------------------------------------------------
**
** diDiagLev_RstTestState()
**
** Description:
**    reset of Level-Test-Counter and Level-Test-State for a dedicated input
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input state to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_RstTestState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagLev_RstTestState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* set default values for test-state */
   diDiagLev_sState.eTestState[u8DiNum] = eDIAG_WAIT_1ST;
   /* set default values for test counter */
   RDS_SET(diDiagLev_sState.u32TestCnt[u8DiNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
}


/*------------------------------------------------------------------------------
**
** diDiagLev_HandleTestState()
**
** Description:
**    State machine of the Level-Test.
**    Test counters are incremented with every function call.
**    => Calling frequency of this function is the time base for time-defines
**       which can be found in "diDiagLev_priv.h"
**    Possible States:
**    eDIAG_WAIT_1ST: time for first test not yet reached, input in the wait/idle
**                    time before first test
**    eDIAG_REQ_1ST:  time for first test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_1ST: execution of first test
**    eDIAG_WAIT_CYC: time for cyclic test not yet reached, input in the wait/idle
**                    time before first test
**    eDIAG_REQ_CYC:  time for cyclic test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_CYC: execution of cyclic test
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diDiagLev_HandleTestState(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand, only function calls and counter increase
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagLev_HandleTestState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* execute actions depending on current Level-Test-State */
   switch(diDiagLev_sState.eTestState[u8DiNum])
   {
      case eDIAG_WAIT_1ST:
         /* reset qualifier, to indicate that the first test is still to be executed */
         DI_ERR_SET_TEST_QUAL(u8DiNum, eGPIO_DIAG_NA);

         /* call function to check if 1st test after DI state change has to be executed,
          * Hint: first test has higher prio then cyclic test (order of calling
          * would change prio)*/
         diDiagLev_Check1stTestStart(u8DiNum);

         /* is it time to execute the first test (state changed by function
          * "diDiagLev_Check1stTestStart")? */
         if (eDIAG_EXEC_1ST == diDiagLev_sState.eTestState[u8DiNum])
         {
            diDiagLev_ExecTest(u8DiNum); /* for faster reaction time call execution
             of test already here (not waiting until the next call of state handler) */
         }

         /* increment test counter (count time to next test) */
         RDS_INC(diDiagLev_sState.u32TestCnt[u8DiNum]);
         break;

      case eDIAG_REQ_1ST:
         /* call function to check if 1st test after DI state change has to be executed,
          * first test has higher prio then cyclic test (order of calling changes prio)*/
         diDiagLev_Check1stTestStart(u8DiNum);
         /* Hint: No increment of test counter in "request" states (test pending
          *       for execution) */
         break;

      case eDIAG_WAIT_CYC:
         /* call function to check if cyclic test of DI has to be executed,
          * hint: cyclic test has lower prio then first test, if another pin is
          * already in test mode then current pin will not change to cyclic test
          * mode (order of calling changes prio) */
         diDiagLev_CheckCycTestStart(u8DiNum);

         /* is it time to execute the cyclic test (state changed by function
          * "diDiagLev_CheckCycTestStart")? */
         if (eDIAG_EXEC_CYC == diDiagLev_sState.eTestState[u8DiNum])
         {
            diDiagLev_ExecTest(u8DiNum);  /* for faster reaction time call
            execution of test already here (not waiting until the next call of state handler) */
         }

         /* increment test counter */
         RDS_INC(diDiagLev_sState.u32TestCnt[u8DiNum]);
         break;

      case eDIAG_REQ_CYC:
         /* call function to check if pending cyclic test may be executed now
          * (will be the case if no other test with higher prio pending) */
         diDiagLev_CheckCycTestStart(u8DiNum);
         /* Hint: No increment of test counter in "request" states (test pending
          *       for execution) */
         break;

      case eDIAG_EXEC_1ST:
         /* no break, fall through, because same test in both cases */
      case eDIAG_EXEC_CYC:
         /* call function to execute level test, see [SRS_108] */
         diDiagLev_ExecTest(u8DiNum);
         /* increment test counter */
         RDS_INC(diDiagLev_sState.u32TestCnt[u8DiNum]);
         break;

      default:
         /* error: invalid state/value of variable */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;

   }
}


/*------------------------------------------------------------------------------
 **
 ** diDiagLev_WasTestExecuted()
 **
 ** Description:
 **    This function returns the value of the Test-Execute-Variable (which shall
 **    be set whenever the Level-Test has been executed) for a dedicated safe DI.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    TRUE_FALSE_ENUM: value of the Test-Execute-Variable
 **
 ** Usage:
 **    x = diDiagLev_WasTestExecuted()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM diDiagLev_WasTestExecuted (CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   return (diDiagLev_sState.eTestExecuted[u8DiNum]);
}


/*------------------------------------------------------------------------------
 **
 ** diDiagLev_RstTestExecutedVar()
 **
 ** Description:
 **    Resets the Test-Execute-Variable of a dedicated safe DI to "eFALSE"
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    diDiagLev_RstTestExecutedVar(x)
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void diDiagLev_RstTestExecutedVar(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   diDiagLev_sState.eTestExecuted[u8DiNum] = eFALSE;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiagLev_Check1stTestStart()
**
** Description:
**    Check if the first Level-Test has to be executed and assign
**    the test-state:
**    If the Test-Counter variable reaches a certain value (defined by
**    DI_DIAG_LEV_1ST_LEVTEST_WAITTIME), then the Test-State
**    is set to "execution of first test" or (if this is not possible) to
**    "request first test"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_Check1stTestStart(x);
**
** Module Test:
**    - NO -
**    Reason: Low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_Check1stTestStart(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */
   /* time for first test? */
   if (RDS_GET(diDiagLev_sState.u32TestCnt[u8DiNum]) 
          >= DI_DIAG_LEV_1ST_LEVTEST_WAITTIME)/*lint !e948*/
   {
      /* and no other test currently ongoing? */
      if (FALSE == diDiag_IsAnyTestOngoing())
      {
         /* set state "execution of first test" */
         diDiagLev_sState.eTestState[u8DiNum] = eDIAG_EXEC_1ST;
         /* reset test counter: only reason is that then both 1st and cyclic test
          * execution starts from start value "0" and can use the same timing values
          * during test execution. */
         RDS_SET(diDiagLev_sState.u32TestCnt[u8DiNum], 0u);
      }
      else
      {
         /* execution mode currently not possible, because other test ongoing,
          * so set state "requesting first test" */
         diDiagLev_sState.eTestState [u8DiNum] = eDIAG_REQ_1ST;  /* assign test state */
      }
   }

}


/*------------------------------------------------------------------------------
**
** diDiagLev_CheckCycTestStart()
**
** Description:
**    Check if the cyclic Level-Test has to be executed and assign
**    the test-state:
**    If the Test-Counter variable reaches a certain value (defined by
**    DI_DIAG_LEV_CYC_LEVTEST_WAITTIME), then the Test-State
**    is set to "execution of cyclic test" or (if this is not possible) to
**    "request cyclic test"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_CheckCycTestStart(x);
**
** Module Test:
**    - YES -
**    Reason:
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_CheckCycTestStart(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */
   /* time for cyclic test? */
   if ( (RDS_GET(diDiagLev_sState.u32TestCnt[u8DiNum])) 
            >= DI_DIAG_LEV_CYC_LEVTEST_WAITTIME)/*lint !e948*/
   {
      /* and no other test currently ongoing? */
      if (FALSE == diDiag_IsAnyTestOngoing())
      {
         /* set state "execution of cyclic test" */
         diDiagLev_sState.eTestState[u8DiNum] = eDIAG_EXEC_CYC;
         /* reset test counter: only reason is that then both 1st and cyclic test
          * execution starts from start value "0" and can use the same timing values
          * during test execution. */
         RDS_SET(diDiagLev_sState.u32TestCnt[u8DiNum], 0u);
      }
      else
      {
         /* execution mode currently not possible, because other test ongoing,
          * so set state "requesting cyclic test" */
         diDiagLev_sState.eTestState [u8DiNum] = eDIAG_REQ_CYC;  /* assign test state */
      }
   }
}


/*------------------------------------------------------------------------------
**
** diDiagLev_ExecTest()
**
** Description:
**    executes the Level-Test of a dedicated input.
**    One test is always done for one channel/one HW-Input, the two channels of
**    one dual channel are tested one after the other.
**    Depending on current u8DiNum the input of uC1 or uC2 is tested, principle:
**    -------------------------------------------------------------------
**    |           test a          |  |           test b          |  ...
**    -------------------------------------------------------------------
**    |         dualChDI1         |  |         dualChDI2         |  ...
**    -------------------------------------------------------------------
**    |     DI0     |     DI1     |  |     DI2     |     DI3     |  ...
**    -------------------------------------------------------------------
**    |     uC1     |     uC2     |  |     uC1     |     uC2     |  ...
**    -------------------------------------------------------------------
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_ExecTest(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_ExecTest(CONST UINT8 u8DiNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* set Identifier/Variable that test function was executed */
   diDiagLev_sState.eTestExecuted[u8DiNum] = eTRUE;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* is u8DiNum equal number, then its an input located at uC1
          * (DI0, DI2, DI4...) */
         if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
         {
            diDiagLev_ExecTestAtOwnCh(u8DiNum);
         }
         /* is u8DiNum not equal number, then its an input located at uC2
          * (DI1, DI3, DI5...) */
         else
         {
            diDiagLev_ExecTestAtOtherCh(u8DiNum);
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* is u8DiNum equal number, then its an input located at uC1
          * (DI0, DI2, DI4...) */
         if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
         {
            diDiagLev_ExecTestAtOtherCh(u8DiNum);
         }
         /* is u8DiNum not equal number, then its an input located at uC2
          * (DI1, DI3, DI5...) */
         else
         {
            diDiagLev_ExecTestAtOwnCh(u8DiNum);
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
** diDiagLev_ExecTestAtOwnCh()
**
** Description:
**    executes the actions for Level-Test of a dedicated input located at the
**    own channel, which are in the different steps (see [SRS_53]):
**    - get values of all pins of own channel for later "short check"
**    - evaluation of test pulse (which has to be set by the other uC)
**    - check if there is a possible short between the safe inputs of "own" uC
**    - reset of test variables
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_ExecTestAtOwnCh(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_ExecTestAtOwnCh(CONST UINT8 u8DiNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* read test-counter value to tmp-variable */
   u32TmpTestCnt = (RDS_GET(diDiagLev_sState.u32TestCnt[u8DiNum]));/*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* In the following if-else construct the PC Lint message 835 is deactivated.
    * The current configuration of the constant DI_DIAG_LEV_SET_PULSE is 0.
    * This constant is a component of various macros.
    */

   /* actions to be taken during test at own channel */
   if (DI_DIAG_LEV_SET_PULSE == u32TmpTestCnt )
   {
      /* call function to get values of safe DIs at start of test, see [SRS_274] */
      diDiagShortCheck_Start();
   }
   else if (DI_DIAG_LEV_READBACK == u32TmpTestCnt)/*lint !e835*/
   {
      /* call function to get values of safe DIs during test, see [SRS_274] */
      diDiagShortCheck_Exec();
      /* call function to check if the test pulse set by other uC was received */
      diDiagLev_EvalTestResultOwnCh(u8DiNum);
   }
   else if (DI_DIAG_LEVEL_END == u32TmpTestCnt)/*lint !e835*/
   {
      /* call function to check if there is a potential short, see [SRS_274] */
      diDiagShortCheck_End(u8DiNum);

      /* set default values for test counter */
      RDS_SET(diDiagLev_sState.u32TestCnt[u8DiNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      /* set test state back to default */
      diDiagLev_sState.eTestState[u8DiNum] = eDIAG_WAIT_CYC;
   }
   else if (DI_DIAG_LEVEL_END < u32TmpTestCnt)/*lint !e835*/
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}


/*------------------------------------------------------------------------------
**
** diDiagLev_ExecTestAtOtherCh()
**
** Description:
**    executes the actions for Level-Test of a dedicated input located at the
**    other channel, which are in the different steps (see [SRS_53]):
**    - set the test pulse (which shall be received by the other uC)
**    - reset the test pulse
**    - reset of test variables
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_ExecTestAtOtherCh(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_ExecTestAtOtherCh(CONST UINT8 u8DiNum)
{
   UINT32 u32TmpTestCnt;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   u32TmpTestCnt = (RDS_GET(diDiagLev_sState.u32TestCnt[u8DiNum]));/*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* In the following if-else construct the PC Lint message 835 is deactivated.
    * The current configuration of the constant DI_DIAG_LEV_SET_PULSE is 0.
    * This constant is a component of various macros.
    */

   /* actions to be taken during test at other channel */
   if (DI_DIAG_LEV_SET_PULSE == u32TmpTestCnt)
   {
      /* set TI_LEVEL Pin for test at other channel */
      diSetGet_SetTiLevelPin(u8DiNum, DICFG_TILEVEL_ACTIVE);
   }
   else if (DI_DIAG_LEV_READBACK == u32TmpTestCnt)/*lint !e835*/
   {
      /* reset TI_LEVEL Pin */
      diSetGet_SetTiLevelPin(u8DiNum, DICFG_TILEVEL_INACTIVE);
   }
   else if (DI_DIAG_LEVEL_END == u32TmpTestCnt)/*lint !e835*/
   {
      /* set default values for test counter */
      RDS_SET(diDiagLev_sState.u32TestCnt[u8DiNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      /* set test state back to default */
      diDiagLev_sState.eTestState[u8DiNum] = eDIAG_WAIT_CYC;
   }
   else if (DI_DIAG_LEVEL_END < u32TmpTestCnt)/*lint !e835*/
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
}


/*------------------------------------------------------------------------------
**
** diDiagLev_EvalTestResultOwnCh()
**
** Description:
**    evaluates the results of the Level test at the uC where the test is ongoing
**    => test pulse shall be received here
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagLev_EvalTestResultOwnCh(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagLev_EvalTestResultOwnCh(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* get input value and evaluate result */
   if (DICFG_DI_INACTIVE == DIINPUT_DI_OWN_FIRST_FILTER(DICFG_GET_DUAL_CH_NUM(u8DiNum)))
   {
      /* test pulse received, inform error handler that TO test OK */
      diErrHdl_LevTestOk(u8DiNum);
   }
   else if(DICFG_DI_ACTIVE == DIINPUT_DI_OWN_FIRST_FILTER(DICFG_GET_DUAL_CH_NUM(u8DiNum)))
   {
      /* error: if DI still active, test pulse was not received */

      /* inform di error handler that level test failed, channel passivation */
      diErrHdl_LevTestFailed(u8DiNum);
   }
   else
   {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}


/*******************************************************************************
**
** End of diDiagLev.c
**
********************************************************************************
*/

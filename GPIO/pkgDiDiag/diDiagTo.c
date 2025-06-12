/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagTo.c
**
** $Id: diDiagTo.c 2618 2017-04-06 11:58:52Z klan $
** $Revision: 2618 $
** $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
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
** Module handles the TO-test states of the digital "safe" inputs.
** TO test is sending a pulse which shall be received back at an input.
** If this pulse is not received, an error on input has to be assumed.
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
#include "fiParam.h"
#include "diSetGetPin.h"
#include "diInput.h"
#include "diDoDiag.h"
#include "diDiag.h"
#include "diErrHdl.h"
#include "diDiagTo.h"
#include "diDiagTo_priv.h"


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
** structure containing TO-test state and counters for the different DIs
**------------------------------------------------------------------------------
*/
STATIC DIDIAGTO_DI_DIAGSTATE_STRUCT diDiagTo_sState;

/*------------------------------------------------------------------------------
** state of TO Test for the different channels
**------------------------------------------------------------------------------
*/
STATIC DI_DIAG_TO_STATE_STRUCT diDiagTo_sChState;      /* state of tests */

/*------------------------------------------------------------------------------
** This variable is used to indicate that the test counter can be incremented.
**------------------------------------------------------------------------------
*/
STATIC TRUE_FALSE_ENUM diDiagTo_u8IncreaseCnt[GPIOCFG_NUM_CHANNELS] = {eFALSE, eFALSE};

/*******************************************************************************
**
** Public Services
**
********************************************************************************
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiagTo_Init()
**
** Description:
**    initialization of the TO-test module of digital inputs
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
**    diDiagTo_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagTo_Init(void)
{
   UINT8 u8Index;

   /* for all digital inputs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* set default values for test-state */
      diDiagTo_RstTestState(u8Index);
            /* set Identifier/Variable that test function was not executed */
      diDiagTo_sState.eTestExecuted[u8Index] = eFALSE;
      /* set default values at TOs */
      diDiagTo_SetTOsDefault(u8Index);
   }

   /* for all channels/uCs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_CHANNELS; u8Index++)
   {
      /* set default values for test-state */
      RDS_SET(diDiagTo_sChState.u32TestCnt[u8Index], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);
      diDiagTo_sChState.eTestExecutionActive[u8Index] = eFALSE;
   }
}


/*------------------------------------------------------------------------------
**
** diDiagTo_HandleTestState()
**
** Description:
**    Calls handling of the TO-Test states.
**    For the different DIs the execution of a TO test can be requested by this
**    function. Depending on the last TO-Test-State of a DI this might be a
**    request for the first or for cyclic TO-Tests.
**    Possible States:
**    eDIAG_WAIT_1ST: time for first test not yet reached, input in the wait/idle
**                    time before first test
**    eDIAG_REQ_1ST:  time for first test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_1ST: execution of first test => not used in this module
**    eDIAG_WAIT_CYC: time for cyclic test not yet reached, input in the wait/idle
**                    time before first test
**    eDIAG_REQ_CYC:  time for cyclic test reached, but test execution could not
**                    directly be entered => set request for first test
**    eDIAG_EXEC_CYC: execution of cyclic test
**
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
**    diDiagTo_HandleTestState(1);
**
** Module Test:
**    - NO -
**    Reason: easy to understand, only function calls and counter increase
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagTo_HandleTestState(CONST UINT8 u8DiNum)
{
   UINT8 u8ChNo;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* if configuration for TO is set to "TO always HIGH", see [SRS_277] */
   if (FIPARAM_DI_TO_TEST_PARAM == (UINT8)FIPARAM_E_DICFG_TO_ON)
   {
      /* set TO Pin to active */
      diSetGet_SetToEnPin(DICFG_TO_ACTIVE);
   }
   /* if configuration for TO is set to "TO always LOW", see [SRS_277] */
   else if (FIPARAM_DI_TO_TEST_PARAM == (UINT8)FIPARAM_E_DICFG_TO_OFF)
   {
      /* set TO Pin to inactive*/
      diSetGet_SetToEnPin(DICFG_TO_INACTIVE);
   }
   else
   {
      /* get channel number (0 or 1) */
      u8ChNo = u8DiNum % GPIOCFG_NUM_CHANNELS;

      switch(diDiagTo_sState.eTestState[u8DiNum])
      {
         case eDIAG_WAIT_1ST:
            /* set Timer to start value. This is done to signal the "handle execute
             * function that TO Test should be started */
            RDS_SET(diDiagTo_sChState.u32TestCnt[u8ChNo], DI_DIAG_TO_START_TEST);
            break;

         case eDIAG_WAIT_CYC:
            /* in this state, set the variable in a way that the test counter in
             * function "diDiagTo_HandleTestExe, cution" can be increased for
             * cyclic testing */
            diDiagTo_u8IncreaseCnt[u8ChNo] = eTRUE;
            break;

         case eDIAG_EXEC_CYC:
            /* state pre-implemented because in future there might
             * be one TO for every DI, so TO-Test may be executed independently
             * on each DI-C in future. Currently, there is nothing to do in these
             * states */
            break;

         /* following states are general states used for Diagnostic modules,
          * but they are not used for TO-Test and therefore invalid values
          * (defensive programming) */
         case eDIAG_REQ_1ST:
         case eDIAG_EXEC_1ST:  /* fall through */
         case eDIAG_REQ_CYC:   /* fall through */
            /* no break, fall through, eDIAG_EXEC_1ST is currently not used in
             * TO test */
         default:
            /* error: invalid state/value of variable */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;

      }
   }
}


/*------------------------------------------------------------------------------
**
** diDiagTo_HandleTestExecution()
**
** Description:
**    calls the execution of the TO-Test function under certain conditions for
**    the different channels (2 uCs => 2 channels):
**    - if any DI is requesting a 1st or cyclic TO Test
**    - if the cyclic test counter reaches the time for TO test repetition
**    - if currently no other test ongoing
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
**    diDiagTo_HandleTestExecution();
**
** Module Test:
**    - YES -
**    Reason: -
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagTo_HandleTestExecution(void)
{
   UINT8 u8Index;

   /* for all channels/uCs: check if TO-Test has to be called */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_CHANNELS; u8Index++)
   {
      /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
      /*lint -e948 -esym(960, 12.4)*/
      if ( (eTRUE == diDiagTo_u8IncreaseCnt[u8Index]) && \
           (RDS_GET(diDiagTo_sChState.u32TestCnt[u8Index]) < DI_DIAG_TO_START_TEST) )
      {
         /* increase test counter */
         RDS_INC(diDiagTo_sChState.u32TestCnt[u8Index]);
      }
      /*lint +e948 +esym(960, 12.4)*/

      /* is any other test currently ongoing? */
      if (FALSE == diDiag_IsAnyTestOngoing())
      {
         /* time to test? */
         /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in 
          * RDS_GET. */
         if (RDS_GET(diDiagTo_sChState.u32TestCnt[u8Index]) >= DI_DIAG_TO_START_TEST)/*lint !e948*/
         {
            /* set test request */
            diDiagTo_sChState.eTestExecutionActive[u8Index] = eTRUE;
            /* Hint: From now on a test is ongoing!  */
         }
      }

      /* TO-Test to be executed? */
      if (eTRUE == diDiagTo_sChState.eTestExecutionActive[u8Index])
      {
         /* call function to execute test, see [SRS_236] */
         diDiagTo_ExecTest(u8Index);
         /* increase test counter for next test step */
         RDS_INC(diDiagTo_sChState.u32TestCnt[u8Index]);
      }
   }
}


/*------------------------------------------------------------------------------
**
** diDiagTo_RstTestState()
**
** Description:
**    reset of TO-Test-State for a dedicated input
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
**    diDiagTo_RstTestState();
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagTo_RstTestState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* set default values for test output test-state */
   diDiagTo_sState.eTestState[u8DiNum] = eDIAG_WAIT_1ST;
}


/*------------------------------------------------------------------------------
 **
 ** diDiagTo_TestExecutionCalled()
 **
 ** Description:
 **    This function sets Test-Execute-Variable
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DiNum: number/id of input
 **
 ** Return:
 **    - None -
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: IRQ Scheduler
 **-----------------------------------------------------------------------------
 */
void diDiagTo_TestExecutionCalled (CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* set Identifier/Variable that test function was called/executed */
   diDiagTo_sState.eTestExecuted[u8DiNum] = eTRUE;
}


/*------------------------------------------------------------------------------
 **
 ** diDiagTo_WasTestExecuted()
 **
 ** Description:
 **    This function returns the value of the Test-Execute-Variable (which shall
 **    be set whenever the TO-Test has been executed) for a dedicated safe DI.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    TRUE_FALSE_ENUM: value of the Test-Execute-Variable
 **
 ** Usage:
 **    x = diDiagTo_WasTestExecuted()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM diDiagTo_WasTestExecuted (CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);
   /* return Identifier/Variable indicating if test function was called/executed */
   return (diDiagTo_sState.eTestExecuted[u8DiNum]);
}


/*------------------------------------------------------------------------------
 **
 ** diDiagTo_RstTestExecutedVar()
 **
 ** Description:
 **    Resets the Test-Execute-Variable of a dedicated safe DI to "eFALSE"
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **      u8DiNum: number/id of input to reset
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    diDiagTo_RstTestExecutedVar(x)
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void diDiagTo_RstTestExecutedVar(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);
   /* reset Identifier/Variable that test function was called/executed */
   diDiagTo_sState.eTestExecuted[u8DiNum] = eFALSE;
}


/*------------------------------------------------------------------------------
**
** diDiagTo_IsToTestOngoing()
**
** Description:
**    returns if at a dedicated pin currently a TO-Test is ongoing
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to check
**
** Return:
**    eTRUE if test is currently ongoing, else eFALSE
**
** Outputs:
**    - None -
**
** Usage:
**    y = diDiagTo_IsToTestOngoing(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM diDiagTo_IsToTestOngoing(CONST UINT8 u8DiNum)
{
   UINT8 u8ChNo;
   TRUE_FALSE_ENUM retVal = eFALSE;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* calculate channel (0 or 1) */
   u8ChNo = u8DiNum % GPIOCFG_NUM_CHANNELS;

   /* is input of Type DI-C? Else no TO-Tests at all */
   if (FIPARAM_DI_INPUTTYPE_CONTACT(u8DiNum))
   {
      /* is at the channel the TO-Test ongoing? Then at all DI-Cs located at
       * this channel the TO-Test is ongoing */
      if (eTRUE == diDiagTo_sChState.eTestExecutionActive[u8ChNo])
      {
         retVal = eTRUE;
      }
   }

   return retVal;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiagTo_SetTOsDefault()
**
** Description:
**    function to set the TO pin to its default value (e.g. after reception of
**    parameters). If an input pin is configured as DI-C (digital
**    contact input), and at the same time not explicitly turned off, then set
**    the TO Pin "active" (LOW-Active)
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to check
**
** Outputs:
**    - void -
**
** Usage:
**    diDiagTo_SetTOsDefault(x)
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagTo_SetTOsDefault(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   if (FIPARAM_DI_INPUTTYPE_CONTACT(u8DiNum))
   {
      if (FIPARAM_DI_TO_TEST_PARAM != (UINT8)FIPARAM_E_DICFG_TO_OFF) /*lint !e835 !e572*/
      {
         /* set TO pin (Pin is inverted, so pin value will be LOW) */
         diSetGet_SetToEnPin(DICFG_TO_ACTIVE);
      }
   }
}


/*------------------------------------------------------------------------------
**
** diDiagTo_ExecTest()
**
** Description:
**    executes the TO-Test for a dedicated channel by calling the test
**    functions. One channel might be located at uC1 or uC2, with one test
**    all inputs of type DI-C are tested which are located at this uC.
**    As both uCs are involved in a test, depending on the current controller
**    different functions have to be called, depending on if the test is
**    executed at the own channel or the other channel.
**    Hint: In current HW only one TO is for 3 input Pins, so up to 3 inputs are
**          tested at once (depending on configuration).
**
**------------------------------------------------------------------------------
** Inputs:
**    u8ChNum: number/id of channel to evaluate => uC1 or uC2
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diDiagTo_ExecToTest(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagTo_ExecTest(CONST UINT8 u8ChNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* is u8DiNum equal number, then its an input located at uC1 */
         if (0u == u8ChNum)
         {
            diDiagTo_ExecTestAtOwnCh(u8ChNum);
         }
         else if (1u == u8ChNum)
         {
            diDiagTo_ExecTestAtOtherCh(u8ChNum);
         }
         else
         {
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_INVALID_PARAM);
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* is u8DiNum non-equal number, then its an input located at uC2 */
         if (0u == u8ChNum)
         {
            diDiagTo_ExecTestAtOtherCh(u8ChNum);
         }
         else if (1u == u8ChNum)
         {
            diDiagTo_ExecTestAtOwnCh(u8ChNum);
         }
         else
         {
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_INVALID_PARAM);
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
** diDiagTo_ExecTestAtOwnCh()
**
** Description:
**    executes the TO-Test of the own channel.
**    actions to be done at own channel (see [SRS_203]):
**    - read back test pulse, reset of variables
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
**    diDiagTo_ExecTestAtOwnCh(x);
**
** Module Test:
**    - No -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagTo_ExecTestAtOwnCh(CONST UINT8 u8ChNum)
{
   /*lint -esym(960, 10.1)*/
   /* Deactivated Misra Rule 10.1, the macro DI_DIAG_TO_RESET_PULSE
    * contains a shift operation that PC-Lint identify as complex integer
    * expression. */

   UINT32 u32TmpCntVal;
   UINT8 u8Index;

   /* check if Channel number is valid */
   GLOBFAIL_SAFETY_ASSERT( (u8ChNum < GPIOCFG_NUM_CHANNELS), GLOB_FAILCODE_INVALID_PARAM);

   /* read test-counter value to tmp-variable */
   u32TmpCntVal = RDS_GET(diDiagTo_sChState.u32TestCnt[u8ChNum]);

   /* actions to be taken during test at own channel */
   if (DI_DIAG_TO_START_TEST == u32TmpCntVal)
   {
      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      for (u8Index = u8ChNum; u8Index < GPIOCFG_NUM_DI_TOT; u8Index+=2)
      /* RSM_IGNORE_END */
      {
         diDiagTo_sState.eTestState[u8Index] = eDIAG_EXEC_CYC;
      }
   }
   else if ( (DI_DIAG_TO_RESET_PULSE) == u32TmpCntVal)
   {
      /* other channel resets test pulse, this channel evaluates result */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      for (u8Index = u8ChNum; u8Index < GPIOCFG_NUM_DI_TOT; u8Index+=2)
      /* RSM_IGNORE_END */
      {
         /* call function to evaluate test result (if test pulse was received
          * and if it is a DI-C input) */
         if (FIPARAM_DI_INPUTTYPE_CONTACT(u8Index))
         {
            diDiagTo_EvalTestResultOwnCh(u8Index);
         }
      }
   }
   else if ( (DI_DIAG_TO_STOP_TEST) == u32TmpCntVal)
   {
      /* set default values for test counter */
      RDS_SET(diDiagTo_sChState.u32TestCnt[u8ChNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);

      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      for (u8Index = u8ChNum; u8Index < GPIOCFG_NUM_DI_TOT; u8Index+=2)
      /* RSM_IGNORE_END */
      {
         diDiagTo_sState.eTestState[u8Index] = eDIAG_WAIT_CYC;
      }

      diDiagTo_sChState.eTestExecutionActive[u8ChNum] = eFALSE;
   }
   else if ( (DI_DIAG_TO_STOP_TEST) < u32TmpCntVal)
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
   /*lint +esym(960, 10.1)*//* Activate Misra Rule 10.1. */
}


/*------------------------------------------------------------------------------
**
** diDiagTo_ExecTestAtOtherCh()
**
** Description:
**    executes the actions for TO-Test for the other channel
**    actions to be done for other channel (see [SRS_203]):
**    - set test pulse which shall be received by other channel
**    - reset test pulse
**    - reset of variables
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
**    diDiagTo_ExecTestAtOtherCh(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagTo_ExecTestAtOtherCh(CONST UINT8 u8ChNum)
{
   /*lint -esym(960, 10.1)*/
   /* Deactivated Misra Rule 10.1, the macro DI_DIAG_TO_RESET_PULSE
    * contains a shift operation that PC-Lint identify as complex integer
    * expression. */

   UINT32 u32TmpCntVal;
   UINT8 u8Index;

   /* check if Channel number is valid */
   GLOBFAIL_SAFETY_ASSERT( (u8ChNum < GPIOCFG_NUM_CHANNELS), GLOB_FAILCODE_INVALID_PARAM);

   /* read test-counter value to tmp-variable */
   u32TmpCntVal = RDS_GET(diDiagTo_sChState.u32TestCnt[u8ChNum]);

   /* actions to be taken during Start of test at other channel */
   if (DI_DIAG_TO_START_TEST == u32TmpCntVal)
   {
      /* deactivate TO Pin to (inverted, so Pin value is HIGH afterwards),
       * see [SRS_236] */
      diSetGet_SetToEnPin(DICFG_TO_INACTIVE);

      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      for (u8Index = u8ChNum; u8Index < GPIOCFG_NUM_DI_TOT; u8Index+=2)
      /* RSM_IGNORE_END */
      {
         diDiagTo_sState.eTestState[u8Index] = eDIAG_EXEC_CYC;
      }
   }
   /* actions to be taken at Stop of test at other channel */
   else if ( (DI_DIAG_TO_RESET_PULSE) == u32TmpCntVal)
   {
      /* re-activate TO Pin to (inverted, so Pin value is LOW afterwards) */
      diSetGet_SetToEnPin(DICFG_TO_ACTIVE);
   }
   /* actions to be taken at Stop of test at other channel */
   else if ( (DI_DIAG_TO_STOP_TEST) == u32TmpCntVal)
   {
      /* set default values for test counter */
      RDS_SET(diDiagTo_sChState.u32TestCnt[u8ChNum], DIDODIAG_DIDO_TEST_CNT_RESET_VAL);

      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      for (u8Index = u8ChNum; u8Index < GPIOCFG_NUM_DI_TOT; u8Index+=2)
      /* RSM_IGNORE_END */
      {
         diDiagTo_sState.eTestState[u8Index] = eDIAG_WAIT_CYC;
      }

      diDiagTo_sChState.eTestExecutionActive[u8ChNum] = eFALSE;
   }
   else if ( (DI_DIAG_TO_STOP_TEST) < u32TmpCntVal)
   {
      /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
   else
   {
      /* nothing to do in case of wait cycles... */
   }
   /*lint +esym(960, 10.1)*//* Activate Misra Rule 10.1. */
}


/*------------------------------------------------------------------------------
**
** diDiagTo_EvalTestResultOwnCh()
**
** Description:
**    evaluates the results of the TO test at the uC where the test is ongoing,
**    for all inputs which are in the TO-Test-State "eDIAG_EXEC_CYC" (that is
**    currently the only test state of TO-Test).
**    => test pulse send by other channel shall be received at these inputs
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
**    diDiagTo_EvalTestResultOwnCh(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diDiagTo_EvalTestResultOwnCh(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* first action for uCx: read back input value */
   if (DICFG_DI_INACTIVE == DIINPUT_DI_OWN_FIRST_FILTER(DICFG_GET_DUAL_CH_NUM(u8DiNum)))
   {
      /* inform error handler that TO test OK */
      diErrHdl_ToTestOk(u8DiNum);
   }
   else if (DICFG_DI_ACTIVE == DIINPUT_DI_OWN_FIRST_FILTER(DICFG_GET_DUAL_CH_NUM(u8DiNum)))
   {
      /* error: if DI still active, test pulse was not received */

      /* inform error handler that test failed, channel passivation */
      diErrHdl_ToTestFailed(u8DiNum);
   }
   else
   {
      /* invalid state/value of input variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}



/*******************************************************************************
**
** End of doDiagTo.c
**
********************************************************************************
*/

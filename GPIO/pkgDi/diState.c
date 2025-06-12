/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diState.c
**
** $Id: diState.c 4086 2023-09-27 15:49:53Z ankr $
** $Revision: 4086 $
** $Date: 2023-09-27 17:49:53 +0200 (Mi, 27 Sep 2023) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** main controlling and state handling of the digital input module.
** State machine and switching of input states.
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
#include "globErrHandler.h"
#include "rds.h"

/* Module header */
#include "cfg_Config-sys.h"
#include "gpio_cfg.h"
#include "diCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "diDiag.h"
#include "diPortMap.h"
#include "diInput.h"
#include "diErrHdl.h"
#include "diState.h"
#include "diState_priv.h"



/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/
/*------------------------------------------------------------------------------
** structure containing the current states of inputs
**------------------------------------------------------------------------------
*/
STATIC DI_STATE_STRUCT diState_sDiState;

/*------------------------------------------------------------------------------
** array containing:
** requested error reset requests coming from SPDU.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "diState.c" and "diLib.c".
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM diState_DiErrRstSPDU[GPIOCFG_NUM_DI_TOT] = { eFALSE, \
                                                             eFALSE, \
                                                             eFALSE, \
                                                             eFALSE, \
                                                             eFALSE, \
                                                             eFALSE};
#if GPIOCFG_NUM_DI_TOT > 6u   /* if the defined number of inputs was changed, send hint! */
  #error "init values of array < diState_DiErrRstSPDU > must be adapted to new DI number!"
#endif

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
** diState_Init()
**
** Description:
**    initialization of the digital input module (state machine)
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
**    diState_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diState_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* check if the DI was disabled by Parametrization (iParameters) */
      if (FIPARAM_DI_DISABLED(DICFG_GET_DUAL_CH_NUM(u8Index)))
      {
         /* set state "don't use", depending on iParameters */
         diState_sDiState.eState[u8Index] = eDI_STATE_DONT_USE;
      }
      else
      {
         /* set default state "inactive" */
         diState_sDiState.eState[u8Index] = eDI_STATE_INACTIVE;
      }

      diState_DiErrRstSPDU[u8Index] = eFALSE;
   }
}


/*------------------------------------------------------------------------------
**
** diState_ProcessDIs()
**
** Description:
**    processing of input states / state machine of inputs.
**    depending on current input state the necessary functions to process inputs
**    and execute tests are called and (depending on conditions) the next state
**    is entered (here or in one of the called state-functions).
**    If during a test an error occured, corresponding Input will be set to an
**    error state.
**    Possible States:
**    eDI_STATE_INACTIVE: DI input is not active
**    eDI_STATE_ACTIVE_WAIT_TEST: DI input is read as active, but not tested by
**                                diagnostic function yet
**    eDI_STATE_ACTIVE_TESTED_AND_VALID: DI input is read as active, and successfully
**                                       tested by diagnostic function
**    eDI_STATE_ERROR: one of the input diagnostic failed
**    eDI_STATE_DONT_USE: do not use pin, ignore its state (possible parametrization)
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
**    diState_ProcessDIs(), called frequently
**
** Module Test:
**    - YES -
**    Reason:
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diState_ProcessDIs(void)
{
   UINT8 u8Index;
   TRUE_FALSE_ENUM eIsDIinWaitTest = eFALSE;

   /* for all digital inputs: handle input state */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* evaluate requested state */
      switch (diState_sDiState.eState[u8Index])
      {
         case eDI_STATE_INACTIVE:
            if (eFALSE == eIsDIinWaitTest)  /* only for faster SW execution:
            as long as another DI is already waiting for test, stay in inactive */
            {
              diState_InactiveState(u8Index);
            }
            break;
         case eDI_STATE_ACTIVE_WAIT_TEST:
            diState_ActiveWaitTestState(u8Index);
            break;
         case eDI_STATE_ACTIVE_TESTED_AND_VALID:
            diState_ActiveState(u8Index);
            break;
         case eDI_STATE_ERROR:
            diState_ErrState(u8Index);
            break;
         case eDI_STATE_DONT_USE:
            /* OK to enter here, but nothing to do... */
            break;
         default:  /* ERROR case, should never enter here */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;
      }

      /* in one of the functions called above the state might have changed,
       * check here if the DI is currently waiting for test*/
      if (eDI_STATE_ACTIVE_WAIT_TEST == diState_sDiState.eState[u8Index])
      {
         eIsDIinWaitTest = eTRUE;
      }

      /* do for all states:
       * check if during processing any test failed / error occurred on input,
       * go to error state only when tests at pin are finished,
       * because test function might fulfill special pin timings and has to
       * reset portpins */
      if (FALSE == diDiag_IsTestAtPinOngoing(u8Index))
      {
         if(DI_ERR_IS_TEST_QUAL_ERR(u8Index))
         {
            diState_sDiState.eState[u8Index] = eDI_STATE_ERROR;
         }
         else
         {
            /* no error occured, no need to enter error state for DI */
         }
      }
   }

   /* process of safe DI finished, so a possible error reset request can be confirmed */
   globErrHandler_DiRstConf();
}


/*------------------------------------------------------------------------------
**
** diState_GetDiState()
**
** Description:
**    returns the state "active" or "not active". This is the DI state to be used
**    for upper SW. Upper SW only has to know if the DI has to be send as "active"
**    or not active (via safety protocol only one bit per DI is send to indicate
**    active/not active).
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    GPIO_STATE_ENUM: is input Active or Not Active
**
** Usage:
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
** Context: Background Task
** Context: main, while(FOREVER)
**------------------------------------------------------------------------------
*/
GPIO_STATE_ENUM diState_GetDiState(CONST UINT8 u8DiNum)
{
   GPIO_STATE_ENUM eRetVal = DICFG_DI_INACTIVE;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   switch (diState_sDiState.eState[u8DiNum])/*lint !e661*/
   {
      /* DI can be reported as active (tested and valid), see [SRS_2012] */
      case eDI_STATE_ACTIVE_TESTED_AND_VALID:
         eRetVal = DICFG_DI_ACTIVE;
         break;
      /* for all other states the input has to be considered as "not active",
       * see [SRS_2012] */
      case eDI_STATE_INACTIVE:
      case eDI_STATE_ACTIVE_WAIT_TEST:  /* fall through */
      case eDI_STATE_ERROR:             /* fall through */
      case eDI_STATE_DONT_USE:          /* fall through */    /* see [SRS_273] */
         eRetVal = DICFG_DI_INACTIVE;
         break;
      default: /* ERROR case, should never enter here */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;

   }

   /* in case of dual channel: if both channels are not equal, value is to be
    * considered "not active", see [SRS_105], [SRS_231], [SRS_2013] */
   if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8DiNum)))
   {
      /* if the states of both channels are different, or the other channel is
       * in error mode, the DI pin state has to be considered as "not active",
       * see [SRS_105], [SRS_608] */
      if ( (diState_sDiState.eState[u8DiNum] != 
            diState_sDiState.eState[DICFG_GET_OTHER_CHANNEL(u8DiNum)]) \
        || (DI_ERR_IS_TEST_QUAL_CH_ERR(u8DiNum)) )
      {
         eRetVal = DICFG_DI_INACTIVE;
      }
   }

   /* return the resulting state (is active or not) of selected input */
   return (eRetVal);
}


/*------------------------------------------------------------------------------
**
** diDiag_HandleToTestExecution()
**
** Description:
**    calls function to check/handle if TO-Test has to be executed.
**    The TO test is only called in the state "tested and valid", means TO test
**    is executed after a successful Level-Test, and only for DIs of Type DI-C.
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
**    diDiag_HandleToTestExecution();
**
** Module Test:
**    - NO -
**    Reason: just function call
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diState_HandleToTestExecution(void)
{
   UINT8 u8Index;

   /* for all digital inputs: is there a request for TO-Test? (see [SRS_236]) */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      if (FIPARAM_DI_INPUTTYPE_CONTACT(u8Index) && \
         (eDI_STATE_ACTIVE_TESTED_AND_VALID == diState_sDiState.eState[u8Index]))
      {
         /* call function to check if TO-test has to be executed */
         diDiag_HandleToTestState(u8Index);
      }
   }

   /* call TO-Test handling in Diag-TO module */
   diDiag_HandleToTestExecution();
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diState_InactiveState()
**
** Description:
**    handles the state "INACTIVE" of a digital input
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
**    doCtrl_inactiveState(x);
**
** Module Test:
**    - NO -
**    Reason: function call only
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diState_InactiveState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* call consistency check only for every second DI to save time */
   if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
   {
      /* trigger check of values of other channel , if there is currently
       * no test ongoing */
      if (FALSE == diDiag_IsTestAtPinOngoing(diCfg_u8ConsDiOtherChNum[u8DiNum]))
      {
         /* call cosistency check, see [SRS_604] */
         diDiag_HandleConsTest(u8DiNum);
      }
   }

   /* is input pin active? Then switch to next state (but only if no error/consistency
    * error occurred at the pin) */
   if (DICFG_DI_ACTIVE == DI_INPUTVAL_DI(u8DiNum))
   {
      /* if filtered value of input pin is true, then go to "test wait" state */
      diState_sDiState.eState[u8DiNum] = eDI_STATE_ACTIVE_WAIT_TEST;
   }
   else
   {
      /* test pin state for invalid values */
      if(DICFG_DI_INACTIVE != DI_INPUTVAL_DI(u8DiNum))
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }
}


/*------------------------------------------------------------------------------
**
** diState_ActiveWaitTestState()
**
** Description:
**    handles the state "ACTIVE and wait for Test" of a digital input, which
**    is the state when a safe input has been detected as "active" but not been
**    tested so far (to judge, if "active" state is valid).
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
**    diState_ActiveWaitTestState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diState_ActiveWaitTestState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* change back to "inactive state" only when no test is ongoing
    * (during tests input state of pin changes intentionally to "inactive") */
   if (FALSE == diDiag_IsTestAtPinOngoing(u8DiNum))
   {
      if (DICFG_DI_INACTIVE == DI_INPUTVAL_DI(u8DiNum))/*lint !e661*/
      {
         diDiag_RstTestStates(u8DiNum);
         /* reset error counters, because there migth have been already some
          * counter increases */
         diErrHdl_rstErrCnt(u8DiNum);
         /* if filtered value of input pin is meanwhile inactive again,
          * then go back to inactive state */
         diState_sDiState.eState[u8DiNum] = eDI_STATE_INACTIVE;/*lint !e661*/
      }
      else
      {
         /* test pin state for invalid values */
         if(DICFG_DI_ACTIVE != DI_INPUTVAL_DI(u8DiNum))
         {
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         }
      }

      /* call consistency check only for every second DI to save time */
      if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
      {
         /* trigger check of values of other channel, if also there is no test
          * ongoing */
         if (FALSE == diDiag_IsTestAtPinOngoing(diCfg_u8ConsDiOtherChNum[u8DiNum]))
         {
            /* call cosistency check, see [SRS_604] */
            diDiag_HandleConsTest(u8DiNum);
         }
      }

   }

   /* trigger first execution of level test (see [SRS_108]), if level test successful,
    * the DI can be seen as "active and tested" */
   diDiag_HandleLevTestStates(u8DiNum);

   /* change to next state only if tests have been finished (to ensure that also tests at pins 
   ** located at other uC has been finished) */
   if (TRUE == diDiag_FirstLevelTestFinished(u8DiNum))
   {
      /* if test was executed and OK (test-qualifier-bit = "1"), go to next state,
       * see [SRS_470], [SRS_231] */
      if (DI_ERR_IS_TEST_QUAL_OK(u8DiNum))/*lint !e661*/
      {
         diState_sDiState.eState[u8DiNum] = eDI_STATE_ACTIVE_TESTED_AND_VALID;/*lint !e661*/
      }         /* if test was executed and OK (test-qualifier-bit = "1"), go to next state */
   }
}


/*------------------------------------------------------------------------------
**
** diState_ActiveState()
**
** Description:
**    handles the state "ACTIVE" of a digital input. In this state the safe DI
**    has been detected as "ACTIVE" and the first test has already been executed.
**    The tests of the inputs are frequently triggered in this state.
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
**    diState_ActiveState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diState_ActiveState(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   if ( FALSE == diDiag_IsTestAtPinOngoing(u8DiNum) )
   {
      /* call consistency check only for every second DI to save time */
      if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
      {
         /* trigger check of values of other channel , if also there is currently
          * no test ongoing */
         if (FALSE == diDiag_IsTestAtPinOngoing(diCfg_u8ConsDiOtherChNum[u8DiNum]))
         {
            /* call cosistency check, see [SRS_604] */
            diDiag_HandleConsTest(u8DiNum);
         }
      }

      switch (DI_INPUTVAL_DI(u8DiNum))
      {
         case DICFG_DI_ACTIVE:
            /* handle test states (cyclic Level Test, see [SRS_108]), check if
             * test has to be executed */
            diDiag_HandleLevTestStates(u8DiNum);
            break;

         case DICFG_DI_INACTIVE:
            diDiag_RstTestStates(u8DiNum);
            /* reset error counters, because there migth have been already some
             * counter increases */
            diErrHdl_rstErrCnt(u8DiNum);
            /* set DI state to "inactive", see [SRS_469] */
            diState_sDiState.eState[u8DiNum] = eDI_STATE_INACTIVE;/*lint !e661*/
            break;

         default:
            /* invalid state of variable */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;
      }
   }
   else
   {
      /* handle test states (Level Test), check if test has to be executed */
      diDiag_HandleLevTestStates(u8DiNum);
   }
}


/*------------------------------------------------------------------------------
**
** diState_ErrState()
**
** Description:
**    handles the internal error state "DI_STATE_ERROR_INT" of a digital input.
**    This is the state entered if any of the input tests (executed by the DI
**    module itself) failed.
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
**    diState_ErrState(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void diState_ErrState(CONST UINT8 u8DiNum)
{
   TRUE_FALSE_ENUM eErrRst = eFALSE;

   LOCAL_STATIC (, TRUE_FALSE_ENUM, aeLastDiErrRstSPDU[GPIOCFG_NUM_DI_TOT], DISTATE_ERR_RESET_INIT);

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* reset all test states */
   diDiag_RstTestStates(u8DiNum);

   /*************************************************************/
   /* check for error reset received via safety protocol, see [SRS_2011] */
   if (eFALSE == aeLastDiErrRstSPDU[u8DiNum])
   {
      /* only accept error reset flag, in case of "rising edge", means if
       * last received error flag was 0. Result is that "eErrRst" is only eTRUE
       * if  "diState_DiErrRstSPDU" is " eTRUE and "aeLastDiErrRstSPDU" is eFALSE*/
      eErrRst = diState_DiErrRstSPDU[u8DiNum];
   }
   else if (eTRUE == aeLastDiErrRstSPDU[u8DiNum])
   {
      /* if last received error reset flag was eTRUE, then set "eErrRst" to eFALSE
       * regardless of current error reset flag */
      eErrRst = eFALSE;
   }
   else
   {
      /* error case, should never enter here */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }

   /**********************************************************************/
   /* if error-reset was not already requested by reset flag received via
    * safety protocol, check for global error reset, see [SRS_2011] */
   if (eFALSE == eErrRst)
   {
      eErrRst = globErrHandler_GetDiRstReq();
   }

   /***************************************************************************/
   /* now check if there was an error reset request, in case of valid reset
    * the error state of DI is left [SRS_2011], check for valid range
    * of variable */
   if (eTRUE == eErrRst)
   {
      /* reset pending errors on DI  */
      diErrHdl_rstErr(u8DiNum);
      /* set default values for error counters */
      diErrHdl_rstErrCnt(u8DiNum);
      /* set to state "inactive" when error resolved/reseted, see [SRS_2011] */
      diState_sDiState.eState[u8DiNum] = eDI_STATE_INACTIVE;/*lint !e661*/
      /* reset value of the last received error reset request to default */
      aeLastDiErrRstSPDU[u8DiNum] = eFALSE;
   }
   else if (eFALSE == eErrRst)
   {
      /* nothing to do, stay in internal error state, see also [SRS_2011] */
   }
   else
   {
      /* error case, should never enter here */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }

   /* save current state of error-reset flag to have the last value available
      in next call */
   aeLastDiErrRstSPDU[u8DiNum] = diState_DiErrRstSPDU[u8DiNum];
}


/*******************************************************************************
**
** End of diState.c
**
********************************************************************************
*/

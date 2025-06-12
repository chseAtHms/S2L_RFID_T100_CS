/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doState.c
**
** $Id: doState.c 4436 2024-05-17 14:28:50Z ankr $
** $Revision: 4436 $
** $Date: 2024-05-17 16:28:50 +0200 (Fr, 17 Mai 2024) $
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
** main controlling and state handling of the digital output module.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013-2024 HMS Industrial Networks AB            **
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
#include "globErrHandler.h"
#include "gpio_cfg.h"
#include "doCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "doSetGetPin.h"
#include "doDiag.h"
#include "doErrHdl.h"
#include "doDiagVsup.h"
#include "doState.h"
#include "doState_priv.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** array containing:
** requested state coming from SPDU, in case of dual channel only one bit per
** channel is valid.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "doState.c", "doLib.c" and doSafeBoundSS1t.c.
**------------------------------------------------------------------------------
*/
GPIO_STATE_ENUM doState_DoReqSPDU[GPIOCFG_NUM_DO_TOT];

/*------------------------------------------------------------------------------
** array containing:
** requested error reset requests coming from SPDU.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "doState.c" and "doLib.c".
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM doState_DoErrRstSPDU[GPIOCFG_NUM_DO_TOT];


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing the current states of outputs
**------------------------------------------------------------------------------
*/
STATIC DO_STATE_STRUCT doState_doState;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doState_Init()
**
** Description:
**    initialization of the digital output module
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
**    doState_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doState_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      /* check if the DO was disabled by Parametrization (iParameters) */
      if (FIPARAM_DO_DISABLED(u8Index))
      {
         /* set state "don't use", depending on iParameters */
         doState_doState.eState[u8Index] = eDO_STATE_DONT_USE;
      }
      else
      {
         /* set default state "inactive" */
         doState_doState.eState[u8Index] = eDO_STATE_INACTIVE;
      }

      doState_RstActiveStateCnt(u8Index);

      doState_DoErrRstSPDU[u8Index] = eFALSE;
   }
}



/*------------------------------------------------------------------------------
**
** doState_ProcessDOs()
**
** Description:
**    processing of outputs states / state machine of outputs.
**    depending on current output state the necessary functions to set outputs
**    or execute tests are called
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
**    doState_ProcessDOs(), called frequently
**
** Module Test:
**    - NO -
**    Reason: easy to understand, different cases resulting in function calls
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doState_ProcessDOs(void)
{
   UINT8 u8Index;
   DO_STATE_ENUM eLastDoState[GPIOCFG_NUM_DO_TOT];

   /* handle VSUP Test */
   doDiagVsup_HandleTestState();
   /* set VSUP Test preventive to inactive, if at least one DO is active the
    * VSUP Test is activated again */
   doDiagVsup_DeactivateVsupTest();

   /* for all digital outputs: handle output state */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      eLastDoState[u8Index] = doState_doState.eState[u8Index];

      /* check if the DO was disabled by Parametrization (iParameters) */
      if (FIPARAM_DO_DISABLED(u8Index))
      {
         /* set state "don't use", depending on iParameters, see also [SRS_273] */
         doState_doState.eState[u8Index] = eDO_STATE_DONT_USE;
      }
      /* else call function to evaluate next/requested state only if no test currently ongoing, 
       * reason: during test mode some timings regarding pin set/reset have to be fulfilled, so it 
       * is saver for the HW to finish test including pin reset before applying new state  */
      else if (FALSE == doDiag_IsTestAtPinOngoing(u8Index))
      {
         doState_EvalNextState(u8Index);
         /* call function to check if the DO pin has its nominal value,
          * see [SRS_638] */
         doSetGetPin_CheckPinState(u8Index);
      }
      else
      {
         doSetGetPin_RstDisDo(u8Index);
      }

      /* evaluate requested state */
      switch (doState_doState.eState[u8Index])
      {
         case eDO_STATE_INACTIVE:
            /* reset error counters of HS test and pin state check if the state
             * was switched */
            if (eDO_STATE_ACTIVE == eLastDoState[u8Index])
            {
               doErrHdl_rstErrCntHsTestPinChk(u8Index);
            }
            doState_InactiveState(u8Index);
            doState_RstActiveStateCnt(u8Index);
            break;
         case eDO_STATE_ACTIVE:
            /* at least one DO is active, call function to activate VSUP Test */
            doDiagVsup_ActivateVsupTest();
            /* reset error counters of HS test and pin state check if the state
             * was switched */
            if (eDO_STATE_INACTIVE == eLastDoState[u8Index])
            {
               doErrHdl_rstErrCntHsTestPinChk(u8Index);
            }
            doState_ActiveState(u8Index);
            doState_IncrActiveStateCnt(u8Index);
            break;
         case eDO_STATE_ERROR:
            doState_ErrState(u8Index);
            doState_RstActiveStateCnt(u8Index);
            break;
         case eDO_STATE_DONT_USE:
            doDiag_RstDoTestStates(u8Index);
            doSetGetPin_SwitchDoPinOff(u8Index);
            /* reset the requested output state coming from SPDU, see [SRS_273] */
            doState_DoReqSPDU[u8Index] = eGPIO_LOW;
            doState_RstActiveStateCnt(u8Index);
            break;
         default:
            /* ERROR case, should never enter here */
            /* invalid value of variable */
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;

      }
   }

   /* process of safe DOs finished, so a possible error reset request can be confirmed */
   globErrHandler_DoRstConf();
}


/*------------------------------------------------------------------------------
**
** doState_ProcessDOsLight()
**
** Description:
**    handles port switching for DO processing (smaller and faster function
**    instead of calling ProcessDOs always)
**    Currently this functions sets only resets the DO after a Highside Switch test
**    was finished (to achieve the waiting time after discharge pin), see also
**    [SRS_641].
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
**    doState_ProcessDOsLight();
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doState_ProcessDOsLight(void)
{
   UINT8 u8DoNum;

   /* for all digital outputs: handle output state */
   for (u8DoNum=0u; u8DoNum<GPIOCFG_NUM_DO_TOT; u8DoNum++)
   {
      /*lint -esym(960, 12.4)*/
      /* check if output is currently in test mode, and check again
       * (just for safety) that request and state are really "output active" */
      if (  (FALSE == doDiag_IsTestAtPinOngoing(u8DoNum))
         && ((eGPIO_HIGH == doState_DoReqSPDU[u8DoNum]) 
              && (eDO_STATE_ACTIVE == doState_doState.eState[u8DoNum]))
         && ( (doSetGetPin_GetDisChrgPin(u8DoNum)) == eGPIO_HIGH ) )
      {
         /* set Port Pin to TRUE (also if it is already TRUE) */
         doSetGetPin_SetDoPin(u8DoNum, eGPIO_HIGH);
      }
      /*lint +esym(960, 12.4)*/
      /* Deactivated because there are no side effects. */
   }
}


/*------------------------------------------------------------------------------
**
** doState_ActiveCycTillTestReached()
**
** Description:
**    evaluates the counter of the active cycles of a DO. Returns eTRUE or eFALSE,
**    depending on if the counters limit is reached.
**    Additional Information: When this limit is reached, the tests of the DO
**    (currently HS-Test) shall have been executed. Feature is used for timeout
**    check of diagnostic module.
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    eTRUE: if the DO was active for a number of cycles, so that the test shall
**           already have been executed
**    eFALSE: DO was active below the limit
**
** Usage:
**    doState_ActiveCycTillTestReached(x);
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: main, while(FOREVER)
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM doState_ActiveCycTillTestReached(CONST UINT8 u8DoNum)
{
   TRUE_FALSE_ENUM eRetVal = eFALSE;
  
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);
  
   /* Attention: RDS variable 'doState_doState.u32ActiveStateCnt' is also accessed from IRQ. 
   ** Since RDS variables are not atomic, it is required to disable IRQs here temporary.
   ** See OnTime Defect#6975 */  
   __disable_irq();
   /* if DO was active for at least x cycles, set return value to true */
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ( ((UINT32)(RDS_GET(doState_doState.u32ActiveStateCnt[u8DoNum]))) 
           >= ((UINT32)DOSTATE_MAX_ACTIVE_CYC_TILL_TEST) ) /*lint !e948*/
   {
      eRetVal = eTRUE;
   }
   else
   {
      /* do nothing in else branch (keep initial value of return variable) */
   }
   /* enable IRQs again */
   __enable_irq();

   return (eRetVal);
}


/*------------------------------------------------------------------------------
**
** doState_SwitchDOsOff()
**
** Description:
**    switches off all safe DOs, specially used during init/startup, see [SRS_75]
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
**    switch off DOs e.g. for state changes
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doState_SwitchDOsOff(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      /* switch off the DO */
      doSetGetPin_SwitchDoPinOff(u8Index);
   }
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doState_EvalNextState()
**
** Description:
**    evaluation of next output state to execute.
**    Depending on the request received from other modules a priorization
**    is done and final state is evaluated.
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
**    doState_EvalNextState(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_EvalNextState(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* check if qualifier is in error state, this is the case if an internal test
    * failed or if the output of the other channel transmitted an error via IPC  */
   if (DO_ERR_TEST_QUAL_ERR(u8DoNum))
   {
      doState_doState.eState[u8DoNum] = eDO_STATE_ERROR;
   }
   else
   {
      /* check request from safety protocol (see [SRS_2007], [SRS_2014]), but set active mode
       * only when Test qualifier is "OK" or "NA" */
      if ((eGPIO_HIGH == doState_DoReqSPDU[u8DoNum]) && (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8DoNum)))
      {
         doState_doState.eState[u8DoNum] = eDO_STATE_ACTIVE;
      }
      else
      {
         doState_doState.eState[u8DoNum] = eDO_STATE_INACTIVE;
      }
   }
}


/*------------------------------------------------------------------------------
**
** doState_ActiveState()
**
** Description:
**    handles the state "ACTIVE" of a digital output (part of [SRS_113])
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
**    doState_ActiveState(2);
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_ActiveState(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* call function to check if high side switch test (output switch off) has to be executed */
   doDiag_HandleDoTestStates(u8DoNum);

   /* check if output is currently in test mode, and check again (just for safety) that request is 
    * really "output active" */
   if ( (FALSE == doDiag_IsTestAtPinOngoing(u8DoNum)) && (eGPIO_HIGH == doState_DoReqSPDU[u8DoNum]))
   {
      /* set Port Pin to TRUE (also if it is already TRUE) */
      doSetGetPin_SwitchDoPinOn(u8DoNum);
   }
}


/*------------------------------------------------------------------------------
**
** doState_InactiveState()
**
** Description:
**    handles the state "INACTIVE" of a digital output
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doState_InactiveState(2);
**
** Module Test:
**    - NO -
**    Reason: function call only
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_InactiveState(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);
   /* call function to reset the test modules */
   doDiag_RstDoTestStates(u8DoNum);
   /* set Port Pin to FALSE (also if it is already FALSE) */
   doSetGetPin_SwitchDoPinOff(u8DoNum);
}


/*------------------------------------------------------------------------------
**
** doState_ErrState()
**
** Description:
**    handles the state "eDO_STATE_ERROR" of a digital output
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doState_ErrState(2);
**
** Module Test:
**    - NO -
**    Reason: function call only
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_ErrState(CONST UINT8 u8DoNum)
{
   TRUE_FALSE_ENUM eErrRst = eFALSE;
   LOCAL_STATIC (, TRUE_FALSE_ENUM, aeLastDoErrRstSPDU[GPIOCFG_NUM_DO_TOT], DOSTATE_ERR_RESET_INIT);

   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* set Port Pin to FALSE, see [SRS_642] (also if it is already FALSE) */
   /* This is done immediately and unconditionally when entering the error state (e.g. no SS1-t
      delay applied, see [SRS_913]). */
   doSetGetPin_SwitchDoPinOff(u8DoNum);

   /* reset all test states */
   doDiag_RstDoTestStates(u8DoNum);

   /*************************************************************/
   /* check for error reset received via safety protocol, see [SRS_2015] */
   if (eFALSE == aeLastDoErrRstSPDU[u8DoNum])
   {
      /* only accept error reset flag, in case of "rising edge", means if
       * last received error flag was 0. Result is that "eErrRst" is only eTRUE
       * if  "diState_DoErrRstSPDU" is " eTRUE and "aeLastDoErrRstSPDU" is eFALSE*/
      eErrRst = doState_DoErrRstSPDU[u8DoNum];
   }
   else if (eTRUE == aeLastDoErrRstSPDU[u8DoNum])
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

   /*********************************************************************/
   /* if error-reset was not already requested by reset flag received via
    * safety protocol, check for global error reset, see [SRS_2015], only allowed when
    * DO request is "OFF" */
   if ((eFALSE == eErrRst) && (eGPIO_LOW == doState_DoReqSPDU[u8DoNum]))
   {
     eErrRst = globErrHandler_GetDoRstReq();
   }

   /**************************************************************************/
   /* now check if there was an error reset request, and check for valid range
    * of variable */
   if (eTRUE == eErrRst)
   {
     /* reset pending errors on DO */
     doErrHdl_RstErr(u8DoNum);
     /* reset error counter of DO  */
     doErrHdl_rstErrCnt(u8DoNum);
     /* set to state "inactive" when error resolved/reseted, [SRS_2015] */
     doState_doState.eState[u8DoNum] = eDO_STATE_INACTIVE;/*lint !e661*/
     /* reset value of the last received error reset request to default */
     aeLastDoErrRstSPDU[u8DoNum] = eFALSE;
   }
   else if (eFALSE == eErrRst)
   {
     /* nothing to do, stay in internal error state, see also [SRS_2015] */
   }
   else
   {
     /* error case, should never enter here */
     GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }

   /* save current state of error-reset flag to have the last value available
     in next call */
   aeLastDoErrRstSPDU[u8DoNum] = doState_DoErrRstSPDU[u8DoNum];

}


/*------------------------------------------------------------------------------
**
** doState_IncrActiveStateCnt()
**
** Description:
**    increments the counter for the active state of a DO up to a defined
**    limit.
**    Additional Information: When this limit is reached, the tests of the DO
**    (currently HS-Test) shall have been executed. Feature is used for timeout
**    check of diagnostic module.
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doState_IncrActiveStateCnt(x);
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_IncrActiveStateCnt(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* increase counter for DO active state, if counter did not already reach
    * the defined limit */
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   if ( ((UINT32)(RDS_GET(doState_doState.u32ActiveStateCnt[u8DoNum]))) 
           < ((UINT32)DOSTATE_MAX_ACTIVE_CYC_TILL_TEST) ) /*lint !e948*/
   {
      RDS_INC(doState_doState.u32ActiveStateCnt[u8DoNum]);
   }
   else
   {
      /* do nothing in else branch */
   }
}


/*------------------------------------------------------------------------------
**
** doState_RstActiveStateCnt()
**
** Description:
**    resets the counter for the active state of an DO
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doState_RstActiveStateCnt(x);
**
** Module Test:
**    - NO -
**    Reason: complexity low, easy to cover by review
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doState_RstActiveStateCnt(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   RDS_SET(doState_doState.u32ActiveStateCnt[u8DoNum], 0u);
}


/*******************************************************************************
**
** End of doState.c
**
********************************************************************************
*/

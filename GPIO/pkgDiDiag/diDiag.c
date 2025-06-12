/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiag.c
**
** $Id: diDiag.c 2448 2017-03-27 13:45:16Z klan $
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
** The module handles the test states of the safe input section.
**
** Currently 3 different types of test are implemented:
**
**    - Level-Test (for all active inputs):
**      A test pulse is generated at the pin of one uC which shall pull an active
**      input of the other uC to "inactive".
**      Internal shorts may be detected by this test
**
**    - Test-Output-Test (TO-Test, for active inputs configured as DI-C):
**      A test pulse is generated at the pin of one uC which is connected to the
**      IO-board and running through an external device (e.g. emergency button).
**      External shorts may be detected by this test.
**
**    - Consistency test:
**      for dual channel, check channels for consistency. Both channels of a
**      safe input must have the same value, if not, the consistency test fails.
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
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

/* Module header */
#include "gpio_cfg.h"
#include "diCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "diDiagCons.h"
#include "diDiagTo.h"
#include "diDiagLev.h"
#include "diDiag.h"


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


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiag_Init()
**
** Description:
**    initialization of the test module of digital inputs
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
**    diDiag_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiag_Init(void)
{
   /* set default/initialisation values for different test-modules */
   /* call initialization of Level-Test module */
   diDiagLev_Init();
   /* call initialization of TO-Test module */
   diDiagTo_Init();
   /* call initialization of Consistency-Test module */
   diDiagCons_Init();
}


/*------------------------------------------------------------------------------
**
** diDiag_HandleConsTest()
**
** Description:
**    calls the consistency check in case of dual channel configuration
**    of an input
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
**    diDiag_HandleConsTest(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiag_HandleConsTest(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* lint e960 disabled, no critical side effects */
   if ( (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8DiNum))) 
      && (0u != fiParam_GetConsFilter((DICFG_GET_DUAL_CH_NUM(u8DiNum)))) ) /*lint !e960*/
   {
      /* call cosistency check, see [SRS_604] */
        diDiagCons_Check(DICFG_GET_DUAL_CH_NUM(u8DiNum));
   }
}


/*------------------------------------------------------------------------------
**
** diDiag_HandleLevTestStates()
**
** Description:
**    calls function to check/handle if Level test has to be executed, see
**    [SRS_108]
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input state to be processed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diDiag_HandleLevTestStates(x);
**    with x: number of input
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiag_HandleLevTestStates(CONST UINT8 u8DiNum)
{
   /* u8DiNum is not checked here for valid range, because its only forwarded */

   /* call function to check if level-test (internal input deactivation) has to
    * be executed */
   diDiagLev_HandleTestState(u8DiNum);
}


/*------------------------------------------------------------------------------
**
** diDiag_RstTestStates()
**
** Description:
**    reset of the several Test states for a dedicated input:
**    - Level-Test
**    - TO-Test
**    - Consistency Test
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
**    diDiag_RstTestStates(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiag_RstTestStates(CONST UINT8 u8DiNum)
{
   /* u8DiNum is not checked here for valid range, because its only forwarded */

   /* set default values for level test-state */
   diDiagLev_RstTestState(u8DiNum);

   /* set default values for test output test-state */
   diDiagTo_RstTestState(u8DiNum);
}


/*------------------------------------------------------------------------------
**
** diDiag_IsAnyTestOngoing()
**
** Description:
**    function checks if any input pin is in test state "DI Level Test"
**    or "TO-Test"
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    diAnyTestOngoing: TRUE - Currently test ongoing
**                      FALSE - Currently no test ongoing
**
** Usage:
**    boolvariable = diDiag_IsAnyTestOngoing();
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
BOOL diDiag_IsAnyTestOngoing(void)
{
  /* declaration and initialization of return value */
  BOOL bDiAnyTestOngoing = FALSE;
  UINT8 u8Index;

  /* for every DI */
  for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
  {
     /*lint -esym(960, 12.4)*/
    /* check if any DI is currently in test state (test ongoing) */
    if ( (DI_DIAG_LEV_TEST_ONGOING(u8Index)) ||
         (eTRUE == diDiagTo_IsToTestOngoing(u8Index)) )
    {
       bDiAnyTestOngoing = TRUE;
       /* exit for-loop */
       /* RSM_IGNORE_QUALITY_BEGIN Notice #44 - Keyword 'break' identified outside a 'switch' 
        * structure */
       break;
       /* RSM_IGNORE_QUALITY_END */
    }
    /*lint +esym(960, 12.4)*/
  }

  return bDiAnyTestOngoing;
}


/*------------------------------------------------------------------------------
**
** diDiag_IsTestAtPinOngoing()
**
** Description:
**    check if currently one of the following test is at a dedicated pin ongoing:
**    - TO-Test
**    - Level-Test
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    doAnyHsTestOngoing: TRUE - Currently test ongoing
**                        FALSE - Currently no test ongoing
**
** Usage:
**    boolvariable = doDiag_IsTestAtPinOngoing(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
BOOL diDiag_IsTestAtPinOngoing(CONST UINT8 u8DiNum)
{
  /* declaration and initialization of return value */
  BOOL bDiTestOngoing = FALSE;
  TRUE_FALSE_ENUM eRet;

  /* check if DI number is valid */
  DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

  eRet = diDiagTo_IsToTestOngoing(u8DiNum);

  /* check if DI is currently in test state (test ongoing) */
  if ( (DI_DIAG_LEV_TEST_ONGOING(u8DiNum)) ||
      (eTRUE == eRet) )
  {
     bDiTestOngoing = TRUE;
  }

  return bDiTestOngoing;
}


/*------------------------------------------------------------------------------
**
** diDiag_FirstLevelTestFinished()
**
** Description:
**    check if the first level test at a pin has been terminated since the pin
**    changed the state to "active", used for [SRS_231], [SRS_470]
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input to evaluate
**
** Register Setting:
**    - None -
**
** Return:
**    bDiFirstTestDone: TRUE - test done
**                      FALSE - test not done
**
** Usage:
**    boolvariable = diDiag_FirstLevelTestFinished(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
BOOL diDiag_FirstLevelTestFinished(CONST UINT8 u8DiNum)
{
   /* declaration and initialization of return value */
   BOOL bDiFirstTestDone = FALSE;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* check if first TI Level test finished */
   if ( (DI_DIAG_LEV_FIRST_TEST_FINISHED(u8DiNum)))
   {
      bDiFirstTestDone = TRUE;
   }

   return bDiFirstTestDone;
}


/*------------------------------------------------------------------------------
 **
 ** diDiag_CheckDiagTimeout()
 **
 ** Description:
 **    This function is used to check for DI diagnostic timeout of input diagnostics.
 **    The function only checks if the Test-Execute-Variables were set to "eTRUE",
 **    which is done with every test execution. If the Test-Execute-Variable
 **    is eTRUE, the Test-Execute-Variable will be reseted by calling a reset
 **    function. If the Test-Execute-Variable is eFALSE, the test was not
 **    executed and therefore fail-safe is entered.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DiNum: number of input to check
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    diDiag_CheckDiagTimeout()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void diDiag_CheckDiagTimeout(CONST UINT8 u8DiNum)
{
   TRUE_FALSE_ENUM eConsTestToBeExec;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* check Level-Test Diagnostic */
   if (diDiagLev_WasTestExecuted(u8DiNum) == eTRUE)
   {
      diDiagLev_RstTestExecutedVar(u8DiNum);
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_DIDIAG_ERR);
   }

   /* check TO-Test Diagnostic */
   if (FIPARAM_DI_INPUTTYPE_CONTACT(u8DiNum))
   {
      if (diDiagTo_WasTestExecuted(u8DiNum) == eTRUE)
      {
         diDiagTo_RstTestExecutedVar(u8DiNum);
      }
      else
      {
         GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_DIDIAG_ERR);
      }
   }


   /* check consistency check only for every second DI to save time, the check
    * is also only executed for every second DI */
   if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
   {
      /* check Consistency Diagnostic (only of interest for dual channel configuration) */
      eConsTestToBeExec = diDiagCons_ConsTestToBeExec(DICFG_GET_DUAL_CH_NUM(u8DiNum));

      if (eTRUE == eConsTestToBeExec)
      {
         if (diDiagCons_WasTestExecuted(DICFG_GET_DUAL_CH_NUM(u8DiNum)) == eTRUE)
         {
            diDiagCons_RstTestExecutedVar(DICFG_GET_DUAL_CH_NUM(u8DiNum));
         }
         else
         {
            GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_DIDIAG_ERR);
         }
      }
      else if (eFALSE == eConsTestToBeExec)
      {
         /* nothing to do */
      }
      else
      {
         /* error case, should never enter here */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }
}


/*------------------------------------------------------------------------------
**
** diDiag_HandleToTestState()
**
** Description:
**    calls function to check if there is a request for a TO Test.
**
**-----------------------------------------------------------------------------
** Input:
**    u8DiNum: number of input to check
**
** Return:
**    - None -
**
** Usage:
**    diDiag_HandleToTestState(x)
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function call
**
** Context: IRQ Scheduler
**-----------------------------------------------------------------------------
*/
void diDiag_HandleToTestState(CONST UINT8 u8DiNum)
{
   diDiagTo_HandleTestState(u8DiNum);
   diDiagTo_TestExecutionCalled(u8DiNum);
}


/*------------------------------------------------------------------------------
**
** diDiag_HandleToTestExecution()
**
** Description:
**    calls function to handle execution of TO Tests
**
**-----------------------------------------------------------------------------
** Input:
**    u8DiNum: number of input to process
**
** Return:
**    - None -
**
** Usage:
**    diDiag_HandleToTestExecution(x)
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function call
**
** Context: IRQ Scheduler
**-----------------------------------------------------------------------------
*/
void diDiag_HandleToTestExecution(void)
{
   diDiagTo_HandleTestExecution();
}

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of diDiag.c
**
********************************************************************************
*/

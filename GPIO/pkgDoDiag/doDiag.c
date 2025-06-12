/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiag.c
**
** $Id: doDiag.c 2192 2017-02-17 08:08:29Z klan $
** $Revision: 2192 $
** $Date: 2017-02-17 09:08:29 +0100 (Fr, 17 Feb 2017) $
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
** The module handles the test states of the safe output section.
**
** Currently two different test type are implemented:
**
**    - Vsup-Test: Test of voltage supervision (HW-Block enabling/disabling the
**                 ability to control the digital outputs). There is only one
**                 voltage supervision block, so one test for the whole output
**                 HW is executed.
**
**    - HS-Test: for active outputs, this test shall examine if output still can
**               be switched off, HS is used as abbreviation for "High Side Switch".
**               Test is executed for each active output separately
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
#include "gpio_cfg.h"
#include "diInput.h"
#include "doSetGetPin.h"
#include "diDoDiag.h"
#include "doDiagHs.h"
#include "doDiagVsup.h"
#include "doDiag.h"

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
** doDiag_Init()
**
** Description:
**    initialization of the test module of digital outputs
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
**    doDiag_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiag_Init(void)
{
   /* init of High Side switch test module */
   doDiagHs_Init();

   /* init of Voltage Supervision test module */
   doDiagVsup_Init();
}


/*------------------------------------------------------------------------------
**
** doDiag_HandleDoTestStates()
**
** Description:
**    call function to handle all test which are executed for every output
**    separately (This is currently only the "HS-Test").
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
**    doDiag_HandleDoTestStates(x);
**    with x: number of output
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiag_HandleDoTestStates(UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* call function to check if HS test has to be executed */
   doDiagHs_HandleTestState(u8DoNum);
}


/*------------------------------------------------------------------------------
**
** doDiag_RstDoTestStates()
**
** Description:
**    reset of all test states which are executed for every output separately
**    (Currently only "HS-Test").
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doDiag_RstDoTestStates(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doDiag_RstDoTestStates(UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* set default values for test-state */
   doDiagHs_RstTestState(u8DoNum);
}


/*------------------------------------------------------------------------------
**
** doDiag_IsAnyTestOngoing()
**
** Description:
**    check if currently any test is ongoing on one of the safe outputs.
**    Possible is an ongoing Voltage Supervision test or HS-Test.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    doAnyHsTestOngoing: TRUE - Currently test ongoing
**                        FALSE - Currently no test ongoing
**
** Usage:
**    boolvariable = doDiag_IsAnyTestOngoing();
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
BOOL doDiag_IsAnyTestOngoing(void)
{
  /* declaration and initialization of return value */
  BOOL bDoAnyTestOngoing = FALSE;
  UINT8 u8Index;

  /* for every DO */
  for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
  {
    /* check if any DO is currently in test state (test ongoing) */
    if ( (DO_DIAG_HS_TEST_ONGOING(u8Index)) || (DO_DIAG_VSUP_TEST_ONGOING) )
    {
       bDoAnyTestOngoing = TRUE;
    }
  }

  return bDoAnyTestOngoing;
}


/*------------------------------------------------------------------------------
**
** doDiag_IsVsupTestOngoing()
**
** Description:
**    check if currently the Voltage Supervision Test is ongoing
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    doAnyHsTestOngoing: TRUE - Currently test ongoing
**                        FALSE - Currently no test ongoing
**
** Usage:
**    boolvariable = doDiag_IsVsupTestOngoing();
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
BOOL doDiag_IsVsupTestOngoing(void)
{
  /* declaration and initialization of return value */
  BOOL bDoVsupTestOngoing = FALSE;

  /* check if any DO is currently in test state (test ongoing) */
  if (DO_DIAG_VSUP_TEST_ONGOING)
  {
     bDoVsupTestOngoing = TRUE;
  }
  else
  {
     bDoVsupTestOngoing = FALSE;
  }
  return bDoVsupTestOngoing;
}


/*------------------------------------------------------------------------------
**
** doDiag_IsTestAtPinOngoing()
**
** Description:
**    Check if currently a test is at a dedicated pin ongoing.
**    Possible is an ongoing Voltage Supervision test or HS-Test.
**
**------------------------------------------------------------------------------
** Inputs:
**    doNum: number/id of output to evaluate
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
BOOL doDiag_IsTestAtPinOngoing(UINT8 u8DoNum)
{
  /* declaration and initialization of return value */
  BOOL bDoTestOngoing = FALSE;

  /* check if DO number is valid */
  DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

  /* check if DO is currently in test state (test ongoing) */
  if ( (DO_DIAG_HS_TEST_ONGOING(u8DoNum)) || (DO_DIAG_VSUP_TEST_ONGOING) )
  {
     bDoTestOngoing = TRUE;
  }

  return bDoTestOngoing;
}


/*------------------------------------------------------------------------------
 **
 ** doDiag_CheckDiagVsupTimeout()
 **
 ** Description:
 **    This function is used to check if the DO diagnostic timeout for VSUP test
 **    is occurred.
 **    The function only checks if the Test-Execute-Variable was set to "eTRUE",
 **    which is done with every test execution. If the Test-Execute-Variable
 **    is eTRUE, the Test-Execute-Variable will be reseted by calling a reset
 **    function. If the Test-Execute-Variable is eFALSE, the test was not
 **    executed and therefore fail-safe is entered.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    - None -
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiag_CheckDiagVsupTimeout()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void doDiag_CheckDiagVsupTimeout(void)
{
   if (doDiagVsup_WasTestExecuted() == eTRUE)
   {
      doDiagVsup_RstTestExecutedVar();
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_DODIAG_ERR);
   }
}


/*------------------------------------------------------------------------------
 **
 ** doDiag_CheckDiagHSTimeout()
 **
 ** Description:
 **    This function is used to check for DO diagnostic timeout of HS-Test.
 **    The function only checks if the Test-Execute-Variable was set to "eTRUE",
 **    which is done with every test execution. If the Test-Execute-Variable
 **    is eTRUE, the Test-Execute-Variable will be reseted by calling a reset
 **    function. If the Test-Execute-Variable is eFALSE, the test was not
 **    executed and therefore fail-safe is entered.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DoNum: number of output to check
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    doDiag_CheckDiagHSTimeout()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void doDiag_CheckDiagHSTimeout(CONST UINT8 u8DoNum)
{
   if (doDiagHs_WasTestExecuted(u8DoNum) == eTRUE)
   {
      doDiagHs_RstTestExecutedVar(u8DoNum);
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_DODIAG_ERR);
   }
}

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of doDiag.c
**
********************************************************************************
*/

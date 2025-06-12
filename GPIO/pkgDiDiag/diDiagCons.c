/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagCons.c
**
** $Id: diDiagCons.c 2448 2017-03-27 13:45:16Z klan $
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
** functions related to the consistency check of the inputs.
** Consistency check is realized as follows:
** If the the state of the two channels (the two HW Inputs) of one dual channel
** safe input differs for a time longer x (with x configurable in the iParameters),
** then the consistency check is considered as "failed".
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

/* Header-file of module */
#include "gpio_cfg.h"
#include "diCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "diErrHdl.h"
#include "diInput.h"

#include "diDiagCons.h"


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
** variable for test execution (used for timeout check)
**------------------------------------------------------------------------------
*/
STATIC TRUE_FALSE_ENUM  diDiagCons_eTestExecuted[GPIOCFG_NUM_DI_PER_uC];


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiagCons_Init()
**
** Description:
**    initialization of the consistency test module
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
**    diDiagCons_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagCons_Init(void)
{
   UINT8 u8Index;

   /* for all digital safe inputs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_PER_uC; u8Index++)
   {
      diDiagCons_eTestExecuted[u8Index] = eFALSE;
   }
}


/*------------------------------------------------------------------------------
**
** diDiagCons_Check()
**
** Description:
**    - checks the consistency of an input to the other channel
**    - calls/informs the DI-Error-Handler about the results
**    details:
**    compares the states of the two channels of a dual channel safe input
**    (one located at uC1, the other at uC2). If the values are not equal, a
**    counter will be increased. If the counter reaches a limit x, then
**    a consistency error will be triggered.
**    If equal, the counter will be reseted to 0.
**    Functionality fulfills [SRS_604]
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DualDiNum: Number of Dual-Channel-DI to process
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diDiagCons_Check(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diDiagCons_Check(CONST UINT8 u8DualDiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   /* if current states of both channels are not equal */
   if (DI_INPUTVAL_DI (DICFG_GET_DI_NUM_C1(u8DualDiNum)) 
         != DI_INPUTVAL_DI (DICFG_GET_DI_NUM_C2(u8DualDiNum)) )
   {
      /* call function to set consistency error, see [SRS_231] */
      diErrHdl_ConsTestFailed(u8DualDiNum);
   }
   else
   {
      diErrHdl_ConsTestOk(u8DualDiNum);
   }

   /* set identifier that the diagnostic was executed */
   diDiagCons_eTestExecuted[u8DualDiNum] = eTRUE;
}


/*------------------------------------------------------------------------------
 **
 ** diDiagCons_WasTestExecuted()
 **
 ** Description:
 **    This function returns the value of the Test-Execute-Variable (which shall
 **    be set whenever the Consistency-Test has been executed) for a dedicated
 **    safe DI.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DualDiNum: Number of Dual-Channel-DI to process
 **
 ** Return:
 **    TRUE_FALSE_ENUM: value of the Test-Execute-Variable
 **
 ** Usage:
 **    x = diDiagCons_WasTestExecuted()
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM diDiagCons_WasTestExecuted (CONST UINT8 u8DualDiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   return (diDiagCons_eTestExecuted[u8DualDiNum]);
}


/*------------------------------------------------------------------------------
 **
 ** diDiagCons_RstTestExecutedVar()
 **
 ** Description:
 **    Resets the Test-Execute-Variable of a dedicated safe DI to "eFALSE"
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DualDiNum: Number of Dual-Channel-DI to process
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    diDiagCons_RstTestExecutedVar(x)
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void diDiagCons_RstTestExecutedVar(CONST UINT8 u8DualDiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   diDiagCons_eTestExecuted[u8DualDiNum] = eFALSE;
}


/*------------------------------------------------------------------------------
 **
 ** diDiagCons_ConsTestToBeExec()
 **
 ** Description:
 **    returns if the consistency test has to be executed or not for the
 **    corresponding DI.
 **    Consistency check has to be executed, if the DI is configured as dual
 **    channel and the consistency filter time different from 0 (which would
 **    be deactivation of consistency filter for dual-channel DI).
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **    u8DualDiNum: Number of Dual-Channel-DI to process
 **
 ** Return:
 **    - None -
 **
 ** Usage:
 **    x = diDiagCons_ConsTestToBeExec(x)
 **
 ** Module Test:
 **    - NO -
 **    Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
TRUE_FALSE_ENUM diDiagCons_ConsTestToBeExec(CONST UINT8 u8DualDiNum)
{
   TRUE_FALSE_ENUM eRetVal = eFALSE;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   /* lint e960 disabled, no critical side effects */
   if (FIPARAM_DI_DUAL_CH(u8DualDiNum) && (0u != fiParam_GetConsFilter(u8DualDiNum))) /*lint !e960*/
   {
      eRetVal = eTRUE;
   }

   return eRetVal;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of diDiagCons.c
**
********************************************************************************
*/

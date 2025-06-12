/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagShortCheck.c
**
** $Id: diDiagShortCheck.c 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** Module contains function to check during a test for a possible short between
** the safe inputs of one uC.
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
#include "diSetGetPin.h"
#include "diInput.h"
#include "diDoDiag.h"
#include "diErrHdl.h"
#include "diDiagShortCheck.h"

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
STATIC GPIO_STATE_ENUM diDiagShortCheck_aeDiValStart[GPIOCFG_NUM_DI_PER_uC];
STATIC GPIO_STATE_ENUM diDiagShortCheck_aeDiValExec[GPIOCFG_NUM_DI_PER_uC];


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diDiagShortCheck_Start()
**
** Description:
**    copies the current state of all safe DIs to a buffer, to be used
**    during test start
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
**    diDiagShortCheck_Start();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diDiagShortCheck_Start(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_PER_uC; u8Index++)
   {
     diDiagShortCheck_aeDiValStart [u8Index] =  DIINPUT_DI_OWN_FIRST_FILTER(u8Index);
   }

}


/*------------------------------------------------------------------------------
**
** diDiagShortCheck_Exec()
**
** Description:
**    copies the current state of all safe DIs to a buffer, to be used
**    during test execution
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
**    diDiagShortCheck_Exec();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diDiagShortCheck_Exec(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_PER_uC; u8Index++)
   {
     diDiagShortCheck_aeDiValExec [u8Index] =  DIINPUT_DI_OWN_FIRST_FILTER(u8Index);
   }

}


/*------------------------------------------------------------------------------
**
** diDiagShortCheck_End()
**
** Description:
**    This function is to be called at the end of the "Level-Test".
**    In this function the three DI input values of the other DIs
**    located are evaluated:
**    - the state before test pulse
**    - the state during the test pulse
**    - the state after the test pulse
**    If the test pulse generated for one DI was received at another DI of the
**    same uC, this is an indicator for a possible short.
**    (See [SRS_274])
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    eTRUE if there is a possible short, eFALSE if not
**
** Usage:
**    diDiagShortCheck_End(y);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diDiagShortCheck_End(CONST UINT8 u8DiNum)
{
   UINT8 u8Index;
   GPIO_STATE_ENUM aeDiValStop[GPIOCFG_NUM_DI_PER_uC];

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* for all digital inputs used */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_PER_uC; u8Index++)
   {
     aeDiValStop [u8Index] =  DIINPUT_DI_OWN_FIRST_FILTER(u8Index);

     if (u8Index != (u8DiNum/2u))
     {
        if ((DICFG_DI_ACTIVE == diDiagShortCheck_aeDiValStart[u8Index])  && \
            (DICFG_DI_INACTIVE == diDiagShortCheck_aeDiValExec[u8Index]) && \
            (DICFG_DI_ACTIVE == aeDiValStop[u8Index]))
        {
           /* potentially a short */
           diErrHdl_ShortTestFailed(u8DiNum);
        }
        else
        {
           diErrHdl_ShortTestOk(u8DiNum);
        }
     }
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
** End of diDiagShortCheck.c
**
********************************************************************************
*/

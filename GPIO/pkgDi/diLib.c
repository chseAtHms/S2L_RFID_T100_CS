/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diLib.c
**
** $Id: diLib.c 2618 2017-04-06 11:58:52Z klan $
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
** contains interface functions:
** - diagnostic timeout check for DI diagnostics
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

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "gpio_cfg.h"
#include "diCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "diDiag.h"
#include "diState.h"
#include "diLib.h"


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
**  This variable contains the time stamp of the last diagnostic timeout check
**------------------------------------------------------------------------------
*/
STATIC RDS_UINT32 diLib_u32RdsDiagTimeoutCheck;



/*******************************************************************************
**
** private Services
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
** diLib_Init()
**
** Description:
**    function for initialization/configuration DI Lib module
**    Currently, only the RDS-Variable used for Diagnostic timeout check is
**    initialized
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
**    diLib_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diLib_Init(void)
{
   UINT32 u32tmp;

   u32tmp = timerHAL_GetSystemTime3();
   RDS_SET(diLib_u32RdsDiagTimeoutCheck, u32tmp);
}


/*------------------------------------------------------------------------------
**
** diLib_CheckDiDiagTimeout()
**
** Description:
**    High-Level function to call the functions for DI diagnostic timeout
**    check:
**    Whenever a time x is expired, the functions to check the execution of
**    the diagnostic tests are called. This is only done for active outputs
**    (inactive inputs are not tested because there is no diagnostic).
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diLib_CheckDiDiagTimeout()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**
** Context: main, while(FOREVER)
**------------------------------------------------------------------------------
*/
void diLib_CheckDiDiagTimeout(void)
{
   UINT8 u8Index;
   UINT32 u32SysTime;
   UINT32 u32TimeDiff;

   /* get current system time and saved "last time" of check */
   __disable_irq();
   u32SysTime = timerHAL_GetSystemTime3();
   u32TimeDiff = RDS_GET(diLib_u32RdsDiagTimeoutCheck);
   __enable_irq();

   /* check when the last diagnostic timeout check was executed */
   u32TimeDiff = u32SysTime - u32TimeDiff;

   /* time for next check reached? */
   if (u32TimeDiff >= DIDIAG_TIMEOUT)
   {
      /* call check function of DI-Test for active inputs */
      for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
      {
         if (DICFG_DI_ACTIVE == diState_GetDiState(u8Index))
         {
            diDiag_CheckDiagTimeout(u8Index);
         }
      }

      /* set time of current diagnostic timeout-check for next interval */
      /* ATTENTION: RDS variable is also written by function diLib_Init()
      ** (IRQ context). Since RDS variables are not atomic, it must be ensured
      ** that this function is not called at the same time with diLib_Init(). 
      */
      RDS_SET(diLib_u32RdsDiagTimeoutCheck, u32SysTime);
   }
}


/*------------------------------------------------------------------------------
**
** diLib_SetErrRstFlags()
**
** Description:
**    The requested state of the error reset flags is received inside the safe
**    packet. This function is called by the Communication/AIC module (which
**    receives the safe packet) to:
**    - set the requested value for the DO error reset flags
**    (see [SRS_2011])
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiErrRstByte: error reset Byte, received via safety protocol
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diLib_SetErrRstFlags(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void diLib_SetErrRstFlags(CONST UINT8 u8DiErrRstByte)
{
   UINT8 u8Index;

   /* do for all DIs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* is DI in dual channel mode? */
      if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8Index)))
      {
         /* in dual channel mode, process only every second bit, see [SRS_2053] */
         if (DIDODIAG_IS_VAL_EQUAL(u8Index))
         {
            if (u8DiErrRstByte & (UINT8)(0x01u << u8Index))
            {
               diState_DiErrRstSPDU[u8Index] = eTRUE;
               diState_DiErrRstSPDU[DICFG_GET_OTHER_CHANNEL(u8Index)] = eTRUE;
            }
            else
            {
               diState_DiErrRstSPDU[u8Index] = eFALSE;
               diState_DiErrRstSPDU[DICFG_GET_OTHER_CHANNEL(u8Index)] = eFALSE;
            }
         }
         else
         {
            /* nothing to do, because every 2 DIs make one dual channel DI */
         }
      }
      /* single channel mode */
      else
      {
         /* process every bit of SPDU packet in single channel mode, see [SRS_2053] */
         if (u8DiErrRstByte & (UINT8)(0x01u << u8Index))
         {
            diState_DiErrRstSPDU[u8Index] = eTRUE;
         }
         else
         {
            diState_DiErrRstSPDU[u8Index] = eFALSE;
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
** End of diLib.c
**
********************************************************************************
*/

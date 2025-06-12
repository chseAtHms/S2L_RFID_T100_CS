/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** clkCtrl.c
**
** $Id: clkCtrl.c 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Module to control and compare the oscillator clock deviation between the two
** channels.
** The algorithm is as follows:
** - Measure the time from a starting point and calculate the elapsed time
** - Send the result to the other channel via IPC
** - Receive the result from the other channel via IPC
** - If the elapsed time is greater than one Clock-Control cycle (defined by
**   CLKCTRL_TIMEDIFF) the two values shall be compared
** - If the values of the two channels differ 0.8% or more, fail safe mode shall
**   be entered
** - Otherwise the algorithm restarts by measuring the next cycle in the same
**   way
**
** Requirements for usage:
** The subsequent function calls of clkCtrl_CalcAndSendTimeDiff() and
** clkCtrl_CheckTimerVal() must be performed at least once within the defined
** Clock-Control cycle. clkCtrl_CalcAndSendTimeDiff() must be called at least
** one time slice before clkCtrl_CheckTimerVal(). Since
** clkCtrl_CalcAndSendTimeDiff() must measure the elapsed time for each channel
** as much as possible at the same time, its function call must be the first one
** in the time slice.
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

/* system includes */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

#include "clkCtrl.h"
#include "clkCtrl_cfg.h"

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
**  This variable contains the system time at the start of clock control
**  measurement.
**------------------------------------------------------------------------------
*/
STATIC RDS_UINT32 clkCtrl_u32RdsSysTimeStart;

/*------------------------------------------------------------------------------
**  This variable contains the elapsed time (difference between current system
**  time and system time at start of clock control measurement) during clock
**  control measurement.
**------------------------------------------------------------------------------
*/
STATIC RDS_UINT32 clkCtrl_u32RdsSysTimeDiff;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** clkCtrl_Init()
**
** Description:
**    initialization of CLK-Control Module, gets the start value of the
**    system timer for later use
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
** Module Test:
**    - NO -
**    Reason: low complexity
**------------------------------------------------------------------------------
*/
void clkCtrl_Init (void)
{
   UINT32 u32CurrTime;

   /* determine the time stamp for the first start of clock control
    * measurement
    */
   u32CurrTime = timerHAL_GetSystemTime3();
   RDS_SET(clkCtrl_u32RdsSysTimeStart, u32CurrTime);

   RDS_SET(clkCtrl_u32RdsSysTimeDiff, 0u);
}


/*------------------------------------------------------------------------------
**
** clkCtrl_CalcAndSendTimeDiff()
**
** Description:
**    The function does the following steps:
**    - Measure the time from a starting point and calculate the elapsed time
**    - Send the result to the other channel via IPC
**    - Store current time difference since last check
**    (see also [SRS_406])
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
** Module Test:
**    - NO -
**    Reason: low complexity
**------------------------------------------------------------------------------
*/
void clkCtrl_CalcAndSendTimeDiff (void)
{
   UINT32 u32SysTimeDiff;
   UINT32 u32SysTime;

   /* get current system time */
   u32SysTime = timerHAL_GetSystemTime3();

   /* calculate elapsed time */
   u32SysTimeDiff = u32SysTime - ((UINT32)(RDS_GET(clkCtrl_u32RdsSysTimeStart)));  /*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False, Problem is in RDS_GET. */


   /* call function to transmit the elapsed time via IPC */
   ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_CLKCTRL, u32SysTimeDiff);

   /* store current time difference since last check */
   RDS_SET(clkCtrl_u32RdsSysTimeDiff, u32SysTimeDiff);

}


/*------------------------------------------------------------------------------
**
** clkCtrl_CheckTimerVal()
**
** Description:
**    The function does the following steps:
**    - Receive the result from the other channel via IPC
**    - If the elapsed time is greater than one Clock-Control cycle (defined by
**      CLKCTRL_TIMEDIFF) the two values shall be compared
**    - If the values of the two channels differ 0.8% or more, fail safe mode
**      shall be entered
**    - Otherwise the algorithm restarts by measuring the next cycle in the
**      same way
**    (see also [SRS_406])
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
** Module Test:
**    - NO -
**    Reason: low complexity
**------------------------------------------------------------------------------
*/
void clkCtrl_CheckTimerVal (void)
{
   UINT32 u32SysTimeDiffRx;
   UINT32 u32SysTimeDiffTx;
   UINT32 u32SysTimeDiffDevMax;

   /* read saved system time difference of own channel */
   u32SysTimeDiffTx = (UINT32)(RDS_GET(clkCtrl_u32RdsSysTimeDiff));  /*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False, Problem is in RDS_GET. */

   /* get received system time difference from other channel via IPC */
   u32SysTimeDiffRx = ipcxSYS_GetUINT32inclWait( IPCXSYS_IPC_ID_CLKCTRL );

   /* time for next clock control measurement reached (see [SRS_701])?
    * Then compare values with other channel
    */
   if (  (u32SysTimeDiffTx >= CLKCTRL_TIMEDIFF) \
      && (u32SysTimeDiffRx >= CLKCTRL_TIMEDIFF))
   {
      if (u32SysTimeDiffTx >= u32SysTimeDiffRx)
      {
         /* Calculate the max. tolerated deviation from the min. system time
          * difference, since the smaller system time difference is assumed
          * to be the correct one, if only a too fast clock should be diagnosed
          * by the clock control measurement.
          */
         u32SysTimeDiffDevMax = CLKCTRL_CALC_DEVIATION(u32SysTimeDiffRx);

         /* check the max. system time difference against the max. tolerated
          * deviation
          */
         if (u32SysTimeDiffTx > (u32SysTimeDiffRx + u32SysTimeDiffDevMax))
         {
            GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_CLK_CTRL);
         }
      }
      else
      {
         /* calculate the max. tolerated deviation from the min. system time
          * difference, since the smaller system time difference is assumed
          * to be the correct one, if only a too fast clock should be diagnosed
          * by the clock control measurement.
          */
         u32SysTimeDiffDevMax = CLKCTRL_CALC_DEVIATION(u32SysTimeDiffTx);

         /* check the max. system time difference against the max. tolerated
          * deviation
          */
         if (u32SysTimeDiffRx > (u32SysTimeDiffTx + u32SysTimeDiffDevMax))
         {
            GLOBFAIL_SAFETY_FAIL(GlOB_FAILCODE_CLK_CTRL);
         }
      }

      /* store system time of current clock control measurement for next
       * interval of clock control measurement
       */
      RDS_ADD(clkCtrl_u32RdsSysTimeStart, u32SysTimeDiffTx);
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
** End of clkCtrl.c
**
********************************************************************************
*/

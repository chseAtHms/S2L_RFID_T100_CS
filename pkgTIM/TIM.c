/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: TIM.c
**     Summary: This file contains functions for providing a synchronized 128us timer, as the
**              CIP Safety SW of both controllers shall work with the same timer value.
**   $Revision: 2329 $
**       $Date: 2017-03-13 07:50:11 +0100 (Mo, 13 Mrz 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: TIM_Init
**             TIM_SendTimerVal
**             TIM_CheckAndSetTimerVal
**             TIM_TimeGet_128us
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "timer-def.h"

#include "timer-hal.h"
#include "cfg_Config-sys.h"

#include "rds.h"
#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"


/* module include */
#include "TIM.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      29u

/* The exchange of the 128us time stamp is performed once within T-cyc:asm (~4ms). Since the
** time stamp depends on the sampling time, the tolerance of the T-cyc;asm shall also be
** considered here. According to [SRS_2234] we can expect the sampling between 3600us and 4400us.
** (T-cyc:asm +-10%). Since the timer deviation check shall not restrict this specified range, the 
** valid range for timer deviations is extended to 3584us..4480us here.
*/
#define k_MIN_TIMER_DIFF_TCYCL  28u /* (28 x 128us = 3584us) */
#define k_MAX_TIMER_DIFF_TCYCL  35u /* (35 x 128us = 4480us) */

/* This variable contains the current local time stamp captured by HW timer */
STATIC RDS_UINT32 u32_LocalTimestampCurrCycl;

/* This variable contains the last time stamp of the local uc */
STATIC volatile RDS_UINT32 u32_LocalTimestampPrevCycl;

/* This variable contains the last time stamp of the remote uc */
STATIC volatile RDS_UINT32 u32_RemoteTimestampPrevCycl;

/* This variable indicates if this is the first exchange */
STATIC TRUE_FALSE_ENUM e_FirstRun = eTRUE;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    TIM_Init

  Description:
    This function is used to initialize the CSS system timer. 

  See also:
    -

  Parameters:
   -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void TIM_Init(void)
{
  /* Attention: Function called before IRQ Scheduler started. So disabling IRQ (before accessing
  ** variables) is not necessary here. */

  RDS_SET(u32_LocalTimestampCurrCycl, 0u);
  RDS_SET(u32_LocalTimestampPrevCycl, 0u);
  RDS_SET(u32_RemoteTimestampPrevCycl, 0u);
}

/***************************************************************************************************
  Function:
    TIM_SendTimerVal

  Description:
    This function is used to send the current value of the application timer (128us) to the other
    controller via IPC. 

  See also:
    -

  Parameters:
   -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void TIM_SendTimerVal(void)
{
  /* local 128us HW timer value */
  UINT32 u32_timestamp;

  /* get current system time from HW timer */
  u32_timestamp = timerHAL_GetSystemTime4();
   
  /* call function to transmit the current system time via IPC */
  ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_APPCLKCTRL, u32_timestamp);
   
  /* store current local time */
  RDS_SET(u32_LocalTimestampCurrCycl, u32_timestamp);
}


/***************************************************************************************************
  Function:
    TIM_CheckAndSetTimerVal

  Description:
    This function is used to receive the timer value from other controller via IPC. Then the 
    difference between current timer value and last timer value call is determined and checked 
    if the value is in expected range. This is done for local and remote timer value.
    If the deviation is more than expected, the failsafe state is entered.

  See also:
    -

  Parameters:
   -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void TIM_CheckAndSetTimerVal(void)
{
  /* time stamp received via IPC for current T-cyc (remote time stamp) */
  UINT32 u32_rxTimestamp;
  /* time stamp sent via IPC for current T-cyc (local time stamp) */
  UINT32 u32_txTimestamp;
  
  /* difference between last and current time stamp (remote) */
  UINT32 u32_rxTimestampDiff;
  /* difference between last and current time stamp (local) */
  UINT32 u32_txTimestampDiff;
  
  /* read saved system time of own channel */
  u32_txTimestamp = (UINT32)(RDS_GET(u32_LocalTimestampCurrCycl));
  /* get received time stamp via IPC */
  u32_rxTimestamp = ipcxSYS_GetUINT32inclWait( IPCXSYS_IPC_ID_APPCLKCTRL );

  /* if not first run -> first exchange of time stamp values already done */
  if ( e_FirstRun == eFALSE )
  {
    /* calculate differences for max. deviation check */
    u32_rxTimestampDiff = u32_rxTimestamp - 
                         (UINT32)(RDS_GET(u32_RemoteTimestampPrevCycl));
    u32_txTimestampDiff = u32_txTimestamp - 
                         (UINT32)(RDS_GET(u32_LocalTimestampPrevCycl));

    /* if remote max. deviation is exceeded */
    if ( (u32_rxTimestampDiff > k_MAX_TIMER_DIFF_TCYCL) ||
         (u32_rxTimestampDiff < k_MIN_TIMER_DIFF_TCYCL) )
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_APPL_TIMER_DEV_ERR, GLOBFAIL_ADDINFO_FILE(1u));
    }
    /* else: if local max. deviation is exceeded */
    else if ( (u32_txTimestampDiff > k_MAX_TIMER_DIFF_TCYCL) ||
              (u32_txTimestampDiff < k_MIN_TIMER_DIFF_TCYCL) )
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_APPL_TIMER_DEV_ERR, GLOBFAIL_ADDINFO_FILE(2u));
    }
    /* else: check successful */
    else
    {
      /* store local time stamp for next check in next cycle */
      RDS_SET(u32_LocalTimestampPrevCycl, u32_txTimestamp);
      /* store remote time stamp for next check in next cycle */
      RDS_SET(u32_RemoteTimestampPrevCycl, u32_rxTimestamp);
    }
  }
  /* else if: first run, no time stamp exchanged yet */
  else if ( e_FirstRun == eTRUE )
  {
    /* do not perform the check during first call, because the "previous timestamp" is
    ** not yet available */

    /* store local time stamp for next check in next cycle */
    RDS_SET(u32_LocalTimestampPrevCycl, u32_txTimestamp);
    /* store remote time stamp for next check in next cycle */
    RDS_SET(u32_RemoteTimestampPrevCycl, u32_rxTimestamp);

    /* update variable */
    e_FirstRun = eFALSE;

  }
  /* else: variable error  */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
  }
}



/***************************************************************************************************
  Function:
    TIM_TimeGet_128us

  Description:
    This function returns the current synchronized system time (128us time stamp).
    The system time is the real time incremented in 128us ticks.
    In this implementation always the time stamp of SAFETY_CONTROLLER_1 is used!

  See also:
    -

  Parameters:
   -

  Return value:
    UINT32 - system tick

  Remarks:
    Context: Background Task

***************************************************************************************************/
UINT32 TIM_TimeGet_128us(void)
{
  UINT32 u32_timerValue;
  
  /* controller id */
  CFG_CONTROLLER_ID_ENUM  eControllerId = cfgSYS_GetControllerID();
  
  /* Attention: Always time stamp of controller 1 is selected */
  /* if controller 1 */
  if (eControllerId == SAFETY_CONTROLLER_1)
  {
    /* take local time stamp */
    
    __disable_irq();
    /* read saved timer value */
    u32_timerValue = (UINT32)(RDS_GET(u32_LocalTimestampPrevCycl));
    __enable_irq();
  }
  /* else: controller 2 */
  else if (eControllerId == SAFETY_CONTROLLER_2)
  {
    /* take remote time stamp */
    
    __disable_irq();
    /* read saved timer value */
    u32_timerValue = (UINT32)(RDS_GET(u32_RemoteTimestampPrevCycl));
    __enable_irq();
  }
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
    /* set return value, only used for unit test */
    u32_timerValue = 0u;
  }
  
  return u32_timerValue;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdMain.c
**     Summary: This module implements the background task. 
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskTrigger
**             SAPL_BkgdTaskCheck
**             SAPL_BkgdTaskExec
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

#include "aicSm.h"

#include "SAPLbkgdMain.h"
#include "SAPLbkgdInt.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      24u

/** k_BKGD_ACTIVITY_TIMEOUT_US
** The background task monitors the IRQ scheduler. This is done if the background task is activated
** cyclically within this timeout. If this timeout is exceeded, the fail safe state is entered.
** It is expected that all IRQ functionality is finished within T-cyc;asm +/-10% and so the 
** background task is activated at least after that time.
*/
#define k_BKGD_ACTIVITY_TIMEOUT_US    ((UINT32)4400U) /* 4.4ms */


/* e_BkgdTaskActivateSema:
** This variable acts as 'semaphore' and is used to trigger the execution of the background task 
** from the IRQ Scheduler. The semaphore is set via SAPL_BkgdTaskTrigger() and reset by the 
** background task (SAPL_BkgdTaskExec()) when finished.
*/
STATIC volatile TRUE_FALSE_ENUM e_BkgdTaskActivateSema = eFALSE;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

  
/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    SAPL_BkgdTaskTrigger

  Description:
    This function is used to trigger the execution of a new background cycle.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPL_BkgdTaskTrigger(void)
{  
  /* if semaphore was not reset by background task */
  if (e_BkgdTaskActivateSema != eFALSE)
  {
    /* call globFail_SafetyHandler, never return from this... */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_BKGD_DELAY_ERR, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: everything fine */
  else
  {
    /* set semaphore */
    e_BkgdTaskActivateSema = eTRUE;
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskCheck

  Description:
    This function is used to ensure the background cycle is already finished (hard deadline).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPL_BkgdTaskCheck(void)
{
  /* if background task is not finished yet */
  if (e_BkgdTaskActivateSema != eFALSE)
  {
    /* call globFail_SafetyHandler, never return from this... */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_BKGD_DELAY_ERR, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: background task finished */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskExec

  Description:
    This function executes the different background tasks depending on the current AIC state.
    Before the background task is executing, the function waits until triggered by IRQ Scheduler
    via SAPL_BkgdTaskTrigger. If SAPL_BkgdTaskTrigger is not called within 
    k_BKGD_ACTIVITY_TIMEOUT_US the FAILSAFE state is entered.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main

***************************************************************************************************/
void SAPL_BkgdTaskExec(void)
{
  
  /* used to detect missing IRQ scheduler trigger */
  UINT32 u32_startTimeout;
  UINT32 u32_timeout;
  /* current AIC state */
  AICSM_STATE_ENUM e_aicState;
  
  /* get the actual system time for background task timeout calculation */
  __disable_irq();
  u32_startTimeout = timerHAL_GetSystemTime3();
  __enable_irq();
  
  /* while loop: wait until a new cycle (4ms) is triggered by IRQ Scheduler via 
  ** SAPL_BkgdTaskTrigger() */
  /* read access to 'e_BkgdTaskActivateSema' is considered as 'atomic'. */
  while (e_BkgdTaskActivateSema != eTRUE)
  {
    /* background task timeout calculation */
    __disable_irq();
    u32_timeout = timerHAL_GetSystemTime3() - u32_startTimeout;
    __enable_irq();
        
    /* if timeout occurred */
    if (u32_timeout > k_BKGD_ACTIVITY_TIMEOUT_US)
    {
      /* call globFail_SafetyHandler, never return from this... */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_BKGD_IRQ_MONITOR_ERR, GLOBFAIL_ADDINFO_FILE(3u));
    }
    /* no timeout (yet) */
    else
    {
      /* empty branch */
    }
  }
  
  /* e_BkgdTaskActivateSema is NOT checked against eFALSE here because it is very unlikely that
  ** e_BkgdTaskActivateSema is changed to eTRUE due to a soft-error. 
  ** In case of an soft-error, we would stay in the while-loop until e_BkgdTaskActivateSema 
  ** is overwritten with eTRUE or a timeout occurs. 
  */
  
  /* read current AIC state used for Background Task, variable is accessed by IRQ Scheduler Task 
  ** but read access is considered as 'atomic' here */
  e_aicState = aicSm_eAicState;
      
  /* background task depends on AIC state */
  switch (e_aicState)
  {
    case AICSM_AIC_STARTUP:          /* fall through */
    case AICSM_AIC_WAIT_FOR_INIT:    /* fall through */
    {
      /* no special task sequence necessary here, simple discard HALC Messages received
      ** via AIC */
      SAPL_BkgdComDiscardHalcMsg();
      break;
    }
    case AICSM_AIC_START_CSS:
    {
      /* call init task sequence */
      SAPL_BkgdTaskInit();
      break;
    }
    case AICSM_AIC_WAIT_FOR_CONFIG:  /* fall through */
    case AICSM_AIC_PROCESS_CONFIG:   /* fall through */
    case AICSM_AIC_INVALID_CONFIG:   /* fall through */
    {
      /* call config task sequence */
      SAPL_BkgdTaskConfig();
      break;
    }
    case AICSM_AIC_IDLE:
    {
      /* call idle task sequence */
      SAPL_BkgdTaskIdle();
      break;
    }
    case AICSM_AIC_EXEC_PROD_ONLY:   /* fall through */
    case AICSM_AIC_EXEC_CONS_ONLY:   /* fall through */
    case AICSM_AIC_EXEC_PROD_CONS:   /* fall through */
    {
      /* Safety PDUs are only processed in EXEC states (see [SIS_017]). This means that the
      ** CIP Safety I/O Connection is already established. */
      /* call executing task sequence */
      SAPL_BkgdTaskExecuting();
      break;
    }
    case AICSM_AIC_WAIT_TUNID:
    {
      /* call wait TUNID task sequence */
      SAPL_BkgdTaskTunid();
      break;
    }
    case AICSM_AIC_WAIT_RESET:
    {
      /* call reset task sequence */
      SAPL_BkgdTaskReset();
      break;
    }
    case AICSM_AIC_ABORT:
    {
      /* call abort task sequence */
      SAPL_BkgdTaskAbort();
      break;
    }
    default:
    {
      /* call globFail_SafetyHandler, never return from this... */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
      break;
    }
  }
  
  /* write access to 'e_BkgdTaskActivateSema' is considered as 'atomic'. */
  e_BkgdTaskActivateSema = eFALSE;
  
/* This function has a lot of paths because all possible AIC states are catched inside the 
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

/***************************************************************************************************
**    static functions
***************************************************************************************************/


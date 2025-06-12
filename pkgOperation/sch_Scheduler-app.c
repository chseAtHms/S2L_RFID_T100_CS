/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: sch_Scheduler-app.c
**     Summary: This module contains the scheduler which executes functions according to the 
**              specified program flow.
**   $Revision: 4436 $
**       $Date: 2024-05-17 16:28:50 +0200 (Fr, 17 Mai 2024) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: schAPP_TriggerMainExecTime
**             schAPP_Init
**             schAPP_StartScheduler
**             schAPP_StopScheduler
**             schAPP_TriggerCycleStatistic
**             TIM1_UP_IRQHandler
**
**             CheckSchedulerTimeout
**             CheckMainExecution
**             timeSlicesStartup
**             timeSlicesRunMode
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

/* Header-file of module */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "ipcs_IpcSync-sys.h"
#include "ipcx_ExchangeData-sys.h"

/* include "packet" for DI/DO module interface functions */
#include "includeDiDo.h"

#include "stHan_SelftestHandler-srv.h"
#include "tm_TemperatureMonitor-srv.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicMgrRx.h"
#include "aicMgrTx.h"
#include "aicSm.h"
#include "clkCtrl.h"

#include "spduIn.h"

#include "globFit_FitTestHandler.h"

/* CSOS common headers */
#include "CSOScfg.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"


#include "TIM.h"
#include "SAPLipc.h"
#include "SAPLbkgdMain.h"
#include "SAPLnvLow.h"
#include "SAPLappObj.h"

/* Header-file of module */
#include "sch_Scheduler-app.h"

#ifdef RFID_ACTIVE
  #include "RFID.h"
#endif /* RFID_ACTIVE */

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      15u

/*! Number of task blocks. */
#define SCHEDULER_BLOCK_COUNT       40u

/*! Index of first block to execute. */
#define SCHEDULER_BLOCK_START_IDX   0u

#if (SCHEDULER_BLOCK_COUNT <= SCHEDULER_BLOCK_START_IDX)
   #error "Invalid SCHEDULER_BLOCK_START_IDX!"
#endif


/*! This constant defines the timeout for the scheduler, the unit is in
 * microseconds, see [SRS_2234] */
#define SCHEDULER_TIMEOUT_MAX_US  ((UINT32)4400u) /* microseconds */
#define SCHEDULER_TIMEOUT_MIN_US  ((UINT32)3600u) /* microseconds */

/*! This constant defines the timeout for the main execution check, the unit is in
 * microseconds. */
#define SCHEDULER_MAIN_TIMEOUT_US  ((UINT32)600000000u) /* us --> 10 min */


/*! Index of current task block. */
STATIC volatile UINT8 u8_BlockIdx = SCHEDULER_BLOCK_START_IDX;

/*  This variable contains the time stamp of the last main trigger time */
STATIC volatile RDS_UINT32 u32_RdsMainLoopTriggerTime;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void timeSlicesStartup (void);
STATIC void timeSlicesRunMode (void);
STATIC void CheckSchedulerTimeout (void);
STATIC void CheckMainExecution (void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    schAPP_TriggerMainExecTime

  Description:
    This function triggers the main timeout.
    The function fetches the current system time and writes it to the variable
    "u32RdsMainLoopTriggerTime" which is used for the timeout calculation.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)
    Context: main, while(FOREVER)

***************************************************************************************************/
void schAPP_TriggerMainExecTime (void)
{
  UINT32 u32_sysTime;

  /* interrupts disabled because this function is called also outside the
   * scheduler interrupt by main, and the RDS access is not atomic */
  __disable_irq();
  u32_sysTime = timerHAL_GetSystemTime3();
  RDS_SET(u32_RdsMainLoopTriggerTime, u32_sysTime);
  __enable_irq();
}

/***************************************************************************************************
  Function:
    schAPP_Init

  Description:
    This function initializes the scheduler timer but does not start it yet.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void schAPP_Init (void)
{
  TIMERDEF_TIMER_IRQ_CONFIG_STRUCT s_timerConfig;

  /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number 
  ** Accepted, Timer Configuration, already used in T100/PS */
  
  /* init timer to get 100us time slices (40 of them fulfill the cycle time, see [SRS_2234]) */
  s_timerConfig.eTimer             = TIMERDEF_TIMER1;
  s_timerConfig.pThis              = &s_timerConfig;
  /* prescaler will divide 72MHz Frequency by (value+1) => 72MHz/(99+1) = 720kHz  */
  s_timerConfig.u16Prescaler       = (UINT16)99u; 
  /* scheduler frequency: when prescaler=99, then 72 steps are necessary to get 100us steps 
  ** (0...71 => 72 steps => reload value 71) */
  s_timerConfig.u16AutoReloadValue = (UINT16)71u; 
  s_timerConfig.u16CounterValue    = (UINT16)0u;
  s_timerConfig.u8IrqPriority      = (UINT8)2u;
  /* RSM_IGNORE_END */

  timerHAL_InitIrq(&s_timerConfig);

  /* initialization of variable used for check of the main loop execution by using the
   * timeout trigger function */
  schAPP_TriggerMainExecTime();

} /* end of schAPP_Init() */

/***************************************************************************************************
  Function:
    schAPP_StartScheduler

  Description:
    This function starts the scheduler timer.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void schAPP_StartScheduler (void)
{
  /* initialize and start watchdog, see [SRS_355] */
  timerHAL_StartWwdg();

  /* start scheduler timer */
  timerHAL_StartTimerIrq(TIMERDEF_TIMER1);

} /* end of schAPP_StartScheduler() */


#ifdef GLOBFIT_FITTEST_ACTIVE
/***************************************************************************************************
  Function:
    schAPP_StopScheduler

  Description:
    This function stops the scheduler timer.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    -
***************************************************************************************************/
void schAPP_StopScheduler (void)
{
  /* stop the scheduler timer */
  timerHAL_StopTimerIrq ( TIMERDEF_TIMER1 );

} /* end of schAPP_StopScheduler() */ /* CCT_SKIP Only for Failure Insertion Test */
#endif


/***************************************************************************************************
  Function:
    schAPP_TriggerCycleStatistic

  Description:
    This function is used to measure the max time slice (block index) touched by background task.
    The function shall be called if background task is finished.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main, while(FOREVER)

***************************************************************************************************/
void schAPP_TriggerCycleStatistic (void)
{
  LOCAL_STATIC(, UINT8, u8_MaxTimeSlice, 0u);
  
  /* current time slice in which this function is called, variable is written by IRQ Scheduler 
  ** but read access is considered as 'atomic' here */
  UINT8 u8_currTimeSlice = u8_BlockIdx;

  /* if new max value reached */
  if (u8_currTimeSlice > u8_MaxTimeSlice)
  {
    /* update max value */
    u8_MaxTimeSlice = u8_currTimeSlice;
    /* and set inside Diagnostic Object */
    SAPL_AppObjDiagMaxTimeSliceSet(u8_MaxTimeSlice);
  }
  /* else: still old max value */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    TIM1_UP_IRQHandler

  Description:
    This function is the scheduler.
    Interrupt service routine of the scheduler timer. Performs the actual scheduling functionality. 
    The software functionality is separated by single, distinct tasks.
    Tasks are organized in blocks. Each block may contain several tasks in a given sequential order.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    -
***************************************************************************************************/
void TIM1_UP_IRQHandler (void)
{
  /* FIT to test the watchdog, with this FIT the watchdog is not triggered */
  /* RSM_IGNORE_QUALITY_BEGIN Notice #9    - 'goto' keyword identified 
  ** only used for FIT test */
  GLOBFIT_FITTEST_NOCMDRESET(GLOBFIT_CMD_WATCHDOG, goto _FitOverJumpWdg);
  /* RSM_IGNORE_END */

  /* trigger watchdog */
  TIMERHAL_TRIGGER_WWDG

#ifdef GLOBFIT_FITTEST_ACTIVE
  _FitOverJumpWdg: /* CCT_NO_PRE_WARNING, only used for FIT test */
#endif

  /* clear irq flag */
  timerHAL_ClearTimerIrq(TIMERDEF_TIMER1);

  /* use "fast" synchronization by GPIOs, [SRS_317] */
  ipcsSYS_Sync();

  /* clear scheduler timer counter for a better synchronization */
  timerHAL_ClearTimer1Counter();

/******************************************************
 * Start of tasks
 ******************************************************/

  /* sample all DIs at every function call */
  gpio_SamplePorts();


  /* depending on current state different scheduler has to be called, also see [SRS_2001] */
  switch (aicSm_eAicState)
  {
    case AICSM_AIC_EXEC_PROD_ONLY:  /* fall through */
    case AICSM_AIC_EXEC_CONS_ONLY:  /* fall through */
    case AICSM_AIC_EXEC_PROD_CONS:  /* fall through */
    case AICSM_AIC_IDLE:            /* fall through */
    {
      timeSlicesRunMode();
      break;
    }

    case AICSM_AIC_STARTUP:         /* fall through */
    case AICSM_AIC_WAIT_FOR_INIT:   /* fall through */
    case AICSM_AIC_START_CSS:       /* fall through */
    case AICSM_AIC_WAIT_TUNID:      /* fall through */
    case AICSM_AIC_WAIT_FOR_CONFIG: /* fall through */
    case AICSM_AIC_PROCESS_CONFIG:  /* fall through */
    case AICSM_AIC_INVALID_CONFIG:  /* fall through */
    case AICSM_AIC_WAIT_RESET:      /* fall through */
    case AICSM_AIC_ABORT:           /* fall through */
    {
      timeSlicesStartup();
      break;
    }

    default:
    {
      /* error case, should never enter here */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
  }
  
  /* set block index to next time slice */
  u8_BlockIdx++;

  /* reset block index if last time slice reached, the maximum time is defined by the cycle time
   * of 4ms, see [SRS_2234]
   * In case of a falsification (u8_BlockIdx > SCHEDULER_BLOCK_COUNT), the SafetyHandler is 
   * called during next cycle inside timeSlicesRunMode() respectively timeSlicesStartup() */
  if (SCHEDULER_BLOCK_COUNT == u8_BlockIdx)
  {
    u8_BlockIdx = SCHEDULER_BLOCK_START_IDX;
  }
  else
  {
    /* empty branch, last time slice not reached yet */
  }
/* This function has a lot of paths because of all possible AIC states catched inside
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
} /* end of TIM1_UP_IRQHandler */
/* RSM_IGNORE_QUALITY_END */

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    CheckSchedulerTimeout

  Description:
    This function checks if a scheduler timeout occurs. It fetches the system time and the 
    scheduler trigger time. When a timeout is detected, the Safety Handler is called.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void CheckSchedulerTimeout (void)
{
  UINT32 u32_currentTime;
  LOCAL_STATIC(, UINT32, u32_LastTime, 0u); /* no RDS needed, because refreshed once per cycle */
  UINT32 u32_elapsedTime;
  LOCAL_STATIC(, TRUE_FALSE_ENUM, e_FirstRun, eTRUE);

  u32_currentTime = timerHAL_GetSystemTime3(); /* in us */
  /* FIT to test the timeout-check for the scheduler. With this FIT the
   * selftests are not executed and a timeout shall be detected after
   * the time T-cyc:asm = (3600..4400)us. */
  GLOBFIT_FITTEST( GLOBFIT_CMD_TCYC_HI, (u32_currentTime = u32_LastTime + 4401u));
  GLOBFIT_FITTEST( GLOBFIT_CMD_TCYC_LO, (u32_currentTime = u32_LastTime + 3599));    
  
  switch (e_FirstRun)
  {
    case eFALSE:
    {
      /* calculate elapsed time since last call */
      u32_elapsedTime = u32_currentTime - u32_LastTime;
      
      /* if time above or below limit, enter fail safe mode, see [SRS_2234] */
      if ((u32_elapsedTime > SCHEDULER_TIMEOUT_MAX_US) || \
          (u32_elapsedTime < SCHEDULER_TIMEOUT_MIN_US) )
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_SCHEDULER_TIMEOUT, GLOBFAIL_ADDINFO_FILE(2u));
      }
      /* else: time OK */
      else
      {
        /* empty branch */
      }
      break;
    }

    case eTRUE:
    {
      /* do not perform the check during first call, because the "last time" is
       * not yet available */
      e_FirstRun = eFALSE;
      break;
    }

    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
      break;
    }
  }

  u32_LastTime = u32_currentTime;
}

/***************************************************************************************************
  Function:
    CheckMainExecution

  Description:
    checks the execution of main loop: The main loop shall be executed at least once every 
    10 minutes

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void CheckMainExecution (void)
{
  UINT32 u32_sysTime;
  UINT32 u32_timeMain;
  UINT32 u32_timeDiff;

  /* get current system time and saved "last time" of check */
  u32_sysTime = timerHAL_GetSystemTime3();
  u32_timeMain = RDS_GET(u32_RdsMainLoopTriggerTime);

  /* check when the last main execution was triggered */
  u32_timeDiff = u32_sysTime - u32_timeMain;

  /* time for next check reached? */
  if (u32_timeDiff > SCHEDULER_MAIN_TIMEOUT_US)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_MAIN_TIMEOUT, GLOBFAIL_ADDINFO_FILE(4u));
  }
  /* else: timeout not reached, everything fine */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    timeSlicesStartup

  Description:
    executes the different tasks for all time-slices in startup mode

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void timeSlicesStartup (void)
{ 
  /* Execute tasks in startup mode */
  switch (u8_BlockIdx)
  {
    case 0:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 1:
    {
      /* do nothing in this time slice */
      break;
    }

    case 2:
    {
      /* read HW counter/timer and send to other controller via IPC */
      TIM_SendTimerVal();
    
      /* do some selftests */
      stHan_DoSelfTests();
      break;
    }

    case 3:
    {
      aicMgrRx_CheckForNewTgm();  /* see [SRS_2235] */
    
      /* check HW timer/counter and set global systemtime used by both controllers */ 
      TIM_CheckAndSetTimerVal();

      /* read softerror variable and CSS path */
      SAPLipc_SetStackVarAndPath();
      break;
    }

    case 4:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      aicMgrRx_PrepAicSync();
      gpio_SendAicSyncViaIPC(); /* see [SRS_685], [SRS_686] */
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 5:
    {
      gpio_GetAicSyncFromIPC(); /* see [SRS_685], [SRS_686] */
      aicMgrRx_DoAicSync();
      /* call sync function because after AIC sync uCs might differ */
      ipcsSYS_Sync();
      break;
    }

    case 6:
    {
      /* NV Memory handler  */
      /* ATTENTION: This function shall only be called in time slices where no NV Memory access
      ** through background task is possible!!!
      */
      SAPL_NvLowHandler();

      /* Update SPDU buffers accessed by application/CSS and 
      ** process non-safe part of the last received AIC message */
      aicMgrRx_ProcessTgm();
      /* Reset Input IO Data since no DI/DO module active */
      aicMgrTx_ResetIoData();
      /* Set background task runnable */
      SAPL_BkgdTaskTrigger();
      break;
    }

    case 7:
    {
      /* do nothing in this time slice */
      #ifdef RFID_ACTIVE
         RFID_Reader_Boot();
      #endif /* RFID_ACTIVE */
      break;
    }

    case 8:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 9:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      break;
    }

    case 10:
    {
      /* check temperature (incl. IPC transmission), see [SRS_662], [SRS_215],
       * [SRS_397] */
      tmSRV_CheckTemperature();
      break;
    }

    case 11:
    {
      /* check temperature of other channel (received via IPC), see [SRS_662],
       * [SRS_215], [SRS_397] */
      tmSRV_SyncTemperature();
      break;
    }

    case 12:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 13:
    {
      /* do nothing in this time slice */
      break;
    }

    case 14:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      
      /* send static/global variables for soft error check and CSS control flow via
      ** IPC
      */
      SAPLipc_SendVar();
      break;
    }

    case 15:
    {
      /* check static/global variables and control flow path received from other controller */
      SAPLipc_RecvAndCheckVar();
      break;
    }

    case 16:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 17:
    {
      aicMgrRx_CheckForNewTgm();  /* see [SRS_2235] */
      break;
    }

    case 18:
    {
      /* call function to send timer/clock value to other channel,
       * see [SRS_663], [SRS_541] */
      clkCtrl_CalcAndSendTimeDiff();
      break;
    }

    case 19:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      
      /* call function to compare timer/clock value with other channel,
       * see [SRS_663], [SRS_541] */
      clkCtrl_CheckTimerVal();
      break;
    }

    case 20:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 21:
    {
      /* do nothing in this time slice */
      break;
    }

    case 22:
    {
      /* do some selftests */
      stHan_DoSelfTests();
      break;
    }

    case 23:
    {
      /* do nothing in this time slice */
      break;
    }

    case 24:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 25:
    {
      /* do nothing in this time slice */
      break;
    }

    case 26:
    {
      /* do nothing in this time slice */
      break;
    }

    case 27:
    {
      /* do nothing in this time slice */
      break;
    }

    case 28:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 29:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      break;
    }

    case 30:
    {
      aicMgrRx_CheckForNewTgm();  /* see [SRS_2235] */
      CheckMainExecution();
      break;
    }

    case 31:
    {
      /* do nothing in this time slice */
      break;
    }

    case 32:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 33:
    {
      /* do nothing in this time slice */
      break;
    }

    case 34:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      break;
    }

    case 35:
    {
      /* do nothing in this time slice */
      break;
    }

    case 36:
    {
      /* switch outputs off (cyclically done for more security) */
      gpio_SwitchOutputsOff();
      break;
    }

    case 37:
    {
      /* do nothing in this time slice */
      break;
    }

    case 38:
    {
      /* check background task cycle, this ensures that all tasks of the background
      ** are finished */
      SAPL_BkgdTaskCheck();
      
      /* if AIC in startup mode, dont call building of startup telegram and 
       * sending of AIC-telegram here,
       * the sending of startup-telegram is triggered below independent of
       * time slices */
      if (AICSM_AIC_STARTUP != aicSm_eAicState)
      {
        /* build AIC ASM telegram (TX) */
        aicMgrTx_BuildAsmTgm();
        /* merge and send AIC ASM telegram if not in startup mode
         * (see [SRS_2235], [SRS_428]) */
        aicMgrTx_MergeSendAsmTgm();
      }
      else
      {
        /* empty branch */
      }

      /* check scheduler cycle of appr. 4ms */
      CheckSchedulerTimeout();

      break;
    }

    case 39:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
    
      /* call state handling of ASM 
      ** ATTENTION: Afterwards the AIC state might be changed, this shall be considered in the 
      ** functions afterwards! */
      aicSm_Statemachine();
      break;
    }

    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
      break;
    }
  }

  /* in AIC startup mode call sending of telegram always for more accuracy
   * regarding the requested 25ms cycle (see [SRS_426]) */
  if (AICSM_AIC_STARTUP == aicSm_eAicState)
  {
    /* build AIC startup telegram and send the data out */
    aicMgrTx_MergeSendStartupTgm();
  }
  else
  {
    /* empty branch */
  }
/* This function has a lot of paths because of all possible time slices.
** But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

/***************************************************************************************************
  Function:
    timeSlicesRunMode

  Description:
    executes the different tasks for all time-slices in run mode (data exchange mode)

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void timeSlicesRunMode (void)
{
  /* Execute tasks in run (data-exchange) mode */
  switch (u8_BlockIdx)
  {
    case 0:
    {
      /* in case of first entry in scheduler, the values read during initialization phase are 
      ** handled here */
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC(); /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;  /* end of "case 0" */
    }

    case 1:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 2:
    {
      /* FIT to test the timeout-check for the selftests. With this FIT the
       * selftests are not executed and a timeout shall be detected after
       * the time T-st. */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #9    - 'goto' keyword identified 
      ** only used for FIT test */
      GLOBFIT_FITTEST_NOCMDRESET(GLOBFIT_NO_SELFTEST_TRIGGER, goto _FitOverJumpSelfTestTrigger);
      /* RSM_IGNORE_END */
      
      /* read HW counter/timer and send to other controller via IPC */
      TIM_SendTimerVal();

      /* do some selftests */
      stHan_DoSelfTests();
    
#ifdef GLOBFIT_FITTEST_ACTIVE
      _FitOverJumpSelfTestTrigger: /* CCT_NO_PRE_WARNING, only used for FIT test */
#endif
      break;
    }

    case 3:
    {
      gpio_ProcessDOs();
      aicMgrRx_CheckForNewTgm();  /* see [SRS_2235] */
    
      /* check HW timer/counter and set global systemtime used by both controllers */ 
      TIM_CheckAndSetTimerVal();

      /* read softerror variable and CSS path */
      SAPLipc_SetStackVarAndPath();
      break;
    }

    case 4:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      gpio_FilterDiRelValues();
      aicMgrRx_PrepAicSync();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC(); /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 5:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 6:
    {
      /* NV Memory handler  */
      /* ATTENTION: This function shall only be called in time slices where no NV Memory access
      ** through background task is possible!!!
      */
      SAPL_NvLowHandler();

      aicMgrRx_DoAicSync();
      /* call sync function because after AIC sync uCs might differ */
      ipcsSYS_Sync();
    
      /* Update SPDU buffers accessed by application/CSS and
      ** process non-safe part of the last received AIC message */
      aicMgrRx_ProcessTgm();
      /* Sample Input IO Data do be accessible through CSS stack */
      aicMgrTx_SampleIoData();
      /* Set background task runnable */
      SAPL_BkgdTaskTrigger();
      break;
    }

    case 7:
    {
      gpio_ProcessDOs();
      #ifdef RFID_ACTIVE
        RFID_ReadTag();
      #endif /* RFID_ACTIVE */
      break;
    }

    case 8:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 9:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 10:
    {
      /* check temperature (incl. IPC transmission), see [SRS_662], [SRS_215],
       * [SRS_397] */
      tmSRV_CheckTemperature();
      break;
    }

    case 11:
    {
      gpio_ProcessDOs();
      /* check temperature of other channel (received via IPC), see [SRS_662],
       * [SRS_215], [SRS_397] */
      tmSRV_SyncTemperature();
      break;
    }

    case 12:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 13:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 14:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();

      /* send static/global variables for soft error check and CSS control flow via
      ** IPC
      */
      SAPLipc_SendVar();
      break;
    }

    case 15:
    {
      gpio_ProcessDOs();
      /* check static/global variables and control flow path received from other controller */
      SAPLipc_RecvAndCheckVar();
      break;
    }

    case 16:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 17:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 18:
    {
      /* call function to send timer/clock value to other channel,
       * see [SRS_663], [SRS_541] */
      clkCtrl_CalcAndSendTimeDiff();
      break;
    }

    case 19:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
         
      /* call function to compare timer/clock value with other channel,
       * see [SRS_663], [SRS_541] */
      clkCtrl_CheckTimerVal();

      gpio_ProcessDOs();
      break;
    }

    case 20:
    {
      /* in case of first entry in scheduler, the values read during initialization phase are 
      ** handled here */
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC(); /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;  /* end of "case 0" */
    }

    case 21:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 22:
    {
      /* FIT to test the timeout-check for the selftests. With this FIT the
       * selftests are not executed and a timeout shall be detected after
       * the time T-st. */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #9    - 'goto' keyword identified 
      ** only used for FIT test */
      GLOBFIT_FITTEST_NOCMDRESET(GLOBFIT_NO_SELFTEST_TRIGGER, goto _FitOverJumpSelfTestTrigger2);
      /* RSM_IGNORE_END */

      /* do some selftests */
      stHan_DoSelfTests();
    
#ifdef GLOBFIT_FITTEST_ACTIVE
      _FitOverJumpSelfTestTrigger2: /* CCT_NO_PRE_WARNING, only used for FIT test */
#endif
      break;
    }

    case 23:
    {
      gpio_ProcessDOs();
      aicMgrRx_CheckForNewTgm();  /* see [SRS_2235] */
      break;
    }

    case 24:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC(); /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 25:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 26:
    {
      /* do nothing in this time slice */
      break;
    }

    case 27:
    {
      gpio_ProcessDOs();
      break;
    }

    case 28:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 29:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 30:
    {
      CheckMainExecution();
      break;
    }

    case 31:
    {
      gpio_ProcessDOs();
      break;
    }

    case 32:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 33:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 34:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();

      /* send subframe(s) (Data Message, Time Coordination Message) generated by CIP Safety stack
      ** to other safety controller.
      */
      spduIn_SendSafeTgm();
      break;
    }

    case 35:
    {
      gpio_ProcessDOs();
      /* receive subframe(s) (Data Message, Time Coordination Message) from other safety
      ** controller. This data is necessary to generate valid CIP Safety frames.
      */
      spduIn_RecvSafeTgm();
      break;
    }

    case 36:
    {
      gpio_FilterDiRelValues();
      gpio_HandleToTestExecution();
      gpio_SendIOvaluesViaIPC();  /* see [SRS_685], [SRS_686] */
      doState_ProcessDOsLight();
      break;
    }

    case 37:
    {
      gpio_GetIOvaluesFromIPC(); /* see [SRS_685], [SRS_686] */
      gpio_ProcessDIs();
      break;
    }

    case 38:
    {
      doSafeBoundSS1t_UpdateTimers();

      /* check background task cycle, this ensures that all tasks of the background
      ** are finished */
      SAPL_BkgdTaskCheck();
      
      /* build AIC ASM telegram (TX) */
      aicMgrTx_BuildAsmTgm();
      /* merge and send AIC ASM telegram (TX) */
      aicMgrTx_MergeSendAsmTgm();   /* see [SRS_2235] */
      
      /* check scheduler cycle of appr. 4ms */
      CheckSchedulerTimeout();
      break;
    }

    case 39:
    {
      /* trigger life signal and CC select generation, see [SRS_40] */
      gpio_CyclicSignals();
      
      /* call state handling of ASM 
      ** ATTENTION: Afterwards the AIC state might be changed, this shall be considered in the 
      ** functions afterwards! */
      aicSm_Statemachine();
    
      /* get IO data processed by safety stack */
      aicMgrRx_GetOutputData();

      gpio_ProcessDOs();
      break;
    }

    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(6u));
      break;
    }
  }
/* This function has a lot of paths because of all possible time slices.
** But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

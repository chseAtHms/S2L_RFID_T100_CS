/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** timer-hal.h
**
** $Id: timer-hal.h 1831 2017-01-23 16:13:25Z klan $
** $Revision: 1831 $
** $Date: 2017-01-23 17:13:25 +0100 (Mo, 23 Jan 2017) $
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
** Header of the timer module.
** This module provides functions to initialize the specified timers in
** different modes. With the GetSystemTime-Function the timer values can read.
**
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef TIMERHAL_H
#define TIMERHAL_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/



/*******************************************************************************
**
** Constants
**
********************************************************************************
*/



/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** TIMERHAL_TRIGGER_WWDG()
**
** Macro to trigger the window watchdog.
** This macro checks if the wwdg can be triggered or not. Is the counter value
** in the allowed area then the wwdg can be triggered.
** PC-Lint message 931 deactivated for this macro because there are no side
** effects.
**------------------------------------------------------------------------------
** Inputs:
**    void
**
** Outputs:
**    void
**
** Usage:
**    -
**
**------------------------------------------------------------------------------
*/
/* lint -emacro(931, TIMERHAL_TRIGGER_WWDG)*/
#define TIMERHAL_TRIGGER_WWDG \
{  \
   if ((WWDG->CR & (UINT32)WWDG_CR_T) < (WWDG->CFR & (UINT32)WWDG_CFR_W)) \
   { \
      WWDG->CR = (UINT32)(WWDG_CR_T0 + WWDG_CR_T1 + WWDG_CR_T6);\
   } \
}


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void timerHAL_InitCounter (CONST TIMERDEF_TIMER_CNT_CONFIG_STRUCT *psTimConfig);
extern void timerHAL_InitIrq (CONST TIMERDEF_TIMER_IRQ_CONFIG_STRUCT *psTimConfig);
extern void timerHAL_StartTimerIrq (TIMERDEF_TIMER_ENUM eTimer);
extern void timerHAL_ClearTimerIrq (TIMERDEF_TIMER_ENUM eTimer);
extern UINT32 timerHAL_GetSystemTime (CONST TIMERDEF_TIMER_ENUM eTimer);
extern UINT32 timerHAL_GetSystemTime3( void );
extern UINT32 timerHAL_GetSystemTime4( void );
extern void timerHAL_ClearTimer1Counter (void);
extern void timerHAL_StartWwdg (void);

#ifdef GLOBFIT_FITTEST_ACTIVE
extern void timerHAL_StopTimerIrq (TIMERDEF_TIMER_ENUM eTimer);
#endif


#endif /* TIMERHAL_H */

/*******************************************************************************
**
** End of timer-hal.h
**
********************************************************************************
*/

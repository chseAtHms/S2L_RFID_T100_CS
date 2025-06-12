/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** timer-hal.c
**
** $Id: timer-hal.c 2048 2017-02-07 11:33:59Z klan $
** $Revision: 2048 $
** $Date: 2017-02-07 12:33:59 +0100 (Di, 07 Feb 2017) $
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
** This module is the timer hardware abstraction layer.
** This module provides functions to initialize the specified timers in
** different modes. With the GetSystemTime-Function the timer values can read.
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

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "timer-def.h"

/* Header-file of module */
#include "timer-hal.h"
#include "timer-hal_loc.h"


/* Unique file id used to build additional info */
#define k_FILEID      31u

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
 ** timerHAL_InitCounter()
 **
 ** Initialize the specified timer in upcounting mode.
 ** This function initialize the specified timer in upcounting mode. The counter
 ** counts from 0 to 0xFFFF and and restarts with 0. The specified irq flag is
 ** set but no interrupt is requested.
 ** Supported Timers: TIMERDEF_TIMER3, TIMERDEF_TIMER4
 **
 ** Timer 2 & 3 synchronized together, Timer 3 triggers
 ** Timer 2. So they are working together as a 32-Bit counter.
 ** Timer 4 & 5 synchronized together, Timer 4 triggers
 ** Timer 5. So they are working together as a 32-Bit counter.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **  *psTimConfig      Timer counter configuration struct.
 **                    (valid range: pointer to initialized struct, checked)
 **
 ** Return:
 **  void
 **
 **  Remarks:
 **    Context: main (during initialization)
 **-----------------------------------------------------------------------------
 */
void timerHAL_InitCounter (CONST TIMERDEF_TIMER_CNT_CONFIG_STRUCT *psTimConfig)
{
   /* check that the pointer to the struct is not corrupted */
   if (psTimConfig->pThis == psTimConfig)
   {
     /* evaluate timer */
     switch (psTimConfig->eTimer)
     {
        case TIMERDEF_TIMER3:
        {
           /* enable clock for timer 2 & 3 */
           RCC->APB1ENR |= RCC_APB1ENR_TIM2EN + RCC_APB1ENR_TIM3EN;
  
           /* init timer 3 as master timer to trigger timer 2 */
           TIM3->CR2 = TIM_CR2_MMS_1;
           TIM3->ARR = TIM_ARR_ARR;
           TIM3->CNT = 0u;
           TIM3->PSC = psTimConfig->u16Prescaler;
           TIM3->DIER = TIM_DIER_UIE;
  
           /* init timer 2 as slave, trigger source is timer 3 */
           TIM2->SMCR = TIM_SMCR_SMS + TIM_SMCR_TS_1;
           TIM2->ARR = TIM_ARR_ARR;
           TIM2->CNT = 0u;
  
           /* clear IRQ */
           TIM2->SR = 0u;
           TIM3->SR = 0u;
  
           /* generate an update of the timer registers */
           TIM3->EGR = (UINT16)TIM_EGR_UG;
           TIM2->EGR = (UINT16)TIM_EGR_UG;
  
           /* enable timer 2 & 3 */
           TIM2->CR1 = TIM_CR1_CEN;
           TIM3->CR1 = TIM_CR1_CEN;
  
           break;
        }
        case TIMERDEF_TIMER4:
        {
           /* Attention: Timer 4 was already used/initialized in bootloader code. 
           ** So some registers perhaps not have their default values after enabling 
           ** the timer clock. Since the SFR checks all configuration registers,
           ** this could lead to an SFR error if any configuration register contains
           ** unexpected data. To prevent this, a peripheral reset is done here for 
          **  timer 4 and timer 5.
           */        
           /* reset TIM4 and TIM5 peripheral registers to their default value */
           RCC->APB1RSTR |= RCC_APB1RSTR_TIM4RST + RCC_APB1RSTR_TIM5RST;
           RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM4RST + RCC_APB1RSTR_TIM5RST);
          
           /* enable clock for timer 4 & 5 */
           RCC->APB1ENR |= RCC_APB1ENR_TIM4EN + RCC_APB1ENR_TIM5EN; 
  
           /* init timer 4 as master timer to trigger timer 5 */
           TIM4->CR2 = TIM_CR2_MMS_1;
           TIM4->ARR = TIM_ARR_ARR;
           TIM4->CNT = 0u;
           TIM4->PSC = psTimConfig->u16Prescaler;
           TIM4->DIER = TIM_DIER_UIE;
  
           /* init timer 5 as slave, trigger source is timer 4 */
           TIM5->SMCR = TIM_SMCR_SMS + TIM_SMCR_TS_1;
           TIM5->ARR = TIM_ARR_ARR;
           TIM5->CNT = 0u;
  
           /* clear IRQ */
           TIM5->SR = 0u;
           TIM4->SR = 0u;
  
           /* generate an update of the timer registers */
           TIM4->EGR = (UINT16)TIM_EGR_UG;
           TIM5->EGR = (UINT16)TIM_EGR_UG;
  
           /* enable timer 4 & 5 */
           TIM5->CR1 = TIM_CR1_CEN;
           TIM4->CR1 = TIM_CR1_CEN;
  
           break;
        }
        case TIMERDEF_TIMER1: /* TIMERDEF_TIMER1 not supported */
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
          break;
        }
        default:
        {
           GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
           break;
        }
     }
   }
   /* pointer to the struct is corrupted */
   else
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
   }
}


/*------------------------------------------------------------------------------
**
** timerHAL_InitIrq()
**
** Initialize the specified timer in upcounting mode and requests an interrupt
** at the overflow event.
**
** This function initialize the specified timer in upcounting mode. The counter
** counts from the start value to the auto-reload value, then the overflow
** interrupt is requested and the timer restarts its counting.
** The specified timer is not started in this function!!! To start the timer
** call the StartTimerIrq function.
** Supported Timers: TIMERDEF_TIMER1
**
**------------------------------------------------------------------------------
** Inputs:
**    *psTimConfig      Timer irq configuration struct.
 **                     (valid range: pointer to initialized struct, checked)
**
** Outputs:
**    void
**
**  Remarks:
**    Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void timerHAL_InitIrq (CONST TIMERDEF_TIMER_IRQ_CONFIG_STRUCT *psTimConfig)
{
   /* check that the pointer to the struct is not corrupted */
   if (psTimConfig->pThis == psTimConfig)
   {
     /* evaluate timer */
     switch (psTimConfig->eTimer)
     {
        case TIMERDEF_TIMER1:
        {
           /*enable clock for timer 1 */
           RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  
           /* init timer 1 */
           TIM1->PSC = psTimConfig->u16Prescaler;
           TIM1->ARR = psTimConfig->u16AutoReloadValue;
           TIM1->CNT = psTimConfig->u16CounterValue;
          
           /* set priority */
           NVIC_SetPriority( TIM1_UP_IRQn,
           ( UINT32 )( ( UINT32 )psTimConfig->u8IrqPriority & TIMERHAL_IRQ_PRIORITY_MASK ) );
  
           /* enable interrupt */
           NVIC_EnableIRQ( TIM1_UP_IRQn );
  
           /* generate an update of the timer registers */
           TIM1->EGR = (UINT16)TIM_EGR_UG;

           /* clear IRQ flag */
           TIM1->SR = 0u;
  
           /* enable update interrupt */
           TIM1->DIER = TIM_DIER_UIE;
  
           break;
        }
        case TIMERDEF_TIMER3: /* TIMERDEF_TIMER3 not supported */
        case TIMERDEF_TIMER4: /* TIMERDEF_TIMER4 not supported */
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(4u));
          break;
        }
        default:
        {
           GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
           break;
        }
     }
   }
   /* pointer to the struct is corrupted */
   else
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(6u));
   }
}


/*------------------------------------------------------------------------------
**
** timerHAL_StartTimerIrq()
**
** This function starts the specified timer.
** Supported Timers: TIMERDEF_TIMER1
**
**------------------------------------------------------------------------------
** Inputs:
**    eTimer      Timer to start.
**                (valid range: TIMERDEF_TIMER1, checked)
**
** Outputs:
**    void
**
** Remarks:
**    Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void timerHAL_StartTimerIrq (TIMERDEF_TIMER_ENUM eTimer)
{
   /* evaluate timer */
   switch (eTimer)
   {
      case TIMERDEF_TIMER1:
      {
         /* start the counter as up counter*/
         TIM1->CR1 = TIM_CR1_CEN;   /* enable timer 1 */
         break;
      }
      case TIMERDEF_TIMER3:  /* TIMERDEF_TIMER3 not supported */
      case TIMERDEF_TIMER4:  /* TIMERDEF_TIMER4 not supported */
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(7u));
        break;
      }
      default:
      {
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(8u));
         break;
      }
   }
}

#ifdef GLOBFIT_FITTEST_ACTIVE
/*------------------------------------------------------------------------------
**
** timerHAL_StopTimerIrq()
**
** This function stops the specified timer.
** Supported Timers: TIMERDEF_TIMER1
**
**------------------------------------------------------------------------------
** Inputs:
**    eTimer      Timer to stop.
**                (valid range: TIMERDEF_TIMER1, checked)
**
** Outputs:
**    void
**
**------------------------------------------------------------------------------
*/
void timerHAL_StopTimerIrq (TIMERDEF_TIMER_ENUM eTimer)
{
   /* evaluate timer */
   switch (eTimer)
   {
      case TIMERDEF_TIMER1:
      {
         /* stop the counter */
         TIM1->CR1 &= ((UINT16)~TIM_CR1_CEN);
         break;
      } /* CCT_SKIP Failure insertion Test */
      case TIMERDEF_TIMER3:  /* TIMERDEF_TIMER3 not supported */
      case TIMERDEF_TIMER4:  /* TIMERDEF_TIMER4 not supported */
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(9u));
        break;
      } /* CCT_SKIP Failure insertion Test */
      default:
      {
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(10u));
         break;
      } /* CCT_SKIP Failure insertion Test */
   }
} /* CCT_SKIP Failure insertion Test */
#endif


/*------------------------------------------------------------------------------
**
** timerHAL_ClearTimerIrq()
**
** This function clears the specified timer status register.
** Supported Timers: TIMERDEF_TIMER1
**
**------------------------------------------------------------------------------
** Inputs:
**    eTimer      Timer to clear irq status register.
**                (valid range: TIMERDEF_TIMER1, checked)
**
** Outputs:
**    void
**
**  Remarks:
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void timerHAL_ClearTimerIrq (TIMERDEF_TIMER_ENUM eTimer)
{
   /* evaluate timer */
   switch (eTimer)
   {
      case TIMERDEF_TIMER1:
      {
         /* clear the counter */
         TIM1->SR = ZERO;
         break;
      }
      case TIMERDEF_TIMER3:  /* TIMERDEF_TIMER3 not supported */
      case TIMERDEF_TIMER4:  /* TIMERDEF_TIMER4 not supported */
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(11u));
        break;
      }
      default:
      {
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(12u));
         break;
      }
   }
}


/*------------------------------------------------------------------------------
**
** timerHAL_GetSystemTime()
**
** This function returns the specified timer counter value.
** Supported Timers: TIMERDEF_TIMER3
**
**------------------------------------------------------------------------------
** Inputs:
**   eTimer            Timer to read counter from 
**                     (valid range: TIMERDEF_TIMER3, checked)
**
** Outputs:
**   u32TimCntValue    Value of the specified timer
**
** Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
UINT32 timerHAL_GetSystemTime( CONST TIMERDEF_TIMER_ENUM eTimer )
{
   UINT32 u32TimCntValue = 0u;

   switch (eTimer)
   {
      case TIMERDEF_TIMER3:
      {
         /* clear all IRQ flags */
         TIM2->SR = 0u;

         /* build UINT32 timer value */
         u32TimCntValue = TIMERHAL_SYSTEM_TIME;

         /* check if a timer 2 trigger event occurred? */
         if ((TIM2->SR & TIM_SR_TIF) == TIM_SR_TIF)
         {
            /* Timer 2 was triggered by timer 3, in this situation the value of
             * the timers are not valid. Wait till next timer step. */
            while (u32TimCntValue == TIMERHAL_SYSTEM_TIME)
            {

            }

            /* build new UINT32 timer value */
            u32TimCntValue = TIMERHAL_SYSTEM_TIME;

            /* clear all IRQ flags */
            TIM2->SR = 0u;
         }
         /* else: no timer 2 event occurred in the meantime so timer value valid */ 
         else
         {
           /* empty branch */
         }
         break;
      }
      case TIMERDEF_TIMER1:  /* TIMERDEF_TIMER1 not supported */
      case TIMERDEF_TIMER4:  /* TIMERDEF_TIMER4 not supported */
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(13u));
        break;
      }
      default:
      {
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(14u));
         break;
      }
   }

   return u32TimCntValue;
}


/*------------------------------------------------------------------------------
**
** timerHAL_GetSystemTime3()
**
** This function returns the 32Bit timer counter value of TIM2 + TIM3.
** The timer counter values of both timer are set to a 32Bit timer value.
**
**------------------------------------------------------------------------------
** Inputs:
**    void
**
** Outputs:
**    u32Ret    32Bit Timer counter value
**
** Remarks:
**    Context: main (during initialization)
**    Context: IRQ Scheduler
**    Context: Background Task
**------------------------------------------------------------------------------
*/
UINT32 timerHAL_GetSystemTime3( void )
{
   UINT32 u32Ret;

   /* clear all IRQ flags */
   TIM2->SR = 0u;

   /* build UINT32 timer value */
   u32Ret = TIMERHAL_SYSTEM_TIME;

   /* check if a timer 2 trigger event occurred? */
   if ((TIM2->SR & TIM_SR_TIF) == TIM_SR_TIF)
   {
      /* Timer 2 was triggered by timer 3, in this situation the value of
       * the timers are not valid. Wait till next timer step. */
      while (u32Ret == TIMERHAL_SYSTEM_TIME)
      {

      }

      /* build new UINT32 timer value */
      u32Ret = TIMERHAL_SYSTEM_TIME;

      /* clear all IRQ flags */
      TIM2->SR = 0u;
   }
   /* else: no timer 2 event occurred in the meantime so timer value valid */ 
   else
   {
     /* empty branch */
   }

   return u32Ret;

} /* end of  timerHAL_GetSystemTime3() */

/*------------------------------------------------------------------------------
**
** timerHAL_GetSystemTime4()
**
** This function returns the 32Bit timer counter value of TIM5 + TIM4.
** The timer counter values of both timer are set to a 32Bit timer value.
**
**------------------------------------------------------------------------------
** Inputs:
**    void
**
** Outputs:
**    u32Ret    32Bit Timer counter value
**
** Remarks:
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
UINT32 timerHAL_GetSystemTime4( void )
{
   UINT32 u32Ret;

   /* clear all IRQ flags */
   TIM5->SR = 0u;

   /* build UINT32 timer value */
   u32Ret = TIMERHAL_APPLICATION_TIME;

   /* check if a timer 5 trigger event occurred? */
   if ((TIM5->SR & TIM_SR_TIF) == TIM_SR_TIF)
   {
      /* Previously build UINT32 timer value is maybe corrupted (because of occurred timer trigger).
       * So build UINT32 timer value again. */
      u32Ret = TIMERHAL_APPLICATION_TIME;

      /* clear all IRQ flags */
      TIM5->SR = 0u;
   }
   /* else: no timer 5 event occurred in the meantime so timer value valid */ 
   else
   {
     /* empty branch */
   }

   return u32Ret;

} /* end of  timerHAL_GetSystemTime5() */


/*------------------------------------------------------------------------------
**
** timerHAL_ClearTimer1Counter()
**
** This function clears the counter register of timer 1.
**
**------------------------------------------------------------------------------
** Inputs:
**    void
**
** Outputs:
**    void
**
** Remarks:
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void timerHAL_ClearTimer1Counter (void)
{
   /* SNA: reset counter (coming from scheduler) not to "0", because already
    * some time elapsed (value verified by measurement of frequency with Osci)
    */
   /* RSM_IGNORE_QUALITY_BEGIN Notice #50   - Variable assignment to a literal number */
   TIM1->CNT = 5u;
   /* RSM_IGNORE_QUALITY_END */
}


#ifndef GLOBFIT_FITTEST_STARTUP_ACTIVE
/*------------------------------------------------------------------------------
**
** timerHAL_StartWwdg()
**
** This function initializes and starts the system watchdog.
** This function activates the window watchdog of the controller. The wwdg is
** set to a timeout of ~500us.
** Calculation of the watchdog value (see STM32F103xC reference manual_V12.pdf
** chapter "20.4 How to program the watchdog timeout"):
** @ Sysclk 72MHz is PCLK1 max. 36MHz
** tPCLK1 in ms, 1/36MHz = 27.8ns => 0.0000278ms
** tWWDG = tPLCK1 * 4096 * 2^WDGTB * (t[5:0] + 1)
** --> 0.00000278 * 4096 * 2^0 * (3 + 1) = 0.455ms => 455us
**
**------------------------------------------------------------------------------
** Inputs:
**    void
**
** Outputs:
**    void
**
** Remarks:
**    Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void timerHAL_StartWwdg (void)
{
   /* enable clock for the wwdg */
   RCC->APB1ENR |= RCC_APB1RSTR_WWDGRST;

   /* clear the "remove reset flag" to set back the wwdg-reset-flag */
   RCC->CSR |= RCC_CSR_RMVF;

   /* stop iwdg during debugging */
   DBGMCU->CR |= DBGMCU_CR_DBG_WWDG_STOP;

   /* enable wwdg + timeout of ~500us, see [SRS_355] */
   WWDG->CFR = (UINT32) (0u                               /* set prescaler to counter clock div 1*/
             + (WWDG_CFR_W6 + WWDG_CFR_W0 + WWDG_CFR_W1));/*lint !e835*/ 
   /* set counter value + the bit 6*/
   /* TSL: add the specified register values with zero to clear the unused bits. */

   WWDG->CR  = (UINT32) (WWDG_CR_WDGA  /* activate watchdog to generate a reset, see [SRS_506] */
             + (WWDG_CR_T6 + WWDG_CR_T0 + WWDG_CR_T1)); /* set counter value + the bit 6*/
}

#else
   #warning "Watchdog is disabled!!!"
   /* only used for failure insertion test */
   void timerHAL_StartWwdg (void)
   {
      return;
   } /* CCT_SKIP Failure insertion Test */
#endif

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of timer-hal.c
**
********************************************************************************
*/

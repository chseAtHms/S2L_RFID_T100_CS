/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** timer-def.h
**
** $Id: timer-def.h 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
** Definitions for the timer module.
** This header defines some enumerations and structures which are used for the
** timer module.
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
*/

#ifndef TIMERDEF_H
#define TIMERDEF_H

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
/*! Timer definition for synchronization */
#define TIMERDEF_SYNC   TIMERDEF_TIMER3


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
/* This enum specifies the available timers. */
typedef enum
{
   TIMERDEF_TIMER1   = 0x2e65u,
   TIMERDEF_TIMER3   = 0x2ff2u,
   TIMERDEF_TIMER4   = 0x30deu
} TIMERDEF_TIMER_ENUM;

/*! This struct specifies the parameters which are used to configure the timer
 * in upcounting mode. */
typedef struct TIMERDEF_TIMER_CNT_CONFIG_STRUCT_TAG
{
   /*!< Specified timer to configure. */
   TIMERDEF_TIMER_ENUM                                eTimer;       
   /*!< Prescaler for the timer. */   
   UINT16                                             u16Prescaler;  
   /*!< Pointer to itself. */
   struct TIMERDEF_TIMER_CNT_CONFIG_STRUCT_TAG*       pThis;         
} TIMERDEF_TIMER_CNT_CONFIG_STRUCT;

/*! This struct specifies the parameters which are used to configure the timer
 * with an interrupt. */
typedef struct TIMERDEF_TIMER_IRQ_CONFIG_STRUCT_TAG
{
   /*!< Specified timer to configure. */
   TIMERDEF_TIMER_ENUM                                eTimer;      
   /*!< Prescaler for the timer. */   
   UINT16                                             u16Prescaler;        
   /*!< Auto-relaod value to count to. */
   UINT16                                             u16AutoReloadValue;  
   /*!< Value of the counter register to start from. */
   UINT16                                             u16CounterValue;     
   /*!< Interrupt priority, it must be a value from 0..15. */
   UINT8                                              u8IrqPriority;       
   /*!< Pointer to itself. */
   struct TIMERDEF_TIMER_IRQ_CONFIG_STRUCT_TAG*       pThis;               
} TIMERDEF_TIMER_IRQ_CONFIG_STRUCT;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/


#endif /* TIMERDEF_H */

/*******************************************************************************
**
** End of timer-def.h
**
********************************************************************************
*/

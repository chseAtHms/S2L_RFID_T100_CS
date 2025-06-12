/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** timer-hal-loc.h
**
** $Id: timer-hal_loc.h 598 2016-09-05 12:21:26Z klan $
** $Revision: 598 $
** $Date: 2016-09-05 14:21:26 +0200 (Mo, 05 Sep 2016) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**  Local header for timer-hal module.
**  Contains the local constants and type definitions of the appendent c-file,
**  see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
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

#ifndef TIMERHAL_LOC_H
#define TIMERHAL_LOC_H

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
/*! This constant defines the interrupt priority mask. The interrupt priority
 *  provides a 4-bit priority field for each interrupt, refer to
 *  STM32F100xx_Reference_Manual.pdf. */
#define TIMERHAL_IRQ_PRIORITY_MASK ((UINT8)0x0Fu)


/*! This macro returns the current system time.
 *  PC-Lint message 931 deactivated for this macro because there are no side
 *  effects. */
/*lint -emacro(931, TIMERHAL_SYSTEM_TIME)*/
#define TIMERHAL_SYSTEM_TIME ((((UINT32)TIM2->CNT) << 16u) + (UINT32)TIM3->CNT)

/*! This macro returns the current application time. 
 *  PC-Lint message 931 deactivated for this macro because there are no side
 *  effects. */
/*lint -emacro(931, TIMERHAL_APPLICATION_TIME)*/
#define TIMERHAL_APPLICATION_TIME ((((UINT32)TIM5->CNT) << 16u) + (UINT32)TIM4->CNT)


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




/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/


#endif /* TIMERHAL_LOC_H */

/*******************************************************************************
**
** End of timer-hal_loc.h
**
********************************************************************************
*/

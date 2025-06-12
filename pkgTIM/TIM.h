/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: TIM.h
**     Summary: Timer module
**              Interface header for Timer unit
**   $Revision: 2324 $
**       $Date: 2017-03-10 15:07:39 +0100 (Fr, 10 Mrz 2017) $
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

#ifndef TIM_H
#define TIM_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
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
void TIM_Init(void);

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
void TIM_SendTimerVal(void);

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
void TIM_CheckAndSetTimerVal(void);

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
UINT32 TIM_TimeGet_128us(void);

#endif


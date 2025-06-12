/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: sch_Scheduler-app.h
**     Summary: Header of module sch_Scheduler-app.c
**   $Revision: 2417 $
**       $Date: 2017-03-23 14:11:41 +0100 (Do, 23 Mrz 2017) $
**      Author: K.Angele
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
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SCHAPP_H
#define SCHAPP_H

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
void schAPP_TriggerMainExecTime (void);

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
void schAPP_Init (void);

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
void schAPP_StartScheduler (void);

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
void schAPP_StopScheduler (void);
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
void schAPP_TriggerCycleStatistic (void);

#endif /* SCHAPP_H */


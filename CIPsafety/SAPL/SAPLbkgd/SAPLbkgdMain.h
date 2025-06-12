/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdMain.h
**     Summary: Interface for synchronization of the Background Task.
**   $Revision: 1974 $
**       $Date: 2017-02-02 09:21:50 +0100 (Do, 02 Feb 2017) $
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

#ifndef SAPL_BKGDTASK_H
#define SAPL_BKGDTASK_H

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
void SAPL_BkgdTaskTrigger(void);

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
void SAPL_BkgdTaskCheck(void);

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
void SAPL_BkgdTaskExec(void);

#endif


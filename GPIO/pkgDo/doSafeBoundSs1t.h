/***************************************************************************************************
**    Copyright (C) 2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: doSafeBoundSs1t.h
**     Summary: Interface header of the module doSafeBoundSs1t.c
**   $Revision: 4595 $
**       $Date: 2024-09-05 15:08:19 +0200 (Do, 05 Sep 2024) $
**      Author: AnKr
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: doSafeBoundSS1t_Init
**             doSafeBoundSS1t_SetSafeBoundVal
**             doSafeBoundSS1t_ReqSet
**             doSafeBoundSS1t_ReqRampDown
**             doSafeBoundSS1t_UpdateTimers
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef DOSAFEBOUNDSS1T_H_
#define DOSAFEBOUNDSS1T_H_


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
    doSafeBoundSS1t_Init

  Description:
    This function initializes the static variables of this module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_Init(void);


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_SetSafeBoundVal

  Description:
    This function sets the input value coming from the local DIs for the "AND gate" of the
    SafeBound feature. The provided value is stored and later used in the state machine.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    b_diSafeBoundValue (IN) - boolean value to be set
                              (valid range: TRUE/FALSE, not checked, only called with constant)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_SetSafeBoundVal(const UINT8 u8_doChannel,
                                     BOOL b_diSafeBoundValue);


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_ReqSet

  Description:
    This function is the state machine for the SS1-t feature. It evaluates the request for the
    output state and considers the DI SafeBound value for the DO.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    u8_doChannelOther (IN)  - in Dual Channel mode: the other DO channel number
                              in Single Channel mode: same value as u8DoChannel
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    e_reqState (IN)         - requested state for the output
                              (valid range: GPIO_STATE_ENUM, not checked, passed to
                               sub-functions, checked there)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_ReqSet(UINT8 u8_doChannel,
                            UINT8 u8_doChannelOther,
                            GPIO_STATE_ENUM e_reqState);


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_ReqRampDown

  Description:
    This function determines the states of the "virtual DIs" for the signalization of SS1-t delay.

  See also:
    -

  Parameters:
    -

  Return value:
    UINT8 - value of virtual DIs

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT8 doSafeBoundSS1t_ReqRampDown(void);


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_UpdateTimers

  Description:
    This function needs to be called once in every scheduler cycle to update the SS1-t delay timers.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_UpdateTimers(void);

#endif  /* inclusion lock */

/*** End Of File ***/

/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutData.h
**     Summary: Interface header of the module 'spduOutData.c'
**   $Revision: 2618 $
**       $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: spduOutData_Reset
**             spduOutData_Process
**             spduOutData_Set
**             spduOutData_Get
**             spduOutData_DoGet
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SPDUOUTDATA_H
#define SPDUOUTDATA_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/



/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* spduOutData_e_IsRunMode
** Status (Idle/Run) of the output data received by CSS.
** eFALSE: IDLE, eTRUE: RUN
*/
extern volatile TRUE_FALSE_ENUM spduOutData_e_IsRunMode;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    spduOutData_Reset

  Description:
    This function resets the received IO values of global IO value structure and sets the
    default values inside module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)
    Context: IRQ Scheduler

***************************************************************************************************/
void spduOutData_Reset (void);

/***************************************************************************************************
  Function:
    spduOutData_Process

  Description:
    This function is used to process the IO Data payload received via CIP Safety IO Data Message.
    Depending on the received RUN/IDLE bit the software:
      - passes the IO Data, DI Reset Flags, DO Reset Flags to the DI/DO Module (-->RUN)
      - requests the passivation on the DOs (-->IDLE)

    Attention: This function shall be called only, if the Consuming Connection is established!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduOutData_Process(void);

/***************************************************************************************************
  Function:
    spduOutData_Set

  Description:
    This function is used to store the received output data (DO Value, DI Error Reset Flags,
    DO Error Reset Flags) inside the module.
    The passed output data keeps valid until a new output data is passed or the function  
    spduOutData_Reset() is called after AIC state has changed.

    This function shall be called if an new IO Data Message was received.

  See also:
    -

  Parameters:
    ps_ioVal (IN)            - Pointer to received output data  (DO Value, DI Error Reset Flags,
                               DO Error Reset Flag)
                               (valid: <>NULL, not checked, already checked in 
                               SAPL_IxsaiIoDataRxClbk())

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void spduOutData_Set(CONST AICMSGDEF_PS_OUT_DATA_STRUCT* ps_ioVal);

/***************************************************************************************************
  Function:
    spduOutData_Get

  Description:
    This function returns the last received output data (DO Value, DI Error Reset Flags,
    DO Error Reset Flags).

    Because no RDS checks are done inside this function, the values shall be used for non-safe 
    functionality only.

    Attention: IRQs not disabled because function called from Background Task and IRQ Scheduler.
    This is task of the caller function.

  See also:
    -

  Parameters:
    -

  Return value:
    AICMSGDEF_PS_OUT_DATA_STRUCT - (DO Value, DI Error Reset Flags, DO Error Reset Flags)

  Remarks:
    Context: Background Task
    Context: IRQ Scheduler

***************************************************************************************************/
AICMSGDEF_PS_OUT_DATA_STRUCT spduOutData_Get(void);

/***************************************************************************************************
  Function:
    spduOutData_DoGet

  Description:
    This function return the current secured (RDS) DO Value.

  See also:
    -

  Parameters:
    -

  Return value:
    DO Value

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT8 spduOutData_DoGet(void);

#endif


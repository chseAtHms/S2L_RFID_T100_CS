/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMgrRx.h
**     Summary: Interface header of module 'aicMgrRx.c"
**   $Revision: 2387 $
**       $Date: 2017-03-17 12:38:18 +0100 (Fr, 17 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMgrRx_CheckForNewTgm
**             aicMgrRx_PrepAicSync
**             aicMgrRx_DoAicSync
**             aicMgrRx_ProcessTgm
**             aicMgrRx_GetOutputData
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMGRRX_H
#define AICMGRRX_H

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
    aicMgrRx_CheckForNewTgm

  Description:
    calls functions to check if there was a new RX telegram received.
    If yes, functions to check (CRC) and split the message are called.
    In case of an error in CRC, function to handle an "invalid" telegram
    is called

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_CheckForNewTgm (void);

/***************************************************************************************************
  Function:
    aicMgrRx_PrepAicSync

  Description:
    Calls function to transfer the current number of received telegrams to
    the module which will send the value via IPC.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_PrepAicSync (void);

/***************************************************************************************************
  Function:
    aicMgrRx_DoAicSync

  Description:
    Gets the number of received telegrams of the other channel (received
    via IPC).
    If the other channel received one telegram more, than the AIC-RX buffer
    is again checked for a telegram.
    Else if both channels received the same number of telegrams, everything
    is fine.
    Else if the difference of the number of received telegrams is bigger than
    1, the fail safe handler is called because one uC missed more than one
    telegram.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_DoAicSync (void);

/***************************************************************************************************
  Function:
    aicMgrRx_ProcessTgm

  Description:
    calls the necessary functions to process an incoming message

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_ProcessTgm (void);

/***************************************************************************************************
  Function:
    aicMgrRx_GetOutputData

  Description:
    High Level function to process the received data if device is in EXCHANGE mode.
    Additionally automatic error reset feature is handled here.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_GetOutputData (void);


#endif


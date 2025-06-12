/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgErr.h
**     Summary: Interface header of module aicMsgErr.c
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgErr_SerialError
**             aicMsgErr_DiscardError
**             aicMsgErr_BuildGetStsResp
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSGHDLERR_H
#define AICMSGHDLERR_H

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
    aicMsgErr_SerialError

  Description:
    informs the non-safe message module about reception errors in Telegram,
    and increases the serial error counter accordingly, see [SRS_284], [SIS_021]

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgErr_SerialError (void);

/***************************************************************************************************
  Function:
    aicMsgErr_DiscardError

  Description:
    increases the serial discard error, this function shall be called when
    unexpected responses occur, see [SRS_432], [SIS_027]

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgErr_DiscardError (void);

/***************************************************************************************************
  Function:
    aicMsgErr_BuildGetStsResp

  Description:
    This function is used to build the response message of 'GetStatus' request. The response
    contains:
     - the 'Discarded responses' (DR) error counter
     - the 'Serial reception' (SR) error counter
     - up to four T100 error events

  See also:
    -

  Parameters:
    ps_txMsg (OUT)     - Message being sent to AM
                         (valid range: <>NULL, not checked, only called with reference to
                         static/global buffer)
    u8_msgId (IN)      - Message ID
                         (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgErr_BuildGetStsResp(AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg, UINT8 u8_msgId);

#endif


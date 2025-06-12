/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgErr.c
**     Summary: This module implements functionality to access (read/write) the error counters
**              and log events specified by [SIS].
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


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"

#include "globErrLog.h"

#include "aicMsgDef.h"
#include "aicMsgSis.h"
#include "aicMsgErr.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* u16_SerialRecErr:
** counter for serial reception errors (according to SIS specification)
*/
STATIC UINT16 u16_SerialRecErr = 0u;

/* u16_SerialDisErr:
** counter for serial discard errors (according to SIS specification)
*/
STATIC UINT16 u16_SerialDisErr = 0u;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
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
void aicMsgErr_SerialError (void)
{
  /* if limit for error counter not already reached */
  if (u16_SerialRecErr < AICMSGHDL_ERRCNT_LIMIT)
  {
    /* increase error counter, see [SIS_021] */
    u16_SerialRecErr++;
  }
  else
  {
    /* if the counter reached the limit, there is really nothing to do! */
  }
}

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
void aicMsgErr_DiscardError (void)
{
  /* if limit for error counter not already reached */
  if (u16_SerialDisErr < AICMSGHDL_ERRCNT_LIMIT)
  {
    /* increase error counter, see [SIS_027] */
    u16_SerialDisErr++;
  }
  else
  {
    /* if the counter reached the limit, there is really nothing to do! */
  }
}

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
void aicMsgErr_BuildGetStsResp(AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg, UINT8 u8_msgId)
{
  /* pointer to error event log */
  CONST volatile GLOBERRLOG_t_LOG* ps_errEvtLog = globErrLog_ErrorLogGet();

  /* message header, [SRS_477](unmodified), [SIS_053] */
  /* copy message ID received inside the request to the response,
   * see [SRS_434], [SIS_029] */
  ps_txMsg->sAicMsgHeader.u8MsgId = u8_msgId;
  /* response bytes mirrored from request, but with bit 14 set to 0,
   * see [SRS_438], [SRS_439], [SIS_032], [SIS_033] */
  ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_GET_ASM_STATUS_RESP;
  ps_txMsg->sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_GET_ASM_STATUS_RES_LENGTH;

  /* Error Counter DR and SE according to SIS chapter 5.3,
   * see also [SRS_284], [SRS_432], [SIS_055]
   * Words transfered in "little-endian"! (see [SRS_475], [SIS_052]) */
  ps_txMsg->au8AicMsgData[0u] = (UINT8)(u16_SerialDisErr & 0x00FFu);
  ps_txMsg->au8AicMsgData[1u] = (UINT8)((u16_SerialDisErr >> 8u) & 0x00FFu);

  ps_txMsg->au8AicMsgData[2u] = (UINT8)(u16_SerialRecErr & 0x00FFu);
  ps_txMsg->au8AicMsgData[3u] = (UINT8)((u16_SerialRecErr >> 8u) & 0x00FFu);

  ps_txMsg->au8AicMsgData[4u]  = (UINT8)(ps_errEvtLog->au16_errEntry[0] & 0x00FFu);
  ps_txMsg->au8AicMsgData[5u]  = (UINT8)((ps_errEvtLog->au16_errEntry[0] >> 8u) & 0x00FFu);

  ps_txMsg->au8AicMsgData[6u]  = (UINT8)(ps_errEvtLog->au16_errEntry[1] & 0x00FFu);
  ps_txMsg->au8AicMsgData[7u]  = (UINT8)((ps_errEvtLog->au16_errEntry[1] >> 8u) & 0x00FFu);

  ps_txMsg->au8AicMsgData[8u]  = (UINT8)(ps_errEvtLog->au16_errEntry[2] & 0x00FFu);
  ps_txMsg->au8AicMsgData[9u]  = (UINT8)((ps_errEvtLog->au16_errEntry[2] >> 8u) & 0x00FFu);

  ps_txMsg->au8AicMsgData[10u] = (UINT8)(ps_errEvtLog->au16_errEntry[3] & 0x00FFu);
  ps_txMsg->au8AicMsgData[11u] = (UINT8)((ps_errEvtLog->au16_errEntry[3] >> 8u) & 0x00FFu);
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/



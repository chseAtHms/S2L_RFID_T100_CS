/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCsal.h
**     Summary: Interface header of module aicMsgCsal.c
**   $Revision: 3806 $
**       $Date: 2023-01-10 14:46:54 +0100 (Di, 10 Jan 2023) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCsal_Init
**             aicMsgCsal_CheckRes
**             aicMsgCsal_MsgGet
**             aicMsgCsal_MsgRelease
**             aicMsgCsal_ReadMsg
**             aicMsgCsal_CalcNumOfFragsToSkip
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_CSAL_H
#define AICMSG_CSAL_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/


/** AICMSGCSAL_t_AICMSG:
    This type defines the messages that are received from AIC.
*/
typedef struct
{
  UINT16  u16_cmd;                                    /* Command code */
  UINT16  u16_len;                                    /* Data length of the passed message */
  UINT32  u32_addInfo;                                /* Additional information */
  UINT8   au8_data[AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE];  /* Data */
} AICMSGCSAL_t_AICMSG;


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgCsal_Init

  Description:
    This function is used to initialize the queue used for received CSAL messages.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void aicMsgCsal_Init(void);

/***************************************************************************************************
  Function:
    aicMsgCsal_CheckRes

  Description:
    This function is used to check if enough resources are available to process the received CSAL
    message without processing the payload itself. Since the resources used for CSAL
    buffering are restricted, only CSAL messages up to AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE bytes of
    payload can be processed. If the length is exceeded the CSAL message shall be refused.
    Furthermore the count of CSAL messages is restricted up to AICMSGCSAL_cfg_UCQ_SIZE. If
    this count is exceeded the CSAL message shall be refused.
    
  See also:
    -

  Parameters:
    ps_rxMsg (IN)         - Received Message from AM
                            (valid range: <>NULL, not checked, only called with reference to
                            static/global buffer)
    b_invalidLength (OUT) - CSAL message cannot be processed because length is invalid
                          - CSAL message length is ok

  Return value:
    TRUE              - CSAL message can be processed
    FALSE             - CSAL message can not be processed because no resources available.

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCsal_CheckRes (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg,
                          BOOL *b_invalidLength);

/***************************************************************************************************
  Function:
    aicMsgCsal_MsgGet

  Description:
    This function returns a request message stored inside the request command queue.
    In case several requests are waiting, this function always returns the oldest request.
    Attention: in order to remove the request from the queue, the function aicMsgCsal_MsgRelease
    shall be called after processing.

  See also:
    aicMsgCsal_MsgRelease

  Parameters:
    -

  Return value:
    NULL            - No data available, request queue is empty
    <>NULL          - Pointer to received request message data.

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST volatile AICMSGCSAL_t_AICMSG* aicMsgCsal_MsgGet(void);

/***************************************************************************************************
  Function:
    aicMsgCsal_MsgRelease

  Description:
    This function releases the oldest received request message from the request message queue.

  See also:
    aicMsgCsal_MsgGet

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void aicMsgCsal_MsgRelease(void);

/***************************************************************************************************
  Function:
    aicMsgCsal_ReadMsg

  Description:
    To be called when a message request containing a HalcCsalMessage is received. Many 
    HalcCsalMessage requests are "fragmented" messages, means more than one telegram is needed to
    transmit all parameters (or the whole message). The function will count the offset of received
    bytes and indicate if the message is still pending (not all parts/fragments received) or
    not (all fragments of message received).
    Hint: Message fragmentation is described in Serial Interface Specification [SIS] and Software 
    Requirements Specification[SRS]

  See also:
    -

  Parameters:
    ps_rxMsg (IN)            - Received Message from AM
                               (valid range: <>NULL, not checked, only called with reference to
                               buffer)

  Return value:
    TRUE                     - CSAL message received completely
    FALSE                    - Still CSAL fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCsal_ReadMsg (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg);

/***************************************************************************************************
  Function:
    aicMsgCsal_CalcNumOfFragsToSkip

  Description:
    This function returns the number of fragments which shall be skipped in case of an AIC
    error. 
    Fragmented CSAL messages are split in the first message and several "data only" messages
    (currently a "standard" non-safe message has 16 Bytes (first message: 11 Bytes header and
    5 Bytes data), a "data only" message contains 16 Bytes of data and no header at all.

  See also:
    -

  Parameters:
    u16_length (IN) - length of CSAL message in bytes 

  Return value:
    Number of fragments which shall be skipped

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT8 aicMsgCsal_CalcNumOfFragsToSkip(UINT16 u16_length);

#endif


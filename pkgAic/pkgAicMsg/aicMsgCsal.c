/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCsal.c
**     Summary: This module reassembles the HalcCsalMessage requests received via AIC. 
**              The reassembled messages are stored for further processing through the CSS.
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
**             CopyRcvdCsalFrag
**             AquireQueEntry
**             ReleaseQueEntry
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
#include "globDef_GlobalDefinitions.h"

/* module includes */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "rds.h"

#include "stdlib-hal.h"

#include "aicMsgDef.h"
#include "aicMsgHalc.h"
#include "aicMsgCsal_cfg.h"
#include "aicMsgCsal.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      5u

/* Init-value (RDS_UINT16) of local offset */
#define k_CSAL_OFS_INIT            {{(UINT16)0u},{(UINT16)~(UINT16)0u}}

/* queue management variables for the "Upward Command Queue" */
/* free queue entry */
STATIC volatile RDS_UINT16 u16_UcqNumFree;

/* queue index to read from */
STATIC RDS_UINT16 u16_UcqRdIdx;
/* queue index to write into */
STATIC RDS_UINT16 u16_UcqWrIdx;

/* memory of the "Upward Command Queue" */
/* HALCS message headers */
STATIC volatile AICMSGCSAL_t_AICMSG as_Ucq[AICMSGCSAL_cfg_UCQ_SIZE];

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void CopyRcvdCsalFrag(CONST UINT8* pu8_src,
                             CONST UINT8  u8_srcOfs,
                             volatile UINT8* pu8_dst,
                             CONST UINT16 u16_dstOfs,
                             CONST UINT8  u8_size);
STATIC volatile AICMSGCSAL_t_AICMSG* AquireQueEntry(void);
STATIC void ReleaseQueEntry(void);

/***************************************************************************************************
**    global functions
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
void aicMsgCsal_Init(void)
{ 
  /* initialize queue management variables */
  RDS_SET(u16_UcqNumFree, AICMSGCSAL_cfg_UCQ_SIZE);
  RDS_SET(u16_UcqRdIdx, 0U);
  RDS_SET(u16_UcqWrIdx, 0U);
}

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
                          BOOL *b_invalidLength)
{
  BOOL b_resAvail;
  
  /* cast generic non-safe message structure to HALC specific one */
  /* Deactivated Lint 
  ** Note 929: cast from pointer to pointer [possibly violation MISRA 2004 Rule 11.4]
  ** Info 740: Unusual pointer cast (incompatible indirect types) [MISRA 2004 Rule 1.2]
  ** cast is necessary and accepted here */
  CONST AICMSGHALC_t_MSG* ps_halcMsg = (CONST AICMSGHALC_t_MSG*)ps_rxMsg; /*lint !e740 !e929*/
  
  /* calc raw length of HALC data field. This excludes the HALC Header (Cmd, AdditionalInfo) */
  UINT16 u16_len = ps_halcMsg->s_aicHdr.u16_msgLength - AICMSGHALC_k_HALC_HDR_LENGTH;
  
  /* if length exceeds the provided memory storage */
  if (u16_len > AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE)
  {
    b_resAvail = FALSE;
    *b_invalidLength = TRUE;
  }
  /* else if queue is full  */
  else if ( 0u == RDS_GET(u16_UcqNumFree) )
  {
    b_resAvail = FALSE;
    *b_invalidLength = FALSE;
  }
  /* else: resource available */
  else
  {
    b_resAvail = TRUE;
    *b_invalidLength = FALSE;
  }
  return b_resAvail;
}

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
CONST volatile AICMSGCSAL_t_AICMSG* aicMsgCsal_MsgGet(void)
{
   /* return value of this function */
  volatile AICMSGCSAL_t_AICMSG *ps_retVal = NULL;
  
  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_UcqRdIdx);
  /*lint +esym(960, 10.1)*/

  /* Attention: 'Upward Command Queue' is accessed from background and scheduler task.
  ** The access to variables is protected by disabling irqs temporarily
  */
  __disable_irq();
  
  /*
  ** from CSAL/AIC to CSS
  */
  /* if at least one element in queue */
  if ( RDS_GET(u16_UcqNumFree) < AICMSGCSAL_cfg_UCQ_SIZE)
  {
    /* Attention: Enable IRQs again */
    __enable_irq();
    
    /* return pointer to current queue read position */
    ps_retVal = &as_Ucq[RDS_GET_VALUE(u16_UcqRdIdx)];
  }
  /* else: queue empty */
  else
  {
    /* Attention: Enable IRQs again */
    __enable_irq();
  }
  
  return (ps_retVal);
}

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
void aicMsgCsal_MsgRelease(void)
{
  
  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_UcqRdIdx);
  /*lint +esym(960, 10.1)*/
  
  
  /*
  ** from CSAL/AIC to CSS
  */
  /* Attention: 'Upward Command Queue' is accessed from background and scheduler task.
  ** The access to variables is protected by disabling irqs temporarily
  */
  __disable_irq();
  
  /* if entry inside queue */
  if ( RDS_GET(u16_UcqNumFree) < AICMSGCSAL_cfg_UCQ_SIZE )
  {
    RDS_INC(u16_UcqNumFree);
    
    /* Attention: Enable IRQs again */
    __enable_irq();
    
    RDS_INC(u16_UcqRdIdx);
    /* if wrap-around of write index */
    if ( RDS_GET_VALUE(u16_UcqRdIdx) == AICMSGCSAL_cfg_UCQ_SIZE )
    {
      RDS_SET(u16_UcqRdIdx, 0U);
    }
    /* else: no wrap-around */
    else
    {
      /* empty branch */
    }
  }
  /* else: queue empty */
  else
  {
    /* Attention: Enable IRQs again */
    __enable_irq();
  }
}

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
BOOL aicMsgCsal_ReadMsg (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg)
{
  /* RDS union initializations */
  /* Deactivated Lint message 708, because a Union initialization is wanted to save
  ** init function */
  LOCAL_STATIC(, RDS_UINT16,  u16_CsalMsgOfs, k_CSAL_OFS_INIT); /*lint !e708*/

  /* variable to indicate if message was received completely */
  BOOL b_csalMsgRcvd;

  UINT16 u16_remBytes; /* bytes still to copy */

  /* Pointer to  "Upward Command Queue" element currently written */
  volatile AICMSGCSAL_t_AICMSG* ps_queElem;
  
  /* cast generic non-safe message structure to HALC specific one */
  /* Deactivated Lint 
  ** Note 929: cast from pointer to pointer [possibly violation MISRA 2004 Rule 11.4]
  ** Info 740: Unusual pointer cast (incompatible indirect types) [MISRA 2004 Rule 1.2]
  ** cast is necessary and accepted here */
  CONST AICMSGHALC_t_MSG* ps_halcMsg = (CONST AICMSGHALC_t_MSG*)ps_rxMsg; /*lint !e740 !e929*/

  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_CsalMsgOfs);
  /*lint +esym(960, 10.1)*/

  /* get pointer to current queue element writeable */
  ps_queElem = AquireQueEntry();
  /* if no valid queue element available */
  if (ps_queElem == NULL)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
    /* set return value (only used for unit test) */
    b_csalMsgRcvd = FALSE;
  }
  /* valid queue element */
  else
  {
    /* if the first fragment is received */
    if ( RDS_GET_VALUE(u16_CsalMsgOfs) == 0u )
    {
      /*****************************************/
      /* build HALC header from first fragment */
      /*****************************************/
      /* Command */
      ps_queElem->u16_cmd = ps_halcMsg->s_halcHdr.u16_halcCmd;
      /* HALC Message length */
      ps_queElem->u16_len = ps_halcMsg->s_aicHdr.u16_msgLength - AICMSGHALC_k_HALC_HDR_LENGTH;
      /* Additional Info */
      ps_queElem->u32_addInfo = ps_halcMsg->s_halcHdr.u32_halcAddInfo;
 
      /*****************************************/
      /* copy payload                          */
      /*****************************************/
      /* if fragmentation required */
      if (ps_queElem->u16_len > AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE)
      {
        /* copy data to receive buffer */
        CopyRcvdCsalFrag((CONST UINT8*)ps_halcMsg,  /*lint !e928 */
                         AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS,
                         ps_queElem->au8_data,
                         (UINT16)0u,
                         AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE);
        
        /* add copied bytes to offset counter */
        RDS_ADD(u16_CsalMsgOfs, AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE);
        
        /* HalcCsalMessage NOT completely received, fragments still missing */
        b_csalMsgRcvd = FALSE;
      }
      /* else: no fragmentation required */
      else
      {
        /* copy data to receive buffer */
        CopyRcvdCsalFrag((CONST UINT8*)ps_halcMsg,  /*lint !e928 */
                         AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS,
                         ps_queElem->au8_data,
                         (UINT16)0u,
                         (UINT8)ps_queElem->u16_len);
 
        /* all data written, release queue entry */
        ReleaseQueEntry();
        
        /* reset offset counter */
        RDS_SET(u16_CsalMsgOfs, 0u);
        
        /* HalcCsalMessage completely received */
        b_csalMsgRcvd = TRUE;
      }
    }
    /* else: second or more fragment, ongoing transfer */
    else
    {
      /* calculate number of remaining bytes required to transfer the message,
      ** depending on this value, the decision is made to receive a full
      ** middle or the last fragment */
      u16_remBytes = ps_queElem->u16_len - RDS_GET_VALUE(u16_CsalMsgOfs);
 
      /* if at least one more fragment is necessary */
      if ( u16_remBytes > (UINT8)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT) )
      {
        /* n-th. fragment, now the "data only" buffer is used, because
        * in this case it was a message without header */
        CopyRcvdCsalFrag((CONST UINT8*)ps_rxMsg,  /*lint !e928 */
                         (UINT8)0u,
                         ps_queElem->au8_data,
                         RDS_GET_VALUE(u16_CsalMsgOfs),
                         (UINT8)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT));
        
        /* add copied bytes to offset counter */
        RDS_ADD(u16_CsalMsgOfs, (UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT));
 
        /* HalcCsalMessage NOT completely received, fragments still missing */
        b_csalMsgRcvd = FALSE;
      }
      /* else: last fragment */
      else
      {
        /* last fragment (length depending on HalcCsal message)
         * now the "data only" buffer is used, because in this case it was a
         * message without header */
        /* lint Note 928 deactivated, pointer casting is OK! */
        CopyRcvdCsalFrag((CONST UINT8*)ps_rxMsg,  /*lint !e928 */
                         (UINT8)0u,
                         ps_queElem->au8_data,
                         RDS_GET_VALUE(u16_CsalMsgOfs),
                         (UINT8)u16_remBytes); /* cast to UINT8 ok, because u16_remBytes checked */
 
        /* all data written, release queue entry */
        ReleaseQueEntry();
        
        /* reset offset counter */
        RDS_SET(u16_CsalMsgOfs, 0u);
        
        /* HalcCsalMessage completely received */
        b_csalMsgRcvd = TRUE;
      }
    }
  }
  return b_csalMsgRcvd;
}

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
UINT8 aicMsgCsal_CalcNumOfFragsToSkip(UINT16 u16_length)
{
  UINT8 u8_noFragToSkip = 0u;

  /* if fragmentation required */
  if (u16_length > AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE)
  {
    /* result of calculation will fit into UINT8 because overall length is limited by previous check
       of (aicMsgCsal_CheckRes) */
    u8_noFragToSkip = (UINT8)((u16_length - (AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE + 1u))  / 
                              ((UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT))) + (UINT8)1u;
  }
  /* else: no fragmentation required */
  else
  {
    /* no further fragments to skip */
    u8_noFragToSkip = 0u;
  }
  return u8_noFragToSkip;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    CopyRcvdCsalFrag

  Description:
    This function is used to copy the HalcCsalMessage payload from the source to the receive buffer.
    The given size and offset are checked, so that no buffer overrun can occur.
    If the parameters are invalid the Safety Handler is called.

  See also:
    -

  Parameters:
    pu8_src (IN)              - Pointer to the source buffer (non-safe part of RX telegram)
    u8_srcOfs (IN)            - Offset inside non-safe part of RX telegram)
    pu8_dst (IN)              - Pointer to destination buffer (CSAL queue entry data field)
    u16_dstOfs (IN)           - Offset inside CSAL queue entry data field
    u8_size (IN)              - Number of bytes to copy

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void CopyRcvdCsalFrag(CONST UINT8* pu8_src,
                             CONST UINT8  u8_srcOfs,
                             volatile UINT8* pu8_dst,
                             CONST UINT16 u16_dstOfs,
                             CONST UINT8  u8_size)
{
  /* parameter check: if parameters lead to overrun of Parameters, then parameter error */
  if ( (((UINT16)u8_srcOfs + u8_size)  > (UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT)) ||
       ((u16_dstOfs + (UINT16)u8_size) > AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE) )
       
  {
    /* invalid values of parameters! */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: parameter OK */
  else
  {
    /*lint -esym(960, 17.4) */
    stdlibHAL_ByteArrCopy(pu8_dst + u16_dstOfs,
                          pu8_src + u8_srcOfs,
                          (UINT16)u8_size);
    /*lint +esym(960, 17.4) */
  }
}

/***************************************************************************************************
  Function:
    AquireQueEntry

  Description:
    This function returns a pointer to the next free queue element.
    It is task of the application to fill/copy the necessary data inside the queue element.
    The function return always the same queue element until the entry is released.

  See also:
    ReleaseQueEntry()

  Parameters:
    -

  Return value:
    AICMSGCSAL_t_AICMSG* - NULL: No resource to store further element inside queue
                         - <>NULL: Address of queue element writable

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC volatile AICMSGCSAL_t_AICMSG* AquireQueEntry(void)
{
  volatile AICMSGCSAL_t_AICMSG* ps_queElem;

  /* if queue is full */
  if ( RDS_GET(u16_UcqNumFree) == 0U )
  {
    /* queue queue is full! */
    ps_queElem = NULL;
  }
  /* else: queue is not full */
  else
  {
    ps_queElem = &as_Ucq[RDS_GET(u16_UcqWrIdx)];
  }
  return ps_queElem;
}

/***************************************************************************************************
  Function:
    ReleaseQueEntry

  Description:
    This function is used to release the write access to the current queue element. This function 
    must be called if a queue element is written completely.

  See also:
    AquireQueEntry()

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void ReleaseQueEntry(void)
{
  /* update queue management variables */
  RDS_DEC(u16_UcqNumFree);
  RDS_INC(u16_UcqWrIdx);

  /* handle wrap-around of write index */
  /* if wrap-around */
  if ( RDS_GET(u16_UcqWrIdx) == AICMSGCSAL_cfg_UCQ_SIZE )
  {
    RDS_SET(u16_UcqWrIdx, 0U);
  }
  /* else: no warp-around */
  else
  {
    /* empty branch */
  }
}

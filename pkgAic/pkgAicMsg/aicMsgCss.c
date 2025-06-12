/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCss.c
**     Summary: This module is used to assemble/fragment the CSS requests that are sent to AM
**              via AIC.
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCss_BuildReqMsg
**
**             WriteMsg
**             CopySendCssFrag
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

/* module includes */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "rds.h"

#include "stdlib-hal.h"

/* CSOS common headers */
#include "CSOSapi.h"

#include "HALCSmsg_cfg.h"
#include "HALCSmsg.h"

#include "aicMsgDef.h"
#include "aicMsgSis.h"
#include "aicMsgHalc.h"
#include "aicMsgCss.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      6u

/* Init-value (RDS_UINT16) of local offset */
#define k_CSS_OFS_INIT            {{(UINT16)0u},{(UINT16)~(UINT16)0u}}

/* current value of Module Status LED (according to SIS specification) */
STATIC UINT8 u8_ModStsLed = 0u;
  
/* current value of Network Status LED (according to SIS specification) */
STATIC UINT8 u8_NetStsLed = 0u;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC BOOL WriteMsg (CONST volatile HALCS_t_AICMSG *ps_halcMsg,
                      AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg);

STATIC void CopySendCssFrag(CONST volatile UINT8* pu8_src,
                            CONST UINT16 u16_srcOfs,
                            UINT8* pu8_dst,
                            CONST UINT8  u8_dstOfs,
                            CONST UINT16 u16_size);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgCss_BuildReqMsg

  Description:
    builds/prepares a message request (request from ASM to AM/ABCC).
    Currently, there are three possible AIC requests types:
    - HalcCssMessage
    - LEDstate
    - SafetyReset
    All messages requested by SAPL or CSS are mapped to one of these request types.
    The message request is specified in the Serial Interface Specification [SIS],
    in this document the structure of the message request and the meaning of the several requests
    is defined.
    The function fulfills also [SRS_440].

  See also:
    -

  Parameters:
    ps_halcMsg (IN)          - HALCS Message located inside HALCS queue
                               (valid range: <>NULL, not checked, already checked in
                                aicMsgCss_BuildMsg)
    ps_txMsg (OUT)           - Buffer of transmit message
                               (valid range: <>NULL, not checked, only called with reference to
                               buffer)

  Return value:
    <>0                      - Request generated, Expected response ID
    0                        - Request pending (fragmented transfer, more fragments necessary)

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT16 aicMsgCss_BuildReqMsg (CONST volatile HALCS_t_AICMSG *ps_halcMsg,
                              AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg)
{
  /* return value of this function, indicates if request is built completely (<>0) */
  UINT16 u16_pendAmResp = 0u;

  switch(ps_halcMsg->u16_cmd)
  {
    /* T100 "HalcCssMessage", see [SIS_204], [SIS_206] */
    /* AICMSGHDL_ASM_HALC_CSS_MSG_REQ */
    case CSOS_k_CMD_IXSSO_DEV_STATE:   /* fall through */
    case CSOS_k_CMD_IXSSO_SNN:         /* fall through */
    case CSOS_k_CMD_IXSSC_TERMINATE:   /* fall through */
    case CSOS_k_CMD_IXSMR_EXPL_RES:    /* fall through */
    case CSOS_k_CMD_IXSCE_SOPEN_RES:   /* fall through */
    case CSOS_k_CMD_IXSCE_SCLOSE_RES:  /* fall through */
    case CSOS_k_CMD_IXSCE_CNXN_CLOSE:  /* fall through */
    {
      /* Since the data does sometimes not fit into a single message,
       * assembling of the message shall be done here */
      /* if CSS message is written completely */
      if (TRUE == WriteMsg(ps_halcMsg, ps_txMsg))
      {
        /* ASM request generated, not pending any more */
        /* HalcCssMessage request build, now the response from AM is expected during the
         * next cycles, used to detect unexpected responses */
        u16_pendAmResp = AICMSGHDL_ASM_HALC_CSS_MSG_RESP;
      }
      /* else: still fragments to send */
      else
      {
        /* ASM request not generated (pending) */
        u16_pendAmResp = 0u;
      }
      break;
    }

    /* "LEDstate", see [SIS_207], [SRS_2126] */
    case HALCS_k_CMD_MOD_STAT_LED_SET:
    {
      /* ASM shall not support multiple requests, set message ID to 0,
       * see [SRS_302], [SIS_028] */
      ps_txMsg->sAicMsgHeader.u8MsgId = AICMSGHDL_DEF_REQ_ID;
      /* set function ID and CMD bit, see [SRS_437] */
      ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_LED_STATE_REQ;
      ps_txMsg->sAicMsgHeader.u8MsgLength   = AICMSGHDL_ASM_LED_STATE_REQ_LENGTH;

      /* store new Module Status LED */
      u8_ModStsLed = (UINT8)ps_halcMsg->u32_addInfo;
      /* set Module Status LED  */
      ps_txMsg->au8AicMsgData[0u] = u8_ModStsLed;
      /* set Network Status LED */
      ps_txMsg->au8AicMsgData[1u] = u8_NetStsLed;

      /* LEDstate request build, now the response from AM is expected during the
       * next cycles, used to detect unexpected responses */
      u16_pendAmResp = AICMSGHDL_ASM_LED_STATE_RESP;
      break;
    }

    /* "LEDstate", see [SIS_207], [SRS_2126] */
    case HALCS_k_CMD_NET_STAT_LED_SET:
    /* AICMSGHDL_ASM_LED_STATE_REQ */
    {
      /* ASM shall not support multiple requests, set message ID to 0,
       * see [SRS_302], [SIS_028] */
      ps_txMsg->sAicMsgHeader.u8MsgId = AICMSGHDL_DEF_REQ_ID;
      /* set function ID and CMD bit, see [SRS_437] */
      ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_LED_STATE_REQ;
      ps_txMsg->sAicMsgHeader.u8MsgLength   = AICMSGHDL_ASM_LED_STATE_REQ_LENGTH;

      /* store new network Status LED */
      u8_NetStsLed = (UINT8)ps_halcMsg->u32_addInfo;
      /* set Module Status LED  */
      ps_txMsg->au8AicMsgData[0u] = u8_ModStsLed;
      /* set Network Status LED */
      ps_txMsg->au8AicMsgData[1u] = u8_NetStsLed;

      /* LEDstate request build, now the response from AM is expected during the
       * next cycles, used to detect unexpected responses */
      u16_pendAmResp = AICMSGHDL_ASM_LED_STATE_RESP;
      break;
    }

    /* "SafetyReset", see [SIS_219], [SRS_2164] */
    /* AICMSGHDL_ASM_SAFETY_RESET_REQ */
    case HALCS_k_CMD_SAFETY_RESET_TYPE0:
    {
      /* ASM shall not support multiple requests, set message ID to 0,
       * see [SRS_302], [SIS_028] */
      ps_txMsg->sAicMsgHeader.u8MsgId = AICMSGHDL_DEF_REQ_ID;
      /* set function ID and CMD bit, see [SRS_437] */
      ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_SAFETY_RESET_REQ;
      ps_txMsg->sAicMsgHeader.u8MsgLength   = AICMSGHDL_ASM_SAFETY_RESET_REQ_LENGTH;

      /* Safety Reset Type is set here */
      ps_txMsg->au8AicMsgData[0u] = AICMSGHDL_ASM_SAFETY_RESET_TYPE0;
      /* reserved byte */
      ps_txMsg->au8AicMsgData[1u] = 0u;

      /* SafetyReset request build */
      u16_pendAmResp = AICMSGHDL_ASM_SAFETY_RESET_RESP;
      break;
    }
    /* "SafetyReset", see [SIS_219], [SRS_2164] */
    /* AICMSGHDL_ASM_SAFETY_RESET_REQ */
    case HALCS_k_CMD_SAFETY_RESET_TYPE1:
    {
      /* ASM shall not support multiple requests, set message ID to 0,
       * see [SRS_302], [SIS_028] */
      ps_txMsg->sAicMsgHeader.u8MsgId = AICMSGHDL_DEF_REQ_ID;
      /* set function ID and CMD bit, see [SRS_437] */
      ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_SAFETY_RESET_REQ;
      ps_txMsg->sAicMsgHeader.u8MsgLength   = AICMSGHDL_ASM_SAFETY_RESET_REQ_LENGTH;

      /* Safety Reset Type is set here */
      ps_txMsg->au8AicMsgData[0u] = AICMSGHDL_ASM_SAFETY_RESET_TYPE1;
      /* reserved byte */
      ps_txMsg->au8AicMsgData[1u] = 0u;

      /* SafetyReset request build */
      u16_pendAmResp = AICMSGHDL_ASM_SAFETY_RESET_RESP;
      break;
    }
    /* "SafetyReset", see [SIS_219], [SRS_2164] */
    /* AICMSGHDL_ASM_SAFETY_RESET_REQ */
    case HALCS_k_CMD_SAFETY_RESET_TYPE2:
    {
      /* ASM shall not support multiple requests, set message ID to 0,
       * see [SRS_302], [SIS_028] */
      ps_txMsg->sAicMsgHeader.u8MsgId = AICMSGHDL_DEF_REQ_ID;
      /* set function ID and CMD bit, see [SRS_437] */
      ps_txMsg->sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_SAFETY_RESET_REQ;
      ps_txMsg->sAicMsgHeader.u8MsgLength   = AICMSGHDL_ASM_SAFETY_RESET_REQ_LENGTH;

      /* Safety Reset Type is set here */
      ps_txMsg->au8AicMsgData[0u] = AICMSGHDL_ASM_SAFETY_RESET_TYPE2;
      /* Attribute Bit Map */
      ps_txMsg->au8AicMsgData[1u] = (UINT8)ps_halcMsg->u32_addInfo;

      /* SafetyReset request build */
      u16_pendAmResp = AICMSGHDL_ASM_SAFETY_RESET_RESP;
      break;
    }

    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
  }
  return u16_pendAmResp;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    WriteMsg

  Description:
    To be called when a message request containing a HalcCssMessage shall be sent to the AM. Many
    HalcCssMessage requests are "fragmented" messages, means more than one telegram is needed to
    transmit all parameters (or the whole message). The function will count the number of sent
    bytes and indicate if the message is still pending (not all parts/fragments sent) or
    not (all fragments of message sent).
    Hint: Message fragmentation is described in Serial Interface Specification [SIS] and Software
    Requirements Specification[SRS]

  See also:
    -

  Parameters:
    ps_halcMsg (IN)          - HALCS Message located inside HALCS queue
                               (valid range: <>NULL, not checked, already checked in
                               aicMsgCss_BuildMsg)
    ps_txMsg (OUT)           - Buffer of transmit message
                               (valid range: <>NULL, not checked, only called with reference to
                               buffer)

  Return value:
    TRUE                     - CSS message written completely
    FALSE                    - Still CSS fragments to send

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL WriteMsg (CONST volatile HALCS_t_AICMSG *ps_halcMsg,
                      AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg)
{
  /* RDS union initializations */
  /* Deactivated Lint message 708, because a Union initialisation is wanted to save
  ** init function */
  LOCAL_STATIC(, RDS_UINT16,  u16_CssMsgOfs, k_CSS_OFS_INIT); /*lint !e708*/

  /* variable to indicate if message was sent completely */
  BOOL b_msgSent;
  
  /* cast generic non-safe message structure to HALC specific one */
  /* Deactivated Lint 
  ** Note 929: cast from pointer to pointer [possibly violation MISRA 2004 Rule 11.4]
  ** Info 740: Unusual pointer cast (incompatible indirect types) [MISRA 2004 Rule 1.2]
  ** cast is necessary and accepted here */
  AICMSGHALC_t_MSG* ps_txCssMsg = (AICMSGHALC_t_MSG*)ps_txMsg; /*lint !e740 !e929*/

  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_CssMsgOfs);
  /*lint +esym(960, 10.1)*/

  /* if the first fragment is going to be sent */
  if ( RDS_GET_VALUE(u16_CssMsgOfs) == 0u)
  {
    /* ASM shall not support multiple requests, set message ID to 0,
     * see [SRS_302] */
    ps_txCssMsg->s_aicHdr.u8_msgId = AICMSGHDL_DEF_REQ_ID;
    /* set function ID and CMD bit, see [SRS_437] */
    ps_txCssMsg->s_aicHdr.u16_msgReqResp = AICMSGHDL_ASM_HALC_CSS_MSG_REQ;
    /* add HALC header length to get length of AIC message */
    ps_txCssMsg->s_aicHdr.u16_msgLength = ps_halcMsg->u16_len + AICMSGHALC_k_HALC_HDR_LENGTH;
    
    /*****************************************/
    /* build HALC header for first fragment */
    /*****************************************/
    ps_txCssMsg->s_halcHdr.u16_halcCmd     = ps_halcMsg->u16_cmd;
    ps_txCssMsg->s_halcHdr.u32_halcAddInfo = ps_halcMsg->u32_addInfo;
    
    /* if AIC fragmentation is NOT required (data fits in one AIC Message) */
    if (ps_halcMsg->u16_len <= AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE)
    {
      /* if it is a command without data */
      if (ps_halcMsg->u16_len == 0u)
      {
        /* this command has no data so we are finished */
      }
      else
      {
        /* copy the remaining data into the message */
        /* lint Note 928 deactivated, pointer casting is OK! */
        CopySendCssFrag(ps_halcMsg->au8_data, /*lint !e928*/
                        (UINT16)0u,                           /* src offset */
                        (UINT8*)ps_txCssMsg,     /*lint !e928*/
                        AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS,  /* dst offset */
                        ps_halcMsg->u16_len);
      }   
      /* reset offset counter */
      RDS_SET(u16_CssMsgOfs, 0u);
      
      /* HalcCssMessage completely sent */
      b_msgSent = TRUE;
    }
    /* else: AIC fragmentation required */
    else
    {
      /* Fill up the first fragment with the first 5 data bytes */
      /* lint Note 928 deactivated, pointer casting is OK! */
      CopySendCssFrag(ps_halcMsg->au8_data,  /*lint !e928*/
                      (UINT16)0u,                           /* src offset */
                      (UINT8*)ps_txCssMsg,      /*lint !e928*/
                      AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS,  /* dst offset */
                      (UINT16)AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE);
      
      /* add copied bytes to offset counter */
      RDS_ADD(u16_CssMsgOfs, AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE);
      
      /* HalcCssMessage NOT completely sent, fragments still missing */
      b_msgSent = FALSE;
    }
  }
  /* else: ongoing AIC transfer */
  else
  {
    /* calculate number of remaining bytes required to transfer the message,
    ** depending on this value, the decision is made to transfer a full
    **  middle or the last fragment */ 
    UINT16 u16_remBytes = ps_halcMsg->u16_len - RDS_GET_VALUE(u16_CssMsgOfs);

    /* if at least one more fragment is necessary */
    if ( u16_remBytes > (UINT8)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT) )
    {
      /* n-th. fragment, now the "data only" buffer is used, because
       * in this case it was a message without header */
      /* lint Note 928 deactivated, pointer casting is OK! */
      CopySendCssFrag(ps_halcMsg->au8_data,  /*lint !e928*/
                      RDS_GET_VALUE(u16_CssMsgOfs),
                      (UINT8*)ps_txMsg,      /*lint !e928*/
                      (UINT8)0u,
                      (UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT));

      /* add copied bytes to offset counter */
      RDS_ADD(u16_CssMsgOfs, (UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT));
      
      /* HalcCssMessage NOT completely sent, fragments still missing */
      b_msgSent = FALSE;
    }
    /* else: last fragment */
    else
    {
      /* last fragment (length depending on HalcCss message)
       * now the "data only" buffer is used, because in this case it was a
       * message without header */
      /* lint Note 928 deactivated, pointer casting is OK! */
      CopySendCssFrag(ps_halcMsg->au8_data,  /*lint !e928*/
                      RDS_GET_VALUE(u16_CssMsgOfs),
                      (UINT8*)ps_txMsg,      /*lint !e928*/
                      (UINT8)0u,
                      u16_remBytes);

      /* reset offset counter */
      RDS_SET(u16_CssMsgOfs, 0u);
      
      /* HalcCssMessage completely sent */
      b_msgSent = TRUE;
    }
  }
  
  return b_msgSent;
}

/***************************************************************************************************
  Function:
    CopySendCssFrag

  Description:
    This function is used to copy the HalcCssMessage payload from the source to the AIC message 
    structure.
    The given size and offset are checked, so that no buffer overrun can occur.
    If the parameters are invalid the Safety Handler is called.

  See also:
    -

  Parameters:
    pu8_src (IN)              - Pointer to the source buffer ((CSS queue entry data field)
    u16_srcOfs (IN)           - Offset inside CSS queue entry data field
    pu8_dst (IN)              - Pointer to destination buffer (non-safe part of TX telegram)
    u8_dstOfs (IN)            - Offset inside non-safe part of TX telegram)
    u16_size (IN)             - Number of bytes to copy

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void CopySendCssFrag(CONST volatile UINT8* pu8_src,
                            CONST UINT16 u16_srcOfs,
                            UINT8* pu8_dst,
                            CONST UINT8  u8_dstOfs,
                            CONST UINT16 u16_size)
{
  /* parameter check: if parameters lead to overrun of Parameters, then parameter error */
  if ( (((UINT32)u16_srcOfs + u16_size) > HALCSMSG_cfg_REQ_PAYLOAD_SIZE) ||
       (((UINT32)u8_dstOfs + u16_size) > sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT)) ) 
  {
    /* invalid values of parameters! */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: parameter OK */
  else
  {

    /*lint -esym(960, 17.4) */
    stdlibHAL_ByteArrCopy(pu8_dst + u8_dstOfs,
                          pu8_src + u16_srcOfs,
                          u16_size);
    /*lint +esym(960, 17.4) */
  }
}

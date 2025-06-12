/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgBld.c
**     Summary: This module contains the functionality to send the response of a received AM
**              request or to initiate a new ASM request.
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgBld_BuildMsg
**             aicMsgBld_PrepRespHdr
**             aicMsgBld_SetMsgState
**
**             BuildRespMsg
**             BuildDefRespMsg
**             BuildErrRespMsg
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


#include "HALCSmsg_cfg.h"
#include "HALCSmsg.h"

#include "aicMsgDef.h"
#include "aicMsgSis.h"
#include "aicMsgCss.h"

#include "aicMsgBld.h"
#include "aicMsgErr.h"
#include "aicMsgCtrlSts.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgBld_s_SendMsg:
** non-safe message from ASM to AM, global used due to performance reasons
** formerly aicDataStack_sAsmMsg
*/
AICMSGDEF_NON_SAFE_MSG_STRUCT aicMsgBld_s_SendMsg =
{
   {0x00u, 0x0000u, 0x00u},
   {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u}
};

/* aicMsgBld_e_ConfAmFrag:
** Bool to identify if the AM fragmentation may be confirmed or not
*/
TRUE_FALSE_ENUM aicMsgBld_e_ConfAmFrag = eFALSE;

/* aicMsgBld_u16_PendingAmRes:
** global to store which AM-response is currently pending
*/
UINT16 aicMsgBld_u16_PendingAmRes      = 0u;

/* aicMsgBld_e_RxState:
** State machine variable used for reception handling.
*/
AICMSGPROC_t_STATE aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_RESP_PROC;


/* aicMsgBld_e_TxState:
** State machine variable used for transfer handling.
*/
AICMSGBLD_t_STATE aicMsgBld_e_TxState   = AICMSGBLD_k_WF_ASM_REQ_RESP_BUILD;


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      9u

/* Init-value (RDS_UINT8) of local message counter */
#define k_MSGCNT_GETCLASSID_INIT        {{(UINT8)0u},{(UINT8)~(UINT8)0u}}

/* response length of the 'GetClassIDs' request */
#define k_GETCLASSID_RESP_LEN           (16u)
/* response length of the 'GetAssemblyInstIDs' request */
#define k_GETASSINSTID_RESP_LEN         (6u)

/* u8_LastRecvdMsgId:
** To keep track of requests and their corresponding response.
*/
STATIC UINT8  u8_LastRecvdMsgId = 0u;

/* u16_LastRecvdMsgReqResp:
** Describes what is contained in the data part of the message.
*/
STATIC UINT16 u16_LastRecvdMsgReqResp = 0u;

/* u16_AmMsgState
** message state assigned to the incoming telegram from AM. Used to indicate if the incoming 
** message is valid/invalid/etc. (see also [SRS_444], [SIS_054])
*/
STATIC UINT16 u16_AmMsgState = AICMSGHDL_MSG_VALID;

/* s_EmptyMsg:
** clear/empty content of non-safe message from ASM to AM, used to reset the non-safe 
** TX-Message buffer to zero values
*/
STATIC CONST AICMSGDEF_NON_SAFE_MSG_STRUCT s_EmptyMsg =
{
   {0x00u, 0x0000u, 0x00u},
   {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u}
};

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC BOOL BuildRespMsg (void);
STATIC BOOL BuildDefRespMsg(void);
STATIC void BuildErrRespMsg (void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgBld_BuildMsg

  Description:
    Builds the non-safe message which shall be sent to the AM.
    This message can be a request to the AM or a response to a request which came from the AM.
    This function prepares the next non-safe message which shall be sent, by calling functions which
    update the message buffers:
    - If the last ASM fragmentation bit received does not match the last one send (checked by 
      "aicMsgCtrlSts_IsAsmFragConf"), or if currently fragments of a message are pending, then the 
      message buffer will not be changed, only the function handling the Status/Ctrl Byte is called,
      without toggling the ASM fragmentation flag.
    - In other cases:
      If there is a response to be sent by the ASM, then the function to build the response is 
        called and the variable to request a toggle of ASM fragmentation flag is set to eTRUE.
      Else if there is a request to be sent by the ASM, then the function to build the request is 
        called and the variable to request a toggle of ASM fragmentation flag is set to eTRUE.
      Else if there is nothing "new" to be sent, the variable to request a toggle of ASM 
        fragmentation flag is set to eFALSE.

      The function to build Status/Ctrl Byte is called, with the previously set variable of ASM 
      fragmentation flag is set as parameter.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgBld_BuildMsg (void)
{
  BOOL b_toggleAsmFragFlag;
  CONST volatile HALCS_t_AICMSG *ps_txHalcMsg = NULL;

  /* was the ASM fragmentation flag confirmed by the AM/ABCC (see [SRS_369]) */
  if ( TRUE == aicMsgCtrlSts_IsAsmFragConf() )
  {
    switch (aicMsgBld_e_TxState)
    {
      /* the ASM is able to transfer a NEW response or request */
      case AICMSGBLD_k_WF_ASM_REQ_RESP_BUILD:
      {
        /* if ASM shall generate a response to a message 
        ** This means all fragments of an incoming message are received (see [SRS_374]) 
        */
        if (aicMsgBld_e_RxState == AICMSGPROC_k_WF_ASM_RESP_BUILD)
        {
          /* build the response to a message */
          /* if all fragments generated/built necessary for response */
          if (TRUE == BuildRespMsg())
          {
            /* switch into state to be able to process another request/response */
            aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_RESP_PROC;
          }
          /* else: still fragments to send */
          else
          {
            /* empty branch, stay in state */
          }
          
          /* ASM has something new to tell... (see [SRS_361]) */
          b_toggleAsmFragFlag = TRUE;
        }
        /* no response shall be generated, how about a new ASM request? */
        else
        {
          /* check message queue for NEW messages (generated by SAPL/CSS) */
          ps_txHalcMsg = HALCS_MsgTxReqGet();
          /* if SAPL/CSS message (ASM request) is available to send */
          if (ps_txHalcMsg != NULL)
          {
            /* clear message content (in fact optional, not safety relevant) */
            aicMsgBld_s_SendMsg = s_EmptyMsg;

            /* Some request from background task expects actions to be done in IRQ scheduler task 
            ** (e.g. update of internal Module Status LED and Network status LED). 
            ** If necessary a ASM request message is triggered afterwards */
            
            /* Generate ASM request, "aicMsgBld_u16_PendingAmRes" shall be set to default
            **(in aicMsgCss_BuildReqMsg) when request message is built completely.
            ** Due to fragmentation this can last several cycles */

            /* call function to build request */
            aicMsgBld_u16_PendingAmRes = aicMsgCss_BuildReqMsg(ps_txHalcMsg,
                                                               &aicMsgBld_s_SendMsg);
            /* if all fragments generated/built necessary for request */
            if (aicMsgBld_u16_PendingAmRes != 0u)
            {
              /* no fragmentation required, so release from queue */
              HALCS_MsgTxReqRelease();
              /* switch in state in order to wait for response */
              aicMsgBld_e_TxState = AICMSGBLD_k_WF_AM_RES_PROC;
            }
            /* else: more fragments necessary */
            else
            {
              /* switch in state to transfer next fragment */
              aicMsgBld_e_TxState = AICMSGBLD_k_WF_ASM_REQ_BUILD;
            }

            /* ASM has something new to tell... (see [SRS_361]) */
            b_toggleAsmFragFlag = TRUE;
          }
          /* else: no ASM request */
          else
          {
            /* else: no new data */
            b_toggleAsmFragFlag = FALSE;
          }
        }

        /* Call the function to build the Ctrl/Status Byte. If the content to be sent has changed
        ** (ASM has something new to tell), the Ctrl/Status Byte will be built with toggled ASM
        ** fragmentation flag, see [SRS_361]. */
        aicMsgCtrlSts_BuildCtrlStsByte(b_toggleAsmFragFlag,
                                      aicMsgBld_e_ConfAmFrag);
        /* reset confirmation flag */
        aicMsgBld_e_ConfAmFrag = eFALSE;
        break;
      }
      
      /* ASM need to transfer more request fragments */
      case AICMSGBLD_k_WF_ASM_REQ_BUILD:
      {
        /* get current queue element for next fragment */
        ps_txHalcMsg = HALCS_MsgTxReqGet();
        /* if SAPL/CSS message is available to send */
        if (ps_txHalcMsg != NULL)
        {
          /* clear message content (in fact optional, not safety relevant) */
          aicMsgBld_s_SendMsg = s_EmptyMsg;

          /* Generate ASM request, "aicMsgBld_u16_PendingAmRes" shall be set to default
          * (in aicMsgCss_BuildReqMsg) when request message is built completely.
          * Due to fragmentation this can last several cycles */

          /* call function to build request */
          aicMsgBld_u16_PendingAmRes = aicMsgCss_BuildReqMsg(ps_txHalcMsg,
                                                             &aicMsgBld_s_SendMsg);
          /* if all fragments generated/built necessary for request */
          if (aicMsgBld_u16_PendingAmRes != 0u)
          {
            /* now that the command is completely sent we can release it */
            HALCS_MsgTxReqRelease();
            /* switch in state in order to wait for response */
            aicMsgBld_e_TxState = AICMSGBLD_k_WF_AM_RES_PROC;
          }
          /* else: more fragments necessary */
          else
          {
            /* stay in state until all fragments sent */
          }

          /* ASM has something new to tell... (see [SRS_361]) */
          b_toggleAsmFragFlag = TRUE;

          /* Call the function to build the Ctrl/Status Byte. If the content to be sent has changed
          ** (ASM has something new to tell), the Ctrl/Status Byte will be built with toggled ASM
          ** fragmentation flag, see [SRS_361]. */
          aicMsgCtrlSts_BuildCtrlStsByte(b_toggleAsmFragFlag,
                                         aicMsgBld_e_ConfAmFrag);
          /* reset confirmation flag */
          aicMsgBld_e_ConfAmFrag = eFALSE;
        }
        /* else: no message in queue although pending transfer... */
        else
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
        }
        break;
      }
      
      /* ASM is waiting for a response */
      case AICMSGBLD_k_WF_AM_RES_PROC:
      {
        if (aicMsgBld_e_RxState == AICMSGPROC_k_WF_ASM_RESP_BUILD)
        {
          /* build the response to a message */
          /* if all fragments generated/built necessary for response */
          if (TRUE == BuildRespMsg())
          {
            /* switch into state to be able to process another request/response */
            aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_RESP_PROC;
          }
          /* else: still fragments to send */
          else
          {
            /* empty branch, stay in state */
          }

          /* ASM has something new to tell... (see [SRS_361]) */
          b_toggleAsmFragFlag = TRUE;
        }
        else
        {
          /* wait until reset by receive path */
          b_toggleAsmFragFlag = FALSE;
        }

        /* Call the function to build the Ctrl/Status Byte. If the content to be sent has changed
        ** (ASM has something new to tell), the Ctrl/Status Byte will be built with toggled ASM
        ** fragmentation flag, see [SRS_361]. */
        aicMsgCtrlSts_BuildCtrlStsByte(b_toggleAsmFragFlag,
                                       aicMsgBld_e_ConfAmFrag);
        /* reset confirmation flag */
        aicMsgBld_e_ConfAmFrag = eFALSE;
        break;
      }

      default:
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
      }
    }
  }
  /* else: fragment not confirmed yet, nothing to do */
  else
  {
    /* leave old message content (fulfills also [SRS_369]), call
     * function to build control/status byte, without toggling fragmentation
     * flag (see [SRS_361]) */
    aicMsgCtrlSts_BuildCtrlStsByte((BOOL)FALSE,
                                   aicMsgBld_e_ConfAmFrag);
    /* reset confirmation flag */
    aicMsgBld_e_ConfAmFrag = eFALSE;
  }
}

/***************************************************************************************************
  Function:
    aicMsgBld_PrepRespHdr

  Description:
    This function is used to:
     - set the Message ID used for the response generation.
     - set the Request/Response field used for the response generation.

  See also:
    -

  Parameters:
    u8_msgId (IN)            - Message ID received in request,  must be mirrored to the proper
                               response.
                               (valid range: any, not checked)
    u16_ReqResp (IN)         - Request/Response field received in request
                               (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgBld_PrepRespHdr(UINT8 u8_msgId, UINT16 u16_ReqResp)
{
  u8_LastRecvdMsgId       = u8_msgId;
  u16_LastRecvdMsgReqResp = u16_ReqResp;
}

/***************************************************************************************************
  Function:
    aicMsgBld_SetMsgState

  Description:
    This function is used to set the message state assigned to the incoming telegram from AM.
    The message state is used to indicate if the incoming message valid/invalid/etc. So either
    an error response or a valid response is built.

  See also:
    -

  Parameters:
    u16_msgState (IN)        - Message state of the incoming telegram.
                               (valid: AICMSGHDL_MSG_VALID, AICMSGHDL_MSG_UNSUPPORTED_FUNCTION,
                               AICMSGHDL_MSG_INVALID_REQUEST_LENGTH, AICMSGHDL_MSG_INVALID_DATA,
                               AICMSGHDL_MSG_INVALID_STATE, AICMSGHDL_MSG_OUT_OF_RES, checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgBld_SetMsgState(UINT16 u16_msgState)
{
  switch (u16_msgState)
  {
    case AICMSGHDL_MSG_VALID:
    case AICMSGHDL_MSG_UNSUPPORTED_FUNCTION:   /* fall through */
    case AICMSGHDL_MSG_INVALID_REQUEST_LENGTH: /* fall through */
    case AICMSGHDL_MSG_INVALID_DATA:           /* fall through */
    case AICMSGHDL_MSG_INVALID_STATE:          /* fall through */
    case AICMSGHDL_MSG_OUT_OF_RES:             /* fall through */
    {
      /* set message state */
      u16_AmMsgState = u16_msgState;
      break;
    }

    default:
    {
      /* should not enter here */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
      break;
    }
  }
}
 
/***************************************************************************************************
**    static functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    BuildRespMsg

  Description:
    builds/prepares the TX non-safe message/response.
    Depending on state the default or an error response is built. If there is a message pending 
    (e.g. acyclic requests with several fragments) no response has to be generated and this 
    function shall not be called.

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                     - Response generated/built completely
    FALSE                    - Still fragments necessary to be generated

  Remarks:
    -

***************************************************************************************************/
STATIC BOOL BuildRespMsg (void)
{
  BOOL b_respBuilt;

  /* clear message content (in fact optional, not safety relevant) */
  aicMsgBld_s_SendMsg = s_EmptyMsg;

  if (AICMSGHDL_MSG_VALID == u16_AmMsgState)
  {
    /* "default" response is the response in case that there was no error */
    b_respBuilt = BuildDefRespMsg();
  }
  else
  {
    /* build error response, see [SRS_444], [SIS_054] */
    BuildErrRespMsg();
    b_respBuilt = TRUE;
  }
  return b_respBuilt;
}

/***************************************************************************************************
  Function:
    BuildDefRespMsg

  Description:
    builds/prepares the default non-safe message part / response to be sent to AM(ABCC).
    The default response is the response to the last received non-safe message. 
    The response is specified in the Serial Interface Specification [SIS], in document the 
    structure of the response is defined. The function fulfills also [SRS_440].
    Just for info (not done in this function): The result will be merged together with the safe 
    packet into one telegram.

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                     - Response generated/built completely
    FALSE                    - Still fragments necessary to be generated

  Remarks:
    -

***************************************************************************************************/
STATIC BOOL BuildDefRespMsg(void)
{
  BOOL b_respBuilt = TRUE;

  switch (u16_LastRecvdMsgReqResp)
  {
     /* "GetStatus", see [SRS_480], [SIS_055] */
     case AICMSGHDL_AM_GET_ASM_STATUS_REQ:
     {
        /* generate response message containing error counters */
        aicMsgErr_BuildGetStsResp(&aicMsgBld_s_SendMsg, /* Tx message */
                                  u8_LastRecvdMsgId);
        
        /* response generated, not pending any more, return value already set */
        break;
     }

     /* "Error Confirmation", see [SRS_493], [SIS_057] */
     case AICMSGHDL_AM_ERROR_CONFIRM_REQ:
     {
        /* message header, [SIS_053] */
        /* copy message ID received inside the request to the response,
         * see [SRS_434], [SIS_029] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
        /* response bytes mirrored from request, but with bit 14 set to 0,
         * see [SRS_438], [SRS_439], [SIS_032], [SIS_033] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_ERROR_CONFIRM_RESP;
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_ERROR_CONFIRM_LENGTH;

        /* response generated, not pending any more, return value already set */
        break;
     }

     /* 'HalcCsalMessage', see [SIS_200], [SIS_202] */
     case AICMSGHDL_AM_HALC_CSAL_MSG_REQ:
     {
       /* copy message ID received inside the request to the response,
        * see [SRS_434], [SIS_029] */
       aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
       /* response bytes mirrored from request, but with bit 14 set to 0,
        * see [SRS_438], [SRS_439] */
       aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_HALC_CSAL_MSG_RESP;
       aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_HALC_CSAL_MSG_RESP_LENGTH;
       
       /* response generated, not pending any more, return value already set */
       break;
     }
       
     /* "SetInitData", see [SIS_203]  */
     case AICMSGHDL_AM_SET_INIT_DATA_REQ:
     {
        /* copy message ID received inside the request to the response,
         * see [SRS_434], [SIS_029] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
        /* response bytes mirrored from request, but with bit 14 set to 0,
         * see [SRS_438], [SRS_439] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_SET_INIT_DATA_RESP;
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_SET_INIT_DATA_RESP_LENGTH;
        
        /* response generated, not pending any more, return value already set */
        break;
     }
     
     /* "GetClassIDs", see [SIS_214] */
     case AICMSGHDL_AM_GET_CLASS_ID_REQ:
     {
        /* RDS union initializations */
        /* PC-Lint message 708 deactivated for this statement. */
        LOCAL_STATIC(, RDS_UINT8,  u8_GetClassIdFragNo, k_MSGCNT_GETCLASSID_INIT); /*lint !e708*/

        /* first fragment */
        if( 0u == RDS_GET(u8_GetClassIdFragNo) )
        {
          /* copy message ID received inside the request to the response,
           * see [SRS_434], [SIS_029] */
          aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
          /* response bytes mirrored from request, but with bit 14 set to 0,
           * see [SRS_438], [SRS_439] */
          aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_GET_CLASS_ID_RESP;
          /* set length of response */
          aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = k_GETCLASSID_RESP_LEN;
          /* set data of response (list of class ids implemented on the T100) */ 
          /* Words transfered in "little-endian"! */
          /* Safety Supervisor Object, Class Code: 0x39 */ 
          aicMsgBld_s_SendMsg.au8AicMsgData[0u] = 0x39u;
          aicMsgBld_s_SendMsg.au8AicMsgData[1u] = 0x00u;
          /* Safety Validator Object, Class Code: 0x3A */ 
          aicMsgBld_s_SendMsg.au8AicMsgData[2u] = 0x3Au;
          aicMsgBld_s_SendMsg.au8AicMsgData[3u] = 0x00u;
          /* Vendor-specific, Diagnostic Object, Class Code: 0x64 (see [SRS_2124])*/
          aicMsgBld_s_SendMsg.au8AicMsgData[4u] = 0x64u;
          aicMsgBld_s_SendMsg.au8AicMsgData[5u] = 0x00u;
          /* Vendor-specific, Failure Code Object, Class Code: 0x65 (see [SRS_2226]) */
          aicMsgBld_s_SendMsg.au8AicMsgData[6u] = 0x65u;
          aicMsgBld_s_SendMsg.au8AicMsgData[7u] = 0x00u;
          /* Profile Specific, Safety Discrete Input Point (SDIP), Class Code: 0x3D
           * (see [SRS_2087] */
          aicMsgBld_s_SendMsg.au8AicMsgData[8u] = 0x3Du;
          aicMsgBld_s_SendMsg.au8AicMsgData[9u] = 0x00u;
          /* Profile Specific, Safety Discrete Input Group (SDIG), Class Code: 0x3E
           * (see [SRS_2090]) */
          aicMsgBld_s_SendMsg.au8AicMsgData[10u] = 0x3Eu;
          aicMsgBld_s_SendMsg.au8AicMsgData[11u] = 0x00u;
         
          /* update message counter */
          RDS_SET(u8_GetClassIdFragNo,  1u);
          /* still fragment to be sent missing */
          b_respBuilt = FALSE;
        }
        /* second fragment */
        else if ( 1u == RDS_GET(u8_GetClassIdFragNo) )
        {
          /* Deactivate lint, cast is necessary to access structure byte by byte */
          UINT8* pu8_MsgDataTx = (UINT8*)&aicMsgBld_s_SendMsg; /*lint !e928 */
          
          /*lint -esym(960, 17.1) -esym(960, 17.4) */
          /* Profile Specific, Safety Discrete Output Point (SDOP), Class Code: 0x3B
           * (see [SRS_2094]) */
          pu8_MsgDataTx[0u] = 0x3Bu;
          pu8_MsgDataTx[1u] = 0x00u;
          /* Profile Specific, Safety Dual Channel output Object (SDCO), Class Code: 0x3F
           * (see [SRS_2097]) */
          pu8_MsgDataTx[2u] = 0x3Fu;
          pu8_MsgDataTx[3u] = 0x00u;
          /*lint +esym(960, 17.1) +esym(960, 17.4) */
          
          /* reset message counter  */
          RDS_SET(u8_GetClassIdFragNo, 0u);
          
          /* response generated, not pending any more, return value already set */
        }
        /* else: invalid */
        else
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
        }
        break;
     }
     
     /* "LinkStatus", see [SIS_216] */
     case AICMSGHDL_AM_LINK_STATUS_REQ:
     {
        /* copy message ID received inside the request to the response,
         * see [SRS_434], [SIS_029] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
        /* response bytes mirrored from request, but with bit 14 set to 0,
         * see [SRS_438], [SRS_439] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_LINK_STATUS_RESP;
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_LINK_STATUS_RESP_LENGTH;
        
        /* response generated, not pending any more, return value already set */
        break;
     }
     
     /* "GetAssemblyInstIDs", see [SIS_217] */
     case AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_REQ:
     {
        /* copy message ID received inside the request to the response,
         * see [SRS_434], [SIS_029] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
        /* response bytes mirrored from request, but with bit 14 set to 0,
         * see [SRS_438], [SRS_439] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_RESP;
        /* set length of response */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = k_GETASSINSTID_RESP_LEN;
        /* set data of response (list of assembly classes implemented on the T100) */ 
        /* Words transfered in "little-endian"! */
        /* Input Assembly, Instance Id: 0x264 (see [SRS_2093]) */
        aicMsgBld_s_SendMsg.au8AicMsgData[0u] = 0x64u;
        aicMsgBld_s_SendMsg.au8AicMsgData[1u] = 0x02u;
        /* Output Assembly, Instance Id: 0x300 (see [SRS_2102]) */
        aicMsgBld_s_SendMsg.au8AicMsgData[2u] = 0x00u;
        aicMsgBld_s_SendMsg.au8AicMsgData[3u] = 0x03u;
        /* Configuration Assembly, Instance Id: 0x340 (see [SRS_2120]) */
        aicMsgBld_s_SendMsg.au8AicMsgData[4u] = 0x40u;
        aicMsgBld_s_SendMsg.au8AicMsgData[5u] = 0x03u;
        
        /* response generated, not pending any more, return value already set */
        break;
     }
     
     /* "SetConfigString", see [SIS_056] */
     case AICMSGHDL_AM_SET_CONFIG_STRING_REQ:
     {
        /* copy message ID received inside the request to the response,
         * see [SRS_434], [SIS_029] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
        /* response bytes mirrored from request, but with bit 14 set to 0,
         * see [SRS_438], [SRS_439] */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_AM_SET_CONFIG_STRING_RESP;
        /* set length of response */
        aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_AM_SET_CONFIG_STRING_RESP_LENGTH;
        
        /* response generated, not pending any more, return value already set */
        break;
     }
     
     default:
     {
        /* should not enter here */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
        break;
     }
  }
  return b_respBuilt;
}

/***************************************************************************************************
  Function:
    BuildErrRespMsg

  Description:
    builds/prepares the non-safe message part in the error case.
    Just for info (not done in this function):
    The result will be merged together with the safe packet into one telegram.
    The error response is specified in the Serial Interface Specification [SIS], in this document 
    the structure of the error response is defined (see [SIS_054]). The function fulfills [SRS_440].

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    -

***************************************************************************************************/
STATIC void BuildErrRespMsg (void)
{

  /* build error response, see [SIS_054] */
  /* mirror id, see [SIS_029] */
  aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgId = u8_LastRecvdMsgId;
  /* set ERR bit, see [SRS_436], see [SIS_030] */
  aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp = \
     (u16_LastRecvdMsgReqResp) | AICMSGHDL_ERROR_RESPONSE_ID_MASK1;
  aicMsgBld_s_SendMsg.sAicMsgHeader.u16MsgReqResp &= AICMSGHDL_ERROR_RESPONSE_ID_MASK2;
  aicMsgBld_s_SendMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_ERROR_RESPONSE_LENGTH;

  /* 2 Byte message state, Words transfered in "little-endian" (see [SRS_475], [SIS_052]) */
  aicMsgBld_s_SendMsg.au8AicMsgData[0u] = (UINT8)(u16_AmMsgState & 0x00FFu);
  aicMsgBld_s_SendMsg.au8AicMsgData[1u] = (UINT8)( (u16_AmMsgState >> 8u) & 0x00FFu);
}

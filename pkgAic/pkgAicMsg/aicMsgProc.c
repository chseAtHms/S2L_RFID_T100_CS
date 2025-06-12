/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgProc.c
**     Summary: This module implements the functionality to handle incoming AM requests or
**              AM responses.
**   $Revision: 4331 $
**       $Date: 2024-03-13 08:58:56 +0100 (Mi, 13 Mrz 2024) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgProc_Init
**             aicMsgProc_ProcessMsg
**
**             ProcessMsg
**             ProcessReqMsg1frag
**             ProcessReqMsgNfrag
**             ProcessRespMsg
**             CalcNumOfFragmentsToSkip
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
#include "globErrHandler.h"
#include "globFail_SafetyHandler.h"
#include "globErrLog.h"

#include "rds.h"

#include "includeDiDo.h"
#include "fiParam.h"

#include "aicSm.h"
#include "aicMsgDef.h"
#include "aicMsgSis.h"
#include "aicMsgCsal_cfg.h"
#include "aicMsgCsal.h"
#include "aicMsgInit.h"
#include "aicMsgCfg.h"

#include "aicMsgBld.h"
#include "aicMsgProc.h"
#include "aicMsgErr.h"
#include "aicMsgCtrlSts.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgProc_s_RcvdMsg
** non-safe message from AM to ASM, global used due to performance reasons
** formerly aicDataStack_sAnybusMsgRx
*/
AICMSGDEF_NON_SAFE_MSG_STRUCT aicMsgProc_s_RcvdMsg =
{
   {0x00u, 0x0000u, 0x00u},
   {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u}
};


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      11u

/* Variable to store the number of fragments to be skipped because of AIC error */
STATIC UINT8 u8_FragmentsToSkip = 0u;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void ProcessMsg (void);
STATIC BOOL ProcessReqMsg1frag (UINT16* pu16_msgState);
STATIC void ProcessRespMsg (void);
STATIC BOOL ProcessReqMsgNfrag (UINT16* pu16_msgState);
STATIC UINT8 CalcNumOfFragmentsToSkip(UINT8 u8_length);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgProc_Init

  Description:
    Calls functions which init variables of modules

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void aicMsgProc_Init(void)
{
  /* init CSAL message module */
  aicMsgCsal_Init();
}

/***************************************************************************************************
  Function:
    aicMsgProc_ProcessMsg

  Description:
    process the incoming non-safe message in following steps:
    - process of the received ctrl/status byte
    - checks if a new fragment is received (check of Anybus-Fragmentation Bit)
    - if yes, the non-safe message will be processed:
       - if the received message is a request, the processing of a request is
         called
       - else the processing of a response is called
    - otherwise, non-safe message will be ignored

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgProc_ProcessMsg (void)
{
  /* call function to process ctrl/status byte of received telegram */
  aicMsgCtrlSts_Process();
  
  /* was the Anybus-Fragmentation bit of AM/ABCC toggled? If yes, new data has
   * been received (see [SRS_299]) */
  if ( TRUE == aicMsgCtrlSts_IsNewFragRcvd() )
  {
    ProcessMsg(); /* new data received */
  }
  /* else: nothing new from AM */
  else
  {
    /* nothing new from AM, ignore message content, see [SRS_300] */
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    ProcessMsg

  Description:
    process an incoming new message (identified by the AM fragmentation flag):
     - if it is the first fragment of a message the message will be processed
       depending on if it is a request or a response:
       - a new request will be accepted if not still an old one is to be
         answered by the ASM, else it will be ignored (info: this leads to
         re-send of request by AM)
       - in case of a response the function to process responses is called
     - if it is not the first fragment of a message, the corresponding function
       to process requests is called

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void ProcessMsg (void)
{
  /* message state of the incoming telegram */
  UINT16 u16_msgState;
  /* flag indicates if all expected fragments are received */
  BOOL b_msgCompleted;

  /* evaluate current state of receive path */
  switch (aicMsgBld_e_RxState)
  {
    /* ASM is able to receive a NEW message (request or response) from AM */
    case AICMSGPROC_k_WF_AM_REQ_RESP_PROC:
    {
      /* is incoming message a request? */
      if (AICMSGHDL_REQ_MASK == ((AICMSGHDL_REQ_MASK) & 
                                 (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp)))
      {
        /* Store Message ID, Function ID for response generation */
        aicMsgBld_PrepRespHdr(aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgId,
                              aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp);
  
        /* handle request message */
        b_msgCompleted = ProcessReqMsg1frag(&u16_msgState);

        /* set message state of incoming telegram (used for response generation) */
        aicMsgBld_SetMsgState(u16_msgState);

        /* if valid message was received */
        if (u16_msgState == AICMSGHDL_MSG_VALID)
        {
          /* if no further fragments are expected */
          if (b_msgCompleted == TRUE)
          {
            /* switch into state to generate response */
            aicMsgBld_e_RxState = AICMSGPROC_k_WF_ASM_RESP_BUILD;
          }
          /* else: further fragments expected */
          else
          {
            /* switch into state to receive more fragments */
            /* the proper state is already set in 'ProcessReqMsg1frag' */
          }
        }
        /* else: invalid message received */ 
        else
        {
          /* Attention: When we received a CSAL message with an invalid SIS command header length
          ** we should send the corresponding error directly. Then we set u8_FragmentsToSkip to zero
          ** here (without evaluating the 16-bit length field).
          ** In case of other errors in combination with CSAL messages (e.g. no space in queue
          ** available) we still wait the number of expected fragments.
          */
          if (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp == AICMSGHDL_AM_HALC_CSAL_MSG_REQ)
          {
            /* if there is a request length error */
            if (u16_msgState == AICMSGHDL_MSG_INVALID_REQUEST_LENGTH)
            {
              /* then we must send the response directly */
              u8_FragmentsToSkip = 0U;
            }
            /* else: other kind of error */
            else
            {
              /* calculate number of fragments to skip, an error response shall be sent after 
              ** ALL missing fragments are received see [SIS_054] */

              /* Calculate the 16 bit data length of a CSAL message:
              ** Generally SIS commands have an 8 bit length field. As an exception to this the
              ** length of the 'HalcCsalMessage' is a 16 bit field. Thus here we calculate the
              ** length from the regular length field plus the directly following first data byte
              ** field (considering the protocol is little endian).
              */
              UINT16 u16_len = aicMsgProc_s_RcvdMsg.au8AicMsgData[0];
              u16_len <<= 8u;
              u16_len += aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgLength;

              u8_FragmentsToSkip = aicMsgCsal_CalcNumOfFragsToSkip(u16_len);
            }
          }
          /* other/unknown command received (using 8-bit length field)*/
          else
          {
            /* calculate number of fragments to skip, an error response shall be sent after 
            ** ALL missing fragments are received see [SIS_054] */
            u8_FragmentsToSkip = 
              CalcNumOfFragmentsToSkip(aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgLength);
          }
          
          /* if still fragments to receive */
          if (u8_FragmentsToSkip > 0U)
          {
            /* set proper state (receive skipped fragments) */
            aicMsgBld_e_RxState = AICMSGPROC_k_SKIP_FRAGMENTS;
          }
          /* else: no further fragments expected */
          else 
          {
            /* switch into state to generate response */
            aicMsgBld_e_RxState = AICMSGPROC_k_WF_ASM_RESP_BUILD;
          } 
        }
      }
      /* else: incoming message is a response */
      else
      {
        /* if the reception of a response is expected at all */
        if (aicMsgBld_e_TxState == AICMSGBLD_k_WF_AM_RES_PROC)
        {
          /* handle response message */
          /* inside function call the global 'aicMsgProc_s_RcvdMsg' is read directly */
          ProcessRespMsg();
  
          /* request-response cycle finished, reset state machine of transmit path */
          aicMsgBld_e_TxState = AICMSGBLD_k_WF_ASM_REQ_RESP_BUILD;
        }
        /* else: no reception expected */
        else
        {
          /* unknown response, call function to increase discard error, see [SRS_432], [SIS_027] */
          aicMsgErr_DiscardError();
        }  
      }
      
      /* new request/response handled, confirm fragment */
      aicMsgBld_e_ConfAmFrag = eTRUE;
      break;
    }
    
    /* ASM expects the next request fragment for HalcCsalMessage, SetInitData,
    ** SetConfigString */
    case AICMSGPROC_k_WF_AM_REQ_HALCCSALMSG_FRAG_PROC:
    case AICMSGPROC_k_WF_AM_REQ_SETINITDATA_FRAG_PROC:  /* fall through */
    case AICMSGPROC_k_WF_AM_REQ_SETCFGSTR_FRAG_PROC:    /* fall through */
    {
      /* handle request message / new fragment */
      /* if all fragments received now */
      if (ProcessReqMsgNfrag(&u16_msgState) == TRUE)
      {
        /* set message state of incoming telegram (used for response generation) */
        aicMsgBld_SetMsgState(u16_msgState);
        /* switch into state to generate response */
        aicMsgBld_e_RxState = AICMSGPROC_k_WF_ASM_RESP_BUILD;
      }
      /* else: still fragments missing of request */
      else
      {
        /* empty branch, stay in state */
      }
  
      /* new fragment processed, confirm fragment */
      aicMsgBld_e_ConfAmFrag = eTRUE;
      break;
    }
  
    /* ASM expects that response is built by transmit path */
    /* No further fragment from AM is accepted here */     
    case AICMSGPROC_k_WF_ASM_RESP_BUILD:
    {
      /* stay in state until response is build */
      break;
    }

    case AICMSGPROC_k_SKIP_FRAGMENTS:
    {
      /* number of skipped frames is decremented */
      u8_FragmentsToSkip--;

       /* if no more fragment shall be skipped */
      if (u8_FragmentsToSkip == 0U)
      {
        /* switch into state to generate (error) response, see [SIS_054] */
        aicMsgBld_e_RxState = AICMSGPROC_k_WF_ASM_RESP_BUILD;
      }
      else /* fragment shall be skipped */
      {
        /* no action is necessary */
      }
      /* new fragment processed, confirm fragment */
      aicMsgBld_e_ConfAmFrag = eTRUE;
      break;
    }

    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
  }
/* Fix of not immediately sending error responses of fragmented messages lifted cyclomatic
   complexity above 15. Subsequent splitting of the function would increase overall complexity. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}

/***************************************************************************************************
  Function:
    ProcessReqMsg1frag

  Description:
    Process of the first fragment of the incoming non-safe message requests.
    Depending on the received Request different actions are executed.
    Steps of function:
    - the received length (Byte of message header) will be read and stored to
      a variable
    - for the different requests, the expected length will be compared to the one
      received. In case that both values are not equal, the message state
      will be set to "AICMSGHDL_MSG_INVALID_REQUEST_LENGTH"
    - for the different requests, different actions are executed as described
      in Serial Interface Specification [SIS]

  See also:
    -

  Parameters:
    pu16_msgState (OUT)      - Message state of the incoming telegram
                               (valid range: <>NULL, not checked, only called with reference to
                               variable)

  Return value:
    TRUE                     - Request received completely (all fragments received) or message is
                               not valid
    FALSE                    - Still fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL ProcessReqMsg1frag (UINT16* pu16_msgState)
{
  UINT8 u8_msgLen;
  BOOL b_reqRcvdCompl;
  UINT16 u16_msgState;
  BOOL b_csalMsgLengthInvalid;

  /* get the length out of the received message header. */
  /* ATTENTION: The length is NOT valid for SIS 'HalcCsalMessage' because
  ** this type is using a 16 Bit field for the length, see [SIS] */ 
  u8_msgLen = aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgLength;

  switch (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp)
  {
    /***********************************************************************************************
    * CIP Safety specific SIS commands
    ***********************************************************************************************/
    /* 'HalcCsalMessage', see [SIS_200], [SIS_202] */
    case AICMSGHDL_AM_HALC_CSAL_MSG_REQ:
    {   
      /* if CSAL Message Length exceeds the available resources */
      if (aicMsgCsal_CheckRes(&aicMsgProc_s_RcvdMsg, &b_csalMsgLengthInvalid) == FALSE)
      {
        /* if invalid CSAL message length is the reason for error */
        if (b_csalMsgLengthInvalid)
        {
          /* invalid length */
          u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
          /* We must send the error response immediately as the received length is invalid.
             Thus any guess how many fragments to wait will be wrong. */
          b_reqRcvdCompl = TRUE;
        }
        /* else: queue full */
        else
        {
          /* queue full, no resources available */
          u16_msgState = AICMSGHDL_MSG_OUT_OF_RES;
          /* Skip reception of further data. Caller will take care of still waiting for fragments
             (contents to skip) */
          b_reqRcvdCompl = TRUE;
        }
      }
      /* else: everything OK */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;
        
        /* Since the data does not fit into a single message, 
         * re-assembling of the message shall be done here
         * After re-assembling the request is put into a queue in order for
         * processing by the background task */
        /* if CSAL message received completely */
        if (TRUE == aicMsgCsal_ReadMsg(&aicMsgProc_s_RcvdMsg))
        {
          /* HalcCsalMessage received => message may be responded now and is
           * not "pending" any more */
          b_reqRcvdCompl = TRUE;
        }
        /* else: still CSAL message to receive */
        else
        {
          /* received message shall not be responded yet, because SW still waits
           * for other fragments and message is therefore "pending" */
          b_reqRcvdCompl = FALSE;
          /* switch into state for HalcCsalMessage fragment reception */
          aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_HALCCSALMSG_FRAG_PROC;
        }
      }
      break;
    }
    
    /* 'GetClassIDs', see [SIS_214] */
    case AICMSGHDL_AM_GET_CLASS_ID_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_GET_CLASS_ID_REQ_LENGTH != u8_msgLen)
      {
        /*invalid length */
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
      }
      /* else: request length valid */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;
      }
      /* no more fragments expected, send error/default response immediately */
      b_reqRcvdCompl = TRUE;
      break;
    }
    
    /* 'LinkStatus', see [SIS_216] */      
    case AICMSGHDL_AM_LINK_STATUS_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_LINK_STATUS_REQ_LENGTH != u8_msgLen)
      {
        /*invalid length */
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
      }
      /* else: request length valid */
      else
      {
        /* Attention: The current implementation of the T100/CS does NOT react on a 
        ** 'LinkStatus' request. Nevertheless, the 'LinkStatus' request is acknowledged here.
        ** In case of a 'LinkDown' the safe state is entered after the time-out of the Safety 
        ** Connection is detected.
        */
        u16_msgState = AICMSGHDL_MSG_VALID;
      }
      /* no more fragments expected, send error/default response immediately */
      b_reqRcvdCompl = TRUE;
      break;
    }
    
    /* 'GetAssemblyInstIDs', see [SIS_217] */
    case AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_REQ_LENGTH != u8_msgLen)
      {
        /*invalid length */
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
      }
      /* else: request length valid */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;
      }
      /* no more fragments expected, send error/default response immediately */
      b_reqRcvdCompl = TRUE;
      break;
    }
      
    /* 'SetInitData', see [SIS_203] */
    case AICMSGHDL_AM_SET_INIT_DATA_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_SET_INIT_DATA_REQ_LENGTH != u8_msgLen)
      {
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
        /* no more fragments expected, send error response immediately */
        b_reqRcvdCompl = TRUE;
      }
      /* else: request length valid */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;

        /* Save received CSS initialization data (see [SRS_2003]). 
         * Since the data does not fit into a single message, 
         * re-assembling of the message shall be done here */
        /* if received completely */
        if (TRUE == aicMsgInit_SetData(&aicMsgProc_s_RcvdMsg))
        {
          /* SetInitData received, shall not occur in first fragment
           * (remember fragmented transfer) */
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
          /* only used for unit test */
          b_reqRcvdCompl = TRUE;
        }
        /* else: still fragments to receive */
        else
        {
          /* received message shall not be responded yet, because SW still waits
           * for other fragments and message is therefore "pending" */
          b_reqRcvdCompl = FALSE;
          /* switch into state for SetInitData fragment reception */
          aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_SETINITDATA_FRAG_PROC;
        }
      }
      break; 
    }         

    /***********************************************************************************************
    * Common SIS commands
    ***********************************************************************************************/
    /* 'GetStatus', see [SIS_055] */
    case AICMSGHDL_AM_GET_ASM_STATUS_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_GET_ASM_STATUS_REQ_LENGTH != u8_msgLen)
      {
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
      }
      /* else: request length valid */
      else
      {
        /* nothing to do with received message data, only answer interesting */
        u16_msgState = AICMSGHDL_MSG_VALID;
      }
      /* no more fragments expected, send error/default response immediately */
      b_reqRcvdCompl = TRUE;
      break;
    }
    
    /* 'ErrorConfirmation', see [SIS_057] */
    case AICMSGHDL_AM_ERROR_CONFIRM_REQ:
    {
      /* handles the error reset by Error Confirmation Request received from the host 
      ** (see [SRS_2011], [SRS_2015]) */
      /* if called in invalid state */
      /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical 
       *  operator: '&&' --> no side effects because state only changed from IRQ context so 
       * reading even multiple times is safe */
      if ( (AICSM_AIC_EXEC_PROD_ONLY != aicSm_eAicState) &&
           (AICSM_AIC_EXEC_CONS_ONLY != aicSm_eAicState) &&  /*lint !e960 */
           (AICSM_AIC_EXEC_PROD_CONS != aicSm_eAicState) )   /*lint !e960 */
      {
        u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
      }
      /* if the Automatic Error Reset Feature is activated for DIs or
      ** DOs the Error Confirmation request from the host is ignored here.
      * (see [SRS_2219] */
      /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of 
       * logical operator: '||', OK here because always the complete IO Configuration data
       * is written at once from background, so no side effects here */
      else if ( (FIPARAM_DI_AUTOMATIC_RESET_ENABLED) ||
                (FIPARAM_DO_AUTOMATIC_RESET_ENABLED) ) /*lint !e960 */
      {
        u16_msgState = AICMSGHDL_MSG_UNSUPPORTED_FUNCTION;
      }
      /* if invalid request length */
      else if (AICMSGHDL_AM_ERROR_CONFIRM_LENGTH != u8_msgLen)
      {
        u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
      }
      /* else: everything fine... */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;
        /* call function for error reset, see [SRS_493], [SIS_034] */
        globErrHandler_ErrReset();
      }
      /* no more fragments expected, send error/default response immediately */
      b_reqRcvdCompl = TRUE;
      break;
    }

    /* 'SetConfigString', see [SIS_056], [SRS_2048] */
    case AICMSGHDL_AM_SET_CONFIG_STRING_REQ:
    {
      /* if invalid request length */
      if (AICMSGHDL_AM_SET_CONFIG_STRING_REQ_LENGTH != u8_msgLen)
      {
         u16_msgState = AICMSGHDL_MSG_INVALID_REQUEST_LENGTH;
         /* no more fragments expected, send error response immediately */
         b_reqRcvdCompl = TRUE;
      }
      /* else: request length valid */
      else
      {
        u16_msgState = AICMSGHDL_MSG_VALID;

        /* Save received configuration data (see [SRS_2048]).
         * Since the data does not fit into a single message, 
         * re-assembling of the message shall be done here */
        /* if received completely */
        if (TRUE == aicMsgCfg_SetData(&aicMsgProc_s_RcvdMsg))
        {
          /* SetConfigString received, shall not occur in first fragment
          * (remember fragmented transfer) */
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
          /* only used for unit test */
          b_reqRcvdCompl = TRUE;
        }
        /* else: still fragments to receive */
        else
        {
          /* received message shall not be responded yet, because SW still waits
           * for other fragments and message is therefore "pending" */
          b_reqRcvdCompl = FALSE;
          /* switch into state for SetConfigString fragment reception */
          aicMsgBld_e_RxState = AICMSGPROC_k_WF_AM_REQ_SETCFGSTR_FRAG_PROC;
        }
        break;
      }
      break;
    }

    default:
    {
      /* unknown IDs will be handled with response, no need for error handling here */
      u16_msgState = AICMSGHDL_MSG_UNSUPPORTED_FUNCTION;
      /* set return value */
      b_reqRcvdCompl = TRUE;
      break;
    }
  }

  /* assign message type */
  *pu16_msgState = u16_msgState;

  return b_reqRcvdCompl;

/* This function has a lot of paths because every request type is caught inside
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/***************************************************************************************************
  Function:
    ProcessReqMsgNfrag

  Description:
    Process of the second (and further) fragment of the incoming non-safe message requests.

  See also:
    -

  Parameters:
    pu16_msgState (OUT)      - Message state of the incoming telegram
                               (valid range: <>NULL, not checked, only called with reference to
                               variable)

  Return value:
    TRUE                     - Request received completely (all fragments received)
    FALSE                    - Still fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL ProcessReqMsgNfrag (UINT16* pu16_msgState)
{
  BOOL b_reqRcvdCompl;
  UINT16 u16_msgState;

  switch (aicMsgBld_e_RxState)
  {
    /* 'HalcCsalMessage', see [SIS_200], [SIS_202] */
    case AICMSGPROC_k_WF_AM_REQ_HALCCSALMSG_FRAG_PROC:
    {
      u16_msgState = AICMSGHDL_MSG_VALID;
      
      /* Since the data does not fit into a single message,
       * re-assembling of the message shall be done here
       * After re-assembling the request is put into a queue in order for
       * processing by the background task */
      /* if CSAL message received completely */
      if (TRUE == aicMsgCsal_ReadMsg(&aicMsgProc_s_RcvdMsg))
      {
        /* if CSAL message received but CSS not initialized */
        /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical 
         * operator: '||' --> no side effects because state only changed from IRQ context so 
         * reading even multiple times is safe */
        if ( (AICSM_AIC_STARTUP == aicSm_eAicState) ||
             (AICSM_AIC_WAIT_FOR_INIT == aicSm_eAicState) || /*lint !e960*/
             (AICSM_AIC_START_CSS == aicSm_eAicState) )      /*lint !e960*/
        {
          /* invalid state, we do not expect CSAL messages if CSS is not initialized yet */
          u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
        }
        /* else: CSS already initialized */
        else
        {
          /* valid */
          u16_msgState = AICMSGHDL_MSG_VALID;
        }
      
        /* HalcCsalMessage received => message may be responded now and is
         * not "pending" any more */
        b_reqRcvdCompl = TRUE;
      }
      /* else: still CSAL message to receive */
      else
      {
        /* received message shall not be responded yet, because SW still waits
         * for other fragments and message is therefore "pending" */
        b_reqRcvdCompl = FALSE;
      }
      break;
    }

    /* 'SetInitData', see [SIS_203] */
    case AICMSGPROC_k_WF_AM_REQ_SETINITDATA_FRAG_PROC:
    {
      /* Save received CSS initialization data (see [SRS_2003]).
       * Since the data does not fit into a single message,
       * re-assembling of the message shall be done here */
      /* if received completely */
      if (TRUE == aicMsgInit_SetData(&aicMsgProc_s_RcvdMsg))
      {
        /* accept "SetInitData" only in startup/init (see also [SRS_2034]) */
        if (AICSM_AIC_WAIT_FOR_INIT == aicSm_eAicState)
        {
          u16_msgState = AICMSGHDL_MSG_VALID;

          /* Set Event 'CSS initialization data' received */
          aicSm_SetEvent(AICSM_k_EVT_CSS_INIT_RECEIVED);
        }
        /* else: invalid state for "SetInitData", see [SRS_2035] */
        else
        {
          u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
        }

        /* SetInitData received => message may be responded now and is
         * not "pending" any more */
        b_reqRcvdCompl = TRUE;
      }
      /* else: still fragments to receive */
      else
      {
        /* shall not occur because reception shall be finished after second fragment */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
        /* only used for unit test */
        b_reqRcvdCompl = FALSE;
        u16_msgState    = AICMSGHDL_MSG_INVALID_STATE;
      }
      break;
    }
    case AICMSGPROC_k_WF_AM_REQ_SETCFGSTR_FRAG_PROC:
    {
      /* Save received configuration data.
       * Since the data does not fit into a single message,
       * re-assembling of the message shall be done here */
      /* if received completely */
      if (TRUE == aicMsgCfg_SetData(&aicMsgProc_s_RcvdMsg))
      {
        /* accept "SetConfigString" only if configuration is not already owned by an Originator */
        /* and only in IDLE or CONFIG (see also [SRS_2037]) */
        /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of 
         * logical operator: '||' --> no side effects because state only changed from IRQ 
         * context so reading even multiple times is safe */
        if (    (aicSm_CfgIsOriginatorOwned() == FALSE)
             && ( (AICSM_AIC_IDLE            == aicSm_eAicState)  ||
                  (AICSM_AIC_WAIT_FOR_CONFIG == aicSm_eAicState)  ||  /*lint !e960 */
                  (AICSM_AIC_INVALID_CONFIG  == aicSm_eAicState) )    /*lint !e960 */
           )                                                          /*lint !e960 */
        {
          u16_msgState = AICMSGHDL_MSG_VALID;

          /* Set Event 'Config received' received (see [SRS_2203])*/
          aicSm_SetEvent(AICSM_k_EVT_CONFIG_RECEIVED);
          /* make sure the configuration is marked as tool/host-owned */
          aicSm_CfgOwnerToolSet((BOOL)TRUE);
        }
        /* else invalid state */
        else
        {
          /* invalid state for "SetConfigString", see [SRS_2038] */
          u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
        }
        /* SetConfigString received => message may be responded now and is
         * not "pending" any more */
        b_reqRcvdCompl = TRUE;
      }
      /* else: still fragments to receive */
      else
      {
        /* received message shall not be responded yet, because SW still waits
         * for other fragments and message is therefore "pending" */
        b_reqRcvdCompl = FALSE;
        u16_msgState    = AICMSGHDL_MSG_VALID;
      }
      break;
    }

    case AICMSGPROC_k_WF_AM_REQ_RESP_PROC:
    case AICMSGPROC_k_WF_ASM_RESP_BUILD:
    case AICMSGPROC_k_SKIP_FRAGMENTS:
    {
      /* invalid states, shall not occur */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
      /* set return value and out param (only used for unit test) */
      b_reqRcvdCompl = TRUE;
      u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
      break;
    } /* CCT_SKIP */ /* can not be reached, function not called for this states */

    default:
    {
      /* shall not occur */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(6u));
      /* set return value and out param (only used for unit test) */
      b_reqRcvdCompl = TRUE;
      u16_msgState = AICMSGHDL_MSG_INVALID_STATE;
      break;
    } /* CCT_SKIP */ /* can not be reached, state is already checked */ 
  }

  /* assign message type */
  *pu16_msgState = u16_msgState;

  return b_reqRcvdCompl;
/* This function has a lot of paths because every request type is caught inside
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

/***************************************************************************************************
  Function:
    ProcessRespMsg

  Description:
    Process the incoming non-safe message response received from AM.
    The function checks if:
    - the response was sent with the correct function number (mirrored from request),
    - the response was sent with the correct message ID (mirrored from request)
    - the response length is zero (or AICMSGHDL_ERROR_RESPONSE_LENGTH in case of an Error Response)
    If one of these checks fails, the response is discarded and an error counter is increased.

    If the error bit of the response is set (Error Response), an Error Event is stored inside 
    the Error Event Log.

    Since not data is transferred inside responses from AM, no further interpretation is necessary
    here.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void ProcessRespMsg (void)
{
  /* if ERR Bit is set, see [SIS], [SRS_2168] */
  if ( (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp & AICMSGHDL_ERROR_RESPONSE_ID_MASK1)
               == AICMSGHDL_ERROR_RESPONSE_ID_MASK1 )
  {
    /* if a malformed Error Response is received, means:
    ** - the received function number does not fit to the request number OR
    ** - the received response ID does not fit to the request ID
    ** - the received response length unequal AICMSGHDL_ERROR_RESPONSE_LENGTH
    */
    if ( ((aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp & AICMSGHDL_FUNC_MASK)
             != aicMsgBld_u16_PendingAmRes) ||
         (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgId != AICMSGHDL_DEF_REQ_ID) ||
         (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgLength != AICMSGHDL_ERROR_RESPONSE_LENGTH) )
    {
      /* call function to increase discard error, see [SRS_432], [SIS_027] */
      aicMsgErr_DiscardError();
    }
    /* else: proper Error Response received */
    else
    {
      /* put info into 'Error Event Log' that Error Response was received,
      ** the data itself is discarded, see [SRS_2168] */
      globErrLog_ErrorAdd(GLOBERRLOG_FILE(7u));
    }
  }
  /* else: ERR Bit is not set */
  else
  {
    /* if an unexpected response is received, means:
    ** - the received function number does not fit to the request number OR
    ** - the received response ID does not fit to the request ID
    ** - the received response length unequal zero
    */
    if ( ((aicMsgProc_s_RcvdMsg.sAicMsgHeader.u16MsgReqResp & AICMSGHDL_FUNC_MASK)
             != aicMsgBld_u16_PendingAmRes) ||
         (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgId != AICMSGHDL_DEF_REQ_ID) ||
         (aicMsgProc_s_RcvdMsg.sAicMsgHeader.u8MsgLength != 0u) )
    {
      /* unknown response, call function to increase discard error, see [SRS_432], [SIS_027] */
      aicMsgErr_DiscardError();
    }
    /* else: proper response is received */
    else
    {
      /* Since no data is received in response messages from AM, no action is necessary here.
      ** We simply want to have the response received.
      */
    }
  }
  
  /* reset pending identifier, so that the request-response cycle is finished */
  aicMsgBld_u16_PendingAmRes = 0u;
}

/***************************************************************************************************
  Function:
    CalcNumOfFragmentsToSkip

  Description:
    This function returns the number of fragments which shall be skipped in case of an AIC
    error. 
    Fragmented messages are split in the first "standard" message and several "data only" messages 
    (currently a "standard" non-safe message has 16 Bytes (4 Bytes header and 12 Bytes data), 
    a "data only" message contains 16 Bytes of data and no header at all.

  See also:
    -

  Parameters:
    u8_length (IN) - length of message in bytes 

  Return value:
    Number of fragments which shall be skipped 

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC UINT8 CalcNumOfFragmentsToSkip(UINT8 u8_length)
{
  UINT8 u8_noFragToSkip = 0u;

  /* if fragments shall be skipped */
  if (u8_length > AICMSGDEF_ASM_MSG_DATA_SIZE)
  {
    u8_noFragToSkip = ((u8_length - (AICMSGDEF_ASM_MSG_DATA_SIZE + 1u)) /
                  ((UINT8)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT))) + (UINT8)1u;
  }
  else /* fragments shall not be skipped */
  {
    u8_noFragToSkip = 0u;
  }
  return u8_noFragToSkip;
}

/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgBld.h
**     Summary: Interface header of the module aicMsgBld.c
**   $Revision: 3806 $
**       $Date: 2023-01-10 14:46:54 +0100 (Di, 10 Jan 2023) $
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
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSGHDLBUILD_H
#define AICMSGHDLBUILD_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/* AICMSGPROC_t_STATE:
** States defined for reception.
*/
typedef enum
{
  /* Ready for request/response processing */
  AICMSGPROC_k_WF_AM_REQ_RESP_PROC             = 0xdc76u,
  /* HalcCsalMessage frag reception */
  AICMSGPROC_k_WF_AM_REQ_HALCCSALMSG_FRAG_PROC = 0xdde1u,
  /* SetInitData frag reception */
  AICMSGPROC_k_WF_AM_REQ_SETINITDATA_FRAG_PROC = 0xdecfu,
  /* SetConfigString frag reception */
  AICMSGPROC_k_WF_AM_REQ_SETCFGSTR_FRAG_PROC   = 0xdf58u,
  /* Wait for response generation */
  AICMSGPROC_k_WF_ASM_RESP_BUILD               = 0xe097u,
  /* Skip further fragments */
  AICMSGPROC_k_SKIP_FRAGMENTS                  = 0xe100u
} AICMSGPROC_t_STATE;

/* AICMSGBLD_t_STATE:
** States defined for transmission.
*/
typedef enum
{
  /* Ready for building request/response */
  AICMSGBLD_k_WF_ASM_REQ_RESP_BUILD  = 0xe100u,
  /* Request frag transfer */
  AICMSGBLD_k_WF_ASM_REQ_BUILD       = 0xe22eu,
  /* Wait for response reception and ready for building response */
  AICMSGBLD_k_WF_AM_RES_PROC         = 0xe3b9u
} AICMSGBLD_t_STATE;

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgBld_s_SendMsg:
** non-safe message from ASM to AM, global used due to performance reasons
** formerly aicDataStack_sAsmMsg
*/
extern AICMSGDEF_NON_SAFE_MSG_STRUCT aicMsgBld_s_SendMsg;

/* aicMsgBld_e_ConfAmFrag:
** Bool to identify if the AM fragmentation may be confirmed or not
*/
extern TRUE_FALSE_ENUM aicMsgBld_e_ConfAmFrag;

/* aicMsgBld_u16_PendingAmRes:
** global to store which AM-response is currently pending
*/
extern UINT16 aicMsgBld_u16_PendingAmRes;

/* aicMsgBld_e_RxState:
** State machine variable used for reception handling.
*/
extern AICMSGPROC_t_STATE aicMsgBld_e_RxState;

/* aicMsgBld_e_TxState:
** State machine variable used for transfer handling.
*/
extern AICMSGBLD_t_STATE aicMsgBld_e_TxState;

/***************************************************************************************************
**    function prototypes
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
void aicMsgBld_BuildMsg (void);

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
void aicMsgBld_PrepRespHdr(UINT8 u8_msgId, UINT16 u16_ReqResp);

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
void aicMsgBld_SetMsgState(UINT16 u16_msgState);

#endif


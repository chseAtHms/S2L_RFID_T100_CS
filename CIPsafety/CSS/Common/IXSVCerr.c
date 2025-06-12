/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCerr.c
**    Summary: IXSVC - Safety Validator Client
**             This file implements the (optional) error strings of the IXSVC
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_ErrorInit
**             IXSVC_ErrorClbk
**             IXSVC_ErrorLastGet
**             IXSVC_ErrorStrGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXSVD.h"
#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  #include "IXSVCerr.h"
#endif


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** aw_LastErrorCode:
    This array stores for each instance the last Error Code.
*/
/* Not relevant for Soft Error Checking - only a diagnostic value */
static CSS_t_WORD aw_LastErrorCode[CSOS_cfg_NUM_OF_SV_CLIENTS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_ErrorInit
**
** Description : This function initializes the global variables of the Error
**               module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ErrorInit(void)
{
  CSS_t_UINT u16_idx;

  /* clear the stored error codes of all instances */
  for (u16_idx = 0U; u16_idx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_idx++)
  {
    aw_LastErrorCode[u16_idx] = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_ErrorClbk
**
** Description : This callback function is used within the IXSVC unit instead of
**               directly calling SAPL_CssErrorClbk(). Instance related errors
**               are stored for being read in SVO attribute 14. This function
**               then calls SAPL_CssErrorClbk().
**
** Parameters  : w_errorCode (IN) - 16 bit value that holds Error Type, Error
**                                  Layer, Unit ID and Unit Error.
**                                  (not checked, any value allowed)
**               u16_instId (IN)  - instance Id of the safety validator
**                                  instance that caused the error
**                                  (checked, IXSVD_SvIdxFromInstGet()
**                                  must return a valid index)
**               dw_addInfo (IN)  - additional error information.
**                                  (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ErrorClbk(CSS_t_WORD w_errorCode,
                     CSS_t_UINT u16_instId,
                     CSS_t_DWORD dw_addInfo)
{
  CSS_t_UINT u16_svcIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if the passed instance has resulted in an invalid index */
  if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    /* This error is not instance related (or so severe that even the passed
       instance ID is invalid) */
  }
  else /* else: SVC index is valid */
  {
    /* store the error code as the new Last Error of the addressed instance */
    aw_LastErrorCode[u16_svcIdx] = w_errorCode;
  }

  /* report the error to SAPL */
  SAPL_CssErrorClbk(w_errorCode, u16_instId, dw_addInfo);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_ErrorLastGet
**
** Description : This function returns the last error code of the addressed
**               Safety Validator Client Instance.
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_WORD      - Last Error Code
**
*******************************************************************************/
CSS_t_WORD IXSVC_ErrorLastGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (aw_LastErrorCode[u16_svcIdx]);
}


#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVC_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVC_k_FSE_xxx})
**               u16_instId (IN)        - instance Id (e.g. of the safety
**                                        validator instance that caused the
**                                        error), usage is error code specific
**                                        (not checked)
**               dw_addInfo (IN)        - additional error information.
**                                        (not checked, any value allowed)
**               u16_errStrBufSize (IN) - size of the error string buffer
**                                        (checked in IXSER_ErrorStrGet(),
**                                        valid range:
**                                        >= IXSER_k_MAX_ERROR_STRING_LEN)
**               pc_errStr (OUT)        - pointer to the error string buffer
**                                        (buffer to where this function will
**                                        write the error string)
**                                        (not checked, checked in
**                                        IXSER_ErrorStrGet())
**                                        valid range: <> CSS_k_NULL
**
** Returnvalue : 1..u16_errStrBufSize   - Success, length of the error string in
**                                        the error string buffer without '\0'
**               else                   - Error string buffer size is too small.
**                                        This is the return value of
**                                        CSS_SNPRINTF(): According to the ANSI
**                                        C99 definition the return value of
**                                        snprintf() is the number of characters
**                                        that would have been written if n had
**                                        been sufficiently large (not counting
**                                        the terminating null character).
**                                        Unfortunately in many implementations
**                                        the return value is < 0 if the
**                                        provided buffer is too small.
**
*******************************************************************************/
CSS_t_INT IXSVC_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSVC_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSVC_k_FSE_AIS_STATE_ERR_TFG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_AIS_STATE_ERR_TFG - IXSVC_TxFrameGenerate() called in "
        "invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_SM_INV_STATE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SM_INV_STATE - Safety Validator Client state machine "
        "detected an invalid state (inst=%" CSS_PRIu16 " state=%" CSS_PRIu32
        ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_DIV_BY_ZERO:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_DIV_BY_ZERO - Division by zero error"));
      break;
    }

    case IXSVC_k_NFSE_RXI_MSG_LEN_ERR_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_MSG_LEN_ERR_C - data received for Validator Client "
        "Inst (%" CSS_PRIu16 ") had invalid length", u16_instId));
      break;
    }

    case IXSVC_k_NFSE_TXH_HALCS_SEND_OCI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_TXH_HALCS_SEND_OCI - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error)(%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_INST_INV_OCI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_INST_INV_OCI - Originator tries to initialize a "
        "Safety Validator Instance that was already closed (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_CLOSE_CONS_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_CLOSE_CONS_NUM - Detected an invalid consumer number "
        "when closing a connection (inst=%" CSS_PRIu16 " consNum=%" CSS_PRIu32
        ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_TXH_HALCS_SEND_CCC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_TXH_HALCS_SEND_CCC - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error) inst=%" CSS_PRIu16
        " error=%" CSS_PRIu32,
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_CLOSE_IDX:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_CLOSE_IDX - Detected an invalid Safety Validator "
        "Client Index when closing a connection (idx=%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_PR_NOTSET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_PR_NOTSET - Time Coordination Message was "
        "received with Ping_Response bit not set (inst=%" CSS_PRIu16
        " consNum=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_INST_CIIG_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_INST_CIIG_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVC_k_NFSE_TXH_HALCS_SEND_TFG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_TXH_HALCS_SEND_TFG - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error) (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_SIIG_IDX_ALLOC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SIIG_IDX_ALLOC - An inconsistency between allocation "
        "status and obtained index was detected when gathering instance info "
        "(%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVC_k_FSE_AIP_PTR_INV_IIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_AIP_PTR_INV_IIG - IXSVC_InstInfoGet() called with an "
        "invalid pointer"));
      break;
    }

    case IXSVC_k_FSE_AIS_STATE_ERR_IIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_AIS_STATE_ERR_IIG - API function called in invalid CSS "
        "state (IXSVC_InstInfoGet(%" CSS_PRIu16 "))", u16_instId));
      break;
    }

    case IXSVC_k_NFSE_RXI_CONS_NUM_CRP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_CONS_NUM_CRP - The passed 'Consumer Number' "
        "parameter is invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_INST_INV_TCI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_INST_INV_TCI - Target tries to initialize a "
        "Safety Validator Instance that was already closed (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_NFSE_RXE_MCASTRC_FORMAT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXE_MCASTRC_FORMAT - Failed to join another Consumer: "
        "Format mismatch (CnxnPoint=%" CSS_PRIu16 " Format=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXE_MCASTRC_RPI_OT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXE_MCASTRC_RPI_OT - Failed to join another Consumer: "
        "O->T RPI mismatch (CnxnPoint=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVC_k_NFSE_RXE_MCASTRC_MCN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXE_MCASTRC_MCN - Failed to join another Consumer: "
        "Max Consumer Number mismatch (CnxnPoint=%" CSS_PRIu16 " MaxConsNum=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXE_MCASTRC_PIEM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXE_MCASTRC_PIEM - Failed to join another Consumer: "
        "Ping_Interval_EPI_Multiplier mismatch (CnxnPoint=%" CSS_PRIu16
        " MaxConsNum=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_SM_INV_EV_IDLE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SM_INV_EV_IDLE - Detected an invalid Event in the "
        "Safety Validator Client state machine in IDLE state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_SM_INV_EV_INIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SM_INV_EV_INIT - Detected an invalid Event in the "
        "Safety Validator Client state machine in INITIALIZING state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_SM_INV_EV_ESTAB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SM_INV_EV_ESTAB - Detected an invalid Event in the "
        "Safety Validator Client state machine in ESTABLISHED state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_SM_INV_EV_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_SM_INV_EV_ESTAB - Detected an invalid Event in the "
        "Safety Validator Client state machine in FAILED state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_NTEM_GET_INV_CI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_NTEM_GET_INV_CI - Detected an invalid Consumer Index "
        "when trying to obtain the Network Time Expectation Multiplier "
        "(inst=%" CSS_PRIu16 " consIdx=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_TMULT_PI_CALC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_TMULT_PI_CALC - Detected an invalid value of the "
        "Timeout Multiplier when initializing a SV client "
        "(inst=%" CSS_PRIu16 " TMult=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_TXFG_INST_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_TXFG_INST_INV - Detected an invalid Instance ID "
        "when trying to generate a TX message (inst=%" CSS_PRIu16, u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_C_ALLOC_MAXCNUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_C_ALLOC_MAXCNUM - Detected an invalid value of "
        "Max_Consumer_Number when trying to allocate a new Consumer Number "
        "(inst=%" CSS_PRIu16 " MaxConsNum=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_TX_GEN_TOO_MUCH:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_TX_GEN_TOO_MUCH - While generating Tx message: "
        "detected that data message is already too long (Time Correction will "
        "not fit into buffer (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_FSE_INC_CON_NUM_IDX_PTR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_CON_NUM_IDX_PTR - Detected an invalid value of "
        "u8_RR_Con_Num_Index_Pntr (inst=%" CSS_PRIu16
        " u8_RR_Con_Num_Index_Pntr=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_TO_SC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_TO_SC - Time Coordination Message Check: "
        "not received in allotted time (Single-cast) (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_TO_MC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_TO_MC - Time Coordination Message Check: "
        "not received in allotted time (Multi-cast) (inst=%" CSS_PRIu16
        " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_TS_DUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_TS_DUP - Time Coordination Message Check: "
        "message with same time stamp already received from this consumer "
        "(inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_PARITY:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_PARITY - Time Coordination Message Check: "
        "parity check error (inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_BASE_TCOO_ACK2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_BASE_TCOO_ACK2 - Time Coordination Message Check: "
        "Ack_Byte_2 check error (inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_5S_LIMIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_5S_LIMIT - Time Coordination Message Check: "
        "not received within the approximately 5 second limit "
        "(inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_TCOO_PC_ERR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_TCOO_PC_ERR - Time Coordination Message Check: "
        "not received within the same ping interval or the next ping "
        "interval (inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_BASE_TCOO_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_BASE_TCOO_CRC - Time Coordination Message Check: "
        "CRC mismatch (inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_NFSE_RXI_EXT_TCOO_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXI_EXT_TCOO_CRC - Time Coordination Message Check: "
        "CRC mismatch (inst=%" CSS_PRIu16 " cons=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_EPI_RANGE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_EPI_RANGE - Invalid EPI detected when initializing "
        "internal Safety Validator Client data (inst=%" CSS_PRIu16 " EPI=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_DATA_SIZE_RANGE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_FSE_INC_DATA_SIZE_RANGE - Invalid produced data size detected "
        "when initializing internal Safety Validator Client data "
        "(inst=%" CSS_PRIu16 " size=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVC_k_FSE_INC_MAX_CONS_NUM_CI:
    {
       i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
         "IXSVC_k_FSE_INC_MAX_CONS_NUM_CI - Invalid MaxConsNum detected when "
         "trying to write RR_Con_Num_Index_Pntr (inst=%" CSS_PRIu16
         " maxConsNum=%" CSS_PRIu32 ")",
         u16_instId, dw_addInfo));
       break;
    }

    case IXSVC_k_FSE_INC_TDPPI_RANGE:
    {
       i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
         "IXSVC_k_FSE_INC_TDPPI_RANGE - Invalid Time_Drift_Per_Ping_Interval "
         "detected when processing received TCOO message (inst=%" CSS_PRIu16
         " TDPPI=%" CSS_PRIu32 ")",
         u16_instId, dw_addInfo));
       break;
    }

    case IXSVC_k_FSE_T_DRIFT_CONST_RANGE:
    {
       i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
         "IXSVC_k_FSE_T_DRIFT_CONST_RANGE - Invalid Time_Drift_Constant "
         "detected in Cold Start Logic (inst=%" CSS_PRIu16 " CCC=%"
         CSS_PRIu32 ")",
         u16_instId, dw_addInfo));
       break;
    }

    case IXSVC_k_FSE_TCREL_OUT_OF_RANGE:
    {
       i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
         "IXSVC_k_FSE_TCREL_OUT_OF_RANGE - Invalid "
         "Time_Coord_Response_EPI_Limit detected in Cold Start Logic "
         "(inst=%" CSS_PRIu16 " CCC=%" CSS_PRIu32 ")", u16_instId, dw_addInfo));
       break;
    }

    case IXSVC_k_FSE_TCOOMMM_RANGE:
    {
       i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
         "IXSVC_k_FSE_TCOOMMM_RANGE - Invalid "
         "Time_Coord_Msg_Min_Multiplier detected during SVC instance "
         "initialization (inst=%" CSS_PRIu16 " TCOOMMM=%" CSS_PRIu32 ")",
         u16_instId, dw_addInfo));
       break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_01:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_01 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_02:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_02 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_03:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_03 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_04:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_04 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_05:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_05 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_06:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_06 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_07:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_07 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_08:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_08 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_09:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_09 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_10:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_10 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_11:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_11 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_12:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_12 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_13:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_13 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_14:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_14 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_15:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_15 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_16:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_16 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_17:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_17 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_18:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_18 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_19:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_19 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_20:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_20 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_21:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_21 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_22:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_22 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_23:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_23 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_24:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_24 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_25:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_25 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_26:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_26 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_27:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_27 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_28:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_28 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_29:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_29 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_30:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_30 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_31:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_31 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_32:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_32 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_33:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_33 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_34:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_34 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_35:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_35 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_36:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_36 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_37:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_37 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_FSE_INC_MSG_FORMAT_38:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_FSE_INC_MSG_FORMAT_38 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVC_k_NFSE_RXE_MCASTRC_RPI_TO:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVC_k_NFSE_RXE_MCASTRC_RPI_TO - Failed to join another Consumer: "
        "T->O RPI mismatch (CnxnPoint=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSVC unit (%" CSS_PRIu16
        ") (%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);

/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
  }
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


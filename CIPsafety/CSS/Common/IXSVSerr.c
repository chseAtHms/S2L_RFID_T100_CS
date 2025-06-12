/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSerr.c
**    Summary: IXSVS - Safety Validator Server
**             This file implements the (optional) error strings of the IXSVS
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_ErrorInit
**             IXSVS_ErrorClbk
**             IXSVS_ErrorLastGet
**             IXSVS_ErrorStrGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)

#include "CSScfg.h"

#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  #include "CSOSapi.h"
#endif

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXSVD.h"
#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVS.h"
#include "IXSVSint.h"
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  #include "IXSVSerr.h"
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
static CSS_t_WORD aw_LastErrorCode[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_ErrorInit
**
** Description : This function initializes the global variables of the Error
**               module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ErrorInit(void)
{
  CSS_t_UINT u16_idx;

  for (u16_idx = 0U; u16_idx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_idx++)
  {
    aw_LastErrorCode[u16_idx] = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_ErrorClbk
**
** Description : This callback function is used within the IXSVS unit instead of
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
void IXSVS_ErrorClbk(CSS_t_WORD w_errorCode,
                     CSS_t_UINT u16_instId,
                     CSS_t_DWORD dw_addInfo)
{
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if the passed instance has resulted in an invalid index */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* This error is not instance related (or so severe that even the passed
       instance ID is invalid) */
  }
  else
  {
    /* store the error code as the new Last Error of the addressed instance */
    aw_LastErrorCode[u16_svsIdx] = w_errorCode;
  }

  /* report the error to SAPL */
  SAPL_CssErrorClbk(w_errorCode, u16_instId, dw_addInfo);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_ErrorLastGet
**
** Description : This function returns the last error code of the addressed
**               Safety Validator Server Instance.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_WORD      - Last Error Code
**
*******************************************************************************/
CSS_t_WORD IXSVS_ErrorLastGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (aw_LastErrorCode[u16_svsIdx]);
}


#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVS_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVO_k_UERR_xxx})
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
CSS_t_INT IXSVS_ErrorStrGet(CSS_t_WORD w_errorCode,
                             CSS_t_UINT u16_instId,
                             CSS_t_DWORD dw_addInfo,
                             CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                             CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSVS_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSVS_k_NFSE_TXH_HALCS_SEND_OSI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_TXH_HALCS_SEND_OSI - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error)(%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_AIS_STATE_ERR_CAM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_AIS_STATE_ERR_CAM - IXSVS_ConsActivityMonitor() called in "
        "invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_SM_INV_STATE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_INV_STATE - Safety Validator Server state machine "
        "detected an invalid state (inst=%" CSS_PRIu16 " state=%" CSS_PRIu32
        ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_TMULT_INV_RANGE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_TMULT_INV_RANGE - Safety Validator Server tried to "
        "initialize the Timeout Multiplier with an invalid value (inst=%"
        CSS_PRIu16 " tmult=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_INV_FLAG_COMB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INV_FLAG_COMB - Invalid combination of Validator "
        "flags (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_MSG_LEN_ERR_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_MSG_LEN_ERR_S - Data received for Validator Server "
        "Inst (%" CSS_PRIu16 ") had invalid length", u16_instId));
      break;
    }

    case IXSVS_k_NFSE_TXH_HALCS_SEND_CCC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_TXH_HALCS_SEND_CCC - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error) inst=%" CSS_PRIu16
        " error=%" CSS_PRIu32,
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_CLOSE_CONS_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_CLOSE_CONS_NUM - Detected an invalid consumer number "
        "when closing a connection (inst=%" CSS_PRIu16 " consNum=%" CSS_PRIu32
        ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_TXH_HALCS_SEND_STS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_TXH_HALCS_SEND_STS - Sending a message to CSAL was not "
        "possible (HALCS_TxDataPut() returned error) (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_INST_SRX_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INST_SRX_NUM - The passed 'instance' parameter "
        "is invalid (internal index = %" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_INST_SIIG_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INST_SIIG_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_INST_INV_OSI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INST_INV_OSI - Originator tries to initialize a "
        "Safety Validator Instance that was already closed (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_INST_INV_TSI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INST_INV_TSI - Target tries to initialize a "
        "Safety Validator Instance that was already closed (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_CLOSE_IDX:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_CLOSE_IDX - Detected an invalid Safety Validator "
        "Server Index when closing a connection (idx=%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_IDX_IS_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_IDX_IS_NUM - Detected an invalid Safety Validator "
        "Server Index when stopping a connection (idx=%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_INST_SCOM_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_INST_SCOM_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_SM_INV_EV_IDLE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_INV_EV_IDLE - Detected an invalid Event in the "
        "Safety Validator Server state machine in IDLE state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")", u16_instId,
        dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_SM_INV_EV_INIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_INV_EV_INIT - Detected an invalid Event in the "
        "Safety Validator Server state machine in INITIALIZING state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")", u16_instId,
        dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_SM_INV_EV_ESTAB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_INV_EV_ESTAB - Detected an invalid Event in the "
        "Safety Validator Server state machine in ESTABLISHED state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_SM_INV_EV_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_INV_EV_ESTAB - Detected an invalid Event in the "
        "Safety Validator Server state machine in FAILED state "
        "(inst=%" CSS_PRIu16 " event=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_SIIG_IDX_ALLOC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SIIG_IDX_ALLOC - An inconsistency between allocation "
        "status and obtained index was detected when gathering instance info "
        "(%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_TSTAMP_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TSTAMP_CRC - Safety Message Check: Timestamp CRC "
        "mismatch (inst=%" CSS_PRIu16 " expected=0x%02" CSS_PRIX32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_TSTAMP_DELTA:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TSTAMP_DELTA - Safety Message Check: Timestamp "
        "Delta zero (message has same time stamp as previous one) "
        "(inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_TSTAMP_NTEM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TSTAMP_NTEM - Safety Message Check: Timestamp "
        "Delta greater than Network Time Expectation (inst=%" CSS_PRIu16
        " delta=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG - Safety Message Check: Data age of a "
        "faulty message greater than Network Time Expectation "
        "(inst=%" CSS_PRIu16 " data age=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG - Safety Message Check: Data age of "
        "an in other respects valid message greater than Network Time "
        "Expectation (inst=%" CSS_PRIu16 " data age=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_SHRT_ACRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_SHRT_ACRC - Safety Message Check: Actual Data "
        "CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_SHRT_CCRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_SHRT_CCRC - Safety Message Check: Complemented "
        "Data CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_SHRT_RBMB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_SHRT_RBMB - Safety Message Check: Redundant "
        "bits in Mode Byte mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_LONG_ACRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_LONG_ACRC - Safety Message Check: Actual Data "
        "CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_LONG_CCRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_LONG_CCRC - Safety Message Check: Complemented "
        "Data CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_LONG_RBMB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_LONG_RBMB - Safety Message Check: Redundant "
        "bits in Mode Byte mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_LONG_AVSC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_LONG_AVSC - Safety Message Check: Actual versus "
        "complemented data check mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_SHORT_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_SHORT_CRC - Safety Message Check: CRC mismatch "
        "(inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_SHORT_RBMB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_SHORT_RBMB - Safety Message Check: Redundant "
        "bits in Mode Byte mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_LONG_ACRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_LONG_ACRC - Safety Message Check: Actual Data "
        "CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_LONG_CCRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_LONG_CCRC - Safety Message Check: Complemented "
        "Data CRC mismatch (inst=%" CSS_PRIu16 " expected=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_LONG_RBMB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_LONG_RBMB - Safety Message Check: Redundant "
        "bits in Mode Byte mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_LONG_AVSC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_LONG_AVSC - Safety Message Check: Actual versus "
        "complemented data check mismatch (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_ACT_MON:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_ACT_MON - Safety Message Check: Consumer Activity "
        "Monitor timeout (inst=%" CSS_PRIu16 " expected message at=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_TCORR_NOT_PI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TCORR_NOT_PI - Safety Message Check: Time "
        "Correction message not received within expected time (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_CROSS_CHECK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_CROSS_CHECK - Safety Message Check: Data cross check "
        "error (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_TCORR_PARITY:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TCORR_PARITY - Safety Message Check: Time Correction "
        "message parity check error (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_TCORR_MC2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_TCORR_MC2 - Safety Message Check: Time "
        "Correction message Mcast_Byte_2 check error (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_TCORR_AI_FLAG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_TCORR_AI_FLAG - Safety Message Check: Time "
        "Correction message Multi_Cast_Active_Idle transitioned to idle "
        "(inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_NFSE_RXI_BASE_TCORR_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_BASE_TCORR_CRC - Safety Message Check: Time "
        "Correction message CRC mismatch (inst=%" CSS_PRIu16 " expected=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_EXT_TCORR_CRC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_EXT_TCORR_CRC - Safety Message Check: Time "
        "Correction message CRC mismatch (inst=%" CSS_PRIu16 " expected=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_NFSE_RXI_INIT_LIMIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_NFSE_RXI_INIT_LIMIT - Safety Message Check: Initialization "
        "not completed within time limit (inst=%" CSS_PRIu16
        " expected completion at=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVS_k_FSE_INC_SD_RECV_LT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SD_RECV_LT - Invalid instance in Safety Validator "
        "Server Reception LT (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_AIP_PTR_INV_IIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_AIP_PTR_INV_IIG - IXSVS_InstInfoGet() called with an "
        "invalid pointer"));
      break;
    }

    case IXSVS_k_FSE_AIS_STATE_ERR_IIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_AIS_STATE_ERR_IIG - API function called in invalid CSS "
        "state (IXSVS_InstInfoGet(%" CSS_PRIu16 "))", u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_SM_EST_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_EST_FAIL - Invalid instance in Safety Validator "
        "Server State Machine - Established->Fault (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_k_FSE_INC_SM_EST_CLOSE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_k_FSE_INC_SM_EST_CLOSE - Invalid instance in Safety Validator "
        "Server State Machine - Established->Close (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_05:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_05 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_06:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_06 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_07:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_07 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_08:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_08 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_09:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_09 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_10:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_10 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_11:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_11 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_12:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_12 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_13:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_13 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_14:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_14 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_15:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_15 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_16:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_16 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_20:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_20 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_21:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_21 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_24:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_24 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_25:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_25 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_26:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_26 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_27:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_27 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_28:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_28 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_29:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_29 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_30:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_30 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_31:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_31 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_32:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_32 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_33:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_33 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_34:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_34 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_35:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_35 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_36:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_36 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_37:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_37 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_38:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_38 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVS_FSE_INC_MSG_FORMAT_39:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVS_FSE_INC_MSG_FORMAT_39 - Format Byte inconsistent (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSVS unit (%" CSS_PRIu16 ") (%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);

/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
/* As this unit has many errors this function gets quite long. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #18   - Function eLOC > maximum 200 eLOC */
  }
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


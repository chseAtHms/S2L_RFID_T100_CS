/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOerr.c
**    Summary: IXSSO - Safety Supervisor Object
**             This file implements the (optional) error strings of the IXSSO
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSScfg.h"

#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)

#include "CSOScfg.h"
#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"

#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSSOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSO_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSSO_k_FSE_xxx and
**                                        IXSSO_k_NFSE_xxx})
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
CSS_t_INT IXSSO_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSSO_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSSO_k_FSE_INC_ASM_NFOUND_GET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_ASM_NFOUND_GET - assembly with requested instance "
        "(%" CSS_PRIu16 ") could not be found", u16_instId));
      break;
    }

    case IXSSO_k_FSE_INC_ASM_NFOUND_SET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_ASM_NFOUND_SET - assembly with requested instance "
        "(%" CSS_PRIu16 ") could not be found", u16_instId));
      break;
    }

    case IXSSO_k_FSE_INC_ASM_INVTYPE_GET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_ASM_INVTYPE_GET - assembly with requested instance "
        "(%" CSS_PRIu16 ") has wrong type for requested operation",
        u16_instId));
      break;
    }

    case IXSSO_k_FSE_INC_ASM_INVTYPE_SET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_ASM_INVTYPE_SET - assembly with requested instance "
        "(%" CSS_PRIu16 ") has wrong type for requested operation",
        u16_instId));
      break;
    }

    case IXSSO_k_FSE_INC_NV_W_SCID_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_W_SCID_2 - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_W_TUNID_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_W_TUNID_2 - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_W_CFUNID_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_W_CFUNID_2 - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_W_OCPUNID_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_W_OCPUNID_2 - error during Non-volatile store "
        "operation, Assembly Instance %" CSS_PRIu16, u16_instId));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_AE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_AE - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_WE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_WE - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_CFUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_CFUNID - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_SCID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_SCID - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_TUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_TUNID - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_NV_R_OCPUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_NV_R_OCPUNID - error during Non-volatile store "
        "operation"));
      break;
    }

    case IXSSO_k_FSE_INC_CNXN_EVENT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_CNXN_EVENT - invalid event (%" CSS_PRIu32
        ") found in SSO state "
        "machine", dw_addInfo));
      break;
    }

    case IXSSO_k_NFSE_TXH_TUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_TXH_TUNID - error in communication via HALC"));
      break;
    }

    case IXSSO_k_NFSE_TXH_DEVST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_TXH_DEVST - error in communication via HALC"));
      break;
    }

    case IXSSO_k_FSE_AIS_OSC_ST_AB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_OSC_ST_AB - Safety Supervisor Object state conflict "
        "(Self Test - Abort)"));
      break;
    }

    case IXSSO_k_FSE_AIS_OSC_STEX_AB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_OSC_STEX_AB - Safety Supervisor Object state "
        "conflict (Self Test Exception - Abort)"));
      break;
    }

    case IXSSO_k_FSE_AIS_OSC_CRIT_AB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_OSC_CRIT_AB - Safety Supervisor Object state "
        "conflict (Critical Fault - Abort)"));
      break;
    }

    case IXSSO_k_NFSE_AIS_INFO_EXCON_CLR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_AIS_INFO_EXCON_CLR - This Info Code is not an error. "
        "Just for stack internal caller notification. Should never be reported "
        "to SAPL!"));
      break;
    }

    case IXSSO_k_FSE_AIS_STATE_ERR_MC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_STATE_ERR_MC - IXSSO_ModeChange() called in invalid "
        "CSS state"));
      break;
    }

    case IXSSO_k_FSE_INC_UNDEF_STATE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_UNDEF_STATE - Safety Supervisor state machine "
        "detected an invalid Device Status (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSO_k_NFSE_RXE_FO_WAIT_TUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_FO_WAIT_TUNID - Forward_Open request received but "
        "not allowed in Wait for TUNID state"));
      break;
    }

    case IXSSO_k_FSE_RXE_FO_SELF_TEST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_FO_SELF_TEST - Forward_Open request received but "
        "not allowed in Self Testing state"));
      break;
    }

    case IXSSO_k_NFSE_RXE_FO_SELF_TESTEX:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_FO_SELF_TESTEX - Forward_Open request received but "
        "not allowed in Self Test Exception state"));
      break;
    }

    case IXSSO_k_NFSE_RXE_FO_ABORT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_FO_ABORT - Forward_Open request received but "
        "not allowed in Abort state"));
      break;
    }

    case IXSSO_k_NFSE_RXE_FO_CRIT_FAULT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_FO_CRIT_FAULT - Forward_Open request received but "
        "not allowed in Critical Fault state"));
      break;
    }

    case IXSSO_k_NFSE_RXE_PROP_TUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_PROP_TUNID - Propose_TUNID service requested in an "
        "invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_NFSE_RXE_APY_TUNID_CHK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_NFSE_RXE_APY_TUNID_CHK - Apply_TUNID service requested in an "
        "invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_AIS_VAL_CFG_CHECK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_VAL_CFG_CHECK - Validate_Configuration requested in "
        "an invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_AIS_APPLY_CFG_CHECK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_APPLY_CFG_CHECK - Apply_Configuration requested in an "
        "invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_AIS_MODE_CHANGE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_MODE_CHANGE - Mode_Change requested in an invalid "
        "state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_AIS_AIRS_IAR_AB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_AIRS_IAR_AB - Safety Supervisor Object state conflict "
        "(Already in Abort state)"));
      break;
    }

    case IXSSO_k_FSE_RXE_OPEN_NOT_PSBL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_OPEN_NOT_PSBL - Safety Supervisor Object state "
        "machine detected an illegal operation (connection established in "
        "state %" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_RXE_DEL_NOT_PSBL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_DEL_NOT_PSBL - Safety Supervisor Object state "
        "machine detected an illegal operation (connection delete in state "
        "%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_INC_TRANS_INV_STATE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_TRANS_INV_STATE - Safety Supervisor State machine "
        "transitions to an undefined state (%" CSS_PRIu32 ") ", dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_INC_SELF_TEST_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_SELF_TEST_FAIL - Self-Test Failed requested in an "
        "invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_INC_SELF_TEST_PASS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_SELF_TEST_PASS - Self-Test Passed requested in an "
        "invalid state: %" CSS_PRIu32, dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_AIS_STATE_ERR_ESS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_STATE_ERR_ESS - IXSSO_ExceptionStatusSet() called in "
        "invalid CSS state"));
      break;
    }

  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
    case IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL - IXCCO_CnxnOpen() called in invalid "
        "SSO state: %" CSS_PRIu32, dw_addInfo));
      break;
    }
  #endif

    case IXSSO_k_FSE_AIS_STATE_ERR_OTALS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIS_STATE_ERR_OTALS - IXSSO_OcpunidTargAsmOutListSet() "
        "called in invalid CSS state"));
      break;
    }

    case IXSSO_k_FSE_AIP_PTR_INV_OTALS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_AIP_PTR_INV_OTALS - IXSSO_OcpunidTargAsmOutListSet() "
        "called with invalid pointer"));
      break;
    }

    case IXSSO_k_FSE_INC_OCPIDX_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_OCPIDX_INV - An invalid Assembly Index was detected "
        "when requesting Assembly Instance IDs for generating OCPUNIDs "
        "(idx=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSO_k_FSE_RXE_PROP_T_SELFTEST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_PROP_T_SELFTEST - Propose_TUNID service requested but "
        "not allowed in Self Testing state"));
      break;
    }

    case IXSSO_k_FSE_RXE_APLY_T_SELFTEST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_APLY_T_SELFTEST - Apply_TUNID service requested but "
        "not allowed in Self Testing state"));
      break;
    }

    case IXSSO_k_FSE_RXE_SRESET_SELFTEST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_SRESET_SELFTEST - Safety_Reset service requested but "
        "not allowed in Self Testing state"));
      break;
    }

    case IXSSO_k_FSE_RXE_FAIL_NOT_PSBL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_RXE_FAIL_NOT_PSBL - Safety Connection Failed event may "
        "not occur in Self Test Exception state"));
      break;
    }

    case IXSSO_k_FSE_INC_SC_DEL_IN_WFT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_SC_DEL_IN_WFT - Safety Connection Delete event may "
        "not occur in Waiting For TUNID state"));
      break;
    }

    case IXSSO_k_FSE_INC_SC_FAIL_IN_WFT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSO_k_FSE_INC_SC_FAIL_IN_WFT - Safety Connection Failed event may "
        "not occur in Waiting For TUNID state"));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSSO unit (%" CSS_PRIu16 ") (%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);

/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
/* It's obvious what this function does. Not much comments necessary */
/* RSM_IGNORE_QUALITY_BEGIN Notice #17   - Function comment content less ... */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */


/*** End Of File ***/



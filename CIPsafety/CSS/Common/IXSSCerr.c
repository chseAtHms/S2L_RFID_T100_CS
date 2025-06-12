/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSCerr.c
**    Summary: IXSSC - Safety Stack Control
**             This file implements the (optional) error strings of the IXSSC
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSC_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSScfg.h"

#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"

#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSSCerr.h"


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
** Function    : IXSSC_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSSC_k_FSE_xxx and
**                                        IXSSC_k_NFSE_xxx})
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
CSS_t_INT IXSSC_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSSC_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSSC_k_FSE_AIS_STATE_ERR_INIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIS_STATE_ERR_INIT - IXSSC_Init() was called in state "
        "IXSSS_k_ST_RUNNING"));
      break;
    }

    case IXSSC_k_FSE_AIS_STATE_ERR_TERM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIS_STATE_ERR_TERM - IXSSC_Terminate() was called in "
        "state IXSSS_k_ST_NON_EXIST"));
      break;
    }

    case IXSSC_k_FSE_INC_STATE_INV_INIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_STATE_INV_INIT - IXSSC_Init() detected that the IXSSC "
        "state machine is in an invalid state (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_INC_STATE_INV_TERM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_STATE_INV_TERM - IXSSC_Terminate() detected that the "
        " IXSSC state machine is in an invalid state (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_AIP_INV_VENDOR_ID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_INV_VENDOR_ID - IXSSC_Init() called with an invalid "
        "Vendor ID (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_AIP_INV_PROD_CODE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_INV_PROD_CODE - IXSSC_Init() called with an invalid "
        "Product Code (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_AIP_INV_REVISION:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_INV_REVISION - IXSSC_Init() called with an Invalid "
        "Revision (%3" CSS_PRIu16 ".%3" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_AIP_PTR_INV_CMDPROC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_PTR_INV_CMDPROC - IXSSC_Init() called with an invalid "
        "pointer"));
      break;
    }

    case IXSSC_k_NFSE_TXH_HALC_COMM_TERM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_NFSE_TXH_HALC_COMM_TERM - Sending termination message to CSAL "
        "was not possible (HALCS_TxDataPut() returned error)"));
      break;
    }

    case IXSSC_k_FSE_AIP_PTR_INV_SEVG_B:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_PTR_INV_SEVG_B - IXSSC_SoftErrByteGet() called with "
        "an invalid pointer"));
      break;
    }

    case IXSSC_k_FSE_INC_MOD_CNT_SEVG_B:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_MOD_CNT_SEVG_B - IXSSC_SoftErrByteGet() detected an "
        "internal inconsistency (module counter invalid, u8_ModuleCnt=%"
        CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_AIP_PTR_INV_SEVG_V:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_AIP_PTR_INV_SEVG_V - IXSSC_SoftErrVarGet() called with an "
        "invalid pointer"));
      break;
    }

    case IXSSC_k_FSE_INC_MOD_CNT_SEVG_V:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_MOD_CNT_SEVG_V - IXSSC_SoftErrVarGet() detected an "
        "internal inconsistency (module counter invalid, u8_ModuleCnt=%"
        CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_INC_INV_LVL_V_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_INV_LVL_V_1 - IXSSC_SoftErrVarGet() detected an "
        "internal inconsistency (invalid incLevel returned from sub-function, "
        "u8_ModuleCnt=%" CSS_PRIu32 ") ",
        dw_addInfo));
      break;
    }

    case IXSSC_k_FSE_INC_INV_LVL_V_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSSC_k_FSE_INC_INV_LVL_V_2 - IXSSC_SoftErrVarGet() detected an "
        "internal inconsistency (invalid incLevel returned from sub-function, "
        "u8_ModuleCnt=%" CSS_PRIu32 ") ",
        dw_addInfo));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSSC unit (%" CSS_PRIu16 ") (%"
        CSS_PRIu32 ")",
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


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */

/*** End Of File ***/



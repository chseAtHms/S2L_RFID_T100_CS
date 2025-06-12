/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIerr.c
**    Summary: IXSAI - Safety Assembly Object Interface
**             This file implements the (optional) error strings of the IXSAI
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_ErrorStrGet
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

#include "IXSCF.h"
#include "IXSERapi.h"

#include "IXSAIerr.h"


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
** Function    : IXSAI_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSAI_k_FSE_xxx})
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
CSS_t_INT IXSAI_ErrorStrGet(CSS_t_WORD w_errorCode,
                             CSS_t_UINT u16_instId,
                             CSS_t_DWORD dw_addInfo,
                             CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                             CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSAI_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSAI_k_FSE_AIS_STATE_ERR_RMS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIS_STATE_ERR_RMS - IXSAI_AsmIoInstDataRunModeSet() "
        "called in invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIS_STATE_ERR_DS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIS_STATE_ERR_DS - IXSAI_AsmIoInstDataSet() called in "
        "invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIS_STATE_ERR_IOS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIS_STATE_ERR_IOS - IXSAI_AsmIoInstSetup() called in "
        "invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIS_STATE_ERROR_DG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIS_STATE_ERROR_DG - IXSAI_AsmIoInstDataGet() called in "
        "invalid CSS state (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_ASM_NOT_FOUND_I:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_ASM_NOT_FOUND_I - internal function was called with "
        "invalid assembly instance (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_ASM_NOT_FOUND_O:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_ASM_NOT_FOUND_O - internal function was called with "
        "invalid assembly instance (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_IO_INST_SETUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_IO_INST_SETUP - IXSAI_AsmIoInstSetup() called with an "
        "invalid Instance ID (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_ASM_TYPE_IOS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_ASM_TYPE_IOS - IXSAI_AsmIoInstSetup() cannot execute "
        "the requested operation on this type of assembly (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_IDX_IS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_IDX_IS - passed assembly index is unknown or "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_IDX_RMS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_IDX_RMS - IXSAI_AsmIoInstDataRunModeSet() "
        "detected an invalid Assembly index (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_IDX_DS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_IDX_DS - IXSAI_AsmIoInstDataSet() detected an "
        "invalid Assembly index (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_IDX_OS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_IDX_OS - passed assembly index is unknown or "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_IDX_DG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_IDX_DG - IXSAI_AsmIoInstDataGet() detected an "
        "invalid Assembly index (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_IO_LEN_SETUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_IO_LEN_SETUP - IXSAI_AsmIoInstSetup() called with an "
        "invalid length (inst=%" CSS_PRIu16 " len=%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_ASM_LEN_IDS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_ASM_LEN_IDS - IXSAI_AsmIoInstDataSet() detected "
        "an invalid length parameter (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_INV_ASM_LEN_DG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_INV_ASM_LEN_DG - passed length parameter is invalid "
        "(%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_INST_IIFIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_INST_IIFIG - passed Instance ID parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_INV_ASM_LEN_ODS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_INV_ASM_LEN_ODS - passed length parameter is invalid "
        "(%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_PTR_DS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_PTR_DS - IXSAI_AsmIoInstDataSet() called "
        "with an invalid pointer (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_AIP_INV_PTR_DG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_AIP_INV_PTR_DG - IXSAI_AsmIoInstDataGet() called "
        "with an invalid pointer (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_IN_NOT_SUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_IN_NOT_SUP - IXSAI_AsmIoInstSetup() called "
        "with an unsupported Assembly type (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_OUT_NOT_SUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_OUT_NOT_SUP - IXSAI_AsmIoInstSetup() called "
        "with an unsupported Assembly type (inst=%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_INV_ASM_ID_SSVI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_INV_ASM_ID_SSVI - Invalid Assembly Index detected "
        "while trying to set Safe Values in Input Assembly (inst=%" CSS_PRIu16
        ")",
        u16_instId));
      break;
    }

    case IXSAI_k_FSE_INC_INV_ASM_ID_SSVO:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSAI_k_FSE_INC_INV_ASM_ID_SSVO - Invalid Assembly Index detected "
        "while trying to set Safe Values in Output Assembly (inst=%" CSS_PRIu16
        ")",
        u16_instId));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSAI unit (%" CSS_PRIu16 ") (%"
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



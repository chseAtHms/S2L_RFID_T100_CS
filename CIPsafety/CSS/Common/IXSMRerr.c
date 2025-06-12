/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSMRerr.c
**    Summary: IXSMR - Safety Message Router Object
**             This file implements the (optional) error strings of the IXSMR
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSMR_ErrorStrGet
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

#include "IXSMRerr.h"


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
** Function    : IXSMR_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSMR_k_FSE_xxx and
**                                        IXSMR_k_NFSE_xxx})
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
CSS_t_INT IXSMR_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSMR_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSMR_k_NFSE_RXH_CMD_UNKNOWN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXH_CMD_UNKNOWN - IXSMR_CmdProcess() has received an "
        "unknown Command from HALC (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSMR_k_NFSE_TXH_RSP_SEND_ERR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_TXH_RSP_SEND_ERR - Error sending Explicit Response "
        "message to CSAL for service 0x%02" CSS_PRIX32, dw_addInfo));
      break;
    }

    case IXSMR_k_FSE_AIS_STATE_ERROR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_FSE_AIS_STATE_ERROR - IXSMR_CmdProcess() called in an invalid "
        "stack state"));
      break;
    }

    case IXSMR_k_FSE_AIP_PTR_INV_CMDPROC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_FSE_AIP_PTR_INV_CMDPROC - IXSMR_CmdProcess() called with an "
        "invalid pointer"));
      break;
    }

    case IXSMR_k_FSE_AIP_PTR_INV_EXPLMSG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_FSE_AIP_PTR_INV_EXPLMSG - IXSMR_CmdProcess() detected an "
        "invalid data pointer in HALC message"));
      break;
    }

    case IXSMR_k_FSE_INC_BUF_OVERWRITE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_FSE_INC_BUF_OVERWRITE - Data in the Explicit response message "
        "was written beyond end of buffer"));
      break;
    }

    case IXSMR_k_NFSE_RXE_MSG_TOO_SHORT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXE_MSG_TOO_SHORT - the received message is too short "
        "for being a valid explicit request (%" CSS_PRIu32 " bytes)",
        dw_addInfo));
      break;
    }

    case IXSMR_k_NFSE_RXE_REQ_PATH_SIZE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXE_REQ_PATH_SIZE - the request path length contained "
        "in the message (%" CSS_PRIu16 ") is more than remaining message "
        "length (%" CSS_PRIu32 " bytes)",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSMR_k_NFSE_RXE_REQ_WO_CLASS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXE_REQ_WO_CLASS - request path in explicit message "
        "did not contain a Class ID"));
      break;
    }

    case IXSMR_k_NFSE_RXE_REQ_WO_INST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXE_REQ_WO_INST - request path in explicit message "
        "did not contain an Instance ID"));
      break;
    }

    case IXSMR_k_NFSE_RXE_UNKNOWN_CLASS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSMR_k_NFSE_RXE_UNKNOWN_CLASS - request path in explicit message "
        "addresses an unknown Class"));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSMR unit (%" CSS_PRIu16 ") (%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);
}


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */

/*** End Of File ***/



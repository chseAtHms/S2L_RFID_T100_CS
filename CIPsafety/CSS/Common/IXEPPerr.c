/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPPerr.c
**    Summary: IXEPP - Encoded Path Parser
**             This module contains the error strings of the EPATH parsing unit.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_ErrorStrGet
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

#include "IXEPPerr.h"


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
** Function    : IXEPP_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXEPP_k_FSE_xxx and
**                                        IXEPP_k_NFSE_xxx})
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
CSS_t_INT IXEPP_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXEPP_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXEPP_k_FSE_INC_PTR_INV_EPPARSE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_FSE_INC_PTR_INV_EPPARSE - IXEPP_EPathParse() called with "
        "an invalid pointer"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX1 - Unknown Segment Type found (0x%02"
        CSS_PRIX32 ")",
        dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX3 - Unknown electronic key format "
        "(0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX4:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX4 - Unknown 8-bit logical segment "
        "(0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX5:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX5 - Unknown 16- or 32-bit logical "
        "segment type (0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX6:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX6 - Unknown data segment type "
        "(0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_SYNTAX7:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_SYNTAX7 - Unknown network segment type "
        "(0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_TOO_SMALL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_TOO_SMALL - Incomplete segment detected"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_ATTR1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_ATTR1 - Too many Attribute IDs found in "
        "EPathParseSegLogicalVal8()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_ATTR2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_ATTR2 - Too many Attribute IDs found in "
        "EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_EKEY:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_EKEY - More than one electronic key found"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_DATA:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_DATA - More than one data segment found"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_SNTWK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_SNTWK - More than one safety network segment "
        "found"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_CLASS1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_CLASS1 - Too many Class IDs found in "
        "EPathParseSegLogicalVal8()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_CLASS2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_CLASS2 - Too many Class IDs found in "
        "EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_INST1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_INST1 - Too many Instance IDs found in "
        "EPathParseSegLogicalVal8()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_INST2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_INST2 - Too many Instance IDs (16 bits) "
        "found in EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_INST3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_INST3 - Too many Instance IDs (32 bits) "
        "found in EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_CP1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_CP1 - Too many Connection Points found in "
        "EPathParseSegLogicalVal8()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_CP2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_CP2 - Too many Connection Points (16 bits) "
        "found in EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_CP3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_CP3 - Too many Connection Points (32 bits) "
        "found in EPathParseSegLogicalVal16_32()"
        ));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_MEMB1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_MEMB1 - Too many Member IDs found in "
        "EPathParseSegLogicalVal8()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_MEMB2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_MEMB2 - Too many Member IDs found in "
        "EPathParseSegLogicalVal16_32()"));
      break;
    }

    case IXEPP_k_NFSE_RXE_EP_REP_MEMB3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXEPP_k_NFSE_RXE_EP_REP_MEMB3 - Too many Member IDs found in "
        "EPathParseSegLogicalVal16_32()"));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXEPP unit (%" CSS_PRIu16 ") (%"
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



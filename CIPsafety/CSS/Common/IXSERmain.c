/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSERmain.c
**    Summary: IXSER - Safety Error Unit
**             This module contains the optional function IXSER_ErrorStrGet() to
**             obtain a textual description to a passed error code.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSER_ErrorStrGet
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

#include "IXEPPerr.h"
#include "IXSSOerr.h"
#include "IXSSCerr.h"
#include "IXSCEerr.h"
#include "IXSVOerr.h"
#include "IXSMRerr.h"
#include "IXSAIerr.h"
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #include "IXSVSerr.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVCerr.h"
#endif
#include "IXSVDerr.h"
#include "IXSFAerr.h"
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "IXCCOerr.h"
#endif
#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  #include "IXSPIerr.h"
#endif
#include "IXSCF.h"

#include "IXSERapi.h"


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
** Function    : IXSER_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly. Every unit stores its error strings and returns them
**               if requested. The caller must provide a buffer for the string.
**               The error strings have variable length, but the caller has to
**               take into account that the passed buffer must be able to store
**               at least IXSER_k_MAX_ERROR_STRING_LEN number of characters. If
**               a larger buffer is provided the space after the string will
**               stay empty.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {error coding} in IXSERapi.h)
**               u16_instId (IN)        - instance Id (e.g. of the safety
**                                        validator instance that caused the
**                                        error), usage is error code specific
**                                        (not checked, any value allowed)
**               dw_addInfo (IN)        - additional error information.
**                                        (not checked, any value allowed)
**               u16_errStrBufSize (IN) - Size of the error string buffer.
**                                        (checked in IXSER_ErrorStrGet(),
**                                        valid range:
**                                        >= IXSER_k_MAX_ERROR_STRING_LEN)
**               pc_errStr (OUT)        - pointer to error string buffer (buffer
**                                        to where this function will write the
**                                        error string)
**                                        (checked)
**                                        valid range: <> CSS_k_NULL
**
** Returnvalue : >0                     - success, length of the error string in
**                                        the error string buffer without '\0'
**               IXSER_k_BUF_PTR_NULL   - pointer to the error string buffer
**                                        (see pc_errStr) is CSS_k_NULL
**               IXSER_k_BUF_TOO_SML    - the error string buffer (see
**                                        u16_errStrBufSize) is too small
**
*******************************************************************************/
CSS_t_INT IXSER_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_UINT u16_errStrBufSize,
                            CSS_t_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  /* if passed pointer is invalid */
  if (pc_errStr == CSS_k_NULL)
  {
    /* passed pointer to buffer is invalid! */
    /* return value already set */
  }
  /* else if passed buffer is too small */
  else if (u16_errStrBufSize < IXSER_k_MAX_ERROR_STRING_LEN)
  {
    i_ret = IXSER_k_BUF_TOO_SML;
  }
  else /* else: passed parameters are ok */
  {
    /* if the error code has not been generated by the CSS */
    if (IXSER_GET_ERROR_LAYER(w_errorCode) != IXSER_LAYER_CSS)
    {
      i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                             (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
        "IXSER_ErrorStrGet() was called with an invalid error code: "
        "the passed error code has not been generated by the CSS"));
    }
    else /* else: it is a CSS error code */
    {
      /* distribute the error to the assigned unit */
      switch (IXSER_GET_UNIT_ID(w_errorCode))
      {
        case CSOS_k_CSS_IXSSC:
        {
          i_ret = IXSSC_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSAI:
        {
          i_ret = IXSAI_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSCE:
        {
          i_ret = IXSCE_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSMR:
        {
          i_ret = IXSMR_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSSO:
        {
          i_ret = IXSSO_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSVO:
        {
          i_ret = IXSVO_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

      #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
        case CSOS_k_CSS_IXSVC:
        {
          i_ret = IXSVC_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }
      #endif

      #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
        case CSOS_k_CSS_IXSVS:
        {
          i_ret = IXSVS_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }
      #endif

        case CSOS_k_CSS_IXCRC:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "unit IXCRC does not generate error codes."));
          break;
        }

        case CSOS_k_CSS_IXSVD:
        {
          i_ret = IXSVD_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSER:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "unit IXSER does not generate error codes."));
          break;
        }

        case CSOS_k_CSS_IXSFA:
        {
          i_ret = IXSFA_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXUTL:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "unit IXUTL does not generate error codes."));
          break;
        }

        case CSOS_k_CSS_IXEPP:
        {
          i_ret = IXEPP_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }

        case CSOS_k_CSS_IXSCF:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "unit IXSCF does not generate error codes."));
          break;
        }

        case CSOS_k_CSS_IXSSS:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "unit IXSSS does not generate error codes."));
          break;
        }

      #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
        case CSOS_k_CSS_IXCCO:
        {
          i_ret = IXCCO_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }
      #endif

      #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
        case CSOS_k_CSS_IXSPI:
        {
          i_ret = IXSPI_ErrorStrGet(w_errorCode, u16_instId, dw_addInfo,
                                    (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
                                    (CSS_t_SNPRINTF_CHAR *)pc_errStr);
          break;
        }
      #endif

        default:
        {
          i_ret = CSS_SNPRINTF(((CSS_t_SNPRINTF_CHAR *)pc_errStr,
                                (CSS_t_SNPRINTF_BUF_SIZE)u16_errStrBufSize,
            "IXSER_ErrorStrGet() was called with an invalid error code: "
            "invalid Unit (%" CSS_PRIu16 ")", IXSER_GET_UNIT_ID(w_errorCode)));
          break;
        }
      }
    }

    /* According to the ANSI C99 definition the return value of snprintf is the
       number of characters that would have been written if n had been
       sufficiently large (not counting the terminating null character).
       Unfortunately in many implementations the return value is < 0 if the
       provided buffer is too small.
       To have a consistent behavior of all implementations we check the return
       value and also change it to -1 if the buffer was too small.
       Some implementation also don't terminate the string with a null character
       if the string is >= buffer.
    */

    /* Suppression of lint warning e960: "MISRA 2004 Required Rule 7.1, Octal
       constant or escape sequence used". This message is only issued by older
       PC-Lint versions. Versions (at least from 8.00x onwards) already
       consider the exception explicitly stated in MISRA Rule 7.1: "The
       integer constant zero (written as a single numeric digit), is strictly
       speaking an octal constant, but is a permitted exception to this rule."
    */
    /* make sure there is a null termination at the end */
    pc_errStr[u16_errStrBufSize-1U] = '\0';  /*lint !e960 (escape sequence) */

    /* if return value indicates that the provided buffer was too small */
    if (i_ret > (CSS_t_INT)u16_errStrBufSize)
    {
      /* adapt the snprintf return value */
      i_ret = IXSER_k_BUF_TOO_SML;
    }/* CCT_SKIP */ /* may be unreachable - (reachability platform dependent) */
    /* else: if the returned length matches exactly the provided buffer size */
    else if (i_ret == (CSS_t_INT)u16_errStrBufSize)
    {
      /* don't change the return value */
    }/* CCT_SKIP */ /* may be unreachable - (reachability platform dependent) */
    /* else: if the returned length is zero or negative */
    else if (i_ret <= 0)
    {
      /* some kind of error */
      /* adapt the snprintf return value */
      i_ret = IXSER_k_BUF_TOO_SML;
    }/* CCT_SKIP */ /* may be unreachable - (reachability platform dependent) */
    else  /* else: buffer was ok and printing the string in it was successful */
    {
      /* nothing to do here */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);

/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */



/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* #if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */

/*** End Of File ***/


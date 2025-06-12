/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSFAerr.c
**    Summary: IXSFA - Safety Frame Assembly
**             This file implements the (optional) error strings of the IXSFA
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSFA_ErrorStrGet
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

#include "IXSFAerr.h"


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
** Function    : IXSFA_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSFA_k_FSE_xxx and
**                                        IXSFA_k_NFSE_xxx})
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
CSS_t_INT IXSFA_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSFA_FSE_INC_EFS_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_EFS_FRAME_LEN - Invalid frame length passed when "
        "generating Extended Format Short message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    case IXSFA_FSE_INC_EFL_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_EFL_FRAME_LEN - Invalid frame length passed when "
        "generating Extended Format Long message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    case IXSFA_FSE_INC_EFTCR_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_EFTCR_FRAME_LEN - Invalid frame length passed when "
        "generating Extended Format Long message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    case IXSFA_FSE_INC_BFS_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_BFS_FRAME_LEN - Invalid frame length passed when "
        "generating Base Format Short message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    case IXSFA_FSE_INC_BFL_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_BFL_FRAME_LEN - Invalid frame length passed when "
        "generating Base Format Long message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    case IXSFA_FSE_INC_BFTCR_FRAME_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSFA_FSE_INC_BFTCR_FRAME_LEN - Invalid frame length passed when "
        "generating Base Format Long message (passed len=%" CSS_PRIu32 ")",
         dw_addInfo));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSFA unit (%" CSS_PRIu16 ") (%"
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



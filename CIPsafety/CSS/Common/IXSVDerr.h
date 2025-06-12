/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVDerr.h
**    Summary: IXSVD - Safety Validator Data
**             IXSVDerr.h defines the error codes of the IXSVD unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVD_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVDERR_H
#define IXSVDERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSVD_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVD_k_NFSE_xxx}
#define IXSVD_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSVD, u8_unitErr)
*/

/** IXSVD_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVD_k_FSE_xxx}
*/
#define IXSVD_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSVD, u8_unitErr)


/** IXSVD_k_FSE_xxx and IXSVD_k_NFSE_xxx:
    Unit Errors of this module.
*/
/* Instance ID / Index mismatch on assignment */
#define IXSVD_k_FSE_INC_IDX_MISM_ASSIGN IXSVD_ERR_____FAIL_SAFE(0x02U)
/* Index mismatch on deleting */
#define IXSVD_k_FSE_INC_IDX_MISM_DELETE IXSVD_ERR_____FAIL_SAFE(0x03U)


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVD_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVD_k_FSE_xxx and
**                                        IXSVD_k_NFSE_xxx})
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
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  CSS_t_INT IXSVD_ErrorStrGet(CSS_t_WORD w_errorCode,
                               CSS_t_UINT u16_instId,
                               CSS_t_DWORD dw_addInfo,
                               CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                               CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSVDERR_H */

/*** End Of File ***/


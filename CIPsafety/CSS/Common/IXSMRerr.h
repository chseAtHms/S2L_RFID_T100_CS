/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSMRerr.h
**    Summary: IXSMR - Safety Message Router Object
**             IXSMRerr.h defines the error codes of the IXSMR unit.
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

#ifndef IXSMRERR_H
#define IXSMRERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSMR_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSMR_k_NFSE_xxx}
*/
#define IXSMR_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSMR, u8_unitErr)

/** IXSMR_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSMR_k_FSE_xxx}
*/
#define IXSMR_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSMR, u8_unitErr)


/** IXSMR_k_FSE_xxx and IXSMR_k_NFSE_xxx:
    Unit Errors of this module.
*/
/* program flow error */
#define IXSMR_k_FSE_INC_PRG_FLOW        IXSMR_ERR_____FAIL_SAFE(0x01U)
/* unit received an unknown cmd */
#define IXSMR_k_NFSE_RXH_CMD_UNKNOWN    IXSMR_ERR_NOT_FAIL_SAFE(0x02U)
/* error sending msg to CSAL */
#define IXSMR_k_NFSE_TXH_RSP_SEND_ERR   IXSMR_ERR_NOT_FAIL_SAFE(0x03U)
/* API func called in inv state */
#define IXSMR_k_FSE_AIS_STATE_ERROR     IXSMR_ERR_____FAIL_SAFE(0x04U)
/* func called with invalid ptr */
#define IXSMR_k_FSE_AIP_PTR_INV_CMDPROC IXSMR_ERR_____FAIL_SAFE(0x05U)
/* func called with invalid ptr */
#define IXSMR_k_FSE_AIP_PTR_INV_EXPLMSG IXSMR_ERR_____FAIL_SAFE(0x06U)
/* written beyond ExplRspBuf */
#define IXSMR_k_FSE_INC_BUF_OVERWRITE   IXSMR_ERR_____FAIL_SAFE(0x07U)
/* received message too short */
#define IXSMR_k_NFSE_RXE_MSG_TOO_SHORT  IXSMR_ERR_NOT_FAIL_SAFE(0x80U)
/* req path size > rem msg len */
#define IXSMR_k_NFSE_RXE_REQ_PATH_SIZE  IXSMR_ERR_NOT_FAIL_SAFE(0x81U)
/* req path w/o contain class */
#define IXSMR_k_NFSE_RXE_REQ_WO_CLASS   IXSMR_ERR_NOT_FAIL_SAFE(0x82U)
/* req path w/o contain inst */
#define IXSMR_k_NFSE_RXE_REQ_WO_INST    IXSMR_ERR_NOT_FAIL_SAFE(0x83U)
/* req addresses unknown class */
#define IXSMR_k_NFSE_RXE_UNKNOWN_CLASS  IXSMR_ERR_NOT_FAIL_SAFE(0x85U)


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
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  CSS_t_INT IXSMR_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                              CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSMRERR_H */

/*** End Of File ***/


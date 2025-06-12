/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSCerr.h
**    Summary: IXSSC - Safety Stack Control
**             IXSSCerr.h defines the error codes of the IXSSC unit.
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

#ifndef IXSSCERR_H
#define IXSSCERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSSC_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSSC_k_NFSE_xxx}
*/
#define IXSSC_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSSC, u8_unitErr)


/** IXSSC_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSSC_k_FSE_xxx}
*/
#define IXSSC_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSSC, u8_unitErr)


/** IXSSC_k_FSE_xxx and IXSSC_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSSC_k_FSE_AIP_PTR_INV_SEVG)   not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_INC_MOD_CNT_SEVG)   not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_AIP_PTR_INV_SEVG_B) not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_INC_MOD_CNT_SEVG_B) not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_AIP_PTR_INV_SEVG_V) not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_INC_MOD_CNT_SEVG_V) not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_INC_INV_LVL_V_1)    not referenced in every cfg */
/*lint -esym(755, IXSSC_k_FSE_INC_INV_LVL_V_2)    not referenced in every cfg */
/* program flow error */
#define IXSSC_k_FSE_INC_PRG_FLOW        IXSSC_ERR_____FAIL_SAFE(0x01U)
/* API call in wrong Stack State */
#define IXSSC_k_FSE_AIS_STATE_ERR_INIT  IXSSC_ERR_____FAIL_SAFE(0x02U)
/* API call in wrong Stack State */
#define IXSSC_k_FSE_AIS_STATE_ERR_TERM  IXSSC_ERR_____FAIL_SAFE(0x03U)
/* state machine in invalid state */
#define IXSSC_k_FSE_INC_STATE_INV_INIT  IXSSC_ERR_____FAIL_SAFE(0x04U)
/* state machine in invalid state */
#define IXSSC_k_FSE_INC_STATE_INV_TERM  IXSSC_ERR_____FAIL_SAFE(0x05U)
/* invalid Vendor ID */
#define IXSSC_k_FSE_AIP_INV_VENDOR_ID   IXSSC_ERR_____FAIL_SAFE(0x06U)
/* invalid product code */
#define IXSSC_k_FSE_AIP_INV_PROD_CODE   IXSSC_ERR_____FAIL_SAFE(0x07U)
/* invalid revision (major/minor) */
#define IXSSC_k_FSE_AIP_INV_REVISION    IXSSC_ERR_____FAIL_SAFE(0x08U)
/* func called with invalid ptr */
#define IXSSC_k_FSE_AIP_PTR_INV_CMDPROC IXSSC_ERR_____FAIL_SAFE(0x09U)
/* sending termination info to CSAL was not possible */
#define IXSSC_k_NFSE_TXH_HALC_COMM_TERM IXSSC_ERR_NOT_FAIL_SAFE(0x0AU)
/* invalid pointer passed to Soft Error checking function */
#define IXSSC_k_FSE_AIP_PTR_INV_SEVG_B  IXSSC_ERR_____FAIL_SAFE(0x0BU)
/* invalid internal variable (module counter) detected in Soft Error checking */
#define IXSSC_k_FSE_INC_MOD_CNT_SEVG_B  IXSSC_ERR_____FAIL_SAFE(0x0CU)
/* invalid pointer passed to Soft Error checking function */
#define IXSSC_k_FSE_AIP_PTR_INV_SEVG_V  IXSSC_ERR_____FAIL_SAFE(0x0DU)
/* invalid internal variable (module counter) detected in Soft Error checking */
#define IXSSC_k_FSE_INC_MOD_CNT_SEVG_V  IXSSC_ERR_____FAIL_SAFE(0x0EU)
/* invalid incLevel returned from Soft Error checking sub-function */
#define IXSSC_k_FSE_INC_INV_LVL_V_1     IXSSC_ERR_____FAIL_SAFE(0x0FU)
/* invalid incLevel returned from Soft Error checking sub-function */
#define IXSSC_k_FSE_INC_INV_LVL_V_2     IXSSC_ERR_____FAIL_SAFE(0x10U)


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
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  CSS_t_INT IXSSC_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                              CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSSCERR_H */

/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIerr.h
**    Summary: IXSAI - Safety Assembly Object Interface
**             IXSAIerr.h defines the error codes of the IXSAI unit.
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

#ifndef IXSAIERR_H
#define IXSAIERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSAI_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSAI_k_NFSE_xxx}
#define IXSAI_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSAI, u8_unitErr)
*/

/** IXSAI_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSAI_k_FSE_xxx}
*/
#define IXSAI_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSAI, u8_unitErr)


/** IXSAI_k_FSE_xxx and IXSAI_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSAI_k_FSE_AIS_STATE_ERR_RMS)  not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIS_STATE_ERR_DS)   not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIS_STATE_ERROR_DG) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_ASM_NOT_FOUND_O) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_ASM_NOT_FOUND_I) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_IO_INST_SETUP)  not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_IDX_IS) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_IDX_RMS) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_IDX_DS) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_IDX_OS) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_IDX_DG) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_ASM_LEN_IDS) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_INV_ASM_LEN_DG) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_INST_IIFIG) not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_INV_ASM_LEN_ODS) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_PTR_DS)     not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_AIP_INV_PTR_DG)     not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_IN_NOT_SUP)     not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_OUT_NOT_SUP)    not referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_INV_ASM_ID_SSVI) n. referenced in every cfg */
/*lint -esym(755, IXSAI_k_FSE_INC_INV_ASM_ID_SSVO) n. referenced in every cfg */
/* program flow error */
#define IXSAI_k_FSE_INC_PRG_FLOW        IXSAI_ERR_____FAIL_SAFE(0x01U)
/* API call in wrong Stack State */
#define IXSAI_k_FSE_AIS_STATE_ERR_RMS   IXSAI_ERR_____FAIL_SAFE(0x03U)
/* API call in wrong Stack State */
#define IXSAI_k_FSE_AIS_STATE_ERR_DS    IXSAI_ERR_____FAIL_SAFE(0x04U)
/* API call in wrong Stack State */
#define IXSAI_k_FSE_AIS_STATE_ERR_IOS   IXSAI_ERR_____FAIL_SAFE(0x05U)
/* API call in wrong Stack State */
#define IXSAI_k_FSE_AIS_STATE_ERROR_DG  IXSAI_ERR_____FAIL_SAFE(0x08U)
/* no matching asm inst found */
#define IXSAI_k_FSE_INC_ASM_NOT_FOUND_O IXSAI_ERR_____FAIL_SAFE(0x09U)
/* no matching asm inst found */
#define IXSAI_k_FSE_INC_ASM_NOT_FOUND_I IXSAI_ERR_____FAIL_SAFE(0x0AU)
/* invalid instance when trying to set up an Assembly Instance */
#define IXSAI_k_FSE_AIP_IO_INST_SETUP   IXSAI_ERR_____FAIL_SAFE(0x0CU)
/* passed asm is of invalid type */
#define IXSAI_k_FSE_AIP_ASM_TYPE_IOS    IXSAI_ERR_____FAIL_SAFE(0x0EU)
/* passed asm idx is invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_IDX_IS  IXSAI_ERR_____FAIL_SAFE(0x0FU)
/* passed asm idx is invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_IDX_RMS IXSAI_ERR_____FAIL_SAFE(0x10U)
/* passed asm idx is invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_IDX_DS  IXSAI_ERR_____FAIL_SAFE(0x11U)
/* passed asm idx is invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_IDX_OS  IXSAI_ERR_____FAIL_SAFE(0x13U)
/* passed asm idx is invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_IDX_DG  IXSAI_ERR_____FAIL_SAFE(0x15U)
/* passed length is of invalid */
#define IXSAI_k_FSE_AIP_IO_LEN_SETUP    IXSAI_ERR_____FAIL_SAFE(0x16U)
/* passed length is of invalid */
#define IXSAI_k_FSE_AIP_INV_ASM_LEN_IDS IXSAI_ERR_____FAIL_SAFE(0x17U)
/* passed length is of invalid */
#define IXSAI_k_FSE_INC_INV_ASM_LEN_DG  IXSAI_ERR_____FAIL_SAFE(0x18U)
/* passed Instance ID is invalid */
#define IXSAI_k_FSE_AIP_INV_INST_IIFIG  IXSAI_ERR_____FAIL_SAFE(0x19U)
/* passed length is of invalid */
#define IXSAI_k_FSE_INC_INV_ASM_LEN_ODS IXSAI_ERR_____FAIL_SAFE(0x1AU)
/* passed pointer is invalid in IXSAI_AsmIoInstDataSet() */
#define IXSAI_k_FSE_AIP_INV_PTR_DS      IXSAI_ERR_____FAIL_SAFE(0x1DU)
/* passed pointer is invalid in IXSAI_AsmInputDataGet() */
#define IXSAI_k_FSE_AIP_INV_PTR_DG      IXSAI_ERR_____FAIL_SAFE(0x1FU)
/* the passed Assembly Type is not supported */
#define IXSAI_k_FSE_INC_IN_NOT_SUP      IXSAI_ERR_____FAIL_SAFE(0x20U)
/* the passed Assembly Type is not supported */
#define IXSAI_k_FSE_INC_OUT_NOT_SUP     IXSAI_ERR_____FAIL_SAFE(0x21U)
/* passed asm instance ID is invalid */
#define IXSAI_k_FSE_INC_INV_ASM_ID_SSVI IXSAI_ERR_____FAIL_SAFE(0x22U)
/* passed asm instance ID is invalid */
#define IXSAI_k_FSE_INC_INV_ASM_ID_SSVO IXSAI_ERR_____FAIL_SAFE(0x23U)


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
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  CSS_t_INT IXSAI_ErrorStrGet(CSS_t_WORD w_errorCode,
                               CSS_t_UINT u16_instId,
                               CSS_t_DWORD dw_addInfo,
                               CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                               CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSAIERR_H */

/*** End Of File ***/


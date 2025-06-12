/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOerr.h
**    Summary: IXSSO - Safety Supervisor Object
**             IXSSOerr.h defines the error codes of the IXSSO unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSOERR_H
#define IXSSOERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSSO_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSSO_k_NFSE_xxx}
*/
#define IXSSO_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSSO, u8_unitErr)

/** IXSSO_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSSO_k_FSE_xxx}
*/
#define IXSSO_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSSO, u8_unitErr)


/** IXSSO_k_FSE_xxx and IXSSO_k_NFSE_xxx:
    Unit Errors of this module.
*/

/*lint -esym(755, IXSSO_k_FSE_INC_ASM_NFOUND_GET) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_ASM_NFOUND_SET) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_ASM_INVTYPE_GET) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_ASM_INVTYPE_SET) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_W_TUNID_2)   not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_R_CFUNID)    not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_R_OCPUNID)   not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_R_SCID)      not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_W_CFUNID_2)  not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_W_OCPUNID_2) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_R_AE)        not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_R_WE)        not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_NV_W_SCID_2)    not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_AIS_STATE_ERR_OTALS) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_AIP_PTR_INV_OTALS)  not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_INC_OCPIDX_INV)     not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_AIS_APPLY_CFG_CHECK) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_AIS_VAL_CFG_CHECK)  not referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_APY_TUNID_CHK) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_PROP_TUNID)    not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SEAI_PROP_TUNID_START)  not referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_FO_ABORT)      not referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_FO_CRIT_FAULT) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_RXE_FO_SELF_TEST)   not referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_FO_SELF_TESTEX) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_NFSE_RXE_FO_WAIT_TUNID) not referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_RXE_PROP_T_SELFTEST) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_RXE_APLY_T_SELFTEST) n. referenced in every cfg */
/*lint -esym(755, IXSSO_k_FSE_RXE_SRESET_SELFTEST) n. referenced in every cfg */
/* program flow error */
#define IXSSO_k_FSE_INC_PRG_FLOW        IXSSO_ERR_____FAIL_SAFE(0x01U)
/* no matching asm inst found */
#define IXSSO_k_FSE_INC_ASM_NFOUND_GET  IXSSO_ERR_____FAIL_SAFE(0x03U)
/* no matching asm inst found */
#define IXSSO_k_FSE_INC_ASM_NFOUND_SET  IXSSO_ERR_____FAIL_SAFE(0x04U)
/* passed asm is of invalid type */
#define IXSSO_k_FSE_INC_ASM_INVTYPE_GET IXSSO_ERR_____FAIL_SAFE(0x05U)
/* passed asm is of invalid type */
#define IXSSO_k_FSE_INC_ASM_INVTYPE_SET IXSSO_ERR_____FAIL_SAFE(0x06U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_W_SCID_2     IXSSO_ERR_____FAIL_SAFE(0x0FU)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_W_TUNID_2    IXSSO_ERR_____FAIL_SAFE(0x10U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_W_CFUNID_2   IXSSO_ERR_____FAIL_SAFE(0x11U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_W_OCPUNID_2  IXSSO_ERR_____FAIL_SAFE(0x12U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_AE         IXSSO_ERR_____FAIL_SAFE(0x13U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_WE         IXSSO_ERR_____FAIL_SAFE(0x14U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_CFUNID     IXSSO_ERR_____FAIL_SAFE(0x16U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_SCID       IXSSO_ERR_____FAIL_SAFE(0x17U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_TUNID      IXSSO_ERR_____FAIL_SAFE(0x18U)
/* NV-storage error */
#define IXSSO_k_FSE_INC_NV_R_OCPUNID    IXSSO_ERR_____FAIL_SAFE(0x1AU)
/* invalid connection event */
#define IXSSO_k_FSE_INC_CNXN_EVENT      IXSSO_ERR_____FAIL_SAFE(0x1BU)
/* HALC communication error */
#define IXSSO_k_NFSE_TXH_TUNID          IXSSO_ERR_NOT_FAIL_SAFE(0x1CU)
/* HALC communication error */
#define IXSSO_k_NFSE_TXH_DEVST          IXSSO_ERR_NOT_FAIL_SAFE(0x1DU)
/* Object State Conflict */
#define IXSSO_k_FSE_AIS_OSC_ST_AB       IXSSO_ERR_____FAIL_SAFE(0x1EU)
/* Object State Conflict */
#define IXSSO_k_FSE_AIS_OSC_STEX_AB     IXSSO_ERR_____FAIL_SAFE(0x1FU)
/* Object State Conflict */
#define IXSSO_k_FSE_AIS_OSC_CRIT_AB     IXSSO_ERR_____FAIL_SAFE(0x20U)
/* NOT an Error - Just a return value giving an Info: Reinit SSO StateMachine */
#define IXSSO_k_NFSE_AIS_INFO_EXCON_CLR IXSSO_ERR_NOT_FAIL_SAFE(0x21U)
/* func called in invalid stack state */
#define IXSSO_k_FSE_AIS_STATE_ERR_MC    IXSSO_ERR_____FAIL_SAFE(0x36U)
/* state machine detected undefined state */
#define IXSSO_k_FSE_INC_UNDEF_STATE     IXSSO_ERR_____FAIL_SAFE(0x37U)
/* Forward_Open req while WAIT FOR TUNID */
#define IXSSO_k_NFSE_RXE_FO_WAIT_TUNID  IXSSO_ERR_NOT_FAIL_SAFE(0x38U)
/* Forward_Open req while SELF TESTING */
#define IXSSO_k_FSE_RXE_FO_SELF_TEST    IXSSO_ERR_____FAIL_SAFE(0x39U)
/* Forward_Open req while SELF TEST EXCEPTION */
#define IXSSO_k_NFSE_RXE_FO_SELF_TESTEX IXSSO_ERR_NOT_FAIL_SAFE(0x3AU)
/* Forward_Open req while ABORT */
#define IXSSO_k_NFSE_RXE_FO_ABORT       IXSSO_ERR_NOT_FAIL_SAFE(0x3BU)
/* Forward_Open req while CRITICAL FAULT */
#define IXSSO_k_NFSE_RXE_FO_CRIT_FAULT  IXSSO_ERR_NOT_FAIL_SAFE(0x3CU)
/* Propose TUNID in invalid state */
#define IXSSO_k_NFSE_RXE_PROP_TUNID     IXSSO_ERR_NOT_FAIL_SAFE(0x3DU)
/* Propose TUNID in invalid state */
#define IXSSO_k_NFSE_RXE_APY_TUNID_CHK  IXSSO_ERR_NOT_FAIL_SAFE(0x3EU)
/* Validate Configuration Request in invalid state */
#define IXSSO_k_FSE_AIS_VAL_CFG_CHECK   IXSSO_ERR_____FAIL_SAFE(0x3FU)
/* Apply Configuration Request in invalid state */
#define IXSSO_k_FSE_AIS_APPLY_CFG_CHECK IXSSO_ERR_____FAIL_SAFE(0x40U)
/* Mode Change Request in invalid state */
#define IXSSO_k_FSE_AIS_MODE_CHANGE     IXSSO_ERR_____FAIL_SAFE(0x42U)
/* Already in Requested State */
#define IXSSO_k_FSE_AIS_AIRS_IAR_AB     IXSSO_ERR_____FAIL_SAFE(0x43U)
/* Not possible to open a connection in this state (blocked by other
   mechanisms (IXSSO_k_SE_FWD_OPEN_REQ on Targets or IXSSO_k_SE_OPEN_CNXN_REQ
   on Originators) */
#define IXSSO_k_FSE_RXE_OPEN_NOT_PSBL   IXSSO_ERR_____FAIL_SAFE(0x46U)
/* Not possible to delete a connection in Self Test Exception (we must assume
   that there cannot be any connection in this state) */
#define IXSSO_k_FSE_RXE_DEL_NOT_PSBL    IXSSO_ERR_____FAIL_SAFE(0x47U)
/* invalid state detected in state machine */
#define IXSSO_k_FSE_INC_TRANS_INV_STATE IXSSO_ERR_____FAIL_SAFE(0x48U)
/* Self-Test Failed event in invalid state */
#define IXSSO_k_FSE_INC_SELF_TEST_FAIL  IXSSO_ERR_____FAIL_SAFE(0x49U)
/* Self-Test Passed event in invalid state */
#define IXSSO_k_FSE_INC_SELF_TEST_PASS  IXSSO_ERR_____FAIL_SAFE(0x4AU)
/* API function called while CSS not initialized */
#define IXSSO_k_FSE_AIS_STATE_ERR_ESS   IXSSO_ERR_____FAIL_SAFE(0x4BU)
/* CCO tries to open a connection in invalid state */
#define IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL IXSSO_ERR_____FAIL_SAFE(0x4CU)
/* API function called while CSS not initialized */
#define IXSSO_k_FSE_AIS_STATE_ERR_OTALS IXSSO_ERR_____FAIL_SAFE(0x4DU)
/* API function called with invalid pointer */
#define IXSSO_k_FSE_AIP_PTR_INV_OTALS   IXSSO_ERR_____FAIL_SAFE(0x4EU)
/* OCPUNID for an invalid Assembly Index was requested */
#define IXSSO_k_FSE_INC_OCPIDX_INV      IXSSO_ERR_____FAIL_SAFE(0x4FU)
/* Propose TUNID in Self Testing */
#define IXSSO_k_FSE_RXE_PROP_T_SELFTEST IXSSO_ERR_____FAIL_SAFE(0x50U)
/* Apply TUNID in Self Testing */
#define IXSSO_k_FSE_RXE_APLY_T_SELFTEST IXSSO_ERR_____FAIL_SAFE(0x51U)
/* Safety Reset in Self Testing */
#define IXSSO_k_FSE_RXE_SRESET_SELFTEST IXSSO_ERR_____FAIL_SAFE(0x52U)
/* Not possible that a connection fails in Self Test Exception (we must assume
   that there cannot be any connection in this state) */
#define IXSSO_k_FSE_RXE_FAIL_NOT_PSBL   IXSSO_ERR_____FAIL_SAFE(0x53U)
/* In Wait For TUNID state there cannot be a Safety Connection Delete Event */
#define IXSSO_k_FSE_INC_SC_DEL_IN_WFT   IXSSO_ERR_____FAIL_SAFE(0x54U)
/* In Wait For TUNID state there cannot be a Safety Connection Failed Event */
#define IXSSO_k_FSE_INC_SC_FAIL_IN_WFT  IXSSO_ERR_____FAIL_SAFE(0x55U)


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
** Function    : IXSSO_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSSO_k_FSE_xxx and
**                                        IXSSO_k_NFSE_xxx})
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
  CSS_t_INT IXSSO_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                              CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSSOERR_H */

/*** End Of File ***/


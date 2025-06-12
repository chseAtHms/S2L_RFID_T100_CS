/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOerr.h
**    Summary: IXSVO - Safety Validator Object
**             IXSVOerr.h defines the error codes of the IXSVO unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVOERR_H
#define IXSVOERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSVO_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVO_k_NFSE_xxx}
*/
#define IXSVO_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSVO, u8_unitErr)


/** IXSVO_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVO_k_FSE_xxx}
*/
#define IXSVO_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSVO, u8_unitErr)


/** IXSVO_k_FSE_xxx and IXSVO_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSVO_k_FSE_AIS_STATE_ERR_DXC)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_CLOSE_INST_INV) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_INV_CNXN_TYPE_A) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_INV_CNXN_TYPE_I) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_AIP_XCHECK_INST_INV) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_AIP_XCHECK_INST_NUM) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_AIP_XCHECK_TYPE_INV) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_PREPARE_INST_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_NFSE_RXE_ORIG_INIT_INST) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_NFSE_RES_ALLOC_ORIG_CLI) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_NFSE_RES_ALLOC_ORIG_SRV) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_NFSE_RES_ALLOC_TARG_SRV) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_NFSE_RES_ALLOC_TARG_CLI) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_TARG_INIT_INST) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_CL_WRONG_TYPE_S) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_CL_WRONG_TYPE_C) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETSTAT_INST_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETSTAT_INST_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETCP_INST_S)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETCP_INST_C)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETAGE_INST_S)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_SETAGE_INST_S)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETFC_INST_S)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETFC_INST_C)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_CLEAR_INST_S)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_CLEAR_INST_C)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_IIG_INST_S)     not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_IIG_INST_C)     not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ALLOC_INST_S)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ALLOC_INST_C)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_TINIT_INST_S)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_TINIT_INST_C)   not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ALLOC_ORIG_NS_C) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ALLOC_ORIG_NS_S) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ORIG_INIT_NS_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_ORIG_INIT_NS_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETPIEM_INST_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETPIEM_INST_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTCMMM_INST_S) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTCMMM_INST_C) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETNTEM_INST_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETNTEM_INST_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTMULT_INST_S) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTMULT_INST_C) n. referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETMCN_INST_S)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETMCN_INST_C)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETLEC_INST_S)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETLEC_INST_C)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_EXP_INST_SRV_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_EXP_INST_SRV_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTYPE_INST_S) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTYPE_INST_C) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETAGE_INST_C)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_SETAGE_INST_C)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTCMMM_I_S_2) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETNTEM_I_S_2)  not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETTMULT_I_S_2) not referenced in every cfg */
/*lint -esym(755, IXSVO_k_FSE_INC_GETFC_INST_S_2) not referenced in every cfg */
/* program flow error */
#define IXSVO_k_FSE_INC_PRG_FLOW        IXSVO_ERR_____FAIL_SAFE(0x01U)
/* API call in wrong Stack State */
#define IXSVO_k_FSE_AIS_STATE_ERR_DXC   IXSVO_ERR_____FAIL_SAFE(0x05U)
/* API call in wrong Stack State */
#define IXSVO_k_FSE_AIS_STATE_ERR_IIG   IXSVO_ERR_____FAIL_SAFE(0x06U)
/* unit received an unknown cmd */
#define IXSVO_k_NFSE_RXH_CMD_UNKNOWN    IXSVO_ERR_NOT_FAIL_SAFE(0x07U)
/* invalid instance ID */
#define IXSVO_k_FSE_INC_CLOSE_INST_INV  IXSVO_ERR_____FAIL_SAFE(0x08U)
/* invalid instance ID */
#define IXSVO_k_FSE_INC_STOP_INST_INV   IXSVO_ERR_____FAIL_SAFE(0x09U)
/* invalid Consumer Number */
#define IXSVO_k_FSE_INC_STOP_CONS_NUM   IXSVO_ERR_____FAIL_SAFE(0x0AU)
/* invalid validator state */
#define IXSVO_k_FSE_INC_VDATR_STAT_IDLE IXSVO_ERR_____FAIL_SAFE(0x0BU)
/* invalid validator state */
#define IXSVO_k_FSE_INC_VDATR_STAT_ESTB IXSVO_ERR_____FAIL_SAFE(0x0CU)
/* invalid validator state */
#define IXSVO_k_FSE_INC_VDATR_STAT_FAIL IXSVO_ERR_____FAIL_SAFE(0x0DU)
/* invalid validator state */
#define IXSVO_k_FSE_INC_VDATR_STAT_INV  IXSVO_ERR_____FAIL_SAFE(0x0EU)
/* invalid connection type */
#define IXSVO_k_FSE_INC_INV_CNXN_TYPE_A IXSVO_ERR_____FAIL_SAFE(0x0FU)
/* invalid connection type */
#define IXSVO_k_FSE_INC_INV_CNXN_TYPE_I IXSVO_ERR_____FAIL_SAFE(0x10U)
/* invalid instance ID */
#define IXSVO_k_FSE_INC_TARG_INIT_INST  IXSVO_ERR_____FAIL_SAFE(0x11U)
/* invalid instance ID */
#define IXSVO_k_NFSE_RXE_ORIG_INIT_INST IXSVO_ERR_NOT_FAIL_SAFE(0x12U)
/* func called with invalid ptr */
#define IXSVO_k_FSE_AIP_PTR_INV_CMDPROC IXSVO_ERR_____FAIL_SAFE(0x13U)
/* func called with invalid ptr */
#define IXSVO_k_FSE_AIP_PTR_INV_CMDDATA IXSVO_ERR_____FAIL_SAFE(0x14U)
/* invalid instance ID */
#define IXSVO_k_FSE_AIP_XCHECK_INST_INV IXSVO_ERR_____FAIL_SAFE(0x15U)
/* instance parameter invalid */
#define IXSVO_k_FSE_AIP_XCHECK_TYPE_INV IXSVO_ERR_____FAIL_SAFE(0x16U)
/* instance parameter invalid */
#define IXSVO_k_FSE_AIP_XCHECK_INST_NUM IXSVO_ERR_____FAIL_SAFE(0x17U)
/* instance parameter invalid */
#define IXSVO_k_NFSE_RXI_INST_RX_NUM    IXSVO_ERR_NOT_FAIL_SAFE(0x18U)
/* instance parameter invalid */
#define IXSVO_k_NFSE_RXI_INST_S_RX_NUM  IXSVO_ERR_NOT_FAIL_SAFE(0x19U)
/* instance parameter invalid */
#define IXSVO_k_NFSE_RXI_INST_C_RX_NUM  IXSVO_ERR_NOT_FAIL_SAFE(0x1AU)
/* func called with invalid ptr */
#define IXSVO_k_FSE_AIP_PTR_INV_STATGET IXSVO_ERR_____FAIL_SAFE(0x1BU)
/* tried to close a not supported Safety Validator Type */
#define IXSVO_k_FSE_INC_CL_WRONG_TYPE_S IXSVO_ERR_____FAIL_SAFE(0x1CU)
/* tried to close a not supported Safety Validator Type */
#define IXSVO_k_FSE_INC_CL_WRONG_TYPE_C IXSVO_ERR_____FAIL_SAFE(0x1DU)
/* tried to stop a not supported Safety Validator Type */
#define IXSVO_k_FSE_INC_SP_WRONG_TYPE_S IXSVO_ERR_____FAIL_SAFE(0x1EU)
/* tried to stop a not supported Safety Validator Type */
#define IXSVO_k_FSE_INC_SP_WRONG_TYPE_C IXSVO_ERR_____FAIL_SAFE(0x1FU)
/* API func called in inv state */
#define IXSVO_k_FSE_AIS_STATE_ERR       IXSVO_ERR_____FAIL_SAFE(0x21U)
/* error on allocation */
#define IXSVO_k_NFSE_RES_ALLOC_ORIG_SRV IXSVO_ERR_NOT_FAIL_SAFE(0x22U)
/* error on allocation */
#define IXSVO_k_NFSE_RES_ALLOC_ORIG_CLI IXSVO_ERR_NOT_FAIL_SAFE(0x24U)
/* error on allocation */
#define IXSVO_k_NFSE_RES_ALLOC_TARG_SRV IXSVO_ERR_NOT_FAIL_SAFE(0x26U)
/* error on allocation */
#define IXSVO_k_NFSE_RES_ALLOC_TARG_CLI IXSVO_ERR_NOT_FAIL_SAFE(0x27U)
/* tried to get state of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETSTAT_INST_S  IXSVO_ERR_____FAIL_SAFE(0x29U)
/* tried to get state of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETSTAT_INST_C  IXSVO_ERR_____FAIL_SAFE(0x2AU)
/* tried to get connection point of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETCP_INST_S    IXSVO_ERR_____FAIL_SAFE(0x2BU)
/* tried to get connection point of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETCP_INST_C    IXSVO_ERR_____FAIL_SAFE(0x2CU)
/* tried to get Data Age of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETAGE_INST_S   IXSVO_ERR_____FAIL_SAFE(0x2DU)
/* tried to set Data Age of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_SETAGE_INST_S   IXSVO_ERR_____FAIL_SAFE(0x2EU)
/* tried to get Fault Counter of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETFC_INST_S    IXSVO_ERR_____FAIL_SAFE(0x2FU)
/* tried to get Fault Counter of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETFC_INST_C    IXSVO_ERR_____FAIL_SAFE(0x30U)
/* tried to prepare a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_PREPARE_INST_S  IXSVO_ERR_____FAIL_SAFE(0x31U)
/* tried to clear a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_CLEAR_INST_S    IXSVO_ERR_____FAIL_SAFE(0x32U)
/* tried to clear a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_CLEAR_INST_C    IXSVO_ERR_____FAIL_SAFE(0x33U)
/* tried to get Instance Info for a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_IIG_INST_S      IXSVO_ERR_____FAIL_SAFE(0x34U)
/* tried to get Instance Info for a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_IIG_INST_C      IXSVO_ERR_____FAIL_SAFE(0x35U)
/* tried to allocate a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_ALLOC_INST_S    IXSVO_ERR_____FAIL_SAFE(0x36U)
/* tried to allocate a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_ALLOC_INST_C    IXSVO_ERR_____FAIL_SAFE(0x37U)
/* tried to initialize a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_TINIT_INST_S    IXSVO_ERR_____FAIL_SAFE(0x38U)
/* tried to initialize a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_TINIT_INST_C    IXSVO_ERR_____FAIL_SAFE(0x39U)
/* invalid safety validator state transition */
#define IXSVO_k_FSE_INC_INV_TRANS_IDLE  IXSVO_ERR_____FAIL_SAFE(0x3AU)
/* invalid safety validator state transition */
#define IXSVO_k_FSE_INC_INV_TRANS_INIT  IXSVO_ERR_____FAIL_SAFE(0x3BU)
/* invalid safety validator state transition */
#define IXSVO_k_FSE_INC_INV_TRANS_ESTAB IXSVO_ERR_____FAIL_SAFE(0x3CU)
/* invalid safety validator state transition */
#define IXSVO_k_FSE_INC_INV_TRANS_FAIL  IXSVO_ERR_____FAIL_SAFE(0x3DU)
/* API function called with invalid instance ID */
#define IXSVO_k_FSE_AIP_INST_STATEGET   IXSVO_ERR_____FAIL_SAFE(0x3EU)
/* Tried to allocate a Client, but Clients are not supported */
#define IXSVO_k_FSE_INC_ALLOC_ORIG_NS_C IXSVO_ERR_____FAIL_SAFE(0x3FU)
/* Tried to allocate a Server, but Servers are not supported */
#define IXSVO_k_FSE_INC_ALLOC_ORIG_NS_S IXSVO_ERR_____FAIL_SAFE(0x40U)
/* Tried to initialize a Client, but Clients are not supported */
#define IXSVO_k_FSE_INC_ORIG_INIT_NS_C  IXSVO_ERR_____FAIL_SAFE(0x41U)
/* Tried to initialize a Server, but Servers are not supported */
#define IXSVO_k_FSE_INC_ORIG_INIT_NS_S  IXSVO_ERR_____FAIL_SAFE(0x42U)
/* tried to get PIEM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETPIEM_INST_S  IXSVO_ERR_____FAIL_SAFE(0x43U)
/* tried to get PIEM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETPIEM_INST_C  IXSVO_ERR_____FAIL_SAFE(0x44U)
/* tried to get TCMMM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTCMMM_INST_S IXSVO_ERR_____FAIL_SAFE(0x45U)
/* tried to get TCMMM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTCMMM_INST_C IXSVO_ERR_____FAIL_SAFE(0x46U)
/* tried to get NTEM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETNTEM_INST_S  IXSVO_ERR_____FAIL_SAFE(0x47U)
/* tried to get NTEM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETNTEM_INST_C  IXSVO_ERR_____FAIL_SAFE(0x48U)
/* tried to get Timeout Mult of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTMULT_INST_S IXSVO_ERR_____FAIL_SAFE(0x49U)
/* tried to get Timeout Mult of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTMULT_INST_C IXSVO_ERR_____FAIL_SAFE(0x4AU)
/* tried to get MaxConsNum of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETMCN_INST_S   IXSVO_ERR_____FAIL_SAFE(0x4BU)
/* tried to get MaxConsNum of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETMCN_INST_C   IXSVO_ERR_____FAIL_SAFE(0x4CU)
/* tried to get Last Error Code of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETLEC_INST_S   IXSVO_ERR_____FAIL_SAFE(0x4DU)
/* tried to get Last Error Code of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETLEC_INST_C   IXSVO_ERR_____FAIL_SAFE(0x4EU)
/* received I/O message is less than minimum length */
#define IXSVO_k_NFSE_RXI_TOO_SHORT      IXSVO_ERR_NOT_FAIL_SAFE(0x4FU)
/* inconsistency in client/server configuration - SV servers not supported */
#define IXSVO_k_FSE_INC_EXP_INST_SRV_S  IXSVO_ERR_____FAIL_SAFE(0x51U)
/* inconsistency in client/server configuration - SV clients not supported */
#define IXSVO_k_FSE_INC_EXP_INST_SRV_C  IXSVO_ERR_____FAIL_SAFE(0x52U)
/* tried to get type attribute of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTYPE_INST_S  IXSVO_ERR_____FAIL_SAFE(0x53U)
/* tried to get type attribute of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTYPE_INST_C  IXSVO_ERR_____FAIL_SAFE(0x54U)
/* tried to get Data Age of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETAGE_INST_C   IXSVO_ERR_____FAIL_SAFE(0x55U)
/* tried to set Data Age of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_SETAGE_INST_C   IXSVO_ERR_____FAIL_SAFE(0x56U)
/* tried to get TCMMM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTCMMM_I_S_2  IXSVO_ERR_____FAIL_SAFE(0x57U)
/* tried to get NTEM of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETNTEM_I_S_2   IXSVO_ERR_____FAIL_SAFE(0x58U)
/* tried to get Timeout Mult of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETTMULT_I_S_2  IXSVO_ERR_____FAIL_SAFE(0x59U)
/* tried to get Fault Counter of a not supported Safety Validator instance */
#define IXSVO_k_FSE_INC_GETFC_INST_S_2  IXSVO_ERR_____FAIL_SAFE(0x5AU)


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
** Function    : IXSVO_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVO_k_FSE_xxx})
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
  CSS_t_INT IXSVO_ErrorStrGet(CSS_t_WORD w_errorCode,
                               CSS_t_UINT u16_instId,
                               CSS_t_DWORD dw_addInfo,
                               CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                               CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSVOERR_H */

/*** End Of File ***/


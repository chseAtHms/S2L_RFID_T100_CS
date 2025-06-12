/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEerr.h
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             IXSCEerr.h defines the error codes of the IXSCE unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCEERR_H
#define IXSCEERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSCE_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSCE_k_NFSE_xxx}
*/
#define IXSCE_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSCE, u8_unitErr)

/** IXSCE_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSCE_k_FSE_xxx}
*/
#define IXSCE_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSCE, u8_unitErr)


/** IXSCE_k_FSE_xxx and IXSCE_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSCE_k_NFSE_RXH_CMD_UNKNOWN)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_TXH_C_ERRRSP_SEND) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_TXH_C_SUCRSP_SEND) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_TXH_O_ERRRSP_SEND) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_TXH_O_SUCRSP_SEND) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_AIP_PTR_INV_CMDDATA) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_AIP_PTR_INV_CMDPROC) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_AIS_STATE_ERROR)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SC_INV_CNXN)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SC_TOO_SHORT)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SC_INV_C_CPT)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CM_INVNCP)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CASM_NULL)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CFUNID)     not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CPCRC_MIS)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CS_INVNCP)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_C_CONS_NN)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_C_PROD_I)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_DUPLICATE)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_EL_KEY_1)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_EL_KEY_2)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_EL_KEY_3)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_CASM)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_MCAST_LEN)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_ITSRV1) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_LEN)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TMULT)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_PCI)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_NTEM)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_MCNUM)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_MFNUM)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_PIEM)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_STMUL)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCCID)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCEPI)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCMMM)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCNCP1)  n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCNCP2)  n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCNCP3)  n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCT_1)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCT_2)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_TCT_3)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_NO_TUNID)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SAPL_APPL)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_OCP_NF)     not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_OCPUNID)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SAPL_DENY)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SCID_MIS)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SCCRC_MIS)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SS_INVNCP)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SSO_SM_1)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SSO_SM_2)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_S_CONS_I)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_STORE_1)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_STORE_2)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_STORE_3)    not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_S_PROD_NN)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_TOOL_OWND)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_TUNID_MIS)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_UNCONF)     not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_BASE_NOT_SUP)  not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_EXT_NOT_SUP)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_CNXN_SIZE_CHECK) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_APP_VER_NO_C)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_APP_VER_NO_S)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_CPVS2_NO_C)     not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_CPVS2_NO_S)     not referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_CPVS2_XPORT_INV) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_INV_ITSRV2) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SRV_EXISTS) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CLI_EXISTS) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CLI_RPI_INV) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SRV_RPI_INV) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_TUNID_RX_FF) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_FSE_INC_MR_RESP_FWD_OP) not referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_SS_INVNCPTO) n. referenced in every cfg */
/*lint -esym(755, IXSCE_k_NFSE_RXE_SO_CS_INVNCPTO) n. referenced in every cfg */
/* program flow error */
#define IXSCE_k_FSE_INC_PRG_FLOW        IXSCE_ERR_____FAIL_SAFE(0x01U)
/* unit received an unknown cmd */
#define IXSCE_k_NFSE_RXH_CMD_UNKNOWN    IXSCE_ERR_NOT_FAIL_SAFE(0x02U)
/* error sending success resp */
#define IXSCE_k_NFSE_TXH_O_SUCRSP_SEND  IXSCE_ERR_NOT_FAIL_SAFE(0x03U)
/* error sending success resp */
#define IXSCE_k_NFSE_TXH_C_SUCRSP_SEND  IXSCE_ERR_NOT_FAIL_SAFE(0x04U)
/* error sending error response */
#define IXSCE_k_NFSE_TXH_O_ERRRSP_SEND  IXSCE_ERR_NOT_FAIL_SAFE(0x05U)
/* error sending error response */
#define IXSCE_k_NFSE_TXH_C_ERRRSP_SEND  IXSCE_ERR_NOT_FAIL_SAFE(0x06U)
/* API func called in inv state */
#define IXSCE_k_FSE_AIS_STATE_ERROR     IXSCE_ERR_____FAIL_SAFE(0x11U)
/* func called with invalid ptr */
#define IXSCE_k_FSE_AIP_PTR_INV_CMDPROC IXSCE_ERR_____FAIL_SAFE(0x12U)
/* func called with invalid ptr */
#define IXSCE_k_FSE_AIP_PTR_INV_CMDDATA IXSCE_ERR_____FAIL_SAFE(0x13U)
/* func called with invalid ptr */
#define IXSCE_k_FSE_INC_PTR_INV_KEY_PRS IXSCE_ERR_____FAIL_SAFE(0x14U)
/* func called with invalid ptr */
#define IXSCE_k_FSE_INC_PTR_INV_NSS_PRS IXSCE_ERR_____FAIL_SAFE(0x15U)

/* Forward_Close with inv. cnxn */
#define IXSCE_k_NFSE_RXE_SC_INV_CNXN    IXSCE_ERR_NOT_FAIL_SAFE(0x80U)
/* SafetyOp message too short */
#define IXSCE_k_NFSE_RXE_SO_TOO_SHORT   IXSCE_ERR_NOT_FAIL_SAFE(0x83U)
/* SafetyOp app path size error */
#define IXSCE_k_NFSE_RXE_SO_AP_SIZE_1   IXSCE_ERR_NOT_FAIL_SAFE(0x84U)
/* SafetyOp app path size error */
#define IXSCE_k_NFSE_RXE_SO_AP_SIZE_2   IXSCE_ERR_NOT_FAIL_SAFE(0x85U)
/* SafetyOp segments invalid */
#define IXSCE_k_NFSE_RXE_SO_SEG_INV     IXSCE_ERR_NOT_FAIL_SAFE(0x87U)
/* SafetyCl message too short */
#define IXSCE_k_NFSE_RXE_SC_TOO_SHORT   IXSCE_ERR_NOT_FAIL_SAFE(0x88U)
/* SafetyOp invalid class */
#define IXSCE_k_NFSE_RXE_SO_INV_CLASS   IXSCE_ERR_NOT_FAIL_SAFE(0x8DU)
/* SafetyOp invalid instance */
#define IXSCE_k_NFSE_RXE_SO_INV_INST    IXSCE_ERR_NOT_FAIL_SAFE(0x8EU)
/* SafetyOp Safety Segment size */
#define IXSCE_k_NFSE_RXE_SO_SSEG_SIZE   IXSCE_ERR_NOT_FAIL_SAFE(0x91U)
/* SafetyOp CPCRC mismatch */
#define IXSCE_k_NFSE_RXE_SO_CPCRC_MIS   IXSCE_ERR_NOT_FAIL_SAFE(0x92U)
/* SafetyOp recv while TUNID inv */
#define IXSCE_k_NFSE_RXE_SO_NO_TUNID    IXSCE_ERR_NOT_FAIL_SAFE(0x93U)
/* SafetyOp recv while TUNID inv */
#define IXSCE_k_NFSE_RXE_SO_TUNID_MIS   IXSCE_ERR_NOT_FAIL_SAFE(0x94U)
/* duplicate Forward_Open recvd */
#define IXSCE_k_NFSE_RXE_SO_DUPLICATE   IXSCE_ERR_NOT_FAIL_SAFE(0x95U)
/* SafetyOp El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_1    IXSCE_ERR_NOT_FAIL_SAFE(0x96U)
/* SafetyOp El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_2    IXSCE_ERR_NOT_FAIL_SAFE(0x97U)
/* SafetyOp El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_3    IXSCE_ERR_NOT_FAIL_SAFE(0x98U)
/* SafetyOp Mcast Min Len check */
#define IXSCE_k_NFSE_RXE_SO_MCAST_LEN   IXSCE_ERR_NOT_FAIL_SAFE(0x99U)
/* SafetyOp Payload length check */
#define IXSCE_k_NFSE_RXE_SO_INV_LEN     IXSCE_ERR_NOT_FAIL_SAFE(0x9AU)
/* SafetyOp Network Cnxn Params OT */
#define IXSCE_k_NFSE_RXE_SO_SS_INVNCP   IXSCE_ERR_NOT_FAIL_SAFE(0x9BU)
/* SafetyOp Network Cnxn Params */
#define IXSCE_k_NFSE_RXE_SO_CS_INVNCP   IXSCE_ERR_NOT_FAIL_SAFE(0x9CU)
/* SafetyOp Network Cnxn Params */
#define IXSCE_k_NFSE_RXE_SO_CM_INVNCP   IXSCE_ERR_NOT_FAIL_SAFE(0x9DU)
/* SafetyOp Timeout Multiplier */
#define IXSCE_k_NFSE_RXE_SO_INV_TMULT   IXSCE_ERR_NOT_FAIL_SAFE(0x9EU)
/* SafetyOp TransportClass/Trig */
#define IXSCE_k_NFSE_RXE_SO_INV_TCT_1   IXSCE_ERR_NOT_FAIL_SAFE(0x9FU)
/* SafetyOp TransportClass/Trig */
#define IXSCE_k_NFSE_RXE_SO_INV_TCT_2   IXSCE_ERR_NOT_FAIL_SAFE(0xA0U)
/* SafetyOp TransportClass/Trig */
#define IXSCE_k_NFSE_RXE_SO_INV_TCT_3   IXSCE_ERR_NOT_FAIL_SAFE(0xA1U)
/* SafetyOp TimeCorrection EPI */
#define IXSCE_k_NFSE_RXE_SO_INV_TCEPI   IXSCE_ERR_NOT_FAIL_SAFE(0xA2U)
/* SafetyOp TimeCorrection NCP (Server) */
#define IXSCE_k_NFSE_RXE_SO_INV_TCNCP1  IXSCE_ERR_NOT_FAIL_SAFE(0xA3U)
/* SafetyOp PIEM invalid */
#define IXSCE_k_NFSE_RXE_SO_INV_PIEM    IXSCE_ERR_NOT_FAIL_SAFE(0xA4U)
/* SafetyOp P_Cnt_Interval inv */
#define IXSCE_k_NFSE_RXE_SO_INV_PCI     IXSCE_ERR_NOT_FAIL_SAFE(0xA5U)
/* SafetyOp TCOO Min Mult inv */
#define IXSCE_k_NFSE_RXE_SO_INV_TCMMM   IXSCE_ERR_NOT_FAIL_SAFE(0xA6U)
/* SafetyOp NTEM invalid */
#define IXSCE_k_NFSE_RXE_SO_INV_NTEM    IXSCE_ERR_NOT_FAIL_SAFE(0xA7U)
/* SafetyOp Safety ToutMulti inv */
#define IXSCE_k_NFSE_RXE_SO_INV_STMUL   IXSCE_ERR_NOT_FAIL_SAFE(0xA8U)
/* SafetyOp MaxConsNum invalid */
#define IXSCE_k_NFSE_RXE_SO_INV_MCNUM   IXSCE_ERR_NOT_FAIL_SAFE(0xA9U)
/* SafetyOp TimeCorr Cnxn ID inv */
#define IXSCE_k_NFSE_RXE_SO_INV_TCCID   IXSCE_ERR_NOT_FAIL_SAFE(0xAAU)
/* SafetyOp Max Fault Number inv */
#define IXSCE_k_NFSE_RXE_SO_INV_MFNUM   IXSCE_ERR_NOT_FAIL_SAFE(0xABU)
/* SafetyOp Init TS or RV inv */
#define IXSCE_k_NFSE_RXE_SO_INV_ITSRV1  IXSCE_ERR_NOT_FAIL_SAFE(0xACU)
/* SafetyOp ConfigClass invalid */
#define IXSCE_k_NFSE_RXE_SO_INV_CASM    IXSCE_ERR_NOT_FAIL_SAFE(0xADU)
/* SafetyOp ConfigInst NULL */
#define IXSCE_k_NFSE_RXE_SO_CASM_NULL   IXSCE_ERR_NOT_FAIL_SAFE(0xAEU)
/* SafetyOp SrvProdCnxnPt nNull */
#define IXSCE_k_NFSE_RXE_SO_S_PROD_NN   IXSCE_ERR_NOT_FAIL_SAFE(0xB0U)
/* SafetyOp SrvConsCnxnPt inv */
#define IXSCE_k_NFSE_RXE_SO_S_CONS_I    IXSCE_ERR_NOT_FAIL_SAFE(0xB1U)
/* SafetyOp SrvProdCnxnPt nNull */
#define IXSCE_k_NFSE_RXE_SO_C_CONS_NN   IXSCE_ERR_NOT_FAIL_SAFE(0xB2U)
/* SafetyOp SrvConsCnxnPt inv */
#define IXSCE_k_NFSE_RXE_SO_C_PROD_I    IXSCE_ERR_NOT_FAIL_SAFE(0xB3U)
/* SafetyOp denied by SAPL */
#define IXSCE_k_NFSE_RXE_SO_SAPL_DENY   IXSCE_ERR_NOT_FAIL_SAFE(0xB4U)
/* SafetyOp not applied by SAPL */
#define IXSCE_k_NFSE_RXE_SO_SAPL_APPL   IXSCE_ERR_NOT_FAIL_SAFE(0xB5U)
/* SafetyOp store SCID */
#define IXSCE_k_NFSE_RXE_SO_STORE_1     IXSCE_ERR_NOT_FAIL_SAFE(0xB6U)
/* SafetyOp store OCPUNID */
#define IXSCE_k_NFSE_RXE_SO_STORE_2     IXSCE_ERR_NOT_FAIL_SAFE(0xB7U)
/* SafetyOp SSO State Machine (Type 1 SO) */
#define IXSCE_k_NFSE_RXE_SO_SSO_SM_1    IXSCE_ERR_NOT_FAIL_SAFE(0xB8U)
/* SafetyOp Type2 but unconfig */
#define IXSCE_k_NFSE_RXE_SO_UNCONF      IXSCE_ERR_NOT_FAIL_SAFE(0xBAU)
/* SafetyOp SCID mismatch */
#define IXSCE_k_NFSE_RXE_SO_SCID_MIS    IXSCE_ERR_NOT_FAIL_SAFE(0xBBU)
/* SafetyOp OCP not found */
#define IXSCE_k_NFSE_RXE_SO_OCP_NF      IXSCE_ERR_NOT_FAIL_SAFE(0xBCU)
/* SafetyOp OCP owner conflict */
#define IXSCE_k_NFSE_RXE_SO_OCPUNID     IXSCE_ERR_NOT_FAIL_SAFE(0xBDU)
/* SafetyOp SCCRC mismatch */
#define IXSCE_k_NFSE_RXE_SO_SCCRC_MIS   IXSCE_ERR_NOT_FAIL_SAFE(0xBEU)
/* SafetyOp config owned by tool */
#define IXSCE_k_NFSE_RXE_SO_TOOL_OWND   IXSCE_ERR_NOT_FAIL_SAFE(0xBFU)
/* SafetyOp config owner confl */
#define IXSCE_k_NFSE_RXE_SO_CFUNID      IXSCE_ERR_NOT_FAIL_SAFE(0xC0U)
/* SafetyOp SSO State Machine (Apply) */
#define IXSCE_k_NFSE_RXE_SO_SSO_SM_2    IXSCE_ERR_NOT_FAIL_SAFE(0xC5U)
/* SafetyOp TimeCorrection NCP (Single-Cast Client) */
#define IXSCE_k_NFSE_RXE_SO_INV_TCNCP2  IXSCE_ERR_NOT_FAIL_SAFE(0xC6U)
/* SafetyOp TimeCorrection NCP (Multi-Cast Client) */
#define IXSCE_k_NFSE_RXE_SO_INV_TCNCP3  IXSCE_ERR_NOT_FAIL_SAFE(0xC7U)
/* SafetyOp store CFUNID */
#define IXSCE_k_NFSE_RXE_SO_STORE_3     IXSCE_ERR_NOT_FAIL_SAFE(0xC8U)
/* SafetyOp message too long */
#define IXSCE_k_NFSE_RXE_SO_TOO_LONG    IXSCE_ERR_NOT_FAIL_SAFE(0xC9U)
/* Received Base Format FwdOpen, but not supported */
#define IXSCE_k_NFSE_RXE_BASE_NOT_SUP   IXSCE_ERR_NOT_FAIL_SAFE(0xCAU)
/* Received Extended Format FwdOpen, but not supported */
#define IXSCE_k_NFSE_RXE_EXT_NOT_SUP    IXSCE_ERR_NOT_FAIL_SAFE(0xCBU)
/* CnxnSizeCheck() called with multicast although not supported in this cfg */
#define IXSCE_k_FSE_INC_CNXN_SIZE_CHECK IXSCE_ERR_____FAIL_SAFE(0xCDU)
/* detected not supported transport trigger (direction) in App Path verify */
#define IXSCE_k_FSE_INC_APP_VER_NO_C    IXSCE_ERR_____FAIL_SAFE(0xCEU)
/* detected not supported transport trigger (direction) in App Path verify */
#define IXSCE_k_FSE_INC_APP_VER_NO_S    IXSCE_ERR_____FAIL_SAFE(0xCFU)
/* detected not supported transport trigger(dir) in CnxnParamsValidateSafety2 */
#define IXSCE_k_FSE_INC_CPVS2_NO_C      IXSCE_ERR_____FAIL_SAFE(0xD0U)
/* detected not supported transport trigger(dir) in CnxnParamsValidateSafety2 */
#define IXSCE_k_FSE_INC_CPVS2_NO_S      IXSCE_ERR_____FAIL_SAFE(0xD1U)
/* detected invalid transport trigger in CnxnParamsValidateSafety2 */
#define IXSCE_k_FSE_INC_CPVS2_XPORT_INV IXSCE_ERR_____FAIL_SAFE(0xD2U)
/* SafetyOp Init TS or RV inv */
#define IXSCE_k_NFSE_RXE_SO_INV_ITSRV2  IXSCE_ERR_NOT_FAIL_SAFE(0xD3U)
/* SafetyOpen El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_1 IXSCE_ERR_NOT_FAIL_SAFE(0xD4U)
/* SafetyOpen El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_2 IXSCE_ERR_NOT_FAIL_SAFE(0xD5U)
/* SafetyOpen El Key mismatch */
#define IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_3 IXSCE_ERR_NOT_FAIL_SAFE(0xD6U)
/* SafetyOpen: same originator tries to reopen a connection of different type */
#define IXSCE_k_NFSE_RXE_SO_SRV_EXISTS  IXSCE_ERR_NOT_FAIL_SAFE(0xD7U)
/* SafetyOpen: same originator tries to reopen a connection of different type */
#define IXSCE_k_NFSE_RXE_SO_CLI_EXISTS  IXSCE_ERR_NOT_FAIL_SAFE(0xD8U)
/* SafetyOpen: Client RPIs invalid */
#define IXSCE_k_NFSE_RXE_SO_CLI_RPI_INV IXSCE_ERR_NOT_FAIL_SAFE(0xD9U)
/* SafetyOpen: Server RPIs invalid */
#define IXSCE_k_NFSE_RXE_SO_SRV_RPI_INV IXSCE_ERR_NOT_FAIL_SAFE(0xDAU)
/* SafetyOp recv with FF TUNID */
#define IXSCE_k_NFSE_RXE_SO_TUNID_RX_FF IXSCE_ERR_NOT_FAIL_SAFE(0xDBU)
/* Message Router Response structure is inconsistent */
#define IXSCE_k_FSE_INC_MR_RESP_FWD_OP  IXSCE_ERR_____FAIL_SAFE(0xDCU)
/* SafetyOp Network Cnxn Params TO */
#define IXSCE_k_NFSE_RXE_SO_SS_INVNCPTO IXSCE_ERR_NOT_FAIL_SAFE(0xDDU)
/* SafetyOp Network Cnxn Params TO */
#define IXSCE_k_NFSE_RXE_SO_CS_INVNCPTO IXSCE_ERR_NOT_FAIL_SAFE(0xDEU)


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
** Function    : IXSCE_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSCE_k_FSE_xxx and
**                                        IXSCE_k_NFSE_xxx})
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
  CSS_t_INT IXSCE_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                              CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSCEERR_H */

/*** End Of File ***/


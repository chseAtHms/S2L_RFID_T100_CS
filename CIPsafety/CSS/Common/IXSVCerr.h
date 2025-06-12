/*******************************************************************************
**    Copyright (C) 2009-2020 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCerr.h
**    Summary: IXSVC - Safety Validator Client
**             IXSVCerr.h defines the error codes of the IXSVC unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVCERR_H
#define IXSVCERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSVC_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVC_k_NFSE_xxx}
*/
#define IXSVC_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSVC, u8_unitErr)


/** IXSVC_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVC_k_FSE_xxx}
*/
#define IXSVC_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSVC, u8_unitErr)


/** IXSVC_k_FSE_xxx and IXSVC_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSVC_k_NFSE_TXH_HALCS_SEND_OCI) n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_FSE_INC_INST_INV_OCI)   not referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXI_BASE_TCOO_ACK2) n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXI_BASE_TCOO_CRC)  n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_FSE_INC_INST_INV_TCI)   not referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXE_MCASTRC_FORMAT) n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXE_MCASTRC_RPI_OT) n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXE_MCASTRC_MCN)   not referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXE_MCASTRC_PIEM)  not referenced in every cfg */
/*lint -esym(755, IXSVC_k_FSE_INC_TXFG_INST_INV)  not referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXI_EXT_TCOO_CRC)  not referenced in every cfg */
/*lint -esym(755, IXSVC_k_FSE_INC_C_ALLOC_MAXCNUM) n. referenced in every cfg */
/*lint -esym(755, IXSVC_k_FSE_INC_TMULT_PI_CALC)  not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_01)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_02)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_03)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_04)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_05)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_06)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_07)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_08)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_09)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_10)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_11)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_12)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_13)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_14)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_15)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_16)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_17)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_18)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_19)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_20)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_21)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_22)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_23)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_24)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_25)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_26)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_27)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_28)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_29)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_30)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_31)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_32)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_33)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_34)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_35)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_36)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_37)    not referenced in every cfg */
/*lint -esym(755, IXSVC_FSE_INC_MSG_FORMAT_38)    not referenced in every cfg */
/*lint -esym(755, IXSVC_k_NFSE_RXE_MCASTRC_RPI_TO) n. referenced in every cfg */
/* program flow error */
#define IXSVC_k_FSE_INC_PRG_FLOW        IXSVC_ERR_____FAIL_SAFE(0x01U)
/* API call in wrong Stack State */
#define IXSVC_k_FSE_AIS_STATE_ERR_TFG   IXSVC_ERR_____FAIL_SAFE(0x03U)
/* State Machine detected invalid state */
#define IXSVC_k_FSE_INC_SM_INV_STATE    IXSVC_ERR_____FAIL_SAFE(0x04U)
/* division by zero */
#define IXSVC_k_FSE_INC_DIV_BY_ZERO     IXSVC_ERR_____FAIL_SAFE(0x05U)
/* received msg had wrong length */
#define IXSVC_k_NFSE_RXI_MSG_LEN_ERR_C  IXSVC_ERR_NOT_FAIL_SAFE(0x09U)
/* HALCS_TxDataPut returned err */
#define IXSVC_k_NFSE_TXH_HALCS_SEND_CCC IXSVC_ERR_NOT_FAIL_SAFE(0x10U)
/* index invalid when trying to close underlaying CSAL connection */
#define IXSVC_k_FSE_INC_CLOSE_IDX       IXSVC_ERR_____FAIL_SAFE(0x11U)
/* HALCS_TxDataPut returned err */
#define IXSVC_k_NFSE_TXH_HALCS_SEND_OCI IXSVC_ERR_NOT_FAIL_SAFE(0x12U)
/* Originator tries to initialize Connection that was already deleted */
#define IXSVC_k_FSE_INC_INST_INV_OCI    IXSVC_ERR_____FAIL_SAFE(0x13U)
/* invalid consumer number detected while trying to close underlaying cnxn */
#define IXSVC_k_FSE_INC_CLOSE_CONS_NUM  IXSVC_ERR_____FAIL_SAFE(0x14U)
/* HALCS_TxDataPut returned err */
#define IXSVC_k_NFSE_TXH_HALCS_SEND_TFG IXSVC_ERR_NOT_FAIL_SAFE(0x15U)
/* allocation <-> index inconsistency in IXSVC_InstInfoGet() */
#define IXSVC_k_FSE_INC_SIIG_IDX_ALLOC  IXSVC_ERR_____FAIL_SAFE(0x16U)
/* function called with invalid ptr */
#define IXSVC_k_FSE_AIP_PTR_INV_IIG     IXSVC_ERR_____FAIL_SAFE(0x17U)
/* Stack is not in running state */
#define IXSVC_k_FSE_AIS_STATE_ERR_IIG   IXSVC_ERR_____FAIL_SAFE(0x18U)
/* a Time Coordination message was received with Ping_Response bit cleared */
#define IXSVC_k_NFSE_RXI_TCOO_PR_NOTSET IXSVC_ERR_NOT_FAIL_SAFE(0x1DU)
/* instance parameter invalid */
#define IXSVC_k_FSE_INC_INST_CIIG_NUM   IXSVC_ERR_____FAIL_SAFE(0x1EU)
/* ConsumerNum parameter invalid */
#define IXSVC_k_NFSE_RXI_CONS_NUM_CRP   IXSVC_ERR_NOT_FAIL_SAFE(0x1FU)
/* Target tries to initialize Connection that was already deleted */
#define IXSVC_k_FSE_INC_INST_INV_TCI    IXSVC_ERR_____FAIL_SAFE(0x21U)

/* Multicast Reconnect: Format Mismatch */
#define IXSVC_k_NFSE_RXE_MCASTRC_FORMAT IXSVC_ERR_NOT_FAIL_SAFE(0x22U)
/* Multicast Reconnect: RPI Mismatch (O->T) */
#define IXSVC_k_NFSE_RXE_MCASTRC_RPI_OT  IXSVC_ERR_NOT_FAIL_SAFE(0x23U)
/* Multicast Reconnect: Max Consumer Number Mismatch */
#define IXSVC_k_NFSE_RXE_MCASTRC_MCN    IXSVC_ERR_NOT_FAIL_SAFE(0x24U)
/* Multicast Reconnect: Ping Interval EPI Multiplier Mismatch */
#define IXSVC_k_NFSE_RXE_MCASTRC_PIEM   IXSVC_ERR_NOT_FAIL_SAFE(0x25U)

/* Event not applicable in current state */
#define IXSVC_k_FSE_INC_SM_INV_EV_IDLE  IXSVC_ERR_____FAIL_SAFE(0x26U)
#define IXSVC_k_FSE_INC_SM_INV_EV_INIT  IXSVC_ERR_____FAIL_SAFE(0x27U)
#define IXSVC_k_FSE_INC_SM_INV_EV_ESTAB IXSVC_ERR_____FAIL_SAFE(0x28U)
#define IXSVC_k_FSE_INC_SM_INV_EV_FAIL  IXSVC_ERR_____FAIL_SAFE(0x29U)

/* Invalid Consumer Index passed to IXSVC_NetTimeExpMultGet() */
#define IXSVC_k_FSE_INC_NTEM_GET_INV_CI IXSVC_ERR_____FAIL_SAFE(0x2AU)
/* Invalid value of timeout multiplier in Originator Client initialization */
#define IXSVC_k_FSE_INC_TMULT_PI_CALC   IXSVC_ERR_____FAIL_SAFE(0x2BU)
/* SV Instance ID invalid in IXSVC_TxFrameGenerate() */
#define IXSVC_k_FSE_INC_TXFG_INST_INV   IXSVC_ERR_____FAIL_SAFE(0x2CU)
/* invalid value in max consumer number detected in ConsIdxAlloc() */
#define IXSVC_k_FSE_INC_C_ALLOC_MAXCNUM IXSVC_ERR_____FAIL_SAFE(0x2DU)
/* while writing Tx message detected that too much data were written */
#define IXSVC_k_FSE_INC_TX_GEN_TOO_MUCH IXSVC_ERR_____FAIL_SAFE(0x2EU)
/* tried to write an invalid value to u8_RR_Con_Num_Index_Pntr */
#define IXSVC_k_FSE_INC_CON_NUM_IDX_PTR IXSVC_ERR_____FAIL_SAFE(0x2FU)
/* Time Coordination Message not received in allotted time (Single-cast) */
#define IXSVC_k_NFSE_RXI_TCOO_TO_SC     IXSVC_ERR_NOT_FAIL_SAFE(0x30U)
/* Time Coordination Message not received in allotted time (Multi-cast) */
#define IXSVC_k_NFSE_RXI_TCOO_TO_MC     IXSVC_ERR_NOT_FAIL_SAFE(0x31U)
/* Time Coordination Message from this consumer with duplicate Time stamp */
#define IXSVC_k_NFSE_RXI_TCOO_TS_DUP    IXSVC_ERR_NOT_FAIL_SAFE(0x32U)
/* Time Coordination Message Parity Error */
#define IXSVC_k_NFSE_RXI_TCOO_PARITY    IXSVC_ERR_NOT_FAIL_SAFE(0x33U)
/* Time Coordination Message Base Format Acknowledge Byte 2 error */
#define IXSVC_k_NFSE_RXI_BASE_TCOO_ACK2 IXSVC_ERR_NOT_FAIL_SAFE(0x34U)
/* Time Coordination Message not received in 5 seconds limit */
#define IXSVC_k_NFSE_RXI_TCOO_5S_LIMIT  IXSVC_ERR_NOT_FAIL_SAFE(0x35U)
/* Time Coordination message had a ping count error */
#define IXSVC_k_NFSE_RXI_TCOO_PC_ERR    IXSVC_ERR_NOT_FAIL_SAFE(0x36U)
/* Time Coordination Message Base Format CRC error */
#define IXSVC_k_NFSE_RXI_BASE_TCOO_CRC  IXSVC_ERR_NOT_FAIL_SAFE(0x37U)
/* Time Coordination Message Extended Format CRC error */
#define IXSVC_k_NFSE_RXI_EXT_TCOO_CRC   IXSVC_ERR_NOT_FAIL_SAFE(0x38U)
/* detected an invalid EPI value when setting IXSVC init values */
#define IXSVC_k_FSE_INC_EPI_RANGE       IXSVC_ERR_____FAIL_SAFE(0x39U)
/* detected an invalid data size when setting IXSVC init values */
#define IXSVC_k_FSE_INC_DATA_SIZE_RANGE IXSVC_ERR_____FAIL_SAFE(0x3AU)
/* detected an invalid maxConsNum when setting u8_RR_Con_Num_Index_Pntr */
#define IXSVC_k_FSE_INC_MAX_CONS_NUM_CI IXSVC_ERR_____FAIL_SAFE(0x3BU)
/* detected an out of range value for Time_Drift_Per_Ping_Interval */
#define IXSVC_k_FSE_INC_TDPPI_RANGE     IXSVC_ERR_____FAIL_SAFE(0x3CU)
/* detected an out of range value for Time_Drift_Constant */
#define IXSVC_k_FSE_T_DRIFT_CONST_RANGE IXSVC_ERR_____FAIL_SAFE(0x3DU)
/* detected an out of range value for Time_Coordination_ResponseEPI_Limit */
#define IXSVC_k_FSE_TCREL_OUT_OF_RANGE  IXSVC_ERR_____FAIL_SAFE(0x3EU)
/* detected an out of range value for Time_Coord_Msg_Min_Multiplier */
#define IXSVC_k_FSE_TCOOMMM_RANGE       IXSVC_ERR_____FAIL_SAFE(0x3FU)

/* several errors caused by b_msgFormat indicating an unsupported format */
#define IXSVC_FSE_INC_MSG_FORMAT_01     IXSVC_ERR_____FAIL_SAFE(0x40U)
#define IXSVC_FSE_INC_MSG_FORMAT_02     IXSVC_ERR_____FAIL_SAFE(0x41U)
#define IXSVC_FSE_INC_MSG_FORMAT_03     IXSVC_ERR_____FAIL_SAFE(0x42U)
#define IXSVC_FSE_INC_MSG_FORMAT_04     IXSVC_ERR_____FAIL_SAFE(0x43U)
#define IXSVC_FSE_INC_MSG_FORMAT_05     IXSVC_ERR_____FAIL_SAFE(0x44U)
#define IXSVC_FSE_INC_MSG_FORMAT_06     IXSVC_ERR_____FAIL_SAFE(0x45U)
#define IXSVC_FSE_INC_MSG_FORMAT_07     IXSVC_ERR_____FAIL_SAFE(0x46U)
#define IXSVC_FSE_INC_MSG_FORMAT_08     IXSVC_ERR_____FAIL_SAFE(0x47U)
#define IXSVC_FSE_INC_MSG_FORMAT_09     IXSVC_ERR_____FAIL_SAFE(0x48U)
#define IXSVC_FSE_INC_MSG_FORMAT_10     IXSVC_ERR_____FAIL_SAFE(0x49U)
#define IXSVC_FSE_INC_MSG_FORMAT_11     IXSVC_ERR_____FAIL_SAFE(0x4AU)
#define IXSVC_FSE_INC_MSG_FORMAT_12     IXSVC_ERR_____FAIL_SAFE(0x4BU)
#define IXSVC_FSE_INC_MSG_FORMAT_13     IXSVC_ERR_____FAIL_SAFE(0x4CU)
#define IXSVC_FSE_INC_MSG_FORMAT_14     IXSVC_ERR_____FAIL_SAFE(0x4DU)
#define IXSVC_FSE_INC_MSG_FORMAT_15     IXSVC_ERR_____FAIL_SAFE(0x4EU)
#define IXSVC_FSE_INC_MSG_FORMAT_16     IXSVC_ERR_____FAIL_SAFE(0x4FU)
#define IXSVC_FSE_INC_MSG_FORMAT_17     IXSVC_ERR_____FAIL_SAFE(0x50U)
#define IXSVC_FSE_INC_MSG_FORMAT_18     IXSVC_ERR_____FAIL_SAFE(0x51U)
#define IXSVC_FSE_INC_MSG_FORMAT_19     IXSVC_ERR_____FAIL_SAFE(0x52U)
#define IXSVC_FSE_INC_MSG_FORMAT_20     IXSVC_ERR_____FAIL_SAFE(0x53U)
#define IXSVC_FSE_INC_MSG_FORMAT_21     IXSVC_ERR_____FAIL_SAFE(0x54U)
#define IXSVC_FSE_INC_MSG_FORMAT_22     IXSVC_ERR_____FAIL_SAFE(0x55U)
#define IXSVC_FSE_INC_MSG_FORMAT_23     IXSVC_ERR_____FAIL_SAFE(0x56U)
#define IXSVC_FSE_INC_MSG_FORMAT_24     IXSVC_ERR_____FAIL_SAFE(0x57U)
#define IXSVC_FSE_INC_MSG_FORMAT_25     IXSVC_ERR_____FAIL_SAFE(0x58U)
#define IXSVC_FSE_INC_MSG_FORMAT_26     IXSVC_ERR_____FAIL_SAFE(0x59U)
#define IXSVC_FSE_INC_MSG_FORMAT_27     IXSVC_ERR_____FAIL_SAFE(0x5AU)
#define IXSVC_FSE_INC_MSG_FORMAT_28     IXSVC_ERR_____FAIL_SAFE(0x5BU)
#define IXSVC_FSE_INC_MSG_FORMAT_29     IXSVC_ERR_____FAIL_SAFE(0x5CU)
#define IXSVC_FSE_INC_MSG_FORMAT_30     IXSVC_ERR_____FAIL_SAFE(0x5DU)
#define IXSVC_FSE_INC_MSG_FORMAT_31     IXSVC_ERR_____FAIL_SAFE(0x5EU)
#define IXSVC_FSE_INC_MSG_FORMAT_32     IXSVC_ERR_____FAIL_SAFE(0x5FU)
#define IXSVC_FSE_INC_MSG_FORMAT_33     IXSVC_ERR_____FAIL_SAFE(0x60U)
#define IXSVC_FSE_INC_MSG_FORMAT_34     IXSVC_ERR_____FAIL_SAFE(0x61U)
#define IXSVC_FSE_INC_MSG_FORMAT_35     IXSVC_ERR_____FAIL_SAFE(0x62U)
#define IXSVC_FSE_INC_MSG_FORMAT_36     IXSVC_ERR_____FAIL_SAFE(0x63U)
#define IXSVC_FSE_INC_MSG_FORMAT_37     IXSVC_ERR_____FAIL_SAFE(0x64U)
#define IXSVC_FSE_INC_MSG_FORMAT_38     IXSVC_ERR_____FAIL_SAFE(0x65U)

/* Multicast Reconnect: RPI Mismatch (T->O) */
#define IXSVC_k_NFSE_RXE_MCASTRC_RPI_TO IXSVC_ERR_NOT_FAIL_SAFE(0x66U)


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
** Function    : IXSVC_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVC_k_FSE_xxx})
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
  CSS_t_INT IXSVC_ErrorStrGet(CSS_t_WORD w_errorCode,
                               CSS_t_UINT u16_instId,
                               CSS_t_DWORD dw_addInfo,
                               CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                               CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSVCERR_H */

/*** End Of File ***/


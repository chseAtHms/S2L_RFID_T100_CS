/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSerr.h
**    Summary: IXSVS - Safety Validator Server
**             IXSVSerr.h defines the error codes of the IXSVS unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVSERR_H
#define IXSVSERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSVS_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVS_k_NFSE_xxx}
*/
#define IXSVS_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXSVS, u8_unitErr)


/** IXSVS_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXSVS_k_FSE_xxx}
*/
#define IXSVS_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXSVS, u8_unitErr)


/** IXSVS_k_FSE_xxx and IXSVS_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXSVS_k_FSE_INC_INST_INV_OSI)   not referenced in every cfg */
/*lint -esym(755, IXSVS_k_FSE_INC_INST_SCOM_NUM)  not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_TXH_HALCS_SEND_OSI) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_FSE_INC_INST_INV_TSI)   not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_LONG_ACRC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_LONG_AVSC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_LONG_CCRC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_LONG_RBMB) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_SHRT_ACRC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_SHRT_CCRC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_SHRT_RBMB) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_TCORR_MC2) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_BASE_TCORR_CRC) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_LONG_ACRC) not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_LONG_AVSC) not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_LONG_CCRC) not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_LONG_RBMB) not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_TSTAMP_CRC)    not referenced in every cfg */
/*lint -esym(755, IXSVS_k_FSE_INC_SD_RECV_LT)     not referenced in every cfg */
/*lint -esym(755, IXSVS_k_FSE_INC_SM_EST_FAIL)    not referenced in every cfg */
/*lint -esym(755, IXSVS_k_FSE_INC_SM_EST_CLOSE)   not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_SHORT_CRC) not referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_SHORT_RBMB) n. referenced in every cfg */
/*lint -esym(755, IXSVS_k_NFSE_RXI_EXT_TCORR_CRC) not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_05)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_06)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_07)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_08)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_09)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_10)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_11)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_12)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_13)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_14)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_15)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_16)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_20)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_21)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_24)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_25)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_26)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_27)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_28)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_29)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_30)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_31)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_32)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_33)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_34)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_35)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_36)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_37)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_38)    not referenced in every cfg */
/*lint -esym(755, IXSVS_FSE_INC_MSG_FORMAT_39)    not referenced in every cfg */
/* program flow error */
#define IXSVS_k_FSE_INC_PRG_FLOW        IXSVS_ERR_____FAIL_SAFE(0x01U)
/* API call in wrong Stack State */
#define IXSVS_k_FSE_AIS_STATE_ERR_CAM   IXSVS_ERR_____FAIL_SAFE(0x02U)
/* State Machine detected invalid state */
#define IXSVS_k_FSE_INC_SM_INV_STATE    IXSVS_ERR_____FAIL_SAFE(0x05U)
/* Timeout Multiplier is out of range */
#define IXSVS_k_FSE_INC_TMULT_INV_RANGE IXSVS_ERR_____FAIL_SAFE(0x06U)
/* Invalid Combination of Flags */
#define IXSVS_k_FSE_INC_INV_FLAG_COMB   IXSVS_ERR_____FAIL_SAFE(0x07U)
/* received msg had wrong length */
#define IXSVS_k_NFSE_RXI_MSG_LEN_ERR_S  IXSVS_ERR_NOT_FAIL_SAFE(0x08U)
/* HALCS_TxDataPut returned err */
#define IXSVS_k_NFSE_TXH_HALCS_SEND_CCC IXSVS_ERR_NOT_FAIL_SAFE(0x09U)
/* detected an invalid consumer number when closing a connection */
#define IXSVS_k_FSE_INC_CLOSE_CONS_NUM  IXSVS_ERR_____FAIL_SAFE(0x10U)
/* HALCS_TxDataPut returned err */
#define IXSVS_k_NFSE_TXH_HALCS_SEND_OSI IXSVS_ERR_NOT_FAIL_SAFE(0x11U)
/* HALCS_TxDataPut returned err */
#define IXSVS_k_NFSE_TXH_HALCS_SEND_STS IXSVS_ERR_NOT_FAIL_SAFE(0x12U)
/* instance parameter invalid */
#define IXSVS_k_FSE_INC_INST_SRX_NUM    IXSVS_ERR_____FAIL_SAFE(0x13U)
/* instance parameter invalid */
#define IXSVS_k_FSE_INC_INST_SIIG_NUM   IXSVS_ERR_____FAIL_SAFE(0x14U)
/* Originator tries to initialize Connection that was already deleted */
#define IXSVS_k_FSE_INC_INST_INV_OSI    IXSVS_ERR_____FAIL_SAFE(0x15U)
/* Target tries to initialize Connection that was already deleted */
#define IXSVS_k_FSE_INC_INST_INV_TSI    IXSVS_ERR_____FAIL_SAFE(0x16U)
/* detected an invalid Safety Validator Index when closing a connection */
#define IXSVS_k_FSE_INC_CLOSE_IDX       IXSVS_ERR_____FAIL_SAFE(0x17U)
/* index parameter invalid */
#define IXSVS_k_FSE_INC_IDX_IS_NUM      IXSVS_ERR_____FAIL_SAFE(0x18U)
/* instance parameter invalid */
#define IXSVS_k_FSE_INC_INST_SCOM_NUM   IXSVS_ERR_____FAIL_SAFE(0x19U)
/* Event not applicable in current state */
#define IXSVS_k_FSE_INC_SM_INV_EV_IDLE  IXSVS_ERR_____FAIL_SAFE(0x1AU)
#define IXSVS_k_FSE_INC_SM_INV_EV_INIT  IXSVS_ERR_____FAIL_SAFE(0x1BU)
#define IXSVS_k_FSE_INC_SM_INV_EV_ESTAB IXSVS_ERR_____FAIL_SAFE(0x1CU)
#define IXSVS_k_FSE_INC_SM_INV_EV_FAIL  IXSVS_ERR_____FAIL_SAFE(0x1DU)
/* allocation <-> index inconsistency in IXSVS_InstInfoGet() */
#define IXSVS_k_FSE_INC_SIIG_IDX_ALLOC  IXSVS_ERR_____FAIL_SAFE(0x1EU)

/* Timestamp CRC mismatch */
#define IXSVS_k_NFSE_RXI_TSTAMP_CRC     IXSVS_ERR_NOT_FAIL_SAFE(0x20U)
/* Timestamp delta is zero (message repetition) */
#define IXSVS_k_NFSE_RXI_TSTAMP_DELTA   IXSVS_ERR_NOT_FAIL_SAFE(0x21U)
/* Timestamp delta larger than NTEM */
#define IXSVS_k_NFSE_RXI_TSTAMP_NTEM    IXSVS_ERR_NOT_FAIL_SAFE(0x22U)
/* Data age too old (of a faulted Extended Format message) */
#define IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG IXSVS_ERR_NOT_FAIL_SAFE(0x23U)
/* Data age too old (of a correct message) */
#define IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG IXSVS_ERR_NOT_FAIL_SAFE(0x24U)
/* Base Format Short Actual Data CRC Error */
#define IXSVS_k_NFSE_RXI_BASE_SHRT_ACRC IXSVS_ERR_NOT_FAIL_SAFE(0x25U)
/* Base Format Short Complemented Data CRC Error */
#define IXSVS_k_NFSE_RXI_BASE_SHRT_CCRC IXSVS_ERR_NOT_FAIL_SAFE(0x26U)
/* Base Format Short Redundant Bits in Mode Byte Error */
#define IXSVS_k_NFSE_RXI_BASE_SHRT_RBMB IXSVS_ERR_NOT_FAIL_SAFE(0x27U)
/* Base Format Long Actual Data CRC Error */
#define IXSVS_k_NFSE_RXI_BASE_LONG_ACRC IXSVS_ERR_NOT_FAIL_SAFE(0x28U)
/* Base Format Long Complemented Data CRC Error */
#define IXSVS_k_NFSE_RXI_BASE_LONG_CCRC IXSVS_ERR_NOT_FAIL_SAFE(0x29U)
/* Base Format Long Redundant Bits in Mode Byte Error */
#define IXSVS_k_NFSE_RXI_BASE_LONG_RBMB IXSVS_ERR_NOT_FAIL_SAFE(0x2AU)
/* Base Format Long Actual vs Complement Data check Error */
#define IXSVS_k_NFSE_RXI_BASE_LONG_AVSC IXSVS_ERR_NOT_FAIL_SAFE(0x2BU)
/* Extended Format Short CRC Error */
#define IXSVS_k_NFSE_RXI_EXT_SHORT_CRC  IXSVS_ERR_NOT_FAIL_SAFE(0x2CU)
/* Extended Format Short Redundant Bits in Mode Byte Error */
#define IXSVS_k_NFSE_RXI_EXT_SHORT_RBMB IXSVS_ERR_NOT_FAIL_SAFE(0x2DU)
/* Extended Format Long Actual Data CRC Error */
#define IXSVS_k_NFSE_RXI_EXT_LONG_ACRC  IXSVS_ERR_NOT_FAIL_SAFE(0x2EU)
/* Extended Format Long Complemented Data CRC Error */
#define IXSVS_k_NFSE_RXI_EXT_LONG_CCRC  IXSVS_ERR_NOT_FAIL_SAFE(0x2FU)
/* Extended Format Long Redundant Bits in Mode Byte Error */
#define IXSVS_k_NFSE_RXI_EXT_LONG_RBMB  IXSVS_ERR_NOT_FAIL_SAFE(0x30U)
/* Extended Format Long Actual vs Complement Data check Error */
#define IXSVS_k_NFSE_RXI_EXT_LONG_AVSC  IXSVS_ERR_NOT_FAIL_SAFE(0x31U)
/* Consumer Activity Monitor Error */
#define IXSVS_k_NFSE_RXI_ACT_MON        IXSVS_ERR_NOT_FAIL_SAFE(0x32U)
/* Time Correction Message not received in time */
#define IXSVS_k_NFSE_RXI_TCORR_NOT_PI   IXSVS_ERR_NOT_FAIL_SAFE(0x33U)
/* Data Cross Check Error */
#define IXSVS_k_NFSE_RXI_CROSS_CHECK    IXSVS_ERR_NOT_FAIL_SAFE(0x34U)
/* Time Correction Message Parity Error */
#define IXSVS_k_NFSE_RXI_TCORR_PARITY   IXSVS_ERR_NOT_FAIL_SAFE(0x35U)
/* Time Correction Message Base Format Multicast Byte 2 error */
#define IXSVS_k_NFSE_RXI_BASE_TCORR_MC2 IXSVS_ERR_NOT_FAIL_SAFE(0x36U)
/* Time Correction Message Multicast Active/Idle transitioned to Idle */
#define IXSVS_k_NFSE_RXI_TCORR_AI_FLAG  IXSVS_ERR_NOT_FAIL_SAFE(0x37U)
/* Time Correction Message Base Format CRC error */
#define IXSVS_k_NFSE_RXI_BASE_TCORR_CRC IXSVS_ERR_NOT_FAIL_SAFE(0x38U)
/* Time Correction Message Extended Format CRC error */
#define IXSVS_k_NFSE_RXI_EXT_TCORR_CRC  IXSVS_ERR_NOT_FAIL_SAFE(0x39U)
/* Initialization process of the connection not completed within limit */
#define IXSVS_k_NFSE_RXI_INIT_LIMIT     IXSVS_ERR_NOT_FAIL_SAFE(0x3AU)
/* Invalid Instance for Safety Data Reception Link Triggered */
#define IXSVS_k_FSE_INC_SD_RECV_LT      IXSVS_ERR_____FAIL_SAFE(0x3CU)
/* function called with invalid ptr */
#define IXSVS_k_FSE_AIP_PTR_INV_IIG     IXSVS_ERR_____FAIL_SAFE(0x3DU)
/* Stack is not in running state */
#define IXSVS_k_FSE_AIS_STATE_ERR_IIG   IXSVS_ERR_____FAIL_SAFE(0x3EU)
/* Invalid Instance for State Machine in Connection Faulted */
#define IXSVS_k_FSE_INC_SM_EST_FAIL     IXSVS_ERR_____FAIL_SAFE(0x3FU)
/* Invalid Instance for State Machine in Connection Faulted */
#define IXSVS_k_FSE_INC_SM_EST_CLOSE    IXSVS_ERR_____FAIL_SAFE(0x40U)


/* several errors caused by b_msgFormat indicating an unsupported format */
#define IXSVS_FSE_INC_MSG_FORMAT_05     IXSVS_ERR_____FAIL_SAFE(0x44U)
#define IXSVS_FSE_INC_MSG_FORMAT_06     IXSVS_ERR_____FAIL_SAFE(0x45U)
#define IXSVS_FSE_INC_MSG_FORMAT_07     IXSVS_ERR_____FAIL_SAFE(0x46U)
#define IXSVS_FSE_INC_MSG_FORMAT_08     IXSVS_ERR_____FAIL_SAFE(0x47U)
#define IXSVS_FSE_INC_MSG_FORMAT_09     IXSVS_ERR_____FAIL_SAFE(0x48U)
#define IXSVS_FSE_INC_MSG_FORMAT_10     IXSVS_ERR_____FAIL_SAFE(0x49U)
#define IXSVS_FSE_INC_MSG_FORMAT_11     IXSVS_ERR_____FAIL_SAFE(0x4AU)
#define IXSVS_FSE_INC_MSG_FORMAT_12     IXSVS_ERR_____FAIL_SAFE(0x4BU)
#define IXSVS_FSE_INC_MSG_FORMAT_13     IXSVS_ERR_____FAIL_SAFE(0x4CU)
#define IXSVS_FSE_INC_MSG_FORMAT_14     IXSVS_ERR_____FAIL_SAFE(0x4DU)
#define IXSVS_FSE_INC_MSG_FORMAT_15     IXSVS_ERR_____FAIL_SAFE(0x4EU)
#define IXSVS_FSE_INC_MSG_FORMAT_16     IXSVS_ERR_____FAIL_SAFE(0x4FU)
#define IXSVS_FSE_INC_MSG_FORMAT_20     IXSVS_ERR_____FAIL_SAFE(0x53U)
#define IXSVS_FSE_INC_MSG_FORMAT_21     IXSVS_ERR_____FAIL_SAFE(0x54U)
#define IXSVS_FSE_INC_MSG_FORMAT_24     IXSVS_ERR_____FAIL_SAFE(0x57U)
#define IXSVS_FSE_INC_MSG_FORMAT_25     IXSVS_ERR_____FAIL_SAFE(0x58U)
#define IXSVS_FSE_INC_MSG_FORMAT_26     IXSVS_ERR_____FAIL_SAFE(0x59U)
#define IXSVS_FSE_INC_MSG_FORMAT_27     IXSVS_ERR_____FAIL_SAFE(0x5AU)
#define IXSVS_FSE_INC_MSG_FORMAT_28     IXSVS_ERR_____FAIL_SAFE(0x5BU)
#define IXSVS_FSE_INC_MSG_FORMAT_29     IXSVS_ERR_____FAIL_SAFE(0x5CU)
#define IXSVS_FSE_INC_MSG_FORMAT_30     IXSVS_ERR_____FAIL_SAFE(0x5EU)
#define IXSVS_FSE_INC_MSG_FORMAT_31     IXSVS_ERR_____FAIL_SAFE(0x5FU)
#define IXSVS_FSE_INC_MSG_FORMAT_32     IXSVS_ERR_____FAIL_SAFE(0x60U)
#define IXSVS_FSE_INC_MSG_FORMAT_33     IXSVS_ERR_____FAIL_SAFE(0x61U)
#define IXSVS_FSE_INC_MSG_FORMAT_34     IXSVS_ERR_____FAIL_SAFE(0x62U)
#define IXSVS_FSE_INC_MSG_FORMAT_35     IXSVS_ERR_____FAIL_SAFE(0x63U)
#define IXSVS_FSE_INC_MSG_FORMAT_36     IXSVS_ERR_____FAIL_SAFE(0x64U)
#define IXSVS_FSE_INC_MSG_FORMAT_37     IXSVS_ERR_____FAIL_SAFE(0x65U)
#define IXSVS_FSE_INC_MSG_FORMAT_38     IXSVS_ERR_____FAIL_SAFE(0x66U)
#define IXSVS_FSE_INC_MSG_FORMAT_39     IXSVS_ERR_____FAIL_SAFE(0x67U)


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
** Function    : IXSVS_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVO_k_UERR_xxx})
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
  CSS_t_INT IXSVS_ErrorStrGet(CSS_t_WORD w_errorCode,
                               CSS_t_UINT u16_instId,
                               CSS_t_DWORD dw_addInfo,
                               CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                               CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXSVSERR_H */

/*** End Of File ***/


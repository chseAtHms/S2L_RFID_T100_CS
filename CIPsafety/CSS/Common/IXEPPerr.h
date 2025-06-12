/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPPerr.h
**    Summary: IXEPP - Encoded Path Parser
**             IXEPPerr.h defines the error codes of the IXEPP unit.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXEPPERR_H
#define IXEPPERR_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXEPP_ERR_NOT_FAIL_SAFE:
    This macro assembles a not fail safe error code according to error coding
    definition (see IXSERapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXEPP_k_NFSE_xxx}
*/
#define IXEPP_ERR_NOT_FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_NOT_FS, CSOS_k_CSS_IXEPP, u8_unitErr)

/** IXEPP_ERR_____FAIL_SAFE:
    This macro assembles a FAIL SAFE error code according to error coding
    definition (see IXERRapi.h).
    u8_unitErr (IN) - unique error in the unit, see {IXEPP_k_FSE_xxx}
*/
#define IXEPP_ERR_____FAIL_SAFE(u8_unitErr) \
              IXSER_ErrCodeAsm(IXSER_TYPE_FS, CSOS_k_CSS_IXEPP, u8_unitErr)


/** IXEPP_k_FSE_xxx and IXEPP_k_NFSE_xxx:
    Unit Errors of this module.
*/
/*lint -esym(755, IXEPP_k_FSE_INC_PRG_FLOW)       not referenced in every cfg */
/* program flow error */
#define IXEPP_k_FSE_INC_PRG_FLOW        IXEPP_ERR_____FAIL_SAFE(0x01U)
/* func called with invalid ptr */
#define IXEPP_k_FSE_INC_PTR_INV_EPPARSE IXEPP_ERR_____FAIL_SAFE(0x03U)
/* unknown segment type found */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX1     IXEPP_ERR_NOT_FAIL_SAFE(0x80U)
/* unknown electr. key format */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX3     IXEPP_ERR_NOT_FAIL_SAFE(0x81U)
/* unknown 8-Bit Logical Seg */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX4     IXEPP_ERR_NOT_FAIL_SAFE(0x82U)
/* unknown 16 or 32-Bit LogSeg */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX5     IXEPP_ERR_NOT_FAIL_SAFE(0x83U)
/* unknown Data Segment type */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX6     IXEPP_ERR_NOT_FAIL_SAFE(0x84U)
/* unknown Network Seg type */
#define IXEPP_k_NFSE_RXE_EP_SYNTAX7     IXEPP_ERR_NOT_FAIL_SAFE(0x85U)
/* incomplete segment detected */
#define IXEPP_k_NFSE_RXE_EP_TOO_SMALL   IXEPP_ERR_NOT_FAIL_SAFE(0x86U)
/* too many attr IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_ATTR1   IXEPP_ERR_NOT_FAIL_SAFE(0x87U)
/* too many attr IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_ATTR2   IXEPP_ERR_NOT_FAIL_SAFE(0x88U)
/* more than one electronic key */
#define IXEPP_k_NFSE_RXE_EP_REP_EKEY    IXEPP_ERR_NOT_FAIL_SAFE(0x89U)
/* more than one data segment */
#define IXEPP_k_NFSE_RXE_EP_REP_DATA    IXEPP_ERR_NOT_FAIL_SAFE(0x8AU)
/* more than one Safety segment */
#define IXEPP_k_NFSE_RXE_EP_REP_SNTWK   IXEPP_ERR_NOT_FAIL_SAFE(0x8BU)
/* too many Class IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_CLASS1  IXEPP_ERR_NOT_FAIL_SAFE(0x8CU)
/* too many Class IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_CLASS2  IXEPP_ERR_NOT_FAIL_SAFE(0x8DU)
/* too many Instance IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_INST1   IXEPP_ERR_NOT_FAIL_SAFE(0x8EU)
/* too many Instance IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_INST2   IXEPP_ERR_NOT_FAIL_SAFE(0x8FU)
/* too many Instance IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_INST3   IXEPP_ERR_NOT_FAIL_SAFE(0x90U)
/* too many Cnxn Points found */
#define IXEPP_k_NFSE_RXE_EP_REP_CP1     IXEPP_ERR_NOT_FAIL_SAFE(0x91U)
/* too many Cnxn Points found */
#define IXEPP_k_NFSE_RXE_EP_REP_CP2     IXEPP_ERR_NOT_FAIL_SAFE(0x92U)
/* too many Cnxn Points found */
#define IXEPP_k_NFSE_RXE_EP_REP_CP3     IXEPP_ERR_NOT_FAIL_SAFE(0x93U)
/* too many Member IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_MEMB1   IXEPP_ERR_NOT_FAIL_SAFE(0x94U)
/* too many Member IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_MEMB2   IXEPP_ERR_NOT_FAIL_SAFE(0x95U)
/* too many Member IDs found */
#define IXEPP_k_NFSE_RXE_EP_REP_MEMB3   IXEPP_ERR_NOT_FAIL_SAFE(0x96U)


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
** Function    : IXEPP_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXEPP_k_FSE_xxx and
**                                        IXEPP_k_NFSE_xxx})
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
  CSS_t_INT IXEPP_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                              CSS_t_SNPRINTF_CHAR *pc_errStr);
#endif


#endif /* #ifndef IXEPPERR_H */

/*** End of File ***/


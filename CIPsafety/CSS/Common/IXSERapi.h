/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSERapi.h
**    Summary: IXSER - Safety Error Unit
**             Interface definition of the IXSER unit of the CSS.
**             This unit is responsible for the error management (see
**             Req.10.4.1-1). It contains a callback function that informs the
**             application about errors that occurred in the stack and also
**             macros to get more information about the reported error codes.
**             IXSERapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: SAPL_CssErrorClbk
**             IXSER_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSER_API_H
#define IXSER_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** error coding:
    This table shows the bit coding of the 16 bit error code of the CSOS Stack.
    It includes error type, error layer (CSS/CSAL), unit that reports the error
    and the unit error::

            +---------++---------+----------+--------+------------------+
            | bit pos ||  15-14  |    13    | 12-08  |      07-00       |
            +---------++---------+----------+--------+------------------+
            | meaning || errType | CSS/CSAL | unitId |    unitErr       |
            +---------++---------+----------+--------+------------------+

    Although errors of the CSS are not relevant within the CSAL and vice versa
    there is a bit to distinguish the error layer. This is to make sure error
    codes are unique throughout the whole system.

    Subsequently macros are provided to decode error type and error layer.
*/

/** IXSER_ErrCodeAsm:
    This macro assembles an error code. An error contains the error type, error
    class, the unit id (see ) and the unit error.
*/
#define IXSER_ErrCodeAsm(u8_errType, u8_unitId, u8_unitErr)                    \
          ((CSS_t_UINT)((CSS_t_UINT)((CSS_t_UINT)(u8_errType)     << 14) |     \
                         (CSS_t_UINT)((CSS_t_UINT)(IXSER_LAYER_CSS) << 13) |   \
                         (CSS_t_UINT)((CSS_t_UINT)(u8_unitId)      <<  8) |    \
                         (CSS_t_UINT)(u8_unitErr)))

/* Each unit will specify its error codes in an own header XXXerr.h */


/** IXSER_GET_ERROR_TYPE:
    This macro returns 2 bit "error type" encoded within the 16 bit error code.
*/
/*lint -esym(755, IXSER_GET_ERROR_TYPE) macro not referenced: usage of this
                                        macro by SAPL is optional */
#define IXSER_GET_ERROR_TYPE(w_errorCode)  ((CSS_t_WORD)(w_errorCode) >> 14)


/** error type:
    These symbols represent the type of an error. The CSOS Stack differs two
    error types:
      - fail safe: a fatal error occurred within the CIP Safety Stack which
        cannot be handled by the CIP Safety Stack. The application must switch
        the device into the fail safe state.
      - not fail safe: a minor error or an information occurred within the CSS
        or CSAL Stack which can be handled by the Stack.
    The encoding of error type guarantees that fail safe-errors and non-fail-
    safe errors differ by at least 2 bits (see Req.10.4.1-5).
*/
#define IXSER_TYPE_FS             3U  /* error type "fail safe" */
#define IXSER_TYPE_NOT_FS         0U  /* error type NOT "fail safe" */


/** IXSER_GET_ERROR_LAYER:
    This macro returns 1 bit "error layer" encoded within the 16 bit error
    code. 1 == CSS, 0 == CSAL
*/
/*lint -esym(755, IXSER_GET_ERROR_LAYER) macro not referenced: usage of this
                                         macro by SAPL is optional */
#define IXSER_GET_ERROR_LAYER(w_errorCode) \
        ((((CSS_t_WORD)(w_errorCode)) & (CSS_t_WORD)(0x2000)) >> 13)


/** error layer:
    These symbols represent the layer in which the error occurred.
*/
#define IXSER_LAYER_CSS           1U    /* CIP Safety Stack */
/* #define IXSER_LAYER_CSAL       0U */ /* CIP Safety Adaptation Layer */


/** IXSER_GET_UNIT_ID:
    This macro returns the 5 bit unit Id encoded within the 16 bit error code.
*/
/*lint -esym(755, IXSER_GET_UNIT_ID) macro not referenced: usage of this macro
                                     by SAPL is optional */
#define IXSER_GET_UNIT_ID(w_errorCode)  \
          ((((CSS_t_WORD)(w_errorCode)) & 0x1F00U) >> 8)


/** IXSER_GET_UNIT_ERR:
    This macro returns the 8 bit unit error encoded within the 16 bit error
    code.
*/
/*lint -esym(755, IXSER_GET_UNIT_ERR) macro not referenced: usage of this macro
                                      by SAPL is optional */
#define IXSER_GET_UNIT_ERR(w_errorCode) \
          ((((CSS_t_WORD)(w_errorCode)) & 0x00FFU))


#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  /** Error code definition of IXSER_ErrorStrGet():
      These defines represent the error codes which can be generated by the
      {IXSER_ErrorStrGet()}.
  */
  #define IXSER_k_BUF_PTR_NULL    0
  #define IXSER_k_BUF_TOO_SML     -1

  /** IXSER_k_MAX_ERROR_STRING_LEN:
      This define determines the maximum length that an error string can have.
  */
  #define IXSER_k_MAX_ERROR_STRING_LEN   200U

#endif

/** IXSER_k_x_NOT_USED:
    This value indicates that this parameter of SAPL_CssErrorClbk() is not in
    use for the passed error code.
*/
/* Parameter Instance ID (u16_instId)  not used */
#define IXSER_k_I_NOT_USED        0U
/* Additional Info (dw_addInfo) not used */
#define IXSER_k_A_NOT_USED        0U


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
** Function    : SAPL_CssErrorClbk
**
** Description : This callback function must be provided by the Safety
**               Application. The function is called by the CIP Safety Stack
**               to signal an internal error of the Stack (see Req.2.5.1.1-1
**               and Req.10.4.1-4).
**
**               **Attention**: If the error type (see IXSER_GET_ERROR_TYPE) is
**               IXSER_TYPE_FS then the application must switch the device in
**               the fail safe state. This function shall then not return and
**               shall not call any further CSS functions. This state shall be
**               kept until the device is restarted. Further platform specific
**               actions may be necessary (e.g. triggering a hardware watchdog,
**               etc.). If the Safety Application wants to leave this state it
**               is recommended to emulate a power cycle as closely as possible.
**
** Context     : to be expected while any API call
**
** See Also    : IXSER_ErrorStrGet()
**
** Parameters  : w_errorCode (IN)  - 16 bit value that holds Error Type, Error
**                                   Layer, Unit ID and Unit Error.
**               u16_instId (IN)   - instance Id (e.g. of the safety validator
**                                   instance that caused the error), usage is
**                                   error code specific
**               dw_addInfo (IN)   - additional error information.
**
** Returnvalue : -
**
*******************************************************************************/
void SAPL_CssErrorClbk(CSS_t_WORD w_errorCode,
                       CSS_t_UINT u16_instId,
                       CSS_t_DWORD dw_addInfo);


/*******************************************************************************
**
** Function    : IXSER_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly. Every unit stores its error strings and returns them
**               if requested. The caller must provide a buffer for the string.
**               The error strings have variable length, but the caller has to
**               take into account that the passed buffer must be able to store
**               at least IXSER_k_MAX_ERROR_STRING_LEN number of characters. If
**               a larger buffer is provided the space after the string will
**               stay empty.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {error coding} in IXSERapi.h)
**               u16_instId (IN)        - instance Id (e.g. of the safety
**                                        validator instance that caused the
**                                        error), usage is error code specific
**                                        (not checked, any value allowed)
**               dw_addInfo (IN)        - additional error information.
**                                        (not checked, any value allowed)
**               u16_errStrBufSize (IN) - Size of the error string buffer.
**                                        (checked in IXSER_ErrorStrGet(),
**                                        valid range:
**                                        >= IXSER_k_MAX_ERROR_STRING_LEN)
**               pc_errStr (OUT)        - pointer to error string buffer (buffer
**                                        to where this function will write the
**                                        error string)
**                                        (checked)
**                                        valid range: <> CSS_k_NULL
**
** Returnvalue : >0                     - success, length of the error string in
**                                        the error string buffer without '\0'
**               IXSER_k_BUF_PTR_NULL   - pointer to the error string buffer
**                                        (see pc_errStr) is CSS_k_NULL
**               IXSER_k_BUF_TOO_SML    - the error string buffer (see
**                                        u16_errStrBufSize) is too small
**
*******************************************************************************/
#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)
  CSS_t_INT IXSER_ErrorStrGet(CSS_t_WORD w_errorCode,
                              CSS_t_UINT u16_instId,
                              CSS_t_DWORD dw_addInfo,
                              CSS_t_UINT u16_errStrBufSize,
                              CSS_t_CHAR *pc_errStr);
#endif


#endif /* IXSER_API_H */

/*** End of File ***/



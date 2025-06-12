/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSCapi.h
**    Summary: IXSSC - Safety Stack Control
**             Interface definition of the IXSSC unit of the CSS.
**             This unit controls the execution of the CIP Safety Stack.
**             The interface functions defined in this unit must be called by
**             the application to initialize and terminate CSS.
**             IXSSCapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSC_Init
**             IXSSC_Terminate
**             IXSSC_SoftErrByteGet
**             IXSSC_SoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSC_API_H
#define IXSSC_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/

/** IXSSC_kac_CSS_VERSION_STR:
    This constant represents the CSS version string. It includes the
    {CSS_k_STACK_VERSION} and the {CSS_k_STACK_STATUS} strings.
*/
extern const CSS_t_CHAR IXSSC_kac_CSS_VERSION_STR[];


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*
**  Functions provided by the IXSSC Unit
*/

/*******************************************************************************
**
** Function    : IXSSC_Init
**
** Description : This API function initializes the CSS units.
**               (see Req.3.1-3)
**
**               **Attention**:
**               It is the responsibility of the calling application
**               to ensure that at this time the HALC communications to the
**               other controller(s) is already initialized and working.
**               Otherwise it will not be possible to exchange safe I/O data.
**
**               **Attention**:
**               This API function must be called at startup to initialize the
**               stack before any other CSS function is called.
**
** See Also    : IXSSC_Terminate()
**
** Parameters  : ps_initInfo (IN)       - pointer to structure of type
**                                        {CSS_t_ID_INFO} which must contain
**                                        information that is necessary for the
**                                        stack to operate.
**                                        (checked, valid range: <> CSS_k_NULL -
**                                        all fields of the structure: not
**                                        checked, any value allowed)
**               u32_sysTime_128us (IN) - Current system time. System time is a
**                                        32-bit counter incremented in 128us
**                                        steps.
**                                        (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSC_Init(const CSS_t_ID_INFO *ps_initInfo,
                CSS_t_UDINT u32_sysTime_128us);


/*******************************************************************************
**
** Function    : IXSSC_Terminate
**
** Description : This API function performs the shutdown of the CIP Safety
**               Stack. The CSS cannot be used afterwards unless {IXSSC_Init()}
**               is called again.
**
**               **Attention**:
**               Before calling this function the application must switch
**               the device into the fail safe state as safety application
**               callback functions like SAPL_IxsvsEventHandlerClbk() or
**               SAPL_IxsvcEventHandlerClbk() e.g. to signal closing of a
**               connection, are not called any more!
**
**               **Attention**:
**               The Safety Application has to call this function if its
**               background diagnostics detects a failure of the communication.
**               This function will then terminate the CSS and thereby also all
**               connections (see FRS120).
**
** See Also    : IXSSC_Init()
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSC_Terminate(void);


/*******************************************************************************
**
** Function    : IXSSC_SoftErrByteGet
**
** Description : This API function fills the provided buffer with data of
**               stack internal static variables which must be protected against
**               Soft Errors (see Req.5.3-17). The Safety Application can
**               implement Soft Error protection by different means (data cross
**               checking with other Safety Controller, calculating CRCs for
**               cross checking, etc.)
**
**               **Attention**:
**               The call of this API function must be synchronized with the
**               other Safety Controller to make sure CSS internal variables
**               have equal values on both controllers.
**
**               **Attention**:
**               This function should be considered deprecated in favor of
**               IXSSC_SoftErrVarGet().
**
** Parameters  : o_start (IN)             - if CSS_k_TRUE is passed then the
**                                          internal counter is reset and
**                                          the first chunk of stack internal
**                                          data is returned.
**                                        - if CSS_k_FALSE is passed consecutive
**                                          chunks are returned. This function
**                                          internally has a counter which
**                                          advances with every call.
**                                          (not checked - any value allowed)
**               pb_softErrCheckBuf (OUT) - The caller must provide a pointer to
**                                          a buffer which has at least a size
**                                          of {CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE}
**                                          bytes. After return this buffer is
**                                          filled with data to be checked.
**                                          (checked,
**                                          valid range: <> CSS_k_NULL)
**               pu8_ModuleCnt (OUT)      - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          module counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu32_VarCnt (OUT)        - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE               - The returned chunk is the last chunk
**                                          of data. The next call of this
**                                          function will return the first chunk
**                                          of data again (independent of
**                                          o_start).
**               CSS_k_FALSE              - The returned chunk is not yet the
**                                          last chunk of data.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_BOOL IXSSC_SoftErrByteGet(CSS_t_BOOL o_start,
                                  CSS_t_BYTE *pb_softErrCheckBuf,
                                  CSS_t_USINT *pu8_ModuleCnt,
                                  CSS_t_UDINT *pu32_VarCnt);
#endif


/*******************************************************************************
**
** Function    : IXSSC_SoftErrVarGet
**
** Description : This API function fills the provided buffer with data of
**               stack internal static variables which must be protected against
**               Soft Errors (see Req.5.3-17). The Safety Application can
**               implement Soft Error protection by different means (data cross
**               checking with other Safety Controller, calculating CRCs for
**               cross checking, etc.)
**
**               **Attention**:
**               The call of this API function must be synchronized with the
**               other Safety Controller to make sure CSS internal variables
**               have equal values on both controllers.
**
** Parameters  : o_start (IN)             - if CSS_k_TRUE is passed then the
**                                          internal counter is reset and
**                                          the first chunk of stack internal
**                                          data is returned.
**                                        - if CSS_k_FALSE is passed consecutive
**                                          chunks are returned. This function
**                                          internally has a counter which
**                                          advances with every call.
**                                          (not checked - any value allowed)
**               pb_softErrCheckBuf (OUT) - The caller must provide a pointer to
**                                          a buffer which has at least a size
**                                          of {CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE}
**                                          bytes. After return this buffer is
**                                          filled with data to be checked.
**                                          (checked,
**                                          valid range: <> CSS_k_NULL)
**               pu16_waster (OUT)        - if <> CSS_k_NULL then this variable
**                                          will provide the number of unused
**                                          bytes in the provided buffer on
**                                          return. As the buffer is filled with
**                                          variables that can be 1, 2 or 4
**                                          bytes long and based on the fact
**                                          that variables will not get
**                                          subdivided up to 3 bytes at the end
**                                          of the buffer may remain unused.
**                                          In case the last chunk is reached
**                                          the buffer may contain even more
**                                          unused bytes.
**                                          (not checked, any value allowed)
**               pu8_ModuleCnt (OUT)      - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          module counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu8_VarLvl (OUT)         - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable level counter after the
**                                          execution of this function. In case
**                                          a Soft Error has been detected this
**                                          helps to analyze which stack
**                                          internal variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu32_VarCnt (OUT)        - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
** Returnvalue : CSS_k_TRUE               - The returned chunk is the last chunk
**                                          of data. The next call of this
**                                          function will return the first chunk
**                                          of data again (independent of
**                                          o_start).
**               CSS_k_FALSE              - The returned chunk is not yet the
**                                          last chunk of data.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  CSS_t_BOOL IXSSC_SoftErrVarGet(CSS_t_BOOL o_start,
                                 CSS_t_BYTE *pb_softErrCheckBuf,
                                 CSS_t_UINT *pu16_waster,
                                 CSS_t_USINT *pu8_ModuleCnt,
                                 CSS_t_USINT *pu8_VarLvl,
                                 CSS_t_UDINT *pu32_VarCnt);
#endif


#endif /* IXSSC_API_H */

/*** End of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAI.h
**    Summary: IXSAI - Safety Assembly Object Interface
**             IXSAI.h is the export header file of the IXSAI unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_Init
**             IXSAI_InSoftErrByteGet
**             IXSAI_InSoftErrVarGet
**             IXSAI_OutSoftErrByteGet
**             IXSAI_OutSoftErrVarGet
**             IXSAI_AsmInstTypeGet
**             IXSAI_AsmInputDataGet
**             IXSAI_AsmOutputDataSet
**             IXSAI_AsmOutInstFromIdxGet
**             IXSAI_AsmOutIdxFromInstGet
**             ISXAI_AsmInInstDataSafeValueSet
**             ISXAI_AsmOutInstDataSafeValueSet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSAI_H
#define IXSAI_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSAI_k_INVALID_ASM_INDEX:
    Indication that an Assembly Index is invalid.
*/
/*lint -esym(755, IXSAI_k_INVALID_ASM_INDEX) not referenced in every config */
#define IXSAI_k_INVALID_ASM_INDEX    0xFFFFU


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
** Function    : IXSAI_Init
**
** Description : This function initializes the IXSAI unit.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
  void IXSAI_Init(void);
#endif


/*******************************************************************************
**
** Function    : IXSAI_InSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
    CSS_t_UDINT IXSAI_InSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE * pb_var);
  #endif
#endif


/*******************************************************************************
**
** Function    : IXSAI_InSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
    void IXSAI_InSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
  #endif
#endif


/*******************************************************************************
**
** Function    : IXSAI_OutSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
    CSS_t_UDINT IXSAI_OutSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
  #endif
#endif


/*******************************************************************************
**
** Function    : IXSAI_OutSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
    void IXSAI_OutSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
  #endif
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmInstTypeGet
**
** Description : This function can be used to determine the type of the
**               specified assembly instance.
**
** See Also    : -
**
** Parameters  : u16_asmInstId (IN)  - Assembly Instance ID
**                                     (not checked, any value allowed)
**
** Returnvalue : IXSAI_k_ASM_INPUT   - Input Assembly Object
**               IXSAI_k_ASM_OUTPUT  - Output Assembly Object
**               IXSAI_k_ASM_INVALID - invalid instance ID
**
*******************************************************************************/
#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
  IXSAI_t_ASM_TYPE IXSAI_AsmInstTypeGet(CSS_t_UINT u16_asmInstId);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmInputDataGet
**
** Description : This function is used by the Safety Validator Client to read
**               the I/O data that it will transmit via a safe connection.
**
** See Also    : IXSAI_AsmOutputDataSet()
**
** Parameters  : u16_asmIoInstId (IN) - Assembly Instance ID
**                                      (not checked, checked in
**                                      AsmInIdxFromInstGet(), valid range:
**                                      1..0xFFFE and
**                                      <> CSS_cfg_ASM_NULL_INST_ID)
**               pb_data (OUT)        - pointer to a buffer of the Safety
**                                      Validator Client
**                                      (not checked, only called with reference
**                                      to array)
**               u16_bufSize (IN)     - size of the buffer to which pb_data
**                                      points
**                                      (checked, valid range: <= length set up
**                                      for this instance with
**                                      IXSAI_AsmIoInstSetup())
**               pb_ri (OUT)          - Run/Idle Flag of this Assembly Instance
**                                      (not checked, only called with reference
**                                      to variable)
**
** Returnvalue : CSS_k_OK             - OK
**               <>CSS_k_OK           - error during execution of the function
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_WORD IXSAI_AsmInputDataGet(CSS_t_UINT u16_asmIoInstId,
                                   CSS_t_BYTE *pb_data,
                                   CSS_t_UINT u16_bufSize,
                                   CSS_t_BYTE *pb_ri);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmOutputDataSet
**
** Description : This function is used by the Safety Validator Server to write
**               the I/O data that it has received via a safe connection.
**
** See Also    : IXSAI_AsmInputDataGet()
**
** Parameters  : u16_asmIoInstId (IN) - Assembly Instance ID
**                                      (not checked, checked in
**                                      IXSAI_AsmOutIdxFromInstGet(),
**                                      valid range:
**                                      1..0xFFFE and
**                                      <> CSS_cfg_ASM_NULL_INST_ID)
**               pb_data (IN)         - pointer to a buffer of the Safety
**                                      Validator Server
**                                      (not checked, any value allowed)
**               u16_dataLen (IN)     - number of bytes to be written
**                                      (checked, valid range:
**                                      (checked, valid range: >= length set up
**                                      for this instance with
**                                      IXSAI_AsmIoInstSetup())
**               b_ri (IN)            - Run/Idle Flag of this Assembly Instance
**                                      (not checked, only called with constants
**                                      valid range:
**                                      CSS_k_RIF_RUN or CSS_k_RIF_IDLE)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
void IXSAI_AsmOutputDataSet(CSS_t_UINT u16_asmIoInstId,
                            const CSS_t_BYTE *pb_data,
                            CSS_t_UINT u16_dataLen,
                            CSS_t_BYTE b_ri);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmOutInstFromIdxGet
**
** Description : This function determines the Instance ID of an Out Assembly
**               from the specified index.
**
** See Also    : -
**
** Parameters  : u16_asmIdx (IN) - Output Assembly Instance index
**                                 (checked, valid range:
**                                 <CSS_cfg_NUM_OF_ASM_OUT_INSTS)
**
** Returnvalue : CSOS_k_INVALID_INSTANCE   - error: the passed index is invalid
**               <>CSOS_k_INVALID_INSTANCE - Output Assembly Instance ID
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  #if (    (CSOS_cfg_TARGET == CSOS_k_ENABLE)                              \
        && (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS == 0U)                     \
             || (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS == CSS_k_NOT_APPLICABLE)   \
           )                                                               \
      )
    CSS_t_UINT IXSAI_AsmOutInstFromIdxGet(CSS_t_UINT u16_asmIdx);
  #endif
#endif


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSAI_AsmOutIdxFromInstGet) */
/*lint -esym(765, IXSAI_AsmOutIdxFromInstGet) */
/*******************************************************************************
**
** Function    : IXSAI_AsmOutIdxFromInstGet
**
** Description : This function determines the Index of an Out Assembly from the
**               specified Instance ID.
**
** See Also    : -
**
** Parameters  : u16_instId (IN) - Assembly Instance ID
**                                 (checked, valid range:
**                                 <> CSS_cfg_ASM_NULL_INST_ID and
**                                 #if (CSS_cfg_IO_ASM_LOOKUP_TABLE ==
**                                   CSS_k_ENABLE)
**                                   1..CSOS_k_MAX_USINT
**                                 #else
**                                   1..0xFFFE
**                                 #endif
**                                 )
**
** Returnvalue : IXSAI_k_INVALID_ASM_INDEX   - error: no assembly with this
**                                             instance is defined
**               <>IXSAI_k_INVALID_ASM_INDEX - Output Assembly index
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_UINT IXSAI_AsmOutIdxFromInstGet(CSS_t_UINT u16_instId);
#endif


/*******************************************************************************
**
** Function    : ISXAI_AsmInInstDataSafeValueSet
**
** Description : This function is used by the CCO to set the safe values (zero)
**               for an Input Assembly instance
**
** Parameters  : u16_instId (IN) - Assembly Instance ID
**                                 (not checked, checked in
**                                 AsmInIdxFromInstGet(), valid range:
**                                 1..0xFFFE and <> CSS_cfg_ASM_NULL_INST_ID)
**
** Returnvalue : CSS_k_OK        - OK
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
    CSS_t_WORD ISXAI_AsmInInstDataSafeValueSet(CSS_t_UINT u16_instId);
  #endif
#endif


/*******************************************************************************
**
** Function    : ISXAI_AsmOutInstDataSafeValueSet
**
** Description : This function is used by the CCO to set the safe values (zero)
**               for an Output Assembly instance
**
** Parameters  : u16_instId (IN) - Assembly Instance ID
**                                 (not checked, checked in
**                                 AsmInIdxFromInstGet(), valid range:
**                                 1..0xFFFE and <> CSS_cfg_ASM_NULL_INST_ID)
**
** Returnvalue : CSS_k_OK        - OK
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
    CSS_t_WORD ISXAI_AsmOutInstDataSafeValueSet(CSS_t_UINT u16_instId);
  #endif
#endif


#endif /* #ifndef IXSAI_H */

/*** End Of File ***/


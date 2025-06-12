/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIapi.h
**    Summary: IXSAI - Safety Assembly Object Interface
**             Interface definition of the IXSAI unit of the CSS.
**             This unit represents the interface to the safe I/O data. If an
**             application requires an Assembly Object it must implement it
**             itself. This unit just models the data interface. Thus no
**             services or attributes of the Assembly Object are implemented
**             in this unit.
**             IXSAIapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_AsmIoInstSetup
**             IXSAI_AsmIoInstDataGet
**             IXSAI_AsmIoInstDataSet
**             IXSAI_AsmIoInstDataRunModeSet
**             SAPL_IxsaiIoDataRxClbk
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSAI_API_H
#define IXSAI_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSAI_t_ASM_TYPE:
    Constants for identifying the type of an assembly instance.
*/
/*lint -esym(756, IXSAI_t_ASM_TYPE) not referenced in every cfg */
typedef enum
{
/*lint -esym(769, IXSAI_k_ASM_INPUT)              not referenced in every cfg */
/*lint -esym(769, IXSAI_k_ASM_OUTPUT)             not referenced in every cfg */
/*lint -esym(769, IXSAI_k_ASM_INVALID)            not referenced in every cfg */
  IXSAI_k_ASM_INPUT   = 0x0110,   /* assembly instance is an input */
  IXSAI_k_ASM_OUTPUT  = 0x0220,   /* assembly instance is an output */
  IXSAI_k_ASM_INVALID = 0x0880    /* assembly instance is not used/configured */
} IXSAI_t_ASM_TYPE;


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*
**  Functions provided by the IXSAI Unit
*/

/*******************************************************************************
**
** Function    : IXSAI_AsmIoInstSetup
**
** Description : This API function must be called by the application to set up
**               an I/O Assembly instance.
**
** Parameters  : u16_instId (IN)    - Assembly Instance ID that is to be
**                                    assigned
**                                    (checked, valid range: 1..0xFFFE
**                                    and <> CSS_cfg_ASM_NULL_INST_ID
**                                    if (CSS_cfg_IO_ASM_LOOKUP_TABLE ==
**                                    CSS_k_ENABLE) then range is limited to
**                                    1..255)
**               u16_length (IN)    - Data length that is to be assigned
**                                    (checked, valid range:
**                                    1..IXSAI_k_ASM_IO_MAX_LEN)
**                                    IXSAI_AsmInputInstSetup() and
**                                    IXSAI_AsmOutputInstSetup())
**               e_type (IN)        - type that is to be assigned (input or
**                                    output, see type definition of
**                                    {IXSAI_t_ASM_TYPE}
**                                    (checked, valid range: IXSAI_k_ASM_INPUT
**                                    or IXSAI_k_ASM_OUTPUT)
**
** Returnvalue : -
**
*******************************************************************************/
#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
  void IXSAI_AsmIoInstSetup(CSS_t_UINT u16_instId,
                            CSS_t_UINT u16_length,
                            IXSAI_t_ASM_TYPE e_type);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmIoInstDataGet
**
** Description : This API function must be called by the application in order to
**               read the last received (see FRS82) Safety I/O data. The
**               function returns a pointer to a buffer that contains the data
**               of the assembly instance and the length of the data. A NULL-
**               pointer is returned in case the associated connection is not in
**               Run Mode.
**
**               **Attention**:
**               The Safety Application can call this function always as a
**               reaction to {SAPL_IxsaiIoDataRxClbk()} and obtain the last
**               received data. Alternatively the Safety application can call
**               this function cyclically with an independent interval compared
**               to the receiving Safety Validator Server (faster or slower, but
**               always considering the system reaction time). The buffer to
**               which the returned pointer refers always contains the last
**               received data.
**
** See Also    : IXSAI_AsmIoInstDataSet()
**
** Parameters  : u16_instId (IN)   - Instance ID the Output Assembly instance
**                                   (not checked, checked in
**                                   IXSAI_AsmOutIdxFromInstGet(), valid range:
**                                   1..0xFFFE and <> CSS_cfg_ASM_NULL_INST_ID)
**               pu16_length (OUT) - returns the length of the data of this
**                                   Assembly instance
**                                   (checked, valid range: <> CSS_k_NULL)
**
** Returnvalue : <>CSS_k_NULL      - pointer to assembly data
**               CSS_k_NULL        - Instance is Idle or error during execution
**                                   of the function
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  const CSS_t_BYTE* IXSAI_AsmIoInstDataGet(CSS_t_UINT u16_instId,
                                           CSS_t_UINT *pu16_length);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmIoInstDataSet
**
** Description : This API function must be called by the application in order to
**               write the Safety I/O data which is to be transmitted. The
**               function fills the requested Input assembly object with the
**               data of the provided buffer. The function checks if the input
**               assembly data buffer is large enough. Otherwise an error is
**               returned. The application must call this function on both
**               controllers in parallel to pass the safe data to the CSS
**               (see Asm.2.5.2.1-1).
**
**               **Attention**:
**               The Safety Application shall call this function always when its
**               data changes. This function then stores the data internally in
**               the stack and when it is time for the next production (EPI)
**               will use this data (see FRS118). Alternatively the Safety
**               Application can set the data cyclically in an independent
**               interval compared to the EPI (the system reaction time must
**               always be considered).
**
** See Also    : IXSAI_AsmIoInstDataGet()
**
** Parameters  : u16_instId (IN)  - Instance ID the Input Assembly instance
**                                  (not checked, checked in
**                                  AsmInIdxFromInstGet(), valid range:
**                                  1..0xFFFE and <> CSS_cfg_ASM_NULL_INST_ID)
**               pb_data (IN)     - pointer to a buffer of the application
**                                  where this function reads the data
**                                  (checked, valid range: <> CSS_k_NULL)
**               u16_dataLen (IN) - number of bytes to copy
**                                  (checked, valid range: >= length set up for
**                                  this instance with IXSAI_AsmIoInstSetup())
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  void IXSAI_AsmIoInstDataSet(CSS_t_UINT u16_instId,
                              const CSS_t_BYTE *pb_data,
                              CSS_t_UINT u16_dataLen);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmIoInstDataRunModeSet
**
** Description : This API function must be called by the Safety Application in
**               order to set the Run_Idle flag in the Mode Byte of a producing
**               Safety Connection. The Run_Idle flag indicates the usability of
**               the data as determined by the Producer Safety Application
**               (see FRS117 and FRS218).
**
**               **Attention**:
**               The Run_Idle flag of producing Safety Connections is only under
**               control of the Safety Application while the Safety Supervisor
**               is in Executing state. Otherwise the Run_Idle flag of producing
**               Safety Connections is forced to "Idle".
**
** See Also    : IXSAI_AsmIoInstDataGet()
**
** Parameters  : u16_instId (IN) - Instance ID the Input Assembly instance
**                                 (not checked, checked in
**                                 AsmInIdxFromInstGet(), valid range:
**                                 1..0xFFFE and <> CSS_cfg_ASM_NULL_INST_ID)
**               o_run (IN)      - CSS_k_TRUE:  Run_Idle flag is set,
**                                              indicates data valid
**                                 CSS_k_FALSE: Run_Idle flag cleared,
**                                              indicates data invalid
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  void IXSAI_AsmIoInstDataRunModeSet(CSS_t_UINT u16_instId,
                                     CSS_t_BOOL o_run);
#endif


/*
** Functions to be provided by the Safety Application
*/

/*******************************************************************************
**
** Function    : SAPL_IxsaiIoDataRxClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case an output
**               assembly object instance has received new data. The safety
**               application has to store this event in its own new data flag
**               (see FRS131). The safety application shall then call
**               {IXSAI_AsmIoInstDataGet()} to copy the new data to the
**               application.
**
** Context     : IXSVO_CmdProcess()
**
** See Also    : IXSAI_AsmIoInstDataGet()
**
** Parameters  : u16_instId (IN) - Instance ID the assembly object that has new
**                                 data.
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  void SAPL_IxsaiIoDataRxClbk(CSS_t_UINT u16_instId);
#endif


#endif /* IXSAI_API_H */

/*** End of File ***/


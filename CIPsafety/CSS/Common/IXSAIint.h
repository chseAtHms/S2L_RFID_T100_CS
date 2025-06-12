/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIint.h
**    Summary: IXSAI - Safety Assembly Object Interface
**             IXSAI.h is the internal header file of the IXSAI unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_InputInit
**             IXSAI_AsmInputInstSetup
**             IXSAI_InputInstCheckExists
**             IXSAI_OutputInit
**             IXSAI_AsmOutputInstSetup
**             IXSAI_OutputInstCheckExists
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSAIINT_H
#define IXSAIINT_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** k_ASM_IO_MAX_LEN:
    Maximum length that can be sent or received by a safety connection.
*/
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  #define IXSAI_k_ASM_IO_MAX_LEN   CSOS_cfg_LONG_FORMAT_MAX_LENGTH
#else
  #define IXSAI_k_ASM_IO_MAX_LEN   CSS_k_SHORT_FORMAT_MAX_LENGTH
#endif


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSAI_t_ASM_INST:
    Structure containing the configuration and data of an assembly instance.
*/
typedef struct
{
  CSS_t_UINT       u16_instId;         /* assembly instance id */
  CSS_t_UINT       u16_length;         /* number of bytes of asm data */
  CSS_t_BYTE       b_runIdle;          /* assembly specific Run/Idle flag */
  CSS_t_BYTE       ab_data[IXSAI_k_ASM_IO_MAX_LEN]; /* array of assembly data */
} IXSAI_t_ASM_INST;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSAI_InputInit
**
** Description : This function initializes all Input Assembly instance
**               variables.
**
** See Also    : IXSAI_OutputInit
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  void IXSAI_InputInit(void);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmInputInstSetup
**
** Description : This function sets up an Input Assembly instance.
**
** See Also    : IXSAI_AsmOutputInstSetup()
**
** Parameters  : u16_instId (IN) - Assembly Instance ID that is to be assigned
**                                 (not checked, checked by
**                                 IXSAI_AsmIoInstSetup())
**               u16_length (IN) - Data length that is to be assigned
**                                 (not checked, checked by
**                                 IXSAI_AsmIoInstSetup())
**
** Returnvalue : CSS_k_OK        - OK
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_UINT IXSAI_AsmInputInstSetup(CSS_t_UINT u16_instId,
                                     CSS_t_UINT u16_length);
#endif


/*******************************************************************************
**
** Function    : IXSAI_InputInstCheckExists
**
** Description : This function checks if the passed instance ID is existing and
**               is an Input Assembly Instance.
**
** See Also    : IXSAI_OutputInstCheckExists()
**
** Parameters  : u16_asmInstId (IN) - Assembly Object Instance ID to be checked
**                                    (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE   - the passed instance ID is an Input Instance
**               CSS_k_FALSE  - the passed instance ID is not an Input Instance
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_BOOL IXSAI_InputInstCheckExists(CSS_t_UINT u16_asmInstId);
#endif


/*******************************************************************************
**
** Function    : IXSAI_OutputInit
**
** Description : This function initializes all Output Assembly instance
**               variables.
**
** See Also    : IXSAI_InputInit
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  void IXSAI_OutputInit(void);
#endif


/*******************************************************************************
**
** Function    : IXSAI_AsmOutputInstSetup
**
** Description : This function sets up an Output Assembly instance.
**
** See Also    : IXSAI_AsmInputInstSetup()
**
** Parameters  : u16_instId (IN) - Assembly Instance ID that is to be assigned
**                                 (not checked, checked by
**                                 IXSAI_AsmIoInstSetup())
**               u16_length (IN) - Data length that is to be assigned
**                                 (not checked, checked by
**                                 IXSAI_AsmIoInstSetup())
**
** Returnvalue : CSS_k_OK        - OK
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_UINT IXSAI_AsmOutputInstSetup(CSS_t_UINT u16_instId,
                                      CSS_t_UINT u16_length);
#endif


/*******************************************************************************
**
** Function    : IXSAI_OutputInstCheckExists
**
** Description : This function checks if the passed instance ID is existing and
**               is an Output Assembly Instance.
**
** See Also    : IXSAI_InputInstCheckExists()
**
** Parameters  : u16_asmInstId (IN) - Assembly Object Instance ID to be checked
**                                    (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE  - the passed instance ID is an Output Instance
**               CSS_k_FALSE - the passed instance ID is not an Output Instance
**
*******************************************************************************/
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  CSS_t_BOOL IXSAI_OutputInstCheckExists(CSS_t_UINT u16_asmInstId);
#endif


#endif /* #ifndef IXSAIINT_H */

/*** End Of File ***/


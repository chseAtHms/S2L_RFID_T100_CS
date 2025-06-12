/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIin.c
**    Summary: IXSAI - Safety Assembly Object Interface
**             This module contains the routines of the Input Assembly
**             Instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_InputInit
**             IXSAI_AsmInputInstSetup
**             IXSAI_InputInstCheckExists
**             IXSAI_AsmIoInstDataRunModeSet
**             IXSAI_AsmIoInstDataSet
**             ISXAI_AsmInInstDataSafeValueSet
**             IXSAI_AsmInputDataGet
**             IXSAI_InSoftErrByteGet
**             IXSAI_InSoftErrVarGet
**
**             AsmInIdxFromInstGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSSS.h"
#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSAIapi.h"
#include "IXSAI.h"
#include "IXSAIint.h"
#include "IXSAIerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** as_AsmInInst:
    Array of structures to store configuration of the Input assembly instances.
*/
static IXSAI_t_ASM_INST as_AsmInInst[CSS_cfg_NUM_OF_ASM_IN_INSTS];

/** u16_AsmInCreateIdx:
    Index in the as_AsmInInst array of the next instance to be created.
*/
static CSS_t_UINT u16_AsmInCreateIdx;

/** au16_AsmInIdxLookupTable:
    Lookup table to store the instance to index relation. The table elements are
    addressed by the instance ID and contain the assembly index.
*/
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  static CSS_t_UINT au16_AsmInIdxLookupTable[CSOS_k_MAX_USINT+1U];
#endif

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #define k_IN_SIZEOF_VAR1   (sizeof(as_AsmInInst))
  #define k_IN_SIZEOF_VAR12  (k_IN_SIZEOF_VAR1 + sizeof(u16_AsmInCreateIdx))
  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
    #define k_IN_SIZEOF_VAR123 (k_IN_SIZEOF_VAR12 +  \
                                sizeof(au16_AsmInIdxLookupTable))
  #else
    #define k_IN_SIZEOF_VAR123 k_IN_SIZEOF_VAR12
  #endif
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_UINT AsmInIdxFromInstGet(CSS_t_UINT u16_instId);


/*******************************************************************************
**    global functions
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
void IXSAI_InputInit(void)
{
  /* loop counter to go through Assembly Object Instances */
  CSS_t_UINT u16_idx = 0U;

  /* index of next instance to be created */
  u16_AsmInCreateIdx = 0U;

  /* make sure the complete structure is zero */
  CSS_MEMSET(as_AsmInInst, 0, sizeof(as_AsmInInst));

  /* mark all Input instances als unconfigured */
  for (u16_idx = 0U; u16_idx < CSS_cfg_NUM_OF_ASM_IN_INSTS; u16_idx++)
  {
    as_AsmInInst[u16_idx].u16_instId = CSS_cfg_ASM_NULL_INST_ID;
    as_AsmInInst[u16_idx].u16_length = 0U;
    as_AsmInInst[u16_idx].b_runIdle  = CSS_k_RIF_IDLE;
    CSS_MEMSET(as_AsmInInst[u16_idx].ab_data, 0,
               sizeof(as_AsmInInst[u16_idx].ab_data));
  }

  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  {
    CSS_t_UINT u16_inst = 0U;

    /* mark all Input instances als unconfigured */
    for (u16_inst = 0U; u16_inst < (CSOS_k_MAX_USINT+1U); u16_inst++)
    {
      au16_AsmInIdxLookupTable[u16_inst] = IXSAI_k_INVALID_ASM_INDEX;
    }
  }
  #endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_UINT IXSAI_AsmInputInstSetup(CSS_t_UINT u16_instId,
                                   CSS_t_UINT u16_length)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSAI_k_FSE_INC_PRG_FLOW);

  /* if there is no free instance left */
  if (u16_AsmInCreateIdx >= CSS_cfg_NUM_OF_ASM_IN_INSTS)
  {
    /* error: all instances already used */
    w_retVal = (IXSAI_k_FSE_AIP_INV_ASM_IDX_IS);
    SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* setup the instance with the passed data */
    as_AsmInInst[u16_AsmInCreateIdx].u16_instId = u16_instId;
    as_AsmInInst[u16_AsmInCreateIdx].u16_length = u16_length;
    as_AsmInInst[u16_AsmInCreateIdx].b_runIdle  = CSS_k_RIF_IDLE;

    #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
    {
      /* store assembly index in the instance lookup table */
      /* it has been checked by the calling function that u16_instId
         is <= CSOS_k_MAX_USINT */
      au16_AsmInIdxLookupTable[u16_instId] = u16_AsmInCreateIdx;
    }
    #endif

    /* prepare for next instance to be created */
    u16_AsmInCreateIdx++;

    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


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
CSS_t_BOOL IXSAI_InputInstCheckExists(CSS_t_UINT u16_asmInstId)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* loop counter to go through Assembly Object Instances */
  CSS_t_UINT u16_idx = 0U;

  /* check the Input assembly instances */
  do
  {
    /* if current instance matches */
    if (as_AsmInInst[u16_idx].u16_instId == u16_asmInstId)
    {
      /* matching instance was found */
      o_retVal = CSS_k_TRUE;
    }
    else
    {
      /* not found */
    }

    u16_idx++;

  } while ((!o_retVal) && (u16_idx < CSS_cfg_NUM_OF_ASM_IN_INSTS));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


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
void IXSAI_AsmIoInstDataRunModeSet(CSS_t_UINT u16_instId,
                                   CSS_t_BOOL o_run)
{
  /* internal index of the addressed instance */
  CSS_t_UINT u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;

  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIS_STATE_ERR_RMS, u16_instId,
                      IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* get the internal instance index from the passed Assembly Instance ID */
    u16_asmIoIdx = AsmInIdxFromInstGet(u16_instId);

    /* if index doesn't exist */
    if (u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX)
    {
      /* error: u16_asmIoIdx is invalid */
      SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_ASM_IDX_RMS,
                        u16_instId, IXSER_k_A_NOT_USED);
    }
    else /* else: index is ok */
    {
      /* set RUN/IDLE flag according to passed parameter */
      /* if passed RUN/IDLE flag indicates "run" */
      if (o_run)
      {
        as_AsmInInst[u16_asmIoIdx].b_runIdle = CSS_k_RIF_RUN;
      }
      else /* else: set RUN/Idle flag to "idle" */
      {
        as_AsmInInst[u16_asmIoIdx].b_runIdle = CSS_k_RIF_IDLE;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
void IXSAI_AsmIoInstDataSet(CSS_t_UINT u16_instId,
                            const CSS_t_BYTE *pb_data,
                            CSS_t_UINT u16_dataLen)
{
  /* internal index of the addressed instance */
  CSS_t_UINT u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;

  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIS_STATE_ERR_DS, u16_instId,
                      IXSER_k_A_NOT_USED);
  }
  /* else: if passed pointer is invalid */
  else if (pb_data == CSS_k_NULL)
  {
    /* error: pointer is invalid */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_PTR_DS, u16_instId,
                      IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* get the internal instance index from the passed Assembly Instance ID */
    u16_asmIoIdx = AsmInIdxFromInstGet(u16_instId);

    /* if index doesn't exist */
    if (u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX)
    {
      /* error: parameter u16_asmIoIdx is invalid */
      SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_ASM_IDX_DS, u16_instId,
                        IXSER_k_A_NOT_USED);
    }
    else /* else: index is ok */
    {
      /* if the passed data length is too large for our internal buffer */
      if (as_AsmInInst[u16_asmIoIdx].u16_length < u16_dataLen)
      {
        /* error: passed buffer is too large */
        SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_ASM_LEN_IDS, u16_instId,
                          IXSER_k_A_NOT_USED);
      }
      else /* else: length is ok */
      {
        /* copy the IO assembly data */
        CSS_MEMCPY(as_AsmInInst[u16_asmIoIdx].ab_data, pb_data, u16_dataLen);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
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
CSS_t_WORD ISXAI_AsmInInstDataSafeValueSet(CSS_t_UINT u16_instId)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSAI_k_FSE_INC_PRG_FLOW);
  /* get the internal instance index from the passed Assembly Instance ID */
  CSS_t_UINT u16_asmIoIdx = AsmInIdxFromInstGet(u16_instId);

  /* if index doesn't exist */
  if (u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX)
  {
    /* error: parameter u16_asmIoIdx is invalid */
    w_retVal = (IXSAI_k_FSE_INC_INV_ASM_ID_SSVI);
    SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: index is ok */
  {
    /* write zero to the Assembly Data */
    CSS_MEMSET(as_AsmInInst[u16_asmIoIdx].ab_data, 0,
               as_AsmInInst[u16_asmIoIdx].u16_length);
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE) */
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
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
CSS_t_WORD IXSAI_AsmInputDataGet(CSS_t_UINT u16_asmIoInstId,
                                 CSS_t_BYTE *pb_data,
                                 CSS_t_UINT u16_bufSize,
                                 CSS_t_BYTE *pb_ri)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSAI_k_FSE_INC_PRG_FLOW);
  /* find the internal index of the addressed assembly instance */
  CSS_t_UINT u16_asmIoIdx = AsmInIdxFromInstGet(u16_asmIoInstId);

  /* if not a valid index could be determined from the passed instance id */
  if (u16_asmIoIdx >= CSS_cfg_NUM_OF_ASM_IN_INSTS)
  {
    /* error: instance does not exist (or is not an I/O instance) */
    w_retVal = (IXSAI_k_FSE_INC_ASM_NOT_FOUND_I);
    SAPL_CssErrorClbk(w_retVal, u16_asmIoInstId, IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* if the passed length is not larger than the data buffer of the
       Assembly Instance */
    if (u16_bufSize < as_AsmInInst[u16_asmIoIdx].u16_length)
    {
      /* error: passed buffer is too small */
      w_retVal = (IXSAI_k_FSE_INC_INV_ASM_LEN_DG);
      SAPL_CssErrorClbk(w_retVal, u16_asmIoInstId, IXSER_k_A_NOT_USED);
    }
    else /* else: length is ok */
    {
      /* copy the IO assembly data */
      CSS_MEMCPY(pb_data, as_AsmInInst[u16_asmIoIdx].ab_data,
                 as_AsmInInst[u16_asmIoIdx].u16_length);

      /* return the Run/Idle Flag value */
      *pb_ri = as_AsmInInst[u16_asmIoIdx].b_runIdle;

      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE) */


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSAI_InSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                   CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_IN_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)as_AsmInInst)+u32_varCnt);
  }
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_IN_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)&u16_AsmInCreateIdx)
                              +(u32_varCnt - k_IN_SIZEOF_VAR1));
  }
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  /* else: if counter indicates we are in the range of the third variable */
  else if (u32_varCnt < (k_IN_SIZEOF_VAR123))
  {
    *pb_var = *(((CSS_t_BYTE*)au16_AsmInIdxLookupTable)
                              +(u32_varCnt - k_IN_SIZEOF_VAR12));
  }
#endif
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_IN_SIZEOF_VAR123;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSAI_InSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* select the level 0 variable */
  switch (au32_cnt[0U])
  {
    case 0U:
    {
      CSS_H2N_CPY16(pb_var, &u16_AsmInCreateIdx);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case 1U:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSS_cfg_NUM_OF_ASM_IN_INSTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case 0U:
          {
            CSS_H2N_CPY16(pb_var, &as_AsmInInst[au32_cnt[1U]].u16_instId);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case 1U:
          {
            CSS_H2N_CPY16(pb_var, &as_AsmInInst[au32_cnt[1U]].u16_length);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case 2U:
          {
            CSS_H2N_CPY8(pb_var, &as_AsmInInst[au32_cnt[1U]].b_runIdle);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            break;
          }

          case 3U:
          {
            /* indicate to caller that we are in range of level 3 or below */
            ps_retStat->u8_incLvl = 3U;

            /* if level 3 counter is larger than number of array elements */
            if (au32_cnt[3U] >= IXSAI_k_ASM_IO_MAX_LEN)
            {
              /* level 3 counter at/above end */
              /* default return values already set */
            }
            else
            {
              CSS_H2N_CPY8(pb_var, &as_AsmInInst[au32_cnt[1U]].ab_data[au32_cnt[3U]]);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            }
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
          }
          break;
        }
      }
      break;
    }

  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
    case 2U:
    {
      /* indicate to caller that we are in range of level 1 */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= (sizeof(au16_AsmInIdxLookupTable)
                          / sizeof(au16_AsmInIdxLookupTable[0])))
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY16(pb_var, &au16_AsmInIdxLookupTable[au32_cnt[1U]]);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      }
      break;
    }
  #endif

    default:
    {
      /* default return values already set */
      break;
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : AsmInIdxFromInstGet
**
** Description : This function determines the Index of an Input Assembly from
**               the specified Instance ID.
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
**               <>IXSAI_k_INVALID_ASM_INDEX - Input Assembly index
**
*******************************************************************************/
static CSS_t_UINT AsmInIdxFromInstGet(CSS_t_UINT u16_instId)
{
  /* return value of this function */
  CSS_t_UINT u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;

  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  {
    /* if passed instance ID parameter is invalid */
    if (    (u16_instId == 0U)                         /* zero */
         || (u16_instId > CSOS_k_MAX_USINT)            /* > lookup table */
         || (u16_instId == CSS_cfg_ASM_NULL_INST_ID)   /* NULL instance */
       )
    {
      SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_INST_IIFIG,
                        u16_instId, IXSER_k_A_NOT_USED);
    }
    else /* else: ok */
    {
      /* Return the requested index from the lookup table. The above check */
      /* makes sure that the passed id doesn't exceed the table. */
      u16_asmIoIdx = au16_AsmInIdxLookupTable[u16_instId];
    }
  }
  #else
  {
    /* if passed instance ID parameter is invalid */
    if (    (u16_instId == 0U)
         || (u16_instId == CSOS_k_INVALID_INSTANCE)
         || (u16_instId == CSS_cfg_ASM_NULL_INST_ID)
       )
    {
      SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_INST_IIFIG,
                        u16_instId, IXSER_k_A_NOT_USED);
    }
    else /* else: ok */
    {
      /* search index */
      CSS_t_UINT u16_idx = 0U;

      /* check the Input assembly instances */
      do
      {
        /* if current instance matches */
        if (as_AsmInInst[u16_idx].u16_instId == u16_instId)
        {
          /* Index was found */
          u16_asmIoIdx = u16_idx;
        }
        else /* else: index not found */
        {
          /* continue search */
        }

        u16_idx++;
      }
      /* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
      /* The following "while" was implemented in one line to avoid a wrong code
         coverage result of the gcov tool */
      while ((u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX) && (u16_idx < CSS_cfg_NUM_OF_ASM_IN_INSTS));
      /* RSM_IGNORE_QUALITY_END */
    }
  }
  #endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_asmIoIdx);
}


#else
  /* just to avoid lint warning "CSOScfg.h not used" */
  #if (CSOS_k_NOT_APPLICABLE == CSOS_k_NOT_APPLICABLE)
  #endif
#endif  /* (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE) */


/*** End Of File ***/



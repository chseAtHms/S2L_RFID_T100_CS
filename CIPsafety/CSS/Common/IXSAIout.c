/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAIout.c
**    Summary: IXSAI - Safety Assembly Object Interface
**             This module contains the routines of the Output Assembly
**             Instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_OutputInit
**             IXSAI_AsmOutputInstSetup
**             IXSAI_OutputInstCheckExists
**             IXSAI_AsmIoInstDataGet
**             IXSAI_AsmOutputDataSet
**             ISXAI_AsmOutInstDataSafeValueSet
**             IXSAI_AsmOutInstFromIdxGet
**             IXSAI_AsmOutIdxFromInstGet
**             IXSAI_OutSoftErrByteGet
**             IXSAI_OutSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)

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

/** as_AsmOutInst:
    Array of structures to store configuration of the Output assembly instances.
*/
static IXSAI_t_ASM_INST as_AsmOutInst[CSS_cfg_NUM_OF_ASM_OUT_INSTS];

/** u16_AsmOutCreateIdx:
    Index in the as_AsmOutInst array of the next instance to be created.
*/
static CSS_t_UINT u16_AsmOutCreateIdx;

/** au16_AsmOutIdxLookupTable:
    Lookup table to store the instance to index relation. The table elements are
    addressed by the instance ID and contain the assembly index.
*/
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  static CSS_t_UINT au16_AsmOutIdxLookupTable[CSOS_k_MAX_USINT+1U];
#endif

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #define k_OUT_SIZEOF_VAR1   (sizeof(as_AsmOutInst))
  #define k_OUT_SIZEOF_VAR12  (k_OUT_SIZEOF_VAR1 + sizeof(u16_AsmOutCreateIdx))
  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
    #define k_OUT_SIZEOF_VAR123 (k_OUT_SIZEOF_VAR12 +  \
                                 sizeof(au16_AsmOutIdxLookupTable))
  #else
    #define k_OUT_SIZEOF_VAR123 k_OUT_SIZEOF_VAR12
  #endif
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSAI_OutputInit(void)
{
  /* loop counter to go through Assembly Object Instances */
  CSS_t_UINT u16_idx = 0U;

  /* index of next instance to be created */
  u16_AsmOutCreateIdx = 0U;

  /* make sure the complete structure is zero */
  CSS_MEMSET(as_AsmOutInst, 0, sizeof(as_AsmOutInst));

  /* mark all Output instances as unconfigured */
  for (u16_idx = 0U; u16_idx < CSS_cfg_NUM_OF_ASM_OUT_INSTS; u16_idx++)
  {
    as_AsmOutInst[u16_idx].u16_instId = CSS_cfg_ASM_NULL_INST_ID;
    as_AsmOutInst[u16_idx].u16_length = 0U;
    as_AsmOutInst[u16_idx].b_runIdle  = CSS_k_RIF_IDLE;
    CSS_MEMSET(as_AsmOutInst[u16_idx].ab_data, 0,
               sizeof(as_AsmOutInst[u16_idx].ab_data));
  }

  #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  {
    CSS_t_UINT u16_inst = 0U;

    /* mark all Input instances als unconfigured */
    for (u16_inst = 0U; u16_inst < (CSOS_k_MAX_USINT+1U); u16_inst++)
    {
      au16_AsmOutIdxLookupTable[u16_inst] = IXSAI_k_INVALID_ASM_INDEX;
    }
  }
  #endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_UINT IXSAI_AsmOutputInstSetup(CSS_t_UINT u16_instId,
                                    CSS_t_UINT u16_length)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSAI_k_FSE_INC_PRG_FLOW);

  /* if there is no free instance left */
  if (u16_AsmOutCreateIdx >= CSS_cfg_NUM_OF_ASM_OUT_INSTS)
  {
    /* error: all instances already used */
    w_retVal = (IXSAI_k_FSE_AIP_INV_ASM_IDX_OS);
    SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* setup the instance with the passed data */
    as_AsmOutInst[u16_AsmOutCreateIdx].u16_instId = u16_instId;
    as_AsmOutInst[u16_AsmOutCreateIdx].u16_length = u16_length;
    as_AsmOutInst[u16_AsmOutCreateIdx].b_runIdle  = CSS_k_RIF_IDLE;

    #if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
    {
      /* store assembly index in the instance lookup table */
      /* it has been checked by the calling function that u16_instId
         is <= CSOS_k_MAX_USINT */
      au16_AsmOutIdxLookupTable[u16_instId] = u16_AsmOutCreateIdx;
    }
    #endif

    /* prepare for next instance to be created */
    u16_AsmOutCreateIdx++;

    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


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
CSS_t_BOOL IXSAI_OutputInstCheckExists(CSS_t_UINT u16_asmInstId)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* loop counter to go through Assembly Object Instances */
  CSS_t_UINT u16_idx = 0U;

  /* check the Output assembly instances */
  do
  {
    /* if current instance matches */
    if (as_AsmOutInst[u16_idx].u16_instId == u16_asmInstId)
    {
      /* matching instance was found */
      o_retVal = CSS_k_TRUE;
    }
    else /* else: index not found */
    {
      /* continue search */
    }

    u16_idx++;
  }
  while ((!o_retVal) && (u16_idx < CSS_cfg_NUM_OF_ASM_OUT_INSTS));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


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
const CSS_t_BYTE* IXSAI_AsmIoInstDataGet(CSS_t_UINT u16_instId,
                                         CSS_t_UINT *pu16_length)
{
  /* return value of this function */
  CSS_t_BYTE *pb_retVal = CSS_k_NULL;
  /* internal index of the addressed instance */
  CSS_t_UINT u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;

  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk((IXSAI_k_FSE_AIS_STATE_ERROR_DG),
                      u16_instId, 0U);
  }
  /* else: if the passed pointer is invalid */
  else if (pu16_length == CSS_k_NULL)
  {
    /* error: pointer is invalid */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_INV_PTR_DG,
                      u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: ok */
  {
    /* number of data bytes in the returned buffer - initialized to zero */
    *pu16_length = 0U;

    /* get the internal instance index from the passed Assembly Instance ID */
    u16_asmIoIdx = IXSAI_AsmOutIdxFromInstGet(u16_instId);

    /* if assembly index doesn't exist */
    if (u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX)
    {
      /* error: parameter u16_asmIoIdx is invalid */
      SAPL_CssErrorClbk((IXSAI_k_FSE_AIP_INV_ASM_IDX_DG),
                        u16_instId, 0U);
    }
    else /* else: assembly index is ok */
    {
      /* if the assembly instance is not in "run" mode */
      if (as_AsmOutInst[u16_asmIoIdx].b_runIdle != CSS_k_RIF_RUN)
      {
        /* data is invalid - return values already set */
      }
      else /* else: assembly instance is in "run" mode */
      {
        /* set return values: pointer to data and length */
        pb_retVal = as_AsmOutInst[u16_asmIoIdx].ab_data;
        *pu16_length = as_AsmOutInst[u16_asmIoIdx].u16_length;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (pb_retVal);
}


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
void IXSAI_AsmOutputDataSet(CSS_t_UINT u16_asmIoInstId,
                            const CSS_t_BYTE *pb_data,
                            CSS_t_UINT u16_dataLen,
                            CSS_t_BYTE b_ri)
{
  /* find the internal index of the addressed assembly instance */
  CSS_t_UINT u16_asmIoIdx = IXSAI_AsmOutIdxFromInstGet(u16_asmIoInstId);

  /* if not a valid index could be determined from the passed instance id */
  if (u16_asmIoIdx >= CSS_cfg_NUM_OF_ASM_OUT_INSTS)
  {
    /* error: instance does not exist (or is not an I/O instance) */
    SAPL_CssErrorClbk(IXSAI_k_FSE_INC_ASM_NOT_FOUND_O,
                      u16_asmIoInstId, IXSER_k_A_NOT_USED);
    /* return value already set */
  }
  else /* else: instance is pk */
  {
    /* if the length of the passed buffer is larger than the data buffer of the
       assembly */
    if (as_AsmOutInst[u16_asmIoIdx].u16_length < u16_dataLen)
    {
      /* error: passed buffer is too large */
      SAPL_CssErrorClbk(IXSAI_k_FSE_INC_INV_ASM_LEN_ODS,
                        u16_asmIoInstId, IXSER_k_A_NOT_USED);
      /* return value already set */
    }
    else /* else: length is ok */
    {
      /* if the RUN/IDLE flag doesn't indicate "run" mode */
      if (b_ri != CSS_k_RIF_RUN)
      {
        /* passed data is not valid because Safety Validator is Idle */
        /* do not copy the assembly data */
        as_AsmOutInst[u16_asmIoIdx].b_runIdle = CSS_k_RIF_IDLE;
        /* return value already set */
      }
      else /* else: assembly is in "run" mode */
      {
        /* result of calculation will always be >= 0 due to check above */
        CSS_t_UINT u16_unwrittenRest = as_AsmOutInst[u16_asmIoIdx].u16_length
                                     - u16_dataLen;

        /* copy the IO assembly data */
        CSS_MEMCPY(as_AsmOutInst[u16_asmIoIdx].ab_data, pb_data, u16_dataLen);

        /* make sure that in case u16_dataLen is less than the assembly the
           rest of the assembly is filled with zeros */
        CSS_MEMSET(&as_AsmOutInst[u16_asmIoIdx].ab_data[u16_dataLen], 0,
                   u16_unwrittenRest);

        /* mark received data as valid (RUN) */
        as_AsmOutInst[u16_asmIoIdx].b_runIdle = CSS_k_RIF_RUN;

        /* Inform the application about new data. Instead of just setting a */
        /* flag (see FRS131) we are calling a callback function.            */
        SAPL_IxsaiIoDataRxClbk(u16_asmIoInstId);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
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
CSS_t_WORD ISXAI_AsmOutInstDataSafeValueSet(CSS_t_UINT u16_instId)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSAI_k_FSE_INC_PRG_FLOW);
  /* get the internal instance index from the passed Assembly Instance ID */
  CSS_t_UINT u16_asmIoIdx = IXSAI_AsmOutIdxFromInstGet(u16_instId);

  /* if index doesn't exist */
  if (u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX)
  {
    /* error: parameter u16_asmIoIdx is invalid */
    w_retVal = (IXSAI_k_FSE_INC_INV_ASM_ID_SSVO);
    SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: index is ok */
  {
    /* write zero to the Assembly Data */
    CSS_MEMSET(as_AsmOutInst[u16_asmIoIdx].ab_data, 0,
               as_AsmOutInst[u16_asmIoIdx].u16_length);
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) */
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


#if (    (CSOS_cfg_TARGET == CSOS_k_ENABLE)                              \
      && (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS == 0U)                     \
           || (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS == CSS_k_NOT_APPLICABLE)   \
         )                                                               \
    )
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
CSS_t_UINT IXSAI_AsmOutInstFromIdxGet(CSS_t_UINT u16_asmIdx)
{
  /* return value and loop counter to go through Assembly Object Instances */
  CSS_t_UINT u16_asmInst = CSOS_k_INVALID_INSTANCE;

  /* if the passed parameter is in valid range */
  if (u16_asmIdx < CSS_cfg_NUM_OF_ASM_OUT_INSTS)
  {
    u16_asmInst = as_AsmOutInst[u16_asmIdx].u16_instId;
  }
  else /* else: the passed parameter is invalid */
  {
    /* return the already set default value */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_asmInst);
}
#endif


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
CSS_t_UINT IXSAI_AsmOutIdxFromInstGet(CSS_t_UINT u16_instId)
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
      /* The passed instance ID is not a valid output assembly instance */
      u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;
    }
    else /* else: ok */
    {
        /* Return the requested index from the lookup table. The above check */
        /* makes sure that the passed id doesn't exceed the table. */
      u16_asmIoIdx = au16_AsmOutIdxLookupTable[u16_instId];
    }
  }
  #else
  {
    /* if the passed instance ID is invalid */
    if (    (u16_instId == 0U)
         || (u16_instId == CSOS_k_INVALID_INSTANCE)
         || (u16_instId == CSS_cfg_ASM_NULL_INST_ID)
       )
    {
      /* The passed instance ID is not a valid output assembly instance */
      u16_asmIoIdx = IXSAI_k_INVALID_ASM_INDEX;
    }
    else /* else: instance is ok */
    {
      /* search index */
      CSS_t_UINT u16_idx = 0U;

      /* check the Output assembly instances */
      do
      {
        /* if current instance matches */
        if (as_AsmOutInst[u16_idx].u16_instId == u16_instId)
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
      while ((u16_asmIoIdx == IXSAI_k_INVALID_ASM_INDEX) && (u16_idx < CSS_cfg_NUM_OF_ASM_OUT_INSTS));
      /* RSM_IGNORE_QUALITY_END */
    }
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_asmIoIdx);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSAI_OutSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                    CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_OUT_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)as_AsmOutInst)+u32_varCnt);
  }
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_OUT_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)&u16_AsmOutCreateIdx)
                              +(u32_varCnt - k_OUT_SIZEOF_VAR1));
  }
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  /* else: if counter indicates we are in the range of the third variable */
  else if (u32_varCnt < (k_OUT_SIZEOF_VAR123))
  {
    *pb_var = *(((CSS_t_BYTE*)au16_AsmOutIdxLookupTable)
                              +(u32_varCnt - k_OUT_SIZEOF_VAR12));
  }
#endif
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_OUT_SIZEOF_VAR123;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSAI_OutSoftErrVarGet(
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
      CSS_H2N_CPY16(pb_var, &u16_AsmOutCreateIdx);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case 1U:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSS_cfg_NUM_OF_ASM_OUT_INSTS)
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
            CSS_H2N_CPY16(pb_var, &as_AsmOutInst[au32_cnt[1U]].u16_instId);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case 1U:
          {
            CSS_H2N_CPY16(pb_var, &as_AsmOutInst[au32_cnt[1U]].u16_length);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case 2U:
          {
            CSS_H2N_CPY8(pb_var, &as_AsmOutInst[au32_cnt[1U]].b_runIdle);
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
              CSS_H2N_CPY8(pb_var, &as_AsmOutInst[au32_cnt[1U]].ab_data[au32_cnt[3U]]);
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
      if (au32_cnt[1U] >= (sizeof(au16_AsmOutIdxLookupTable)
                          / sizeof(au16_AsmOutIdxLookupTable[0])))
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY16(pb_var, &au16_AsmOutIdxLookupTable[au32_cnt[1U]]);
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

#else
  /* just to avoid lint warning "CSOScfg.h not used" */
  #if (CSOS_k_NOT_APPLICABLE == CSOS_k_NOT_APPLICABLE)
  #endif
#endif  /* (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) */

/*** End Of File ***/

/*lint +efile(766, CSOScfg.h) */


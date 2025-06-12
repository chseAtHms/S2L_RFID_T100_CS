/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVScons.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains functions to consume a Safety I/O message.
**             These routines are derived from the Example Code provided in CIP
**             Networks Library Volume 5.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_ConsLocalsClear
**             IXSVS_ConsColdStartLogic
**             IXSVS_CombinedDataConsumption
**             IXSVS_ConsCnxnStatusGet
**             IXSVS_MaxDataAgeGet
**             IXSVS_MaxDataAgeSet
**             IXSVS_DataAgeGet
**             IXSVS_ConsSoftErrByteGet
**             IXSVS_ConsSoftErrVarGet
**
**             CombinedDataConsumpt2ndStageBfs
**             CombinedDataConsumpt2ndStageBfl
**             CombinedDataConsumpt2ndStageEfs
**             CombinedDataConsumpt2ndStageEfl
**             SingleCastConsumerFunction
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXUTL.h"
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVS.h"
#include "IXSVSint.h"
#include "IXSVSerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* Time_Stamp_Delta is not calculated before Init_Complete_Out is set. Thus it
   must be initialized to a reasonable value. */
#define k_INIT_TIME_STAMP_DELTA    1U


/** t_SVS_LOCALS_CONS:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* S_Run_Idle_Out (see Vol. 5 Chapt. 2-4.6.1.2): shall be used by the
     consuming application to determine if it should put the safety data in a
     safety state.
     Legal values: IXSVD_k_IDLE (0) and IXSVD_k_RUN (1) */
  CSS_t_BYTE        b_S_Run_Idle_Out;

  /* Init_Complete_Out (see Vol. 5 Chapt. 2-4.6.1.3): indicates whether the
     time stamp initialization is complete or not.
     Legal values: 0 and 1 */
  CSS_t_BYTE        b_Init_Complete_Out;

  /* Corrected_Data_Time_Stamp (see Vol. 5 Chapt. 2-4.6.4.4): is the consumer
     copy of the received Data_Time_Stamp.
     Legal range: 0..65535 */
  CSS_t_UINT        u16_Corrected_Data_Time_Stamp_128us;

  /* Last_Data_Time_Stamp (see Vol. 5 Chapt. 2-4.6.4.5): is the saved copy of
     the received Data_Time_Stamp.
     Legal range: 0..65535 */
  CSS_t_UINT        u16_Last_Data_Time_Stamp_128us;

  /* Data_Age (see Vol. 5 Chapt. 2-4.6.4.10): indicates the worst case age of
     the data in 128 uSec increment.
     Legal range: 0..65535 */
  CSS_t_UINT        u16_Data_Age_128us;


  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* Last Time Stamp for Rollover */
    CSS_t_UINT      u16_Last_Time_Stamp_For_Rollover_128us;

    /* Time Stamp Rollover Count */
    CSS_t_UINT      u16_TS_Rollover_Count;

    /* corrected data timestamp for recalculation of the data age */
    CSS_t_UINT      u16_Last_Corrected_Data_Time_Stamp_128us;

    /* Rollover count used in CRC calculations */
    CSS_t_UINT      u16_RC_Used_in_CRC;
  #endif

  /* Max_Data_Age (see Vol. 5 Chapt. 2-4.6.4.11): indicates the maximum value of
     Data_Age since the last time that it was reset.
     Legal range: 0..65535 */
  CSS_t_UINT        u16_Max_Data_Age_128us;

} t_SVS_LOCALS_CONS;

/* array of structs to store the local variables for each instance */
static t_SVS_LOCALS_CONS as_SvsLocals[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)    \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) \
    )
static void CombinedDataConsumpt2ndStageBfs(CSS_t_UINT u16_svsIdx);
#endif
#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) \
    )
static void CombinedDataConsumpt2ndStageBfl(CSS_t_UINT u16_svsIdx);
#endif
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
    )
static CSS_t_BOOL CombinedDataConsumpt2ndStageEfs(CSS_t_UINT u16_svsIdx);
#endif
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
static CSS_t_BOOL CombinedDataConsumpt2ndStageEfl(CSS_t_UINT u16_svsIdx);
#endif
static void SingleCastConsumerFunction(CSS_t_UINT u16_svsIdx,
                                 const IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_in,
                                 IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_out);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_ConsLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_Init(),
**                                 IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsLocalsClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Safety Validator structure */
  CSS_MEMSET(&as_SvsLocals[u16_svsIdx], 0, sizeof(as_SvsLocals[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_ConsColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection to the INITIALIZING state.
**
** Parameters  : u16_svsIdx (IN)        - Index of the Safety Validator Server
**                                        instance
**                                        (not checked, checked in
**                                        IXSVS_TargServerInit(),
**                                        IXSVS_OrigServerInit() and
**                                        IXCCO_CnxnOpen())
**               b_msgFormat (IN)       - Bit-mapped byte of flags to indicate
**                                        what kind of safety connection this
**                                        Safety Validator Instance refers to
**                                        (not checked, only one bit evaluated,
**                                        any value allowed)
**               u16_initialTS_i (IN)   - only used in case of multi-cast (then
**                                        we are definitely Target): Initial
**                                        Time Stamp received in the
**                                        Forward_Open response.
**                                        (not checked, any value allowed)
**               u16_initialRV_i (IN)   - Initial Rollover Value (in) (usage
**                                        identical to pu16_initialTS_i)
**                                        (not checked, any value allowed)
**               pu16_initialTS_o (OUT) - only used in case of single-cast: If
**                                        we are the Originator then this is the
**                                        value to be sent in the Forward_Open
**                                        request. If our node is Target then
**                                        this is the value to be returned in
**                                        the Forward_Open response.
**                                        (not checked, only called with
**                                        reference to variable)
**               pu16_initialRV_o (OUT) - Initial Rollover Value (out) (usage
**                                        identical to pu16_initialTS_o)
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsColdStartLogic(CSS_t_UINT u16_svsIdx
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                             ,CSS_t_BYTE b_msgFormat
                             ,CSS_t_UINT u16_initialTS_i
                             ,CSS_t_UINT u16_initialRV_i
                             ,CSS_t_UINT *pu16_initialTS_o
                             ,CSS_t_UINT *pu16_initialRV_o
#endif
                             )
{
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /*
  //////////////////////////////////////////////////////////////////
  // Cold start after connection establishment processing
  //////////////////////////////////////////////////////////////////
  // This Logic should be executed at the transition of the
  // consuming connection from closed to open.
  //////////////////////////////////////////////////////////////////
  */
  /* S_Con_Flt_C_Out = OK, */
  /* moved to IXSVS_TcorrColdStartLogic() */
  /* Init_Complete_Out = 0,*/
  ps_svs->b_Init_Complete_Out = IXSVS_k_INIT;
  /* S_Run_Idle_Out = Idle, */
  ps_svs->b_S_Run_Idle_Out = IXSVD_k_IDLE;  /* (see FRS278) */
  /* Last_Ping_Count = 3, */
  /* moved to IXSVS_TcooColdStartLogic() */
  /* Time_Coordination_Count_Down = 0x0000, */
  /* moved to IXSVS_TcooColdStartLogic() */
  /* Corrected_Data_Time_Stamp = 0x0000, */
  ps_svs->u16_Corrected_Data_Time_Stamp_128us = 0U;
  /* Last_Data_Time_Stamp = 0x0000, */
  ps_svs->u16_Last_Data_Time_Stamp_128us = 0U;  /* (see FRS300) */
  /* Last_Rcved_Multi_Cast_Active_Idle = Idle, */
  /* moved to IXSVS_TcorrColdStartLogic() */
  /* Last_Rcved_Time_Correction_Value = 0x0000, */
  /* moved to IXSVS_TcorrColdStartLogic() */
  /* Time_Correction_Ping_Interval_Count = 0x0000, */
  /* moved to IXSVS_TcorrColdStartLogic() */
  /* Time_Correction_Received_Flag = 0, */
  /* moved to IXSVS_TcorrColdStartLogic() */
  /* Data_Age = 0x0000, */
  ps_svs->u16_Data_Age_128us = 0U;
  /* Max_Data_Age = 0x0000, */
  ps_svs->u16_Max_Data_Age_128us = 0U; /* (see SRS79) */

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* IF (ExtendedFormat),
  THEN
  */
  /* It makes no difference if the following code is executed for Extended
     Format only or if it is also executed for base format. In case of Base
     Format the variables that are initialized below will never get accessed
     again. */
  {
    /* Consumer_Fault_Counter = 0 */
    /* moved to IXSVS_TcorrColdStartLogic() */
    /* RC_Used_in_CRC = 0x0000 */
    ps_svs->u16_RC_Used_in_CRC = 0x0000U;
    /* IF (Multi-cast),
    THEN
    */
    if (IXSVD_IsMultiCast(b_msgFormat))
    {
      /* Multi-Cast Server -> we are Originator */
      /* Last_Time_Stamp_For_Rollover = Initial_TS from SafetyOpenResponse */
      ps_svs->u16_Last_Time_Stamp_For_Rollover_128us = u16_initialTS_i;
      /* TS_Rollover_Count = Initial_Rollover_Value from SafetyOpenResponse */
      ps_svs->u16_TS_Rollover_Count = u16_initialRV_i;
    }
    /* ENDIF
    IF (Single-cast),
    THEN */
    else
    {
      /* IF (Originator)
      THEN
      {
        Initial_TS for SafetyOpen = Consumer_Clk_Count
        Initial_Rollover_Value for SafetyOpen = Consumer_Clk_Count
          + Initial_RC_Offset
      }
      IF (Target)
      THEN
      {
        Initial_TS for SafetyOpenResponse = Consumer_Clk_Count
        Initial_Rollover_Value for SafetyOpenResponse = Consumer_Clk_Count
          + Initial_RC_Offset
      } */

      /* as the above pseudo code for Originator and Target is identical we can
         do it here just once */
      *pu16_initialTS_o = IXSVS_ConsumerClkCountGet();
      *pu16_initialRV_o = IXSVS_ConsumerClkCountGet();
      *pu16_initialRV_o = (CSS_t_UINT)(*pu16_initialRV_o
                                      + IXSVD_InitRcOffsetIncrementGet());

      /* Initial_RC_Offset = Initial_RC_Offset + 1 */
      /* already incremented by previous function call */
      /* Last_Time_Stamp_For_Rollover = Initial_TS */
      ps_svs->u16_Last_Time_Stamp_For_Rollover_128us = *pu16_initialTS_o;
      /* TS_Rollover_Count = Initial_Rollover_Value */
      ps_svs->u16_TS_Rollover_Count = *pu16_initialRV_o;
    }
    /* ENDIF */

    /* This variable is not initialized by the example code. Thus we set it here
       to the current time, as it is used in calculation of the data age */
    ps_svs->u16_Last_Corrected_Data_Time_Stamp_128us =
      IXSVS_ConsumerClkCountGet();
  }
  /* ELSE: not relevant for base format */
  /* ENDIF */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */

  /* update state machine is to be done by the caller */

  /*
  //////////////////////////////////////////////////////////////////
  // end, Cold start after connection establishment processing
  //////////////////////////////////////////////////////////////////
  */

  /* also initialize Time Coordination and Time Correction specific variables
     (moved from Volume 5 example code to own function in other module for
     reasons of variable/function coherence) */
  IXSVS_TcooColdStartLogic(u16_svsIdx);
  IXSVS_TcorrColdStartLogic(u16_svsIdx);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_CombinedDataConsumption
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE      - Extended Format Safety Validator Server has
**                                 discarded current message due to errors, but
**                                 because Max_Fault_Number is not yet reached
**                                 the connection is still running.
**               CSS_k_FALSE     - else
**
*******************************************************************************/
CSS_t_BOOL IXSVS_CombinedDataConsumption(CSS_t_UINT u16_svsIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* call sub function depending on message format */
  switch ((IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)) & IXSVD_k_MSG_FORMAT_MASK)
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)             /* CCT_NO_PRE_WARNING */
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      case IXSVD_k_MSG_FORMAT_BFS:
      {
        CombinedDataConsumpt2ndStageBfs(u16_svsIdx);
        /* return value already initialized */
        break;
      }
    #endif

    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      case IXSVD_k_MSG_FORMAT_BFL:
      {
        CombinedDataConsumpt2ndStageBfl(u16_svsIdx);
        /* return value already initialized */
        break;
      }
    #endif
  #endif

  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)         /* CCT_NO_PRE_WARNING */
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      case IXSVD_k_MSG_FORMAT_EFS:
      {
        o_retVal = CombinedDataConsumpt2ndStageEfs(u16_svsIdx);
        break;
      }
    #endif
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      case IXSVD_k_MSG_FORMAT_EFL:
      {
        o_retVal = CombinedDataConsumpt2ndStageEfl(u16_svsIdx);
        break;
      }
    #endif
  #endif

    default:
    {
      /* Format not supported */
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_16,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
      /* For Unit Test purpose: code not reachable if all formats are enabled */
      break;
    }
  }

  /*
  //////////////////////////////////////////////////////////////////
  // end Safety Data Consumption
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


/*******************************************************************************
**
** Function    : IXSVS_ConsCnxnStatusGet
**
** Description : This function returns the status of a consuming connection
**               (i.e. Safety Validator Server).
**
** See Also    : CSS_k_CNXN_STATUS_xxx
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in IXSVS_InstInfoGet()
**                                 and IXSVS_RxProc())
**
** Returnvalue : see {CSS_k_CNXN_STATUS_xxx}
**
*******************************************************************************/
CSS_t_USINT IXSVS_ConsCnxnStatusGet(CSS_t_UINT u16_svsIdx)
{
  CSS_t_USINT u8_status = CSS_k_CNXN_STATUS_FAULTED;

  /* (see FRS274) */

  /* if the connection is Faulted */
  if (IXSVS_ConFaultFlagGet(u16_svsIdx) == IXSVD_k_CNXN_FAULTED)
  {
    u8_status = CSS_k_CNXN_STATUS_FAULTED;
  }
  /* else: if connection is Idle */
  else if (as_SvsLocals[u16_svsIdx].b_S_Run_Idle_Out == IXSVD_k_IDLE)
  {
    u8_status = CSS_k_CNXN_STATUS_IDLE;
  }
  /* else: if the initialization of the connection is completed */
  else if (as_SvsLocals[u16_svsIdx].b_Init_Complete_Out == IXSVS_k_COMPL)
  {
    u8_status = CSS_k_CNXN_STATUS_CONS_RUN;
  }
  else /* else: invalid flag combination */
  {
    /* illegal condition - cannot occur */
    IXSVS_ErrorClbk(IXSVS_k_FSE_INC_INV_FLAG_COMB,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_status);
}


/*******************************************************************************
**
** Function    : IXSVS_MaxDataAgeGet
**
** Description : This function returns the Max_Data_Age attribute of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler() and
**                                 IXSVS_InstInfoGet())
**
** Returnvalue : CSS_t_UINT      - Max_Data_Age attribute of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_MaxDataAgeGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return as_SvsLocals[u16_svsIdx].u16_Max_Data_Age_128us;
}


/*******************************************************************************
**
** Function    : IXSVS_MaxDataAgeSet
**
** Description : This function sets the Max_Data_Age attribute of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN)           - index of the Safety Validator
**                                           Server Instance
**                                           (not checked, checked in
**                                           IXSVO_ExplMsgHandler())
**               u16_maxDataAge_128us (IN) - value to be written to the
**                                           Max_Data_Age attribute of the
**                                           requested Safety Validator Server
**                                           (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_MaxDataAgeSet(CSS_t_UINT u16_svsIdx,
                         CSS_t_UINT u16_maxDataAge_128us)
{
  as_SvsLocals[u16_svsIdx].u16_Max_Data_Age_128us = u16_maxDataAge_128us;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_DataAgeGet
**
** Description : This function returns the Data_Age attribute of the requested
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_UINT      - Data_Age attribute of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataAgeGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return as_SvsLocals[u16_svsIdx].u16_Data_Age_128us;
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_ConsSoftErrByteGet
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
CSS_t_UDINT IXSVS_ConsSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_SvsLocals))
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvsLocals)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_SvsLocals);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVS_ConsSoftErrVarGet
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
void IXSVS_ConsSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* level 0 counter at/above end */
    /* default return values already set */
  }
  else /* level 0 counter in range of array elements */
  {
    /* indicate to caller that we are in range of level 1 or below */
    ps_retStat->u8_incLvl = 1U;

    /* select the level 1 variable */
    switch (au32_cnt[1U])
    {
      case 0U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_S_Run_Idle_Out);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 1U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_Init_Complete_Out);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Corrected_Data_Time_Stamp_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 3U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Last_Data_Time_Stamp_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 4U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Data_Age_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 5U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Max_Data_Age_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      case 6U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Last_Time_Stamp_For_Rollover_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 7U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_TS_Rollover_Count);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 8U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Last_Corrected_Data_Time_Stamp_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 9U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_RC_Used_in_CRC);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }
    #endif

      default:
      {
        /* level 1 counter at/above end */
        ps_retStat->u8_cpyLen = 0U;
        break;
      }
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : CombinedDataConsumpt2ndStageBfs
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server with Base Format Short
**               Messages (excluding Ping Count checking).
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void CombinedDataConsumpt2ndStageBfs(CSS_t_UINT u16_svsIdx)
{
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* time stamp of the safety validator server */
  const CSS_t_UINT u16_timeStamp_128us = IXSVS_DataTimeStampGet(u16_svsIdx);
  /* temporary variable for calculating the Time_Stamp_Delta */
  CSS_t_UINT u16_timeStampDelta_128us = 0U;
  /* temporary flags grouped in a structure */
  IXSVS_t_TEMP_CONS_FLAGS s_tempFlags;
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /* initialization of temporary flags */
  CSS_MEMSET(&s_tempFlags, 0, sizeof(s_tempFlags));
  s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_OK;
  s_tempFlags.b_Init_Complete_Out_Temp = IXSVS_k_INIT;
  s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_OK;
  s_tempFlags.b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

  /* if Base Format */
  {
    /* temporary variable for CRC checking */
    /* make sure init values are different from each other */
    CSS_t_USINT u8_expCrc = 0U;
    CSS_t_USINT u8_recCrc = (CSS_t_USINT)(~u8_expCrc);
    /* get a pointer to the data message of the Safety Validator */
    const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);

    u8_recCrc = ps_dataMsg->u8_tCrcS1;

    /* For base format do the Time Stamp CRC checking as the next step */
    /* (see FRS9-2), (see FRS127-2) */
    u8_expCrc = IXCRC_BfMsgTsCrcCalc(ps_dataMsg, kps_initCalc->u8_pidCrcS1);

    /* if calculated CRC doesn't match received CRC */
    if (u8_expCrc != u8_recCrc)
    {
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED; /* (see FRS5) */

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_CRC,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u8_expCrc);
    }
    else
    {
      /* CRC correct */
    }
  }

  /* // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  THEN */
  /* continue checks with Actual and Complemented CRCs and Cross Check */
  /* (see FRS9-4 FRS9-5 FRS9-6) */
  if (!IXSVS_MsgIntegCheckBaseShort(u16_svsIdx))  /* (see FRS127-3) */
  {
    /* Temp_Fault_Flag = Faulted, */
    s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

    /* detailed error was already reported to SAPL in IXSVS_MsgIntegCheck() */
  }
  else
  {
    /* all integrity checks passed */
  }
  /* ENDIF */

  /*
  ** Now follows the Time coordination with the producer (see FRS127-1)
  ** (evaluation of time stamps, see FRS9-3)
  */

  /*
  // Set up Time_Stamp_Delta, to be used to check for out of sequence
  // messages Time_Stamp_Delta is not calculated until the EPI
  // reception after Init_Complete_Out is set to 1.
  IF(Init_Complete_Out == 0),
  THEN
  */
  if (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
  {
    /* Time_Stamp_Delta = 1, */
    u16_timeStampDelta_128us = k_INIT_TIME_STAMP_DELTA;
  }
  /* ELSE */
  else
  {
    /* Time_Stamp_Delta = Time_Stamp_Section.Data_Time_Stamp -
    Last_Data_Time_Stamp, */
    /* calculate the Time_Stamp_Delta: to calculate this value with all 16-bit
       unsigned integers is good enough. Wrap-arounds are considered. There is
       no gain of information when calculating this as a 32-bit integer. There
       are cases where it is doubtable if the message is in correct sequence or
       is one from the past. But this is unavoidable as the only source of
       information is the 16-bit time stamps of the messages. In such cases the
       Data_Age check has to detect such errors. */
    u16_timeStampDelta_128us
      = u16_timeStamp_128us - ps_svs->u16_Last_Data_Time_Stamp_128us;
  }
  /* ENDIF */

  /* if an error was already detected */
  if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
  {
    /* omit the following check of the Time Stamp Delta to prevent that
       multiple errors are thrown when there is just one cause */
    /* This is an extension to the Volume 5 sample code */
  }
  else /* else: not yet an error detected */
  {
    /*
    // Take a look at the time stamp delta and determine if it indicates
    // the message was received in the right order.
    IF ((Time_Stamp_Delta < 0) OR
        (Time_Stamp_Delta > Network_Time_Expectation_Multiplier)),
    THEN
    */
    /* Time_Stamp_Delta is calculated as a 16-bit unsigned integer (see
       explanation above). Thus it cannot be negative. So this line of the
       sample-code doesn't make sense. */
    /* Sample-code doesn't consider Time_Stamp_Delta == 0 (repeated messages) */
    /* if message was repeated */
    if (u16_timeStampDelta_128us == 0U)
    {
      /* This message had the same time stamp as the message before. Either
         this is a repetition of the previous message or the producer has failed
         to increase the time stamp. */
      /* (see FRS84, FRS372) */
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_DELTA,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
    }
    /* else: if delta is greater than the NTEM (NTEM is expected to be > 0) */
    else if (u16_timeStampDelta_128us
        > kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
    {
      /* this message is considered to be out of sequence */
      /* (see FRS299, FRS84, FRS372) */
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */
      /* Temp_Fault_Flag = Faulted,*/
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_NTEM,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u16_timeStampDelta_128us);
    }
    else
    {
      /* ok, consumed message is considered to be in the correct sequence */
    }
    /* ENDIF */
  }

  /* if there was not yet an error detected before */
  /* but for Base Format that is not possible */
  {
    /*
    // execute a function that checks for ping count changes and
    // determines if it is time to produce a time coordination message
    ping_count_check_in_consumer_function(),
    */
    /* The call of IXSVS_TcooPingCountCheck(u16_svsIdx) has been moved before */
    /* this function. This is possible as they are independent but this makes */
    /* it possible to use the same code for Link and Application Triggered    */
    /* Safety Validator Server (though the latter one is not implemented in   */
    /* CSS).                                                                  */

    /* Init_Complete_Out_Temp = 0, */
    /* b_Init_Complete_Out_Temp already set at initialization */

    /* // execute the safety connection type specific functions
    IF (Connection_Type == Multi-cast),
    THEN */
    if (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
      /* multi_cast_consumer_function(), */
      IXSVS_MultiCastConsumerFunction(u16_svsIdx, ps_svs->b_S_Run_Idle_Out,
                        ps_svs->b_Init_Complete_Out, &s_tempFlags,
                        &s_tempFlags,
                        &ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    /* ELSE */
    else
    {
      /* single_cast_consumer_function(), */
      SingleCastConsumerFunction(u16_svsIdx, &s_tempFlags, &s_tempFlags);
    }
    /* ENDIF */

    /* // Save Time stamp for next reception sequence check
    Last_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp, */
    ps_svs->u16_Last_Data_Time_Stamp_128us = u16_timeStamp_128us;

    /* // Calculate the worst case age
    IF (Init_Complete_Out_Temp == 1),
    THEN */
    if (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
    {
      /* (see FRS74) */
      /* Data_Age = Consumer_Clk_Count - Corrected_Data_Time_Stamp, */
      ps_svs->u16_Data_Age_128us = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
                                 - ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    else
    {
      /* connection still in initialization phase */
    }
    /* ENDIF */

    /* // Calculate the maximum worst case age
    IF (Data_Age > Max_Data_Age)
    THEN */
    if (ps_svs->u16_Data_Age_128us
        > ps_svs->u16_Max_Data_Age_128us)
    {
      /* Max_Data_Age = Data_Age, */   /* (see SRS78) */
      ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
    }
    else
    {
      /* current data_age smaller or equal than max_data_age */
    }
    /* ENDIF */

    /* (see FRS80) */
    /* Time Stamp Check starts when Init_Complete_Out is set
       (see FRS286 and FRS306)
    // Check for Age, Integrity, and out of sequence faults
    IF (( unsigned int_16( Data_Age > Network_Time_Expectation_Multiplier)
          AND (Init_Complete_Out_Temp == 1))
          OR (Temp_Fault_Flag == Faulted))
    THEN
    */

    /* (see FRS127-4, FRS81, FRS87-2, FRS288) */
    /* if Data_Age is too old or previous checks failed */
    if (    (    (ps_svs->u16_Data_Age_128us >  /* (see FRS130) */
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
            )
         || (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
       )
    {
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */

      /* (see FRS3, FRS5, FRS86 and FRS288, FRS340) */
      /* // The connection will be closed, the data should not be used
      S_Con_Flt_C_Out_Temp = Faulted, */
      s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_FAULTED;

      /* just for error reporting to SAPL: */
      /* if data age exceeds Network Time Expectation */
      if (    (ps_svs->u16_Data_Age_128us >
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);
      }
      else
      {
        /* nothing to report */
      }

      /* Increment the fault counter */
      IXSVS_StateFaultCountInc();
      /* Update states and notify application (see FRS12-1) */
      /* (see Req.2.5.2.2-2) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
    }
    else
    {
      /* checks passed - no fault detected */
    }
    /* ENDIF */

    /*
    //////////////////////////////////////////////////////////////////
    // The following information is passed on to the consuming application
    // with block integrity: Safety Data,Init_Complete_Out,S_Run_Idle_Out
    // and S_Con_Flt_C_Out.
    // The time between the checking of the age of the data and passing
    // the data off to the consuming application must be protected under
    // the time_stamp check or the consuming application time checks.
    //////////////////////////////////////////////////////////////////
    */
    /* Make Safety Data Available to Consuming Application */
    /* Init_Complete_Out = Init_Complete_Out_Temp, */
    ps_svs->b_Init_Complete_Out =
      s_tempFlags.b_Init_Complete_Out_Temp;
    /* S_Run_Idle_Out = S_Run_Idle_Out_Temp, */
    ps_svs->b_S_Run_Idle_Out =
      s_tempFlags.b_S_Run_Idle_Out_Temp;
    /* S_Con_Flt_C_Out = S_Con_Flt_C_Out_Temp, */
    IXSVS_ConFaultFlagSet(u16_svsIdx, s_tempFlags.b_S_Con_Flt_C_Out_Temp);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

/* This function is strictly based on example code presented in Volume 5. Thus
   the structure should not be altered. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : CombinedDataConsumpt2ndStageBfl
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server with Base Format Long
**               Messages (excluding Ping Count checking).
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void CombinedDataConsumpt2ndStageBfl(CSS_t_UINT u16_svsIdx)
{
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* time stamp of the safety validator server */
  const CSS_t_UINT u16_timeStamp_128us = IXSVS_DataTimeStampGet(u16_svsIdx);
  /* temporary variable for calculating the Time_Stamp_Delta */
  CSS_t_UINT u16_timeStampDelta_128us = 0U;
  /* temporary flags grouped in a structure */
  IXSVS_t_TEMP_CONS_FLAGS s_tempFlags;
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /* initialization of temporary flags */
  CSS_MEMSET(&s_tempFlags, 0, sizeof(s_tempFlags));
  s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_OK;
  s_tempFlags.b_Init_Complete_Out_Temp = IXSVS_k_INIT;
  s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_OK;
  s_tempFlags.b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

  /* if Base Format */
  {
    /* temporary variable for CRC checking */
    /* make sure init values are different from each other */
    CSS_t_USINT u8_expCrc = 0U;
    CSS_t_USINT u8_recCrc = (CSS_t_USINT)(~u8_expCrc);
    /* get a pointer to the data message of the Safety Validator */
    const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);

    u8_recCrc = ps_dataMsg->u8_tCrcS1;

    /* For base format do the Time Stamp CRC checking as the next step */
    /* (see FRS9-2), (see FRS127-2) */
    u8_expCrc = IXCRC_BfMsgTsCrcCalc(ps_dataMsg, kps_initCalc->u8_pidCrcS1);

    /* if calculated CRC doesn't match received CRC */
    if (u8_expCrc != u8_recCrc)
    {
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED; /* (see FRS5) */

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_CRC,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u8_expCrc);
    }
    else
    {
      /* CRC correct */
    }
  }

  /* // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  THEN */
  /* continue checks with Actual and Complemented CRCs and Cross Check */
  /* (see FRS9-4 FRS9-5 FRS9-6) */
  if (!IXSVS_MsgIntegCheckBaseLong(u16_svsIdx))  /* (see FRS127-3) */
  {
    /* Temp_Fault_Flag = Faulted, */
    s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

    /* detailed error was already reported to SAPL in IXSVS_MsgIntegCheck() */
  }
  else
  {
    /* all integrity checks passed */
  }
  /* ENDIF */

  /*
  ** Now follows the Time coordination with the producer (see FRS127-1)
  ** (evaluation of time stamps, see FRS9-3)
  */

  /*
  // Set up Time_Stamp_Delta, to be used to check for out of sequence
  // messages Time_Stamp_Delta is not calculated until the EPI
  // reception after Init_Complete_Out is set to 1.
  IF(Init_Complete_Out == 0),
  THEN
  */
  if (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
  {
    /* Time_Stamp_Delta = 1, */
    u16_timeStampDelta_128us = k_INIT_TIME_STAMP_DELTA;
  }
  /* ELSE */
  else
  {
    /* Time_Stamp_Delta = Time_Stamp_Section.Data_Time_Stamp -
    Last_Data_Time_Stamp, */
    /* calculate the Time_Stamp_Delta: to calculate this value with all 16-bit
       unsigned integers is good enough. Wrap-arounds are considered. There is
       no gain of information when calculating this as a 32-bit integer. There
       are cases where it is doubtable if the message is in correct sequence or
       is one from the past. But this is unavoidable as the only source of
       information is the 16-bit time stamps of the messages. In such cases the
       Data_Age check has to detect such errors. */
    u16_timeStampDelta_128us
      = u16_timeStamp_128us - ps_svs->u16_Last_Data_Time_Stamp_128us;
  }
  /* ENDIF */

  /* if an error was already detected */
  if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
  {
    /* omit the following check of the Time Stamp Delta to prevent that
       multiple errors are thrown when there is just one cause */
    /* This is an extension to the Volume 5 sample code */
  }
  else /* else: not yet an error detected */
  {
    /*
    // Take a look at the time stamp delta and determine if it indicates
    // the message was received in the right order.
    IF ((Time_Stamp_Delta < 0) OR
        (Time_Stamp_Delta > Network_Time_Expectation_Multiplier)),
    THEN
    */
    /* Time_Stamp_Delta is calculated as a 16-bit unsigned integer (see
       explanation above). Thus it cannot be negative. So this line of the
       sample-code doesn't make sense. */
    /* Sample-code doesn't consider Time_Stamp_Delta == 0 (repeated messages) */
    /* if message was repeated */
    if (u16_timeStampDelta_128us == 0U)
    {
      /* This message had the same time stamp as the message before. Either
         this is a repetition of the previous message or the producer has failed
         to increase the time stamp. */
      /* (see FRS84, FRS372) */
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_DELTA,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
    }
    /* else: if delta is greater than the NTEM (NTEM is expected to be > 0) */
    else if (u16_timeStampDelta_128us
        > kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
    {
      /* this message is considered to be out of sequence */
      /* (see FRS299, FRS84, FRS372) */
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */
      /* Temp_Fault_Flag = Faulted,*/
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_NTEM,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u16_timeStampDelta_128us);
    }
    else
    {
      /* ok, consumed message is considered to be in the correct sequence */
    }
    /* ENDIF */
  }

  /* if there was not yet an error detected before */
  /* but for Base Format that is not possible */
  {
    /*
    // execute a function that checks for ping count changes and
    // determines if it is time to produce a time coordination message
    ping_count_check_in_consumer_function(),
    */
    /* The call of IXSVS_TcooPingCountCheck(u16_svsIdx) has been moved before */
    /* this function. This is possible as they are independent but this makes */
    /* it possible to use the same code for Link and Application Triggered    */
    /* Safety Validator Server (though the latter one is not implemented in   */
    /* CSS).                                                                  */

    /* Init_Complete_Out_Temp = 0, */
    /* b_Init_Complete_Out_Temp already set at initialization */

    /* // execute the safety connection type specific functions
    IF (Connection_Type == Multi-cast),
    THEN */
    if (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
      /* multi_cast_consumer_function(), */
      IXSVS_MultiCastConsumerFunction(u16_svsIdx, ps_svs->b_S_Run_Idle_Out,
                        ps_svs->b_Init_Complete_Out, &s_tempFlags,
                        &s_tempFlags,
                        &ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    /* ELSE */
    else
    {
      /* single_cast_consumer_function(), */
      SingleCastConsumerFunction(u16_svsIdx, &s_tempFlags, &s_tempFlags);
    }
    /* ENDIF */


    /* // Save Time stamp for next reception sequence check
    Last_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp, */
    ps_svs->u16_Last_Data_Time_Stamp_128us = u16_timeStamp_128us;

    /* // Calculate the worst case age
    IF (Init_Complete_Out_Temp == 1),
    THEN */
    if (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
    {
      /* (see FRS74) */
      /* Data_Age = Consumer_Clk_Count - Corrected_Data_Time_Stamp, */
      ps_svs->u16_Data_Age_128us = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
                                 - ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    else
    {
      /* connection still in initialization phase */
    }
    /* ENDIF */

    /* // Calculate the maximum worst case age
    IF (Data_Age > Max_Data_Age)
    THEN */
    if (ps_svs->u16_Data_Age_128us
        > ps_svs->u16_Max_Data_Age_128us)
    {
      /* Max_Data_Age = Data_Age, */   /* (see SRS78) */
      ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
    }
    else
    {
      /* current data_age smaller or equal than max_data_age */
    }
    /* ENDIF */

    /* (see FRS80) */
    /* Time Stamp Check starts when Init_Complete_Out is set
       (see FRS286 and FRS306)
    // Check for Age, Integrity, and out of sequence faults
    IF (( unsigned int_16( Data_Age > Network_Time_Expectation_Multiplier)
          AND (Init_Complete_Out_Temp == 1))
          OR (Temp_Fault_Flag == Faulted))
    THEN
    */

    /* (see FRS127-4, FRS81, FRS87-2, FRS288) */
    /* if Data_Age is too old or previous checks failed */
    if (    (    (ps_svs->u16_Data_Age_128us >  /* (see FRS130) */
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
            )
         || (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
       )
    {
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */

      /* (see FRS3, FRS5, FRS86 and FRS288, FRS340) */
      /* // The connection will be closed, the data should not be used
      S_Con_Flt_C_Out_Temp = Faulted, */
      s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_FAULTED;

      /* just for error reporting to SAPL: */
      /* if data age exceeds Network Time Expectation */
      if (    (ps_svs->u16_Data_Age_128us >
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);
      }
      else
      {
        /* nothing to report */
      }

      /* Increment the fault counter */
      IXSVS_StateFaultCountInc();
      /* Update states and notify application (see FRS12-1) */
      /* (see Req.2.5.2.2-2) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
    }
    else
    {
      /* checks passed - no fault detected */
    }
    /* ENDIF */

    /*
    //////////////////////////////////////////////////////////////////
    // The following information is passed on to the consuming application
    // with block integrity: Safety Data,Init_Complete_Out,S_Run_Idle_Out
    // and S_Con_Flt_C_Out.
    // The time between the checking of the age of the data and passing
    // the data off to the consuming application must be protected under
    // the time_stamp check or the consuming application time checks.
    //////////////////////////////////////////////////////////////////
    */
    /* Make Safety Data Available to Consuming Application */
    /* Init_Complete_Out = Init_Complete_Out_Temp, */
    ps_svs->b_Init_Complete_Out =
      s_tempFlags.b_Init_Complete_Out_Temp;
    /* S_Run_Idle_Out = S_Run_Idle_Out_Temp, */
    ps_svs->b_S_Run_Idle_Out =
      s_tempFlags.b_S_Run_Idle_Out_Temp;
    /* S_Con_Flt_C_Out = S_Con_Flt_C_Out_Temp, */
    IXSVS_ConFaultFlagSet(u16_svsIdx, s_tempFlags.b_S_Con_Flt_C_Out_Temp);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

/* This function is strictly based on example code presented in Volume 5. Thus
   the structure should not be altered. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : CombinedDataConsumpt2ndStageEfs
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server with Extended Format
**               Short Messages (excluding Ping Count checking).
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE      - Extended Format Safety Validator Server has
**                                 discarded current message due to errors, but
**                                 because Max_Fault_Number is not yet reached
**                                 the connection is still running.
**               CSS_k_FALSE     - else
**
*******************************************************************************/
static CSS_t_BOOL CombinedDataConsumpt2ndStageEfs(CSS_t_UINT u16_svsIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* time stamp of the safety validator server */
  const CSS_t_UINT u16_timeStamp_128us = IXSVS_DataTimeStampGet(u16_svsIdx);
  /* temporary variable for calculating the Time_Stamp_Delta */
  CSS_t_UINT u16_timeStampDelta_128us = 0U;
  /* flag indicating that further processing of the message must be aborted */
  CSS_t_BOOL o_abortProcessing = CSS_k_FALSE;
  /* temporary flags grouped in a structure */
  IXSVS_t_TEMP_CONS_FLAGS s_tempFlags;
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  CSS_t_UDINT u32_pidRcCrcS5 = 0U;

  /* initialization of temporary flags */
  CSS_MEMSET(&s_tempFlags, 0, sizeof(s_tempFlags));
  s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_OK;
  s_tempFlags.b_Init_Complete_Out_Temp = IXSVS_k_INIT;
  s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_OK;
  s_tempFlags.b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

  /*
  // If the extended format is being used, we must determine a rollover value to
  // be used when calculating CRCs before we attempt to validate the message
  // (since the rollover is part of CRC values in the message).
  IF (ExtendedFormat),
  THEN */
  {
    /* mode byte of the safety validator server */
    const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);

    /*
    // For single-cast, a time_stamp other than 0 or a Run
    // indication, indicate that the Time_Coordination_Message has been
    // received by the Client and the corrected time stamp is being used.
    IF ((SingleCast) AND (Init_Complete_Out == 0) AND
        (Time_Stamp_Section.Data_Time_Stamp == 0) AND
        (Mode_Byte.Run_Idle == IDLE)),
    THEN
    */
    if (    (IXSVD_IsSingleCast(
               IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))

         && (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
         && (u16_timeStamp_128us == 0U)
         && (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_RUN_IDLE)
             == IXSVD_k_IDLE)
       )
    {
      /* RC_Used_in_CRC = 0x0000, */
      ps_svs->u16_RC_Used_in_CRC = 0U;  /* (see FRS376) */
    }
    /* ELSE */
    else
    {
      /* RC_Used_in_CRC = TS_Rollover_Count, */
      ps_svs->u16_RC_Used_in_CRC = ps_svs->u16_TS_Rollover_Count;

      /* // check for time stamp rollover
      IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp <
         Last_Time_Stamp_For_Rollover))
      THEN */
      if (u16_timeStamp_128us < ps_svs->u16_Last_Time_Stamp_For_Rollover_128us)
      {
        /* RC_Used_in_CRC = RC_Used_in_CRC + 1, */
        ps_svs->u16_RC_Used_in_CRC++;
      }
      else
      {
        /* not yet time for rollover */
      }
      /* ENDIF */
    }
    /* ENDIF */

    u32_pidRcCrcS5 = IXCRC_PidRcCrcS5Calc(ps_svs->u16_RC_Used_in_CRC,
                                          kps_initCalc->u32_pidCrcS5);
  }

  /* // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  THEN */
  /* continue checks with Actual and Complemented CRCs and Cross Check */
  /* (see FRS9-4 FRS9-5 FRS9-6) */
  if (!IXSVS_MsgIntegCheckExtShort(u16_svsIdx,
                                   u32_pidRcCrcS5))  /* (see FRS127-3) */
  {
    /* Temp_Fault_Flag = Faulted, */
    s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

    /* detailed error was already reported to SAPL in IXSVS_MsgIntegCheck() */
  }
  else
  {
    /* all integrity checks passed */
  }
  /* ENDIF */

  /*
  ** Now follows the Time coordination with the producer (see FRS127-1)
  ** (evaluation of time stamps, see FRS9-3)
  */

  /*
  // Set up Time_Stamp_Delta, to be used to check for out of sequence
  // messages Time_Stamp_Delta is not calculated until the EPI
  // reception after Init_Complete_Out is set to 1.
  IF(Init_Complete_Out == 0),
  THEN
  */
  if (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
  {
    /* Time_Stamp_Delta = 1, */
    u16_timeStampDelta_128us = k_INIT_TIME_STAMP_DELTA;
  }
  /* ELSE */
  else
  {
    /* Time_Stamp_Delta = Time_Stamp_Section.Data_Time_Stamp -
    Last_Data_Time_Stamp, */
    /* calculate the Time_Stamp_Delta: to calculate this value with all 16-bit
       unsigned integers is good enough. Wrap-arounds are considered. There is
       no gain of information when calculating this as a 32-bit integer. There
       are cases where it is doubtable if the message is in correct sequence or
       is one from the past. But this is unavoidable as the only source of
       information is the 16-bit time stamps of the messages. In such cases the
       Data_Age check has to detect such errors. */
    u16_timeStampDelta_128us
      = u16_timeStamp_128us - ps_svs->u16_Last_Data_Time_Stamp_128us;
  }
  /* ENDIF */

  /* if an error was already detected */
  if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
  {
    /* omit the following check of the Time Stamp Delta to prevent that
       multiple errors are thrown when there is just one cause */
    /* This is an extension to the Volume 5 sample code */
  }
  else /* else: not yet an error detected */
  {
    /*
    // Take a look at the time stamp delta and determine if it indicates
    // the message was received in the right order.
    IF ((Time_Stamp_Delta < 0) OR
        (Time_Stamp_Delta > Network_Time_Expectation_Multiplier)),
    THEN
    */
    /* Time_Stamp_Delta is calculated as a 16-bit unsigned integer (see
       explanation above). Thus it cannot be negative. So this line of the
       sample-code doesn't make sense. */
    /* Sample-code doesn't consider Time_Stamp_Delta == 0 (repeated messages) */
    /* if message was repeated */
    if (u16_timeStampDelta_128us == 0U)
    {
      /* This message had the same time stamp as the message before. Either
         this is a repetition of the previous message or the producer has failed
         to increase the time stamp. */
      /* (see FRS84, FRS372) */
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_DELTA,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
    }
    /* else: if delta is greater than the NTEM (NTEM is expected to be > 0) */
    else if (u16_timeStampDelta_128us
        > kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
    {
      /* this message is considered to be out of sequence */
      /* (see FRS299, FRS84, FRS372) */
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */
      /* Temp_Fault_Flag = Faulted,*/
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_NTEM,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u16_timeStampDelta_128us);
    }
    else
    {
      /* ok, consumed message is considered to be in the correct sequence */
    }
    /* ENDIF */
  }


  /*
  // If the extended format is being used, we can drop some packets with
  // data integrity/message validation or out of order errors.
  IF ((ExtendedFormat) AND (Temp_Fault_Flag == Faulted)),
  THEN
  */
  {
    if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
    {
      /* increment Consumer_Fault_Counter */
      CSS_t_USINT u8_consFaultCount =
        IXSVS_ConsFaultCounterIncrement(u16_svsIdx);

      /* // Calculate updated data age for the data in the last good packet
      IF (Init_Complete_Out)
      THEN */
      if (ps_svs->b_Init_Complete_Out != IXSVS_k_INIT)
      {
        /* Data_Age = Consumer_Clk_Count - Last_Corrected_Data_Time_Stamp, */
        /* (see FRS74) */
        ps_svs->u16_Data_Age_128us
          = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
          - ps_svs->u16_Last_Corrected_Data_Time_Stamp_128us);
      }
      else
      {
        /* time stamp initialization still in progress */
      }
      /* ENDIF */

      /* // Calculate the maximum worst case age
      IF (Data_Age > Max_Data_Age)
      THEN */
      if (ps_svs->u16_Data_Age_128us > ps_svs->u16_Max_Data_Age_128us)
      {
        /* Max_Data_Age = Data_Age, */  /* (see SRS78) */
        ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
      }
      else
      {
        /* current data_age smaller or equal than max_data_age */
      }
      /* ENDIF */

      /*
      // Is it OK to ignore this error?
      // It is OK if the consumer fault counter is not greater than the
      // maximum faults allowed and the updated data age from the
      // last good packet still meets our data age criteria
      // (less than the time expectation multiplier)
      IF (
           (Consumer_Fault_Counter < Max_Fault_Number) AND
           (Data_Age <= Network_Time_Expectation_Multiplier)
         )
      THEN
      */
      /* (see FRS127-4, FRS81, FRS86, FRS87-2, FRS288) */
      if (    (u8_consFaultCount < kps_initCalc->u8_Max_Fault_Number)
           && (ps_svs->u16_Data_Age_128us
                <= kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
         )
      {
        /* Currently processed message will be discarded. But previously
           received data is still considered to be ok. */

        /* Abort the processing of this packet. Do Not close the connection.  */
        /* (see FRS5) */
        o_abortProcessing = CSS_k_TRUE;

        /* Do not use the Data. */
        /* Indicate to the caller that this message must be discarded */
        o_retVal = CSS_k_TRUE;
        /* Do not update Last_Corrected_Data_Time_Stamp or                    */
        /* Last_Data_Time_Stamp */

        /* The reason why this message has been qualified as faulted was
           reported to SAPL already at the point where this was detected */
      }
      /* ELSE */
      else
      {
        /* S_Con_Flt_C_Out = Faulted, */
        /* (see FRS340) */
        IXSVS_ConFaultFlagSet(u16_svsIdx, IXSVD_k_CNXN_FAULTED);

        /* Abort the processing of this packet. Close the connection. */
        /* (see FRS3 and FRS5) */
        o_abortProcessing = CSS_k_TRUE;

        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);

        /* Increment global IXSVS fault counter */
        IXSVS_StateFaultCountInc();

        /* Update states and notify application (see FRS12-1) */
        /* (see Req.2.5.2.2-2) */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
      }
      /* ENDIF */
    }
    else
    {
      /* connection is not faulted */
    }
  }


  /* if there was not yet an error detected before */
  /* suppress lint warning: always false in case only Base Format supported */
  if (!o_abortProcessing)                                        /*lint !e774 */
  {
    /*
    // execute a function that checks for ping count changes and
    // determines if it is time to produce a time coordination message
    ping_count_check_in_consumer_function(),
    */
    /* The call of IXSVS_TcooPingCountCheck(u16_svsIdx) has been moved before */
    /* this function. This is possible as they are independent but this makes */
    /* it possible to use the same code for Link and Application Triggered    */
    /* Safety Validator Server (though the latter one is not implemented in   */
    /* CSS).                                                                  */

    /* Init_Complete_Out_Temp = 0, */
    /* b_Init_Complete_Out_Temp already set at initialization */

    /* // execute the safety connection type specific functions
    IF (Connection_Type == Multi-cast),
    THEN */
    if (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
      /* multi_cast_consumer_function(), */
      IXSVS_MultiCastConsumerFunction(u16_svsIdx, ps_svs->b_S_Run_Idle_Out,
                        ps_svs->b_Init_Complete_Out, &s_tempFlags,
                        &s_tempFlags,
                        &ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    /* ELSE */
    else
    {
      /* single_cast_consumer_function(), */
      SingleCastConsumerFunction(u16_svsIdx, &s_tempFlags, &s_tempFlags);
    }
    /* ENDIF */

    /*
    // If we are using the ExtendedFormat, at this point we have a properly
    // formatted message that was received in the right order. With the
    // extended format update the rollover count and last timestamp for
    // rollover.
    IF (ExtendedFormat),
    THEN
    */
    {
      /*
      // Only update the rollover count if this is a multicast connection
      // or if this is a singlecast connection that is out of the initialization
      // phase.
      IF ((Connection_Type == Multi-cast) OR
          (Init_Complete_Out_Temp == 1)),
      THEN
      */
      if (    (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
           || (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        /*
        // Check the timestamp value in the message to determine if a
        // rollover has occurred, and if so, update the rollover count
        // accordingly.
        IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp <
             Last_Time_Stamp_For_Rollover))
        THEN
        */
        if (u16_timeStamp_128us
            < ps_svs->u16_Last_Time_Stamp_For_Rollover_128us)
        {
          /* TS_Rollover_Count = TS_Rollover_Count + 1, */
          ps_svs->u16_TS_Rollover_Count++;
        }
        else
        {
          /* not yet time for a rollover */
        }
        /* ENDIF */

        /* Last_Time_Stamp_For_Rollover = Time_Stamp_Section.Data_Time_Stamp, */
        ps_svs->u16_Last_Time_Stamp_For_Rollover_128us = u16_timeStamp_128us;
      }
      else
      {
        /* not relevant for single-cast cnxns in initialization phase */
      }
      /* ENDIF */

      /* // Save the corrected data timestamp so we can recalculate a data age
      // if the next packet has errors.
      Last_Corrected_Data_Time_Stamp = Corrected_Data_Time_Stamp, */
      ps_svs->u16_Last_Corrected_Data_Time_Stamp_128us =
        ps_svs->u16_Corrected_Data_Time_Stamp_128us;
    }


    /* // Save Time stamp for next reception sequence check
    Last_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp, */
    ps_svs->u16_Last_Data_Time_Stamp_128us = u16_timeStamp_128us;

    /* // Calculate the worst case age
    IF (Init_Complete_Out_Temp == 1),
    THEN */
    if (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
    {
      /* (see FRS74) */
      /* Data_Age = Consumer_Clk_Count - Corrected_Data_Time_Stamp, */
      ps_svs->u16_Data_Age_128us = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
                                 - ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    else
    {
      /* connection still in initialization phase */
    }
    /* ENDIF */

    /* // Calculate the maximum worst case age
    IF (Data_Age > Max_Data_Age)
    THEN */
    if (ps_svs->u16_Data_Age_128us
        > ps_svs->u16_Max_Data_Age_128us)
    {
      /* Max_Data_Age = Data_Age, */   /* (see SRS78) */
      ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
    }
    else
    {
      /* current data_age smaller or equal than max_data_age */
    }
    /* ENDIF */

    /* (see FRS80) */
    /* Time Stamp Check starts when Init_Complete_Out is set
       (see FRS286 and FRS306)
    // Check for Age, Integrity, and out of sequence faults
    IF (( unsigned int_16( Data_Age > Network_Time_Expectation_Multiplier)
          AND (Init_Complete_Out_Temp == 1))
          OR (Temp_Fault_Flag == Faulted))
    THEN
    */

    /* (see FRS127-4, FRS81, FRS87-2, FRS288) */
    /* if Data_Age is too old or previous checks failed */
    if (    (    (ps_svs->u16_Data_Age_128us >  /* (see FRS130) */
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
            )
         || (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
       )
    {
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */

      /* (see FRS3, FRS5, FRS86 and FRS288, FRS340) */
      /* // The connection will be closed, the data should not be used
      S_Con_Flt_C_Out_Temp = Faulted, */
      s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_FAULTED;

      /* just for error reporting to SAPL: */
      /* if data age exceeds Network Time Expectation */
      if (    (ps_svs->u16_Data_Age_128us >
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);
      }
      else
      {
        /* nothing to report */
      }

      /* Increment the fault counter */
      IXSVS_StateFaultCountInc();
      /* Update states and notify application (see FRS12-1) */
      /* (see Req.2.5.2.2-2) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
    }
    else
    {
      /* checks passed - no fault detected */
    }
    /* ENDIF */

    /*
    //////////////////////////////////////////////////////////////////
    // The following information is passed on to the consuming application
    // with block integrity: Safety Data,Init_Complete_Out,S_Run_Idle_Out
    // and S_Con_Flt_C_Out.
    // The time between the checking of the age of the data and passing
    // the data off to the consuming application must be protected under
    // the time_stamp check or the consuming application time checks.
    //////////////////////////////////////////////////////////////////
    */
    /* Make Safety Data Available to Consuming Application */
    /* Init_Complete_Out = Init_Complete_Out_Temp, */
    ps_svs->b_Init_Complete_Out =
      s_tempFlags.b_Init_Complete_Out_Temp;
    /* S_Run_Idle_Out = S_Run_Idle_Out_Temp, */
    ps_svs->b_S_Run_Idle_Out =
      s_tempFlags.b_S_Run_Idle_Out_Temp;
    /* S_Con_Flt_C_Out = S_Con_Flt_C_Out_Temp, */
    IXSVS_ConFaultFlagSet(u16_svsIdx, s_tempFlags.b_S_Con_Flt_C_Out_Temp);
  }
  else
  {
    /* processing already aborted */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);

/* This function is strictly based on example code presented in Volume 5. Thus
   the structure should not be altered. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */




#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : CombinedDataConsumpt2ndStageEfl
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server with Extended Format
**               Long Messages (excluding Ping Count checking).
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE      - Extended Format Safety Validator Server has
**                                 discarded current message due to errors, but
**                                 because Max_Fault_Number is not yet reached
**                                 the connection is still running.
**               CSS_k_FALSE     - else
**
*******************************************************************************/
static CSS_t_BOOL CombinedDataConsumpt2ndStageEfl(CSS_t_UINT u16_svsIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* time stamp of the safety validator server */
  const CSS_t_UINT u16_timeStamp_128us = IXSVS_DataTimeStampGet(u16_svsIdx);
  /* temporary variable for calculating the Time_Stamp_Delta */
  CSS_t_UINT u16_timeStampDelta_128us = 0U;
  /* flag indicating that further processing of the message must be aborted */
  CSS_t_BOOL o_abortProcessing = CSS_k_FALSE;
  /* temporary flags grouped in a structure */
  IXSVS_t_TEMP_CONS_FLAGS s_tempFlags;
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  CSS_t_UINT u16_pidRcCrcS3 = 0U;
  CSS_t_UDINT u32_pidRcCrcS5 = 0U;

  /* initialization of temporary flags */
  CSS_MEMSET(&s_tempFlags, 0, sizeof(s_tempFlags));
  s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_OK;
  s_tempFlags.b_Init_Complete_Out_Temp = IXSVS_k_INIT;
  s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_OK;
  s_tempFlags.b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

  /*
  // If the extended format is being used, we must determine a rollover value to
  // be used when calculating CRCs before we attempt to validate the message
  // (since the rollover is part of CRC values in the message).
  IF (ExtendedFormat),
  THEN */
  {
    /* mode byte of the safety validator server */
    const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);

    /*
    // For single-cast, a time_stamp other than 0 or a Run
    // indication, indicate that the Time_Coordination_Message has been
    // received by the Client and the corrected time stamp is being used.
    IF ((SingleCast) AND (Init_Complete_Out == 0) AND
        (Time_Stamp_Section.Data_Time_Stamp == 0) AND
        (Mode_Byte.Run_Idle == IDLE)),
    THEN
    */
    if (    (IXSVD_IsSingleCast(
               IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))

         && (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
         && (u16_timeStamp_128us == 0U)
         && (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_RUN_IDLE)
             == IXSVD_k_IDLE)
       )
    {
      /* RC_Used_in_CRC = 0x0000, */
      ps_svs->u16_RC_Used_in_CRC = 0U;  /* (see FRS376) */
    }
    /* ELSE */
    else
    {
      /* RC_Used_in_CRC = TS_Rollover_Count, */
      ps_svs->u16_RC_Used_in_CRC = ps_svs->u16_TS_Rollover_Count;

      /* // check for time stamp rollover
      IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp <
         Last_Time_Stamp_For_Rollover))
      THEN */
      if (u16_timeStamp_128us < ps_svs->u16_Last_Time_Stamp_For_Rollover_128us)
      {
        /* RC_Used_in_CRC = RC_Used_in_CRC + 1, */
        ps_svs->u16_RC_Used_in_CRC++;
      }
      else
      {
        /* not yet time for rollover */
      }
      /* ENDIF */
    }
    /* ENDIF */

    /* recalculate PID/RolloverCount CRC seeds */
    u16_pidRcCrcS3 = IXCRC_PidRcCrcS3Calc(ps_svs->u16_RC_Used_in_CRC,
                                            kps_initCalc->u16_pidCrcS3);
    u32_pidRcCrcS5 = IXCRC_PidRcCrcS5Calc(ps_svs->u16_RC_Used_in_CRC,
                                          kps_initCalc->u32_pidCrcS5);
  }


  /* // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  THEN */
  /* continue checks with Actual and Complemented CRCs and Cross Check */
  /* (see FRS9-4 FRS9-5 FRS9-6) */
  if (!IXSVS_MsgIntegCheckExtLong(u16_svsIdx,
                                  u32_pidRcCrcS5,
                                  u16_pidRcCrcS3))  /* (see FRS127-3) */
  {
    /* Temp_Fault_Flag = Faulted, */
    s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

    /* detailed error was already reported to SAPL in IXSVS_MsgIntegCheck() */
  }
  else
  {
    /* all integrity checks passed */
  }
  /* ENDIF */

  /*
  ** Now follows the Time coordination with the producer (see FRS127-1)
  ** (evaluation of time stamps, see FRS9-3)
  */

  /*
  // Set up Time_Stamp_Delta, to be used to check for out of sequence
  // messages Time_Stamp_Delta is not calculated until the EPI
  // reception after Init_Complete_Out is set to 1.
  IF(Init_Complete_Out == 0),
  THEN
  */
  if (ps_svs->b_Init_Complete_Out == IXSVS_k_INIT)
  {
    /* Time_Stamp_Delta = 1, */
    u16_timeStampDelta_128us = k_INIT_TIME_STAMP_DELTA;
  }
  /* ELSE */
  else
  {
    /* Time_Stamp_Delta = Time_Stamp_Section.Data_Time_Stamp -
    Last_Data_Time_Stamp, */
    /* calculate the Time_Stamp_Delta: to calculate this value with all 16-bit
       unsigned integers is good enough. Wrap-arounds are considered. There is
       no gain of information when calculating this as a 32-bit integer. There
       are cases where it is doubtable if the message is in correct sequence or
       is one from the past. But this is unavoidable as the only source of
       information is the 16-bit time stamps of the messages. In such cases the
       Data_Age check has to detect such errors. */
    u16_timeStampDelta_128us
      = u16_timeStamp_128us - ps_svs->u16_Last_Data_Time_Stamp_128us;
  }
  /* ENDIF */

  /* if an error was already detected */
  if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
  {
    /* omit the following check of the Time Stamp Delta to prevent that
       multiple errors are thrown when there is just one cause */
    /* This is an extension to the Volume 5 sample code */
  }
  else /* else: not yet an error detected */
  {
    /*
    // Take a look at the time stamp delta and determine if it indicates
    // the message was received in the right order.
    IF ((Time_Stamp_Delta < 0) OR
        (Time_Stamp_Delta > Network_Time_Expectation_Multiplier)),
    THEN
    */
    /* Time_Stamp_Delta is calculated as a 16-bit unsigned integer (see
       explanation above). Thus it cannot be negative. So this line of the
       sample-code doesn't make sense. */
    /* Sample-code doesn't consider Time_Stamp_Delta == 0 (repeated messages) */
    /* if message was repeated */
    if (u16_timeStampDelta_128us == 0U)
    {
      /* This message had the same time stamp as the message before. Either
         this is a repetition of the previous message or the producer has failed
         to increase the time stamp. */
      /* (see FRS84, FRS372) */
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_DELTA,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
    }
    /* else: if delta is greater than the NTEM (NTEM is expected to be > 0) */
    else if (u16_timeStampDelta_128us
        > kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
    {
      /* this message is considered to be out of sequence */
      /* (see FRS299, FRS84, FRS372) */
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */
      /* Temp_Fault_Flag = Faulted,*/
      s_tempFlags.b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TSTAMP_NTEM,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u16_timeStampDelta_128us);
    }
    else
    {
      /* ok, consumed message is considered to be in the correct sequence */
    }
    /* ENDIF */
  }


  /*
  // If the extended format is being used, we can drop some packets with
  // data integrity/message validation or out of order errors.
  IF ((ExtendedFormat) AND (Temp_Fault_Flag == Faulted)),
  THEN
  */
  {
    if (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
    {
      /* increment Consumer_Fault_Counter */
      CSS_t_USINT u8_consFaultCount =
        IXSVS_ConsFaultCounterIncrement(u16_svsIdx);

      /* // Calculate updated data age for the data in the last good packet
      IF (Init_Complete_Out)
      THEN */
      if (ps_svs->b_Init_Complete_Out != IXSVS_k_INIT)
      {
        /* Data_Age = Consumer_Clk_Count - Last_Corrected_Data_Time_Stamp, */
        /* (see FRS74) */
        ps_svs->u16_Data_Age_128us
          = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
          - ps_svs->u16_Last_Corrected_Data_Time_Stamp_128us);
      }
      else
      {
        /* time stamp initialization still in progress */
      }
      /* ENDIF */

      /* // Calculate the maximum worst case age
      IF (Data_Age > Max_Data_Age)
      THEN */
      if (ps_svs->u16_Data_Age_128us > ps_svs->u16_Max_Data_Age_128us)
      {
        /* Max_Data_Age = Data_Age, */  /* (see SRS78) */
        ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
      }
      else
      {
        /* current data_age smaller or equal than max_data_age */
      }
      /* ENDIF */

      /*
      // Is it OK to ignore this error?
      // It is OK if the consumer fault counter is not greater than the
      // maximum faults allowed and the updated data age from the
      // last good packet still meets our data age criteria
      // (less than the time expectation multiplier)
      IF (
           (Consumer_Fault_Counter < Max_Fault_Number) AND
           (Data_Age <= Network_Time_Expectation_Multiplier)
         )
      THEN
      */
      /* (see FRS127-4, FRS81, FRS86, FRS87-2, FRS288) */
      if (    (u8_consFaultCount < kps_initCalc->u8_Max_Fault_Number)
           && (ps_svs->u16_Data_Age_128us
                <= kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
         )
      {
        /* Currently processed message will be discarded. But previously
           received data is still considered to be ok. */

        /* Abort the processing of this packet. Do Not close the connection.  */
        /* (see FRS5) */
        o_abortProcessing = CSS_k_TRUE;

        /* Do not use the Data. */
        /* Indicate to the caller that this message must be discarded */
        o_retVal = CSS_k_TRUE;
        /* Do not update Last_Corrected_Data_Time_Stamp or                    */
        /* Last_Data_Time_Stamp */

        /* The reason why this message has been qualified as faulted was
           reported to SAPL already at the point where this was detected */
      }
      /* ELSE */
      else
      {
        /* S_Con_Flt_C_Out = Faulted, */
        /* (see FRS340) */
        IXSVS_ConFaultFlagSet(u16_svsIdx, IXSVD_k_CNXN_FAULTED);

        /* Abort the processing of this packet. Close the connection. */
        /* (see FRS3 and FRS5) */
        o_abortProcessing = CSS_k_TRUE;

        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_F_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);

        /* Increment global IXSVS fault counter */
        IXSVS_StateFaultCountInc();

        /* Update states and notify application (see FRS12-1) */
        /* (see Req.2.5.2.2-2) */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
      }
      /* ENDIF */
    }
    else
    {
      /* connection is not faulted */
    }
  }


  /* if there was not yet an error detected before */
  /* suppress lint warning: always false in case only Base Format supported */
  if (!o_abortProcessing)                                        /*lint !e774 */
  {
    /*
    // execute a function that checks for ping count changes and
    // determines if it is time to produce a time coordination message
    ping_count_check_in_consumer_function(),
    */
    /* The call of IXSVS_TcooPingCountCheck(u16_svsIdx) has been moved before */
    /* this function. This is possible as they are independent but this makes */
    /* it possible to use the same code for Link and Application Triggered    */
    /* Safety Validator Server (though the latter one is not implemented in   */
    /* CSS).                                                                  */

    /* Init_Complete_Out_Temp = 0, */
    /* b_Init_Complete_Out_Temp already set at initialization */

    /* // execute the safety connection type specific functions
    IF (Connection_Type == Multi-cast),
    THEN */
    if (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
      /* multi_cast_consumer_function(), */
      IXSVS_MultiCastConsumerFunction(u16_svsIdx, ps_svs->b_S_Run_Idle_Out,
                        ps_svs->b_Init_Complete_Out, &s_tempFlags,
                        &s_tempFlags,
                        &ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    /* ELSE */
    else
    {
      /* single_cast_consumer_function(), */
      SingleCastConsumerFunction(u16_svsIdx, &s_tempFlags, &s_tempFlags);
    }
    /* ENDIF */

    /*
    // If we are using the ExtendedFormat, at this point we have a properly
    // formatted message that was received in the right order. With the
    // extended format update the rollover count and last timestamp for
    // rollover.
    IF (ExtendedFormat),
    THEN
    */
    {
      /*
      // Only update the rollover count if this is a multicast connection
      // or if this is a singlecast connection that is out of the initialization
      // phase.
      IF ((Connection_Type == Multi-cast) OR
          (Init_Complete_Out_Temp == 1)),
      THEN
      */
      if (    (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
           || (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        /*
        // Check the timestamp value in the message to determine if a
        // rollover has occurred, and if so, update the rollover count
        // accordingly.
        IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp <
             Last_Time_Stamp_For_Rollover))
        THEN
        */
        if (u16_timeStamp_128us
            < ps_svs->u16_Last_Time_Stamp_For_Rollover_128us)
        {
          /* TS_Rollover_Count = TS_Rollover_Count + 1, */
          ps_svs->u16_TS_Rollover_Count++;
        }
        else
        {
          /* not yet time for a rollover */
        }
        /* ENDIF */

        /* Last_Time_Stamp_For_Rollover = Time_Stamp_Section.Data_Time_Stamp, */
        ps_svs->u16_Last_Time_Stamp_For_Rollover_128us = u16_timeStamp_128us;
      }
      else
      {
        /* not relevant for single-cast cnxns in initialization phase */
      }
      /* ENDIF */

      /* // Save the corrected data timestamp so we can recalculate a data age
      // if the next packet has errors.
      Last_Corrected_Data_Time_Stamp = Corrected_Data_Time_Stamp, */
      ps_svs->u16_Last_Corrected_Data_Time_Stamp_128us =
        ps_svs->u16_Corrected_Data_Time_Stamp_128us;
    }

    /* // Save Time stamp for next reception sequence check
    Last_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp, */
    ps_svs->u16_Last_Data_Time_Stamp_128us = u16_timeStamp_128us;

    /* // Calculate the worst case age
    IF (Init_Complete_Out_Temp == 1),
    THEN */
    if (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
    {
      /* (see FRS74) */
      /* Data_Age = Consumer_Clk_Count - Corrected_Data_Time_Stamp, */
      ps_svs->u16_Data_Age_128us = (CSS_t_UINT)(IXSVS_ConsumerClkCountGet()
                                 - ps_svs->u16_Corrected_Data_Time_Stamp_128us);
    }
    else
    {
      /* connection still in initialization phase */
    }
    /* ENDIF */

    /* // Calculate the maximum worst case age
    IF (Data_Age > Max_Data_Age)
    THEN */
    if (ps_svs->u16_Data_Age_128us
        > ps_svs->u16_Max_Data_Age_128us)
    {
      /* Max_Data_Age = Data_Age, */   /* (see SRS78) */
      ps_svs->u16_Max_Data_Age_128us = ps_svs->u16_Data_Age_128us;
    }
    else
    {
      /* current data_age smaller or equal than max_data_age */
    }
    /* ENDIF */

    /* (see FRS80) */
    /* Time Stamp Check starts when Init_Complete_Out is set
       (see FRS286 and FRS306)
    // Check for Age, Integrity, and out of sequence faults
    IF (( unsigned int_16( Data_Age > Network_Time_Expectation_Multiplier)
          AND (Init_Complete_Out_Temp == 1))
          OR (Temp_Fault_Flag == Faulted))
    THEN
    */

    /* (see FRS127-4, FRS81, FRS87-2, FRS288) */
    /* if Data_Age is too old or previous checks failed */
    if (    (    (ps_svs->u16_Data_Age_128us >  /* (see FRS130) */
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
            )
         || (s_tempFlags.b_Temp_Fault_Flag == IXSVD_k_CNXN_FAULTED)
       )
    {
      /* inadvertently received standard messages would not contain a valid   */
      /* time stamp (see FRS8-4)                                              */

      /* (see FRS3, FRS5, FRS86 and FRS288, FRS340) */
      /* // The connection will be closed, the data should not be used
      S_Con_Flt_C_Out_Temp = Faulted, */
      s_tempFlags.b_S_Con_Flt_C_Out_Temp = IXSVD_k_CNXN_FAULTED;

      /* just for error reporting to SAPL: */
      /* if data age exceeds Network Time Expectation */
      if (    (ps_svs->u16_Data_Age_128us >
                  kps_initCalc->u16_Network_Time_Expectation_Multiplier_128us)
              && (s_tempFlags.b_Init_Complete_Out_Temp == IXSVS_k_COMPL)
         )
      {
        IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_DATA_AGE_V_MSG,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)ps_svs->u16_Data_Age_128us);
      }
      else
      {
        /* nothing to report */
      }

      /* Increment the fault counter */
      IXSVS_StateFaultCountInc();
      /* Update states and notify application (see FRS12-1) */
      /* (see Req.2.5.2.2-2) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
    }
    else
    {
      /* checks passed - no fault detected */
    }
    /* ENDIF */

    /*
    //////////////////////////////////////////////////////////////////
    // The following information is passed on to the consuming application
    // with block integrity: Safety Data,Init_Complete_Out,S_Run_Idle_Out
    // and S_Con_Flt_C_Out.
    // The time between the checking of the age of the data and passing
    // the data off to the consuming application must be protected under
    // the time_stamp check or the consuming application time checks.
    //////////////////////////////////////////////////////////////////
    */
    /* Make Safety Data Available to Consuming Application */
    /* Init_Complete_Out = Init_Complete_Out_Temp, */
    ps_svs->b_Init_Complete_Out =
      s_tempFlags.b_Init_Complete_Out_Temp;
    /* S_Run_Idle_Out = S_Run_Idle_Out_Temp, */
    ps_svs->b_S_Run_Idle_Out =
      s_tempFlags.b_S_Run_Idle_Out_Temp;
    /* S_Con_Flt_C_Out = S_Con_Flt_C_Out_Temp, */
    IXSVS_ConFaultFlagSet(u16_svsIdx, s_tempFlags.b_S_Con_Flt_C_Out_Temp);
  }
  else
  {
    /* processing already aborted */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);

/* This function is strictly based on example code presented in Volume 5. Thus
   the structure should not be altered. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


/*******************************************************************************
**
** Function    : SingleCastConsumerFunction
**
** Description : This function is called by the consumer safety data reception
**               logic to handle single cast specific jobs.
**
** Parameters  : u16_svsIdx (IN)        - index of the Safety Validator Server
**                                        instance
**                                        (not checked, checked in
**                                        IXSVS_RxProc())
**               ps_tempFlags_in (IN)   - Temporary flags for this Safety Data
**                                        Consumption (input to function)
**                                        (not checked, only called with
**                                        reference to structure)
**               ps_tempFlags_out (OUT) - Temporary flags for this Safety Data
**                                        Consumption (output of function)
**                                        (not checked, only called with
**                                        reference to structure)
**
** Returnvalue : -
**
*******************************************************************************/
static void SingleCastConsumerFunction(CSS_t_UINT u16_svsIdx,
                                 const IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_in,
                                 IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_out)
{
  /*
  //////////////////////////////////////////////////////////////////
  // Start of single-cast consumer function
  //////////////////////////////////////////////////////////////////
  single_cast_consumer_function()
  */
  /* pointer to safety validator server structure */
  t_SVS_LOCALS_CONS *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* mode byte of the safety validator server */
  const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);

  /* initialize TempFlags_out with the input values (this is necessary as
     IN/OUT) parameters are not allowed in this software) */
  *ps_tempFlags_out = *ps_tempFlags_in;

  /* // For single-cast, no time stamp correction is required
  Corrected_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp, */
  ps_svs->u16_Corrected_Data_Time_Stamp_128us =
    IXSVS_DataTimeStampGet(u16_svsIdx);

  /*
  // Determine the Init_Complete_Out state to be passed on to the
  // consuming application.
  // For single-cast or bi-dir, a time_stamp other than 0 or a Run
  // indication, indicate that the Time_Coordination_Message has been
  // received by the Client and the corrected time stamp is being used.
  IF ((Init_Complete_Out == 1) OR
      (Corrected_Data_Time_Stamp != 0) OR
      (Mode_Byte.Run_Idle == Run)),
  THEN
  */
  if (    (ps_svs->b_Init_Complete_Out == IXSVS_k_COMPL)
       || (ps_svs->u16_Corrected_Data_Time_Stamp_128us != 0U)
       || (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_RUN_IDLE) != IXSVD_k_IDLE)
     )
  {
    /* Init_Complete_Out_Temp = 1, */  /* (see FRS281) */
    /* This also enables checking of the Data Time Stamp */
    /* (see FRS284 and FRS287) */
    ps_tempFlags_out->b_Init_Complete_Out_Temp = IXSVS_k_COMPL;

    /* if flag changed */
    if (ps_svs->b_Init_Complete_Out != IXSVS_k_COMPL)
    {
      /* update state machine */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_ACTIVE);
      /* stop the initialization limit timer */
      IXSVS_StateInitTimerStop(u16_svsIdx);
    }
    else
    {
      /* Safety Validator Server was already completely initialized */
    }
  }
  else
  {
    /* nothing to do */
  }
  /* ENDIF */

  /*
  // Determine the Run_Idle indication to be passed on to the consuming
  // application.
  IF (((Mode_Byte.Run_Idle == Idle) OR (Init_Complete_Out_Temp == 0))
  THEN
  */
  if (    (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_RUN_IDLE) == IXSVD_k_IDLE)
       || (ps_tempFlags_out->b_Init_Complete_Out_Temp == IXSVS_k_INIT)
     )
  {
    /* (see FRS277 and FRS279) */
    /* S_Run_Idle_Out_Temp = Idle, */
    ps_tempFlags_out->b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

    /* if flag changed */
    if (ps_svs->b_S_Run_Idle_Out != IXSVD_k_IDLE)
    {
      /* update state machine - this will be reported to SAPL so that it can
         react accordingly (see FRS276) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_IDLE);
    }
    else
    {
      /* Safety Validator Server was already idle */
    }
  }
  /* ELSE */
  else
  {
    /* S_Run_Idle_Out_Temp = Run, */
    ps_tempFlags_out->b_S_Run_Idle_Out_Temp = IXSVD_k_RUN;

    /* if flag changed */
    if (ps_svs->b_S_Run_Idle_Out != IXSVD_k_RUN)
    {
      /* update state machine */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_RUN);
    }
    else
    {
      /* Safety Validator Server was already in RUN mode */
    }
  }
  /* ENDIF */

  /*
  END single_cast_consumer_function()
  //////////////////////////////////////////////////////////////////
  // end of single-cast processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVStcorr.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains the functions to process a received Time
**             Correction message.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_TcorrLocalsClear
**             IXSVS_TcorrColdStartLogic
**             IXSVS_TcorrRxCopy
**             IXSVS_ConFaultFlagGet
**             IXSVS_ConFaultFlagSet
**             IXSVS_TimeoutMultiplierInit
**             IXSVS_TimeoutMultiplierGet
**             IXSVS_TcorrPingIntCountIncrement
**             IXSVS_ConsFaultCounterIncrement
**             IXSVS_MultiCastConsumerFunction
**             IXSVS_TimeCorrMsgReceptionLt
**             IXSVS_RxDataCrossCheckError
**             IXSVS_ConsFaultCounterGet
**             IXSVS_ConsFaultCounterReset
**             IXSVS_TcorrSoftErrByteGet
**             IXSVS_TcorrSoftErrVarGet
**
**             TcorrMsgReception2ndStage
**             TcorrCrcIsWrong
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
#include "IXSFA.h"
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

/** t_SVS_LOCALS_TCORR:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* structure for storing the data of the Time Correction message */
  CSS_t_TCORR_MSG   s_tcorrMsg;

  /* Last_Rcved_Time_Correction_Value (see Vol. 5 Chapt. 2-4.6.4.6): is the
     saved copy of the received Time_Correction_Value.
     Legal range: 0..65535 */
  CSS_t_UINT        u16_Last_Rcved_Time_Correction_Value_128us;

  /* Timeout_Multiplier (see Vol. 5 Chapt. 2-4.5.2.2): indicates the number of
     data production retries to use in the connection failure detection
     equations. */
  struct
  {
    CSS_t_USINT     u8_PI;    /* Legal range: 1..4 */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_USINT     u8_ef;    /* Legal range: 1..255 */
  #endif
  } s_Timeout_Multiplier;

  /* Last_Rcved_Multi_Cast_Active_Idle (see Vol. 5 Chapt. 2-4.6.4.6): is the
     saved copy of the received Multic_Cast_Active_Idle.
     Legal values: IXSVD_k_IDLE (0) and IXSVD_k_ACTIVE (1) */
  CSS_t_BYTE        b_Last_Rcved_Multi_Cast_Active_Idle;


  /* Time_Correction_Ping_Interval_Count (see Vol. 5 Chapt. 2-4.6.4.8):
     indicates the number of Ping Intervals that have occurred since the last
     reception of a Time Correction message.
     Legal range:  1..6 */
  CSS_t_USINT       u8_Time_Correction_Ping_Interval_Count;

  /* Time_Correction_Received_Flag (see Vol. 5 Chapt. 2-4.6.4.9): indicates
     that a Time_Correction message has been received for this multicast
     consumer and the time stamp can be corrected and checked.
     Legal values: 0 and 1 */
  CSS_t_BYTE        b_Time_Correction_Received_Flag;

  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* Consumer_Fault_Counter (see Vol. 5 Chapt. 2-4.6.4.12): temporary value
       used to track the number of data integrity errors the consumer detects on
       the reception of Data and Time Correction messages.
       Legal range: 0..255 */
    CSS_t_USINT     u8_Consumer_Fault_Counter;
  #endif

  /* S_Con_Flt_C_Out (see Vol. 5 Chapt. 2-4.6.1.1): indicates to the consuming
     application if it should put the safety data in a safety state because the
     safety validators have detected a safety connection fault.
     Legal values: IXSVD_k_CNXN_OK (0) and IXSVD_k_CNXN_FAULTED (1) */
  CSS_t_BYTE        b_S_Con_Flt_C_Out;

} t_SVS_LOCALS_TCORR;

/* array of structs to store the local variables for each instance */
static t_SVS_LOCALS_TCORR as_SvsLocals[CSOS_cfg_NUM_OF_SV_SERVERS];


/* possible values for Time_Correction_Received_Flag */
#define k_NOT_RECEIVED                   0U
#define k_RECEIVED                       1U


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  /* define IDs of the variables depending on which variables are supported */
  enum
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
     k_SOFT_ERR_OPT_VAR_CRCS5,
  #endif
     k_SOFT_ERR_VAR_TCV,
     k_SOFT_ERR_VAR_MCB
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    ,k_SOFT_ERR_OPT_VAR_MCB2
    ,k_SOFT_ERR_OPT_VAR_CRCS3
  #endif
  };
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void TcorrMsgReception2ndStage(CSS_t_UINT u16_svsIdx);
static CSS_t_BOOL TcorrCrcIsWrong(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_TcorrLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in IXSVS_Init() and
**                                  SafetyOpenProc3())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrLocalsClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Safety Validator structure */
  CSS_MEMSET(&as_SvsLocals[u16_svsIdx], 0, sizeof(as_SvsLocals[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TcorrColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection from closed to open and initializes Time
**               Correction specific Variables of the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit(),
**                                 IXSVS_OrigServerInit() and IXCCO_CnxnOpen())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrColdStartLogic(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCORR *const ps_svs = &as_SvsLocals[u16_svsIdx];

  /* S_Con_Flt_C_Out = OK, */
  ps_svs->b_S_Con_Flt_C_Out = IXSVD_k_CNXN_OK;/* (see FRS275) */
  /* Last_Rcved_Multi_Cast_Active_Idle = Idle, */             /* (see FRS302) */
  ps_svs->b_Last_Rcved_Multi_Cast_Active_Idle = IXSVD_k_IDLE;
  /* Last_Rcved_Time_Correction_Value = 0x0000, */            /* (see FRS304) */
  ps_svs->u16_Last_Rcved_Time_Correction_Value_128us = 0U;
  /* Time_Correction_Ping_Interval_Count = 0x0000, */
  ps_svs->u8_Time_Correction_Ping_Interval_Count = 0U;
  /* Time_Correction_Received_Flag = 0, */
  ps_svs->b_Time_Correction_Received_Flag = k_NOT_RECEIVED;

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  {
    /* Consumer_Fault_Counter = 0 */
    ps_svs->u8_Consumer_Fault_Counter = 0U;
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TcorrRxCopy
**
** Description : This function copies the received Time Correction Message
**               Byte-Stream into the Time Correction Message structure of this
**               instance.
**
** Parameters  : u16_svsIdx (IN)  - index of the Safety Validator Server
**                                  instance
**                                  (not checked, checked in IXSVS_RxProc())
**               pb_data (IN)     - pointer to the Time Correction Message in
**                                  the received byte stream.
**                                  (not checked, called with reference to
**                                  checked received HALCS message buffer plus
**                                  offset)
**               b_msgFormat (IN) - message format byte of the addressed
**                                  instance
**                                  (not checked, only one bit evaluated, any
**                                  value allowed)
**
** Returnvalue : CSS_t_UINT       - number of bytes copied
**
*******************************************************************************/
CSS_t_UINT IXSVS_TcorrRxCopy(CSS_t_UINT u16_svsIdx,
                             const CSS_t_BYTE *pb_data,
                             CSS_t_BYTE b_msgFormat)
{
  /* return value of this function */
  CSS_t_UINT u16_retVal = 0U;

  /* if is Base Format */
  if (IXSVD_IsBaseFormat(b_msgFormat))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /* read the data from the received byte stream to the message structure */
      u16_retVal = IXSFA_BfTcrMsgRead(pb_data,
                     &as_SvsLocals[u16_svsIdx].s_tcorrMsg);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_13,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* read the data from the received byte stream to the message structure */
      u16_retVal = IXSFA_EfTcrMsgRead(pb_data,
                     &as_SvsLocals[u16_svsIdx].s_tcorrMsg);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_14,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_retVal);
}


/*******************************************************************************
**
** Function    : IXSVS_ConFaultFlagGet
**
** Description : This function returns the S_Con_Flt_C_Out flag of this Safety
**               Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE      - S_Con_Flt_C_Out flag of this Safety
**                                 Validator Server
**
*******************************************************************************/
CSS_t_BYTE IXSVS_ConFaultFlagGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvsLocals[u16_svsIdx].b_S_Con_Flt_C_Out);
}


/*******************************************************************************
**
** Function    : IXSVS_ConFaultFlagSet
**
** Description : This function sets the S_Con_Flt_C_Out flag of this Safety
**               Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by IXSVS_RxProc())
**               b_flag (IN)     - value that is to be assigned to the
**                                 S_Con_Flt_C_Out flag
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConFaultFlagSet(CSS_t_UINT u16_svsIdx, CSS_t_BYTE b_flag)
{
  as_SvsLocals[u16_svsIdx].b_S_Con_Flt_C_Out = b_flag;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TimeoutMultiplierInit
**
** Description : This function initializes the Timeout Multiplier structure
**               based on the passed timeout multiplier.
**
** Parameters  : u16_svsIdx (IN)     - index of Safety Validator Server
**                                     structure
**                                     (not checked, checked in
**                                     IXSVS_TargServerInit() and
**                                     IXSVS_OrigServerInit())
**               u8_timeoutMult (IN) - timeout multiplier value
**                                     (not checked, checked in
**                                     CnxnParamsValidateSafety1() and
**                                     RangeCheckInstSafetyParams())
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TimeoutMultiplierInit(CSS_t_UINT u16_svsIdx,
                                 CSS_t_USINT u8_timeoutMult)
{
  /* Timeout_Multiplier */
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  {
    as_SvsLocals[u16_svsIdx].s_Timeout_Multiplier.u8_ef = u8_timeoutMult;
  }
#endif

  /* if timeout multiplier is in range 1..4 */
  if (    (u8_timeoutMult >= CSS_k_MIN_TMULT_PI)
       && (u8_timeoutMult <= CSS_k_MAX_TMULT_PI)
     )
  {
    /* simply copy the passed value */
    as_SvsLocals[u16_svsIdx].s_Timeout_Multiplier.u8_PI = u8_timeoutMult;
  }
  else /* else: 0 or 5..255 */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* if timeout multiplier is not zero */
    if (u8_timeoutMult >= CSS_k_MIN_TMULT_PI)
    {
      /* limit Timeout_Multiplier.PI to 4 (see FRS374) */
      as_SvsLocals[u16_svsIdx].s_Timeout_Multiplier.u8_PI = CSS_k_MAX_TMULT_PI;
    }
    else /* timeout multiplier is zero */
  #endif
    {
      /* error - should have been prevented by other checks */
      IXSVS_ErrorClbk(IXSVS_k_FSE_INC_TMULT_INV_RANGE,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u8_timeoutMult);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TimeoutMultiplierGet
**
** Description : This function returns the value of the Timeout Multiplier of
**               the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_USINT     - Timeout Multiplier
**
*******************************************************************************/
CSS_t_USINT IXSVS_TimeoutMultiplierGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  {
    /* mute lint warning "Return statement before end of function" */
    return (as_SvsLocals[u16_svsIdx].s_Timeout_Multiplier.u8_ef);/*lint !e904 */
  }
#else
  {
    return (as_SvsLocals[u16_svsIdx].s_Timeout_Multiplier.u8_PI);
  }
#endif
/* The function has just one return point (preprocessor 'if') */
/* RSM_IGNORE_QUALITY_BEGIN Notice #27 - Number of function return points > 1 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : IXSVS_TcorrPingIntCountIncrement
**
** Description : This function increments the Time Correction Ping Interval
**               Counter.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrPingIntCountIncrement(CSS_t_UINT u16_svsIdx)
{
  as_SvsLocals[u16_svsIdx].u8_Time_Correction_Ping_Interval_Count++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterIncrement
**
** Description : This function increments the Consumer Fault Counter and returns
**               the new value.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by IXSVS_RxProc())
**
** Returnvalue : CSS_t_USINT     - new Consumer Fault Counter value
**
*******************************************************************************/
CSS_t_USINT IXSVS_ConsFaultCounterIncrement(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (++(as_SvsLocals[u16_svsIdx].u8_Consumer_Fault_Counter));
}
#endif


/*******************************************************************************
**
** Function    : IXSVS_MultiCastConsumerFunction
**
** Description : This function is called by the consumer safety data reception
**               logic to handle multi-cast specific jobs.
**
** Parameters  : u16_svsIdx (IN)          - index of the Safety Validator Server
**                                          instance
**                                          (not checked, checked by
**                                          IXSVS_RxProc())
**               b_s_Run_Idle_Out (IN)    - value of the Safety Validator's
**                                          S_Run_Idle_Out flag
**                                          (not checked)
**               b_init_Complete_Out (IN) - value of the Safety Validator's
**                                          Init_Complete_Out flag
**                                          (not checked)
**               ps_tempFlags_in (IN)     - Temporary flags for this Safety Data
**                                          Consumption (input to function)
**                                          (not checked, only called with
**                                          reference to structure)
**               ps_tempFlags_out (OUT)   - Temporary flags for this Safety Data
**                                          Consumption (output of function)
**                                          (not checked, only called with
**                                          reference to structure)
**               pu16_corrected_Data_Time_Stamp_128us (OUT) - calculated by this
**                                          function, to be assigned to the
**                                          u16_Corrected_Data_Time_Stamp_128us
**                                          of the instance.
**                                          (not checked, only called with
**                                          reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_MultiCastConsumerFunction(CSS_t_UINT u16_svsIdx,
                              CSS_t_BYTE b_s_Run_Idle_Out,
                              CSS_t_BYTE b_init_Complete_Out,
                              const IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_in,
                              IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_out,
                              CSS_t_UINT *pu16_corrected_Data_Time_Stamp_128us)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCORR *const ps_svs = &as_SvsLocals[u16_svsIdx];

  /*
  //////////////////////////////////////////////////////////////////
  // Start of multi-cast consumer function
  //////////////////////////////////////////////////////////////////
  multi_cast_consumer_function()
  */
  /* mode byte of the safety validator server */
  const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);

  /* initialize TempFlags_out with the input values (this is necessary as
     IN/OUT) parameters are not allowed in this software) */
  *ps_tempFlags_out = *ps_tempFlags_in;

  /*
  // If multi-cast, check that a time correction message has been
  // received within the last Timeout_Multiplier.PI + 1 Ping Intervals
  IF (ps_svs->Time_Correction_Ping_Interval_Count > (Timeout_Multiplier.PI + 1))
  THEN
  */
  if (ps_svs->u8_Time_Correction_Ping_Interval_Count >
      (ps_svs->s_Timeout_Multiplier.u8_PI + 1U))
  {
    /* (see FRS305) */
    /* Temp_Fault_Flag = Faulted, */
    ps_tempFlags_out->b_Temp_Fault_Flag = IXSVD_k_CNXN_FAULTED;

    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TCORR_NOT_PI,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* previous time correction message has been received in time */
  }
  /* ENDIF */

  /* (see FRS28 and FRS61 and FRS298 and FRS303) */
  /*  // If multicast, add correction value to time stamp
  Corrected_Data_Time_Stamp = Time_Stamp_Section.Data_Time_Stamp +
    Last_Rcved_Time_Correction_Value */
  *pu16_corrected_Data_Time_Stamp_128us =
    (CSS_t_UINT)(IXSVS_DataTimeStampGet(u16_svsIdx)
    + ps_svs->u16_Last_Rcved_Time_Correction_Value_128us);

  /*
  // Determine the Init_Complete_Out state to be passed on to the
  // consuming application.
  // For multi-cast, the reception of a Time_Correction Message, indicate
  // that data may be used.
  IF (Time_Correction_Received_Flag == 1)
  THEN
  */
  /* (see FRS306) */
  if (ps_svs->b_Time_Correction_Received_Flag == k_RECEIVED)
  {
    /* Init_Complete_Out_Temp = 1, */  /* (see FRS282) */
    /* This also enables checking of the Data Time Stamp */
    ps_tempFlags_out->b_Init_Complete_Out_Temp = IXSVS_k_COMPL;

    /* check if flag changed */
    if (b_init_Complete_Out != IXSVS_k_COMPL)
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
  IF ((Mode_Byte.Run_Idle == Idle) OR
      (Init_Complete_Out_Temp == 0) OR
       (Time_Correction_Received_Flag == 0) OR //used only on multi-cast
       (Last_Rcved_Multi_Cast_Active_Idle==Idle))//used only on multi-cast
  THEN
  */
  if (    (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_RUN_IDLE) == IXSVD_k_IDLE)
          /* (see FRS107) */
       || (ps_tempFlags_out->b_Init_Complete_Out_Temp == IXSVS_k_INIT)
       || (ps_svs->b_Time_Correction_Received_Flag == k_NOT_RECEIVED)
       || (ps_svs->b_Last_Rcved_Multi_Cast_Active_Idle == IXSVD_k_IDLE)
     )
  {
    /* (see FRS277 and FRS279) */
    /* S_Run_Idle_Out_Temp = Idle, */
    ps_tempFlags_out->b_S_Run_Idle_Out_Temp = IXSVD_k_IDLE;

    /* if Run Idle flag just changes to IDLE */
    if (b_s_Run_Idle_Out != IXSVD_k_IDLE)
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

    /* if Run Idle flag just changes to RUN */
    if (b_s_Run_Idle_Out != IXSVD_k_RUN)
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
  end multi_cast_consumer_function()
  //////////////////////////////////////////////////////////////////
  // end of multi-cast processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TimeCorrMsgReceptionLt
**
** Description : This function contains the logic associated with the Time
**               Correction Message reception for the multi-cast safety
**               connection type, for the Consumer - Link Triggered.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TimeCorrMsgReceptionLt(CSS_t_UINT u16_svsIdx)
{
  /*
  //////////////////////////////////////////////////////////////////
  // Consumer processing - Time Correction Message reception
  //////////////////////////////////////////////////////////////////
  */
  /* get a pointer to the struct containing the Consumer_Num of this instance */
  const IXSVS_t_INIT_CALC* const kps_cnum =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /* Reserved bits of Mcast are ignored except for CRC checking (see FRS216) */

  /*
  // Is the Time Correction message for this Consumer ?
  IF (MCast_Byte.Consumer_Num = Consumer_Num)
  // Perform integrity checks.
  THEN
  */
  /* (see FRS206) */
  if (IXUTL_BIT_GET_U8(as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte,
                       IXSVD_k_MCB_CONS_NUM)
      == kps_cnum->u8_Consumer_Num)
  {
    /* Link triggered - so immediately continue with second stage */
    TcorrMsgReception2ndStage(u16_svsIdx);
  }
  else /* else: time correction message does not address this consumer */
  {
    /* This message is received by all consumers but only relevant for the
       addressed consumer_number. Therefor no error is generated. */
  }
  /* ENDIF */

  /*
  //////////////////////////////////////////////////////////////////
  // end - Time Correction Message reception
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_RxDataCrossCheckError
**
** Description : This function reports an error to a Safety Validator Server
**               instance. Base format connection will be set to faulted,
**               extended format connection will increment fault counter and
**               stay open unless fault counter reached limit.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVO_RxDataCrossCheckError())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_RxDataCrossCheckError(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCORR *const ps_svs = &as_SvsLocals[u16_svsIdx];

  /* report error to SAPL */
  IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_CROSS_CHECK,
                  IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);

  /* (see FRS86) */
  {
    /* if is Extended Format */
    if (IXSVD_IsExtendedFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      /* get a pointer to the struct containing the Max_Fault_Number of this
         instance */
      const IXSVS_t_INIT_CALC* const kps_mfn =
        IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

      /* increment Consumer_Fault_Counter */
      ps_svs->u8_Consumer_Fault_Counter++;

      /* if consumer fault counter has not yet reached the limit */
      if (ps_svs->u8_Consumer_Fault_Counter < kps_mfn->u8_Max_Fault_Number)
      {
        /* connection may stay open */
      }
      else /* else: Max Fault Number is reached */
      {
        ps_svs->b_S_Con_Flt_C_Out = IXSVD_k_CNXN_FAULTED;

        /* Increment global IXSVS fault counter */
        IXSVS_StateFaultCountInc();

        /* Update states and notify application (see FRS12-1)
           (see Req.2.5.2.2-2). */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_26,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Base Format */
    {
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        /* S_Con_Flt_C_Out = Faulted */
        ps_svs->b_S_Con_Flt_C_Out = IXSVD_k_CNXN_FAULTED;

        /* increment the Fault Counter */
        IXSVS_StateFaultCountInc();
        /* Update states and notify application (see FRS12-1)
           (see Req.2.5.2.2-2). */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_27,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterGet
**
** Description : This function returns the Consumer Fault Counter of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_USINT     - Consumer Fault Counter of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
CSS_t_USINT IXSVS_ConsFaultCounterGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvsLocals[u16_svsIdx].u8_Consumer_Fault_Counter);
}


/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterReset
**
** Description : This function resets the Consumer Fault Counters of all Safety
**               Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsFaultCounterReset(void)
{
  CSS_t_UINT u16_svsIdx;

  for (u16_svsIdx = 0U; u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_svsIdx++)
  {
    /* Consumer_Fault_Counter = 0 */
    as_SvsLocals[u16_svsIdx].u8_Consumer_Fault_Counter = 0U;
  }
}
#endif


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_TcorrSoftErrByteGet
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
CSS_t_UDINT IXSVS_TcorrSoftErrByteGet(CSS_t_UDINT u32_varCnt,
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
** Function    : IXSVS_TcorrSoftErrVarGet
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
void IXSVS_TcorrSoftErrVarGet(
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
        /* indicate to caller that we are in range of level 2 */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_CRCS5:
          {
            CSS_H2N_CPY32(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcorrMsg.u32_crcS5);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }
        #endif

          case k_SOFT_ERR_VAR_TCV:
          {
            CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcorrMsg.u16_timeCorrVal_128us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case k_SOFT_ERR_VAR_MCB:
          {
            CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcorrMsg.b_mCastByte);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            break;
          }

        #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_MCB2:
          {
            CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcorrMsg.b_mCastByte2);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            break;
          }

          case k_SOFT_ERR_OPT_VAR_CRCS3:
          {
            CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcorrMsg.u16_crcS3);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }
        #endif

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
            break;
          }
        }
        break;
      }

      case 1U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].u16_Last_Rcved_Time_Correction_Value_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_Last_Rcved_Multi_Cast_Active_Idle);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 3U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].u8_Time_Correction_Ping_Interval_Count);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 4U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_Time_Correction_Received_Flag);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 5U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_S_Con_Flt_C_Out);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 6U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_Timeout_Multiplier.u8_PI);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      case 7U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_Timeout_Multiplier.u8_ef);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 8U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].u8_Consumer_Fault_Counter);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }
    #endif

      default:
      {
        /* level 1 counter at/above end */
        /* default return values already set */
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

/*******************************************************************************
**
** Function    : TcorrMsgReception2ndStage
**
** Description : This function contains the logic associated with the Time
**               Correction Message reception for the multi-cast safety
**               connection type, for the Consumer. But this function only
**               contains the parts that are common to Link and Application
**               Triggered servers.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void TcorrMsgReception2ndStage(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCORR *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* flag indicating that further processing of the message must be aborted */
  CSS_t_BOOL o_abortProcessing = CSS_k_FALSE;

  /* Reserved bits of Mcast are ignored except for CRC checking (see FRS216) */

  /*
  IF (Time Coordination message CRC incorrect)
      OR (MCast_Byte parity incorrect)
      // MCast_Byte_2 not included in ExtendedFormat
      OR {MCast_Byte_2 != (((MCast_Byte XOR0xFF)AND0x55)
      OR(MCast_Byte AND0xAA))}
      OR {(MCast_Byte.Multi_Cast_Active_Idle == Idle)
      AND (Last_Rcved_Multi_Cast_Active_Idle == Active)},
      // only one transition from connection idle to active will
      // be allowed, to prevent an erroneous idle indication,
      // a transistion from active to idle will be latched into
      // a fault state
  THEN
  */
  /* Error in Sample-Code: Of course the CRC of the Time CORRECTION message */
  /* is to be checked here (not Time Coordination).                         */
  /* (see FRS62) */
  CSS_t_BOOL o_mCastByteParityCheck = CSS_k_FALSE;
  CSS_t_BOOL o_mCastByte2Check = CSS_k_FALSE;
  CSS_t_BOOL o_aiFlagChanged = CSS_k_FALSE;

  /* if calculated parity is different than received parity */
  if (IXCRC_ParityCalc(as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte)
      != as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte)
  {
    o_mCastByteParityCheck = CSS_k_TRUE;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TCORR_PARITY,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* o_mCastByteParityCheck already initialized to false */
  }

  if (IXSVD_IsBaseFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    if (ps_svs->s_tcorrMsg.b_mCastByte2
         != IXCRC_TcByte2Calc(as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte))
    {
      o_mCastByte2Check = CSS_k_TRUE;
      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_TCORR_MC2,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
    }
    else
    {
      /* o_mCastByte2Check already initialized to false */
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_15,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else
  {
    /* MCast_Byte_2 not included in ExtendedFormat */
  }

  /* if connection transitioned from active to idle (see FRS301) */
  if (    (IXUTL_BIT_GET_U8(as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte,
                            IXSVD_k_MCB_MAI) == IXSVD_k_IDLE)
       && (ps_svs->b_Last_Rcved_Multi_Cast_Active_Idle == IXSVD_k_ACTIVE)
     )
  {
    o_aiFlagChanged = CSS_k_TRUE;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_TCORR_AI_FLAG,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* o_aiFlagChanged already initialized to false */
  }

  /* CRC check or any of the above calculated flags indicate error */
  if (    (TcorrCrcIsWrong(u16_svsIdx))
       || (o_mCastByteParityCheck)
       || (o_mCastByte2Check)
          /* (see FRS211) */
       || (o_aiFlagChanged)
            /* not all conditions apply to every cfg - thus mute lint warning */
     )                                                      /*lint !e774 !e845*/
  {
    /* IF (ExtendedFormat)
    THEN */
    if (IXSVD_IsExtendedFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      /* get a pointer to the struct containing the Max_Fault_Number of this
         instance */
      const IXSVS_t_INIT_CALC* const kps_mfn =
        IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

      /* increment Consumer_Fault_Counter */
      ps_svs->u8_Consumer_Fault_Counter++;

      /* IF (Consumer_Fault_Counter < Max_Fault_Number)
      THEN */
      if (ps_svs->u8_Consumer_Fault_Counter < kps_mfn->u8_Max_Fault_Number)
      {
        /* Abort the processing of this packet. Do not close the connection,
        Do not use the Data. */
        o_abortProcessing = CSS_k_TRUE;
      }
      /* ELSE */
      else
      {
        /* S_Con_Flt_C_Out = Faulted, */
        ps_svs->b_S_Con_Flt_C_Out = IXSVD_k_CNXN_FAULTED;

        /* Abort the processing of this packet. Close the connection */
        o_abortProcessing = CSS_k_TRUE;

        /* Increment global IXSVS fault counter */
        IXSVS_StateFaultCountInc();

        /* Update states and notify application (see FRS12-1)
           (see Req.2.5.2.2-2) */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
      }
      /* ENDIF */
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_28,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    /* ELSE */
    else
    {
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        /* S_Con_Flt_C_Out = Faulted */
        ps_svs->b_S_Con_Flt_C_Out = IXSVD_k_CNXN_FAULTED;

        /* increment the Fault Counter */
        IXSVS_StateFaultCountInc();
        /* Update states and notify application (see FRS12-1)
           (see Req.2.5.2.2-2). */
        IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);

        /* Missing in Sample-Code: Abort processing in case of error */
        o_abortProcessing = CSS_k_TRUE;
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_29,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  }
  else
  {
    /* all checks passed */
  }
  /* ENDIF */

  /* if there was not yet an error detected before */
  if (!o_abortProcessing)
  {
    /*
    // Save the Time_Correction_Value.
    Last_Rcved_Time_Correction_Value =
    Time_Correction_Section.Time_Correction_Value,
    */
    /* (see FRS61) */
    ps_svs->u16_Last_Rcved_Time_Correction_Value_128us =
      ps_svs->s_tcorrMsg.u16_timeCorrVal_128us;

    /*
    // Save the Multi_Cast_Active_Idle bit.
    Last_Rcved_Multi_Cast_Active_Idle =
    MCast_Byte.Multi_Cast_Active_Idle,
    */
    if (IXUTL_BIT_GET_U8(as_SvsLocals[u16_svsIdx].s_tcorrMsg.b_mCastByte,
                       IXSVD_k_MCB_MAI) == IXSVD_k_MCB_MAI)
    {
      ps_svs->b_Last_Rcved_Multi_Cast_Active_Idle = IXSVD_k_ACTIVE;
    }
    else
    {
      ps_svs->b_Last_Rcved_Multi_Cast_Active_Idle = IXSVD_k_IDLE;
    }

    /*
    // Set the Time_Correction_Received_Flag.
    Time_Correction_Received_Flag = 1,
    */
    ps_svs->b_Time_Correction_Received_Flag = k_RECEIVED;

    /*
    // Reset the Time Correction timeout counter
    Time_Correction_Ping_Interval_Count = 0,
    */
    ps_svs->u8_Time_Correction_Ping_Interval_Count = 0U;
  }
  else
  {
    /* processing already aborted */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : TcorrCrcIsWrong
**
** Description : This function checks the Time Correction message of a Safety
**               Validator Server Instance and returns CSS_k_TRUE in case of an
**               error. The caller of this function does not have to care
**               whether the Validator Instance refers to Base or Extended
**               Format.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE  - error in Time Correction message (CRC error)
**               CSS_k_FALSE - Time Correction message is ok
**
*******************************************************************************/
static CSS_t_BOOL TcorrCrcIsWrong(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCORR *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* function's return value */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* pointer to the CRC seeds of this instance */
  const IXSVS_t_INIT_CALC* const kps_crcSeeds =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /* if is Extended Format */
  if (IXSVD_IsExtendedFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UDINT u32_expCrc = 0U;

    /* Calculate expected CRC using PID as seed (see FRS156) */
    u32_expCrc =
      IXCRC_EfTcrMsgCrcCalc(&ps_svs->s_tcorrMsg,
                            kps_crcSeeds->u32_pidCrcS5);

    /* if calculated CRC doesn't match received CRC */
    if (ps_svs->s_tcorrMsg.u32_crcS5 != u32_expCrc)
    {
      o_retVal = CSS_k_TRUE;
      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_TCORR_CRC,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx), u32_expCrc);
    }
    else
    {
      /* o_retVal already initialized to false */
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_30,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else /* else: must be Base Format */

  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    CSS_t_UINT u16_expCrc = 0U;

    /* Calculate expected CRC using PID as seed (see FRS156) */
    u16_expCrc =
      IXCRC_BfTcrMsgCrcCalc(&ps_svs->s_tcorrMsg,
                            kps_crcSeeds->u16_pidCrcS3);

    /* if calculated CRC doesn't match received CRC */
    if (ps_svs->s_tcorrMsg.u16_crcS3 != u16_expCrc)
    {
      o_retVal = CSS_k_TRUE;
      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_TCORR_CRC,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)u16_expCrc);
    }
    else
    {
      /* o_retVal already initialized to false */
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_31,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


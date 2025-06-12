/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSstate.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains the state machine of the Safety Validator
**             Server instances. Also the Fault Counter (counting faults over
**             all Server instances) is contained in this module.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_StateNumServerCnxnGet
**             IXSVS_StateFaultCountInc
**             IXSVS_StateFaultCountGet
**             IXSVS_StateFaultCountReset
**             IXSVS_StateInit
**             IXSVS_StateGet
**             IXSVS_StateIsInitOrEstablished
**             IXSVS_StateMachine
**             IXSVS_StateUnderlayingComClose
**             IXSVS_StateInitTimerStop
**             IXSVS_StateInitLimitCalc
**             IXSVS_StateInitLimitCheck
**             IXSVS_StateSoftErrByteGet
**             IXSVS_StateSoftErrVarGet
**
**             StateTransition
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
#include "HALCSapi.h"

#include "IXSERapi.h"
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  #include "IXSAIapi.h"
  #include "IXSAI.h"
#endif
#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  #include "IXSPI.h"
#endif
#include "IXSVD.h"
#include "IXSCF.h"
#include "IXUTL.h"

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

/** u16_FaultCount:
    Diagnostic Counter that is a running count (with auto-rollover) of Safety
    connection Faults (Server only). Incremented when any consuming safety
    connection fails for any reason.
*/
/* Not relevant for Soft Error Checking - only a diagnostic value */
static CSS_t_UINT u16_FaultCount;

/** au8_SvsState:
    Safety Validator Server State Attribute (1) */
static CSS_t_USINT au8_SvsState[CSOS_cfg_NUM_OF_SV_SERVERS];


/* constants for calculating the time limit when the initialization process must
   be finished */
/*lint -esym(750, k_INITIALIZATION_LIMIT_BASE_128US) not ref. in every cfg */
/*lint -esym(750, k_INITIALIZATION_LIMIT_EXT_128US)  not ref. in every cfg */
#define k_INITIALIZATION_LIMIT_BASE_128US     78125UL  /* 10 seconds */
#define k_INITIALIZATION_LIMIT_EXT_128US      64842UL  /* 8.3 seconds */

/* scheduled time before which the initialization must be finished */
static CSS_t_UDINT au32_initLimit_128us[CSOS_cfg_NUM_OF_SV_SERVERS];

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #define k_STATE_SIZEOF_VAR1   (sizeof(au8_SvsState))
  #define k_STATE_SIZEOF_VAR12  (k_STATE_SIZEOF_VAR1 + \
                                sizeof(au32_initLimit_128us))
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void StateTransition(CSS_t_UINT u16_svsIdx,
                            CSS_t_USINT u8_oldState,
                            CSS_t_USINT u8_newState);


/*******************************************************************************
**    global functions
*******************************************************************************/

#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVS_StateNumServerCnxnGet
**
** Description : This function returns the number of Safety Validator
**               Server Connections that are currently open (in Initializing or
**               Established state).
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Number of open Server Connections
**
*******************************************************************************/
CSS_t_UINT IXSVS_StateNumServerCnxnGet(void)
{
  /* return value of this function */
  CSS_t_UINT u16_numServerCnxn = 0U;
  /* loop counter */
  CSS_t_UINT u16_svsIdx;

  /* check all Safety Validator Servers */
  for (u16_svsIdx = 0U; u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_svsIdx++)
  {
    /* if the state is Init or Established */
    if (IXSVS_StateIsInitOrEstablished(u16_svsIdx))
    {
      /* This Safety Validator is involved in a safety connection */
      u16_numServerCnxn++;
    }
    else /* else: other states */
    {
      /* this Safety Validator is not involved in a safety connection */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_numServerCnxn);
}
#endif


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountInc
**
** Description : This function increments the value of the Fault Counter of the
**               Safety Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateFaultCountInc(void)
{
  /* increment fault counter without considering overflows (see SRS75) */
  u16_FaultCount++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountGet
**
** Description : This function returns the current value of the Fault Counter of
**               the Safety Validator Server instances. Counts (with
**               auto-rollover) any consuming safety connection was faulted for
**               any reason.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - value of the Safety Validator Fault Count
**
*******************************************************************************/
CSS_t_UINT IXSVS_StateFaultCountGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_FaultCount);
}


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountReset
**
** Description : This function resets the value of the Fault Counter of the
**               Safety Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateFaultCountReset(void)
{
  u16_FaultCount = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_StateInit
**
** Description : This function initializes the state variables of a Safety
**               Validator Server instance
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_Init() and
**                                 IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInit(CSS_t_UINT u16_svsIdx)
{
  au8_SvsState[u16_svsIdx] = CSS_k_SV_STATE_IDLE;
  au32_initLimit_128us[u16_svsIdx] = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_StateGet
**
** Description : This function returns the state of the requested Safety
**               Validator Server Instance.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - State of the requested Safety Validator
**                                 Server Instance
**
*******************************************************************************/
CSS_t_USINT IXSVS_StateGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (au8_SvsState[u16_svsIdx]);
}


/*******************************************************************************
**
** Function    : IXSVS_StateIsInitOrEstablished
**
** Description : This function checks the state of the addressed Safety
**               Validator Server Instance
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - state of the instance is
**                                 CSS_k_SV_STATE_INITIALIZING or
**                                 CSS_k_SV_STATE_ESTABLISHED
**               CSS_k_FALSE     - instance is in other state
**
*******************************************************************************/
CSS_t_BOOL IXSVS_StateIsInitOrEstablished(CSS_t_UINT u16_svsIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if the state is Initializing or Established */
  if (    (au8_SvsState[u16_svsIdx] == CSS_k_SV_STATE_INITIALIZING)
       || (au8_SvsState[u16_svsIdx] == CSS_k_SV_STATE_ESTABLISHED)
     )
  {
    o_retVal = CSS_k_TRUE;
  }
  else /* else: any other states */
  {
    /* return value already set */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


/*******************************************************************************
**
** Function    : IXSVS_StateMachine
**
** Description : This function implements the Safety Validator Object State
**               machine as specified by the CIP Safety Specification.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**               e_svEvent (IN)  - specifies the event that occurred
**                                 (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateMachine(CSS_t_UINT u16_svsIdx,
                        IXSVS_t_VALIDATOR_EVENT e_svEvent)
{

  /* signal the event to the application (see FRS12-2) */
  SAPL_IxsvsEventHandlerClbk(IXSVS_InstIdFromIdxGet(u16_svsIdx), e_svEvent);

  /* if the event is a fault */
  if (e_svEvent == IXSVS_k_VE_SERVER_FAULTED)
  {
    /* close the underlaying connection (see FRS12-4) but not the Safety
       Validator */
    /* This happens if a consumer detects an error (see FRS11) */
    /* ignore return value - errors are already reported to SAPL */
    (void)IXSVS_StateUnderlayingComClose(u16_svsIdx,
                                         CSOS_k_MAX_CONSUMER_NUM_SCAST);
  }
  else /* else: other events */
  {
    /* event is not of interest for the fault counter */
  }

  /*
  ** Behavior is dependent on current state
  */
  switch (au8_SvsState[u16_svsIdx])
  {
    /* State IDLE and UNALLOCATED are different terms in the CIP Safety       */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_IDLE:
    {
      /* if event is "Server Open" */
      if (e_svEvent == IXSVS_k_VE_SERVER_OPEN)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_IDLE,
                        CSS_k_SV_STATE_INITIALIZING);
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_INV_EV_IDLE,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    case CSS_k_SV_STATE_INITIALIZING:
    {
      /* if event is "First Data Received" */
      if (e_svEvent == IXSVS_k_VE_1ST_DATA_REC)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_ESTABLISHED);
      }
      /* else: if event is "Server Faulted" */
      else if (e_svEvent == IXSVS_k_VE_SERVER_FAULTED)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_FAILED);
      }
      /* else: if event is "Server Close" */
      else if (e_svEvent == IXSVS_k_VE_SERVER_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_IDLE);
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_INV_EV_INIT,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    /* State EXECUTING and ESTABLISHED are different terms in the CIP Safety  */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_ESTABLISHED:
    {
      /* This could also happen because the periodic transmissions cease.
         (Activity Timer expired, message may arrive beyond Consumer Activity
         Monitor). Transitioning to FAILED will put the connection in a Safety
         state (see FRS11 and FRS79 and FRS87-1 and FRS87-2). */
      /* if event is "Server Faulted" */
      if (e_svEvent == IXSVS_k_VE_SERVER_FAULTED)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_ESTABLISHED,
                        CSS_k_SV_STATE_FAILED);

      #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
        if (IXSVS_InstIdFromIdxGet(u16_svsIdx) <= CSS_cfg_NUM_OF_CCO_INSTS)
        {
          /* length of the received data */
          CSS_t_USINT u8_len = 0U;
          /* pointer to the last received safety I/O data */
          const CSS_t_BYTE *pb_svBuf = IXSVS_DataPtrGet(u16_svsIdx, &u8_len);
          IXSPI_ConsImageDataSet(IXSVS_InstIdFromIdxGet(u16_svsIdx),
                                 IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                                 pb_svBuf, (CSS_t_UINT)u8_len,
                                 (CSS_t_BYTE)CSS_k_RIF_IDLE);
        }
        else
      #endif  /* (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE) */
      #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
        {
          /* Inform the output assembly about Safety Validator Server fault. */
          IXSAI_AsmOutputDataSet(IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                                 CSS_k_NULL,           /* no data is passed */
                                 (CSS_t_UINT)0U,       /* length not relevant */
                                 (CSS_t_BYTE)CSS_k_RIF_IDLE);
        }
      #else
        {
          /* error: invalid instance! */
          IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_EST_FAIL,
                          IXSVS_InstIdFromIdxGet(u16_svsIdx),
                          IXSER_k_A_NOT_USED);
          /* return value already set */
        }/* CCT_SKIP */ /* unreachable code - defensive programming */
      #endif  /* (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) */
      }
      /* else: if event is "Server Close" */
      else if (e_svEvent == IXSVS_k_VE_SERVER_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_ESTABLISHED,
                        CSS_k_SV_STATE_IDLE);

      #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
        if (IXSVS_InstIdFromIdxGet(u16_svsIdx) <= CSS_cfg_NUM_OF_CCO_INSTS)
        {
          /* length of the received data */
          CSS_t_USINT u8_len = 0U;
          /* pointer to the last received safety I/O data */
          const CSS_t_BYTE *pb_svBuf = IXSVS_DataPtrGet(u16_svsIdx, &u8_len);
          IXSPI_ConsImageDataSet(IXSVS_InstIdFromIdxGet(u16_svsIdx),
                                 IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                                 pb_svBuf, (CSS_t_UINT)u8_len,
                                 (CSS_t_BYTE)CSS_k_RIF_IDLE);
        }
        else
      #endif  /* (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE) */
      #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
        {
          /* Inform the output assembly about Safety Validator Server close. */
          IXSAI_AsmOutputDataSet(IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                                 CSS_k_NULL,           /* no data is passed */
                                 (CSS_t_UINT)0U,       /* length not relevant */
                                 (CSS_t_BYTE)CSS_k_RIF_IDLE);
        }
      #else
        {
          /* error: invalid instance! */
          IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_EST_CLOSE,
                          IXSVS_InstIdFromIdxGet(u16_svsIdx),
                          IXSER_k_A_NOT_USED);
          /* return value already set */
        }/* CCT_SKIP */ /* unreachable code - defensive programming */
      #endif  /* (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) */
      }
      else if (    (e_svEvent == IXSVS_k_VE_SERVER_ACTIVE)
                || (e_svEvent == IXSVS_k_VE_SERVER_IDLE)
                || (e_svEvent == IXSVS_k_VE_SERVER_RUN)
              )
      {
        /* these events are ignored in this state (only for reporting to
           Safety Application) */
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_INV_EV_ESTAB,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    /* State FAILED and FAULTED are different terms in the CIP Safety         */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_FAILED:
    {
      /* if event is "Server Open" */
      if (e_svEvent == IXSVS_k_VE_SERVER_OPEN)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_FAILED,
                        CSS_k_SV_STATE_INITIALIZING);
      }
      /* else: if event is "Server Close" */
      else if (e_svEvent == IXSVS_k_VE_SERVER_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svsIdx, CSS_k_SV_STATE_FAILED,
                        CSS_k_SV_STATE_IDLE);
      }
      else if (e_svEvent == IXSVS_k_VE_SERVER_FAULTED)
      {
        /* this event is ignored - we are already in the FAILED state */
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_INV_EV_FAIL,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    default:
    {
      /* invalid state! */
      IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SM_INV_STATE,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      (CSS_t_UDINT)au8_SvsState[u16_svsIdx]);
      break;
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
/* This function has a lot of paths. But dividing this state machine into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : IXSVS_StateUnderlayingComClose
**
** Description : This function sends a close command to the CSAL.
**
** Parameters  : u16_svsIdx (IN) - Safety Validator Server index
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_SERVERS-1)
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range:
**                                 1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: HALC not ready
**
*******************************************************************************/
CSS_t_WORD IXSVS_StateUnderlayingComClose(CSS_t_UINT u16_svsIdx,
                                          CSS_t_USINT u8_consNum)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVS_k_FSE_INC_PRG_FLOW);

  /* if instance is invalid */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* index is invalid */
    w_retVal = (IXSVS_k_FSE_INC_CLOSE_IDX);
    IXSVS_ErrorClbk(w_retVal, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_svsIdx);
  }
  else
  {
    /* Instance ID of the Safety Validator */
    const CSS_t_UINT u16_instId = IXSVS_InstIdFromIdxGet(u16_svsIdx);

    /* if the consumer number is invalid */
    if ((u8_consNum == 0U) || (u8_consNum > CSOS_k_MAX_CONSUMER_NUM_MCAST))
    {
      /* consumer number invalid! */
      w_retVal = (IXSVS_k_FSE_INC_CLOSE_CONS_NUM);
      IXSVS_ErrorClbk(w_retVal, u16_instId, (CSS_t_UDINT)u8_consNum);
    }
    else /* else: consumer number is ok */
    {
      /* structure for passing the success response to HALC */
      HALCS_t_MSG  s_halcMsg;
      CSS_t_USINT u8_tmpConsNum = u8_consNum;

      /* completely erase structure */
      CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

      /* initialize the message that will be sent to CSAL */
      s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_CNXN_CLOSE;
      s_halcMsg.u16_len = CSOS_k_SIZEOF_BYTE;
      s_halcMsg.u32_addInfo = (CSS_t_UDINT)u16_instId;
      /* link message with local variable containing the Consumer Number */
      s_halcMsg.pb_data = &u8_tmpConsNum;

      /* Consumer Number */
      CSS_H2N_CPY8(s_halcMsg.pb_data, &u8_consNum);

      /* if sending the message to CSAL is successful */
      if (HALCS_TxDataPut(&s_halcMsg))
      {
        /* success */
        w_retVal = CSS_k_OK;
      }
      else /* else: error */
      {
        /* error while trying to send the message to CSAL */
        w_retVal = (IXSVS_k_NFSE_TXH_HALCS_SEND_CCC);
        IXSVS_ErrorClbk(w_retVal, u16_instId,
                        (CSS_t_UDINT)CSOS_k_CMD_IXSCE_CNXN_CLOSE);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVS_StateInitTimerStop
**
** Description : This function stops the initialization limit timer
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitTimerStop(CSS_t_UINT u16_svsIdx)
{
  au32_initLimit_128us[u16_svsIdx] = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_StateInitLimitCalc
**
** Description : This function calculates and saves the point of time when the
**               the initialization process of a connection must be completed.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit()))
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitLimitCalc(CSS_t_UINT u16_svsIdx)
{
  if (IXSVD_IsBaseFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /* Initialization must be finished in now + 10 seconds */
      au32_initLimit_128us[u16_svsIdx] = IXUTL_GetSysTime()
                                       + k_INITIALIZATION_LIMIT_BASE_128US;
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_32,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* Initialization must be finished in now + 8.3 seconds */
      au32_initLimit_128us[u16_svsIdx] = IXUTL_GetSysTime()
                                       + k_INITIALIZATION_LIMIT_EXT_128US;
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_33,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
  }

  /* if the calculated value is zero */
  if (au32_initLimit_128us[u16_svsIdx] == 0U)
  {
    /* to avoid the need for another flag the value zero is reserved for
       indicating that the timer is currently not in use. Thus if the
       calculation results in zero we just prepone the time limit by one tick */
    au32_initLimit_128us[u16_svsIdx]--;
  }
  else
  {
    /* calculated time limit is valid */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_StateInitLimitCheck
**
** Description : This function checks if the initialization of the connection
**               is in progress. If the initialization is not completed within
**               time limit then the connection is closed. FRS280 states that
**               this is a task of the application but in our implementation
**               this is performed inside CSS.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitLimitCheck(CSS_t_UINT u16_svsIdx)
{
  /* if an initialization process is currently running */
  if (au32_initLimit_128us[u16_svsIdx] != 0U)
  {
    /* if initialization time limit reached */
    if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(),
                                   au32_initLimit_128us[u16_svsIdx]))
    {
      /* Initialization process not completed within time limit */
      /* report to SAPL */
      IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_INIT_LIMIT,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      au32_initLimit_128us[u16_svsIdx]);
      /* deactivate timer */
      au32_initLimit_128us[u16_svsIdx] = 0U;

      /* Set the fault flag to make sure the Connection Status of this
         consuming safety connection instance is reflected correctly */
      IXSVS_ConFaultFlagSet(u16_svsIdx, IXSVD_k_CNXN_FAULTED);

      /* close the connection (see FRS280) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
    }
    else
    {
      /* initialization process still in progress */
    }
  }
  else
  {
    /* initialization not in progress */
  }
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_StateSoftErrByteGet
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
CSS_t_UDINT IXSVS_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                      CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_STATE_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)au8_SvsState)+u32_varCnt);
  }
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_STATE_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)au32_initLimit_128us)
                              +(u32_varCnt - k_STATE_SIZEOF_VAR1));
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_STATE_SIZEOF_VAR12;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVS_StateSoftErrVarGet
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
void IXSVS_StateSoftErrVarGet(
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
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SV_SERVERS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY8(pb_var, &au8_SvsState[au32_cnt[1U]]);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
      }
      break;
    }

    case 1U:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SV_SERVERS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY32(pb_var, &au32_initLimit_128us[au32_cnt[1U]]);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      }
      break;
    }

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
** Function    : StateTransition
**
** Description : This function performs a state transition of the Safety
**               Validator Server state machine. It also indicates this state
**               transition to the SVO unit.
**
** Parameters  : u16_svsIdx (IN)  - index of the Safety Validator Server
**                                  instance
**                                  (not checked, checked by callers)
**               u8_oldState (IN) - the state in which the instance was up to
**                                  now
**                                  (not checked, only called with constants)
**               u8_newState (IN) - new state of this instance
**                                  (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
static void StateTransition(CSS_t_UINT u16_svsIdx,
                            CSS_t_USINT u8_oldState,
                            CSS_t_USINT u8_newState)
{
  au8_SvsState[u16_svsIdx] = u8_newState;

  IXSVO_IxsvsCnxnStateTransClbk(u8_oldState, u8_newState);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


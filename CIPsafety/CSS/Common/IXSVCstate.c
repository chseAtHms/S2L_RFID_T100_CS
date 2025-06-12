/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCstate.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the state machine of the Safety Validator
**             Client instances. Also the Fault Counter (counting faults over
**             all Client instances) is contained in this module.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_StateInit
**             IXSVC_StateMachine
**             IXSVC_StateGet
**             IXSVC_StateIsInitOrEstablished
**             IXSVC_StateNumClientCnxnGet
**             IXSVC_StateUnderlayingComClose
**             IXSVC_FaultCountInc
**             IXSVC_FaultCountGet
**             IXSVC_FaultCountReset
**             IXSVC_StateSoftErrByteGet
**             IXSVC_StateSoftErrVarGet
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

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#include "IXSVCerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** u16_FaultCount:
    Diagnostic Counter that is a running count (with auto-rollover) of Safety
    connection Faults (Client only). Incremented when any producing safety
    connection fails for any reason.
*/
/* Not relevant for Soft Error Checking - only a diagnostic value */
static CSS_t_UINT u16_FaultCount;


/** au8_SvcState:
    Safety Validator Client State Attribute (1) */
static CSS_t_USINT au8_SvcState[CSOS_cfg_NUM_OF_SV_CLIENTS];



/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void StateTransition(CSS_t_UINT u16_svcIdx,
                            CSS_t_USINT u8_oldState,
                            CSS_t_USINT u8_newState);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_StateInit
**
** Description : This function initializes the state variables of a Safety
**               Validator Client instance
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client instance
**                                 (not checked, checked in IXSVC_Init() and
**                                 IXSVC_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_StateInit(CSS_t_UINT u16_svcIdx)
{
  au8_SvcState[u16_svcIdx] = CSS_k_SV_STATE_IDLE;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_StateMachine
**
** Description : This function implements the Safety Validator Object State
**               machine as specified by the CIP Safety Specification.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked by callers)
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, only passed to
**                                 IXSVC_StateUnderlayingComClose() and checked
**                                 there)
**               e_svEvent (IN)  - specifies the event that occurred
**                                 (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_StateMachine(CSS_t_UINT u16_svcIdx,
                        CSS_t_USINT u8_consNum,
                        IXSVC_t_VALIDATOR_EVENT e_svEvent)
{

  /* signal the event to the application (see FRS12-2) */
  SAPL_IxsvcEventHandlerClbk(IXSVC_InstIdFromIdxGet(u16_svcIdx),
                             e_svEvent);

  /* if the event indicates that all consumers are faulted */
  if (e_svEvent == IXSVC_k_VE_ALL_CONS_FAULTED)
  {
    /* close the underlaying connection (see FRS12-4) but not the Safety
       Validator */
    /* ignore return value - errors are already reported to SAPL */
    (void)IXSVC_StateUnderlayingComClose(u16_svcIdx, u8_consNum);
  }
  /* else: if the whole instance is to be closed */
  else if (e_svEvent == IXSVC_k_VE_CLIENT_CLOSE)
  {
    /* close the underlaying connections to all consumers (see FRS12-4) but not
       the Safety Validator (consumer number zero indicates that all consumers
       must be closed) */
    /* ignore return value - errors are already reported to SAPL */
    (void)IXSVC_StateUnderlayingComClose(u16_svcIdx, 0U);
  }
  else /* else: any other event */
  {
    /* event is not of interest for the fault counter */
  }

  /*
  ** Behavior is dependent on current state
  */
  switch (au8_SvcState[u16_svcIdx])
  {
    /* State IDLE and UNALLOCATED are different terms in the CIP Safety       */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_IDLE:
    {
      /* if event is "Client Open" */
      if (e_svEvent == IXSVC_k_VE_CLIENT_OPEN)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_IDLE,
                        CSS_k_SV_STATE_INITIALIZING);
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SM_INV_EV_IDLE,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    case CSS_k_SV_STATE_INITIALIZING:
    {
      /* if event is "First Handshake Completed" */
      if (e_svEvent == IXSVC_k_VE_1ST_HS_COMPLETE)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_ESTABLISHED);
      }
      /* else: if event is "Client Faulted" */
      else if (e_svEvent == IXSVC_k_VE_ALL_CONS_FAULTED)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_FAILED);
      }
      /* else: if event is "Client Close" */
      else if (e_svEvent == IXSVC_k_VE_CLIENT_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_INITIALIZING,
                        CSS_k_SV_STATE_IDLE);
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_JOIN)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_LEAVE)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_FAULTED)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SM_INV_EV_INIT,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    /* State EXECUTING and ESTABLISHED are different terms in the CIP Safety  */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_ESTABLISHED:
    {
      /* if event is "All Consumers Faulted" */
      if (e_svEvent == IXSVC_k_VE_ALL_CONS_FAULTED)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_ESTABLISHED,
                        CSS_k_SV_STATE_FAILED);
      }
      /* else: if event is "Client Close" */
      else if (e_svEvent == IXSVC_k_VE_CLIENT_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_ESTABLISHED,
                        CSS_k_SV_STATE_IDLE);
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_ACTIVE)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_JOIN)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_LEAVE)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_FAULTED)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SM_INV_EV_ESTAB,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    /* State FAILED and FAULTED are different terms in the CIP Safety         */
    /* Specification for the same state.                                      */
    case CSS_k_SV_STATE_FAILED:
    {
      /* if event is "Client Open" */
      if (e_svEvent == IXSVC_k_VE_CLIENT_OPEN)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_FAILED,
                        CSS_k_SV_STATE_INITIALIZING);
      }
      /* else: if event is "Client Close" */
      else if (e_svEvent == IXSVC_k_VE_CLIENT_CLOSE)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_FAILED, CSS_k_SV_STATE_IDLE);
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_JOIN)
      {
        /* state transition */
        StateTransition(u16_svcIdx, CSS_k_SV_STATE_FAILED,
                        CSS_k_SV_STATE_INITIALIZING);
      }
      else if (e_svEvent == IXSVC_k_VE_CONSUMER_LEAVE)
      {
        /* nothing to do, this event is just a notification for the SAPL */
      }
      else /* else: other events */
      {
        /* all other events are not applicable in this state */
        IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SM_INV_EV_FAIL,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)e_svEvent);
      }

      break;
    }

    default:
    {
      /* invalid state! */
      IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SM_INV_STATE,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      (CSS_t_UDINT)au8_SvcState[u16_svcIdx]);
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
** Function    : IXSVC_StateGet
**
** Description : This function returns the state of the requested Safety
**               Validator Client Instance.
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - State of the requested Safety Validator
**                                 Client Instance
**
*******************************************************************************/
CSS_t_USINT IXSVC_StateGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (au8_SvcState[u16_svcIdx]);
}


/*******************************************************************************
**
** Function    : IXSVC_StateIsInitOrEstablished
**
** Description : This function checks the state of the addressed Safety
**               Validator Client Instance
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - state of the instance is
**                                 CSS_k_SV_STATE_INITIALIZING or
**                                 CSS_k_SV_STATE_ESTABLISHED
**               CSS_k_FALSE     - instance is in other state
**
*******************************************************************************/
CSS_t_BOOL IXSVC_StateIsInitOrEstablished(CSS_t_UINT u16_svcIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if the state is Initializing or Established */
  if (    (au8_SvcState[u16_svcIdx] == CSS_k_SV_STATE_INITIALIZING)
       || (au8_SvcState[u16_svcIdx] == CSS_k_SV_STATE_ESTABLISHED)
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


#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVC_StateNumClientCnxnGet
**
** Description : This function returns the number of Safety Validator
**               Client Connections that are currently open (in Initializing or
**               Established state).
**
** See Also    : IXSVS_NumServerCnxnGet
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Number of open client Connections
**
*******************************************************************************/
CSS_t_UINT IXSVC_StateNumClientCnxnGet(void)
{
  /* return value of this function */
  CSS_t_UINT u16_numClientCnxn = 0U;
  /* loop counter */
  CSS_t_UINT u16_svcIdx;

  /* check all Safety Validator Servers */
  for (u16_svcIdx = 0U; u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_svcIdx++)
  {
    /* if the state is Initializing or Established */
    if (IXSVC_StateIsInitOrEstablished(u16_svcIdx))
    {
      /* This Safety Validator is involved in a safety connection */
      u16_numClientCnxn++;
    }
    else /* else: any other states */
    {
      /* this Safety Validator is not involved in a safety connection */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_numClientCnxn);
}
#endif


/*******************************************************************************
**
** Function    : IXSVC_StateUnderlayingComClose
**
** Description : This function sends a close command to the CSAL. For multicast
**               connections: if a consumer number is passed
**               (1..CSOS_k_MAX_CONSUMER_NUM_MCAST) then only the connection
**               to this consumer is closed. If 0 is passed then the connections
**               to all consumers of this instance are closed.
**
** Parameters  : u16_svcIdx (IN) - Safety Validator Server index
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_CLIENTS-1)
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range:
**                                 0 and 1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: HALC not ready
**
*******************************************************************************/
CSS_t_WORD IXSVC_StateUnderlayingComClose(CSS_t_UINT u16_svcIdx,
                                          CSS_t_USINT u8_consNum)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVC_k_FSE_INC_PRG_FLOW);

  /* if instance is invalid */
  if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    /* connection is not open - so it can not be closed */
    w_retVal = (IXSVC_k_FSE_INC_CLOSE_IDX);
    IXSVC_ErrorClbk(w_retVal, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_svcIdx);
  }
  else /* else: instance ok */
  {
    /* Instance ID of the Safety Validator */
    const CSS_t_UINT u16_instId = IXSVC_InstIdFromIdxGet(u16_svcIdx);

    /* if the consumer number is invalid */
    if (u8_consNum > CSOS_k_MAX_CONSUMER_NUM_MCAST)
    {
      /* consumer number invalid! */
      w_retVal = (IXSVC_k_FSE_INC_CLOSE_CONS_NUM);
      IXSVC_ErrorClbk(w_retVal, u16_instId, (CSS_t_UDINT)u8_consNum);
    }
    else /* else: consumer number is ok (consumer number 0 indicates that all
                  consumers must be closed) */
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
        w_retVal = (IXSVC_k_NFSE_TXH_HALCS_SEND_CCC);

        /* error while trying to send the message to CSAL */
        IXSVC_ErrorClbk(w_retVal, u16_instId,
                        (CSS_t_UDINT)CSOS_k_CMD_IXSCE_CNXN_CLOSE);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVC_FaultCountInc
**
** Description : This function increments the value of the Fault Counter of the
**               Safety Validator Client instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_FaultCountInc(void)
{
  /* increment fault counter without considering overflows (see SRS75) */
  u16_FaultCount++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_FaultCountGet
**
** Description : This macro returns the current value of the Fault Counter of
**               the Safety Validator Client instances. Counts (with
**               auto-rollover) any producing safety connection was faulted for
**               any reason.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - value of the Safety Validator Fault Count
**
*******************************************************************************/
CSS_t_UINT IXSVC_FaultCountGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_FaultCount);
}


/*******************************************************************************
**
** Function    : IXSVC_FaultCountReset
**
** Description : This macro resets the value of the Fault Counter of the Safety
**               Validator Client instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_FaultCountReset(void)
{
  u16_FaultCount = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVC_StateSoftErrByteGet
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
CSS_t_UDINT IXSVC_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                      CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(au8_SvcState))
  {
    *pb_var = *(((CSS_t_BYTE*)au8_SvcState)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(au8_SvcState);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVC_StateSoftErrVarGet
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
void IXSVC_StateSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    /* level 0 counter at/above end */
    /* default return values already set */
  }
  else /* level 0 counter in range of array elements */
  {
    CSS_H2N_CPY8(pb_var, &au8_SvcState[au32_cnt[0U]]);
    ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
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
**               Validator Client state machine. It also indicates this state
**               transition to the SVO unit.
**
** Parameters  : u16_svcIdx (IN)  - Instance index of the Safety Validator
**                                  Client instance
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
static void StateTransition(CSS_t_UINT u16_svcIdx,
                            CSS_t_USINT u8_oldState,
                            CSS_t_USINT u8_newState)
{
  /* copy the new state */
  au8_SvcState[u16_svcIdx] = u8_newState;

  /* report via callback to upper layers */
  IXSVO_IxsvcCnxnStateTransClbk(u8_oldState, u8_newState);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


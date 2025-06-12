/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOstate.c
**    Summary: IXSSO - Safety Supervisor Object
**             This module contains the state machine of the Safety Supervisor
**             Object.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_StateMachineInit
**             IXSSO_DeviceStatusGet
**             IXSSO_IxsvcDeviceStatusGetClbk
**             IXSSO_SafetyDeviceMajorFault
**             IXSSO_StateMachine
**             IXSSO_IxsvoCnxnStateTransClbk
**             IXSSO_StateSoftErrByteGet
**             IXSSO_StateSoftErrVarGet
**
**             SsoSmSelfTesting
**             SsoSmIdle
**             SsoSmSelfTestException
**             SsoSmExecuting
**             SsoSmAbort
**             SsoSmCriticalFault
**             SsoSmConfiguring
**             SsoSmWaitingForTunid
**             StateTransition
**             DevStateToCsalSend
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#include "HALCSapi.h"

#include "IXSSS.h"
#include "IXSERapi.h"
#include "IXSVO.h"

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVC.h"
#endif
#include "IXSCF.h"

#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXSSOint.h"
#include "IXSSOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* Device Status Attribute */
static CSS_t_USINT u8_DevStat;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD SsoSmSelfTesting(CSS_t_USINT u8_event);
static CSS_t_WORD SsoSmIdle(CSS_t_USINT u8_event,
                            CSS_t_UINT u16_addInfo);
static CSS_t_WORD SsoSmSelfTestException(CSS_t_USINT u8_event);
static CSS_t_WORD SsoSmExecuting(CSS_t_USINT u8_event,
                                 CSS_t_UINT u16_addInfo);
static CSS_t_WORD SsoSmAbort(CSS_t_USINT u8_event);
static CSS_t_WORD SsoSmCriticalFault(CSS_t_USINT u8_event);
static CSS_t_WORD SsoSmConfiguring(CSS_t_USINT u8_event);
static CSS_t_WORD SsoSmWaitingForTunid(CSS_t_USINT u8_event,
                                       CSS_t_UINT u16_addInfo);
static void StateTransition(CSOS_t_SSO_DEV_STATUS e_newDevStat);
static void DevStateToCsalSend(CSOS_t_SSO_DEV_STATUS e_newDevStat);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSO_StateMachineInit
**
** Description : This function initializes the SSO State Machine.
**               Initial state is Self-Testing and LEDs are switched to
**               Red/Green flashing.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_StateMachineInit(void)
{
  /* set initial state */
  StateTransition(CSOS_k_SSO_DS_SELF_TESTING);
}


/*******************************************************************************
**
** Function    : IXSSO_DeviceStatusGet
**
** Description : This API function may be called by the application to request
**               the device status (current state of the device). If the
**               application implements further CIP objects this function shall
**               be called as all CIP objects in a safety device are subservient
**               to the Safety Supervisor Object (see SRS66).
**
** Parameters  : -
**
** Returnvalue : See type definition of CSOS_t_SSO_DEV_STATUS.
**
*******************************************************************************/
CSOS_t_SSO_DEV_STATUS IXSSO_DeviceStatusGet(void)
{
  CSOS_t_SSO_DEV_STATUS e_devStatus = CSOS_k_SSO_DS_UNDEFINED;

  /* if CIP Safety Stack is in an improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    /* Not an error! CSOS_k_SSO_DS_UNDEFINED is returned. This is also the
       possibility for the SAPL to check the initialization state of the
       stack. */
  }
  else /* else: stack state is ok */
  {
    e_devStatus = (CSOS_t_SSO_DEV_STATUS)u8_DevStat;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (e_devStatus);
}


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : IXSSO_IxsvcDeviceStatusGetClbk
**
** Description : This callback function is called inside the IXSVC unit to
**               obtain the device status (current state of the device). This is
**               needed as all CIP objects in a safety device are subservient to
**               the Safety Supervisor Object (see SRS66).
**
** Parameters  : -
**
** Returnvalue : See type definition of {CSOS_t_SSO_DEV_STATUS}.
**
*******************************************************************************/
CSOS_t_SSO_DEV_STATUS IXSSO_IxsvcDeviceStatusGetClbk(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return IXSSO_DeviceStatusGet();
}
#endif


/*******************************************************************************
**
** Function    : IXSSO_SafetyDeviceMajorFault
**
** Description : This API function is to be used by the safety application to
**               indicate Major Fault events that occurred inside the Safety
**               Application.
**
** Parameters  : o_recoverable (IN) - If CSS_k_TRUE then this is a recoverable
**                                    fault ( == "Internal Abort Request").
**                                    Else this is an unrecoverable fault
**                                    ( == "Critical Fault"). See Volume 5
**                                    section 5-4.5.2 for details on Safety
**                                    Supervisor Object States and Events.
**                                    (not checked, any value allowed)
**
** Returnvalue : -
*******************************************************************************/
void IXSSO_SafetyDeviceMajorFault(CSS_t_BOOL o_recoverable)
{
  /* if it is a recoverable error */
  if (o_recoverable)
  {
    /* Recoverable Major Fault -> Device will transition to Abort state       */
    (void)IXSSO_StateMachine(IXSSO_k_SE_INT_ABORT_REQ, IXSSO_k_SEAI_NONE);
  }
  else /* else: error is unrecoverable */
  {
    /* Unrecoverable Major Fault -> Device will transition to Critical Fault  */
    /* state                                                                  */
    (void)IXSSO_StateMachine(IXSSO_k_SE_CRIT_FAULT, IXSSO_k_SEAI_NONE);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSSO_StateMachine
**
** Description : This function implements the state machine of the Safety
**               Supervisor Object according to the specification.
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (checked, valid range: all IXSSO_k_SE_xxx
**                                  definitions. Unknown events are ignored).
**               u16_addInfo (IN) - additional information - meaning is specific
**                                  to the event. See also {IXSSO_k_SEAI_xxx}.
**                                  (checked, Unknown AddInfos will be ignored)
**
** Returnvalue : CSS_k_OK         - success
**               <>CSS_k_OK       - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_StateMachine(CSS_t_USINT u8_event,
                              CSS_t_UINT u16_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* All device state transitions shall be handled with integrity (see SRS41) */
  /*
  ** check for events that are applicable in any state
  */
  /* if "Critical Fault" event */
  if (u8_event == IXSSO_k_SE_CRIT_FAULT)
  {
    /* Critical Fault! */
    /* Drop all connections and transition to Critical-Fault */
    IXSVO_CnxnDropAll();
    StateTransition(CSOS_k_SSO_DS_CRIT_FAULT);
    w_retVal = CSS_k_OK;
  }
  else /* else: all other events must be handled specific to current state */
  {
    /* switch for current device state */
    switch (u8_DevStat)
    {
      /*
      ** all the states are processed in own sub-functions
      */

      case CSOS_k_SSO_DS_SELF_TESTING:
      {
        w_retVal = SsoSmSelfTesting(u8_event);
        break;
      }

      case CSOS_k_SSO_DS_IDLE:
      {
        w_retVal = SsoSmIdle(u8_event, u16_addInfo);
        break;
      }

      case CSOS_k_SSO_DS_SELF_TEST_EX:
      {
        w_retVal = SsoSmSelfTestException(u8_event);
        break;
      }

      case CSOS_k_SSO_DS_EXECUTING:
      {
        w_retVal = SsoSmExecuting(u8_event, u16_addInfo);
        break;
      }

      case CSOS_k_SSO_DS_ABORT:
      {
        w_retVal = SsoSmAbort(u8_event);
        break;
      }

      case CSOS_k_SSO_DS_CRIT_FAULT:
      {
        w_retVal = SsoSmCriticalFault(u8_event);
        break;
      }

      case CSOS_k_SSO_DS_CONFIGURING:
      {
        w_retVal = SsoSmConfiguring(u8_event);
        break;
      }

      case CSOS_k_SSO_DS_WAIT_TUNID:
      {
        w_retVal = SsoSmWaitingForTunid(u8_event, u16_addInfo);
        break;
      }

      default:
      {
        /* Device is in an undefined state! */
        w_retVal = IXSSO_k_FSE_INC_UNDEF_STATE;
        SAPL_CssErrorClbk((w_retVal),
                          IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_DevStat);

        /* Drop all connections... */
        IXSVO_CnxnDropAll();
        /* ...and bring the device back to one of the defined states */
        StateTransition(CSOS_k_SSO_DS_CRIT_FAULT);
        break;
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    }

    /* if a program flow error appears here this means that the event handling
       function has ignored the event */
    if (w_retVal == IXSSO_k_FSE_INC_PRG_FLOW)
    {
      /* indicate ok to the caller */
      w_retVal = CSS_k_OK;
    }
    else /* else: any other error */
    {
      /* pass the error to the caller */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state machine into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : IXSSO_IxsvoCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Object
**               when a connection is established, deleted or fails.
**
** Parameters  : u8_event (IN)          - Safety Connection event
**                                        (checked, allowed value see definition
**                                        of {IXSVO_k_CNXN_EVENT_xxx})
**               u16_numEstabCnxns (IN) - number of connections that are
**                                        currently in the established state
**                                        (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_IxsvoCnxnStateTransClbk(CSS_t_USINT u8_event,
                                         CSS_t_UINT u16_numEstabCnxns)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /*
  ** event translation (SVO -> SSO)
  */
  /* if "Connection Failed" event */
  if (u8_event == IXSVO_k_CNXN_EVENT_FAIL)
  {
    w_retVal = IXSSO_StateMachine(IXSSO_k_SE_SCNXN_FAIL, u16_numEstabCnxns);
  }
  /* else: if "Connection Established" event */
  else if (u8_event == IXSVO_k_CNXN_EVENT_ESTAB)
  {
    w_retVal = IXSSO_StateMachine(IXSSO_k_SE_SCNXN_ESTAB, u16_numEstabCnxns);
  }
  /* else: if "Connection Deleted" event */
  else if (u8_event == IXSVO_k_CNXN_EVENT_DEL)
  {
    w_retVal = IXSSO_StateMachine(IXSSO_k_SE_SCNXN_DEL, u16_numEstabCnxns);
  }
  else /* else: any other event */
  {
    w_retVal = (IXSSO_k_FSE_INC_CNXN_EVENT);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_event);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSSO_StateSoftErrByteGet
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
CSS_t_UDINT IXSSO_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                      CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(u8_DevStat))
  {
    *pb_var = *(((CSS_t_BYTE*)&u8_DevStat)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(u8_DevStat);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSSO_StateSoftErrVarGet
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
void IXSSO_StateSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* there is only one variable so this is the only variable in level 1 */
  /* if level counter is larger than number of variables */
  if (au32_cnt[0U] > 0U)
  {
    /* return value already set */
  }
  else
  {
    CSS_H2N_CPY8(pb_var, &u8_DevStat);
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
** Function    : SsoSmSelfTesting
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Self Testing
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmSelfTesting(CSS_t_USINT u8_event)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);
  /* get a pointer to our TUNID (list) */
  const CSS_t_UNID *pas_ourTunid = IXSSO_TunidArrayPtrGet();

  /* if "Self-Test passed" event */
  if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    /* if device does not have a valid TUNID */
    if (!IXSSO_DeviceHasValidTunid(pas_ourTunid))
    {
      /* state transition to Waiting-For-TUNID */
      StateTransition(CSOS_k_SSO_DS_WAIT_TUNID);
    }
    else
    {
      /* If TUNID represents a valid number (not equal to out-of-box) */
      /* this number shall be compared to the NodeID (see SRS119). */
      /* if our TUNID doesn't match our NodeID */
      /* In case of a Multiple Safety Port Device all TUNIDs and NodeIDs */
      if (!IXSSO_TunidVsNodeIdCheck(pas_ourTunid))
      {
        /* state transition to Abort */
        StateTransition(CSOS_k_SSO_DS_ABORT);
      }
      else /* else: TUNID matches SCID */
      {
        /* if our device does not have a valid configuration (SCID == 0) */
        if (IXSSO_ScidIsZero())
        {
          /* state transition to Configuring (see FRS116-2) */
          StateTransition(CSOS_k_SSO_DS_CONFIGURING);
        }
        else /* else: device has a valid configuration */
        {
          /* state transition to Idle */
          StateTransition(CSOS_k_SSO_DS_IDLE);
        }
      }
    }
  }
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    /* state transition to Self-Test-Exception */
    StateTransition(CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* not allowed in current state */
    w_retVal = (IXSSO_k_FSE_AIS_OSC_ST_AB);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_RXE_FO_SELF_TEST);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_FSE_RXE_PROP_T_SELFTEST);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_RXE_APLY_T_SELFTEST);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TESTING);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TESTING);
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TESTING);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TESTING);
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TESTING);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    w_retVal = (IXSSO_k_FSE_RXE_SRESET_SELFTEST);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    **  - IXSSO_k_SE_APPLY_REQ
    **  - IXSSO_k_SE_SCNXN_FAIL
    **  - IXSSO_k_SE_SCNXN_DEL
    **  - IXSSO_k_SE_T1_SOPEN
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */



/*******************************************************************************
**
** Function    : SsoSmIdle
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Idle
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**               u16_addInfo (IN) - additional information - meaning is specific
**                                  to the event. See also {IXSSO_k_SEAI_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmIdle(CSS_t_USINT u8_event,
                            CSS_t_UINT u16_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Mode Change" event */
  if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    /* if additional info indicates "change to Executing" */
    if (u16_addInfo == IXSSO_k_MC_EXEC)
    {
      StateTransition(CSOS_k_SSO_DS_EXECUTING);
    }
    else /* else: additional info indicates "change to Idle" */
    {
      /* already in requested state - ignore event */
    }
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* state transition to Abort */
    StateTransition(CSOS_k_SSO_DS_ABORT);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    /* Forward_Open requests are accepted in Idle state */
    w_retVal = CSS_k_OK;
  }
  /* else: if "Type 1 SafetyOpen" event */
  else if (u8_event == IXSSO_k_SE_T1_SOPEN)
  {
    /* state transition to Configuring */
    StateTransition(CSOS_k_SSO_DS_CONFIGURING);
  }
#endif
  /* else: if "Safety I/O Connection failed/established/deleted" events */
  else if (    (u8_event == IXSSO_k_SE_SCNXN_FAIL)
            || (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
            || (u8_event == IXSSO_k_SE_SCNXN_DEL)
          )
  {
    /* Let the Safety Application decide what to do */
    /* if Safety Application permits the mode change */
    if (SAPL_IxssoProfDepStateChangeClbk(CSOS_k_SSO_DS_IDLE,
                                         u8_event,
                                         u16_addInfo))
    {
      /* state transition to Executing */
      StateTransition(CSOS_k_SSO_DS_EXECUTING);
    }
    else /* else: Safety Application does not permit Mode Change */
    {
      /* stay in the current state */
    }
  }
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    /* Connection opening is accepted in Idle state */
    w_retVal = CSS_k_OK;
  }
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
#endif
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_IDLE);
  }
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SsoSmSelfTestException
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Self Test Exception
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmSelfTestException(CSS_t_USINT u8_event)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Exception Condition Cleared" event */
  if (u8_event == IXSSO_k_SE_EX_COND_CLEAR)
  {
    /* Restart self-test. Will cause transition to Self-Testing */
    /* We cannot do this here directly as this would cause recursive function */
    /* calls. Thus indicate this to the caller by a special return value      */
    w_retVal = (IXSSO_k_NFSE_AIS_INFO_EXCON_CLR);
    /* We do not call the Error Callback here as this is not an error - just  */
    /* an Info to the caller!                                                 */
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* not allowed in current state */
    w_retVal = (IXSSO_k_FSE_AIS_OSC_STEX_AB);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_FO_SELF_TESTEX);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if Connection Delete */
  else if (u8_event == IXSSO_k_SE_SCNXN_DEL)
  {
    w_retVal = (IXSSO_k_FSE_RXE_DEL_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if Connection fails */
  else if (u8_event == IXSSO_k_SE_SCNXN_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_RXE_FAIL_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_SELF_TEST_EX);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_APPLY_REQ
    **  - IXSSO_k_SE_T1_SOPEN
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SsoSmExecuting
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Executing
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**               u16_addInfo (IN) - additional information - meaning is specific
**                                  to the event. See also {IXSSO_k_SEAI_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmExecuting(CSS_t_USINT u8_event,
                                 CSS_t_UINT u16_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Mode Change" event */
  if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    /* if additional info indicates "change to Idle" */
    if (u16_addInfo == IXSSO_k_MC_IDLE)
    {
      /* state transition to Idle */
      StateTransition(CSOS_k_SSO_DS_IDLE);
    }
    else /* else: additional info indicates "change to Executing" */
    {
      /* already in requested state - ignore event */
    }
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* state transition to Abort */
    StateTransition(CSOS_k_SSO_DS_ABORT);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    /* Forward_Open requests are accepted in Executing state */
    w_retVal = CSS_k_OK;
  }
  /* else: if "Type 1 SafetyOpen" event */
  else if (u8_event == IXSSO_k_SE_T1_SOPEN)
  {
    /* devices being configured shall not have any active safety         */
    /* connections and shall not be in the executing mode (see FRS116-1) */
    /* Drop connections and set application to safe state */
    IXSVO_CnxnDropAll();

    /* state transition to Configuring */
    StateTransition(CSOS_k_SSO_DS_CONFIGURING);
  }
#endif
  /* else: if "Safety I/O Connection failed/established/deleted" events */
  else if (    (u8_event == IXSSO_k_SE_SCNXN_FAIL)
            || (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
            || (u8_event == IXSSO_k_SE_SCNXN_DEL)
          )
  {
    /* Let the Safety Application decide what to do */
    /* if Safety Application permits the mode change */
    if (SAPL_IxssoProfDepStateChangeClbk(CSOS_k_SSO_DS_EXECUTING,
                                         u8_event,
                                         u16_addInfo))
    {
      /* state transition to Executing */
      StateTransition(CSOS_k_SSO_DS_IDLE);
    }
    else /* else: Safety Application does not permit Mode Change */
    {
      /* stay in state */
    }
  }
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    /* Connection opening is accepted in Executing state */
    w_retVal = CSS_k_OK;
  }
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
#endif
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_EXECUTING);
  }
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    **  - IXSSO_k_SE_APPLY_REQ
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state machine into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */



/*******************************************************************************
**
** Function    : SsoSmAbort
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Abort
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmAbort(CSS_t_USINT u8_event)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Self-Test failed" event */
  if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_FO_ABORT);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* already in requested state */
    w_retVal = (IXSSO_k_FSE_AIS_AIRS_IAR_AB);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
  /* else: if Connection Delete */
  else if (u8_event == IXSSO_k_SE_SCNXN_DEL)
  {
    /* existing connections may be deleted in this state (though it is not
       possible to create new ones) */
    w_retVal = CSS_k_OK;
  }
  /* else: if Connection fails */
  else if (u8_event == IXSSO_k_SE_SCNXN_FAIL)
  {
    /* existing connections may fail in this state (though it is not possible
       to create new ones) */
    w_retVal = CSS_k_OK;
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_ABORT);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    /* Basically Safety Reset is allowed in the Abort state. E.g. for moving
       the device to the "Waiting for TUNID" state in case of a Switch-NVS
       setting mismatch with a reset to out-of-box settings (see SRS120). */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    **  - IXSSO_k_SE_APPLY_REQ
    **  - IXSSO_k_SE_T1_SOPEN
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SsoSmCriticalFault
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Critical Fault
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmCriticalFault(CSS_t_USINT u8_event)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Internal Abort Request" event */
  if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* In Critical Fault it is not allowed to have an Abort request */
    w_retVal = (IXSSO_k_FSE_AIS_OSC_CRIT_AB);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_FO_CRIT_FAULT);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
  /* else: if Connection Delete */
  else if (u8_event == IXSSO_k_SE_SCNXN_DEL)
  {
    /* existing connections may be deleted in this state (though it is not
       possible to create new ones) */
    w_retVal = CSS_k_OK;
  }
  /* else: if Connection fails */
  else if (u8_event == IXSSO_k_SE_SCNXN_FAIL)
  {
    /* existing connections may fail in this state (though it is not possible
       to create new ones) */
    w_retVal = CSS_k_OK;
  }
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CRIT_FAULT);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* There is no way out of this state. Any events are ignored.       */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    **  - IXSSO_k_SE_APPLY_REQ
    **  - IXSSO_k_SE_T1_SOPEN
    **  - IXSSO_k_SE_CRIT_FAULT
    */
    /* To clear this fault it needs a reset or power cycle (see         */
    /* FRS121). This causes the self-tests to be executed again (see    */
    /* FRS122).                                                         */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SsoSmConfiguring
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Configuring
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmConfiguring(CSS_t_USINT u8_event)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Apply Request" event */
  if (u8_event == IXSSO_k_SE_APPLY_REQ)
  {
    /* state transition to Idle */
    StateTransition(CSOS_k_SSO_DS_IDLE);
  }
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* state transition to Abort */
    StateTransition(CSOS_k_SSO_DS_ABORT);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward_Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    /* Forward_Open requests are accepted in Configuring state (during
       Forward_Open processing Type 2 will be rejected and valid Type 1 can
       be accepted). */
    w_retVal = CSS_k_OK;
  }
  /* else: if "Type 1 SafetyOpen" event */
  else if (u8_event == IXSSO_k_SE_T1_SOPEN)
  {
    /* Event ignored here. Transition to Idle will occur when the "Apply"
       follows subsequently. */
    /* Transition to Executing will occur when connection establishment is
       finished. */
    w_retVal = CSS_k_OK;
  }
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* else: if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* This service is only allowed in the Waiting-For-TUNID state */
    /* (see SRS194) */
    w_retVal = (IXSSO_k_NFSE_RXE_PROP_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_APY_TUNID_CHK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    /* in this state the Validate_Configuration is allowed */
    w_retVal = CSS_k_OK;
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    /* in this state the Validate_Configuration is allowed */
    w_retVal = CSS_k_OK;
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
  /* else: if Connection Delete */
  else if (u8_event == IXSSO_k_SE_SCNXN_DEL)
  {
    /* Ignore event */
    w_retVal = CSS_k_OK;
  }
  /* else: if Connection fails */
  else if (u8_event == IXSSO_k_SE_SCNXN_FAIL)
  {
    /* Ignore event */
    w_retVal = CSS_k_OK;
  }
  /* else: if "Self-Test failed" event */
  else if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_CONFIGURING);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_APPLY_TUNID
    **  - IXSSO_k_SE_EX_COND_CLEAR
    */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SsoSmWaitingForTunid
**
** Description : This function implements one state of the Safety Supervisor
**               state machine: Waiting for TUNID
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (not checked, any value allowed)
**               u16_addInfo (IN) - additional information - meaning is specific
**                                  to the event. See also {IXSSO_k_SEAI_xxx}.
**                                  (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
static CSS_t_WORD SsoSmWaitingForTunid(CSS_t_USINT u8_event,
                                       CSS_t_UINT u16_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if "Self-Test failed" event */
  if (u8_event == IXSSO_k_SE_ST_FAIL)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_FAIL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
  /* else: if "Self-Test passed" event */
  else if (u8_event == IXSSO_k_SE_ST_PASS)
  {
    w_retVal = (IXSSO_k_FSE_INC_SELF_TEST_PASS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* This service is only allowed in the Waiting-For-TUNID state */
  /* (see SRS194) */
  /* if "Propose TUNID" event */
  else if (u8_event == IXSSO_k_SE_PROP_TUNID)
  {
    /* if additional info indicates that the proposing TUNID operation
       starts */
    if (u16_addInfo == IXSSO_k_SEAI_PROP_TUNID_START)
    {
      /* start Network Status LED flash sequence */
      HALCS_NetStatLedSet(HALCS_k_LED_FLASH_RED_GREEN);
    }
    else /* else: stop the TUNID proposing operation */
    {
      /* stop Network Status LED flash sequence */
      HALCS_NetStatLedSet(HALCS_k_LED_FLASH_GREEN);
    }
  }
  /* if "Apply TUNID" event */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID)
  {
    /* stop Network Status LED flash sequence */
    HALCS_NetStatLedSet(HALCS_k_LED_FLASH_GREEN);

    /* state transition to Configuring */
    StateTransition(CSOS_k_SSO_DS_CONFIGURING);
  }
  /* else: if "Apply TUNID" is being checked */
  else if (u8_event == IXSSO_k_SE_APPLY_TUNID_CHECK)
  {
    /* in this state the Apply TUNID service is allowed */
    w_retVal = CSS_k_OK;
  }
#endif
  /* else: if "Internal Abort Request" event */
  else if (u8_event == IXSSO_k_SE_INT_ABORT_REQ)
  {
    /* stop Network Status LED flash sequence */
    HALCS_NetStatLedSet(HALCS_k_LED_FLASH_GREEN);

    /* state transition to Abort */
    StateTransition(CSOS_k_SSO_DS_ABORT);
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if Forward_Open Request */
  else if (u8_event == IXSSO_k_SE_FWD_OPEN_REQ)
  {
    w_retVal = (IXSSO_k_NFSE_RXE_FO_WAIT_TUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if Validate_Configuration Request */
  else if (u8_event == IXSSO_k_SE_VALIDATE_CFG)
  {
    w_retVal = (IXSSO_k_FSE_AIS_VAL_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
  /* else: if Apply Request */
  else if (u8_event == IXSSO_k_SE_APPLY_CHECK)
  {
    w_retVal = (IXSSO_k_FSE_AIS_APPLY_CFG_CHECK);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
#endif
  /* else: if Mode Change Request */
  else if (u8_event == IXSSO_k_SE_MODE_CHANGE)
  {
    w_retVal = (IXSSO_k_FSE_AIS_MODE_CHANGE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
  /* else: if Connection Established */
  else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
  {
    w_retVal = (IXSSO_k_FSE_RXE_OPEN_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
  /* else: if Connection Delete */
  else if (u8_event == IXSSO_k_SE_SCNXN_DEL)
  {
    /* In this state there cannot exist any safety I/O connections. Thus it is
       not possible that a safety connection deleted event appears */
    w_retVal = (IXSSO_k_FSE_INC_SC_DEL_IN_WFT);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if Connection fails */
  else if (u8_event == IXSSO_k_SE_SCNXN_FAIL)
  {
    /* In this state there cannot exist any safety I/O connections. Thus it is
       not possible that a safety connection failed event appears */
    w_retVal = (IXSSO_k_FSE_INC_SC_FAIL_IN_WFT);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* else: if CCO tries to open a connection */
  else if (u8_event == IXSSO_k_SE_OPEN_CNXN_REQ)
  {
    w_retVal = (IXSSO_k_FSE_AIS_CCO_OP_NOT_PSBL);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)CSOS_k_SSO_DS_WAIT_TUNID);
  }
#endif
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /* else: if Safety_Reset requested */
  else if (u8_event == IXSSO_k_SE_RESET_REQ)
  {
    /* ignore event - reset implementation must be done in user application */
    w_retVal = CSS_k_OK;
  }
#endif
  else /* else: any other event */
  {
    /* event undefined or not applicable in this state - ignore event */
    /* These are:
    **  - IXSSO_k_SE_EX_COND_CLEAR
    **  - IXSSO_k_SE_APPLY_REQ
    **  - IXSSO_k_SE_T1_SOPEN
    */
  #if (CSS_cfg_SET_TUNID != CSS_k_ENABLE)
    {
      /* just to mute compiler warning */
      u16_addInfo = u16_addInfo;
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);  /*lint !e438 Last value assigned to variable
                                  'u16_addInfo' may be unused (depending on
                                  configuration,
                                  i.e. CSS_cfg_SET_TUNID == CSS_k_DISABLE) */
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : StateTransition
**
** Description : This function performs a transition to the passed state. It
**               also performs actions that need to be executed always upon
**               entering this state (e.g. switch LED to required pattern).
**
** Parameters  : e_newDevStat (IN) - New state of the device (see type
**                                   definitions of {CSOS_t_SSO_DEV_STATUS})
**                                   (checked, allowed values see type
**                                   definitions of {CSOS_t_SSO_DEV_STATUS})
**
** Returnvalue : -
**
*******************************************************************************/
static void StateTransition(CSOS_t_SSO_DEV_STATUS e_newDevStat)
{
  /* perform state transition */
  u8_DevStat = (CSS_t_USINT)e_newDevStat;

  /* set Module Status LED according to new state */
  switch (u8_DevStat)
  {
    case CSOS_k_SSO_DS_SELF_TESTING:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_RED_GREEN);
      break;
    }

    case CSOS_k_SSO_DS_IDLE:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_GREEN);
      break;
    }

    case CSOS_k_SSO_DS_SELF_TEST_EX:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_RED);
      break;
    }

    case CSOS_k_SSO_DS_EXECUTING:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_GREEN);
      break;
    }

    case CSOS_k_SSO_DS_ABORT:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_RED);
      break;
    }

    case CSOS_k_SSO_DS_CRIT_FAULT:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_RED);
      break;
    }

    case CSOS_k_SSO_DS_CONFIGURING:
    {
      CSS_t_DATE_AND_TIME s_scts;

      /* completely erase structure */
      CSS_MEMSET(&s_scts, 0, sizeof(s_scts));

      /*
      ** When a device enters Configuring mode, the SCID attribute shall be set
      ** to 0 and maintained at this value (through power cycles) (see FRS116-2
      ** and SRS136) until a successful Validate has been executed (see SRS198).
      */
      /* if setting the SCID fails */
      if (IXSSO_ScidSet(0UL, &s_scts) != CSS_k_OK)
      {
        /* error already handled within IXSSO_ScidSet() */
      }
      else /* else: success */
      {
      }
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_RED_GREEN);
      break;
    }

    case CSOS_k_SSO_DS_WAIT_TUNID:
    {
      HALCS_ModStatLedSet(HALCS_k_LED_FLASH_RED_GREEN);
      break;
    }

    default:
    {
      /* A transition out of the known states can only happen by accident */
      SAPL_CssErrorClbk(IXSSO_k_FSE_INC_TRANS_INV_STATE, IXSER_k_I_NOT_USED,
                        (CSS_t_UDINT)u8_DevStat);

      /* Drop all connections... */
      IXSVO_CnxnDropAll();
      /* ... and transition to Critical-Fault */
      u8_DevStat = (CSS_t_USINT)CSOS_k_SSO_DS_CRIT_FAULT;
      HALCS_ModStatLedSet(HALCS_k_LED_RED);
      break;
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  }

  /* Also inform CSAL about the change of the Device State */
  DevStateToCsalSend((CSOS_t_SSO_DEV_STATUS)u8_DevStat);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : DevStateToCsalSend
**
** Description : This function sends the passed device state to CSAL via HALC.
**               CSAL needs to know all Safety Supervisor state changes because
**               Safety devices replace their Identity object state behavior
**               with the behavior of the Safety Supervisor object (see SRS142).
**
** Parameters  : e_devStat (IN) - Device state. See definitions of
**                                {CSOS_t_SSO_DEV_STATUS})
**                                (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
static void DevStateToCsalSend(CSOS_t_SSO_DEV_STATUS e_devStat)
{
  /* HALC message to be transmitted to CSAL */
  HALCS_t_MSG s_halcTxMsg;

  /* completely erase structure */
  CSS_MEMSET(&s_halcTxMsg, 0, sizeof(s_halcTxMsg));

  /* prepare message */
  s_halcTxMsg.u16_cmd = CSOS_k_CMD_IXSSO_DEV_STATE;
  s_halcTxMsg.u16_len = 0U;
  s_halcTxMsg.u32_addInfo = (CSS_t_UDINT)e_devStat;
  s_halcTxMsg.pb_data = CSS_k_NULL;

  /* if sending Device State to CSAL fails */
  if (!HALCS_TxDataPut(&s_halcTxMsg))
  {
    SAPL_CssErrorClbk((IXSSO_k_NFSE_TXH_DEVST),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
  {
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*** End Of File ***/



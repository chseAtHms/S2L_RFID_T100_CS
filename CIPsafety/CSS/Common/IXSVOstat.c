/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOstat.c
**    Summary: IXSVO - Safety Validator Object
**             This module contains functions to request the state of a Safety
**             Validator instance and to process state transitions in Safety
**             Validator instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_Init
**             IXSVO_InstanceInfoGet
**             IXSVO_IxsvcCnxnStateTransClbk
**             IXSVO_IxsvsCnxnStateTransClbk
**
**             CnxnStateTransition
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
#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVCapi.h"
  #include "IXSVC.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #include "IXSVSapi.h"
  #include "IXSVS.h"
#endif
#include "IXSCF.h"

#include "IXSVOapi.h"
#include "IXSVO.h"
#include "IXSVOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** t_CNXN_COUNTERS:
    Data type for counting how many connections are established and how many are
    failed.
*/
typedef struct
{
  CSS_t_UINT u16_estab;
  CSS_t_UINT u16_fail;
}t_CNXN_COUNTERS;

/* connection counter variables */
/* Not relevant for Soft Error Checking - not part of the safety function */
static t_CNXN_COUNTERS s_CnxnCnt;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void CnxnStateTransition(CSS_t_USINT u8_oldState,
                                CSS_t_USINT u8_newState);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVO_Init
**
** Description : This function initializes the IXSVO unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_Init(void)
{
  /* make sure the complete structure is zero */
  CSS_MEMSET(&s_CnxnCnt, 0, sizeof(s_CnxnCnt));

  /* initialize unit global variables */
  s_CnxnCnt.u16_estab = 0U;
  s_CnxnCnt.u16_fail  = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVO_InstanceInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Object Instance (e.g. state, status
**               (see FRS125), type, etc.). See type definition of
**               {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range: 1..
**                                   (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS))
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_InstanceInfoGet(CSS_t_UINT u16_instId,
                           CSS_t_VALIDATOR_INFOS *ps_svoInfos)
{
  /* if pointer to output is invalid */
  if (ps_svoInfos == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_PTR_INV_STATGET,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: parameters are valid */
  {
    /* completely erase structure */
    CSS_MEMSET(ps_svoInfos, 0, sizeof(*ps_svoInfos));

    /* initialize output parameters */
    ps_svoInfos->u8_state             = CSS_k_SV_STATE_IDLE;
    ps_svoInfos->u8_type              = CSS_k_TYPE_CLIENT | CSS_k_TYPE_SCAST;
    ps_svoInfos->u16_maxDataAge_128us = 0xFFFFU;
    ps_svoInfos->u16_cnxnPoint        = CSOS_k_INVALID_INSTANCE;

    /* if passed parameter is invalid */
    if (    (u16_instId == 0U)
         || (u16_instId > (CSOS_cfg_NUM_OF_SV_CLIENTS
                          +CSOS_cfg_NUM_OF_SV_SERVERS))
       )
    {
      SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_INST_STATEGET, u16_instId,
                        IXSER_k_A_NOT_USED);
    }
    else /* else: instance ID is ok */
    {
      /* if CIP Safety Stack is in an improper state */
      if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
      {
        /* CIP Safety Stack is not in running state */
        SAPL_CssErrorClbk(IXSVO_k_FSE_AIS_STATE_ERR_IIG,
                          u16_instId, IXSER_k_A_NOT_USED);
      }
      else /* else: CSS is in a proper state */
      {
        /* if the requested instance is currently not open */
        if (IXSVD_SvIdxFromInstGet(u16_instId) == CSOS_k_INVALID_IDX)
        {
          /* ps_svoInfos already initialized */
          /* do nothing */
        }
        /* else: if it is a Server instance */
        else if (IXSVD_InstIsServer(u16_instId))
        {
        #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
          {
            IXSVS_InstInfoGet(u16_instId, ps_svoInfos);
          }
        #else
          {
            SAPL_CssErrorClbk(IXSVO_k_FSE_INC_IIG_INST_S,
                              u16_instId, IXSER_k_A_NOT_USED);
          }
        #endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */
        }
        else /* else: it is a Client instance */
        {
        #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
          {
            IXSVC_InstInfoGet(u16_instId, ps_svoInfos);
          }
        #else
          {
            SAPL_CssErrorClbk(IXSVO_k_FSE_INC_IIG_INST_C,
                              u16_instId, IXSER_k_A_NOT_USED);
          }
        #endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : IXSVO_IxsvcCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Client
**               whenever the state of an instance changes.
**
** Parameters  : u8_oldState (IN) - the state in which the safety validator
**                                  instance was up to now
**                                  (not checked - any value allowed)
**               u8_newState (IN) - new state of this instance
**                                  (not checked - checked in
**                                  CnxnStateTransition())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_IxsvcCnxnStateTransClbk(CSS_t_USINT u8_oldState,
                                   CSS_t_USINT u8_newState)
{
  /* call function that handles state transitions of Safety Validator Clients */
  /* and Servers in common */
  CnxnStateTransition(u8_oldState, u8_newState);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
/*******************************************************************************
**
** Function    : IXSVO_IxsvsCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Server
**               whenever the state of an instance changes.
**
** Parameters  : u8_oldState (IN) - the state in which the safety validator
**                                  instance was up to now
**                                  (not checked - any value allowed)
**               u8_newState (IN) - new state of this instance
**                                  (not checked - checked in
**                                  CnxnStateTransition())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_IxsvsCnxnStateTransClbk(CSS_t_USINT u8_oldState,
                                   CSS_t_USINT u8_newState)
{
  /* call function that handles state transitions of Safety Validator Clients */
  /* and Servers in common */
  CnxnStateTransition(u8_oldState, u8_newState);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : CnxnStateTransition
**
** Description : This callback function is called by the Safety Validator Client
**               and Server whenever the state of an instance changes.
**
** Parameters  : u8_oldState (IN) - the state in which the safety validator
**                                  instance was up to now
**                                  (not checked - any value allowed)
**               u8_newState (IN) - new state of this instance
**                                  (checked -
**                                  valid range: see {CSS_k_SV_STATE_xxx})
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnStateTransition(CSS_t_USINT u8_oldState,
                                CSS_t_USINT u8_newState)
{
  /* for checking errors of called functions */
  CSS_t_WORD w_err = 0U;

  /* switch for the new state */
  switch (u8_newState)
  {
    case CSS_k_SV_STATE_IDLE:
    {
      /* if we leave the established state */
      if (u8_oldState == CSS_k_SV_STATE_ESTABLISHED)
      {
        s_CnxnCnt.u16_estab--;
      }
      /* else: if we leave the failed state */
      else if (u8_oldState == CSS_k_SV_STATE_FAILED)
      {
        s_CnxnCnt.u16_fail--;
      }
      /* else: if we leave the initializing state */
      else if (u8_oldState == CSS_k_SV_STATE_INITIALIZING)
      {
        /* not of interest here */
      }
      else /* else: we come from some other state */
      {
        /* other state transitions are not possible */
        w_err = (IXSVO_k_FSE_INC_INV_TRANS_IDLE);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }

      /* A connection has been deleted. This event must be reported to        */
      /* the Safety Supervisor. Also report how many connections are          */
      /* currently in established state.                                      */
      if (IXSSO_IxsvoCnxnStateTransClbk(IXSVO_k_CNXN_EVENT_DEL,
                                        s_CnxnCnt.u16_estab) != CSS_k_OK)
      {
        w_err = (IXSVO_k_FSE_INC_VDATR_STAT_IDLE);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }
      else
      {
        /* ok */
      }

      break;
    }

    case CSS_k_SV_STATE_INITIALIZING:
    {
      /* if we leave the failed state */
      if (u8_oldState == CSS_k_SV_STATE_FAILED)
      {
        s_CnxnCnt.u16_fail--;
      }
      /* else: if we leave the idle state */
      else if (u8_oldState == CSS_k_SV_STATE_IDLE)
      {
        /* not of interest here */
      }
      else /* else: we come from some other state */
      {
        /* other state transitions are not possible */
        w_err = (IXSVO_k_FSE_INC_INV_TRANS_INIT);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }
      break;
    }

    case CSS_k_SV_STATE_ESTABLISHED:
    {
      /* if we leave the initializing state */
      if (u8_oldState == CSS_k_SV_STATE_INITIALIZING)
      {
        /* count number of established connections */
        s_CnxnCnt.u16_estab++;
      }
      else /* else: we come from some other state */
      {
        /* other state transitions are not possible */
        w_err = (IXSVO_k_FSE_INC_INV_TRANS_ESTAB);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }

      /* A connection has been established. This event must be reported to    */
      /* the Safety Supervisor. Also report how many connections are now in   */
      /* established state.                                                   */
      if (IXSSO_IxsvoCnxnStateTransClbk(IXSVO_k_CNXN_EVENT_ESTAB,
                                        s_CnxnCnt.u16_estab) != CSS_k_OK)
      {
        w_err = (IXSVO_k_FSE_INC_VDATR_STAT_ESTB);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }
      else
      {
        /* ok */
      }

      break;
    }

    case CSS_k_SV_STATE_FAILED:
    {
      /* if we leave the established state */
      if (u8_oldState == CSS_k_SV_STATE_ESTABLISHED)
      {
        s_CnxnCnt.u16_estab--;
        s_CnxnCnt.u16_fail++;
      }
      /* else: if we leave the initializing state */
      else if (u8_oldState == CSS_k_SV_STATE_INITIALIZING)
      {
        /* count number of failed connections */
        s_CnxnCnt.u16_fail++;
      }
      else /* else: we come from some other state */
      {
        /* other state transitions are not possible */
        w_err = (IXSVO_k_FSE_INC_INV_TRANS_FAIL);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }

      /* A connection failed. This event must be reported to the Safety       */
      /* Supervisor. Also report how many connections are currently in        */
      /* established state.                                                   */
      if (IXSSO_IxsvoCnxnStateTransClbk(IXSVO_k_CNXN_EVENT_FAIL,
                                        s_CnxnCnt.u16_estab) != CSS_k_OK)
      {
        w_err = (IXSVO_k_FSE_INC_VDATR_STAT_FAIL);
        SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      }
      else
      {
        /* ok */
      }

      break;
    }

    default:
    {
      /* Fatal error: state machine is out of the valid states */
      w_err = (IXSVO_k_FSE_INC_VDATR_STAT_INV);
      SAPL_CssErrorClbk(w_err, IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_oldState);
      break;
    }
  }

  /*
  ** determine how the Network Status LED must be switched in current state
  */
  /* if there are connections in the failed state */
  if (s_CnxnCnt.u16_fail > 0U)
  {
    /* Connection Failed */
    HALCS_NetStatLedSet(HALCS_k_LED_FLASH_RED);
  }
  /* else: if there are connections in the established state */
  else if (s_CnxnCnt.u16_estab > 0U)
  {
    /* Link OK, On-line, Connected */
    HALCS_NetStatLedSet(HALCS_k_LED_GREEN);
  }
  else /* else: there are no connections existing currently */
  {
    /* On-line, Not Connected */
    HALCS_NetStatLedSet(HALCS_k_LED_FLASH_GREEN);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
/* This function has a lot of paths. But dividing this state-handler into
   several functions wouldn't make understanding the code easier */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*** End Of File ***/


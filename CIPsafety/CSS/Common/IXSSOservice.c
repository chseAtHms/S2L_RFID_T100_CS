/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOservice.c
**    Summary: IXSSO - Safety Supervisor Object
**             This module contains the implementation of the CIP services of
**             the Safety Supervisor Object. It supports the baseline Safety
**             Supervisor definition (see SRS65).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_Init
**             IXSSO_ExceptionStatusSet
**             IXSSO_ExplMsgHandler
**             IXSSO_ServiceSoftErrByteGet
**             IXSSO_ServiceSoftErrVarGet
**
**             DataInit
**             SrvcDataInit
**             ExceptionStatusGet
**             SrvcClassGetAttrSingle
**             SrvcInstGetAttrSingle
**             SrvcInstSetAttrSingle
**             SrvcInstSafetyReset
**             SrvcInstProposeTunid
**             SrvcInstApplyTunid
**             SrvcInstProposeTunidList
**             SrvcInstApplyTunidList
**             UnidListAttrGet
**             ReceivedTunidListCopyandCheck
**             PortsAndTunidCheck
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
#if (    (    (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)                   \
           && (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)                        \
         )                                                             \
      || (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)       \
    )
  #include "CSS.h"
#endif

#include "IXSERapi.h"
#include "IXSSS.h"
#include "IXSCF.h"
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    #include "IXSVC.h"
  #endif
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    #include "IXSVS.h"
  #endif
#endif

#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXSSOint.h"
#include "IXSSOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/

/** IXSSO_ks_ffUnid:
    UNID with all 0xFFs. Needed for comparison if the received TUNID is equal to
    this one.
*/
const CSS_t_UNID IXSSO_ks_ffUnid =
{
  0xFFFFFFFFU,             /* NodeID */
  {0xFFFFFFFFU, 0xFFFFU}   /* SNN */
};

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /** IXSSO_ks_ZeroUnid:
      UNID with all 0x00s. Needed for comparison if the received TUNID is equal
      to this one.
  */
  const CSS_t_UNID IXSSO_ks_ZeroUnid =
  {
     0x00000000U,            /* NodeID */
    {0x00000000U, 0x0000U}   /* SNN */
  };
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** k_CSSC_xxx:
    Safety Supervisor Class specific service codes
*/
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  #define k_CSSC_SAFETY_RESET      0x54U
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)
    #define k_CSSC_PROPOSE_TUNID            0x56U     /* Propose_TUNID */
    #define k_CSSC_APPLY_TUNID              0x57U     /* Apply_TUNID */
  #else
    #define k_CSSC_PROPOSE_TUNID_LIST       0x58U     /* Propose_TUNID_List */
    #define k_CSSC_APPLY_TUNID_LIST         0x59U     /* Apply_TUNID_List */
  #endif
#endif


/** k_IAID_xxx:
    Instance Attribute IDs
*/
/*lint -esym(750, k_IAID_AL_ENABLE)               not referenced in every cfg */
/*lint -esym(750, k_IAID_WN_ENABLE)               not referenced in every cfg */
/*lint -esym(750, k_IAID_TRG_UNID)                not referenced in every cfg */
/*lint -esym(750, k_IAID_OCPUNID)                 not referenced in every cfg */
/*lint -esym(750, k_IAID_PROP_TUNID)              not referenced in every cfg */
/*lint -esym(750, k_IAID_TRG_UNID_LIST)           not referenced in every cfg */
/*lint -esym(750, k_IAID_PROP_TUNID_LIST)         not referenced in every cfg */
#define k_IAID_DEV_STATUS        11U    /* Device Status */
#define k_IAID_EXC_STATUS        12U    /* Exception Status */
#define k_IAID_AL_ENABLE         15U    /* Alarm Enable */
#define k_IAID_WN_ENABLE         16U    /* Warning Enable */
#define k_IAID_CFG_UNID          25U    /* Configuration UNID */
#define k_IAID_SCID              26U    /* Safety Configuration Identifier */
#define k_IAID_TRG_UNID          27U    /* Target UNID (=UNID of this device) */
#define k_IAID_OCPUNID           28U    /* Output Connection Point Owners */
#define k_IAID_PROP_TUNID        29U    /* Proposed TUNID */
#define k_IAID_TRG_UNID_LIST     30U    /* Target UNID List */
#define k_IAID_PROP_TUNID_LIST   31U    /* Proposed TUNID List */


/** k_SSO_CLASS_REVISION_VALUE:
    Determines the revision of the object specification that this implementation
    is based on.
*/
#define k_SSO_CLASS_REVISION_VALUE   1U


#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /** k_PS_SAFETY_RESET_01:
      Size of the parameters (number of bytes) of the Safety_Reset type 0 and
      type 1 service.
  */
  #define k_PS_SAFETY_RESET_01     (CSOS_k_SIZEOF_USINT   +  \
                                    IXSSO_k_PASSWORD_SIZE +  \
                                    CSOS_k_SIZEOF_UNID)
  /** k_PS_SAFETY_RESET_2:
      Size of the parameters (number of bytes) of the Safety_Reset type 2
  */
  #define k_PS_SAFETY_RESET_2      (CSOS_k_SIZEOF_USINT   +  \
                                    IXSSO_k_PASSWORD_SIZE +  \
                                    CSOS_k_SIZEOF_UNID    +  \
                                    CSOS_k_SIZEOF_BYTE)
#endif


#if (IXSSO_k_NUM_OCPUNIDS != 0U)

  /* Size of the EPATH corresponding to an OCPUNID */
  #define k_OCPUNID_EPATH_SIZE      ( (3U * CSOS_k_SIZEOF_USINT)         \
                                       + (CSOS_k_SIZEOF_UINT)            \
                                    )
  /* Size of the data of the Output Connection Point Owners Attribute */
  #define k_OCP_OWNERS_ATTRIB_SIZE  (CSOS_k_SIZEOF_UINT                  \
                                     + (IXSSO_k_NUM_OCPUNIDS             \
                                        * (CSOS_k_SIZEOF_UNID            \
                                           + CSOS_k_SIZEOF_USINT         \
                                           + k_OCPUNID_EPATH_SIZE        \
                                          )                              \
                                       )                                 \
                                    )

  /* static checking if the Output Connection Point Owner attribute can fit into
     the Explicit Message Response Data buffer */
  #if (k_OCP_OWNERS_ATTRIB_SIZE > CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE)
    #error k_OCP_OWNERS_ATTRIB_SIZE exceeds CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
  #endif
#endif

#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  /* Variables for storing service related data of the Safety Supervisor Obj */
  /* Proposed TUNID / Proposed TUNID List Attribute (volatile) */
  static CSS_t_UNID  as_PropUnid[CSOS_cfg_NUM_OF_SAFETY_PORTS];
  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    /* To be able to compare that the number of received TUNIDs in the
       Propose_TUNID_List and Apply_TUNID_List services are equal we need to
       store this number. */
    static CSS_t_USINT u8_PropNumPorts;
  #endif
#endif  /* (CSS_cfg_SET_TUNID == CSS_k_ENABLE) */

#if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
  /* Alarm Enable Attribute (NV) */
  /* Not relevant for Soft Error Checking - not part of the safety function */
  static CSS_t_BOOL  o_AlarmEnable;
  /* Warning Enable Attribute (NV) */
  /* Not relevant for Soft Error Checking - not part of the safety function */
  static CSS_t_BOOL  o_WarningEnable;
#endif

/* Exception Status Attribute */
/* Not relevant for Soft Error Checking - not part of the safety function */
static CSS_t_BYTE  b_ExceptionStatus;

#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
  /* Maximum size of the data of the Target UNID List Attribute */
  #define k_TUNID_LIST_ATTRIB_SIZE  (CSOS_k_SIZEOF_USINT                 \
                                     + (CSOS_cfg_NUM_OF_SAFETY_PORTS     \
                                        * (CSOS_k_SIZEOF_UINT            \
                                           + CSOS_k_SIZEOF_UNID          \
                                          )                              \
                                       )                                 \
                                    )
  /* static checking if the Target UNID List attribute can fit into
     the Explicit Message Response Data buffer */
  #if ((k_TUNID_LIST_ATTRIB_SIZE) > CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE)
    #error k_TUNID_LIST_ATTRIB_SIZE exceeds CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
  #endif
#endif

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
    #define k_SSOS_SIZEOF_VAR1   (sizeof(as_PropUnid))
    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
      #define k_SSOS_SIZEOF_VAR12  (k_SSOS_SIZEOF_VAR1 +sizeof(u8_PropNumPorts))
    #else
      #define k_SSOS_SIZEOF_VAR12  k_SSOS_SIZEOF_VAR1
    #endif
  #endif
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD DataInit(void);
static void SrvcDataInit(void);
static CSS_t_BYTE ExceptionStatusGet(void);
static void SrvcClassGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                   CSS_t_EXPL_RSP *ps_explResp,
                                   CSS_t_BYTE * const pb_rspData);
static void SrvcInstGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                  CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_BYTE * const pb_rspData);
static void SrvcInstSetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                  CSS_t_EXPL_RSP *ps_explResp);
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
static void SrvcInstSafetyReset(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp);
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)
    static void SrvcInstProposeTunid(const CSS_t_EXPL_REQ *ps_explReq,
                                     CSS_t_EXPL_RSP *ps_explResp);
    static void SrvcInstApplyTunid(const CSS_t_EXPL_REQ *ps_explReq,
                                   CSS_t_EXPL_RSP *ps_explResp);
  #else
    static void SrvcInstProposeTunidList(const CSS_t_EXPL_REQ *ps_explReq,
                                         CSS_t_EXPL_RSP *ps_explResp);
    static void SrvcInstApplyTunidList(const CSS_t_EXPL_REQ *ps_explReq,
                                       CSS_t_EXPL_RSP *ps_explResp);
  #endif
#endif  /* (CSS_cfg_SET_TUNID) */
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
  static CSS_t_UINT UnidListAttrGet(const CSS_t_UNID *pas_unids,
                                    CSS_t_BYTE * const pb_rspData);
#endif
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    static CSS_t_BOOL ReceivedTunidListCopyandCheck(CSS_t_USINT u8_recNumPorts,
                                                  const CSS_t_BYTE *pb_reqData,
                                                  CSS_t_UNID *as_recTunids,
                                                  CSS_t_UINT *au16_recPortNums);
    static CSS_t_BOOL PortsAndTunidCheck(CSS_t_USINT u8_recNumPorts,
                                         const CSS_t_UNID *as_recTunids,
                                         const CSS_t_UINT *au16_recPortNums);
#endif
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSO_Init
**
** Description : This function initializes the IXSSO unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_Init(void)
{
  /* event for passing to SSO state machine */
  CSS_t_USINT u8_event = IXSSO_k_SE_UNDEF;

  /* Initialize the SSO state machine */
  IXSSO_StateMachineInit();

  /* Initialize SSO attributes */
  b_ExceptionStatus = 0x00U;

  /* if Initialization of SSO data fails */
  if (DataInit() != CSS_k_OK)
  {
    /* Error during initialization of SSO data structure. */
    /* This is considered to be a Self Test Error. */
    u8_event = IXSSO_k_SE_ST_FAIL;
  }
  else /* else: no error */
  {
    /* if SAPL indicates that self test passed */
    if (SAPL_IxssoDeviceSelfTestClbk())
    {
      /* self test passed */
      u8_event = IXSSO_k_SE_ST_PASS;
    }
    else /* else: self test failed */
    {
      u8_event = IXSSO_k_SE_ST_FAIL;
    }
  }

  /* report self test result to SSO state machine */
  /* if SSO state machine doesn't accept self test result */
  if (IXSSO_StateMachine(u8_event, IXSSO_k_SEAI_NONE) != CSS_k_OK)
  {
    /* already reported by SSO state machine to SAPL via SAPL_CssErrorClbk() */
  }
  else /* else: no error */
  {
    /* success - continue */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSSO_ExceptionStatusSet
**
** Description : This API function must be called by the application to set the
**               device's Safety Supervisor Object Exception Status bits. Only
**               the Basic Method is supported (Bit 7 must always be 0).
**
** Parameters  : b_excStat (IN)    - Combination of bits. See definition of
**                                   {IXSSO_k_ES_xxx}.
**                                   (checked, see {IXSSO_k_ES_xxx})
**
** Returnvalue : CSS_k_TRUE        - The IXSSO has accepted the passed Status.
**               CSS_k_FALSE       - The passed IXSSO status was not accepted
**                                   due to a reserved bit was set or a IXSSO
**                                   state mismatch
**
*******************************************************************************/
CSS_t_BOOL IXSSO_ExceptionStatusSet(CSS_t_BYTE b_excStat)
{
  /* return value of this function */
  CSS_t_BOOL o_ret = CSS_k_FALSE;

  /* if CIP Safety Stack is in an improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state - return flag is already set */
    SAPL_CssErrorClbk(IXSSO_k_FSE_AIS_STATE_ERR_ESS, IXSER_k_I_NOT_USED,
                      IXSER_k_A_NOT_USED);
  }
  else /* else: stack state is ok */
  {
    /* if bit "Expanded Method" is set */
    if (b_excStat & (CSS_t_BYTE)IXSSO_k_ES_METHOD)
    {
      /* only Basic Method is supported by this stack */
    }
    /* else: if reserved bit is set */
    else if (b_excStat & (CSS_t_BYTE)IXSSO_k_ES_RESERVED_03)
    {
      /* a reserved bit is set! */
    }
    else /* else: only valid bits are set */
    {
      /* if there are exception bits set and the API function wants to clear
         them all */
      if ((b_ExceptionStatus != 0x00U) && (b_excStat == 0x00U))
      {
        /* pass this event to the state machine */
        CSS_t_WORD w_retCode = IXSSO_StateMachine(IXSSO_k_SE_EX_COND_CLEAR,
                                                  IXSSO_k_SEAI_NONE);

        /* if the state machine indicates that the exceptions may be cleared */
        if (w_retCode == IXSSO_k_NFSE_AIS_INFO_EXCON_CLR)
        {
          /* Restart self-test. Will cause transition to Self-Testing */
          IXSSO_Init();
          /* return success */
          o_ret = CSS_k_TRUE;
        }
        else /* else: event ignored by the SSO state machine */
        {
          /* take over the exception status bits */
          b_ExceptionStatus = b_excStat;
          /* return success */
          o_ret = CSS_k_TRUE;
        }
      }
      else /* else: this is not an "Exception Condition Cleared" */
      {
        /* take over the exception status bits */
        b_ExceptionStatus = b_excStat;
        /* return success */
        o_ret = CSS_k_TRUE;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_ret);
}


/*******************************************************************************
**
** Function    : IXSSO_ExplMsgHandler
**
** Description : This function handles all explicit requests addressed to the
**               Safety Supervisor Object. It checks if a class or Instance
**               Service is requested and calls the corresponding function to
**               serve the request (see Req.5.3.1-3).
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - pointer to the response message data that
**                                   is to be returned
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
CSS_t_WORD IXSSO_ExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* if Instance ID is Class designator */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    /* switch for the Service Code */
    switch (ps_explReq->u8_service)
    {
      case CSOS_k_CCSC_GET_ATTR_SINGLE:
      {
        SrvcClassGetAttrSingle(ps_explReq, ps_explResp, pb_rspData);
        w_retVal = CSS_k_OK;
        break;
      }

      default:
      {
        /* Class Service not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
        w_retVal = CSS_k_OK;
      }
    }
  }
  /* else: an instance is addressed */
  else
  {
    /* if anything else than Instance 1 is addressed */
    if (ps_explReq->u32_instance != 1U)
    {
      /* there is always only one instance of the Safety Supervisor Object */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
      w_retVal = CSS_k_OK;
    }
    else /* else: Instance ID is valid */
    {
      /* Switch for the Service Code and call the corresponding function to   */
      /* serve it.                                                            */
      switch (ps_explReq->u8_service)
      {
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          SrvcInstGetAttrSingle(ps_explReq, ps_explResp, pb_rspData);
          w_retVal = CSS_k_OK;
          break;
        }

        case CSOS_k_CCSC_SET_ATTR_SINGLE:
        {
          SrvcInstSetAttrSingle(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }

      #if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
        case k_CSSC_SAFETY_RESET:
        {
          SrvcInstSafetyReset(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }
      #endif

    #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)             /* CCT_NO_PRE_WARNING */

      /* Multiple safety port devices shall not implement the  Propose_TUNID and
         Apply_TUNID services (see FRS380). */
      /* Single safety port devices shall not support Propose_TUNID_List and
         Apply_TUNID_List services (see FRS381). */
      #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)  /* Single Port Safety devices */

        case k_CSSC_PROPOSE_TUNID:
        {
          SrvcInstProposeTunid(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }

        case k_CSSC_APPLY_TUNID:
        {
          SrvcInstApplyTunid(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }

      #else   /* Multiple Port Safety devices */

        case k_CSSC_PROPOSE_TUNID_LIST:
        {
          SrvcInstProposeTunidList(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }

        case k_CSSC_APPLY_TUNID_LIST:
        {
          SrvcInstApplyTunidList(ps_explReq, ps_explResp);
          w_retVal = CSS_k_OK;
          break;
        }
      #endif  /* (CSOS_cfg_NUM_OF_SAFETY_PORTS) */
    #endif  /* (CSS_cfg_SET_TUNID) */

        default:
        {
          /* Instance Service not supported */
          ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
          w_retVal = CSS_k_OK;
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSSO_ServiceSoftErrByteGet
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
CSS_t_UDINT IXSSO_ServiceSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_SSOS_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)as_PropUnid)+u32_varCnt);
  }
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_SSOS_SIZEOF_VAR12))
  {
    *pb_var = u8_PropNumPorts;
  }
#endif
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_SSOS_SIZEOF_VAR12;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSSO_ServiceSoftErrVarGet
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
void IXSSO_ServiceSoftErrVarGet(
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
    case 0:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SAFETY_PORTS)
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
            CSS_H2N_CPY32(pb_var, &as_PropUnid[au32_cnt[1U]].u32_nodeId);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 1U:
          {
            CSS_H2N_CPY32(pb_var, &as_PropUnid[au32_cnt[1U]].s_snn.u32_time);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 2U:
          {
            CSS_H2N_CPY16(pb_var, &as_PropUnid[au32_cnt[1U]].s_snn.u16_date);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
            break;
          }
        }
      }
      break;
    }

  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    case 1:
    {
      CSS_H2N_CPY8(pb_var, &u8_PropNumPorts);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
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
#endif  /* (CSS_cfg_SET_TUNID == CSS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : DataInit
**
** Description : This function initializes all the Safety Supervisor variables.
**
** Parameters  : -
**
** Returnvalue : CSS_k_OK             - success
**               <>CSS_k_OK           - error
**
*******************************************************************************/
static CSS_t_WORD DataInit(void)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

#if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
  /* if restoring the "Alarm Enable" attribute fails */
  if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_AL_ENABLE,
         IXSSO_k_SDS_AL_ENABLE,
         (CSS_t_BYTE *)&o_AlarmEnable))
  {
    /* error while restoring the Alarm Enable attribute */
    w_retVal = (IXSSO_k_FSE_INC_NV_R_AE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if restoring the "Warning Enable" attribute fails */
  else if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_WN_ENABLE,
           IXSSO_k_SDS_WN_ENABLE,
           (CSS_t_BYTE *)&o_WarningEnable))
  {
    w_retVal = (IXSSO_k_FSE_INC_NV_R_WE);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
#endif
  {
    w_retVal = IXSSO_IdsInit();

    SrvcDataInit();
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : SrvcDataInit
**
** Description : This function initializes volatile data of this file.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcDataInit(void)
{
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  {
    /* loop counter for initializing all safety ports */
    CSS_t_USINT u8_portIdx;

    /* completely erase structure */
    CSS_MEMSET(as_PropUnid, 0, sizeof(as_PropUnid));

    for (u8_portIdx = 0U;
         u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS;
         u8_portIdx++)
    {
      /* proposed TUNID struct */
      as_PropUnid[u8_portIdx].u32_nodeId     = IXSSO_ks_ffUnid.u32_nodeId;
      as_PropUnid[u8_portIdx].s_snn.u32_time = IXSSO_ks_ffUnid.s_snn.u32_time;
      as_PropUnid[u8_portIdx].s_snn.u16_date = IXSSO_ks_ffUnid.s_snn.u16_date;
    }

  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    {
      u8_PropNumPorts = 0U;
    }
  #endif
  }
#endif  /* (CSS_cfg_SET_TUNID == CSS_k_ENABLE) */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : ExceptionStatusGet
**
** Description : This function returns the Exception Status with respect to
**               Alarm Enable and Warning Enable.
**
** Parameters  : -
**
** Returnvalue : CSS_t_BYTE - exception status, combination of bits, (see
**                            {IXSSO_k_ES_xxx} for bit definitions)
**
*******************************************************************************/
static CSS_t_BYTE ExceptionStatusGet(void)
{
  /* return value of this function */
  CSS_t_BYTE b_maskedStatus = b_ExceptionStatus;

#if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
  /* Temporary variable needed when calculating bit mask to avoid warnings on
     various compilers. */
  CSS_t_BYTE b_temp;

  /* if alarms are disabled */
  if (o_AlarmEnable == CSS_k_FALSE)
  {
    /* Alarms are disabled. Make sure respective bits are cleared */
    b_temp = (CSS_t_BYTE)IXSSO_k_ES_AL_DEV_COMM
           | (CSS_t_BYTE)IXSSO_k_ES_AL_DEV_SPEC
           | (CSS_t_BYTE)IXSSO_k_ES_AL_MAN_SPEC;
    b_temp = (CSS_t_BYTE)~b_temp;
    b_maskedStatus &= b_temp;
  }
  else /* else: alarms are enabled */
  {
    /* leave these bits untouched */
  }

  /* warning bits may only be returned when warnings are enabled */
  /* if warnings are disabled */
  if (o_WarningEnable == CSS_k_FALSE)
  {
    /* Warnings are disabled. Make sure respective bits are cleared */
    b_temp = (CSS_t_BYTE)IXSSO_k_ES_WN_DEV_COMM
           | (CSS_t_BYTE)IXSSO_k_ES_WN_DEV_SPEC
           | (CSS_t_BYTE)IXSSO_k_ES_WN_MAN_SPEC;
    b_temp = (CSS_t_BYTE)~b_temp;
    b_maskedStatus &= b_temp;
  }
  else /* else: warnings are enabled */
  {
    /* leave these bits untouched */
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (b_maskedStatus);
}


/*******************************************************************************
**
** Function    : SrvcClassGetAttrSingle
**
** Description : This function serves the Get_Attribute_Single service for the
**               Safety Supervisor Class.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - pointer to the response message data that
**                                   is to be returned
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcClassGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                   CSS_t_EXPL_RSP *ps_explResp,
                                   CSS_t_BYTE * const pb_rspData)
{
  /* temporary variable for copying attribute data to the explicit response */
  CSS_t_UINT u16_temp = 0U;

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
  }
  /* else: if received request data length is not ok */
  else if (ps_explReq->u16_reqDataLen > 0U)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
  }
  else /* else: previous checks passed */
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Class Revision */
      case CSOS_k_CCCA_REVISION:
      {
        u16_temp = k_SSO_CLASS_REVISION_VALUE;
        CSS_H2N_CPY16(pb_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : SrvcInstGetAttrSingle
**
** Description : This function serves the Get_Attribute_Single service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - pointer to the response message data that
**                                   is to be returned
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                  CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_BYTE * const pb_rspData)
{
  /* temporary variable for copying data to the message buffer */
  CSS_t_USINT u8_temp;

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
  }
  /* else: if received request data length is not ok */
  else if (ps_explReq->u16_reqDataLen > 0U)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
  }
  else /* else: previous checks passed */
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Device Status */
      case k_IAID_DEV_STATUS:
      {
        /* read device status and convert to USINT */
        CSOS_t_SSO_DEV_STATUS e_tempDevStatus = IXSSO_DeviceStatusGet();
        u8_temp = (CSS_t_USINT)e_tempDevStatus;
        CSS_H2N_CPY8(pb_rspData, &u8_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      /* Exception Status */
      case k_IAID_EXC_STATUS:
      {
        /* read exception status (with respect to alarm/warning enable bits */
        u8_temp = (CSS_t_USINT)ExceptionStatusGet();
        CSS_H2N_CPY8(pb_rspData, &u8_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

    #if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
      /* Alarm Enable */
      case k_IAID_AL_ENABLE:
      {
        u8_temp = (CSS_t_USINT)o_AlarmEnable;
        CSS_H2N_CPY8(pb_rspData, &u8_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      /* Warning Enable */
      case k_IAID_WN_ENABLE:
      {
        u8_temp = (CSS_t_USINT)o_WarningEnable;
        CSS_H2N_CPY8(pb_rspData, &u8_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
    #endif

      /* Configuration UNID (see Req.5.3-16) */
      case k_IAID_CFG_UNID:
      {
      #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
        /* pointer to our CFUNID */
        const CSS_t_UNID * ps_cfgUnid = IXSSO_CfUnidPtrGet();
      #else
        /* For Originator-only devices we return all 0xFF. This means that the
           configuration is owned by a tool */
        /* there may be padding bytes with undefined values, but this struct is
           only used within this block and only the members are accessed */
        const CSS_t_UNID * ps_cfgUnid = &IXSSO_ks_ffUnid;
      #endif

        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, 0U),
                      &ps_cfgUnid->s_snn.u32_time);
        CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT),
                      &ps_cfgUnid->s_snn.u16_date);
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT +
                                                 CSOS_k_SIZEOF_UINT),
                      &ps_cfgUnid->u32_nodeId);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UNID;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      /* Safety Configuration Identifier (SCID) */
      case k_IAID_SCID:
      {
        /* pointer to our SCID */
        const IXSSO_t_SCID *ps_scid = IXSSO_ScidPtrGet();

        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, 0U),
                      &ps_scid->u32_sccrc);
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT),
                      &ps_scid->s_scts.u32_time);
        CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT +
                                                 CSOS_k_SIZEOF_UDINT),
                      &ps_scid->s_scts.u16_date);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UDINT +
                                      CSOS_k_SIZEOF_UDINT +
                                      CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)
      /* Target UNID */
      /* Single safety port devices shall support Attribute 27 (see FRS384) */
      case k_IAID_TRG_UNID:
      {
        /* pointer to our TUNID */
        const CSS_t_UNID *pas_ourTunid = IXSSO_TunidArrayPtrGet();

        /* As reading is done within CSS this has safety integrity */
        /* (see SRS124). */
        /* This will return either the default or the TUNID set during TUNID  */
        /* setting process (see SRS107) */
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, 0U),
                      &pas_ourTunid[0U].s_snn.u32_time);
        CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT),
                      &pas_ourTunid[0U].s_snn.u16_date);
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT +
                                                 CSOS_k_SIZEOF_UINT),
                      &pas_ourTunid[0U].u32_nodeId);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UNID;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
    #else
      /* Target UNID List */
      /* Multiple safety port devices shall support Attribute 30 (see FRS383) */
      case k_IAID_TRG_UNID_LIST:
      {
        /* pointer to our TUNIDs */
        /* This will return either the default TUNID list or the TUNIDs set */
        /* during TUNID list setting process (see SRS107) */
        const CSS_t_UNID *pas_ourTunids = IXSSO_TunidArrayPtrGet();

        /* As reading is done within CSS this has safety integrity */
        /* (see SRS124). */
        ps_explResp->u16_rspDataLen = UnidListAttrGet(pas_ourTunids,
                                                      pb_rspData);
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
    #endif

    #if (IXSSO_k_NUM_OCPUNIDS != 0U)
      /* Output Connection Point Owners (OCPUNIDs) */
      case k_IAID_OCPUNID:
      {
        /* temporary variable for copying data to the message buffer */
        CSS_t_UINT u16_temp = (CSS_t_UINT)IXSSO_k_NUM_OCPUNIDS;
        /* Assembly Instance ID == Connection Point of an OCPUNID entry */
        CSS_t_UINT u16_asmInstId = CSOS_k_INVALID_INSTANCE;
        /* loop counter */
        CSS_t_UINT u16_idx;
        /* pointer to the OCPUNID of one Safety Validator Server */
        const CSS_t_UNID* ps_ocpUnid;

        /* Number of OCPUNID struct entries */
        CSS_H2N_CPY16(pb_rspData, &u16_temp);
        u16_temp = CSOS_k_SIZEOF_UINT;

        /* loop over all Output Assembly Instances instances */
        for (u16_idx = 0U; u16_idx < IXSSO_k_NUM_OCPUNIDS; u16_idx++)
        {
          /* get the OCPUNID from ID module of this unit */
          ps_ocpUnid = IXSSO_OcpUnidPtrAndInstGet(u16_idx, &u16_asmInstId);
          /* OCPUNID[u16_idx] */
          CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, u16_temp),
                        &ps_ocpUnid->s_snn.u32_time);
          CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, u16_temp +
                                                   CSOS_k_SIZEOF_UDINT),
                        &ps_ocpUnid->s_snn.u16_date);
          CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, u16_temp +
                                                   CSOS_k_SIZEOF_UDINT +
                                                   CSOS_k_SIZEOF_UINT),
                        &ps_ocpUnid->u32_nodeId);
          u16_temp = u16_temp + CSOS_k_SIZEOF_UNID;

          /* EPATH Size */
          u8_temp = k_OCPUNID_EPATH_SIZE;
          CSS_H2N_CPY8(CSS_ADD_OFFSET(pb_rspData, u16_temp), &u8_temp);
          u16_temp = u16_temp + CSOS_k_SIZEOF_USINT;

          /* Application Resource (EPATH) */
          u8_temp = CSOS_k_LOG_SEG_CLASS_ID_8;
          CSS_H2N_CPY8(CSS_ADD_OFFSET(pb_rspData, u16_temp), &u8_temp);
          u16_temp = u16_temp + CSOS_k_SIZEOF_USINT;
          u8_temp = (CSS_t_USINT)CSOS_k_CCC_ASSEMBLY_OBJ;
          CSS_H2N_CPY8(CSS_ADD_OFFSET(pb_rspData, u16_temp), &u8_temp);
          u16_temp = u16_temp + CSOS_k_SIZEOF_USINT;
          u8_temp = CSOS_k_LOG_SEG_INSTANCE_ID_16;
          CSS_H2N_CPY8(CSS_ADD_OFFSET(pb_rspData, u16_temp), &u8_temp);
          u16_temp = u16_temp + CSOS_k_SIZEOF_USINT;
          CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, u16_temp), &u16_asmInstId);
          u16_temp = u16_temp + CSOS_k_SIZEOF_UINT;
        }

        ps_explResp->u16_rspDataLen = k_OCP_OWNERS_ATTRIB_SIZE;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;

        break;
      }
    #endif  /* (IXSSO_k_NUM_OCPUNIDS != 0U) */

  #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)               /* CCT_NO_PRE_WARNING */
    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)
      /* Proposed TUNID */
      /* Single safety port devices shall support Attribute #29 */
      case k_IAID_PROP_TUNID:
      {
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, 0U),
                      &as_PropUnid[0U].s_snn.u32_time);
        CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT),
                      &as_PropUnid[0U].s_snn.u16_date);
        CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, CSOS_k_SIZEOF_UDINT +
                                                 CSOS_k_SIZEOF_UINT),
                      &as_PropUnid[0U].u32_nodeId);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UNID;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
    #else
      /* Proposed TUNID List */
      /* Multiple safety port devices shall only support Attribute 31
         (see FRS382) */
      case k_IAID_PROP_TUNID_LIST:
      {
        ps_explResp->u16_rspDataLen = UnidListAttrGet(as_PropUnid, pb_rspData);
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
    #endif
  #endif  /* (CSS_cfg_SET_TUNID == CSS_k_ENABLE) */

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SrvcInstSetAttrSingle
**
** Description : This function serves the Set_Attribute_Single service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstSetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                  CSS_t_EXPL_RSP *ps_explResp)
{
#if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
  /* temporary variable for copying data from the message buffer */
  CSS_t_USINT u8_temp;
#endif

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
  }
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Device Status */
      case k_IAID_DEV_STATUS:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }

      case k_IAID_EXC_STATUS:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }

    #if (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE)
      case k_IAID_AL_ENABLE:
      {
        /* if received request data length is longer than expected */
        if (ps_explReq->u16_reqDataLen > CSOS_k_SIZEOF_BOOL)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
        }
        /* else: if received request data length is shorter than expected */
        else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_BOOL)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
        }
        else /* else: received data length is ok */
        {
          CSS_N2H_CPY8(&u8_temp, ps_explReq->pb_reqData);

          /* if received data value is not a BOOL value */
          if (u8_temp > 1U)
          {
            ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_ATTR_VAL;
          }
          else /* else: BOOL value ok */
          {
            /* if storing the new value to NV-memory fails */
            if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_AL_ENABLE,
                                             IXSSO_k_SDS_AL_ENABLE,
                                             &u8_temp))
            {
              /* error while storing NV-data */
              ps_explResp->u8_genStat = CSOS_k_CGSC_STORE_OP_FAILURE;
            }
            else /* no error */
            {
              /* take over the received value and return success response */
              o_AlarmEnable = (CSS_t_BOOL)u8_temp;
              ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            }
          }
        }
        break;
      }

      case k_IAID_WN_ENABLE:
      {
        /* if received request data length is longer than expected */
        if (ps_explReq->u16_reqDataLen > CSOS_k_SIZEOF_BOOL)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
        }
        /* else: if received request data length is shorter than expected */
        else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_BOOL)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
        }
        else /* else: received data length is ok */
        {
          CSS_N2H_CPY8(&u8_temp, ps_explReq->pb_reqData);

          /* if received data value is not a BOOL value */
          if (u8_temp > 1U)
          {
            ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_ATTR_VAL;
          }
          else /* else: BOOL value ok */
          {
            /* if storing the new value to NV-memory fails */
            if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_WN_ENABLE,
                                             IXSSO_k_SDS_WN_ENABLE,
                                             &u8_temp))
            {
              /* error while storing NV-data */
              ps_explResp->u8_genStat = CSOS_k_CGSC_STORE_OP_FAILURE;
            }
            else /* no error */
            {
              /* take over the received value and return success response */
              o_WarningEnable = (CSS_t_BOOL)u8_temp;
              ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            }
          }
        }
        break;
      }
    #endif /* (CSS_cfg_ALARM_WARNING_ENABLE == CSS_k_ENABLE) */

      case k_IAID_CFG_UNID:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }

      case k_IAID_SCID:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }

    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)            /* CCT_NO_PRE_WARNING */
      case k_IAID_TRG_UNID:
    #else                                               /* CCT_NO_PRE_WARNING */
      case k_IAID_TRG_UNID_LIST:
    #endif
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }

    #if (IXSSO_k_NUM_OCPUNIDS != 0U)
      case k_IAID_OCPUNID:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }
    #endif

  #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)               /* CCT_NO_PRE_WARNING */
    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)            /* CCT_NO_PRE_WARNING */
      case k_IAID_PROP_TUNID:
    #else                                               /* CCT_NO_PRE_WARNING */
      case k_IAID_PROP_TUNID_LIST:
    #endif
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        break;
      }
    #endif

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : SrvcInstSafetyReset
**
** Description : This function serves the Safety_Reset service for the Safety
**               Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to the response message that is
**                                   to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstSafetyReset(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp)
{
  /* received value (here as an USINT to be able to check for 0, 1 or else */
  CSS_t_USINT u8_recResTyp = 0xFFU;
  /* expected data length (dependent on reset type) */
  CSS_t_USINT u8_expDataLen = 0U;
  /* received Attribute_Bit_Map parameter */
  CSS_t_BYTE  b_recAttrBitMap = 0x00U;  /* only overwritten in case of type 2 */
  /* pointer to received password */
  CSS_t_CHAR *pc_recPwd = CSS_k_NULL;
  /* received UNID */
  CSS_t_UNID s_recUnid;
  /* get a pointer to the list of our UNID(s) */
  const CSS_t_UNID *pas_ourTunid = IXSSO_TunidArrayPtrGet();

  /* completely erase structure */
  CSS_MEMSET(&s_recUnid, 0, sizeof(s_recUnid));

  /* set the general Status to a defined value so that after the first checks we
     can decide that all of them have passed */
  ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_STATUS_CODE;

  /* if the request EPATH contained an attribute ID */
  if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
  {
    /* wrong path format - for this service we don't need an attribute */
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
  }
  /* else: if the received data length is less than what we need for type
     0/1 reset */
  else if (ps_explReq->u16_reqDataLen < k_PS_SAFETY_RESET_01)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
  }
  else  /* else: we have received at least the minimum length */
  {
    /* copy the received Reset Type parameter */
    CSS_N2H_CPY8(&u8_recResTyp, ps_explReq->pb_reqData);

    /* if reset type is 0 or 1 */
    if (    (u8_recResTyp == IXSSO_k_RESET_TYPE_0)
         || (u8_recResTyp == IXSSO_k_RESET_TYPE_1)
       )
    {
      /* set expected length for reset type 0 or 1 service */
      u8_expDataLen = k_PS_SAFETY_RESET_01;
    }
    /* else: if reset type 2 */
    else if (u8_recResTyp == IXSSO_k_RESET_TYPE_2)
    {
      /* set expected length for reset type 2 service */
      u8_expDataLen = k_PS_SAFETY_RESET_2;
    }
    else  /* else: unsupported reset type parameter */
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
    }
  }

  /* if an error was discovered before */
  if (ps_explResp->u8_genStat != CSOS_k_CGSC_INVALID_STATUS_CODE)
  {
    /* error already set - nothing more to do in this function */
  }
  else
  {
    /* if received less data than expected */
    if (ps_explReq->u16_reqDataLen < u8_expDataLen)
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
    }
    /* else: if received more data than expected */
    else if (ps_explReq->u16_reqDataLen > u8_expDataLen)
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
    }
    else  /* else: length is ok */
    {
      /* make a pointer to the received password parameter */
      pc_recPwd = (CSS_t_CHAR*)CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                              CSOS_k_SIZEOF_USINT);

      /* read the received TUNID parameter into a variable */
      CSS_N2H_CPY32(&s_recUnid.s_snn.u32_time,
                    CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                   CSOS_k_SIZEOF_USINT +
                                   IXSSO_k_PASSWORD_SIZE));
      CSS_N2H_CPY16(&s_recUnid.s_snn.u16_date,
                    CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                   CSOS_k_SIZEOF_USINT +
                                   IXSSO_k_PASSWORD_SIZE +
                                   CSOS_k_SIZEOF_UDINT));
      CSS_N2H_CPY32(&s_recUnid.u32_nodeId,
                    CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                   CSOS_k_SIZEOF_USINT +
                                   IXSSO_k_PASSWORD_SIZE +
                                   CSOS_k_SIZEOF_UDINT +
                                   CSOS_k_SIZEOF_UINT));

      /* if reset type 2 */
      if (u8_recResTyp == IXSSO_k_RESET_TYPE_2)
      {
        /* Reset type 2 has an additional parameter: */
        /* read the received Attribute_Bit_Map parameter into a variable */
        CSS_N2H_CPY8(&b_recAttrBitMap, CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                                      k_PS_SAFETY_RESET_01));
      }
      else  /* else: other reset types */
      {
        /* nothing to do here */
      }

      /* We can immediately reject a received FF TUNID if we are not in "Waiting
         for TUNID" and the device has (at least on one port) a TUNID
         assigned. */
      /* Mute lint warning "side effects on right hand of logical operator".
         Functions IXSSO_UnidCompare and IXSSO_DeviceHasValidTunid have no side
         effects. */
      /* if we are not in this state and have a valid TUNID and received TUNID
         is an FF TUNID */
      if (    (IXSSO_DeviceStatusGet() != CSOS_k_SSO_DS_WAIT_TUNID)
           && (IXSSO_DeviceHasValidTunid(pas_ourTunid))          /*lint !e960 */
           && (IXSSO_UnidCompare(&IXSSO_ks_ffUnid, &s_recUnid))  /*lint !e960 */
         )                                                       /*lint !e960 */
      {
        /* Received TUNID does not match. Service will return "Invalid        */
        /* Parameter" error (see SRS146) */
        ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
      }
      /* else: if this is a misrouted message
         (compare received TUNID against ours) */
      else if (!IXSSO_UnidListCompare(&s_recUnid, pas_ourTunid))
      {
        /* Received TUNID does not match. Service will return "Invalid        */
        /* Parameter" error (see SRS146) */
        ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
      }
      else  /* else: received TUNID matches */
      {
        /* if there are safety connections currently open */
        if (
             (
               #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)    /* CCT_NO_PRE_WARNING */
                 IXSVS_StateNumServerCnxnGet()
               #else /* CCT_NO_PRE_WARNING */
                 0U
               #endif
               +
               #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)    /* CCT_NO_PRE_WARNING */
                 IXSVC_StateNumClientCnxnGet()
               #else                                    /* CCT_NO_PRE_WARNING */
                 0U
               #endif
             ) > 0U                                              /*lint !e931 */
           )
           /* mute lint warning "both sides have side effects". Obviously only
              the left side has side effects. */
        {
          /* Reject reset request when a safety cnxn is open (see SRS15). */
          ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_STATE_CONFLICT;
        }
        else  /* else: no safety connections currently open */
        {
          /* if the SSO state machine doesn't accept the event */
          if (IXSSO_StateMachine(IXSSO_k_SE_RESET_REQ,
                                 (CSS_t_UINT)u8_recResTyp) != CSS_k_OK)
          {
            /* In this case an FSE error is reported by SSO state machine to
               SAPL via SAPL_CssErrorClbk(). Therefore this code is unreachable
               but left for unit test purposes and future changes. */
            ps_explResp->u8_genStat = CSOS_k_CGSC_RES_UNAVAIL;
          }
          else  /* else: SSO state machine has accepted */
          {
            /* Inform the Safety Application about this event. It will be
               the responsibility of the Safety Application to actually reset
               the platform. But this may not happen until the response to
               this service has been sent out. */
            /* Return value of the callback function decides about the returned
               explicit general status code. */
            ps_explResp->u8_genStat =
              SAPL_IxssoSafetyResetClbk(u8_recResTyp, b_recAttrBitMap,
                                        pc_recPwd);
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* Function was continuously enhanced with further checks. Basic structure of
   this function is still the same and simple. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE) */


#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS == 1U)
/*******************************************************************************
**
** Function    : SrvcInstProposeTunid
**
** Description : This function serves the Propose_TUNID service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstProposeTunid(const CSS_t_EXPL_REQ *ps_explReq,
                                 CSS_t_EXPL_RSP *ps_explResp)
{
  /* received UNID */
  CSS_t_UNID s_recTunid;
  /* temporary variable for being passed to SSO state machine as addInfo */
  CSS_t_USINT u8_state = IXSSO_k_SEAI_PROP_TUNID_STOP;

  /* completely erase structure */
  CSS_MEMSET(&s_recTunid, 0, sizeof(s_recTunid));

  /* if the request EPATH contained an attribute ID */
  if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
  {
    /* wrong path format - for this service we don't need an attribute */
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
  }
  /* else: if received request data length is longer than expected */
  else if (ps_explReq->u16_reqDataLen > CSOS_k_SIZEOF_UNID)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
  }
  /* else: if received request data length is shorter than expected */
  else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_UNID)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
  }
  else /* else: received data length is ok */
  {
    /* read the received data into a UNID structure */
    CSS_N2H_CPY32(&s_recTunid.s_snn.u32_time,
                  ps_explReq->pb_reqData);
    CSS_N2H_CPY16(&s_recTunid.s_snn.u16_date,
                  CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                 CSOS_k_SIZEOF_UDINT));
    CSS_N2H_CPY32(&s_recTunid.u32_nodeId,
                  CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                 CSOS_k_SIZEOF_UDINT +
                                 CSOS_k_SIZEOF_UINT));

    /* sending a propose service with a TUNID of all 0xFFs cancels the */
    /* operation (see SRS196) */
    /* if TUNID is all 0xFFs */
    if (IXSSO_UnidCompare(&IXSSO_ks_ffUnid, &s_recTunid))
    {
      /* cancel propose/apply operation: indicate stop TUNID propose */
      u8_state = IXSSO_k_SEAI_PROP_TUNID_STOP;

      ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    }
    else /* else: TUNID is not 0xFFs */
    {
      /* The NodeID portion of the TUNID must match our NodeID */
      /* (see SRS195) */
      /* if NodeID doesn't match TUNID */
      if (!IXSSO_TunidVsNodeIdCheck(&s_recTunid))
      {
        /* NodeID part of the TUNID does not match our device */
        ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
      }
      else /* else: NodeID matches TUNID */
      {
        /* indicate start TUNID propose */
        u8_state = IXSSO_k_SEAI_PROP_TUNID_START;

        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
      }
    }
  }

  /* if service was successful up to now (u8_genStat was definitely set in the
     previous block) */
  if (ps_explResp->u8_genStat == CSOS_k_CGSC_SUCCESS)
  {
    /* if update to the state machine returns an error */
    if (IXSSO_StateMachine(IXSSO_k_SE_PROP_TUNID,
                           (CSS_t_UINT)u8_state) != CSS_k_OK)
    {
      /* state machine returned an error */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_STATE_CONFLICT;
      /* already reported by SSO state machine to SAPL via
         SAPL_CssErrorClbk() */
    }
    else /* else: success */
    {
      /* Copy the received UNID to the Proposed TUNID attribute. */
      /* Either this is a UNID that matches our device or is an all 0xFF-UNID
         to cancel the operation. */
      CSS_MEMCPY(&as_PropUnid[0U], &s_recTunid, sizeof(as_PropUnid[0U]));
    }
  }
  else /* else: execution of the service failed */
  {
    /* no update of the state machine */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : SrvcInstApplyTunid
**
** Description : This function serves the Apply_TUNID service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstApplyTunid(const CSS_t_EXPL_REQ *ps_explReq,
                               CSS_t_EXPL_RSP *ps_explResp)
{
  /* received UNID */
  CSS_t_UNID s_recTunid;
  /* return value of called functions */
  CSS_t_WORD w_retCode = IXSSO_k_FSE_INC_PRG_FLOW;

  /* make sure the whole structure is initialized (also padding bytes)
     because of Soft Error Checking) */
  CSS_MEMSET(&s_recTunid, 0, sizeof(s_recTunid));

  /* this service is only allowed in the Waiting-For-TUNID state */
  /* if device is not in the Waiting-For-TUNID state */
  if (IXSSO_StateMachine(IXSSO_k_SE_APPLY_TUNID_CHECK,
                         IXSSO_k_SEAI_NONE) != CSS_k_OK)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_STATE_CONFLICT;
    /* already reported by SSO state machine to SAPL via
       SAPL_CssErrorClbk() */
  }
  else /* device is in Waiting-For-TUNID state */
  {
    /* if the request EPATH contained an attribute ID */
    if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
    {
      /* wrong path format - for this service we don't need an attribute */
      ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
    /* else: if received request data length is longer than expected */
    else if (ps_explReq->u16_reqDataLen > CSOS_k_SIZEOF_UNID)
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
    }
    /* else: if received request data length is shorter than expected */
    else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_UNID)
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
    }
    else /* else: received data length is ok */
    {
      /* read the received data into a UNID structure */
      CSS_N2H_CPY32(&s_recTunid.s_snn.u32_time,
                    ps_explReq->pb_reqData);
      CSS_N2H_CPY16(&s_recTunid.s_snn.u16_date,
                    CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                   CSOS_k_SIZEOF_UDINT));
      CSS_N2H_CPY32(&s_recTunid.u32_nodeId,
                    CSS_ADD_OFFSET(ps_explReq->pb_reqData,
                                   CSOS_k_SIZEOF_UDINT +
                                   CSOS_k_SIZEOF_UINT));

      /* if Apply_TUNID parameter doesn't match proposed TUNID */
      if (!IXSSO_UnidCompare(&as_PropUnid[0U], &s_recTunid))
      {
        /* TUNID received with this service is invalid */
        ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
      }
      else /* else: parameter matches */
      {
        /* if the received TUNID is all 0xFFs */
        if (IXSSO_UnidCompare(&IXSSO_ks_ffUnid, &s_recTunid))
        {
          /* TUNID received with this service is invalid */
          ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
        }
        else /* else: received TUNID is ok */
        {
          /* TUNID is already validated against proposed value. Now the TUNID */
          /* will be saved and the Safety Supervisor will be updated          */
          /* (see SRS197) */

          /* if the state machine returns an error */
          if (IXSSO_StateMachine(IXSSO_k_SE_APPLY_TUNID,
                                 IXSSO_k_SEAI_NONE) != CSS_k_OK)
          {
            ps_explResp->u8_genStat = CSOS_k_CGSC_RES_UNAVAIL;
            /* already reported by SSO state machine to SAPL via
               SAPL_CssErrorClbk() */
          }
          else /* else: no error */
          {
            /* Store the new value to NV-memory (see SRS116) */
            w_retCode = IXSSO_TunidSet(&s_recTunid);

            /* if previous function returned an error */
            if (w_retCode != CSS_k_OK)
            {
              /* error while storing NV-data */
              ps_explResp->u8_genStat = CSOS_k_CGSC_STORE_OP_FAILURE;
             }
            else /* else: no error */
            {
              /* set the proposed_TUNID to all 0xFFs */
              as_PropUnid[0U].u32_nodeId     = IXSSO_ks_ffUnid.u32_nodeId;
              as_PropUnid[0U].s_snn.u32_time = IXSSO_ks_ffUnid.s_snn.u32_time;
              as_PropUnid[0U].s_snn.u16_date = IXSSO_ks_ffUnid.s_snn.u16_date;

              ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            }
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#else  /* (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U) */
/*******************************************************************************
**
** Function    : SrvcInstProposeTunidList
**
** Description : This function serves the Propose_TUNID_List service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstProposeTunidList(const CSS_t_EXPL_REQ *ps_explReq,
                                     CSS_t_EXPL_RSP *ps_explResp)
{
  /* temporary variable for being passed to SSO state machine as addInfo */
  CSS_t_USINT u8_state = IXSSO_k_SEAI_PROP_TUNID_STOP;
  /* Number of Proposed Target UNID list entries */
  CSS_t_USINT u8_recNumPorts = 0U;
  /* expected number of received bytes */
  CSS_t_UINT u16_expNumBytes = 0U;
  /* received UNIDs */
  CSS_t_UNID as_recTunids[CSOS_cfg_NUM_OF_SAFETY_PORTS];
  /* received Port Numbers */
  CSS_t_UINT au16_recPortNums[CSOS_cfg_NUM_OF_SAFETY_PORTS];
  /* loop counter for processing all safety ports */
  CSS_t_USINT u8_portIdx;

  /* completely erase structures */
  CSS_MEMSET(as_recTunids, 0, sizeof(as_recTunids));

  /* initialize all structures in the array (TUNIDs to FF and Ports to 0 */
  for (u8_portIdx = 0U;
       u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS;
       u8_portIdx++)
  {
    as_recTunids[u8_portIdx].u32_nodeId     = IXSSO_ks_ffUnid.u32_nodeId;
    as_recTunids[u8_portIdx].s_snn.u32_time = IXSSO_ks_ffUnid.s_snn.u32_time;
    as_recTunids[u8_portIdx].s_snn.u16_date = IXSSO_ks_ffUnid.s_snn.u16_date;
    au16_recPortNums[u8_portIdx] = 0U;
  }

  /* if the request EPATH contained an attribute ID */
  if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
  {
    /* wrong path format - for this service we don't need an attribute */
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
  }
  /* else: if received request data length is much too short */
  else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_USINT)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
  }
  else  /* received length is ok so we can do the next step */
  {
    /* read the received Number of Proposed TUNIDs into a variable */
    CSS_N2H_CPY8(&u8_recNumPorts, ps_explReq->pb_reqData);

    /* if received number of ports is not meeting our expectations */
    if (    (u8_recNumPorts == 0U)
         || (u8_recNumPorts > CSOS_cfg_NUM_OF_SAFETY_PORTS)
       )
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
    }
    else  /* parameter is in range */
    {
      /* calculate the expected number of bytes */
      u16_expNumBytes = CSOS_k_SIZEOF_USINT
                      + ((CSS_t_UINT)u8_recNumPorts * (CSOS_k_SIZEOF_UINT
                                                       + CSOS_k_SIZEOF_UNID));

      /* if received request data length is shorter than expected */
      if (ps_explReq->u16_reqDataLen < u16_expNumBytes)
      {
        ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
      }
      /* else: if received more data than expected */
      else if (ps_explReq->u16_reqDataLen > u16_expNumBytes)
      {
        ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
      }
      else /* else: received data length is ok */
      {
        CSS_t_BOOL o_foundValidTunid =
          ReceivedTunidListCopyandCheck(u8_recNumPorts, ps_explReq->pb_reqData,
                                        as_recTunids, au16_recPortNums);

        /* If we received a Propose_TUNID_List service with all TUNIDs 0xFF this
           cancels the operation (see SRS196) */
        /* if all the received TUNIDs are all FF TUNIDs */
        if (!o_foundValidTunid)
        {
          /* cancel propose/apply operation: indicate stop TUNID propose */
          u8_state = IXSSO_k_SEAI_PROP_TUNID_STOP;

          ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        }
        else /* else: TUNIDs are not all 0xFFs */
        {
          /* Get the list of the device's Port Numbers */
          const CSS_t_UINT *pau16_ourPorts = IXSSS_PortNumArrayPtrGet();
          CSS_t_BOOL o_portMismatch = CSS_k_FALSE;

          /* check if the received Port Numbers match with our device's ports */
          for (u8_portIdx = 0U;
               u8_portIdx < u8_recNumPorts;
               u8_portIdx++)
          {
            /* if received port matches port in our list */
            if (au16_recPortNums[u8_portIdx] == pau16_ourPorts[u8_portIdx])
            {
              /* ok - leave flag unchanged */
            }
            else  /* else: mismatch */
            {
              /* indicate this with flag */
              o_portMismatch = CSS_k_TRUE;
            }
          }

          /* if we received TUNIDs for ports that we don't have */
          if (o_portMismatch)
          {
            ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
          }
          else  /* else: received Port Numbers are ok */
          {
            /* The NodeID portion of all TUNIDs in the list must match our */
            /* NodeIDs (see SRS195) */
            if (!IXSSO_TunidVsNodeIdCheck(as_recTunids))
            {
              /* NodeID part of the TUNID does not match our device */
              ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
            }
            else /* else: NodeID matches TUNID */
            {
              /* indicate start TUNID propose */
              u8_state = IXSSO_k_SEAI_PROP_TUNID_START;

              ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            }
          }
        }
      }
    }
  }

  /* if service was successful up to now (u8_genStat was definitely set in the
     previous block) */
  if (ps_explResp->u8_genStat == CSOS_k_CGSC_SUCCESS)
  {
    /* if update to the state machine returns an error */
    if (IXSSO_StateMachine(IXSSO_k_SE_PROP_TUNID,
                           (CSS_t_UINT)u8_state) != CSS_k_OK)
    {
      /* state machine returned an error */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_STATE_CONFLICT;
      /* already reported by SSO state machine to SAPL via
         SAPL_CssErrorClbk() */
    }
    else /* else: success */
    {
      /* Copy the received UNID list to the Proposed TUNID List attribute. */
      /* TUNIDs in the list either match our devices Node ID or are left at
         their 0xFF default value. If all received UNIDs are 0xFF then this
         cancels the operation (and sets the Proposed TUNID List Attribute to
         default) */
      CSS_MEMCPY(as_PropUnid, as_recTunids, sizeof(as_PropUnid));
      /* Also store the number of received TUNIDs. */
      u8_PropNumPorts = u8_recNumPorts;
    }
  }
  else /* else: execution of the service failed */
  {
    /* no update of the state machine */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : SrvcInstApplyTunidList
**
** Description : This function serves the Apply_TUNID_List service for the
**               Safety Supervisor Instance.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void SrvcInstApplyTunidList(const CSS_t_EXPL_REQ *ps_explReq,
                                   CSS_t_EXPL_RSP *ps_explResp)
{
  /* Number of Applied Target UNID list entries */
  CSS_t_USINT u8_recNumPorts;
  /* expected number of received bytes */
  CSS_t_UINT u16_expNumBytes = 0U;
  /* received UNIDs */
  CSS_t_UNID as_recTunids[CSOS_cfg_NUM_OF_SAFETY_PORTS];
  /* received Port Numbers */
  CSS_t_UINT au16_recPortNums[CSOS_cfg_NUM_OF_SAFETY_PORTS];
  /* loop counter for processing all safety ports */
  CSS_t_USINT u8_portIdx;
  /* return value of called functions */
  CSS_t_WORD w_retCode = IXSSO_k_FSE_INC_PRG_FLOW;

  /* completely erase structures */
  CSS_MEMSET(as_recTunids, 0, sizeof(as_recTunids));

  /* initialize all structures in the array (TUNIDs to FF and Ports to 0 */
  for (u8_portIdx = 0U;
       u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS;
       u8_portIdx++)
  {
    as_recTunids[u8_portIdx].u32_nodeId     = IXSSO_ks_ffUnid.u32_nodeId;
    as_recTunids[u8_portIdx].s_snn.u32_time = IXSSO_ks_ffUnid.s_snn.u32_time;
    as_recTunids[u8_portIdx].s_snn.u16_date = IXSSO_ks_ffUnid.s_snn.u16_date;
    au16_recPortNums[u8_portIdx] = 0U;
  }

  /* this service is only allowed in the Waiting-For-TUNID state */
  /* if device is not in the Waiting-For-TUNID state */
  if (IXSSO_StateMachine(IXSSO_k_SE_APPLY_TUNID_CHECK,
                         IXSSO_k_SEAI_NONE) != CSS_k_OK)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_STATE_CONFLICT;
    /* already reported by SSO state machine to SAPL via
       SAPL_CssErrorClbk() */
  }
  else /* device is in Waiting-For-TUNID state */
  {
    /* if the request EPATH contained an attribute ID */
    if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
    {
      /* wrong path format - for this service we don't need an attribute */
      ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
    /* else: if received request data length is much too short */
    else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_USINT)
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
    }
    else  /* received length is ok to do the next step */
    {
      /* read the received Number of applied TUNIDs into a variable */
      CSS_N2H_CPY8(&u8_recNumPorts, ps_explReq->pb_reqData);

      /* if received number of ports is not meeting our expectations */
      if (    (u8_recNumPorts == 0U)
           || (u8_recNumPorts > CSOS_cfg_NUM_OF_SAFETY_PORTS)
           || (u8_recNumPorts != u8_PropNumPorts)
         )
      {
        ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
      }
      else  /* parameter is in range */
      {
        /* calculate the expected number of bytes */
        u16_expNumBytes = CSOS_k_SIZEOF_USINT
                        + ((CSS_t_UINT)u8_recNumPorts * (CSOS_k_SIZEOF_UINT
                                                         + CSOS_k_SIZEOF_UNID));

        /* if received request data length is shorter than expected */
        if (ps_explReq->u16_reqDataLen < u16_expNumBytes)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
        }
        /* else: if received more data than expected */
        else if (ps_explReq->u16_reqDataLen > u16_expNumBytes)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
        }
        else /* else: received data length is ok */
        {
          CSS_t_BOOL o_foundValidTunid =
            ReceivedTunidListCopyandCheck(u8_recNumPorts,
                                          ps_explReq->pb_reqData, as_recTunids,
                                          au16_recPortNums);

          /* if all the received TUNIDs are all FF TUNIDs */
          if (!o_foundValidTunid)
          {
            /* TUNIDs received with this service are invalid */
            ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
          }
          else /* else: TUNIDs are not all 0xFFs */
          {
            /* if TUNID lists are different */
            if (PortsAndTunidCheck(u8_recNumPorts, as_recTunids,
                                   au16_recPortNums))
            {
              ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_PARAM;
            }
            else
            {
              /* TUNID list is already validated against proposed list. Now the
                 TUNID list will be saved and the Safety Supervisor will be
                 updated (see SRS197) */

              /* if the state machine returns an error */
              if (IXSSO_StateMachine(IXSSO_k_SE_APPLY_TUNID,
                                     IXSSO_k_SEAI_NONE) != CSS_k_OK)
              {
                ps_explResp->u8_genStat = CSOS_k_CGSC_RES_UNAVAIL;
                /* already reported by SSO state machine to SAPL via
                   SAPL_CssErrorClbk() */
              }
              else /* else: no error */
              {
                /* Store the new value to NV-memory (see SRS116) */
                w_retCode = IXSSO_TunidSet(as_recTunids);

                /* if previous function returned an error */
                if (w_retCode != CSS_k_OK)
                {
                  /* error while storing NV-data */
                  ps_explResp->u8_genStat = CSOS_k_CGSC_STORE_OP_FAILURE;
                 }
                else /* else: no error */
                {
                  /* set the Proposed_TUNID_List back to all 0xFFs */
                  for (u8_portIdx = 0U;
                       u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS;
                       u8_portIdx++)
                  {
                    as_PropUnid[u8_portIdx].u32_nodeId =
                      IXSSO_ks_ffUnid.u32_nodeId;
                    as_PropUnid[u8_portIdx].s_snn.u32_time =
                      IXSSO_ks_ffUnid.s_snn.u32_time;
                    as_PropUnid[u8_portIdx].s_snn.u16_date =
                      IXSSO_ks_ffUnid.s_snn.u16_date;
                  }
                  /* re-initialize number of proposed TUNIDs */
                  u8_PropNumPorts = 0U;

                  ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
                }
              }
            }
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif /*(CSOS_cfg_NUM_OF_SAFETY_PORTS) */
#endif  /* (CSS_cfg_SET_TUNID) */


#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
/*******************************************************************************
**
** Function    : UnidListAttrGet
**
** Description : This function writes the UNIDs from the passed array into the
**               passed buffer.
**
** Parameters  : pas_unids (IN)     - pointer to array with UNID structures
**                                    (not checked, only called with reference)
**               pb_rspData (OUT)   - pointer to buffer where the byte stream
**                                    is written to
**                                    (not checked, only called with reference)
**
** Returnvalue : CSS_t_UINT          - number of bytes written into the buffer
**
*******************************************************************************/
static CSS_t_UINT UnidListAttrGet(const CSS_t_UNID *pas_unids,
                                  CSS_t_BYTE * const pb_rspData)
{
  CSS_t_UINT u16_respOffset;
  CSS_t_USINT u8_portIdx;
  const CSS_t_UINT *pau16_ourPorts = IXSSS_PortNumArrayPtrGet();

  /* Number of Target UNIDs */
  u8_portIdx = CSOS_cfg_NUM_OF_SAFETY_PORTS;
  CSS_H2N_CPY8(pb_rspData, &u8_portIdx);
  u16_respOffset = CSOS_k_SIZEOF_USINT;

  /* for all ports */
  for (u8_portIdx = 0U;
       u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS;
       u8_portIdx++)
  {
    /* Port Number */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, u16_respOffset),
                  &pau16_ourPorts[u8_portIdx]);
    u16_respOffset += CSOS_k_SIZEOF_UINT;
    /* Target UNID Time */
    CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, u16_respOffset),
                  &pas_unids[u8_portIdx].s_snn.u32_time);
    u16_respOffset += CSOS_k_SIZEOF_UDINT;
    /* Target UNID Date */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData, u16_respOffset),
                  &pas_unids[u8_portIdx].s_snn.u16_date);
    u16_respOffset += CSOS_k_SIZEOF_UINT;
    /* Target UNID NodeID */
    CSS_H2N_CPY32(CSS_ADD_OFFSET(pb_rspData, u16_respOffset),
                  &pas_unids[u8_portIdx].u32_nodeId);
    u16_respOffset += CSOS_k_SIZEOF_UDINT;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_respOffset);  /* return number of bytes written into the buffer */
}
#endif  /* (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U) */


#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
/*******************************************************************************
**
** Function    : ReceivedTunidListCopyandCheck
**
** Description : This function copies from the passed (byte stream) buffer the
**               contained TUNID list into the passed array pointers and checks
**               if in the received TUNID list there are TUNIDs that are
**               different than all 0xFF.
**
** Parameters  : u8_recNumPorts (IN)    - number of TUNIDs contained in
**                                        pb_reqData
**               pb_reqData (IN)        - pointer to buffer with the received
**                                        explicit message data
**                                        (not checked, only called with
**                                        reference)
**               as_recTunids (OUT)     - pointer to array of TUNID structs to
**                                        where the received data is written to
**                                        (not checked, only called with
**                                        reference)
**               au16_recPortNums (OUT) - pointer to array with received Port
**                                        Numbers
**                                        (not checked, only called with
**                                        reference)
**
** Returnvalue : CSS_k_TRUE             - received TUNID list contains TUNID(s)
**                                        different than 0xFF-TUNID
**               CSS_k_FALSE            - all received TUNIDs equal to all 0xFF
**
*******************************************************************************/
static CSS_t_BOOL ReceivedTunidListCopyandCheck(CSS_t_USINT u8_recNumPorts,
                                                const CSS_t_BYTE *pb_reqData,
                                                CSS_t_UNID *as_recTunids,
                                                CSS_t_UINT *au16_recPortNums)
{
  /* return value of this function */
  CSS_t_BOOL o_foundValidTunid = CSS_k_FALSE;
  /* temporary offset into received buffer while copying the received values */
  CSS_t_UINT u16_reqOffset = CSOS_k_SIZEOF_USINT;
  /* loop counter for processing all safety ports */
  CSS_t_USINT u8_portIdx;

  /* for all ports in the received list */
  for (u8_portIdx = 0U;
       u8_portIdx < u8_recNumPorts;
       u8_portIdx++)
  {
    /*
    ** copy the received data in local structures
    */
    /* Port Number */
    CSS_N2H_CPY16(&au16_recPortNums[u8_portIdx],
                  CSS_ADD_OFFSET(pb_reqData, u16_reqOffset));
    u16_reqOffset += CSOS_k_SIZEOF_UINT;
    /* TUNID.time */
    CSS_N2H_CPY32(&as_recTunids[u8_portIdx].s_snn.u32_time,
                  CSS_ADD_OFFSET(pb_reqData, u16_reqOffset));
    u16_reqOffset += CSOS_k_SIZEOF_UDINT;
    /* TUNID.date */
    CSS_N2H_CPY16(&as_recTunids[u8_portIdx].s_snn.u16_date,
                  CSS_ADD_OFFSET(pb_reqData, u16_reqOffset));
    u16_reqOffset += CSOS_k_SIZEOF_UINT;
    /* TUNID.NodeID */
    CSS_N2H_CPY32(&as_recTunids[u8_portIdx].u32_nodeId,
                  CSS_ADD_OFFSET(pb_reqData, u16_reqOffset));
    u16_reqOffset += CSOS_k_SIZEOF_UDINT;

    /* if the received TUNID for this port is all 0xFFs */
    if (IXSSO_UnidCompare(&IXSSO_ks_ffUnid, &as_recTunids[u8_portIdx]))
    {
      /* this is a FF TUNID - don't change the flag */
    }
    else  /* we have found a TUNID that is not FF */
    {
      /* set the flag */
      o_foundValidTunid = CSS_k_TRUE;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_foundValidTunid);
}


/*******************************************************************************
**
** Function    : PortsAndTunidCheck
**
** Description : This function compares the received TUNID list against the
**               previously received Proposed TUNID list. Additionally it checks
**               if the received Port Numbers are matching our list of supported
**               Port Numbers.
**
** Parameters  : u8_recNumPorts (IN)   - number of TUNIDs contained in
**                                       as_recTunids
**               as_recTunids (IN)     - pointer to array of TUNID structs to
**                                       where the received data is written to
**                                       (not checked, only called with
**                                       reference)
**               au16_recPortNums (IN) - pointer to array with received Port
**                                       Numbers
**                                       (not checked, only called with
**                                       reference)
**
** Returnvalue : CSS_k_TRUE             - received TUNID list was found to be
**                                        different than the Proposed TUNID list
**                                        or Port Numbers mismatch
**               CSS_k_FALSE            - all received TUNIDs and Port Numbers
**                                        are equal
**
*******************************************************************************/
static CSS_t_BOOL PortsAndTunidCheck(CSS_t_USINT u8_recNumPorts,
                                     const CSS_t_UNID *as_recTunids,
                                     const CSS_t_UINT *au16_recPortNums)
{
  /* return value of this function */
  CSS_t_BOOL o_unidListDifferent = CSS_k_FALSE;
  /* Get the list of the device's Port Numbers */
  const CSS_t_UINT *pau16_ourPorts = IXSSS_PortNumArrayPtrGet();
  /* loop counter for processing all safety ports */
  CSS_t_USINT u8_portIdx;

  /* check the received TUNID list against the Proposed TUNID list */
  /* check all received ports */
  for (u8_portIdx = 0U;
       u8_portIdx < u8_recNumPorts;
       u8_portIdx++)
  {
    /* if Apply_TUNID[i] matches Proposed_TUNID[i] */
    if (    (IXSSO_UnidCompare(&as_recTunids[u8_portIdx],
                               &as_PropUnid[u8_portIdx]))
         && (au16_recPortNums[u8_portIdx] == pau16_ourPorts[u8_portIdx])
       )
    {
      /* ok - TUNIDs are equal */
    }
    else  /* we have found a TUNID mismatch */
    {
      /* set the flag */
      o_unidListDifferent = CSS_k_TRUE;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_unidListDifferent);
}
#endif  /* (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U) */
#endif  /* (CSS_cfg_SET_TUNID == CSS_k_ENABLE) */

/*** End Of File ***/


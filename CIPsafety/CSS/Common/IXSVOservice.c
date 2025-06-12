/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOservice.c
**    Summary: IXSVO - Safety Validator Object
**             This module contains the implementation of the CIP services of
**             the Safety Validator Object.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_ExplMsgHandler
**             IXSVO_FaultCountersReset
**
**             SrvcInst
**             SrvcClassGetAttrSingle
**             SrvcClassResetErrCounts
**             SrvcInstGetAttrSingle
**             SrvcInstSetAttrSingle
**             TimeCoordMsgMinMultAttribGet
**             NetTimeExpectMultAttribGet
**             TimeoutMultAttribGet
**             FaultCountAttribGet
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

#include "IXSERapi.h"

#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVC.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
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

/** k_CSSC_xxx:
    Safety Validator Class specific service codes
*/
#define k_CSSC_RESET_ERR_COUNTS  0x4BU  /* "Reset all error counters" Service */


/** k_CAID_xxx:
    Class Attribute IDs
*/
#define k_CAID_SC_FAULT_COUNT    8U     /* Safety Connection Fault Count */


/** k_IAID_xxx:
    Instance Attribute IDs
*/
#define k_IAID_SV_STATE          1U     /* Safety Validator State */
#define k_IAID_SV_TYPE           2U     /* Safety Validator Type */
#define k_IAID_PIEM              3U     /* Ping Interval EPI Multiplier */
#define k_IAID_TCOOMMM           4U     /* Time Coordination Message Min Mult */
#define k_IAID_NTEM              5U     /* Network Time Expectation Mult */
#define k_IAID_TIMEOUT_MULT      6U     /* Timeout Multiplier */
#define k_IAID_MAX_CONS_NUM      7U     /* Max Consumer Number */
#define k_IAID_MAX_DATA_AGE      12U    /* Max Data Age */
#define k_IAID_APP_DATA_PATH     13U    /* Application Data Path */
#define k_IAID_ERROR_CODE        14U    /* Error Code */
/*lint -esym(750, k_IAID_EF_FAULT_COUNT)       not referenced in every config */
#define k_IAID_EF_FAULT_COUNT    15U    /* Producer/Consumer Fault Counters */


/** k_SVO_CLASS_REVISION_VALUE:
    Determines the revision of the object specification that this implementation
    is based on.
*/
#define k_SVO_CLASS_REVISION_VALUE   1U


/** k_LOG_SEG_CNXNPOINT_16:
    This define specifies the logical segment for a 16 bit connection point as
    needed in instance attribute 13 (Application Data Path). See Volume 1
    Section C-1.4.2.
*/
#define k_LOG_SEG_CNXNPOINT_16   (   (CSS_t_USINT)(0x01U << 5U)  \
                                   | (CSS_t_USINT)(0x03U << 2U)  \
                                   | (CSS_t_USINT)(0x01U)        \
                                 )


/* Maximum size of the data of the Time Coordination Message Min Multiplier
   Attribute */
#define k_TCMMM_ATTRIB_SIZE  (CSOS_k_SIZEOF_USINT                      \
                                   + (CSOS_k_MAX_CONSUMER_NUM_MCAST    \
                                      * (CSOS_k_SIZEOF_UINT            \
                                        )                              \
                                     )                                 \
                             )
/* static checking if the Time Coordination Message Min Multiplier attribute can
   fit into the Explicit Message Response Data buffer */
#if ((k_TCMMM_ATTRIB_SIZE) > CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE)
  #error k_TCMMM_ATTRIB_SIZE exceeds CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD SrvcInst(CSS_t_BOOL o_isServer,
                           CSS_t_UINT u16_svIdx,
                           const CSS_t_EXPL_REQ *ps_explReq,
                           CSS_t_EXPL_RSP *ps_explResp,
                           CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD SrvcClassGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                         CSS_t_EXPL_RSP *ps_explResp,
                                         CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD SrvcClassResetErrCounts(const CSS_t_EXPL_REQ *ps_explReq,
                                          CSS_t_EXPL_RSP *ps_explResp);
static CSS_t_WORD SrvcInstGetAttrSingle(CSS_t_BOOL o_isServer,
                                        CSS_t_UINT u16_svIdx,
                                        const CSS_t_EXPL_REQ *ps_explReq,
                                        CSS_t_EXPL_RSP *ps_explResp,
                                        CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD SrvcInstSetAttrSingle(CSS_t_BOOL o_isServer,
                                        CSS_t_UINT u16_svIdx,
                                        const CSS_t_EXPL_REQ *ps_explReq,
                                        CSS_t_EXPL_RSP *ps_explResp);
static CSS_t_WORD TimeCoordMsgMinMultAttribGet(CSS_t_BOOL o_isServer,
                                               CSS_t_UINT u16_svIdx,
                                               CSS_t_BOOL o_isMcast,
                                               CSS_t_EXPL_RSP *ps_explResp,
                                               CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD NetTimeExpectMultAttribGet(CSS_t_BOOL o_isServer,
                                             CSS_t_UINT u16_svIdx,
                                             CSS_t_BOOL o_isMcast,
                                             CSS_t_EXPL_RSP *ps_explResp,
                                             CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD TimeoutMultAttribGet(CSS_t_BOOL o_isServer,
                                       CSS_t_UINT u16_svIdx,
                                       CSS_t_BOOL o_isMcast,
                                       CSS_t_EXPL_RSP *ps_explResp,
                                       CSS_t_BYTE * const pb_rspData);
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
static CSS_t_WORD FaultCountAttribGet(CSS_t_BOOL o_isServer,
                                      CSS_t_UINT u16_svIdx,
                                      CSS_t_BOOL o_isMcast,
                                      CSS_t_EXPL_RSP *ps_explResp,
                                      CSS_t_BYTE * const pb_rspData);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVO_ExplMsgHandler
**
** Description : This function handles all explicit requests addressed to the
**               Safety Validator Object. It checks if a class or Instance
**               Service is requested and calls the corresponding function to
**               serve the request.
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
CSS_t_WORD IXSVO_ExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* if Instance ID is Class designator */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    /* switch for the Service Code */
    switch (ps_explReq->u8_service)
    {
      case CSOS_k_CCSC_GET_ATTR_SINGLE:
      {
        w_retVal = SrvcClassGetAttrSingle(ps_explReq, ps_explResp, pb_rspData);

        break;
      }

      case k_CSSC_RESET_ERR_COUNTS:
      {
        w_retVal = SrvcClassResetErrCounts(ps_explReq, ps_explResp);

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
  else /* else: service addresses an instance */
  {
    /* if invalid Instance ID received */
    if (ps_explReq->u32_instance > (CSOS_cfg_NUM_OF_SV_CLIENTS
                                   +CSOS_cfg_NUM_OF_SV_SERVERS))
    {
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
      w_retVal = CSS_k_OK;
    }
    else /* else: instance Id is valid */
    {
      /* passed instance ID (UINT as there can't be more Validator instances) */
      CSS_t_UINT u16_instance = (CSS_t_UINT)ps_explReq->u32_instance;
      /* index of the Safety Validator (depending on Server or Client) */
      CSS_t_UINT u16_svIdx = IXSVD_SvIdxFromInstGet(u16_instance);

      /* if it is server instance (range has been checked above) */
      if (IXSVD_InstIsServer(u16_instance))
      {
      #if  (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
        /* if this instance doesn't exist */
        if (u16_svIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
          w_retVal = CSS_k_OK;
        }
        else /* else: instance exists */
        {
          w_retVal = SrvcInst(CSS_k_TRUE, u16_svIdx,
                              ps_explReq, ps_explResp, pb_rspData);
        }
      #else
        {
          w_retVal = (IXSVO_k_FSE_INC_EXP_INST_SRV_S);
          SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
          ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
        }
      #endif
      }
      else /* else: it is a client instance */
      {
      #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
        /* o_isServer already initialized */

        /* if this instance doesn't exist */
        if (u16_svIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
        {
          ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
          w_retVal = CSS_k_OK;
        }
        else /* else: instance exists */
        {
          w_retVal = SrvcInst(CSS_k_FALSE, u16_svIdx,
                              ps_explReq, ps_explResp, pb_rspData);
        }
      #else
        {
          w_retVal = (IXSVO_k_FSE_INC_EXP_INST_SRV_C);
          SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
          ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
        }
      #endif
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVO_FaultCountersReset
**
** Description : This API function resets the Fault Counters of all Safety
**               Validator instances similar like the class specific service
**               "Reset all error counters" for the Safety Validator Class.
**
**               **Attention**: According to CIP Networks Library Volume 5 the
**               Fault Counters must be reset to 0 each hour. The CSS only
**               counts the errors but doesn't reset the counters. Thus it is
**               the responsibility of the Safety Application to call this
**               function each hour.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_FaultCountersReset(void)
{
  /* reset the fault counters of server and client instances */
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  {
    IXSVS_StateFaultCountReset();
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      IXSVS_ConsFaultCounterReset();
    }
  #endif
  }
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  {
    IXSVC_FaultCountReset();
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      IXSVC_TcooProdFaultCounterReset();
    }
  #endif
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : SrvcInst
**
** Description : This function handles the instance services for the Safety
**               Validator Instances.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               ps_explReq (IN)   - pointer to request message structure
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
static CSS_t_WORD SrvcInst(CSS_t_BOOL o_isServer,
                           CSS_t_UINT u16_svIdx,
                           const CSS_t_EXPL_REQ *ps_explReq,
                           CSS_t_EXPL_RSP *ps_explResp,
                           CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* switch for the Service Code */
  switch (ps_explReq->u8_service)
  {
    case CSOS_k_CCSC_GET_ATTR_SINGLE:
    {
      w_retVal = SrvcInstGetAttrSingle(o_isServer, u16_svIdx,
                                       ps_explReq, ps_explResp,
                                       pb_rspData);
      break;
    }

    case CSOS_k_CCSC_SET_ATTR_SINGLE:
    {
      w_retVal = SrvcInstSetAttrSingle(o_isServer, u16_svIdx,
                                       ps_explReq, ps_explResp);
      break;
    }

    default:
    {
      /* Instance Service not supported */
      ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}



/*******************************************************************************
**
** Function    : SrvcClassGetAttrSingle
**
** Description : This function serves the Get_Attribute_Single service for the
**               Safety Validator Class.
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
static CSS_t_WORD SrvcClassGetAttrSingle(const CSS_t_EXPL_REQ *ps_explReq,
                                         CSS_t_EXPL_RSP *ps_explResp,
                                         CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* temporary variable for copying attribute data to the explicit response */
  CSS_t_UINT u16_temp = 0U;

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
    w_retVal = CSS_k_OK;
  }
  /* else: if received request data length isn't ok */
  else if (ps_explReq->u16_reqDataLen > 0U)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
    w_retVal = CSS_k_OK;
  }
  else /* else: received data length is ok */
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Class Revision */
      case CSOS_k_CCCA_REVISION:
      {
        u16_temp = k_SVO_CLASS_REVISION_VALUE;
        CSS_H2N_CPY16(pb_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        w_retVal = CSS_k_OK;
        break;
      }

      /* Safety Connection Fault Count */
      case k_CAID_SC_FAULT_COUNT:
      {
        /* Add the fault counters from server and client instances. We don't  */
        /* have to care about overflows because this counter is auto-rollover */
        /* (see SRS75) */
        u16_temp = 0U;
      #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
        {
          u16_temp = (CSS_t_UINT)(u16_temp + IXSVS_StateFaultCountGet());
        }
      #endif
      #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
        {
          u16_temp = (CSS_t_UINT)(u16_temp + IXSVC_FaultCountGet());
        }
      #endif

        CSS_H2N_CPY16(pb_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        w_retVal = CSS_k_OK;
        break;
      }

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        w_retVal = CSS_k_OK;
        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : SrvcClassResetErrCounts
**
** Description : This function serves the class specific service "Reset all
**               error counters" for the Safety Validator Class.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to the response message that is
**                                   to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD SrvcClassResetErrCounts(const CSS_t_EXPL_REQ *ps_explReq,
                                          CSS_t_EXPL_RSP *ps_explResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* if the request EPATH contained an attribute ID */
  if (ps_explReq->u16_attribute != CSOS_k_INVALID_ATTRIBUTE)
  {
    /* wrong path format - for this service we don't need an attribute */
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
    w_retVal = CSS_k_OK;
  }
  /* else: if received request data length isn't ok */
  else if (ps_explReq->u16_reqDataLen > 0U)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
    w_retVal = CSS_k_OK;
  }
  else /* else: received data length is ok */
  {
    /* reset the fault counters of server and client instances */
    IXSVO_FaultCountersReset();

    ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : SrvcInstGetAttrSingle
**
** Description : This function serves the Get_Attribute_Single service for the
**               Safety Validator Instances.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               ps_explReq (IN)   - pointer to request message structure
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
static CSS_t_WORD SrvcInstGetAttrSingle(CSS_t_BOOL o_isServer,
                                        CSS_t_UINT u16_svIdx,
                                        const CSS_t_EXPL_REQ *ps_explReq,
                                        CSS_t_EXPL_RSP *ps_explResp,
                                        CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
#if ((CSOS_cfg_NUM_OF_SV_SERVERS == 0U) || (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U))
  /* passed instance ID (UINT as there can't be more Validator instances) */
  /* That it will fit into an UINT is sure as u32_instance has already been
     checked in IXSVO_ExplMsgHandler() */
  CSS_t_UINT u16_instance = (CSS_t_UINT)ps_explReq->u32_instance;
#endif

  /* already copy state and msg format to temporary variables */
  CSS_t_USINT u8_svState  = 0U;
  CSS_t_BYTE  b_msgFormat = 0x00U;

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
    w_retVal = CSS_k_OK;
  }
  /* else: if received request data length isn't ok */
  else if (ps_explReq->u16_reqDataLen > 0U)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
    w_retVal = CSS_k_OK;
  }
  else /* else: received data length is ok */
  {
    /*
    ** get the state of the safety validator instance and the message format
    */
    /* if it is a Server instance */
    if (o_isServer)
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      {
        u8_svState = IXSVS_StateGet(u16_svIdx);
        b_msgFormat = IXSVS_InitInfoMsgFrmtGet(u16_svIdx);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_GETSTAT_INST_S);
        SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    #endif
    }
    else /* else: it is a Client instance */
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      {
        u8_svState = IXSVC_StateGet(u16_svIdx);
        b_msgFormat = IXSVC_InitInfoMsgFrmtGet(u16_svIdx);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_GETSTAT_INST_C);
        SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    #endif
    }

  #if ((CSOS_cfg_NUM_OF_SV_CLIENTS == 0U) || (CSOS_cfg_NUM_OF_SV_SERVERS == 0U))
    if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
    {
      /* already found an error before - do not continue */
      ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
    else
  #endif
    {
      switch (ps_explReq->u16_attribute)
      {
        /* Safety Validator State */
        case k_IAID_SV_STATE:
        {
          CSS_H2N_CPY8(pb_rspData, &u8_svState);
          ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
          ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          w_retVal = CSS_k_OK;
          break;
        }

        /* Safety Validator Type */
        case k_IAID_SV_TYPE:
        {
          CSS_t_USINT u8_svType = 0U;

          /* if it is a Server instance */
          if (o_isServer)
          {
        #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              u8_svType = CSS_k_TYPE_SERVER;
            }
        #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETTYPE_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
          }
          else /* else: it is a Client instance */
          {
        #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              u8_svType = CSS_k_TYPE_CLIENT;
            }
        #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETTYPE_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
          }

          /* if it is a Single-cast Safety Validator */
          if (IXSVD_IsSingleCast(b_msgFormat))
          {
            u8_svType |= CSS_k_TYPE_SCAST;
          }
          else /* else: Multi-cast */
          {
            u8_svType |= CSS_k_TYPE_MCAST;
          }

          CSS_H2N_CPY8(pb_rspData, &u8_svType);
          ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
          ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          w_retVal = CSS_k_OK;
          break;
        }

        /* Ping Interval EPI Multiplier (see Req.5.3-15) */
        case k_IAID_PIEM:
        {
          CSS_t_UINT u16_piem = 0U;

          /* if it is a Server instance */
          if (o_isServer)
          {
          #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              u16_piem = IXSVS_PingIntEpiMultGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETPIEM_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }
          else /* else: it is a Client instance */
          {
          #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              u16_piem = IXSVC_PingIntEpiMultGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETPIEM_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }

        #if (    (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U)  \
              || (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)  \
            )
          if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
          {
            /* already found an error before - do not continue */
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
          else
        #endif
          {
            CSS_H2N_CPY16(pb_rspData, &u16_piem);
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            w_retVal = CSS_k_OK;
          }
          break;
        }

        /* Time Coordination Message Min Multiplier (see Req.5.3-15) */
        case k_IAID_TCOOMMM:
        {
          /* sub function will produce the response to get the Time Coordination
             Message Min Multiplier */
          /* mute lint warning about type conversions of Multi-cast flag */
          w_retVal =
            TimeCoordMsgMinMultAttribGet(o_isServer, u16_svIdx,
              (CSS_t_BOOL)IXSVD_IsMultiCast(b_msgFormat), /*lint !e960 */
              ps_explResp, pb_rspData);
          break;
        }

        /* Network Time Expectation Multiplier (see Req.5.3-15) */
        case k_IAID_NTEM:
        {
          /* sub function will produce the response to get the Network Time
             Expectation Multiplier */
          /* mute lint warning about type conversions of Multi-cast flag */
          w_retVal =
            NetTimeExpectMultAttribGet(o_isServer, u16_svIdx,
              (CSS_t_BOOL)IXSVD_IsMultiCast(b_msgFormat), /*lint !e960 */
              ps_explResp, pb_rspData);
          break;
        }

        /* Timeout Multiplier (see Req.5.3-15) */
        case k_IAID_TIMEOUT_MULT:
        {
          /* sub function will produce the response to get the Timeout
             Multiplier */
          /* mute lint warning about type conversions of Multi-cast flag */
          w_retVal =
            TimeoutMultAttribGet(o_isServer, u16_svIdx,
              (CSS_t_BOOL)IXSVD_IsMultiCast(b_msgFormat), /*lint !e960 */
              ps_explResp, pb_rspData);
          break;
        }

        /* Max Consumer Number (see Req.5.3-15) */
        case k_IAID_MAX_CONS_NUM:
        {
          CSS_t_USINT u8_maxConsNum = 0U;

          /* if it is a Server instance */
          if (o_isServer)
          {
          #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              u8_maxConsNum = IXSVS_MaxConsNumGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETMCN_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }
          else /* else: it is a Client instance */
          {
          #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              u8_maxConsNum = IXSVC_InitInfoMaxConsNumGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETMCN_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }

        #if (    (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U)  \
              || (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)  \
            )
          if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
          {
            /* already found an error before - do not continue */
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
          else
        #endif
          {
            CSS_H2N_CPY8(pb_rspData, &u8_maxConsNum);
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            w_retVal = CSS_k_OK;
          }
          break;
        }

        /* Max Data Age */
        case k_IAID_MAX_DATA_AGE:
        {
          CSS_t_UINT u16_maxDataAge_128us = 0U;

          /* if it is a Server instance */
          if (o_isServer)
          {
          #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              u16_maxDataAge_128us = IXSVS_MaxDataAgeGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETAGE_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }
          else /* else: it is a Client instance */
          {
          #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              u16_maxDataAge_128us = 0U;
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETAGE_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }

        #if (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)
          if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
          {
            /* already found an error before - do not continue */
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
          else
        #endif
          {
            CSS_H2N_CPY16(pb_rspData, &u16_maxDataAge_128us);
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            w_retVal = CSS_k_OK;
          }
          break;
        }

        /* Error Code (see Req.5.3-15) */
        case k_IAID_ERROR_CODE:
        {
          CSS_t_WORD w_lastErrCode = CSS_k_OK;

          /* if it is a Server instance */
          if (o_isServer)
          {
          #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              w_lastErrCode = IXSVS_ErrorLastGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETLEC_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }
          else /* else: it is a Client instance */
          {
          #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              w_lastErrCode = IXSVC_ErrorLastGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETLEC_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }

        #if (    (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U)  \
              || (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)  \
            )
          if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
          {
            /* already found an error before - do not continue */
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
          else
        #endif
          {
            CSS_H2N_CPY16(pb_rspData, &w_lastErrCode);
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_WORD;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            w_retVal = CSS_k_OK;
          }
          break;
        }

        /* Application Data Path */
        case k_IAID_APP_DATA_PATH:
        {
          CSS_t_UINT  u16_cp = 0U;
          CSS_t_USINT u8_logSeg = k_LOG_SEG_CNXNPOINT_16;

          /* if it is a Server instance */
          if (o_isServer)
          {
          #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            {
              u16_cp = IXSVS_InitInfoCnxnPointGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETCP_INST_S);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }
          else /* else: it is a Client instance */
          {
          #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              u16_cp = IXSVC_InitInfoCnxnPointGet(u16_svIdx);
            }
          #else
            {
              w_retVal = (IXSVO_k_FSE_INC_GETCP_INST_C);
              SAPL_CssErrorClbk(w_retVal, u16_instance, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
          #endif
          }

        #if (    (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U)  \
              || (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)  \
            )
          if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
          {
            /* already found an error before - do not continue */
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
          else
        #endif
          {
            CSS_H2N_CPY8(pb_rspData, &u8_logSeg);
            CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_rspData,
                                         CSOS_k_SIZEOF_USINT), &u16_cp);
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT
                                        + CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            w_retVal = CSS_k_OK;
          }
          break;
        }

      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        /* Producer/Consumer Fault Counters */
        case k_IAID_EF_FAULT_COUNT:
        {
          /* sub function will produce the response to get the Fault Counters */
          /* mute lint warning about type conversions of Multi-cast flag */
          w_retVal =
            FaultCountAttribGet(o_isServer, u16_svIdx,
              (CSS_t_BOOL)IXSVD_IsMultiCast(b_msgFormat), /*lint !e960 */
              ps_explResp, pb_rspData);
          break;
        }
      #endif

        default:
        {
          /* attribute not supported */
          ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
          w_retVal = CSS_k_OK;
          break;
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : SrvcInstSetAttrSingle
**
** Description : This function serves the Set_Attribute_Single service for the
**               Safety Validator Instances.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to the response message that is
**                                   to be returned
**                                   (not checked, only called with reference)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD SrvcInstSetAttrSingle(CSS_t_BOOL o_isServer,
                                        CSS_t_UINT u16_svIdx,
                                        const CSS_t_EXPL_REQ *ps_explReq,
                                        CSS_t_EXPL_RSP *ps_explResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* if request path doesn't contain an Attribute ID */
  if (ps_explReq->u16_attribute == CSOS_k_INVALID_ATTRIBUTE)
  {
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
    w_retVal = CSS_k_OK;
  }
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Safety Validator State */
      case k_IAID_SV_STATE:            /* missing break statement is intended */
      /* Safety Validator Type */
      case k_IAID_SV_TYPE:             /* missing break statement is intended */
      /* Ping Interval EPI Multiplier */
      case k_IAID_PIEM:                /* missing break statement is intended */
      /* Time Coordination Message Min Multiplier */
      case k_IAID_TCOOMMM:             /* missing break statement is intended */
      /* Network Time Expectation Multiplier */
      case k_IAID_NTEM:                /* missing break statement is intended */
      /* Timeout Multiplier */
      case k_IAID_TIMEOUT_MULT:        /* missing break statement is intended */
      /* Max Consumer Number */
      case k_IAID_MAX_CONS_NUM:        /* missing break statement is intended */
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        w_retVal = CSS_k_OK;
        break;
      }

      /* Max Data Age */
      case k_IAID_MAX_DATA_AGE:
      {
        /* if it is a Server instance */
        if (o_isServer)
        {
        #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
          {
            CSS_t_UINT u16_maxDataAge_128us = 0U;

            /* if received more data than expected */
            if (ps_explReq->u16_reqDataLen > CSOS_k_SIZEOF_UINT)
            {
              ps_explResp->u8_genStat = CSOS_k_CGSC_TOO_MUCH_DATA;
            }
            /* else: if received less data than expected */
            else if (ps_explReq->u16_reqDataLen < CSOS_k_SIZEOF_UINT)
            {
              ps_explResp->u8_genStat = CSOS_k_CGSC_NOT_ENOUGH_DATA;
            }
            else /* else: received data length is ok */
            {
              /* read the received data into a temporary variable */
              CSS_N2H_CPY16(&u16_maxDataAge_128us, ps_explReq->pb_reqData);

              /* only a set to zero is allowed */
              /* if the received attribute value is not zero */
              if (u16_maxDataAge_128us != 0U)
              {
                /* reject with error: invalid attribute value */
                ps_explResp->u8_genStat = CSOS_k_CGSC_INVALID_ATTR_VAL;
              }
              else  /* else: received attribute value is zero */
              {
                /* copy the received value to the SVS instance */
                IXSVS_MaxDataAgeSet(u16_svIdx, u16_maxDataAge_128us);
                ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
              }
            }
          }
        #else
          {
            u16_svIdx = u16_svIdx;   /* just to mute compiler warning */
            w_retVal = (IXSVO_k_FSE_INC_SETAGE_INST_S);
            /* earlier checks make sure instance fits in an UINT */
            SAPL_CssErrorClbk(w_retVal, (CSS_t_UINT)ps_explReq->u32_instance,
                              IXSER_k_A_NOT_USED);
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
        }
        else /* else: it is a Client instance */
        {
        #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
          {
            /* for producers this attribute is not settable */
            ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
          }
        #else
          {
            u16_svIdx = u16_svIdx;   /* just to mute compiler warning */
            w_retVal = (IXSVO_k_FSE_INC_SETAGE_INST_C);
            /* earlier checks make sure instance fits in an UINT */
            SAPL_CssErrorClbk(w_retVal, (CSS_t_UINT)ps_explReq->u32_instance,
                              IXSER_k_A_NOT_USED);
            ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
        }

        w_retVal = CSS_k_OK;
        break;
      }

      /* Application Data Path */
      case k_IAID_APP_DATA_PATH:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        w_retVal = CSS_k_OK;
        break;
      }

      /* Error Code */
      case k_IAID_ERROR_CODE:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        w_retVal = CSS_k_OK;
        break;
      }

    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      /* Producer/Consumer Fault Counters */
      case k_IAID_EF_FAULT_COUNT:
      {
        /* attribute not settable */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SETTABLE;
        w_retVal = CSS_k_OK;
        break;
      }
    #endif

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        w_retVal = CSS_k_OK;
        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal); /*lint !e438 Last value assigned to variable
                                  'u16_svIdx' may be unused (depending on
                                  configuration,
                                  i.e. CSOS_cfg_NUM_OF_SV_SERVERS == 0) */

/* Any missing break is commented above */
/* RSM_IGNORE_QUALITY_BEGIN Notice #14   - 'case' conditions do not equal ... */
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : TimeCoordMsgMinMultAttribGet
**
** Description : This function is a sub function of SrvcInstGetAttrSingle for
**               generating the response data for the Time Coordination Message
**               Min Multiplier attribute.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               o_isMcast (IN)    - TRUE if this is a multi-cast connection
**                                   (not checked - any value allowed)
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
static CSS_t_WORD TimeCoordMsgMinMultAttribGet(CSS_t_BOOL o_isServer,
                                               CSS_t_UINT u16_svIdx,
                                               CSS_t_BOOL o_isMcast,
                                               CSS_t_EXPL_RSP *ps_explResp,
                                               CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* number of members of the Producer/Consumer Fault array */
  CSS_t_USINT u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
  /* temporary variable */
  CSS_t_UINT u16_temp;
  /* temporary pointer to the response data buffer */
  CSS_t_BYTE  *pb_rspDataBuf = pb_rspData;

  /*
  ** determine Array Size
  */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTCMMM_I_S_2);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    if (o_isMcast)
    {
      u8_size = IXSVC_InitInfoMaxConsNumGet(u16_svIdx);
    }
    else
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      o_isMcast = o_isMcast;    /* just to avoid unused parameter warning */
      /* u8_size already initialized */
      /* IXSVO_k_FSE_INC_GETTCMMM_INST_C will be generated below */
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

  /* Maximum size of this attribute is 31 (1+15*2) bytes. This will always    */
  /* fit into the explicit message response buffer. Thus no size check        */
  /* required here.                                                           */

  /* put Array Size into response buffer */
  CSS_H2N_CPY8(pb_rspDataBuf, &u8_size);
  pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);

  /* put Time Coordination Message Min Multiplier(s) into the buffer depending
     on the type of connection */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u16_temp = IXSVS_TcooMsgMinMultGet(u16_svIdx);
      CSS_H2N_CPY16(pb_rspDataBuf, &u16_temp);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTCMMM_INST_S);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    CSS_t_USINT u8_idx = 0U;

    for (u8_idx = 0U; u8_idx < u8_size; u8_idx++)
    {
      u16_temp = IXSVC_TcooMsgMinMultGet(u16_svIdx, u8_idx);
      CSS_H2N_CPY16(pb_rspDataBuf, &u16_temp);
      pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_UINT);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTCMMM_INST_C);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

#if ((CSOS_cfg_NUM_OF_SV_SERVERS == 0U) || (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U))
  if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
  {
    /* an error occurred - return value already set */
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
#endif
  {
    /* determine the number of bytes that have been written to the        */
    /* response buffer.                                                   */
    ps_explResp->u16_rspDataLen = (CSS_t_UINT)(CSOS_k_SIZEOF_USINT +
      ((CSS_t_UINT)u8_size * CSOS_k_SIZEOF_UINT));
    ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal); /*lint !e438 Last value assigned to variable
                                  'o_isMcast' may be unused (depending on
                                  configuration,
                                  i.e. CSOS_cfg_NUM_OF_SV_CLIENTS == 0) */
}


/*******************************************************************************
**
** Function    : NetTimeExpectMultAttribGet
**
** Description : This function is a sub function of SrvcInstGetAttrSingle for
**               generating the response data for the Network Time Expectation
**               Multiplier attribute.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               o_isMcast (IN)    - TRUE if this is a multi-cast connection
**                                   (not checked - any value allowed)
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
static CSS_t_WORD NetTimeExpectMultAttribGet(CSS_t_BOOL o_isServer,
                                             CSS_t_UINT u16_svIdx,
                                             CSS_t_BOOL o_isMcast,
                                             CSS_t_EXPL_RSP *ps_explResp,
                                             CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* number of members of the Producer/Consumer Fault array */
  CSS_t_USINT u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
  /* temporary variable */
  CSS_t_UINT u16_temp;
  /* temporary pointer to the response data buffer */
  CSS_t_BYTE  *pb_rspDataBuf = pb_rspData;

  /*
  ** determine Array Size
  */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETNTEM_I_S_2);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    if (o_isMcast)
    {
      u8_size = IXSVC_InitInfoMaxConsNumGet(u16_svIdx);
    }
    else
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      o_isMcast = o_isMcast;    /* just to avoid unused parameter warning */
      /* u8_size already initialized */
      /* IXSVO_k_FSE_INC_GETNTEM_INST_C will be generated below */
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

  /* Maximum size of this attribute is 31 (1+15*2) bytes. This will always    */
  /* fit into the explicit message response buffer. Thus no size check        */
  /* required here.                                                           */

  /* put Array Size into response buffer */
  CSS_H2N_CPY8(pb_rspDataBuf, &u8_size);
  pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);

  /* put Network Time Expectation Multiplier(s) into the buffer depending on
     the type of connection */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u16_temp = IXSVS_InitInfoNetTimeExpMultGet(u16_svIdx);
      CSS_H2N_CPY16(pb_rspDataBuf, &u16_temp);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETNTEM_INST_S);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    CSS_t_USINT u8_idx = 0U;

    for (u8_idx = 0U; u8_idx < u8_size; u8_idx++)
    {
      u16_temp = IXSVC_NetTimeExpMultGet(u16_svIdx, u8_idx);
      CSS_H2N_CPY16(pb_rspDataBuf, &u16_temp);
      pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_UINT);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETNTEM_INST_C);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

#if ((CSOS_cfg_NUM_OF_SV_SERVERS == 0U) || (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U))
  if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
  {
    /* an error occurred - return value already set */
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
#endif
  {
    /* determine the number of bytes that have been written to the        */
    /* response buffer.                                                   */
    ps_explResp->u16_rspDataLen = (CSS_t_UINT)(CSOS_k_SIZEOF_USINT +
      ((CSS_t_UINT)u8_size * CSOS_k_SIZEOF_UINT));
    ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal); /*lint !e438 Last value assigned to variable
                                  'o_isMcast' may be unused (depending on
                                  configuration,
                                  i.e. CSOS_cfg_NUM_OF_SV_CLIENTS == 0) */
}


/*******************************************************************************
**
** Function    : TimeoutMultAttribGet
**
** Description : This function is a sub function of SrvcInstGetAttrSingle for
**               generating the response data for the Timeout Multiplier
**               attribute.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               o_isMcast (IN)    - TRUE if this is a multi-cast connection
**                                   (not checked - any value allowed)
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
static CSS_t_WORD TimeoutMultAttribGet(CSS_t_BOOL o_isServer,
                                       CSS_t_UINT u16_svIdx,
                                       CSS_t_BOOL o_isMcast,
                                       CSS_t_EXPL_RSP *ps_explResp,
                                       CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* number of members of the Producer/Consumer Fault array */
  CSS_t_USINT u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
  /* temporary variable */
  CSS_t_USINT u8_temp;
  /* temporary pointer to the response data buffer */
  CSS_t_BYTE  *pb_rspDataBuf = pb_rspData;

  /*
  ** determine Array Size
  */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTMULT_I_S_2);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    if (o_isMcast)
    {
      u8_size = IXSVC_InitInfoMaxConsNumGet(u16_svIdx);
    }
    else
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      o_isMcast = o_isMcast;    /* just to avoid unused parameter warning */
      /* u8_size already initialized */
      /* IXSVO_k_FSE_INC_GETTMULT_INST_C will be generated below */
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

  /* Maximum size of this attribute is 16 (1+15) bytes. This will always fit  */
  /* into the explicit message response buffer. Thus no size check required   */
  /* here.                                                                    */

  /* put Array Size into response buffer */
  CSS_H2N_CPY8(pb_rspDataBuf, &u8_size);
  pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);

  /* put Timeout Multiplier(s) into response buffer depending on the type of
     connection */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_temp = IXSVS_TimeoutMultiplierGet(u16_svIdx);
      CSS_H2N_CPY8(pb_rspDataBuf, &u8_temp);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTMULT_INST_S);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    CSS_t_USINT u8_idx = 0U;

    for (u8_idx = 0U; u8_idx < u8_size; u8_idx++)
    {
      u8_temp = IXSVC_InstInfoTimeoutMultGet(u16_svIdx, u8_idx);
      CSS_H2N_CPY8(pb_rspDataBuf, &u8_temp);
      pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETTMULT_INST_C);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

#if ((CSOS_cfg_NUM_OF_SV_SERVERS == 0U) || (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U))
  if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
  {
    /* an error occurred - return value already set */
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
#endif
  {
    /* determine the number of bytes that have been written to the        */
    /* response buffer.                                                   */
    ps_explResp->u16_rspDataLen = (CSS_t_UINT)(CSOS_k_SIZEOF_USINT +
      ((CSS_t_UINT)u8_size * CSOS_k_SIZEOF_USINT));
    ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal); /*lint !e438 Last value assigned to variable
                                  'o_isMcast' may be unused (depending on
                                  configuration,
                                  i.e. CSOS_cfg_NUM_OF_SV_CLIENTS == 0) */
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : FaultCountAttribGet
**
** Description : This function is a sub function of SrvcInstGetAttrSingle for
**               generating the response data for the Producer/Consumer Fault
**               Counter attribute.
**
** Parameters  : o_isServer (IN)   - boolean flag indicating if this instance is
**                                   a Server (CSS_k_TRUE) or a client
**                                   (CSS_k_FALSE)
**                                   (not checked, any value allowed)
**               u16_svIdx (IN)    - Server or Client Safety Validator Index
**                                   (only usable in conjunction with the
**                                   o_isServer flag)
**                                   (not checked, checked by
**                                   IXSVO_ExplMsgHandler())
**               o_isMcast (IN)    - TRUE if this is a multi-cast connection
**                                   (not checked - any value allowed)
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
static CSS_t_WORD FaultCountAttribGet(CSS_t_BOOL o_isServer,
                                      CSS_t_UINT u16_svIdx,
                                      CSS_t_BOOL o_isMcast,
                                      CSS_t_EXPL_RSP *ps_explResp,
                                      CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* number of members of the Producer/Consumer Fault array */
  CSS_t_USINT u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
  /* temporary variable */
  CSS_t_USINT u8_temp;
  /* temporary pointer to the response data buffer */
  CSS_t_BYTE  *pb_rspDataBuf = pb_rspData;

  /*
  ** determine Array Size
  */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETFC_INST_S_2);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    if (o_isMcast)
    {
      u8_size = IXSVC_InitInfoMaxConsNumGet(u16_svIdx);
    }
    else
    {
      u8_size = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }
  #else
    {
      o_isMcast = o_isMcast;    /* just to avoid unused parameter warning */
      /* u8_size already initialized */
      /* IXSVO_k_FSE_INC_GETFC_INST_C will be generated below */
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

  /* Maximum size of this attribute is 16 (1+15) bytes. This will always fit  */
  /* into the explicit message response buffer. Thus no size check required   */
  /* here.                                                                    */

  /* put Array Size into response buffer */
  CSS_H2N_CPY8(pb_rspDataBuf, &u8_size);
  pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);

  /* put Consumer- or Producer_Fault_Counter(s) into response buffer
     depending on the type of connection (see FRS375) */
  /* if it is a Server instance */
  if (o_isServer)
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      u8_temp = IXSVS_ConsFaultCounterGet(u16_svIdx);
      CSS_H2N_CPY8(pb_rspDataBuf, &u8_temp);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETFC_INST_S);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a Client instance */
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    CSS_t_USINT u8_idx = 0U;

    for (u8_idx = 0U; u8_idx < u8_size; u8_idx++)
    {
      u8_temp = IXSVC_TcooFaultCounterGet(u16_svIdx, u8_idx);
      CSS_H2N_CPY8(pb_rspDataBuf, &u8_temp);
      pb_rspDataBuf = CSS_ADD_OFFSET(pb_rspDataBuf, CSOS_k_SIZEOF_USINT);
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_GETFC_INST_C);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

#if ((CSOS_cfg_NUM_OF_SV_SERVERS == 0U) || (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U))
  if (w_retVal != IXSVO_k_FSE_INC_PRG_FLOW)
  {
    /* an error occurred - return value already set */
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
#endif
  {
    /* determine the number of bytes that have been written to the        */
    /* response buffer.                                                   */
    ps_explResp->u16_rspDataLen = (CSS_t_UINT)(CSOS_k_SIZEOF_USINT +
      ((CSS_t_UINT)u8_size * CSOS_k_SIZEOF_USINT));
    ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal); /*lint !e438 Last value assigned to variable
                                  'o_isMcast' may be unused (depending on
                                  configuration,
                                  i.e. CSOS_cfg_NUM_OF_SV_CLIENTS == 0) */
}
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


/*** End Of File ***/


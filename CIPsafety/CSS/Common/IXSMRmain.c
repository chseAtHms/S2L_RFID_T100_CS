/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSMRmain.c
**    Summary: IXSMR - Safety Message Router Object
**             This unit implements the safety related parts of the Message
**             Router Object. It obtains the service requests from the CSAL
**             and distributes them to the safety objects respectively to the
**             application.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSMR_CmdProcess
**
**             ExplMsgHandler
**             ExplMsgProcess
**             ExplMsgProcess2
**             ExplMsgRoute
**             ExplMsgRespSend
**             ExplReqInit
**             ExplRespInit
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

#include "IXEPP.h"
#include "IXSERapi.h"
#include "IXSCE.h"
#include "IXSVO.h"
#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXSSS.h"
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "IXCCO.h"
#endif
#include "IXSCF.h"

#include "IXSMRapi.h"
#include "IXSMRerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** k_INV_EXPL_RESP_DATA_LEN:
    This define is the initial value for the explicit response data length. If
    this value is still set after the processing of the Object returns then it
    is obvious, that the Object has not written this field.
*/
#define k_INV_EXPL_RESP_DATA_LEN   0xFFFFU

/*
** The following defines are used for the "Message Router Request" and "Message
** Router Response" according to Volume 1 (Edition 3.10 Section 2-4).
*/

/** k_EXPL_RESP_HDR_MIN_SIZE:
    This define determines the minimum size of the header of an explicit
    response message (consisting of "Reply Service", "reserved octet", "General
    Status" and "Size of Additional Status").
*/
#define k_EXPL_RESP_HDR_MIN_SIZE   (4U * CSOS_k_SIZEOF_USINT)

/** k_LEN_SMRR_xxx:
    These defines determine the number of bytes of the fields of explicit
    request messages.
*/
/* Service Code */
#define k_LEN_SMRR_SRVC_CODE       CSOS_k_SIZEOF_USINT
/* Request Path Size */
#define k_LEN_SMRR_REQ_PATH_SIZE   CSOS_k_SIZEOF_USINT

/** k_OFS_MRR_xxx:
    These defines determine the offset of the fields of explicit request
    messages within the received byte stream.
*/
/* Service Code */
#define k_OFS_SMRR_SRVC_CODE       (0U)
/* Request Path Size */
#define k_OFS_SMRR_REQ_PATH_SIZE   (k_OFS_SMRR_SRVC_CODE + k_LEN_SMRR_SRVC_CODE)
/* Request Path */
#define k_OFS_SMRR_REQ_PATH        (k_OFS_SMRR_REQ_PATH_SIZE + \
                                   k_LEN_SMRR_REQ_PATH_SIZE)

/** k_MIN_EXPL_REQ_LEN
    This define determines the minimum length of a valid explicit request
    message.
*/
#define k_MIN_EXPL_REQ_LEN         (   k_OFS_SMRR_SRVC_CODE        \
                                     + k_LEN_SMRR_SRVC_CODE        \
                                     + k_LEN_SMRR_REQ_PATH_SIZE    \
                                   )


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD ExplMsgHandler(const HALCS_t_MSG *ps_rxHalcMsg);
static CSS_t_WORD ExplMsgProcess(const HALCS_t_MSG *ps_rxHalcMsg,
                                 CSS_t_EXPL_RSP *ps_explResp,
                                 CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD ExplMsgProcess2(CSS_t_DWORD dw_valid,
                                  const IXEPP_t_FIELDS *ps_ePathField,
                                  CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_BYTE * const pb_rspData,
                                  CSS_t_EXPL_REQ *ps_explReq);
static CSS_t_WORD ExplMsgRoute(const CSS_t_EXPL_REQ *ps_explReq,
                               CSS_t_EXPL_RSP *ps_explResp,
                               CSS_t_BYTE * const pb_rspData);
static CSS_t_WORD ExplMsgRespSend(CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_UDINT u32_addInfo,
                                  CSS_t_BYTE * const pb_rspData);
static void ExplReqInit(CSS_t_EXPL_REQ *ps_explReq);
static void ExplRespInit(CSS_t_EXPL_RSP *ps_explResp);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSMR_CmdProcess
**
** Description : This API function processes the commands received via HALC
**               which address the IXSMR unit.
**
**               **Attention**:
**               This function processes received explicit request messages and
**               produces the responses. After reception of an IXSMR command
**               this function must be called fast enough not to run into
**               explicit message timeouts on the requesting client.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure (checked):
**                                   valid range: <> CSS_k_NULL
**                                   - u16_cmd (checked)
**                                     valid range: CSOS_k_CMD_IXMRO_EXPL_REQ
**                                   - u16_len (not checked, checked in
**                                     ExplMsgProcess(),
**                                     valid range: k_MIN_EXPL_REQ_LEN .. 65535)
**                                   - u32_addInfo
**                                     (not checked, any value allowed)
**                                   - pb_data (not checked, checked in
**                                     ExplMsgProcess())
**                                   **Attention**:
**                                   By processing an explicit request a
**                                   response is  generated and passed to
**                                   HALCS_TxDataPut(). If sending via
**                                   HALCS_TxDataPut() is not possible an info
**                                   error (IXSMR_k_NFSE_TXH_RSP_SEND_ERR) is
**                                   generated and the application must react
**                                   appropriately.
**
** Returnvalue : -
**
*******************************************************************************/
void IXSMR_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* if passed pointer is invalid */
  if (ps_rxHalcMsg == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSMR_k_FSE_AIP_PTR_INV_CMDPROC, IXSER_k_I_NOT_USED,
                      IXSER_k_A_NOT_USED);
  }
  else /* passed pointer is valid */
  {
    /* if CIP Safety Stack is in an improper state */
    if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
    {
      /* CIP Safety Stack is not in running state */
      SAPL_CssErrorClbk(IXSMR_k_FSE_AIS_STATE_ERROR, IXSER_k_I_NOT_USED,
                        IXSER_k_A_NOT_USED);
    }
    else
    {
      /* determine which command was issued */
      switch (ps_rxHalcMsg->u16_cmd)
      {
        case CSOS_k_CMD_IXMRO_EXPL_REQ:
        {
          /* pass message to handler function for received explicit messages */
          /* return value ignored (called function provides this for historical
             reasons */
          (void)ExplMsgHandler(ps_rxHalcMsg);

          break;
        }

        default:
        {
          /* command unknown to this unit */
          SAPL_CssErrorClbk(IXSMR_k_NFSE_RXH_CMD_UNKNOWN, IXSER_k_I_NOT_USED,
                            (CSS_t_UDINT)ps_rxHalcMsg->u16_cmd);
          break;
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : ExplMsgHandler
**
** Description : This function handles received explicit message requests. It
**               manages processing of the request and sending the response.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   (not checked, checked in
**                                   IXSMR_CmdProcess())
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD ExplMsgHandler(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSMR_k_FSE_INC_PRG_FLOW;
  /* Explicit Response structure */
  CSS_t_EXPL_RSP s_explResp;
  /* buffer for explicit response message (header + data) */
  CSS_t_BYTE ab_respBuf[CSOS_k_EXPL_RESP_HDR_MAX_SIZE
                         + CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE];
  CSS_t_BYTE *pb_rspData;

  /* completely erase structure */
  CSS_MEMSET(&s_explResp, 0, sizeof(s_explResp));

  /* Initialize the explicit response message structure */
  ExplRespInit(&s_explResp);

  /* Make a pointer that points to the Explicit Response message buffer. But  */
  /* at the begin we leave space for the header.                              */
  pb_rspData = CSS_ADD_OFFSET(ab_respBuf, CSOS_k_EXPL_RESP_HDR_MAX_SIZE);

  /* After processing returns we can check for this initial value to find     */
  /* out if the Object has set the response data length.                      */
  s_explResp.u16_rspDataLen = k_INV_EXPL_RESP_DATA_LEN;

  /* call explicit message processing function */
  w_retVal = ExplMsgProcess(ps_rxHalcMsg, &s_explResp, pb_rspData);

  /* if previous function returned error */
  if (w_retVal != CSS_k_OK)
  {
    /* In case there is such a severe error that explicit message processing  */
    /* function returns with error we better do not try to send a response.   */
  }
  else /* ok */
  {
    /* let response sending function do the job */
    w_retVal = ExplMsgRespSend(&s_explResp, ps_rxHalcMsg->u32_addInfo,
                               pb_rspData);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExplMsgProcess
**
** Description : This function processes a received explicit message. It parses
**               the received message and stores the parameters to a structure.
**               Afterwards it calls the routing function.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   - u16_cmd (not checked, checked in
**                                     IXSMR_CmdProcess())
**                                   - u16_len (checked,
**                                     valid range: k_MIN_EXPL_REQ_LEN ..
**                                     65535))
**                                   - u32_addInfo (not checked, any value
**                                     allowed)
**                                   - pb_data (checked)
**                                     valid range: <> CSS_k_NULL
**               ps_explResp (OUT) - pointer to the response message that is
**                                   to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - Pointer to Explicit Response Data (not
**                                   checked, only called with fixed offset to
**                                   buffer reference)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD ExplMsgProcess(const HALCS_t_MSG *ps_rxHalcMsg,
                                 CSS_t_EXPL_RSP *ps_explResp,
                                 CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSMR_k_FSE_INC_PRG_FLOW;
  /* Request_Path_Size in words */
  CSS_t_UINT u16_reqPathSizeB;
  /* remaining request length */
  CSS_t_UINT u16_remLen;
  /* number of words in the Request_Path */
  CSS_t_USINT  u8_reqPathSizeW;
  /* EPATH structure */
  IXEPP_t_FIELDS s_ePathField;
  /* bit string to signal which EPATH fields are found */
  CSS_t_DWORD dw_valid = IXEPP_k_SFB_NO_VALID_FIELD;
  /* Explicit Request structure */
  CSS_t_EXPL_REQ s_explReq;

  /* completely erase structures */
  CSS_MEMSET(&s_ePathField, 0, sizeof(s_ePathField));
  CSS_MEMSET(&s_explReq, 0, sizeof(s_explReq));

  /* if the data pointer in the HALC message is invalid */
  if (ps_rxHalcMsg->pb_data == CSS_k_NULL)
  {
    w_retVal = IXSMR_k_FSE_AIP_PTR_INV_EXPLMSG;
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* pointer in HALC message is valid */
  {
    /* Initialize the explicit request message structure */
    ExplReqInit(&s_explReq);

    /* extract the request ID */
    s_explReq.u32_reqId = ps_rxHalcMsg->u32_addInfo;

    /* if length is less than minimum length of an explicit message */
    if (ps_rxHalcMsg->u16_len < k_MIN_EXPL_REQ_LEN)
    {
      /* received message is too short for being a valid explicit request */
      ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SIZE_INVALID;
      SAPL_CssErrorClbk(IXSMR_k_NFSE_RXE_MSG_TOO_SHORT,
                        IXSER_k_I_NOT_USED,
                        (CSS_t_UDINT)ps_rxHalcMsg->u16_len);

      /* Function return value is not an error. Instead the Explicit Response
         message indicates the Error */
      w_retVal = CSS_k_OK;
    }
    else /* length ok */
    {
      /* calculate start of CIP data */
      u16_remLen = (CSS_t_UINT)(ps_rxHalcMsg->u16_len - k_OFS_SMRR_SRVC_CODE);

      /* extract the service code */
      CSS_N2H_CPY8(&s_explReq.u8_service,
                   CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                  k_OFS_SMRR_SRVC_CODE));

      /* already set the reply service code in the response */
      ps_explResp->u8_service = (CSS_t_USINT)(s_explReq.u8_service
                              | CSOS_k_BIT_REPLY_SERVICE_CODE);

      /* extract Request_Path_Size */
      CSS_N2H_CPY8(&u8_reqPathSizeW,
                   CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                  k_OFS_SMRR_REQ_PATH_SIZE));
      u16_reqPathSizeB = (CSS_t_UINT)((CSS_t_UINT)u8_reqPathSizeW * 2U);

      /* update remaining length */
      u16_remLen -= (CSS_t_UINT)(k_LEN_SMRR_SRVC_CODE
                                 + k_LEN_SMRR_REQ_PATH_SIZE);

      /* if remaining length is less than Request_Path_Size */
      if (u16_remLen < u16_reqPathSizeB)
      {
        /* received message is too short for being a valid explicit request */
        ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SIZE_INVALID;
        SAPL_CssErrorClbk(IXSMR_k_NFSE_RXE_REQ_PATH_SIZE,
                          u16_remLen,
                          (CSS_t_UDINT)u16_reqPathSizeB);

        /* CIP error is returned, but this function succeeds */
        w_retVal = CSS_k_OK;
      }
      else /* length ok */
      {
        /* update remaining length */
        u16_remLen = (CSS_t_UINT)(u16_remLen - u16_reqPathSizeB);

        /* if the request contains Request_Data */
        if (u16_remLen > 0U)
        {
          /* store the length of the Request Data */
          s_explReq.u16_reqDataLen = u16_remLen;
          /* store a pointer to the request data */
          s_explReq.pb_reqData = CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                                k_OFS_SMRR_REQ_PATH +
                                                u16_reqPathSizeB);
        }
        else /* no request data */
        {
          /* ok: this is a service without Request_Data */
          s_explReq.u16_reqDataLen = 0U;
          s_explReq.pb_reqData = CSS_k_NULL;
        }

        /* Request_Path is parsed */
        ps_explResp->u8_genStat =
          IXEPP_EPathParse(CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                          k_OFS_SMRR_REQ_PATH),
                           CSS_k_TRUE, u16_reqPathSizeB,
                           &dw_valid, &s_ePathField);

        /* if previous function returned error */
        if (ps_explResp->u8_genStat != CSOS_k_CGSC_SUCCESS)
        {
          /* error: EPATH parsing failed */
          /* EPP has already reported a precise error info to the application */
          /* CIP error is returned, but this function succeeds */
          w_retVal = CSS_k_OK;
        }
        else /* ok */
        {
          /* if request doesn't contain an Electronic Key */
          if ((dw_valid & IXEPP_k_SFB_ELECTRONIC_KEY)
               != IXEPP_k_SFB_ELECTRONIC_KEY)
          {
            /* Electronic Key is optional - continue */
          }
          else /* else: Electronic Key is present */
          {
            /* When there is an Electronic Key then it must be checked */
            /* structure to store the parsed values */
            CSS_t_ELECTRONIC_KEY s_elKey;

            /* completely erase structure */
            CSS_MEMSET(&s_elKey, 0, sizeof(s_elKey));

            /* parse the received Electronic Key Segment */
            w_retVal = IXSCE_ElectronicKeyParse(s_ePathField.s_logSeg.pba_elKey,
                                                &s_elKey);
            /* if previous function returned an error */
            if (w_retVal != CSS_k_OK)
            {
              ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_SEG_ERROR;
              w_retVal = CSS_k_OK;
            }
            else /* else: parsing successful */
            {
              /* if received Electronic Key doesn't match with our identity */
              if (!IXSCE_ElectronicKeyMatchCheck(&s_elKey,
                                                 &ps_explResp->u8_genStat,
                                                 &ps_explResp->au16_addStat[0]))
              {
                /* if the electronic key checking function returned a
                   Connection Failure */
                if (ps_explResp->u8_genStat == CSOS_k_CGSC_CNXN_FAILURE)
                {
                  /* As this function is shared between Message Router and
                     Electronic Key Checking of the Connection Manager (i.e.
                     Safety Validator Connection Establishment Engine) we have
                     to adapt the return value in cases where Connection Manager
                     specific values are returned. This can be done this way as
                     the Message Router Object defines its extended status
                     codes identically as the Connection Manager. */
                  ps_explResp->u8_genStat = CSOS_k_CGSC_KEY_FAILURE_IN_PATH;
                }
                else /* else: exact general status code is already set */
                {
                  /* nothing to be done here */
                }

                /* if no additional status has been set */
                if (ps_explResp->au16_addStat[0] == CSS_k_CMEXTSTAT_UNDEFINED)
                {
                  /* no additional Status */
                }
                else /* else: there is an additional status */
                {
                  ps_explResp->u8_addStatSize++;
                  /* extended status value already set */
                }

                /* w_retVal already set to CSS_k_OK */
              }
              else /* else: Key matches */
              {
                /* w_retVal already set to CSS_k_OK */
              }
            }
          }

          /* if previous checks returned error */
          if (ps_explResp->u8_genStat != CSOS_k_CGSC_SUCCESS)
          {
            /* stop processing - error codes already set */
          }
          else /* else: previous steps successful */
          {
            w_retVal = ExplMsgProcess2(dw_valid, &s_ePathField,
                                       ps_explResp, pb_rspData, &s_explReq);
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExplMsgProcess2
**
** Description : This function continues to processes a received explicit
**               message. It parses the received message and stores the
**               parameters to a structure. Afterwards it calls the routing
**               function.
**
** Parameters  : dw_valid (IN)      - bit string to signal which EPATH fields
**                                    were found
**                                    (not checked, any value allowed)
**               ps_ePathField (IN) - pointer to structure with data that were
**                                    contained in the EPATH
**               ps_explResp (OUT)  - pointer to the response message that is
**                                    to be returned
**                                    (not checked, only called with reference)
**               pb_rspData (OUT)   - Pointer to Explicit Response Data (not
**                                    checked, only called with fixed offset to
**                                    buffer reference)
**               ps_explReq (OUT)   - pointer to the request message that is to
**                                    be passed to further functions for
**                                    processing
**                                    (not checked, only called with reference
**                                    to structure variable)
**
** Returnvalue : CSS_k_OK           - success
**               <>CSS_k_OK         - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD ExplMsgProcess2(CSS_t_DWORD dw_valid,
                                  const IXEPP_t_FIELDS *ps_ePathField,
                                  CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_BYTE * const pb_rspData,
                                  CSS_t_EXPL_REQ *ps_explReq)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSMR_k_FSE_INC_PRG_FLOW;

  /* if request doesn't contain a Class ID */
  if ((dw_valid & IXEPP_k_SFB_CLASS_ID_1) != IXEPP_k_SFB_CLASS_ID_1)
  {
    /* error: cannot route a request without a class ID */
    ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
    SAPL_CssErrorClbk(IXSMR_k_NFSE_RXE_REQ_WO_CLASS,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);

    /* CIP error is returned, but this function succeeds */
    w_retVal = CSS_k_OK;
  }
  else /* received a Class ID */
  {
    /* extract Class ID */
    ps_explReq->u16_class = ps_ePathField->s_logSeg.u16_classId1;

    /* if request doesn't contain an Instance ID */
    if ((dw_valid & IXEPP_k_SFB_INST_ID_1) != IXEPP_k_SFB_INST_ID_1)
    {
      /* error: cannot route a request that doesn't contain an
         Instance ID */
      ps_explResp->u8_genStat = CSOS_k_CGSC_PATH_DST_UNKNOWN;
      SAPL_CssErrorClbk(IXSMR_k_NFSE_RXE_REQ_WO_INST,
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);

      /* CIP error is returned, but this function succeeds */
      w_retVal = CSS_k_OK;
    }
    else /* received an Instance ID */
    {
      /* extract Instance ID */
      ps_explReq->u32_instance = ps_ePathField->s_logSeg.u32_instId1;

      /* if request doesn't contain an Attribute ID */
      if ((dw_valid & IXEPP_k_SFB_ATTR_ID) != IXEPP_k_SFB_ATTR_ID)
      {
        /* Attribute ID is optional. If there is none this is ok */
      }
      else /* received an Attribute Id */
      {
        ps_explReq->u16_attribute = ps_ePathField->s_logSeg.u16_attrId;
      }

      /* if request contains a Member ID */
      if ((dw_valid & IXEPP_k_SFB_MEMB_ID) != IXEPP_k_SFB_MEMB_ID)
      {
        /* Member ID is optional. If there is none this is ok */
      }
      else /* received a Member Id */
      {
        ps_explReq->u32_member = ps_ePathField->s_logSeg.u32_membId;
      }

      /* Perform the message routing */
      w_retVal = ExplMsgRoute(ps_explReq, ps_explResp, pb_rspData);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExplMsgRoute
**
** Description : This function performs the message routing. It analyzes to
**               which class the request is addressed and calls the handler
**               function of this class. If it is not addressed to a known class
**               then it calls the callback to let the application handle the
**               request.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**                                   - u32_reqId (not checked, any value
**                                     allowed)
**                                   - u8_service (not checked, checked in
**                                     called functions)
**                                   - u16_class  (not checked, any value
**                                     allowed)
**                                   - u32_instance (not checked, any value
**                                     allowed)
**                                   - u16_attribute (not checked, any value
**                                     allowed)
**                                   - u16_reqDataLen (not checked, checked in
**                                     called functions)
**                                   - pb_reqData (not checked, any value
**                                     allowed)
**               ps_explResp (OUT) - pointer to the response message that is
**                                   to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - Pointer to Explicit Response Data
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
static CSS_t_WORD ExplMsgRoute(const CSS_t_EXPL_REQ *ps_explReq,
                               CSS_t_EXPL_RSP *ps_explResp,
                               CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSMR_k_FSE_INC_PRG_FLOW;

  /* route the request to the given Class ID */
  switch (ps_explReq->u16_class)
  {
    case CSOS_k_CCC_S_SUPERVISOR_OBJ:
    {
      /* Call message handler of the Safety Supervisor Object */
      w_retVal = IXSSO_ExplMsgHandler(ps_explReq, ps_explResp, pb_rspData);
      break;
    }

    case CSOS_k_CCC_S_VALIDATOR_OBJ:
    {
      /* Call message handler of the Safety Validator Object */
      w_retVal = IXSVO_ExplMsgHandler(ps_explReq, ps_explResp, pb_rspData);
      break;
    }

  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
    /* the class ID used for the Connection Configuration Object is user-
       configurable */
    case CSS_cfg_CCO_CLASS_ID:
    {
      w_retVal = IXCCO_ExplMsgHandler(ps_explReq, ps_explResp, pb_rspData);
      break;
    }
  #endif

    default:
    {
      /* the request is not addressed to an object that is implemented in   */
      /* the stack. Thus forward this request to the application.           */
      /* if the API function serviced the request */
      if (SAPL_IxsmrObjReqClbk(ps_explReq, ps_explResp, pb_rspData)
          != CSS_k_FALSE)
      {
        /* processing finished */
        w_retVal = CSS_k_OK;
      }
      else /* Application has not serviced the request */
      {
        /* error: Object does not exist */
        ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
        SAPL_CssErrorClbk(IXSMR_k_NFSE_RXE_UNKNOWN_CLASS,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);

        /* CIP error is returned, but this function succeeds */
        w_retVal = CSS_k_OK;
      }

      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExplMsgRespSend
**
** Description : This function sends an explicit response message.
**
** Parameters  : ps_explResp (IN) - pointer to the response message that is
**                                  to be sent
**                                  (not checked, only called with reference to
**                                   struct)
**                                  - o_respond (not checked, any value allowed)
**                                  - u8_service (not checked, any value
**                                    allowed)
**                                  - u8_genStat (not checked, any value
**                                    allowed)
**                                  - u8_addStatSize (checked, valid range:
**                                    0..CSOS_cfg_ADD_STAT_SIZE_MAX)
**                                  - au16_addStat (not checked, any values
**                                    allowed)
**                                  - u16_rspDataLen (checked, valid range:
**                                    0..CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE)
**               u32_addInfo (IN) - addInfo Parameter from CSAL that was
**                                  obtained from the request and must be echoed
**                                  back with the response
**                                  (not checked, any value allowed)
**               pb_rspData (OUT) - Pointer to Explicit Response Data
**                                  (not checked, only called with fixed offset
**                                  to buffer reference)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while sending the message
**
*******************************************************************************/
static CSS_t_WORD ExplMsgRespSend(CSS_t_EXPL_RSP *ps_explResp,
                                  CSS_t_UDINT u32_addInfo,
                                  CSS_t_BYTE * const pb_rspData)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSMR_k_FSE_INC_PRG_FLOW;
  /* loop counter */
  CSS_t_USINT u8_idx = 0U;
  /* pointer into the response buffer to the position to where we want to     */
  /* write the next field                                                     */
  CSS_t_BYTE *pb_nextBytePos = pb_rspData;
  /* Tx HALC message for sending the Explicit Response */
  HALCS_t_MSG s_txHalcMsg;
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  CSS_t_BYTE ab_tempBuf[CSOS_k_SIZEOF_UDINT + CSOS_k_SIZEOF_UINT] =
    {0U, 0U, 0U, 0U, 0U, 0U};
#endif

  /* completely erase structure */
  CSS_MEMSET(&s_txHalcMsg, 0, sizeof(s_txHalcMsg));

  /* some safety checks */
  /* if too much data written into response buffer or too many additional
     status words */
  if (    (    (ps_explResp->u16_rspDataLen != k_INV_EXPL_RESP_DATA_LEN)
            && (ps_explResp->u16_rspDataLen > CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE)
          )
       || (ps_explResp->u8_addStatSize > CSOS_cfg_ADD_STAT_SIZE_MAX)
     )
  {
    /* Fatal error! Called functions have written beyond the Explicit
       message buffer. Other variables and stack may be corrupted! */
    w_retVal = IXSMR_k_FSE_INC_BUF_OVERWRITE;
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* data in response buffer is ok */
  {
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
    /* if the CSAL has to insert data into this explicit response */
    if (ps_explResp->o_csalInsert != CSS_k_FALSE)
    {
      /* temporary store Application Reference and Config Data Offset */
      CSS_MEMCPY(ab_tempBuf, pb_nextBytePos, sizeof(ab_tempBuf));
      /* correct the pointer to the payload data */
      pb_nextBytePos = CSS_ADD_OFFSET(pb_nextBytePos, sizeof(ab_tempBuf));
    }
    else
    {
      /* pb_nextBytePos already correctly initialized */
    }
#endif
    /*
    ** Place the explicit message response header in front of the data:
    ** pb_rspData points to the beginning of the Explicit Message Response
    ** Data (payload). The buffer for the whole Explicit Response message is
    ** larger: before the payload data there is space for the header of the
    ** Explicit Response message (CSOS_k_EXPL_RESP_HDR_MAX_SIZE bytes). Thus
    ** we calculate negative offsets for copying the fields of the header
    ** into the buffer. Because Additional Status has variable size we write
    ** the header from back to front.
    ** Refer to Volume 1 Edition 3.10 Section 2-4.2 for Message Router
    ** Response format.
    */
    /* additional status words */
    for (u8_idx = 0U; u8_idx < ps_explResp->u8_addStatSize; u8_idx++)
    {
      CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_nextBytePos,
                                   (-1 * ((CSS_t_INT)(u8_idx) + 1)
                                    * (CSS_t_INT)CSOS_k_SIZEOF_WORD)),
                    &ps_explResp->au16_addStat[u8_idx]);
    }

    /* calculate byte position of Additional Status Size */
    pb_nextBytePos =
      CSS_ADD_OFFSET(pb_nextBytePos,
                      ((-1 *
                       ((CSS_t_INT)ps_explResp->u8_addStatSize)
                       * (CSS_t_INT)CSOS_k_SIZEOF_WORD)) - 1);
    /* number of additional status words */
    CSS_H2N_CPY8(pb_nextBytePos, &ps_explResp->u8_addStatSize);

    /* calculate byte position of General Status */
    pb_nextBytePos =
      CSS_ADD_OFFSET(pb_nextBytePos,
                     ((-1 * (CSS_t_INT)CSOS_k_SIZEOF_USINT)));
    /* copy general status */
    CSS_H2N_CPY8(pb_nextBytePos, &ps_explResp->u8_genStat);

    /* calculate byte position of Reserved Byte */
    pb_nextBytePos =
      CSS_ADD_OFFSET(pb_nextBytePos,
                     ((-1 * (CSS_t_INT)CSOS_k_SIZEOF_USINT)));
    /* reserved byte */
    u8_idx = 0U;
    CSS_H2N_CPY8(pb_nextBytePos, &u8_idx);

    /* calculate byte position of Reply Service */
    pb_nextBytePos =
      CSS_ADD_OFFSET(pb_nextBytePos,
                     ((-1 * (CSS_t_INT)CSOS_k_SIZEOF_USINT)));
    /* copy the service code */
    CSS_H2N_CPY8(pb_nextBytePos, &ps_explResp->u8_service);

    /* if the object did not modify the response length */
    if (ps_explResp->u16_rspDataLen == k_INV_EXPL_RESP_DATA_LEN)
    {
      /* Object that generated this response did not set the              */
      /* response length. In this case we can assume that this is a       */
      /* regular success or error response without data.                  */
      /* Thus set the length of the response to be sent to zero */
      ps_explResp->u16_rspDataLen = 0U;
    }
    else /* object has set the response length */
    {
      /* Don't change the response length */
    }

    /*
    ** assemble the HALC message
    */
  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
    /* if the CSAL has to insert data into this explicit response */
    if (ps_explResp->o_csalInsert != CSS_k_FALSE)
    {
      /* adjust Config Data Offset with Message Router Response Header */
      CSS_t_UINT u16_cfgDataOffset;
      CSS_N2H_CPY16(&u16_cfgDataOffset,
                    CSS_ADD_OFFSET(ab_tempBuf, CSOS_k_SIZEOF_UDINT));
      u16_cfgDataOffset =
        (CSS_t_UINT)(u16_cfgDataOffset
                   + (CSS_t_UINT)(k_EXPL_RESP_HDR_MIN_SIZE
                                + (ps_explResp->u8_addStatSize
                                   * (CSS_t_UINT)CSOS_k_SIZEOF_WORD)));

      /* restore Application Reference and Config Data Offset */
      pb_nextBytePos = CSS_ADD_OFFSET(pb_nextBytePos,
                                      ((-1 * (CSS_t_INT)sizeof(ab_tempBuf))));
      CSS_MEMCPY(pb_nextBytePos, ab_tempBuf, CSOS_k_SIZEOF_UDINT);
      CSS_H2N_CPY16(CSS_ADD_OFFSET(pb_nextBytePos, CSOS_k_SIZEOF_UDINT),
                    &u16_cfgDataOffset);

      /* use different command code to indicate this to CSAL */
      s_txHalcMsg.u16_cmd = CSOS_k_CMD_IXSMR_EXPL_RES_INS;
    }
    else /* no CSAL insertion required */
    {
      /* use regular command for HALC message */
      s_txHalcMsg.u16_cmd = CSOS_k_CMD_IXSMR_EXPL_RES;
    }
  #else
    {
      s_txHalcMsg.u16_cmd = CSOS_k_CMD_IXSMR_EXPL_RES;
    }
  #endif
    s_txHalcMsg.u16_len = (CSS_t_UINT)(k_EXPL_RESP_HDR_MIN_SIZE
                        + (ps_explResp->u8_addStatSize
                           * (CSS_t_UINT)CSOS_k_SIZEOF_WORD)
                        + ps_explResp->u16_rspDataLen);
    s_txHalcMsg.u32_addInfo = u32_addInfo;
    s_txHalcMsg.pb_data = pb_nextBytePos;

    /* send the response to HALC */
    /* if transmit function returns an error */
    if (HALCS_TxDataPut(&s_txHalcMsg) == CSS_k_FALSE)
    {
      /* error while trying to send the message to CSAL */
      w_retVal = IXSMR_k_NFSE_TXH_RSP_SEND_ERR;
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, u32_addInfo);
    }
    else /* HALC message transmitted successfully */
    {
      /* return success */
      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExplReqInit
**
** Description : This function initializes an explicit request structures with
**               default values.
**
** Parameters  : ps_explReq (IN) - pointer to structure to be initialized
**                                 (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void ExplReqInit(CSS_t_EXPL_REQ *ps_explReq)
{
  ps_explReq->u32_reqId       = 0U;
  ps_explReq->u8_service      = CSOS_k_CCSC_INVALID_SERVICE;
  ps_explReq->u16_class       = CSOS_k_CCC_INVALID_CLASS;
  ps_explReq->u32_instance    = CSOS_k_INVALID_INSTANCE_32;
  ps_explReq->u16_attribute   = CSOS_k_INVALID_ATTRIBUTE;
  ps_explReq->u32_member      = CSOS_k_INVALID_MEMBER_32;
  ps_explReq->u16_reqDataLen  = 0U;
  ps_explReq->pb_reqData      = CSS_k_NULL;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : ExplRespInit
**
** Description : This function initializes an explicit response structure with
**               default values.
**
** Parameters  : ps_explResp (OUT) - pointer to structure to be initialized
**                                   (not checked, only called with reference)
**
** Returnvalue : -
**
*******************************************************************************/
static void ExplRespInit(CSS_t_EXPL_RSP *ps_explResp)
{
  /* loop counter */
  CSS_t_USINT u8_idx = 0U;

  ps_explResp->u8_addStatSize = 0U;
  for (u8_idx = 0U; u8_idx < CSOS_cfg_ADD_STAT_SIZE_MAX; u8_idx++)
  {
    ps_explResp->au16_addStat[u8_idx] = CSOS_k_INVALID_ADD_STATUS;
  }

  ps_explResp->u8_service     = 0U;  /* will be copied from request */
  ps_explResp->u8_genStat     = CSOS_k_CGSC_INVALID_STATUS_CODE;

  /* After processing of the explicit request returns we can check for this   */
  /* initial value to find out if the Object has set the response data length */
  ps_explResp->u16_rspDataLen = k_INV_EXPL_RESP_DATA_LEN;

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  {
    ps_explResp->o_csalInsert = CSS_k_FALSE;/* default: no insertions by CSAL */
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*** End Of File ***/


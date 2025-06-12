/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEmain.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module contains the main routines of the IXSCE unit
**             (command handler, initiation of Forward_Open/Forward_CLose
**             processing, sending of CIP response messages).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_CmdProcess
**
**             FwdOpenHandler
**             FwdCloseHandler
**             FwdOpenSuccessReplySend
**             FwdOpenErrorReplySend
**             FwdCloseSuccessReplySend
**             FwdCloseErrorReplySend
**             ExtStatCodeHasMoreWords
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXSSOapi.h"
#include "IXSSOerr.h"
#include "IXSSO.h"
#include "IXCRC.h"
#include "IXSSS.h"
#include "IXSCF.h"

#include "IXSCEapi.h"
#include "IXSCE.h"
#include "IXSCEint.h"
#include "IXSCEerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* data sizes needed for assembling response messages */
#define k_FWD_OPEN_SUCREP_HDR_SIZE_B       4U
#define k_FWD_OPEN_SUCREP_APP_SIZE_BASE_W  5U
#define k_FWD_OPEN_SUCREP_APP_SIZE_EXT_W   7U
#define k_FWD_OPEN_SUCREP_RSPDAT_SIZE_B    26U
#define k_FWD_OPEN_SUCREP_SIZE_BASE_B (CSOS_k_SIZEOF_OPEN_CNXN_HDR +           \
                                       k_FWD_OPEN_SUCREP_HDR_SIZE_B +          \
                                       (k_FWD_OPEN_SUCREP_APP_SIZE_BASE_W*2U)+ \
                                       k_FWD_OPEN_SUCREP_RSPDAT_SIZE_B)
#define k_FWD_OPEN_SUCREP_SIZE_EXT_B  (CSOS_k_SIZEOF_OPEN_CNXN_HDR +           \
                                       k_FWD_OPEN_SUCREP_HDR_SIZE_B +          \
                                       (k_FWD_OPEN_SUCREP_APP_SIZE_EXT_W*2U) + \
                                       k_FWD_OPEN_SUCREP_RSPDAT_SIZE_B)
#define k_FWD_OPEN_ERRREP_SIZE_B      (CSOS_k_SIZEOF_OPEN_CNXN_HDR + 16U +     \
                                       (CSS_k_MAX_NUM_EXT_STATUS_WORDS *       \
                                       CSOS_k_SIZEOF_WORD))
#define k_FWD_CLOSE_SUCREP_SIZE_B     (CSOS_k_SIZEOF_CLOSE_CNXN_HDR +       \
                                       4U+10U)
#define k_FWD_CLOSE_ERRREP_SIZE_B     (CSOS_k_SIZEOF_CLOSE_CNXN_HDR+6U+10U)


/* default number of WORDs to be returned as additional status in error resp */
#define k_ERRREP_ADDSTAT_SIZE          1U


/*
** Offsets for generating Forward_Open/Forward_Close Success/Error Responses
*/

/* Message Router Response Format Offsets */
#define k_OFS_MRR_SERVICE_CODE          0U   /* Reply Service Code */
#define k_OFS_MRR_RESERVED              1U   /* Reserved Byte */
#define k_OFS_MRR_GEN_STATUS            2U   /* General Status */
#define k_OFS_MRR_ADD_STATUS_SIZE       3U   /* Additional Status Size */
#define k_OFS_MRR_ADD_STATUS            4U   /* Add Status Word (1st) */
#define k_OFS_MRR_ADD_STATUS_MORE       6U   /* Add Status Word (2nd) */

/* Successful Forward_Open Response Offsets */
#define k_OFS_SFOR_NCNXNID_OT           4U   /* Network Connection ID O->T */
#define k_OFS_SFOR_NCNXNID_TO           8U   /* Network Connection ID T->O */
#define k_OFS_SFOR_CNXN_S_NUM           12U  /* Connection Serial Number */
#define k_OFS_SFOR_ORIG_V_ID            14U  /* Originator Vendor ID */
#define k_OFS_SFOR_ORIG_S_NUM           16U  /* Orig Device Serial Num */
#define k_OFS_SFOR_API_OT               20U  /* Actual Packet Interval O->T */
#define k_OFS_SFOR_API_TO               24U  /* Actual Packet Interval T->O */
#define k_OFS_SFOR_APP_REPLY_SIZE       28U  /* Application Reply Size */
#define k_OFS_SFOR_RESERVED             29U  /* Reserved Byte */

/* Application Reply Offsets (Successful Forward_Open Response) */
/*lint -esym(750, k_OFS_APPR_INIT_TIME_STAMP)  not referenced in every config */
/*lint -esym(750, k_OFS_APPR_INIT_ROLLOVER_VAL) n. referenced in every config */
#define k_OFS_APPR_CONS_NUM             30U  /* Consumer Number */
#define k_OFS_APPR_PCID_TARG_V_ID       32U  /* PID/CID Target Vendor ID */
#define k_OFS_APPR_PCID_TARG_DEV_S_NUM  34U  /* PID/CID Targ Dev Serial Num */
#define k_OFS_APPR_PCID_TARG_CNXN_S_NUM 38U  /* PID/CID Targ Cnxn Ser Num */
#define k_OFS_APPR_INIT_TIME_STAMP      40U  /* Init Time Stamp (ext only) */
#define k_OFS_APPR_INIT_ROLLOVER_VAL    42U  /* Init Rollover Value (e.o.) */

/* Successful Forward_Close Response Offsets */
#define k_OFS_SFCR_CNXN_S_NUM            4U  /* Connection Serial Number */
#define k_OFS_SFCR_ORIG_V_ID             6U  /* Originator Vendor ID */
#define k_OFS_SFCR_ORIG_S_NUM            8U  /* Orig Device Serial Num */
#define k_OFS_SFCR_APP_REPLY_SIZE       12U  /* Application Reply Size */
#define k_OFS_SFCR_RESERVED             13U  /* Reserved Byte */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD FwdOpenHandler(const HALCS_t_MSG *ps_rxHalcMsg);
static CSS_t_WORD FwdCloseHandler(const HALCS_t_MSG *ps_rxHalcMsg);

static CSS_t_WORD FwdOpenSuccessReplySend(const CSS_t_SOPEN_PARA
                                             *ps_sOpenPar,
                                          const CSS_t_SOPEN_RESP
                                             *ps_sOpenResp,
                                          CSS_t_UDINT u32_addInfo);
static CSS_t_WORD FwdOpenErrorReplySend(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                        CSS_t_UDINT u32_addInfo,
                                  const CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static CSS_t_WORD FwdCloseSuccessReplySend(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                           const IXSCE_t_SCLOSE_RESP
                                              *ps_sCloseResp,
                                           CSS_t_UDINT u32_addInfo);
static CSS_t_WORD FwdCloseErrorReplySend(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                         const IXSCE_t_SCLOSE_RESP
                                            *ps_sCloseResp,
                                         CSS_t_UDINT u32_addInfo);
static CSS_t_BOOL ExtStatCodeHasMoreWords(CSS_t_USINT u8_genStat,
                                          CSS_t_UINT u16_extStatCode);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_CmdProcess
**
** Description : This API function processes the commands received via HALC
**               which address the IXSCE unit.
**
**               **Attention**:
**               This function processes received Forward_Open and Forward_Close
**               requests and produces the responses. After reception of an
**               IXSCE command this function must be called fast enough not to
**               run into timeouts of these services on the originator.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure (checked):
**                                   valid range: <> CSS_k_NULL
**                                   - u16_cmd (checked)
**                                     valid range: CSOS_k_CMD_IXCMO_SOPEN_REQ,
**                                     CSOS_k_CMD_IXCMO_SCLOSE_REQ
**                                   - u16_len (not checked, checked in
**                                     FwdOpenHandler() and FwdCloseHandler()
**                                     valid range: 0 .. 65535)
**                                   - u32_addInfo
**                                     (not checked, any value allowed)
**                                   - pb_data (checked,
**                                     valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCE_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* if passed pointer is invalid */
  if (ps_rxHalcMsg == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSCE_k_FSE_AIP_PTR_INV_CMDPROC, IXSER_k_I_NOT_USED,
                      IXSER_k_A_NOT_USED);
  }
  else /* passed pointer is valid */
  {
    /* if data pointer in passed structure is invalid */
    if (ps_rxHalcMsg->pb_data == CSS_k_NULL)
    {
      SAPL_CssErrorClbk(IXSCE_k_FSE_AIP_PTR_INV_CMDDATA, IXSER_k_I_NOT_USED,
                        IXSER_k_A_NOT_USED);
    }
    else /* data pointer is valid */
    {
      /* if CIP Safety Stack is in an improper state */
      if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
      {
        /* CIP Safety Stack is not in running state */
        SAPL_CssErrorClbk(IXSCE_k_FSE_AIS_STATE_ERROR, IXSER_k_I_NOT_USED,
                          IXSER_k_A_NOT_USED);
      }
      else /* CSS is in a proper state */
      {
        /* Forward_Open and Forward_Close is the main entry point to Target   */
        /* functionality (see Req.5.2-3 and Req.5.3-3).                       */
        /* determine which command was issued */
        switch (ps_rxHalcMsg->u16_cmd)
        {
          case CSOS_k_CMD_IXCMO_SOPEN_REQ:
          {
            /* pass message to handler function for the specified service */
            /* return value ignored (called function provides this for
               historical reasons */
            (void)FwdOpenHandler(ps_rxHalcMsg);

            break;
          }

          case CSOS_k_CMD_IXCMO_SCLOSE_REQ:
          {
            /* pass message to handler function for the specified service */
            /* return value ignored (called function provides this for
               historical reasons */
            (void)FwdCloseHandler(ps_rxHalcMsg);

            break;
          }

          default:
          {
            /* command unknown to this unit */
            SAPL_CssErrorClbk(IXSCE_k_NFSE_RXH_CMD_UNKNOWN, IXSER_k_I_NOT_USED,
                              (CSS_t_UDINT)ps_rxHalcMsg->u16_cmd);
            break;
          }
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
** Function    : FwdOpenHandler
**
** Description : This function handles the Forward_Open (SafetyOpen) commands
**               received via HALC.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure
**                                   (not checked, checked in
**                                   IXSCE_CmdProcess())
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the command
**
*******************************************************************************/
static CSS_t_WORD FwdOpenHandler(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for storing the parameters of the received SafetyOpen message */
  CSS_t_SOPEN_PARA  s_sOpenPara;
  /* structure for storing additional (CSS-internal) information */
  IXSCE_t_SOPEN_AUX s_sOpenAuxData;
  /* structure for storing information that is needed  for the response */
  CSS_t_SOPEN_RESP  s_sOpenResp;

  /* initialization of the structs */
  CSS_MEMSET(&s_sOpenPara,    0, sizeof(s_sOpenPara)    );
  CSS_MEMSET(&s_sOpenAuxData, 0, sizeof(s_sOpenAuxData) );
  CSS_MEMSET(&s_sOpenResp,    0, sizeof(s_sOpenResp)    );

  /* make sure CIP error return value is different than CSOS_k_CGSC_SUCCESS */
  s_sOpenResp.s_status.u8_gen = CSOS_k_CGSC_INVALID_STATUS_CODE;
  s_sOpenResp.s_status.u16_ext = CSOS_k_INVALID_ADD_STATUS;
  s_sOpenResp.s_status.u8_numMoreExt = 0U;

  /* let the SSO state machine check if it is allowed to open a cnxn */
  /* (see SRS80) */
  w_retVal = IXSSO_StateMachine(IXSSO_k_SE_FWD_OPEN_REQ, IXSSO_k_SEAI_NONE);

  /* if is in WAIT_FOR_TUNID state */
  if (w_retVal == IXSSO_k_NFSE_RXE_FO_WAIT_TUNID)
  {
    /* Opening a connection is not allowed while TUNID isn't yet set */
    s_sOpenResp.s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    s_sOpenResp.s_status.u16_ext = CSS_k_CMEXTSTAT_TUNID_NOT_SET;
    /* already reported by SSO state machine to SAPL via SAPL_CssErrorClbk() */
    w_retVal = CSS_k_OK;
  }
  /* else: if is in other invalid states */
  else if (w_retVal != CSS_k_OK)
  {
    /* Connection is not allowed in current device state */
    s_sOpenResp.s_status.u8_gen = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    s_sOpenResp.s_status.u16_ext = CSOS_k_INVALID_ADD_STATUS;
    /* already reported by SSO state machine to SAPL via SAPL_CssErrorClbk() */
    w_retVal = CSS_k_OK;
  }
  else /* else: state is valid */
  {
    /* parse the Forward_Open request and put the parameters into a structure */
    w_retVal = IXSCE_FwdOpenParse(ps_rxHalcMsg->pb_data,
                                  ps_rxHalcMsg->u16_len,
                                  CSS_k_FALSE,
                                  &s_sOpenPara,
                                  &s_sOpenAuxData,
                                  &s_sOpenResp.s_status);

    /* if previous function returned an error */
    if (w_retVal != CSS_k_OK)
    {
      /* parsing error: CIP error code already set */
    }
    else /* else: no error */
    {
      /* if parsing found an error */
      if (s_sOpenResp.s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
      {
        /* parsing found an error in received Forward_Open parameters */
      }
      else /* else: parsing result ok */
      {
        CSS_t_BOOL o_isExtFormat;

        /* if is base format */
        if (s_sOpenPara.s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
        {
          o_isExtFormat = CSS_k_FALSE;
        }
        else /* else: extended format */
        {
          o_isExtFormat = CSS_k_TRUE;
        }

        /* Calculate the CPCRC over the received SafetyOpen data */
        s_sOpenAuxData.u32_cpcrcCalc = IXSCE_CpcrcCalc(ps_rxHalcMsg->pb_data,
                                                       o_isExtFormat,
                                                       &s_sOpenAuxData);

        /* make sure CIP error return value is different than
           CSOS_k_CGSC_SUCCESS */
        s_sOpenResp.s_status.u8_gen = CSOS_k_CGSC_INVALID_STATUS_CODE;
        s_sOpenResp.s_status.u16_ext = CSOS_k_INVALID_ADD_STATUS;
        s_sOpenResp.s_status.u8_numMoreExt = 0U;

        /* process the received SafetyOpen request */
        w_retVal = IXSCE_SafetyOpenProc(&s_sOpenPara,
                                        &s_sOpenAuxData,
                                        &s_sOpenResp);
      }
    }
  }

  /* if there was a severe processing error (normal protocol errors lead to
     a CIP error code being set but w_retVal is still ok) */
  if (w_retVal != CSS_k_OK)
  {
    /* don't respond - error already reported to application */
  }
  else /* else: no error */
  {
    /* determine what kind of response must be sent */
    /* if status is not success */
    if (s_sOpenResp.s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
    {
      /* opening the connection was not successful */
      w_retVal = FwdOpenErrorReplySend(&s_sOpenPara,
                                       ps_rxHalcMsg->u32_addInfo,
                                       &s_sOpenResp.s_status);
    }
    else /* else: success */
    {
      /* connection successfully opened */
      w_retVal = FwdOpenSuccessReplySend(&s_sOpenPara,
                                         &s_sOpenResp,
                                         ps_rxHalcMsg->u32_addInfo);
    }
  }

  /* Request has been processed. CSS is now ready for processing further */
  /* requests (see SRS11 bullet 13 and SRS 12 bullet 13) */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : FwdCloseHandler
**
** Description : This function handles the Forward_Close commands received via
**               HALC.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure
**                                   (not checked, checked in
**                                   IXSCE_CmdProcess())
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the command
**
*******************************************************************************/
static CSS_t_WORD FwdCloseHandler(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for storing the parameters of received Forward_Close message */
  CSS_t_CNXN_TRIAD s_cnxnTriad;
  /* structure for storing information that is needed  for the response */
  IXSCE_t_SCLOSE_RESP s_sCloseResp;

  /* initialization of the structs */
  CSS_MEMSET(&s_cnxnTriad, 0, sizeof(s_cnxnTriad));
  CSS_MEMSET(&s_sCloseResp, 0, sizeof(s_sCloseResp));

  /* make sure CIP error return value is different than CSOS_k_CGSC_SUCCESS */
  s_sCloseResp.s_status.u8_gen = CSOS_k_CGSC_INVALID_STATUS_CODE;

  /* parse the Forward_Open request and put the parameters into a structure */
  IXSCE_FwdCloseParse(ps_rxHalcMsg, &s_cnxnTriad, &s_sCloseResp.s_status);

  /* if previous function returned an error or parsing failed */
  if (s_sCloseResp.s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
  {
    /* error while parsing the received Forward_Close request */
  }
  else /* parsing was successful */
  {
    /* process the Forward_Close service */
    IXSCE_SafetyCloseProc(&s_cnxnTriad, &s_sCloseResp);
  }

  /* if processing the Forward_Close was not successful */
  if (s_sCloseResp.s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
  {
    /* send error response */
    w_retVal = FwdCloseErrorReplySend(&s_cnxnTriad,
                                      &s_sCloseResp,
                                      ps_rxHalcMsg->u32_addInfo);
  }
  else /* else: success */
  {
    /* send success response */
    w_retVal = FwdCloseSuccessReplySend(&s_cnxnTriad,
                                        &s_sCloseResp,
                                        ps_rxHalcMsg->u32_addInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : FwdOpenSuccessReplySend
**
** Description : This function sends a CIP successful response message to HALC
**               after successful processing of a SafetyOpen request.
**
** Parameters  : ps_sOpenPar (IN)  - structure that contains the parameters that
**                                   were received with the SafetyOpen
**                                   (not checked, only called with
**                                   reference to structure variables)
**               ps_sOpenResp (IN) - structure that contains the parameters to
**                                   be returned in the SafetyOpen response
**                                   message
**                                   (not checked, only called with
**                                   reference to structure variables)
**               u32_addInfo (IN)  - this value was received from CSAL and must
**                                   be returned in the response message to CSAL
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while trying to send the response msg
**
*******************************************************************************/
static CSS_t_WORD FwdOpenSuccessReplySend(const CSS_t_SOPEN_PARA
                                             *ps_sOpenPar,
                                          const CSS_t_SOPEN_RESP
                                             *ps_sOpenResp,
                                          CSS_t_UDINT u32_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for passing the success response to HALC */
  HALCS_t_MSG s_halcMsg;
  /* array for passing data of the success response to HALC (size of extended */
  /* format as this is always larger than base format)                        */
  CSS_t_BYTE ab_rspData[k_FWD_OPEN_SUCREP_SIZE_EXT_B];
  /* temporary variable needed for assembling the response message */
  CSS_t_BYTE b_temp = 0U;

  /* completely erase structure */
  CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

  /*
  ** initialize HALC struct fields
  */
  /* HALC command code */
  s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_SOPEN_RES;
  /* the size/format of the response is dependent on Message Format */
  /* if is base format */
  if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
  {
    s_halcMsg.u16_len = k_FWD_OPEN_SUCREP_SIZE_BASE_B;
  }/* CCT_SKIP */ /* not reachable in all Unit Test configurations */
                  /* (unless stubs would return inconsistent states) */
  else /* else: extended format */
  {
    s_halcMsg.u16_len = k_FWD_OPEN_SUCREP_SIZE_EXT_B;
  }/* CCT_SKIP */ /* not reachable in all Unit Test configurations */
                  /* (unless stubs would return inconsistent states) */
  /* echo back the addInfo parameter */
  s_halcMsg.u32_addInfo = u32_addInfo;
  /* link with local buffer to place the response data in it */
  s_halcMsg.pb_data = ab_rspData;


  /*
  ** At the beginning of the HALC message data field place some stack internal
  ** parameters
  */
  /* Safety Validator instance ID */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_SV_INST_ID),
                &ps_sOpenResp->s_appReply.u16_instId);

  /* CSAL needs a Consumer Number in the range of 1..15. For Single-Cast it   */
  /* must be 1. But for CIP the Consumer Number is 0xFFFF in case of          */
  /* Single-Cast. Thus in the HALC message we have to set it to 1 for CSAL.   */
  if (ps_sOpenResp->s_appReply.u16_consNum <= CSOS_k_MAX_CONSUMER_NUM_MCAST)
  {
    b_temp = (CSS_t_USINT)ps_sOpenResp->s_appReply.u16_consNum;
  }
  else /* else: must be single cast */
  {
    b_temp = CSOS_k_MAX_CONSUMER_NUM_SCAST;
  }
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CONS_NUM),
               &b_temp);

  /* Connection Type */
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CNXN_TYPE),
               &ps_sOpenResp->s_appReply.b_cnxnType);

  /* Connection Point */
  /* if consuming connection (only Single-cast is possible on Targets) */
  if (ps_sOpenResp->s_appReply.b_cnxnType == CSOS_k_CNXN_CONS_SINGLE)
  {
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_OPEN_CNXN_POINT),
                  &ps_sOpenPar->u16_cnxnPointCons);
  }
  else
  {
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_OPEN_CNXN_POINT),
                  &ps_sOpenPar->u16_cnxnPointProd);
  }


  /*
  ** Continue in the HALC message data field with the CIP data of the
  ** Forward_Open Success Response
  */

  /*
  ** Message Router Response Format
  */
  /* Reply service code */
  b_temp = CSS_k_SRVC_FORWARD_OPEN | CSOS_k_BIT_REPLY_SERVICE_CODE;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_SERVICE_CODE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_RESERVED),
               &b_temp);
  /* General Status: Success */
  b_temp = CSOS_k_CGSC_SUCCESS;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_GEN_STATUS),
               &b_temp);

  /* Size of Additional Status */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_ADD_STATUS_SIZE),
               &b_temp);
  /* Additional Status size is zero, so no more data following */


  /*
  ** Forward_Open Success Response Data
  */
  /* The Network Connection IDs are not used within CIP Safety on the Target.
     Thus they are not stored. We fill in the same value as received by the
     Originator in the request. In case the Originator has sent a value of
     zero (which means the Target shall allocate the ID) then this has to be
     done afterwards in a lower layer (i.e. CSAL/EtherNet/IP stack). */

  /* O->T Network Connection ID (see FRS174) */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_NCNXNID_OT),
                &ps_sOpenPar->u32_netCnxnId_OT);

  /* T->O Network Connection ID (see FRS174) */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_NCNXNID_TO),
                &ps_sOpenPar->u32_netCnxnId_TO);
  /* Connection Serial Number */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_CNXN_S_NUM),
                &ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum);
  /* Originator Vendor ID */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_ORIG_V_ID),
                &ps_sOpenPar->s_cnxnTriad.u16_origVendId);
  /* Originator Serial Number */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_ORIG_S_NUM),
                &ps_sOpenPar->s_cnxnTriad.u32_origSerNum);
  /* O->T API (for Safety Connections this always equals the RPI, see FRS345) */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_API_OT),
                &ps_sOpenPar->u32_rpiOT_us);
  /* T->O API (for Safety Connections this always equals the RPI, see FRS345) */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_SFOR_API_TO),
                &ps_sOpenPar->u32_rpiTO_us);
  /* Application Reply Size */
  /* set for Base Format, will be corrected later if we have Extended Format */
  b_temp = k_FWD_OPEN_SUCREP_APP_SIZE_BASE_W;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_SFOR_APP_REPLY_SIZE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_SFOR_RESERVED),
               &b_temp);


  /*
  ** Application Reply
  */
  /* Consumer_Number (encoded as UINT) */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_APPR_CONS_NUM),
                &ps_sOpenResp->s_appReply.u16_consNum);
  /* PID/CID: Target's Vendor ID */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_APPR_PCID_TARG_V_ID),
                &ps_sOpenResp->s_appReply.u16_trgVendId);
  /* PID/CID: Target's Device Serial Number */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_APPR_PCID_TARG_DEV_S_NUM),
                &ps_sOpenResp->s_appReply.u32_trgDevSerNum);
  /* Target Connection Serial Number */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_OPEN_MRR_RESP_DATA
                             + k_OFS_APPR_PCID_TARG_CNXN_S_NUM),
                &ps_sOpenResp->s_appReply.u16_trgCnxnSerNum);

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* if is extended format */
  if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
  {
    /* Additional fields of the 14-byte Extended Format SafetyOpen Target */
    /* Application reply (see FRS363) */

    /* Initial Time Stamp */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                 CSOS_k_OFS_OPEN_MRR_RESP_DATA
                               + k_OFS_APPR_INIT_TIME_STAMP),
                &ps_sOpenResp->s_appReply.u16_initialTS);
    /* Initial Rollover Value */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                 CSOS_k_OFS_OPEN_MRR_RESP_DATA
                               + k_OFS_APPR_INIT_ROLLOVER_VAL),
                &ps_sOpenResp->s_appReply.u16_initialRV);

    /* correct the Application reply size */
    b_temp = k_FWD_OPEN_SUCREP_APP_SIZE_EXT_W;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                CSOS_k_OFS_OPEN_MRR_RESP_DATA
                              + k_OFS_SFOR_APP_REPLY_SIZE),
                &b_temp);
  }
  else /* else: base format */
  {
    /* Target Format has no additional fields */
  }

#endif

  /* if sending the message to CSAL succeeded */
  if (HALCS_TxDataPut(&s_halcMsg))
  {
    /* success */
    w_retVal = CSS_k_OK;
  }
  else /* else: failed */
  {
    /* error while trying to send the message to CSAL */
    w_retVal = (IXSCE_k_NFSE_TXH_O_SUCRSP_SEND);
    /* signal error to Safety Application */
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, u32_addInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : FwdOpenErrorReplySend
**
** Description : This function sends a CIP unsuccessful response message to HALC
**               after detection of an error while processing a SafetyOpen
**               request.
**
** Parameters  : ps_sOpenPar (IN)      - structure that contains the parameters
**                                       that were received with the SafetyOpen
**                                       (not checked, only called with
**                                       reference to structure variables)
**               u32_addInfo (IN)      - this value was received from CSAL and
**                                       must be returned in the response
**                                       message to CSAL
**                                       (not checked, any value allowed)
**               ps_sOpenRespStat (IN) - structure that contains the Status
**                                       Code(s) to be returned (General +
**                                       Extended) 
**                                       (not checked, only called with
**                                       reference to structure variables)
**
** Returnvalue : CSS_k_OK              - success
**               <>CSS_k_OK            - error while trying to send the response
**                                       message
**
*******************************************************************************/
static CSS_t_WORD FwdOpenErrorReplySend(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                        CSS_t_UDINT u32_addInfo,
                                   const CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for passing the unsuccessful response to HALC */
  HALCS_t_MSG s_halcMsg;
  /* array for passing data of the unsuccessful response to HALC */
  CSS_t_BYTE ab_rspData[k_FWD_OPEN_ERRREP_SIZE_B];
  /* temporary variables needed for assembling the response message */
  CSS_t_BYTE b_temp = 0U;
  CSS_t_WORD w_temp = 0U;
  /* offset counter for assembling the response message */
  CSS_t_USINT u8_offset;
  /* loop counter for copying extended status words */
  CSS_t_USINT u8_idx;

  /* completely erase structure */
  CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

  /*
  ** initialize HALC struct fields
  */
  s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_SOPEN_RES; /* HALC command code */
  /* HALC message length will be set at the end */
  /* echo back the addInfo parameter */
  s_halcMsg.u32_addInfo = u32_addInfo;
  /* link with local buffer to place the response data in it */
  s_halcMsg.pb_data = ab_rspData;


  /*
  ** At the beginning of the HALC message data field place some stack internal
  ** parameters
  */
  /* Safety Validator instance ID (invalid) */
  w_temp = CSOS_k_INVALID_INSTANCE;
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_SV_INST_ID),
                &w_temp);
  /* Consumer number (invalid) */
  b_temp = 0xFFU;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CONS_NUM),
               &b_temp);
  /* Connection Type (invalid) */
  b_temp = 0xFFU;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CNXN_TYPE),
               &b_temp);
  /* Connection Point - invalid */
  w_temp = CSOS_k_INVALID_INSTANCE;
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_OPEN_CNXN_POINT),
                &w_temp);

  /*
  ** Continue in the HALC message data field with the CIP data of the
  ** Forward_Open Error Response
  */

  /*
  ** Message Router Response Format
  */
  /* Reply service code */
  b_temp = CSS_k_SRVC_FORWARD_OPEN | CSOS_k_BIT_REPLY_SERVICE_CODE;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_SERVICE_CODE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_RESERVED),
               &b_temp);
  /* General Status */
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_OPEN_MRR_RESP_DATA
                            + k_OFS_MRR_GEN_STATUS),
               &ps_sOpenRespStat->u8_gen);

  /* if this error has an Additional Status WORD */
  if (ps_sOpenRespStat->u16_ext != CSS_k_CMEXTSTAT_UNDEFINED)
  {
    /* Size of Additional Status: will be copied into the response buffer later
       (and in case there are more additional status words this will be added
       later) */
    b_temp = k_ERRREP_ADDSTAT_SIZE;

    /* first extended Status Word: Extended Status Code */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                 CSOS_k_OFS_OPEN_MRR_RESP_DATA
                               + k_OFS_MRR_ADD_STATUS),
                  &ps_sOpenRespStat->u16_ext);
    /* calculate the offset for the next field (in case there are more
       additional status words this will be added later) */
    u8_offset = CSOS_k_OFS_OPEN_MRR_RESP_DATA + k_OFS_MRR_ADD_STATUS
              + CSOS_k_SIZEOF_WORD;
    /* if this Extended Status Code is expected to consist of one word only */
    if (!ExtStatCodeHasMoreWords(ps_sOpenRespStat->u8_gen,
                                 ps_sOpenRespStat->u16_ext))
    {
      /* Extended status is now already complete */
    }
    else  /* else: we need to copy more additional status words */
    {
      /* if number of more extended status words is invalid */
      if (ps_sOpenRespStat->u8_numMoreExt > CSS_k_MAX_NUM_EXT_STATUS_WORDS)
      {
        /* Message Router Response structure is inconsistent */
        SAPL_CssErrorClbk(IXSCE_k_FSE_INC_MR_RESP_FWD_OP, IXSER_k_I_NOT_USED,
                          IXSER_k_A_NOT_USED);
      }
      else  /* else: number of more extended status words is ok */
      {
        b_temp = k_ERRREP_ADDSTAT_SIZE + ps_sOpenRespStat->u8_numMoreExt;

        /* copy more extended status words */
        for (u8_idx = 0U; u8_idx < ps_sOpenRespStat->u8_numMoreExt; u8_idx++)
        {
          CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                       CSOS_k_OFS_OPEN_MRR_RESP_DATA
                                     + k_OFS_MRR_ADD_STATUS_MORE
                                     + (CSOS_k_SIZEOF_WORD * u8_idx)),
                        &ps_sOpenRespStat->au16_moreExt[u8_idx]);
        }
        /* add the additional extended status words to the offset for the
           next field */
        u8_offset = u8_offset + (CSS_t_USINT)(ps_sOpenRespStat->u8_numMoreExt
                                              * CSOS_k_SIZEOF_WORD);
      }
    }

    /* now write the actual number of additional status words into the
       response buffer */
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                            CSOS_k_OFS_OPEN_MRR_RESP_DATA
                          + k_OFS_MRR_ADD_STATUS_SIZE),
                 &b_temp);
  }
  else  /* else: no Additional Status */
  {
    /* Size of Additional Status: 0 */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                CSOS_k_OFS_OPEN_MRR_RESP_DATA
                              + k_OFS_MRR_ADD_STATUS_SIZE),
                 &b_temp);
    /* calculate the offset for the next field */
    u8_offset = CSOS_k_OFS_OPEN_MRR_RESP_DATA + k_OFS_MRR_ADD_STATUS_SIZE
              + CSOS_k_SIZEOF_USINT;
  }

  /* if parsing the connection triad of the request was not successful */
  if (    (ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum == 0U)
       && (ps_sOpenPar->s_cnxnTriad.u16_origVendId == 0U)
       && (ps_sOpenPar->s_cnxnTriad.u32_origSerNum == 0UL)
     )
  {
    /* just send a Message Router Response only (already completed) */
  }
  else  /* else: we have a connection triad */
  {
    /*
    ** Forward_Open Error Response Data
    */
    /* Connection Serial Number */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum);
    u8_offset += CSOS_k_SIZEOF_UINT;
    /* Originator Vendor ID */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_sOpenPar->s_cnxnTriad.u16_origVendId);
    u8_offset += CSOS_k_SIZEOF_UINT;
    /* Originator Serial Number */
    CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_sOpenPar->s_cnxnTriad.u32_origSerNum);
    u8_offset += CSOS_k_SIZEOF_UDINT;

    /* Remaining Path Size */
    /* According to Volume 1 the "Remaining Path Size" field is required only if
       the error was detected by a router (optional if detected by the target).
       But even more than 5 years after this has been clarified in the CIP
       Networks Library implementations in the Conformance Test, Wireshark and
       others still can't handle responses without this field. Thus send the
       remaining path size field with zero values. */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                 &b_temp);
    u8_offset += CSOS_k_SIZEOF_USINT;
    /* Reserved */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                 &b_temp);
    u8_offset += CSOS_k_SIZEOF_USINT;
  }

  /* now finally, set the length of the HALC message */
  s_halcMsg.u16_len = (CSS_t_UINT)u8_offset;    /* data length */

  /* if sending the message to CSAL succeeded */
  if (HALCS_TxDataPut(&s_halcMsg))
  {
    /* success */
    w_retVal = CSS_k_OK;
  }
  else /* else: failed */
  {
    /* error while trying to send the message to CSAL */
    w_retVal = (IXSCE_k_NFSE_TXH_O_ERRRSP_SEND);
    /* signal error to Safety Application */
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, u32_addInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : FwdCloseSuccessReplySend
**
** Description : This function sends a CIP successful response message to HALC
**               after successful processing of a Safety Close request.
**
** Parameters  : ps_cnxnTriad (IN)  - structure that contains the relevant
**                                    parameters that were received with the
**                                    Forward_Close
**                                    (not checked, only called with
**                                    reference to structure variables)
**               ps_sCloseResp (IN) - structure that contains the Status Codes
**                                    to be returned and additional information
**                                    for CSAL
**                                    (not checked, only called with
**                                    reference to structure variables)
**               u32_addInfo (IN)   - this value was received from CSAL and must
**                                    be returned in the response message to
**                                    CSAL
**                                    (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK           - success
**               <>CSS_k_OK         - error while trying to send the response
**                                    msg
**
*******************************************************************************/
static CSS_t_WORD FwdCloseSuccessReplySend(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                           const IXSCE_t_SCLOSE_RESP
                                              *ps_sCloseResp,
                                           CSS_t_UDINT u32_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for passing the success response to HALC */
  HALCS_t_MSG s_halcMsg;
  /* array for passing data of the success response to HALC */
  CSS_t_BYTE ab_rspData[k_FWD_CLOSE_SUCREP_SIZE_B];
  /* temporary variable needed for assembling the response message */
  CSS_t_BYTE b_temp = 0U;

  /* completely erase structure */
  CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

  /*
  ** initialize HALC struct fields
  */
  s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_SCLOSE_RES;  /* HALC command code */
  s_halcMsg.u16_len = k_FWD_CLOSE_SUCREP_SIZE_B;    /* data length */
  /* echo back the addInfo parameter */
  s_halcMsg.u32_addInfo = u32_addInfo;
  /* link with local buffer to place the response data in it */
  s_halcMsg.pb_data = ab_rspData;


  /*
  ** At the beginning of the HALC message data field place some stack internal
  ** parameters
  */
  /* Safety Validator instance ID */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_SV_INST_ID),
                &ps_sCloseResp->u16_instId);
  /* Consumer number */
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CONS_NUM),
               &ps_sCloseResp->u8_consNum);
  /* Pad byte (0) */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CNXN_TYPE),
               &b_temp);


  /*
  ** Continue in the HALC message data field with the CIP data of the
  ** Forward_Close Success Response
  */

  /*
  ** Message Router Response Format
  */
  /* Reply service code */
  b_temp = CSS_k_SRVC_FORWARD_CLOSE | CSOS_k_BIT_REPLY_SERVICE_CODE;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_SERVICE_CODE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_RESERVED),
               &b_temp);
  /* General Status: Success */
  b_temp = CSOS_k_CGSC_SUCCESS;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_GEN_STATUS),
               &b_temp);
  /* Size of Additional Status */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_ADD_STATUS_SIZE),
               &b_temp);
  /* Additional Status size is zero, so no more data following */


  /*
  ** Forward_Close Success Response Data
  */
  /* Connection Serial Number */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                             + k_OFS_SFCR_CNXN_S_NUM),
                &ps_cnxnTriad->u16_cnxnSerNum);
  /* Originator Vendor ID */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                             + k_OFS_SFCR_ORIG_V_ID),
                &ps_cnxnTriad->u16_origVendId);
  /* Originator Serial Number */
  CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                               CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                             + k_OFS_SFCR_ORIG_S_NUM),
                &ps_cnxnTriad->u32_origSerNum);
  /* Application Reply Size */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_SFCR_APP_REPLY_SIZE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_SFCR_RESERVED),
               &b_temp);

  /* if sending the message to CSAL succeeded */
  if (HALCS_TxDataPut(&s_halcMsg))
  {
    /* success */
    w_retVal = CSS_k_OK;
  }
  else /* else: failed */
  {
    /* error while trying to send the message to CSAL */
    w_retVal = (IXSCE_k_NFSE_TXH_C_SUCRSP_SEND);
    /* signal error to Safety Application */
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, u32_addInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : FwdCloseErrorReplySend
**
** Description : This function sends a CIP unsuccessful response message to HALC
**               after detection of an error while processing a Safety Close
**               request.
**
** Parameters  : ps_cnxnTriad (IN)  - structure that contains the parameters
**                                    that were received with the Forward_Close
**                                    (not checked, only called with
**                                    reference to structure variables)
**               ps_sCloseResp (IN) - structure that contains the Status Codes
**                                    to be returned
**                                    (not checked, only called with
**                                    reference to structure variables)
**               u32_addInfo (IN)   - this value was received from CSAL and must
**                                    be returned in the response message to
**                                    CSAL
**                                    (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK           - success
**               <>CSS_k_OK         - error while trying to send the response
**                                    msg
**
*******************************************************************************/
static CSS_t_WORD FwdCloseErrorReplySend(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                         const IXSCE_t_SCLOSE_RESP
                                            *ps_sCloseResp,
                                         CSS_t_UDINT u32_addInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* structure for passing the unsuccessful response to HALC */
  HALCS_t_MSG s_halcMsg;
  /* array for passing data of the unsuccessful response to HALC */
  CSS_t_BYTE ab_rspData[k_FWD_CLOSE_ERRREP_SIZE_B];
  /* temporary variable needed for assembling the response message */
  CSS_t_BYTE b_temp = 0U;
  CSS_t_WORD w_temp = 0U;
  /* offset counter for assembling the response message */
  CSS_t_USINT u8_offset;

  /* completely erase structure */
  CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

  /*
  ** initialize HALC struct fields
  */
  s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_SCLOSE_RES;  /* HALC command code */
  /* HALC message length will be set at the end */
  /* echo back the addInfo parameter */
  s_halcMsg.u32_addInfo = u32_addInfo;
  /* link with local buffer to place the response data in it */
  s_halcMsg.pb_data = ab_rspData;


  /*
  ** At the beginning of the HALC message data field place some stack internal
  ** parameters
  */
  /* Safety Validator instance ID (invalid) */
  w_temp = CSOS_k_INVALID_INSTANCE;
  CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_SV_INST_ID),
                &w_temp);
  /* Consumer number (invalid) */
  b_temp = 0xFFU;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CONS_NUM),
               &b_temp);
  /* Pad byte (0) */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_OFS_CNXN_TYPE),
               &b_temp);


  /*
  ** Continue in the HALC message data field with the CIP data of the
  ** Forward_Close Error Response
  */

  /*
  ** Message Router Response Format
  */
  /* Reply service code */
  b_temp = CSS_k_SRVC_FORWARD_CLOSE | CSOS_k_BIT_REPLY_SERVICE_CODE;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_SERVICE_CODE),
               &b_temp);
  /* Reserved */
  b_temp = 0U;
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_RESERVED),
               &b_temp);
  /* General Status: function parameter */
  CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                              CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                            + k_OFS_MRR_GEN_STATUS),
               &ps_sCloseResp->s_status.u8_gen);

  /* if this error has an Additional Status WORD */
  if (ps_sCloseResp->s_status.u16_ext != CSS_k_CMEXTSTAT_UNDEFINED)
  {
    /* Size of Additional Status: 1 Word */
    b_temp = k_ERRREP_ADDSTAT_SIZE;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                              + k_OFS_MRR_ADD_STATUS_SIZE),
                 &b_temp);
    /* extended Status Word: function parameter */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                 CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                               + k_OFS_MRR_ADD_STATUS),
                  &ps_sCloseResp->s_status.u16_ext);
    /* calculate the offset for the next field */
    u8_offset = CSOS_k_OFS_CLOSE_MRR_RESP_DATA + k_OFS_MRR_ADD_STATUS
              + CSOS_k_SIZEOF_WORD;
  }
  else  /* else: no Additional Status */
  {
    /* Size of Additional Status: 0 */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data,
                                CSOS_k_OFS_CLOSE_MRR_RESP_DATA
                              + k_OFS_MRR_ADD_STATUS_SIZE),
                 &b_temp);
    /* calculate the offset for the next field */
    u8_offset = CSOS_k_OFS_CLOSE_MRR_RESP_DATA + k_OFS_MRR_ADD_STATUS_SIZE
              + CSOS_k_SIZEOF_USINT;
  }

  /* if parsing the connection triad of the request was not successful */
  if (    (ps_cnxnTriad->u16_cnxnSerNum == 0U)
       && (ps_cnxnTriad->u16_origVendId == 0U)
       && (ps_cnxnTriad->u32_origSerNum == 0UL)
     )
  {
    /* just send a Message Router Response only (already completed) */
  }
  else  /* else: we have a connection triad */
  {
    /*
    ** Forward_Close Error Response Data
    */
    /* Connection Serial Number */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_cnxnTriad->u16_cnxnSerNum);
    u8_offset += CSOS_k_SIZEOF_UINT;
    /* Originator Vendor ID */
    CSS_H2N_CPY16(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_cnxnTriad->u16_origVendId);
    u8_offset += CSOS_k_SIZEOF_UINT;
    /* Originator Serial Number */
    CSS_H2N_CPY32(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                  &ps_cnxnTriad->u32_origSerNum);
    u8_offset += CSOS_k_SIZEOF_UDINT;

    /* Remaining Path Size */
    /* According to Volume 1 the "Remaining Path Size" field is required only if
       the error was detected by a router (optional if detected by the target).
       But even more than 5 years after this has been clarified in the CIP
       Networks Library implementations in the Conformance Test, Wireshark and
       others still can't handle responses without this field. Thus send the
       remaining path size field with zero values. */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                 &b_temp);
    u8_offset += CSOS_k_SIZEOF_USINT;
    /* Reserved */
    b_temp = 0U;
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, u8_offset),
                 &b_temp);
    u8_offset += CSOS_k_SIZEOF_USINT;
  }

  /* now finally, set the length of the HALC message */
  s_halcMsg.u16_len = (CSS_t_UINT)u8_offset;    /* data length */

  /* if sending the message to CSAL succeeded */
  if (HALCS_TxDataPut(&s_halcMsg))
  {
    /* success */
    w_retVal = CSS_k_OK;
  }
  else /* else: failed */
  {
    /* error while trying to send the message to CSAL */
    w_retVal = (IXSCE_k_NFSE_TXH_C_ERRRSP_SEND);
    /* signal error to Safety Application */
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, u32_addInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ExtStatCodeHasMoreWords
**
** Description : This function checks the passed Extended Status Code according
**               to the Connection Manager Object Instance Errors.
**
** Parameters  : u8_genStat (IN)      - General Status Code value
**                                      (not checked, any value allowed)
**               u16_extStatCode (IN) - Extended Status Code value (first word)
**                                      (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE           - This code defines more than one Extended
**                                      Status Word
**               CSS_k_FALSE          - This code consists of only one Extended
**                                      Status Word
**
*******************************************************************************/
static CSS_t_BOOL ExtStatCodeHasMoreWords(CSS_t_USINT u8_genStat,
                                          CSS_t_UINT u16_extStatCode)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if this is a "Communication Related Problem" */
  if (u8_genStat == CSOS_k_CGSC_CNXN_FAILURE)
  {
    /* if this is one of the Extended Status Code that defines additional
       Extended Status Words */
    if (u16_extStatCode == CSS_k_CMEXTSTAT_RPI_NOT_ACCPT)
    {
      o_retVal = CSS_k_TRUE;
    }
    else  /* else: any other Extended Status Code */
    {
      /* return value already set */
    }
  }
  else  /* else: any other General Status Code */
  {
    /* return value already set */
  }

  return (o_retVal);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*** End Of File ***/


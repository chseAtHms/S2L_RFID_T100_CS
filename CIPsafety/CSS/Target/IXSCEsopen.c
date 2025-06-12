/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEsopen.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module handles a received SafetyOpen by doing all necessary
**             checks.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_SafetyOpenProc
**
**             TunidMatchCheck
**             CnxnSizeCheck
**             CnxnParamsValidateServer
**             CnxnParamsValidateClient
**             CnxnParamsValidate
**             RpiCheck
**             CnxnParamsValidateSafety1
**             CnxnParamsValidateSafety2
**             AppPathVerify
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
#include "IXSSS.h"
#include "IXSAIapi.h"
#include "IXSAI.h"
#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXSVO.h"
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

/* maximum value of Connection Timeout Multiplier */
#define k_MAX_CNXN_TO_MULT   7U

/* converting seconds to micro seconds */
#define k_SEC_TO_USEC(u32)  ((u32) * 1000000U)

/* Time Correction Connection ID is only relevant for DeviceNet.
   Thus here not used */
#define k_TIME_CORR_CNXN_ID_NOT_USED     0xFFFFFFFFU
/* Initial Time Stamp and Rollover Value are only relevant for producers. Thus
   for consumers it can be checked for the "not used" default value */
/*lint -esym(750, k_INITIAL_TIME_STAMP_NOT_USED)  not referenced in every cfg */
/*lint -esym(750, k_INITIAL_ROLLOVER_NOT_USED)    not referenced in every cfg */
#define k_INITIAL_TIME_STAMP_NOT_USED  0xFFFFU
#define k_INITIAL_ROLLOVER_NOT_USED    0xFFFFU

/* maximum value for Max_Fault_Number */
#define k_MAX_MAX_FAULT_NUM  255U


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void TunidMatchCheck(const CSS_t_UNID *ps_tunid,
                            CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void CnxnSizeCheck(CSS_t_UINT u16_reqCnxnSize,
                          CSS_t_BOOL o_multiCast,
                          CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                          CSS_t_USINT *pu8_payloadSize);
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  static void CnxnParamsValidateServer(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                       CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                                       CSS_t_USINT *pu8_payloadSize);
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  static void CnxnParamsValidateClient(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                       CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                                       CSS_t_USINT *pu8_payloadSize);
#endif
static void CnxnParamsValidate(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                               CSS_t_USINT *pu8_payloadSize);
static void RpiCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                     CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void CnxnParamsValidateSafety1(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void CnxnParamsValidateSafety2(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void AppPathVerify(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                          CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_SafetyOpenProc
**
** Description : This function processes a SafetyOpen request. It checks all
**               parameters and determines if it can open the connection or not.
**
** Parameters  : ps_sOpenPar (IN)     - pointer to the structure which
**                                      contains the data that were received
**                                      in the Forward_Open request.
**                                      (not checked, only called with
**                                      reference to structure variable)
**               ps_sOpenAuxData (IN) - Auxiliary data that have been generated
**                                      when parsing the Forward_Open Request.
**                                      This information is for CSS internal
**                                      use only and will not be passed to the
**                                      application.
**                                      (not checked, only called with
**                                      reference to structure variable)
**               ps_sOpenResp (OUT)   - pointer to structure for returning the
**                                      parameters for a Forward_Open response
**                                      (not checked, only called with
**                                      reference to structure variable)
**
** Returnvalue : CSS_k_OK             - success
**               <>CSS_k_OK           - error
**
*******************************************************************************/
CSS_t_WORD IXSCE_SafetyOpenProc(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                const IXSCE_t_SOPEN_AUX *ps_sOpenAuxData,
                                CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* length of the payload of the Safety I/O data */
  CSS_t_USINT u8_payloadSize = 0U;
  /* In case the connection to be opened already exists this struct stores    */
  /* the Instance ID and Consumer Number of it.                               */
  CSS_t_SV_ID s_svInfo;

  /* completely erase structure */
  CSS_MEMSET(&s_svInfo, 0, sizeof(s_svInfo));

  /* This function and its auxiliary function IXSCE_SafetyOpenProc2() will    */
  /* perform the following checks:                                            */
  /* - for configured Input Targets receiving Type 1 SO according to SRS11    */
  /* - for configured Output Targets receiving Type 1 SO according to SRS12   */
  /* - for configured Targets receiving Type 2a or 2b SO according to SRS180  */

  /*
  ** First and most important check: Safety Open Message corrupted?
  ** (see SRS11 bullet 5 and SRS12 bullet 6 and SRS180 bullet 5)
  */
  /* if calculated CPCRC differs from CPCRC contained in request message */
  if (ps_sOpenAuxData->u32_cpcrcCalc != ps_sOpenPar->s_nsd.u32_cpcrc)
  {
    ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_CPCRC;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CPCRC_MIS),
                      IXSER_k_I_NOT_USED, ps_sOpenAuxData->u32_cpcrcCalc);
    w_retVal = CSS_k_OK;
  }
  else /* else: CPCRC matches */
  {
    /* Check if User or System has misrouted the connection request */
    /* (see SRS11 bullet 1 and SRS12 bullet 2 and SRS180 bullet 3) */
    TunidMatchCheck(&ps_sOpenPar->s_nsd.s_tunid, &ps_sOpenResp->s_status);

    /* if TUNID mismatch */
    if (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
    {
      /* Found an error. Do not continue processing */
      /* CIP Error Code already set -> an error response will be returned */
      w_retVal = CSS_k_OK;
    }
    else /* else: TUNID matches */
    {
      /* Check if there is already a connection with matching parameters:     */
      /* Could be a Duplicate Forward_Open or a Safety Validator that has a   */
      /* matching Consumer/Producer with Consuming/Producing Connection       */
      /* Status "Faulted" and Originator wants to reopen that connection.     */
      CSS_t_BOOL o_cnxnAlreadyExists = IXSVO_CnxnFind(&ps_sOpenPar->s_cnxnTriad,
                                                      &s_svInfo);

      /* if an existing connection with this requested Triad was found */
      if (o_cnxnAlreadyExists)
      {
        /* if the found existing connection is a Server */
        if (s_svInfo.o_isServer)
        {
          /* if the requested connection is a Client */
          if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
          {
            /* Error: There must be a significant change in the Originator
               configuration. We cannot accept this connection while the
               previous one was not closed. */
            ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
            ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
            SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SRV_EXISTS),
                              s_svInfo.u16_instId, IXSER_k_A_NOT_USED);
            w_retVal = CSS_k_OK;
          }
          else  /* else: requested connection is also a Server */
          {
            /* Ok, existing Safety Validator Server instance can be overwritten
               with the received parameters. In case b_transportTypeTrigger has
               some invalid value then this will be detected by later checks. */
          }
        }
        else  /* else: the found existing connection is a Client */
        {
          /* if the requested connection is a Server */
          if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_SERVER_APP_CL0)
          {
            /* Error: There must be a significant change in the Originator
               configuration. We cannot accept this connection while the
               previous one was not closed. */
            ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
            ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
            SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CLI_EXISTS),
                              s_svInfo.u16_instId, IXSER_k_A_NOT_USED);
            w_retVal = CSS_k_OK;
          }
          else  /* else: requested connection is also a Client */
          {
            /* Ok, existing Safety Validator Client instance can be overwritten
               with the received parameters. In case b_transportTypeTrigger has
               some invalid value then this will be detected by later checks. */
          }
        }
      }
      else  /* no matching connection found */
      {
        /* nothing to do here */
      }

      /* if an error was already found */
      if (w_retVal != IXSCE_k_FSE_INC_PRG_FLOW)
      {
        /* don't continue the SafetyOpen processing */
      }
      /* else: if we received a duplicate Forward_Open */
      else  if (    (o_cnxnAlreadyExists)
                 && (s_svInfo.u8_cnxnStatus != CSS_k_CNXN_STATUS_FAULTED)
               )
      {
        ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_DUP_FWD_OPEN;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_DUPLICATE),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        w_retVal = CSS_k_OK;
      }
      else /* else: this is not a duplicate Forward_Open */
      {
        /* Check if the request inside this safety network has reached the    */
        /* intended target                                                    */
        /* (see SRS11 bullet 3 and see SRS12 bullet 3 and SRS180 bullet 4)    */
        /* if Electronic Key mismatch */
        if (!IXSCE_ElectronicKeyMatchCheck(&ps_sOpenPar->s_elKey,
                                           &ps_sOpenResp->s_status.u8_gen,
                                           &ps_sOpenResp->s_status.u16_ext))
        {
          /* Found an error. Do not continue processing */
          /* general and extended status already set */
          w_retVal = CSS_k_OK;
        }
        else /* else: Electronic Key matches */
        {
          /* check if connection with these parameters can be served */
          CnxnParamsValidate(ps_sOpenPar, &ps_sOpenResp->s_status,
                             &u8_payloadSize);

          /* if Connection Parameters are invalid */
          if (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
          {
            /* Found an error. Do not continue processing */
            w_retVal = CSS_k_OK;
          }
          else /* Connection Parameters are valid */
          {
            /* Check if Input/Output Connection point exists */
            /* (see SRS11 bullet 9 and SRS12 bullet 9 and SRS180 bullet 7) */
            AppPathVerify(ps_sOpenPar, &ps_sOpenResp->s_status);

            /* if Application Path invalid */
            if (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
            {
              /* Found an error. Do not continue processing */
              /* CIP Error Code already set */
              /* -> an error response will be returned */
              w_retVal = CSS_k_OK;
            }
            else /* else: Application Path is ok */
            {
              /* To keep the complexity of this function low we continue the  */
              /* opening process in another function.                         */
              w_retVal = IXSCE_SafetyOpenProc2(ps_sOpenPar, &s_svInfo,
                                               u8_payloadSize, ps_sOpenResp);
            }
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : TunidMatchCheck
**
** Description : This function checks if the TUNID in received Forward_Open
**               parameters matches ours.
**
** Parameters  : ps_tunid (IN)          - pointer to the TUNID of the received
**                                        Forward_Open parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void TunidMatchCheck(const CSS_t_UNID *ps_tunid,
                            CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* will point to our TUNID(s) array */
  const CSS_t_UNID *pas_ourTunid;

  /* Get our TUNID(s) */
  pas_ourTunid = IXSSO_TunidArrayPtrGet();

/* if we are a Multiple Safety Port Device */
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
  /* This check is not required for Single Safety Port Devices because there
     the checking for FF TUNID within IXSSO_DeviceHasValidTunid() is
     sufficient. Multiple Safety Port devices may have TUNIDs not for all ports
     set. In this case a received FF TUNID would accidentally match our TUNID
     list (pas_ourTunid). */
  /* if received TUNID is an FF TUNID */
  if (IXSSO_UnidCompare(&IXSSO_ks_ffUnid, ps_tunid))
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TUNID_MISM;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_TUNID_RX_FF),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else
#endif
  {
    /* if not yet a TUNID assigned to our device */
    if (!IXSSO_DeviceHasValidTunid(pas_ourTunid))
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TUNID_NOT_SET;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_NO_TUNID),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* if our TUNID is valid */
    {
      /* if received TUNID matches (one of) our device's TUNID (see FRS164) */
      if (IXSSO_UnidListCompare(ps_tunid, pas_ourTunid))
      {
        /* ok, TUNID matches */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
      else /* else we received a Forward_Open with a wrong TUNID */
      {
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TUNID_MISM;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_TUNID_MIS),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : CnxnSizeCheck
**
** Description : This function checks if the requested Connection Size received
**               in the Network Connection Parameters can be serviced by this
**               device.
**
** Parameters  : u16_reqCnxnSize (IN)   - Received cnxn size. Includes safety
**                                        data and safety overhead.
**                                        (valid range: CSOS_k_IO_MSGLEN_TCOO..
**                                        CSOS_k_IO_MSGLEN_LONG_MAX, checked)
**               o_multiCast (IN)       - CSS_k_TRUE in case of a multi-cast
**                                        cnxn
**                                        CSS_k_FALSE in case of a single-cast
**                                        cnxn
**                                        (not checked, any value allowed)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu8_payloadSize (OUT)  - calculated payload size of the cnxn
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnSizeCheck(CSS_t_UINT u16_reqCnxnSize,
                          CSS_t_BOOL o_multiCast,
                          CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                          CSS_t_USINT *pu8_payloadSize)
{
  /* make sure we return a defined result */
  *pu8_payloadSize = 0U;

  /* if it is a multicast connection */
  if (o_multiCast)
  {
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    /* Multicast Producer must additionally send Time Correction */
    /* if length is shorter than message length of a Time Correction Message */
    if (u16_reqCnxnSize < CSOS_k_IO_MSGLEN_TCORR)
    {
      /* much too short */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCNXN_SIZE;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_MCAST_LEN),
                        IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_reqCnxnSize);
    }
    else /* else: length is ok (for now, further checks will follow) */
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
    }
  #else
    {
      SAPL_CssErrorClbk((IXSCE_k_FSE_INC_CNXN_SIZE_CHECK),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: it is a singlecast connection */
  {
    /* Single-cast */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
  }

  /* if previous checks already discovered an error */
  if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
  {
    /* don't continue */
  }
  else /* else: no error */
  {
    /* calculate payload size from connection size */
    *pu8_payloadSize = IXSCE_MsgLenToPayloadLen(u16_reqCnxnSize, o_multiCast);

    /* if an invalid payload size has been detected */
    if (*pu8_payloadSize == 0U)
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCNXN_SIZE;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_LEN),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: payload size is valid */
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
/*******************************************************************************
**
** Function    : CnxnParamsValidateServer
**
** Description : This function is a sub function of CnxnParamsValidate().
**               It checks checks various connection parameters if they contain
**               valid values.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu8_payloadSize (OUT)  - calculated payload size of the cnxn
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnParamsValidateServer(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                     CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                                     CSS_t_USINT *pu8_payloadSize)
{
  /* copy of the OT Network Connection Parameters - size field blanked out */
  CSS_t_WORD w_netCnxnParOtNoSize = ps_sOpenPar->w_netCnxnPar_OT
                               & (CSS_t_WORD)(~(CSS_t_WORD)CSS_k_NCP_CNXN_SIZE);
  /* make sure we return a defined result */
  *pu8_payloadSize = 0U;

  /* if O_to_T Network_Connection_Parameters invalid */
  if (    (w_netCnxnParOtNoSize != CSS_k_NCP_PTP_HP_FIX)
       && (w_netCnxnParOtNoSize != CSS_k_NCP_PTP_SP_FIX)
     )
  {
    /* Invalid Network Connection Parameters */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NET_CNXN_PAR;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SS_INVNCP),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if T_to_O Network_Connection_Parameters invalid */
  else if (    (ps_sOpenPar->w_netCnxnPar_TO != CSS_k_NCP_PTP_HP_FIX_TMSG_LEN)
            && (ps_sOpenPar->w_netCnxnPar_TO != CSS_k_NCP_PTP_SP_FIX_TMSG_LEN)
          )
  {
    /* Invalid Network Connection Parameters */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NET_CNXN_PAR;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SS_INVNCPTO),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if Time Correction Network_Connection_Parameters are invalid */
  else if ((ps_sOpenPar->s_nsd.w_tcorrNetCnxnPar) != CSS_k_NCP_NOT_USED)
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCCP;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCNCP1),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: all Network_Connection Parameters are valid */
  {
    /* Check Connection Size */
    CnxnSizeCheck((ps_sOpenPar->w_netCnxnPar_OT & CSS_k_NCP_CNXN_SIZE),
                  CSS_k_FALSE, ps_sOpenRespStat, pu8_payloadSize);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : CnxnParamsValidateClient
**
** Description : This function is a sub function of CnxnParamsValidate().
**               It checks checks various connection parameters if they contain
**               valid values.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu8_payloadSize (OUT)  - calculated payload size of the cnxn
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnParamsValidateClient(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                     CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                                     CSS_t_USINT *pu8_payloadSize)
{
  /* copy of the TO Network Connection Parameters - size field blanked out */
  CSS_t_WORD w_netCnxnParToNoSize = ps_sOpenPar->w_netCnxnPar_TO
                               & (CSS_t_WORD)(~(CSS_t_WORD)CSS_k_NCP_CNXN_SIZE);
  /* make sure we return a defined result */
  *pu8_payloadSize = 0U;

  /* if O_to_T Network_Connection_Parameters invalid */
  if (    (ps_sOpenPar->w_netCnxnPar_OT != CSS_k_NCP_PTP_HP_FIX_TMSG_LEN)
       && (ps_sOpenPar->w_netCnxnPar_OT != CSS_k_NCP_PTP_SP_FIX_TMSG_LEN)
     )
  {
    /* Invalid Network Connection Parameters */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NET_CNXN_PAR;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CS_INVNCP),
                    IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: if Single-cast Producer */
  else if (CSS_IsPointToPoint(ps_sOpenPar->w_netCnxnPar_TO))
  {
    /* if T_to_O Network_Connection_Parameters invalid */
    if (    (w_netCnxnParToNoSize != CSS_k_NCP_PTP_HP_FIX)
         && (w_netCnxnParToNoSize != CSS_k_NCP_PTP_SP_FIX)
       )
    {
      /* Invalid Network Connection Parameters */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NET_CNXN_PAR;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CS_INVNCPTO),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    /* else: if Time Correction Network_Connection_Parameters are invalid */
    else if ((ps_sOpenPar->s_nsd.w_tcorrNetCnxnPar) != CSS_k_NCP_NOT_USED)
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCCP;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCNCP2),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: all Network_Connection Parameters are valid */
    {
      /* Check Connection Size */
      CnxnSizeCheck((CSS_t_UINT)(ps_sOpenPar->w_netCnxnPar_TO
                                 & CSS_k_NCP_CNXN_SIZE),
                    CSS_k_FALSE, ps_sOpenRespStat, pu8_payloadSize);
    }
  }
  else /* else: Multi-cast Producer */
  {
    /* if T_to_O Network_Connection_Parameters invalid */
    if (    (w_netCnxnParToNoSize != CSS_k_NCP_MC_HP_FIX)
         && (w_netCnxnParToNoSize != CSS_k_NCP_MC_SP_FIX)
       )
    {
      /* Invalid Network Connection Parameters */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NET_CNXN_PAR;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CM_INVNCP),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    /* else: if Time Correction Network_Connection_Parameters are invalid:
             (CSS_k_NCP_NOT_USED: general case on EtherNet/IP,
             CSS_k_NCP_MC_HP_FIX_TMSG_LEN or CSS_k_NCP_MC_SP_FIX_TMSG_LEN:
             when request is bridged from DeviceNet or also allowed on
             EtherNet/IP).
             See Volume 5 Tables 3-3.5 and C-3.1 and C-5.1 */
    else if (    (ps_sOpenPar->s_nsd.w_tcorrNetCnxnPar != CSS_k_NCP_NOT_USED)
              && (ps_sOpenPar->s_nsd.w_tcorrNetCnxnPar !=
                  CSS_k_NCP_MC_HP_FIX_TMSG_LEN)
              && (ps_sOpenPar->s_nsd.w_tcorrNetCnxnPar !=
                  CSS_k_NCP_MC_SP_FIX_TMSG_LEN)
            )
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCCP;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCNCP3),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: all Network_Connection Parameters are valid */
    {
      /* Check Connection Size */
      CnxnSizeCheck((CSS_t_UINT)(ps_sOpenPar->w_netCnxnPar_TO
                                 & CSS_k_NCP_CNXN_SIZE),
                    CSS_k_TRUE, ps_sOpenRespStat, pu8_payloadSize);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*******************************************************************************
**
** Function    : CnxnParamsValidate
**
** Description : This function checks various connection parameters if they
**               contain valid values.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu8_payloadSize (OUT)  - calculated payload size of the cnxn
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnParamsValidate(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat,
                               CSS_t_USINT *pu8_payloadSize)
{
  /* make sure we return a defined result */
  *pu8_payloadSize = 0U;

  /* if Connection Timeout Multiplier is invalid */
  if (ps_sOpenPar->u8_cnxnTimeoutMultiplier > k_MAX_CNXN_TO_MULT)
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TOUT_MULT;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TMULT),
                      IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)ps_sOpenPar->u8_cnxnTimeoutMultiplier);
  }
  else /* else: Connection Timeout Multiplier is ok */
  {
    /*
    ** Check Transport Type/Trigger
    */
    /* if this is a server */
    if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_SERVER_APP_CL0)
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      {
        /* (see SRS12 bullet 8 and SRS180 bullet 6) */
        CnxnParamsValidateServer(ps_sOpenPar, ps_sOpenRespStat,
                                 pu8_payloadSize);
      }
    #else
      {
        /* Invalid Transport Class/Trigger */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCT;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCT_1),
                          IXSER_k_I_NOT_USED,
                          (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
      }
    #endif
    }
    /* else if this is a client */
    else if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      {
        /* (see SRS11 bullet 8 and SRS180 bullet 6) */
        CnxnParamsValidateClient(ps_sOpenPar, ps_sOpenRespStat,
                                 pu8_payloadSize);
      }
    #else
      {
        /* Invalid Transport Class/Trigger */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCT;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCT_2),
                          IXSER_k_I_NOT_USED,
                          (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
      }
    #endif
    }
    else /* else: Invalid Transport Class/Trigger */
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCT;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCT_3),
                        IXSER_k_I_NOT_USED,
                        (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
    }

    /* if an error was detected in previous checks */
    if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
    {
      /* already found an error - skip further checks */
    }
    else /* else: no error */
    {
      /* Check if RPI is within valid range */
      RpiCheck(ps_sOpenPar, ps_sOpenRespStat);
      /* if an error was detected in previous checks */
      if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
      {
        /* already found an error - skip further checks */
      }
      else /* else: no error */
      {
        /*
        ** call sub functions for range checking of the Safety Parameters
        */
        /* (see SRS11 bullet 11 and SRS12 bullet 11 and SRS180 bullet 9) */
        CnxnParamsValidateSafety1(ps_sOpenPar, ps_sOpenRespStat);

        /* if an error was detected in previous checks */
        if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
        {
          /* already found an error - skip further checks */
        }
        else /* else: no error */
        {
          /* so many checks - thus it was split in 2 functions */
          /* (see SRS11 bullet 11 and SRS12 bullet 11 and SRS180 bullet 9) */
          CnxnParamsValidateSafety2(ps_sOpenPar, ps_sOpenRespStat);
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : RpiCheck
**
** Description : This function checks if O->T and T->O RPIs are within valid
**               ranges.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void RpiCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                     CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* maximum data RPI allowed for this connection (micro seconds) */
  CSS_t_UDINT u32_maxRpiData_us;

/* RSM_IGNORE_QUALITY_BEGIN Notice #22       - if not bound with scope braces */
  #if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)      \
        && (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)  \
      )
    /* if Base Format */                                /* CCT_NO_PRE_WARNING */
    if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
  #endif
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      u32_maxRpiData_us = IXSCE_k_MAX_RPI_DATA_BASE_US;
    }
  #endif
  #if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)      \
        && (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)  \
      )
    else /* else: it is Extended Format */              /* CCT_NO_PRE_WARNING */
  #endif
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      u32_maxRpiData_us = IXSCE_k_MAX_RPI_DATA_EXT_US;
    }
  #endif
/* RSM_IGNORE_QUALITY_END */

/* RSM_IGNORE_QUALITY_BEGIN Notice #22       - if not bound with scope braces */
  #if ((CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) && (CSOS_cfg_NUM_OF_SV_SERVERS > 0U))
    /* if Transport_Class/Trigger indicates Client */   /* CCT_NO_PRE_WARNING */
    if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
  #endif
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    {
      /* if one of the RPIs is out of the specified range */
      if (    (ps_sOpenPar->u32_rpiTO_us < IXSCE_k_MIN_RPI_DATA_US)
           || (ps_sOpenPar->u32_rpiTO_us > u32_maxRpiData_us)
           || (ps_sOpenPar->u32_rpiOT_us < IXSCE_k_MIN_RPI_TCOO_US)
           || (ps_sOpenPar->u32_rpiOT_us > IXSCE_k_MAX_RPI_TCOO_US)
         )
      {
        /* Error: Invalid RPI(s) */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_RPI_NOT_SUP;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CLI_RPI_INV),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
      else  /* else: RPIs valid */
      {
        /* success */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
    }
  #endif
  #if ((CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) && (CSOS_cfg_NUM_OF_SV_SERVERS > 0U))
    else /* else: it is a Server */                     /* CCT_NO_PRE_WARNING */
  #endif
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      /* if one of the RPIs is out of the specified range */
      if (    (ps_sOpenPar->u32_rpiOT_us < IXSCE_k_MIN_RPI_DATA_US)
           || (ps_sOpenPar->u32_rpiOT_us > u32_maxRpiData_us)
           || (ps_sOpenPar->u32_rpiTO_us < IXSCE_k_MIN_RPI_TCOO_US)
           || (ps_sOpenPar->u32_rpiTO_us > IXSCE_k_MAX_RPI_TCOO_US)
         )
      {
        /* Error: Invalid RPI(s) */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_RPI_NOT_SUP;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SRV_RPI_INV),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
      else  /* else: RPIs valid */
      {
        /* success */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
    }
  #endif
/* RSM_IGNORE_QUALITY_END */
}


/*******************************************************************************
**
** Function    : CnxnParamsValidateSafety1
**
** Description : This function checks the parameters of the Safety Network
**               Segment. As there are so many checks this function only checks
**               from Time Correction EPI to Timeout_Multiplier.
**
** See Also    : CnxnParamsValidateSafety2()
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnParamsValidateSafety1(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* check Time Correction EPI - must be 0 for Single-Cast */
  /* if Time Correction EPI is invalid */
  if (    (ps_sOpenPar->s_nsd.u32_tcorrEpi_us != 0U)
       && (!CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
     )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCEPI),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else Time Correction EPI is valid */
  {
    /* Time Correction Network_Connection_Parameters already checked in
       CnxnParamsValidateServer() or CnxnParamsValidateClient() */

  /* The Ping_Interval_EPI_Multiplier shall be greater than or equal to:
     [{the Max Timeout_Multiplier.PI (C#) for all consumers}
      * Max_Consumer_Num] + 15   (see FRS239) */
  /* For Multicast connection: as the Ping_Interval_EPI_Multiplier and
     Max_Consumer_Number must be equal for all consumers range checking the
     PIEM here and using the Timeout_Multiplier.PI of the Originator that now
     wants to join is sufficient because if this check will fail then this
     Originator will be denied to take part and so the requirement is fulfilled
     for the whole Multicast connection. */

    /* Ping_Interval_EPI_Multiplier */
    /* calculate the minimum value for the PIEM */
    CSS_t_UINT u16_piemMin = ps_sOpenPar->s_nsd.u8_timeoutMult;

    /* if larger than maximum then limit to maximum */
    if (ps_sOpenPar->s_nsd.u8_timeoutMult > CSS_k_MAX_TMULT_PI)
    {
      u16_piemMin = CSS_k_MAX_TMULT_PI;
    }
    else /* else: */
    {
      /* u16_piemMin already set */
    }

    u16_piemMin = (CSS_t_UINT)(u16_piemMin
                                * ps_sOpenPar->s_nsd.u8_maxConsNum);
    u16_piemMin = (CSS_t_UINT)(u16_piemMin + CSOS_k_MAX_CONSUMER_NUM_MCAST);

    /* check PIEM (see FRS235) */
    /* if PIEM is invalid */
    if (    (ps_sOpenPar->s_nsd.u16_pingIntEpiMult < u16_piemMin)
         || (ps_sOpenPar->s_nsd.u16_pingIntEpiMult > CSS_k_MAX_PIEM)
       )
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PIEM;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_PIEM),
                    IXSER_k_I_NOT_USED,
                    (CSS_t_UDINT)ps_sOpenPar->s_nsd.u16_pingIntEpiMult);
    }
    else /* else: PIEM is ok */
    {
      /* Ping_Count_Interval must be less than 100 seconds (see FRS318) */
      /* Ping_Count_Interval = EPI * Ping_Interval_EPI_Multiplier */
      /* on a producing Target the EPI is equal to T->O RPI */
      /* on a consuming Target the EPI is equal to O->T RPI */
      CSS_t_UDINT u32_epi_us = 0U;

/* RSM_IGNORE_QUALITY_BEGIN Notice #22       - if not bound with scope braces */
    #if ((CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) && (CSOS_cfg_NUM_OF_SV_SERVERS > 0U))
      /* if Transport_Class/Trigger indicates Client */ /* CCT_NO_PRE_WARNING */
      if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
    #endif
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      {
        u32_epi_us = ps_sOpenPar->u32_rpiTO_us;
      }
    #endif
    #if ((CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) && (CSOS_cfg_NUM_OF_SV_SERVERS > 0U))
      else /* else: it is a Server */                   /* CCT_NO_PRE_WARNING */
    #endif
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      {
        u32_epi_us = ps_sOpenPar->u32_rpiOT_us;
      }
    #endif
/* RSM_IGNORE_QUALITY_END */

      /* if Ping_Count_Interval is too large */
      if (   (CSS_t_UDINT)(u32_epi_us * ps_sOpenPar->s_nsd.u16_pingIntEpiMult)
           > (CSS_t_UDINT)k_SEC_TO_USEC((CSS_t_UDINT)100U)
         )
      {
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PIEM;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_PCI),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        /* coverage result of the gcov tool:
           this block is not reachable in every configuration */
      }
      else /* else: PIEM is ok */
      {
        /* if Time_Coord_Msg_Min_Multiplier is too large (see FRS243) */
        if (ps_sOpenPar->s_nsd.u16_tcooMsgMinMult_128us
            > CSS_k_MAX_TCOOM_MIN_MULT_128US)
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCMMM;
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCMMM),
            IXSER_k_I_NOT_USED,
            (CSS_t_UDINT)ps_sOpenPar->s_nsd.u16_tcooMsgMinMult_128us);
        }
        else /* else Time_Coord_Msg_Min_Multiplier is ok */
        {
          /* if Network_Time_Expectation_Multiplier is too large */
          if (ps_sOpenPar->s_nsd.u16_NetTimeExpMult_128us
              > CSS_k_MAX_NTE_MULT_128US)
          {
            ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
            ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TEXP_MULT;
            SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_NTEM),
              IXSER_k_I_NOT_USED,
              (CSS_t_UDINT)ps_sOpenPar->s_nsd.u16_NetTimeExpMult_128us);
          }
          else /* else: Network_Time_Expectation_Multiplier is ok */
          {
            /* Timeout_Multiplier */
            /* Base Format 1..4  Extended Format 1..255 (see FRS230) */
            /* if Timeout_Multiplier is not in limits described above */
            if (    (ps_sOpenPar->s_nsd.u8_timeoutMult < CSS_k_MIN_TMULT_PI)
                 || (    (ps_sOpenPar->s_nsd.u8_format
                          == CSOS_k_SNS_FORMAT_TARGET_BASE)
                      && (ps_sOpenPar->s_nsd.u8_timeoutMult >
                          CSS_k_MAX_TMULT_PI)
                    )
               )
            {
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
              ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TOUT_MULT;
              SAPL_CssErrorClbk(
                (IXSCE_k_NFSE_RXE_SO_INV_STMUL),
                IXSER_k_I_NOT_USED,
                (CSS_t_UDINT)ps_sOpenPar->s_nsd.u8_timeoutMult);
            }
            else /* else: Timeout_Multiplier is ok */
            {
              /* all checks successful */
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
            }
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : CnxnParamsValidateSafety2
**
** Description : This function checks the parameters of the Safety Network
**               Segment. As there are so many checks this function only checks
**               from Max_Consumer_Number to the end.
**
** See Also    : CnxnParamsValidateSafety1()
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CnxnParamsValidateSafety2(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* Max_Consumer_Number */
  CSS_t_USINT u8_maxConsnum;

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  /* if this is a multicast connection */
  if (CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
  {
    u8_maxConsnum = CSOS_k_MAX_CONSUMER_NUM_MCAST;  /* (see FRS242) */
  }
  else /* else: this is singlecast */
#endif
  {
    u8_maxConsnum = CSOS_k_MAX_CONSUMER_NUM_SCAST;  /* (see FRS241) */
  }

  /* if Max_Consumer_Number is invalid */
  if (    (ps_sOpenPar->s_nsd.u8_maxConsNum == 0U)
       || (ps_sOpenPar->s_nsd.u8_maxConsNum > u8_maxConsnum)
     )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MAX_CONS_NUM;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_MCNUM),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: Max_Consumer_Number is ok */
  {
    /* Time Correction Connection ID:
         singlecast: not used - must be k_TIME_CORR_CNXN_ID_NOT_USED
         multicast: not checked - any value allowed */
    /* if Time Correction Connection ID is invalid (singlecast only) */
    if (    CSS_IsPointToPoint(ps_sOpenPar->w_netCnxnPar_TO)
         && (ps_sOpenPar->s_nsd.u32_tcorrCnxnId != k_TIME_CORR_CNXN_ID_NOT_USED)
       )
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCCID;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_TCCID),
                        IXSER_k_I_NOT_USED, ps_sOpenPar->s_nsd.u32_tcorrCnxnId);
    }
    else /* else: Time Correction Connection ID is ok */
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      /* if is base format */
      if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
      {
        /* for base format all parameters have been checked */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
      else /* else: is extended format */
      {
        /* Max_Fault_Number - Volume 5 Table C-5.1 limits the range to 0-255 */

        /* if Max_Fault_Number is invalid */
        if (ps_sOpenPar->s_nsd.u16_maxFaultNum > k_MAX_MAX_FAULT_NUM)
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_MFNUM),
                            IXSER_k_I_NOT_USED,
                            (CSS_t_UDINT)ps_sOpenPar->s_nsd.u16_maxFaultNum);
        }
        else /* else: Max_Fault_Num is ok */
        {
          /* if is a producer */
          if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
          {
        #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
            {
              if (CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
              {
                /* if Initial Timestamp and Initial Rollover have invalid
                   values */
                if  (    (ps_sOpenPar->s_nsd.u16_initalTs
                          != k_INITIAL_TIME_STAMP_NOT_USED)
                      || (ps_sOpenPar->s_nsd.u16_initialRv
                          != k_INITIAL_ROLLOVER_NOT_USED)
                    )
                {
                  ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
                  ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PARAM_ERR;
                  SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_ITSRV1),
                                    IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
                }
                else
                {
                  /* all tests were successful */
                  ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
                }
              }
              else
              {
                /* for single-cast producers Initial Rollover and Initial Time
                   Stamp cannot be checked */
                ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
              }
            }
        #else
            {
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
              ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
              SAPL_CssErrorClbk((IXSCE_k_FSE_INC_CPVS2_NO_C),
                                IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
          }
          /* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
          /* The following "else if" was implemented in one line to avoid a wrong code
             coverage result of the gcov tool */
          else if (ps_sOpenPar->b_transportTypeTrigger  == IXSVO_k_TCT_SERVER_APP_CL0)
          /* RSM_IGNORE_QUALITY_END */
          {
        #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
            /* if Initial Timestamp and Initial Rollover have invalid values */
            if  (    (ps_sOpenPar->s_nsd.u16_initalTs
                      != k_INITIAL_TIME_STAMP_NOT_USED)
                  || (ps_sOpenPar->s_nsd.u16_initialRv
                      != k_INITIAL_ROLLOVER_NOT_USED)
                )
            {
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
              ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PARAM_ERR;
              SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_ITSRV2),
                                IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
            }
            else
            {
              /* all tests were successful */
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
            }
        #else
            {
              ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
              ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
              SAPL_CssErrorClbk((IXSCE_k_FSE_INC_CPVS2_NO_S),
                                IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
            }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
          }
          else
          {
            /* transport trigger completely invalid */
            /* should have been detected in earlier check! */
            ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
            ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_TCT;
            SAPL_CssErrorClbk((IXSCE_k_FSE_INC_CPVS2_XPORT_INV),
                              IXSER_k_I_NOT_USED,
                              (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
        }
      }
    #else
      {
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
    #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : AppPathVerify
**
** Description : This function checks the 3 Application Paths that are contained
**               in a SafetyOpen request. After some common checks depending on
**               the kind of connection it will check with the IXSAI if the
**               requested Assembly Instance/Connection Point exists.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to received Forward_Open
**                                        parameters
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void AppPathVerify(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                          CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_BOOL o_configPathError = CSS_k_TRUE;

  /*
  ** Check configuration path
  */
  /* if class ID in configuration path is invalid */
  if (ps_sOpenPar->u16_cfgClass != CSOS_k_CCC_ASSEMBLY_OBJ)
  {
    /* Configuration Path must always point to the Assembly Object */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CONFIG_PATH;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_CASM),
                      IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)ps_sOpenPar->u16_cfgClass);
    /* Flag indicating Configuration Path Error already set */
  }
  else /* class ID in configuration path is ok */
  {
    /* if the configuration path addresses is the NULL instance */
    if (ps_sOpenPar->u16_cfgInst == CSS_cfg_ASM_NULL_INST_ID)
    {
      /* Target support for Type 1 and Type 2 SafetyOpen (see Req.5.2-5) */
      /* if the Forward_Open does not contain configuration data */
      if (ps_sOpenPar->pb_cfgData == CSS_k_NULL)
      {
        /* ... this is ok as the SafetyOpen does not contain config data. */
        o_configPathError = CSS_k_FALSE;
      }
      else /* else: Type 1 SafetyOpen */
      {
        /* ...this is not ok as there is config data */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CONFIG_PATH;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CASM_NULL),
                          ps_sOpenPar->u16_cfgInst, IXSER_k_A_NOT_USED);
        /* Flag indicating Configuration Path Error already set */
      }
    }
    else /* else: configuration path instance is non-null */
    {
      /* For now accept any Configuration Assembly Instance ID. This parameter
         is completely in responsibility of the Safety Application
         (SAPL_IxsceSafetyOpenValidateClbk()). */
      o_configPathError = CSS_k_FALSE;
    }
  }

  /* if an error was found in the configuration path */
  if (o_configPathError != CSS_k_FALSE)
  {
    /* There was already an error in the configuration path so we do not      */
    /* have to continue path checking.                                        */
    /* CIP Error Code is already set */
  }
  else /* else: configuration path was ok */
  {
    /* if is a server */
    if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_SERVER_APP_CL0)
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      /*
      ** Check Producing and Consuming Connection Points for Server Connections
      ** (see SRS12 bullet 10 and SRS180 bullet 8)
      */
      /* if a Producing Cnxn Point is non-null */
      if (ps_sOpenPar->u16_cnxnPointProd != CSS_cfg_ASM_NULL_INST_ID)
      {
        /* error: a Server only consumes data and does not produce */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PROD_PATH;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_S_PROD_NN),
                          ps_sOpenPar->u16_cnxnPointProd, IXSER_k_A_NOT_USED);
      }
      else /* else: Producing Cnxn Point is null */
      {
        /* ok, a Server only consumes data so check Consuming Cnxn Point */
        /* if consuming Cnxn Point addresses an Output Assembly */
        if (IXSAI_AsmInstTypeGet(ps_sOpenPar->u16_cnxnPointCons)
             == IXSAI_k_ASM_OUTPUT)
        {
          /* ok, Consuming Connection Point is valid */
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
        }
        else /* else: addressed Assembly Instance is either not existing or not
                      a consuming Assembly */
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CONS_PATH;
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_S_CONS_I),
                            ps_sOpenPar->u16_cnxnPointCons, IXSER_k_A_NOT_USED);
        }
      }
    #else
      {
        SAPL_CssErrorClbk((IXSCE_k_FSE_INC_APP_VER_NO_C),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    #endif
    }
    else /* else: is a client */
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      /*
      ** Check Producing and Consuming Connection Points for Client Connections
      ** (see SRS11 bullet 10 and SRS180 bullet 8)
      */
      /* if a Consuming Cnxn Point is non-null */
      if (ps_sOpenPar->u16_cnxnPointCons != CSS_cfg_ASM_NULL_INST_ID)
      {
        /* error: a Client only produces data and does not consume */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CONS_PATH;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_C_CONS_NN),
                          ps_sOpenPar->u16_cnxnPointCons, IXSER_k_A_NOT_USED);
      }
      else /* else: Consuming Cnxn Point is null */
      {
        /* ok, a Client only produces data so check Producing Cnxn Point */
        /* if producing Cnxn Point addresses an Output Assembly */
        if (IXSAI_AsmInstTypeGet(ps_sOpenPar->u16_cnxnPointProd)
             == IXSAI_k_ASM_INPUT)
        {
          /* ok, Consuming Connection Point is valid */
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
        }
        else /* else: addressed Assembly Instance is either not existing or not
                      a producing Assembly */
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_PROD_PATH;
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_C_PROD_I),
                          ps_sOpenPar->u16_cnxnPointProd, IXSER_k_A_NOT_USED);
        }
      }
    #else
      {
        SAPL_CssErrorClbk((IXSCE_k_FSE_INC_APP_VER_NO_S),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    #endif
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*** End Of File ***/


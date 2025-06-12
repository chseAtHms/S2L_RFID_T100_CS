/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEsop2.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module continues checking of a received SafetyOpen.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_SafetyOpenProc2
**
**             SafetyOpenProc3
**             ConfigDataCheck
**             ScidCheck
**             OcpunidCheck
**             SccrcCheck
**             CfunidCheck
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

#include "IXSAIapi.h"
#include "IXSAI.h"
#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXCRC.h"
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


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD SafetyOpenProc3(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                  const CSS_t_SV_ID *ps_svInfo,
                                  CSS_t_USINT u8_payloadSize,
                                  CSS_t_SOPEN_RESP *ps_sOpenResp);
static void ConfigDataCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                            CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void ScidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
static CSS_t_BOOL OcpunidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
#endif
static void SccrcCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                       CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static void CfunidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_SafetyOpenProc2
**
** Description : This function is called from {IXSCE_SafetyOpenProc()} and
**               implements further checks and allocation and initialization
**               of a safety validator instance.
**
** Parameters  : ps_sOpenPar (IN)    - pointer to received Forward_Open
**                                     parameters
**                                     (not checked, only called with
**                                     reference to structure variable)
**               ps_svInfo (IN)      - in case the connection is already
**                                     existing this struct contains information
**                                     about it.
**                                     (not checked, only called with reference
**                                     to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as the
**                                     payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error
**
*******************************************************************************/
CSS_t_WORD IXSCE_SafetyOpenProc2(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                 const CSS_t_SV_ID *ps_svInfo,
                                 CSS_t_USINT u8_payloadSize,
                                 CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  /* flag indicating of saving a new OCPUNID is required */
  CSS_t_BOOL o_saveOcpunid = CSS_k_FALSE;
#endif

  /* if this is a Type 1 SafetyOpen */
  if (ps_sOpenPar->pb_cfgData != CSS_k_NULL)
  {
    /* We do not check if the received SCID is different than the stored      */
    /* SCID. Thus when the device receives a Type 1 SafetyOpen containing     */
    /* configuration data and an SCID that matches its existing configuration */
    /* the configuration is re-applied and all rules checks are executed      */
    /* again (see SRS171).                                                    */

    /* In case of a Type 1 SafetyOpen the reconfiguration will take place     */
    /* immediately. Thus there cannot be any further incoming connection      */
    /* requests during reconfiguration (see SRS11 bullet 6)                   */

    /* Type 1 SafetyOpen (with configuration data) */
    ConfigDataCheck(ps_sOpenPar, &ps_sOpenResp->s_status);
  }
  else /* else: Type 2 SafetyOpen */
  {
    /* Type 2 SafetyOpen (without configuration data) */
    ScidCheck(ps_sOpenPar, &ps_sOpenResp->s_status);
  }

  /* if previous check discovered an error */
  if (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
  {
    /* Found an error. Do not continue processing */
    /* CIP Error Code already set -> an error response will be returned */
    w_retVal = CSS_k_OK;
  }
  else /* else: no error */
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    {
      /* For Output Connections check if Originator is the owner of the       */
      /* requested Output Connection Point                                    */
      /* (see SRS12 bullet 1 and SRS180 bullet 1)                             */
      o_saveOcpunid = OcpunidCheck(ps_sOpenPar, &ps_sOpenResp->s_status);
    }

    /* if previous check discovered an error */
    if (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
    {
      /* Found an error. Do not continue processing */
      /* CIP Error Code already set -> an error response will be returned */
      w_retVal = CSS_k_OK;
    }
    else /* else: no error */
  #endif
    {
      /* Last but not least let the application decide if it permits */
      /* the connection request                                      */
      w_retVal = SAPL_IxsceSafetyOpenValidateClbk(ps_sOpenPar, u8_payloadSize);

      /* if Safety Application denied the connection */
      if (w_retVal != CSS_k_OK)
      {
        ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenResp->s_status.u16_ext = w_retVal;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SAPL_DENY),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        w_retVal = CSS_k_OK;
      }
      else /* else: Safety Application permits the connection */
      {
        /* if Type 1 SafetyOpen */
        if (ps_sOpenPar->pb_cfgData != CSS_k_NULL)
        {
          /* inform the Safety Supervisor about Type 1 SafetyOpen */
          /* (see SRS11 bullet 12 and SRS12 bullet 12) */
          w_retVal = IXSSO_StateMachine(IXSSO_k_SE_T1_SOPEN,
                                        IXSSO_k_SEAI_NONE);

          /* if SSO state machine returned an error */
          if (w_retVal != CSS_k_OK)
          {
            /* state machine returned an error */
            SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SSO_SM_1),
                              IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
          }
          else /* else: no error */
          {
            /* if Application doesn't apply the configuration data */
            if (!SAPL_IxsceCfgApplyClbk())
            {
              w_retVal = (IXSCE_k_NFSE_RXE_SO_SAPL_APPL);
              /* application returned with error from apply service */
              SAPL_CssErrorClbk(w_retVal,
                                IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
            }
            else /* else: configuration data applied */
            {
              /* inform the Safety Supervisor about Type 1 SafetyOpen */
              /* (see SRS11 bullet 12 and SRS12 bullet 12) */
              w_retVal = IXSSO_StateMachine(IXSSO_k_SE_APPLY_REQ,
                                            IXSSO_k_SEAI_NONE);

              /* if SSO state machine returned an error */
              if (w_retVal != CSS_k_OK)
              {
                /* state machine returned an error */
                SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SSO_SM_2),
                                  IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
              }
              else /* else: no error */
              {
                /* Save the SCID. This makes the originator the new owner of  */
                /* the configuration (see SRS7 and SRS11 bullet 7 and         */
                /* SRS12 bullet 7)                                            */
                w_retVal = IXSSO_ScidSet(ps_sOpenPar->s_nsd.u32_sccrc,
                                         &ps_sOpenPar->s_nsd.s_scts);

                /* if storage error */
                if (w_retVal != CSS_k_OK)
                {
                  /* error while storing SCID - will be handled a few code    */
                  /* lines below together with a possible CFUNID storing      */
                  /* error (next check of w_retVal)                           */
                  SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_STORE_1),
                                     IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
                }
                else /* else: no error */
                {
                  /* Save the CFUNID */
                  w_retVal = IXSSO_CfUnidSet(&ps_sOpenPar->s_nsd.s_ounid);

                  /* if storage error */
                  if (w_retVal != CSS_k_OK)
                  {
                    /* error while storing SCID - will be handled a few code  */
                    /* lines below (next check of w_retVal)                   */
                    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_STORE_3),
                                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
                  }
                  else
                  {
                    /* success -> continue with w_retVal = CSS_k_OK */
                  }
                }
              }
            }
          }
        }
        else /* else: Type 2 SafetyOpen */
        {
          /* nothing to do for the application here */
        }

        /* if previous check discovered an error */
        if (w_retVal != CSS_k_OK)
        {
          /* error while storing SCID or CFUNID or SSO state machine or Apply */
          ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
          w_retVal = CSS_k_OK;
        }
        else /* else: no error */
        {
        #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
          /* if we don't have to save the OCPUNID */
          if (!o_saveOcpunid)
          {
            /* This is a producing connection or this output is already owned */
          }
          else /* else: OCPUNID must be saved */
          {
            /* Save the OCPUNID. This makes the originator the new owner of   */
            /* the output (see SRS8).                                         */
            w_retVal = IXSSO_OcpUnidForCnxnPointSet(
                         ps_sOpenPar->u16_cnxnPointCons,
                         &ps_sOpenPar->s_nsd.s_ounid);
          }

          /* if previous check discovered an error */
          if (w_retVal != CSS_k_OK)
          {
            /* error while storing OCPUNID */
            ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
            ps_sOpenResp->s_status.u16_ext = CSS_k_CMEXTSTAT_MISCELLANEOUS;
            SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_STORE_2),
                              IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
            w_retVal = CSS_k_OK;
          }
          else /* else: no error */
        #endif
          {
            /* to reduce complexity of this function let a sub-function do the
               rest */
            w_retVal = SafetyOpenProc3(ps_sOpenPar, ps_svInfo, u8_payloadSize,
                                       ps_sOpenResp);
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
** Function    : SafetyOpenProc3
**
** Description : This function is called from {IXSCE_SafetyOpenProc2()} and
**               implements further checks and allocation and initialization
**               of a safety validator instance.
**
** Parameters  : ps_sOpenPar (IN)    - pointer to received Forward_Open
**                                     parameters
**                                     (not checked, only called with
**                                     reference to structure variable)
**               ps_svInfo (IN)      - in case the connection is already
**                                     existing this struct contains information
**                                     about it.
**                                     (not checked, only called with reference
**                                     to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as the
**                                     payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error
**
*******************************************************************************/
static CSS_t_WORD SafetyOpenProc3(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                  const CSS_t_SV_ID *ps_svInfo,
                                  CSS_t_USINT u8_payloadSize,
                                  CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* Instance ID of the Safety Validator that is to be allocated here */
  CSS_t_UINT u16_svInst = CSOS_k_INVALID_INSTANCE;

  /* if the requested connection is not yet existing */
  if (ps_svInfo->u16_instId == CSOS_k_INVALID_INSTANCE)
  {
    /* Allocate an IXSVC or IXSVS instance */
    w_retVal = IXSVO_TargAlloc(ps_sOpenPar, &u16_svInst,
                               &ps_sOpenResp->s_status);
  }
  else /* else: requested connection is already existing */
  {
    /* if this is a join to an established (or initializing) multicast cnxn */
    if (    (ps_svInfo->o_multiProd)
         && (ps_svInfo->u8_state != CSS_k_SV_STATE_FAILED)
       )
    {
      /* Allocate an IXSVC instance */
      w_retVal = IXSVO_TargAlloc(ps_sOpenPar, &u16_svInst,
                                 &ps_sOpenResp->s_status);
    }
    else  /* else: singlecast or failed cnxn */
    {
      /* reinitialize the Client or Server instance (see FRS12-5) */
      u16_svInst = ps_svInfo->u16_instId;
      w_retVal = IXSVO_TargClear(ps_sOpenPar, ps_svInfo);
    }
  }

  /* if previous check discovered an error */
  if (    (w_retVal != CSS_k_OK)
       || (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
     )
  {
    /* no instance available */
    w_retVal = CSS_k_OK;
  }
  else /* else: no error */
  {
    /* set the instance number that is reported to CSAL */
    ps_sOpenResp->s_appReply.u16_instId = u16_svInst;

    /* initialize the new Safety Validator (see SRS11 bullet 12 and
       SRS12 bullet 12 and SRS180 bullet 10) */
    w_retVal = IXSVO_TargInit(ps_sOpenPar, u8_payloadSize, u16_svInst,
                              ps_sOpenResp);

    /* if previous check discovered an error */
    if (    (w_retVal != CSS_k_OK)
         || (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
       )
    {
      /* error found during init of the Safety Validator */
      w_retVal = CSS_k_OK;
    }
    else /* else: no error */
    {
      /* opening the connection was successful */
      ps_sOpenResp->s_status.u8_gen = CSOS_k_CGSC_SUCCESS;
      ps_sOpenResp->s_status.u16_ext = CSOS_k_INVALID_ADD_STATUS;
      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ConfigDataCheck
**
** Description : This function checks if a received SafetyOpen contains
**               configuration data. If there is configuration data it checks
**               the SCCRC and in case this check is successful it checks the
**               CFUNID.
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
static void ConfigDataCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                            CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* check the SCCRC */
  SccrcCheck(ps_sOpenPar, ps_sOpenRespStat);

  /* if previous check discovered an error */
  if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
  {
    /* SCCRC mismatch */
    /* CIP Error Code already set */
  }
  else /* else: no error */
  {
    /* check the Configuration UNID (our CFUNID vs. received OUNID)
       (see FRS166 and SRS11 bullet 2 and SRS12 bullet 4) */
    CfunidCheck(ps_sOpenPar, ps_sOpenRespStat);

    /* if previous check discovered an error */
    if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
    {
      /* We already have a different connection owner. */
      /* CIP Error Code already set */
    }
    else /* else: no error */
    {
      /* Device gets reconfigured, SCID will be saved in a later stage of   */
      /* the opening process IXSSO_ScidSet(). But further checks need to be */
      /* done before that.                                                  */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : ScidCheck
**
** Description : This function checks if a non-zero SCID has been received in
**               Safety Open and if so compares it against our configured SCID.
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
static void ScidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                      CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* pointer to our SCID */
  const IXSSO_t_SCID *ps_scid = IXSSO_ScidPtrGet();

  /* if our device is not yet configured */
  if (    (ps_scid->u32_sccrc == 0U)
       && (ps_scid->s_scts.u32_time == 0U)
       && (ps_scid->s_scts.u16_date == 0U)
     )
  {
    /* Our device is not yet configured and thus cannot accept a Type 2       */
    /* SafetyOpen (SRS173).                                                  */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_DEV_NOT_CFG;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_UNCONF),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: our device is already configured */
  {

    /* if we receive a Zero SCID */
    if (    (ps_sOpenPar->s_nsd.u32_sccrc == 0U)
         && (ps_sOpenPar->s_nsd.s_scts.u32_time == 0U)
         && (ps_sOpenPar->s_nsd.s_scts.u16_date == 0U)
       )
    {
      /* Zero SCIDs are always accepted. It is then the originators           */
      /* responsibility to make sure our device is configured as he expects   */
      /* (see FRS163) */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
    }
    else /* else: SCID is non-zero */
    {
      /* if the SCID matches the configured value
         (see SRS2 and SRS180 bullet 2) */
      if (    (ps_sOpenPar->s_nsd.u32_sccrc == ps_scid->u32_sccrc)
           && (ps_sOpenPar->s_nsd.s_scts.u32_time == ps_scid->s_scts.u32_time)
           && (ps_sOpenPar->s_nsd.s_scts.u16_date == ps_scid->s_scts.u16_date)
         )
      {
        /* OK, received SCID matches our SCID */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
      }
      else /* else: SCID mismatch */
      {
        /* Do not accept connection */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCID;
        SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SCID_MIS),
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
/*******************************************************************************
**
** Function    : OcpunidCheck
**
** Description : This function is for output connections only. It checks if the
**               requested output connection point is already owned by another
**               originator. If it is unowned it takes over ownership for the
**               requesting originator.
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
** Returnvalue : CSS_k_TRUE             - caller must save new OCPUNID
**               CSS_k_FALSE            - caller doesn't have to save OCPUNID
**
*******************************************************************************/
static CSS_t_BOOL OcpunidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if this is a producer */
  if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
  {
    /* no further checks are necessary here */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
  }
  else /* else: this is a consumer */
  {
    /* This is a consuming connection, so we must check the OCPUNID */
    CSS_t_UNID s_ocpunid;

    /* completely erase structure */
    CSS_MEMSET(&s_ocpunid, 0, sizeof(s_ocpunid));

    /* if function to obtain the OCPUNID for this cnxn point returns an error */
    if (IXSSO_OcpUnidForCnxnPointGet(ps_sOpenPar->u16_cnxnPointCons,
                                     &s_ocpunid) != CSS_k_OK)
    {
      /* CnxnPoint not found */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CONS_PATH;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_OCP_NF),
                        ps_sOpenPar->u16_cnxnPointCons,
                        IXSER_k_A_NOT_USED);
      /* return value already set */
    }
    else /* else: no error */
    {
      /* if Output Connection Point is not yet owned */
      if (IXSSO_UnidCompare(&s_ocpunid, &IXSSO_ks_ZeroUnid))
      {
        /* There is not yet an owner. So we will allocate the ownership in a  */
        /* later stage of the opening process                                 */
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
        o_retVal = CSS_k_TRUE;
      }
      else /* else: Output Connection Point already has an owner */
      {
        /* if the same owner wants to open the connection */
        if (IXSSO_UnidCompare(&s_ocpunid, &ps_sOpenPar->s_nsd.s_ounid))
        {
          /* OK, Originator that sent the SafetyOpen is already owner of this */
          /* connection point.                                                */
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
          o_retVal = CSS_k_TRUE;
        }
        else /* else: Output Connection Point is owned by somebody else */
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_OUNID_OUT;
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_OCPUNID),
                            IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        }
        /* return value already set */
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*******************************************************************************
**
** Function    : SccrcCheck
**
** Description : This function calculates the SCCRC over the received
**               configuration data and checks if it matches the received SCCRC.
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
static void SccrcCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                       CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* SCCRC is a CRC-S4 (32-bit) */
  CSS_t_UDINT u32_sccrc = k_SCCRC_SEED_VALUE;
  /* configuration data size is stored in WORDs, convert to number of bytes */
  CSS_t_UINT u16_cfgDataSizeB =
    (CSS_t_UINT)((CSS_t_UINT)ps_sOpenPar->u8_cfgDataSizeW * CSOS_k_SIZEOF_WORD);

  /* Calculate SCCRC over entire Application Data Section                     */
  /* (=Config Data Segment) (see FRS110 and SRS11 bullet 4 and                */
  /* SRS12 bullet 5).                                                         */
  u32_sccrc = IXCRC_CrcS4compute(ps_sOpenPar->pb_cfgData,
                                 u16_cfgDataSizeB,
                                 u32_sccrc);

  /* if calculated SCCRC matches received SCCRC (see SRS36) */
  if (u32_sccrc == ps_sOpenPar->s_nsd.u32_sccrc)
  {
    /* received SCCRC is valid */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
  }
  else /* else: mismatch in received SCCRC */
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCID;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SCCRC_MIS),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : CfunidCheck
**
** Description : This function checks if it is allowed to accept the received
**               configuration data.
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
static void CfunidCheck(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* get a pointer to our CFUNID */
  const CSS_t_UNID *ps_cfunid = IXSSO_CfUnidPtrGet();

  /* if the configuration is not yet owned (see SRS70 bullet 1) */
  if (IXSSO_UnidCompare(ps_cfunid, &IXSSO_ks_ZeroUnid))
  {
    /* Device was not yet owned. So accept the new owner (see SRS205) */
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;

    /* CFUNID will be saved in a later stage of the opening process */

    /* It will be the responsibility of the application to copy and store    */
    /* received configuration data. */
  }
  else /* else: there is already an owner of the configuration */
  {
    /* if the originator of this Forward_Open request is not the            */
    /* configuration owner.                                                 */
    if (!IXSSO_UnidCompare(ps_cfunid, &ps_sOpenPar->s_nsd.s_ounid))
    {
      /* Configuration is owned by somebody else (see SRS9). */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_OUNID_CFG;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_CFUNID),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    /* else: if the configuration is owned by the configuration tool
             (see SRS70 bullet 2) */
    else if (IXSSO_UnidCompare(&ps_sOpenPar->s_nsd.s_ounid, &IXSSO_ks_ffUnid))
    {
      /* Configuration via SafetyOpen not allowed (see FRS165) */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_CFGOP_NOT_AL;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_TOOL_OWND),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: configuration is owned by the requesting originator */
    {
      /* we may accept this new configuration */
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;

      /* it will be the responsibility of the application to copy and store */
      /* received configuration data */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*** End Of File ***/


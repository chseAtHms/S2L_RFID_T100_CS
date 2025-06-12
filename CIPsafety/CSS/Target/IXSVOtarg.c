/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOtarg.c
**    Summary: IXSVO - Safety Validator Object
**             This module contains the functions specific for Targets
**             concerning the Safety Validator.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_TargAlloc
**             IXSVO_TargClear
**             IXSVO_TargInit
**             IXSVO_CnxnFind
**
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

#include "IXSERapi.h"
#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVCapi.h"
  #include "IXSVC.h"
#endif
#if  (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #include "IXSVSapi.h"
  #include "IXSVS.h"
#endif
#include "IXSCF.h"

#include "IXSVO.h"
#include "IXSVOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVO_TargAlloc
**
** Description : This function allocates for a Target a new Safety Validator
**               (Client or Server) if necessary. A consuming Originator could
**               connect to an already existing Multi-Cast connection. In this
**               case this function returns the Instance of the existing
**               connection.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to parameters received with
**                                        the Forward_Open request
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu16_svInst (OUT)      - pointer to the variable to where the
**                                        returned instance is written to (in
**                                        case of success, otherwise
**                                        CSOS_k_INVALID_INSTANCE)
**                                        (not checked, only called with
**                                        reference to variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_OK               - success
**               <>CSS_k_OK             - error
**
*******************************************************************************/
CSS_t_WORD IXSVO_TargAlloc(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                           CSS_t_UINT *pu16_svInst,
                           CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* index of the allocated Safety Validator object */
  CSS_t_UINT u16_svIdx;

  /* indicate that no instance has been assigned yet */
  *pu16_svInst = CSOS_k_INVALID_INSTANCE;

  /* if we are Client in this connection? */
  if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
  {
    /* This is a Client Connection (Producing Safety Data) */
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

    /* flag that indicates if allocation of new instance is really necessary */
    CSS_t_BOOL o_allocNeeded = CSS_k_TRUE;

    /* if this is a multicast connection */
    if (CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
    {
      /* If the Originator wants to open a Multicast connection we have to    */
      /* check if there is already a matching connection running.             */
      if (!IXSVC_McastCnxnFind(ps_sOpenPar, pu16_svInst, ps_sOpenRespStat))
      {
        /* o_allocNeeded already set */
      }
      else
      {
        /* if we found a similar connection */
        if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
        {
          /* There is already such a connection, but parameters don't match */
          /* CIP error code is already set -> This will be returned to the
             originator */
          o_allocNeeded = CSS_k_FALSE;
        }
        else /* else: we found a matching connection */
        {
          /* if a valid instance ID has been allocated */
          if (*pu16_svInst != CSOS_k_INVALID_INSTANCE)
          {
            /* There is already such a connection, parameters are matching,   */
            /* so we have the case that another consuming originator wants to */
            /* connect to an existing Safety Validator Client. Thus we must   */
            /* not allocate a new Safety Validator.                           */
            /* This also ensures that all Consumers are of Connection_Type    */
            /* Multi-Cast (see FRS231).                                       */
            o_allocNeeded = CSS_k_FALSE;
          }
          else
          {
            /* This is the first consuming originator opening this connection */
            /* allocation of a new instance is needed                         */
            /* o_allocNeeded already set */
          }
        }
      }
    }
    else /* else: single-cast */
    {
      /* always needs allocation */
      /* o_allocNeeded flag already initialized */
    }

    if (o_allocNeeded)
    {
      /* now allocate the client instance */
      u16_svIdx = IXSVC_InstAlloc();

      /* if allocation was not successful */
      if (u16_svIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
        ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NO_MORE_CNXN;
        w_retVal = CSS_k_OK;
      }
      else /* else: allocation was successful */
      {
        /* allocate an instance ID */
      #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
        {
          *pu16_svInst = IXSVD_NextFreeInstIdGet(CSS_cfg_NUM_OF_CCO_INSTS+1U);
        }
      #else
        {
          *pu16_svInst = IXSVD_NextFreeInstIdGet(1U);
        }
      #endif

        /* if allocation was not successful */
        if (*pu16_svInst == CSOS_k_INVALID_INSTANCE)
        {
          w_retVal = (IXSVO_k_NFSE_RES_ALLOC_TARG_CLI);
          SAPL_CssErrorClbk(w_retVal, *pu16_svInst, IXSER_k_A_NOT_USED);
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_RES_UNAVAIL;
        }
        else /* else: allocation was successful */
        {
          /* assign the instance ID to the newly allocated Safety Validator */
          IXSVC_InstIdIdxAssign(*pu16_svInst, u16_svIdx);

          w_retVal = CSS_k_OK;
        }
      }
    }
    else
    {
      /* Do not allocate a new connection (either connection is already       */
      /* existing or an error occurred before).                               */
      w_retVal = CSS_k_OK;
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_ALLOC_INST_C);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                        (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
    }
  #endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
  }
  else /* else: we are Server in this connection */
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    /* This is a Server Connection (Consuming Safety Data) */

    /* Server always needs to allocate a new Safety Validator Instance */
    u16_svIdx = IXSVS_InstAlloc();

    /* if allocation was not successful */
    if (u16_svIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
    {
      ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_NO_MORE_CNXN;
      w_retVal = CSS_k_OK;
    }
    else /* else: allocation was successful */
    {
      /* allocate an instance ID */
    #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
      {
        *pu16_svInst = IXSVD_NextFreeInstIdGet(CSS_cfg_NUM_OF_CCO_INSTS+1U);
      }
    #else
      {
        *pu16_svInst = IXSVD_NextFreeInstIdGet(1U);
      }
    #endif

      /* if allocation was not successful */
      if (*pu16_svInst == CSOS_k_INVALID_INSTANCE)
      {
        w_retVal = (IXSVO_k_NFSE_RES_ALLOC_TARG_SRV);
        SAPL_CssErrorClbk(w_retVal, *pu16_svInst, IXSER_k_A_NOT_USED);
        ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_RES_UNAVAIL;
      }
      else /* else: allocation was successful */
      {
        /* assign the instance ID to the newly allocated Safety Validator */
        IXSVS_InstIdIdxAssign(*pu16_svInst, u16_svIdx);
        w_retVal = CSS_k_OK;
      }
    }
  #else
    {
      w_retVal = (IXSVO_k_FSE_INC_ALLOC_INST_S);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                        (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
    }
  #endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVO_TargClear
**
** Description : This function reinitializes a Safety Validator (Client or
**               Server).
**
** Parameters  : ps_sOpenPar (IN) - pointer to parameters received with
**                                  the Forward_Open request
**                                  (not checked, only called with reference
**                                  to structure variable)
**               ps_svInfo (IN)   - this struct contains Instance ID and
**                                  Consumer Number of the Safety Validator
**                                  Instance
**                                  (not checked, only called with reference
**                                  to structure variable)
**                                  - u16_instId (not checked, checked in
**                                    SafetyOpenProc3())
**                                  - u8_consNum (not checked, checked in
**                                    IXSVS_InstFind() and IXSVC_InstFind())
**                                  - u8_state (not checked, checked in
**                                    IXSVS_InstFind() and IXSVC_InstFind())
**
** Returnvalue : CSS_k_OK         - success
**               <>CSS_k_OK       - error
**
*******************************************************************************/
CSS_t_WORD IXSVO_TargClear(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                           const CSS_t_SV_ID *ps_svInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);
  /* get the index of the Safety Validator (client or server) */
  CSS_t_UINT u16_svIdx = IXSVD_SvIdxFromInstGet(ps_svInfo->u16_instId);

  /* if we are Client in this connection */
  if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
  {

  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    if (u16_svIdx < CSOS_cfg_NUM_OF_SV_CLIENTS)
    {
      IXSVC_InstInit(u16_svIdx);
      w_retVal = CSS_k_OK;
    }
    else
  #endif
    {
      w_retVal = (IXSVO_k_FSE_INC_CLEAR_INST_C);
      SAPL_CssErrorClbk(w_retVal, ps_svInfo->u16_instId, IXSER_k_A_NOT_USED);
    }
  }
  else /* else: we are server */
  {
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    if (u16_svIdx < CSOS_cfg_NUM_OF_SV_SERVERS)
    {
      IXSVS_InstInit(u16_svIdx);
      w_retVal = CSS_k_OK;
    }
    else
  #endif
    {
      w_retVal = (IXSVO_k_FSE_INC_CLEAR_INST_S);
      SAPL_CssErrorClbk(w_retVal, ps_svInfo->u16_instId, IXSER_k_A_NOT_USED);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVO_TargInit
**
** Description : This function initializes an instance of the Safety
**               Validator Object (Client or Server) for a Connection in that we
**               are the Target.
**
** Parameters  : ps_sOpenPar (IN)    - pointer to parameters received with
**                                     the Forward_Open request
**                                     (not checked, only called with
**                                     reference to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as
**                                     the payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               u16_svInst (IN)     - Safety Validator instance ID that is
**                                     already allocated for this connection
**                                     (checked, valid range: 1..
**                                     (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                     CSOS_cfg_NUM_OF_SV_SERVERS))
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error while initializing the Safety
**                                     Validator Instance
**
*******************************************************************************/
CSS_t_WORD IXSVO_TargInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                          CSS_t_USINT u8_payloadSize,
                          CSS_t_UINT u16_svInst,
                          CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* if passed parameter is invalid */
  if (    (u16_svInst == 0U)
       || (u16_svInst > (CSOS_cfg_NUM_OF_SV_CLIENTS+CSOS_cfg_NUM_OF_SV_SERVERS))
     )
  {
    w_retVal = (IXSVO_k_FSE_INC_TARG_INIT_INST);
    SAPL_CssErrorClbk((w_retVal), u16_svInst, IXSER_k_A_NOT_USED);
  }
  else
  {
    /* if we are Client in this connection */
    if (ps_sOpenPar->b_transportTypeTrigger == IXSVO_k_TCT_CLIENT_APP_CL0)
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      {
        w_retVal = IXSVC_TargClientInit(ps_sOpenPar, u8_payloadSize,
                                        u16_svInst, ps_sOpenResp);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_TINIT_INST_C);
        SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                          (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
      }
    #endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
    }
    else /* else: then we must be Server in this connection */
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      {
        w_retVal = IXSVS_TargServerInit(ps_sOpenPar, u8_payloadSize,
                                        u16_svInst, &ps_sOpenResp->s_appReply);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_TINIT_INST_S);
        SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                          (CSS_t_UDINT)ps_sOpenPar->b_transportTypeTrigger);
      }
    #endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVO_CnxnFind
**
** Description : This function searches the Safety Validator Instance which
**               matches the passed Connection Triad.
**
** Parameters  : ps_cnxnTriad (IN) - pointer to the structure which contains
**                                   the description of the connection
**                                   (not checked, only called with
**                                   reference to structure variable)
**               ps_svInfo (OUT)   - pointer to the structure to where the found
**                                   Safety Validator Instance Type, Index, and
**                                   Consumer_Number will be written
**                                   (not checked, only called with
**                                   reference to structure variable)
**
** Returnvalue : CSS_k_TRUE        - connection was found (index in ps_cnxnIdx)
**               CSS_k_FALSE       - connection not found
**
*******************************************************************************/
CSS_t_BOOL IXSVO_CnxnFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo)
{
  /* flag to indicate that the connection that is to be closed was found */
  CSS_t_BOOL o_foundFlag = CSS_k_FALSE;

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  {
    /* find the connection with this serial number in the Server Connections */
    o_foundFlag = IXSVS_InstFind(ps_cnxnTriad, ps_svInfo);
  }

  /* if already found in the Server Instances */
  if (o_foundFlag)
  {
    /* Connection has been found in the Safety Validator Servers */
  }
  else /* else: not found in the Server Instances */
#endif
  {
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
    {
      /* Let's try to find the connection with this serial number in the      */
      /* Client Connections.                                                  */
      o_foundFlag = IXSVC_InstFind(ps_cnxnTriad, ps_svInfo);
    }
#endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_foundFlag);
}


/*******************************************************************************
**    static functions
*******************************************************************************/

#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*** End Of File ***/


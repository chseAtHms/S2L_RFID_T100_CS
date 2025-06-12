/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOclose.c
**    Summary: IXSVO - Safety Validator Object
**             This module contains the functions to close a Safety Validator
**             Instance.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_InstanceClose
**             IXSVO_InstanceStop
**             IXSVO_CnxnDropAll
**
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

#include "IXSERapi.h"

#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVC.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
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


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVO_InstanceClose
**
** Description : This function closes the passed Safety Validator Server or
**               Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, valid range: 1..
**                                 (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                 CSOS_cfg_NUM_OF_SV_SERVERS))
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, only called with constants)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
CSS_t_WORD IXSVO_InstanceClose(CSS_t_UINT u16_instId,
                               CSS_t_USINT u8_consNum)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVO_k_FSE_INC_PRG_FLOW);

  /* if passed parameter is invalid */
  if (    (u16_instId == 0U)
       || (u16_instId > (CSOS_cfg_NUM_OF_SV_CLIENTS+CSOS_cfg_NUM_OF_SV_SERVERS))
     )
  {
    w_retVal = (IXSVO_k_FSE_INC_CLOSE_INST_INV);
    SAPL_CssErrorClbk((w_retVal), u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: passed instance ID is ok */
  {
    /* if it is a Server Instance */
    if (IXSVD_InstIsServer(u16_instId))
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      {
        w_retVal = IXSVS_InstClose(u16_instId, u8_consNum, CSS_k_TRUE);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_CL_WRONG_TYPE_S);
        SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: then it must be a Client Instance */
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      {
        w_retVal = IXSVC_InstClose(u16_instId, u8_consNum, CSS_k_TRUE);
      }
    #else
      {
        w_retVal = (IXSVO_k_FSE_INC_CL_WRONG_TYPE_C);
        SAPL_CssErrorClbk(w_retVal, u16_instId, IXSER_k_A_NOT_USED);
      }
    #endif
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVO_InstanceStop
**
** Description : This function stops/deletes the passed Safety Validator Server
**               or Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, valid range: 1..
**                                 (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                 CSOS_cfg_NUM_OF_SV_SERVERS))
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range: 1..
**                                 CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_InstanceStop(CSS_t_UINT u16_instId,
                        CSS_t_USINT u8_consNum)
{
  /* if passed parameter is invalid */
  if (    (u16_instId == 0U)
       || (u16_instId > (CSOS_cfg_NUM_OF_SV_CLIENTS+CSOS_cfg_NUM_OF_SV_SERVERS))
     )
  {
    SAPL_CssErrorClbk(IXSVO_k_FSE_INC_STOP_INST_INV, u16_instId,
                      IXSER_k_A_NOT_USED);
  }
  else if ((u8_consNum == 0U) || (u8_consNum > CSOS_k_MAX_CONSUMER_NUM_MCAST))
  {
    SAPL_CssErrorClbk(IXSVO_k_FSE_INC_STOP_CONS_NUM, u16_instId,
                      (CSS_t_UDINT)u8_consNum);
  }
  else /* else: passed instance ID and Consumer Number is ok */
  {
    CSS_t_UINT u16_svIdx = IXSVD_SvIdxFromInstGet(u16_instId);

    /* if it is a Server Instance */
    if (IXSVD_InstIsServer(u16_instId))
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      /* if the Safety Validator Server index is valid */
      if (u16_svIdx < CSOS_cfg_NUM_OF_SV_SERVERS)
      {
        IXSVS_InstStop(u16_svIdx);
      }
      else /* else: index is invalid */
    #endif
      {
        SAPL_CssErrorClbk(IXSVO_k_FSE_INC_SP_WRONG_TYPE_S,
                          u16_instId, IXSER_k_A_NOT_USED);
      }
    }
    else /* else: then it must be a Client Instance */
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      /* if the Safety Validator Client index is valid */
      if (u16_svIdx < CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        /* if we close the last consumer */
        if (IXSVC_InstConsumerStop(u16_svIdx, u8_consNum))
        {
          if (IXSVC_InstIdIdxStop(u16_svIdx))
          {
            IXSVC_InstDealloc(u16_svIdx);
          }
          else
          {
            /* error already reported to SAPL */
          }
        }
        else /* else: there are consumers remaining */
        {
          /* instance stays open */
        }
      }
      else /* else: index is invalid */
    #endif
      {
        SAPL_CssErrorClbk(IXSVO_k_FSE_INC_SP_WRONG_TYPE_C,
                          u16_instId, IXSER_k_A_NOT_USED);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVO_CnxnDropAll
**
** Description : This function drops all connections at once. Needed e.g. by
**               the Safety Supervisor Object when a Configure Request service
**               is received.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_CnxnDropAll(void)
{
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  {
    IXSVC_InstCloseAll();
  }
#endif

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  {
    IXSVS_InstCloseAll();
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


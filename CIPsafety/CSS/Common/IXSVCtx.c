/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCtx.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the API function to generate a Safety I/O
**             message.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_InstTxFrameGenerate
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXUTL.h"
#include "IXSSS.h"
#include "IXSVD.h"
#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#include "IXSVCerr.h"


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
** Function    : IXSVC_InstTxFrameGenerate
**
** Description : This API function handles one Safety Validator Client Instance.
**               This function must be called cyclically to update timing
**               information and to initiate generation of Transmit Safety
**               Frames.
**
**               **Attention**:
**               This function must be called at least once per producing EPI.
**
**               **Attention**:
**               It is recommended that the calling interval of this function
**               is the connection's EPI or a multiple of it to avoid a jitter
**               in the produced messages.
**
**               **Attention**:
**               For reasons of convenience of the API this function will not
**               return an error when called with an invalid Instance ID.
**               Instead it will return without processing anything. This way
**               the user can call this function in a loop running over all
**               Instance IDs without having to distinguish between servers and
**               clients.
**
** Parameters  : u16_svcInstId (IN) - Instance Id of the Safety Validator Client
**                                    (checked, valid range:
**                                    1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                    CSOS_cfg_NUM_OF_SV_SERVERS) and
**                                    instance must be a Safety Validator
**                                    Client.
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstTxFrameGenerate(CSS_t_UINT u16_svcInstId)
{
  /* Safety Validator Instance index */
  CSS_t_UINT u16_svcIdx = IXSVD_SvIdxFromInstGet(u16_svcInstId);

  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    IXSVC_ErrorClbk(IXSVC_k_FSE_AIS_STATE_ERR_TFG, u16_svcInstId,
                    IXSER_k_A_NOT_USED);
  }
  else /* else: stack state is ok */
  {
    /* if the returned index is invalid */
    if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
    {
      /* an invalid safety validator instance was passed */
      /* this is not an error, but this function won't do anything */
    }
    /* else: if the passed instance is a Safety Validator Server */
    else if (IXSVD_InstIsServer(u16_svcInstId))
    {
      /* This instance is a Safety Validator Server. */
      /* So this function is not applicable. */
    }
    else /* else: passed Instance ID is ok */
    {
      /* if the state of the Safety Validator Client is init or established */
      if (IXSVC_StateIsInitOrEstablished(u16_svcIdx))
      {
        CSS_t_UDINT u32_nextTxTime_128us = IXSVC_ProdNextTxTimeGet(u16_svcIdx);

        /* if it is time for a transmission (EPI, see FRS123) */
        /* (see also FRS16 and FRS23) */
        if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(),
                                       u32_nextTxTime_128us))
        {
          /* generate a transmit frame for this instance */
          /* return value ignored (called function provides this for historical
             reasons */
          (void)IXSVC_TxFrameGenerate(u16_svcInstId, u16_svcIdx);
        }
        else /* else: not yet time to transmit another message */
        {
          /* nothing to be done here */
        }

        /* update the quarantine status of faulted consumers */
        {
          CSS_t_USINT u8_consNum = IXSVC_QuarConsUpdate(u16_svcIdx);

          /* if there is a consumer that needs to be closed */
          if (    (u8_consNum > 0U)
               && (u8_consNum <= CSOS_k_MAX_CONSUMER_NUM_MCAST)
             )
          {
            /* A consumer number that was under quarantine can be closed now. */
            /* return value ignored (called function provides this for
               historical reasons */
            (void)IXSVC_InstClose(u16_svcInstId, u8_consNum, CSS_k_TRUE);
          }
          else /* else: there is currently no cnxn that needs to be closed */
          {
          }
        }
      }
      else /* else: state not init or established */
      {
        /* This instance cannot be processed as it is currently not in use or */
        /* is faulted.                                                        */
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


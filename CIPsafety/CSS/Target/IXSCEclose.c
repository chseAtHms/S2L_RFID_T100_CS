/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEclose.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module contains the processing of Forward_Close requests.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_SafetyCloseProc
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
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXSAIapi.h"
#include "IXSAI.h"
#include "IXSVOapi.h"
#include "IXSVO.h"
#include "IXSCF.h"

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

/* The Safety Close_Service is used to close connections with Safety          */
/* Targets (see FRS334) */


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_SafetyCloseProc
**
** Description : This function searches the Safety Validator Instance which
**               matches the received Forward_Close parameters and then closes
**               this instance.
**
** Parameters  : ps_cnxnTriad (IN)   - pointer to received Forward_Close
**                                     parameters (only relevant data)
**                                     (not checked, only called with
**                                     reference to structure variable)
**               ps_sCloseResp (OUT) - pointer to structure for returning the
**                                     parameters for a Forward_Close response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCE_SafetyCloseProc(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                 IXSCE_t_SCLOSE_RESP *ps_sCloseResp)
{
  /* information where the addressed connection can be found */
  CSS_t_SV_ID s_svInfo;
  /* information about the associated Safety Validator */
  CSS_t_VALIDATOR_INFOS s_svoInfos;

  /* completely erase structures */
  CSS_MEMSET(&s_svInfo, 0, sizeof(s_svInfo));
  CSS_MEMSET(&s_svoInfos, 0, sizeof(s_svoInfos));

  /* if we found the Safety Validator with matching connection Triad */
  if (IXSVO_CnxnFind(ps_cnxnTriad, &s_svInfo))
  {
    /* Found the connection with matching Originator Vendor ID,           */
    /* Connection Serial Number and Originator Serial Number. Other       */
    /* data in the Connection_Path of the Forward_Close request is not    */
    /* relevant (see FRS337) */

    /* get information from Safety Validator (type and Connection Point)  */
    IXSVO_InstanceInfoGet(s_svInfo.u16_instId, &s_svoInfos);

    /* close connection and deallocate all resources (see FRS335).        */
    /* The Safety Close_Service is used to close connections with Safety  */
    /* Targets (see FRS334).                                              */
    IXSVO_InstanceStop(s_svInfo.u16_instId, s_svInfo.u8_consNum);
    /* copy instance ID and Consumer Number for Close Response message */

    /* if this Safety Validator is in states INITIALIZING or ESTABLISHED */
    if (    (s_svoInfos.u8_state == CSS_k_SV_STATE_INITIALIZING)
         || (s_svoInfos.u8_state == CSS_k_SV_STATE_ESTABLISHED)
       )
    {
      /* pass the Instance ID and Consumer Number to CSAL for closing the
         underlying communications */
      ps_sCloseResp->u16_instId = s_svInfo.u16_instId;
      ps_sCloseResp->u8_consNum = s_svInfo.u8_consNum;
    }
    else /* other states (IDLE or FAILED) */
    {
      /* indicate to CSAL that no underlaying communications is existing */
      ps_sCloseResp->u16_instId = CSOS_k_INVALID_INSTANCE;
      ps_sCloseResp->u8_consNum = 0xFFU;
    }

    /* indicate success */
    ps_sCloseResp->s_status.u8_gen = CSOS_k_CGSC_SUCCESS;
  }
  else /* else: connection not found */
  {
    /* error - connection could not be closed (cnxn not found) */
    ps_sCloseResp->s_status.u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sCloseResp->s_status.u16_ext = CSS_k_CMEXTSTAT_CNXN_NOT_FND;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SC_INV_CNXN),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*** End Of File ***/


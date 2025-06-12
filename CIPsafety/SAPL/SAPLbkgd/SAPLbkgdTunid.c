/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdTunid.c
**     Summary: This module implements the background task functionality in the state
**              'WAIT_TUNID'.
**   $Revision: 2412 $
**       $Date: 2017-03-23 08:32:18 +0100 (Do, 23 Mrz 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskTunid
**
**             BkgdTaskCheckForTunidState
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicSm.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* CSS headers */
#include "IXSSOapi.h"

/* SAPL headers */
#include "SAPLbkgdInt.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      26u


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void BkgdTaskCheckForTunidState(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskTunid

  Description:
    This function is used to execute the functionality in wait tunid 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskTunid(void)
{
  /* Attention: The CSS API functions are called in every AIC state to trigger the CSS internal
  ** state machine and to handle external services.
  ** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2197]).
  */

  /* set current system time of CSS and handle reset of the fault counters */
  SAPL_BkgdComSetSysTime();

  /* IXSVS_ConsInstActivityMonitor() shall be called cyclically (see [3.9-2:]),
  ** so done here (inside SomActivityMonitor()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComActivityMonitor();

  /* IXSVC_InstTxFrameGenerate() shall be called cyclically (see [3.8-1:]),
  ** so done here (inside SAPL_BkgdComTxFrameGenerate()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComTxFrameGenerate();

  /******************************************************************************/
  /* Execute state specific part                                                */
  /******************************************************************************/
  /* Check if TUNID is set now */
  BkgdTaskCheckForTunidState();

  /******************************************************************************/
  /* Processing of HALC (CSAL) Message                                          */
  /******************************************************************************/
  /* Process HALC message, only a single message is processed here */
  SAPL_BkgdComRxProcessHalcMsg();
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    BkgdTaskCheckForTunidState

  Description:
    This function checks if the TUNID is set now via Explicit Messaging service.
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskCheckForTunidState(void)
{
  /* Current device status */
  CSOS_t_SSO_DEV_STATUS e_deviceStatus = IXSSO_DeviceStatusGet();
  
  /* evaluate CSS state */
  switch (e_deviceStatus)
  {
    /* State 'Wait for TUNID */
    case CSOS_k_SSO_DS_WAIT_TUNID:
    {
      /* still waiting for TUNID */
      break;
    }
    /* State 'Configuring' */
    case CSOS_k_SSO_DS_CONFIGURING:
    {
      /* Set Event 'TUNID received' (see [SRS_2198])*/
      aicSm_SetEvent(AICSM_k_EVT_TUNID_RECEIVED);
      break;
    }
    /* other State */
    case CSOS_k_SSO_DS_IDLE:
    case CSOS_k_SSO_DS_UNDEFINED:     /* fall through */
    case CSOS_k_SSO_DS_SELF_TESTING:  /* fall through */
    case CSOS_k_SSO_DS_SELF_TEST_EX:  /* fall through */
    case CSOS_k_SSO_DS_EXECUTING:     /* fall through */
    case CSOS_k_SSO_DS_ABORT:         /* fall through */
    case CSOS_k_SSO_DS_CRIT_FAULT:    /* fall through */
    {
      /* not expected after calling CSS initialization, never return from this... */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
      break;
    }
  }
}

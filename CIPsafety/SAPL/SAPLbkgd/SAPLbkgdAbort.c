/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdAbort.c
**     Summary: This module implements the background task functionality in the state
**              'ABORT'.
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskAbort
**
**             BkgdTaskCheckForAbortState
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
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

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
#define k_FILEID      37u

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC void BkgdTaskCheckForAbortState(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskAbort

  Description:
    This function is used to execute the functionality in 'ABORT'

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskAbort(void)
{
  /* Attention: The CSS API functions are called in every AIC state to trigger the CSS internal
  ** state machine and to handle external services.
  ** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2217]).
  */

  /* Check if CSS is already in Abort state */
  BkgdTaskCheckForAbortState();

  /* set current system time of CSS and handle reset of the fault counters */
  SAPL_BkgdComSetSysTime();

  /* IXSVS_ConsInstActivityMonitor() shall be called cyclically (see [3.9-2:]),
  ** so done here (inside SAPL_BkgdComActivityMonitor()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComActivityMonitor();

  /* IXSVC_InstTxFrameGenerate() shall be called cyclically (see [3.8-1:]),
  ** so done here (inside SAPL_BkgdComTxFrameGenerate()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComTxFrameGenerate();

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
    BkgdTaskCheckForAbortState

  Description:
    This function checks the current state of the CIP Safety Target (CSS). If the CSS is not
    already in Abort state, this state is requested via 'Internal Abort Request' by calling
    IXSSO_SafetyDeviceMajorFault() (see [CS_MAN], [SRS_2247]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskCheckForAbortState(void)
{
  /* Current device status */
  CSOS_t_SSO_DEV_STATUS e_deviceStatus = IXSSO_DeviceStatusGet();

  /* evaluate CSS state */
  switch (e_deviceStatus)
  {
    /* Already in ABORT state */
    case CSOS_k_SSO_DS_ABORT:
    {
      /* nothing to do */
      break;
    }

    /* states in which to trigger 'Internal Abort request' */
    case CSOS_k_SSO_DS_WAIT_TUNID:    /* fall through */
    case CSOS_k_SSO_DS_CONFIGURING:   /* fall through */
    case CSOS_k_SSO_DS_IDLE:          /* fall through */
    case CSOS_k_SSO_DS_EXECUTING:     /* fall through */
    {
      /* set CSS into 'Abort' state, see [SRS_2247], [3.6-3:] */
      IXSSO_SafetyDeviceMajorFault(CSS_k_TRUE); /* recoverable fault */
      break;
    }

    /* invalid states, shall not occur */
    case CSOS_k_SSO_DS_UNDEFINED:     /* fall through */
    case CSOS_k_SSO_DS_SELF_TESTING:  /* fall through */
    case CSOS_k_SSO_DS_SELF_TEST_EX:  /* fall through */
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

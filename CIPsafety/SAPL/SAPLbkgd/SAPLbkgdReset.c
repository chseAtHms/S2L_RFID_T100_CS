/***************************************************************************************************
**    Copyright (C) 2016-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdReset.c
**     Summary: This module implements the background task functionality in the state
**              'WAIT_RESET'.
**   $Revision: 3457 $
**       $Date: 2020-04-22 15:06:25 +0200 (Mi, 22 Apr 2020) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskReset
**
**             BkgdTaskResetHandler
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
#include "rds.h"
#include "timer-def.h"
#include "timer-hal.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicSm.h"
#include "aicMsgCsal_cfg.h"
#include "aicMsgCsal.h"

/* CSOS common headers */
#include "CSOSapi.h"
/* HALCS headers */
#include "HALCSmsg_cfg.h"
#include "HALCSmsg.h"

/* SAPL headers */
#include "SAPLnvHigh.h"
#include "SAPLnvLow.h"
#include "SAPLbkgdInt.h"
#include "SAPLreset.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      25u

#ifndef __UNIT_TESTING_ON__
  /* Init-value (RDS_UINT8) of local Attribute Bit Map */
  #define k_LOCAL_ATTR_BIT_MAP_INIT         {{(UINT8)0u},{(UINT8)~(UINT8)0u}}
#else /* __UNIT_TESTING_ON__ */
  /* Init-Value used for unit tests. RDS functionality is turned off here */
  #define k_LOCAL_ATTR_BIT_MAP_INIT         0U
#endif /* __UNIT_TESTING_ON__ */

  /* FSM in 'WAIT_RESET' state */
typedef enum
{
  k_CHECK_RESET        = 0x7118u,
  k_WRITE_BUSY         = 0x7236u,
  k_TRIGGER_REQUEST    = 0x73a1u,
  k_ENDLESS_LOOP       = 0x746au
} e_FSM_RESET;


/* current FSM state */
STATIC e_FSM_RESET e_ResetState = k_CHECK_RESET;
/* current pending reset type */
STATIC SAPL_t_SAFETY_RESET_TYPE e_ResetType = SAPL_k_SAFETY_RESET_NONE;
/* current Attribute Bit Map */
/* Deactivated Lint message 708, because a Union initialization is wanted to save
** init function */
STATIC RDS_UINT8 u8_AttrBitMap = k_LOCAL_ATTR_BIT_MAP_INIT; /*lint !e708 */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void BkgdTaskResetHandler(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskReset

  Description:
    This function is used to execute the functionality in 'WAIT_RESET'. 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskReset(void)
{
  /* Attention: The CSS API functions are called in every AIC state to trigger the CSS internal
  ** state machine and to handle external services.
  ** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2205],
  ** [SRS_2165]).
  */

  /******************************************************************************/
  /* Execute state specific part                                                */
  /******************************************************************************/
  /* Process Reset FSM */
  BkgdTaskResetHandler();

  /******************************************************************************/
  /* Processing of HALC (CSAL) Message                                          */
  /******************************************************************************/
  /* Discard pending HALC Message */
  SAPL_BkgdComDiscardHalcMsg();
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    BkgdTaskResetHandler

  Description:
    This function executes the state machine for Safety Reset handling
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskResetHandler(void)
{
  /* ATTENTION: According to the description of SAPL_IxssoSafetyResetClbk() no other CSS API 
  ** function shall be called any more!
  ** No processing of IO Data Messages and Time Coordination Messages from SPDU is done here, 
  ** means no cyclic CIP Safety communication possible (see [SRS_2205], [SRS_2165]).
  */
  
  /* evaluate FSM state */
  switch (e_ResetState)
  {
    /* The state 'Abort' is requested */
    case k_CHECK_RESET:
    { 
      /* Attribute Bit Map received by Safety Reset (only relevant for Safety Reset Type 2) */
      UINT8 u8_attrBitMap;
      
      /* get and store pending reset type */
      e_ResetType = SAPL_ResetTypeGet(&u8_attrBitMap);
      /* store Attribute Bit Map */
      RDS_SET(u8_AttrBitMap, u8_attrBitMap);
      
      /* if Safety Reset Type 0 received */
      if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE0)
      {
        /* if flash is currently busy */
        if ( TRUE == SAPL_NvLowIsBusy() )
        {
          /* enter 'write busy' state */
          e_ResetState = k_WRITE_BUSY;
        }
        /* else no flash action pending */
        else
        {
          /* no flash action pending, so trigger of reset could be done now */
          e_ResetState = k_TRIGGER_REQUEST;
        }
      }
      /* else if Safety Reset Type 1 received */
      else if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE1)
      {
        /* Return as closely as possible to the default configuration */
        /* Request Default Settings inside NV memory (see [SRS_2170]) */
        SAPL_NvHighDefaultStore();
        /* enter next state */
        e_ResetState = k_WRITE_BUSY;
      }
      /* else if Safety Reset Type 2 received */
      else if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE2)
      {
        /* Return as closely as possible to the default configuration for values 
        ** not preserved by the Attribute Bit Map, see [SRS_2171] */
        SAPL_NvHighResetType2Store(RDS_GET(u8_AttrBitMap));      
        /* enter next state */
        e_ResetState = k_WRITE_BUSY;
      }
      /* else: variable error, soft error? */
      else
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
      }
      break;
    }
    
    /* The FSM waits until default values are written into NV memory */
    case k_WRITE_BUSY:
    { 
      /* if flash is (still) busy */
      if ( TRUE == SAPL_NvLowIsBusy() )
      {
        /* empty branch */
      }
      /* else: flash has finished */
      else
      {
        /* enter next state */
        e_ResetState = k_TRIGGER_REQUEST;
      }
      break;
    }
    
    /* The software triggers the Safety Reset Request via SIS command */
    case k_TRIGGER_REQUEST:
    {
      /* trigger request to ABCC, this will inform the non-safe host that a valid Safety Reset 
      ** Request was received (see [SRS_2164]) */
      
      /* if Safety Reset Type 0 received */
      if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE0)
      {
        HALCS_MsgTxReqSafetyReset(HALCS_k_CMD_SAFETY_RESET_TYPE0, (UINT8)0U);
      }
      /* else if Safety Reset Type 1 received */
      else if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE1)
      {
        HALCS_MsgTxReqSafetyReset(HALCS_k_CMD_SAFETY_RESET_TYPE1, (UINT8)0U);
      }
      /* else if Safety Reset Type 2 received */
      else if (e_ResetType == SAPL_k_SAFETY_RESET_TYPE2)
      {
        HALCS_MsgTxReqSafetyReset(HALCS_k_CMD_SAFETY_RESET_TYPE2, RDS_GET(u8_AttrBitMap));
      }
      /* else: variable error, soft error? */
      else
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
      }
      
      /* enter next state */
      e_ResetState = k_ENDLESS_LOOP;
      break;
    }
    
    /* The software waits until a HW reset occurs */
    case k_ENDLESS_LOOP:
    {
      /* empty branch, waiting on reset...  */
      break;
    }
    
    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
      break;
    }
  }
}

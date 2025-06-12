/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdConfig.c
**     Summary: This module implements the background task functionality in the state
**              'CONFIG'.
**   $Revision: 4440 $
**       $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskConfig
**
**             BkgdTaskConfigProcess
**             BkgdTaskIoCfgVerify
**             BkgdTaskIoCfgStore
**             BkgdTaskIoCfgVerifyAndStore
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
#include "cfg_Config-sys.h"
#include "timer-def.h"

#include "gpio_cfg.h"
#include "fiParam.h"

#include "timer-hal.h"

#include "aicMsgDef.h"
#include "aicSm.h"
#include "aicMsgCfg.h"


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
#include "IXSSO.h"

/* SAPL headers */
#include "SAPLcfg.h"
#include "SAPLnvHigh.h"
#include "SAPLnvLow.h"
#include "SAPLbkgdInt.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      22u

/* FSM in 'CONFIG' state */
typedef enum
{
  k_CHECK_STATE      = 0x7945u,
  k_WRITE_CONFIG     = 0x7a6bu,
  k_APPLY_CONFIG     = 0x7bfcu
} t_CONFIG_STATE;


/* current FSM state */
STATIC t_CONFIG_STATE e_State = k_CHECK_STATE;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void BkgdTaskConfigProcess(void);
STATIC BOOL BkgdTaskIoCfgVerify(CONST AICMSGCFG_SETCONFIGSTRDATA* ps_cfgStrData);
STATIC void BkgdTaskIoCfgStore(CONST AICMSGCFG_SETCONFIGSTRDATA* ps_cfgStrData);
STATIC BOOL BkgdTaskIoCfgVerifyAndStore(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskConfig

  Description:
    This function is used to execute the functionality in configuration mode

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskConfig(void)
{

  /* Attention: The CSS API functions are called in every AIC state to trigger the CSS internal
  ** state machine and to handle external services.
  ** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2199]).
  */

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
  /* Execute state specific part                                                */
  /******************************************************************************/
  /* Process Configuration FSM */
  BkgdTaskConfigProcess();

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
    BkgdTaskConfigProcess

  Description:
    This function implements a state machine for re-configuration of the device. The main tasks 
    are:
      - waiting for a 'SetConfigString' request from ABCC (in AIC states AICSM_AIC_WAIT_FOR_CONFIG 
        or AICSM_AIC_INVALID_CONFIG
      - putting CSS into 'Configuring' state (if not already done) via 'Type 1 SafetyOpen' event
      - verifying received IO Configuration Data against received SCCRC
      - requesting the storage of the IO Configuration Data and SCCRC (non-volatile memory)
      - waiting until IO Configuration Data and SCCRC are stored.
      - applying of IO Configuration Data and SCCRC inside pkgParam (enables IO Configuration)
      - putting CSS into 'Idle' state via 'Apply Request'

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskConfigProcess(void)
{ 
  /* evaluate current substate */
  switch (e_State)
  {
    /* Check state of CSS and AIC */
    case k_CHECK_STATE:
    {
      /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
      ** considered as 'atomic' here */
      AICSM_STATE_ENUM e_aicState = aicSm_eAicState; 
      
      /* In 'WAIT_FOR_CONFIG' and 'INVALID_CONFIG' we still wait for a new IO Configuration Data 
      ** and SCID via SetConfig string request (see [SRS_2201]). 
      ** So until this is received, we stay here... */
      if ( (e_aicState == AICSM_AIC_WAIT_FOR_CONFIG) ||
           (e_aicState == AICSM_AIC_INVALID_CONFIG) )
      {
        /* empty branch */
      }
      /* else: in 'PROCESS_CONFIG' state we check the state of the CSS.
      ** if necessary, a transition into 'Configuring' state is triggered.
      */
      else if (e_aicState == AICSM_AIC_PROCESS_CONFIG)
      {
        /* Current device status */
        CSOS_t_SSO_DEV_STATUS e_deviceStatus = IXSSO_DeviceStatusGet();
        
        /* if state transition direct from IDLE. This means that a reconfiguration 
        ** was requested by the non-safe host */
        if (e_deviceStatus == CSOS_k_SSO_DS_IDLE)
        {
          /* ATTENTION: There is currently no chance to switch from 'Idle' into
          ** 'Configuring' state via CSS API. Due this, the CSS internal state machine
          ** is triggered here with the 'Type 1 SafetyOpen' event. The 'Type 1 SafetyOpen' event
          ** is used because the 'ConfigureRequest' event is currently only available on 
          ** originator implementations. Both events put the CSS into 'Configuring' state.
          ** ATTENTION: This Event deletes the SCID stored inside NV memory!!!
          */
          
          /* ATTENTION: During the call of IXSSO_StateMachine() the CSS calls the callback 
          ** SAPL_IxssoNvAttribStoreClbk() which triggers flash access */
          /* if state change not executed */
          if ( IXSSO_StateMachine((UINT8)IXSSO_k_SE_T1_SOPEN, 
                                  (UINT16)IXSSO_k_SEAI_NONE) != CSS_k_OK ) /* no additional info */
          {
            /* we strongly assume that state is changed here, correctly */
            GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(1u));
          }
          /* else: everything fine */
          else
          {
            /* if IO Configuration Data matches to SCCRC */
            if (BkgdTaskIoCfgVerifyAndStore() == TRUE)
            {
              /* enter next state */
              e_State = k_WRITE_CONFIG;
            }
            /* else: IO Configuration Data does not match */
            else
            {
              /* set the 'Invalid Configuration' flag to trigger global state machine */
              aicSm_SetEvent(AICSM_k_EVT_INVLD_CONFIG);
              /* stay in state, because no valid IO Configuration was written */
            }
          }
        }
        /* else: if CSS state is already in state 'Configuring' */
        else if (e_deviceStatus == CSOS_k_SSO_DS_CONFIGURING)
        {
          /* if IO Configuration Data matches to SCCRC */
          if (BkgdTaskIoCfgVerifyAndStore() == TRUE)
          {
            /* enter next state */
            e_State = k_WRITE_CONFIG;
          }
          /* else: IO Configuration Data does not match */
          else
          {
            /* set the 'Invalid Configuration' flag to trigger global state machine */
            aicSm_SetEvent(AICSM_k_EVT_INVLD_CONFIG);
            /* stay in state, because no valid IO Configuration was written */
          }
        }
        /* else: invalid CSS state, shall NOT happen */
        else
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(2u));
        }
      }
      /* else: invalid AIC state, shall NOT happen */
      else
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
      }
      break;
    }
    
    /* State: Program/Write IO Configuration Data + SCID */
    case k_WRITE_CONFIG:
    { 
      /* if flash is still busy */
      if ( TRUE == SAPL_NvLowIsBusy() )
      {
        /* empty branch */
      }
      /* else: flash has finished */
      else
      {
        /* enter next state */
        e_State = k_APPLY_CONFIG;
      }
      break;
    }
    
    /* State: Apply */
    case k_APPLY_CONFIG:
    {
      /* The IO Configuration Data + SCID is stored inside NV memory so the configuration
      ** is checked again (IO Configuration vs. SCCRC) and applied.
      */
      SAPL_BkgdComIoCfgApply();
      
      /* Trigger transition to CSS 'Idle' state manually here */
      /* ATTENTION: there is currently no chance to switch from 'Configuring' into
      ** 'Idle' state via CSS API. Due this, the CSS internal state machine
      ** is triggered here with the 'Apply' event.
      */
      
      /* if state change not executed */
      if ( IXSSO_StateMachine((UINT8)IXSSO_k_SE_APPLY_REQ, 
                              (UINT16)IXSSO_k_SEAI_NONE) != CSS_k_OK) /* no additional info */
      {
        /* we strongly assume that state is changed here, correctly */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(4u));
      }
      /* else: everything fine */
      else
      {
        /* The received SCCRC matches to the received IO Configuration data and the
        ** values are stored inside NV memory. So set the 'Configuration Done' flag to
        ** trigger global state machine to change into IDLE (see [SRS_2202]).
        ** Additionally the IO Configuration is activated inside GPIO module
        ** (see [SRS_2155]) then.
        */
        aicSm_SetEvent(AICSM_k_EVT_CONFIG_DONE);
      
        /* reset state machine */
        e_State = k_CHECK_STATE;
      }
      break;
    }
    
    default:
    {
      /* invalid state */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
    }
  }
}

/***************************************************************************************************
  Function:
    BkgdTaskIoCfgVerify

  Description:
    This function is used to verify the received IO Configuration Data. For this purpose the 
    IO Configuration Data is verified against the received SCCRC (see [SRS_2131]). Also the
    configuration bytes for unsupported features are checked to be zero.

  See also:
    -

  Parameters:
    ps_cfgStrData (IN)    - Pointer to SetConfigString data received by SetConfigString
                            request from AM
                            (valid range: <>NULL, not checked, only called by reference)

  Return value:
    TRUE   - Received IO Configuration Data is valid
    FALSE  - Received IO Configuration Data is invalid

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC BOOL BkgdTaskIoCfgVerify(CONST AICMSGCFG_SETCONFIGSTRDATA* ps_cfgStrData)
{
  /* return value */
  BOOL b_retVal = FALSE;
  /* calculated SCCRC */
  UINT32 u32_calcSccrc;

  /* calculate SCCRC */
  u32_calcSccrc = fiParam_SccrcCompute(ps_cfgStrData->au8_ioConfigData, 
                                       SAPL_k_SDS_CONF_DATA_IO);

  /* if calculated SCCRC doesn't match to the received one */
  if (ps_cfgStrData->u32_sccrc != u32_calcSccrc)
  {
    /* SCCRC mismatch */
    /* empty branch, return value already set */
  }
  /* else if: calculated and received SCCRC are zero */
  else if (u32_calcSccrc == 0UL)
  {
    /* calculated SCCRC matches the received SCCRC, but is zero: This is a very special case
       which isn't clearly covered by the CIP Safety specification. To avoid any problems
       (e.g. later Type 2 SafetyOpens will skip SCCRC check) we reject this configuration. */
    /* empty branch, return value already set */
  }
  /* else if: value and consistency checking fails */
  else if (SAPL_ConfigStringCheck(ps_cfgStrData->au8_ioConfigData) != CSS_k_OK)
  {
    /* Configuration is not valid */
    /* empty branch, return value already set */
  }
  /* else: SCCRC matches and configuration values are ok */
  else
  {
    b_retVal = TRUE;
  }

  return b_retVal;
}

/***************************************************************************************************
  Function:
    BkgdTaskIoCfgStore

  Description:
    The function:
      - triggers the flash process of raw IO Configuration Data
      - triggers the flash process of SCID via CSS internal function

  See also:
    -

  Parameters:
    ps_cfgStrData (IN)    - Pointer to SetConfigString data received by SetConfigString
                            request from AM
                            (valid range: <>NULL, not checked, only called by reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskIoCfgStore(CONST AICMSGCFG_SETCONFIGSTRDATA* ps_cfgStrData)
{
  /* date and time  */
  CSS_t_DATE_AND_TIME s_dateTime;
  /* SCCRC */
  CSS_t_UDINT u32_sccrc;
  
  /* store IO Configuration Data, see [SRS_2143] */
  SAPL_NvHighConfigStore(&ps_cfgStrData->au8_ioConfigData[0]);
  
  /* clear memory of structure to ensure padding bytes are set to zero */
  CSS_MEMSET(&s_dateTime, 0, sizeof(s_dateTime));
  /* assign received values for SCTS (Date + Time) and SCCRC */
  u32_sccrc = ps_cfgStrData->u32_sccrc;
  s_dateTime.u32_time  = ps_cfgStrData->u32_scts_time;
  s_dateTime.u16_date  = ps_cfgStrData->u16_scts_date;
  
  /* set SCID via CSS internal function, the function also stores the SCID in
  ** NV memory (SAPL_IxssoNvAttribStoreClbk()), see [SRS_2143].
  ** ATTENTION: There is currently no chance to store the SCID from 'higher' level
  ** here. So we are using the internal CSS function here 
  */
  
  /* if not executed correctly */
  if ( IXSSO_ScidSet(u32_sccrc, &s_dateTime) != CSS_k_OK)
  {
    /* we strongly assume that function executed correctly */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(6u));
  }
  /* else: everything fine */
  else
  {
    /* In case of AIC background configuration set CFUNID to "tool owned configuration" via CSS
    ** internal function (see [SRS_2116]).
    ** The function also stores the CFUNID in NV memory
    ** (SAPL_IxssoNvAttribStoreClbk()), see [SRS_2143].
    ** ATTENTION: There is currently no chance to store the CFUNID from 'higher' level
    ** here. So we are using the internal CSS function here.
    */
    
    /* if not executed correctly */
    if ( IXSSO_CfUnidSet(&IXSSO_ks_ffUnid) != CSS_k_OK)
    {
      /* we strongly assume that function executed correctly */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(7u));
    }
    /* else: everything fine */
    else
    {
      /* empty branch */
    } 
  }
}

/***************************************************************************************************
  Function:
    BkgdTaskIoCfgVerifyAndStore

  Description:
    This function is used to verify the received IO Configuration Data. For this purpose the 
    IO Configuration Data is verified against the received SCCRC (see [SRS_2131]). Also the
    configuration bytes for unsupported features are checked to be zero.

    In case the received data is ok and the SCCRC matches, the function:
      - triggers the flash process of raw IO Configuration Data
      - triggers the flash process of SCID via CSS internal function

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE   - Received IO Configuration Data is valid
    FALSE  - Received IO Configuration Data is invalid

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC BOOL BkgdTaskIoCfgVerifyAndStore(void)
{
  /* return value of function, indicates if SCCRC matches to Io Configuration Data */
  BOOL b_retVal;
  /* IO Configuration Data + SCID that was received from host application */
  AICMSGCFG_SETCONFIGSTRDATA s_setCfgStrData; 
  
  /* read SetConfigString data from aicMsgCfg */
  /* Attention data is accessed from background task and IRQ scheduler, so shall be locked */
  __disable_irq();
  s_setCfgStrData = *aicMsgCfg_GetData();
  __enable_irq();
  
  /* verify SCCRC against IO Configuration Data (see [SRS_2131]) */
  /* if SCCRC is false */
  if (BkgdTaskIoCfgVerify(&s_setCfgStrData) == FALSE)
  {
    /* set return value */
    b_retVal = FALSE;
  }
  /* else: SCCRC is correct */
  else
  {
    /* Write configuration data, see [SRS_2143]) */
    BkgdTaskIoCfgStore(&s_setCfgStrData);
    /* set return value */
    b_retVal = TRUE;
  }
  return b_retVal;
}


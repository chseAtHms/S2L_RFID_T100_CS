/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdCommon.c
**     Summary: This module implements the common function used by background task.
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdComIoCfgApply
**             SAPL_BkgdComActivityMonitor
**             SAPL_BkgdComSetSysTime
**             SAPL_BkgdComInitFaultCnt
**             SAPL_BkgdComTxFrameGenerate
**             SAPL_BkgdComRxProcessHalcMsg
**             SAPL_BkgdComDiscardHalcMsg
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
#include "globErrLog.h"

#ifdef GLOBFIT_FITTEST_ACTIVE
#include "globFit_FitTestHandler.h" /* FIT test commands */
#endif

#include "stHan_SelftestHandler-srv.h"

#include "rds.h"

#include "gpio_cfg.h"
#include "fiParam.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicSm.h"
#include "aicMgrTx.h"
#include "aicMsgCsal_cfg.h"
#include "aicMsgCsal.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* HALCS headers */
#include "HALCSapi.h"

/* CSS headers */
#include "IXUTLapi.h"
#include "IXSSOapi.h"
#include "IXSVOapi.h"
#include "IXSCEapi.h"
#include "IXSMRapi.h"
#include "IXSVSapi.h"
#include "IXSAIapi.h"
#include "IXSVCapi.h"

/* SAPL headers */
#include "TIM.h"
#include "SAPLcfg.h"
#include "SAPLipc.h"
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
#define k_FILEID      21u

/* this define is just there to avoid a warning when initializing the loop for
   running though all Safety Validators */
#define k_INSTANCE_ID_1                 1U


/** k_CNT_ERR_RESET_128US_TICKS
** According to VOL5 the Safety Connection Fault Count has to be reseted every hour.
** In order to use the 128us time base, the number of ticks is determined here.
*/
#define k_CNT_ERR_RESET_128US_TICKS     28125000U  /* every hour (60*60*10^6us / 128us) */

/* This variable indicates when the Safety Connection Fault Count was reseted last */
STATIC RDS_UINT32 u32_LastConnFaultCntResetTime128us;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

  
/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdComIoCfgApply

  Description:
    This function is used to apply a valid IO Configuration. For this purpose the function:
      - reads the current IO Configuration Data + SCID from non-volatile memory
      - calculates the SCCRC over IO Configuration Data 
      - compares calculated SCCRC with SCCRC stored inside non-volatile memory
      - sets IO Configuration Data inside pkgParam 
      - sets SCCRC inside pkgParam (for cyclic check)

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComIoCfgApply(void)
{
  /* Io Configuration Data */
  UINT8 au8_nvIoCfg[SAPL_k_SDS_CONF_DATA_IO];
  /* SCID */
  CSS_t_SCID s_nvScid;
  /* SCCRC */
  UINT32 u32_sccrc;
  
  /* read current IO Configuration Data from NV memory */
  SAPL_NvHighConfigRestore(au8_nvIoCfg);
  
  /* read current SCID from NV memory */
  /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
  if (SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_SCID,
                                    (CSS_t_UINT)sizeof(s_nvScid), 
                                    (CSS_t_BYTE*)&s_nvScid) != CSS_k_TRUE) /*lint !e928 */
  {
    /* NV memory error */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_APPL_ERR, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* check if calculated and stored CRC matches */
  else
  {
    /* get calculated SCCRC */
    u32_sccrc = fiParam_SccrcCompute(au8_nvIoCfg, SAPL_k_SDS_CONF_DATA_IO);
    
    /* if SCCRC matches */
    if (u32_sccrc == s_nvScid.u32_crc)
    {
      /* set IO Configuration Data + SCCRC inside pkgParam */
      fiParam_setIParam (au8_nvIoCfg, (UINT8)sizeof(au8_nvIoCfg));
      fiParam_setIParCrc(u32_sccrc);
    }
    /* else: SCCRC mismatch */ 
    else
    {
      /* unrecoverable NV error */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_APPL_ERR, GLOBFAIL_ADDINFO_FILE(2u));
    } 
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdComActivityMonitor

  Description:
    This function performs the Consumer Activity Monitoring [3.9-2:]. This checks the reception
    of IO Data Messages. Furthermore the generation of Time Coordination Messages are initiated
    here.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComActivityMonitor(void)
{
  /* loop counter for processing all Safety Validator Server instances */
  CSS_t_UINT u16_svInst;
  
  /* This function handles all Safety Validator Server Instances. */
  /* Attention: The function also responsible to generate the Time Coordination Messages */ 
  
  /* This function must be called cyclically to perform Consumer Activity Monitoring
  ** (see [SRS_2031]). If a timeout is detected, the Connection is 'faulted' which means
  ** that no further Time Coordination messages are produced by the CSS (see [SRS_2031]).
  */

  /* check all Safety Validator instances (see [3.9-2:])*/
  for (u16_svInst = k_INSTANCE_ID_1;
       u16_svInst <= (CSOS_cfg_NUM_OF_SV_SERVERS + CSOS_cfg_NUM_OF_SV_CLIENTS);
       u16_svInst++)
  {
    /* Check reception of IO Data Messages and initiate generation of Time Coordination Messages,
    ** has to be called periodically (see [3.9-2:]) */
    IXSVS_ConsInstActivityMonitor(u16_svInst);
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdComSetSysTime

  Description:
    This function sets the current system time of the CSS stack [3.15-1:] and handles the
    reset of the fault counters [3.7-2:].

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComSetSysTime(void)
{
  /* set application global timer variable */
  CSS_t_UDINT u32_time128us = TIM_TimeGet_128us();

  /* set global time in the CIP Safety Stack (see [3.15-1:]) */
  IXUTL_SetSysTime(u32_time128us);

  /* According to VOL5 the Safety Connection Fault Count has to be reseted every hour. The
  ** 128us tick is used for this purpose.
  */

  /* if an hour since last reset is over */
  if ( (u32_time128us - RDS_GET(u32_LastConnFaultCntResetTime128us))
        >= k_CNT_ERR_RESET_128US_TICKS )
  {
    /* clear Safety Connection Fault Count (see [3.7-2:]) */
    IXSVO_FaultCountersReset();
    /* restart timer */
    RDS_SET(u32_LastConnFaultCntResetTime128us, u32_time128us);
  }
  /* else: still waiting */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdComInitFaultCnt

  Description:
    This function initializes the timer used to reset the fault timers. This function is typically
    called once within startup.

  See also:
    -

  Parameters:
    u32_sysTime (IN)         - Current system time (128us ticks)
                               (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComInitFaultCnt(UINT32 u32_sysTime)
{
  /* init timer */
  RDS_SET(u32_LastConnFaultCntResetTime128us, u32_sysTime);
}

/***************************************************************************************************
  Function:
    SAPL_BkgdComTxFrameGenerate

  Description:
    This function is used to trigger the generation of new IO Data Messages. Therefore the
    Input Assembly is updated additionally here.
    Furthermore the reception of Time Coordination Messages are checked inside.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComTxFrameGenerate(void)
{
  /* loop counter for processing all Safety Validator Client instances */
  CSS_t_UINT u16_svInst;
  /* variable contains current Device Status */
  CSOS_t_SSO_DEV_STATUS e_deviceState;
  
  /* get current device state */
  e_deviceState = IXSSO_DeviceStatusGet();
  
  /* evaluate CSS state */
  switch (e_deviceState)
  {
    /* device state in executing or idle */
    case CSOS_k_SSO_DS_EXECUTING:     /* fall through */
    case CSOS_k_SSO_DS_IDLE:          /* fall through */
    {
      /* working copy of IO Data to be sent, copy is used to in order to lock IRQ scheduler only
      ** for a very short time   */
      AICMSGDEF_PS_INPUT_DATA_STRUCT s_workingCopyTxData;
      
      /** Attention: Data structure accessed from IRQ scheduler and background task. A local 
      ** working copy is made here during disabled scheduler *
      */
      __disable_irq();
      s_workingCopyTxData = aicMgrTx_s_IoVal;
      __enable_irq();
      
      /* pass data to be sent to CSS (see [3.3-2:], [3.3-5:]) */
      /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
      IXSAI_AsmIoInstDataSet(SAPL_IO_IN_ASSEMBLY_ID,                   
                             (const CSS_t_BYTE*)&s_workingCopyTxData,   /*lint !e928*/
                             (CSS_t_UINT)sizeof(s_workingCopyTxData)); 
      break;
    }
    /* states which do not update Input Assembly */
    case CSOS_k_SSO_DS_WAIT_TUNID:    /* fall through */
    case CSOS_k_SSO_DS_CONFIGURING:   /* fall through */
    case CSOS_k_SSO_DS_ABORT:         /* fall through */
    {
      /* empty branch, no update necessary */
      break;
    }
    /* states which should really not be reached here */
    case CSOS_k_SSO_DS_UNDEFINED:     /* fall through */
    case CSOS_k_SSO_DS_SELF_TESTING:  /* fall through */
    case CSOS_k_SSO_DS_SELF_TEST_EX:  /* fall through */
    case CSOS_k_SSO_DS_CRIT_FAULT:    /* fall through */
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(3u));
      break;
    }
    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
      break;
    }
  }
  
  /* Attention: The function 'IXSVC_InstTxFrameGenerate' is also responsible to check if 
  ** a Time Coordination Message was received in time (see [SRS_2184]).
  ** If no Time Coordination Message was received in time, the connection is 'faulted' 
  ** which means that no further IO Data Messages are produced by the CSS (see [SRS_2184]) */ 
  
  /* check all Safety Validator instances */
  for (u16_svInst = k_INSTANCE_ID_1;
       u16_svInst <= (CSOS_cfg_NUM_OF_SV_SERVERS + CSOS_cfg_NUM_OF_SV_CLIENTS);
       u16_svInst++)
  {
    /* Initiate generation of IO Data Messages and check reception of Time Coordination Messages,
    ** has to be called periodically (see [3.8-1:]) */
    IXSVC_InstTxFrameGenerate(u16_svInst);
  }
}



/***************************************************************************************************
  Function:
    SAPL_BkgdComRxProcessHalcMsg

  Description:
    This function checks if new HALC message was received. If so, the message is passed to the 
    proper module of the CSS.
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComRxProcessHalcMsg(void)
{
  /* read current HALC CSAL request for processing */
  const volatile AICMSGCSAL_t_AICMSG* ps_halcCsalMsg = aicMsgCsal_MsgGet();
  /* if new HalcCsalMessage received */
  if (ps_halcCsalMsg != NULL)
  {  
    /* HALCS message which shall be processed by CSS */
    HALCS_t_MSG s_rxHalcMsg;
    
    /* prepare HALC message header for CSS API call */
    s_rxHalcMsg.u16_cmd     = ps_halcCsalMsg->u16_cmd;
    s_rxHalcMsg.u16_len     = ps_halcCsalMsg->u16_len;
    s_rxHalcMsg.u32_addInfo = ps_halcCsalMsg->u32_addInfo;
    /* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away const/volatile
    ** from a pointer or reference 
    ** Note 926: cast from pointer to pointer [MISRA 2004 Rule 11.4]
    ** -->OK because data in au8_data is set via volatile access function stdlibHAL_ByteArrCopy,
    ** so data must be located in buffer */
    s_rxHalcMsg.pb_data     = (const CSS_t_BYTE*)ps_halcCsalMsg->au8_data; /*lint !e960, !e926*/
    
    /* switch for the unit handle in the command */
    switch (CSOS_UNIT_HANDLE_GET(s_rxHalcMsg.u16_cmd))
    {
      case CSOS_k_CSS_IXSCE:
      {
        /* Process ForwardOpen, ForwardClose (see [3.4-1:]) */
        IXSCE_CmdProcess(&s_rxHalcMsg);
        break;
      }
      case CSOS_k_CSS_IXSMR:
      {
        /* Process Explicit Message Requests (see [3.5-1:])*/
        IXSMR_CmdProcess(&s_rxHalcMsg);
        break;
      }
      case CSOS_k_CSS_IXSVO:
      {
        /* Since no I/O Data is expected over this interface, the data is ignored
        ** but logged inside Error Event log (see [SRS_2163]).
        */
        globErrLog_ErrorAdd(GLOBERRLOG_FILE(5u));
        break;
      }
      default:
      {
        /* Data is ignored but logged inside Error Event log (see [SRS_2163]). */
        globErrLog_ErrorAdd(GLOBERRLOG_FILE(6u));
        break;
      }
    }
    /* release message from queue */
    aicMsgCsal_MsgRelease();
  }
  /* else: no (new) message for processing */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    SAPL_BkgdComDiscardHalcMsg

  Description:
    This function discards a pending HALCS Message 
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComDiscardHalcMsg(void)
{
  /* release pending message (if available)  
  ** Since the function simply returns if no message is pending, it is OK to call this function 
  ** cyclically. */
  aicMsgCsal_MsgRelease();
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

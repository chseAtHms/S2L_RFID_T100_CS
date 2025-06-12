/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdInit.c
**     Summary: This module implements the background task functionality in the state
**              'INIT'.
**   $Revision: 4337 $
**       $Date: 2024-03-15 13:23:28 +0100 (Fr, 15 Mrz 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskInit
**
**             BkgdTaskCssInit
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

#include "aicMsgDef.h"
#include "aicSm.h"
#include "aicMgrTx.h"
#include "aicMsgInit.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* CSS headers */
#include "IXSSCapi.h"
#include "IXSAIapi.h"
#include "IXSSOapi.h"
#include "IXSSO.h"

/* SAPL headers */
#include "SAPLcfg.h"
#include "SAPLbkgdInt.h"
#include "TIM.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      23u


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void BkgdTaskCssInit(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    SAPL_BkgdTaskInit

  Description:
    This function is used to initialize the IXXAT CIP Safety Target Stack. It is required that the
    CSS initialization data were already received via 'SetInitData' request from AM.
    This means, the AIC state AICSM_AIC_START_CSS is expected here.
  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskInit(void)
{
  /* Current device status */
  CSOS_t_SSO_DEV_STATUS e_deviceStatus;
  /* pointer to the device configuration owner UNID */
  const CSS_t_UNID *ps_cfUnid;

  /* Attention: Since the AIC state is changed during IRQ scheduler afterwards, this function is
  ** only executed once.
  */
  
  /** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2002]).
  ** Attention: No processing of HALC Messages is done here, means no CIP (Ethernet/IP)
  ** communication possible (see [SRS_2127]).
  */

  /******************************************************************************/
  /* Execute state specific part                                                */
  /******************************************************************************/
  /* init CIP Safety Target Stack with received initialization data */
  BkgdTaskCssInit();
  
  /* read current device status */
  e_deviceStatus = IXSSO_DeviceStatusGet();

  /*
  ** This function is called only in AICSM_AIC_START_CSS. Thus, we can rule out that the IRQ
  ** scheduler is in parallel processing a configuration received via AIC. Thus no further
  ** precautions are necessary to set the configuration ownership here.
  */

  /* if CSS state is already 'IDLE', the device is configured because SCID != 0 */
  if (e_deviceStatus == CSOS_k_SSO_DS_IDLE)
  {
    /* The device is already configured, so the configuration is read from NV memory
    ** checked again (IO Configuration Data vs. SCCRC) and applied.
    */
    SAPL_BkgdComIoCfgApply();
    
    /* and request new AIC state IDLE afterwards (see [SRS_2004]) */
    /* Set Event 'CSS initialized' received, this also triggers the activation of the
    ** IO Configuration inside GPIO module (see [SRS_2155]) */
    aicSm_SetEvent(AICSM_k_EVT_CSS_INIT_IDLE);

    /* after CSS is now initialized get a pointer to the CF-UNID */
    ps_cfUnid = IXSSO_CfUnidPtrGet();

    /* if the Configuration Owner Attribute indicates the device is un-owned */
    if (IXSSO_UnidCompare(ps_cfUnid, &IXSSO_ks_ZeroUnid) == CSS_k_TRUE)
    {
      /* pass this information to the AIC state machine */
      aicSm_CfgOwnerToolSet((BOOL)FALSE);
      (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)FALSE);
    }
    /* else: if the Configuration Owner Attribute indicates the device is tool/host owned */
    else if (IXSSO_UnidCompare(ps_cfUnid, &IXSSO_ks_ffUnid) == CSS_k_TRUE)
    {
      /* pass this information to the AIC state machine (to allow re-configuration from host) */
      aicSm_CfgOwnerToolSet((BOOL)TRUE);
      (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)FALSE);
    }
    else  /* else: device is Originator-owned */
    {
       /* pass this information to the AIC state machine (prevent re-configuration from host) */
       aicSm_CfgOwnerToolSet((BOOL)FALSE);
      (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)TRUE);
    }
  }
  /* else: if CSS state is 'CONFIGURING' no configuration is set yet */ 
  else if (e_deviceStatus == CSOS_k_SSO_DS_CONFIGURING)
  {
    /* request new AIC state 'CONFIG' (see [SRS_2196] */
    aicSm_SetEvent(AICSM_k_EVT_CSS_INIT_WAIT_CONFIG);
    /* mark configuration as un-owned */
    aicSm_CfgOwnerToolSet((BOOL)FALSE);
    (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)FALSE);
  }
  /* else: if CSS state 'WAIT_TUNID', -> no TUNID is set */
  else if (e_deviceStatus == CSOS_k_SSO_DS_WAIT_TUNID)
  {
    /* request new AIC state 'WAIT_TUNID' (see [SRS_2195]) */
    aicSm_SetEvent(AICSM_k_EVT_CSS_INIT_WAIT_TUNID);
    /* mark configuration as un-owned */
    aicSm_CfgOwnerToolSet((BOOL)FALSE);
    (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)FALSE);
  }
  /* else: if CSS state 'Abort', -> invalid Node ID (ip address) is set */
  else if (e_deviceStatus == CSOS_k_SSO_DS_ABORT)
  {
    /* request new AIC state 'ABORT' (see [SRS_2207] */
    aicSm_SetEvent(AICSM_k_EVT_CSS_INIT_INVLD_NODEID);
    
    /* ATTENTION: If the received Node ID does not match to the TUNID 
    ** stored in NV memory, the CSS internal state 'Abort' shall be entered.
    ** This is done by the CSS itself (see [SRS_2208]).
    */
    /* As we are in Abort state CIP Safety will not set the configuration and herewith we make sure
       that AIC also cannot set a configuration in this state. */
    aicSm_CfgOwnerToolSet((BOOL)FALSE);
    (void)aicSm_CfgOwnerOriginatorGetSet((BOOL)TRUE);
  }
  /* else: Invalid state here */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  
  /* Discard pending HALC Message */
  SAPL_BkgdComDiscardHalcMsg();
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    BkgdTaskCssInit

  Description:
    This function is used to initialize the IXXAT CIP Safety Target Stack. 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskCssInit(void)
{
  /* parameter to pass to CSS stack */
  CSS_t_ID_INFO s_cssInitInfo;
  /* current system time */
  UINT32 u32_systemTime;
  
  /* working copy of IO Data used for initialization, copy is used to in order to lock IRQ 
  ** scheduler only for a very short time   */
  AICMSGDEF_PS_INPUT_DATA_STRUCT s_workingCopyTxData;
  /* working copy of CSS initialization data */
  CONST volatile AICMSGINIT_CSSINITDATA* ps_aicCssInitData;
  
  /* get current system time */
  u32_systemTime = TIM_TimeGet_128us();

  /* read CSS Initialization data form aicMsgHdl */
  /* Attention data is accessed from background task and IRQ scheduler, so shall be locked */
  __disable_irq();
  ps_aicCssInitData = aicMsgInit_GetData();
  
  /* copy data received from AM (SetInitData) request into CSS API structure */
  /* The Incarnation ID is deprecated and not used any more */
  s_cssInitInfo.u32_serNum        = ps_aicCssInitData->u32_serialNumber;
  /* only one NodeID required because of Single Safety Port Device,
  ** (see [SRS_2248]) */
  s_cssInitInfo.au32_nodeId[0]    = ps_aicCssInitData->u32_nodeId; 
  s_cssInitInfo.u16_vendorId      = ps_aicCssInitData->u16_vendorId;
  s_cssInitInfo.u16_devType       = ps_aicCssInitData->u16_deviceType;
  s_cssInitInfo.u16_prodCode      = ps_aicCssInitData->u16_productType;
  s_cssInitInfo.u8_majorRev       = ps_aicCssInitData->u8_majorRevision;       
  s_cssInitInfo.u8_minorRev       = ps_aicCssInitData->u8_minorRevision;
  
  /* enable IRQS again */
  __enable_irq();
  
  /* initialization of CSS with received parameters (see [SRS_2128]) before any other
  ** CSS API function is called (see [3.2-1:]) */
  IXSSC_Init(&s_cssInitInfo, (CSS_t_UDINT)u32_systemTime);
  
  /***********************************/
  /* setup of fixed Output Assembly  */
  /***********************************/
  /* setup of Output Assembly Object 0x300 (see [SRS_2102])
  ** fixed, neither dynamic allocation nor single bit access is possible
  ** (see [SRS_2081], [SRS_2082]). */
  IXSAI_AsmIoInstSetup(SAPL_IO_OUT_ASSEMBLY_ID, 
                       SAPL_IO_OUT_ASSEMBLY_LEN,
                       IXSAI_k_ASM_OUTPUT);
   
  /***********************************/ 
  /* setup of fixed Input Assembly   */
  /***********************************/             
  /* setup of Input Assembly Object 0x264 (see [SRS_2093])
  ** fixed, neither dynamic allocation nor single bit access is possible
  ** (see [SRS_2081], [SRS_2082]). */
  IXSAI_AsmIoInstSetup(SAPL_IO_IN_ASSEMBLY_ID, 
                       SAPL_IO_IN_ASSEMBLY_LEN,
                       IXSAI_k_ASM_INPUT);
  
  /* set time (128us ticks), so start the Safety Connection Fault Count reset time */ 
  SAPL_BkgdComInitFaultCnt(u32_systemTime);
  
  /** Attention: Data structure accessed from IRQ scheduler and background task. A local 
  ** working copy is made here during disabled scheduler *
  */
  __disable_irq();
  s_workingCopyTxData = aicMgrTx_s_IoVal;
  __enable_irq();
                       
  /* Set the Run/Idle flag of the Producing Connection to 'Run' (see [SRS_2137], [3.3-4:]). 
  ** The Run/Idle flag is only under control of the Safety Application while the Safety 
  ** Supervisor is in Executing state. Otherwise the Run/Idle flag is forced to Idle.
  ** Due this it is OK calling IXSAI_AsmIoInstDataRunModeSet once during initialization. */
  IXSAI_AsmIoInstDataRunModeSet(SAPL_IO_IN_ASSEMBLY_ID, CSS_k_TRUE);

  /* pass data to be sent to CSS (see [3.3-2:]) */
  /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
  IXSAI_AsmIoInstDataSet(SAPL_IO_IN_ASSEMBLY_ID,
                         (const CSS_t_BYTE*)&s_workingCopyTxData, /*lint !e928 */
                         (CSS_t_UINT)sizeof(s_workingCopyTxData));
}

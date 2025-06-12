/***************************************************************************************************
**    Copyright (C) 2015-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLcbfk.c
**     Summary: This module implements the callback functions called by the CIP Safety Target
**              Stack.
**   $Revision: 4693 $
**       $Date: 2024-11-06 11:18:57 +0100 (Mi, 06 Nov 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_IxsaiIoDataRxClbk
**             SAPL_IxssoProfDepStateChangeClbk
**             SAPL_IxsceSafetyOpenValidateClbk
**             SAPL_IxsceCfgApplyClbk
**             SAPL_IxsceCompElectronicKeyClbk
**             SAPL_CssErrorClbk
**             SAPL_IxsvsEventHandlerClbk
**             SAPL_IxsvcEventHandlerClbk
**             SAPL_IxssoDeviceSelfTestClbk
**             SAPL_CssProduceEvenBytesClbk
**             SAPL_ConfigStringCheck
**
**             CheckAicState
**             ValidateType1
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

#include "gpio_cfg.h"
#include "fiParam.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicMsgErr.h"
#include "aicSm.h"

#include "SpduOutData.h"

#include "globErrLog.h"
#include "version.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSS.h"
#include "CSSapi.h"

/* HALCS headers */
#include "HALCSapi.h"

/* CSS headers */
#include "IXSAIapi.h"
#include "IXSVSapi.h"
#include "IXSSOapi.h"
#include "IXSCEapi.h"
#include "IXSVCapi.h"
#include "IXSERapi.h"

/* SAPL headers */
#include "SAPLcfg.h"
#include "SAPLnvHigh.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      17u


/* This extended status code shall be returned as the result of specifying an O->T connection type
** that is not supported, see Volume 1 (Edition 3.20) section 3-5.5,
** 'INVALID ORIGINATOR TO TARGET NETWORK CONNECTION TYPE' */
#define k_CMEXTSTAT_INV_OT_NETW_CONN_TYPE  0x0123u

/* This extended status code shall be returned as the result of specifying a T->O connection type
** that is not supported, see Volume 1 (Edition 3.20) section 3-5.5,
** 'INVALID TARGET TO ORIGINATOR NETWORK CONNECTION TYPE' */
#define k_CMEXTSTAT_INV_TO_NETW_CONN_TYPE  0x0124u

/* This extended status code is returned as the result of specifying an invalid configuration length
** in the SafetyOpen service, see Volume 1 (Edition 3.35) section 3-5.7.
** The new code Invalid Connection Size would match better, but requires to send another additional
** Status Word. SAPL_IxsceSafetyOpenValidateClbk() doesn't allow to send this.
** 'INVALID CONNECTION SIZE' */
#define k_CMEXTSTAT_INV_CNXN_SIZE          0x0109u

/* This extended status code is returned as the result of specifying inconsistent configuration
** data - specifically in relation to the safe consume data.
** See Volume 1 (Edition 3.35) section 3-5.7, 'INCONSISTENT CONSUME DATA FORMAT' */
#define k_CMEXTSTAT_INC_CONS_DATA_FRMT     0x0130u

/* This constant defines the size of the Type 1 SafetyOpen configuration header (Software Major
** Version, Hardware ID) */
#define k_CFG_HDR_SIZE                     (CSOS_k_SIZEOF_USINT+CSOS_k_SIZEOF_UINT)

/* The whole configuration (Header + I/O config) must haven an even length (as SafetyOpen transports
** configuration data in WORDs). */
#if (((AICMSGHDL_IOCONFIGDATA_LEN + k_CFG_HDR_SIZE) & 0x0001U) == 1U)
  #error Configuration Error: Length of configuration string must be an even number of bytes
#else
  /* length is ok */
#endif

/* This constant defines the size of the expected length of the Type1 configuration data (in
** 16-Bit Words) */
#define k_EXPECTED_CFG_SIZE_W              ((AICMSGHDL_IOCONFIGDATA_LEN + k_CFG_HDR_SIZE) / 2U)


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC UINT16 CheckAicState(CSS_t_BOOL o_type1SafetyOpen);
STATIC UINT16 ValidateType1(const CSS_t_SOPEN_PARA *ps_safetyOpenParams);

/***************************************************************************************************
**    global functions
***************************************************************************************************/


/*******************************************************************************
**
** Function    : SAPL_IxsaiIoDataRxClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case an output
**               assembly object instance has received new data. The safety
**               application has to store this event in its own new data flag
**               (see FRS131). The safety application shall then call
**               {IXSAI_AsmIoInstDataGet()} to copy the new data to the
**               application.
**
** Context     : IXSVO_CmdProcess()
**
** See Also    : IXSAI_AsmIoInstDataGet()
**
** Parameters  : u16_instId (IN) - Instance ID the assembly object that has new
**                                 data.
**
** Returnvalue : -
**
*******************************************************************************/
void SAPL_IxsaiIoDataRxClbk(CSS_t_UINT u16_instId)
{
  /* Callback required according to [CS_MAN], (see [3.3-6:]) */

  /* pointer to received data */
  const CSS_t_BYTE *pau8_asmData;
  /* length of received data */
  CSS_t_UINT u16_len;

  /* Since the 'new data' is immediately read via calling IXSAI_AsmIoInstDataGet(),
  ** no additional 'new data flag' according to FRS131 (see function description)
  ** is necessary in this implementation.
  */

  /* get the received IO data from the CSS, (see [3.3-8:]) */
  pau8_asmData = IXSAI_AsmIoInstDataGet(u16_instId,      /* assembly instance */
                                        &u16_len);       /* length of assembly */

  /* if data not valid (e.g. instance is Idle) */
  if (pau8_asmData == CSS_k_NULL)
  {
    /* assembly instance is not in "run" mode, so request passivation */
    __disable_irq();
    spduOutData_e_IsRunMode = eFALSE;
    __enable_irq();
  }
  /* else: invalid length received */
  else if (u16_len != sizeof(AICMSGDEF_PS_OUT_DATA_STRUCT))
  {
    /* call globFail_SafetyHandler, never return from this... */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: data valid */
  else
  {
    /* pass data of received IO Data Message to output module (see [SRS_2014], [3.3-8:]) */

    /* Note 927: cast from pointer to pointer [MISRA 2004 Rule 11.4] */
    /* Info 826: Suspicious pointer-to-pointer conversion (area too small) [possibly violation
    ** MISRA 2004 Rule 11.4]
    ** Deactivated because length of received data (u16_len) checked, and transfer from buffer
    ** to output structure necessary here.
    */
    spduOutData_Set((CONST AICMSGDEF_PS_OUT_DATA_STRUCT*)pau8_asmData); /*lint !e927, !e826*/
  }
}


/*******************************************************************************
**
** Function    : SAPL_IxssoProfDepStateChangeClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when there is an
**               internal request for a change of the Safety Supervisor Device
**               State (Instance Attribute 11 - Device Status) between IDLE and
**               EXECUTING. The conditions under which such a state change may
**               occur is defined by the device profile. Thus the return value
**               of this function determines if the state changes from IDLE to
**               EXECUTING or from EXECUTING to IDLE. Additionally the Safety
**               Supervisor Object Device state can be influenced by means of
**               the function IXSSO_ModeChange().
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** Parameters  : e_devState (IN)        - current Device State (IDLE or
**                                        EXECUTING), see definition of
**                                        CSOS_t_SSO_DEV_STATUS
**               u8_event (IN)          - event that is currently being
**                                        processed by the Safety Supervisor
**                                        Object. See definition of
**                                         - IXSSO_k_SE_SCNXN_FAIL
**                                         - IXSSO_k_SE_SCNXN_ESTAB
**                                         - IXSSO_k_SE_SCNXN_DEL
**               u16_numEstabCnxns (IN) - number of connections that are
**                                        currently in the established state
**
** Returnvalue : CSS_k_TRUE     - Safety Application permits state change
**               CSS_k_FALSE    - Safety Application doesn't permit state change
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoProfDepStateChangeClbk(CSOS_t_SSO_DEV_STATUS e_devState,
                                            CSS_t_USINT u8_event,
                                            CSS_t_UINT u16_numEstabCnxns)
{
  /* Callback required according to [CS_MAN], (see [3.6-8:]) */

  /* return value of this function */
  CSS_t_BOOL b_retVal = CSS_k_FALSE;

  /* This function is used to implement the necessary checks to permit or
  ** prohibit a state change between idle and executing. The behavior is
  ** completely profile dependent.
  ** Since the Safety Discrete I/O Device profile is implemented, the relevant
  ** conditions are listed in Table 6-6.4
  ** (Safety Supervisor Profile-dependent State Event behavior)
  */

  /* if current state is "IDLE", see ("IDLE" State behavior) */
  if (e_devState == CSOS_k_SSO_DS_IDLE)
  {
    /* According to profile: "In this profile, device is in IDLE unless at least one
    ** standard or Safety I/O connection is established"
    */

    /* if "Safety I/O Connection established" */
    if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
    {

      /* if the first connection was established */
      if (u16_numEstabCnxns == 1U)
      {
        /* permit transition to executing */
        b_retVal = CSS_k_TRUE;
      }
      /* else: second connection */
      else
      {
        /* shall not occur because the first connection should trigger a switch
        ** to executing state...
        */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
      }
    }
    /* else if "Safety Connection Failed/Closed" */
    else if ((u8_event == IXSSO_k_SE_SCNXN_FAIL) ||
             (u8_event == IXSSO_k_SE_SCNXN_DEL))
    {
      /* if no Safety IO Connection is established */
      if (u16_numEstabCnxns == 0U)
      {
        /* remain in IDLE */
      }
      /* else: still a Safety Connection is established */
      else
      {
        /* shall not occur because executing state is only left if no IO Connections
        ** are established any more */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(13u));
      }
    }
    /* else: other events */
    else
    {
      /* shall not occur because all defined events are tested previously...*/
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
    }
  }

  /* else if current state is "EXECUTING", see ("Executing" State behavior) */
  else if (e_devState == CSOS_k_SSO_DS_EXECUTING)
  {
    /* if "Safety Connection Failed/Closed" */
    if ((u8_event == IXSSO_k_SE_SCNXN_FAIL) ||
        (u8_event == IXSSO_k_SE_SCNXN_DEL))
    {
      /* if any safety I/O connection is still open */
      if (u16_numEstabCnxns == 1U)
      {
        /* remain in EXECUTING */
      }
      /* else if no safety I/O connection is open any more */
      else if (u16_numEstabCnxns == 0U)
      {
        /* permit transition to IDLE */
        b_retVal = CSS_k_TRUE;
      }
      /* else: unexpected number of open safety I/O connections */
      else
      {
        /* shall not occur because only two connections are possible and at least one
        ** is closed.
        */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(12u));
      }
    }
    /* else if: "Safety I/O Connection established" */
    else if (u8_event == IXSSO_k_SE_SCNXN_ESTAB)
    {
      /* if the second connection was established */
      if (u16_numEstabCnxns == 2U)
      {
        /* stay in EXECUTING state */
      }
      /* else: 'other' connection */
      else
      {
        /* shall not occur because only two connections are possible in
        ** executing state
        */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
      }
    }

    /* else: other events */
    else
    {
      /* shall not occur because all defined events are tested previously...*/
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
    }
  }

  /* else: other state than "IDLE" or "EXECUTING" */
  else
  {
    /* shall not occur because callback function is only called in states IDLE
    ** and EXECUTING.
    */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(6u));
  }

  return (b_retVal);
}

/*******************************************************************************
**
** Function    : SAPL_IxsceSafetyOpenValidateClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case a SafetyOpen
**               service has been received and was successfully checked. By
**               applying further application specific criteria it is now up to
**               the application to decide if it wants to accept the safety
**               connection. This decision may be based on the passed parameter
**               structure and the state of the application (e.g. device is
**               configured) (see Req.5.3.1-5).
**               The application must also copy the configuration data (of a
**               Type 1 SafetyOpen) and validate it with SIL 3 integrity (see
**               SRS37), but not yet save it to NV-memory.
**
**               **Attention**:
**               CSS doesn't check the length of the safe I/O data in the
**               SafetyOpen. It is the duty of the safe application to check if
**               the requested size (u8_payloadSize) matches with the addressed
**               assembly instance (which was defined in the call of
**               IXSAI_AsmIoInstSetup()). For this comparison it must be
**               considered:
**               - The payload length of the connection may be equal or less
**                 than the assembly length.
**                 - If a connection consumes less than the length of the output
**                   assembly then the rest is filled up with zeros. If the
**                   connection consumes more than the assembly length this will
**                   lead to a fail safe error on first data consumption.
**                 - A connection may produce less than the length of the input
**                   assembly. If the connection is to produce more than the
**                   assembly length this will lead to a fail safe error on
**                   first data production.
**
**               **Attention**:
**               CSS doesn't check the Configuration Assembly Instance ID. It is
**               the duty of the safe application to restrict the
**               ps_safetyOpenParams->u16_cfgInst to valid Configuration
**               Assembly Instances.
**
** Context     : IXSCE_CmdProcess()
**
** See Also    : CSS_t_SOPEN_PARA
**
** Parameters  : ps_safetyOpenParams (IN) - pointer to structure containing the
**                                          parameters received with the
**                                          SafetyOpen service, possibly with
**                                          safety configuration data
**               u8_payloadSize (IN)      - number of bytes of safe payload data
**                                          to be transported by the safety
**                                          connection (T->O for Safety
**                                          Validator Clients, O->T for Safety
**                                          Validator Servers)
**
** Returnvalue : CSS_k_OK                 - Application grants the permission to
**                                          accept the connection request.
**               <>CSS_k_OK               - Application denies the connection
**                                          request. Returned value will be
**                                          responded in the Extended Status of
**                                          the Unsuccessful Forward_Open
**                                          Response.
**
*******************************************************************************/
CSS_t_WORD SAPL_IxsceSafetyOpenValidateClbk(
                       const CSS_t_SOPEN_PARA *ps_safetyOpenParams,
                       CSS_t_USINT u8_payloadSize)
{
  /* Callback required according to [CS_MAN], (see [3.4-2:]) */

  /* Whenever this callback function is called, the CSS has already verified
  ** the following initialization parameters (see [SIS_CS] as part of the
  ** ElectronicKey check (IXSCE_ElectronicKeyMatchCheck() )or the TUNID check
  ** (TunidMatchCheck()), (see [SRS_2058], [SRS_2151], [SRS_2036]):
  ** - Vendor ID
  ** - Product Code
  ** - Device Type
  ** - Major Revision, Minor Revision
  ** - Node ID
  ** The following initialization parameters are not part of the Safety function and
  ** can not be checked:
  ** - Serial Number
  **
  ** Since we reject the connection even if Compatibility Bit is set (see
  ** SAPL_IxsceCompElectronicKeyClbk(), we can be sure that parameter are OK.
  */

  /* return value of this function */
  CSS_t_WORD u16_ret;
  /* type of SafetyOpen (CSS_k_TRUE in case of Type 1) */
  CSS_t_BOOL o_type1;

  /* if this SafetyOpen received configuration data */
  if (ps_safetyOpenParams->u8_cfgDataSizeW > 0U)
  {
    /* then it is a Type 1 SafetyOpen */
    o_type1 = CSS_k_TRUE;
  }
  else  /* else: no configuration data */
  {
    /* else it is a Type 2 SafetyOpen */
    o_type1 = CSS_k_FALSE;
  }

  /* check if SafetyOpen is accepted in AIC state */
  u16_ret = CheckAicState(o_type1);

  /* if Safety Open is received in invalid AIC state */
  if (u16_ret != CSS_k_OK)
  {
    /* empty branch, return already set */
  }
  /* if the Configuration Assembly Instance doesn't meet SAPL's expectation
  ** (see [SRS_2120]) or NULL Instance */
  else if ( (ps_safetyOpenParams->u16_cfgInst != SAPL_IO_CFG_ASSEMBLY_ID) &&
            (ps_safetyOpenParams->u16_cfgInst != CSS_cfg_ASM_NULL_INST_ID) )
  {
    u16_ret = CSS_k_CMEXTSTAT_CONFIG_PATH;
  }
  /* else if the received SafetyOpen is a Type 1 SafetyOpen */
  else if (o_type1)
  {
    /* perform Type 1 SafetyOpen specific validation checks */
    u16_ret = ValidateType1(ps_safetyOpenParams);
  }
  else  /* else: received SafetyOpen is a Type 2 SafetyOpen */
  {
    /* common SafetyOpen checking continues right below */
  }

  /* if there was already an error set before */
  if (u16_ret != CSS_k_OK)
  {
    /* skip further checks, return value already set */
  }
  /* else if multi-cast connection requested for (T->O) */
  else if ( CSS_IsMultiCast(ps_safetyOpenParams->w_netCnxnPar_TO) )
  {
    /* multi-cast connections shall not be supported, so blocked here, (see [SRS_2071]))
    ** return 'INVALID TARGET TO ORIGINATOR NETWORK CONNECTION TYPE' means,
    ** This extended status code shall be returned as the result of specifying a T->O
    ** connection type that is not supported. */
    u16_ret = k_CMEXTSTAT_INV_TO_NETW_CONN_TYPE;
  }
  /* else if multi-cast connection requested for (O->T) */
  else if ( CSS_IsMultiCast(ps_safetyOpenParams->w_netCnxnPar_OT) )
  {
    /* multi-cast connections shall not be supported, so blocked here, (see [SRS_2071]))
    ** return 'INVALID ORIGINATOR TO TARGET NETWORK CONNECTION TYPE' means,
    ** This extended status code shall be returned as the result of specifying an O->T
    ** connection type that is not supported. */
    u16_ret = k_CMEXTSTAT_INV_OT_NETW_CONN_TYPE;
  }
  /* else: received SafetyOpen is single cast */
  else
  {
    /* Attention: The CSS itself checks that the received SCID matches to the one stored
    ** inside NV memory (see [SRS_2206], [SRS_2043], [SRS_2046]) and returns
    ** 'SCID Mismatch' if the SCID does not fit (see [SRS_2156]).
    **
    ** The CSS also handles the acceptance of SCID=0, see [SRS_2157].
    ** The SCID = 0 in a Safety Open Type 2 has the special meaning according to specification
    ** that the SCID shall not be checked. This allows originators who are not concerned about
    ** the configuration to make connections without knowing the SCID.
    */

    /* if requested RPI is less than the min. supported RPI, the connection establishment
    ** shall be blocked (see [SRS_2147])
    */
    if ( (ps_safetyOpenParams->u32_rpiOT_us < SAPL_cfg_MIN_RPI_SUPPORTED) ||
         (ps_safetyOpenParams->u32_rpiTO_us < SAPL_cfg_MIN_RPI_SUPPORTED) )
    {
      /* Requested RPI not supported Return 'RPI not supported' */
      u16_ret = CSS_k_CMEXTSTAT_RPI_NOT_SUP;
    }
    /* else: RPI valid */
    else
    {

      /* CSS doesn't check the length of the safe I/O data in the SafetyOpen. It is the duty of
      ** the safe application to check if the requested size (in w_netCnxnPar_OT and
      ** w_netCnxnPar_TO) matches with the addressed assembly instance (which was defined
      ** in the call of IXSAI_AsmIoInstSetup()).
      */

      /* For a definition of the bits of the Transport Class and Trigger see
       Volume 1 (Edition 3.20) section 3-4.4.3 */

      /* if we are consumer (check direction bit for 'Server') */
      if (ps_safetyOpenParams->b_transportTypeTrigger & 0x80U)
      {
        /* if requested connection size doesn't match with output assembly length */
        if (u8_payloadSize != SAPL_IO_OUT_ASSEMBLY_LEN)
        {
          /* return 'Safety Connection Size Invalid' */
          u16_ret = CSS_k_CMEXTSTAT_SCNXN_SIZE;
        }
        /* else: connection size and output assembly are equal */
        else
        {
          /* accept connection */
          u16_ret = CSS_k_OK;
        }
      }
      /* else: then we are producer */
      else
      {
        /* if requested connection size doesn't match with input assembly length */
        if (u8_payloadSize != SAPL_IO_IN_ASSEMBLY_LEN)
        {
          /* return 'Safety Connection Size Invalid' */
          u16_ret = CSS_k_CMEXTSTAT_SCNXN_SIZE;
        }
        /* else: connection size and input assembly are equal */
        else
        {
          /* accept connection */
          u16_ret = CSS_k_OK;
        }
      }
    }
  }
  return (u16_ret);
}


/*******************************************************************************
**
** Function    : SAPL_IxsceCfgApplyClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function during processing of
**               a Type 1 SafetyOpen. It is the job of this function
**               to save pending application configuration data to NV memory.
**
** Context       IXSCE_CmdProcess()
**
** Parameters  : -
**
** Returnvalue : CSS_k_TRUE       - Application successfully saved Config data
**                                  to NV memory
**               CSS_k_FALSE      - Applying Configuration data failed:
**                                  SafetyOpen will be rejected, Safety
**                                  Supervisor state machine will stay in
**                                  "Configuring" state.
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxsceCfgApplyClbk(void)
{
  /* Callback required according to [CS_MAN], (see [3.4-3:]) */

  CSS_t_BOOL o_retVal = CSS_k_FALSE;         /* return value of this function */
  UINT32 u32_sccrc;                          /* received SCCRC value */
  /* get a pointer to temporary storage of the received configuration data */
  CSS_t_BYTE *pb_cfgData = fiParam_getCandidateIParam(&u32_sccrc);

  /* if the temporary stored configuration is consistent */
  if (fiParam_checkCandidateIParam() == (BOOL)TRUE)
  {
    /* We must take care about concurrent access to configuration ownership via
    ** AIC (from IRQ scheduler). */
    /* if we can set the configuration ownership to indicate Originator owner */
    if (aicSm_CfgOwnerOriginatorGetSet((BOOL)TRUE))
    {
      /* Set Event 'Config received' */
      aicSm_SetEvent(AICSM_k_EVT_CONFIG_RECEIVED);

      /* store IO Configuration Data to NV memory, see [SRS_2143] */
      SAPL_NvHighConfigStore(pb_cfgData);

      /* Set Event 'Config done' */
      aicSm_SetEvent(AICSM_k_EVT_CONFIG_DONE);

      /* When we return CSS will store the OUNID received in the SafetyOpen as
      ** our new CFUNID [SRS_2252] */

      /* indicate success */
      o_retVal = CSS_k_TRUE;
    }
    else  /* else: configuration ownership has been snatched away from us */
    {
      /* AIC is also right now working on the configuration */
      /* return value already set */
    }
  }
  else  /* else: SCCRC mismatch */
  {
    /* unrecoverable NV error */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_APPL_ERR,
                            GLOBFAIL_ADDINFO_FILE(1u));
    /* return value already set */
  }

  /* At this point we must assume that storing the configuration in background
     will be successful. In case that assumption turns out to be wrong later:
      - we can continue operation with the configuration we have now in RAM
        (SCCRC was successfully checked before)
      - check of configuration (SCCRC check) will then fail on next power-up
  */

  return (o_retVal);
}

/*******************************************************************************
**
** Function    : SAPL_IxsceCompElectronicKeyClbk
**
** Description : This callback function is called when an explicit Message
**               request (e.g. SafetyOpen) has been received with an electronic
**               key that does not match our device but with the Compatibility
**               Bit set. A compatibility set to 1 allows the device to accept
**               any key which the device can emulate. The default behavior is
**               to reject the connection.
**
** Context     : IXSCE_CmdProcess()
**
** See Also    : -
**
** Parameters  : ps_elKey (IN) - pointer to structure containing the received
**                               electronic key
**
** Returnvalue : CSS_k_TRUE    - Key matches, accept the connection
**               CSS_k_FALSE   - Key mismatch, reject the connection
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxsceCompElectronicKeyClbk(const CSS_t_ELECTRONIC_KEY *ps_elKey)
{
  /* Callback required according to [CS_MAN], (see [3.4-4:]) */

  /* just to avoid warnings */
  (void)ps_elKey;

  /* reject the connection even is Compatibility Bit is set */
  return (CSS_k_FALSE);
}


/*******************************************************************************
**
** Function    : SAPL_CssErrorClbk
**
** Description : This callback function must be provided by the Safety
**               Application. The function is called by the CIP Safety Stack
**               to signal an internal error of the Stack (see Req.2.5.1.1-1
**               and Req.10.4.1-4).
**
**               **Attention**: If the error type (see IXSER_GET_ERROR_TYPE) is
**               IXSER_TYPE_FS then the application must switch the device in
**               the fail safe state. This function shall then not return and
**               shall not call any further CSS functions. This state shall be
**               kept until the device is restarted. Further platform specific
**               actions may be necessary (e.g. triggering a hardware watchdog,
**               etc.). If the Safety Application wants to leave this state it
**               is recommended to emulate a power cycle as closely as possible.
**
** Context     : to be expected while any API call
**
** See Also    : IXSER_ErrorStrGet()
**
** Parameters  : w_errorCode (IN)  - 16 bit value that holds Error Type, Error
**                                   Layer, Unit ID and Unit Error.
**               u16_instId (IN)   - instance Id (e.g. of the safety validator
**                                   instance that caused the error), usage is
**                                   error code specific
**               dw_addInfo (IN)   - additional error information.
**
** Returnvalue : -
**
*******************************************************************************/
void SAPL_CssErrorClbk(CSS_t_WORD w_errorCode,
                       CSS_t_UINT u16_instId,
                       CSS_t_DWORD dw_addInfo)
{

  /* Callback required according to [CS_MAN], (see [3.13-1:]) */

  (void)u16_instId;    /* just to avoid warnings */
  (void)dw_addInfo;    /* just to avoid warnings */

  /* if this is a Fail-Safe Error */
  if (IXSER_GET_ERROR_TYPE(w_errorCode) == IXSER_TYPE_FS)
  {
    /*
    ** Switch the application and all I/O to safe state !!! (see [SRS_2193], [3.13-2:])
    ** Signal error to AM via AIC message. According to the existing T100 concept,
    ** all fail safe errors of the CSS are mapped to a single T100/CS error code,
    ** (see [SRS_2162]). The CSS error code is provided as additional info.
    ** The device safe state is NOT left until a power-cycle is done [1.4-1:]
    */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_CSS_FATAL, w_errorCode);  /* [SRS_2029] */
  }
  /* else if: Non Fail-Safe Error */
  else if (IXSER_GET_ERROR_TYPE(w_errorCode) == IXSER_TYPE_NOT_FS)
  {
    /* only a Non-Fail-Safe error - continue operation but insert error code
    ** in Error Event log' (see [SRS_2163]).
    */
    globErrLog_ErrorAdd(w_errorCode);
  }
  /* else: variable error */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(7u));
  }
}


/*******************************************************************************
**
** Function    : SAPL_IxsvsEventHandlerClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case the state of
**               a consuming Safety Connection changes. With this function the
**               application can initiate application specific reaction to
**               consuming Safety Connection events. Such events can be:
**               - opening/closing a connection
**               - consumer of safety data failed
**               - Run/Idle Flag changed
**
**               **Attention**:
**               For the following events the application has to make sure the
**               output is set to the safe state:
**               - IXSVS_k_VE_SERVER_CLOSE
**               - IXSVS_k_VE_SERVER_FAULTED (see FRS12-3)
**               - IXSVS_k_VE_SERVER_IDLE (see FRS276)
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxsvcEventHandlerClbk()
**
** Parameters  : u16_instId (IN) - instance ID of the associated Safety
**                                 Validator object
**               e_event (IN)    - Unique code describing the kind of event. See
**                                 type definition of {IXSVS_t_VALIDATOR_EVENT}.
**
** Returnvalue : -
**
*******************************************************************************/
void SAPL_IxsvsEventHandlerClbk(CSS_t_UINT u16_instId,
                                IXSVS_t_VALIDATOR_EVENT e_event)
{

  /* Callback required according to [CS_MAN], (see [3.9-3:]) */

  /* if invalid instance id, valid range, 1..(CSOS_cfg_NUM_OF_SV_CLIENTS +
  ** CSOS_cfg_NUM_OF_SV_SERVERS) */
  if ( (u16_instId == 0u) ||
       (u16_instId > (CSOS_cfg_NUM_OF_SV_CLIENTS + CSOS_cfg_NUM_OF_SV_SERVERS)) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(14u));
  }
  /* else: valid instance id */
  else
  {
    switch (e_event)
    {
      case IXSVS_k_VE_SERVER_OPEN:
      {
        /* SV server opened */

        /* Consuming connection is established (see [SRS_2006]) */
        aicSm_SetEvent(AICSM_k_EVT_CONS_CONN_ESTABLISHED);
        break;
      }

      case IXSVS_k_VE_SERVER_CLOSE:
      {
        /* SV server closed */

        /* Ensure safe state of the outputs (see [SRS_2167]). This 'Passivation' request is
        ** signalized via global variable 'spduOutData_e_IsRunMode' and handled in the
        ** AIC states AICSM_AIC_EXEC_CONS_ONLY and AICSM_AIC_EXEC_PROD_CONS
        ** (see aicMgrRx_GetOutputData() and spduOutData_Process())
        */
        __disable_irq();
        spduOutData_e_IsRunMode = eFALSE;
        __enable_irq();

        /* Since the Consuming connection is faulted, this also means that the output shall
        ** be set into safe state (see [SRS_2167]). This is also ensured by the global AIC
        ** state machine because the event 'AICSM_k_EVT_CONS_CONN_CLOSED' forces the transition
        ** into AICSM_AIC_EXEC_PROD_ONLY or AICSM_AIC_IDLE. In these states the outputs are not
        ** active (see aicMgrRx_GetOutputData().
        */
        /* Consuming connection is closed/faulted (see [SRS_2136]) */
        aicSm_SetEvent(AICSM_k_EVT_CONS_CONN_CLOSED);
        break;
      }

      case IXSVS_k_VE_1ST_DATA_REC:
      {
        /* SV server rec 1st data */

        break;
      }

      case IXSVS_k_VE_SERVER_FAULTED:
      {
        /* SV server failed */

        /* Ensure safe state of the outputs (see [SRS_2167], [SRS_2031]). This 'Passivation'
        ** request is signalized via global variable 'spduOutData_e_IsRunMode' and handled in the
        ** AIC states AICSM_AIC_EXEC_CONS_ONLY and AICSM_AIC_EXEC_PROD_CONS
        ** (see aicMgrRx_GetOutputData() and spduOutData_Process())
        */
        __disable_irq();
        spduOutData_e_IsRunMode = eFALSE;
        __enable_irq();

        /* Since the Consuming connection is faulted, this also means that the output shall
        ** be set into safe state (see [SRS_2167]). This is also ensured by the global AIC
        ** state machine because the event 'AICSM_k_EVT_CONS_CONN_CLOSED' forces the transition
        ** into AICSM_AIC_EXEC_PROD_ONLY or AICSM_AIC_IDLE. In these states the outputs are not
        ** active (see aicMgrRx_GetOutputData().
        */
        /* Consuming connection is closed/faulted (see [SRS_2136]) */
        aicSm_SetEvent(AICSM_k_EVT_CONS_CONN_FAULTED);
        break;
      }

      case IXSVS_k_VE_SERVER_IDLE:
      {
        /* SV server Idle */

        /* The Consuming Connection is still active but the Originator has requested the
        ** 'Passivation' mode via 'Idle'. This means that the outputs shall be set into
        ** safe state (see [SRS_2167]). The 'Passivation' request is signalized via global
        ** variable 'spduOutData_e_IsRunMode' and handled in the AIC states
        ** AICSM_AIC_EXEC_CONS_ONLY and AICSM_AIC_EXEC_PROD_CONS
        ** (see aicMgrRx_GetOutputData() and spduOutData_Process())
        */

        /* 'Idle', request for processing */
        __disable_irq();
        spduOutData_e_IsRunMode = eFALSE;
        __enable_irq();
        break;
      }

      case IXSVS_k_VE_SERVER_RUN:
      {
        /* SV server Run */

        /* 'Run', request for output processing, handled in the AIC states
        ** AICSM_AIC_EXEC_CONS_ONLY and AICSM_AIC_EXEC_PROD_CONS
        ** (see aicMgrRx_GetOutputData() and spduOutData_Set())
        */
        __disable_irq();
        spduOutData_e_IsRunMode = eTRUE;
        __enable_irq();
        break;
      }

      case IXSVS_k_VE_SERVER_ACTIVE:
      {
        /* SV server Time Stamp init complete */

        break;
      }

      default:
      {
        /* ERROR - unknown event */

        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(8u));
        break;
      }
    }
  }
}

/*******************************************************************************
**
** Function    : SAPL_IxsvcEventHandlerClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case the state of
**               a producing Safety Connection changes. With this function the
**               application can initiate application specific reaction to
**               producing Safety Connection events. Such events can be:
**               - opening/closing a connection
**               - consumer joined/leaved Multicast connection
**               - consumer of safety data failed
**               - consumer active
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxsvsEventHandlerClbk()
**
** Parameters  : u16_instId (IN) - instance ID of the associated Safety
**                                 Validator object
**               e_event (IN)    - Unique code describing the kind of event. See
**                                 type definition of {IXSVC_t_VALIDATOR_EVENT}.
**
** Returnvalue : -
**
*******************************************************************************/
void SAPL_IxsvcEventHandlerClbk(CSS_t_UINT u16_instId,
                                IXSVC_t_VALIDATOR_EVENT e_event)
{

  /* Callback required according to [CS_MAN], (see [3.8-2:]) */

  /* if invalid instance id, valid range, 1..(CSOS_cfg_NUM_OF_SV_CLIENTS +
  ** CSOS_cfg_NUM_OF_SV_SERVERS) */
  if ( (u16_instId == 0u) ||
       (u16_instId > (CSOS_cfg_NUM_OF_SV_CLIENTS + CSOS_cfg_NUM_OF_SV_SERVERS)) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(15u));
  }
  /* else: valid instance id */
  else
  {
    switch (e_event)
    {
      case IXSVC_k_VE_CLIENT_OPEN:
      {
        /* SV client opened */

        /* Make sure that when a producing connection is opened that our
           application is in run mode (TST21 expects that a producing multicast
           connection immediately sends data with the Run_Idle bBit set.) */

        /* Producing connection is established (see [SRS_2006]) */
        aicSm_SetEvent(AICSM_k_EVT_PROD_CONN_ESTABLISHED);
        break;
      }

      case IXSVC_k_VE_CLIENT_CLOSE:
      {
        /* SV client closed */
        /* Producing connection is closed/faulted (see [SRS_2136]) */
        aicSm_SetEvent(AICSM_k_EVT_PROD_CONN_CLOSED);
        break;
      }

      /* SVC consumer joined */
      /* SVC consumer left */
      /* SVC consumer faulted */
      /* Events only used for Multicasting, since not supported (see [SRS_2071])
      ** these events shall not occur
      */
      case IXSVC_k_VE_CONSUMER_JOIN:     /* fall through */
      case IXSVC_k_VE_CONSUMER_LEAVE:    /* fall through */
      case IXSVC_k_VE_CONSUMER_FAULTED:  /* fall through */
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(16u));
        break;
      }

      case IXSVC_k_VE_ALL_CONS_FAULTED:
      {
        /* SVC all consumers faulted */
        /* Producing connection is closed/faulted (see [SRS_2136], [SRS_2184]) */
        aicSm_SetEvent(AICSM_k_EVT_PROD_CONN_FAULTED);
        break;
      }

      case IXSVC_k_VE_CONSUMER_ACTIVE:
      {
        /* SVC consumer active */

        break;
      }

      case IXSVC_k_VE_1ST_HS_COMPLETE:
      {
        /* SVC handshake completed */

        break;
      }

      default:
      {
        /* ERROR - unknown event */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(9u));
        break;
      }
    }
  }
}


/*******************************************************************************
**
** Function    : SAPL_IxssoDeviceSelfTestClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function during initialization
**               to request the result of the device self test.
**
** Context     : IXSSC_Init()
**
** Parameters  : -
**
** Returnvalue : CSS_k_TRUE   - Self Test passed
**               CSS_k_FALSE  - Self Test failed
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoDeviceSelfTestClbk(void)
{
  /* Callback required according to [CS_MAN], (see [3.6-4:]) */

  /* Attention: The self test of the device are done, before the CIP Safety
  ** Target Stack is initialized at all. In case of any failure, this callback
  ** would not be reached because the device is already in FAILSAFE state then.
  ** So here we can return always CSS_k_TRUE, means 'Self Test passed'
  */
  return (CSS_k_TRUE);
}

/*******************************************************************************
**
** Function    : SAPL_CssProduceEvenBytesClbk
**
** Description : This callback function must be implemented by the application.
**               It is called by CSS to identify if this Safety Controller
**               produces the even or the odd bytes.
**
** Parameters  : -
**
** Returnvalue : CSS_k_FALSE - Safety Controller produces the odd bytes
**               CSS_k_TRUE  - Safety Controller produces the even bytes
**
*******************************************************************************/
CSS_t_BOOL SAPL_CssProduceEvenBytesClbk(void)
{
  CSS_t_BOOL b_evenBytes;
  CFG_CONTROLLER_ID_ENUM             eControllerId;

  /* on both safety controllers the same FW is running (this includes the CSS)
  ** acting as dual controller (see [SRS_2027]). So it is necessary to get at
  ** runtime the information if the even or the odd bytes shall be generated by
  ** CSS. */

  /* get the controller ID */
  eControllerId = cfgSYS_GetControllerID();

  /* if safety controller 1 */
  if (eControllerId == SAFETY_CONTROLLER_1)
  {
    /* even bytes */
    b_evenBytes = CSS_k_TRUE;
  }
  /* else: safety controller 2 */
  else if (eControllerId == SAFETY_CONTROLLER_2)
  {
    /* odd bytes */
    b_evenBytes = CSS_k_FALSE;
  }
  /* else: invalid controller id */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(10u));
    /* set return value, only used for unit test */
    b_evenBytes = CSS_k_TRUE;
  }
  return b_evenBytes;
}

/***************************************************************************************************
  Function:
    SAPL_ConfigStringCheck

  Description:
    This function performs several value and consistency checks on the received configuration data
    string.

  See also:
    -

  Parameters:
    ab_cfgData (IN)  - array with received configuration data (without header). The length was
                       already checked by the caller.

  Return value:
    CSS_k_OK         - configuration string successfully checked.
    <>CSS_k_OK       - deny the connection request. Returned value will be responded in
                       the Extended Status of the Unsuccessful Forward_Open Response.

  Remarks:
    Context: Background Task

***************************************************************************************************/
UINT16 SAPL_ConfigStringCheck(const CSS_t_BYTE ab_cfgData[])
{
  /* return value of this function */
  CSS_t_WORD u16_ret;

  /* if a safe output is disabled in the configuration then the required configuration value for
     SS1-t Delay Time is zero (see [SRS_912]) */
  if (    (    (ab_cfgData[FIPARAM_k_OFS_SS1T_DLY_B1] != 0u)
            || (ab_cfgData[FIPARAM_k_OFS_SS1T_DLY_B2] != 0u)
          )
       && ((ab_cfgData[FIPARAM_k_OFS_DO_PARAMS] & FIPARAM_k_MASK_DO_ENABLED) == 0u)
     )
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
  /* else if: DI12 is disabled then the required configuration value for SafeBound 1+2 is also
    "disable" (see[SRS_914]) */
  else if (    ((ab_cfgData[FIPARAM_k_OFS_DI12_PARAMS] & FIPARAM_k_MASK_DI_ENABLED) == 0u)
            && ((ab_cfgData[FIPARAM_k_OFS_SAFEBOUND_B1] & FIPARAM_k_MASK_SAFEBOUND_B12) != 0u)
          )
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
  /* else if: DI34 is disabled then the required configuration value for SafeBound 3+4 is also
    "disable" (see[SRS_914]) */
  else if (    ((ab_cfgData[FIPARAM_k_OFS_DI34_PARAMS] & FIPARAM_k_MASK_DI_ENABLED) == 0u)
            && ((ab_cfgData[FIPARAM_k_OFS_SAFEBOUND_B1] & FIPARAM_k_MASK_SAFEBOUND_B34) != 0u)
          )
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
  /* else if: DI56 is disabled then the required configuration value for SafeBound 5+6 is also
    "disable" (see[SRS_914]) */
  else if (    ((ab_cfgData[FIPARAM_k_OFS_DI56_PARAMS] & FIPARAM_k_MASK_DI_ENABLED) == 0u)
            && ((ab_cfgData[FIPARAM_k_OFS_SAFEBOUND_B1] & FIPARAM_k_MASK_SAFEBOUND_B56) != 0u)
          )
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
#if (CFG_S2L_SS1T_SUPPORTED != TRUE)
  /* else if: When SS1-t is not supported then these configuration bytes must be zero
    (see [SRS_911])*/
  else if (    (ab_cfgData[FIPARAM_k_OFS_SS1T_DLY_B1] != 0u)
            || (ab_cfgData[FIPARAM_k_OFS_SS1T_DLY_B2] != 0u)
          )
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
#endif
#if (CFG_S2L_SAFEBOUND_SUPPORTED != TRUE)
  /* else if: When SafeBound is not supported then this configuration byte must be zero
      (see [SRS_903]) */
  else if (ab_cfgData[FIPARAM_k_OFS_SAFEBOUND_B1] != 0u)
  {
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
#endif
  else  /* else: Configuration String is ok */
  {
    u16_ret = CSS_k_OK;
  }

  return (u16_ret);
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    CheckAicState

  Description:
    This function is used to check if the Safety Open Request is accepted in the current AIC
    state.

  See also:
    -

  Parameters:
    o_type1SafetyOpen (IN)   - CSS_k_TRUE if SafetyOpen is a Type 1 SafetyOpen
                               CSS_k_FALSE if SafetyOpen is a Type 2 SafetyOpen

  Return value:
    CSS_k_OK                 - Application grants the permission to  request.
    <>CSS_k_OK               - Application denies the connection request. Returned value will
                               be responded in the Extended Status of the Unsuccessful
                               Forward_Open Response.

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC UINT16 CheckAicState(CSS_t_BOOL o_type1SafetyOpen)
{
  /* return value, indicates if SafetyOpen shall be accepted */
  CSS_t_WORD u16_ret = 0xFFFFu;
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;

  /* check current AIC state */
  switch(e_aicState)
  {
     case AICSM_AIC_IDLE:               /* fall through */
     case AICSM_AIC_EXEC_PROD_ONLY:     /* fall through */
     case AICSM_AIC_EXEC_CONS_ONLY:     /* fall through */
     {
       /* accept Type 2 SafetyOpen (we are already configured) */
       /* accept Type 1 SafetyOpen (reconfigure the device, see [SRS_2251]) */
       u16_ret = CSS_k_OK;
       break;
     }
     case AICSM_AIC_WAIT_FOR_CONFIG:    /* fall through */
     case AICSM_AIC_INVALID_CONFIG:     /* fall through */
     {
       /* if we are processing a Type 1 SafetyOpen */
       if (o_type1SafetyOpen)
       {
         /* Type 1 SafetyOpen will provide us with configuration and will thus be accepted in
            these states (see [SRS_2250]) */
         u16_ret = CSS_k_OK;
       }
       else  /* else: then it is a Type 2 SafetyOpen */
       {
         /* Our device is not yet configured and thus cannot accept a Type 2 Safety Open here */
         /* ATTENTION: The CSS does block the request itself (by returning 'Device not configured')
         ** if no SCID is already set. But since this implementation expects that the SCID is
         ** written completely into NV memory (and read back), the 'Configuring' state can last
         ** longer and must be additionally considered here (see [SRS_2218]).
         */
         /* set Extended Status information */
         u16_ret = CSS_k_CMEXTSTAT_DEV_NOT_CFG;
       }
       break;
     }
     case AICSM_AIC_PROCESS_CONFIG:     /* fall through */
     {
      /* Configuration is in progress. Block any SafetyOpen request (see [SRS_2218]). */
       u16_ret = CSS_k_CMEXTSTAT_DEV_NOT_CFG;
       break;
     }
     case AICSM_AIC_STARTUP:            /* fall through */
     case AICSM_AIC_WAIT_FOR_INIT:      /* fall through */
     case AICSM_AIC_START_CSS:          /* fall through */
     case AICSM_AIC_WAIT_TUNID:         /* fall through */
     case AICSM_AIC_EXEC_PROD_CONS:     /* fall through */
     case AICSM_AIC_WAIT_RESET:         /* fall through */
     case AICSM_AIC_ABORT:              /* fall through */
     {
       /* In these AIC states it is not expected to receive a Safety Open Request,
       ** so blocked. */
       u16_ret = CSS_k_CMEXTSTAT_MISCELLANEOUS;
       break;
     }
     default:
     {
       /* invalid value of variable */
       GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(11u));
       break;
     }
  }
  return u16_ret;
}


/***************************************************************************************************
  Function:
    ValidateType1

  Description:
    This function is a sub-function of SAPL_IxsceSafetyOpenValidateClbk(). It is used to perform
    specific checks for a Type 1 SafetyOpen and then store the received configuration data
    temporarily to a static variable.

  See also:
    -

  Parameters:
    ps_safetyOpenParams (IN) - pointer to structure containing the parameters received with the
                               SafetyOpen service, also containing the safety configuration data

  Return value:
    CSS_k_OK                 - type 1 configuration successfully checked.
    <>CSS_k_OK               - deny the connection request. Returned value will be responded in
                               the Extended Status of the Unsuccessful Forward_Open Response.

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC UINT16 ValidateType1(const CSS_t_SOPEN_PARA *ps_safetyOpenParams)
{
  /* return value of this function */
  CSS_t_WORD u16_ret;

  /* if received configuration data length doesn't match */
  if (ps_safetyOpenParams->u8_cfgDataSizeW != k_EXPECTED_CFG_SIZE_W)
  {
    u16_ret = k_CMEXTSTAT_INV_CNXN_SIZE;
  }
  else if (ps_safetyOpenParams->s_nsd.u32_sccrc == 0UL)  /* else: if the received SCCRC is 0 */
  {
    /* The SCCRC matches the configuration data, but in this special case it is zero */
    /* To avoid any problems (e.g. later Type 2 SafetyOpens will skip SCCRC check) we reject
       this request. */
    u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
  }
  else  /* else: length of received configuration data is ok */
  {
    /*lint -esym(960, 17.4)*/
    /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    * other than array indexing used
    * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
    /* extract received "Software Major Version" and "Hardware ID" */
    CSS_t_USINT u8_swMajorVersion = ps_safetyOpenParams->pb_cfgData[0u];
    CSS_t_UINT  u16_hwId = ps_safetyOpenParams->pb_cfgData[1u]
                          + (CSS_t_UINT)((CSS_t_UINT)ps_safetyOpenParams->pb_cfgData[2u] << 8u);

    /* if received Software Major Version and Hardware ID don't match our expectation */
    if (    (u8_swMajorVersion != VERSION_SW_VERSION_MAJOR)
         || (u16_hwId != VERSION_ASM_MODULE_ID)
       )
    {
      u16_ret = k_CMEXTSTAT_INC_CONS_DATA_FRMT;
    }
    else  /* else: Version and ID information is ok */
    {
      /* do further value and consistency checking of the configuration string */
      u16_ret = SAPL_ConfigStringCheck(&ps_safetyOpenParams->pb_cfgData[k_CFG_HDR_SIZE]);

      if (u16_ret != CSS_k_OK)
      {
        /* return value already set */
      }
      else  /* else: Configuration String is ok */
      {
        /* store received configuration data temporarily */
        /* Note 926: cast from pointer to pointer [MISRA 2004 Rule 11.4] */
        /* need to convert from CSS to T100 data type */
        fiParam_setCandidateIParam((CONST UINT8*)&ps_safetyOpenParams->pb_cfgData[3], /*lint !e926*/
                                  (UINT8)(AICMSGHDL_IOCONFIGDATA_LEN),
                                  (UINT32)ps_safetyOpenParams->s_nsd.u32_sccrc);
        u16_ret = CSS_k_OK;
      }
    }
    /*lint +esym(960, 17.4)*/
  }

  return (u16_ret);
}

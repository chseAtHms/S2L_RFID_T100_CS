/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOapi.h
**    Summary: IXSSO - Safety Supervisor Object
**             Interface definition of the IXSSO unit of the CSS.
**             This unit implements the Baseline Safety Supervisor Object
**             (see Req.5.3-1). The SNCT Supervisor is not supported
**             (Req.5.3-9). Besides the services and attributes of the Safety
**             Supervisor Object it also implements the behavior in form of a
**             state machine. This module also controls the Module status LED.
**             IXSSOapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_DeviceStatusGet
**             IXSSO_SafetyDeviceMajorFault
**             IXSSO_ExceptionStatusSet
**             IXSSO_ModeChange
**             IXSSO_OcpunidTargAsmOutListSet
**             SAPL_IxssoDeviceSelfTestClbk
**             SAPL_IxssoSafetyResetClbk
**             SAPL_IxssoProfDepStateChangeClbk
**             SAPL_IxssoNvAttribStoreClbk
**             SAPL_IxssoNvAttribRestoreClbk
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSO_API_H
#define IXSSO_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSSO_k_NUM_OCPUNIDS:
    Determines the number of Output Assemblies for which the OCPUNID exist.
    The OCPUNID attribute shall be supported for all safety output assemblies
    (see SRS135). But only for those used in connections where this device is
    the Target. */
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)                     \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  #define IXSSO_k_NUM_OCPUNIDS  CSS_cfg_NUM_OF_T_ASM_OUT_INSTS
#elif (    (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
        && (CSOS_cfg_TARGET == CSOS_k_ENABLE)                      \
      )
  #define IXSSO_k_NUM_OCPUNIDS  CSS_cfg_NUM_OF_ASM_OUT_INSTS
#else
  #define IXSSO_k_NUM_OCPUNIDS  0U
#endif


#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  /** IXSSO_k_PASSWORD_SIZE:
      Number of octets of the Safety Supervisor Password.
  */
  #define IXSSO_k_PASSWORD_SIZE   16U

  /** IXSSO_k_RESET_TYPE_x:
      Parameter of the Safety Reset service that determines the type of reset
  */
  #define IXSSO_k_RESET_TYPE_0    0U
  #define IXSSO_k_RESET_TYPE_1    1U
  #define IXSSO_k_RESET_TYPE_2    2U
#endif


/** IXSSO_k_ES_xxx:
    Bit coding of the Safety Supervisor Exception Status Byte.
*/
/*lint -esym(755, IXSSO_k_ES_AL_DEV_COMM)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_ES_AL_DEV_SPEC)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_ES_AL_MAN_SPEC)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_ES_WN_DEV_COMM)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_ES_WN_DEV_SPEC)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_ES_WN_MAN_SPEC)         not referenced in every cfg */
#define IXSSO_k_ES_AL_DEV_COMM  0x01U   /* ALARM/device-common */
#define IXSSO_k_ES_AL_DEV_SPEC  0x02U   /* ALARM/device-specific */
#define IXSSO_k_ES_AL_MAN_SPEC  0x04U   /* ALARM/manufacturer-specific */
#define IXSSO_k_ES_RESERVED_03  0x08U   /* reserved -- must be set to 0 */
#define IXSSO_k_ES_WN_DEV_COMM  0x10U   /* WARNING/device-common */
#define IXSSO_k_ES_WN_DEV_SPEC  0x20U   /* WARNING/device-specific */
#define IXSSO_k_ES_WN_MAN_SPEC  0x40U   /* WARNING/manufacturer-specific */
#define IXSSO_k_ES_METHOD       0x80U   /* 0 == Basic Method
                                           1 == Expanded Method */

/** IXSSO_t_STORAGE_ID:
    Storage IDs for the {SAPL_IxssoNvAttribStoreClbk()} and
    {SAPL_IxssoNvAttribRestoreClbk()} functions.
*/
typedef enum
{
  IXSSO_k_SID_AL_ENABLE,            /* Alarm Enable Attribute */
  IXSSO_k_SID_WN_ENABLE,            /* Warning Enable Attribute */
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  IXSSO_k_SID_CFUNID,               /* Configuration UNID */
#endif
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  IXSSO_k_SID_OCPUNID,              /* Output Cnxn Point Owners Attribute */
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* On our originator implementation the SCID is not stored in NV-memory */
#else
  IXSSO_k_SID_SCID,                 /* Safety Configuration ID Attribute */
#endif
  IXSSO_k_SID_TUNID                 /* Target UNID Attribute (UNID of this
                                       device, also in case of Originator) or
                                       Target UNID List Attribute (in case of
                                       Multiple Safety Port Device)*/
} IXSSO_t_STORAGE_ID;


/** IXSSO_k_SDS_xxx:
    Storage Data Size of the Safety Supervisor Storage Parameters referring to
    {IXSSO_t_STORAGE_ID}.
*/
#define IXSSO_k_SDS_AL_ENABLE    (CSS_t_UINT)1U
#define IXSSO_k_SDS_WN_ENABLE    (CSS_t_UINT)1U
#define IXSSO_k_SDS_CFUNID       (CSS_t_UINT)sizeof(CSS_t_UNID)
#define IXSSO_k_SDS_SCID         (CSS_t_UINT)sizeof(IXSSO_t_SCID)
#define IXSSO_k_SDS_TUNID        ((CSS_t_UINT)sizeof(                          \
                                     CSS_t_UNID[CSOS_cfg_NUM_OF_SAFETY_PORTS]))
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  #define IXSSO_k_SDS_OCPUNID    ((CSS_t_UINT)                                 \
                                      sizeof(CSS_t_UNID[IXSSO_k_NUM_OCPUNIDS]))
#else
  #define IXSSO_k_SDS_OCPUNID    (CSS_t_UINT)0U
#endif


/** IXSSO_k_SE_xxx:
    Safety Supervisor Events. These events are passed to the Safety Supervisor
    State machine (the comments after the define represents the name of the
    event according to the Safety Supervisor State Event Matrix in Volume 5).
*/
/*lint -esym(755, IXSSO_k_SE_PROP_TUNID)          not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_APPLY_TUNID)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_VALIDATE_CFG)        not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_APPLY_REQ)           not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_T1_SOPEN)            not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_RESET_REQ)           not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_FWD_OPEN_REQ)        not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_APPLY_TUNID_CHECK)   not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_APPLY_CHECK)         not referenced in every cfg */
/*lint -esym(755, IXSSO_k_SE_OPEN_CNXN_REQ)       not referenced in every cfg */
#define IXSSO_k_SE_UNDEF              0U   /* init value (just for safety) */
/* #define IXSSO_k_SE_POWER_APPLIED     1U *//* Power Applied - not supported */
#define IXSSO_k_SE_ST_PASS            2U   /* Self-Test Passed */
#define IXSSO_k_SE_ST_FAIL            3U   /* Self-Test Failed */
#define IXSSO_k_SE_PROP_TUNID         4U   /* Propose TUNID */
#define IXSSO_k_SE_APPLY_TUNID        5U   /* Apply TUNID */
#define IXSSO_k_SE_EX_COND_CLEAR      6U   /* Exception Condition Cleared */
#define IXSSO_k_SE_CRIT_FAULT         7U   /* Critical Fault */
/* #define IXSSO_k_SE_CFG_REQ         8U */ /* Config Request - not supported */
#define IXSSO_k_SE_VALIDATE_CFG       9U   /* Validate Configuration */
#define IXSSO_k_SE_APPLY_REQ         10U   /* Apply Request */
#define IXSSO_k_SE_T1_SOPEN          11U   /* Type 1 SafetyOpen */
#define IXSSO_k_SE_RESET_REQ         12U   /* Reset Request */
#define IXSSO_k_SE_INT_ABORT_REQ     13U   /* Internal Abort Request */
/* #define IXSSO_k_SE_RECOVER_REQ    14U *//* Recover Request - not supported */
/* #define IXSSO_k_SE_PERFORM_DIAG_REQ 15U */ /* Perform Diagnostics Request
                                                 - not supported */
#define IXSSO_k_SE_SCNXN_FAIL        16U   /* Safety Connection Failed */
#define IXSSO_k_SE_SCNXN_ESTAB       17U   /* Safety Connection Established */
/* According to the Safety Supervisor State Event Matrix in Volume 5 the event
   "Safety I/O Connection Deleted" is used when a connection is closed. In
   device profiles (e.g. Safety Discrete I/O Device) different terms may appear:
   "Safety Connection Closed" -> equivalent to "Safety I/O Connection Deleted"
   "Safety I/O Connection Deleted" -> refers to the standard DeviceNet Delete
   Service (which is not supported for safe connections) */
#define IXSSO_k_SE_SCNXN_DEL         18U   /* Safety Connection Deleted */
#define IXSSO_k_SE_MODE_CHANGE       19U   /* Mode Change */
/* #define IXSSO_k_SE_RESTORE        20U */ /* Restore - not supported */
/* #define IXSSO_k_SE_LOCK           21U */ /* Lock/Unlock - not supported */
/* implementation specific events */
#define IXSSO_k_SE_FWD_OPEN_REQ      22U   /* Forward_Open Request, T1 or T2 */
#define IXSSO_k_SE_APPLY_TUNID_CHECK 23U   /* Check state before Apply_TUNID */
#define IXSSO_k_SE_APPLY_CHECK       24U   /* Check state before Apply Cfg */
#define IXSSO_k_SE_OPEN_CNXN_REQ     25U   /* CCO Open_Cnxn Request */


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSSO_t_SCID:
    This data type stores the Safety Configuration Identifier.
*/
typedef struct
{
  CSS_t_UDINT u32_sccrc;        /* Safety Configuration CRC (SCCRC) */
  CSS_t_DATE_AND_TIME s_scts;   /* Safety Configuration Time Stamp (SCTS) */
} IXSSO_t_SCID;                 /* Safety Configuration Identifier (SCID) */


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*
**  Functions provided by the IXSSO Unit
*/
/*******************************************************************************
**
** Function    : IXSSO_DeviceStatusGet
**
** Description : This API function may be called by the application to request
**               the device status (current state of the device). If the
**               application implements further CIP objects this function shall
**               be called as all CIP objects in a safety device are subservient
**               to the Safety Supervisor Object (see SRS66).
**
** Parameters  : -
**
** Returnvalue : See type definition of CSOS_t_SSO_DEV_STATUS.
**
*******************************************************************************/
CSOS_t_SSO_DEV_STATUS IXSSO_DeviceStatusGet(void);


/*******************************************************************************
**
** Function    : IXSSO_SafetyDeviceMajorFault
**
** Description : This API function is to be used by the safety application to
**               indicate Major Fault events that occurred inside the Safety
**               Application.
**
** Parameters  : o_recoverable (IN) - If CSS_k_TRUE then this is a recoverable
**                                    fault ( == "Internal Abort Request").
**                                    Else this is an unrecoverable fault
**                                    ( == "Critical Fault"). See Volume 5
**                                    section 5-4.5.2 for details on Safety
**                                    Supervisor Object States and Events.
**                                    (not checked, any value allowed)
**
** Returnvalue : -
*******************************************************************************/
void IXSSO_SafetyDeviceMajorFault(CSS_t_BOOL o_recoverable);


/*******************************************************************************
**
** Function    : IXSSO_ExceptionStatusSet
**
** Description : This API function must be called by the application to set the
**               device's Safety Supervisor Object Exception Status bits. Only
**               the Basic Method is supported (Bit 7 must always be 0).
**
** Parameters  : b_excStat (IN)    - Combination of bits. See definition of
**                                   {IXSSO_k_ES_xxx}.
**                                   (checked, see {IXSSO_k_ES_xxx})
**
** Returnvalue : CSS_k_TRUE        - The IXSSO has accepted the passed Status.
**               CSS_k_FALSE       - The passed IXSSO status was not accepted
**                                   due to a reserved bit was set or a IXSSO
**                                   state mismatch
**
*******************************************************************************/
CSS_t_BOOL IXSSO_ExceptionStatusSet(CSS_t_BYTE b_excStat);


/* Usage of this API function is optional. Thus mute lint warnings concerning
   - function not referenced
   - could be moved from header to module
   - could be made static
*/
/*lint -esym(714, IXSSO_ModeChange) */
/*lint -esym(759, IXSSO_ModeChange) */
/*lint -esym(765, IXSSO_ModeChange) */
/*******************************************************************************
**
** Function    : IXSSO_ModeChange
**
** Description : This API function moves the device from the Idle to the
**               Executing state or back from Executing to Idle.
**
** Parameters  : o_executing (IN) - CSS_k_TRUE - move from IDLE to EXECUTING
**                                  CSS_k_FALSE - move from EXECUTING to IDLE
**                                  (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_ModeChange(CSS_t_BOOL o_executing);


/*******************************************************************************
**
** Function    : IXSSO_OcpunidTargAsmOutListSet
**
** Description : This API function stores a list (lookup table) of the Output
**               Assembly Instances used for connections in which this device is
**               the Target. It must be called after setting up the Assembly
**               Instances (IXSAI_AsmIoInstSetup()) and before applying the
**               configuration (IXCCO_Apply()).
**               See the description of CSS_cfg_NUM_OF_T_ASM_OUT_INSTS for which
**               configurations this function is relevant.
**
** Parameters  : au16_targAsmOutInsts (IN) - pointer to an array of Output
**                                           Assembly Instances (array size
**                                           CSS_cfg_NUM_OF_T_ASM_OUT_INSTS
**                                           elements)
**                                           (checked,
**                                           valid range: <> CSS_k_NULL)
** Returnvalue : -
**
*******************************************************************************/
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  void IXSSO_OcpunidTargAsmOutListSet(
      const CSS_t_UINT au16_targAsmOutInsts[CSS_cfg_NUM_OF_T_ASM_OUT_INSTS]);
#endif


/*
** Functions to be provided by the Safety Application
*/

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
CSS_t_BOOL SAPL_IxssoDeviceSelfTestClbk(void);


/*******************************************************************************
**
** Function    : SAPL_IxssoSafetyResetClbk
**
** Description : This callback function is called when the CSS has received a
**               valid Safety_Reset service. The password parameter of the
**               Safety_Reset service is passed also. The Safety Application has
**               to store this event and must then return. The return value of
**               this callback function decides about the returned explicit
**               general status code. The Safety Application shall not call any
**               other CSS API functions from now on. The Safety Application
**               must ensure that all pending HALCS messages are delivered to
**               and executed by CSAL. Then after the response message to the
**               Safety_Reset request is sent out to the network the device must
**               be reset.
**
** Context:      IXSMR_CmdProcess()
**
** Parameters  : u8_resetType (IN) - Type of Safety Reset. Dependent on this
**                                   parameter the Safety Application has to
**                                   restore the default values of the
**                                   non-volatile parameters. Supported values:
**                                   IXSSO_k_RESET_TYPE_0, IXSSO_k_RESET_TYPE_1,
**                                   IXSSO_k_RESET_TYPE_2
**               b_attrBitMap (IN) - Only meaningful in case u8_resetType is
**                                   IXSSO_k_RESET_TYPE_2. Then this bit pattern
**                                   determines which settings shall be
**                                   preserved.
**               ac_checkPwd (IN)  - Safety_Reset password. This must be
**                                   checked by this function. If the check of
**                                   the password in the application fails the
**                                   function shall return with
**                                   CSOS_k_CGSC_PRIVILEGE_VIOLATION
**                                   (see SRS150).
**
** Returnvalue : CSOS_k_CGSC_SUCCESS - Safety_Reset will be performed
**                                     (Password matches, etc. )
**               else                - error: This is the general status code
**                                     that will be returned for this explicit
**                                     service. See the definition of the
**                                     CIP General Status Codes
**                                     (CSOS_k_CGSC_xxx) in CSOSapi.h.
**
*******************************************************************************/
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
CSS_t_USINT SAPL_IxssoSafetyResetClbk(CSS_t_USINT u8_resetType,
                           CSS_t_BYTE b_attrBitMap,
                           const CSS_t_CHAR ac_checkPwd[IXSSO_k_PASSWORD_SIZE]);
#endif


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
                                            CSS_t_UINT u16_numEstabCnxns);


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, SAPL_IxssoNvAttribStoreClbk) */
/*lint -esym(765, SAPL_IxssoNvAttribStoreClbk) */
/*******************************************************************************
**
** Function    : SAPL_IxssoNvAttribStoreClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when it wants to write
**               the values for the Safety Supervisor Object Attributes to
**               non-volatile memory.
**
**               **Attention**:
**               Storing and restoring of non-volatile parameters shall be done
**               with safety integrity.
**
**               **Attention**:
**               The data passed in pb_storeData is stored/restored in processor
**               format. This means that endianness and possibly padding bytes
**               within structures (e.g. IXSSO_k_SID_OCPUNID = array of
**               CSS_t_UNID) is exactly the same like the data is stored in the
**               variables used by CSS.
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxssoNvAttribRestoreClbk()
**
** Parameters  : e_storeId (IN)    - Unique store identifier to be able to
**                                   reference the stored data later in the
**                                   {SAPL_IxssoNvAttribRestoreClbk()}. If a
**                                   store identifier was already used then the
**                                   stored data for this identifier will be
**                                   overwritten. In this case the
**                                   ``u32_dataSize`` must be the same.
**               u16_dataSize (IN) - size of the data in byte to be stored
**               pb_storeData (IN) - pointer to the data to be stored
**
** Returnvalue : CSS_k_TRUE    - Application was able to store the requested
**                               values
**               CSS_k_FALSE   - Application encountered an error while trying
**                               to write the NV values. The values are not
**                               saved!
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoNvAttribStoreClbk(IXSSO_t_STORAGE_ID e_storeId,
                                       CSS_t_UINT u16_dataSize,
                                       const CSS_t_BYTE * pb_storeData);


/*******************************************************************************
**
** Function    : SAPL_IxssoNvAttribRestoreClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when it wants to read
**               the stored values for the Safety Supervisor Object Attributes.
**               In case of an unconfigured device default values must be
**               returned:
**               - Alarm Enable = 1
**               - Warning enable = 1
**               - CFUNID = 0 (see SRS134) (Target only)
**               - OCPUNIDs = 0 (see SRS201) (consuming Target only)
**               - SCID = 0 (see SRS184) (Target only)
**               - TUNID.SNN: A device in Manufacturers default state shall have
**                 an invalid UNID value in its NVS
**                 (e.g. FF FF FF FF FF FF) (see SRS117)
**               - TUNID.NodeID: device specific default value (see SRS115)
**
**               One way of initializing the NV memory of an unconfigured device
**               (or to restore the factory default settings) is to assign the
**               default value to a variable and call
**               {SAPL_IxssoNvAttribStoreClbk()} to store it. See the CSS demo
**               for details.
**
**               **Attention**:
**               Storing and restoring of non-volatile parameters shall be done
**               with safety integrity.
**
**               **Attention**:
**               The data passed in pb_storeData is stored/restored in processor
**               format. This means that endianness and possibly padding bytes
**               within structures (e.g. IXSSO_k_SID_OCPUNID = array of
**               CSS_t_UNID) is exactly the same like the data is stored in the
**               variables used by CSS.
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxssoNvAttribStoreClbk()
**
** Parameters  : e_storeId (IN)     - unique store identifier that was passed at
**                                    the call of
**                                    {SAPL_IxssoNvAttribStoreClbk()}
**               u16_dataSize (IN)  - size of the data in byte to be restored
**               pb_storeData (OUT) - pointer to the memory space where the
**                                    stored data is restored
**
** Returnvalue : CSS_k_TRUE     - Application was able to provide the requested
**                                values
**               CSS_k_FALSE    - Application encountered an error while trying
**                                to read the NV values. Thus this function
**                                returns the default values for all
**                                attributes.
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoNvAttribRestoreClbk(IXSSO_t_STORAGE_ID e_storeId,
                                         CSS_t_UINT u16_dataSize,
                                         CSS_t_BYTE * pb_storeData);


#endif /* IXSSO_API_H */

/*** End of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVC.h
**    Summary: IXSVC - Safety Validator Client
**             IXSVC.h is the export header file of the IXSVC unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_Init
**             IXSVC_InstInit
**             IXSVC_TargClientInit
**             IXSVC_OrigClientInit
**             IXSVC_InstIdIdxAssign
**             IXSVC_McastCnxnFind
**             IXSVC_InstFind
**             IXSVC_InstAlloc
**             IXSVC_InstConsumerStop
**             IXSVC_InstIdIdxStop
**             IXSVO_IxsvcCnxnStateTransClbk
**             IXSVC_FaultCountGet
**             IXSVC_FaultCountReset
**             IXSVC_StateNumClientCnxnGet
**             IXSVC_TcooRxProc
**             IXSVC_InstClose
**             IXSVC_InstCloseAll
**             IXSSO_IxsvcDeviceStatusGetClbk
**             IXSVC_InitInfoCnxnPointGet
**             IXSVC_TcooFaultCounterGet
**             IXSVC_TcooProdFaultCounterReset
**             IXSVC_InitInfoMaxConsNumGet
**             IXSVC_PingIntEpiMultGet
**             IXSVC_TcooMsgMinMultGet
**             IXSVC_NetTimeExpMultGet
**             IXSVC_InstInfoTimeoutMultGet
**             IXSVC_ErrorLastGet
**             IXSVC_StateGet
**             IXSVC_InitInfoMsgFrmtGet
**             IXSVC_InstDealloc
**             IXSVC_InitSoftErrByteGet
**             IXSVC_InitSoftErrVarGet
**             IXSVC_InitInfoSoftErrByteGet
**             IXSVC_InitInfoSoftErrVarGet
**             IXSVC_InstSoftErrByteGet
**             IXSVC_InstSoftErrVarGet
**             IXSVC_InstIdSoftErrByteGet
**             IXSVC_InstIdSoftErrVarGet
**             IXSVC_InstInfoSoftErrByteGet
**             IXSVC_InstInfoSoftErrVarGet
**             IXSVC_ProdSoftErrByteGet
**             IXSVC_ProdSoftErrVarGet
**             IXSVC_QuarSoftErrByteGet
**             IXSVC_QuarSoftErrVarGet
**             IXSVC_StateSoftErrByteGet
**             IXSVC_StateSoftErrVarGet
**             IXSVC_TcooSoftErrByteGet
**             IXSVC_TcooSoftErrVarGet
**             IXSVC_UtilSoftErrByteGet
**             IXSVC_UtilSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVC_H
#define IXSVC_H


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSVC_k_NTEM_INVALID_VALUE: indicates that no valid value is available for
    the Network_Time_Expectation_Multiplier */
#define IXSVC_k_NTEM_INVALID_VALUE      0xFFFFU


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_Init
**
** Description : This function initializes the IXSVC unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_Init(void);


/*******************************************************************************
**
** Function    : IXSVC_InstInit
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the IXSVC
**               structures.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInit(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TargClientInit
**
** Description : This function initializes a Client instance of the Safety
**               Validator Object for a Connection in that we are the Target.
**
** Parameters  : ps_sOpenPar (IN)    - parameters received with the Forward
**                                     Open request
**                                     (not checked, only called with
**                                     reference to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as
**                                     the payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               u16_svInst (IN)     - Safety Validator instance ID that is
**                                     already allocated for this connection
**                                     (not checked, checked in
**                                     IXSVO_TargInit())
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error while initializing the Safety
**                                     Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSVC_TargClientInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                CSS_t_USINT u8_payloadSize,
                                CSS_t_UINT u16_svInst,
                                CSS_t_SOPEN_RESP *ps_sOpenResp);
#endif


/*******************************************************************************
**
** Function    : IXSVC_OrigClientInit
**
** Description : This function initializes a Client instance of the Safety
**               Validator Object for a Connection in that we are the
**               Originator.
**
** Parameters  : ps_openRespRx (IN)      - parameters received with the Forward
**                                         Open response
**                                         (not checked, only called with
**                                         reference to structure variable)
**               ps_ccoSopenInitVal (IN) - pointer to parameters coming from the
**                                         CCO instance data
**                                         (not checked, only called with
**                                         reference to structure variable)
**               u16_svInst (IN)         - Safety Validator instance ID that is
**                                         already allocated for this connection
**                                         (not checked, checked in
**                                         IXSVO_OrigInit())
**
** Returnvalue : CSS_k_OK                - success
**               <>CSS_k_OK              - error while initializing the Safety
**                                         Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
CSS_t_WORD IXSVC_OrigClientInit(
                           const CSS_t_SOPEN_RESP_RX *ps_openRespRx,
                           const CSS_t_CCO_SOPEN_INIT_VAL *ps_ccoSopenInitVal,
                           CSS_t_UINT u16_svInst);
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVC_InstIdIdxAssign
**
** Description : This function assigns the Safety Validator Client index and
**               Instance ID dependency.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXCCO_CnxnOpen()
**                                 and IXSVO_TargAlloc())
**               u16_svcIdx (IN) - array index of the Safety Validator Client
**                                 array
**                                 (not checked, checked in IXSVO_OrigAlloc()
**                                 and IXSVO_TargAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstIdIdxAssign(CSS_t_UINT u16_instId, CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_McastCnxnFind
**
** Description : This function is called before allocating a new Safety
**               Validator instance. It checks if a Multicast connection with
**               matching connection parameters already exists and if so returns
**               the Instance ID of this connection.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to parameters received with
**                                        the Forward_Open request (Target) or
**                                        with accordingly set data in case of
**                                        Originator
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu16_svInst (OUT)      - pointer to allocated Safety Validator
**                                        instance ID in case of success,
**                                        CSOS_k_INVALID_INSTANCE otherwise
**                                        (not checked, only called with
**                                        reference to variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_TRUE             - found a matching connection
**               CSS_k_FALSE            - no matching connection found
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_BOOL IXSVC_McastCnxnFind(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_UINT *pu16_svInst,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstFind
**
** Description : This function searches the Safety Validator Client Instance
**               which matches the passed Connection Triad.
**
** Parameters  : ps_cnxnTriad (IN) - pointer to the structure which contains
**                                   the description of the connection
**                                   (not checked, only called with reference to
**                                   structure)
**               ps_svInfo (OUT)   - pointer to the structure to where the found
**                                   Safety Validator Instance Type, Index, and
**                                   Consumer_Number will be written
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_k_TRUE        - connection was found (index in ps_cnxnIdx)
**               CSS_k_FALSE       - connection not found
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_BOOL IXSVC_InstFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstAlloc
**
** Description : This function allocates a free Client instance number of the
**               Safety Validator Object. Dependent on the parameter
**               u16_reqSvInst this function either tries to allocate the passed
**               instance ID or tries to allocate the next free instance ID.
**
** Parameters  : -
**
** Returnvalue : >=CSOS_cfg_NUM_OF_SV_CLIENTS - error: it was not possible to
**                                              allocate an instance ID
**               <CSOS_cfg_NUM_OF_SV_CLIENTS  - success: allocated Safety
**                                              Validator instance ID
**
*******************************************************************************/
CSS_t_UINT IXSVC_InstAlloc(void);


/*******************************************************************************
**
** Function    : IXSVC_InstConsumerStop
**
** Description : This function closes the passed Consumer of the passed Safety
**               Validator Client and checks if there are further consumers of
**               this connection left.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in IXSVC_InstClose()
**                                 and IXSVO_InstanceStop())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in IXSVC_InstClose()
**                                 and IXSVO_InstanceStop())
**
** Returnvalue : CSS_k_TRUE      - This function has just closed the last
**                                 remaining Consumer of this connection.
**               CSS_k_FALSE     - There are still Consumers left.
**
*******************************************************************************/
CSS_t_BOOL IXSVC_InstConsumerStop(CSS_t_UINT u16_svcIdx,
                                  CSS_t_USINT u8_consNum);


/*******************************************************************************
**
** Function    : IXSVC_InstIdIdxStop
**
** Description : This function stops/deletes the passed Safety Validator Client
**               (which includes all consumers).
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit(), IXSVC_InstClose() and
**                                 IXSVO_InstanceStop())
**
** Returnvalue : CSS_k_TRUE      - stopping was successful
**               CSS_k_FALSE     - error (error already reported to SAPL)
**
*******************************************************************************/
CSS_t_BOOL IXSVC_InstIdIdxStop(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVO_IxsvcCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Client
**               whenever the state of an instance changes.
**
** Parameters  : u8_oldState (IN) - the state in which the safety validator
**                                  instance was up to now
**                                  (not checked - any value allowed)
**               u8_newState (IN) - new state of this instance
**                                  (not checked - checked in
**                                  CnxnStateTransition())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_IxsvcCnxnStateTransClbk(CSS_t_USINT u8_oldState,
                                   CSS_t_USINT u8_newState);


/*******************************************************************************
**
** Function    : IXSVC_FaultCountGet
**
** Description : This macro returns the current value of the Fault Counter of
**               the Safety Validator Client instances. Counts (with
**               auto-rollover) any producing safety connection was faulted for
**               any reason.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - value of the Safety Validator Fault Count
**
*******************************************************************************/
CSS_t_UINT IXSVC_FaultCountGet(void);


/*******************************************************************************
**
** Function    : IXSVC_FaultCountReset
**
** Description : This macro resets the value of the Fault Counter of the Safety
**               Validator Client instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_FaultCountReset(void);


/*******************************************************************************
**
** Function    : IXSVC_StateNumClientCnxnGet
**
** Description : This function returns the number of Safety Validator
**               Client Connections that are currently open (in Initializing or
**               Established state).
**
** See Also    : IXSVS_NumServerCnxnGet
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Number of open client Connections
**
*******************************************************************************/
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  CSS_t_UINT IXSVC_StateNumClientCnxnGet(void);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooRxProc
**
** Description : This function processes received Safety I/O messages that are
**               directed to Safety Validator Client Instances
**
** Parameters  : u16_svcIdx (IN) - Instance Index of the Safety Validator Client
**                                 (not checked, checked in ValidatorRxProc())
**               u8_consNum (IN) - consumer number
**                                 (checked, valid range:
**                                 1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**               u16_len (IN)    - length of the received message
**                                 (not checked, checked in called function
**                                 TcooRxCopy())
**               pb_data (IN)    - pointer to byte stream that contains the
**                                 received message
**                                 (not checked, called with pointer to received
**                                 HALC message plus offset)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcooRxProc(CSS_t_UINT u16_svcIdx,
                      CSS_t_USINT u8_consNum,
                      CSS_t_UINT u16_len,
                      const CSS_t_BYTE *pb_data);


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSVC_InstClose) */
/*lint -esym(765, IXSVC_InstClose) */
/*******************************************************************************
**
** Function    : IXSVC_InstClose
**
** Description : This function closes the passed Safety Validator Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, IXSVD_SvIdxFromInstGet()
**                                 must return a valid index)
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in called function
**                                 IXSVC_StateUnderlayingComClose())
**               o_stop (IN)     - if CSS_k_TRUE the instance is stopped and the
**                                 event is reported to the SAPL
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
CSS_t_WORD IXSVC_InstClose(CSS_t_UINT u16_instId,
                           CSS_t_USINT u8_consNum,
                           CSS_t_BOOL o_stop);


/*******************************************************************************
**
** Function    : IXSVC_InstCloseAll
**
** Description : This function drops all connections at once. Needed e.g. by
**               the Safety Supervisor Object when a Configure Request service
**               is received.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstCloseAll(void);


/*******************************************************************************
**
** Function    : IXSSO_IxsvcDeviceStatusGetClbk
**
** Description : This callback function is called inside the IXSVC unit to
**               obtain the device status (current state of the device). This is
**               needed as all CIP objects in a safety device are subservient to
**               the Safety Supervisor Object (see SRS66).
**
** Parameters  : -
**
** Returnvalue : See type definition of {CSOS_t_SSO_DEV_STATUS}.
**
*******************************************************************************/
CSOS_t_SSO_DEV_STATUS IXSSO_IxsvcDeviceStatusGetClbk(void);


/*******************************************************************************
**
** Function    : IXSVC_InitInfoCnxnPointGet
**
** Description : This function returns the Connection Point which produces the
**               data to be sent by the requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVC_TargClientInit(), IXSVC_InstInfoGet(),
**                                 IXSVC_InstTxFrameGenerate(),
**                                 IXSVC_McastCnxnFind and
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_UINT      - Connection Point associated with the
**                                 requested Safety Validator Client
**
*******************************************************************************/
CSS_t_UINT IXSVC_InitInfoCnxnPointGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TcooFaultCounterGet
**
** Description : This function returns the Producer Fault Counter of the
**               requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client Instance
**                                 (not checked, checked by
**                                 IXSVO_ExplMsgHandler())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**                                 (not checked, checked in
**                                 FaultCountAttribGet())
**
** Returnvalue : CSS_t_USINT     - Producer Fault Counter of the requested
**                                 Safety Validator Client
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_USINT IXSVC_TcooFaultCounterGet(CSS_t_UINT u16_svcIdx,
                                        CSS_t_USINT u8_consIdx);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooProdFaultCounterReset
**
** Description : This function resets the Producer Fault Counters of all Safety
**               Validator Client instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  void IXSVC_TcooProdFaultCounterReset(void);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InitInfoMaxConsNumGet
**
** Description : This function returns the Maximum Consumer Number of the
**               requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - Maximum Consumer Number of the requested
**                                 Safety Validator Client Instance
**
*******************************************************************************/
CSS_t_USINT IXSVC_InitInfoMaxConsNumGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_PingIntEpiMultGet
**
** Description : This function returns the value of the Ping Interval EPI
**               Multiplier of the addressed instance.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_UINT      - Ping Interval EPI Multiplier
**
*******************************************************************************/
CSS_t_UINT IXSVC_PingIntEpiMultGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TcooMsgMinMultGet
**
** Description : This function returns the value of the Time Coordination
**               Message Min Multiplier of the addressed instance's consumer.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked by
**                                 IXSVO_ExplMsgHandler())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**                                 (not checked, checked in
**                                 TimeCoordMsgMinMultAttribGet())
**
** Returnvalue : CSS_t_UINT      - Time Coordination Message Min Multiplier
**
*******************************************************************************/
CSS_t_UINT IXSVC_TcooMsgMinMultGet(CSS_t_UINT u16_svcIdx,
                                   CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_NetTimeExpMultGet
**
** Description : This function returns the value of the Network Time Expectation
**               Multiplier of the addressed instance's consumer.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**               u8_consIdx (IN) - Consumer index
**                                 (checked, valid range:
**                                 0..(CSOS_k_MAX_CONSUMER_NUM_MCAST-1))
**
** Returnvalue : CSS_t_UINT      - Network Time Expectation Multiplier
**                                 success: 0..CSS_k_MAX_NTE_MULT_128US
**                                 error: CSOS_k_MAX_CONSUMER_NUM_MCAST
**
*******************************************************************************/
CSS_t_UINT IXSVC_NetTimeExpMultGet(CSS_t_UINT u16_svcIdx,
                                   CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_InstInfoTimeoutMultGet
**
** Description : This function returns the value of the Timeout Multiplier of
**               the addressed instance's consumer.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**               u8_consIdx (IN) - Consumer Index (0..Max_Consumer_Number-1)
**                                 (not checked, checked in
**                                 TimeoutMultAttribGet())
**
** Returnvalue : CSS_t_USINT     - Timeout Multiplier
**
*******************************************************************************/
CSS_t_USINT IXSVC_InstInfoTimeoutMultGet(CSS_t_UINT u16_svcIdx,
                                         CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_ErrorLastGet
**
** Description : This function returns the last error code of the addressed
**               Safety Validator Client Instance.
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_WORD      - Last Error Code
**
*******************************************************************************/
CSS_t_WORD IXSVC_ErrorLastGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_StateGet
**
** Description : This function returns the state of the requested Safety
**               Validator Client Instance.
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - State of the requested Safety Validator
**                                 Client Instance
**
*******************************************************************************/
CSS_t_USINT IXSVC_StateGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InitInfoMsgFrmtGet
**
** Description : This function returns the message format of the requested
**               Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE      - message format of the requested Safety
**                                 Validator Client Instance
**
*******************************************************************************/
CSS_t_BYTE IXSVC_InitInfoMsgFrmtGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InstDealloc
**
** Description : This function deallocates the addressed Safety Validator Object
**               by clearing its allocation bit.
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client instance
**                                 (not checked, checked in IXSVC_Init(),
**                                 IXSVC_OrigClientInit() and
**                                 IXSVO_InstanceStop())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstDealloc(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InitSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_InitSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InitSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_InitSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InitInfoSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_InitInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                           CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InitInfoSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_InitInfoSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_InstSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_InstSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstIdSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_InstIdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                         CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstIdSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_InstIdSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstInfoSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_InstInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                           CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstInfoSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_InstInfoSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_ProdSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_ProdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_ProdSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_ProdSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_QuarSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_QuarSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_QuarSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_QuarSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_StateSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_StateSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_StateSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
CSS_t_UDINT IXSVC_TcooSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_TcooSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_UtilSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVC_UtilSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVC_UtilSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVC_UtilSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


#endif /* #ifndef IXSVC_H */

/*** End Of File ***/


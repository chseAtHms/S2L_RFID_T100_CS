/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVS.h
**    Summary: IXSVS - Safety Validator Server
**             IXSVS.h is the export header file of the IXSVS unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_Init
**             IXSVS_TargServerInit
**             IXSVS_OrigServerInit
**             IXSVS_InstIdIdxAssign
**             IXSVS_InstAlloc
**             IXSVS_InstFind
**             IXSVS_ConsColdStartLogic
**             IXSVS_InstStop
**             IXSVS_RxDataCrossCheckError
**             IXSVO_IxsvsCnxnStateTransClbk
**             IXSVS_StateFaultCountGet
**             IXSVS_StateFaultCountReset
**             IXSVS_RxProc
**             IXSVS_InstClose
**             IXSVS_InstCloseAll
**             IXSVS_StateGet
**             IXSVS_InitInfoMsgFrmtGet
**             IXSVS_MaxDataAgeGet
**             IXSVS_MaxDataAgeSet
**             IXSVS_InitInfoCnxnPointGet
**             IXSVS_ConsFaultCounterGet
**             IXSVS_ConsFaultCounterReset
**             IXSVS_InstInit
**             IXSVS_PingIntEpiMultGet
**             IXSVS_TcooMsgMinMultGet
**             IXSVS_InitInfoNetTimeExpMultGet
**             IXSVS_TimeoutMultiplierGet
**             IXSVS_MaxConsNumGet
**             IXSVS_ErrorLastGet
**             IXSVS_StateNumServerCnxnGet
**             IXSVS_ConsSoftErrByteGet
**             IXSVS_ConsSoftErrVarGet
**             IXSVS_InitInfoSoftErrByteGet
**             IXSVS_InitInfoSoftErrVarGet
**             IXSVS_InstSoftErrByteGet
**             IXSVS_InstSoftErrVarGet
**             IXSVS_InstIdSoftErrByteGet
**             IXSVS_InstIdSoftErrVarGet
**             IXSVS_RecvSoftErrByteGet
**             IXSVS_RecvSoftErrVarGet
**             IXSVS_StateSoftErrByteGet
**             IXSVS_StateSoftErrVarGet
**             IXSVS_TcooSoftErrByteGet
**             IXSVS_TcooSoftErrVarGet
**             IXSVS_TcorrSoftErrByteGet
**             IXSVS_TcorrSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVS_H
#define IXSVS_H


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)


/*******************************************************************************
**    constants and macros
*******************************************************************************/


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
** Function    : IXSVS_Init
**
** Description : This function initializes the IXSVS unit.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_Init(void);


/*******************************************************************************
**
** Function    : IXSVS_TargServerInit
**
** Description : This function initializes a Server instance of the Safety
**               Validator Object for a Connection in that we are the Target.
**
** Parameters  : ps_sOpenPar (IN)    - parameters received with the Forward
**                                     Open request
**                                     (not checked, only called with
**                                     reference to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as the
**                                     payload of this safety connection
**                                     (not checked, valid range: 1..250,
**                                     checked in CnxnSizeCheck())
**               u16_svInst (IN)     - Safety Validator instance ID that is
**                                     already allocated for this connection
**                                     (checked, IXSVD_SvIdxFromInstGet()
**                                     must return a valid index)
**               ps_appReply (OUT)   - pointer to structure for returning the
**                                     Application Reply of a Forward_Open
**                                     response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error while initializing the Safety
**                                     Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSVS_TargServerInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                CSS_t_USINT u8_payloadSize,
                                CSS_t_UINT u16_svInst,
                                CSS_t_APP_REPLY *ps_appReply);
#endif


/*******************************************************************************
**
** Function    : IXSVS_OrigServerInit
**
** Description : This function initializes a Server instance of the Safety
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
**                                         (checked,
**                                         IXSVD_SvIdxFromInstGet() must
**                                         return a valid index)
**
** Returnvalue : CSS_k_OK                - success
**               <>CSS_k_OK              - error while initializing the Safety
**                                         Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
CSS_t_WORD IXSVS_OrigServerInit(
                           const CSS_t_SOPEN_RESP_RX *ps_openRespRx,
                           const CSS_t_CCO_SOPEN_INIT_VAL *ps_ccoSopenInitVal,
                           CSS_t_UINT u16_svInst);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstIdIdxAssign
**
** Description : This function assigns the Safety Validator Server index and
**               Instance ID dependency.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXCCO_CnxnOpen() and
**                                 IXSVO_TargAlloc())
**               u16_svsIdx (IN) - array index of the Safety Validator Server
**                                 array
**                                 (not checked, checked in IXSVO_OrigAlloc()
**                                 IXSVO_TargAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdIdxAssign(CSS_t_UINT u16_instId, CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InstAlloc
**
** Description : This function allocates the first free Server Index of the
**               Safety Validator Object.
**
** Parameters  : -
**
** Returnvalue : >=CSOS_cfg_NUM_OF_SV_CLIENTS - error: it was not possible to
**                                              allocate an index
**               <CSOS_cfg_NUM_OF_SV_CLIENTS  - success: allocated Safety
**                                              Validator index
**
*******************************************************************************/
CSS_t_UINT IXSVS_InstAlloc(void);


/*******************************************************************************
**
** Function    : IXSVS_InstFind
**
** Description : This function searches the Safety Validator Server Instance
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
CSS_t_BOOL IXSVS_InstFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo);
#endif


/*******************************************************************************
**
** Function    : IXSVS_ConsColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection to the INITIALIZING state.
**
** Parameters  : u16_svsIdx (IN)        - Index of the Safety Validator Server
**                                        instance
**                                        (not checked, checked in
**                                        IXSVS_TargServerInit(),
**                                        IXSVS_OrigServerInit() and
**                                        IXCCO_CnxnOpen())
**               b_msgFormat (IN)       - Bit-mapped byte of flags to indicate
**                                        what kind of safety connection this
**                                        Safety Validator Instance refers to
**                                        (not checked, only one bit evaluated,
**                                        any value allowed)
**               u16_initialTS_i (IN)   - only used in case of multi-cast (then
**                                        we are definitely Target): Initial
**                                        Time Stamp received in the
**                                        Forward_Open response.
**                                        (not checked, any value allowed)
**               u16_initialRV_i (IN)   - Initial Rollover Value (in) (usage
**                                        identical to pu16_initialTS_i)
**                                        (not checked, any value allowed)
**               pu16_initialTS_o (OUT) - only used in case of single-cast: If
**                                        we are the Originator then this is the
**                                        value to be sent in the Forward_Open
**                                        request. If our node is Target then
**                                        this is the value to be returned in
**                                        the Forward_Open response.
**                                        (not checked, only called with
**                                        reference to variable)
**               pu16_initialRV_o (OUT) - Initial Rollover Value (out) (usage
**                                        identical to pu16_initialTS_o)
**                                        (not checked, only called with
**                                        reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsColdStartLogic(CSS_t_UINT u16_svsIdx
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                             ,CSS_t_BYTE b_msgFormat
                             ,CSS_t_UINT u16_initialTS_i
                             ,CSS_t_UINT u16_initialRV_i
                             ,CSS_t_UINT *pu16_initialTS_o
                             ,CSS_t_UINT *pu16_initialRV_o
#endif
                             );


/*******************************************************************************
**
** Function    : IXSVS_InstStop
**
** Description : This function stops/deletes the passed Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_SERVERS)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstStop(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_RxDataCrossCheckError
**
** Description : This function reports an error to a Safety Validator Server
**               instance. Base format connection will be set to faulted,
**               extended format connection will increment fault counter and
**               stay open unless fault counter reached limit.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVO_RxDataCrossCheckError())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_RxDataCrossCheckError(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVO_IxsvsCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Server
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
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  void IXSVO_IxsvsCnxnStateTransClbk(CSS_t_USINT u8_oldState,
                                     CSS_t_USINT u8_newState);
#endif


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountGet
**
** Description : This function returns the current value of the Fault Counter of
**               the Safety Validator Server instances. Counts (with
**               auto-rollover) any consuming safety connection was faulted for
**               any reason.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - value of the Safety Validator Fault Count
**
*******************************************************************************/
CSS_t_UINT IXSVS_StateFaultCountGet(void);


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountReset
**
** Description : This function resets the value of the Fault Counter of the
**               Safety Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateFaultCountReset(void);


/*******************************************************************************
**
** Function    : IXSVS_RxProc
**
** Description : This function processes received Safety I/O messages that are
**               directed to Safety Validator Server Instances
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (checked,
**                                 valid range: 0..CSOS_cfg_NUM_OF_SV_SERVERS-1)
**               u16_len (IN)    - length of the received message
**                                 (not checked, checked in
**                                 ValidatorServerRxCopy())
**               pb_data (IN)    - pointer to byte stream that contains the
**                                 received message
**                                 (not checked, called with pointer to received
**                                 HALC message plus offset)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_RxProc(CSS_t_UINT u16_svsIdx,
                  CSS_t_UINT u16_len,
                  const CSS_t_BYTE *pb_data);


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSVS_InstClose) */
/*lint -esym(765, IXSVS_InstClose) */
/*******************************************************************************
**
** Function    : IXSVS_InstClose
**
** Description : This function closes the passed Safety Validator Server.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXSVS_InstCloseAll()
**                                 and IXSVO_InstanceClose() and
**                                 SafetyOpenProc3())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in
**                                 IXSVS_StateUnderlayingComClose())
**               o_stop (IN)     - if CSS_k_TRUE the instance is stopped and the
**                                 event is reported to the SAPL
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
CSS_t_WORD IXSVS_InstClose(CSS_t_UINT u16_instId,
                           CSS_t_USINT u8_consNum,
                           CSS_t_BOOL o_stop);


/*******************************************************************************
**
** Function    : IXSVS_InstCloseAll
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
void IXSVS_InstCloseAll(void);


/*******************************************************************************
**
** Function    : IXSVS_StateGet
**
** Description : This function returns the state of the requested Safety
**               Validator Server Instance.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - State of the requested Safety Validator
**                                 Server Instance
**
*******************************************************************************/
CSS_t_USINT IXSVS_StateGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InitInfoMsgFrmtGet
**
** Description : This function returns the message format of the requested
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE - message format of the requested Safety Validator
**                            Server Instance
**
*******************************************************************************/
CSS_t_BYTE IXSVS_InitInfoMsgFrmtGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_MaxDataAgeGet
**
** Description : This function returns the Max_Data_Age attribute of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler() and
**                                 IXSVS_InstInfoGet())
**
** Returnvalue : CSS_t_UINT      - Max_Data_Age attribute of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_MaxDataAgeGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_MaxDataAgeSet
**
** Description : This function sets the Max_Data_Age attribute of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN)           - index of the Safety Validator
**                                           Server Instance
**                                           (not checked, checked in
**                                           IXSVO_ExplMsgHandler())
**               u16_maxDataAge_128us (IN) - value to be written to the
**                                           Max_Data_Age attribute of the
**                                           requested Safety Validator Server
**                                           (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_MaxDataAgeSet(CSS_t_UINT u16_svsIdx,
                         CSS_t_UINT u16_maxDataAge_128us);


/*******************************************************************************
**
** Function    : IXSVS_InitInfoCnxnPointGet
**
** Description : This function returns the Connection Point which consumes the
**               data received by the requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server
**                                 structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - Connection Point associated with the
**                                 requested Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_InitInfoCnxnPointGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterGet
**
** Description : This function returns the Consumer Fault Counter of the
**               requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_USINT     - Consumer Fault Counter of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_USINT IXSVS_ConsFaultCounterGet(CSS_t_UINT u16_svsIdx);
#endif


/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterReset
**
** Description : This function resets the Consumer Fault Counters of all Safety
**               Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  void IXSVS_ConsFaultCounterReset(void);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstInit
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the IXSVS
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_Init() and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstInit(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_PingIntEpiMultGet
**
** Description : This function returns the value of the Ping Interval EPI
**               Multiplier of the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_UINT      - Ping Interval EPI Multiplier
**
*******************************************************************************/
CSS_t_UINT IXSVS_PingIntEpiMultGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcooMsgMinMultGet
**
** Description : This function returns the value of the Time Coordination
**               Message Min Multiplier of the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_UINT      - Time Coordination Message Min Multiplier
**                                 (128us ticks)
**
*******************************************************************************/
CSS_t_UINT IXSVS_TcooMsgMinMultGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InitInfoNetTimeExpMultGet
**
** Description : This function returns the value of the Network Time Expectation
**               Multiplier of the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_UINT      - Network Time Expectation Multiplier
**                                 (128us ticks)
**
*******************************************************************************/
CSS_t_UINT IXSVS_InitInfoNetTimeExpMultGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TimeoutMultiplierGet
**
** Description : This function returns the value of the Timeout Multiplier of
**               the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_USINT     - Timeout Multiplier
**
*******************************************************************************/
CSS_t_USINT IXSVS_TimeoutMultiplierGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_MaxConsNumGet
**
** Description : This function returns the value of the Max Consumer Number of
**               the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_USINT     - Max Consumer Number
**
*******************************************************************************/
CSS_t_USINT IXSVS_MaxConsNumGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ErrorLastGet
**
** Description : This function returns the last error code of the addressed
**               Safety Validator Server Instance.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_WORD      - Last Error Code
**
*******************************************************************************/
CSS_t_WORD IXSVS_ErrorLastGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateNumServerCnxnGet
**
** Description : This function returns the number of Safety Validator
**               Server Connections that are currently open (in Initializing or
**               Established state).
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Number of open Server Connections
**
*******************************************************************************/
#if (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)
  CSS_t_UINT IXSVS_StateNumServerCnxnGet(void);
#endif  /* (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVS_ConsSoftErrByteGet
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
  CSS_t_UDINT IXSVS_ConsSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_ConsSoftErrVarGet
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
  void IXSVS_ConsSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InitInfoSoftErrByteGet
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
  CSS_t_UDINT IXSVS_InitInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                           CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InitInfoSoftErrVarGet
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
  void IXSVS_InitInfoSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstSoftErrByteGet
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
  CSS_t_UDINT IXSVS_InstSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstSoftErrVarGet
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
  void IXSVS_InstSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstIdSoftErrByteGet
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
  CSS_t_UDINT IXSVS_InstIdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                         CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstIdSoftErrVarGet
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
  void IXSVS_InstIdSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_RecvSoftErrByteGet
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
  CSS_t_UDINT IXSVS_RecvSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_RecvSoftErrVarGet
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
  void IXSVS_RecvSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_StateSoftErrByteGet
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
  CSS_t_UDINT IXSVS_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_StateSoftErrVarGet
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
  void IXSVS_StateSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_TcooSoftErrByteGet
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
  CSS_t_UDINT IXSVS_TcooSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_TcooSoftErrVarGet
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
  void IXSVS_TcooSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_TcorrSoftErrByteGet
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
  CSS_t_UDINT IXSVS_TcorrSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVS_TcorrSoftErrVarGet
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
  void IXSVS_TcorrSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#endif /* #ifndef IXSVS_H */

/*** End Of File ***/


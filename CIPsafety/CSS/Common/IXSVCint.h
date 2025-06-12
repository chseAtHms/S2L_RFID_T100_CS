/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCint.h
**    Summary: IXSVC - Safety Validator Client
**             IXSVCint.h is the internal header file of the IXSVC unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_TcorrMsgProd
**             IXSVC_ProducerClkCountGet
**             IXSVC_RoundupDivision
**             IXSVC_QuarLocalsClear
**             IXSVC_QuarConsStart
**             IXSVC_QuarConsUpdate
**             IXSVC_ConsumerRemainingCheck
**             IXSVC_ErrorInit
**             IXSVC_ErrorClbk
**             IXSVC_StateUnderlayingComClose
**             IXSVC_TcooLocalsClear
**             IXSVC_TcooMsgMinMultSet
**             IXSVC_TcooCidCrcS3Set
**             IXSVC_TcooCidCrcS5Set
**             IXSVC_TcooMaxFaultNumberSet
**             IXSVC_TcooColdStartLogic
**             IXSVC_InitInfoClear
**             IXSVC_InitInfoCalcPtrGet
**             IXSVC_InitInfoSet
**             IXSVC_TcorrLocalsClear
**             IXSVC_TcorrBaseTxCopy
**             IXSVC_TcorrExtTxCopy
**             IXSVC_ProdNextTxTimeSet
**             IXSVC_ProdLocalsClear
**             IXSVC_RrConNumIdxPntrGet
**             IXSVC_RrConNumIdxPntrSet
**             IXSVC_ProdPingIntEpiCntGet
**             IXSVC_StateInit
**             IXSVC_StateIsInitOrEstablished
**             IXSVC_InstIdInit
**             IXSVC_InstIdFromIdxGet
**             IXSVC_InstInfoPerConsSet
**             IXSVC_InstInfoPerConsGet
**             IXSVC_ProdColdStartLogic
**             IXSVC_ProdMcastReinit
**             IXSVC_ProdTcooMsgReception
**             IXSVC_ConsumerIsActive
**             IXSVC_ConsumerActiveIdleSet
**             IXSVC_ConsTimeCorrValGet
**             IXSVC_ConsTimeCorrValSet
**             IXSVC_ProdModeByteGet
**             IXSVC_TxCopy
**             IXSVC_CrcCalc
**             IXSVC_InstInfoClear
**             IXSVC_InstInfoConsumerClose
**             IXSVC_ConnectionFaultFlagSet
**             IXSVC_ConnectionIsFaulted
**             IXSVC_TxFrameGenerate
**             IXSVC_ProdNextTxTimeGet
**             IXSVC_UtilLocalsClear
**             IXSVC_InstInitAll
**             IXSVC_StateMachine
**             IXSVC_FaultCountInc
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVCINT_H
#define IXSVCINT_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSVC_t_INIT_CALC:
    Values that are calculated during the initialization of the connection and
    constant afterwards.
*/
typedef struct
{
  CSS_t_UDINT     u32_rpiOT_us;   /* O_to_T RPI */
  CSS_t_UDINT     u32_rpiTO_us;   /* T_to_O_RPI */

  /* EPI (see Vol. 5 Chapt. 2-4.5.2.1): expected packet interval. Safety data
     shall be produced at a rate not exceeding EPI. The EPI is expressed as an
     integer value in uSec.
     Legal range: 100..1000000 */
  CSS_t_UDINT     u32_EPI_us;

  /* EPI in 128 micro second ticks. If EPI is not divisable by 128 us then
     this is a little less than the EPI in micro seconds. */
  CSS_t_UDINT u32_EPI_128us;
  /* When the EPI is not divisable by 128us then this rest number of micro
     seconds must be added */
  CSS_t_UDINT u32_EpiDeviation_us;

  /* Ping_Interval_EPI_Multiplier (see Vol. 5 Chapt. 2-4.5.2.4): is the number
     that defines the Ping_Count_Interval for a particular connection. */
  CSS_t_UINT      u16_Ping_Interval_EPI_Multiplier;

  /*
  ** seed values for CRC calculations
  */
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    CSS_t_USINT   u8_pidCrcS1;    /* CRC-S1 over PID */
  #endif
  CSS_t_UINT      u16_pidCrcS3;   /* CRC-S3 over PID */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UDINT   u32_pidCrcS5;   /* CRC-S5 over PID */
  #endif

  /* Produced Connection Size: number of payload data to be transmitted */
  CSS_t_USINT     u8_prodDataSize;

} IXSVC_t_INIT_CALC;




/** IXSVC_t_INIT_INFO:
    Values per Instance being written during initialization.
*/
typedef struct
{
  /** b_msgFormat:
      Bit-mapped byte of flags (combinations of {IXSVD_k_MSG_FORMAT_xxx}) to
      indicate what kind of safety connection this Safety Validator Instance
      refers to:
      Bit positions:
      7  6  5  4  3  2  1  0
      +---------  |  |  |  |
      |           |  |  |  +------ 0 == Short        1 == Long Message Format
      |           |  |  +--------- 0 == Base         1 == Extended Msg Format
      |           |  +------------ 0 == Single-Cast  1 == Multi-Cast
      |           +--------------- 0 == Target       1 == Originator
      +--------------------------- not defined
  */
  CSS_t_BYTE b_msgFormat;

  /* Max_Consumer_Number (see Vol. 5 Chapt. 2-4.5.2.5): maximum number of
     consumers that may be configured.
     Legal range: single-cast: 1  multi-cast: 1..15 */
  CSS_t_USINT     u8_Max_Consumer_Number; /* maximum number of consumers */

  /* Producing Connection Point
     (Input Assembly Instance or Index of Produced Data Process Image) */
  CSS_t_UINT      u16_ProdCnxnPoint;

  /* struct for storing further initialization data (CRC seeds, etc.) */
  IXSVC_t_INIT_CALC s_initCalc;



} IXSVC_t_INIT_INFO;


/** IXSVC_t_INST_INFO_PER_CONS:
    Values per Consumer.
*/
typedef struct
{
  /* Consumer_Open (see Vol. 5 Chapt. 2-4.5.1.1): indicates whether or not a
     particular consumer of the produced data has an active open connection.
     Legal Values: CSS_k_TRUE ( == open) or CSS_k_FALSE (== false) */
  CSS_t_BYTE      o_Consumer_Open;  /* (see FRS323) */

  /* Timeout_Multiplier (see Vol. 5 Chapt. 2-4.5.2.2): indicates the number of
     data production retries to use in the connection failure detection
     equations.
  */
  struct
  {
    CSS_t_USINT u8_PI;    /* PI = Ping Interval*/  /* Legal range: 1..4 */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_USINT u8_ef;    /* Legal range: 1..255 */
  #endif
  } s_Timeout_Multiplier;

  /* CID: Consumer Identification (storage is needed for comparison in
     Forward_Close) */
  CSS_t_PIDCID   s_cid;

} IXSVC_t_INST_INFO_PER_CONS;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_TcorrMsgProd
**
** Description : This function produces a Time Correction message. This logic
**               was part of the IXSVC_MultiCastProducerFunction() but put into
**               an own function for clarity and symmetry to the Safety
**               Validator Server.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcorrMsgProd(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_ProducerClkCountGet
**
** Description : This function returns the current value of the global timer.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - counter that is incremented by a periodic timer
**                             every 128 uSec (16 bits, see FRS249).
**
*******************************************************************************/
CSS_t_UINT IXSVC_ProducerClkCountGet(void);


/*******************************************************************************
**
** Function    : IXSVC_RoundupDivision
**
** Description : This function performs a division and rounds up the result.
**               This means that whenever there is a rest in the division then
**               the result will be rounded up to the next integer.
**               This operation is needed several times in the Safety Validator.
**
** Parameters  : u32_dividend (IN) - dividend
**                                   (not checked, any value allowed)
**               u32_divisor (IN)  - divisor
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT       - rounded up result of division or zero in
**                                   case the passed divisor is zero
**
*******************************************************************************/
CSS_t_UDINT IXSVC_RoundupDivision(CSS_t_UDINT u32_dividend,
                                  CSS_t_UDINT u32_divisor);


/*******************************************************************************
**
** Function    : IXSVC_QuarLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_QuarLocalsClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_QuarConsStart
**
** Description : This function starts the quarantine timer for a consumer number
**               of a Validator Client Instance.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate() and
**                                 ValidatorRxProc())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in
**                                 MultiCastProducerFunction() and
**                                 IXSVC_TcooRxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_QuarConsStart(CSS_t_UINT u16_svcIdx,
                         CSS_t_USINT u8_consNum);


/*******************************************************************************
**
** Function    : IXSVC_QuarConsUpdate
**
** Description : This function must be called cyclically to update the
**               quarantine timers of one Safety Validator Client instance.
**               It must be called at least once per producing EPI.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : 1..CSOS_k_MAX_CONSUMER_NUM_MCAST - consumer number that the
**                                                  caller may close now
**               else                             - no consumer needs to be
**                                                  closed
**
*******************************************************************************/
CSS_t_USINT IXSVC_QuarConsUpdate(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_ConsumerRemainingCheck
**
** Description : This function checks if a Safety Validator Client is connected
**               to consumers
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in IXSVC_InstClose()
**                                 IXSVO_InstanceStop(),
**                                 IXSVC_InstTxFrameGenerate() and
**                                 ValidatorRxProc())
**
** Returnvalue : CSS_k_TRUE      - There are still Consumers left
**               CSS_k_FALSE     - Safety Validator has no Consumers
**
*******************************************************************************/
CSS_t_BOOL IXSVC_ConsumerRemainingCheck(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_ErrorInit
**
** Description : This function initializes the global variables of the Error
**               module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ErrorInit(void);


/*******************************************************************************
**
** Function    : IXSVC_ErrorClbk
**
** Description : This callback function is used within the IXSVC unit instead of
**               directly calling SAPL_CssErrorClbk(). Instance related errors
**               are stored for being read in SVO attribute 14. This function
**               then calls SAPL_CssErrorClbk().
**
** Parameters  : w_errorCode (IN) - 16 bit value that holds Error Type, Error
**                                  Layer, Unit ID and Unit Error.
**                                  (not checked, any value allowed)
**               u16_instId (IN)  - instance Id of the safety validator
**                                  instance that caused the error
**                                  (checked, IXSVD_SvIdxFromInstGet()
**                                  must return a valid index)
**               dw_addInfo (IN)  - additional error information.
**                                  (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ErrorClbk(CSS_t_WORD w_errorCode,
                     CSS_t_UINT u16_instId,
                     CSS_t_DWORD dw_addInfo);


/*******************************************************************************
**
** Function    : IXSVC_StateUnderlayingComClose
**
** Description : This function sends a close command to the CSAL. For multicast
**               connections: if a consumer number is passed
**               (1..CSOS_k_MAX_CONSUMER_NUM_MCAST) then only the connection
**               to this consumer is closed. If 0 is passed then the connections
**               to all consumers of this instance are closed.
**
** Parameters  : u16_svcIdx (IN) - Safety Validator Server index
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_CLIENTS-1)
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range:
**                                 0 and 1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: HALC not ready
**
*******************************************************************************/
CSS_t_WORD IXSVC_StateUnderlayingComClose(CSS_t_UINT u16_svcIdx,
                                          CSS_t_USINT u8_consNum);


/*******************************************************************************
**
** Function    : IXSVC_TcooLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcooLocalsClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TcooMsgMinMultSet
**
** Description : This function sets the value of the Time Coordination
**               Message Min Multiplier of the addressed instance's consumer to
**               the passed value.
**
** Parameters  : u16_svcIdx (IN)      - index of Safety Validator Client
**                                      structure
**                                      (not checked, checked in
**                                      IXSVC_OrigClientInit() and
**                                      TargClientConsumerInit())
**               u8_consIdx (IN)      - Consumer number
**                                      (0..Max_Consumer_Number-1)
**                                      (not checked, called with constant or
**                                      checked in ConsIdxAlloc())
**               u16_value_128us (IN) - value to be set
**                                      (checked, valid range:
**                                      0..k_MAX_TCOO_MSG_MIN_MULTIPLIER_128US)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcooMsgMinMultSet(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consIdx,
                             CSS_t_UINT u16_value_128us);


/*******************************************************************************
**
** Function    : IXSVC_TcooCidCrcS3Set
**
** Description : This function sets the value of the u16_cidCrcS3 of the
**               addressed instance's consumer to the passed value.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit() and
**                                 TargClientConsumerInit())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**                                 (not checked, called with constant or checked
**                                 in ConsIdxAlloc())
**               u16_value (IN)  - value to be set
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  void IXSVC_TcooCidCrcS3Set(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consIdx,
                             CSS_t_UINT u16_value);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooCidCrcS5Set
**
** Description : This function sets the value of the u32_cidCrcS5 of the
**               addressed instance's consumer to the passed value.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit() and
**                                 TargClientConsumerInit())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**                                 (not checked, called with constant or checked
**                                 in ConsIdxAlloc())
**               u32_value (IN)  - value to be set
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  void IXSVC_TcooCidCrcS5Set(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consIdx,
                             CSS_t_UDINT u32_value);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooMaxFaultNumberSet
**
** Description : This function sets the value of the u8_Max_Fault_Number
**               of the addressed instance to the passed value.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit() and
**                                 TargClientConsumerInit())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**                                 (not checked, called with constant or checked
**                                 in ConsIdxAlloc())
**               u8_value (IN)   - value to be set
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  void IXSVC_TcooMaxFaultNumberSet(CSS_t_UINT u16_svcIdx,
                                   CSS_t_USINT u8_consIdx,
                                   CSS_t_USINT u8_value);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcooColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection from closed to open and initializes Time
**               Coordination specific Variables of the Safety Validator Client.
**               For Single-Cast connections this logic is performed any time
**               the connection is Opened or Re-Opened. For multicast producers
**               this is performed after every successful open to a multicast
**               consumer while the production is in process to re-initialize
**               production to that consumer.
**
** Parameters  : u16_svcIdx (IN)  - index of the Safety Validator Client
**                                  instance
**                                  (not checked, checked in
**                                  IXSVC_OrigClientInit() and
**                                  TargClientConsumerInit())
**               u8_consIdx (IN)  - if equal or greater than
**                                  CSOS_k_MAX_CONSUMER_NUM_MCAST:
**                                  initialize all consumers
**                                  else: initialize only the consumer with the
**                                  passed index addressed instance
**                                  (checked, see above)
**               ps_initInfo (IN) - pointer to a struct for collecting
**                                  initialization data
**                                  (not checked, only called with
**                                  reference to struct)
**               ps_iipc (IN)     - pointer to a struct containing per consumer
**                                  data
**                                  (not checked, only called with
**                                  reference to struct)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcooColdStartLogic(CSS_t_UINT u16_svcIdx,
                              CSS_t_USINT u8_consIdx,
                              const IXSVC_t_INIT_INFO *ps_initInfo,
                              const IXSVC_t_INST_INFO_PER_CONS *ps_iipc);


/*******************************************************************************
**
** Function    : IXSVC_InitInfoClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structures.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVC_Init() and IXSVC_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InitInfoClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InitInfoCalcPtrGet
**
** Description : This function returns a const pointer to the values calculated
**               during initialization of the connection (e.g. PID/CID CRC
**               seeds) of the requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN)    - index of Safety Validator Client
**                                    structure
**                                    (not checked, checked in
**                                    IXSVC_InstTxFrameGenerate(),
**                                    ValidatorRxProc(), IXSVC_TargClientInit()
**                                    and IXSVC_McastCnxnFind())
**
** Returnvalue : IXSVC_t_INIT_CALC* - const pointer to the structure containing
**                                    the init calc data
**
*******************************************************************************/
const IXSVC_t_INIT_CALC* IXSVC_InitInfoCalcPtrGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InitInfoSet
**
** Description : This function assigns the values of the passed struct to the
**               elements of the local arrays.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in
**                                  IXSVC_OrigClientInit() and
**                                  TargClientConsumerInit())
**               ps_initInfo (IN) - pointer to struct containing the init infos
**                                  (pointer not checked, only called with
**                                  reference to structure variable
**                                  valid ranges:
**                                  -b_msgFormat (not checked, only called with
**                                   constants)
**                                  -u8_Max_Consumer_Number (not checked,
**                                   checked in CnxnParamsValidateSafety2() or
**                                   called with constant)
**                                  -u16_ProdCnxnPoint (not checked, any value
**                                   allowed)
**                                  -s_initCalc.u32_rpiOT_us (not checked, any
**                                   allowed value)
**                                  -s_initCalc.u32_rpiTO_us (not checked, any
**                                   allowed value)
**                                  -s_initCalc.u32_EPI_us (checked, valid
**                                   range: k_EPI_MIN_US..k_EPI_MAX_US)
**                                  -s_initCalc.u16_Ping_Interval_EPI_Multiplier
**                                   (not checked, checked in
**                                   RangeCheckInstPiem() and
**                                   CnxnParamsValidateSafety1())
**                                  -s_initCalc.u8_pidCrcS1 (not checked, any
**                                   allowed value)
**                                  -s_initCalc.u16_pidCrcS3 (not checked, any
**                                   allowed value)
**                                  -s_initCalc.u32_pidCrcS5 (not checked, any
**                                   allowed value)
**                                  -s_initCalc.u8_prodDataSize (checked, valid
**                                   range: if Base supported: 1..2 and/or
**                                   if Extended supported:
**                                   3..CSOS_cfg_LONG_FORMAT_MAX_LENGTH)
**
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InitInfoSet(CSS_t_UINT u16_svcIdx,
                       const IXSVC_t_INIT_INFO *ps_initInfo);


/*******************************************************************************
**
** Function    : IXSVC_TcorrLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcorrLocalsClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TcorrBaseTxCopy
**
** Description : This function copies the data of a Time Correction Message
**               to be transmitted to the passed byte stream (Base Format).
**
** Parameters  : u16_svcIdx (IN)   - Instance Index of the Safety Validator
**                                   Client
**                                   (not checked, checked in
**                                   IXSVC_InstTxFrameGenerate())
**               u16_offset (IN)   - offset in the passed buffer where the Tcorr
**                                   message will start
**                                   (not checked, checked in
**                                   IXSVC_TxFrameGenerate())
**               u16_frameLen (IN) - overall length of the provided buffer
**                                   (not checked, checked in called function
**                                   IXSFA_BfTcrMsgWrite())
**               pb_data (OUT)     - pointer to byte stream where the transmit
**                                   message is to be stored
**                                   (not checked, only called with pointer to
**                                   array variable)
**
** Returnvalue : CSS_t_UINT        - number of bytes that are now in the pb_data
**                                   buffer (u16_offset + number of bytes copied
**                                   by this function
**
*******************************************************************************/
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_UINT IXSVC_TcorrBaseTxCopy(CSS_t_UINT u16_svcIdx,
                                   CSS_t_UINT u16_offset,
                                   CSS_t_UINT u16_frameLen,
                                   CSS_t_BYTE *pb_data);
#endif


/*******************************************************************************
**
** Function    : IXSVC_TcorrExtTxCopy
**
** Description : This function copies the data of a Time Correction Message
**               to be transmitted to the passed byte stream (Extended Format).
**
** Parameters  : u16_svcIdx (IN)   - Instance Index of the Safety Validator
**                                   Client
**                                   (not checked, checked in
**                                   IXSVC_InstTxFrameGenerate())
**               u16_offset (IN)   - offset in the passed buffer where the Tcorr
**                                   message will start
**                                   (not checked, checked in
**                                   IXSVC_TxFrameGenerate())
**               u16_frameLen (IN) - overall length of the provided buffer
**                                   (not checked, checked in called function
**                                   IXSFA_EfTcrMsgWrite())
**               pb_data (OUT)     - pointer to byte stream where the transmit
**                                   message is to be stored
**                                   (not checked, only called with pointer to
**                                   array variable)
**
** Returnvalue : CSS_t_UINT        - number of bytes that are now in the pb_data
**                                   buffer (u16_offset + number of bytes copied
**                                   by this function
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UINT IXSVC_TcorrExtTxCopy(CSS_t_UINT u16_svcIdx,
                                  CSS_t_UINT u16_offset,
                                  CSS_t_UINT u16_frameLen,
                                  CSS_t_BYTE *pb_data);
#endif


/*******************************************************************************
**
** Function    : IXSVC_ProdNextTxTimeSet
**
** Description : This function sets the time when the next data message is to be
**               produced.
**
** Parameters  : u16_svcIdx (IN)       - index of Safety Validator Client
**                                       structure
**                                       (not checked, checked in
**                                       IXSVC_OrigClientInit(),
**                                       TargClientConsumerInit() and
**                                       IXSVC_InstTxFrameGenerate())
**               u32_txTime_128us (IN) - system time when the next transmission
**                                       will occur
**                                       (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ProdNextTxTimeSet(CSS_t_UINT u16_svcIdx,
                             CSS_t_UDINT u32_txTime_128us);


/*******************************************************************************
**
** Function    : IXSVC_ProdLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ProdLocalsClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_RrConNumIdxPntrGet
**
** Description : This function returns the value of the RR Consumer Number Index
**               Pointer of the addressed Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : CSS_t_USINT     - RR Consumer Number Index Pointer
**
*******************************************************************************/
CSS_t_USINT IXSVC_RrConNumIdxPntrGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_RrConNumIdxPntrSet
**
** Description : This function returns the value of the RR Consumer Number Index
**               Pointer of the addressed Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN)    - index of Safety Validator Client structure
**                                    (not checked, checked in
**                                    IXSVC_OrigClientInit(),
**                                    TargClientConsumerInit() and
**                                    IXSVC_InstTxFrameGenerate())
**               u8_value (IN)      - value to be set
**                                    (checked, valid range:
**                                    0..(as_initInfo.u8_Max_Consumer_Number-1)
**                                    additionally it may also be set to
**                                    as_initInfo.u8_Max_Consumer_Number to
**                                    indicate that all consumers have been
**                                    served in the current Ping Interval.
**               u8_maxConsNum (IN) - Maximum Consumer Number - for checking if
**                                    the passed value is valid
**                                    (checked, valid range:
**                                    1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_RrConNumIdxPntrSet(CSS_t_UINT u16_svcIdx,
                              CSS_t_USINT u8_value,
                              CSS_t_USINT u8_maxConsNum);


/*******************************************************************************
**
** Function    : IXSVC_ProdPingIntEpiCntGet
**
** Description : This function returns the value of the Producer Ping Interval
**               EPI Counter of the addressed Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in ValidatorRxProc())
**
** Returnvalue : CSS_t_UINT      - Producer Ping Interval EPI Counter
**
*******************************************************************************/
CSS_t_UINT IXSVC_ProdPingIntEpiCntGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_StateInit
**
** Description : This function initializes the state variables of a Safety
**               Validator Client instance
**
** Parameters  : u16_svcIdx (IN) - Index of the Safety Validator Client instance
**                                 (not checked, checked in IXSVC_Init() and
**                                 IXSVC_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_StateInit(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_StateIsInitOrEstablished
**
** Description : This function checks the state of the addressed Safety
**               Validator Client Instance
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - state of the instance is
**                                 CSS_k_SV_STATE_INITIALIZING or
**                                 CSS_k_SV_STATE_ESTABLISHED
**               CSS_k_FALSE     - instance is in other state
**
*******************************************************************************/
CSS_t_BOOL IXSVC_StateIsInitOrEstablished(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InstIdInit
**
** Description : This function initializes the lookup table for the Safety
**               Validator Client Instance Ids
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstIdInit(void);


/*******************************************************************************
**
** Function    : IXSVC_InstIdFromIdxGet
**
** Description : This function converts a Safety Validator Client array index
**               into the corresponding Instance ID.
**
** Parameters  : u16_svcIdx (IN) - array index of the Safety Validator Client
**                                 array
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - instance ID of the Client Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVC_InstIdFromIdxGet(CSS_t_UINT u16_svcIdx);



/*******************************************************************************
**
** Function    : IXSVC_InstInfoPerConsSet
**
** Description : This function sets the data of the per consumer instance info
**               of the addressed Safety Validator Client's Consumer.
**
** Parameters  : u16_svcIdx (IN)   - Instance index of the Safety Validator
**                                   Client instance
**                                   (not checked, checked in
**                                   IXSVC_OrigClientInit() and
**                                   IXSVC_TargClientInit())
**               u8_consIdx (IN)   - Consumer Index of the addressed consumer
**                                   (not checked, called with constant or
**                                   checked in ConsIdxAlloc())
**               ps_iiPerCons (IN) - pointer to structure containing the data
**                                   values to be set
**                                   (not checked, only called with reference to
**                                   structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInfoPerConsSet(CSS_t_UINT u16_svcIdx,
                              CSS_t_USINT u8_consIdx,
                              const IXSVC_t_INST_INFO_PER_CONS *ps_iiPerCons);


/*******************************************************************************
**
** Function    : IXSVC_InstInfoPerConsGet
**
** Description : This function returns a pointer to the per consumer instance
**               info of the addressed Safety Validator Client's Consumer.
**
** Parameters  : u16_svcIdx (IN)             - Instance index of the Safety
**                                             Validator Client instance
**                                             (not checked, checked by callers)
**               u8_consIdx (IN)             - Consumer Index of the addressed
**                                             consumer
**                                             (not checked, checked by callers)
**
** Returnvalue : IXSVC_t_INST_INFO_PER_CONS* - const pointer to the structure
**                                             containing the per consumer data
**
*******************************************************************************/
const IXSVC_t_INST_INFO_PER_CONS* IXSVC_InstInfoPerConsGet(
                                                        CSS_t_UINT u16_svcIdx,
                                                        CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_ProdColdStartLogic
**
** Description : This function is executed at the transition of the producing
**               connection from closed to open. For Single-Cast connections
**               this logic is performed any time the connection is Opened or
**               Re-Opened.
**
** Parameters  : u16_svcIdx (IN)         - Instance index of the Safety
**                                         Validator Client instance
**                                         (not checked, checked in
**                                         IXSVC_OrigClientInit() and
**                                         TargClientConsumerInit())
**               ps_initInfo (IN)        - pointer to a struct for collecting
**                                         initialization data
**                                         (not checked, only called with
**                                         reference to struct)
**               pu16_InitialTS (IN/OUT) - If our Node is the Originator: then
**                                         this is the Initial Time Stamp
**                                         received in the Forward_Open
**                                         response. If our Node is the Target:
**                                         then this is the value to be returned
**                                         in the Forward_Open response.
**                                         (not checked, only called with
**                                         reference to variable)
**               pu16_InitialRV (IN/OUT) - If our Node is the Originator: then
**                                         this is the Initial Rollover Value
**                                         received in the Forward_Open
**                                         response. If our Node is the Target:
**                                         then this is the value to be returned
**                                         in the Forward_Open response.
**                                         (not checked, only called with
**                                         reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ProdColdStartLogic(CSS_t_UINT u16_svcIdx,
                              const IXSVC_t_INIT_INFO *ps_initInfo
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                              ,CSS_t_UINT *pu16_InitialTS
                              ,CSS_t_UINT *pu16_InitialRV
#endif
                             );


/*******************************************************************************
**
** Function    : IXSVC_ProdMcastReinit
**
** Description : This function is executed after a multicast consumer performs
**               a successful open to a multicast producer while the production
**               is in process to re-initialize production to that consumer.
**
** Parameters  : u16_svcIdx (IN)      - Instance index of the Safety Validator
**                                      Client instance
**                                      (not checked, checked in
**                                      TargClientConsumerInit())
**               u8_consIdx (IN)      - consumer index
**                                      (not checked, checked in ConsIdxAlloc())
**               pu16_InitialTS (OUT) - our Node is the Target: this is the
**                                      Initial Time Stamp to be returned in the
**                                      Forward_Open response.
**                                      (not checked, only called with reference
**                                      to variable)
**               pu16_InitialRV (OUT) - this is the Initial Rollover Value to be
**                                      returned in the Forward_Open response.
**                                      (not checked, only called with reference
**                                      to variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  void IXSVC_ProdMcastReinit(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consIdx
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                              ,CSS_t_UINT *pu16_InitialTS
                              ,CSS_t_UINT *pu16_InitialRV
  #endif
                             );
#endif


/*******************************************************************************
**
** Function    : IXSVC_ProdTcooMsgReception
**
** Description : This function is a sub-function of IXSVC_TcooMsgReception and
**               is based on Volume 5 sample code.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in ValidatorRxProc())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in IXSVC_TcooRxProc())
**               u16_Time_Drift_Per_Ping_Interval_128us (IN) - represents the
**                                 worst-case time drift due to crystal
**                                 inaccuracy during one Ping Interval
**                                 (checked, valid range:
**                                 k_TIME_DRIFT_PER_PING_INT_MIN_128US..
**                                 k_TIME_DRIFT_PER_PING_INT_MAX_128US)
**               u16_Worst_Case_Consumer_Time_Correction_Value_128us (IN) - This
**                                 is a temporary value used to determine the
**                                 Consumer_Time_Correction_Value
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ProdTcooMsgReception(CSS_t_UINT u16_svcIdx,
                CSS_t_USINT u8_consNum,
                CSS_t_UINT u16_Time_Drift_Per_Ping_Interval_128us,
                CSS_t_UINT u16_Worst_Case_Consumer_Time_Correction_Value_128us);


/*******************************************************************************
**
** Function    : IXSVC_ConsumerIsActive
**
** Description : This function returns true if the Consumer_Active_Idle flag of
**               the addressed instance's consumer is ACTIVE.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in IXSVC_InstInfoGet(),
**                                 ValidatorRxProc() and
**                                 IXSVC_InstTxFrameGenerate())
**               u8_consIdx (IN) - Consumer Index
**                                 (not checked, checked in IXSVC_InstInfoGet(),
**                                 IXSVC_TcooRxProc() and IXSVC_TcorrMsgProd())
**
** Returnvalue : CSS_k_TRUE      - consumer is ACTIVE
**               CSS_k_FALSE     - consumer is IDLE
**
*******************************************************************************/
CSS_t_BOOL IXSVC_ConsumerIsActive(CSS_t_UINT u16_svcIdx,
                                  CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_ConsumerActiveIdleSet
**
** Description : This function sets the Consumer_Active_Idle flag of the
**               addressed instance's consumer to the passed value.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in ValidatorRxProc(),
**                                 IXSVC_OrigClientInit() and
**                                 TargClientConsumerInit())
**               u8_consIdx (IN) - Consumer Index
**                                 (not checked, checked in
**                                 IXSVC_ProdColdStartLogic(), ConsIdxAlloc()
**                                 and IXSVC_TcooRxProc())
**               b_value (IN)    - value to be set
**                                 (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ConsumerActiveIdleSet(CSS_t_UINT u16_svcIdx,
                                 CSS_t_USINT u8_consIdx,
                                 CSS_t_BYTE b_value);


/*******************************************************************************
**
** Function    : IXSVC_ConsTimeCorrValGet
**
** Description : This function returns the Consumer_Time_Correction_Value of
**               the addressed instance's consumer.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in ValidatorRxProc()
**                                 and IXSVC_InstTxFrameGenerate())
**               u8_consIdx (IN) - Consumer Index
**                                 (not checked, checked in IXSVC_TcooRxProc()
**                                 and IXSVC_TcorrMsgProd() or called with
**                                 constant)
**
** Returnvalue : CSS_t_UINT      - Consumer_Time_Correction_Value
**
*******************************************************************************/
CSS_t_UINT IXSVC_ConsTimeCorrValGet(CSS_t_UINT u16_svcIdx,
                                    CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_ConsTimeCorrValSet
**
** Description : This function sets the Consumer_Time_Correction_Value of
**               the addressed instance's consumer to the passed value.
**
** Parameters  : u16_svcIdx (IN)      - Instance index of the Safety Validator
**                                      Client instance
**                                      (not checked, checked in
**                                      ValidatorRxProc(),
**                                      IXSVC_OrigClientInit() and
**                                      TargClientConsumerInit())
**               u8_consIdx (IN)      - Consumer Index
**                                      (not checked, checked in
**                                      IXSVC_ProdColdStartLogic(),
**                                      ConsIdxAlloc() and IXSVC_TcooRxProc())
**               u16_value_128us (IN) - value to be set
**                                      (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ConsTimeCorrValSet(CSS_t_UINT u16_svcIdx,
                              CSS_t_USINT u8_consIdx,
                              CSS_t_UINT u16_value_128us);


/*******************************************************************************
**
** Function    : IXSVC_ProdModeByteGet
**
** Description : This function returns the Mode byte of the Data message.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in ValidatorRxProc())
**
** Returnvalue : CSS_t_BYTE      - Mode Byte
**
*******************************************************************************/
CSS_t_BYTE IXSVC_ProdModeByteGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_TxCopy
**
** Description : This function determines the message format of a Safety
**               Validator Client Instance and copies the data to be transmitted
**               to the passed byte stream.
**
** Parameters  : u16_svcIdx (IN) - Instance Index of the Safety Validator Client
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**               ps_dataMsg (IN) - pointer to the data message of this instance
**                                 (not checked, only called with reference to
**                                 structure variable)
**               pu16_len (OUT)  - length of the transmit message
**                                 (not checked, only called with reference to
**                                 variable)
**               pb_data (OUT)   - pointer to byte stream where the transmit
**                                 message is to be stored
**                                 (not checked, only called with reference to
**                                 array variable)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
CSS_t_WORD IXSVC_TxCopy(CSS_t_UINT u16_svcIdx,
                        const CSS_t_DATA_MSG *ps_dataMsg,
                        CSS_t_UINT *pu16_len,
                        CSS_t_BYTE *pb_data);


/*******************************************************************************
**
** Function    : IXSVC_CrcCalc
**
** Description : This function calculates the complement data and all CRCs of
**               the message produced by the Safety Validator Client. Thus the
**               caller of this function does not have to care whether the
**               Validator Instance refers to Base or Extended and Short or Long
**               Format.
**
** Parameters  : u16_svcIdx (IN)     - Instance index of the Safety Validator
**                                     Client instance
**                                     (not checked, checked in
**                                     IXSVC_InstTxFrameGenerate())
**               u16_pidRcCrcS3 (IN) - value of the CRC-S3 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               u32_pidRcCrcS5 (IN) - value of the CRC-S5 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               ps_dataMsg (IN/OUT) - pointer to the data message of this
**                                     instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - ab_cData
**                                        - u32_cCrcS5
**                                        - u16_aCrcS3
**                                        - u16_cCrcS3
**                                        - u8_tCrcS1
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                     (not checked, only called with reference
**                                     to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_CrcCalc(CSS_t_UINT u16_svcIdx,
               #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                 #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
                   CSS_t_UINT u16_pidRcCrcS3,
                 #endif
                   CSS_t_UDINT u32_pidRcCrcS5,
               #endif
                   CSS_t_DATA_MSG *ps_dataMsg);


/*******************************************************************************
**
** Function    : IXSVC_InstInfoClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structures.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVC_Init() and IXSVC_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInfoClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InstInfoConsumerClose
**
** Description : This function sets the o_Consumer_Open flag to FALSE of the
**               addressed consumer.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**               u8_consIdx (IN) - Consumer number (0..Max_Consumer_Number-1)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInfoConsumerClose(CSS_t_UINT u16_svcIdx,
                                 CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_ConnectionFaultFlagSet
**
** Description : This function sets the S_Connection_Fault flag of
**               the addressed instance's consumer to the passed value.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit(),
**                                 TargClientConsumerInit(),
**                                 IXSVC_InstTxFrameGenerate() and
**                                 ValidatorRxProc())
**               u8_consIdx (IN) - Consumer Index
**                                 (not checked, checked in
**                                 IXSVC_ProdColdStartLogic(), ConsIdxAlloc(),
**                                 MultiCastProducerFunction() and
**                                 IXSVC_TcooRxProc() or called with constant)
**               b_value (IN)    - value to be set
**                                 (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_ConnectionFaultFlagSet(CSS_t_UINT u16_svcIdx,
                                  CSS_t_USINT u8_consIdx,
                                  CSS_t_BYTE b_value);


/*******************************************************************************
**
** Function    : IXSVC_ConnectionIsFaulted
**
** Description : This function returns true if the S_Connection_Fault flag of
**               the addressed instance's consumer is FAULTED.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in IXSVC_InstInfoGet(),
**                                 ValidatorRxProc(),
**                                 IXSVC_InstTxFrameGenerate(),
**                                 IXSVC_InstClose() and IXSVO_InstanceStop())
**               u8_consIdx (IN) - Consumer Index
**                                 (not checked, checked in IXSVC_InstInfoGet(),
**                                 IXSVC_TcooRxProc(), IXSVC_QuarConsUpdate(),
**                                 IXSVC_TcorrMsgProd() and
**                                 IXSVC_ConsumerRemainingCheck())
**
** Returnvalue : CSS_k_TRUE      - consumer is in FAULT state
**               CSS_k_FALSE     - consumer is OK
**
*******************************************************************************/
CSS_t_BOOL IXSVC_ConnectionIsFaulted(CSS_t_UINT u16_svcIdx,
                                     CSS_t_USINT u8_consIdx);


/*******************************************************************************
**
** Function    : IXSVC_TxFrameGenerate
**
** Description : This function is a sub-function of IXSVC_InstTxFrameGenerate().
**               It is called when a Safety Validator Client Instance has to
**               produce data.
**
** Parameters  : u16_svcInstId (IN) - Instance Id of the Safety Validator Client
**                                    (not checked, checked in
**                                    IXSVC_InstTxFrameGenerate())
**               u16_svcIdx (IN)    - index of the Safety Validator Client
**                                    Instance
**                                    (not checked, checked in
**                                    IXSVC_InstTxFrameGenerate())
**
** Returnvalue : CSS_k_OK           - OK
**               <>CSS_k_OK         - error during execution of the function
**
*******************************************************************************/
CSS_t_WORD IXSVC_TxFrameGenerate(CSS_t_UINT u16_svcInstId,
                                 CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_ProdNextTxTimeGet
**
** Description : This function returns the time when the next data message is to
**               be produced.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : CSS_t_UDINT     - absolute time (in 128us system ticks) when
**                                 the next transmission has to be performed for
**                                 this instance
**
*******************************************************************************/
CSS_t_UDINT IXSVC_ProdNextTxTimeGet(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_UtilLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_UtilLocalsClear(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**
** Function    : IXSVC_InstInitAll
**
** Description : This function initializes all static variables of this module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInitAll(void);


/*******************************************************************************
**
** Function    : IXSVC_StateMachine
**
** Description : This function implements the Safety Validator Object State
**               machine as specified by the CIP Safety Specification.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked by callers)
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, only passed to
**                                 IXSVC_StateUnderlayingComClose() and checked
**                                 there)
**               e_svEvent (IN)  - specifies the event that occurred
**                                 (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_StateMachine(CSS_t_UINT u16_svcIdx,
                        CSS_t_USINT u8_consNum,
                        IXSVC_t_VALIDATOR_EVENT e_svEvent);


/*******************************************************************************
**
** Function    : IXSVC_FaultCountInc
**
** Description : This function increments the value of the Fault Counter of the
**               Safety Validator Client instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_FaultCountInc(void);


#endif /* #ifndef IXSVCINT_H */

/*** End Of File ***/


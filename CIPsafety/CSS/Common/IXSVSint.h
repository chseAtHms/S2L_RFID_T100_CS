/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSint.h
**    Summary: IXSVS - Safety Validator Server
**             IXSVSint.h is the internal header file of the IXSVS unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_InitInfoSet
**             IXSVS_InitInfoClear
**             IXSVS_InitInfoCalcPtrGet
**             IXSVS_ConsumerClkCountGet
**             IXSVS_ConsLocalsClear
**             IXSVS_TimeoutMultiplierInit
**             IXSVS_RecvLocalsClear
**             IXSVS_ConsDataSizeSet
**             IXSVS_ConsCnxnStatusGet
**             IXSVS_StateInitTimerStop
**             IXSVS_StateInitLimitCalc
**             IXSVS_StateInitLimitCheck
**             IXSVS_InstIdInit
**             IXSVS_InstIdIdxStop
**             IXSVS_StateInit
**             IXSVS_StateIsInitOrEstablished
**             IXSVS_TcooLocalsClear
**             IXSVS_TcooColdStartLogic
**             IXSVS_TcooPingCountCheck
**             IXSVS_MultiCastConsumerFunction
**             IXSVS_TcorrLocalsClear
**             IXSVS_TcorrColdStartLogic
**             IXSVS_TcorrPingIntCountIncrement
**             IXSVS_ConsFaultCounterIncrement
**             IXSVS_TcorrRxCopy
**             IXSVS_DataLocalsClear
**             IXSVS_DataPtrGet
**             IXSVS_DataModeByteGet
**             IXSVS_DataTimeStampGet
**             IXSVS_DataMsgPtrGet
**             IXSVS_DataRxCopy
**             IXSVS_ConFaultFlagGet
**             IXSVS_ConFaultFlagSet
**             IXSVS_MsgIntegCheckBaseShort
**             IXSVS_MsgIntegCheckBaseLong
**             IXSVS_MsgIntegCheckExtShort
**             IXSVS_MsgIntegCheckExtLong
**             IXSVS_ErrorInit
**             IXSVS_ErrorClbk
**             IXSVS_StateUnderlayingComClose
**             IXSVS_ActMonIntervalSet
**             IXSVS_InstIdFromIdxGet
**             IXSVS_CombinedDataConsumption
**             IXSVS_TimeCorrMsgReceptionLt
**             IXSVS_StateMachine
**             IXSVS_TcooTimeToSend
**             IXSVS_StateFaultCountInc
**             IXSVS_DataAgeGet
**             IXSVS_InstInitAll
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVSINT_H
#define IXSVSINT_H


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/* possible values for Init_Complete_Out flag */
#define IXSVS_k_INIT                     0U
#define IXSVS_k_COMPL                    1U


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSVS_t_INIT_CALC:
    Values that are calculated during the initialization of the connection and
    constant afterwards.
*/
typedef struct
{
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UDINT    u32_pidCrcS5;   /* CRC-S5 over PID */
  CSS_t_UDINT    u32_cidCrcS5;   /* CRC-S5 over CID */
#endif
  /* PID: Producer Identification (storage is needed for comparison in
     Forward_Close) */
  CSS_t_PIDCID   s_pid;

  CSS_t_UINT     u16_pidCrcS3;   /* CRC-S3 over PID */
  CSS_t_UINT     u16_cidCrcS3;   /* CRC-S3 over CID */

  /* Network_Time_Expectation_Multiplier (see Vol. 5 Chapt. 2-4.6.2.3): is the
     maximum number of 128uSec increments that a consumer should allow the age
     of the safety data to reach.
     Legal range: 0..45313 (0..5.8sec) */
  CSS_t_UINT     u16_Network_Time_Expectation_Multiplier_128us;

  /* Consumer_Num (see 2.4.6.2.1): is the number a Multi-cast Consumer is
     assigned.
     Legal range: 1..15 */
  CSS_t_USINT    u8_Consumer_Num;

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* Max Fault Number
     Legal range: 0..255 */
  CSS_t_USINT    u8_Max_Fault_Number;
#endif

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_USINT    u8_pidCrcS1;    /* CRC-S1 over PID */
#endif

} IXSVS_t_INIT_CALC;


/** IXSVS_t_INIT_INFO:
    Values being written during initialization.
*/
typedef struct
{
  /* struct for storing further initialization data (CRC seeds, etc.) */
  IXSVS_t_INIT_CALC s_initCalc;

  /* Consumed Connection Point
     (Output Assembly Instance or Index of Consumed Data Process Image) */
  CSS_t_UINT u16_consCnxnPoint;

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

} IXSVS_t_INIT_INFO;


/** IXSVS_t_TEMP_CONS_FLAGS:
    Group of temporary flags needed during safety data consumption. At the end
    of processing the Safety Validator Server Consumption the value of these
    flags are copied to the corresponding Safety Validator Server flags.
*/
typedef struct
{
  CSS_t_BYTE   b_Temp_Fault_Flag;
  CSS_t_BYTE   b_Init_Complete_Out_Temp;
  CSS_t_BYTE   b_S_Con_Flt_C_Out_Temp;
  CSS_t_BYTE   b_S_Run_Idle_Out_Temp;
} IXSVS_t_TEMP_CONS_FLAGS;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_InitInfoSet
**
** Description : This function assigns the values of the passed struct to the
**               elements of the local arrays.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in calling functions
**                                  IXSVS_TargServerInit() and
**                                  IXSVS_OrigServerInit())
**               ps_initInfo (IN) - pointer to struct containing the init infos
**                                  (not checked, only called with reference to
**                                  structure)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InitInfoSet(CSS_t_UINT u16_svsIdx,
                       const IXSVS_t_INIT_INFO *ps_initInfo);


/*******************************************************************************
**
** Function    : IXSVS_InitInfoClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structures.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_Init() and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InitInfoClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InitInfoCalcPtrGet
**
** Description : This function returns a const pointer to the values calculated
**               during initialization of the connection (e.g. PID/CID CRC
**               seeds) of the requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN)     - index of Safety Validator Server
**                                     structure
**                                     (not checked, checked by callers)
**
** Returnvalue : IXSVS_t_INIT_CALC*  - const pointer to the structure containing
**                                     the init calc data
**
*******************************************************************************/
const IXSVS_t_INIT_CALC* IXSVS_InitInfoCalcPtrGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ConsumerClkCountGet
**
** Description : This function returns the current value of the global timer.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - counter that is incremented by a periodic timer
**                            every 128 uSec (16 bits, see FRS249).
**
*******************************************************************************/
CSS_t_UINT IXSVS_ConsumerClkCountGet(void);


/*******************************************************************************
**
** Function    : IXSVS_ConsLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_Init(),
**                                 IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsLocalsClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TimeoutMultiplierInit
**
** Description : This function initializes the Timeout Multiplier structure
**               based on the passed timeout multiplier.
**
** Parameters  : u16_svsIdx (IN)     - index of Safety Validator Server
**                                     structure
**                                     (not checked, checked in
**                                     IXSVS_TargServerInit() and
**                                     IXSVS_OrigServerInit())
**               u8_timeoutMult (IN) - timeout multiplier value
**                                     (not checked, checked in
**                                     CnxnParamsValidateSafety1() and
**                                     RangeCheckInstSafetyParams())
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TimeoutMultiplierInit(CSS_t_UINT u16_svsIdx,
                                 CSS_t_USINT u8_timeoutMult);


/*******************************************************************************
**
** Function    : IXSVS_RecvLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_Init() and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_RecvLocalsClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ConsDataSizeSet
**
** Description : This function initializes the Consumed Data Size based on the
**               passed value.
**
** Parameters  : u16_svsIdx (IN)      - index of Safety Validator Server
**                                      structure
**                                      (not checked, checked in
**                                      IXSVS_TargServerInit() and
**                                      IXSVS_OrigServerInit()
**               u8_consDataSize (IN) - value to be set
**                                      (not checked, checked in CnxnSizeCheck()
**                                      and FwdOpenRespCheck())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsDataSizeSet(CSS_t_UINT u16_svsIdx,
                           CSS_t_USINT u8_consDataSize);


/*******************************************************************************
**
** Function    : IXSVS_ConsCnxnStatusGet
**
** Description : This function returns the status of a consuming connection
**               (i.e. Safety Validator Server).
**
** See Also    : CSS_k_CNXN_STATUS_xxx
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in IXSVS_InstInfoGet()
**                                 and IXSVS_RxProc())
**
** Returnvalue : see {CSS_k_CNXN_STATUS_xxx}
**
*******************************************************************************/
CSS_t_USINT IXSVS_ConsCnxnStatusGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateInitTimerStop
**
** Description : This function stops the initialization limit timer
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitTimerStop(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateInitLimitCalc
**
** Description : This function calculates and saves the point of time when the
**               the initialization process of a connection must be completed.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit()))
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitLimitCalc(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateInitLimitCheck
**
** Description : This function checks if the initialization of the connection
**               is in progress. If the initialization is not completed within
**               time limit then the connection is closed. FRS280 states that
**               this is a task of the application but in our implementation
**               this is performed inside CSS.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInitLimitCheck(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InstIdInit
**
** Description : This function initializes the lookup table for the Safety
**               Validator Instance Ids
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdInit(void);


/*******************************************************************************
**
** Function    : IXSVS_InstIdIdxStop
**
** Description : This function stops/deletes the passed Safety Validator Server
**               index and Instance dependency.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_InstanceStop())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdIdxStop(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateInit
**
** Description : This function initializes the state variables of a Safety
**               Validator Server instance
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_Init() and
**                                 IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateInit(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateIsInitOrEstablished
**
** Description : This function checks the state of the addressed Safety
**               Validator Server Instance
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - state of the instance is
**                                 CSS_k_SV_STATE_INITIALIZING or
**                                 CSS_k_SV_STATE_ESTABLISHED
**               CSS_k_FALSE     - instance is in other state
**
*******************************************************************************/
CSS_t_BOOL IXSVS_StateIsInitOrEstablished(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcooLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in IXSVS_Init(),
**                                  SafetyOpenProc3())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooLocalsClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcooColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection from closed to open and initializes Time
**               Coordination specific Variables of the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                (not checked, checked in
**                                IXSVS_TargServerInit(),
**                                IXSVS_OrigServerInit() and IXCCO_CnxnOpen())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooColdStartLogic(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcooPingCountCheck
**
** Description : This function is called by the consumer safety data reception
**               logic to handle ping count checking.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooPingCountCheck(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_MultiCastConsumerFunction
**
** Description : This function is called by the consumer safety data reception
**               logic to handle multi-cast specific jobs.
**
** Parameters  : u16_svsIdx (IN)          - index of the Safety Validator Server
**                                          instance
**                                          (not checked, checked by
**                                          IXSVS_RxProc())
**               b_s_Run_Idle_Out (IN)    - value of the Safety Validator's
**                                          S_Run_Idle_Out flag
**                                          (not checked)
**               b_init_Complete_Out (IN) - value of the Safety Validator's
**                                          Init_Complete_Out flag
**                                          (not checked)
**               ps_tempFlags_in (IN)     - Temporary flags for this Safety Data
**                                          Consumption (input to function)
**                                          (not checked, only called with
**                                          reference to structure)
**               ps_tempFlags_out (OUT)   - Temporary flags for this Safety Data
**                                          Consumption (output of function)
**                                          (not checked, only called with
**                                          reference to structure)
**               pu16_corrected_Data_Time_Stamp_128us (OUT) - calculated by this
**                                          function, to be assigned to the
**                                          u16_Corrected_Data_Time_Stamp_128us
**                                          of the instance.
**                                          (not checked, only called with
**                                          reference to variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_MultiCastConsumerFunction(CSS_t_UINT u16_svsIdx,
                              CSS_t_BYTE b_s_Run_Idle_Out,
                              CSS_t_BYTE b_init_Complete_Out,
                              const IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_in,
                              IXSVS_t_TEMP_CONS_FLAGS *ps_tempFlags_out,
                              CSS_t_UINT *pu16_corrected_Data_Time_Stamp_128us);


/*******************************************************************************
**
** Function    : IXSVS_TcorrLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in IXSVS_Init() and
**                                  SafetyOpenProc3())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrLocalsClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcorrColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection from closed to open and initializes Time
**               Correction specific Variables of the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit(),
**                                 IXSVS_OrigServerInit() and IXCCO_CnxnOpen())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrColdStartLogic(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TcorrPingIntCountIncrement
**
** Description : This function increments the Time Correction Ping Interval
**               Counter.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcorrPingIntCountIncrement(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ConsFaultCounterIncrement
**
** Description : This function increments the Consumer Fault Counter and returns
**               the new value.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by IXSVS_RxProc())
**
** Returnvalue : CSS_t_USINT     - new Consumer Fault Counter value
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
CSS_t_USINT IXSVS_ConsFaultCounterIncrement(CSS_t_UINT u16_svsIdx);
#endif


/*******************************************************************************
**
** Function    : IXSVS_TcorrRxCopy
**
** Description : This function copies the received Time Correction Message
**               Byte-Stream into the Time Correction Message structure of this
**               instance.
**
** Parameters  : u16_svsIdx (IN)  - index of the Safety Validator Server
**                                  instance
**                                  (not checked, checked in IXSVS_RxProc())
**               pb_data (IN)     - pointer to the Time Correction Message in
**                                  the received byte stream.
**                                  (not checked, called with reference to
**                                  checked received HALCS message buffer plus
**                                  offset)
**               b_msgFormat (IN) - message format byte of the addressed
**                                  instance
**                                  (not checked, only one bit evaluated, any
**                                  value allowed)
**
** Returnvalue : CSS_t_UINT       - number of bytes copied
**
*******************************************************************************/
CSS_t_UINT IXSVS_TcorrRxCopy(CSS_t_UINT u16_svsIdx,
                             const CSS_t_BYTE *pb_data,
                             CSS_t_BYTE b_msgFormat);


/*******************************************************************************
**
** Function    : IXSVS_DataLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_DataLocalsClear(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_DataPtrGet
**
** Description : This function returns a pointer to the payload data and their
**               length of the requested Safety Validator instance.
**
** Parameters  : u16_svsIdx (IN)   - Index of the Safety Validator Server
**                                   Instance
**                                   (not checked, checked in IXSVS_RxProc())
**               pu8_dataLen (OUT) - outputs the length of the payload data
**                                   (not checked, only called with reference to
**                                   variable)
**
** Returnvalue : CSS_t_BYTE*       - pointer to the payload data
**
*******************************************************************************/
const CSS_t_BYTE* IXSVS_DataPtrGet(CSS_t_UINT u16_svsIdx,
                                   CSS_t_USINT *pu8_dataLen);


/*******************************************************************************
**
** Function    : IXSVS_DataModeByteGet
**
** Description : This function returns the Mode Byte of a Safety Validator
**               Server. Thus the user of this function does not have to care
**               whether the Validator Instance refers to Base or Extended and
**               Short or Long Format.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_BYTE      - Mode Byte
**
*******************************************************************************/
CSS_t_BYTE IXSVS_DataModeByteGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_DataTimeStampGet
**
** Description : This function returns the Time Stamp of the data message of a
**               Safety Validator Server. Thus the user of this function does
**               not have to care whether the Validator Instance refers to Base
**               or Extended and Short or Long Format.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_UINT      - Time Stamp (128us ticks)
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataTimeStampGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_DataMsgPtrGet
**
** Description : This function returns a pointer to the data message of a
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_DATA_MSG  - pointer to the data message
**
*******************************************************************************/
const CSS_t_DATA_MSG* IXSVS_DataMsgPtrGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_DataRxCopy
**
** Description : This function copies the received Safety Data I/O Message Byte-
**               Stream into the Data Message structure of this instance.
**
** Parameters  : u16_svsIdx (IN)   - index of the Safety Validator Server
**                                   instance
**                                   (not checked, checked in IXSVS_RxProc())
**               pb_data (IN)      - pointer to the Time Correction Message in
**                                   the received byte stream.
**                                   (not checked, called with offset to HALC
**                                   message buffer which is checked in
**                                   IXSVO_CmdProcess())
**               b_msgFormat (IN)  - message format byte of the addressed
**                                   instance
**                                   (checked, allowed values: bit mask
**                                   combinations of {IXSVD_k_MSG_FORMAT_xxx})
**               u8_expPayLen (IN) - number of data bytes that the Safety
**                                   Validator is configured to consume (Safety
**                                   (I/O payload)
**                                   (not checked, checked in CnxnSizeCheck(),
**                                   IXSCE_MsgLenToPayloadLen())
**
** Returnvalue : CSS_t_UINT        - number of bytes copied
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataRxCopy(CSS_t_UINT u16_svsIdx,
                            const CSS_t_BYTE *pb_data,
                            CSS_t_BYTE b_msgFormat,
                            CSS_t_USINT u8_expPayLen);


/*******************************************************************************
**
** Function    : IXSVS_ConFaultFlagGet
**
** Description : This function returns the S_Con_Flt_C_Out flag of this Safety
**               Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE      - S_Con_Flt_C_Out flag of this Safety
**                                 Validator Server
**
*******************************************************************************/
CSS_t_BYTE IXSVS_ConFaultFlagGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_ConFaultFlagSet
**
** Description : This function sets the S_Con_Flt_C_Out flag of this Safety
**               Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by IXSVS_RxProc())
**               b_flag (IN)     - value that is to be assigned to the
**                                 S_Con_Flt_C_Out flag
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConFaultFlagSet(CSS_t_UINT u16_svsIdx, CSS_t_BYTE b_flag);


/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckBaseShort
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Base Format Short
**               safety message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)    \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) \
    )
  CSS_t_BOOL IXSVS_MsgIntegCheckBaseShort(CSS_t_UINT u16_svsIdx);
#endif



/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckBaseLong
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Base Format Long
**               safety message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) \
    )
  CSS_t_BOOL IXSVS_MsgIntegCheckBaseLong(CSS_t_UINT u16_svsIdx);
#endif



/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckExtShort
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Extended Format
**               Short safety message.
**
** Parameters  : u16_svsIdx (IN)     - index of the Safety Validator Server
**                                     instance
**                                     (not checked, checked in IXSVS_RxProc())
**               u32_pidRcCrcS5 (IN) - CRC-S5 over Producer Identifier
**                                     (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
    )
  CSS_t_BOOL IXSVS_MsgIntegCheckExtShort(CSS_t_UINT u16_svsIdx,
                                         CSS_t_UDINT u32_pidRcCrcS5);
#endif


/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckExtLong
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Extended Format
**               Long safety message.
**
** Parameters  : u16_svsIdx (IN)     - index of the Safety Validator Server
**                                     instance
**                                     (not checked, checked in IXSVS_RxProc())
**               u32_pidRcCrcS5 (IN) - CRC-S5 over Producer Identifier
**                                     (not checked, any value allowed)
**               u16_pidRcCrcS3 (IN) - CRC-S3 over Producer Identifier
**                                     (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
  CSS_t_BOOL IXSVS_MsgIntegCheckExtLong(CSS_t_UINT u16_svsIdx,
                                        CSS_t_UDINT u32_pidRcCrcS5,
                                        CSS_t_UINT u16_pidRcCrcS3);
#endif


/*******************************************************************************
**
** Function    : IXSVS_ErrorInit
**
** Description : This function initializes the global variables of the Error
**               module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ErrorInit(void);


/*******************************************************************************
**
** Function    : IXSVS_ErrorClbk
**
** Description : This callback function is used within the IXSVS unit instead of
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
void IXSVS_ErrorClbk(CSS_t_WORD w_errorCode,
                     CSS_t_UINT u16_instId,
                     CSS_t_DWORD dw_addInfo);


/*******************************************************************************
**
** Function    : IXSVS_StateUnderlayingComClose
**
** Description : This function sends a close command to the CSAL.
**
** Parameters  : u16_svsIdx (IN) - Safety Validator Server index
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_SERVERS-1)
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range:
**                                 1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: HALC not ready
**
*******************************************************************************/
CSS_t_WORD IXSVS_StateUnderlayingComClose(CSS_t_UINT u16_svsIdx,
                                          CSS_t_USINT u8_consNum);


/*******************************************************************************
**
** Function    : IXSVS_ActMonIntervalSet
**
** Description : This function initializes the base part of the interval which
**               will be checked by the Consumer Activity Monitor. Later on
**               the Data_Age will be considered in the calculation of the
**               Activity Monitor expiration times. Additionally this function
**               schedules the timeout for the first consumed message.
**
** Parameters  : u16_svsIdx (IN)               - index of Safety Validator
**                                               Server structure
**                                               (not checked, checked in
**                                               IXSVS_TargServerInit() and
**                                               IXSVS_OrigServerInit())
**               u16_NetTimeExpMult_128us (IN) - Network Time Expectation (in
**                                               128us ticks).
**                                               (not checked, checked in
**                                               CnxnParamsValidateSafety1() and
**                                               RangeCheckInstSafetyParams())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ActMonIntervalSet(CSS_t_UINT u16_svsIdx,
                             CSS_t_UINT u16_NetTimeExpMult_128us);


/*******************************************************************************
**
** Function    : IXSVS_InstIdFromIdxGet
**
** Description : This function converts a Safety Validator Server array index
**               into the corresponding Instance ID.
**
** Parameters  : u16_svsIdx (IN) - array index of the Safety Validator Server
**                                 array
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - instance ID of the Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_InstIdFromIdxGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_CombinedDataConsumption
**
** Description : This function implements the consumer safety data reception
**               logic for the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE      - Extended Format Safety Validator Server has
**                                 discarded current message due to errors, but
**                                 because Max_Fault_Number is not yet reached
**                                 the connection is still running.
**               CSS_k_FALSE     - else
**
*******************************************************************************/
CSS_t_BOOL IXSVS_CombinedDataConsumption(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_TimeCorrMsgReceptionLt
**
** Description : This function contains the logic associated with the Time
**               Correction Message reception for the multi-cast safety
**               connection type, for the Consumer - Link Triggered.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TimeCorrMsgReceptionLt(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateMachine
**
** Description : This function implements the Safety Validator Object State
**               machine as specified by the CIP Safety Specification.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**               e_svEvent (IN)  - specifies the event that occurred
**                                 (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateMachine(CSS_t_UINT u16_svsIdx,
                        IXSVS_t_VALIDATOR_EVENT e_svEvent);


/*******************************************************************************
**
** Function    : IXSVS_TcooTimeToSend
**
** Description : This function checks if it is time to send a Time Coordination
**               message by checking the corresponding flag of the Safety
**               Validator Server. It also clears the flag.
**               This function also checks if the timeout for sending a Time
**               Coordination message after a change of the Ping_Count expired.
**               This is to ensure that the Time Coordination message is sent no
**               later than 5 seconds after Ping_Count change. In case of
**               timeout it sends a time coordination message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooTimeToSend(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_StateFaultCountInc
**
** Description : This function increments the value of the Fault Counter of the
**               Safety Validator Server instances.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_StateFaultCountInc(void);


/*******************************************************************************
**
** Function    : IXSVS_DataAgeGet
**
** Description : This function returns the Data_Age attribute of the requested
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_UINT      - Data_Age attribute of the requested
**                                 Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataAgeGet(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**
** Function    : IXSVS_InstInitAll
**
** Description : This function initializes all static variables of this module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstInitAll(void);


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#endif /* #ifndef IXSVSINT_H */

/*** End Of File ***/


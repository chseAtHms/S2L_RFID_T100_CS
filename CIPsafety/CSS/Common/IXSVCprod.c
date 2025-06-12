/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCprod.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the functions to produce a Safety I/O
**             message. These routines are derived from the Example Code
**             provided in CIP Networks Library Volume 5.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_ProdLocalsClear
**             IXSVC_ProdColdStartLogic
**             IXSVC_ProdMcastReinit
**             IXSVC_ProdModeByteGet
**             IXSVC_ProdPingIntEpiCntGet
**             IXSVC_ProdTcooMsgReception
**             IXSVC_ProdNextTxTimeSet
**             IXSVC_ProdNextTxTimeGet
**             IXSVC_TxFrameGenerate
**             IXSVC_ProdSoftErrByteGet
**             IXSVC_ProdSoftErrVarGet
**
**             CombinedDataProduction
**             SingleCastProducerFunction
**             MultiCastProducerFunction
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXUTL.h"
#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  #include "IXSPI.h"
#endif
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  #include "IXSAIapi.h"
  #include "IXSAI.h"
#endif
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#include "IXSVCerr.h"



/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** k_DATA_MSG_STREAM_LENGTH:
**  max number of bytes of a Data Message (on the network, depending on formats
**  supported)
*/
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  #define k_DATA_MSG_STREAM_LENGTH      CSOS_k_IO_MSGLEN_LONG_MAX
#else
  #define k_DATA_MSG_STREAM_LENGTH      CSOS_k_IO_MSGLEN_SHORT_MAX
#endif

/* minimum and maximum valid values for Time_Drift_Per_Ping_Interval */
#define k_TIME_DRIFT_PER_PING_INT_MIN_128US   1U
#define k_TIME_DRIFT_PER_PING_INT_MAX_128US   313U


/** IXSVC_t_VC_PER_CONSUMER_DATA:
    This structure stores data that a Safety Validator Client Instance needs
    for each of its Consumers.
    Structure fields sorted by description order in the CIP Safety
    specification.
*/
typedef struct
{

  /* Ping_Int_Since_Last_Time_Coord_Msg_Count (see Vol. 5 Chapt. 2-4.5.5.4): is
     a run time count of the number of Ping Intervals between Time Coordination
     Messages for a particular consumer.
     Legal range: 0..5 */
  CSS_t_USINT     u8_Ping_Int_Since_Last_Time_Coord_Msg_Count;

} IXSVC_t_VC_PER_CONSUMER_DATA;


/** t_SVC_LOCALS_PROD:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* array of structures for storing the data that the validator client needs
     per consumer */
  IXSVC_t_VC_PER_CONSUMER_DATA as_perCons[CSOS_k_MAX_CONSUMER_NUM_MCAST];

  /*
  ** seed values for CRC calculations
  */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UINT      u16_pidRcCrcS3; /* CRC-S3 over PID and Rollover Count */
    CSS_t_UDINT     u32_pidRcCrcS5; /* CRC-S5 over PID and Rollover Count */
  #endif

  /* Producer_Safe_Data_TS (see Vol. 5 Chapt. 2-4.5.4.2): is a variable that
     shall be set equal to Producer_clk_Count at the point in time that the
     Safety_Data for a particular EPI data production is sampled and captured
     from the producing application.
     Legal range: 0..65535 */
  CSS_t_UINT      u16_Producer_Safe_Data_TS_128us;

  /* Ping_Interval_EPI_Count (see Vol. 5 Chapt. 2-4.5.4.4): is a count of the
     number of EPI time periods between the incrementing of the Ping_Count.
     Legal range: 0..Ping_Interval_EPI_Multiplier */
  CSS_t_UINT      u16_Ping_Interval_EPI_Count;

  /* The following variables are used for extended format only. These
     variables appear in the Pseudo Code presented in the CIP Safety
     specification, but in contrast to the other variables which are used there
     no further description for these variables exists. */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)

    /* Last Time Stamp for Rollover */
    CSS_t_UINT      u16_Last_Time_Stamp_For_Rollover_128us;

    /* Time Stamp Rollover Count */
    CSS_t_UINT      u16_TS_Rollover_Count;
  #endif

  /* scheduled system time at which the next transmission has to occur */
  CSS_t_UDINT       u32_nextTxTime_128us;

  /* cumulative number of micro seconds until an additonal tick must be
     inserted */
  CSS_t_UDINT       u32_txTickRestCount_us;

} t_SVC_LOCALS_PROD;


static t_SVC_LOCALS_PROD as_SvcLocals[CSOS_cfg_NUM_OF_SV_CLIENTS];

/* structure for storing the data of the associated data message */
/* Not relevant for Soft Error Checking - part of the safety function but value
   is updated before each use */
static CSS_t_DATA_MSG as_DataMsg[CSOS_cfg_NUM_OF_SV_CLIENTS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void CombinedDataProduction(CSS_t_UINT u16_svcIdx,
                                   CSS_t_BYTE b_application_Run_Idle);
static void SingleCastProducerFunction(CSS_t_UINT u16_svcIdx);
static void MultiCastProducerFunction(CSS_t_UINT u16_svcIdx);


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_ProdLocalsClear(CSS_t_UINT u16_svcIdx)
{
  CSS_MEMSET(&as_SvcLocals[u16_svcIdx], 0, sizeof(as_SvcLocals[0]));

  CSS_MEMSET(&as_DataMsg[u16_svcIdx], 0, sizeof(as_DataMsg[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                             )
{
  /*
  //////////////////////////////////////////////////////////////////
  // Cold start after connection establishment processing
  //////////////////////////////////////////////////////////////////
  // This Logic should be executed at the transition of the
  // producing connection from closed to open.
  // For Single-Cast connections this logic may
  // be performed any time the connection is Opened or Re-Opened.
  //////////////////////////////////////////////////////////////////
  */
  /* index to consumer of the safety validator client */
  CSS_t_USINT u8_consIdx = 0U;

  /* Ping_Interval_EPI_Count = 0,*/
  as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count = 0U;
  /* RR_Con_Num_Index_Pntr = Max_Consumer_Number, */
  IXSVC_RrConNumIdxPntrSet(u16_svcIdx, ps_initInfo->u8_Max_Consumer_Number,
                           ps_initInfo->u8_Max_Consumer_Number);

  /*
  // Initialize the ping count in the safety message to 0
  Mode_Byte.Ping_Count = 0, */
  /* As the consumer initializes its u8_Last_Ping_Count variable to 3 this    */
  /* immediately request an initial ping response (see FRS70).                */
  IXUTL_BIT_CLR_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_PING_COUNT);

  /*
  // Time_Drift_Per_Ping_Interval, the minimum value is 1
  Time_Drift_Per_Ping_Interval =
    Roundup(EPI * Ping_Interval_EPI_Multiplier / 320000),
  */
  /* Time_Drift_Per_Ping_Interval, the minimum value is 1 */
  /* performed in IXSVC_TcooColdStartLogic() */

  /* FOR (Consumer_Num = 1 to Max_Consumer_Number), */
  for (u8_consIdx = 0U;
       u8_consIdx < ps_initInfo->u8_Max_Consumer_Number;
       u8_consIdx++)
  {
    IXSVC_t_VC_PER_CONSUMER_DATA *const ps_svcPerCons =
                               &as_SvcLocals[u16_svcIdx].as_perCons[u8_consIdx];

    /*
    // Producer Dynamic Variables
    Consumer_Active_Idle[Consumer_Num-1] = Idle,
    S_Connection_Fault[Consumer_Num-1] = OK,
    Producer_Rcved_Time_Value[Consumer_Num-1] = 0x0000,
    Consumer_Time_Correction_Value[Consumer_Num-1] = 0x0000,
    Ping_Int_Since_Last_Time_Coord_Msg_Count[Consumer_Num-1] = 0x0000,
    Consumer_Time_Value[Consumer_Num-1] = 0x0000,
    Producer_Fault_Counter[Consumer_Num-1] = 0, //For ExtendedFormat only
    */
    /* (see FRS104, FRS210 and FRS221) */
    IXSVC_ConsumerActiveIdleSet(u16_svcIdx, u8_consIdx, IXSVD_k_IDLE);
    /* (see FRS225) */
    IXSVC_ConnectionFaultFlagSet(u16_svcIdx, u8_consIdx, IXSVD_k_CNXN_OK);
    /* (see FRS268) */
    IXSVC_ConsTimeCorrValSet(u16_svcIdx, u8_consIdx, 0U);
    /* (see FRS273) */
    ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count = 0U;

    /* further variable initialization of the sample code has been put into the
       function IXSVC_TcooColdStartLogic() */
  }
  /* ENDFOR */

  /*
  IF (ExtendedFormat),
  THEN
  */
  if (IXSVD_IsExtendedFormat(ps_initInfo->b_msgFormat))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* RC_Used_in_CRC = 0x0000 */
    /* optimization: As this variable is initialized every time before use we
       made it a local variable in the respective functions. Thus global
       initialization is not necessary. */

    /*
    IF (Multi-Cast),
    THEN
    */
    if (IXSVD_IsMultiCast(ps_initInfo->b_msgFormat))
    {
      /* Multi-Cast Client -> we are Target */
      /*
      // The rollover value is initialized to the local Time Stamp clock value
      // plus an offset
      // This is considered as a random number for this purpose
      Initial_TS for SafetyOpenResponse = Producer_Clk_Count
      */
      *pu16_InitialTS = IXSVC_ProducerClkCountGet();
      /*
      Initial_Rollover_Value for SafetyOpenResponse =
        Producer_Clk_Count + Initial_RC_Offset
      */
      *pu16_InitialRV = IXSVC_ProducerClkCountGet();
      *pu16_InitialRV = (CSS_t_UINT)(*pu16_InitialRV
                                    + IXSVD_InitRcOffsetIncrementGet());

      /* Last_Time_Stamp_For_Rollover = Initial_TS */
      as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us =
        *pu16_InitialTS;
      /* TS_Rollover_Count = Initial_Rollover_Value */
      as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count = *pu16_InitialRV;
    }
    else
    {
      /* not relevant for single-cast */
    }
    /*
    ENDIF
    IF (Single-Cast AND Originator),
    THEN
    */
    if (    (IXSVD_IsSingleCast(ps_initInfo->b_msgFormat))
         && (IXSVD_IsOriginator(ps_initInfo->b_msgFormat))
       )
    {
      /* Last_Time_Stamp_For_Rollover = Initial_TS from SafetyOpenResponse */
      as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us =
        *pu16_InitialTS;
      /* TS_Rollover_Count = Initial_Rollover_Value from SafetyOpenResponse */
      as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count = *pu16_InitialRV;

    }
    else
    {
      /* not relevant for multi-cast or targets */
    }
    /*
    ENDIF
    IF (Single-Cast AND Target),
    THEN
    */
    if (    (IXSVD_IsSingleCast(ps_initInfo->b_msgFormat))
         && (IXSVD_IsTarget(ps_initInfo->b_msgFormat))
       )
    {
      /* Last_Time_Stamp_For_Rollover = Initial_TS from SafetyOpen */
      as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us =
        *pu16_InitialTS;
      /* TS_Rollover_Count = Initial_Rollover_Value from SafetyOpen */
      as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count = *pu16_InitialRV;

    }
    else
    {
      /* not relevant for multi-cast or originators */
    }
    /* ENDIF */
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_21,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else
  {
    /* no else : not relevant for base format */
  }
  /* ENDIF */

  /* connection is now initializing */
  IXSVC_StateMachine(u16_svcIdx, CSOS_k_CNUM_INVALID, IXSVC_k_VE_CLIENT_OPEN);

  /*
  //////////////////////////////////////////////////////////////////
  // end, Cold start after connection establishment processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
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
void IXSVC_ProdMcastReinit(CSS_t_UINT u16_svcIdx,
                           CSS_t_USINT u8_consIdx
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                            ,CSS_t_UINT *pu16_InitialTS
                            ,CSS_t_UINT *pu16_InitialRV
#endif
                           )
{
  /*
  //////////////////////////////////////////////////////////////////
  // Re-initialization of Multi-Cast prodution to individual consumers
  //////////////////////////////////////////////////////////////////
  // This Logic should be executed after a multicast consumer performs
  // a successful open to a multi-cast producer while the production
  // is in process to re-initialize production to that consumer number
  //////////////////////////////////////////////////////////////////
  */
  /* pointer to per Consumer data */
  IXSVC_t_VC_PER_CONSUMER_DATA *const ps_svcPerCons =
    &as_SvcLocals[u16_svcIdx].as_perCons[u8_consIdx];

  /*
  ** It does not make sense to search the consumer that is currently being
  ** opened using a for loop (as it is implemented in the Vol. 5 sample code.
  ** Instead the index of the consumer is passed as a function parameter.
  */

  /* FOR (Consumer_Num = 1 to Max_Consumer_Number),
    IF (Consumer_Open(Consumer_Num-1)transitions from Closed to Open)
    THEN
    */
  /*
  // Producer Dynamic Variables
  Consumer_Active_Idle[Consumer_Num-1] = Idle,
  S_Connection_Fault[Consumer_Num-1] = OK,
  Producer_Rcved_Time_Value[Consumer_Num-1] = 0x0000,
  Consumer_Time_Correction_Value[Consumer_Num-1] = 0x0000,
  Ping_Int_Since_Last_Time_Coord_Msg_Count[Consumer_Num-1 ] = 0x0000,
  Producer_Fault_Counter[Consumer_Num-1] = 0, //For ExtendedFormat only
  */
  /* (see FRS104 and FRS210) */
  IXSVC_ConsumerActiveIdleSet(u16_svcIdx, u8_consIdx, IXSVD_k_IDLE);
  /* (see FRS225) */
  IXSVC_ConnectionFaultFlagSet(u16_svcIdx, u8_consIdx, IXSVD_k_CNXN_OK);
  /* (see FRS268) */
  IXSVC_ConsTimeCorrValSet(u16_svcIdx, u8_consIdx, 0U);
  /* (see FRS273) */
  ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count = 0U;

  /* It's ok not to initialize Consumer_Time_Value here as it is only */
  /* used after Consumer_Active_Idle is active                        */

  /* further variable initialization of the sample code has been put into the
     function IXSVC_TcooColdStartLogic() */
    /* ENDIF */
  /* ENDFOR */

  /*
  IF (ExtendedFormat),
  THEN
  */
  if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* Initial_TS for SafetyOpenResponse = Last_Time_Stamp_For_Rollover */
      *pu16_InitialTS =
        as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us;
      /* Initial_Rollover_Value for SafetyOpenResponse = TS_Rollover_Count */
      *pu16_InitialRV = as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count;
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_22,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else
  {
    /* not relevant for base format */
  }
  /* ENDIF */


  /* another consumer is joining */
  IXSVC_StateMachine(u16_svcIdx, (CSS_t_USINT)(u8_consIdx + 1U),
                     IXSVC_k_VE_CONSUMER_JOIN);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


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
CSS_t_BYTE IXSVC_ProdModeByteGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_DataMsg[u16_svcIdx].b_modeByte);
}


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
CSS_t_UINT IXSVC_ProdPingIntEpiCntGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count);
}


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
                CSS_t_UINT u16_Worst_Case_Consumer_Time_Correction_Value_128us)
{
  /* pointer to per consumer data of the processed consumer */
  IXSVC_t_VC_PER_CONSUMER_DATA *const ps_svcPerCons =
                          &as_SvcLocals[u16_svcIdx].as_perCons[u8_consNum - 1U];
  /* CIP spec describes this variable as "per Consumer", but it is used in    */
  /* this function only to calculate the Consumer_Time_Value (which is stored */
  /* for each consumer). Thus it can just be a local variable (see FRS262).   */
  CSS_t_UINT u16_Time_Drift_Since_Last_Time_Coord_128us;
  /* temporary variable for making mathematical expressions less complex */
  CSS_t_UINT u16_temp = 0U;
  /* local copy of the Consumer_Time_Correction_Value variable */
  CSS_t_UINT u16_Consumer_Time_Correction_Value_128us =
    IXSVC_ConsTimeCorrValGet(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U));

  /* if passed parameter is out of range */
  if (    (u16_Time_Drift_Per_Ping_Interval_128us
           < k_TIME_DRIFT_PER_PING_INT_MIN_128US)
       || (u16_Time_Drift_Per_Ping_Interval_128us
           > k_TIME_DRIFT_PER_PING_INT_MAX_128US)
     )
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_TDPPI_RANGE,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u16_Time_Drift_Per_Ping_Interval_128us);
  }
  else
  {
    /*
    // Determine the worst case time drift since the last
    // Time Coordination information was received. This is equal to the
    // number of Ping Intervals since the last Time Coordination message
    // times the Time Drift per Ping Interval plus 1 for the asyncronous
    // clocks. This value will not be used until the second
    // Time_Coordination message is recived.
    Time_Drift_Since_Last_Time_Coord =
      ({Ping_Int_Since_Last_Time_Coord_Msg_Count[Consumer_Num-1]+ 1}
      * Time_Drift_Per_Ping_Interval) + 1,
    */
    /* result of multiplication will always fit into an UINT since maximum */
    /* multiplication is max 6 * 313 */
    /* (see FRS260) */
    u16_temp = (CSS_t_UINT)
      (((CSS_t_UINT)ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count
       + 1U));
    u16_temp = (CSS_t_UINT)(u16_temp * u16_Time_Drift_Per_Ping_Interval_128us);
    u16_Time_Drift_Since_Last_Time_Coord_128us = (CSS_t_UINT)(u16_temp+1U);

    /*
    // If this Time_Coordination Transport delay is greater then the
    // last Time_Coordination Transport delay by more than the appropriate
    // number of Time_Drift_Per_Ping_Intervals, then, use the previous
    // Correction value minus the appropriate number of
    // Time_Drift_Per_Ping_Intervals. This action is not taken until the
    // second Time_Coordination message is received.
    // The math for this check should be 16 bit or greater
    IF ((Consumer_Active_Idle[Consumer_Num-1] == Active) AND
        ((Consumer_Time_Correction_Value[Consumer_Num-1]-
        Worst_Case_Consumer_Time_Correction_Value[Consumer_Num-1] -
        Time_Drift_Since_Last_Time_Coord) AND 0x8000 == 0)),
    THEN
    */
    /* (see FRS260 and FRS261) */
    if (
            (IXSVC_ConsumerIsActive(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U)))
         && (((CSS_t_UINT)((u16_Consumer_Time_Correction_Value_128us
              - u16_Worst_Case_Consumer_Time_Correction_Value_128us)
              - u16_Time_Drift_Since_Last_Time_Coord_128us) & 0x8000U) == 0U)
       )
    {
      /*
      Consumer_Time_Correction_Value[Consumer_Num-1] =
        Consumer_Time_Correction_Value[Consumer_Num-1] -
        Time_Drift_Since_Last_Time_Coord,
      */
      /* u16_Consumer_Time_Correction_Value is the offset of the Time Stamp that
         will be applied to future production (see FRS72) */
      u16_Consumer_Time_Correction_Value_128us
        = (CSS_t_UINT)(u16_Consumer_Time_Correction_Value_128us
        - u16_Time_Drift_Since_Last_Time_Coord_128us);
    }
    /* ELSE */
    else
    {
      /*
      Consumer_Time_Correction_Value[Consumer_Num-1] =
        Worst_Case_Consumer_Time_Correction_Value[Consumer_Num-1],
      */
      /* (see FRS72) */
      u16_Consumer_Time_Correction_Value_128us =
        u16_Worst_Case_Consumer_Time_Correction_Value_128us;  /* see (FRS26) */
    }
    /* ENDIF */

    /* Store the just calculated Consumer Time Correction Value (see FRS18) */
    IXSVC_ConsTimeCorrValSet(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U),
                             u16_Consumer_Time_Correction_Value_128us);


    /*
    // Set the flag indicating a Time_Coordination message has been received
    IF (S_Connection_Fault[Consumer_Num-1] = OK)
    THEN
    */
    /* (see FRS219) */
    if (!IXSVC_ConnectionIsFaulted(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U)))
    {
      /* If this is the first Time Coordination Message from any of the */
      /* consumers */
      if (IXSVC_StateGet(u16_svcIdx) == CSS_k_SV_STATE_INITIALIZING)
      {
        /* then the connection transitions from Initializing to Established   */
        IXSVC_StateMachine(u16_svcIdx, u8_consNum, IXSVC_k_VE_1ST_HS_COMPLETE);
      }
      else /* else: continuous Tcoo message */
      {
        /* nothing to do */
      }

      /* if consumer is not yet in the active state */
      if (!IXSVC_ConsumerIsActive(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U)))
      {
        /* then set to active */
        IXSVC_StateMachine(u16_svcIdx, u8_consNum, IXSVC_k_VE_CONSUMER_ACTIVE);
      }
      else /* else: already active */
      {
        /* nothing to do */
      }

      /* Consumer_Active_Idle[Consumer_Num-1] = Active, */
      /* (see FRS106 and FRS181) */
      IXSVC_ConsumerActiveIdleSet(u16_svcIdx, (CSS_t_USINT)(u8_consNum-1U),
                                  IXSVD_k_ACTIVE);
    }
    else
    {
      /* connection already faulted */
    }
    /* ENDIF */

    /*
    // Reset the Time_Coordination message timer
    Ping_Int_Since_Last_Time_Coord_Msg_Count[Consumer_Num-1] = 0,
    */
    /* (see FRS271) */
    ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count = 0U;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                             CSS_t_UDINT u32_txTime_128us)
{
  as_SvcLocals[u16_svcIdx].u32_nextTxTime_128us = u32_txTime_128us;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_UDINT IXSVC_ProdNextTxTimeGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvcLocals[u16_svcIdx].u32_nextTxTime_128us);
}


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
                                 CSS_t_UINT u16_svcIdx)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* value of Run/Idle flag for feeding into the Safety Validator */
  CSS_t_BYTE b_riValue = CSS_k_RIF_IDLE;
  /* buffer for storing the output stream */
  CSS_t_BYTE ab_clientTxMsg[k_DATA_MSG_STREAM_LENGTH +
                            CSOS_k_IO_MSGLEN_TCORR];
  /* actual length of the Tx byte stream */
  CSS_t_UINT u16_clientTxMsgLen = 0U;
  /* structure for passing the Tx Frame to HALCS */
  HALCS_t_MSG s_halcTxMsg;
  /* pointer to the initially calculated values of this instance (EPI and
     Produced Data Size are needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);
  /* temporary variable for calculating the next transmit time */
  CSS_t_UDINT u32_txTime_128us =
   IXSVC_ProdNextTxTimeGet(u16_svcIdx) + kps_initCalc->u32_EPI_128us;

  /* due to tick granularity: add the number of missing micro seconds */
  as_SvcLocals[u16_svcIdx].u32_txTickRestCount_us +=
    kps_initCalc->u32_EpiDeviation_us;

  /* if rest micro seconds summed up to one tick or more */
  if (as_SvcLocals[u16_svcIdx].u32_txTickRestCount_us >= CSS_k_TICK_BASE_US)
  {
    /* insert one tick and subtract this tick from the rest micro seconds */
    u32_txTime_128us++;
    as_SvcLocals[u16_svcIdx].u32_txTickRestCount_us -= CSS_k_TICK_BASE_US;
  }
  else  /* else: rest micro seconds less than a tick */
  {
    /* nothing to do */
  }

  /* if calculated next Tx Time is in the past */
  if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(), u32_txTime_128us))
  {
    /* we are lagging behind for more than one RPI */
    /* then we must catch up (i.e. omit a frame) */
    u32_txTime_128us = IXUTL_GetSysTime() + kps_initCalc->u32_EPI_128us;
  }
  else  /* else: calculated next Tx time is in the future */
  {
    /* nothing to do here */
  }

  /* already schedule next transmission */
  IXSVC_ProdNextTxTimeSet(u16_svcIdx, u32_txTime_128us);

  /* completely erase structure */
  CSS_MEMSET(&s_halcTxMsg, 0, sizeof(s_halcTxMsg));

#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)

  /* if instance ID indicates that this is an originating connection */
  if (u16_svcInstId <= CSS_cfg_NUM_OF_CCO_INSTS)
  {
    /* Read the I/O data (and Run/Idle Flag) from the Produced Data
       Process Image (= Safety_Data_Out from the producing application
       (see FRS124)) */
    w_retVal =
      IXSPI_ProdImageDataGet(u16_svcIdx,
                             IXSVC_InitInfoCnxnPointGet(u16_svcIdx),
                             as_DataMsg[u16_svcIdx].ab_aData,
                             (CSS_t_UINT)kps_initCalc->u8_prodDataSize,
                             &b_riValue);
  }
  else /* else: must be target connection */
#endif
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  {
    /* Read the I/O data (and Run/Idle Flag) from the input assembly */
    /* (= Safety_Data_Out from the producing application
       (see FRS124)) */
    w_retVal =
      IXSAI_AsmInputDataGet(IXSVC_InitInfoCnxnPointGet(u16_svcIdx),
                            as_DataMsg[u16_svcIdx].ab_aData,
                            (CSS_t_UINT)kps_initCalc->u8_prodDataSize,
                            &b_riValue);
  }
#else
  {
    w_retVal = (IXSVC_k_FSE_INC_TXFG_INST_INV);
    IXSVC_ErrorClbk(w_retVal, u16_svcInstId, IXSER_k_A_NOT_USED);
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
#endif

  /* if previously called function returned an error */
  if (w_retVal != CSS_k_OK)
  {
    /* error while input data from the assembly object */
  }
  else /* else: ok */
  {
    /* set the data length of the message */
    as_DataMsg[u16_svcIdx].u8_len = kps_initCalc->u8_prodDataSize;

    /* if device state is not executing */
    if (IXSSO_IxsvcDeviceStatusGetClbk() != CSOS_k_SSO_DS_EXECUTING)
    {
      /* Make sure that if device is not in executing state the application   */
      /* data is marked as invalid (IDLE). But the packet contains the data   */
      /* obtained from the Process Image/Assembly Instance.                   */
      b_riValue = CSS_k_RIF_IDLE;
    }
    else /* else: device state is executing */
    {
      /* b_riValue remains at the value which was provided via                */
      /* Assembly/process Image     */
    }

    /* if the state of the Safety Validator Client is not Init or Established */
    if (!IXSVC_StateIsInitOrEstablished(u16_svcIdx))
    {
      /* Safety Validator is no longer in ESTABLISHED or INITIALIZING state - */
      /* don't create and send a message */
    }
    else /* else: state is Init or Established */
    {
      /* call Safety Validator Client function to produce a message           */
      /* (= building Mode_Byte, Actual_Data, Complement_Data and Time_Stamp   */
      /* (see FRS124)                                                         */
      CombinedDataProduction(u16_svcIdx, b_riValue);

      /* convert produced message to byte stream */
      w_retVal = IXSVC_TxCopy(u16_svcIdx, &as_DataMsg[u16_svcIdx],
                              &u16_clientTxMsgLen, ab_clientTxMsg);

      /* if previously called function returned an error */
      if (w_retVal != CSS_k_OK)
      {
        /* error while converting the produced message to byte stream */
      }
      else /* else: ok */
      {
        /* if Multicast */
        if (IXSVD_IsMultiCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
        {
          /* for Multicast messages we must also copy the Time Correction Msg */
          /* (concatenated to the safety data message (see FRS30 and FRS96)) */

          /* already calculate the length of the whole resulting message */
          CSS_t_UINT u16_frameLen = (CSS_t_UINT)(u16_clientTxMsgLen
                                  + CSOS_k_IO_MSGLEN_TCORR);

          /* if already written too much data into the buffer */
          if (u16_clientTxMsgLen > k_DATA_MSG_STREAM_LENGTH)
          {
            w_retVal = (IXSVC_k_FSE_INC_TX_GEN_TOO_MUCH);
            IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                            IXSER_k_A_NOT_USED);
          }
          else /* else: length is ok */
          {
            /* If Base Format */
            if (IXSVD_IsBaseFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
            {
            #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
              {
                /* produce the Time Correction Message */
                u16_clientTxMsgLen = IXSVC_TcorrBaseTxCopy(u16_svcIdx,
                                                           u16_clientTxMsgLen,
                                                           u16_frameLen,
                                                           ab_clientTxMsg);
              }
            #else
              {
                w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_29);
                IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                                IXSER_k_A_NOT_USED);
              }
            #endif
            }
            else /* else: must be Extended Format */
            {
            #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
              {
                /* produce the Time Correction Message */
                u16_clientTxMsgLen = IXSVC_TcorrExtTxCopy(u16_svcIdx,
                                                          u16_clientTxMsgLen,
                                                          u16_frameLen,
                                                          ab_clientTxMsg);
              }
            #else
              {
                w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_30);
                IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                                IXSER_k_A_NOT_USED);
              }
            #endif
            }
          }
        }
        else /* else: Singlecast */
        {
          /* has no Time Correction Msg */
        }

        /* if the connection is already closed/faulted or an error was detected
           previously */
        if (    (!IXSVC_StateIsInitOrEstablished(u16_svcIdx))
             || (w_retVal != CSS_k_OK)
           )
        {
          /* error while converting the produced message to byte stream */
        }
        else /* else: ok */
        {
          s_halcTxMsg.u16_cmd = CSOS_k_CMD_IXSVO_IO_DATA;
          s_halcTxMsg.u16_len = u16_clientTxMsgLen;
          s_halcTxMsg.u32_addInfo = (CSS_t_UDINT)u16_svcInstId;
          s_halcTxMsg.pb_data = ab_clientTxMsg;

          /* Send message considering mixing data ob both Safety Controllers */
          /* if sending was successful */
          if (HALCS_TxDataPutMix(&s_halcTxMsg))
          {
            /* success */
          }
          else /* else: error */
          {
            /* error while trying to send the message to CSAL */
            w_retVal = (IXSVC_k_NFSE_TXH_HALCS_SEND_TFG);
            IXSVC_ErrorClbk(w_retVal, u16_svcInstId, IXSER_k_A_NOT_USED);
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVC_ProdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_SvcLocals))
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvcLocals)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_SvcLocals);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVC_ProdSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    /* level 0 counter at/above end */
    /* default return values already set */
  }
  else /* level 0 counter in range of array elements */
  {
    /* indicate to caller that we are in range of level 1 or below */
    ps_retStat->u8_incLvl = 1U;

    /* select the level 1 variable */
    switch (au32_cnt[1U])
    {
      case 0U:
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* if level 2 counter is larger than number of array elements */
        if (au32_cnt[2U] >= CSOS_k_MAX_CONSUMER_NUM_MCAST)
        {
          /* level 2 counter at/above end */
          /* default return values already set */
        }
        else
        {
          CSS_H2N_CPY8(pb_var, &as_SvcLocals[au32_cnt[0U]].as_perCons[au32_cnt[2U]].u8_Ping_Int_Since_Last_Time_Coord_Msg_Count);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        }
        break;
      }

      case 1U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].u16_Producer_Safe_Data_TS_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].u16_Ping_Interval_EPI_Count);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 3U:
      {
        CSS_H2N_CPY32(pb_var, &as_SvcLocals[au32_cnt[0U]].u32_nextTxTime_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

      case 4U:
      {
        CSS_H2N_CPY32(pb_var, &as_SvcLocals[au32_cnt[0U]].u32_txTickRestCount_us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      case 5U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].u16_pidRcCrcS3);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 6U:
      {
        CSS_H2N_CPY32(pb_var, &as_SvcLocals[au32_cnt[0U]].u32_pidRcCrcS5);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

      case 7U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].u16_Last_Time_Stamp_For_Rollover_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 8U:
      {
        CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].u16_TS_Rollover_Count);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }
    #endif

      default:
      {
        /* level 1 counter at/above end */
        /* default return values already set */
        break;
      }
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : CombinedDataProduction
**
** Description : This function produces a data message for the addressed Safety
**               Validator Client Instance.
**               This function assumes that the application has passed the
**               Safety Data and Application_Run_Idle to the Safety Validator.
**
** Parameters  : u16_svcIdx (IN)             - Instance index of the Safety
**                                             Validator Client instance
**                                             (not checked, checked in
**                                             IXSVC_InstTxFrameGenerate())
**               b_application_Run_Idle (IN) - value of the Run/Idle flag to be
**                                             transmitted in the Safety message
**                                             CSS_k_RIF_IDLE = Idle, else = run
**                                             (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
static void CombinedDataProduction(CSS_t_UINT u16_svcIdx,
                                   CSS_t_BYTE b_application_Run_Idle)
{
  /*
  //////////////////////////////////////////////////////////////////
  // start of EPI Safety Data production processing
  //////////////////////////////////////////////////////////////////
  // This logic assumes that the application has passed the Safety
  // Data and Application_Run_Idle to the Safety Validator.
  //
  //////////////////////////////////////////////////////////////////
  */
  /* pointer to the initially calculated values of this instance
     (Ping_Interval_EPI_Multiplier is needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);

  /*
  // Capture the time to be used for the time stamp
  Producer_Safe_Data_TS = Producer_Clk_Count,
  */
  /* (see FRS250) */
  as_SvcLocals[u16_svcIdx].u16_Producer_Safe_Data_TS_128us =
    IXSVC_ProducerClkCountGet();

  /*
  // Check if it is time to increment the Ping Count
  Ping_Interval_EPI_Count = Ping_Interval_EPI_Count + 1,
  */
  as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count++;

  /*
  IF (Ping_Interval_EPI_Count >= Ping_Interval_EPI_Multiplier),
  THEN
  */
  if (    as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count
       >= kps_initCalc->u16_Ping_Interval_EPI_Multiplier
     )
  {
    /* Start of Ping_Count_Interval. As this function is executed exactly     */
    /* once per EPI this happens each EPI * Ping_Interval_EPI_Multiplier      */
    /* (see FRS232). */
    /* Increment Mode_Byte.Ping_Count to request Time Coordination Messages   */
    /* (see FRS75, FRS89 and FRS233) (ping request, see FRS90) */
    if (((as_DataMsg[u16_svcIdx].b_modeByte) & IXSVD_k_MB_PING_COUNT)
          == IXSVD_k_MB_PING_COUNT)
    {
      as_DataMsg[u16_svcIdx].b_modeByte &=
        (CSS_t_BYTE)~(CSS_t_BYTE)IXSVD_k_MB_PING_COUNT;
    }
    else
    {
      (as_DataMsg[u16_svcIdx].b_modeByte)++;
    }

    /* Ping_Interval_EPI_Count = 0,*/
    as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count = 0U;
  }
  else
  {
    /* Ping_Count_Interval is already running */
  }
  /* ENDIF */

  /*
  // Set the remainder of the Mode_Byte bits
  Mode_Byte.Run_Idle = Application_Run_Idle,
  Mode_Byte.TBD_Bit = 0,
  Mode_Byte.TBD_2_Bit = 0,
  */
  /* (see FRS34, FRS117, FRS180 and FRS182) */
  /* Also set for Single-Cast producers. Might be overwritten later if not    */
  /* yet a Time Coordination message received (see FRS181).                   */
  if (b_application_Run_Idle != 0U)
  {
    IXUTL_BIT_SET_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_RUN_IDLE);
  }
  else
  {
    IXUTL_BIT_CLR_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_RUN_IDLE);
  }
  /* (see FRS126) */
  IXUTL_BIT_CLR_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_TBD_BIT);
  IXUTL_BIT_CLR_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_TBD_2_BIT);

  /*
  // Execute the safety producer, connection type specific functions
  IF (Connection_Type == multi-cast),
  THEN
  */
  if (IXSVD_IsMultiCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
    /* multi_cast_producer_function(), (see FRS23) */
    MultiCastProducerFunction(u16_svcIdx);
  }
  /* ELSE */
  else
  {
    /* single_cast_producer_function(), (see FRS16) */
    SingleCastProducerFunction(u16_svcIdx);
  }
  /* ENDIF */

  /*
  // set Mode_Byte redundant bits (2:4) to the correct values
  Mode_Byte = (Mode_Byte AND 0xe3)OR((Mode_Byte>>3) AND 0x1c XOR 0x14),
  */
  /* (see FRS183 and FRS191) */
  as_DataMsg[u16_svcIdx].b_modeByte =
    IXCRC_ModeByteRedundantBitsCalc(as_DataMsg[u16_svcIdx].b_modeByte);

  /* Calculate CRC(s) based on the format used */
  IXSVC_CrcCalc(u16_svcIdx,
          #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) /* CCT_NO_PRE_WARNING */
            #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) /* CCT_NO_PRE_WARNING */
                as_SvcLocals[u16_svcIdx].u16_pidRcCrcS3,
            #endif
                as_SvcLocals[u16_svcIdx].u32_pidRcCrcS5,
          #endif
                &as_DataMsg[u16_svcIdx]);

  /* //Trigger the sending of the Data Message, */
  /* Message must be sent by the caller */

  /*
  // If on DeviceNet and Multi-cast, send the Time Correction
  // Message if it is time.
  IF ((Connetion_Type == DeviceNet) AND
    (Send_Time_Correction_Message == 1)),
  THEN
  {
    Trigger the sending of the Time Correction Message,
    Send_Time_Correction_Message = 0,
  }
  ENDIF
  //////////////////////////////////////////////////////////////////
  // end of safety data production processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : SingleCastProducerFunction
**
** Description : This function checks for a single-cast producer if a Time
**               Coordination message has been received in the allotted time. It
**               also handles special behavior after initiating the connection
**               until the first Time Coordination message was received.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : -
**
*******************************************************************************/
static void SingleCastProducerFunction(CSS_t_UINT u16_svcIdx)
{
  /*
  //////////////////////////////////////////////////////////////////
  // Start of single-cast production function
  //////////////////////////////////////////////////////////////////
  single_cast_producer_function()
  */
  /* pointer to per consumer data of the processed consumer */
  IXSVC_t_VC_PER_CONSUMER_DATA *const ps_svcPerCons =
    &as_SvcLocals[u16_svcIdx].as_perCons[0U];
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* pointer to the initially calculated values of this instance
     (u16_pidCrcS3 and u32_pidCrcS5 are needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);
#endif
  /* get a pointer to per consumer data */
  const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
    IXSVC_InstInfoPerConsGet(u16_svcIdx, 0U);

  /*
  // Check if a Time Coordination message has been received in
  // the allotted time.
  IF (Ping_Interval_EPI_Count == 8),
  THEN
  */
  if (as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count == 8U)
  {
    /* Increment Ping_Int_Since_Last_Time_Coord_Msg_Count[0], */
    /* (see FRS269) */
    ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count++;

    /*
    IF (Ping_Int_Since_Last_Time_Coord_Msg_Count[0]
       >= (Timeout_Multiplier.PI [0] + 2)),
    THEN
    */
    /* (see FRS229) */
    if (ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count
       >= (kps_iiperCons->s_Timeout_Multiplier.u8_PI + 2U))
    {
      /* S_Connection_Fault[0]=Faulted, */
      /* (see FRS222, FRS223 and FRS272) */
      IXSVC_ConnectionFaultFlagSet(u16_svcIdx, 0U,
                                   IXSVD_k_CNXN_FAULTED);
      /* Increment fault counter */
      IXSVC_FaultCountInc();

      IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_TO_SC,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);

      /* as this is a single-cast connection this was the last/only consumer */
      /* Update states and notify application (see FRS10 and FRS12-1). */
      IXSVC_StateMachine(u16_svcIdx, CSOS_k_MAX_CONSUMER_NUM_SCAST,
                         IXSVC_k_VE_ALL_CONS_FAULTED);
    }
    else
    {
      /* connection is still ok */
    }
    /* ENDIF */
  }
  else
  {
    /* not yet the 8th EPI count */
  }
  /* ENDIF */

  /*
  //Hold Data_Time_Stamp = 0 until time coordination msg received (see FRS283)
  IF(Consumer_Active_Idle[0] == Idle),
  THEN
  */
  if (!IXSVC_ConsumerIsActive(u16_svcIdx, 0U))
  {
    /* Mode_Byte.Run_Idle = Idle, */  /* (see FRS180) */
    IXUTL_BIT_CLR_U8(as_DataMsg[u16_svcIdx].b_modeByte, IXSVD_k_MB_RUN_IDLE);
    /* Time_Stamp_Section.Data_Time_Stamp = 0x0000, */
    as_DataMsg[u16_svcIdx].u16_timeStamp_128us = 0x0000U;
    /*
    IF (ExtendedFormat),
    THEN
    */
    if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        /* Rollover count used in CRC calculations */
        /*
        // Set seed value to 0 for time stamp equal to 0
        RC_Used_in_CRC = 0x0000
        */
        CSS_t_UINT u16_RC_Used_in_CRC = 0x0000U;

        /* recalculate PID/RolloverCount CRC seeds */
        as_SvcLocals[u16_svcIdx].u16_pidRcCrcS3 =
          IXCRC_PidRcCrcS3Calc(u16_RC_Used_in_CRC, kps_initCalc->u16_pidCrcS3);
        as_SvcLocals[u16_svcIdx].u32_pidRcCrcS5 =
          IXCRC_PidRcCrcS5Calc(u16_RC_Used_in_CRC, kps_initCalc->u32_pidCrcS5);
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_14,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else
    {
      /* not relevant for base format */
    }
    /* ENDIF */

  }
  /* ELSE */
  else
  {
    /*
    Time_Stamp_Section.Data_Time_Stamp = Producer_Safe_Data_TS +
      Consumer_Time_Correction_Value[0],
    */
    /* (see FRS18 and FRS73) */
    as_DataMsg[u16_svcIdx].u16_timeStamp_128us = (CSS_t_UINT)
      (as_SvcLocals[u16_svcIdx].u16_Producer_Safe_Data_TS_128us +
      IXSVC_ConsTimeCorrValGet(u16_svcIdx, 0U));

    /*
    IF (ExtendedFormat),
    THEN
    */
    if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      /* Rollover count used in CRC calculations */
      CSS_t_UINT u16_RC_Used_in_CRC;

      /*
      // check for rollover
      IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp <
          Last_Time_Stamp_For_Rollover))
      THEN
      */
      if (as_DataMsg[u16_svcIdx].u16_timeStamp_128us
          < as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us)
      {
        /* TS_Rollover_Count = TS_Rollover_Count + 1 */
        as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count++;
      }
      else
      {
        /* nothing to do */
      }
      /* ENDIF */
      /*
      // set the seed value
      RC_Used_in_CRC = TS_Rollover_Count
      */
      u16_RC_Used_in_CRC = as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count;

      /* recalculate PID/RolloverCount CRC seeds */
      as_SvcLocals[u16_svcIdx].u16_pidRcCrcS3 =
        IXCRC_PidRcCrcS3Calc(u16_RC_Used_in_CRC, kps_initCalc->u16_pidCrcS3);
      as_SvcLocals[u16_svcIdx].u32_pidRcCrcS5 =
        IXCRC_PidRcCrcS5Calc(u16_RC_Used_in_CRC, kps_initCalc->u32_pidCrcS5);
      /*
      // save the time stamp
      Last_Time_Stamp_For_Rollover = Time_Stamp_Section.Data_Time_Stamp
      */
      as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us =
        as_DataMsg[u16_svcIdx].u16_timeStamp_128us;
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_15,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else
    {
      /* not relevant for base format */
    }
    /* ENDIF */

  }
  /* ENDIF */
  /*
  END single_cast_ producer_function()
  //////////////////////////////////////////////////////////////////
  // end of single-cast processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : MultiCastProducerFunction
**
** Description : This function checks if it's time to start sending Time
**               Correction messages. Time Correction messages are sent starting
**               at the 8th EP production within the Ping interval.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : -
**
*******************************************************************************/
static void MultiCastProducerFunction(CSS_t_UINT u16_svcIdx)
{
  /*
  //////////////////////////////////////////////////////////////////
  // Start of multi-cast production function
  //////////////////////////////////////////////////////////////////
  multi_cast_producer_function()
  */
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* pointer to the initially calculated values of this instance
     (u16_pidCrcS3 and u32_pidCrcS5 are needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);
#endif
  /* local copy of the RR Consumer Number Index Pointer */
  CSS_t_USINT u8_RR_Con_Num_Index_Pntr = 0U;
  /* local copy of Maximum_Consumer_Number */
  const CSS_t_USINT u8_maxConsNum = IXSVC_InitInfoMaxConsNumGet(u16_svcIdx);

  /*
  // Check if its time to start sending Time Correction messages
  // Time Correction messages are sent starting at the 8th EPI production
  // within the Ping interval (see FRS237)
  IF (Ping_Interval_EPI_Count == 8),
  THEN
  */
  /* (see FRS257) */
  if (as_SvcLocals[u16_svcIdx].u16_Ping_Interval_EPI_Count == 8U)
  {
    /* RR_Con_Num_Index_Pntr = 0, */
    IXSVC_RrConNumIdxPntrSet(u16_svcIdx, 0U, u8_maxConsNum);  /* (see FRS258) */
  }
  else
  {
    /* not yet the 8th EPI count */
    /* set local variable to the current value */
    u8_RR_Con_Num_Index_Pntr = IXSVC_RrConNumIdxPntrGet(u16_svcIdx);
  }
  /* ENDIF */

  /*
  // Check if a Time Coordination message has been received in
  // the allotted time.
  IF (RR_Con_Num_Index_Pntr < Max_Consumer_Number),
  THEN
  */
  if (u8_RR_Con_Num_Index_Pntr < u8_maxConsNum)
  {
    /* pointer to per consumer data of the processed consumer */
    IXSVC_t_VC_PER_CONSUMER_DATA *const ps_svcPerCons =
      &as_SvcLocals[u16_svcIdx].as_perCons[u8_RR_Con_Num_Index_Pntr];
    /* get a pointer to Instance info per consumer */
    const IXSVC_t_INST_INFO_PER_CONS *const kps_iiperCons =
      IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_RR_Con_Num_Index_Pntr);

    /* (see FRS270) */
    /* Increment
         Ping_Int_Since_Last_Time_Coord_Msg_Count[RR_Con_Num_Index_Pntr], */
    ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count++;
    /*
    IF (Ping_Int_Since_Last_Time_Coord_Msg_Count[RR_Con_Num_Index_Pntr]
        >= (Timeout_Multiplier.PI [RR_Con_Num_Index_Pntr] + 2)),
    THEN
    */
    /* Additionally to the example code we have to check if this consumer is
       actually open and we have to check the Producer Connection Status if
       this consumer is not already faulted (to prevent the following code from
       being executed each ping interval again) */
    /* (see FRS229) */
    if (    (!IXSVC_ConnectionIsFaulted(u16_svcIdx, u8_RR_Con_Num_Index_Pntr))
         && (kps_iiperCons->o_Consumer_Open)
         && (ps_svcPerCons->u8_Ping_Int_Since_Last_Time_Coord_Msg_Count
             >= (kps_iiperCons->s_Timeout_Multiplier.u8_PI + 2U))
       )
    {
      /* Set S_Connection_Fault[RR_Con_Num_Index_Pntr]=Faulted, */
      /* (see FRS222, FRS98, FRS223 and FRS272) */
      IXSVC_ConnectionFaultFlagSet(u16_svcIdx, u8_RR_Con_Num_Index_Pntr,
                                   IXSVD_k_CNXN_FAULTED);
      /* Increment fault counter */
      IXSVC_FaultCountInc();

      IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_TO_MC,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      ((CSS_t_UDINT)u8_RR_Con_Num_Index_Pntr + 1U));

      /* if there are consumers left */
      if (IXSVC_ConsumerRemainingCheck(u16_svcIdx))
      {
        /* one consumer failed, there are still other consumers active */
        /* Update states and notify application (see FRS10 and FRS12-1). */
        IXSVC_StateMachine(u16_svcIdx,
                           (CSS_t_USINT)(u8_RR_Con_Num_Index_Pntr + 1U),
                           IXSVC_k_VE_CONSUMER_FAULTED);

        /* start quarantine timer to prevent re-use of consumer number */
        IXSVC_QuarConsStart(u16_svcIdx,
                            (CSS_t_USINT)(u8_RR_Con_Num_Index_Pntr + 1U));
      }
      else /* else: last consumer faulted */
      {
        /* This was the last consumer of the connection which now failed */
        /* Update states and notify application (see FRS10 and FRS12-1). */
        IXSVC_StateMachine(u16_svcIdx,
                           (CSS_t_USINT)(u8_RR_Con_Num_Index_Pntr + 1U),
                           IXSVC_k_VE_ALL_CONS_FAULTED);
      }
    }
    else
    {
      /* connection is still ok (or not open or already faulted) */
    }
    /* ENDIF */
  }
  else
  {
    /* all consumers already handled */
  }
  /* ENDIF */

  /*
  // Check if its time to send an active non-errored Time Correction
  // message
  */
  IXSVC_TcorrMsgProd(u16_svcIdx);

  /*
  // If Time Correction messages are being sent, increment to the
  // next consumer
  IF (RR_Con_Num_Index_Pntr < Max_Consumer_Number),
  THEN
  */
  if (u8_RR_Con_Num_Index_Pntr < u8_maxConsNum)
  {
    /* RR_Con_Num_Index_Pntr = RR_Con_Num_Index_Pntr + 1,*/
    u8_RR_Con_Num_Index_Pntr++;  /* (see FRS258) */
    IXSVC_RrConNumIdxPntrSet(u16_svcIdx, u8_RR_Con_Num_Index_Pntr,
                             u8_maxConsNum);
  }
  else
  {
    /* all consumers already handled */
  }
  /* ENDIF */

  /* For Multi-cast the time stamp is equal to producer clock (see FRS285) */
  /*
  // Set the time stamp value
  Time_Stamp_Section.Data_Time_Stamp = Producer_Safe_Data_TS,
  */  /* (see FRS24) */
  as_DataMsg[u16_svcIdx].u16_timeStamp_128us =
    as_SvcLocals[u16_svcIdx].u16_Producer_Safe_Data_TS_128us;

  /*
  IF (ExtendedFormat),
  THEN
  */
  if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* Rollover count used in CRC calculations */
    CSS_t_UINT u16_RC_Used_in_CRC;

    /*
    // check for rollover
    IF (unsigned compare(Time_Stamp_Section.Data_Time_Stamp
         < Last_Time_Stamp_For_Rollover))
    THEN
    */
    if (as_DataMsg[u16_svcIdx].u16_timeStamp_128us
        < as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us)
    {
      /* TS_Rollover_Count = TS_Rollover_Count + 1 */
      as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count++;
    }
    else
    {
      /* no rollover */
    }
    /* ENDIF */
    /*
    // set the seed value
    RC_Used_in_CRC = TS_Rollover_Count
    */
    u16_RC_Used_in_CRC = as_SvcLocals[u16_svcIdx].u16_TS_Rollover_Count;

    /* recalculate PID/RolloverCount CRC seeds */
    /* In contrast to singlecast the Rollover Count is her always used,
       independent of active/idle (see FRS378) */
    as_SvcLocals[u16_svcIdx].u16_pidRcCrcS3 =
      IXCRC_PidRcCrcS3Calc(u16_RC_Used_in_CRC, kps_initCalc->u16_pidCrcS3);
    as_SvcLocals[u16_svcIdx].u32_pidRcCrcS5 =
      IXCRC_PidRcCrcS5Calc(u16_RC_Used_in_CRC, kps_initCalc->u32_pidCrcS5);
    /*
    // save the time stamp
    Last_Time_Stamp_For_Rollover = Time_Stamp_Section.Data_Time_Stamp
    */
    as_SvcLocals[u16_svcIdx].u16_Last_Time_Stamp_For_Rollover_128us =
      as_DataMsg[u16_svcIdx].u16_timeStamp_128us;
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_16,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else
  {
    /* not relevant for base format */
  }
  /* ENDIF */

  /*
  END multi_cast_producer_function()
  //////////////////////////////////////////////////////////////////
  // end of multi-cast processing
  //////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


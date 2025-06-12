/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCtcoo.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the functions to process a received Time
**             Coordination message.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_TcooLocalsClear
**             IXSVC_TcooRxProc
**             IXSVC_TcooMsgMinMultSet
**             IXSVC_TcooCidCrcS3Set
**             IXSVC_TcooCidCrcS5Set
**             IXSVC_TcooMaxFaultNumberSet
**             IXSVC_TcooColdStartLogic
**             IXSVC_TcooMsgMinMultGet
**             IXSVC_TcooFaultCounterGet
**             IXSVC_TcooProdFaultCounterReset
**             IXSVC_TcooSoftErrByteGet
**             IXSVC_TcooSoftErrVarGet
**
**             TcooColdStartLogic
**             TcooMsgReception
**             TcooRxCopy
**             TcooCrcIntegrityCheck
**             TcooPingCntReplyVsPingCntCheck
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if  (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXUTL.h"
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSFA.h"
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

/* maximum value for the Time_Coord_Response_EPI_Limit (micro seconds) */
#define k_MAX_TCOO_RESP_EPI_LIMIT_US         1000U
/* minimum value for the Time_Coord_Response_EPI_Limit (micro seconds) */
#define k_MIN_TCOO_RESP_EPI_LIMIT_US         5U

/* maximum value for the Time_Coordination_msg_Min_Multiplier (128us ticks) */
#define k_MAX_TCOO_MSG_MIN_MULTIPLIER_128US  7813U   /* 1sec */

/* maximum value for the Connection_Correction_Constant (128us ticks) */
#define k_MAX_TIME_DRIFT_CONST_128US         1563U   /* 200ms */


/** t_SVC_LOCALS_TCOO:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* Time_Drift_Per_Ping_Interval (see Vol. 5 Chapt. 2-4.5.3.1): value in
     128uSec increments that represents the worst-case time drift due to crystal
     inaccuracy during on Ping Interval.
     Legal range: k_TIME_DRIFT_PER_PING_INT_MIN_128US..
                  k_TIME_DRIFT_PER_PING_INT_MAX_128US (128us ticks) */
  /* Error in Example Code: Time_Drift_Per_Ping_Interval exists only once,
     not per consumer. */
  CSS_t_UINT      u16_Time_Drift_Per_Ping_Interval_128us;

} t_SVC_LOCALS_TCOO;


/** t_SVC_LOCALS_PER_CONS_TCOO:
    type for the local variables which have to be stored for each consumer
    grouped into a struct
*/
typedef struct
{
  /* Time_Coord_Msg_Min_Multiplier (see Vol. 5 Chapt. 2-4.5.2.6): is the minimum
     number of 128 uSec increments it could take for a Time Coordination Message
     to traverse from the consumer to the producer.
     Legal range: 0..k_MAX_TCOO_MSG_MIN_MULTIPLIER_128US (0..1sec) */
  CSS_t_UINT      u16_Time_Coord_Msg_Min_Multiplier_128us;

  /* Connection_Correction_Constant (see Vol. 5 Chapt. 2-4.5.3.2): value in 128
     uSec increments that is subtracted from the time stamp to represent the
     worst case error.
     Legal range: -7811..1564 (just for information, this value is calculated
                               from other values already checked before) */
  CSS_t_UINT      u16_Connection_Correction_Constant_128us;

  /* Time_Coord_Response_EPI_Limit (see Vol. 5 Chapt. 2-4.5.3.3): used to ensure
     that a Time Coordination message that returns to the producer requesting
     it, arrives within a time sufficient to prevent the possibility of a 16 bit
     time stamp math rollover.
     Legal range: 5..1000 (micro seconds) */
  CSS_t_UINT      u16_Time_Coord_Response_EPI_Limit_us;

  /* Consumer_Time_Value (see Vol. 5 Chapt. 2-4.5.5.1): is the time value sent
     to the producer of a time stamp from the consumer in the Time Coordination
     Message.
     Legal range: 0..65535 (128us ticks) */
  CSS_t_UINT      u16_Consumer_Time_Value_128us;

  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* Producer_Fault_Counter (see Vol. 5 Chapt. 2-4.5.5.5): tracks the number
       of data integrity errors the producer detects on the reception of Time
       Coordination messages.
       Legal range: 0..255 */
    CSS_t_USINT   u8_Producer_Fault_Counter;

    /* Max Fault Number
       Legal range: 0..255 */
    CSS_t_USINT     u8_Max_Fault_Number;
  #endif

  /* Producer_Rcved_Time_Value (see Vol. 5 Chapt. 2-4.5.5.2): point in time when
     the Time Coordination Message information is received from the time stamp
     of the consumer.
     Legal range: 0..65535 (128us ticks) */
  CSS_t_UINT      u16_Producer_Rcved_Time_Value_128us;  /* (see FRS265) */

  /*
  ** seed values for CRC calculations
  */
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    CSS_t_UINT      u16_cidCrcS3;     /* CRC-S3 over CID */
  #endif
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UDINT   u32_cidCrcS5;     /* CRC-S5 over CID */
  #endif

} t_SVC_LOCALS_PER_CONS_TCOO;


static t_SVC_LOCALS_TCOO as_SvcLocals[CSOS_cfg_NUM_OF_SV_CLIENTS];

static t_SVC_LOCALS_PER_CONS_TCOO
  aas_SvcPcLocals[CSOS_cfg_NUM_OF_SV_CLIENTS][CSOS_k_MAX_CONSUMER_NUM_MCAST];

/* structure for storing received Time Coordination Messages */
/* Not relevant for Soft Error Checking - part of the safety function but value
   is updated before each use */
static CSS_t_TCOO_MSG as_TcooMsg[CSOS_cfg_NUM_OF_SV_CLIENTS];

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #define k_TCOO_SIZEOF_VAR1   (sizeof(as_SvcLocals))
  #define k_TCOO_SIZEOF_VAR12  (k_TCOO_SIZEOF_VAR1 + sizeof(aas_SvcPcLocals))

#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/* define IDs of the variables depending on which variables are supported */
enum
{
   k_SOFT_ERR_VAR_TCMMM
  ,k_SOFT_ERR_VAR_CCC
  ,k_SOFT_ERR_VAR_TCREL
  ,k_SOFT_ERR_VAR_CNOSTV
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_PFC
  ,k_SOFT_ERR_OPT_VAR_MFN
#endif
  ,k_SOFT_ERR_VAR_PRTV
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_CIDCRCS3
#endif
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_CIDCRCS5
#endif
};
#endif  /* CSS_cfg_SOFT_ERR_METHOD */



/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void TcooColdStartLogic(CSS_t_UINT u16_svcIdx,
                               CSS_t_USINT u8_consIdx,
                               const IXSVC_t_INST_INFO_PER_CONS *ps_iipc,
                               const IXSVC_t_INIT_INFO *ps_initInfo);
static void TcooMsgReception(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consNum);
static CSS_t_WORD TcooRxCopy(CSS_t_UINT u16_svcIdx,
                             CSS_t_UINT u16_len,
                             const CSS_t_BYTE *pb_data);
static CSS_t_BOOL TcooCrcIntegrityCheck(CSS_t_UINT u16_svcIdx,
                                        CSS_t_USINT u8_consNum);
static CSS_t_BOOL TcooPingCntReplyVsPingCntCheck(CSS_t_BYTE b_ackByte,
                                                 CSS_t_BYTE b_modeByte);


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_TcooLocalsClear(CSS_t_UINT u16_svcIdx)
{
  CSS_MEMSET(&as_SvcLocals[u16_svcIdx], 0, sizeof(as_SvcLocals[0]));
  CSS_MEMSET(aas_SvcPcLocals[u16_svcIdx], 0, sizeof(aas_SvcPcLocals[0]));
  CSS_MEMSET(&as_TcooMsg[u16_svcIdx], 0, sizeof(as_TcooMsg[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                      const CSS_t_BYTE *pb_data)
{
  /* if passed consumer number is invalid */
  if (    (u8_consNum == 0U)
       || (u8_consNum > CSOS_k_MAX_CONSUMER_NUM_MCAST)
     )
  {
    IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_CONS_NUM_CRP,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    IXSER_k_A_NOT_USED);
  }
  else/* else: consumer number is ok */
  {
    /* if this instance is in state Init or Established */
    if (IXSVC_StateIsInitOrEstablished(u16_svcIdx))
    {
      /* if copying the received Time Coord Msg into message structs failed */
      if (TcooRxCopy(u16_svcIdx, u16_len, pb_data) != CSS_k_OK)
      {
        /* received message contained errors (error code already set) */
      }
      else /* else: ok */
      {
        /* continue processing in sub-function */
        TcooMsgReception(u16_svcIdx, u8_consNum);
      }
    }
    else /* else: not in use or faulted */
    {
      /* this instance cannot be processed */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}



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
                             CSS_t_UINT u16_value_128us)
{
  /* if passed value is out of range */
  if (u16_value_128us > k_MAX_TCOO_MSG_MIN_MULTIPLIER_128US)
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_TCOOMMM_RANGE,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u16_value_128us);
    /* limit to maximum value */
    aas_SvcPcLocals[u16_svcIdx][u8_consIdx].
      u16_Time_Coord_Msg_Min_Multiplier_128us
        = k_MAX_TCOO_MSG_MIN_MULTIPLIER_128US;
  }
  else /* else: passed value is ok */
  {
    aas_SvcPcLocals[u16_svcIdx][u8_consIdx].
      u16_Time_Coord_Msg_Min_Multiplier_128us = u16_value_128us;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
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
void IXSVC_TcooCidCrcS3Set(CSS_t_UINT u16_svcIdx,
                           CSS_t_USINT u8_consIdx,
                           CSS_t_UINT u16_value)
{

  aas_SvcPcLocals[u16_svcIdx][u8_consIdx].u16_cidCrcS3 = u16_value;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
void IXSVC_TcooCidCrcS5Set(CSS_t_UINT u16_svcIdx,
                           CSS_t_USINT u8_consIdx,
                           CSS_t_UDINT u32_value)
{
  aas_SvcPcLocals[u16_svcIdx][u8_consIdx].u32_cidCrcS5 = u32_value;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
void IXSVC_TcooMaxFaultNumberSet(CSS_t_UINT u16_svcIdx,
                                 CSS_t_USINT u8_consIdx,
                                 CSS_t_USINT u8_value)
{
  aas_SvcPcLocals[u16_svcIdx][u8_consIdx].u8_Max_Fault_Number = u8_value;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
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
                              const IXSVC_t_INST_INFO_PER_CONS *ps_iipc)
{

  /* if the instance must be initialized completely */
  if (u8_consIdx >= CSOS_k_MAX_CONSUMER_NUM_MCAST)
  {
    /* loop counter for the Consumer Index */
    CSS_t_USINT u8_cidx;

    /*
    // Time_Drift_Per_Ping_Interval, the minimum value is 1
    Time_Drift_Per_Ping_Interval =
      Roundup(EPI * Ping_Interval_EPI_Multiplier / 320000),
    */
    /* The divide by 320000 factor in the Time_Drift_Per_Ping_Interval
       calculation is equivalent to multiplying by the worst case clock
       drift of .0004 and dividing by 128 uSeconds. */
    /* Time_Drift_Per_Ping_Interval, the minimum value is 1 */
    as_SvcLocals[u16_svcIdx].u16_Time_Drift_Per_Ping_Interval_128us =
      (CSS_t_UINT)IXSVC_RoundupDivision(ps_initInfo->s_initCalc.u32_EPI_us *
                      ps_initInfo->s_initCalc.u16_Ping_Interval_EPI_Multiplier,
                      320000UL);

    /* call the Tcoo Cold Start Logic for each consumer */
    for (u8_cidx = 0U; u8_cidx < ps_initInfo->u8_Max_Consumer_Number; u8_cidx++)
    {
      TcooColdStartLogic(u16_svcIdx, u8_cidx, ps_iipc, ps_initInfo);
    }
  }
  else /* else: only one consumer must be re-initialized */
  {
    TcooColdStartLogic(u16_svcIdx, u8_consIdx, ps_iipc, ps_initInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                   CSS_t_USINT u8_consIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (aas_SvcPcLocals[u16_svcIdx][u8_consIdx].
          u16_Time_Coord_Msg_Min_Multiplier_128us);
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
CSS_t_USINT IXSVC_TcooFaultCounterGet(CSS_t_UINT u16_svcIdx,
                                      CSS_t_USINT u8_consIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (aas_SvcPcLocals[u16_svcIdx][u8_consIdx].u8_Producer_Fault_Counter);
}


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
void IXSVC_TcooProdFaultCounterReset(void)
{
  CSS_t_UINT u16_svcIdx;
  CSS_t_USINT u8_consIdx;

  for (u16_svcIdx = 0U; u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_svcIdx++)
  {
    for (u8_consIdx = 0U;
         u8_consIdx < CSOS_k_MAX_CONSUMER_NUM_MCAST;
         u8_consIdx++)
    {
      aas_SvcPcLocals[u16_svcIdx][u8_consIdx].u8_Producer_Fault_Counter = 0U;
    }
  }
}
#endif


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVC_TcooSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_TCOO_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvcLocals)+u32_varCnt);
  }
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_TCOO_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)aas_SvcPcLocals)
                              +(u32_varCnt - k_TCOO_SIZEOF_VAR1));
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_TCOO_SIZEOF_VAR12;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVC_TcooSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* select the level 0 variable */
  switch (au32_cnt[0U])
  {
    case 0U:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else /* level 1 counter in range of array elements */
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case 0U:
          {
            CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[1U]].u16_Time_Drift_Per_Ping_Interval_128us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            ps_retStat->u8_cpyLen = 0U;
            break;
          }
        }
      }
    }
    break;

    case 1U:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else /* level 1 counter in range of array elements */
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* if level 2 counter is larger than number of array elements */
        if (au32_cnt[2U] >= CSOS_k_MAX_CONSUMER_NUM_MCAST)
        {
          /* level 2 counter at/above end */
          /* default return values already set */
        }
        else /* level 2 counter in range of array elements */
        {
          /* indicate to caller that we are in range of level 3 or below */
          ps_retStat->u8_incLvl = 3U;

          /* select the level 2 variable */
          switch (au32_cnt[3U])
          {
            case k_SOFT_ERR_VAR_TCMMM:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_Time_Coord_Msg_Min_Multiplier_128us);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

            case k_SOFT_ERR_VAR_CCC:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_Connection_Correction_Constant_128us);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

            case k_SOFT_ERR_VAR_TCREL:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_Time_Coord_Response_EPI_Limit_us);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

            case k_SOFT_ERR_VAR_CNOSTV:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_Consumer_Time_Value_128us);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

          #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
            case k_SOFT_ERR_OPT_VAR_PFC:
            {
              CSS_H2N_CPY8(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u8_Producer_Fault_Counter);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
              break;
            }

            case k_SOFT_ERR_OPT_VAR_MFN:
            {
              CSS_H2N_CPY8(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u8_Max_Fault_Number);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
              break;
            }
          #endif

            case k_SOFT_ERR_VAR_PRTV:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_Producer_Rcved_Time_Value_128us);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

          #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
            case k_SOFT_ERR_OPT_VAR_CIDCRCS3:
            {
              CSS_H2N_CPY16(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u16_cidCrcS3);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

          #endif

          #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
            case k_SOFT_ERR_OPT_VAR_CIDCRCS5:
            {
              CSS_H2N_CPY32(pb_var, &aas_SvcPcLocals[au32_cnt[1U]][au32_cnt[2U]].u32_cidCrcS5);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
              break;
            }
          #endif

            default:
            {
              /* level 3 counter at/above end */
              /* default return values already set */
              break;
            }
          }
        }
      }
      break;
    }

    default:
    {
      /* level 0 counter at/above end */
      /* default return values already set */
      break;
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
** Function    : TcooColdStartLogic
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
**               u8_consIdx (IN)  - index of the consumer of the addressed
**                                  instance
**                                  (not checked, checked in
**                                  IXSVC_TcooColdStartLogic())
**               ps_iipc (IN)     - pointer to a struct containing per consumer
**                                  data
**                                  (not checked, only called with
**                                  reference to struct)
**               ps_initInfo (IN) - pointer to a struct for collecting
**                                  initialization data
**                                  (not checked, only called with
**                                  reference to struct)
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooColdStartLogic(CSS_t_UINT u16_svcIdx,
                               CSS_t_USINT u8_consIdx,
                               const IXSVC_t_INST_INFO_PER_CONS *ps_iipc,
                               const IXSVC_t_INIT_INFO *ps_initInfo)
{
  /* pointer to local data of this consumer */
  t_SVC_LOCALS_PER_CONS_TCOO *const ps_svcPerCons =
    &aas_SvcPcLocals[u16_svcIdx][u8_consIdx];
  /* temporary variable for making mathematical expressions less complex */
  CSS_t_UDINT u32_temp = 0UL;
  /* temporary variable needed for calculating the
     Connection_Correction_Constant */
  CSS_t_UINT u16_Time_Drift_Constant_128us;

  /*
  // Producer Derived Variables

  // Time_Drift_Constant, the minimum value is 1. (Time_Drift_Constant is not
  // saved but is used in the calculation of the
  // Connection_Correction_Constant below.)
  Time_Drift_Constant =
    Roundup((Timeout_Multiplier.PI [Consumer_Num-1] +1) * EPI *
    Ping_Interval_EPI_Multiplier / 320000),
  */
  /* (see FRS245) */
  /* The 1/320000 factor in the Time_Drift_Constant calculation is equivalent to
     multiplying by the worst case clock drift of .0004 and dividing by 128. */
  u32_temp = ps_iipc->s_Timeout_Multiplier.u8_PI;
  u32_temp++;
  u32_temp *= ps_initInfo->s_initCalc.u32_EPI_us;
  u32_temp *= ps_initInfo->s_initCalc.u16_Ping_Interval_EPI_Multiplier;
  u16_Time_Drift_Constant_128us = (CSS_t_UINT)IXSVC_RoundupDivision(u32_temp,
                                                                    320000UL);
  /* if the calculated Time_Drift_Constant is out of range */
  if (u16_Time_Drift_Constant_128us > k_MAX_TIME_DRIFT_CONST_128US)
  {
    /* Fail Safe Error */
    IXSVC_ErrorClbk(IXSVC_k_FSE_T_DRIFT_CONST_RANGE,
      IXSVC_InstIdFromIdxGet(u16_svcIdx),
      (CSS_t_UDINT)u16_Time_Drift_Constant_128us);
    /* limit to maximum value */
    u16_Time_Drift_Constant_128us = k_MAX_TIME_DRIFT_CONST_128US;
  }
  else /* else: ok */
  {
    /* do nothing */
  }

  /* (see FRS266 */
  ps_svcPerCons->u16_Producer_Rcved_Time_Value_128us = 0U;
  /* (see FRS263) */
  /* Error in Example Code: This variable is only initialized in              */
  /* Cold-Start-Logic, but also needs to be re-initialized in Multi-cast      */
  /* re-initialization.                                                       */
  ps_svcPerCons->u16_Consumer_Time_Value_128us = 0U;
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      ps_svcPerCons->u8_Producer_Fault_Counter = 0U;
    }
  #endif

  /*
  // Connection_Correction_Constant
  Connection_Correction_Constant[Consumer_Num-1] =
    Time_Drift_Constant + 1 - Time_Coord_Msg_Min_Multiplier [Consumer_Num-1],
  */
  ps_svcPerCons->u16_Connection_Correction_Constant_128us =
    (CSS_t_UINT)((u16_Time_Drift_Constant_128us + 1U) -
    ps_svcPerCons->u16_Time_Coord_Msg_Min_Multiplier_128us);

  /*
  // Time_Coord_Response_EPI_Limit
  Time_Coord_Response_EPI_Limit[Consumer_Num-1] = Roundup((5000000 +
    (Time_Coord_Msg_Min_Multiplier[Consumer_Num-1]*128) +
    (EPI * ( Consumer_Num - 1))) / EPI),
  */
  u32_temp = ps_svcPerCons->u16_Time_Coord_Msg_Min_Multiplier_128us;
  u32_temp *= CSS_k_TICK_BASE_US;
  u32_temp += IXSVD_k_5_SECONDS_US;
  u32_temp += (ps_initInfo->s_initCalc.u32_EPI_us * u8_consIdx);
  ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us = /* (see FRS247) */
  (CSS_t_UINT)IXSVC_RoundupDivision(u32_temp,
                                    ps_initInfo->s_initCalc.u32_EPI_us);

  /*
  // Time_Coord_Response_EPI_Limit has a maximum value of 1000
  IF (Time_Coord_Response_EPI_Limit[Consumer_Num-1] > 1000),
    THEN
    {
      Time_Coord_Response_EPI_Limit[Consumer_Num-1] = 1000,
    }
  ENDIF
  */
  if (ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us
      > k_MAX_TCOO_RESP_EPI_LIMIT_US)
  {
    /* then limit to maximum value -> this is not an error (see FRS247) */
    ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us
      = k_MAX_TCOO_RESP_EPI_LIMIT_US;
  }
  else if (ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us
           < k_MIN_TCOO_RESP_EPI_LIMIT_US)
  {
    /* Fail Safe Error -> if all other values were in range than this would not
       have happened */
    IXSVC_ErrorClbk(IXSVC_k_FSE_TCREL_OUT_OF_RANGE,
      IXSVC_InstIdFromIdxGet(u16_svcIdx),
      (CSS_t_UDINT)ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us);
    /* limit to minimum value */
    ps_svcPerCons->u16_Time_Coord_Response_EPI_Limit_us
      = k_MIN_TCOO_RESP_EPI_LIMIT_US;
  }
  else
  {
    /* value in range: no need to limit the value */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : TcooMsgReception
**
** Description : This function is only executed if AC_Byte.Ping_Response of the
**               message is equal to 1 AND Consumer_Open[Consumer_Num-1] is
**               equal to Open. If Ack_Byte.Ping_Response is equal to 0 OR
**               Consumer_Open[Consumer_Num-1] is equal to Closed, this message
**               is ignored.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in ValidatorRxProc())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in IXSVC_TcooRxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooMsgReception(CSS_t_UINT u16_svcIdx,
                             CSS_t_USINT u8_consNum)
{
  /*
  //////////////////////////////////////////////////////////////////
  //Time Coordination message reception processing
  //////////////////////////////////////////////////////////////////
  // Consumer_Num equals 1 for single-cast
  // Consumer_Num equals the consumer the message
  // was received from for multi-cast
  //////////////////////////////////////////////////////////////////
  */
  /* temporary flag for Ack_Byte checking */
  CSS_t_BOOL o_Ack_Byte_Error = CSS_k_FALSE;
  /* temporary flag for Ping Count checking */
  CSS_t_BOOL o_pingCountError = CSS_k_FALSE;
  /* temporary flag for checking if message received within 5 second limit */
  CSS_t_BOOL o_retIn5sec = CSS_k_FALSE;
  /* flag indicating that further processing of the message must be aborted */
  CSS_t_BOOL o_abortTcooProcessing = CSS_k_FALSE;
  /* flags indicating this consumer's Active/Idle flag */
  CSS_t_BOOL o_isActive = CSS_k_FALSE;
  /* get a pointer to per consumer data */
  const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
    IXSVC_InstInfoPerConsGet(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U));

  /*
  // This Logic should only be executed If Ack_Byte.Ping_Response
  // of the message is equal to 1 AND Consumer_Open[Consumer_Num-1]
  // is equal to Open. If Ack_Byte.Ping_Response is equal to 0 OR
  // Consumer_Open[Consumer_Num-1]is equal to Closed, this
  // message should be ignored (see FRS195 and FRS196).
  */
  if (    (as_TcooMsg[u16_svcIdx].b_ackByte & IXSVD_k_AB_PING_RESPONSE)
       && (kps_iiperCons->o_Consumer_Open)
     )
  {

    /*
    // reject Time Coordination packets if CONSUMER_TIMESTAMP has previously
    // been captured AND
    // this is not the first time coordination message from this consumer
    IF ( (Consumer_Time_Value[Consumer_Num-1] ==
         Time_Coordination_Section.Consumer_Time_Value) AND
         (Consumer_Active_Idle[Consumer_Num-1] == Active) )
    THEN
    */
    /* (see FRS360) */
    if (    (IXSVC_ConsumerIsActive(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U)))
         && (aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
             u16_Consumer_Time_Value_128us
             == as_TcooMsg[u16_svcIdx].u16_consTimeVal_128us)
       )
    {
      /*
      // the only time this would be the case is if that Time Coordination was
      // already received
      Abort the processing of this packet. Do Not close the connection.
      */
      o_abortTcooProcessing = CSS_k_TRUE;
      IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_TS_DUP,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      (CSS_t_UDINT)u8_consNum);
    }
    else
    {
      /* not relevant for base format */
    }
    /* ENDIF */

    if (!o_abortTcooProcessing)
    {
      /*
      // Capture the received time
      Producer_Rcved_Time_Value[Consumer_Num-1] = Producer_Clk_Count,
      */
      /* (see FRS264) */
      aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
        u16_Producer_Rcved_Time_Value_128us = IXSVC_ProducerClkCountGet();

      /*
      // Perform the integrity checks
      Check the CRC of the Time Coordination Section, based on format used,
      Ack_Byte_Error = false,
      IF ((Ack_Byte parity incorrect)
      THEN
      */
      /* (see FRS52) */
      if (IXCRC_ParityCalc(as_TcooMsg[u16_svcIdx].b_ackByte)
          != as_TcooMsg[u16_svcIdx].b_ackByte)
      {
        /* Ack_Byte_Error = true, */
        o_Ack_Byte_Error = CSS_k_TRUE;
        IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_PARITY,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)u8_consNum);
      }
      else
      {
        /* parity is correct */
      }
      /*
      ENDIF
      */

      /*
      IF (BaseFormat)
      THEN
      */
      if (IXSVD_IsBaseFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
      {
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        CSS_t_BOOL o_ackByte2incorrect = CSS_k_FALSE;

        /* reserved Bits of the Ack_Byte will be ignored except for CRC and */
        /* Ack_Byte_2 checking (see FRS204) */

        if (as_TcooMsg[u16_svcIdx].b_ackByte2
            != IXCRC_TcByte2Calc(as_TcooMsg[u16_svcIdx].b_ackByte))
        {
          o_ackByte2incorrect = CSS_k_TRUE;
          IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_BASE_TCOO_ACK2,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          (CSS_t_UDINT)u8_consNum);
        }
        else
        {
          /* o_ackByte2incorrect already initialized to false */
        }

        /*
        //ckeck Ack_Byte_2 if BaseFormat
        IF ((Ack_Byte parity incorrect) OR (Ack_Byte_2 !=
            (((Ack_Byte XOR 0xFF) AND 0x55) OR (Ack_Byte AND 0xAA))))
        THEN
        */
        if (    (o_ackByte2incorrect) || (o_Ack_Byte_Error) )
        {
          /* Ack_Byte_Error = true, */
          o_Ack_Byte_Error = CSS_k_TRUE;
        }
        /* ELSE */
        else
        {
          /* Ack_Byte_Error = false, */
          o_Ack_Byte_Error = CSS_k_FALSE;
        }
        /* ENDIF */
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_13,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          IXSER_k_A_NOT_USED);
        }/* CCT_SKIP */ /* unreachable code - defensive programming */
      #endif
      }
      else
      {
        /* Ack_Byte_2 does not exist in extended format */
      }
      /* ENDIF */

      /* If the Consumer_Active_Idle[Consumer_Num-1] == Active (see FRS320) */
      o_isActive = IXSVC_ConsumerIsActive(u16_svcIdx,
                                          (CSS_t_USINT)(u8_consNum - 1U));
      /* if Time Coordination message returned within the 5 second limit */
      if (    (IXSVC_ProdPingIntEpiCntGet(u16_svcIdx)
               > aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
                   u16_Time_Coord_Response_EPI_Limit_us)
           && (o_isActive)
         )
      {
        o_retIn5sec = CSS_k_TRUE;
        IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_5S_LIMIT,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        (CSS_t_UDINT)u8_consNum);
      }
      else
      {
        /* o_retIn5sec already initialized to false */
      }

      /*
      IF ((CRC_Error based on format used) OR (Ack_Byte_Error = true) OR
      // Ensure that the Time Coordination message returned
      // with the same ping interval or the next ping interval.
      // The next ping interval is allowed for the case that a
      // multi-cast consumer connects to an existing producer.
      // If the new consumer receives its first message near the end
      // of the ping interval, the time coordination may arrive back
      // at the producer during the next ping interval.
        ((Ack_Byte.Ping_Count_Reply != Ping_Count) AND
         (Ack_Byte.Ping_Count_Reply != Ping_Count-1)) OR
      // Ensure that the Time Coordination message returned
      // within the approximatley 5 second limit
      ((Consumer_Active_Idle[Consumer_Num-1] == Active) AND
        (Ping_Interval_EPI_Count >
        Time_Coord_Response_EPI_Limit[Consumer_Num-1])),
      THEN
      */
      /* (see FRS55) */
      /* if Ping Count Reply doesn't match with the Ping Count */
      if (!TcooPingCntReplyVsPingCntCheck(as_TcooMsg[u16_svcIdx].b_ackByte,
                                          IXSVC_ProdModeByteGet(u16_svcIdx)))
      {
        o_pingCountError = CSS_k_TRUE;
        IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_PC_ERR,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
      else
      {
        /* o_pingCountError already initialized to CSS_k_FALSE */
      }

      if (    (!TcooCrcIntegrityCheck(u16_svcIdx, u8_consNum))
           || (o_Ack_Byte_Error)
           || (o_pingCountError)
           || (o_retIn5sec)
         )
      {
        /*
        IF (ExtendedFormat),
        THEN
        */
        if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
        {
        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          /*
          //decide whether to close the connection or not
          Increment Producer_Fault_Counter[Consumer_Num-1]
          */
          aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
            u8_Producer_Fault_Counter++;
          /*
          IF (Producer_Fault_Counter[Consumer_Num-1] >= Max_Fault_Number)
          THEN
          */
          if (aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
              u8_Producer_Fault_Counter >=
                aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
                  u8_Max_Fault_Number)
          {
            /* S_Connection_Fault[Consumer_Num-1] = Faulted, */
            /* (see FRS222 and FRS223) */
            IXSVC_ConnectionFaultFlagSet(u16_svcIdx,
                                         (CSS_t_USINT)(u8_consNum - 1U),
                                         IXSVD_k_CNXN_FAULTED);
            /* Increment global IXSVC fault counter */
            IXSVC_FaultCountInc();

            /* Abort the processing of this packet. Close the connection. */
            o_abortTcooProcessing = CSS_k_TRUE;

            if (IXSVC_ConsumerRemainingCheck(u16_svcIdx))
            {
              /* one consumer failed, there are still other consumers active  */
              /* Update states and notify application (see FRS10 and FRS12-1).*/
              IXSVC_StateMachine(u16_svcIdx, u8_consNum,
                                 IXSVC_k_VE_CONSUMER_FAULTED);

              /* start quarantine timer to prevent re-use of consumer number  */
              IXSVC_QuarConsStart(u16_svcIdx, u8_consNum);
            }
            else
            {
              /* This was the last consumer of the connection which now       */
              /* failed. Update states and notify application (see FRS10 and  */
              /* FRS12-1).                                                    */
              IXSVC_StateMachine(u16_svcIdx, u8_consNum,
                                 IXSVC_k_VE_ALL_CONS_FAULTED);
            }
          }
          /* ELSE */
          else
          {
            /* Abort the processing of this packet. Do Not close the          */
            /* connection.                                                    */
            o_abortTcooProcessing = CSS_k_TRUE;
          }
          /* ENDIF */
        #else
          {
            IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_23,
                            IXSVC_InstIdFromIdxGet(u16_svcIdx),
                            IXSER_k_A_NOT_USED);
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
        }
        /* ELSE */
        else
        {
        #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
          /* S_Connection_Fault[Consumer_Num-1] = Faulted, */
          /* (see FRS222 and FRS223) */
          IXSVC_ConnectionFaultFlagSet(u16_svcIdx,
                                       (CSS_t_USINT)(u8_consNum - 1U),
                                       IXSVD_k_CNXN_FAULTED);
          /* Increment fault counter */
          IXSVC_FaultCountInc();

          /* Abort the processing of this packet. Close the connection. */
          o_abortTcooProcessing = CSS_k_TRUE;

          if (IXSVC_ConsumerRemainingCheck(u16_svcIdx))
          {
            /* one consumer failed, there are still other consumers active    */
            /* Update states and notify application (see FRS10 and FRS12-1).  */
            IXSVC_StateMachine(u16_svcIdx, u8_consNum,
                               IXSVC_k_VE_CONSUMER_FAULTED);

            /* start quarantine timer to prevent re-use of consumer number  */
            IXSVC_QuarConsStart(u16_svcIdx, u8_consNum);
          }
          else
          {
            /* This was the last consumer of the connection which now         */
            /* failed. Update states and notify application (see FRS10 and    */
            /* FRS12-1).                                                      */
            IXSVC_StateMachine(u16_svcIdx, u8_consNum,
                               IXSVC_k_VE_ALL_CONS_FAULTED);
          }
        #else
          {
            IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_24,
                            IXSVC_InstIdFromIdxGet(u16_svcIdx),
                            IXSER_k_A_NOT_USED);
          }/* CCT_SKIP */ /* unreachable code - defensive programming */
        #endif
        }
        /* ENDIF */
      }
      else
      {
        /* everything correct (all checks passed) */
      }
      /* ENDIF */
    }
    else
    {
      /* processing already aborted */
    }

    if (!o_abortTcooProcessing)
    {
      /* temporary variable for timing calculations */
      CSS_t_UINT u16_Worst_Case_Consumer_Time_Correction_Value_128us;

      /*
      // at this point we know we have a good, non-redundant time coordination
      // packet
      // Capture the received CONSUMER time
      Consumer_Time_Value[Consumer_Num-1] =
        Time_Coordination_Section.Consumer_Time_Value;
      */
      aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].u16_Consumer_Time_Value_128us
        = as_TcooMsg[u16_svcIdx].u16_consTimeVal_128us;

      /*
      // Determine the worst case Consumer_Time_Correction_Value based on the
      // received Time_Coordination section
      Worst_Case_Consumer_Time_Correction_Value[Consumer_Num-1] =
        Time_Coordination_Section.Consumer_Time_Value
        - Producer_Rcved_Time_Value[Consumer_Num-1]
        - Connection_Correction_Constant[Consumer_Num-1],
      */
      /* (see FRS18) */
      /* Worst_Case_Consumer_Time_Correction_Value is just a temporary        */
      /* variable and thus it is not necessary to store it per consumer       */
      u16_Worst_Case_Consumer_Time_Correction_Value_128us = (CSS_t_UINT)
       ((as_TcooMsg[u16_svcIdx].u16_consTimeVal_128us
         - aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
             u16_Producer_Rcved_Time_Value_128us)
         - aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].
             u16_Connection_Correction_Constant_128us);

      /* The sample code contains further things to be done in this function.
         For reason of data encapsulation and for reducing the complexity of
         this function this has been separated and put into an own function. */
      IXSVC_ProdTcooMsgReception(u16_svcIdx, u8_consNum,
        as_SvcLocals[u16_svcIdx].u16_Time_Drift_Per_Ping_Interval_128us,
        u16_Worst_Case_Consumer_Time_Correction_Value_128us);

      /*
      //////////////////////////////////////////////////////////////////
      // end producer time coordination information reception processing
      //////////////////////////////////////////////////////////////////
      */
    }
    else
    {
      /* processing already aborted */
    }
  }
  else
  {
    /* Ping Response bit not set or consumer not open */
    /* ignore this message */
    IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_TCOO_PR_NOTSET,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u8_consNum);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

/* This function is strictly based on example code presented in Volume 5. Thus
   the structure should not be altered. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : TcooRxCopy
**
** Description : This function determines the message format of a Safety
**               Validator Client Instance and copies the received byte stream
**               to the Safety Validator client data structure.
**
** Parameters  : u16_svcIdx (IN) - Instance Index of the Safety Validator Client
**                                 (not checked, checked in ValidatorRxProc())
**               u16_len (IN)    - length of the received message
**                                 (checked, valid range: CSOS_k_IO_MSGLEN_TCOO)
**               pb_data (IN)    - pointer to byte stream that contains the
**                                 received message
**                                 (not checked, called with pointer to received
**                                 HALC message plus offset)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
static CSS_t_WORD TcooRxCopy(CSS_t_UINT u16_svcIdx,
                             CSS_t_UINT u16_len,
                             const CSS_t_BYTE *pb_data)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* if Base Format */
  if (IXSVD_IsBaseFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /* Base Format */

      /* read the data from the received byte stream to the message structure */
      u16_cpIdx = IXSFA_BfTcooMsgRead(pb_data, &as_TcooMsg[u16_svcIdx]);
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_11,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* Extended Format */
      /* read the data from the received byte stream to the message structure */
      u16_cpIdx = IXSFA_EfTcooMsgRead(pb_data, &as_TcooMsg[u16_svcIdx]);
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_12,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
  }

  /* if the length of the received message doesn't match to the number of     */
  /* bytes we have copied from it                                             */
  if (u16_cpIdx != u16_len)
  {
    /* error */
    w_retVal = (IXSVC_k_NFSE_RXI_MSG_LEN_ERR_C);
    IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    IXSER_k_A_NOT_USED);
  }
  else /* else: length check successful */
  {
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : TcooCrcIntegrityCheck
**
** Description : This function checks the Time Coordination message of a Safety
**               Validator Client Instance and returns CSS_k_FALSE in case of
**               an error. The caller of this function does not have to care
**               whether the Validator Instance refers to Base or Extended
**               Format.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in ValidatorRxProc())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in IXSVC_TcooRxProc())
**
** Returnvalue : CSS_k_TRUE  - Time Coordination message is ok
**               CSS_k_FALSE - error in Time Coordination message (CRC error)
**
*******************************************************************************/
static CSS_t_BOOL TcooCrcIntegrityCheck(CSS_t_UINT u16_svcIdx,
                                        CSS_t_USINT u8_consNum)
{
  /* function's return value */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* use either base or extended format (see FRS370) */
  /* if Extended Format */
  if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UDINT u32_expCrc = 0U;

    /* Calculate expected CRC using CID as seed (see FRS331) */
    u32_expCrc = IXCRC_EfTcooMsgCrcCalc(&as_TcooMsg[u16_svcIdx],
                   aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].u32_cidCrcS5);

    /* if received CRC doesn't match calculated CRC */
    if (as_TcooMsg[u16_svcIdx].u32_crcS5 != u32_expCrc)
    {
      /* CRC mismatch */
      o_retVal = CSS_k_FALSE;
      IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_EXT_TCOO_CRC,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      (CSS_t_UDINT)u8_consNum);
    }
    else /* else: CRC is correct */
    {
      o_retVal = CSS_k_TRUE;
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_25,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }
  else /* else: must be Base Format */
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    CSS_t_UINT u16_expCrc = 0U;

    /* Calculate expected CRC using CID as seed (see FRS331) */
    u16_expCrc = IXCRC_BfTcooMsgCrcCalc(&as_TcooMsg[u16_svcIdx],
                   aas_SvcPcLocals[u16_svcIdx][u8_consNum - 1U].u16_cidCrcS3);

    /* if received CRC doesn't match calculated CRC */
    if (as_TcooMsg[u16_svcIdx].u16_crcS3 != u16_expCrc)
    {
      /* CRC mismatch */
      o_retVal = CSS_k_FALSE;
      IXSVC_ErrorClbk(IXSVC_k_NFSE_RXI_BASE_TCOO_CRC,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      (CSS_t_UDINT)u8_consNum);
    }
    else /* else: CRC is correct */
    {
      o_retVal = CSS_k_TRUE;
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_26,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


/*******************************************************************************
**
** Function    : TcooPingCntReplyVsPingCntCheck
**
** Description : This function checks if the Ping Count Reply in the Ack_Byte of
**               a Time Coordination message matches the Ping Count in the Mode
**               Byte of the previously sent data message.
**
** Parameters  : b_ackByte (IN)  - received Ack_Byte
**                                 (not checked, only least significant bits are
**                                 relevant, any value allowed)
**               b_modeByte (IN) - previously sent Mode Byte
**                                 (not checked, only least significant bits are
**                                 relevant, any value allowed)
**
** Returnvalue : CSS_k_TRUE      - ping count matches
**               CSS_k_FALSE     - mismatch of ping count versus ping count
**                                 reply
**
*******************************************************************************/
static CSS_t_BOOL TcooPingCntReplyVsPingCntCheck(CSS_t_BYTE b_ackByte,
                                                 CSS_t_BYTE b_modeByte)
{
  /* functions return value */
  CSS_t_BOOL  o_retVal = CSS_k_FALSE;
  /* ping count reply from Ack_Byte */
  CSS_t_USINT b_pcr    = (CSS_t_USINT)
                           IXUTL_BIT_GET_U8(b_ackByte,
                                            IXSVD_k_AB_PING_COUNT_REPLY);
  /* ping count from Mode Byte */
  CSS_t_USINT b_pc     = (CSS_t_USINT)IXUTL_BIT_GET_U8(b_modeByte,
                                                       IXSVD_k_MB_PING_COUNT);
  /* ping count less one */
  CSS_t_USINT b_pcL1   = 0U;

  /* calculate ping count less one, considering that ping count is an */
  /* unsigned 2 bit integer value */
  if (b_pc > 0U)
  {
    b_pcL1 = (CSS_t_USINT)(b_pc - 1U);
  }
  else
  {
    b_pcL1 = 0x03U;  /* wrap around */
  }

  /*
  // Ensure that the Time Coordination message returned
  // with the same ping interval or the next ping interval.
  // The next ping interval is allowed for the case that a
  // multi-cast consumer connects to an existing producer.
  // If the new consumer receives its first message near the end
  // of the ping interval, the time coordination may arrive back
  // at the producer during the next ping interval.
     ((Ack_Byte.Ping_Count_Reply != Ping_Count) AND
     (Ack_Byte.Ping_Count_Reply != Ping_Count-1)) OR
  */
  /* (see FRS333) */
  if ( (b_pcr != b_pc) && (b_pcr != b_pcL1) )
  {
    o_retVal = CSS_k_FALSE;
  }
  else
  {
    o_retVal = CSS_k_TRUE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


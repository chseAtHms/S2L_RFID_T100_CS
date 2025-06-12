/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSrecv.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains functions for receiving of Safety Validator
**             Server messages and also the Consumer Activity Monitor which
**             supervises if messages are received in time.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_RecvLocalsClear
**             IXSVS_ConsDataSizeSet
**             IXSVS_ActMonIntervalSet
**             IXSVS_ConsInstActivityMonitor
**             IXSVS_RxProc
**             IXSVS_RecvSoftErrByteGet
**             IXSVS_RecvSoftErrVarGet
**
**             ValidatorServerRxCopy
**             RxProcLt
**             NextRxTimeCalc
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"
#include "IXUTL.h"

#include "IXSSS.h"
#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  #include "IXSPI.h"
#endif
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  #include "IXSAIapi.h"
  #include "IXSAI.h"
#endif
#include "IXSVD.h"
#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVS.h"
#include "IXSVSint.h"
#include "IXSVSerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** k_INITIAL_ACT_MON_INTERVAL_128US: This constant determines the first
    consumer activity monitoring interval after setting up the connection.
    For practical reasons (arrival of first packet takes longer due to transmit
    time of Forward_Open response, response processing and connection setup) it
    was specified that the first interval is longer and is a constant
    independently of EPI and Timeout Multiplier.
*/
/*lint -esym(750, k_INITIAL_ACT_MON_INTERVAL_128US)
                                                  not referenced in every cfg */
                                         /* 10 sec in ticks */
#define k_INITIAL_ACT_MON_INTERVAL_128US (10000000U / CSS_k_TICK_BASE_US)


/** t_SVS_LOCALS_RECV:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* time interval for Consumer Activity Monitoring */
  CSS_t_UDINT       u32_actMonInt_128us;
  /* scheduled system time before which the next reception has to occur */
  CSS_t_UDINT       u32_nextRxTime_128us;

  /* Consumed Connection Size: number of payload data to be received */
  CSS_t_USINT       u8_consDataSize;
} t_SVS_LOCALS_RECV;

/* array of structs to store the local variables for each instance */
static t_SVS_LOCALS_RECV as_SvsLocals[CSOS_cfg_NUM_OF_SV_SERVERS];


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  /* define IDs of the variables depending on which variables are supported */
  enum
  {
    k_SOFT_ERR_OPT_VAR_AMI,
    k_SOFT_ERR_OPT_VAR_NRT,
    k_SOFT_ERR_VAR_CDS
  };
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD ValidatorServerRxCopy(CSS_t_UINT u16_svsIdx,
                                        CSS_t_UINT u16_recLen,
                                        const CSS_t_BYTE *pb_data);
static void RxProcLt(CSS_t_UINT u16_svsIdx);
static void NextRxTimeCalc(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVS_RecvLocalsClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Safety Validator structure */
  CSS_MEMSET(&as_SvsLocals[u16_svsIdx], 0, sizeof(as_SvsLocals[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                           CSS_t_USINT u8_consDataSize)
{
  as_SvsLocals[u16_svsIdx].u8_consDataSize = u8_consDataSize;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                             CSS_t_UINT u16_NetTimeExpMult_128us)
{
  /* The Consumer Activity Monitor supervises the Network Time Expectation */
  as_SvsLocals[u16_svsIdx].u32_actMonInt_128us
    = (CSS_t_UDINT)u16_NetTimeExpMult_128us;

  /* Calculate the first Consumer Activity Monitor Timeout */
  /* For practical reasons (arrival of first packet takes longer due to transmit
     time of Forward_Open response, response processing and connection setup) it
     was specified that the first interval is longer and is a constant
     independently of EPI and Timeout Multiplier. */
  as_SvsLocals[u16_svsIdx].u32_nextRxTime_128us
    = IXUTL_GetSysTime() + k_INITIAL_ACT_MON_INTERVAL_128US;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_ConsInstActivityMonitor
**
** Description : This API function handles the Consumer Activity Monitor of one
**               Safety Validator Server Instance. Additionally it transmits
**               the Time Coordination Messages (if a transmission is currently
**               pending).
**               This function can be called with any Safety Validator Instance
**               ID (see Demo). In case the passed instance is a Client Instance
**               or doesn't exist it will just return (and not generate an
**               error).
**
**               **Attention**:
**               This function checks if a packet has been received within
**               Network_Time_Expectation.
**               This function must be called cyclically to perform Consumer
**               Activity Monitoring. The calling interval must be fast enough
**               to meet the System Reaction Time. Worst case timeout detection
**               occurs one calling interval later.
**
** Parameters  : u16_svsInstId (IN) - Instance ID of the Safety Validator Server
**                                    (checked, valid range:
**                                    1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                    CSOS_cfg_NUM_OF_SV_SERVERS) and
**                                    instance must be a Safety Validator
**                                    Server.
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_ConsInstActivityMonitor(CSS_t_UINT u16_svsInstId)
{
  /* Safety Validator index */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_svsInstId);

  /* if CIP Safety Stack is in an improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    IXSVS_ErrorClbk(IXSVS_k_FSE_AIS_STATE_ERR_CAM,
                    u16_svsInstId, IXSER_k_A_NOT_USED);
  }
  else /* else: stack state is ok */
  {
    /* if an invalid safety validator instance was passed */
    if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
    {
      /* this is not an error, but this function won't do anything */
    }
    /* else: if this instance is not a Server */
    else if (IXSVD_InstIsServer(u16_svsInstId) == CSS_k_FALSE)
    {
      /* This instance is a Safety Validator Client. */
      /* So this function is not applicable. */
    }
    else /* else: it is a valid Safety Validator Server */
    {
      /* check if initialization is in progress and if it was completed within
         the time limit */
      IXSVS_StateInitLimitCheck(u16_svsIdx);

      /* if the Safety Validator Server is in states "Init" or "Established" */
      if (IXSVS_StateIsInitOrEstablished(u16_svsIdx))
      {
        /* if we did not receive a message in the expected time (see FRS3) */
        if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(),
              as_SvsLocals[u16_svsIdx].u32_nextRxTime_128us))
        {
          /* Consumer Activity Monitor detected a failure (timeout) */
          /* (see FRS11, FRS79 and FRS87-1). */
          IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_ACT_MON, u16_svsInstId,
                          as_SvsLocals[u16_svsIdx].u32_nextRxTime_128us);

          /* Increment the Fault Counter */
          IXSVS_StateFaultCountInc();

          /* Set the fault flag to make sure the Connection Status of this
             consuming safety connection instance is reflected correctly */
          IXSVS_ConFaultFlagSet(u16_svsIdx, IXSVD_k_CNXN_FAULTED);

          /* Update states and notify application (see FRS12-1). */
          /* (see Req.2.5.2.2-2). */
          IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_FAULTED);
        }
        else /* else: message had been received in expected time */
        {
          /* Last safety data had been received within Network Time           */
          /* Expectation (calculated from producer time stamp) (see FRS130)   */
        }

        /* Send a Time Coordination Message if it is time for that */
        IXSVS_TcooTimeToSend(u16_svsIdx);
      }
      else
      {
        /* This instance cannot be processed as it is currently not in use or */
        /* is faulted.                                                        */
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                  const CSS_t_BYTE *pb_data)
{
  /* return value of called functions */
  CSS_t_WORD w_errCode = (IXSVS_k_FSE_INC_PRG_FLOW);

  /* if the passed index is invalid */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    IXSVS_ErrorClbk(IXSVS_k_FSE_INC_INST_SRX_NUM,
                    IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_svsIdx);
    /* return value already set */
  }
  else /* else: u16_svsIdx is valid */
  {
    /* if the Safety Validator Server is in states "Init" or "Established" */
    if (IXSVS_StateIsInitOrEstablished(u16_svsIdx))
    {
      /* copy the received data into the message structures */
      w_errCode = ValidatorServerRxCopy(u16_svsIdx, u16_len, pb_data);

      /* if previous function returned an error */
      if (w_errCode != CSS_k_OK)
      {
        /* error while copying received message */
        /* return value already set */
      }
      else /* else: ok */
      {
        /*
        ** Safety Validator Server - Link Triggered
        */
        RxProcLt(u16_svsIdx);
      }
    }
    else
    {
      /* This instance cannot be processed as it is currently not in use or   */
      /* is faulted.                                                          */
      /* return value already set */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVS_RecvSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_SvsLocals))
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvsLocals)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_SvsLocals);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVS_RecvSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_SERVERS)
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
      case k_SOFT_ERR_OPT_VAR_AMI:
      {
        CSS_H2N_CPY32(pb_var, &as_SvsLocals[au32_cnt[0U]].u32_actMonInt_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

      case k_SOFT_ERR_OPT_VAR_NRT:
      {
        CSS_H2N_CPY32(pb_var, &as_SvsLocals[au32_cnt[0U]].u32_nextRxTime_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

      case k_SOFT_ERR_VAR_CDS:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].u8_consDataSize);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

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
** Function    : ValidatorServerRxCopy
**
** Description : This function determines the message format of a Safety
**               Validator Server Instance and copies the received byte stream
**               to the Safety Validator Server data structure.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server Instance
**                                 (not checked, checked in IXSVS_RxProc())
**               u16_recLen (IN) - length of the received message
**                                 (checked, must be equal to return value of
**                                 IXSVS_DataRxCopy() and IXSVS_TcorrRxCopy())
**               pb_data (IN)    - pointer to byte stream that contains the
**                                 received message
**                                 (not checked, called with pointer to received
**                                 HALC message plus offset)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
static CSS_t_WORD ValidatorServerRxCopy(CSS_t_UINT u16_svsIdx,
                                        CSS_t_UINT u16_recLen,
                                        const CSS_t_BYTE *pb_data)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVS_k_FSE_INC_PRG_FLOW);
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;
  /* message format byte of the addressed instance */
  CSS_t_BYTE const b_msgFormat = IXSVS_InitInfoMsgFrmtGet(u16_svsIdx);

  /* read the data from the received byte stream to the message structure */
  u16_cpIdx = IXSVS_DataRxCopy(u16_svsIdx, pb_data, b_msgFormat,
                               as_SvsLocals[u16_svsIdx].u8_consDataSize);

  /* if is Multi-cast */
  if (IXSVD_IsMultiCast(b_msgFormat))
  {
    /* for Multi-Cast messages we must also copy the Time Correction Msg */
    u16_cpIdx = (CSS_t_UINT)(u16_cpIdx
                           + IXSVS_TcorrRxCopy(u16_svsIdx, &pb_data[u16_cpIdx],
                                               b_msgFormat));
  }
  else /* else: Single-cast */
  {
    /* has not Time Correction Msg */
  }

  /* if the length of the received message doesn't match the number of bytes  */
  /* we have copied from it */
  if (u16_cpIdx != u16_recLen)
  {
    /* inadvertently received standard messages might not be in correct       */
    /* format (see FRS8-5)                                                    */

    /* error: length mismatch */
    w_retVal = (IXSVS_k_NFSE_RXI_MSG_LEN_ERR_S);
    IXSVS_ErrorClbk(w_retVal, IXSVS_InstIdFromIdxGet(u16_svsIdx),
                    IXSER_k_A_NOT_USED);
  }
  else /* else: length ok */
  {
    /* length check successful */
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : RxProcLt
**
** Description : This function processes received Safety I/O messages that are
**               directed to Safety Validator Server Instances in case of
**               Link Triggered Safety Validator Servers.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void RxProcLt(CSS_t_UINT u16_svsIdx)
{
  /* status of the connection after reception of the message */
  CSS_t_USINT u8_cnxnStatus = CSS_k_CNXN_STATUS_FAULTED;

  /* if is Multi-cast */
  if (IXSVD_IsMultiCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
    /* a Time Correction Section follows the Data Msg */
    /* call Safety Validator Server function to process the message     */
    /* (see FRS129) (perform all aspects of the safety protocol on each */
    /* Time_Correction section received)                                */
    IXSVS_TimeCorrMsgReceptionLt(u16_svsIdx);
  }
  else /* else: must be Single-cast */
  {
    /* no Time Correction Section */
  }

  /* Check the state of the Safety Validator. This check has already been done
     by the calling function, but the state may already have changed due to
     time correction message reception */
  /* if state is not "init" or "established" */
  if (!IXSVS_StateIsInitOrEstablished(u16_svsIdx))
  {
    /* ignore the data message */
    /* return value already set */
  }
  else
  {
    /* (first stage)                                                      */
    /*
    // execute a function that checks for ping count changes and
    // determines if it is time to produce a time coordination message
    ping_count_check_in_consumer_function(),
    */
    /* first check to be executed (see FRS9-1) */
    IXSVS_TcooPingCountCheck(u16_svsIdx);

    /* call Safety Validator Server functions to consume the message      */
    /* (see FRS128) (perform all aspects of the safety protocol on each   */
    /* message received)                                                  */
    /* if the received message was discarded (but connection still open) */
    if (IXSVS_CombinedDataConsumption(u16_svsIdx))
    {
      /* Don't update the data in the ProcessImage / AssemblyInterface */
      /* Discarding message but connection stays active - can only happen in
         case of Extended Format. */
      /* Application has already been informed by the consumption logic by means
         of an NFSE error */
      /* return value already set */
    }
    else /* else: message has been processed or connection was closed and
                  message discarded (applies both for Base Format and Extended
                  Format) */
    {
      /* Run mode byte indicates if data are valid */
      CSS_t_BYTE b_runMode;
      /* length of the received data */
      CSS_t_USINT u8_len = 0U;
      /* pointer to the received safety I/O data */
      const CSS_t_BYTE *pb_svBuf = CSS_k_NULL;

      /* get the status after processing the received data */
      u8_cnxnStatus = IXSVS_ConsCnxnStatusGet(u16_svsIdx);

    #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
      if (IXSVS_InstIdFromIdxGet(u16_svsIdx) <= CSS_cfg_NUM_OF_CCO_INSTS)
      {
        /* Obtain a pointer to the Safety Validator's data buffer   */
        pb_svBuf = IXSVS_DataPtrGet(u16_svsIdx, &u8_len);

        /* if the Safety Validator Server is in running state */
        if (u8_cnxnStatus == CSS_k_CNXN_STATUS_CONS_RUN)
        {
          b_runMode = CSS_k_RIF_RUN;
        }
        else /* else: is idle */
        {
          b_runMode = CSS_k_RIF_IDLE;
        }

        /* Copy the received data to the Consumed Data Process Image (in    */
        /* case the application has not yet obtained the previous data this */
        /* will overwrite the recent data (see FRS88).                      */
        IXSPI_ConsImageDataSet(IXSVS_InstIdFromIdxGet(u16_svsIdx),
                               IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                               pb_svBuf, (CSS_t_UINT)u8_len, b_runMode);
      }
      else
    #endif  /* (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE) */
    #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
      {
        /* if the Safety Validator Server is in running state */
        if (u8_cnxnStatus == CSS_k_CNXN_STATUS_CONS_RUN)
        {

          /* indicate that data is valid */
          b_runMode = CSS_k_RIF_RUN;
          /* Obtain a pointer to the Safety Validator's data buffer   */
          /* to be able to copy the received data to the assembly     */
          /* object (in case the application has not yet obtained the */
          /* previous data this will overwrite the recent data        */
          /* (see FRS88).                                             */
          pb_svBuf = IXSVS_DataPtrGet(u16_svsIdx, &u8_len);

        }
        else /* else: is idle */
        {
          /* connection status is != active */
          b_runMode = CSS_k_RIF_IDLE;
          /* u8_len and pb_svBuf already initialized */
        }

        /* Update the associated assembly instance */
        IXSAI_AsmOutputDataSet(IXSVS_InitInfoCnxnPointGet(u16_svsIdx),
                               pb_svBuf,
                               (CSS_t_UINT)u8_len,
                               b_runMode);
      }
    #else
      {
        /* error: invalid instance! */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SD_RECV_LT,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
        /* return value already set */
      }/* CCT_SKIP */ /* unreachable code - defensive programming */
    #endif  /* (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) */

      /* Data consumption process is finished here. Thus we will start  */
      /* right now the timeout for the next consumption (see FRS78).    */
      /* In case the connection has been closed still starting the Activity
         Monitor doesn't do any harm because IXSVS_ConsInstActivityMonitor()
         checks the connection state before doing any action. */
      NextRxTimeCalc(u16_svsIdx);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : NextRxTimeCalc
**
** Description : This function calculates and saves the point of time when the
**               next data message must have been received.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc(),
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : -
**
*******************************************************************************/
static void NextRxTimeCalc(CSS_t_UINT u16_svsIdx)
{
  /* Get the current Data_Age to be able to calculate the next timeout */
  CSS_t_UINT u16_dataAge = IXSVS_DataAgeGet(u16_svsIdx);

  /* Consumer Activity Monitor will expire (u32_actMonInt_128us - Data_Age)
     ticks from now (see Req.5.3-18) */
  as_SvsLocals[u16_svsIdx].u32_nextRxTime_128us = IXUTL_GetSysTime()
    + (as_SvsLocals[u16_svsIdx].u32_actMonInt_128us - u16_dataAge);

  /* Underflow consideration: In the case that the calculated time is in the
     past (i.e. u16_dataAge > u32_actMonInt_128us) the next check of the
     activity monitor will immediately fail. */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}

#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


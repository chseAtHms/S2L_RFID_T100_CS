/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCinit.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the functions to initialize the SVC unit and
**             to start the data production of a Safety Validator Client
**             instance.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_Init
**             IXSVC_PingIntEpiMultGet
**             IXSVC_NetTimeExpMultGet
**             IXSVC_TargClientInit
**             IXSVC_OrigClientInit
**             IXSVC_InitSoftErrByteGet
**             IXSVC_InitSoftErrVarGet
**
**             TargClientConsumerInit
**             ConsIdxAlloc
**             TimeoutMultiplierPiCalc
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
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "HALCSapi.h"
#endif

#include "IXSSS.h"
#include "IXSERapi.h"

#include "IXUTL.h"
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSCF.h"

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
    #include "IXSPIapi.h"
  #endif
#endif

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

/* data type for storing values needed for SVO attributes */
typedef struct
{
  CSS_t_UINT u16_piem;   /* Ping Interval EPI Multiplier */
  /* Network Time Expectation Multiplier (per Consumer)
  Legal range: 0..45313 (= 0..5.8sec) */
  CSS_t_UINT au16_ntem_128us[CSOS_k_MAX_CONSUMER_NUM_MCAST];
} t_SVC_ATTR_DATA;

/* array of structures storing data for SVO attributes */
static t_SVC_ATTR_DATA as_SvoAttrData[CSOS_cfg_NUM_OF_SV_CLIENTS];

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  /* Consumer Index of Singlecast Connections is always zero */
  #define k_CONS_IDX_SCAST                0U
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
static CSS_t_WORD TargClientConsumerInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                         CSS_t_USINT u8_payloadSize,
                                         CSS_t_UINT u16_svcIdx,
                                         CSS_t_BOOL o_reInit,
                                         const IXSVC_t_INIT_INFO *ps_initInfo,
                                         CSS_t_SOPEN_RESP *ps_sOpenResp);
static CSS_t_WORD ConsIdxAlloc(CSS_t_UINT u16_svcIdx,
                               CSS_t_USINT u8_maxConsNum,
                               CSS_t_BYTE b_msgFormat,
                               CSS_t_USINT *pu8_consIdx,
                               CSS_t_MR_RESP_STATUS *ps_status);
#endif
static CSS_t_USINT TimeoutMultiplierPiCalc(CSS_t_UINT u16_svsIdx,
                                           CSS_t_USINT u8_timeoutMult,
                                           CSS_t_BYTE b_msgFormat);


/*******************************************************************************
**    global functions
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
void IXSVC_Init(void)
{
  CSS_t_UINT u16_svcIdx = 0U;

  /* clear the locals of this unit */
  CSS_MEMSET(as_SvoAttrData, 0, sizeof(as_SvoAttrData));

  /* make sure all instances are unallocated */
  IXSVC_InstInitAll();

  /* make sure all Validator instances are unallocated and in Idle state */
  for (u16_svcIdx = 0U; u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_svcIdx++)
  {
    IXSVC_InstInit(u16_svcIdx);
    IXSVC_StateInit(u16_svcIdx);
  }

  /* initialize the fault counter of the Safety Validator Clients */
  IXSVC_FaultCountReset();

  /* initialize the lookup table for the index <-> instance ID relations */
  IXSVC_InstIdInit();

  /* Initialize the Error module (Last Error Codes) */
  IXSVC_ErrorInit();

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_UINT IXSVC_PingIntEpiMultGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvoAttrData[u16_svcIdx].u16_piem);
}


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
                                   CSS_t_USINT u8_consIdx)
{
  CSS_t_UINT w_retVal = IXSVC_k_NTEM_INVALID_VALUE;

  /* if passed consumer index is invalid */
  if (u8_consIdx >= CSOS_k_MAX_CONSUMER_NUM_MCAST)
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_NTEM_GET_INV_CI,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u8_consIdx);
  }
  else /* else: consumer index is ok */
  {
    w_retVal = as_SvoAttrData[u16_svcIdx].au16_ntem_128us[u8_consIdx];
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
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
CSS_t_WORD IXSVC_TargClientInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                CSS_t_USINT u8_payloadSize,
                                CSS_t_UINT u16_svInst,
                                CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* index of the Safety Validator Client */
  CSS_t_UINT u16_svcIdx = IXSVD_SvIdxFromInstGet(u16_svInst);
  /* local struct for collecting initialization data */
  IXSVC_t_INIT_INFO s_initInfo;

  /* completely erase the init structure */
  CSS_MEMSET(&s_initInfo, 0, sizeof(s_initInfo));

  /* if an invalid safety validator instance was passed */
  if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    w_retVal = (IXSVC_k_FSE_INC_INST_INV_TCI);
    IXSVC_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);
  }
  else /* else: Safety Validator Instance exists */
  {
    /* Flag indicating if this instance is already existing and only a new    */
    /* consumer is now attached or if it is a completely new opened           */
    /* connection                                                             */
    CSS_t_BOOL o_reInit;
    /* get the state Safety Validator */
    const CSS_t_USINT u8_svcState = IXSVC_StateGet(u16_svcIdx);

    /* Either this is an existing multicast connection and we just want to add
       another consumer. Then reading back MsgFrmt will return a valid value
       from the existing connection.
       Or this is a brand new connection that has just been allocated and
       cleared. In this case all variables of this instance are zero. Thus also
       b_msgFormat is zero and the check for Multicast in the following
       statement will then definitely return FALSE. This will then result in a
       complete initialization (o_reInit = FALSE). */
    s_initInfo.b_msgFormat = IXSVC_InitInfoMsgFrmtGet(u16_svcIdx);
    /* the same for the Max_Consumer_Number */
    s_initInfo.u8_Max_Consumer_Number = IXSVC_InitInfoMaxConsNumGet(u16_svcIdx);

    /* if multi-cast and connection is not idle */
    if (    IXSVD_IsMultiCast(s_initInfo.b_msgFormat)
         && (u8_svcState != CSS_k_SV_STATE_IDLE)
       )
    {
      o_reInit = CSS_k_TRUE;
    }
    else /* else: single-cast or connection is still idle */
    {
      o_reInit = CSS_k_FALSE;
    }

    /* If the connection already exists we don't have to initialize variables */
    /* that are already in use.                                               */
    if (!o_reInit)
    {
      /* if Multicast */
      if (CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
      {
        s_initInfo.b_msgFormat = IXSVD_k_MSG_FORMAT_MCAST;
      }
      else /* else: Singlecast (anything else was checked before) */
      {
        /* make sure the variable has a defined value */
        s_initInfo.b_msgFormat = 0x00U;
      }

      /* if Extended Format */
      if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
      {
        s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_EXTENDED;
      }
      else /* else: must be Base Format (anything else was checked before) */
      {
        /* bit is already cleared */
      }

      /* if Long Format */
      if (u8_payloadSize > CSS_k_SHORT_FORMAT_MAX_LENGTH)
      {
      #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
        {
          s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_LONG;
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_35, u16_svInst,
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }
      else /* else: can only be short format */
      {
      #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
        {
          /* bit is already cleared */
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_36, u16_svInst,
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }

      /* IXSVD_k_MSG_FORMAT_ORIGINATOR in b_msgFormat is not set because we
         are Target */

      /* Max_Consumer_Number from SafetyOpen Request */
      /* (needed for consumer allocation) */
      s_initInfo.u8_Max_Consumer_Number = ps_sOpenPar->s_nsd.u8_maxConsNum;

      /* completely erase the storage of Network Time Expectation Multipliers */
      CSS_MEMSET(as_SvoAttrData[u16_svcIdx].au16_ntem_128us, 0,
                 sizeof(as_SvoAttrData[u16_svcIdx].au16_ntem_128us));
    }
    else /* else: re-initialization of multicast consumer */
    {
      /* nothing to do */
    }

    /* set connection type that is reported to the CSAL */
    /* if multicast */
    if (IXSVD_IsMultiCast(s_initInfo.b_msgFormat))
    {
      ps_sOpenResp->s_appReply.b_cnxnType = CSOS_k_CNXN_PROD_MULTI;
    }
    else /* else: singlecast */
    {
      ps_sOpenResp->s_appReply.b_cnxnType = CSOS_k_CNXN_PROD_SINGLE;
    }

    /* continue initialization in sub-function */
    w_retVal = TargClientConsumerInit(ps_sOpenPar, u8_payloadSize,
                                      u16_svcIdx, o_reInit, &s_initInfo,
                                      ps_sOpenResp);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
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
CSS_t_WORD IXSVC_OrigClientInit(
                           const CSS_t_SOPEN_RESP_RX *ps_openRespRx,
                           const CSS_t_CCO_SOPEN_INIT_VAL *ps_ccoSopenInitVal,
                           CSS_t_UINT u16_svInst)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* index of the Safety Validator Client */
  CSS_t_UINT u16_svcIdx = IXSVD_SvIdxFromInstGet(u16_svInst);
  /* local struct for collecting initialization data */
  IXSVC_t_INIT_INFO s_initInfo;
  /* local struct for collection per consumer data */
  IXSVC_t_INST_INFO_PER_CONS s_iiPerCons;

  /* completely erase the init structures */
  CSS_MEMSET(&s_initInfo, 0, sizeof(s_initInfo));
  CSS_MEMSET(&s_iiPerCons, 0, sizeof(s_iiPerCons));

  /* if the passed Safety Validator Instance doesn't exist */
  if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    w_retVal = (IXSVC_k_FSE_INC_INST_INV_OCI);
    IXSVC_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);
  }
  else /* else: Safety Validator Instance exists */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* temporary variables for passing Initial TimeStamp and Initial */
    /* Rollover Value to Cold Start Logic                            */
    CSS_t_UINT u16_initialTs = 0U;
    CSS_t_UINT u16_initialRv = 0U;
  #endif

    /* we are originator of this connection */
    s_initInfo.b_msgFormat = IXSVD_k_MSG_FORMAT_ORIGINATOR;
    /* Can only be Single-Cast on producing Originator */
    /* bit is already cleared */

    /* if Extended Format */
    if (ps_ccoSopenInitVal->u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
    {
      s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_EXTENDED;
    }
    else /* else: must be Base Format (anything else was checked before) */
    {
      /* bit is already cleared */
    }

    /* if Long Format */
    if (ps_ccoSopenInitVal->u8_dataLen > CSS_k_SHORT_FORMAT_MAX_LENGTH)
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_LONG;
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_33, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: can only be short format */
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* bit is already cleared */
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_34, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }

    /* Max_Consumer_Number (for Originator only Single-Cast is possible) */
    /* (see FRS241) */
    s_initInfo.u8_Max_Consumer_Number = CSOS_k_MAX_CONSUMER_NUM_SCAST;

    /* Indicate that this consumer structure is in use now (see FRS322 and
       FRS324). */
    s_iiPerCons.o_Consumer_Open = CSS_k_TRUE;

    /* RPIs */
    s_initInfo.s_initCalc.u32_rpiOT_us = ps_openRespRx->u32_apiOT_us;
    s_initInfo.s_initCalc.u32_rpiTO_us = ps_openRespRx->u32_apiTO_us;
    /* expected packet rate for data production is equal to O->T RPI */
    s_initInfo.s_initCalc.u32_EPI_us = s_initInfo.s_initCalc.u32_rpiOT_us;

    /* calculate basic number of ticks between two transmissions */
    s_initInfo.s_initCalc.u32_EPI_128us =
      s_initInfo.s_initCalc.u32_EPI_us / CSS_k_TICK_BASE_US;

    /* calculate deviation due to granularity of RPI in us and 128us ticks */
    s_initInfo.s_initCalc.u32_EpiDeviation_us =
      s_initInfo.s_initCalc.u32_EPI_us -
      (s_initInfo.s_initCalc.u32_EPI_128us * CSS_k_TICK_BASE_US);

    /* set connection size */
    s_initInfo.s_initCalc.u8_prodDataSize = ps_ccoSopenInitVal->u8_dataLen;

    /* in case Process Image is enabled: Byte-Index in process image */
    /* else                          : Connection Point (= Assembly Instance) */
    s_initInfo.u16_ProdCnxnPoint = ps_ccoSopenInitVal->u16_dataOffset;

    /* Ping_Interval_EPI_Multiplier */
    s_initInfo.s_initCalc.u16_Ping_Interval_EPI_Multiplier =
      ps_ccoSopenInitVal->u16_pingIntEpiMult;

    /* Time_Coord_Msg_Min_Multiplier */
    IXSVC_TcooMsgMinMultSet(u16_svcIdx, k_CONS_IDX_SCAST,
                            ps_ccoSopenInitVal->u16_tcooMsgMinMult_128us);

    /* if Extended Format */
    if (IXSVD_IsExtendedFormat(s_initInfo.b_msgFormat))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        /* Max_Fault_Number (typecasting to USINT as this attribute has been
             checked to be <= 255 before) */
        IXSVC_TcooMaxFaultNumberSet(u16_svcIdx, k_CONS_IDX_SCAST, (CSS_t_USINT)
                                    ps_ccoSopenInitVal->u16_MaxFault_Number);

        /* make a temporary copy of received initial Timestamp and Rollover
           val */
        u16_initialTs = ps_openRespRx->u16_initialTs;
        u16_initialRv = ps_openRespRx->u16_initialRv;
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_20,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: Base Format */
    {
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        /* nothing to do for Base Format */
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_32, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }

    /* Timeout_Multiplier */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      s_iiPerCons.s_Timeout_Multiplier.u8_ef =
        ps_ccoSopenInitVal->u8_timeoutMult;
    }
  #endif
    s_iiPerCons.s_Timeout_Multiplier.u8_PI =
      TimeoutMultiplierPiCalc(u16_svcIdx, ps_ccoSopenInitVal->u8_timeoutMult,
                              s_initInfo.b_msgFormat);

    /* Schedule first transmission: immediately */
    IXSVC_ProdNextTxTimeSet(u16_svcIdx, IXUTL_GetSysTime());

    /* Now that all Safety Validator Format Settings are made we can execute  */
    /* the Cold Start Logic.                                                  */
    IXSVC_ProdColdStartLogic(u16_svcIdx,
                             &s_initInfo
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)         /* CCT_NO_PRE_WARNING */
                           ,&u16_initialTs
                           ,&u16_initialRv
  #endif
                          );
    /* execute the Time Coordination specific parts of the Cold Start Logic */
    IXSVC_TcooColdStartLogic(u16_svcIdx,
                             CSOS_k_MAX_CONSUMER_NUM_MCAST,
                             &s_initInfo,
                             &s_iiPerCons);

    /* calculate CRC seeds (see SRS178) */
    {
      /* copy CID from SafetyOpen Response (see SRS152) */
      s_iiPerCons.s_cid.u16_vendId     = ps_openRespRx->u16_trgVendId;
      s_iiPerCons.s_cid.u32_devSerNum  = ps_openRespRx->u32_trgDevSerNum;
      s_iiPerCons.s_cid.u16_cnxnSerNum = ps_openRespRx->u16_trgCnxnSerNum;
      IXSVC_InstInfoPerConsSet(u16_svcIdx, k_CONS_IDX_SCAST, &s_iiPerCons);

      /* Calc CID CRC seed (per consumer) */
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        IXSVC_TcooCidCrcS3Set(u16_svcIdx, k_CONS_IDX_SCAST,
          IXCRC_PidCidCrcS3Calc(&s_iiPerCons.s_cid));
      }
    #endif
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        IXSVC_TcooCidCrcS5Set(u16_svcIdx, k_CONS_IDX_SCAST,
          IXCRC_PidCidCrcS5Calc(&s_iiPerCons.s_cid));
      }
    #endif
      {
        /* PID only a local variable. No need to be stored */
        CSS_t_PIDCID s_pid;

        /* completely erase structure */
        CSS_MEMSET(&s_pid, 0, sizeof(s_pid));

        s_pid.u16_vendId     = IXSSS_VendorIdGet();
        s_pid.u32_devSerNum  = IXSSS_SerialNumGet();
        s_pid.u16_cnxnSerNum = ps_openRespRx->s_cnxnTriad.u16_cnxnSerNum;

        /* Calc PID CRC seeds */
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        {
          s_initInfo.s_initCalc.u8_pidCrcS1 = IXCRC_PidCidCrcS1Calc(&s_pid);
        }
      #endif
        {
          s_initInfo.s_initCalc.u16_pidCrcS3 = IXCRC_PidCidCrcS3Calc(&s_pid);
        }
      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        {
          s_initInfo.s_initCalc.u32_pidCrcS5 = IXCRC_PidCidCrcS5Calc(&s_pid);
        }
      #endif

      /* Extended Format PID/RolloverCount CRC seeds don't have to be calculated
         in advance as they are re-calculated at every message reception
         anyway. */
      }

      /* pass the init data to the init info module */
      IXSVC_InitInfoSet(u16_svcIdx, &s_initInfo);
    }

    {
      /* structure for passing the success response to HALC */
      HALCS_t_MSG  s_halcMsg;
      /* array for passing data of the response to HALC */
      CSS_t_BYTE ab_rspData[  CSOS_k_SIZEOF_UDINT
                             + CSOS_k_SIZEOF_BYTE];

      /* completely erase structure */
      CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

      /* initialize the message that will be sent to CSAL */
      s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_CNXN_OPEN;
      s_halcMsg.u16_len = CSOS_k_SIZEOF_UDINT
                        + CSOS_k_SIZEOF_BYTE;
      s_halcMsg.u32_addInfo = (CSS_t_UDINT)u16_svInst;

      /* link message with local buffer to place the response data in it */
      s_halcMsg.pb_data = ab_rspData;

      /* application reference */
      CSS_H2N_CPY32(s_halcMsg.pb_data, &ps_ccoSopenInitVal->u32_appRef);
      /* Cnxn Type */
      CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_SIZEOF_UDINT),
                   &ps_ccoSopenInitVal->b_cnxnType);

      /* if sending the message to CSAL succeeds */
      if (HALCS_TxDataPut(&s_halcMsg))
      {
        /* success */
        w_retVal = CSS_k_OK;
      }
      else /* else: HALCS returned error */
      {
        /* error while trying to send the message to CSAL */
        w_retVal = (IXSVC_k_NFSE_TXH_HALCS_SEND_OCI);
        IXSVC_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);

        /* Clear the InstInfo (especially the Consumer_Open flag) */
        IXSVC_InstInfoClear(u16_svcIdx);

        /* so then close the connection that we tried to initialize */
        /* if stopping was successful */
        if (IXSVC_InstIdIdxStop(u16_svcIdx))
        {
          IXSVC_InstDealloc(u16_svcIdx);
        }
        else /* else: error */
        {
          /* error already reported to SAPL */
        }
      }
    }

    /* store data needed for SVO attributes */
    as_SvoAttrData[u16_svcIdx].u16_piem =
                               ps_ccoSopenInitVal->u16_pingIntEpiMult;
    as_SvoAttrData[u16_svcIdx].au16_ntem_128us[k_CONS_IDX_SCAST] =
                               ps_ccoSopenInitVal->u16_NetTimeExpMult_128us;

  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
    {
      /* make sure the Run_Idle flag of this connection is by default "Idle" */
      IXSPI_SvClientRunModeSet(u16_svInst, CSS_k_FALSE);
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVC_InitSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_SvoAttrData))
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvoAttrData)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_SvoAttrData);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVC_InitSoftErrVarGet(
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
        CSS_H2N_CPY16(pb_var, &as_SvoAttrData[au32_cnt[0U]].u16_piem);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 1U:
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
          CSS_H2N_CPY16(pb_var, &as_SvoAttrData[au32_cnt[0U]].au16_ntem_128us[au32_cnt[2U]]);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        }
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

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : TargClientConsumerInit
**
** Description : This function is a sub-function of
**               IXSVC_TargClientInit() to initialize the consumer
**               data of a Safety Validator Client instance of the Safety
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
**               u16_svcIdx (IN)     - index of Safety Validator Client
**                                     structure
**                                     (not checked, checked in
**                                     IXSVC_TargClientInit())
**               o_reInit (IN)       - Flag indicating if this instance is
**                                     already existing and only a new consumer
**                                     is now attached or if it is a completely
**                                     new opened connection
**                                     (not checked, any value allowed)
**               ps_initInfo (IN)    - pointer to a struct for collecting
**                                     initialization data
**                                     (not checked, only called with reference
**                                     to struct)
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error while initializing the Safety
**                                     Validator Instance
**
*******************************************************************************/
static CSS_t_WORD TargClientConsumerInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                         CSS_t_USINT u8_payloadSize,
                                         CSS_t_UINT u16_svcIdx,
                                         CSS_t_BOOL o_reInit,
                                         const IXSVC_t_INIT_INFO *ps_initInfo,
                                         CSS_t_SOPEN_RESP *ps_sOpenResp)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* multi-cast consumer number */
  CSS_t_USINT u8_consIdx = 0U;
  /* local struct for collecting initialization data
     (padding bytes set to zero in calling function) */
  IXSVC_t_INIT_INFO s_initInfo = *ps_initInfo;
  /* local struct for collection per consumer data */
  IXSVC_t_INST_INFO_PER_CONS s_iiPerCons;

  /* completely erase the init structure */
  CSS_MEMSET(&s_iiPerCons, 0, sizeof(s_iiPerCons));

  /* In case it is a multi-cast connection we need to find the first empty  */
  /* consumer number (see FRS326).                                          */
  /* In case of single-cast this function will return 0.                    */
  w_retVal = ConsIdxAlloc(u16_svcIdx, s_initInfo.u8_Max_Consumer_Number,
                          s_initInfo.b_msgFormat,
                          &u8_consIdx, &ps_sOpenResp->s_status);

  /* if previous function returned an error */
  if (    (w_retVal != CSS_k_OK)
       || (ps_sOpenResp->s_status.u8_gen != CSOS_k_CGSC_SUCCESS)
     )
  {
    /* Maximum number of consumers already reached. */
  }
  else /* else: successfully allocated a consumer index */
  {
    /* Indicate that this consumer structure is in use now (see FRS322 and
       FRS324). */
    s_iiPerCons.o_Consumer_Open = CSS_k_TRUE;

    /* if the safety connection already exists then most of the variables   */
    /* are already initialized                                              */
    if (o_reInit)
    {
      s_initInfo.b_msgFormat = IXSVC_InitInfoMsgFrmtGet(u16_svcIdx);
      s_initInfo.u16_ProdCnxnPoint = IXSVC_InitInfoCnxnPointGet(u16_svcIdx);
      s_initInfo.s_initCalc = *IXSVC_InitInfoCalcPtrGet(u16_svcIdx);

      /* if Extended Format */
      if (IXSVD_IsExtendedFormat(s_initInfo.b_msgFormat))
      {
      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        {
          /* Max_Fault_Number (typecasting to USINT as this attribute has been
             checked to be <= 255 before) */
          IXSVC_TcooMaxFaultNumberSet(u16_svcIdx, u8_consIdx, (CSS_t_USINT)
                                      ps_sOpenPar->s_nsd.u16_maxFaultNum);
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_37,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }
      else /* else: Base Format */
      {
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        {
          /* nothing to do for Base Format */
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_38,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }
    }
    else /* else: this is a completely new instance (full initialization) */
    {
      /* RPIs */
      s_initInfo.s_initCalc.u32_rpiOT_us = ps_sOpenPar->u32_rpiOT_us;
      s_initInfo.s_initCalc.u32_rpiTO_us = ps_sOpenPar->u32_rpiTO_us;
      /* expected packet rate for data production is equal to T->O RPI */
      s_initInfo.s_initCalc.u32_EPI_us = s_initInfo.s_initCalc.u32_rpiTO_us;

      /* calculate basic number of ticks between two transmissions */
      s_initInfo.s_initCalc.u32_EPI_128us =
        s_initInfo.s_initCalc.u32_EPI_us / CSS_k_TICK_BASE_US;

      /* calculate deviation due to granularity of RPI in us and 128us ticks */
      s_initInfo.s_initCalc.u32_EpiDeviation_us =
        s_initInfo.s_initCalc.u32_EPI_us -
        (s_initInfo.s_initCalc.u32_EPI_128us * CSS_k_TICK_BASE_US);

      /* set connection size */
      s_initInfo.s_initCalc.u8_prodDataSize = u8_payloadSize;

      /* Connection Path */
      s_initInfo.u16_ProdCnxnPoint = ps_sOpenPar->u16_cnxnPointProd;

      /* Ping_Interval_EPI_Multiplier */
      s_initInfo.s_initCalc.u16_Ping_Interval_EPI_Multiplier =
        ps_sOpenPar->s_nsd.u16_pingIntEpiMult;

      /* if Extended Format */
      if (IXSVD_IsExtendedFormat(s_initInfo.b_msgFormat))
      {
      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        {
          /* Max_Fault_Number (typecasting to USINT as this attribute has been
             checked to be <= 255 before) */
          IXSVC_TcooMaxFaultNumberSet(u16_svcIdx, u8_consIdx, (CSS_t_USINT)
                                      ps_sOpenPar->s_nsd.u16_maxFaultNum);

          /* Initial Time Stamp */
          ps_sOpenResp->s_appReply.u16_initialTS =
            ps_sOpenPar->s_nsd.u16_initalTs;

          /* Initial Rollover Value */
          ps_sOpenResp->s_appReply.u16_initialRV =
            ps_sOpenPar->s_nsd.u16_initialRv;
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_19,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }
      else /* else: Base Format */
      {
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        {
          /* nothing to do for Base Format */
        }
      #else
        {
          IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_31,
                          IXSVC_InstIdFromIdxGet(u16_svcIdx),
                          IXSER_k_A_NOT_USED);
        }
      #endif
      }
    }

    /* Time_Coord_Msg_Min_Multiplier */
    /* per Consumer */
    IXSVC_TcooMsgMinMultSet(u16_svcIdx, u8_consIdx,
                            ps_sOpenPar->s_nsd.u16_tcooMsgMinMult_128us);

    /* Timeout_Multiplier, per Consumer */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      s_iiPerCons.s_Timeout_Multiplier.u8_ef =
        ps_sOpenPar->s_nsd.u8_timeoutMult;
    }
  #endif
    s_iiPerCons.s_Timeout_Multiplier.u8_PI =
      TimeoutMultiplierPiCalc(u16_svcIdx, ps_sOpenPar->s_nsd.u8_timeoutMult,
                              s_initInfo.b_msgFormat);

    /* Call the cold start logic or Reinit. For Extended Format this        */
    /* returns also the response values of initial time stamp and initial   */
    /* rollover value (see FRS377).                                         */
    /* if only a multicast consumer is added to existing connection */
    if (o_reInit)
    {
      /* attach the new consumer to the already existing safety connection  */
      IXSVC_ProdMcastReinit(u16_svcIdx,
                            u8_consIdx
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)     /* CCT_NO_PRE_WARNING */
                          ,&ps_sOpenResp->s_appReply.u16_initialTS
                          ,&ps_sOpenResp->s_appReply.u16_initialRV
    #endif
                          );

      /* execute the Time Coordination specific parts of the Cold Start Logic */
      IXSVC_TcooColdStartLogic(u16_svcIdx, u8_consIdx,
                               &s_initInfo, &s_iiPerCons);
    }
    else /* else: this is a completely new instance (full initialization) */
    {
      /* Schedule first transmission: immediately */
      IXSVC_ProdNextTxTimeSet(u16_svcIdx, IXUTL_GetSysTime());

      /* For a new connection the cold start logic must be executed. */
      IXSVC_ProdColdStartLogic(u16_svcIdx,
                               &s_initInfo
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)     /* CCT_NO_PRE_WARNING */
                             ,&ps_sOpenResp->s_appReply.u16_initialTS
                             ,&ps_sOpenResp->s_appReply.u16_initialRV
    #endif
                            );
      /* execute the Time Coordination specific parts of the Cold Start Logic */
      IXSVC_TcooColdStartLogic(u16_svcIdx,
                               CSOS_k_MAX_CONSUMER_NUM_MCAST,
                               &s_initInfo,
                               &s_iiPerCons);
    }

    /* Initialize the data for the Forward_Open Success Response */
    /* Consumer Number (as previously allocated, see FRS327) */
    /* if multicast */
    if (CSS_IsMultiCast(ps_sOpenPar->w_netCnxnPar_TO))
    {
      ps_sOpenResp->s_appReply.u16_consNum =
        (CSS_t_UINT)((CSS_t_UINT)u8_consIdx + 1U);
    }
    else /* else: singlecast */
    {
      /* for single-cast connections return a consumer number of 0xFFFF */
      /* (see FRS174) */
      ps_sOpenResp->s_appReply.u16_consNum = CSS_k_RESP_CONS_NUM_SCAST;
    }
    /* PID (see SRS152) */
    ps_sOpenResp->s_appReply.u16_trgVendId = IXSSS_VendorIdGet();
    ps_sOpenResp->s_appReply.u32_trgDevSerNum = IXSSS_SerialNumGet();
    ps_sOpenResp->s_appReply.u16_trgCnxnSerNum =
                                       IXSVC_InstIdFromIdxGet(u16_svcIdx);

    /* calculate CRC seeds (see SRS178) */
    {
      /* copy CID from SafetyOpen (see SRS152) */
      s_iiPerCons.s_cid.u16_vendId    = ps_sOpenPar->s_cnxnTriad.u16_origVendId;
      s_iiPerCons.s_cid.u32_devSerNum = ps_sOpenPar->s_cnxnTriad.u32_origSerNum;
      s_iiPerCons.s_cid.u16_cnxnSerNum= ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum;
      IXSVC_InstInfoPerConsSet(u16_svcIdx, u8_consIdx, &s_iiPerCons);

      /* Calc CID CRC seed (per consumer) */
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        IXSVC_TcooCidCrcS3Set(u16_svcIdx, u8_consIdx,
          IXCRC_PidCidCrcS3Calc(&s_iiPerCons.s_cid));
      }
    #endif
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        IXSVC_TcooCidCrcS5Set(u16_svcIdx, u8_consIdx,
          IXCRC_PidCidCrcS5Calc(&s_iiPerCons.s_cid));
      }
    #endif

      /* Again variables that are already in use don't need to be           */
      /* recalculated.                                                      */
      /* if only a multicast consumer is added to existing connection */
      if (o_reInit)
      {
        /* nothing to do */
      }
      else /* else: this is a completely new instance (full initialization) */
      {
        /* PID only a local variable. No need to be stored */
        CSS_t_PIDCID s_pid;

        /* completely erase structure */
        CSS_MEMSET(&s_pid, 0, sizeof(s_pid));

        s_pid.u16_vendId     = IXSSS_VendorIdGet();
        s_pid.u32_devSerNum  = IXSSS_SerialNumGet();
        s_pid.u16_cnxnSerNum = IXSVC_InstIdFromIdxGet(u16_svcIdx);

        /* Calc PID CRC seeds */
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        {
          s_initInfo.s_initCalc.u8_pidCrcS1 = IXCRC_PidCidCrcS1Calc(&s_pid);
        }
      #endif
        {
          s_initInfo.s_initCalc.u16_pidCrcS3 = IXCRC_PidCidCrcS3Calc(&s_pid);
        }
      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        {
          s_initInfo.s_initCalc.u32_pidCrcS5 = IXCRC_PidCidCrcS5Calc(&s_pid);
        }
      #endif

        /* Extended Format PID/RolloverCount CRC seeds don't have to be
           calculated in advance as they are re-calculated at every message
           reception anyway. */

        /* pass the init data to the init info module */
        IXSVC_InitInfoSet(u16_svcIdx, &s_initInfo);
      }
    }

    /* store data needed for SVO attributes */
    as_SvoAttrData[u16_svcIdx].u16_piem =
                               ps_sOpenPar->s_nsd.u16_pingIntEpiMult;
    as_SvoAttrData[u16_svcIdx].au16_ntem_128us[u8_consIdx] =
                               ps_sOpenPar->s_nsd.u16_NetTimeExpMult_128us;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : ConsIdxAlloc
**
** Description : This function allocates the next free consumer index of a
**               Multi-Cast Client connection.
**
** Parameters  : u16_svcIdx (IN)    - index of the addressed Safety Validator
**                                    Client instance
**                                    (not checked, checked in
**                                    IXSVC_TargClientInit())
**               u8_maxConsNum (IN) - maximum consumer number of the addressed
**                                    instance
**                                    (checked, valid range:
**                                    1..CSOS_k_MAX_CONSUMER_NUM_MCAST)
**               b_msgFormat (IN)   - Bit-mapped byte of flags to indicate what
**                                    kind of safety connection this Safety
**                                    Validator Instance refers to
**                                    (not checked, only bit 2 is of interest
**                                    here and for this bit any value is
**                                    allowed)
**               pu8_consIdx (OUT)  - Allocated Consumer Number in case of
**                                    success, CSOS_k_CNUM_INVALID in case of
**                                    error
**                                    (not checked, only called with reference
**                                    to variable)
**                                    returned values in case of success:
**                                    0..CSOS_k_MAX_CONSUMER_NUM_MCAST-1
**                                    in case of error: CSOS_k_CNUM_INVALID
**               ps_status (OUT)    - pointer to structure for returning the
**                                    status for a Forward_Open response
**                                    (not checked, only called with reference
**                                    to structure variable)
**
** Returnvalue : CSS_k_OK           - success
**               <>CSS_k_OK         - error while allocating a consumer number
**
*******************************************************************************/
static CSS_t_WORD ConsIdxAlloc(CSS_t_UINT u16_svcIdx,
                               CSS_t_USINT u8_maxConsNum,
                               CSS_t_BYTE b_msgFormat,
                               CSS_t_USINT *pu8_consIdx,
                               CSS_t_MR_RESP_STATUS *ps_status)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVC_k_FSE_INC_PRG_FLOW);

  /* indicate that no free consumer index has been found yet */
  *pu8_consIdx = CSOS_k_CNUM_INVALID;

  /* if passed parameter is invalid */
  if ((u8_maxConsNum == 0U) || (u8_maxConsNum > CSOS_k_MAX_CONSUMER_NUM_MCAST))
  {
    w_retVal = IXSVC_k_FSE_INC_C_ALLOC_MAXCNUM;
    IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u8_maxConsNum);
  }
  else /* else: ok */
  {
    /* if Multicast */
    if (IXSVD_IsMultiCast(b_msgFormat))
    {
      CSS_t_USINT u8_cIdx = 0U;

      do
      {
        const IXSVC_t_INST_INFO_PER_CONS *cps_iipc =
          IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_cIdx);

        /* if the currently tested consumer index is not yet in use */
        if (!cps_iipc->o_Consumer_Open)
        {
          /* We have found an unused consumer structure. Let's return this */
          *pu8_consIdx = u8_cIdx;
        }
        else /* else: this consumer structure is already in use */
        {
          /* look for another one */
        }

        u8_cIdx++;
      }
      while (    (u8_cIdx < u8_maxConsNum) /* not yet processed all ConsNum */
              && (*pu8_consIdx == CSOS_k_CNUM_INVALID) /* not yet found */
            );

      /* if there was no free consumer structure */
      if (*pu8_consIdx == CSOS_k_CNUM_INVALID)
      {
        /* set error code */
        ps_status->u8_gen = CSOS_k_CGSC_RES_UNAVAIL;
        ps_status->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
        w_retVal = CSS_k_OK;
      }
      else /* else: ok */
      {
        /* consumer number is now allocated */
        ps_status->u8_gen = CSOS_k_CGSC_SUCCESS;
        ps_status->u16_ext = CSOS_k_INVALID_ADD_STATUS;
        w_retVal = CSS_k_OK;
      }
    }
    else /* else: singlecast */
    {
      /* For Single-Cast connections Consumer Number is always 0 */
      *pu8_consIdx = 0U;
      /* success */
      ps_status->u8_gen = CSOS_k_CGSC_SUCCESS;
      ps_status->u16_ext = CSOS_k_INVALID_ADD_STATUS;
      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : TimeoutMultiplierPiCalc
**
** Description : This function calculates the value for the
**               Timeout_Multiplier.PI based on the passed timeout multiplier.
**
** Parameters  : u16_svcIdx (IN)     - index of Safety Validator Client
**                                     structure
**                                     (not checked, checked in
**                                     TargClientConsumerInit() and
**                                     IXSVC_OrigClientInit())
**               u8_timeoutMult (IN) - timeout multiplier value
**                                     (not checked, checked in
**                                     CnxnParamsValidateSafety1() and
**                                     RangeCheckInstSafetyParams())
**               b_msgFormat (IN)    - message format of the addressed instance
**                                     (not checked, checked in
**                                     TargClientConsumerInit() and
**                                     IXSVC_OrigClientInit())
** Returnvalue : CSS_t_USINT         - The value to be assigned to
**                                     s_Timeout_Multiplier.u8_PI
**
*******************************************************************************/
static CSS_t_USINT TimeoutMultiplierPiCalc(CSS_t_UINT u16_svcIdx,
                                           CSS_t_USINT u8_timeoutMult,
                                           CSS_t_BYTE b_msgFormat)
{
  CSS_t_USINT u8_retVal = CSS_k_MIN_TMULT_PI;

#if (CSS_cfg_EXTENDED_FORMAT != CSS_k_ENABLE)
  {
    /* just to avoid "unused parameter" warning */
    b_msgFormat = b_msgFormat;
  }
#endif

  /* if timeout multiplier is in range 1..4 */
  if (    (u8_timeoutMult >= CSS_k_MIN_TMULT_PI)
       && (u8_timeoutMult <= CSS_k_MAX_TMULT_PI)
     )
  {
    /* simply copy the passed value */
    u8_retVal = u8_timeoutMult;
  }
  else /* else: 0 or 5..255 */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* if timeout multiplier is not zero */
    if (    (u8_timeoutMult >= CSS_k_MIN_TMULT_PI)
         && (IXSVD_IsExtendedFormat(b_msgFormat))
       )
    {
      /* limit Timeout_Multiplier.PI to 4 (see FRS374) */
      u8_retVal = CSS_k_MAX_TMULT_PI;
    }
    else /* timeout multiplier is zero */
  #endif
    {
      /* error - should have been prevented by other checks */
      IXSVC_ErrorClbk(IXSVC_k_FSE_INC_TMULT_PI_CALC,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      (CSS_t_UDINT)u8_timeoutMult);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_retVal);
}


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


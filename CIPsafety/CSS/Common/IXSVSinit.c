/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSinit.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains the functions to initialize the SVS unit and
**             to start the data consumption of a Safety Validator Server
**             instance.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_Init
**             IXSVS_PingIntEpiMultGet
**             IXSVS_TcooMsgMinMultGet
**             IXSVS_MaxConsNumGet
**             IXSVS_TargServerInit
**             IXSVS_OrigServerInit
**
**             SendCnxnOpenMsg
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
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "HALCSapi.h"
#endif

#include "IXSSS.h"
#include "IXSERapi.h"

#include "IXSVD.h"
#include "IXCRC.h"
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

/* data type for storing values needed for SVO attributes */
typedef struct
{
  CSS_t_UINT  u16_piem;          /* Ping Interval EPI Multiplier */
  CSS_t_UINT  u16_tcoommm_128us; /* Time Coordination Message Min Multiplier */
  CSS_t_USINT u8_maxConsNum;     /* Max Consumer Number */
} t_SVS_ATTR_DATA;

/* array of structures storing data for SVO attributes */
/* Not relevant for Soft Error Checking - only a diagnostic value */
static t_SVS_ATTR_DATA as_SvoAttrData[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
static CSS_t_WORD SendCnxnOpenMsg(CSS_t_UINT u16_svInst,
                                  CSS_t_UDINT u32_appRef,
                                  CSS_t_BYTE b_cnxnType);
#endif


/*******************************************************************************
**    global functions
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
void IXSVS_Init(void)
{
  CSS_t_UINT u16_svsIdx = 0U;

  /* clear the locals of this unit */
  CSS_MEMSET(as_SvoAttrData, 0, sizeof(as_SvoAttrData));

  /* make sure all instances are unallocated */
  IXSVS_InstInitAll();

  /* make sure all Validator instances are unallocated and in Idle state */
  for (u16_svsIdx = 0U; u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_svsIdx++)
  {
    IXSVS_InstInit(u16_svsIdx);
    IXSVS_StateInit(u16_svsIdx);
  }

  /* initialize the fault counter of the Safety Validator Servers */
  IXSVS_StateFaultCountReset();

  /* initialize the lookup table for the index <-> instance ID relations */
  IXSVS_InstIdInit();

  /* Initialize the Error module (Last Error Codes) */
  IXSVS_ErrorInit();

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_UINT IXSVS_PingIntEpiMultGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvoAttrData[u16_svsIdx].u16_piem);
}


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
CSS_t_UINT IXSVS_TcooMsgMinMultGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvoAttrData[u16_svsIdx].u16_tcoommm_128us);
}


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
CSS_t_USINT IXSVS_MaxConsNumGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvoAttrData[u16_svsIdx].u8_maxConsNum);
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
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
CSS_t_WORD IXSVS_TargServerInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                CSS_t_USINT u8_payloadSize,
                                CSS_t_UINT u16_svInst,
                                CSS_t_APP_REPLY *ps_appReply)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVS_k_FSE_INC_PRG_FLOW);
  /* index of the Safety Validator Server */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_svInst);
  /* local struct for collecting initialization data */
  IXSVS_t_INIT_INFO s_initInfo;

  /* completely erase the init structure */
  CSS_MEMSET(&s_initInfo, 0, sizeof(s_initInfo));

  /* if an invalid safety validator instance was passed */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    w_retVal = (IXSVS_k_FSE_INC_INST_INV_TSI);
    IXSVS_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);
  }
  else /* else: Safety Validator Instance exists */
  {
    /* Target && Server -> this must be single-cast */
    /* MCast bit is already cleared */
    /* set connection type that is reported to the CSAL */
    ps_appReply->b_cnxnType = CSOS_k_CNXN_CONS_SINGLE;

    /* if is Extended Format */
    if (ps_sOpenPar->s_nsd.u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
    {
      s_initInfo.b_msgFormat = IXSVD_k_MSG_FORMAT_EXTENDED;
    }
    else /* else: must be Base Format (anything else was checked before) */
    {
      /* bit is already cleared */
    }

    /* if is Long Message Format */
    if (u8_payloadSize > CSS_k_SHORT_FORMAT_MAX_LENGTH)
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_LONG;
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_35, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Short Message Format */
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* bit is already cleared */
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_36, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }

    /* set connection size */
    IXSVS_ConsDataSizeSet(u16_svsIdx, u8_payloadSize);

    /* Connection Path */
    s_initInfo.u16_consCnxnPoint = ps_sOpenPar->u16_cnxnPointCons;

    /* As this is a single cast connection the Consumer Number is 1 */
    s_initInfo.s_initCalc.u8_Consumer_Num = CSOS_k_MAX_CONSUMER_NUM_SCAST;

    /* if is Extended Format */
    if (IXSVD_IsExtendedFormat(s_initInfo.b_msgFormat))
    {
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        /* Max_Fault_Number (typecasting to USINT as this attribute has been
           checked to be <= 255 during Validate in RangeCheckInst()) */
        s_initInfo.s_initCalc.u8_Max_Fault_Number =
          (CSS_t_USINT)ps_sOpenPar->s_nsd.u16_maxFaultNum;

        /* Initial Time Stamp */
        ps_appReply->u16_initialTS = ps_sOpenPar->s_nsd.u16_initalTs;

        /* Initial Rollover Value */
        ps_appReply->u16_initialRV = ps_sOpenPar->s_nsd.u16_initialRv;
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_20, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else
    {
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        /* nothing to do for Base Format */
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_34, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }

    /* Network_Time_Expectation_Multiplier */
    s_initInfo.s_initCalc.u16_Network_Time_Expectation_Multiplier_128us =
      ps_sOpenPar->s_nsd.u16_NetTimeExpMult_128us;

    /* Timeout_Multiplier */
    IXSVS_TimeoutMultiplierInit(u16_svsIdx, ps_sOpenPar->s_nsd.u8_timeoutMult);

    /* Now that all Safety Validator Format Settings are made we can execute  */
    /* the Cold Start Logic.                                                  */
    /* For Extended Format this returns also the response values of initial   */
    /* time stamp and initial rollover value (see FRS379).                    */
    IXSVS_ConsColdStartLogic(u16_svsIdx
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)         /* CCT_NO_PRE_WARNING */
                             ,s_initInfo.b_msgFormat
                             ,ps_appReply->u16_initialTS
                             ,ps_appReply->u16_initialRV
                             ,&ps_appReply->u16_initialTS
                             ,&ps_appReply->u16_initialRV
  #endif
                            );

    /* Time Coordination timeout (u32_tcooTimer_128us) already set to zero by
       IXSVS_InstInit(). Initialize Time Coordination timeout to an
       arbitrary value is safe as sending Time Coordination messages is
       blocked by u8_Time_Coordination_Count_Down */

    /* Initialize the data for the Forward_Open Success Response */
    /* for single-cast connections return a consumer number of 0xFFFF */
    /* CID (see SRS152) */
    ps_appReply->u16_consNum = CSS_k_RESP_CONS_NUM_SCAST; /* (see FRS174) */
    ps_appReply->u16_trgVendId = IXSSS_VendorIdGet();
    ps_appReply->u32_trgDevSerNum = IXSSS_SerialNumGet();
    ps_appReply->u16_trgCnxnSerNum = u16_svInst;

    /* calculate CRC seeds (see SRS178) */
    {
      /* CID only a local variable. No need to be stored */
      CSS_t_PIDCID s_cid;

      /* completely erase structure */
      CSS_MEMSET(&s_cid, 0, sizeof(s_cid));

      /* copy PID from SafetyOpen (see SRS152) */
      s_initInfo.s_initCalc.s_pid.u16_vendId     =
        ps_sOpenPar->s_cnxnTriad.u16_origVendId;
      s_initInfo.s_initCalc.s_pid.u32_devSerNum  =
        ps_sOpenPar->s_cnxnTriad.u32_origSerNum;
      s_initInfo.s_initCalc.s_pid.u16_cnxnSerNum =
        ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum;

      /* Store local CID for calculation of CRCs */
      s_cid.u16_vendId     = IXSSS_VendorIdGet();
      s_cid.u32_devSerNum  = IXSSS_SerialNumGet();
      s_cid.u16_cnxnSerNum = u16_svInst;

      /* Calc CID CRC seed */
      s_initInfo.s_initCalc.u16_cidCrcS3 = IXCRC_PidCidCrcS3Calc(&s_cid);
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u32_cidCrcS5 = IXCRC_PidCidCrcS5Calc(&s_cid);
      }
    #endif

      /* Calc PID CRC seeds */
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u8_pidCrcS1 =
          IXCRC_PidCidCrcS1Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #endif
      {
        s_initInfo.s_initCalc.u16_pidCrcS3 =
          IXCRC_PidCidCrcS3Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u32_pidCrcS5 =
          IXCRC_PidCidCrcS5Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #endif

      /* pass the init data to the init info module */
      IXSVS_InitInfoSet(u16_svsIdx, &s_initInfo);
    }

    /* set the supervision interval of the Consumer Activity Monitor and
       calculate point in time when first data must be received (timeout) */
    IXSVS_ActMonIntervalSet(u16_svsIdx,
                            ps_sOpenPar->s_nsd.u16_NetTimeExpMult_128us);

    /* Calculate point in time when initialization must be completed */
    /* (see FRS280) */
    IXSVS_StateInitLimitCalc(u16_svsIdx);

    /* update state machine */
    IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_OPEN);

    /* store data needed for SVO attributes */
    as_SvoAttrData[u16_svsIdx].u16_piem = ps_sOpenPar->s_nsd.u16_pingIntEpiMult;
    as_SvoAttrData[u16_svsIdx].u16_tcoommm_128us =
      ps_sOpenPar->s_nsd.u16_tcooMsgMinMult_128us;
    as_SvoAttrData[u16_svsIdx].u8_maxConsNum = ps_sOpenPar->s_nsd.u8_maxConsNum;

    /* there are no other errors that could occur within this function */
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
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
CSS_t_WORD IXSVS_OrigServerInit(
                           const CSS_t_SOPEN_RESP_RX *ps_openRespRx,
                           const CSS_t_CCO_SOPEN_INIT_VAL *ps_ccoSopenInitVal,
                           CSS_t_UINT u16_svInst)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVS_k_FSE_INC_PRG_FLOW);
  /* index of the Safety Validator Server */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_svInst);
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* temporary variables for passing Initial TimeStamp and Initial Rollover   */
  /* Value to Cold Start Logic                                                */
  CSS_t_UINT u16_initialTs = 0U;
  CSS_t_UINT u16_initialRv = 0U;
#endif
  /* local struct for collecting initialization data */
  IXSVS_t_INIT_INFO s_initInfo;

  /* completely erase the init structure */
  CSS_MEMSET(&s_initInfo, 0, sizeof(s_initInfo));

  /* if an invalid safety validator instance was passed */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    w_retVal = (IXSVS_k_FSE_INC_INST_INV_OSI);
    IXSVS_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);
  }
  else /* else: Safety Validator Instance exists */
  {
    /* we are originator of this connection */
    s_initInfo.b_msgFormat = IXSVD_k_MSG_FORMAT_ORIGINATOR;

    /* Multi-Cast Server Connections must be fully initialized whereas        */
    /* Single-Cast Server Connections are already partially initialized.      */

    /* If is Multi-cast */
    if (ps_ccoSopenInitVal->b_cnxnType == CSOS_k_CNXN_CONS_MULTI)
    {
      /* we are originator of this connection */
      s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_MCAST;

      /* Set the consumer number from the SafetyOpen response */
      /* TMS320: Most significant 8 bits are certainly zero because
                 FwdOpenRespCheck() has checked before that consumer number is
                 in the range 1..15 */
      s_initInfo.s_initCalc.u8_Consumer_Num =
        (CSS_t_USINT)ps_openRespRx->u16_consNum;
    }
    else /* else: must be Single-cast (anything else was checked before) */
    {
      /* Single-Cast Server Connections are already partially initialized */
      s_initInfo.s_initCalc.u8_Consumer_Num = CSOS_k_MAX_CONSUMER_NUM_SCAST;
    }

    /* if is Extended Format */
    if (ps_ccoSopenInitVal->u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
    {
      s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_EXTENDED;
    }
    else /* else: must be Base Format (anything else was checked before) */
    {
      s_initInfo.b_msgFormat &=
        (CSS_t_BYTE)(~(CSS_t_BYTE)IXSVD_k_MSG_FORMAT_EXTENDED);
    }

    /* if is Long Message Format */
    if (ps_ccoSopenInitVal->u8_dataLen > CSS_k_SHORT_FORMAT_MAX_LENGTH)
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        s_initInfo.b_msgFormat |= IXSVD_k_MSG_FORMAT_LONG;
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_38, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Short Message Format */
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* bit is already cleared */
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_39, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }

    /* set connection size */
    IXSVS_ConsDataSizeSet(u16_svsIdx, ps_ccoSopenInitVal->u8_dataLen);

    /* in case Process Image is enabled: Byte-Index in process image */
    /* else                          : Connection Point (= Assembly Instance) */
    s_initInfo.u16_consCnxnPoint = ps_ccoSopenInitVal->u16_dataOffset;

    /* if is Extended Format */
    if (IXSVD_IsExtendedFormat(s_initInfo.b_msgFormat))
    {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        /* Max_Fault_Number (typecasting to USINT as this attribute has been
           checked to be <= 255 before) */
        s_initInfo.s_initCalc.u8_Max_Fault_Number =
          (CSS_t_USINT)ps_ccoSopenInitVal->u16_MaxFault_Number;

        /* make a temporary copy of received initial Timestamp and Rollover
           val */
        u16_initialTs = ps_openRespRx->u16_initialTs;
        u16_initialRv = ps_openRespRx->u16_initialRv;
      }
  #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_21,
                        u16_svInst, IXSER_k_A_NOT_USED);
      }
  #endif
    }
    else
    {
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        /* nothing to do for Base Format */
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_37, u16_svInst,
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }


    /* Network_Time_Expectation_Multiplier */
    s_initInfo.s_initCalc.u16_Network_Time_Expectation_Multiplier_128us =
      ps_ccoSopenInitVal->u16_NetTimeExpMult_128us;

    /* Timeout_Multiplier */
    IXSVS_TimeoutMultiplierInit(u16_svsIdx, ps_ccoSopenInitVal->u8_timeoutMult);

    /* if is Multi-cast */
    if (ps_ccoSopenInitVal->b_cnxnType == CSOS_k_CNXN_CONS_MULTI)
    {
      /* Now that all Safety Validator Format Settings are made we can        */
      /* execute the Cold Start Logic.                                        */
      IXSVS_ConsColdStartLogic(u16_svsIdx
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)       /* CCT_NO_PRE_WARNING */
                               ,s_initInfo.b_msgFormat
                               ,u16_initialTs
                               ,u16_initialRv
                               ,&u16_initialTs
                               ,&u16_initialRv
    #endif
                              );
    }
    else /* else: Single-cast */
    {
      /* For Single-Cast Server Connections the Cold Start Logic was already  */
      /* executed.                                                            */
    }

    /* Time Coordination timeout (u32_tcooTimer_128us) already set to zero by
       IXSVS_InstInit(). Initialize Time Coordination timeout to an
       arbitrary value is safe as sending Time Coordination messages is
       blocked by u8_Time_Coordination_Count_Down */

    /* calculate CRC seeds (see SRS178) */
    {
      /* CID only a local variable. No need to be stored */
      CSS_t_PIDCID s_cid;

      /* completely erase the structure */
      CSS_MEMSET(&s_cid, 0, sizeof(s_cid));

      /* copy PID from SafetyOpen Response (see SRS152) */
      s_initInfo.s_initCalc.s_pid.u16_vendId     =
        ps_openRespRx->u16_trgVendId;
      s_initInfo.s_initCalc.s_pid.u32_devSerNum  =
        ps_openRespRx->u32_trgDevSerNum;
      s_initInfo.s_initCalc.s_pid.u16_cnxnSerNum =
        ps_openRespRx->u16_trgCnxnSerNum;

      /* Store local CID for calculation of CRCs */
      s_cid.u16_vendId     = IXSSS_VendorIdGet();
      s_cid.u32_devSerNum  = IXSSS_SerialNumGet();
      s_cid.u16_cnxnSerNum = ps_openRespRx->s_cnxnTriad.u16_cnxnSerNum;

      /* Calc CID CRC seed */
      s_initInfo.s_initCalc.u16_cidCrcS3 = IXCRC_PidCidCrcS3Calc(&s_cid);
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u32_cidCrcS5 = IXCRC_PidCidCrcS5Calc(&s_cid);
      }
    #endif

      /* Calc PID CRC seeds */
    #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u8_pidCrcS1 =
          IXCRC_PidCidCrcS1Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #endif
      {
        s_initInfo.s_initCalc.u16_pidCrcS3 =
          IXCRC_PidCidCrcS3Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
      {
        s_initInfo.s_initCalc.u32_pidCrcS5 =
          IXCRC_PidCidCrcS5Calc(&s_initInfo.s_initCalc.s_pid);
      }
    #endif

      /* pass the init data to the init info module */
      IXSVS_InitInfoSet(u16_svsIdx, &s_initInfo);
    }

    /* set the supervision interval of the Consumer Activity Monitor and
       calculate point in time when first data must be received (timeout) */
    IXSVS_ActMonIntervalSet(u16_svsIdx,
                            ps_ccoSopenInitVal->u16_NetTimeExpMult_128us);

    /* Calculate point in time when initialization must be completed */
    /* (see FRS280) */
    IXSVS_StateInitLimitCalc(u16_svsIdx);

    /* store data needed for SVO attributes */
    as_SvoAttrData[u16_svsIdx].u16_piem =
                                 ps_ccoSopenInitVal->u16_pingIntEpiMult;
    as_SvoAttrData[u16_svsIdx].u16_tcoommm_128us =
                                 ps_ccoSopenInitVal->u16_tcooMsgMinMult_128us;
    as_SvoAttrData[u16_svsIdx].u8_maxConsNum =
                                 ps_ccoSopenInitVal->u8_maxConsNum;

    w_retVal = SendCnxnOpenMsg(u16_svInst, ps_ccoSopenInitVal->u32_appRef,
                               ps_ccoSopenInitVal->b_cnxnType);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : SendCnxnOpenMsg
**
** Description : This function sends the CSOS_k_CMD_IXSCE_CNXN_OPEN command to
**               CSAL via HALC when an originator opens a connection.
**
** Parameters  : u16_svInst (IN) - Safety Validator instance ID that is already
**                                 allocated for this connection
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**               u32_appRef (IN) - Application Reference
**                                 (not checked, any value allowed)
**               b_cnxnType (IN) - Flags describing the connection. See
**                                 definition of {CSOS_k_CNXN_xxx}.
**                                 (not checked, checked in
**                                 IXSVS_OrigServerInit()
**
** Returnvalue : CSS_t_USINT     - Max Consumer Number
**
*******************************************************************************/
static CSS_t_WORD SendCnxnOpenMsg(CSS_t_UINT u16_svInst,
                                  CSS_t_UDINT u32_appRef,
                                  CSS_t_BYTE b_cnxnType)
{
  /* return value of this function */
  CSS_t_WORD w_retVal  = (IXSVS_k_FSE_INC_PRG_FLOW);
  /* index of the Safety Validator Server */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_svInst);
  /* structure for passing the success response to HALC */
  HALCS_t_MSG  s_halcMsg;
  /* array for passing data of the response to HALC */
  CSS_t_BYTE ab_rspData[CSOS_k_SIZEOF_UDINT + CSOS_k_SIZEOF_BYTE];

  /* completely erase structure */
  CSS_MEMSET(&s_halcMsg, 0, sizeof(s_halcMsg));

  /* if an invalid safety validator instance was passed */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    w_retVal = (IXSVS_k_FSE_INC_INST_SCOM_NUM);
    IXSVS_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
  {
    /* initialize the message that will be sent to CSAL */
    s_halcMsg.u16_cmd = CSOS_k_CMD_IXSCE_CNXN_OPEN;
    s_halcMsg.u16_len = CSOS_k_SIZEOF_UDINT
                      + CSOS_k_SIZEOF_BYTE;

    s_halcMsg.u32_addInfo = (CSS_t_UDINT)u16_svInst;

    /* link message with local buffer to place the response data in it */
    s_halcMsg.pb_data = ab_rspData;

    /* application reference */
    CSS_H2N_CPY32(s_halcMsg.pb_data, &u32_appRef);
    /* Cnxn Type */
    CSS_H2N_CPY8(CSS_ADD_OFFSET(s_halcMsg.pb_data, CSOS_k_SIZEOF_UDINT),
                 &b_cnxnType);

    /* if sending the message to CSAL is successful */
    if (HALCS_TxDataPut(&s_halcMsg))
    {
      /* update state machine */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_OPEN);
      /* success */
      w_retVal = CSS_k_OK;
    }
    else /* else: error */
    {
      /* error while trying to send the message to CSAL */
      w_retVal = (IXSVS_k_NFSE_TXH_HALCS_SEND_OSI);
      IXSVS_ErrorClbk(w_retVal, u16_svInst, IXSER_k_A_NOT_USED);

      /* so then close the connection that we tried to initialize */
      IXSVS_InstStop(u16_svsIdx);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


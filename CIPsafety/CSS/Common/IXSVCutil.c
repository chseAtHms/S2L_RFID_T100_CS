/*******************************************************************************
**    Copyright (C) 2009-2020 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCutil.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains utility functions for the implementation of
**             the Safety Validator Client functionality and also the state
**             machine of the Safety Validator Client instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_UtilLocalsClear
**             IXSVC_ConnectionFaultFlagSet
**             IXSVC_ConnectionIsFaulted
**             IXSVC_RrConNumIdxPntrGet
**             IXSVC_RrConNumIdxPntrSet
**             IXSVC_ConsumerIsActive
**             IXSVC_ConsumerActiveIdleSet
**             IXSVC_ConsTimeCorrValGet
**             IXSVC_ConsTimeCorrValSet
**             IXSVC_McastCnxnFind
**             IXSVC_ProducerClkCountGet
**             IXSVC_RoundupDivision
**             IXSVC_ConsumerRemainingCheck
**             IXSVC_UtilSoftErrByteGet
**             IXSVC_UtilSoftErrVarGet
**             CheckMatchingConnection
**
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

#include "IXSERapi.h"
#include "IXSVD.h"
#include "IXUTL.h"

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

/** t_SVC_LOCALS_UTIL:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* Consumer_Active_Idle (see Vol. 5 Chapt. 2-4.5.1.3): run time flag to
     indicating if valid Time Coordination Message information has been received
     for this consumer.
     Legal values: IXSVD_k_IDLE (0) and IXSVD_k_ACTIVE (1) */
     /* (see FRS220) */
  CSS_t_BYTE ab_Consumer_Active_Idle[CSOS_k_MAX_CONSUMER_NUM_MCAST];

  /* Consumer_Time_Correction_Value (see Vol. 5 Chapt. 2-4.5.5.3): is added to
     the producer's data time stamp to allow the result to indicate the
     worst-case earliest time that the data could have been sampled relative to
     the consumer's clock value.
     Legal range: 0..65535 */
  CSS_t_UINT
    au16_Consumer_Time_Correction_Value_128us[CSOS_k_MAX_CONSUMER_NUM_MCAST];

  /* RR_Con_Num_Index_Pntr (see Vol. 5 Chapt. 2-4.5.4.5): variable used by the
     multi-cast producer to control the checking of the timeout on the reception
     of Time Coordination messages, and to control the sending of Time
     Correction messages. */
  CSS_t_USINT     u8_RR_Con_Num_Index_Pntr;

  /* S_Connection_Fault (see Vol. 5 Chapt. 2-4.5.1.4): run time flag that
     indicates whether the safety connection to the consumer is OK or Faulted.
     Legal values: IXSVD_k_CNXN_OK (0) and IXSVD_k_CNXN_FAULTED (1) */
     /* (see FRS224) */
  CSS_t_BYTE ab_S_Connection_Fault[CSOS_k_MAX_CONSUMER_NUM_MCAST];

} t_SVC_LOCALS_UTIL;


static t_SVC_LOCALS_UTIL as_SvcLocals[CSOS_cfg_NUM_OF_SV_CLIENTS];


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* Extended Status 0x0112: number of more additional status words */
  #define k_EXT_STATUS_112_NUM_MORE_EXT  5U
  /* Extended Status 0x0112: Acceptable O->T and T->O RPI type values */
  #define k_EXT_STATUS_112_MISMATCH_OT_RPI ((CSS_t_UINT)(4U))
  #define k_EXT_STATUS_112_MISMATCH_TO_RPI (CSS_t_UINT)((CSS_t_UINT)4U << 8U)

  /* consistency check */
  #if (k_EXT_STATUS_112_NUM_MORE_EXT > CSS_k_MAX_NUM_EXT_STATUS_WORDS)
    #error Extended Status 0x0112 too long for CSS_t_MR_RESP_STATUS.au16_moreExt
  #endif
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
static CSS_t_BOOL CheckMatchingConnection(CSS_t_UINT u16_svcIdx,
                                          const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                          CSS_t_BYTE b_msgFormat,
                                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_UtilLocalsClear(CSS_t_UINT u16_svcIdx)
{
  CSS_MEMSET(&as_SvcLocals[u16_svcIdx], 0, sizeof(as_SvcLocals[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                  CSS_t_BYTE b_value)
{
  as_SvcLocals[u16_svcIdx].ab_S_Connection_Fault[u8_consIdx] = b_value;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                     CSS_t_USINT u8_consIdx)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if the addressed consumer is faulted */
  if (as_SvcLocals[u16_svcIdx].ab_S_Connection_Fault[u8_consIdx]
      == IXSVD_k_CNXN_FAULTED)
  {
    o_retVal = CSS_k_TRUE;
  }
  else /* else: not faulted */
  {
    /* return value already set */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


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
CSS_t_USINT IXSVC_RrConNumIdxPntrGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvcLocals[u16_svcIdx].u8_RR_Con_Num_Index_Pntr);
}


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
                              CSS_t_USINT u8_maxConsNum)
{
  /* if the passed Maximum Consumer Number is out of range */
  if ((u8_maxConsNum == 0U) || (u8_maxConsNum > CSOS_k_MAX_CONSUMER_NUM_MCAST))
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_MAX_CONS_NUM_CI,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)u8_maxConsNum);
  }
  /* else: if passed value is out of range */
  else if (u8_value > u8_maxConsNum /*IXSVC_InitInfoMaxConsNumGet(u16_svcIdx)*/)
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_CON_NUM_IDX_PTR,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx), (CSS_t_UDINT)u8_value);
  }
  else /* else: passed parameters are ok */
  {
    as_SvcLocals[u16_svcIdx].u8_RR_Con_Num_Index_Pntr = u8_value;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                  CSS_t_USINT u8_consIdx)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if the addressed consumer is active */
  if (as_SvcLocals[u16_svcIdx].ab_Consumer_Active_Idle[u8_consIdx]
      == IXSVD_k_ACTIVE)
  {
    o_retVal = CSS_k_TRUE;
  }
  else /* else: not active */
  {
    /* return value already set */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


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
                                 CSS_t_BYTE b_value)
{
  as_SvcLocals[u16_svcIdx].ab_Consumer_Active_Idle[u8_consIdx] = b_value;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                    CSS_t_USINT u8_consIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_SvcLocals[u16_svcIdx].
          au16_Consumer_Time_Correction_Value_128us[u8_consIdx]);
}


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
                              CSS_t_UINT u16_value_128us)
{
  as_SvcLocals[u16_svcIdx].au16_Consumer_Time_Correction_Value_128us[u8_consIdx]
    = u16_value_128us;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
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
CSS_t_BOOL IXSVC_McastCnxnFind(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                               CSS_t_UINT *pu16_svInst,
                               CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  /* flag indicating if a matching multicast connection has been found */
  CSS_t_BOOL o_cnxnMatchFound = CSS_k_FALSE;
  /* Index for searching in Safety Validator Client Structure array */
  CSS_t_UINT u16_svcIdx = 0U;

  /* initialize output parameters to defaults
     (default is: there is no matching connection) */
  *pu16_svInst = CSOS_k_INVALID_INSTANCE;
  ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
  ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;

  /* check all Safety Validator Client structures (or until a match is found) */
  do
  {
    /* if instance is currently in use */
    if (IXSVC_StateGet(u16_svcIdx) != CSS_k_SV_STATE_IDLE)
    {
      CSS_t_BYTE b_msgFormat = IXSVC_InitInfoMsgFrmtGet(u16_svcIdx);

      /* if Multi-Cast producer */
      if (IXSVD_IsMultiCast(b_msgFormat))
      {
        /* if new connection wants to connect to same connection point */
        if (IXSVC_InitInfoCnxnPointGet(u16_svcIdx)
             == ps_sOpenPar->u16_cnxnPointProd)
        {
          o_cnxnMatchFound = CSS_k_TRUE;

          /* if the parameters that must be equal for being able to connect
             another consumer are matching */
          if (CheckMatchingConnection(u16_svcIdx, ps_sOpenPar, b_msgFormat,
                                  ps_sOpenRespStat))
          {
             ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_SUCCESS;
            *pu16_svInst = IXSVC_InstIdFromIdxGet(u16_svcIdx);
          }
          else
          {
            /* *pu16_svInst and ps_sOpenRespStat already set */
          }
        }
        else /* else: different connection point */
        {
          /* not of interest, continue */
        }
      }
      else /* else: Single-Cast */
      {
        /* not of interest, continue */
      }
    }
    else /* else: unused instance */
    {
      /* not of interest, continue */
    }

    /* advance to next instance */
    u16_svcIdx++;
  }
  while ((u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS) && (!o_cnxnMatchFound));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_cnxnMatchFound);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


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
CSS_t_UINT IXSVC_ProducerClkCountGet(void)
{
  /* return only the least significant 16 bits of the global system time */
  /* (see FRS248) */
  CSS_t_UINT u16_clkCount_128us =
    (CSS_t_UINT)(IXUTL_GetSysTime() & 0x0000FFFFU);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_clkCount_128us);
}


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
                                  CSS_t_UDINT u32_divisor)
{
  CSS_t_UDINT u32_quotient = 0U;

  /* if divisor is non-zero */
  if (u32_divisor != 0U)
  {
    u32_quotient = u32_dividend / u32_divisor;

    /* if a roundup is necessary */
    if ((u32_quotient * u32_divisor) < u32_dividend)
    {
      u32_quotient++;
    }
    else /* else: no roundup needed */
    {
      /* nothing to do */
    }
  }
  else /* else: division by zero! */
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_DIV_BY_ZERO, IXSER_k_I_NOT_USED,
                    IXSER_k_A_NOT_USED);

    /* return a defined value for this undefined operation */
    u32_quotient = 0U;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_quotient);
}


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
CSS_t_BOOL IXSVC_ConsumerRemainingCheck(CSS_t_UINT u16_svcIdx)
{
  CSS_t_USINT u8_cIdx = 0U;
  CSS_t_BOOL  o_consLeft = CSS_k_FALSE;

  /* check if this connection has any consumers left */
  for (u8_cIdx = 0U;
       u8_cIdx < IXSVC_InitInfoMaxConsNumGet(u16_svcIdx);
       u8_cIdx++)
  {
    /* get a pointer to per consumer data */
    const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
      IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_cIdx);

    /* if connection to consumer is open and not faulted */
    if (    (!IXSVC_ConnectionIsFaulted(u16_svcIdx, u8_cIdx))
         && (kps_iiperCons->o_Consumer_Open)
       )
    {
      o_consLeft = CSS_k_TRUE;
    }
    else
    {
      /* no change of the flag */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_consLeft);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVC_UtilSoftErrByteGet(CSS_t_UDINT u32_varCnt,
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVC_UtilSoftErrVarGet(
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
          CSS_H2N_CPY8(pb_var, &as_SvcLocals[au32_cnt[0U]].ab_Consumer_Active_Idle[au32_cnt[2U]]);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        }
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
        else
        {
          CSS_H2N_CPY16(pb_var, &as_SvcLocals[au32_cnt[0U]].au16_Consumer_Time_Correction_Value_128us[au32_cnt[2U]]);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        }
        break;
      }

      case 2U:
      {
          CSS_H2N_CPY8(pb_var, &as_SvcLocals[au32_cnt[0U]].u8_RR_Con_Num_Index_Pntr);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 3U:
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
          CSS_H2N_CPY8(pb_var, &as_SvcLocals[au32_cnt[0U]].ab_S_Connection_Fault[au32_cnt[2U]]);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        }
        break;
      }

      default:
      {
        /* level 1 counter at/above end */
        ps_retStat->u8_cpyLen = 0U;
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
** Function    : CheckMatchingConnection
**
** Description : This function is called when a Multicast Forward_Open wants to
**               connect to another connection to the same matching connection
**               point. This function then checks if other required parameters
**               are equal.
**
** Parameters  : u16_svcIdx (IN)        - Instance index of the Safety Validator
**                                        Client instance
**                                        (not checked, checked in
**                                        IXSVC_McastCnxnFind())
**               ps_sOpenPar (IN)       - pointer to parameters received with
**                                        the Forward_Open request (Target) or
**                                        with accordingly set data in case of
**                                        Originator
**                                        (not checked, only called with
**                                        reference to structure variable)
**               b_msgFormat (IN)       - message format of the Safety Validator
**                                        Client Instance
**                                        (not checked, checked in
**                                        IXSVC_McastCnxnFind())
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_TRUE             - connection can connect the requested
**                                        new consumer
**               CSS_k_FALSE            - new consumer cannot connect
**
*******************************************************************************/
static CSS_t_BOOL CheckMatchingConnection(CSS_t_UINT u16_svcIdx,
                                          const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                          CSS_t_BYTE b_msgFormat,
                                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* pointer to the initially calculated values of this instance
     (Ping_Interval_EPI_Multiplier is needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);

  /* if Ping_Interval_EPI_Multiplier mismatch */
  if (
       kps_initCalc->u16_Ping_Interval_EPI_Multiplier !=
       ps_sOpenPar->s_nsd.u16_pingIntEpiMult
     )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MC_PIEM;
    /* report directly to SAPL (not IXSVC_ErrorClbk() as this is
       part of the SafetyOpen checks) */
    SAPL_CssErrorClbk(IXSVC_k_NFSE_RXE_MCASTRC_PIEM,
                      ps_sOpenPar->u16_cnxnPointProd,
                      (CSS_t_UDINT)ps_sOpenPar->s_nsd.u16_pingIntEpiMult);
  }
  /* else: if Max_Consumer_Number mismatch */
  else if (
            IXSVC_InitInfoMaxConsNumGet(u16_svcIdx) !=
            ps_sOpenPar->s_nsd.u8_maxConsNum
          )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MC_MCN;
    /* report directly to SAPL (not IXSVC_ErrorClbk() as this is
       part of the SafetyOpen checks) */
    SAPL_CssErrorClbk(IXSVC_k_NFSE_RXE_MCASTRC_MCN,
                      ps_sOpenPar->u16_cnxnPointProd,
                      (CSS_t_UDINT)ps_sOpenPar->s_nsd.u8_maxConsNum);
  }
  /* else: if Format (Base/Extended) mismatch */
  else if (
               (    IXSVD_IsBaseFormat(b_msgFormat)
                 && (ps_sOpenPar->s_nsd.u8_format !=
                     CSOS_k_SNS_FORMAT_TARGET_BASE)
               )
            || (    IXSVD_IsExtendedFormat(b_msgFormat)
                 && (ps_sOpenPar->s_nsd.u8_format !=
                     CSOS_k_SNS_FORMAT_TARGET_EXT)
               )
          )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_MC_SNST;
    /* report directly to SAPL (not IXSVC_ErrorClbk() as this is
       part of the SafetyOpen checks) */
    SAPL_CssErrorClbk(IXSVC_k_NFSE_RXE_MCASTRC_FORMAT,
                      ps_sOpenPar->u16_cnxnPointProd,
                      (CSS_t_UDINT)ps_sOpenPar->s_nsd.u8_format);
  }
  /* else: if RPIs mismatch */
  else if (
               (kps_initCalc->u32_rpiOT_us != ps_sOpenPar->u32_rpiOT_us)
            || (kps_initCalc->u32_rpiTO_us != ps_sOpenPar->u32_rpiTO_us)
          )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_RPI_NOT_ACCPT;
    ps_sOpenRespStat->u8_numMoreExt = k_EXT_STATUS_112_NUM_MORE_EXT;
    ps_sOpenRespStat->au16_moreExt[0] = 0x0000U;

    /* if O->T RPI mismatch */
    if (kps_initCalc->u32_rpiOT_us != ps_sOpenPar->u32_rpiOT_us)
    {
      ps_sOpenRespStat->au16_moreExt[0] |= k_EXT_STATUS_112_MISMATCH_OT_RPI;


      /* report directly to SAPL (not IXSVC_ErrorClbk() as this is
         part of the SafetyOpen checks) */
      SAPL_CssErrorClbk(IXSVC_k_NFSE_RXE_MCASTRC_RPI_OT,
                        ps_sOpenPar->u16_cnxnPointProd, IXSER_k_A_NOT_USED);
    }
    else  /* else: O->T RPI is ok */
    {
      /* the RPI specified in the Forward_Open was acceptable
         (the Originator to Target RPI value is ignored). */
    }
    /* if T->O RPI mismatch */
    if (kps_initCalc->u32_rpiTO_us != ps_sOpenPar->u32_rpiTO_us)
    {
      ps_sOpenRespStat->au16_moreExt[0] |= k_EXT_STATUS_112_MISMATCH_TO_RPI;

      /* report directly to SAPL (not IXSVC_ErrorClbk() as this is
         part of the SafetyOpen checks) */
      SAPL_CssErrorClbk(IXSVC_k_NFSE_RXE_MCASTRC_RPI_TO,
                        ps_sOpenPar->u16_cnxnPointProd, IXSER_k_A_NOT_USED);
    }
    else  /* else: T->O RPI is ok */
    {
      /* the RPI specified in the Forward_Open was acceptable
         (the Target to Originator RPI value is ignored). */
    }

    /* write RPIs of the existing connection into the Extended Status */
    ps_sOpenRespStat->au16_moreExt[1] =
      (CSS_t_UINT)(kps_initCalc->u32_rpiOT_us & 0x0000FFFFU);
    ps_sOpenRespStat->au16_moreExt[2] =
      (CSS_t_UINT)((kps_initCalc->u32_rpiOT_us >> 16U) & 0x0000FFFFU);
    ps_sOpenRespStat->au16_moreExt[3] =
      (CSS_t_UINT)(kps_initCalc->u32_rpiTO_us & 0x0000FFFFU);
    ps_sOpenRespStat->au16_moreExt[4] =
      (CSS_t_UINT)((kps_initCalc->u32_rpiTO_us >> 16U) & 0x0000FFFFU);
  }
  else  /* else: RPIs are identical */
  {
    /* all parameters identical -> reconnect */
    o_retVal = CSS_k_TRUE;
  }

  return (o_retVal);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


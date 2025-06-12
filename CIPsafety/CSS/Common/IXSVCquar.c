/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCquar.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the functions to handle the quarantine Timer
**             of a Safety Validator Client instance.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_QuarLocalsClear
**             IXSVC_QuarConsStart
**             IXSVC_QuarConsUpdate
**             IXSVC_QuarSoftErrByteGet
**             IXSVC_QuarSoftErrVarGet
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

#include "IXUTL.h"

#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* As a consumer number of a failed consumer may not be re-used immediately
   to prevent that 2 consumers might use the same consumer number we have
   to start a timer when a Multi-cast consumer fails. */
static CSS_t_UDINT aau32_quarantineTimer_128us[CSOS_cfg_NUM_OF_SV_CLIENTS]
                                              [CSOS_k_MAX_CONSUMER_NUM_MCAST];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_QuarLocalsClear(CSS_t_UINT u16_svcIdx)
{
  CSS_MEMSET(aau32_quarantineTimer_128us[u16_svcIdx], 0,
             sizeof(aau32_quarantineTimer_128us[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                         CSS_t_USINT u8_consNum)
{
  /* local variables to make complex calculations easier */
  CSS_t_UDINT u32_temp;
  CSS_t_USINT u8_temp;
  /* pointer to the initially calculated values of this instance (EPI and
     Ping_Interval_EPI_Multiplier is needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);
  /* get a pointer to per consumer data */
  const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
    IXSVC_InstInfoPerConsGet(u16_svcIdx, (CSS_t_USINT)(u8_consNum - 1U));

  /* Quarantine will end Timeout_Multiplier.PI+2 ping intervals from now */
  /* (see FRS325) */
  /* overrun can not occur due to previous range checks of the used variables */
  u8_temp = (CSS_t_USINT)(kps_iiperCons->s_Timeout_Multiplier.u8_PI + 2U);
  u32_temp = ((kps_initCalc->u32_EPI_us
             * kps_initCalc->u16_Ping_Interval_EPI_Multiplier)
             / CSS_k_TICK_BASE_US) + 1U;

  u32_temp = u32_temp * u8_temp;

  aau32_quarantineTimer_128us[u16_svcIdx][u8_consNum - 1U] = IXUTL_GetSysTime()
                                                           + (u32_temp);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
CSS_t_USINT IXSVC_QuarConsUpdate(CSS_t_UINT u16_svcIdx)
{
  /* return value of this function */
  CSS_t_USINT u8_retCons = CSOS_k_MAX_USINT;
  /* loop counter */
  CSS_t_USINT u8_cIdx = 0U;

  /* if this instance is in Initializing or Established state */
  if (IXSVC_StateIsInitOrEstablished(u16_svcIdx))
  {
    /* check if there are consumers in quarantine */
    do
    {
      /* get a pointer to per consumer data */
      const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
        IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_cIdx);

      /* if currently checked consumer is open and faulted */
      if (    (IXSVC_ConnectionIsFaulted(u16_svcIdx, u8_cIdx))
           && (kps_iiperCons->o_Consumer_Open)
         )
      {
        /* this consumer is under quarantine */
        /* if quarantine time is over */
        if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(),
              aau32_quarantineTimer_128us[u16_svcIdx][u8_cIdx]))
        {
          /* consumer can be deleted now (by the caller) */
          u8_retCons = u8_cIdx + 1U;
        }
        else /* else: time not yet over */
        {
          /* consumer must remain in quarantine */
        }
      }
      else /* else: this consumer is not opened or not faulted */
      {
        /* Not of interest here. */
      }

      u8_cIdx++;
    }
    while (    (u8_cIdx < IXSVC_InitInfoMaxConsNumGet(u16_svcIdx))
            && (u8_retCons == CSOS_k_MAX_USINT)
          );
  }
  else  /* else: state is Idle or Faulted */
  {
    /* Quarantining not active as only a complete connection re-initialization
       can bring us back to established */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_retCons);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSVC_QuarSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(aau32_quarantineTimer_128us))
  {
    *pb_var = *(((CSS_t_BYTE*)aau32_quarantineTimer_128us)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(aau32_quarantineTimer_128us);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVC_QuarSoftErrVarGet(
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

    /* if level 1 counter is larger than number of array elements */
    if (au32_cnt[1U] >= CSOS_k_MAX_CONSUMER_NUM_MCAST)
    {
      /* level 1 counter at/above end */
      /* default return values already set */
    }
    else /* level 1 counter in range of array elements */
    {
      CSS_H2N_CPY32(pb_var, &aau32_quarantineTimer_128us[au32_cnt[0U]][au32_cnt[1U]]);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


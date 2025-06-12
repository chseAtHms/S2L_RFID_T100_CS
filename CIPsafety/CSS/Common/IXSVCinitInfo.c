/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCinitInfo.c
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
**  Functions: IXSVC_InitInfoClear
**             IXSVC_InitInfoSet
**             IXSVC_InitInfoMsgFrmtGet
**             IXSVC_InitInfoMaxConsNumGet
**             IXSVC_InitInfoCnxnPointGet
**             IXSVC_InitInfoCalcPtrGet
**             IXSVC_InitInfoSoftErrByteGet
**             IXSVC_InitInfoSoftErrVarGet
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

/* array of structs to store data for each instance */
static IXSVC_t_INIT_INFO as_InitInfo[CSOS_cfg_NUM_OF_SV_CLIENTS];

/* minimum and maximum values for the EPI according to Vol. 5 Chapt. 2-4.5.2.1
   in micro seconds */
#define k_EPI_MIN_US   100U
#define k_EPI_MAX_US   1000000U

#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/* define IDs of the variables depending on which variables are supported */
enum
{
   k_SOFT_ERR_VAR_RPIOT
  ,k_SOFT_ERR_VAR_RPITO
  ,k_SOFT_ERR_VAR_EPIUS
  ,k_SOFT_ERR_VAR_EPI128US
  ,k_SOFT_ERR_VAR_EPIDIVUS
  ,k_SOFT_ERR_VAR_PIEM
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_PIDCRCS1
#endif
  ,k_SOFT_ERR_VAR_PIDCRCS3
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_PIDCRCS5
#endif
  ,k_SOFT_ERR_VAR_PDS
};
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_InitInfoClear(CSS_t_UINT u16_svcIdx)
{
  /* completely erase the Init Info structure for this instance */
  CSS_MEMSET(&as_InitInfo[u16_svcIdx], 0, sizeof(as_InitInfo[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                       const IXSVC_t_INIT_INFO *ps_initInfo)
{
  /* if EPI out of range */
  if (    (ps_initInfo->s_initCalc.u32_EPI_us < k_EPI_MIN_US)
       || (ps_initInfo->s_initCalc.u32_EPI_us > k_EPI_MAX_US)
     )
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_EPI_RANGE,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    ps_initInfo->s_initCalc.u32_EPI_us);
  }
  /* else: EPI is ok, if produced data size is out of range */
  else if (
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)            /* CCT_NO_PRE_WARNING */
               (ps_initInfo->s_initCalc.u8_prodDataSize
                < CSS_k_SHORT_FORMAT_MIN_LENGTH)
#else                                                   /* CCT_NO_PRE_WARNING */
               (ps_initInfo->s_initCalc.u8_prodDataSize
                < CSS_k_LONG_FORMAT_MIN_LENGTH)
#endif
            ||
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)             /* CCT_NO_PRE_WARNING */
               (ps_initInfo->s_initCalc.u8_prodDataSize
                 > CSOS_cfg_LONG_FORMAT_MAX_LENGTH)
#else                                                   /* CCT_NO_PRE_WARNING */
               (ps_initInfo->s_initCalc.u8_prodDataSize
                > CSS_k_SHORT_FORMAT_MAX_LENGTH)
#endif
          )
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_INC_DATA_SIZE_RANGE,
                    IXSVC_InstIdFromIdxGet(u16_svcIdx),
                    (CSS_t_UDINT)ps_initInfo->s_initCalc.u8_prodDataSize);
  }
  /* else: data size ok */
  else
  {
    /* assign passed values to elements of local arrays */
    as_InitInfo[u16_svcIdx] = *ps_initInfo;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InitInfoMsgFrmtGet
**
** Description : This function returns the message format of the requested
**               Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE      - message format of the requested Safety
**                                 Validator Client Instance
**
*******************************************************************************/
CSS_t_BYTE IXSVC_InitInfoMsgFrmtGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svcIdx].b_msgFormat);
}


/*******************************************************************************
**
** Function    : IXSVC_InitInfoMaxConsNumGet
**
** Description : This function returns the Maximum Consumer Number of the
**               requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_USINT     - Maximum Consumer Number of the requested
**                                 Safety Validator Client Instance
**
*******************************************************************************/
CSS_t_USINT IXSVC_InitInfoMaxConsNumGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svcIdx].u8_Max_Consumer_Number);
}


/*******************************************************************************
**
** Function    : IXSVC_InitInfoCnxnPointGet
**
** Description : This function returns the Connection Point which produces the
**               data to be sent by the requested Safety Validator Client.
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVC_TargClientInit(), IXSVC_InstInfoGet(),
**                                 IXSVC_InstTxFrameGenerate(),
**                                 IXSVC_McastCnxnFind and
**                                 IXSVO_ExplMsgHandler())
**
** Returnvalue : CSS_t_UINT      - Connection Point associated with the
**                                 requested Safety Validator Client
**
*******************************************************************************/
CSS_t_UINT IXSVC_InitInfoCnxnPointGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svcIdx].u16_ProdCnxnPoint);
}


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
const IXSVC_t_INIT_CALC* IXSVC_InitInfoCalcPtrGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&as_InitInfo[u16_svcIdx].s_initCalc);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVC_InitInfoSoftErrByteGet
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
CSS_t_UDINT IXSVC_InitInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                         CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_InitInfo))
  {
    *pb_var = *(((CSS_t_BYTE*)as_InitInfo)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_InitInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVC_InitInfoSoftErrVarGet
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
void IXSVC_InitInfoSoftErrVarGet(
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
        CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].b_msgFormat);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 1U:
      {
        CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].u8_Max_Consumer_Number);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].u16_ProdCnxnPoint);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 3U:
      {
        /* indicate to caller that we are in range of level 2 */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case k_SOFT_ERR_VAR_RPIOT:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_rpiOT_us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_VAR_RPITO:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_rpiTO_us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_VAR_EPIUS:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_EPI_us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_VAR_EPI128US:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_EPI_128us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_VAR_EPIDIVUS:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_EpiDeviation_us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_VAR_PIEM:
          {
            CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u16_Ping_Interval_EPI_Multiplier);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

        #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_PIDCRCS1:
          {
            CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u8_pidCrcS1);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
            break;
          }
        #endif

          case k_SOFT_ERR_VAR_PIDCRCS3:
          {
            CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u16_pidCrcS3);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_PIDCRCS5:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_pidCrcS5);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }
        #endif

          case k_SOFT_ERR_VAR_PDS:
          {
            CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u8_prodDataSize);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
          }
          break;
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


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


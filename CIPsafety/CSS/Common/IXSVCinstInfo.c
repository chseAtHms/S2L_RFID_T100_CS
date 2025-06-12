/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCinstInfo.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains Per Consumer Instance Information and
**             functions to access this data.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_InstInfoClear
**             IXSVC_InstInfoPerConsSet
**             IXSVC_InstInfoPerConsGet
**             IXSVC_InstInfoTimeoutMultGet
**             IXSVC_InstInfoConsumerClose
**             IXSVC_InstInfoSoftErrByteGet
**             IXSVC_InstInfoSoftErrVarGet
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

/** aas_InstInfoPerCons:
    array of array of structs containing 'per consumer' information of the
    Safety Validator Client instances
*/
static IXSVC_t_INST_INFO_PER_CONS
  aas_InstInfoPerCons[CSOS_cfg_NUM_OF_SV_CLIENTS]
                     [CSOS_k_MAX_CONSUMER_NUM_MCAST];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVC_InstInfoClear(CSS_t_UINT u16_svcIdx)
{
  /* erase local variables of this module */
  CSS_MEMSET(aas_InstInfoPerCons[u16_svcIdx], 0,
             sizeof(aas_InstInfoPerCons[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                              const IXSVC_t_INST_INFO_PER_CONS *ps_iiPerCons)
{
  /* copy the data from the passed pointer */
  aas_InstInfoPerCons[u16_svcIdx][u8_consIdx] = *ps_iiPerCons;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
                                                        CSS_t_USINT u8_consIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&aas_InstInfoPerCons[u16_svcIdx][u8_consIdx]);
}


/*******************************************************************************
**
** Function    : IXSVC_InstInfoTimeoutMultGet
**
** Description : This function returns the value of the Timeout Multiplier of
**               the addressed instance's consumer.
**
** Parameters  : u16_svcIdx (IN) - index of Safety Validator Client structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**               u8_consIdx (IN) - Consumer Index (0..Max_Consumer_Number-1)
**                                 (not checked, checked in
**                                 TimeoutMultAttribGet())
**
** Returnvalue : CSS_t_USINT     - Timeout Multiplier
**
*******************************************************************************/
CSS_t_USINT IXSVC_InstInfoTimeoutMultGet(CSS_t_UINT u16_svcIdx,
                                         CSS_t_USINT u8_consIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  {
    /* mute lint warning "Return statement before end of function" */
    return (aas_InstInfoPerCons[u16_svcIdx][u8_consIdx].         /*lint !e904 */
            s_Timeout_Multiplier.u8_ef);
  }
#else
  {
    return (aas_InstInfoPerCons[u16_svcIdx][u8_consIdx].
            s_Timeout_Multiplier.u8_PI);
  }
#endif
/* The function has just one return point (preprocessor 'if') */
/* RSM_IGNORE_QUALITY_BEGIN Notice #27 - Number of function return points > 1 */
}
/* RSM_IGNORE_QUALITY_END */


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
                                 CSS_t_USINT u8_consIdx)
{
  aas_InstInfoPerCons[u16_svcIdx][u8_consIdx].o_Consumer_Open = CSS_k_FALSE;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVC_InstInfoSoftErrByteGet
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
CSS_t_UDINT IXSVC_InstInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                         CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(aas_InstInfoPerCons))
  {
    *pb_var = *(((CSS_t_BYTE*)aas_InstInfoPerCons)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(aas_InstInfoPerCons);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVC_InstInfoSoftErrVarGet
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
void IXSVC_InstInfoSoftErrVarGet(
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
      /* indicate to caller that we are in range of level 2 or below */
      ps_retStat->u8_incLvl = 2U;

      /* select the level 2 variable */
      switch (au32_cnt[2U])
      {
        case 0U:
        {
          CSS_H2N_CPY8(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].o_Consumer_Open);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BOOL;
          break;
        }

        case 1U:
        {
          /* indicate to caller that we are in range of level 3 or below */
          ps_retStat->u8_incLvl = 3U;

          /* select the level 3 variable */
          switch (au32_cnt[3U])
          {
            case 0U:
            {
              CSS_H2N_CPY32(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].s_cid.u32_devSerNum);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
              break;
            }

            case 1U:
            {
              CSS_H2N_CPY16(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].s_cid.u16_vendId);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

            case 2U:
            {
              CSS_H2N_CPY16(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].s_cid.u16_cnxnSerNum);
              ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
              break;
            }

            default:
            {
              /* level 3 counter at/above end */
              /* default return values already set */
            }
            break;
          }
          break;
        }

        case 2U:
        {
          CSS_H2N_CPY8(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].s_Timeout_Multiplier.u8_PI);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
          break;
        }

      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        case 3U:
        {
          CSS_H2N_CPY8(pb_var, &aas_InstInfoPerCons[au32_cnt[0U]][au32_cnt[1U]].s_Timeout_Multiplier.u8_ef);
          ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
          break;
        }
      #endif

        default:
        {
          /* level 2 counter at/above end */
          /* default return values already set */
        }
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


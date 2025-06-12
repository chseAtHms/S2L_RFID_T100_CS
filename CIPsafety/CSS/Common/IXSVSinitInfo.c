/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSinitInfo.c
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
**  Functions: IXSVS_InitInfoSet
**             IXSVS_InitInfoClear
**             IXSVS_InitInfoMsgFrmtGet
**             IXSVS_InitInfoCalcPtrGet
**             IXSVS_InitInfoCnxnPointGet
**             IXSVS_InitInfoNetTimeExpMultGet
**             IXSVS_InitInfoSoftErrByteGet
**             IXSVS_InitInfoSoftErrVarGet
**
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

#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #include "CSOSapi.h"
#endif

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVS.h"
#include "IXSVSint.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* array of structs to store data for each instance */
static IXSVS_t_INIT_INFO as_InitInfo[CSOS_cfg_NUM_OF_SV_SERVERS];


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  /* define IDs of the variables depending on which variables are supported */
  enum
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
     k_SOFT_ERR_OPT_VAR_IC_PIDCRCS5
    ,k_SOFT_ERR_OPT_VAR_IC_CIDCRCS5,
  #endif
     k_SOFT_ERR_VAR_IC_PID
    ,k_SOFT_ERR_VAR_IC_PIDCRCS3
    ,k_SOFT_ERR_VAR_IC_CIDCRCS3
    ,k_SOFT_ERR_VAR_IC_NTEM
    ,k_SOFT_ERR_VAR_IC_CN
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    ,k_SOFT_ERR_OPT_VAR_IC_MFN
  #endif
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    ,k_SOFT_ERR_OPT_VAR_IC_PIDCRCS1
  #endif
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
                       const IXSVS_t_INIT_INFO *ps_initInfo)
{
  /* assign passed values to elements of local arrays */
  as_InitInfo[u16_svsIdx] = *ps_initInfo;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


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
void IXSVS_InitInfoClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Init Info structure for this instance */
  CSS_MEMSET(&as_InitInfo[u16_svsIdx], 0, sizeof(as_InitInfo[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InitInfoMsgFrmtGet
**
** Description : This function returns the message format of the requested
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_BYTE - message format of the requested Safety Validator
**                            Server Instance
**
*******************************************************************************/
CSS_t_BYTE IXSVS_InitInfoMsgFrmtGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svsIdx].b_msgFormat);
}


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
const IXSVS_t_INIT_CALC* IXSVS_InitInfoCalcPtrGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&as_InitInfo[u16_svsIdx].s_initCalc);
}


/*******************************************************************************
**
** Function    : IXSVS_InitInfoCnxnPointGet
**
** Description : This function returns the Connection Point which consumes the
**               data received by the requested Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server
**                                 structure
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - Connection Point associated with the
**                                 requested Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_InitInfoCnxnPointGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svsIdx].u16_consCnxnPoint);
}


/*******************************************************************************
**
** Function    : IXSVS_InitInfoNetTimeExpMultGet
**
** Description : This function returns the value of the Network Time Expectation
**               Multiplier of the addressed instance.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in
**                                 IXSVS_TargServerInit() and
**                                 IXSVS_OrigServerInit())
**
** Returnvalue : CSS_t_UINT      - Network Time Expectation Multiplier
**                                 (128us ticks)
**
*******************************************************************************/
CSS_t_UINT IXSVS_InitInfoNetTimeExpMultGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InitInfo[u16_svsIdx].s_initCalc.
          u16_Network_Time_Expectation_Multiplier_128us);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_InitInfoSoftErrByteGet
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
CSS_t_UDINT IXSVS_InitInfoSoftErrByteGet(CSS_t_UDINT u32_varCnt,
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
** Function    : IXSVS_InitInfoSoftErrVarGet
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
void IXSVS_InitInfoSoftErrVarGet(
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
      case 0U:
      {
        /* indicate to caller that we are in range of level 2 */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_IC_PIDCRCS5:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_pidCrcS5);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case k_SOFT_ERR_OPT_VAR_IC_CIDCRCS5:
          {
            CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u32_cidCrcS5);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }
        #endif

          case k_SOFT_ERR_VAR_IC_PID:
          {
            /* indicate to caller that we are in range of level 3 */
            ps_retStat->u8_incLvl = 3U;

            /* select the level 2 variable */
            switch (au32_cnt[3U])
            {
              case 0U:
              {
                CSS_H2N_CPY32(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.s_pid.u32_devSerNum);
                ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
                break;
              }

              case 1U:
              {
                CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.s_pid.u16_vendId);
                ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
                break;
              }

              case 2U:
              {
                CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.s_pid.u16_cnxnSerNum);
                ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
                break;
              }

              default:
              {
                /* level 3 counter at/above end */
                ps_retStat->u8_cpyLen = 0U;
                break;
              }
            }
            break;
          }

          case k_SOFT_ERR_VAR_IC_PIDCRCS3:
          {
            CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u16_pidCrcS3);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case k_SOFT_ERR_VAR_IC_CIDCRCS3:
          {
            CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u16_cidCrcS3);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case k_SOFT_ERR_VAR_IC_NTEM:
          {
            CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u16_Network_Time_Expectation_Multiplier_128us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case k_SOFT_ERR_VAR_IC_CN:
          {
            CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u8_Consumer_Num);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
            break;
          }

        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_IC_MFN:
          {
            CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u8_Max_Fault_Number);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
            break;
          }
        #endif

        #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_IC_PIDCRCS1:
          {
            CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].s_initCalc.u8_pidCrcS1);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
            break;
          }
        #endif

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
            break;
          }
        }
        break;
      }

      case 1U:
      {
        CSS_H2N_CPY16(pb_var, &as_InitInfo[au32_cnt[0U]].u16_consCnxnPoint);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY8(pb_var, &as_InitInfo[au32_cnt[0U]].b_msgFormat);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
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


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


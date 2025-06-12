/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSinstId.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains utility functions for the managing the
**             instance <-> index relationship of the Server Safety Validator
**             instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_InstIdInit
**             IXSVS_InstIdFromIdxGet
**             IXSVS_InstIdIdxAssign
**             IXSVS_InstIdIdxStop
**             IXSVS_InstIdSoftErrByteGet
**             IXSVS_InstIdSoftErrVarGet
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

#include "CSStypes.h"
#include "CSOSapi.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSVD.h"
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

/** au16_instId:
    Lookup table storing the Safety Validator Instance ID of all instances
*/
static CSS_t_UINT au16_instIdTbl[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_InstIdInit
**
** Description : This function initializes the lookup table for the Safety
**               Validator Instance Ids
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdInit(void)
{
  CSS_t_UINT u16_i;

  /* Completely erase the Safety Validator Instance ID Lookup Table entry */
  for (u16_i=0U; u16_i < CSOS_cfg_NUM_OF_SV_SERVERS; u16_i++)
  {
    au16_instIdTbl[u16_i] = CSOS_k_INVALID_INSTANCE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstIdFromIdxGet
**
** Description : This function converts a Safety Validator Server array index
**               into the corresponding Instance ID.
**
** Parameters  : u16_svsIdx (IN) - array index of the Safety Validator Server
**                                 array
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - instance ID of the Safety Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVS_InstIdFromIdxGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (au16_instIdTbl[u16_svsIdx]);
}


/*******************************************************************************
**
** Function    : IXSVS_InstIdIdxAssign
**
** Description : This function assigns the Safety Validator Server index and
**               Instance ID dependency.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXCCO_CnxnOpen() and
**                                 IXSVO_TargAlloc())
**               u16_svsIdx (IN) - array index of the Safety Validator Server
**                                 array
**                                 (not checked, checked in IXSVO_OrigAlloc()
**                                 IXSVO_TargAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdIdxAssign(CSS_t_UINT u16_instId, CSS_t_UINT u16_svsIdx)
{
  /* if update of the lookup table is successful */
  if (IXSVD_SvIndexAssign(u16_instId, u16_svsIdx, CSS_k_TRUE))
  {
    au16_instIdTbl[u16_svsIdx] = u16_instId;
  }
  else /* else: error in instance ID or index */
  {
    /* already reported to SAPL by IXSVD_SvIndexAssign() */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstIdIdxStop
**
** Description : This function stops/deletes the passed Safety Validator Server
**               index and Instance dependency.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server Instance
**                                 (not checked, checked in
**                                 IXSVO_InstanceStop())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstIdIdxStop(CSS_t_UINT u16_svsIdx)
{
  /* if update of the lookup table is successful */
  if (IXSVD_SvIndexDelete(au16_instIdTbl[u16_svsIdx]))
  {
    au16_instIdTbl[u16_svsIdx] = CSOS_k_INVALID_INSTANCE;
  }
  else /* else: error in instance ID */
  {
    /* already reported to SAPL by IXSVD_SvIndexDelete() */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_InstIdSoftErrByteGet
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
CSS_t_UDINT IXSVS_InstIdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = CSS_k_FALSE;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(au16_instIdTbl))
  {
    *pb_var = *(((CSS_t_BYTE*)au16_instIdTbl)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(au16_instIdTbl);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVS_InstIdSoftErrVarGet
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
void IXSVS_InstIdSoftErrVarGet(
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
    CSS_H2N_CPY16(pb_var, &au16_instIdTbl[au32_cnt[0U]]);
    ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


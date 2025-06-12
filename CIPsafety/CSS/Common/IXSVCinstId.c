/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCinstId.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains utility functions for the managing the
**             instance <-> index relationship of the Client Safety Validator
**             instances.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_InstIdInit
**             IXSVC_InstIdFromIdxGet
**             IXSVC_InstIdIdxAssign
**             IXSVC_InstIdIdxStop
**             IXSVC_InstIdSoftErrByteGet
**             IXSVC_InstIdSoftErrVarGet
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

#include "IXSVD.h"
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

/** au16_instId:
    Lookup table storing the Safety Validator Instance ID of all instances
*/
static CSS_t_UINT au16_instIdTbl[CSOS_cfg_NUM_OF_SV_CLIENTS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_InstIdInit
**
** Description : This function initializes the lookup table for the Safety
**               Validator Client Instance Ids
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstIdInit(void)
{
  CSS_t_UINT u16_i;

  /* Completely erase the Safety Validator Instance ID Lookup Table entry */
  for (u16_i=0U; u16_i < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_i++)
  {
    au16_instIdTbl[u16_i] = CSOS_k_INVALID_INSTANCE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstIdFromIdxGet
**
** Description : This function converts a Safety Validator Client array index
**               into the corresponding Instance ID.
**
** Parameters  : u16_svcIdx (IN) - array index of the Safety Validator Client
**                                 array
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_t_UINT      - instance ID of the Client Validator Server
**
*******************************************************************************/
CSS_t_UINT IXSVC_InstIdFromIdxGet(CSS_t_UINT u16_svcIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (au16_instIdTbl[u16_svcIdx]);
}


/*******************************************************************************
**
** Function    : IXSVC_InstIdIdxAssign
**
** Description : This function assigns the Safety Validator Client index and
**               Instance ID dependency.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXCCO_CnxnOpen()
**                                 and IXSVO_TargAlloc())
**               u16_svcIdx (IN) - array index of the Safety Validator Client
**                                 array
**                                 (not checked, checked in IXSVO_OrigAlloc()
**                                 and IXSVO_TargAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstIdIdxAssign(CSS_t_UINT u16_instId,
                           CSS_t_UINT u16_svcIdx)
{
  /* if update of the lookup table is successful */
  if (IXSVD_SvIndexAssign(u16_instId, u16_svcIdx, CSS_k_FALSE))
  {
    /* store the passed instance ID into the struct's field */
    au16_instIdTbl[u16_svcIdx] = u16_instId;
  }
  else /* else: error in instance ID or index */
  {
    /* already reported to SAPL */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstIdIdxStop
**
** Description : This function stops/deletes the passed Safety Validator Client
**               (which includes all consumers).
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client Instance
**                                 (not checked, checked in
**                                 IXSVC_OrigClientInit(), IXSVC_InstClose() and
**                                 IXSVO_InstanceStop())
**
** Returnvalue : CSS_k_TRUE      - stopping was successful
**               CSS_k_FALSE     - error (error already reported to SAPL)
**
*******************************************************************************/
CSS_t_BOOL IXSVC_InstIdIdxStop(CSS_t_UINT u16_svcIdx)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if the connection is idle */
  if (IXSVC_StateGet(u16_svcIdx) == CSS_k_SV_STATE_IDLE)
  {
    /* the connection is not open - so no reason to execute the state machine */
  }
  else /* else: connection is in one of the other states */
  {
    /* execute the state machine (so that SAPL will be informed),
       consumer number not relevant here */
    IXSVC_StateMachine(u16_svcIdx, CSOS_k_CNUM_INVALID,
                       IXSVC_k_VE_CLIENT_CLOSE);
  }

  /* if update of the lookup table is successful */
  if (IXSVD_SvIndexDelete(au16_instIdTbl[u16_svcIdx]))
  {
    au16_instIdTbl[u16_svcIdx] = CSOS_k_INVALID_INSTANCE;
    o_retVal = CSS_k_TRUE;
  }
  else /* else: error in instance ID */
  {
    /* already reported to SAPL */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVC_InstIdSoftErrByteGet
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
CSS_t_UDINT IXSVC_InstIdSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

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
** Function    : IXSVC_InstIdSoftErrVarGet
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
void IXSVC_InstIdSoftErrVarGet(
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
    CSS_H2N_CPY16(pb_var, &au16_instIdTbl[au32_cnt[0U]]);
    ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


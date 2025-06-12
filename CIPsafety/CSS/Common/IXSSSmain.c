/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSSmain.c
**    Summary: IXSSS - Safety Stack Status
**             This module controls the operating state of the CIP Safety Stack.
**             All other units can obtain the current state from this unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSS_StateSet
**             IXSSS_StateGet
**             IXSSS_MainSoftErrByteGet
**             IXSSS_MainSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #include "CSOSapi.h"
#endif

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #include "CSS.h"
#endif

#include "IXSCF.h"

#include "IXSSS.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** IXSSS_e_InitState:
    This enum represents the state of the CIP Safety Stack.
*/
static IXSSS_t_STATE e_State = IXSSS_k_ST_NON_EXIST;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSS_StateSet
**
** Description : This function sets the state variable to the passed value.
**
** Parameters  : e_state (IN) - Global CIP Safety Stack state that is to be set.
**                              See definition of {IXSSS_t_STATE}.
**                              (not checked, only called with constants)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSS_StateSet(IXSSS_t_STATE e_state)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  e_State = e_state;
}


/*******************************************************************************
**
** Function    : IXSSS_StateGet
**
** Description : This function returns the global state of the CIP Safety Stack.
**
** Parameters  : -
**
** Returnvalue : Global CIP Safety Stack state.  See definition of
**               {IXSSS_t_STATE}.
**
*******************************************************************************/
IXSSS_t_STATE IXSSS_StateGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (e_State);
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSSS_MainSoftErrByteGet
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
CSS_t_UDINT IXSSS_MainSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(e_State))
  {
    *pb_var = *(((CSS_t_BYTE*)&e_State)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(e_State);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSSS_MainSoftErrVarGet
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
void IXSSS_MainSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* there is only one variable so this is the only variable in level 1 */
  /* if level counter is larger than number of variables */
  if (au32_cnt[0U] > 0U)
  {
    /* return value already set */
  }
  else
  {
    /* convert enum to something where we know the size on all platforms */
    CSS_t_UDINT u32_tmp = (CSS_t_UDINT)e_State;
    CSS_H2N_CPY32(pb_var, &u32_tmp);
    ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
  }
}
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


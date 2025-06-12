/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSSident.c
**    Summary: IXSSS - Safety Stack Status
**             This module stores the Identity Information of the device and.
**             provides functions to the other units for reading this
**             information.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSS_IdentityInfoStore
**             IXSSS_VendorIdGet
**             IXSSS_DeviceTypeGet
**             IXSSS_ProductCodeGet
**             IXSSS_MajorRevGet
**             IXSSS_MinorRevGet
**             IXSSS_SerialNumGet
**             IXSSS_NodeIdGet
**             IXSSS_PortNumArrayPtrGet
**             IXSSS_IdentSoftErrByteGet
**             IXSSS_IdentSoftErrVarGet
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

/** IXSSS_s_IdInfo:
    This structure stores the identity information of the stack. It is set
    during initialization and makes this information available to other units of
    the stack by providing the IXSSS_xxxGet() functions.
*/
static CSS_t_ID_INFO s_IdInfo;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSS_IdentityInfoStore
**
** Description : This function stores the passed identity information (at
**               initialization time). This information will then be available
**               to the other units of the stack.
**
** Parameters  : ps_idInfo (IN) - pointer to structure of type {IXSSS_t_ID_INFO}
**                                which must contain the identity information of
**                                the stack.
**                                (not checked, only called with reference to
**                                struct)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSS_IdentityInfoStore(const CSS_t_ID_INFO *ps_idInfo)
{
  /* loop counter for copying the NodeID of all safety ports */
  CSS_t_USINT u8_portIdx;

  /* make sure the whole array of structures is initialized (also padding bytes)
     because of Soft Error Checking) */
  CSS_MEMSET(&s_IdInfo, 0, sizeof(s_IdInfo));

  /* copy passed information to module global static struct variable */
  /* copy element by element (instead of structure assignment or memory copy)
     to make sure padding bytes in the static structure remain zero */
  s_IdInfo.u32_serNum        = ps_idInfo->u32_serNum;
  s_IdInfo.u16_vendorId      = ps_idInfo->u16_vendorId;
  s_IdInfo.u16_devType       = ps_idInfo->u16_devType;
  s_IdInfo.u16_prodCode      = ps_idInfo->u16_prodCode;
  s_IdInfo.u8_majorRev       = ps_idInfo->u8_majorRev;
  s_IdInfo.u8_minorRev       = ps_idInfo->u8_minorRev;

  /* copy all Node IDs (one for each port) and port Numbers */
  for (u8_portIdx = 0U; u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS; u8_portIdx++)
  {
    s_IdInfo.au32_nodeId[u8_portIdx] = ps_idInfo->au32_nodeId[u8_portIdx];
    #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    {
      s_IdInfo.au16_portNumbers[u8_portIdx] =
        ps_idInfo->au16_portNumbers[u8_portIdx];
    }
    #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSSS_VendorIdGet
**
** Description : This function returns the Vendor ID of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - our Vendor ID
**
*******************************************************************************/
CSS_t_UINT IXSSS_VendorIdGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u16_vendorId);
}


/*******************************************************************************
**
** Function    : IXSSS_DeviceTypeGet
**
** Description : This function returns the Device Type of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - our Device Type
**
*******************************************************************************/
CSS_t_UINT IXSSS_DeviceTypeGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u16_devType);
}


/*******************************************************************************
**
** Function    : IXSSS_ProductCodeGet
**
** Description : This function returns the Product Code of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - our Product Code
**
*******************************************************************************/
CSS_t_UINT IXSSS_ProductCodeGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u16_prodCode);
}


/*******************************************************************************
**
** Function    : IXSSS_MajorRevGet
**
** Description : This function returns the Major Revision of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_USINT - our Major Revision
**
*******************************************************************************/
CSS_t_USINT IXSSS_MajorRevGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u8_majorRev);
}


/*******************************************************************************
**
** Function    : IXSSS_MinorRevGet
**
** Description : This function returns the Minor Revision of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_USINT - our Minor Revision
**
*******************************************************************************/
CSS_t_USINT IXSSS_MinorRevGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u8_minorRev);
}


/*******************************************************************************
**
** Function    : IXSSS_SerialNumGet
**
** Description : This function returns the Serial Number of this device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UDINT - our Serial Number
**
*******************************************************************************/
CSS_t_UDINT IXSSS_SerialNumGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.u32_serNum);
}


/*******************************************************************************
**
** Function    : IXSSS_NodeIdGet
**
** Description : This function returns one of the NodeIDs of this device.
**
** Parameters  : u8_portIdx (IN) - index of the requested NodeID in the internal
**                                 table
**                                 (not checked, only called with values from 
**                                 0..CSOS_cfg_NUM_OF_SAFETY_PORTS-1 loop)
**
** Returnvalue : CSS_t_UDINT - requested NodeID
**
*******************************************************************************/
CSS_t_UDINT IXSSS_NodeIdGet(CSS_t_USINT u8_portIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.au32_nodeId[u8_portIdx]);
}


#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
/*******************************************************************************
**
** Function    : IXSSS_PortNumArrayPtrGet
**
** Description : This function returns a pointer to the array containing the
**               Port Numbers of the device.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT* - Pointer to array with port numbers. Array has
**                             CSOS_cfg_NUM_OF_SAFETY_PORTS members
**
*******************************************************************************/
CSS_t_UINT* IXSSS_PortNumArrayPtrGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (s_IdInfo.au16_portNumbers);
}
#endif


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSSS_IdentSoftErrByteGet
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
CSS_t_UDINT IXSSS_IdentSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                      CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(s_IdInfo))
  {
    *pb_var = *(((CSS_t_BYTE*)&s_IdInfo)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(s_IdInfo);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSSS_IdentSoftErrVarGet
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
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSSS_IdentSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* select the level 0 variable */
  switch (au32_cnt[0U])
  {
    case 0U:
    {
      CSS_H2N_CPY32(pb_var, &s_IdInfo.u32_serNum);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      break;
    }

    case 1U:
    {
      /* indicate to caller that we are in range of level 1 or below */
       ps_retStat->u8_incLvl = 1U;

       /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SAFETY_PORTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY32(pb_var, &s_IdInfo.au32_nodeId[au32_cnt[1U]]);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      }
      break;
    }

    case 2U:
    {
      CSS_H2N_CPY16(pb_var, &s_IdInfo.u16_vendorId);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case 3U:
    {
      CSS_H2N_CPY16(pb_var, &s_IdInfo.u16_devType);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case 4U:
    {
      CSS_H2N_CPY16(pb_var, &s_IdInfo.u16_prodCode);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case 5U:
    {
      CSS_H2N_CPY8(pb_var, &s_IdInfo.u8_majorRev);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
      break;
    }

    case 6U:
    {
      CSS_H2N_CPY8(pb_var, &s_IdInfo.u8_minorRev);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
      break;
    }

  #if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
    case 7U:
    {
      /* indicate to caller that we are in range of level 1 or below */
       ps_retStat->u8_incLvl = 1U;

       /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSOS_cfg_NUM_OF_SAFETY_PORTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY16(pb_var, &s_IdInfo.au16_portNumbers[au32_cnt[1U]]);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      }
      break;
    }
  #endif

    default:
    {
      /* default return values already set */
      break;
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSS.h
**    Summary: IXSSS - Safety Stack Status
**             IXSSS.h is the export header file of the IXSSS unit.
**             This unit controls the operating state of the CIP Safety Stack.
**             All other units can obtain the current state from this unit. It
**             also stores the Identity Information of the device.
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
**             IXSSS_StateSet
**             IXSSS_StateGet
**             IXSSS_IdentSoftErrByteGet
**             IXSSS_IdentSoftErrVarGet
**             IXSSS_MainSoftErrByteGet
**             IXSSS_MainSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSS_H
#define IXSSS_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/* Indicator for an invalid Node ID */
#define IXSSS_k_INVALID_NODEID          0xFFFFFFFFUL


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSSS_t_STATE:
    Definition of the states of the IXSSS state machine.
*/
typedef enum
{
  IXSSS_k_ST_NON_EXIST,       /* after power up: Non-Existent */
  IXSSS_k_ST_RUNNING          /* stack is in running state */
} IXSSS_t_STATE;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
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
void IXSSS_IdentityInfoStore(const CSS_t_ID_INFO *ps_idInfo);


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
CSS_t_UINT IXSSS_VendorIdGet(void);


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
CSS_t_UINT IXSSS_DeviceTypeGet(void);


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
CSS_t_UINT IXSSS_ProductCodeGet(void);


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
CSS_t_USINT IXSSS_MajorRevGet(void);


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
CSS_t_USINT IXSSS_MinorRevGet(void);


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
CSS_t_UDINT IXSSS_SerialNumGet(void);


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
CSS_t_UDINT IXSSS_NodeIdGet(CSS_t_USINT u8_portIdx);


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
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
  CSS_t_UINT* IXSSS_PortNumArrayPtrGet(void);
#endif


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
void IXSSS_StateSet(IXSSS_t_STATE e_state);


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
IXSSS_t_STATE IXSSS_StateGet(void);


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
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSSS_IdentSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
#endif


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
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSSS_IdentSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


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
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSSS_MainSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var);
#endif


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
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSSS_MainSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


#endif /* #ifndef IXSSS_H */

/*** End Of File ***/


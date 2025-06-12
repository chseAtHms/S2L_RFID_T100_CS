/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSO.h
**    Summary: IXSSO - Safety Supervisor Object
**             IXSSO.h is the export header file of the IXSSO unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_Init
**             IXSSO_IdsSoftErrByteGet
**             IXSSO_IdsSoftErrVarGet
**             IXSSO_ServiceSoftErrByteGet
**             IXSSO_ServiceSoftErrVarGet
**             IXSSO_StateSoftErrByteGet
**             IXSSO_StateSoftErrVarGet
**             IXSSO_StateMachine
**             IXSSO_ExplMsgHandler
**             IXSSO_ScidSet
**             IXSSO_ScidPtrGet
**             IXSSO_TunidArrayPtrGet
**             IXSSO_CfUnidSet
**             IXSSO_CfUnidPtrGet
**             IXSSO_OcpUnidForCnxnPointSet
**             IXSSO_OcpUnidForCnxnPointGet
**             IXSSO_UnidCompare
**             IXSSO_UnidListCompare
**             IXSSO_DeviceHasValidTunid
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSO_H
#define IXSSO_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXSSO_k_SEAI_xxx:
    Safety Supervisor Event Additional Infos. These values are passed as
    Additional Info parameter to the Safety Supervisor State machine.
    Meaning is specific to the event.
*/
/*lint -esym(755, IXSSO_k_SEAI_PROP_TUNID_STOP)   not referenced in every cfg */
#define IXSSO_k_SEAI_NONE              0U   /* Additional Info is not used */
#define IXSSO_k_SEAI_PROP_TUNID_START  1U   /* Propose_TUNID service starts */
#define IXSSO_k_SEAI_PROP_TUNID_STOP   2U   /* Propose_TUNID service finished */


/** IXSSO_k_MC_xxx:
    Safety Supervisor Mode Change Service parameters.
*/
#define IXSSO_k_MC_IDLE        0U   /* change mode from Executing to Idle */
#define IXSSO_k_MC_EXEC        1U   /* change mode from Idle to Exectuing */


/** IXSSO_ks_ffUnid:
    UNID with all 0xFFs. Needed for comparison if the received TUNID is equal to
    this one.
*/
extern const CSS_t_UNID IXSSO_ks_ffUnid;

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /** IXSSO_ks_ZeroUnid:
      UNID with all 0x00s. Needed for comparison if the received TUNID is equal
      to this one.
  */
  extern const CSS_t_UNID IXSSO_ks_ZeroUnid;
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSO_Init
**
** Description : This function initializes the IXSSO unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_Init(void);


/*******************************************************************************
**
** Function    : IXSSO_IdsSoftErrByteGet
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
  CSS_t_UDINT IXSSO_IdsSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                      CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSSO_IdsSoftErrVarGet
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
  void IXSSO_IdsSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSSO_ServiceSoftErrByteGet
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
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
    CSS_t_UDINT IXSSO_ServiceSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                            CSS_t_BYTE *pb_var);
  #endif
#endif


/*******************************************************************************
**
** Function    : IXSSO_ServiceSoftErrVarGet
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
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSSO_ServiceSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
  #endif
#endif

/*******************************************************************************
**
** Function    : IXSSO_StateSoftErrByteGet
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
  CSS_t_UDINT IXSSO_StateSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                        CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSSO_StateSoftErrVarGet
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
  void IXSSO_StateSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSSO_StateMachine
**
** Description : This function implements the state machine of the Safety
**               Supervisor Object according to the specification.
**
** Parameters  : u8_event (IN)    - the event which is the reason for this
**                                  function being called. See also
**                                  {IXSSO_k_SE_xxx}.
**                                  (checked, valid range: all IXSSO_k_SE_xxx
**                                  definitions. Unknown events are ignored).
**               u16_addInfo (IN) - additional information - meaning is specific
**                                  to the event. See also {IXSSO_k_SEAI_xxx}.
**                                  (checked, Unknown AddInfos will be ignored)
**
** Returnvalue : CSS_k_OK         - success
**               <>CSS_k_OK       - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_StateMachine(CSS_t_USINT u8_event,
                              CSS_t_UINT u16_addInfo);


/*******************************************************************************
**
** Function    : IXSSO_ExplMsgHandler
**
** Description : This function handles all explicit requests addressed to the
**               Safety Supervisor Object. It checks if a class or Instance
**               Service is requested and calls the corresponding function to
**               serve the request (see Req.5.3.1-3).
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - pointer to the response message data that
**                                   is to be returned
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
CSS_t_WORD IXSSO_ExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE * const pb_rspData);


/*******************************************************************************
**
** Function    : IXSSO_ScidSet
**
** Description : This function stores the Safety Configuration Identifier (SCID)
**
** See Also    : IXSSO_ScidGet()
**
** Parameters  : u32_sccrc (IN) - new SCCRC
**                                (not checked, any value allowed)
**               ps_scts (IN)   - pointer to new SCTS
**                                (not checked, only called with reference to
**                                struct)
**
** Returnvalue : CSS_k_OK       - success
**               <>CSS_k_OK     - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_ScidSet(CSS_t_UDINT u32_sccrc,
                         const CSS_t_DATE_AND_TIME *ps_scts);


/*******************************************************************************
**
** Function    : IXSSO_ScidPtrGet
**
** Description : This function returns a pointer to the Safety Configuration
**               Identifier (SCID) from the Safety Supervisor Object
**
** See Also    : IXSSO_ScidSet()
**
** Parameters  : -
**
** Returnvalue : const IXSSO_t_SCID* - pointer to our device's SCID
**
*******************************************************************************/
const IXSSO_t_SCID* IXSSO_ScidPtrGet(void);


/*******************************************************************************
**
** Function    : IXSSO_TunidArrayPtrGet
**
** Description : This function returns a pointer to the Target UNID (TUNID)
**               Array from the Safety Supervisor Object.
**
** See Also    : IXSSO_TunidSet()
**
** Parameters  : -
**
** Returnvalue : const CSS_t_UNID* - pointer to our device's TUNID list
**
*******************************************************************************/
const CSS_t_UNID* IXSSO_TunidArrayPtrGet(void);


/*******************************************************************************
**
** Function    : IXSSO_CfUnidSet
**
** Description : This function stores the passed UNID as the new Configuration
**               owning UNID (CFUNID) of the device.
**
** See Also    : IXSSO_CfUnidPtrGet()
**
** Parameters  : ps_cfunid (IN) - pointer to the new CFUNID
**                                (not checked, only called with reference to
**                                struct)
**
** Returnvalue : CSS_k_OK       - success
**               <>CSS_k_OK     - error
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  CSS_t_WORD IXSSO_CfUnidSet(const CSS_t_UNID *ps_cfunid);
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSSO_CfUnidPtrGet
**
** Description : This function obtains the current CFUNID from the Safety
**               Supervisor Object
**
** See Also    : IXSSO_CfUnidSet()
**
** Parameters  : -
**
** Returnvalue : const CSS_t_UNID* - pointer to our device's CFUNID
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  const CSS_t_UNID* IXSSO_CfUnidPtrGet(void);
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSSO_OcpUnidForCnxnPointSet
**
** Description : This function stores the OCPUNID for the passed connection
**               point.
**
** See Also    : IXSSO_OcpUnidForCnxnPointGet()
**
** Parameters  : u16_outCnxnPoint (IN) - ID of the output connection point
**                                       (= output assembly instance ID)
**                                       (not checked, checked in
**                                       IXSAI_AsmOutIdxFromInstGet())
**               ps_ocpunid (IN)       - pointer to new OCPUNID of this
**                                       connection point
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_k_OK              - success
**               <>CSS_k_OK            - error
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    CSS_t_WORD IXSSO_OcpUnidForCnxnPointSet(CSS_t_UINT u16_outCnxnPoint,
                                            const CSS_t_UNID *ps_ocpunid);
  #endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */

/*******************************************************************************
**
** Function    : IXSSO_OcpUnidForCnxnPointGet
**
** Description : This function returns the OCPUNID to the passed connection
**               point
**
** See Also    : IXSSO_OcpUnidForCnxnPointSet()
**
** Parameters  : u16_outCnxnPoint (IN) - ID of the output connection point
**                                       (= output assembly instance ID)
**                                       (not checked, checked in
**                                       IXSAI_AsmOutIdxFromInstGet())
**               ps_ocpunid (OUT)      - pointer to OCPUNID of this connection
**                                       point
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_k_OK              - success
**               <>CSS_k_OK            - error
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    CSS_t_WORD IXSSO_OcpUnidForCnxnPointGet(CSS_t_UINT u16_outCnxnPoint,
                                            CSS_t_UNID *ps_ocpunid);
  #endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSSO_UnidCompare) */
/*lint -esym(765, IXSSO_UnidCompare) */
/*******************************************************************************
**
** Function    : IXSSO_UnidCompare
**
** Description : This function compares 2 UNID structures.
**
** Parameters  : ps_u1 (IN) - pointer to first UNID structure
**                            (not checked, only called with reference)
**               ps_u2 (IN) - pointer to second UNID structure
**                            (not checked, only called with reference)
**
** Returnvalue : CSS_k_TRUE  - UNIDs are equal
**               CSS_k_FALSE - UNIDs are different
**
*******************************************************************************/
CSS_t_BOOL IXSSO_UnidCompare(const CSS_t_UNID *ps_u1,
                             const CSS_t_UNID *ps_u2);


/*******************************************************************************
**
** Function    : IXSSO_UnidListCompare
**
** Description : This function checks if a single UNID (referenced by ps_u1)
**               appears in a list of UNIDS (referenced by pas_unidList).
**
** Parameters  : ps_u1 (IN)        - pointer to a single UNID structure
**                                   (not checked, only called with reference)
**               pas_unidList (IN) - pointer to an array of UNID structures with
**                                   CSOS_cfg_NUM_OF_SAFETY_PORTS members.
**                                   (not checked, only called with reference)
**
** Returnvalue : CSS_k_TRUE  - equal TUNID was found in the list
**               CSS_k_FALSE - none of the UNIDs from the list matched
**
*******************************************************************************/
#if (    (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)                                \
      || (CSOS_cfg_TARGET == CSOS_k_ENABLE)                                    \
    )
  CSS_t_BOOL IXSSO_UnidListCompare(const CSS_t_UNID *ps_u1,
                                   const CSS_t_UNID *pas_unidList);
#endif


/*******************************************************************************
**
** Function    : IXSSO_DeviceHasValidTunid
**
** Description : This function checks if the Device has a valid (non-FF) TUNID.
**               In case of a Multiple Safety Port Device at least one TUNID in
**               the list must be different than the default TUNID.
**
** Parameters  : pas_unidList (IN) - pointer to an array of UNID structures with
**                                   CSOS_cfg_NUM_OF_SAFETY_PORTS members.
**                                   (not checked, only called with reference)
**
** Returnvalue : CSS_k_TRUE  - TUNID of the device is set
**               CSS_k_FALSE - none of the device's TUNIDs is set
**
*******************************************************************************/
CSS_t_BOOL IXSSO_DeviceHasValidTunid(const CSS_t_UNID *pas_unidList);


#endif /* #ifndef IXSSO_H */

/*** End Of File ***/


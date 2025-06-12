/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOint.h
**    Summary: IXSSO - Safety Supervisor Object
**             IXSSOint.h is the internal header file of the IXSSO unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_StateMachineInit
**             IXSSO_IdsInit
**             IXSSO_SnnToCsalSend
**             IXSSO_OcpUnidPtrAndInstGet
**             IXSSO_ScidIsZero
**             IXSSO_TunidSet
**             IXSSO_TunidVsNodeIdCheck
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSSOINT_H
#define IXSSOINT_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


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
** Function    : IXSSO_StateMachineInit
**
** Description : This function initializes the SSO State Machine.
**               Initial state is Self-Testing and LEDs are switched to
**               Red/Green flashing.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_StateMachineInit(void);


/*******************************************************************************
**
** Function    : IXSSO_IdsInit
**
** Description : This function initializes the Safety Supervisor variables
**               provided by this module.
**
** Parameters  : -
**
** Returnvalue : CSS_k_OK             - success
**               <>CSS_k_OK           - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_IdsInit(void);


/*******************************************************************************
**
** Function    : IXSSO_SnnToCsalSend
**
** Description : This function sends the Safety Network Number(s) (SNN) of the
**               passed TUNID(s) to CSAL via HALC.
**
** Parameters  : pas_unidList (IN) - pointer to array of TUNID structs. The
**                                   Safety Network Number (SNN) is part of each
**                                   of these TUNIDs.
**                                   (not checked, only called with reference to
**                                   struct)
**
** Returnvalue : CSS_k_OK    - success
**               <>CSS_k_OK  - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_SnnToCsalSend(const CSS_t_UNID *pas_unidList);


/*******************************************************************************
**
** Function    : IXSSO_OcpUnidPtrAndInstGet
**
** Description : This function returns a pointer to the OCPUNID and the Assembly
**               Instance ID of the Safety Validator Server related to the
**               passed OCPUNID index.
**
** Parameters  : u16_ocpIdx (IN)      - OCPUNID index of the Output Assembly
**                                      Instance
**                                      (not checked, checked in
**                                      SrvcInstGetAttrSingle())
**               pu16_asmInstId (OUT) - returns the Output Assembly Instance ID
**                                      corresponding to the requested OCPUNID
**                                      index
**
** Returnvalue : const CSS_t_UNID*    - pointer to the OCPUNID of the requested
**                                      Safety Validator Server
**
*******************************************************************************/
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  const CSS_t_UNID* IXSSO_OcpUnidPtrAndInstGet(CSS_t_UINT u16_ocpIdx,
                                               CSS_t_UINT *pu16_asmInstId);
#endif


/*******************************************************************************
**
** Function    : IXSSO_ScidIsZero
**
** Description : This function checks if the current Safety Configuration
**               Identifier (SCID) from the Safety Supervisor Object is ZERO.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : CSS_k_TRUE   - SCID is zero
**               CSS_k_FALSE  - SCID is non-zero
**
*******************************************************************************/
CSS_t_BOOL IXSSO_ScidIsZero(void);


/*******************************************************************************
**
** Function    : IXSSO_TunidSet
**
** Description : This function stores the passed UNID list as the new Target
**               UNID (TUNID) of the device.
**
** See Also    : IXSSO_TunidGet()
**
** Parameters  : pas_unidList (IN) - pointer to an array of UNID structures with
**                                   CSOS_cfg_NUM_OF_SAFETY_PORTS members.
**                                   (not checked, only called with reference)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error
**
*******************************************************************************/
#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
  CSS_t_WORD IXSSO_TunidSet(const CSS_t_UNID *pas_unidList);
#endif


/*******************************************************************************
**
** Function    : IXSSO_TunidVsNodeIdCheck
**
** Description : This function checks if the NodeID part of the TUNID matches
**               the device's currently set NodeID. For Multiple Safety Port
**               Devices this check is done for all ports' NodeIDs. In case our
**               NodeID of a port is not yet set (0xFFFFFFFF) this check also
**               includes checking of the TUNID.SNN to be all 0xFF. If any of
**               the TUNIDs in the list is all 0xFF this is also acceptable (to
**               indicate that this port is not used for CIP Safety).
**
** Parameters  : pas_tunid (IN) - pointer to array with our device's TUNIDs
**                                (not checked, only called with reference to
**                                array of struct)
**
** Returnvalue : CSS_k_TRUE     - all TUNIDs match NodeIDs
**               CSS_k_FALSE    - TUNID versus NodeID mismatch (at least one) or
**                                SNN set for an unconfigured port
**
*******************************************************************************/
CSS_t_BOOL IXSSO_TunidVsNodeIdCheck(const CSS_t_UNID *pas_tunid);


#endif /* #ifndef IXSSOINT_H */


/*** End Of File ***/



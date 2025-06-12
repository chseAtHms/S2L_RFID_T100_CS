/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOmisc.c
**    Summary: IXSSO - Safety Supervisor Object
**             This module contains several auxiliary routines of the IXSSO
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_TunidVsNodeIdCheck
**             IXSSO_UnidCompare
**             IXSSO_UnidListCompare
**             IXSSO_DeviceHasValidTunid
**             IXSSO_SnnToCsalSend
**             IXSSO_ModeChange
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

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #include "CSS.h"
#endif
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXSSS.h"
#include "IXSCF.h"

#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXSSOint.h"
#include "IXSSOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
CSS_t_BOOL IXSSO_TunidVsNodeIdCheck(const CSS_t_UNID *pas_tunid)
{
  /* return value of this function */
  CSS_t_BYTE o_ret = CSS_k_TRUE;
  /* loop counter for checking all safety ports */
  CSS_t_USINT u8_portIdx;

  /* check all TUNIDs of the list */
  for (u8_portIdx = 0U; u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS; u8_portIdx++)
  {
    CSS_t_UDINT u32_ourNodeId = IXSSS_NodeIdGet(u8_portIdx);

    /* if the Node ID of this port is not set */
    if (u32_ourNodeId == IXSSS_k_INVALID_NODEID)
    {
      /* if the SNN part of this Port is different than FF */
      if (    (pas_tunid[u8_portIdx].s_snn.u32_time != 0xFFFFFFFFUL)
           || (pas_tunid[u8_portIdx].s_snn.u16_date != 0xFFFFU)
         )
      {
        /* Unconfigured port must not have an SNN */
        o_ret = CSS_k_FALSE;
      }
      else  /* else: SNN part for this port is correct */
      {
        /* don't modify return value */
      }
    }
    else  /* else: this port has a Node ID */
    {
      /* only do the common checks below */
    }

    /* if this TUNID is not set (all 0xFF) */
    if (IXSSO_UnidCompare(&pas_tunid[u8_portIdx], &IXSSO_ks_ffUnid))
    {
      /* An FF TUNID can be assigned to any port (to make it unusable for
         CIP Safety). Then skip the check against the NodeID. */
      /* don't modify return value */
    }
    else /* else: this TUNID is set */
    {
      /* if NodeID portion of the passed TUNID matches NodeID of the device */
      if (pas_tunid[u8_portIdx].u32_nodeId == u32_ourNodeId)
      {
        /* if time and/or date is 0xFF */
        if (    (pas_tunid[u8_portIdx].s_snn.u32_time == 0xFFFFFFFFUL)
             || (pas_tunid[u8_portIdx].s_snn.u16_date == 0xFFFFU)
           )
        {
          /* this is not a valid SNN */
          o_ret = CSS_k_FALSE;
        }
        else
        {
          /* don't modify return value */
        }
      }
      else /* else: mismatch */
      {
        /* in case of a mismatch of at least one NodeID make sure the return
           value is set to CSS_k_FALSE */
        o_ret = CSS_k_FALSE;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_ret);
}


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
                             const CSS_t_UNID *ps_u2)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if all fields of the struct are equal */
  if (    (ps_u1->s_snn.u32_time == ps_u2->s_snn.u32_time)
       && (ps_u1->s_snn.u16_date == ps_u2->s_snn.u16_date)
       && (ps_u1->u32_nodeId == ps_u2->u32_nodeId)
     )
  {
    /* both UNIDs are equal */
    o_retVal = CSS_k_TRUE;
  }
  else /* else: UNIDs are different */
  {
    /* UNIDs are different */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#if (    (CSS_cfg_SAFETY_RESET == CSS_k_ENABLE)                                \
      || (CSOS_cfg_TARGET == CSOS_k_ENABLE)                                    \
    )
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
CSS_t_BOOL IXSSO_UnidListCompare(const CSS_t_UNID *ps_u1,
                                 const CSS_t_UNID *pas_unidList)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* loop counter for checking all safety ports */
  CSS_t_USINT u8_portIdx;

  /* Check all TUNIDs in the list. If one TUNID in the list matches we set
     o_retVal to CSS_k_TRUE */
  for (u8_portIdx = 0U; u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS; u8_portIdx++)
  {
    /* if all fields of the struct are equal */
    if (    (ps_u1->s_snn.u32_time == pas_unidList[u8_portIdx].s_snn.u32_time)
         && (ps_u1->s_snn.u16_date == pas_unidList[u8_portIdx].s_snn.u16_date)
         && (ps_u1->u32_nodeId == pas_unidList[u8_portIdx].u32_nodeId)
       )
    {
      /* both UNIDs are equal */
      o_retVal = CSS_k_TRUE;
    }
    else /* else: UNIDs are different */
    {
      /* UNIDs are different */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}
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
CSS_t_BOOL IXSSO_DeviceHasValidTunid(const CSS_t_UNID *pas_unidList)
{
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* loop counter for checking all safety ports */
  CSS_t_USINT u8_portIdx;

  /* Check all TUNIDs in the list. If one TUNID in the list is different than
     an all FF TUNID then return CSS_k_TRUE */
  for (u8_portIdx = 0U; u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS; u8_portIdx++)
  {
    /* if all fields of the struct are not equal to the default TUNID */
    if (    (pas_unidList[u8_portIdx].s_snn.u32_time != 0xFFFFFFFFUL)
         && (pas_unidList[u8_portIdx].s_snn.u16_date != 0xFFFFU)
         && (pas_unidList[u8_portIdx].u32_nodeId != 0xFFFFFFFFUL)
       )
    {
      /* we have a non-default TUNID */
      o_retVal = CSS_k_TRUE;
    }
    else /* else: this is a default TUNID */
    {
      /* don't modify function return value */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


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
CSS_t_WORD IXSSO_SnnToCsalSend(const CSS_t_UNID *pas_unidList)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = CSS_k_OK;
  /* buffer to store the SNN */
  CSS_t_BYTE ab_snn[CSOS_k_SIZEOF_UDINT + CSOS_k_SIZEOF_UINT];
  /* HALC message to be transmitted to CSAL */
  HALCS_t_MSG s_halcTxMsg;
  /* loop counter for sending the SNNs of all safety ports */
  CSS_t_USINT u8_portIdx;

  /* completely erase structure */
  CSS_MEMSET(&s_halcTxMsg, 0, sizeof(s_halcTxMsg));

  /* prepare message */
  s_halcTxMsg.u16_cmd = CSOS_k_CMD_IXSSO_SNN;
  s_halcTxMsg.u16_len = CSOS_k_SIZEOF_UDINT + CSOS_k_SIZEOF_UINT;
  s_halcTxMsg.pb_data = ab_snn;

  /* for all ports */
  for (u8_portIdx = 0U; u8_portIdx < CSOS_cfg_NUM_OF_SAFETY_PORTS; u8_portIdx++)
  {
    /* only continue processing if no error was yet detected */
    if (w_retVal == CSS_k_OK)
    {
      /* write the port index into the additional Info field */
      s_halcTxMsg.u32_addInfo = (CSS_t_UDINT)u8_portIdx;

      /* copy SNN to buffer */
      CSS_H2N_CPY32(CSS_ADD_OFFSET(ab_snn,
                                    0U),
                    &pas_unidList[u8_portIdx].s_snn.u32_time);
      CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_snn,
                                    CSOS_k_SIZEOF_UDINT),
                    &pas_unidList[u8_portIdx].s_snn.u16_date);

      /* if sending SNN to CSAL fails */
      if (!HALCS_TxDataPut(&s_halcTxMsg))
      {
        w_retVal = IXSSO_k_NFSE_TXH_TUNID;
        SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
      else /* else: no error */
      {
        w_retVal = CSS_k_OK;
      }
    }
    else  /* else: there was an error */
    {
      /* do nothing for the rest of the loop */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSSO_ModeChange
**
** Description : This API function moves the device from the Idle to the
**               Executing state or back from Executing to Idle.
**
** Parameters  : o_executing (IN) - CSS_k_TRUE - move from IDLE to EXECUTING
**                                  CSS_k_FALSE - move from EXECUTING to IDLE
**                                  (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_ModeChange(CSS_t_BOOL o_executing)
{
  /* if CIP Safety Stack is in an improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk(IXSSO_k_FSE_AIS_STATE_ERR_MC,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* CSS is in a proper state */
  {
    /* SSO State Machine checks if allowed in current state and performs the
       changing of the Device Status */
    /* if an error is returned then this is already reported by SSO state
       machine to SAPL via SAPL_CssErrorClbk() */
    (void)IXSSO_StateMachine(IXSSO_k_SE_MODE_CHANGE,
                             (CSS_t_UINT)o_executing);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/



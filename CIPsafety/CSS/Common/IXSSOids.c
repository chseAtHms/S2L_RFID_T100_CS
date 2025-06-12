/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSOids.c
**    Summary: IXSSO - Safety Supervisor Object
**             This module contains access functions for the various IDs managed
**             by the SSO: SCID, TUNID, CFUNID, OCPUNID
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSO_IdsInit
**             IXSSO_OcpunidTargAsmOutListSet
**             IXSSO_ScidSet
**             IXSSO_ScidPtrGet
**             IXSSO_ScidIsZero
**             IXSSO_TunidSet
**             IXSSO_TunidArrayPtrGet
**             IXSSO_OcpUnidPtrAndInstGet
**             IXSSO_OcpUnidForCnxnPointSet
**             IXSSO_OcpUnidForCnxnPointGet
**             IXSSO_CfUnidSet
**             IXSSO_CfUnidPtrGet
**             IXSSO_IdsSoftErrByteGet
**             IXSSO_IdsSoftErrVarGet
**
**             OcpUnidIdxFromAsmInstGet
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

#include "IXSERapi.h"

/* Following headers are not used in all configurations */
#include "IXSAIapi.h" /*lint -efile(766, IXSAIapi.h) */
#include "IXSAI.h"    /*lint -efile(766, IXSAI.h) */

#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)                     \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  #include "IXSSS.h"
#endif
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

/* SCID Attribute (NV) */
static IXSSO_t_SCID s_Scid;

/* TUNID (list) Attribute (NV) (Array of TUNIDs, one for each port) */
static CSS_t_UNID as_TargetUnid[CSOS_cfg_NUM_OF_SAFETY_PORTS];

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* CFUNID Attribute (NV) */
  static CSS_t_UNID s_ConfigUnid;
#endif

#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  /* OCPUNIDs Attribute (NV) */
  static CSS_t_UNID as_OcpUnid[IXSSO_k_NUM_OCPUNIDS];
#endif


#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  static CSS_t_UINT au16_TargetAsmOutInstList[CSS_cfg_NUM_OF_T_ASM_OUT_INSTS];
#endif


/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  /*lint -esym(750, k_IDS_SIZEOF_VAR123)       not referenced in every config */
  /*lint -esym(750, k_IDS_SIZEOF_VAR1234)      not referenced in every config */
  /*lint -esym(750, k_IDS_SIZEOF_VAR12345)     not referenced in every config */
  #define k_IDS_SIZEOF_VAR1        (sizeof(s_Scid))
  #define k_IDS_SIZEOF_VAR12       (k_IDS_SIZEOF_VAR1 + sizeof(as_TargetUnid))
  #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    #define k_IDS_SIZEOF_VAR123    (k_IDS_SIZEOF_VAR12 + sizeof(s_ConfigUnid))
  #else
    #define k_IDS_SIZEOF_VAR123    (k_IDS_SIZEOF_VAR12 + 0U)
  #endif
  #if (IXSSO_k_NUM_OCPUNIDS != 0U)
    #define k_IDS_SIZEOF_VAR1234   (k_IDS_SIZEOF_VAR123 + sizeof(as_OcpUnid))
  #else
    #define k_IDS_SIZEOF_VAR1234   (k_IDS_SIZEOF_VAR123 + 0U)
  #endif
  #if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
    #define k_IDS_SIZEOF_VAR12345  (k_IDS_SIZEOF_VAR1234 + \
                                   sizeof(au16_TargetAsmOutInstList))
  #else
    #define k_IDS_SIZEOF_VAR12345  (k_IDS_SIZEOF_VAR1234 + 0U)
  #endif

#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)

/* define IDs of the variables depending on which variables are supported */
enum
{
   k_SOFT_ERR_VAR_SCID_SCCRC
  ,k_SOFT_ERR_VAR_SCID_TIME
  ,k_SOFT_ERR_VAR_SCID_DATE
  ,k_SOFT_ERR_VAR_TARGUNID
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  ,k_SOFT_ERR_OPT_VAR_CFGUNID_NID
  ,k_SOFT_ERR_OPT_VAR_CFGUNID_TIME
  ,k_SOFT_ERR_OPT_VAR_CFGUNID_DATE
#endif
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  ,k_SOFT_ERR_OPT_VAR_OCPUNID
#endif
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  ,k_SOFT_ERR_OPT_VAR_TARGASM_LIST
#endif
};
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  static CSS_t_UINT OcpUnidIdxFromAsmInstGet(CSS_t_UINT u16_asmInstId);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

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
CSS_t_WORD IXSSO_IdsInit(void)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* make sure the whole structure is initialized (also padding bytes)
     because of Soft Error Checking) */
  CSS_MEMSET(&s_Scid, 0, sizeof(s_Scid));
  CSS_MEMSET(as_TargetUnid, 0, sizeof(as_TargetUnid));

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  {
    CSS_MEMSET(&s_ConfigUnid, 0, sizeof(s_ConfigUnid));
  }
#endif
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  {
    CSS_MEMSET(as_OcpUnid, 0, sizeof(as_OcpUnid));
  }
#endif
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)                     \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  {
    CSS_MEMSET(au16_TargetAsmOutInstList, 0,
               sizeof(au16_TargetAsmOutInstList));
  }
#endif

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* if restoring the "CFUNID" attribute fails */
  if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_CFUNID,
         IXSSO_k_SDS_CFUNID,
         (CSS_t_BYTE *)&s_ConfigUnid))
  {
    w_retVal = (IXSSO_k_FSE_INC_NV_R_CFUNID);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
#endif
  {
  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
    {
      /* For the Originator the SCID is passed/calculated during the CCO's
         Validate function. Thus set our device's SCID to the default value
         and don't read from NV. */
      s_Scid.u32_sccrc = 0UL;
      s_Scid.s_scts.u32_time = 0UL;
      s_Scid.s_scts.u16_date = 0U;
    }
  #else
    /* if restoring the "SCID" attribute fails */
    if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_SCID,
           IXSSO_k_SDS_SCID,
           (CSS_t_BYTE *)&s_Scid))
    {
      w_retVal = (IXSSO_k_FSE_INC_NV_R_SCID);
      SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: no error */
  #endif
    {
      /* if restoring the "TUNID"/"TUNID list" attribute (see SRS118) fails */
      if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_TUNID,
             IXSSO_k_SDS_TUNID,
             (CSS_t_BYTE *)as_TargetUnid))
      {
        w_retVal = (IXSSO_k_FSE_INC_NV_R_TUNID);
        SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                          IXSER_k_A_NOT_USED);
      }
      else /* else: no error */
      {
      #if (IXSSO_k_NUM_OCPUNIDS != 0U)
        /* if restoring the "OCPUNID" attribute fails */
        if (!SAPL_IxssoNvAttribRestoreClbk(IXSSO_k_SID_OCPUNID,
               IXSSO_k_SDS_OCPUNID,
               (CSS_t_BYTE *)as_OcpUnid))
        {
          w_retVal = (IXSSO_k_FSE_INC_NV_R_OCPUNID);
          SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                            IXSER_k_A_NOT_USED);
        }
        else /* else: no error */
      #endif
        {
          /* all initializations successful up to now */
          /* inform CSAL about Safety Network Number(s) (SNN is part of */
          /* TUNID) (see SRS118) */
          w_retVal = IXSSO_SnnToCsalSend(as_TargetUnid);
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)                     \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
/*******************************************************************************
**
** Function    : IXSSO_OcpunidTargAsmOutListSet
**
** Description : This API function stores a list (lookup table) of the Output
**               Assembly Instances used for connections in which this device is
**               the Target. It must be called after setting up the Assembly
**               Instances (IXSAI_AsmIoInstSetup()) and before applying the
**               configuration (IXCCO_Apply()).
**               See the description of CSS_cfg_NUM_OF_T_ASM_OUT_INSTS for which
**               configurations this function is relevant.
**
** Parameters  : au16_targAsmOutInsts (IN) - pointer to an array of Output
**                                           Assembly Instances (array size
**                                           CSS_cfg_NUM_OF_T_ASM_OUT_INSTS
**                                           elements)
**                                           (checked,
**                                           valid range: <> CSS_k_NULL)
** Returnvalue : -
**
*******************************************************************************/
void IXSSO_OcpunidTargAsmOutListSet(
      const CSS_t_UINT au16_targAsmOutInsts[CSS_cfg_NUM_OF_T_ASM_OUT_INSTS])
{
  CSS_t_UINT u16_i;

  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    SAPL_CssErrorClbk(IXSSO_k_FSE_AIS_STATE_ERR_OTALS,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* CIP Safety Stack is in running state */
  {
    /* if the passed pointer is invalid */
    if (au16_targAsmOutInsts == CSS_k_NULL)
    {
      SAPL_CssErrorClbk(IXSSO_k_FSE_AIP_PTR_INV_OTALS,
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    }
    else /* else: passed pointer is valid */
    {
      for (u16_i=0U; u16_i<CSS_cfg_NUM_OF_T_ASM_OUT_INSTS; u16_i++)
      {
        au16_TargetAsmOutInstList[u16_i] = au16_targAsmOutInsts[u16_i];
      }
    }
  }
}
#endif


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
                         const CSS_t_DATE_AND_TIME *ps_scts)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);
  /* temporary SCID value */
  IXSSO_t_SCID s_tempScid;

  /* make sure the whole structure is initialized (also padding bytes)
     because of Soft Error Checking) */
  CSS_MEMSET(&s_tempScid, 0, sizeof(s_tempScid));

  /* copy received data to temporary structure variable */
  s_tempScid.u32_sccrc = u32_sccrc;
  s_tempScid.s_scts.u32_time = ps_scts->u32_time;
  s_tempScid.s_scts.u16_date = ps_scts->u16_date;

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  {
    /* On our originator implementation the SCID is not stored in NV-memory */
  }
#else
  /* if storing the new SCID to NV-memory (see SRS5) fails */
  if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_SCID,
                                   IXSSO_k_SDS_SCID,
                                   (CSS_t_BYTE *)&s_tempScid))
  {
    /* error while storing NV-data */
    w_retVal = (IXSSO_k_FSE_INC_NV_W_SCID_2);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
#endif
  {
    /* take over the new SCID value */
    s_Scid.u32_sccrc = s_tempScid.u32_sccrc;
    s_Scid.s_scts.u32_time = s_tempScid.s_scts.u32_time;
    s_Scid.s_scts.u16_date = s_tempScid.s_scts.u16_date;

    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


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
const IXSSO_t_SCID* IXSSO_ScidPtrGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&s_Scid);
}


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
CSS_t_BOOL IXSSO_ScidIsZero(void)
{
  CSS_t_BOOL o_retVal;

  /* if SCID is zero */
  if (    (s_Scid.u32_sccrc == 0U)
       && (s_Scid.s_scts.u32_time == 0U)
       && (s_Scid.s_scts.u16_date == 0U)
     )
  {
    o_retVal = CSS_k_TRUE;
  }
  else /* else: SCID is non-zero */
  {
    o_retVal = CSS_k_FALSE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
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
CSS_t_WORD IXSSO_TunidSet(const CSS_t_UNID *pas_unidList)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* Store the new value to NV-memory. As it is done inside CSS this has      */
  /* safety integrity (see SRS125). */
  /* if storing the new TUNID to NV-memory fails */
  if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_TUNID,
                                   IXSSO_k_SDS_TUNID,
                                   (CSS_t_BYTE *)pas_unidList))
  {
    /* error while storing NV-data */
    w_retVal = (IXSSO_k_FSE_INC_NV_W_TUNID_2);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
  {
    /* also inform CSAL about new Safety Network Number(s) (SNN is part of */
    /* the TUNID) (see SRS191) */
    w_retVal = IXSSO_SnnToCsalSend(pas_unidList);

    /* if previous function returned an error */
    if (w_retVal != CSS_k_OK)
    {
      /* error while sending TUNID to CSAL - do not continue */
    }
    else /* else: no error */
    {
      /* take over the new TUNID (list) value */
      CSS_MEMCPY(as_TargetUnid, pas_unidList, sizeof(as_TargetUnid));
      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif


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
const CSS_t_UNID* IXSSO_TunidArrayPtrGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_TargetUnid);
}


#if (IXSSO_k_NUM_OCPUNIDS != 0U)
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
const CSS_t_UNID* IXSSO_OcpUnidPtrAndInstGet(CSS_t_UINT u16_ocpIdx,
                                             CSS_t_UINT *pu16_asmInstId)
{
  /* return the Assembly Instance ID of the requested OCPUNID */
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  {
    *pu16_asmInstId = au16_TargetAsmOutInstList[u16_ocpIdx];
  }
#else
  *pu16_asmInstId = IXSAI_AsmOutInstFromIdxGet(u16_ocpIdx);

  /* if the returned instance ID is invalid (index doesn't exist) */
  if (*pu16_asmInstId == CSOS_k_INVALID_INSTANCE)
  {
    SAPL_CssErrorClbk(IXSSO_k_FSE_INC_OCPIDX_INV,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_ocpIdx);
  }
  else
  {
    /* instance is valid */
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* return the OCPUNID */
  return (&as_OcpUnid[u16_ocpIdx]);
}
#endif  /* (IXSSO_k_NUM_OCPUNIDS != 0U) */


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
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
CSS_t_WORD IXSSO_OcpUnidForCnxnPointSet(CSS_t_UINT u16_outCnxnPoint,
                                        const CSS_t_UNID *ps_ocpunid)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);
  /* get OCPUNID index from passed Cnxn Point */
  CSS_t_UINT u16_ocpIdx = OcpUnidIdxFromAsmInstGet(u16_outCnxnPoint);

  /* if output assembly doesn't exist */
  if (u16_ocpIdx == IXSAI_k_INVALID_ASM_INDEX)
  {
    /* connection point (==assembly_instance) not found */
    w_retVal = (IXSSO_k_FSE_INC_ASM_NFOUND_SET);
    SAPL_CssErrorClbk(w_retVal, u16_outCnxnPoint, IXSER_k_A_NOT_USED);
  }
  else /* else: assembly exists */
  {
    /* if type of this assembly is not "output" */
    if (IXSAI_AsmInstTypeGet(u16_outCnxnPoint) != IXSAI_k_ASM_OUTPUT)
    {
      /* passed connection point is not an output */
      w_retVal = (IXSSO_k_FSE_INC_ASM_INVTYPE_SET);
      SAPL_CssErrorClbk(w_retVal, u16_outCnxnPoint, IXSER_k_A_NOT_USED);
    }
    else /* else: it is an output assembly */
    {
      /* if the passed UNID matches the stored OCPUNID for this cnxn point */
      if (IXSSO_UnidCompare(ps_ocpunid, &as_OcpUnid[u16_ocpIdx]))
      {
        /* This OCPUNID is already stored. So no need to store it again */
        w_retVal = CSS_k_OK;
      }
      else /* else: mismatch */
      {
        /* temporary store the old OCPUNID in case NV-storing goes wrong */
        CSS_t_UNID s_temp;

        /* completely erase structure */
        CSS_MEMSET(&s_temp, 0, sizeof(s_temp));
        /* copy values from the OCPUNID table */
        s_temp.u32_nodeId     = as_OcpUnid[u16_ocpIdx].u32_nodeId;
        s_temp.s_snn.u32_time = as_OcpUnid[u16_ocpIdx].s_snn.u32_time;
        s_temp.s_snn.u16_date = as_OcpUnid[u16_ocpIdx].s_snn.u16_date;

        /* copy the passed UNID */
        as_OcpUnid[u16_ocpIdx].u32_nodeId     = ps_ocpunid->u32_nodeId;
        as_OcpUnid[u16_ocpIdx].s_snn.u32_time = ps_ocpunid->s_snn.u32_time;
        as_OcpUnid[u16_ocpIdx].s_snn.u16_date = ps_ocpunid->s_snn.u16_date;

        /* Store the Output Connection Owner UNID to NV-memory */
        /* (see FRS168 and SRS6) */
        /* if storing the OCPUNID to NV-memory fails */
        if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_OCPUNID,
                                         IXSSO_k_SDS_OCPUNID,
                                         (CSS_t_BYTE *)as_OcpUnid))
        {
          /* error while storing NV-data - restore previous value */
          as_OcpUnid[u16_ocpIdx].u32_nodeId     = s_temp.u32_nodeId;
          as_OcpUnid[u16_ocpIdx].s_snn.u32_time = s_temp.s_snn.u32_time;
          as_OcpUnid[u16_ocpIdx].s_snn.u16_date = s_temp.s_snn.u16_date;

          /* return error response */
          w_retVal = (IXSSO_k_FSE_INC_NV_W_OCPUNID_2);
          SAPL_CssErrorClbk(w_retVal, u16_outCnxnPoint, IXSER_k_A_NOT_USED);
        }
        else /* else: no error */
        {
          w_retVal = CSS_k_OK;
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


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
CSS_t_WORD IXSSO_OcpUnidForCnxnPointGet(CSS_t_UINT u16_outCnxnPoint,
                                        CSS_t_UNID *ps_ocpunid)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);
  /* get OCPUNID index from passed Cnxn Point */
  CSS_t_UINT u16_ocpIdx = OcpUnidIdxFromAsmInstGet(u16_outCnxnPoint);

  /* if output assembly doesn't exist */
  if (u16_ocpIdx == IXSAI_k_INVALID_ASM_INDEX)
  {
    /* connection point (==assembly_instance) not found */
    w_retVal = (IXSSO_k_FSE_INC_ASM_NFOUND_GET);
    SAPL_CssErrorClbk(w_retVal, u16_outCnxnPoint,
                      IXSER_k_A_NOT_USED);
  }
  else /* else: assembly exists */
  {
    /* if type of this assembly is not "output" */
    if (IXSAI_AsmInstTypeGet(u16_outCnxnPoint) != IXSAI_k_ASM_OUTPUT)
    {
      /* passed connection point is not an output */
      w_retVal = (IXSSO_k_FSE_INC_ASM_INVTYPE_GET);
      SAPL_CssErrorClbk(w_retVal, u16_outCnxnPoint, IXSER_k_I_NOT_USED);
    }
    else /* else: it is an output assembly */
    {
      /* return the owner of the cnxn point */
      *ps_ocpunid = as_OcpUnid[u16_ocpIdx];

      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (IXSSO_k_NUM_OCPUNIDS != 0U) */


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
CSS_t_WORD IXSSO_CfUnidSet(const CSS_t_UNID *ps_cfunid)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSSO_k_FSE_INC_PRG_FLOW);

  /* Store the new Configuration owning UNID to NV-memory */
  /* (see FRS167 and SRS4) */
  /* if storing the CFUNID to NV-memory fails */
  if (!SAPL_IxssoNvAttribStoreClbk(IXSSO_k_SID_CFUNID,
                                   IXSSO_k_SDS_CFUNID,
                                   (CSS_t_BYTE *)ps_cfunid))
  {
    /* error while storing NV-data */
    w_retVal = (IXSSO_k_FSE_INC_NV_W_CFUNID_2);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
  {
    /* take over the new CFUNID value */
    s_ConfigUnid = *ps_cfunid;

    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


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
const CSS_t_UNID* IXSSO_CfUnidPtrGet(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&s_ConfigUnid);  /* return pointer to the CFUNID */
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
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
CSS_t_UDINT IXSSO_IdsSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                   CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_IDS_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)&s_Scid) + u32_varCnt);
  }
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_IDS_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)as_TargetUnid)
                              + (u32_varCnt - k_IDS_SIZEOF_VAR1));
  }
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /* else: if counter indicates we are in the range of the third variable */
  else if (u32_varCnt < (k_IDS_SIZEOF_VAR123))
  {
    *pb_var = *(((CSS_t_BYTE*)&s_ConfigUnid)
                              + (u32_varCnt - (k_IDS_SIZEOF_VAR12)));
  }
#endif
#if (IXSSO_k_NUM_OCPUNIDS != 0U)
  /* else: if counter indicates we are in the range of the fourth variable */
  else if (u32_varCnt < (k_IDS_SIZEOF_VAR1234))
  {
    *pb_var = *(((CSS_t_BYTE*)as_OcpUnid)
                              + (u32_varCnt - (k_IDS_SIZEOF_VAR123)));
  }
#endif
#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  /* else: if counter indicates we are in the range of the fifth variable */
  else if (u32_varCnt < (k_IDS_SIZEOF_VAR12345))
  {
    *pb_var = *(((CSS_t_BYTE*)au16_TargetAsmOutInstList)
                              + (u32_varCnt - (k_IDS_SIZEOF_VAR1234)));
  }
#endif
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_IDS_SIZEOF_VAR12345;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
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
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSSO_IdsSoftErrVarGet(
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
    case k_SOFT_ERR_VAR_SCID_SCCRC:
    {
      CSS_H2N_CPY32(pb_var, &s_Scid.u32_sccrc);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      break;
    }

    case k_SOFT_ERR_VAR_SCID_TIME:
    {
      CSS_H2N_CPY32(pb_var, &s_Scid.s_scts.u32_time);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      break;
    }

    case k_SOFT_ERR_VAR_SCID_DATE:
    {
      CSS_H2N_CPY16(pb_var, &s_Scid.s_scts.u16_date);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }

    case k_SOFT_ERR_VAR_TARGUNID:
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
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case 0:
          {
            CSS_H2N_CPY32(pb_var, &as_TargetUnid[au32_cnt[1U]].u32_nodeId);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 1:
          {
            CSS_H2N_CPY32(pb_var, &as_TargetUnid[au32_cnt[1U]].s_snn.u32_time);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 2:
          {
            CSS_H2N_CPY16(pb_var, &as_TargetUnid[au32_cnt[1U]].s_snn.u16_date);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
            break;
          }
        }
      }
      break;
    }

  #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    case k_SOFT_ERR_OPT_VAR_CFGUNID_NID:
    {
      CSS_H2N_CPY32(pb_var, &s_ConfigUnid.u32_nodeId);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      break;
    }

    case k_SOFT_ERR_OPT_VAR_CFGUNID_TIME:
    {
      CSS_H2N_CPY32(pb_var, &s_ConfigUnid.s_snn.u32_time);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
      break;
    }

    case k_SOFT_ERR_OPT_VAR_CFGUNID_DATE:
    {
      CSS_H2N_CPY16(pb_var, &s_ConfigUnid.s_snn.u16_date);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }
  #endif

  #if (IXSSO_k_NUM_OCPUNIDS != 0U)
    case k_SOFT_ERR_OPT_VAR_OCPUNID:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= IXSSO_k_NUM_OCPUNIDS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case 0U:
          {
            CSS_H2N_CPY32(pb_var, &as_OcpUnid[au32_cnt[1U]].u32_nodeId);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 1U:
          {
            CSS_H2N_CPY32(pb_var, &as_OcpUnid[au32_cnt[1U]].s_snn.u32_time);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }

          case 2U:
          {
            CSS_H2N_CPY16(pb_var, &as_OcpUnid[au32_cnt[1U]].s_snn.u16_date);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
            break;
          }
        }
      }
      break;
    }
  #endif

  #if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
        && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
      )
    case k_SOFT_ERR_OPT_VAR_TARGASM_LIST:
    {
      /* indicate to caller that we are in range of level 1 or below */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= CSS_cfg_NUM_OF_T_ASM_OUT_INSTS)
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else
      {
        CSS_H2N_CPY16(pb_var, &au16_TargetAsmOutInstList[au32_cnt[1U]]);
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

#if (IXSSO_k_NUM_OCPUNIDS != 0U)
/*******************************************************************************
**
** Function    : OcpUnidIdxFromAsmInstGet
**
** Description : This function obtains the OCPUNID index corresponding to the
**               passed Output Assembly Instance ID
**
** Parameters  : u16_asmInstId (IN) - Output Assembly Instance ID
**                                    (checked, must match an entry in
**                                    au16_TargetAsmOutInstList)
**
** Returnvalue : CSS_t_UINT         - index in the as_OcpUnid array
**                                    corresponding to the passed Output
**                                    Assembly Instance ID
**
*******************************************************************************/
static CSS_t_UINT OcpUnidIdxFromAsmInstGet(CSS_t_UINT u16_asmInstId)
{
  /* return value of this function */
  CSS_t_UINT u16_retVal = IXSAI_k_INVALID_ASM_INDEX;

#if (    (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > 0U)  \
      && (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
    )
  /* array search index */
  CSS_t_UINT u16_i;

  /* check all entries of the Target Output Assembly list */
  for (u16_i=0U; u16_i<IXSSO_k_NUM_OCPUNIDS; u16_i++)
  {
    /* if searched instance not yet found */
    if (u16_retVal == IXSAI_k_INVALID_ASM_INDEX)
    {
      /* if current entry's Instance ID matches */
      if (au16_TargetAsmOutInstList[u16_i] == u16_asmInstId)
      {
        u16_retVal = u16_i;
      }
      else
      {
        /* doesn't match */
      }
    }
    else
    {
      /* instance already found */
    }
  }
#else
  {
    u16_retVal = IXSAI_AsmOutIdxFromInstGet(u16_asmInstId);
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_retVal);
}
#endif


/*** End Of File ***/



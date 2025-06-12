/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEelKey.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module contains functions for Electronic Key parsing and
**             checking.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_ElectronicKeyParse
**             IXSCE_ElectronicKeyMatchCheck
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
/*#include "CSOS.h"*/

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  #include "HALCSapi.h"
#endif

#include "IXSERapi.h"
#include "IXSSS.h"
#include "IXSCF.h"

#include "IXSCEapi.h"
#include "IXSCE.h"
/*#include "IXSCEint.h"*/
#include "IXSCEerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* masks for interpreting the electronic key */
#define k_ELKEY_MASK_COMPATIBILITY   0x80U
#define k_ELKEY_MASK_MAJ_REV         0x7FU

/* Relative byte offset of the fields in an Electronic Key Logical Segment. */
#define k_OFS_ELKEY_VENDID        2U    /* Vendor ID */
#define k_OFS_ELKEY_DEVTYPE       4U    /* Device Type */
#define k_OFS_ELKEY_PRODCOD       6U    /* Product Code */
#define k_OFS_ELKEY_COMP_MAJ_REV  8U    /* Compatibility Bit / Major Revision */
#define k_OFS_ELKEY_MIN_REV       9U    /* Minor Revision */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_ElectronicKeyParse
**
** Description : This function parses the fields of an Electronic Key in an
**               Explicit message and stores the values in the passed Electronic
**               Key structure.
**
** Parameters  : pba_em (IN)    - pointer to the received explicit
**                                message (byte stream)
**                                (valid range: <> CSS_k_NULL, checked)
**               ps_elKey (OUT) - pointer to the structure to where the
**                                data of the electronic key segment is to be
**                                stored
**                                (not checked, only called with reference to
**                                structure variable)
**
** Returnvalue : CSS_k_OK       - success
**               <>CSS_k_OK     - error while processing the message
**
*******************************************************************************/
CSS_t_WORD IXSCE_ElectronicKeyParse(const CSS_t_BYTE *pba_em,
                                    CSS_t_ELECTRONIC_KEY *ps_elKey)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSCE_k_FSE_INC_PRG_FLOW;

  /* if passed pointer is invalid */
  if (pba_em == CSS_k_NULL)
  {
    w_retVal = (IXSCE_k_FSE_INC_PTR_INV_KEY_PRS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: pointer is valid */
  {
    /* Electronic Key segment must be present in all SafetyOpen (see FRS341)  */
    /* parse Electronic Key segment */

    CSS_N2H_CPY16(&ps_elKey->u16_vendId,
                  CSS_ADD_OFFSET(pba_em, k_OFS_ELKEY_VENDID));
    CSS_N2H_CPY16(&ps_elKey->u16_devType,
                  CSS_ADD_OFFSET(pba_em, k_OFS_ELKEY_DEVTYPE));
    CSS_N2H_CPY16(&ps_elKey->u16_prodCode,
                  CSS_ADD_OFFSET(pba_em, k_OFS_ELKEY_PRODCOD));
    CSS_N2H_CPY8(&ps_elKey->b_compMajorRev,
                  CSS_ADD_OFFSET(pba_em, k_OFS_ELKEY_COMP_MAJ_REV));
    CSS_N2H_CPY8(&ps_elKey->u8_minorRev,
                  CSS_ADD_OFFSET(pba_em, k_OFS_ELKEY_MIN_REV));

    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSCE_ElectronicKeyMatchCheck
**
** Description : This function checks if the Electronic Key in a received
**               Explicit Request matches ours.
**
** Parameters  : ps_elKey (IN)      - pointer to Electronic Key structure of
**                                    received parameters
**                                    (not checked, only called with
**                                    reference to structure variable)
**               pu8_genStat (OUT)  - pointer to the general Status Code to be
**                                    returned in the response message.
**                                    (not checked, only called with reference
**                                    to variable)
**               pu16_extStat (OUT) - pointer to the extended Status Code to be
**                                    returned in the response message.
**                                    Extended Status Codes concerning
**                                    Electronic Key checking of the Connection
**                                    Manager Object are Identical with the
**                                    Message Router Object.
**                                    (not checked, only called with reference
**                                    to variable)
**
** Returnvalue : CSS_k_TRUE         - Electronic Key matches
**               CSS_k_FALSE        - Electronic Key does not match
**
*******************************************************************************/
CSS_t_BOOL IXSCE_ElectronicKeyMatchCheck(const CSS_t_ELECTRONIC_KEY *ps_elKey,
                                         CSS_t_USINT *pu8_genStat,
                                         CSS_t_UINT *pu16_extStat)
{
  /* return value of this function: flag indicating if Electronic Key matches */
  CSS_t_BOOL o_elKeyMatch = CSS_k_FALSE;

  /* any valid path of this function will overwrite these initial values */
  *pu8_genStat = CSOS_k_CGSC_INVALID_STATUS_CODE;
  *pu16_extStat = CSS_k_CMEXTSTAT_UNDEFINED;

  /*
  ** As this check is performed here in CSS (and optionally in the Safety
  ** Application) it is processed with high integrity (see FRS342).
  */

  /*
  ** As the additional error codes for Electronic Keys directed to the Message
  ** Router Object are identical to those of the Connection Manager Object
  ** no differentiation is necessary for these two cases.
  */

  /*
  ** Wildcards (fields set to 0) are not allowed for safety devices
  ** (see FRS344).
  */

  /* if Vendor Id or Product Code contain wildcards */
  if ((ps_elKey->u16_vendId == 0U) || (ps_elKey->u16_prodCode == 0U))
  {
    /* o_elKeyMatch already initialized to CSS_k_FALSE */
    *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
    *pu16_extStat = CSS_k_CMEXTSTAT_VID_OR_PRODC;
    SAPL_CssErrorClbk(IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_1,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: Vendor ID and Product Code don't contain wildcards */
  /* if Device Type is zero */
  else if (ps_elKey->u16_devType == 0U)
  {
    /* We also forbid 0 for Device Type. Deprecated Generic devices are not
     supported! */
    /* o_elKeyMatch already initialized to CSS_k_FALSE */
    *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
    *pu16_extStat = CSS_k_CMEXTSTAT_DEV_TYPE;
    SAPL_CssErrorClbk(IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_2,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* else: Vendor ID, Product Code and Device Type all non zero */
  /* if Major or Minor Revision is zero */
  else if (    ((ps_elKey->b_compMajorRev & k_ELKEY_MASK_MAJ_REV) == 0U)
            || (ps_elKey->u8_minorRev == 0U)
          )
  {
    /* o_elKeyMatch already initialized to CSS_k_FALSE */
    *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
    *pu16_extStat = CSS_k_CMEXTSTAT_REVISION;
    SAPL_CssErrorClbk(IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_3,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: non of the fields in the electronic keys contain zero */
  {
    /* auxiliary error code to inform SAPL about exact protocol error */
    CSS_t_WORD  w_auxErrCode = (IXSCE_k_FSE_INC_PRG_FLOW);

    /* if Vendor ID and Product Code match exactly */
    if (    (ps_elKey->u16_vendId == IXSSS_VendorIdGet())
         && (ps_elKey->u16_prodCode == IXSSS_ProductCodeGet())
       )                          /*lint !e960 (function has no side effects) */
    {
      /* if Device Type matches exactly */
      if (IXSSS_DeviceTypeGet() == ps_elKey->u16_devType)
      {
        /* if Major and Minor Revision match exactly */
        if (    ((ps_elKey->b_compMajorRev & k_ELKEY_MASK_MAJ_REV)
                  == IXSSS_MajorRevGet())
             && (ps_elKey->u8_minorRev == IXSSS_MinorRevGet())
           )                      /*lint !e960 (function has no side effects) */
        {
          /* received Electronic Key matches our device's Key exactly */
          o_elKeyMatch = CSS_k_TRUE;
          *pu8_genStat = CSOS_k_CGSC_SUCCESS;
          *pu16_extStat = CSS_k_CMEXTSTAT_UNDEFINED;
        }
        else /* else: Major/Minor Revision don't match exactly */
        {
          /* o_elKeyMatch already initialized to CSS_k_FALSE */
          *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
          *pu16_extStat = CSS_k_CMEXTSTAT_REVISION;
          w_auxErrCode = (IXSCE_k_NFSE_RXE_SO_EL_KEY_3);
        }
      }
      else /* else: Device Type doesn't match */
      {
        /* o_elKeyMatch already initialized to CSS_k_FALSE */
        *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
        *pu16_extStat = CSS_k_CMEXTSTAT_DEV_TYPE;
        w_auxErrCode = (IXSCE_k_NFSE_RXE_SO_EL_KEY_2);
      }
    }
    else /* else: Vendor ID and Product code don't match */
    {
      /* o_elKeyMatch already initialized to CSS_k_FALSE */
      *pu8_genStat = CSOS_k_CGSC_CNXN_FAILURE;
      *pu16_extStat = CSS_k_CMEXTSTAT_VID_OR_PRODC;
      w_auxErrCode = (IXSCE_k_NFSE_RXE_SO_EL_KEY_1);
    }

    /* if key matches exactly */
    if (o_elKeyMatch)
    {
      /* nothing to be done here */
      /* return values already set */
    }
    else /* else: key doesn't match exactly */
    {
      /* if compatibility bit is set */
      if (ps_elKey->b_compMajorRev & k_ELKEY_MASK_COMPATIBILITY)
      {
        /* if application decides that this Electronic Key can be accepted */
        if (SAPL_IxsceCompElectronicKeyClbk(ps_elKey))
        {
          /* Safety application can emulate electronic key. */
          /* Connection is permitted */
          o_elKeyMatch = CSS_k_TRUE;
          /* status codes to be returned had already been set to error, so
             we have to reassign success */
          *pu8_genStat = CSOS_k_CGSC_SUCCESS;
          *pu16_extStat = CSS_k_CMEXTSTAT_UNDEFINED;
        }
        else /* else: Safety Application does not permit the connection */
        {
          SAPL_CssErrorClbk(w_auxErrCode,
                            IXSER_k_I_NOT_USED, (CSS_t_DWORD)CSS_k_TRUE);
        }
      }
      else /* else: Compatibility Bit is not set */
      {
        /* Compatibility Bit (0) enforces exactly matching Electronic Key     */
        /* (see FRS344).  As it is not matching we return the already set     */
        /* error code and thus won't accept the connection.                   */
        SAPL_CssErrorClbk(w_auxErrCode,
                          IXSER_k_I_NOT_USED, (CSS_t_DWORD)CSS_k_FALSE);
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_elKeyMatch);
}


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


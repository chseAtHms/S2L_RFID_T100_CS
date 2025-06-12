/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPPlogicalSeg.c
**    Summary: IXEPP - Encoded Path Parser
**             This module contains functions to parse the logical segments in
**             EPATH strings.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_EPathParseSegLogical
**
**             EPathParseSegLogicalVal8
**             EPathParseSegLogicalVal16_32
**             EPathClassFound
**             EPathInstanceFound
**             EPathCnxnPointFound
**             EPathFieldSetValid
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
#include "IXSCF.h"
#include "CSSapi.h"

#include "IXSERapi.h"

#include "IXEPP.h"
#include "IXEPPint.h"
#include "IXEPPerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** LOGICAL_FORMAT_8:
    Definition of 8-bit logical segment format.
*/
#define LOGICAL_FORMAT_8    0x00U

/** LOGICAL_FORMAT_GET:
    Returns the format from the passed byte containing a logical segment
    descriptor.
*/
#define LOGICAL_FORMAT_GET(u8_segTypeFormat)   (u8_segTypeFormat & 0x03U)


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_USINT EPathParseSegLogicalVal8(CSS_t_USINT u8_segTypeFormat,
                                         const CSS_t_BYTE *pb_logicalVal,
                                         CSS_t_UINT *pu16_procBytes,
                                         CSS_t_DWORD *pdw_valid,
                                         IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);
static CSS_t_USINT EPathParseSegLogicalVal16_32(CSS_t_USINT u8_segTypeFormat,
                                         const CSS_t_BYTE *pb_logicalVal,
                                         CSS_t_UINT *pu16_procBytes,
                                         CSS_t_DWORD *pdw_valid,
                                         IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);
static CSS_t_UINT* EPathClassFound(CSS_t_DWORD *pdw_valid,
                                   IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);
static CSS_t_UDINT* EPathInstanceFound(CSS_t_DWORD *pdw_valid,
                                       IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);
static CSS_t_UDINT* EPathCnxnPointFound(CSS_t_DWORD *pdw_valid,
                                        IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);
static CSS_t_BOOL EPathFieldSetValid(CSS_t_DWORD *pdw_valid,
                                     CSS_t_DWORD dw_fieldBit);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXEPP_EPathParseSegLogical
**
** Description : This function parses a logical segment inside an EPATH. It uses
**               several sub functions to parse 8- 16- or 32-bit logical
**               segments.
**
** See Also    : EPathParseSegLogicalVal8(), EPathParseSegLogicalVal16_32()
**
** Parameters  : u8_segTypeFormat (IN) - segment Type/Format byte
**                                       (not checked, any value allowed)
**               pb_ePath (IN)         - pointer to a logical segment in the
**                                       EPATH to be processed by this function
**                                       (not checked, checked in calling
**                                       functions)
**               o_padded (IN)         - CSS_k_TRUE  : EPATH is padded
**                                       CSS_k_FALSE : EPATH is packed
**                                       (not checked, any value allowed)
**               pu16_procBytes (OUT)  - number of bytes that are processed by
**                                       the function in this segment of the
**                                       EPATH
**                                       (not checked, only called with
**                                       reference to variable)
**               pdw_valid (OUT)       - bit string to signal which EPATH fields
**                                       are found
**                                       (not checked, only called with
**                                       reference to variable)
**               ps_ePathLogSeg (OUT)  - pointer to the Logical Segment
**                                       structure in the EPATH Fields structure
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_t_USINT           - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS - success
**               !=CSOS_k_CGSC_SUCCESS - protocol failure
**
*******************************************************************************/
CSS_t_USINT IXEPP_EPathParseSegLogical(CSS_t_USINT u8_segTypeFormat,
                                       const CSS_t_BYTE *pb_ePath,
                                       CSS_t_BOOL o_padded,
                                       CSS_t_UINT *pu16_procBytes,
                                       CSS_t_DWORD *pdw_valid,
                                       IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  /* pointer to a Logical Value in the EPATH to be processed */
  const CSS_t_BYTE *pb_logicalVal = CSS_k_NULL;

  /* *pu16_procBytes not set to zero here because this function calls
     sub-functions to do the processing. */

  /* if the logical value format is 8bit (no pad byte is available) */
  if (LOGICAL_FORMAT_GET(u8_segTypeFormat) == LOGICAL_FORMAT_8)
  {
    /* set pointer to value (jump over segment type) */
    pb_logicalVal = CSS_ADD_OFFSET(pb_ePath, CSOS_k_SIZEOF_USINT);
    u8_genStatus = EPathParseSegLogicalVal8(u8_segTypeFormat, pb_logicalVal,
                                            pu16_procBytes, pdw_valid,
                                            ps_ePathLogSeg);

    /* if previous function succeeded */
    if (u8_genStatus == CSOS_k_CGSC_SUCCESS)
    {
      /* Segment Type/Format byte is processed */
      *pu16_procBytes = (CSS_t_UINT)(*pu16_procBytes + CSOS_k_SIZEOF_USINT);
    }
    else /* else: an error occurred in previous called function */
    {
      /* error code already set - to be handled in calling function */
    }
  }
  else /* logical value format is not 8bit (pad byte is available)*/
  {
    /* if the EPATH format is padded */
    if (o_padded)
    {
      /* pad byte is available and ignored */
      pb_logicalVal = CSS_ADD_OFFSET(pb_ePath, 2U * CSOS_k_SIZEOF_USINT);
      u8_genStatus = EPathParseSegLogicalVal16_32(u8_segTypeFormat,
                                                  pb_logicalVal, pu16_procBytes,
                                                  pdw_valid, ps_ePathLogSeg);

      /* if previous function succeeded */
      if (u8_genStatus == CSOS_k_CGSC_SUCCESS)
      {
        /* Segment Type/Format byte is processed */
        *pu16_procBytes = (CSS_t_UINT)(*pu16_procBytes
                                       + (2U * CSOS_k_SIZEOF_USINT));
      }
      else /* else: an error occurred in previous called function */
      {
        /* error code already set - to be handled in calling function */
      }
    }
    else /* the EPATH format is packed */
    {
      /* pad byte is not available */
      pb_logicalVal = CSS_ADD_OFFSET(pb_ePath, CSOS_k_SIZEOF_USINT);
      u8_genStatus = EPathParseSegLogicalVal16_32(u8_segTypeFormat,
                                                  pb_logicalVal, pu16_procBytes,
                                                  pdw_valid, ps_ePathLogSeg);

      /* if previous function succeeded */
      if (u8_genStatus == CSOS_k_CGSC_SUCCESS)
      {
        /* Segment Type/Format byte is processed */
        *pu16_procBytes = (CSS_t_UINT)(*pu16_procBytes + CSOS_k_SIZEOF_USINT);
      }
      else /* else: an error occurred in previous called function */
      {
        /* error code already set - to be handled in calling function */
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_genStatus);
}


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : EPathParseSegLogicalVal8
**
** Description : This function parses an 8-bit logical segment inside an EPATH.
**
** Parameters  : u8_segTypeFormat (IN) - segment Type/Format byte
**                                       (not checked, any value allowed)
**               pb_logicalVal (IN)    - pointer to the logical value to be
**                                       processed
**                                       (not checked, only called with pointer
**                                       with offset to pointer checked in
**                                       calling function)
**               pu16_procBytes (OUT)  - number of bytes that are processed by
**                                       the function in this segment of the
**                                       EPATH
**                                       (not checked, only called with
**                                       reference to variable)
**               pdw_valid (OUT)       - bit string to signal which EPATH fields
**                                       are found
**                                       (not checked, only called with
**                                       reference to variable)
**               ps_ePathLogSeg (OUT)  - pointer to the Logical Segment
**                                       structure in the EPATH Fields structure
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_t_USINT           - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS - success
**               !=CSOS_k_CGSC_SUCCESS - protocol failure
**
*******************************************************************************/
static CSS_t_USINT EPathParseSegLogicalVal8(CSS_t_USINT u8_segTypeFormat,
                                         const CSS_t_BYTE *pb_logicalVal,
                                         CSS_t_UINT *pu16_procBytes,
                                         CSS_t_DWORD *pdw_valid,
                                         IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  CSS_t_USINT u8_tmp; /* temporary variable to copy the different IDs */
  CSS_t_UDINT *pu32_inst = CSS_k_NULL; /* pointer to the instance ID */
  CSS_t_UINT *pu16_cl = CSS_k_NULL;    /* pointer to the class ID */
  CSS_t_UDINT *pu32_cp = CSS_k_NULL;   /* pointer to the connection point */

  *pu16_procBytes = 0U;

  switch (u8_segTypeFormat)
  {
    case CSOS_k_LOG_SEG_CLASS_ID_8:
    {
      pu16_cl = EPathClassFound(pdw_valid, ps_ePathLogSeg);

      if (pu16_cl != CSS_k_NULL)
      {
        CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);
        *pu16_cl = u8_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_USINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_CLASS1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_INSTANCE_ID_8:
    {
      pu32_inst = EPathInstanceFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_inst != CSS_k_NULL)
      {
        CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);
        *pu32_inst = u8_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_USINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_INST1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_ATTRIBUTE_ID_8:
    {
      /* ATTRIBUTE ID U8 is extracted */
      CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);
      ps_ePathLogSeg->u16_attrId = u8_tmp;
      *pu16_procBytes = CSOS_k_SIZEOF_USINT;

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_ATTR_ID))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_ATTR1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
      break;
    }

    case CSOS_k_LOG_SEG_MEMBER_ID_8:
    {
      /* MEMBER ID U8 is extracted */
      CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);
      ps_ePathLogSeg->u32_membId = u8_tmp;
      *pu16_procBytes = CSOS_k_SIZEOF_USINT;

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_MEMB_ID))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_MEMB1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      }
      break;
    }

    case CSOS_k_LOG_SEG_CNXN_POINT_8:
    {
      pu32_cp = EPathCnxnPointFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_cp != CSS_k_NULL)
      {
        CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);
        *pu32_cp = u8_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_USINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_CP1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_ELECTRONIC_KEY:
    {
      CSS_N2H_CPY8(&u8_tmp, pb_logicalVal);

      /* if the key format is key format table */
      if (u8_tmp == CSOS_k_KEY_FORMAT_TABLE)
      {
        *pu16_procBytes = (CSOS_k_SIZEOF_USINT + /* Key format */
                           CSOS_k_SIZEOF_UINT  + /* Vendor ID */
                           CSOS_k_SIZEOF_UINT  + /* Device Type */
                           CSOS_k_SIZEOF_UINT  + /* Product Code */
                           CSOS_k_SIZEOF_BYTE  + /* Major Revision */
                           CSOS_k_SIZEOF_USINT); /* Minor Revision */

        /* if EPATH field has not been found yet */
        if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_ELECTRONIC_KEY))
        {
          /* store the pointer to (beginning of) the electronic key segment */
          ps_ePathLogSeg->pba_elKey = (CSS_t_BYTE*)pb_logicalVal - 1U;
          u8_genStatus = CSOS_k_CGSC_SUCCESS;
        }
        else /* repeated field has been found */
        {
          SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_EKEY,
                            IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
          u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
        }
      }
      else
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX3,
                          IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_tmp);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    default:
    {
      SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX4,
                        IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_segTypeFormat);
      u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_genStatus);
}


/*******************************************************************************
**
** Function    : EPathParseSegLogicalVal16_32
**
** Description : This function parses a 16- and 32-bit logical segment inside an
**               EPATH.
**
** Parameters  : u8_segTypeFormat (IN) - segment Type/Format byte
**                                       (not checked, any value allowed)
**               pb_logicalVal (IN)    - pointer to the logical value to be
**                                       processed
**                                       (not checked, only called with pointer
**                                       with offset to pointer checked in
**                                       calling function)
**               pu16_procBytes (OUT)  - number of bytes that are processed by
**                                       the function in this segment of the
**                                       EPATH
**                                       (not checked, only called with
**                                       reference to variable)
**               pdw_valid (OUT)       - bit string to signal which EPATH fields
**                                       are found
**                                       (not checked, only called with
**                                       reference to variable)
**               ps_ePathLogSeg (OUT)  - pointer to the Logical Segment
**                                       structure in the EPATH Fields structure
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_t_USINT           - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS - success
**               !=CSOS_k_CGSC_SUCCESS - protocol failure
**
*******************************************************************************/
static CSS_t_USINT EPathParseSegLogicalVal16_32(CSS_t_USINT u8_segTypeFormat,
                                          const CSS_t_BYTE *pb_logicalVal,
                                          CSS_t_UINT *pu16_procBytes,
                                          CSS_t_DWORD *pdw_valid,
                                          IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  CSS_t_UINT u16_tmp; /* temporary variable to copy the different IDs */
  CSS_t_UDINT *pu32_inst = CSS_k_NULL; /* pointer to the instance ID */
  CSS_t_UINT *pu16_cl = CSS_k_NULL;    /* pointer to the class ID */
  CSS_t_UDINT *pu32_cp = CSS_k_NULL;   /* pointer to the connection point */

  *pu16_procBytes = 0U;

  /* switch for the Segment Type/Format byte */
  switch (u8_segTypeFormat)
  {
    case CSOS_k_LOG_SEG_CLASS_ID_16:
    {
      pu16_cl = EPathClassFound(pdw_valid, ps_ePathLogSeg);

      if (pu16_cl != CSS_k_NULL)
      {
        CSS_N2H_CPY16(&u16_tmp, pb_logicalVal);
        *pu16_cl = u16_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_UINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_CLASS2,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_INSTANCE_ID_16:
    {
      pu32_inst = EPathInstanceFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_inst != CSS_k_NULL)
      {
        CSS_N2H_CPY16(&u16_tmp, pb_logicalVal);
        *pu32_inst = u16_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_UINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_INST2,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_INSTANCE_ID_32:
    {
      pu32_inst = EPathInstanceFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_inst != CSS_k_NULL)
      {
        CSS_N2H_CPY32(pu32_inst, pb_logicalVal);
        *pu16_procBytes = CSOS_k_SIZEOF_UDINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_INST3,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_ATTRIBUTE_ID_16:
    {
      /* ATTRIBUTE ID U16 is extracted */
      CSS_N2H_CPY16(&u16_tmp, pb_logicalVal);
      ps_ePathLogSeg->u16_attrId = u16_tmp;
      *pu16_procBytes = CSOS_k_SIZEOF_UINT;

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_ATTR_ID))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_ATTR2,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_MEMBER_ID_16:
    {
      /* MEMBER ID U16 is extracted */
      CSS_N2H_CPY16(&u16_tmp, pb_logicalVal);
      ps_ePathLogSeg->u32_membId = u16_tmp;
      *pu16_procBytes = CSOS_k_SIZEOF_UINT;

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_MEMB_ID))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_MEMB2,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_MEMBER_ID_32:
    {
      /* MEMBER ID U32 is extracted */
      CSS_N2H_CPY32(&ps_ePathLogSeg->u32_membId, pb_logicalVal);
      *pu16_procBytes = CSOS_k_SIZEOF_UDINT;

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_MEMB_ID))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_MEMB3,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_CNXN_POINT_16:
    {
      pu32_cp = EPathCnxnPointFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_cp != CSS_k_NULL)
      {
        CSS_N2H_CPY16(&u16_tmp, pb_logicalVal);
        *pu32_cp = u16_tmp;
        *pu16_procBytes = CSOS_k_SIZEOF_UINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_CP2,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    case CSOS_k_LOG_SEG_CNXN_POINT_32:
    {
      pu32_cp = EPathCnxnPointFound(pdw_valid, ps_ePathLogSeg);

      if (pu32_cp != CSS_k_NULL)
      {
        CSS_N2H_CPY32(pu32_cp, pb_logicalVal);
        *pu16_procBytes = CSOS_k_SIZEOF_UDINT;
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_CP3,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }
      break;
    }

    default:
    {
      SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX5,
                        IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_segTypeFormat);
      u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_genStatus);
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** Function    : EPathClassFound
**
** Description : This function marks a Class ID as found in the bit-mask and
**               returns the Class ID. This function is prepared for 3 Class
**               IDs. If more are found then an error will be indicated by
**               returning a NULL-pointer.
**
** Parameters  : pdw_valid (OUT)      - bit string to signal which EPATH fields
**                                      are valid
**                                      (not checked, only called with reference
**                                      to variable)
**               ps_ePathLogSeg (OUT) - pointer to the Logical Segment
**                                      structure in the EPATH Fields structure
**                                      (not checked, only called with
**                                      reference to struct)
**
** Returnvalue : != CSS_k_NULL        - pointer to where the parsed field is to
**                                      be stored
**               == CSS_k_NULL        - error happened
**
*******************************************************************************/
static CSS_t_UINT* EPathClassFound(CSS_t_DWORD *pdw_valid,
                                   IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_UINT *pu16_retVal = CSS_k_NULL;

  /* if EPATH field has not been found yet */
  if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_CLASS_ID_1))
  {
    pu16_retVal = &ps_ePathLogSeg->u16_classId1;
  }
  /* else if EPATH field has not been found yet */
  else if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_CLASS_ID_2))
  {
    pu16_retVal = &ps_ePathLogSeg->u16_classId2;
  }
  /* else if EPATH field has not been found yet */
  else if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_CLASS_ID_3))
  {
    pu16_retVal = &ps_ePathLogSeg->u16_classId3;
  }
  else /* repeated field has been found */
  {
    /* function will indicate this to the caller by returning a NULL-pointer */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (pu16_retVal);
}


/*******************************************************************************
**
** Function    : EPathInstanceFound
**
** Description : This function marks an Instance ID as found in the bit-mask and
**               returns the Instance ID. This function is prepared for 3
**               Instance IDs. If more are found then an error will be indicated
**               by returning a NULL-pointer.
**
** Parameters  : pdw_valid (OUT)     - bit string to signal which EPATH fields
**                                     (not checked, only called with reference
**                                     to variable)
**               ps_ePathLogSeg (IN) - pointer to the Logical Segment
**                                     structure in the EPATH Fields structure
**                                     (not checked, only called with
**                                     reference to struct)
**
** Returnvalue : != CSS_k_NULL       - pointer to where the parsed field is to
**                                     be stored
**               == CSS_k_NULL       - error happened, see pw_errCode
**
*******************************************************************************/
static CSS_t_UDINT* EPathInstanceFound(CSS_t_DWORD *pdw_valid,
                                       IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_UDINT *pu32_retVal = CSS_k_NULL;

  /* if EPATH field has not been found yet */
  if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_INST_ID_1))
  {
    pu32_retVal = &ps_ePathLogSeg->u32_instId1;
  }
  /* else if EPATH field has not been found yet */
  else if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_INST_ID_2))
  {
    pu32_retVal = &ps_ePathLogSeg->u32_instId2;
  }
  /* else if EPATH field has not been found yet */
  else if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_INST_ID_3))
  {
    pu32_retVal = &ps_ePathLogSeg->u32_instId3;
  }
  else /* repeated field has been found */
  {
    /* function will indicate this to the caller by returning a NULL-pointer */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (pu32_retVal);
}


/*******************************************************************************
**
** Function    : EPathCnxnPointFound
**
** Description : This function marks a connection point as found in the bit-mask
**               and returns the connection point ID. This function is prepared
**               for 2 connection points. If more are found then an error will
**               be indicated by returning a NULL-pointer.
**
** Parameters  : pdw_valid (OUT)     - bit string to signal which EPATH fields
**                                     are valid
**                                     (not checked, only called with reference
**                                     to variable)
**               ps_ePathLogSeg (IN) - pointer to the Logical Segment
**                                     structure in the EPATH Fields structure
**                                     (not checked, only called with
**                                     reference to struct)
**
** Returnvalue : != CSS_k_NULL       - pointer to where the parsed field is to
**                                     be stored
**               == CSS_k_NULL       - error happened
**
*******************************************************************************/
static CSS_t_UDINT* EPathCnxnPointFound(CSS_t_DWORD *pdw_valid,
                                        IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg)
{
  /* return value */
  CSS_t_UDINT *pu32_retVal = CSS_k_NULL;

  /* if EPATH field has not been found yet */
  if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_CNXN_POINT_1))
  {
    pu32_retVal = &ps_ePathLogSeg->u32_cpId1;
  }
  /* else if EPATH field has not been found yet */
  else if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_CNXN_POINT_2))
  {
    pu32_retVal = &ps_ePathLogSeg->u32_cpId2;
  }
  else /* repeated field has been found */
  {
    /* function will indicate this to the caller by returning a NULL-pointer */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (pu32_retVal);
}


/*******************************************************************************
**
** Function    : EPathFieldSetValid
**
** Description : This function sets a bit in the passed DWORD according to the
**               passed mask. In case the bit was already set the function
**               indicates this by returning CSS_k_FALSE.
**
** Parameters  : pdw_valid (IN)   - bit string to signal which EPATH fields
**                                  (not checked, only called with reference
**                                  to variable)
**               dw_fieldBit (IN) - EPATH field bit that was found
**                                  (not checked, only called with constants)
**
** Returnvalue : CSS_k_TRUE       - success
**               CSS_k_FALSE      - failure
**
*******************************************************************************/
static CSS_t_BOOL EPathFieldSetValid(CSS_t_DWORD *pdw_valid,
                                     CSS_t_DWORD dw_fieldBit)
{
  /* return value */
  CSS_t_BOOL o_ret = CSS_k_FALSE;

  /* if the field has not been found yet */
  if (((*pdw_valid) & dw_fieldBit) != dw_fieldBit)
  {
    *pdw_valid = ((*pdw_valid) | dw_fieldBit);
    o_ret = CSS_k_TRUE;
  }
  else
  {
    /* repeated field has been found */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_ret);
}


/*** End Of File ***/


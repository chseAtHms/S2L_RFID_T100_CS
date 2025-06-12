/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPPmain.c
**    Summary: IXEPP - Encoded Path Parser
**             This module contains functions to parse EPATH strings.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_EPathParse
**
**             EPathParseSegData
**             EPathParseSegNetwork
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


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_USINT EPathParseSegData(CSS_t_USINT u8_segTypeFormat,
                                     const CSS_t_BYTE *pb_ePath,
                                     CSS_t_UINT *pu16_procBytes,
                                     CSS_t_DWORD *pdw_valid,
                                     IXEPP_t_FIELD_DATA_SEG *ps_ePathDataSeg);
static CSS_t_USINT EPathParseSegNetwork(CSS_t_USINT u8_segTypeFormat,
                                        const CSS_t_BYTE *pb_ePath,
                                        CSS_t_UINT *pu16_procBytes,
                                        CSS_t_DWORD *pdw_valid,
                                        IXEPP_t_FIELD_NET_SEG *ps_ePathNetSeg);

static CSS_t_BOOL EPathFieldSetValid(CSS_t_DWORD *pdw_valid,
                                     CSS_t_DWORD dw_fieldBit);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXEPP_EPathParse
**
** Description : This function parses the passed EPATH string. A bit field
**               indicates what segments have been found (and parsed) in the
**               EPATH and the parsed values are placed into a structure.
**
** Parameters  : pb_ePath (IN)          - pointer to the EPATH (checked,
**                                        valid range: <> CSS_k_NULL)
**               o_padded (IN)          - CSS_k_TRUE  : EPATH is padded
**                                        CSS_k_FALSE : EPATH is packed
**                                        (not checked, any value allowed)
**               u16_reqPathSize (IN)   - size of the Request_Path (number of
**                                        bytes)
**                                        (not checked, any value allowed)
**               pdw_valid (OUT)        - bit string to signal what has been
**                                        found in the EPATH. See the bit
**                                        defines {IXEPP_k_SFB_xxx}
**                                        (not checked, only called with
**                                        reference to variable)
**               ps_ePathFields (OUT)   - pointer to structure that contains the
**                                        parsed EPATH field values.
**                                        (not checked, only called with
**                                        reference to struct)
**
** Returnvalue : CSS_t_USINT            - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS  - success
**               !=CSOS_k_CGSC_SUCCESS  - protocol failure
**
*******************************************************************************/
CSS_t_USINT IXEPP_EPathParse(const CSS_t_BYTE *pb_ePath,
                             CSS_t_BOOL o_padded,
                             CSS_t_UINT u16_reqPathSize,
                             CSS_t_DWORD *pdw_valid,
                             IXEPP_t_FIELDS *ps_ePathFields)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  CSS_t_BOOL o_break = CSS_k_FALSE; /* flag to break the while loop */
  CSS_t_USINT u8_segTypeFormat = 0U; /* Segment Type/Format byte */
  CSS_t_UINT u16_procBytes = 0U; /* number of processed bytes */
  /* pointer to the EPATH field to be processed */
  const CSS_t_BYTE *pb_ePathField = pb_ePath;
  /* number of bytes not yet processed:
     at the beginning the complete path is remaining */
  CSS_t_UINT u16_remPathSize = u16_reqPathSize;

  /* if passed pointer is invalid */
  if (pb_ePath == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXEPP_k_FSE_INC_PTR_INV_EPPARSE,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    u8_genStatus = CSOS_k_CGSC_RES_UNAVAIL;
  }
  else /* passed pointer is valid */
  {
    /* parse the segments in the path one after another */
    do
    {
      /* SEGMENT TYPE/FORMAT byte is extracted */
      CSS_N2H_CPY8(&u8_segTypeFormat, pb_ePathField);

      /* switch for the segment type */
      switch (CSOS_SEG_TYPE_GET(u8_segTypeFormat))
      {
        /*
        ** Each of the functions called in the cases below processes one segment
        ** and are not aware of other segments existing possibly before or after
        ** it. Thus the passed pointer pb_ePathField is a pointer to the segment
        ** that is to be processed (somewhere inside pb_ePath). The function
        ** parameter u16_procBytes is strictly an OUT-parameter for the parsing
        ** functions. This means that the parse functions ignore the value on IN
        ** but initialize and set it before they return. This output value is
        ** then evaluated here in this function after this switch statement.
        */

        case CSOS_k_SEG_TYPE_LOGICAL:
        {
          u8_genStatus = IXEPP_EPathParseSegLogical(u8_segTypeFormat,
                                                    pb_ePathField, o_padded,
                                                    &u16_procBytes, pdw_valid,
                                                    &ps_ePathFields->s_logSeg);

          /* update the number of bytes in logical segments */
          ps_ePathFields->u16_elKeyAndAppPathSize = (CSS_t_UINT)
            (ps_ePathFields->u16_elKeyAndAppPathSize + u16_procBytes);
          break;
        }

        case CSOS_k_SEG_TYPE_DATA:
        {
          u8_genStatus = EPathParseSegData(u8_segTypeFormat, pb_ePathField,
                                           &u16_procBytes, pdw_valid,
                                           &ps_ePathFields->s_dataSeg);
          break;
        }

        case CSOS_k_SEG_TYPE_NETWORK:
        {
          u8_genStatus = EPathParseSegNetwork(u8_segTypeFormat, pb_ePathField,
                                              &u16_procBytes, pdw_valid,
                                              &ps_ePathFields->s_netSeg);
          break;
        }

        default:
        {
          /* An unknown or invalid segment was found. Report this error to the
             application and set the error code. This also causes that
             processing the EPATH is aborted. */
          /* The calling function may throw another error as the number of
             bytes processed by this function is shorter than the EPATH
             length */
          SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX1, IXSER_k_I_NOT_USED,
                            (CSS_t_UDINT)u8_segTypeFormat);
          u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
        }
      }

      /* if a parse error has already been detected */
      /* or if no data have been processed. This is just an additional safety
         check to prevent that this function could ever get stuck in an infinite
         loop. */
      if ((u8_genStatus != CSOS_k_CGSC_SUCCESS) || (u16_procBytes == 0U))
      {
        /* Stop parsing. Error Code is already set */
        o_break = CSS_k_TRUE;
      }
      else
      {
        /* if the parsing is still within the EPATH string */
        if (u16_procBytes > u16_remPathSize)
        {
          /* we already parsed beyond the EPATH string! */
          /* Stop parsing. Set Error Code */
          SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_TOO_SMALL,
                            IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
          u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
          o_break = CSS_k_TRUE;
        }
        else /* else: the EPATH size is right */
        {
          u16_remPathSize = (CSS_t_UINT)(u16_remPathSize - u16_procBytes);

          /* if the end of the string is not yet reached */
          if (u16_remPathSize != 0U)
          {
            /* Advance in EPATH string - continue parsing */
            pb_ePathField = CSS_ADD_OFFSET(pb_ePathField, u16_procBytes);
          }
          else /* else: end of the EPATH is reached */
          {
            /* Stop parsing. Success */
            o_break = CSS_k_TRUE;
            u8_genStatus = CSOS_k_CGSC_SUCCESS;
          }
        }
      }
    }
    /* stop the loop if the o_break flag is set */
    while (o_break == CSS_k_FALSE);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_genStatus);
}


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : EPathParseSegData
**
** Description : This function parses a simple data segment inside an EPATH.
**
** Parameters  : u8_segTypeFormat (IN) - segment Type/Format byte
**                                       (not checked, any value allowed)
**               pb_ePath (IN)         - pointer to a logical segment in the
**                                       EPATH to be processed by this function
**                                       (not checked, checked in calling
**                                       functions)
**               pu16_procBytes (OUT)  - number of bytes that are processed by
**                                       the function in this segment of the
**                                       EPATH
**                                       (not checked, only called with
**                                       reference to variable)
**               pdw_valid (OUT)       - bit string to signal which EPATH fields
**                                       are found
**                                       (not checked, only called with
**                                       reference to variable)
**               ps_ePathDataSeg (OUT) - pointer to the Data Segment structure
**                                       in the EPATH Fields structure
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_t_USINT           - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS - success
**               !=CSOS_k_CGSC_SUCCESS - protocol failure
**
*******************************************************************************/
static CSS_t_USINT EPathParseSegData(CSS_t_USINT u8_segTypeFormat,
                                     const CSS_t_BYTE *pb_ePath,
                                     CSS_t_UINT *pu16_procBytes,
                                     CSS_t_DWORD *pdw_valid,
                                     IXEPP_t_FIELD_DATA_SEG *ps_ePathDataSeg)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  /* pointer to the data segment length */
  const CSS_t_USINT *pb_dataSegLen = CSS_k_NULL;
  /* data segment length */
  CSS_t_USINT u8_dataSegLenW = 0U;

  *pu16_procBytes = 0U;

  /* switch for the Segment Type/Format byte */
  switch (u8_segTypeFormat)
  {
    case CSOS_k_DATA_SEG_SIMPLE:
    {
      /* Data segment length is extracted */
      pb_dataSegLen = CSS_ADD_OFFSET(pb_ePath, CSOS_k_SIZEOF_BYTE);
      CSS_N2H_CPY8(&u8_dataSegLenW, pb_dataSegLen);
      /* update the number of processed bytes */
      *pu16_procBytes = (CSS_t_UINT)
                        (CSOS_k_SIZEOF_BYTE  /* segment type/Format */
                       + CSOS_k_SIZEOF_USINT /* segment length */
                       + ((CSS_t_UINT)(u8_dataSegLenW) * CSOS_k_SIZEOF_WORD));

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_DATA_SEG))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_DATA,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }

      /* store size of data segment */
      ps_ePathDataSeg->u8_cfgSizeW = u8_dataSegLenW;
      /* store the pointer to the data segment */
      ps_ePathDataSeg->pba_cfgData =
        CSS_ADD_OFFSET(pb_ePath, (CSOS_k_SIZEOF_BYTE + CSOS_k_SIZEOF_USINT));
      break;
    }

    default:
    {
      SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX6,
                        IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_segTypeFormat);
      u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return u8_genStatus;
}


/*******************************************************************************
**
** Function    : EPathParseSegNetwork
**
** Description : This function parses a Safety network segment inside an EPATH.
**
** See Also    : -
**
** Parameters  : u8_segTypeFormat (IN) - segment Type/Format byte
**                                       (not checked, any value allowed)
**               pb_ePath (IN)         - pointer to a logical segment in the
**                                       EPATH to be processed by this function
**                                       (not checked, checked in calling
**                                       functions)
**               pu16_procBytes (OUT)  - number of bytes that are processed by
**                                       the function in this segment of the
**                                       EPATH
**                                       (not checked, only called with
**                                       reference to variable)
**               pdw_valid (OUT)       - bit string to signal which EPATH fields
**                                       are found
**                                       (not checked, only called with
**                                       reference to variable)
**               ps_ePathNetSeg (OUT)  - pointer to the Network Segment
**                                       structure in the EPATH Fields structure
**                                       (not checked, only called with
**                                       reference to struct)
**
** Returnvalue : CSS_t_USINT           - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS - success
**               !=CSOS_k_CGSC_SUCCESS - protocol failure
**
*******************************************************************************/
static CSS_t_USINT EPathParseSegNetwork(CSS_t_USINT u8_segTypeFormat,
                                        const CSS_t_BYTE *pb_ePath,
                                        CSS_t_UINT *pu16_procBytes,
                                        CSS_t_DWORD *pdw_valid,
                                        IXEPP_t_FIELD_NET_SEG *ps_ePathNetSeg)
{
  /* return value */
  CSS_t_USINT u8_genStatus = CSOS_k_CGSC_INVALID_STATUS_CODE;
  /* pointer to the data segment length */
  const CSS_t_USINT *pb_segLen = CSS_k_NULL;
  /* segment length */
  CSS_t_USINT u8_segLen = 0U;

  *pu16_procBytes = 0U;

  /* switch for the Segment Type/Format byte */
  switch (u8_segTypeFormat)
  {
    case CSOS_k_NET_SEG_SAFETY:
    {
      /* Network segment length is extracted */
      pb_segLen = CSS_ADD_OFFSET(pb_ePath, CSOS_k_SIZEOF_USINT);
      CSS_N2H_CPY8(&u8_segLen, pb_segLen);
      /* update the number of processed bytes */
      *pu16_procBytes = (CSS_t_UINT)
                        (CSOS_k_SIZEOF_BYTE  /* segment type/Format */
                       + CSOS_k_SIZEOF_USINT /* segment length */
                       + ((CSS_t_UINT)(u8_segLen) * CSOS_k_SIZEOF_WORD));

      /* if EPATH field has not been found yet */
      if (EPathFieldSetValid(pdw_valid, IXEPP_k_SFB_SAFETY_NTWK_SEG))
      {
        u8_genStatus = CSOS_k_CGSC_SUCCESS;
      }
      else /* repeated field has been found */
      {
        SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_REP_SNTWK,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
      }

      /* store size of Network Segment Safety */
      ps_ePathNetSeg->u8_nsdSize = u8_segLen;
      /* store the pointer to Network Segment Safety */
      ps_ePathNetSeg->pba_nsd = pb_ePath;
      break;
    }

    default:
    {
      SAPL_CssErrorClbk(IXEPP_k_NFSE_RXE_EP_SYNTAX7,
                        IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_segTypeFormat);
      u8_genStatus = CSOS_k_CGSC_PATH_SEG_ERROR;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return u8_genStatus;
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

  return o_ret;
}


/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEmisc.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module contains some miscellaneous functions for the IXSCE
**             unit (initialization, CPCRC calculation, management of the
**             Connection IDs, Calculation of safety message payload length).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_CpcrcCalc
**             IXSCE_MsgLenToPayloadLen
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
#include "CSS.h"
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  #include "HALCSapi.h"
#endif

#include "IXCRC.h"
#include "IXSCF.h"

#include "IXSCE.h"
#include "IXSCEint.h"


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
** Function    : IXSCE_CpcrcCalc
**
** Description : This function calculates the CPCRC from the passed Safety
**               Open.
**
** Parameters  : pb_msgBuf (IN)          - pointer to the buffer which contains
**                                         the received Foward_Open message
**                                         (not checked, only called with
**                                         checked pointers or references to
**                                         variables)
**               o_isExtendedFormat (IN) - CSS_k_TRUE if it is an Extended
**                                         Format Connection
**                                         (not checked, any value allowed)
**               ps_sOpenAuxData (IN)    - pointer to structure with additional
**                                         information concerning the received
**                                         Forward_Open request
**                                         (not checked, only called with
**                                         reference to structure variables)
**
** Returnvalue : CSS_t_UDINT             - Calculated CPCRC value
**
*******************************************************************************/
CSS_t_UDINT IXSCE_CpcrcCalc(const CSS_t_BYTE *pb_msgBuf,
                            CSS_t_BOOL o_isExtendedFormat,
                            const IXSCE_t_SOPEN_AUX *ps_sOpenAuxData)
{
  /* CPCRC is a CRC-S4 (32-bit) */
  CSS_t_UDINT u32_cpcrc = k_CPCRC_SEED_VALUE;

  /* calculate crc from Cnxn Serial Number to Originator Vendor ID */
  u32_cpcrc = IXCRC_CrcS4compute(CSS_ADD_OFFSET(pb_msgBuf,
                                                k_OFS_FWDO_CNXN_S_NUM),
                                 CSOS_k_SIZEOF_UINT +
                                 CSOS_k_SIZEOF_UINT,
                                 u32_cpcrc);
  /* Continue calculation from Cnxn Timeout Multiplier to Cnxn Path Size */
  u32_cpcrc = IXCRC_CrcS4compute(CSS_ADD_OFFSET(pb_msgBuf,
                                                k_OFS_FWDO_CNXN_TMULT),
                                 CSOS_k_SIZEOF_USINT +
                                 (3U * CSOS_k_SIZEOF_USINT) +
                                 CSOS_k_SIZEOF_UDINT +
                                 CSOS_k_SIZEOF_WORD +
                                 CSOS_k_SIZEOF_UDINT +
                                 CSOS_k_SIZEOF_WORD +
                                 CSOS_k_SIZEOF_BYTE +
                                 CSOS_k_SIZEOF_USINT,
                                 u32_cpcrc);

  /* Continue calculation from Electronic Key to Application Path 3 */
  u32_cpcrc = IXCRC_CrcS4compute(ps_sOpenAuxData->pba_elKey,
                                 ps_sOpenAuxData->u16_elKeyAndAppPathSize,
                                 u32_cpcrc);

  /* Network Segment Safety is different for Base or Extended Format */
  /* if is base format */
  if (o_isExtendedFormat == CSS_k_FALSE)
  {
    /* Continue calculation from Safety Segment Name to Max_Consumer_Number */
    /* This also includes the SCCRC (see FRS158) and the SCTS (see FRS160). */
    u32_cpcrc = IXCRC_CrcS4compute(ps_sOpenAuxData->pba_nsd,
                                   IXSCE_k_CRC_SIZE_NSEG_SAFETY_BASE,
                                   u32_cpcrc);
  }
  else /* else: is extended format */
  {
    /* Continue calculation from Safety Segment Name to Max_Fault_Number */
    /* This also includes the SCCRC (see FRS158) and the SCTS (see FRS160). */
    u32_cpcrc = IXCRC_CrcS4compute(ps_sOpenAuxData->pba_nsd,
                                   IXSCE_k_CRC_SIZE_NSEG_SAFETY_EXT,
                                   u32_cpcrc);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* return the calculated value */
  return (u32_cpcrc);
}


/*******************************************************************************
**
** Function    : IXSCE_MsgLenToPayloadLen
**
** Description : This function calculates the length of the payload data of a
**               Safety Connection from the absolute length transported over the
**               connection - considering if this connection is single- or
**               multicast.
**
** Parameters  : u16_msgLen (IN)    - number of bytes transported by the
**                                    connection
**                                    (valid range: CSOS_k_IO_MSGLEN_SHORT_MIN..
**                                    (CSOS_k_IO_MSGLEN_LONG_MAX +
**                                    CSOS_k_IO_MSGLEN_TCORR), checked)
**               o_isMulticast (IN) - CSS_k_TRUE if this checks a multicast
**                                    connection, CSS_k_FALSE in case of
**                                    singlecast
**
** Returnvalue : 0   - error (this length is not valid for a safety connection)
**               >0  - length of the payload data
**
*******************************************************************************/
CSS_t_USINT IXSCE_MsgLenToPayloadLen(CSS_t_UINT u16_msgLen,
                                     CSS_t_BOOL o_isMulticast)
{
  CSS_t_USINT u8_retVal = 0U;
  CSS_t_UINT  u16_dataMsgLen = 0U;

  /* if it is a multicast connection */
  if (o_isMulticast)
  {
    /* in case this direction is multicast there must be a Time Correction */
    /* if length is shorter than message length of a Time Correction Message */
    if (u16_msgLen < CSOS_k_IO_MSGLEN_TCORR)
    {
      /* much too short */
      /* return value already set */
    }
    else /* else: length is ok (for now, further checks will follow) */
    {
      /* subtract length of Time Correction message, then we can apply */
      /* further tests just like Single-cast */
      u16_dataMsgLen = (CSS_t_UINT)(u16_msgLen - CSOS_k_IO_MSGLEN_TCORR);
    }
  }
  else /* else: it is a singlecast connection */
  {
    /* Single-cast */
    u16_dataMsgLen = u16_msgLen;
  }

  /* if shorter than min short message */
  if (u16_dataMsgLen < CSOS_k_IO_MSGLEN_SHORT_MIN)
  {
    /* return value already set */
  }
  /* else if length is in range of short message */
  else if (    (u16_dataMsgLen >= CSOS_k_IO_MSGLEN_SHORT_MIN)
            && (u16_dataMsgLen <= CSOS_k_IO_MSGLEN_SHORT_MAX)
          )
  {
  #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
    {
      /* valid length for short message (will definitely fit into USINT var) */
      u8_retVal = (CSS_t_USINT)(u16_dataMsgLen - CSOS_k_IO_MSGLEN_SHORT_OVHD);
    }
  #else
    {
      /* not supported */
      /* return value already set */
    }
  #endif
  }
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  /* else if longer than max short msg but shorter than min long msg */
  else if (    (u16_dataMsgLen > CSOS_k_IO_MSGLEN_SHORT_MAX)
            && (u16_dataMsgLen < CSOS_k_IO_MSGLEN_LONG_MIN)
          )
  {
    /* return value already set */
  }
  /* else if length is in range of long message */
  else if (    (u16_dataMsgLen >= CSOS_k_IO_MSGLEN_LONG_MIN)
            && (u16_dataMsgLen <= CSOS_k_IO_MSGLEN_LONG_MAX)
          )
  {
    /* length must be even (because of 8 bytes overhead and 2 * payload) */
    if (u16_dataMsgLen & 0x00001U)
    {
      /* length is invalid */
      /* return value already set */
    }
    else
    {
      /* length is valid. For calculation of payload size we must consider  */
      /* that data is contained twice (will definitely fit into USINT var). */
      u8_retVal = (CSS_t_USINT)((u16_dataMsgLen - CSOS_k_IO_MSGLEN_LONG_OVHD)
                                / 2U);
    }
  }
  else /* longer than max long message */
  {
    /* return value already set */
  }
#else
  else /* longer than max short message */
  {
    /* return value already set */
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_retVal);
}


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


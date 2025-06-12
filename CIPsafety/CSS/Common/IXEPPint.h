/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPPint.h
**    Summary: IXEPP - Encoded Path Parser
**             This header file defines the unit internal interface of the IXEPP
**             unit.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_EPathParseSegLogical
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


#ifndef IXEPP_INT_H
#define IXEPP_INT_H


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
                                       IXEPP_t_FIELD_LOG_SEG *ps_ePathLogSeg);


#endif /* #ifndef IXEPP_INT_H */

/*** End of File ***/


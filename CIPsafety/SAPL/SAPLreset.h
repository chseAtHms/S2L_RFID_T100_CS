/***************************************************************************************************
**    Copyright (C) 2016-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLreset.h
**     Summary: Interface header of module SAPLreset.c
**   $Revision: 3418 $
**       $Date: 2020-04-07 14:31:08 +0200 (Di, 07 Apr 2020) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_ResetTypeGet
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_RESET_H
#define SAPL_RESET_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/*!< Defines the valid Safety Reset Types */
typedef enum
{
  SAPL_k_SAFETY_RESET_TYPE0  = 0xe472u,
  SAPL_k_SAFETY_RESET_TYPE1  = 0xe5e5u,
  SAPL_k_SAFETY_RESET_TYPE2  = 0xe6cbu,
  SAPL_k_SAFETY_RESET_NONE   = 0xe75cu
} SAPL_t_SAFETY_RESET_TYPE;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_ResetTypeGet

  Description:
    This function returns the current pending Safety Reset Type. Additionally the received
    Attribute Bit Map is delivered by the function.
    Note: The Attribute Bit Map is only relevant for Safety Reset Type 2.

  See also:
    -

  Parameters:
    pu8_attrBitMap (OUT) - Address to store the received 'Attribute Bit Map'
                           (valid: <>NULL, not checked, only called with reference to variable)

  Return value:
    SAPL_t_SAFETY_RESET_TYPE

  Remarks:
    Context: Background Task

***************************************************************************************************/
SAPL_t_SAFETY_RESET_TYPE SAPL_ResetTypeGet(UINT8* pu8_attrBitMap);

#endif


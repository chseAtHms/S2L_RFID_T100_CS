/***************************************************************************************************
**    Copyright (C) 2016 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: globPreChk.h
**     Summary: Pre-processor check macro
**   $Revision: 2381 $
**       $Date: 2017-03-17 10:34:30 +0100 (Fr, 17 Mrz 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: -
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef GLOBPRECHK_H
#define GLOBPRECHK_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* The following macro is used to perform preprocessor checks and can even handle the sizeof 
** operator.
** For each check an array is defined with length of 1, if the check is OK. Otherwise the array
** is defined with length -1. This leads directly into a compiler error and indicates, that the 
** check failed. 
*/

/*lint -save -e960 -e912 -e506 -e961 -e9024 */
/* suppress following messages:
   error 912:  Note -- Implicit binary conversion from int to unsigned int
   error 960:  Note -- Violates MISRA 2004 Required Rule 10.1, Prohibited Implicit
               Conversion: Signed versus Unsigned
   error 961:  Note -- Violates MISRA 2004 Advisory Rule 19.13, '#/##' operator used
   error 9024: Note -- Violates MISRA 2012 Advisory Rule 20.10, '#/##' operator used
   */

#ifndef IXX_TASSERT
  #define IXX_TASSERT(exp, name) typedef int IXX_DUMMY_##name [(exp) ? 1 : -1];
#endif

/*lint -restore */

/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


#endif


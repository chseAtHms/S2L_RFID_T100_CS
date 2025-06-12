/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globDef_GlobalDefinitions.h
**
** $Id: globDef_GlobalDefinitions.h 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** This header file contains various constants.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef GLOBDEF_GLOBALDEFINITIONS_H
#define GLOBDEF_GLOBALDEFINITIONS_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! If NULL is not defined, define it for the SFR-Test. */
#ifndef NULL
/* This constants define the NULL parameter and pointer */
/*lint -e(960)*//* Definition of NULL is only if it is not defined. */
#define NULL 0u
#endif

/*! Definition for NULL-Pointer. */
#define NULL_PNT ((void*)0u)

/* These constants defines some masks for the UINT16 handling. */
/*!< Mask for handling the high byte of a UINT16 value.*/
#define GLOBDEF_MASK_UINT16_HIGH_BYTE ((UINT16) 0xFF00u)
/*!< Mask for handling the low byte of a UINT16 value.*/
#define GLOBDEF_MASK_UINT16_LOW_BYTE  ((UINT16) 0x00FFu)

/* These constants defines some masks for the UINT32 handling. */
/*!< Mask for handling the 0. byte of a UINT32 value.*/
#define GLOBDEF_MASK_UINT32_BYTE_0  0x000000FFuL  
/*!< Mask for handling the 1. byte of a UINT32 value.*/ 
#define GLOBDEF_MASK_UINT32_BYTE_1  0x0000FF00uL   
/*!< Mask for handling the 2. byte of a UINT32 value.*/
#define GLOBDEF_MASK_UINT32_BYTE_2  0x00FF0000uL   
/*!< Mask for handling the 3. byte of a UINT32 value.*/
#define GLOBDEF_MASK_UINT32_BYTE_3  0xFF000000uL   

/* These constants defines some masks for the UINT32 handling. */
/*!< Mask for handling the 0. word of a UINT32 value.*/
#define GLOBDEF_MASK_UINT32_WORD_0  0x0000FFFFuL   
/*!< Mask for handling the 0. word of a UINT32 value.*/
#define GLOBDEF_MASK_UINT32_WORD_1  0xFFFF0000uL   

/*! This constant defines the system clock. */
#define GLOBDEF_SYSTEM_CLOCK        72000000UL


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/



#endif /* GLOBDEF_GLOBALDEFINITIONS_H */

/*******************************************************************************
**
** End of globDef_GlobalDefinitions.h
**
********************************************************************************
*/

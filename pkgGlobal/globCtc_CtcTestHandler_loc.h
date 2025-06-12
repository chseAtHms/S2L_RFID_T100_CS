/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globCtc_CtcTestHandler_loc.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globCtc_CtcTestHandler_loc.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Local header of the CTC-Test Handler module.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef GLOBCTC_CTCTESTHANDLER_LOC_H
#define GLOBCTC_CTCTESTHANDLER_LOC_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*! Constants to define the CRC_array page start address. */
#define GLOBCTC_PAGE_STRT_ADDR  0x0803F800uL

/*! Constants to define the FLASH keys. */
#define GLOBCTC_FLASH_KEY1    0x45670123uL /*!< Value of the FLASH key 1. */
#define GLOBCTC_FLASH_KEY2    0xCDEF89ABuL /*!< Value of the FLASH key 2. */


#ifndef GLOBCTC_ARRAY_SIZE
   #error "The constant GLOBCTC_ARRAY_SIZE must be defined in the project options!!!"
#endif
/*! Check of the constant, the value must be even because the values of the
 *  CTC_array are written 16Bit-wise to the flash. */
#if ((GLOBCTC_ARRAY_SIZE % 2) != 0)
   #error "The constant GLOBCTC_ARRAY_SIZE must be a even value!!!"
#endif

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* GLOBCTC_CTCTESTHANDLER_LOC_H */

/**** Last line of source code                                             ****/

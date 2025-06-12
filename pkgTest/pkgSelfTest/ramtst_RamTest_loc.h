/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: ramtst_RamTest_loc.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          ramtst_RamTest_loc.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Local header file of the RAM-Test.
 * @description:
 * Contains the local constants and type definitions of the appendent c-file,
 * see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
 */
/************************************************************************//*@}*/

#ifndef RAMTST_RAMTEST_LOC_H
#define RAMTST_RAMTEST_LOC_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/


/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/

/*! This enum contains the RAM-Test Steps. */
typedef enum
{
   RAMTST_E_STEP1    = 0x19f7u,   /*!< RAM-Test Step 1 */
   RAMTST_E_STEP2    = 0x1ad9u,   /*!< RAM-Test Step 2 */
   RAMTST_E_NOINIT   = 0x1b4eu    /*!< RAM-Test not initialized */
} RAMTST_TESTSTEPS_ENUM;

/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* RAMTST_RAMTEST_LOC_H */

/**** Last line of source code                                             ****/

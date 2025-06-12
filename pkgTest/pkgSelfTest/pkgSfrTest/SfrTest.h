/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SfrTest.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SfrTest.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Contains the service interface for the special function
 *                register test.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef SFR_TEST_H
#define SFR_TEST_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/


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


/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

#if SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE
void sfrTest_InitReference(CONST SAFEINDEX si);
void sfrTest_ConfirmReference(CONST SAFEINDEX si);
#endif /* SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE */
void sfrTest_Reset(CONST SAFEINDEX si);
SFRTEST_STATE_ENUM sfrTest_Execute(CONST SAFEINDEX si);


/***** End of: Function-Prototypes ********************************************/


#endif /* SFR_TEST_H */

/**** Last line of source code                                             ****/

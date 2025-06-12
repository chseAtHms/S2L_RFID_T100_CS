/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globCtc_CtcTestHandler.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globCtc_CtcTestHandler.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Header file of the CTC-Test Handler module.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef GLOBCTC_CTCTESTHANDLER_H
#define GLOBCTC_CTCTESTHANDLER_H


/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#ifdef __CTC_TESTING_ON__


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
extern void globCtc_Init (void);
extern void globCtc_CtcTestHandler (void);


/***** End of: Function-Prototypes ********************************************/

#endif /* __CTC_TESTING_ON__ */
#endif /* GLOBCTC_CTCTESTHANDLER_H */

/**** Last line of source code                                             ****/

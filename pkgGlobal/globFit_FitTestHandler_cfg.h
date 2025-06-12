/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globFit_FitTestHandler_cfg.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globFit_FitTestHandler_cfg.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Configuration header of the FIT-Test Handler module.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef GLOBFIT_FITTESTHANDLER_CFG_H
#define GLOBFIT_FITTESTHANDLER_CFG_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/

//#ifndef GLOBFIT_FITTEST_ACTIVE
//  #define GLOBFIT_FITTEST_ACTIVE
//#endif

/* This constant defines the RAM address of the FIT-Test command variable.
 * For further informations go to the definition of the FIT Test variable. */
#define GLOBFIT_LOCATION_OF_FITCMD_VAR_RAM   0x2000BFFC

/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/


/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/





#endif /* GLOBFIT_FITTESTHANDLER_CFG_H */

/**** Last line of source code                                             ****/

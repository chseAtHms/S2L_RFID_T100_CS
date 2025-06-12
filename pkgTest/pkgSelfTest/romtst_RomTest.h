/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: romtst_RomTest.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          romtst_RomTest.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        ROM-Test header
 * @description:
 * This module provides the ROM-Test at startup and for the runtime.
 * The ROM-Test for the runtime is executed consecutively by calling the
 * DoRomTest-function. The Startup-ROM-Test is called by DoStartupRomTest.
 */
/************************************************************************//*@}*/

#ifndef ROMTST_ROMTEST_H
#define ROMTST_ROMTEST_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#include "romtst_RomTest_cfg.h"

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
extern void romtst_InitRomTest (void);
extern STDEF_RETCODE_ENUM romtst_DoRomTest (void);
extern void romtst_DoStartupRomTest (void);

/***** End of: Function-Prototypes ********************************************/


#endif /* ROMTST_ROMTEST_H */

/**** Last line of source code                                             ****/

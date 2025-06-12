/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: cputst_CpuTest.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          cputst_CpuTest.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief         This modul contains the CPU-Test functions.
 * @description:
* This module provides the following CPU-Tests for the runtime:
* - OpCode Test
* - Special Function Register Test
* - Core-Register Test
* The tests are executed consecutively by calling the DoCpuTests-function.
*
* Furthermore a Startup-CPU-Test is provided.est
 */
/************************************************************************//*@}*/

#ifndef CPUTST_CPUTEST_H
#define CPUTST_CPUTEST_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#include "cputst_CpuTest_cfg.h"

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
extern void cputst_InitCpuTests (void);
extern STDEF_RETCODE_ENUM cputst_DoCpuTests (void);
extern void cputst_StartupCpuTests (void);
extern void asm_OpcodeTest (void);
extern void asm_CoreRegTestStep1 (void);
extern void asm_CoreRegTestStep2 (void);


/***** End of: Function-Prototypes ********************************************/


#endif /* CPUTST_CPUTEST_H */

/**** Last line of source code                                             ****/

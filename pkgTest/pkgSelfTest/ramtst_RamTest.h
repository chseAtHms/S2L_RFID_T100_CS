/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: ramtst_RamTest.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          ramtst_RamTest.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        This module contains the RAM-Test.
 * @description:
 * This module provides the following RAM-Tests:
 * - MarchC
 *   The MarchC-Test is used on Startup to check the RAM. The MarchC-Test
 *   destroys the data in RAM.
 *
 * - Stack-Test
 *   The Stack-Test is executed during runtime from the Test-Manager.
 *
 * - WALKPATH
 *   The WALKPATH is the RAM-Test during the runtime. It is executed
 *   consecutively from the Test-Manager. It is written in assembler.
 */
/************************************************************************//*@}*/
#ifndef RAMTST_RAMTEST_H
#define RAMTST_RAMTEST_H


/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#include "ramtst_RamTest_cfg.h"

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
/*
   The function ramtst_DoStartupRamTest is called from the startup file and is
   defined in ramtst_RamTest.c.
   This function is not referenced from a c-file but from an assembler file.
*/
/*lint -esym(765, ramtst_DoStartupRamTest)*/
/*lint -esym(759, ramtst_DoStartupRamTest)*/
/*lint -esym(714, ramtst_DoStartupRamTest)*/
extern void ramtst_DoStartupRamTest (UINT32 u32StartAddr, UINT32 u32EndAddr);


extern void ramtst_InitRamTest (void);
extern STDEF_RETCODE_ENUM ramtst_DoRamTest (void);
extern void ramtst_DoStackTest (void);

/*! This function is defined in the asm_Ramtest. */
extern STDEF_RETCODE_ENUM asm_RamTestStep1 ( UINT32 mu32ActBCAddr,
                                             UINT32 mu32OCBlkSize);
extern STDEF_RETCODE_ENUM asm_RamTestStep2 ( UINT32 mu32ActBCAddr,
                                             UINT32 mu32OCStartAddr,
                                             UINT32 mu32OCBlkSize);

/***** End of: Function-Prototypes ********************************************/


#endif /* RAMTST_RAMTEST_H */

/**** Last line of source code                                             ****/



/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: romtst_RomTest_cfg.h 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          romtst_RomTest_cfg.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Config header file of the ROM-Test.
 * @description:
 * This is the configuration header. It contains all configurations which
 * are used in the module, e.g. constant definitions for array sizes.
 * For further information see VA_C_Programmierstandard rule STYL-2-3-20 and
 * LIB-0-0-10.
 */
/************************************************************************//*@}*/

#ifndef ROMTST_ROMTEST_CFG_H
#define ROMTST_ROMTEST_CFG_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/

/*! This compiler switch deactivates the ROM-Test. The ROM-Test returns the
 * return value STDEF_RET_DONE after an execution .
 * During the compiling procedure a compiler warning is generated which informs
 * that the ROM-Test is disabled.
 */
#if defined(__DEBUG__) || defined(__CTC_TESTING_ON__)
#define ROMTST_DISABLE  TRUE
#else
#define ROMTST_DISABLE  FALSE
#endif

/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*! This constant defines the ROM start address for the assembler modules. */
#define ROMTST_ROM_START_ASM        0x08004000
/*! This constant defines the last ROM address to be tested for the assembler modules. */
#define ROMTST_ROM_END_ASM          0x0803bffb
/*! ROM address of the ROM-CRC (see [SRS_50]), for the assembler modules. */
#define ROMTST_ROMCRC_ADDRESS_ASM   0x0803bffc


#ifndef __ASM__

#ifdef __UNIT_TESTING_ON__
/* Constants for the ROM-Test */
UINT8 * ROMTST_ROM_START_testvar;
 /*!< This constant defines the ROM start address. */
#define ROMTST_ROM_START         ROMTST_ROM_START_testvar  

UINT8 *  ROMTST_ROM_END_testvar;
/*!< This constant defines the last ROM address to be tested. */
#define ROMTST_ROM_END           ROMTST_ROM_END_testvar   

#else

/* Constants for the ROM-Test */
/*!< This constant defines the ROM start address. */
#define ROMTST_ROM_START         (UINT32)ROMTST_ROM_START_ASM   
/*!< This constant defines the last ROM address to be tested. */
#define ROMTST_ROM_END           (UINT32)ROMTST_ROM_END_ASM     

#endif /* __UNIT_TESTING_ON__ */

/*!< Test size of the ROM. */
#define ROMTST_SIZE              200uL         
/*!< Size of the whole ROM. */
#define ROMTST_ROM_MAX_SIZE      0x40000uL     
/*!< ROM address of the ROM-CRC (see [SRS_50]). */
#define ROMTST_ROMCRC_ADDRESS    (UINT32)ROMTST_ROMCRC_ADDRESS_ASM   
/*! Test cycles of the ROM-Test. */
#define ROMTST_CYCLES            ((UINT16)(((UINT32)(ROMTST_ROM_END - ROMTST_ROM_START) + \
                                   ROMTST_SIZE) / ROMTST_SIZE))


#ifndef __UNIT_TESTING_ON__
/* Plausibility-check of the ROM-Test specific settings. */
#if (ROMTST_ROM_END_ASM <= ROMTST_ROM_START_ASM)
   #error "(ROMTST_ROM_END <= ROMTST_ROM_START)"
#endif
#if ((ROMTST_SIZE == 0U) || (ROMTST_SIZE >= ROMTST_ROM_MAX_SIZE))
   #error "((ROMTST_SIZE == 0U) || (ROMTST_SIZE >= ROMTST_ROM_MAX_SIZE))"
#endif
#endif /* __UNIT_TESTING_ON__ */

#endif /* __ASM__ */

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/



#endif /* ROMTST_ROMTEST_CFG_H */

/**** Last line of source code                                             ****/

/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: ramtst_RamTest_cfg.h 4393 2024-04-22 13:59:26Z ankr $
 * $Revision: 4393 $
 *     $Date: 2024-04-22 15:59:26 +0200 (Mo, 22 Apr 2024) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          ramtst_RamTest_cfg.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Config header file of the RAM-Test.
 * @description:
 * This is the configuration header of the RAM-Test. It contains all
 * configurations which are used in the RAM-Test.
 */
/************************************************************************//*@}*/

#ifndef RAMTST_RAMTEST_CFG_H
#define RAMTST_RAMTEST_CFG_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/


/* These constants defines the RAM and the area to be tested. ASM specific
 * declaration.
 * Deactivate misra rule 10.6 because the u-suffix behind the values causes
 * errors in the assembler compiler. */
/*lint -esym(960, 10.6)*/
/*! This constant defines the start address of the RAM (used for [SRS_47]). */
#define RAMTST_RAM_START_ASM        0x20000000
/*! This constant defines the RAM end address (used for [SRS_47]). */
#define RAMTST_RAM_END_ASM          (RAMTST_RAM_START_ASM + RAMTST_SIZE)
/*! This constant defines the max size of the RAM, this controller has 48kB.
 * (used for [SRS_47]) */
#define RAMTST_RAM_MAX_SIZE_ASM     0xC000
/*! This constant defines the size of the RAM to be tested. */
#define RAMTST_SIZE                 0x1D00

/* These constants splits the 32 bit RAM-End address into 16 bit high and low.
 * This is used for the ASM-RAM-Test because a 32bit value can not handled. */
#define RAMTST_RAM_END_ASM_LO    (RAMTST_RAM_END_ASM & 0x0000FFFF)
#define RAMTST_RAM_END_ASM_HI    ((RAMTST_RAM_END_ASM & 0xFFFF0000) >> 16)

/* Assembler specific constants. */
/*! Size of the over- / underflow area to be tested.*/
#define RAMTST_STACK_MONITORING_SIZE_ASM  0x10
/*! Heap size */
#define RAMTST_HEAP_SIZE_ASM              0
/*! Stack size */
#define RAMTST_STACK_SIZE_ASM             0x800
/*! *End of stack */
#define RAMTST_STACK_END_ASM              (RAMTST_RAM_START_ASM + \
                                           RAMTST_STACK_SIZE_ASM + \
                                           (2 * RAMTST_STACK_MONITORING_SIZE_ASM) - 1)
/*! Test pattern for the over- / underflow area (see [SRS_448]) */
#define RAMTEST_STACK_TEST_PATTERN_ASM    0x5A5A5A5A
/*! The stack underflow area is at the RAM start address, see also scatter-file PF-SIL-FW.sct */
#define RAMTST_STACK_UNDFLW_START_ASM     RAMTST_RAM_START_ASM
/*! The stack overflow area starts after the underflow and stack size. */
#define RAMTST_STACK_OVFLW_START_ASM      (RAMTST_STACK_UNDFLW_START_ASM +  \
                                           RAMTST_STACK_MONITORING_SIZE_ASM + \
                                           RAMTST_STACK_SIZE_ASM)
/*! Start address of the Stack, after the underflow area.*/
#define RAMTST_STACK_START_ASM            (RAMTST_STACK_UNDFLW_START_ASM + \
                                           RAMTST_STACK_MONITORING_SIZE_ASM)
/*! Start address for the data in the RAM. */
#define RAMTST_RAM_DATA_START_ASM         (RAMTST_STACK_END_ASM + 1)
/*lint +esym(960, 10.6)*/


#ifndef __ASM__

/* C specific declaration of the RAM sizes. */

/*! This constant defines the start address of the RAM (used for [SRS_47]). */
#define RAMTST_RAM_START            ((UINT32)RAMTST_RAM_START_ASM)
/*! This constant defines the RAM end address (used for [SRS_47]). */
#define RAMTST_RAM_END              ((UINT32)RAMTST_RAM_END_ASM)
/*! This constant defines the maximum RAM size */
#define RAMTST_RAM_MAX_SIZE         ((UINT32)RAMTST_RAM_MAX_SIZE_ASM)
/*! This constant defines the memory alignment */
#define RAMTST_MEMORY_ALIGNMENT     1u


/* These constants defines the RAM Test. */

/*! Size of the cyclic tested area at the test step 1. */
#define RAMTST_STEP1_TEST_WORDS     8u
/*! Size of the OC area at the test step 2. */
#define RAMTST_STEP2_OC_TEST_BYTES  128u
/*! The test cycles for the test step 1. */
#define RAMTST_CYCLES_STEP1         (RAMTST_SIZE / \
                                    (RAMTST_STEP1_TEST_WORDS * (UINT32)sizeof(UINT32)))
/*! The test cycles for the test step 2. */
#define RAMTST_CYCLES_STEP2         ((RAMTST_SIZE / RAMTST_STEP2_OC_TEST_BYTES) * RAMTST_SIZE)


/* These constants defines the STACK and the associated STACK Test.
 * IMPORTANT: If changes are necessary, the associated scatter file
 * "PF-SIL-FW.sct" have to be changed as well.  */

/*! The stack underflow area is at the RAM start address, see also scatter-file PF-SIL-FW.sct */
#define RAMTST_STACK_UNDFLW_START      RAMTST_RAM_START
/*! The stack overflow area starts after the underflow and stack size. */
#define RAMTST_STACK_OVFLW_START       (RAMTST_STACK_UNDFLW_START + \
                                        RAMTST_STACK_MONITORING_SIZE + \
                                        RAMTST_STACK_SIZE)
/*! Start address of the Stack, after the underflow area.*/
#define RAMTST_STACK_START             (RAMTST_STACK_UNDFLW_START + RAMTST_STACK_MONITORING_SIZE)
/*! End address of the Stack, after the overflow area.*/
#define RAMTST_STACK_END               ((UINT32) RAMTST_STACK_END_ASM)
/*! Test pattern for the over- / underflow area (see [SRS_448]) */
#define RAMTEST_STACK_TEST_PATTERN     ((UINT8)((UINT32)RAMTEST_STACK_TEST_PATTERN_ASM & \
                                                (UINT32)0x000000FFuL))

/* C specific constants. */
#define RAMTST_STACK_MONITORING_SIZE      ((UINT8)RAMTST_STACK_MONITORING_SIZE_ASM)
#define RAMTST_HEAP_SIZE                  ((UINT8)RAMTST_HEAP_SIZE_ASM)
#define RAMTST_STACK_SIZE                 ((UINT16)RAMTST_STACK_SIZE_ASM)



/* These preprocessor directives do a plausibility check of the specified RAM
 * settings. If they are not correct, the compiler generates an error.  */
/*lint -save -e960*/
/* TSL: Avoid lint message because a type-cast (UINT32) is not accepted of the
 * compiler in preprocessor directives. */
#if (RAMTST_RAM_END_ASM <= RAMTST_RAM_START_ASM)
   #error "RAMTST_RAM_END <= RAMTST_RAM_START"
#endif

#if (((RAMTST_RAM_START_ASM % RAMTST_MEMORY_ALIGNMENT) != 0u) || \
     ((RAMTST_RAM_END_ASM % RAMTST_MEMORY_ALIGNMENT) != 0u))
  #error "RAMTST_RAM_START & RAMTST_RAM_END are not in memory alignment"
#endif

#if ((RAMTST_STEP2_OC_TEST_BYTES == 0u) || (RAMTST_STEP2_OC_TEST_BYTES >= RAMTST_RAM_MAX_SIZE_ASM))
  #error "(RAMTST_STEP2_OC_TEST_BYTES == 0U) || (RAMTST_STEP2_OC_TEST_BYTES >= RAMTST_RAM_MAX_SIZE)"
#endif

#if ((RAMTST_STEP1_TEST_WORDS == 0u) || (RAMTST_STEP1_TEST_WORDS >= RAMTST_RAM_MAX_SIZE_ASM))
  #error "(RAMTST_STEP1_TEST_WORDS == 0U) || (RAMTST_STEP1_TEST_WORDS >= RAMTST_RAM_MAX_SIZE)"
#endif

#if (((RAMTST_RAM_END_ASM - RAMTST_RAM_START_ASM) % RAMTST_STEP2_OC_TEST_BYTES) != 0u)
  #error "(((RAMTST_RAM_END - RAMTST_RAM_START) % RAMTST_STEP2_OC_TEST_BYTES) != 0)"
#endif

#if (((RAMTST_RAM_END_ASM - RAMTST_RAM_START_ASM) % RAMTST_STEP1_TEST_WORDS) != 0u)
  #error "(((RAMTST_RAM_END_ASM - RAMTST_RAM_START_ASM) % RAMTST_STEP1_TEST_WORDS) != 0)"
#endif


#endif /* __ASM__ */
/*lint -restore*/

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/



#endif /* RAMTST_RAMTEST_CFG_H */

/**** Last line of source code                                             ****/

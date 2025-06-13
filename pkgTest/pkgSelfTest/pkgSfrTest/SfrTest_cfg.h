/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SfrTest_cfg.h 4109 2023-11-21 16:30:41Z ankr $
 * $Revision: 4109 $
 *     $Date: 2023-11-21 17:30:41 +0100 (Di, 21 Nov 2023) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SfrTest_cfg.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Config file for the sfr test.
 * @description:  This module contains the configuration of the SFR-Test which
 *                contains the SFRs to test and the specified masks to get the
 *                correct value for calculating the checksum.
 *                Furthermore the specified checksum (or checksums) for
 *                calculating the checksum over the SFRs are also defined in
 *                this module.
 *                The SFRs and the specified masks are defined in the document
 *                SFR-Map_HMS_ASM.xls.
 */
/************************************************************************//*@}*/

#ifndef SFRTEST_CFG_H
#define SFRTEST_CFG_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/


/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*! This compiler switch enables or disables the functionality of the parameter
 *  CRC calculation. If it is used in the project it can be enabled by setting
 *  the compiler switch to TRUE. */
#define SFRTEST_ENABLE_PARAM_CRC_CALC              FALSE

/*! This compiler switch sets the SFR-CRC-Check to always TRUE.
 *  Only for debug purposes!!! */
#if defined(__DEBUG__) || defined(__CTC_TESTING_ON__)
   #define SFRTEST_SET_SFRCRCCHECK_ALWAYS_TRUE     TRUE
#else
   #define SFRTEST_SET_SFRCRCCHECK_ALWAYS_TRUE     TRUE
#endif

/*! This compiler switch enables a buffer where the crc is logged block by
 *  block.
 *  Only for debug purposes!!!  */
#define SFRTEST_ENABLE_BLOCK_CRC_LOG_FOR_DEBUG     FALSE

/*! This constant defines the maximal size of the config block array. */
#define SFRTEST_MAX_SIZEOF_CFG_BLOCKS     22u

/*! These constants defines the number of the cfg. */
#define SFRTEST_CFG_0   ((UINT8)0x00u)    /*!< Cfg number 0. */
#define SFRTEST_CFG_1   ((UINT8)0x01u)    /*!< Cfg number 1. */


/*! These constants defines the SAFEINDEX for the SFR-Config-Array. */
#define SFRTEST_SAFEINDEX_0   (((UINT32)((UINT16)~(UINT16)SFRTEST_CFG_0) << 16u)  + \
                                 (SFRTEST_CFG_0))   /*!< SAFEINDEX 0 */
#define SFRTEST_SAFEINDEX_1   (((UINT32)((UINT16)~(UINT16)SFRTEST_CFG_1) << 16u)  + \
                                 (SFRTEST_CFG_1))   /*!< SAFEINDEX 1 */

/*! Number of all cfg's. */
#define SFRTEST_CFG_DIM (SFRTEST_CFG_1 + 1u)

/*! This constant defines the number of registers which are tested during a
 * cycle. */
#define SFRTEST_NO_REGS_PER_CYCL ((UINT16)15u)

/*! The start CRC that is used if an error occurs in the SFR test. */
#define STARTCRC 0xFFFFFFFFu

/*!
   Function-like Macro for definition of a dynamic data object for sfr-test
   Parameter:
      i   index of structure in the array of dynamic data instances
*/
#define SFRTEST_INITDATA(i) {                                          \
                               {{0u}, {~(UINT32)0u}},                  \
                               {{0u}, {~(UINT32)0u}},                  \
                               {{0u}, {~(UINT32)0u}},                  \
                               STARTCRC,                               \
                               SFRTEST_STATE_NOTINITIALIZED,           \
                               SFRTEST_ERRSTATE_NONE,                  \
                               &sfrTest_asData[i]                      \
                            }

/*!
   Macro containing the initialization values for all dynamic data instances;
   Shall be assigned to sfrTest_asData[]
*/
#define SFRTEST_DATA_INIT {                        \
                             SFRTEST_INITDATA(0),  \
                             SFRTEST_INITDATA(1)   \
                          }

/*!
   Macros containing the initialization values for the single instances of
   the test configuration.
   Shall be assigned to sfrTest_Cfg
   Note: sfrTst_asData is a local object in sfrtest.c, so the following
         macros can be expanded only sfrtest.c. This is intended, since
         the test configuration is local in sfrtest.c, too.
*/
#define SFRTEST_CFG0_INIT {\
                             &sfrTstCfg_asMap[SFRTEST_CFG_0],  /*!< pSfrMap       */    \
                             &sfrTest_asData[SFRTEST_CFG_0],   /*!< pTestData     */    \
                             SFRTEST_NO_REGS_PER_CYCL,         /*!< uRegsPerCycle */    \
                             &sfrTest_asCfg[SFRTEST_CFG_0]     /*!< pThis         */    \
                          }

#define SFRTEST_CFG1_INIT {\
                             &sfrTstCfg_asMap[SFRTEST_CFG_1],  /*!< pSfrMap       */    \
                             &sfrTest_asData[SFRTEST_CFG_1],   /*!< pTestData     */    \
                             SFRTEST_NO_REGS_PER_CYCL,         /*!< uRegsPerCycle */    \
                             &sfrTest_asCfg[SFRTEST_CFG_1]     /*!< pThis         */    \
                          }


/*!
   Macro containing the initialization sets for all instances of the test
   configuration
*/
#define SFRTEST_CFG_INIT  {                                                \
                                SFRTEST_CFG0_INIT,                         \
                                SFRTEST_CFG1_INIT                          \
                          }

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/

#define SFRCRC_AddU8(crc,u8val) crc_AddU8(crc,u8val)
#define SFRCRC_AddU16(crc,u16val) crc_AddU16(crc,u16val)
#define SFRCRC_AddU32(crc,u32val) crc_AddU32(crc,u32val)


/*
   Info 835
   Macro checks the safe index, the first index is zero.
*/
/*lint -emacro(835, SFRTEST_SAFEINDEX_SAFETY_ASSERT)*/
#define SFRTEST_SAFEINDEX_SAFETY_ASSERT(x)              \
   GLOBFAIL_SAFETY_ASSERT((SFRTEST_SAFEINDEX_0 == x) || \
                          (SFRTEST_SAFEINDEX_1 == x),   \
                          GLOB_FAILCODE_VARIABLE_ERR)

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
extern CONST SFRTEST_REGISTERMAP_STRUCT sfrTstCfg_asMap[SFRTEST_CFG_DIM];


/***** End of: Function-Prototypes ********************************************/


#endif /* SFRTEST_CFG_H */

/**** Last line of source code                                             ****/

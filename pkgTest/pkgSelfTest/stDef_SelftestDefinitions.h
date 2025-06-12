/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: stDef_SelftestDefinitions.h 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          stDef_SelftestDefinitions.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Header file for some definitions of the selftest package.
 * @description:  This header defines some constants, macros and return codes
 *                which are used for the selftest package.
 */
/************************************************************************//*@}*/

#ifndef STDEF_SELFTESTDEFINITIONS_H
#define STDEF_SELFTESTDEFINITIONS_H

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
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/

#ifdef __ASM__

#define STDEF_RET_BUSY  0x2638

#else

/* Definitions of return codes for the selftest package. */
typedef enum
{
   STDEF_RET_DONE = 0x2516u,
   STDEF_RET_BUSY = 0x2638u,
   STDEF_RET_ERR  = 0x27afu
} STDEF_RETCODE_ENUM;
#endif

/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/

/* Macros for handling the HW-CRC unit. */
/*!< Enable the HW-CRC unit */
#define STDEF_MCU_CRC_ENABLE           { RCC->AHBENR |= RCC_AHBENR_CRCEN; }          
/*!< Reset the HW-CRC unit incl. short wait */
#define STDEF_MCU_CRC_RESET            { CRC->CR = CRC_CR_RESET; __NOP(); __NOP(); } 
/*!< Add a value to the HW-CRC unit */
#define STDEF_MCU_CRC_ADD_VALUE(value) { CRC->DR = (UINT32)value; }    
/*!< Get the calculated CRC of the HW-CRC unit */              
#define STDEF_MCU_CRC_GETRESULT        ((UINT32)CRC->DR)           

/* This constant defines the default value of the CRC32 */
#define STDEF_CRC32_PRESET 0xFFFFFFFFuL
#define STDEF_CRC16_PRESET ((UINT16) 0xFFFFu)

/* This constant defines the shift value*/
#define STDEF_SHIFT_8_BIT  ((UINT8) 8u)

/* These constants defines some masks for the UINT16 handling. */
#define STDEF_MASK_UINT16_HIGH_BYTE ((UINT16) 0xFF00u)
#define STDEF_MASK_UINT16_LOW_BYTE  ((UINT16) 0x00FFu)

#define STDEF_RETCODE_SAFETY_ASSERT(x) \
   GLOBFAIL_SAFETY_ASSERT((STDEF_RET_DONE == x) || (STDEF_RET_BUSY == x),  \
                          GLOB_FAILCODE_VARIABLE_ERR)

#define STDEF_RETCODE_SAFETY_ASSERT_RET(x, ret) \
   GLOBFAIL_SAFETY_ASSERT_RET((STDEF_RET_DONE == x) || (STDEF_RET_BUSY == x), \
                              GLOB_FAILCODE_VARIABLE_ERR, ret)

/***** End of: macros Macros ********************************************//*@}*/

/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* STDEF_SELFTESTDEFINITIONS_H */

/**** Last line of source code                                             ****/

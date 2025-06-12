/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: pflow_ProgFlowCtrl.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          pflow_ProgFlowCtrl.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Header of program flow control.
 * @description:
 * The Program Flow Control is to ensure that the firmware works as expected
 * and that any safety relevant function is executed by an expected call.
 * For this purpose the Program Flow Control uses a global checksum calculation.
 * Each called safety function adds an unique ID to the Program Flow Control.
 * At the end of the respective program sequence, the calculated checksum is
 * checked against an expected value which was calculated during the development
 * process.
 * If the compared checksums equal, the Program Flow Control is reset. If
 * they are not equal, the safety handler is called.
 *
 * This module provides functions to initialize this module, to add a signature
 * to the checksum calculation and a function to check the calculated checksum
 * against an expected.
 */
/************************************************************************//*@}*/

#ifndef PFLOW_PROGFLOWCTRL_H
#define PFLOW_PROGFLOWCTRL_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#include "pflow_ProgFlowCtrl_cfg.h"

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*! Expected checksum of the selftests stored as fixed value (see [SRS_698]) */
#define PFLOW_SIGNATURE_SELFTEST  (UINT16)0x9AF6u

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/

/*! This enum contains the IDs for the Program Flow Control for the selftests.
 *  The values of this enum are provided with a hamming distance. */
typedef enum
{
   PFLOW_E_SIG_ROM_TEST       = 0x1c85u,   /*!< ROM Test */
   PFLOW_E_SIG_RAM_TEST       = 0x1d12u,   /*!< RAM Test */
   PFLOW_E_SIG_STACK_TEST     = 0x1e3cu,   /*!< Stack Test */
   PFLOW_E_SIG_OPCODE_TEST    = 0x1fabu,   /*!< Op-code Test */
   PFLOW_E_SIG_SFR_TEST       = 0x2064u,   /*!< SFR Test */
   PFLOW_E_SIG_CORE_TEST      = 0x21f3u,   /*!< Core-Register Test */
   PFLOW_E_SIG_SAFCON         = 0x22ddu    /*!< Safety-Container Test */
} PFLOW_PRGFLW_SIGNATURE_ENUM;

/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/



/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

extern void pflow_Init (void);
extern void pflow_AddSignature (CONST PFLOW_PRGFLW_SIGNATURE_ENUM eSignature);
extern void pflow_CheckSignature (CONST UINT16 u16Crc);

/***** End of: Function-Prototypes ********************************************/


#endif /* PFLOW_PROGFLOWCTRL_H */

/**** Last line of source code                                             ****/

/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: pflow_ProgFlowCtrl.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          pflow_ProgFlowCtrl.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        This module contains the Program Flow Control.
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

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "stDef_SelftestDefinitions.h"
#include "crc-hal.h"

/* Module Header */
#include "pflow_ProgFlowCtrl.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! Calculated CRC for the function signatures (self test)*/
STATIC UINT16 pflow_u16PflowCtrl1Crc;

/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/



/***** End of: globvar Moduleglobal Variables ***************************//*@}*/


/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Local Functions                                                            */
/********************************//*!@addtogroup locfunc Local Functions*//*@{*/

/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/


/******************************************************************************/
/*!
@Name                pflow_Init

@Description         Initialization of the Program Flow Control.

@note                The init function set the modulglobal CRC-variable to its
                     preset value.

@Parameter
   @return           void
 */
/******************************************************************************/
void pflow_Init (void)
{
   pflow_u16PflowCtrl1Crc = STDEF_CRC16_PRESET;
}


/******************************************************************************/
/*!
@Name                pflow_AddSignature

@Description         This function adds a signature to the Program Flow Control.

@note                -

@Parameter
   @param [in]       eSignature    Signature of the function
   @return           void
 */
/******************************************************************************/
void pflow_AddSignature (CONST PFLOW_PRGFLW_SIGNATURE_ENUM eSignature)
{
   /* add signature to crc */
   pflow_u16PflowCtrl1Crc = crcHAL_CalcCRC16(pflow_u16PflowCtrl1Crc,
                (UINT8)(((UINT16)eSignature) & STDEF_MASK_UINT16_LOW_BYTE));
   pflow_u16PflowCtrl1Crc = crcHAL_CalcCRC16(pflow_u16PflowCtrl1Crc,
                (UINT8)((((UINT16)eSignature) & STDEF_MASK_UINT16_HIGH_BYTE) >> STDEF_SHIFT_8_BIT));
}


/******************************************************************************/
/*!
@Name                pflow_CheckSignature

@Description         This function checks the calculated checksum of the Program
                     Flow against the expected.

@note                If the compared checksums equal, the Program Flow Control
                     is reset. If they are not equal, the error handler is
                     called.
                     After checking the checksums the variable for the crc
                     calculation is set to its preset value.

@Parameter
   @param [in]       u16Crc         Expected checksum of the current Program Flow
   @return           void
 */
/******************************************************************************/
void pflow_CheckSignature (CONST UINT16 u16Crc)
{
   /* Check the calculated checksum against the expected (see [SRS_697]).
    * If they are not equal, call the error handler, see [SRS_359]. */
   GLOBFAIL_SAFETY_ASSERT(u16Crc == pflow_u16PflowCtrl1Crc, GLOB_FAILCODE_PFLOW_SIGNATURE);

   /* reset the checksum for the next cycle */
   pflow_u16PflowCtrl1Crc = STDEF_CRC16_PRESET;
}

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

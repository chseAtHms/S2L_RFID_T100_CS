/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: safcon_SafetyContainer.c 4693 2024-11-06 10:18:57Z ankr $
 * $Revision: 4693 $
 *     $Date: 2024-11-06 11:18:57 +0100 (Mi, 06 Nov 2024) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          safcon_SafetyContainer.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief         This module contains the Safety-Container Test.
 * @description:
 * The module Safety container is responsible for backing up the configuration
 * parameters. The configuration parameters are data that will never change
 * during runtime. They are for example read at startup from the EEPROM and
 * then kept in RAM.
 * To protect the data against corruption a checksum is calculated cyclically
 * over the values of the parameters in the RAM.
 *
 * At startup an initial checksum is calculated over the parameters. This
 * checksum is the basic value and it is used to check against the cyclically
 * calculated checksum. If the checksums not equal at the checking time the
 * error handler will be called.
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "globFit_FitTestHandler.h"
#include "stDef_SelftestDefinitions.h"
#include "pflow_ProgFlowCtrl.h"

/* includes of the modules for the safety container */
#include "cfg_Config-sys.h"
#include "gpio_cfg.h"
#include "fiParam.h"

/* Header-file of module */
#include "safcon_SafetyContainer.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! Safety-Container CRC calculated at startup. */
STATIC UINT32 safcon_u32SafeConBaseCrc;

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



/******************************************************************************/
/*!
@Name                safcon_CalcContainerCrc

@Description         This function calculates the checksum over the
                     Safety-Container.

@note                This function must have an access to other modules to get
                     the values of the data for the safety container.
                     It collect the data of the specified data and calculates
                     a checksum over this data.

@Parameter
   @return           crc - calculated checksum
 */
/******************************************************************************/
STATIC UINT32 safcon_CalcContainerCrc (void)
{
   CFG_CONTROLLER_ID_ENUM eControllerID = cfgSYS_GetControllerID();
   CFG_CONTROLLER_ID_SAFETY_ASSERT(eControllerID)

   STDEF_MCU_CRC_RESET;

   /* Add the controller ID to the Safety Container crc */
   STDEF_MCU_CRC_ADD_VALUE((UINT32)eControllerID);

   /* Add values to CRC of the Safety Container of the fiParam module,
    * (iParameter and F-Parameter, see [SRS_2050]) */
   fiParam_CalcFiParamSafeConCrc();

   return ((UINT32)STDEF_MCU_CRC_GETRESULT);
}

/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/

/******************************************************************************/
/*!
@Name                safcon_Init

@Description         This function initialize the Safety-Container and
                     calculates the basic checksum.

@note                -

@Parameter
   @return           void
 */
/******************************************************************************/
void safcon_Init (void)
{
   /* calculate basic checksum */
   safcon_u32SafeConBaseCrc = safcon_CalcContainerCrc();
}


/******************************************************************************/
/*!
@Name                safcon_CheckCrc

@Description         This function calculates the Safety-Container checksum and
                     checks it against the basic checksum.

@note                Additional the signature for the program flow is set.

@Parameter
   @return           void
 */
/******************************************************************************/
void safcon_CheckCrc (void)
{
   UINT32   u32Crc;

   /* calculate the Safety-Container checksum, see [SRS_464] */
   u32Crc = safcon_CalcContainerCrc();

   /* This FIT test manipulates the logical program flow. The RAM test
    * is skipped. */
   /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
   GLOBFIT_FITTEST(GLOBFIT_CMD_SAFETY_CONTAINER, u32Crc ^= 1u);
   /* RSM_IGNORE_QUALITY_END */

   /* check the calculated checksum against the basic checksum, enter FS state
    * in case of an error, see [SRS_359] */
   GLOBFAIL_SAFETY_ASSERT((u32Crc == safcon_u32SafeConBaseCrc),
                          GLOB_FAILCODE_SAFCON_CRC);

   /* set signature to logical program flow control, see [SRS_697] */
   pflow_AddSignature(PFLOW_E_SIG_SAFCON);
}

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

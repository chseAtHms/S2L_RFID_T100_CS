/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: sfrcrc_Interface.c 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          sfrcrc_Interface.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        The interface for the CRC calculation mapping.
 * @description:  -
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

/* Module header */
#include "stDef_SelftestDefinitions.h"
#include "SFRCRC_typ.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* Header-file of module */
#include "sfrcrc_Interface.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/


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
@Name                crc_AddU8

@Description         Interface for the CRC8 calculation.
                     If the function is not used, insert a jump to the
                     Safety Handler.

@note                -

@Parameter
   @param [in]       uCrc        CRC to add the value.
   @param [in]       u8Value     Value to add to the CRC.
   @return           uCrc        New calculated CRC.
 */
/******************************************************************************/
SFRCRC crc_AddU8 (CONST SFRCRC uCrc, CONST UINT8 u8Value)
{
   /*! include here the CRC8 calculation */

   (void)u8Value;

   /* Go to the Safety Handler if this function is called.*/
   GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_SFR_TEST);

   return uCrc;
}


/******************************************************************************/
/*!
@Name                crc_AddU16

@Description         Interface for the CRC16 calculation.
                     If the function is not used, insert a jump to the
                     Safety Handler.

@note                -

@Parameter
   @param [in]       uCrc        CRC to add the value.
   @param [in]       u16Value    Value to add to the CRC.
   @return           uCrc        New calculated CRC.
 */
/******************************************************************************/
SFRCRC crc_AddU16 (CONST SFRCRC uCrc, CONST UINT16 u16Value)
{
   /*! include here the CRC16 calculation */

   (void)u16Value;

   /* Go to the Safety Handler if this function is called.*/
   GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_SFR_TEST);

   return uCrc;
}


/******************************************************************************/
/*!
@Name                crc_AddU32

@Description         Interface for the CRC32 calculation.
                     If the function is not used, insert a jump to the
                     Safety Handler.

@note                This function calls the controller internal CRC calculation
                     unit for calculating the CRC32.

@Parameter
   @param [in]       uCrc        CRC to add the value.
   @param [in]       u32Value    Value to add to the CRC.
   @return           uCrc        New calculated CRC.
 */
/******************************************************************************/
SFRCRC crc_AddU32 (CONST SFRCRC uCrc, CONST UINT32 u32Value)
{
   /* check if the crc is not changed */
   GLOBFAIL_SAFETY_ASSERT_RET(uCrc == STDEF_MCU_CRC_GETRESULT,
                              GLOB_FAILCODE_SFR_TEST,
                              uCrc);

   /*! CRC32 calculation */
   STDEF_MCU_CRC_ADD_VALUE(u32Value);

   return STDEF_MCU_CRC_GETRESULT;
}


/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

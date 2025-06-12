/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: romtst_RomTest.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          romtst_RomTest.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        This module contains the ROM-Test.
 * @description:
 * This module provides the ROM-Test at startup and for the runtime.
 * The ROM-Test for the runtime is executed consecutively by calling the
 * DoRomTest-function. The Startup-ROM-Test is called by DoStartupRomTest.
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
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "stDef_SelftestDefinitions.h"
#include "pflow_ProgFlowCtrl.h"
#include "globFit_FitTestHandler.h"

/* Header-file of module */
#include "romtst_RomTest.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER ****************/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/


/*! This variable contains the current ROM-address of the ROM-Test */
STATIC RDS_UINT32 romtst_u32RdsCurrentAddr;

/*! This variable saves the CRC during the ROM-Test cycles. */
STATIC UINT32 romtst_u32Crc = STDEF_CRC32_PRESET;

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
@Name                romtst_InitRomTest

@Description         Init-function of the ROM-Test

@note                This function initialize the ROM-Test. The initialization
                     contains the activation of the HW-CRC module and resets
                     the CRC-Value to the preset value. Furthermore it sets the
                     start address of the ROM Test to the first ROM address.

@Parameter
   @return           void
 */
/******************************************************************************/
void romtst_InitRomTest (void)
{
   /* enable the HW-CRC module. */
   STDEF_MCU_CRC_ENABLE;

   /* reset the HW-CRC calculation, init value is 0xFFFFFFFF */
   STDEF_MCU_CRC_RESET;

   /* set the ROM start address */
   RDS_SET(romtst_u32RdsCurrentAddr, ROMTST_ROM_START)

   /* set the ROM-CRC default value */
   romtst_u32Crc = STDEF_CRC32_PRESET;
}


/******************************************************************************/
/*!
@Name                romtst_DoRomTest

@Description         ROM-Test

@note                This function executes consecutively the ROM-Test (see [SRS_690]).
                     It is tested section for section (see [SRS_695]).
                     The checksum is calculated with the HW-CRC unit.
                     The HW-CRC unit holds the checksum after a calculation
                     procedure up to the next cycle. The calculated checksum
                     is checked against the checksum in the ROM after a
                     complete cycle (see [SRS_51]). In case of an error the
                     SAFETY_FAIL-Function is called directly.
                     The ROM test cycles are counted. At every function call the
                     counter is checked, is it greater than the maximum count of
                     cycles, the Safety Handler is called.

@Parameter
   @return           eRet        STDEF_RET_BUSY - ROM-Test in process
                                 STDEF_RET_DONE - ROM-Test successful done
 */
/******************************************************************************/
STDEF_RETCODE_ENUM romtst_DoRomTest (void)
{
   UINT32 u32EndOfTestRange; /* the rest of the testable bytes */
   UINT32 u32Address; /* variable of the currently tested memory cell */
   STDEF_RETCODE_ENUM eRet = STDEF_RET_ERR;
#ifdef GLOBFIT_FITTEST_ACTIVE
   LOCAL_STATIC(, UINT32*, pu32RomPointer, (UINT32*) ROMTST_ROMCRC_ADDRESS);
#else
   /* see [SRS_50], ROM test gets the generated CRC */
   LOCAL_STATIC(CONST, UINT32* CONST, pu32RomPointer, (CONST UINT32*) ROMTST_ROMCRC_ADDRESS);
#endif
   LOCAL_STATIC(, UINT16, u16RomTestCycles, ((UINT16)0u));/*ROM-Test cycles */

   /* Lint message Note 948 "Operator '==' always evaluates to False"
    * deactivated because problem is in RDS_GET. */

   /* check the test cycles of the ROM-Test */
   GLOBFAIL_SAFETY_ASSERT_RET(u16RomTestCycles < ROMTST_CYCLES,
                              GLOB_FAILCODE_ROM_TEST,
                              STDEF_RET_ERR)
   /* check the current address that it is smaller than the ROM-END */
   GLOBFAIL_SAFETY_ASSERT_RET(RDS_GET(romtst_u32RdsCurrentAddr) < ROMTST_ROM_END,/*lint !e948*/
                              GLOB_FAILCODE_ROM_TEST,
                              STDEF_RET_ERR)

   /* reset the CRC in the first ROMtest cycle */
   if (u16RomTestCycles == 0u)
   {
      /* reset the HW-CRC unit */
      STDEF_MCU_CRC_RESET;
   }

   u16RomTestCycles++;

   /* check if all OC-Blocks tested */
   /* calculate the ROM end address for the current cycle */
   if (((ROMTST_ROM_END + 1u) - RDS_GET(romtst_u32RdsCurrentAddr)) 
         > (UINT32)ROMTST_SIZE)/*lint !e948*/
   {
      u32EndOfTestRange = (UINT32) RDS_GET(romtst_u32RdsCurrentAddr)
         + (UINT32) ROMTST_SIZE;/*lint !e948*/
   }
   else
   {
      u32EndOfTestRange = ROMTST_ROM_END + 1u;
   }

#if (ROMTST_DISABLE == FALSE)
   /* check if the HW-CRC has changed since the last cycle */
   GLOBFAIL_SAFETY_ASSERT_RET(romtst_u32Crc == STDEF_MCU_CRC_GETRESULT, 
                              GLOB_FAILCODE_VARIABLE_ERR, 
                              STDEF_RET_ERR);
#endif

   /* calculate the CRC of the specified range, see [SRS_51], [SRS_695] */
   for (u32Address = RDS_GET(romtst_u32RdsCurrentAddr); u32Address
      < u32EndOfTestRange; u32Address++)
   {
      STDEF_MCU_CRC_ADD_VALUE(*((UINT8*) u32Address));/*lint !e923*/
      /* PC-Lint Message deactivated because the value of u32Address is a
       * memory address and the value on this address is used for the CRC
       * calculation. */
   }

   /* set the current address to the next range */
   RDS_SET(romtst_u32RdsCurrentAddr, u32EndOfTestRange)

   /* FIT to manipulate the ROM test cycle counter, set the value to an invalid value */
   GLOBFIT_FITTEST(GLOBFIT_CMD_ROM_TEST_CYCL1, (u16RomTestCycles = (ROMTST_CYCLES + 1u)));

   /* all cells are tested */
   if (u32EndOfTestRange > (ROMTST_ROM_END))
   {
      /* FIT to manipulate the ROM-CRC calculation */
      GLOBFIT_FITTEST(GLOBFIT_CMD_ROM_TEST_CRC1, STDEF_MCU_CRC_ADD_VALUE(1u));

      /* FIT to manipulate the CRC-value which is stored in the ROM */
      GLOBFIT_FITTEST(GLOBFIT_CMD_ROM_TEST_CRC2, pu32RomPointer++);

#if (ROMTST_DISABLE == FALSE)
      /* check the calculated checksum against the checksum in the ROM, see [SRS_51] */
      if (STDEF_MCU_CRC_GETRESULT == *pu32RomPointer)
      {
#else
#warning ------------ ROM-Test is disabled! ------------
      if (pu32RomPointer != 0x00u)
      {
#endif
         /* insert the ROM-Test to the program flow control, see [SRS_697] */
         pflow_AddSignature(PFLOW_E_SIG_ROM_TEST);

         /* ROM-Test successful */
         eRet = STDEF_RET_DONE;

         /* reset the HW-CRC unit */
         STDEF_MCU_CRC_RESET;
      }
      else
      {
         /* the calculated checksum does not match the existing in ROM,
          * see [SRS_359] */
         GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_ROM_TEST, STDEF_RET_ERR);
      }

      RDS_SET(romtst_u32RdsCurrentAddr, ROMTST_ROM_START)
      romtst_u32Crc = STDEF_CRC32_PRESET;
      u16RomTestCycles = ((UINT16)0U);
   }
   else
   {
      /* buffer the calculated checksum*/
      romtst_u32Crc = STDEF_MCU_CRC_GETRESULT;
      eRet = STDEF_RET_BUSY;
   }

   GLOBFAIL_SAFETY_ASSERT_RET((eRet == STDEF_RET_DONE) || (eRet == STDEF_RET_BUSY),
                              GLOB_FAILCODE_ROM_TEST,
                              STDEF_RET_ERR)

   return (eRet);
}


/******************************************************************************/
/*!
@Name             romtst_DoStartupRomTest

@Description      Startup ROM-Test

@note             This function executes the ROM-Test at startup (see [SRS_689]).
                  The checksum is calculated with the HW-CRC unit. The HW-CRC
                  unit holds the checksum after a calculation procedure up to
                  the next cycle.
                  The calculated checksum is checked against the checksum in the
                  ROM after a complete cycle (see [SRS_51]).
                  In case of an error the SAFETY_FAIL-Function is called directly.

@Parameter
   @return           void
 */
/******************************************************************************/
void romtst_DoStartupRomTest (void)
{
   UINT32 u32Address = ROMTST_ROM_START;

   /* pointer to the checksum in ROM, see [SRS_50] */
   LOCAL_STATIC(CONST, UINT32* , pu32Crc, (CONST UINT32*) ROMTST_ROMCRC_ADDRESS);

   /* init the HW-CRC unit */
   STDEF_MCU_CRC_RESET;

   /* calculate the ROM-CRC, see [SRS_51] */
   do
   {
      STDEF_MCU_CRC_ADD_VALUE(*((UINT8*) u32Address));/*lint !e923*/
      /* PC-Lint Message deactivated because the value of u32Address is a
       * memory address and the value on this address is used for the CRC
       * calculation. */
      u32Address++;
   } while (u32Address < (ROMTST_ROM_END + 1u));

   /* FIT to manipulate the calculated ROM-CRC. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_ROM_TEST_STARTUP, STDEF_MCU_CRC_ADD_VALUE(1u));

   /* check the checksum, see [SRS_51] */
   if (*pu32Crc != STDEF_MCU_CRC_GETRESULT)
   {
   #if (ROMTST_DISABLE == TRUE)
   #warning ------------ ROM-Test is disabled! ------------
   #else
      /* checksum is not valid => enter FS, see [SRS_82], [SRS_359] */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_ROM_TEST);
   #endif
   }
}/*lint !e843*/
/* Info 843: Variable 'pu32Crc' (line 279) could be declared as const possibly
 * vioalation of [MISRA 2004 Rule 10.1]
 * --> Variable is declared as const, PC-Lint has problems with LOCAL_STATIC. */

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

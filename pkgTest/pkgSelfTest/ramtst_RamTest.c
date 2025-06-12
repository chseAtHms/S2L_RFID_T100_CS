/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: ramtst_RamTest.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          ramtst_RamTest.c
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

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "stDef_SelftestDefinitions.h"
#include "globFail_SafetyHandler.h"
#include "globFit_FitTestHandler.h"
#include "pflow_ProgFlowCtrl.h"

/* Header-file of module */
#include "ramtst_RamTest.h"
#include "ramtst_RamTest_loc.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER ****************/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! This variable contains the address of the currently tested RAM cell */
STATIC RDS_UINT32 ramtst_u32RdsBcAddr;

/*! This variable contains the start address of the OC area (test area). */
STATIC RDS_UINT32 ramtst_u32RdsOcBlkAddr;

/*! This variable contains the number of the test cycles */
STATIC UINT32 ramtst_u32RamTestCycles;

/*! This variable contains the two RAM-Test steps. */
STATIC RAMTST_TESTSTEPS_ENUM ramtst_eTestStep = RAMTST_E_NOINIT;

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
@Name                ramtst_DoRamTestStep1

@Description         This function handles the RAM-Test Step 1.

@note                In the first step of the RAM-Test the RAM is tested
                     against Stuck-At-Failures. For further information how the
                     RAM-Test is working, see the "BGIA-Report 7/2006" chapter
                     4.2. A short description of the functionality is also in
                     the header description of the asm_RamTestStep1 function.

                     As input for the RAM Test Step 1 is the BC address. The
                     BC address must be on the RAM_START or greater and less
                     than the RAM_END. If the BC address is not in range, the
                     Safety Handler is called.
                     Furthermore, the RAM test cycles are counted. At every
                     function call the counter is checked, is it greater than
                     the maximum count of cycles, the Safety Handler is called.

@Parameter
   @return           eRet        STDEF_RET_BUSY - RAM-Test in process
                                 STDEF_RET_DONE - RAM-Test successful done
 */
/******************************************************************************/
STATIC STDEF_RETCODE_ENUM ramtst_DoRamTestStep1 (void)
{
   STDEF_RETCODE_ENUM eRet;

   /* the number of the cycles have to equal with the expected */
   GLOBFAIL_SAFETY_ASSERT_RET((ramtst_u32RamTestCycles < RAMTST_CYCLES_STEP1),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)
   /* check that BC-Address is on the RAM-START or greater and less than the RAM-END. */
   GLOBFAIL_SAFETY_ASSERT_RET( (   (RDS_GET(ramtst_u32RdsBcAddr) < RAMTST_RAM_END) \
                                && (RDS_GET(ramtst_u32RdsBcAddr) >= RAMTST_RAM_START)),
                               GLOB_FAILCODE_RAM_TEST,
                               STDEF_RET_ERR)/*lint !e948*/
   /* Deactivated Lint message because it comes from the RDS macro. */

   /* disable all interrupts to avoid an interrupt of the RAM-Test */
   __disable_irq();

   /* RAM-Test Step 1 */
   eRet = asm_RamTestStep1 (RDS_GET(ramtst_u32RdsBcAddr),
                            RAMTST_STEP1_TEST_WORDS);/*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* check the return value, see [SRS_359] */
   GLOBFAIL_SAFETY_ASSERT_RET((eRet == STDEF_RET_BUSY),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)

   /* enable all interrupts */
   __enable_irq();

   /* increase the BC */
   RDS_ADD(ramtst_u32RdsBcAddr,
           (RAMTST_STEP1_TEST_WORDS * ((UINT32) sizeof(UINT32))));

   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */
   /* check if end of RAM achieved? */
   if (RDS_GET(ramtst_u32RdsBcAddr) >= RAMTST_RAM_END)/*lint !e948*/
   {
      /* set BC to RAM start */
      RDS_SET(ramtst_u32RdsBcAddr,
              RAMTST_RAM_START);

      /* delete test cycles for the next test */
      ramtst_u32RamTestCycles = 0UL;

      /* RAM Test Step 1 successful */
      eRet = STDEF_RET_DONE;
   }
   else
   {
      /* increase the RAM test cycles */
      ramtst_u32RamTestCycles++;
   }

   /* FIT to set the cycle counter of the test steps to invalid value. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAM_TEST_STEP1_3,
                   ramtst_u32RamTestCycles = RAMTST_CYCLES_STEP1 + 1u);

   return eRet;
}


/******************************************************************************/
/*!
@Name                ramtst_DoRamTestStep2

@Description         This function handles the RAM-Test Step 2.

@note                In the second step of the RAM-Test the RAM is tested
                     against following failures:
                     - DC fault model for data and addresses
                     - Static cross-over for memory cells
                     For further information how the RAM-Test is working, see
                     the "BGIA-Report 7/2006" chapter 4.2. A short description
                     of the functionality is also in the header description of
                     the asm_RamTestStep2 function.

                     As input for the RAM Test Step 2 is the BC and OC block
                     address.
                     The BC address must be on the RAM_START or greater and less
                     than the RAM_END - 1. If the BC address is not in range,
                     the Safety Handler is called.
                     The OC block address must be on the RAM_START or greater
                     and less than the RAM_END. If the OC block address is not
                     in range, the Safety Handler is called.
                     Furthermore, the RAM test cycles are counted. At every
                     function call the counter is checked, is it greater than
                     the maximum count of cycles, the Safety Handler is called.

@Parameter
   @return           eRet        STDEF_RET_BUSY - RAM-Test in process
                                 STDEF_RET_DONE - RAM-Test successful done
 */
/******************************************************************************/
STATIC STDEF_RETCODE_ENUM ramtst_DoRamTestStep2 (void)
{
   STDEF_RETCODE_ENUM eRet;

   /* FIT to set the cycle counter of the test steps to invalid value. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAM_TEST_STEP2_3, ramtst_u32RamTestCycles = RAMTST_CYCLES_STEP2);

   /* check if an overflow of the cycles happens */
   GLOBFAIL_SAFETY_ASSERT_RET((ramtst_u32RamTestCycles < RAMTST_CYCLES_STEP2),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)
   /* check that BC-Address is on the RAM-START or greater and on the RAM-END or less. */
   GLOBFAIL_SAFETY_ASSERT_RET((   (RDS_GET(ramtst_u32RdsBcAddr) <= (RAMTST_RAM_END - 1u)) \
                               && (RDS_GET(ramtst_u32RdsBcAddr) >= (RAMTST_RAM_START))),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)/*lint !e948*/
   /* check that OC-Address is on the RAM-Start and greater and less than the RAM-END. */
   GLOBFAIL_SAFETY_ASSERT_RET((   (RDS_GET(ramtst_u32RdsOcBlkAddr) <  RAMTST_RAM_END) \
                               && (RDS_GET(ramtst_u32RdsOcBlkAddr) >= RAMTST_RAM_START)),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)/*lint !e948*/
   /* Deactivated Lint message because it comes from the RDS macro. */

   /* disable all interrupts to avoid an interrupt of the RAM-Test */
   __disable_irq();

   /* call the assembler test routine */
   eRet = asm_RamTestStep2(RDS_GET( ramtst_u32RdsBcAddr),
                           RDS_GET(ramtst_u32RdsOcBlkAddr),
                           RAMTST_STEP2_OC_TEST_BYTES);/*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* check the return value, see [SRS_359] */
   GLOBFAIL_SAFETY_ASSERT_RET((eRet == STDEF_RET_BUSY),
                              GLOB_FAILCODE_RAM_TEST,
                              STDEF_RET_ERR)

   /* enable all interrupts */
   __enable_irq();

   /* increase the RAM test cycles */
   ramtst_u32RamTestCycles++;

   /* calculate the next address of the OC-Block */
   RDS_ADD(ramtst_u32RdsOcBlkAddr, RAMTST_STEP2_OC_TEST_BYTES)

   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */
   /* check if all OC-Blocks tested */
   if (RDS_GET(ramtst_u32RdsOcBlkAddr) >= RAMTST_RAM_END)/*lint !e948*/
   {
      if (RDS_GET(ramtst_u32RdsBcAddr) == (RAMTST_RAM_END - 1u))/*lint !e948*/
      {
         /* all BC-Blocks are complete, restart the RAM-Test */
         RDS_SET(ramtst_u32RdsBcAddr, RAMTST_RAM_START)

         /* the number of the cycles have to equal with the expected, see [SRS_359] */
         GLOBFAIL_SAFETY_ASSERT_RET((ramtst_u32RamTestCycles == RAMTST_CYCLES_STEP2),
                                    GLOB_FAILCODE_RAM_TEST,
                                    STDEF_RET_ERR)

         /* RAM-Test successful */
         eRet = STDEF_RET_DONE;


         /* set RAM test cycles to start value */
         ramtst_u32RamTestCycles = 0UL;
      }
      else
      {
         /* complete OC-Block tested, increase BC */
         RDS_INC(ramtst_u32RdsBcAddr)
      }
      /* set OC to the RAM start address to test the next BC */
      RDS_SET(ramtst_u32RdsOcBlkAddr, RAMTST_RAM_START)
   }

   return (eRet);
}


/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/

/******************************************************************************/
/*!
@Name                ramtst_DoStartupRamTest

@Description         This function executes the MarchC Test at Startup.

@note
The MarchC-Test is executed at startup and tests the RAM (see [SRS_263]).
It is important to start this test before the RAM is initialized. The MarchC
destroys the data in RAM.
The MarchC-Test detects the following failures:
Stuck-at faults (SAF), Transition faults (TF), Coupling faults (CF) und
 Address decoder faults (AF)
In case of an error the safety handler is called.

@Parameter
 @param[in] u16StartAddr   start address of the test area
 @param[in] u16EndAddr     last address to test

 @return    void
 */
/******************************************************************************/
#ifndef __CTC_TESTING_ON__
void ramtst_DoStartupRamTest (CONST UINT32 u32StartAddr, CONST UINT32 u32EndAddr)
/* function must be not static and avoid lint message, it is called from the startup assembler 
** file. */
{
   UINT32 u32Index;
   UINT8* pu8;       /* pointer to RAM */
   UINT32 u32BlkLen;

   /* --- the end address must be greater than the start address */
   GLOBFAIL_SAFETY_ASSERT((u32EndAddr >= u32StartAddr), GLOB_FAILCODE_INVALID_PARAM)

   u32BlkLen = (u32EndAddr + 1u) - u32StartAddr;
   /* --- the start address and the block length must be divisible by the
    *     alignment without a rest */
   GLOBFAIL_SAFETY_ASSERT(((u32StartAddr % RAMTST_MEMORY_ALIGNMENT) == 0U) &&
                          ((u32BlkLen % RAMTST_MEMORY_ALIGNMENT) == 0U),
                          GLOB_FAILCODE_INVALID_PARAM)/*lint !e587*/
   /* Avoid Lint message because something modulo 1 is always 0, MEMORY_ALIGNMENT has the value 1.*/

   /* write 0 to all cells */
   pu8 = MAKE_RAM_PTR (UINT8, u32StartAddr);
   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      *pu8 = (UINT8)0U;
      /* RSM_IGNORE_QUALITY_BEGIN Notice #7 - Pre-increment operator '++' identified */
      ++pu8;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
      /* RSM_IGNORE_QUALITY_END */
   }

   /* begin in ascendending sequence with the start address and test every cell
    * against 0 and write 0xFF to it  */
   pu8 = MAKE_RAM_PTR (UINT8, u32StartAddr);

   /* This FIT Test manipulates a RAM cell. One bit in the RAM cell is set from
    * 0 to 1. This simulates a stuck-at high failure. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAMTEST_STARTUP_ERR1, (*(pu8 + (u32BlkLen/2u)) = 0x01u));

   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      if (*pu8 == 0u)
      {
         *pu8 = (UINT8)0xFFU;
         /* RSM_IGNORE_QUALITY_BEGIN Notice #7 - Pre-increment operator '++' identified */
         ++pu8;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
         /* RSM_IGNORE_QUALITY_END */
      }
      else
      {
         /* FS in error case, see [SRS_82], [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_MARCHC_TEST);
      }
   }

   /* This FIT Test manipulates a RAM cell. One bit in the RAM cell is set from
    * 1 to 0. This simulates a stuck-at low failure. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAMTEST_STARTUP_ERR2, (*(pu8 - ((u32BlkLen/2u) - 2u)) = 0xDFu));

   /* begin in ascendending sequence with the start address and test every cell
    * against 0xFF and write 0 to it  */
   pu8 = MAKE_RAM_PTR (UINT8,u32StartAddr);
   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      if (*pu8 == 0xFFU)
      {
         *pu8 = (UINT8)0U;
         pu8++;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
      }
      else
      {
         /* FS in error case, see [SRS_82], [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_MARCHC_TEST);
      }
   }

   /* This FIT Test manipulates a RAM cell. One bit in the RAM cell is set from
    * 0 to 1. This simulates a stuck-at high failure. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAMTEST_STARTUP_ERR3, (*(pu8 - ((u32BlkLen/2u) + 8u)) = 0x10u));

   /* begin in descendending sequence with the end address and test every cell
    * against 0 and write 0xFF to it  */
   pu8 = MAKE_RAM_PTR (UINT8,u32EndAddr);
   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      if (*pu8 == 0U)
      {
         *pu8 = (UINT8)0xFFU;
         /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
         --pu8;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
         /* RSM_IGNORE_QUALITY_END */
      }
      else
      {
         /* FS in error case, see [SRS_82], [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_MARCHC_TEST);
      }
   }

   /* This FIT Test manipulates a RAM cell. One bit in the RAM cell is set from
    * 1 to 0. This simulates a stuck-at high failure. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAMTEST_STARTUP_ERR4, (*(pu8 + ((u32BlkLen/2u) + 5u)) = 0xFDu));

   /* begin in descendending sequence with the end address and test every cell
    * against 0xFF and write 0 to it  */
   pu8 = MAKE_RAM_PTR (UINT8,u32EndAddr);
   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      if (*pu8 == 0xFFU)
      {
         *pu8 = (UINT8)0x00U;
         /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
         --pu8;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
         /* RSM_IGNORE_QUALITY_END */
      }
      else
      {
         /* FS in error case, see [SRS_82], [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_MARCHC_TEST);
      }
   }

   /* This FIT Test manipulates a RAM cell. One bit in the RAM cell is set from
    * 0 to 1. This simulates a stuck-at high failure. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RAMTEST_STARTUP_ERR5, (*(pu8 + ((u32BlkLen/2u) - 9u)) = 0x08u));

   /* test all cells against 0 */
   pu8 = MAKE_RAM_PTR (UINT8,u32StartAddr);
   /* RSM_IGNORE_QUALITY_BEGIN Notice #6 - Pre-decrement operator '--' identified */
   for (u32Index = u32BlkLen; u32Index > 0UL; --u32Index)
   /* RSM_IGNORE_QUALITY_END */
   {
      if (*pu8 == 0U)
      {
         /* RSM_IGNORE_QUALITY_BEGIN Notice #7 - Pre-increment operator '++' identified */
         ++pu8;   /*lint !e960*//* Increment of pointer is desired to get the next RAM-address. */
         /* RSM_IGNORE_QUALITY_END */
      }
      else
      {
         /* FS in error case, see [SRS_82], [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_MARCHC_TEST);
      }
   }
}
#else /* __CTC_TESTING_ON__ */
void ramtst_DoStartupRamTest (CONST UINT32 u32StartAddr, CONST UINT32 u32EndAddr)
{
   (void)u32StartAddr;
   (void)u32EndAddr;
}
#endif /* __CTC_TESTING_ON__ */

/******************************************************************************/
/*!
@Name                ramtst_InitRamTest

@Description         Initialize the RAM-Test

@note                This function initialize the RAM-Test. The RAM-Test is
                     set to the first address to be tested and the variable
                     that counts the RAM-Test cycles is set to their initial
                     value. Furthermore the state machine for the RAM-Test steps
                     is set to the first step.

@Parameter
@return    void
 */
/******************************************************************************/
void ramtst_InitRamTest (void)
{
   /* initialize the RDS-Variables for the RAM-Test */
   RDS_SET(ramtst_u32RdsOcBlkAddr, RAMTST_RAM_START)
   RDS_SET(ramtst_u32RdsBcAddr, RAMTST_RAM_START)

   /* delete the RAM-Test cycles */
   ramtst_u32RamTestCycles = 0uL;

   /* set the RAM-Test to the first step */
   ramtst_eTestStep = RAMTST_E_STEP1;
}


/******************************************************************************/
/*!
@Name                ramtst_DoRamTest

@Description         Test-Manager for the RAM-Test

@note                This function is the state machine for the RAM-Test.
                     The RAM-Test is split into two steps (see [SRS_695]) which are
                     handled with this state machine.
                     For further information how the RAM-Test is working see the
                     respective function headers.
                     Before this function is used, the InitRamTest function must
                     be called. If this is not the case, the
                     Safety-Fail-Function is called.

@Parameter
 @return             eRet        STDEF_RET_BUSY - RAM-Test in process
                                 STDEF_RET_DONE - RAM-Test successful done
 */
/******************************************************************************/
STDEF_RETCODE_ENUM ramtst_DoRamTest (void)
{
   STDEF_RETCODE_ENUM eRet = STDEF_RET_ERR;

   /* evaluate the test step, see [SRS_695] */
   switch (ramtst_eTestStep)
   {
      case RAMTST_E_STEP1:
      {
         /* do the RAM test step 1 */
         eRet = ramtst_DoRamTestStep1();

         STDEF_RETCODE_SAFETY_ASSERT_RET(eRet, STDEF_RET_ERR);

         if (eRet == STDEF_RET_DONE)
         {
            eRet = STDEF_RET_BUSY;

            /* next RAM-Test is Step 2 */
            ramtst_eTestStep = RAMTST_E_STEP2;
         }

         break;
      }
      case RAMTST_E_STEP2:
      {
         /* do the RAM test step 2 */
         eRet = ramtst_DoRamTestStep2();

         STDEF_RETCODE_SAFETY_ASSERT_RET(eRet, STDEF_RET_ERR);

         if (eRet == STDEF_RET_DONE)
         {
            /*  set signature to logical program flow control, see [SRS_697] */
            pflow_AddSignature(PFLOW_E_SIG_RAM_TEST);

            /* next RAM-Test is Step 1 */
            ramtst_eTestStep = RAMTST_E_STEP1;
         }

         break;
      }
      case RAMTST_E_NOINIT:
      {
         GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_MODULE_NOINIT, STDEF_RET_ERR);
         break;
      }
      default:
      {
         GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_VARIABLE_ERR, STDEF_RET_ERR);
         break;
      }
   }

   return eRet;
}


/******************************************************************************/
/*!
@Name                ramtst_DoStackTest

@Description         Stack-Test

@note                This function executes the Stack-Test. The intention of
                     the Stack-Test is to detect an overflow or an underflow of
                     the Stack.
                     Above and below the Stack a unique signature is defined,
                     which is tested against corruption during the Stack-Test.
                     If a corruption of the signature is detected, the
                     Safety-Fail-Function is called.

@Parameter
 @return             void
 */
/******************************************************************************/
void ramtst_DoStackTest (void)
{
   UINT32 u32Index;
   UINT8* pu8StackOverflowAddr;
   UINT8* pu8StackUnderflowAddr;

   /* get the under- and overflow addresses */

   /* A cast to a pointer is OK, because in the following steps a access to the RAM is desired. */
   pu8StackOverflowAddr = (UINT8*) RAMTST_STACK_OVFLW_START; /*lint !e923*/
   pu8StackUnderflowAddr = (UINT8*) RAMTST_STACK_UNDFLW_START;/*lint !e923*/

   /* This FIT test manipulates the STACK overflow area. One byte of the
    * overflow is set to another value. An overflow is simulated. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_STACK_OVFLW_TEST,
                   (*(pu8StackOverflowAddr + (RAMTST_STACK_MONITORING_SIZE / 2u)) = 0x27u));

   /* This FIT test manipulates the STACK underflow area. One byte of the
    * underflow is set to another value. An underflow is simulated. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_STACK_UNDFLW_TEST,
                   (*(pu8StackUnderflowAddr + (RAMTST_STACK_MONITORING_SIZE / 2u)) = 0x27u));

   for (u32Index = 0u; u32Index < RAMTST_STACK_MONITORING_SIZE; u32Index++)
   {
      /* check the stack overflow area for a valid signature, see [SRS_449] */
      if (RAMTEST_STACK_TEST_PATTERN != *pu8StackOverflowAddr)
      {
         /* FS state entry in case of an error, see [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_STACK_TEST);
      }
      /* Increment of pointer is desired to get the next RAM-address. */
      pu8StackOverflowAddr++; /*lint !e960*/

      /* check the stack underflow area for a valid signature, see [SRS_449] */
      if (RAMTEST_STACK_TEST_PATTERN != *pu8StackUnderflowAddr)
      {
         /* FS state entry in case of an error, see [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_STACK_TEST);
      }
      /* Increment of pointer is desired to get the next RAM-address. */
      pu8StackUnderflowAddr++; /*lint !e960*/
   }

   /* set signature to logical program flow control, see [SRS_697] */
   pflow_AddSignature(PFLOW_E_SIG_STACK_TEST);


}

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

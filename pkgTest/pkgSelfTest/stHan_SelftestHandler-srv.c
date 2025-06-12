/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** stHan_SelftestHandler-srv.c
**
** $Id: stHan_SelftestHandler-srv.c 4134 2023-12-06 08:59:37Z ankr $
** $Revision: 4134 $
** $Date: 2023-12-06 09:59:37 +0100 (Mi, 06 Dez 2023) $
** $Author: ankr $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** This module contains the selftest handler.
** The selftest handler executes consecutively the following selftests:
** 1. Safety-Container Check
** 2. ROM-Test
** 3. STACK-Test
** 4. CPU-Test
**    a. Opcode Test
**    b. SFR Test
**    c. CoreRegister Test
** 5. RAM-Test
** 6. Program-Flow Check of the selftest handler
**
** To execute the selftests the DoSelfTest-function has to be called.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

/*******************************************************************************
**
** includes
**
********************************************************************************
*/
/* system includes */
#ifdef GLOBFIT_FITTEST_ACTIVE
#include "stm32f10x.h"
#endif
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"
#include "cfg_config-sys.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "globFit_FitTestHandler.h"
#include "stDef_SelftestDefinitions.h"
#include "romtst_RomTest.h"
#include "ramtst_RamTest.h"
#include "cputst_CpuTest.h"
#include "safcon_SafetyContainer.h"
#include "pflow_ProgFlowCtrl.h"
#include "timer-def.h"
#include "timer-hal.h"

/* Header-file of module */
#include "stHan_SelftestHandler-srv.h"
#include "stHan_SelftestHandler-srv_loc.h"

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*! This variable contains the states of the selftest handler. */
STATIC STHAN_SELF_TEST_TASK_ENUM stHan_eSelfTestTask;

/*! This variable counts the number of function calls of the stHan_DoSelfTests function. */
STATIC RDS_UINT32 stHan_u32RdsFktCallsSelfTest;

/*! This variable contains the time stamp of the last successful selftest duration. */
STATIC RDS_UINT32 stHan_u32RdsSelfTestTriggerTime;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** stHan_DoSelfTests()
**
** Description:
** Selftest Handler
**
** The selftest handler executes consecutively the following selftests (see
** [SRS_667]) :
** 1. Safety-Container Check
** 2. ROM-Test
** 3. STACK-Test
** 4. CPU-Test
**    a. Opcode Test
**    b. SFR Test
**    c. CoreRegister Test
** 5. RAM-Test
** 6. Program-Flow Check
**
** In case of an error, the error-handler is called directly.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    safcon_CheckCrc()
**    romtst_DoRomTest()
**    ramtst_DoStackTest()
**    cputst_DoCpuTests()
**    ramtst_DoRamTest()
**    pflow_CheckSignature()
**    stHan_TriggerSelfTestTimeout()
**
** Module Test: - YES -
**      Reason: -
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void stHan_DoSelfTests (void)
{
   STDEF_RETCODE_ENUM eRet = STDEF_RET_ERR;

   RDS_INC(stHan_u32RdsFktCallsSelfTest);

   /* This FIT enables the clock of a periphery, this simulates a unwanted
    * activating of a periphery through an error. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_SFR_TEST, RCC->APB2ENR |= RCC_APB2ENR_SPI1EN);

   /* FIT to manipulate the number of test cycles*/
   GLOBFIT_FITTEST( GLOBFIT_CMD_SELFTEST_LPC3,
                    RDS_SET( stHan_u32RdsFktCallsSelfTest,
                           ( STHANLOC_NUMBER_OF_TESTCYCLES + 1 ) ) );

   /* check number of test cycles*/
   GLOBFAIL_SAFETY_ASSERT( ( STHANLOC_NUMBER_OF_TESTCYCLES 
                               >= RDS_GET( stHan_u32RdsFktCallsSelfTest ) ),
                             GLOB_FAILCODE_STHAN_TIMEOUT);/*lint !e948*/
   /* TSL:
    * Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* FIT to simulate a bit-flip in a variable which is secured by a hamming distance */
   /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
   GLOBFIT_FITTEST(GLOBFIT_CMD_HAMMING_BITFLIP,
                   stHan_eSelfTestTask ^= 1u);
   /* RSM_IGNORE_QUALITY_END */

   /* execute consecutively the specified selftests */
   switch (stHan_eSelfTestTask)
   {
      case STHAN_SAFECON_TEST_ENUM:
      {
         /* cyclic call to check safety data / safety container, see [SRS_464] */
         safcon_CheckCrc();

         eRet = STDEF_RET_DONE;

         /* Safety-Container Test finished successful, set next test */
         stHan_eSelfTestTask = STHAN_ROM_TEST_ENUM;

         break;
      }
      case STHAN_ROM_TEST_ENUM:
      {
         /* see [SRS_690] */
         eRet = romtst_DoRomTest();

         STDEF_RETCODE_SAFETY_ASSERT(eRet);
         /* ROM test needs more than 1 cycle to finish, see [SRS_695] */
         if (STDEF_RET_DONE == eRet)
         {
            /* ROM-Test finished successful, set next test */
            stHan_eSelfTestTask = STHAN_STACK_TEST_ENUM;
         }

         break;
      }
      case STHAN_STACK_TEST_ENUM:
      {
         /* call cyclic stack test, see [SRS_450] */
         ramtst_DoStackTest(); /* --- Stack test */

         eRet = STDEF_RET_DONE;

         /* STACK-Test finished successful, set next test */
         stHan_eSelfTestTask = STHAN_CPU_TEST_ENUM;
         break;
      }
      case STHAN_CPU_TEST_ENUM:
      {
         eRet = cputst_DoCpuTests();

         STDEF_RETCODE_SAFETY_ASSERT(eRet);
         /* CPU test (containing Opcode-, Core- and SFR-Test) needs more than
          * 1 cycle to finish, see [SRS_695] */
         if (STDEF_RET_DONE == eRet)
         {
            /* CPU-Test finished successful, set next test */
            stHan_eSelfTestTask = STHAN_RAM_TEST_ENUM;

            /* This FIT test manipulates the logical program flow. The RAM test
             * is skipped. */
            GLOBFIT_FITTEST(GLOBFIT_CMD_SELFTEST_LPC1, 
                            (stHan_eSelfTestTask = STHAN_CHECK_PFLOW_ENUM));
         }

         break;
      }
      case STHAN_RAM_TEST_ENUM:
      {
         /* calling the cyclic RAM Test, see [SRS_46]  */
         eRet = ramtst_DoRamTest();
         STDEF_RETCODE_SAFETY_ASSERT(eRet);
         /* RAM test needs more than 1 cycle to finish, see [SRS_695] */
         if (STDEF_RET_DONE == eRet)
         {
            /* RAM-Test finished successful, set next test */
            stHan_eSelfTestTask = STHAN_CHECK_PFLOW_ENUM;
         }

         break;
      }
      case STHAN_CHECK_PFLOW_ENUM:
      {
         /* All selftests are completed, check the signature, see [SRS_698],
          * [SRS_697] */
         pflow_CheckSignature(PFLOW_SIGNATURE_SELFTEST);

         /* FIT to manipulate the number of test cycles*/
         GLOBFIT_FITTEST( GLOBFIT_CMD_SELFTEST_LPC2,
                          RDS_SET( stHan_u32RdsFktCallsSelfTest,
                             (STHANLOC_NUMBER_OF_TESTCYCLES - 1u) ) );

         /* check number of test cycles, enter FS state in case of an error,
          * see [SRS_359] */
         GLOBFAIL_SAFETY_ASSERT((STHANLOC_NUMBER_OF_TESTCYCLES 
                                   == RDS_GET(stHan_u32RdsFktCallsSelfTest)),
                                GLOB_FAILCODE_PFLOW_SIGNATURE);/*lint !e948*/
         /* TSL:
          * Note 948: Operator '==' always evaluates to False
          * Problem is in RDS_GET. */

         RDS_SET(stHan_u32RdsFktCallsSelfTest, 0u);

         /* Trigger timeout for the selftests, see [SRS_353] */
         stHan_TriggerSelfTestTimeout();

         eRet = STDEF_RET_DONE;

         /* continue with the safety container test */
         stHan_eSelfTestTask = STHAN_SAFECON_TEST_ENUM;

         break;
      }
      default:
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;
      }
   }

   STDEF_RETCODE_SAFETY_ASSERT(eRet);
}


/*------------------------------------------------------------------------------
**
** stHan_InitSelfTests()
**
** Description:
** This function initialize the selftests. After this the startup tests will
** be executed.
**
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    timerHAL_GetSystemTime3()
**    romtst_InitRomTest()
**    ramtst_InitRamTest()
**    cputst_InitCpuTests()
**    safcon_Init()
**    pflow_Init()
**    cputst_StartupCpuTests()
**
** Module Test: - NO -
**      Reason: low complexity
**
** Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void stHan_InitSelfTests (void)
{
   UINT32 u32SysTime;

   /* start the selftests with the Safety Container Test */
   stHan_eSelfTestTask = STHAN_SAFECON_TEST_ENUM;

   /* clear the call counter of the selftests */
   RDS_SET( stHan_u32RdsFktCallsSelfTest, 0u );

   /* initialize the selftests */
   romtst_InitRomTest();
   ramtst_InitRamTest();
   cputst_InitCpuTests();
   safcon_Init();
   pflow_Init();

   /* Do the startup tests, see [SRS_689] */
   romtst_DoStartupRomTest();
   cputst_StartupCpuTests();

   /* write the current system time to the "Self-Test trigger time" */
   u32SysTime = timerHAL_GetSystemTime3();
   RDS_SET(stHan_u32RdsSelfTestTriggerTime, u32SysTime);
}

/*------------------------------------------------------------------------------
 **
 ** stHan_CheckSelfTestTimeout()
 **
 ** This function checks if a selftest timeout is occurred (covers part of
 ** [SRS_690], [SRS_46], [SRS_692], [SRS_693], [SRS_450], [SRS_696], [SRS_464]).
 ** The function fetches the current system time and the last trigger time of a
 ** successful selftest duration. All interrupts are disabled when these values
 ** are fetched.
 ** These time stamps are used for the timeout calculation. The time of the last
 ** selftest trigger is subtracted from the current system time to get the time
 ** difference. If this time is greater than the timeout time the Safety Handler
 ** will be called.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **  void
 **
 ** Return:
 **  void
 **
 ** Usage:
 **  timerHAL_GetSystemTime3
 **
 ** Module Test: - NO -
 **      Reason: low complexity
 **
 ** Context: main, while(FOREVER)
 **-----------------------------------------------------------------------------
 */
void stHan_CheckSelfTestTimeout (void)
{
   UINT32 u32DiffTime;
   UINT32 u32SysTime;

   __disable_irq();

   /* This FIT toggle one bit in the raw value of the UNIT32 data, this simulates
    * a softerror. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RDS_UINT32, stHan_u32RdsSelfTestTriggerTime.val.val ^= 0x10u);

   u32DiffTime = RDS_GET(stHan_u32RdsSelfTestTriggerTime);
   u32SysTime = timerHAL_GetSystemTime3();

   __enable_irq();

   u32DiffTime = u32SysTime - u32DiffTime;

   /* check if the time difference between "stHan_u32RdsSelfTestTriggerTime" and
    * current time exceeds "STHANLOC_TIMEOUT", see [SRS_665] */
   GLOBFAIL_SAFETY_ASSERT(u32DiffTime < STHANLOC_TIMEOUT, GLOB_FAILCODE_STHAN_TIMEOUT);
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/
/*------------------------------------------------------------------------------
 **
 ** stHan_TriggerSelfTestTimeout()
 **
 ** This function triggers the selftest timeout.
 ** The function fetches the current system time and writes it to the variable
 ** "SelfTestTriggerTime" which is used for the timeout calculation.
 **
 **-----------------------------------------------------------------------------
 ** Input:
 **  void
 **
 ** Return:
 **  void
 **
 ** Usage:
 **  timerHAL_GetSystemTime3
 **
 ** Module Test: - NO -
 **      Reason: low complexity
 **
 ** Context: IRQ Scheduler
 **-----------------------------------------------------------------------------
 */
STATIC void stHan_TriggerSelfTestTimeout (void)
{
   UINT32 u32SysTime;

   u32SysTime = timerHAL_GetSystemTime3();
   /* store current system time, see [SRS_353] */
   RDS_SET(stHan_u32RdsSelfTestTriggerTime, u32SysTime);
}


/*******************************************************************************
**
** End of stHan_SelftestHandler-srv.c
**
********************************************************************************
*/

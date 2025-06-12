/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: cputst_CpuTest.c 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          cputst_CpuTest.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief         This module contains the CPU-Test functions.
* @description:
* This module provides the following CPU-Tests for the runtime:
* - OpCode Test
* - Special Function Register Test
* - Core-Register Test
* The tests are executed consecutively by calling the DoCpuTests-function.
*
* Furthermore a Startup-CPU-Test is provided.
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
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "stDef_SelftestDefinitions.h"
#include "pflow_ProgFlowCtrl.h"
#include "cfg_Config-sys.h"

#include "SFRCRC_typ.h"
#include "SfrTest_cfg.h"
#include "SfrTest.h"

#include "globFit_FitTestHandler.h"

/* Header-file of module */
#include "cputst_Cputest.h"
#include "cputst_Cputest_loc.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/


/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! This variable contains the states for the CPU-Test state machine. */
STATIC CPUTEST_E_TASK cputst_eCpuTask;

/*! This array contains the specified SFR-Tests for each controller. */
STATIC SAFEINDEX cputst_auSfrTstList[CPUTEST_MAXNUMOF_SFRTEST_CFGS];

/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

/***** End of: globvar Moduleglobal Variables ***************************//*@}*/


/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/
#if SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE
STATIC void cputst_CalcParamSfrCrc (SAFEINDEX si);
#endif
STATIC STDEF_RETCODE_ENUM cputst_DoSfrTest (void);
STATIC void cputst_DoOpCodeTest (void);
STATIC STDEF_RETCODE_ENUM cputst_DoCoreRegTest (void);


/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Local Functions                                                            */
/********************************//*!@addtogroup locfunc Local Functions*//*@{*/

#if SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE
/******************************************************************************/
/*!
@Name                cputst_CalcParamSfrCrc

@Description         This function initialize the parameter SFR-Test and
                     calculates the base CRC.

@note                -

@Parameter
   @param[in]        SAFEINDEX   Safeindex of the SFR-Test config array.
   @return           void
 */
/******************************************************************************/
STATIC void cputst_CalcParamSfrCrc (SAFEINDEX si)
{
   SFRTEST_STATE_ENUM eSfrTestRet;

   /* initialize the param SFR-Test */
   sfrTest_InitReference(si);

   do
   {
      /* calculate the base CRC */
      eSfrTestRet = sfrTest_Execute(si);
      if (  !((eSfrTestRet == SFRTEST_STATE_REFDONE)
         || (eSfrTestRet == SFRTEST_STATE_SAMPLINGREF)))
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }
   while( eSfrTestRet != SFRTEST_STATE_REFDONE);
   #warning "insert timeout in while loop when using parameter SFR-Test"

   sfrTest_ConfirmReference(si);
   sfrTest_Reset(si);
}
#endif /* SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE */

/******************************************************************************/
/*!
@Name                cputst_DoSfrTest

@Description         SFR-Test

@note                This function executes the SFR-Test.

The purpose of the test is to find failures, e.g. Stuck-at, in the registers of
the Cortex. The SFR-Test calculates a checksum over the constant register values
and check it against a expected checksum.
If the checksums are not equal, the safety-handler will be called.

The expected register values are defined in the file
SFR-Map_HMS_ASM.xls.

To calculate the checksum over the constant registers the HW-CRC unit is used.

@Parameter
   @return           STDEF_RETCODE_ENUM
                        STDEF_RET_DONE - SFR-Test is successful
                        STDEF_RET_BUSY - SFR-Test is under progress
                        STDEF_RET_ERR  - SFR-Test error. If this error occur
                                         the Safety-Handler will be called.
                                         No return value.
 */
/******************************************************************************/
STATIC STDEF_RETCODE_ENUM cputst_DoSfrTest (void)
{
   SFRTEST_STATE_ENUM eSfrTestRet;
   STDEF_RETCODE_ENUM eRet;
   /*! Index for the SFR-Test list. */
   LOCAL_STATIC( , RDS_UINT8, u8RdsSfrTstLstIdxCnt, CPUTEST_SFRTSTLSTIDXCNT_INIT);/*lint !e708 */
   /* Deactivated Lint message 708, because a Union initialisation is wanted.
    * The macro LOCAL_STATIC is used for unit test instrumentation. */

   /* This FIT toggle one bit in the raw value of the UNIT8 data, this simulates
    * a softerror. */
   GLOBFIT_FITTEST(GLOBFIT_CMD_RDS_UINT8, u8RdsSfrTstLstIdxCnt.inv.raw ^= 0x01u);

   /* execute SFR-Test to calculate the CRC */
   eSfrTestRet = sfrTest_Execute(cputst_auSfrTstList[RDS_GET(u8RdsSfrTstLstIdxCnt)]);/*lint !e948*/
   /* Note 948: Operator '==' always evaluates to False
    * Problem is in RDS_GET. */

   /* check the SFR-Test return value */
   if (eSfrTestRet == SFRTEST_STATE_DONE)
   {/* SFR-Test is done successful */

      /* reset actual SFR-Test for the next cycle */
      sfrTest_Reset(cputst_auSfrTstList[RDS_GET(u8RdsSfrTstLstIdxCnt)]);/*lint !e948*/

      /* increment index counter to set to the next SFR-Test */
      RDS_INC(u8RdsSfrTstLstIdxCnt);

      if (RDS_GET(u8RdsSfrTstLstIdxCnt) == CPUTEST_MAXNUMOF_SFRTEST_CFGS)/*lint !e948*/
      {
         eRet = STDEF_RET_DONE;
         RDS_SET(u8RdsSfrTstLstIdxCnt, 0x00u);
      }
      else
      {
         eRet = STDEF_RET_BUSY;
      }
   }
   else if (eSfrTestRet == SFRTEST_STATE_SAMPLING)
   {
      eRet = STDEF_RET_BUSY;
   }
   else
   {
      eRet = STDEF_RET_ERR;
      GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_SFR_TEST, eRet);
   }

   return eRet;
}


/******************************************************************************/
/*!
@Name                cputst_DoOpCodeTest

@Description         Opcode-Test

@note
This function executes the Opcode-Test.

The purpose of the test is to check the opcode of the Cortex to find failures
in the arithmetic logic unit (ALU) and in the Instruction Decoder of the CPU.
The opcodes which are tested are defined in the Opcode-Test file.


@Parameter
   @return           void
 */
/******************************************************************************/
STATIC void cputst_DoOpCodeTest (void)
{
   /* call the Opcode-Test in the assembler file. */
   asm_OpcodeTest();

}


/******************************************************************************/
/*!
@Name                cputst_DoCoreRegTest

@Description         CoreRegister-Test

@note
This function executes the coreRegister-Test in two steps.

The purpose of the test is to check the core registers of the Cortex to find
the following failures:
 - Transition faults (TF)
 - Coupling faults (CF)
 - Stuck-at faults (SAF)
 - Address decoder faults (AF)

In case of an error the safety handler will be called directly.


@Parameter
   @return           STDEF_RETCODE_ENUM
                        STDEF_RET_BUSY: The core register test is still busy
                        STDEF_RET_DONE: The core register test is finished
                                        without errors.
                        STDEF_RET_ERR:  An error in the core register test was
                                        detected.
 */
/******************************************************************************/
STATIC STDEF_RETCODE_ENUM cputst_DoCoreRegTest (void)
{
   LOCAL_STATIC(, CPUTEST_E_COREREGTEST_STEPS, eTestStep, CPUTEST_E_COREREGTEST_STEP1);
   STDEF_RETCODE_ENUM eRet = STDEF_RET_ERR;

   switch(eTestStep)
   {
      case CPUTEST_E_COREREGTEST_STEP1:
      {
         asm_CoreRegTestStep1();

         eTestStep = CPUTEST_E_COREREGTEST_STEP2;
         eRet = STDEF_RET_BUSY;

         break;
      }
      case CPUTEST_E_COREREGTEST_STEP2:
      {
         asm_CoreRegTestStep2();

         eTestStep = CPUTEST_E_COREREGTEST_STEP1;
         eRet = STDEF_RET_DONE;

         break;
      }
      default:
      {
         GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_VARIABLE_ERR, eRet);
         break;
      }
   }

   return eRet;
}


/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/


/******************************************************************************/
/*!
@Name                cputst_DoCpuTests

@Description         CPU-Test-Manager

@note
This function manages the CPU-Tests. These are:
 - Opcode Test
 - Core-Register Test
 - Special-Function-Register Test

The various tests are executed step by step. During this, the function returns
"BUSY". If each test is executed successfully the function returns "OK". If one
test is erroneous the Safety-Handler is called directly.
The OpCode Test and the CoreRegister Test are executed by one test cycle, the
SFR-Test needs more than one test cycle.

@Parameter
   @return     eRet     STDEF_RET_DONE - no errors
                        STDEF_RET_BUSY - selftest is still in progress
                        STDEF_RET_ERR  - an error occurred
 */
/******************************************************************************/
STDEF_RETCODE_ENUM cputst_DoCpuTests (void)
{
   STDEF_RETCODE_ENUM eRet = STDEF_RET_ERR;

   /* execute consecutively the specified cpu-tests, see [SRS_695] */
   switch (cputst_eCpuTask)
   {
      case CPUTEST_E_OPCODE_TEST:
      {
         /* deactivate irqs during opcode-test */
         __disable_irq();
         /* cyclic call to opcode test, see [SRS_696] */
         cputst_DoOpCodeTest(); /* time period ca. 20us */

         __enable_irq();

         eRet = STDEF_RET_BUSY;

         /* set to next test */
         cputst_eCpuTask = CPUTEST_E_SFR_TEST;

         /* set signature to logical program flow control, see [SRS_697] */
         pflow_AddSignature(PFLOW_E_SIG_OPCODE_TEST);
         break;
      }
      case CPUTEST_E_SFR_TEST:
      {
         /* call to SFR test, see [SRS_693] */
         eRet = cputst_DoSfrTest(); /* time period max. ca. 340us */

         if (eRet == STDEF_RET_DONE)
         {
            eRet = STDEF_RET_BUSY;

            /* set to next test */
            cputst_eCpuTask = CPUTEST_E_CORE_TEST;

            /* set signature to logical program flow control, see [SRS_697] */
            pflow_AddSignature(PFLOW_E_SIG_SFR_TEST);
         }
         break;
      }
      case CPUTEST_E_CORE_TEST:
      {

         /* disable irqs during corereg-test */
         __disable_irq();
         /* call to core register test, see [SRS_692] */
         eRet = cputst_DoCoreRegTest();

         __enable_irq();

         if (eRet == STDEF_RET_DONE)
         {
            /* set to next test step */
            cputst_eCpuTask = CPUTEST_E_OPCODE_TEST;

            /* set signature to logical program flow control, see [SRS_697] */
            pflow_AddSignature(PFLOW_E_SIG_CORE_TEST);

         }

         break;
      }
      default:
      {
         GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_VARIABLE_ERR, STDEF_RET_ERR);
         break;
      }
   }

   GLOBFAIL_SAFETY_ASSERT_RET((eRet == STDEF_RET_BUSY) || (eRet == STDEF_RET_DONE),
                              GLOB_FAILCODE_VARIABLE_ERR,
                              STDEF_RET_ERR)

   return (eRet);
}


/******************************************************************************/
/*!
@Name                cputst_StartupCpuTests

@Description         CPU-Startup-Test manager

@note
This function executes the specified CPU-Tests at startup. These are:
 - Opcode Test
 - Core Register Test

@Parameter
   @return           void
 */
/******************************************************************************/
void cputst_StartupCpuTests (void)
{
   /* execute the Opcode Test, see [SRS_454] */
   asm_OpcodeTest();

   /* execute the Core Register Test, see [SRS_691] */
   asm_CoreRegTestStep1();
   asm_CoreRegTestStep2();
}


/******************************************************************************/
/*!
@Name                cputst_InitCpuTests

@Description         Initialize the CPU-Tests

@note
This function initializes the CPU-tests.

@Parameter
   @return           void
 */
/******************************************************************************/
void cputst_InitCpuTests (void)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* start with the Opcode test */
   cputst_eCpuTask = CPUTEST_E_OPCODE_TEST;

   /* initialize the SFR-Test for the param and config SFR-Test for each
    * controller */
   eControllerID = cfgSYS_GetControllerID();
   if (eControllerID == SAFETY_CONTROLLER_1)
   {
      /* Deactivated lint message 835 because safe index 0 is made up with
       * 0 and ~0. */

      /* initialize config sfr for uC1 */
      sfrTest_Reset(SFRTEST_SAFEINDEX_0);/*lint !e835*/

      /* set the specified SFR-Tests for uC1 to the list */
      cputst_auSfrTstList[0] = SFRTEST_SAFEINDEX_0;/*lint !e835*/
   }
   else if (eControllerID == SAFETY_CONTROLLER_2)
   {
      /* initialize config sfr for uC2 */
      sfrTest_Reset(SFRTEST_SAFEINDEX_1);

      /* set the specified SFR-Tests for uC2 to the list */
      cputst_auSfrTstList[0] = SFRTEST_SAFEINDEX_1;
   }
   else
   {
      /* wrong controller ID */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
   }

}

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

/******************************************************************************/
/*!
 *******************************************************************************
 *       $Id: asm_OpcodeTest.s 4436 2024-05-17 14:28:50Z ankr $
 * $Revision: 4436 $
 *     $Date: 2024-05-17 16:28:50 +0200 (Fr, 17 Mai 2024) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          asm_OpcodeTest.s
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Opcode Test
 * @description:
 * This modul containes the Opcode Test.
 */
/******************************************************************************/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#define __ASM__
   #include "globFail_Errorcodes.h"
   #include "globFail_SafetyHandler.h"
   #include "stDef_SelftestDefinitions.h"
   #include "ramtst_RamTest_cfg.h"
   #include "globFit_FitTestHandler.h"
#undef __ASM__

#ifdef GLOBFIT_FITTEST_ACTIVE
#warning FITs in asm_OpCodeTest.s active!!!
#endif

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

/******************************************************************************/
/* Assembler Start                                                            */
/******************************************************************************/
   PRESERVE8
   AREA    OPCODETEST, CODE, READONLY


/******************************************************************************/
/* Assembler Imports / Exports                                                */
/******************************************************************************/

   EXPORT asm_OpcodeTest

   IMPORT globFail_SafetyHandler

#ifdef GLOBFIT_FITTEST_ACTIVE
   IMPORT globFit_u8FitCommand
#endif
/******************************************************************************/
/*!
@Name                asm_OpcodeTest

@Description         Opcode Test (see [SRS_453])

@Note
In case of an error, the safety-Handler will be called.

Register summary:
 R2 - Working Register
 R3 - Working Register
 R4 - Register with test-pattern
 R5 - Register with test-pattern
 R6 - Register with test-pattern
 R7 - Working Register
 R8 - Register defined with 0, it is used to clear the flags in APSR

@Parameter
   @return           void
 */
/******************************************************************************/
asm_OpcodeTest

   PUSH  {R2-R8}                 /* push / store Register content on the STACK */

   MOV   R8, #0                  /* prepare registers for test */

/*******************************************************************************
* This section tests 3 opcodes of the group "Memory access instructions"
* defined in "Cortex-M3 Devices - Generic User Guide", chapter 3.4.
*
* The opcodes which are tested are LDR, STR, PUSH and POP.
*******************************************************************************/
   MOVW  R4, #0x4321                /* Load test-pattern to R4 */
   MOVT  R4, #0x8765                /* Load test-pattern to R4 */
   MOV   R2, #RAMTST_RAM_START_ASM  /* Load RAM_START to R2 */
   MOV   R5, #0                     /* clear R5 */

   ADD   R2, R2, #RAMTST_STACK_SIZE_ASM     /* calculate the RAM_START_DATA */
   ADD   R2, R2, #RAMTST_STACK_MONITORING_SIZE_ASM
   ADD   R2, R2, #RAMTST_STACK_MONITORING_SIZE_ASM

   LDR   R7, [R2]                /* load the RAM-Value of the RAM_START_DATA into R7, save the value for this test */

/*--- Test of the opcode LDR & STR -------------------------------------------*/
   STR   R4, [R2]                /* store test-pattern to RAM */
   LDR   R5, [R2]                /* load test-pattern from RAM */
_FitTestLdrStr GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_LDR_STR, R3, MOV, R5, R7 /* This FIT Test manipulates the LDR and STR test. */
   CMP   R5, R4                  /* check if stored test-pattern equals the loaded */
   BNE.W _Opcode_Test_Failed     /* if registers not equal, goto safety handler */

   STR   R7, [R2]                /* restore RAM value */

/*--- Test of the opcode PUSH & POP ------------------------------------------*/
   PUSH  {R4}                    /* store/push test-pattern to STACK */
   MOV   R4, #0                  /* clear R4 */
   POP   {R4}                    /* load/pop test-pattern from STACK */
_FitTestPushPop GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_PUSH_POP, R3, MOV, R4, #0 /* This FIT Test manipulates the PUSH and POP test. */
   CMP   R5, R4                  /* check if stored test-pattern equals the loaded */
   BNE.W _Opcode_Test_Failed     /* if registers not equal, goto safety handler */


/*******************************************************************************
* This section tests 3 opcodes of the group "General data processing
* instructions" defined in "Cortex-M3 Devices - Generic User Guide",
* chapter 3.5.
*
* The opcodes which are tested are MOV, ADD, LSR.
*******************************************************************************/
/*--- Test of the opcode MOV -------------------------------------------------*/
   MOVW  R4, #0xAA55             /* prepare registers for test */
   MOV   R6, #0xFFFFFFFF         /* prepare registers for test */
   MSR   APSR, R8                /* clear all Flags in the Application Programm Status Register */

   MOVW  R5, #0xAA55             /* write test-pattern 0xAA55 to R4 */
_FitTestMov1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MOV1, R3, MOV, R5, #0 /* This FIT Test manipulates the MOV test. */
   CMP   R4, R5                  /* check R4 against R5 if the test-pattern is equal, this operation clears the N-Flag */
   BNE.W _Opcode_Test_Failed     /* if registers not equal, goto safety handler */

_FitTestMov2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MOV2, R3, MOV, R6, #0x7FFFFFFF /* This FIT Test manipulates the MOV test. */
   MSR   APSR, R8                /* clear all flags */
   MOVS  R5, R6                  /* move test-pattern from R6 to R5, operation set N-Flag (negative) */
   BPL.W _Opcode_Test_Failed     /* check if N-flag is set, if not jump to safety handler */
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */

   MOV R2, #0
_FitTestMov3 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MOV3, R3, MOV, R2, #1 /* This FIT Test manipulates the MOV test. */
   MSR   APSR, R8                /* clear all flags */
   MOVS  R5, R2                  /* write zero to R5, this operation set the Z-Flag and clear the N-Flag */
   BNE.W _Opcode_Test_Failed     /* chek if Z-Flag is set, if not jump to safety handler */
   BMI.W _Opcode_Test_Failed     /* if N-Flag is still set, jump to safety handler */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */

   MOV   R4, #0x01
_FitTestMov4 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MOV4, R3, MOV, R4, #2 /* This FIT Test manipulates the MOV test. */
   MSR   APSR, R8                /* clear all flags */
   MOVS  R4, R4, LSR #1          /* shift R4 one pos to the left and store it in R4, this operation sets the C-Flag */
   BCC.W _Opcode_Test_Failed     /* check if the C-Flag is set, if not jump to safety handler */
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */
   BMI.W _Opcode_Test_Failed     /* check N = 0 */


/*--- Test of the opcode ADD -------------------------------------------------*/
   MOV   R4, #0xAAAAAAAA         /* load the 1. test-pattern to R4 */
   MOV   R5, #0x55555555         /* load the 2. test-pattern to R5 */
_FitTestAdd1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_ADD1, R3, MOVT, R4, #0x2AAA /* This FIT Test manipulates the ADD test. */
   MSR   APSR, R8                /* clear all flags */

   ADDS  R4, R4, R5              /* add R4 with R5 and set status flags */
   BVS.W _Opcode_Test_Failed     /* check V = 0 */
   BPL.W _Opcode_Test_Failed     /* check N = 1 */
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */
   CMP   R4, R6                  /* check if R4 equals R6 */
   BNE.W _Opcode_Test_Failed     /* jump to safety handler if not equal */

   MSR   APSR, R8                /* clear all flags */
   MOV   R5, #1                  /* load the 2. test-pattern to R5 */
   ADDS  R4, R4, R5              /* add R4 with R5 and set status flags */
   BCC.W _Opcode_Test_Failed     /* check C = 1 */
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */
   BMI.W _Opcode_Test_Failed     /* check N = 0 */
   BVS.W _Opcode_Test_Failed     /* check V = 0 */
_FitTestAdd2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_ADD2, R3, MOV, R4, #0xFFFFFFFF /* This FIT Test manipulates the ADD test. */
   CMP.W R4, R8                  /* compare and branch on non-zero */
   BNE.W _Opcode_Test_Failed

   MSR   APSR, R8                /* clear all flags */
   MOV   R4, #0x7FFFFFFF         /* load the 1. test-pattern to R4 */
   ADDS  R4, R5, R4              /* add R4 with R5 and set status flags */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   BPL.W _Opcode_Test_Failed     /* check N = 1 */
   BVC.W _Opcode_Test_Failed     /* check V = 1 */
   MOV   R6, #0x80000000         /* load the expected result to R6 */
   CMP   R6, R4                  /* compare the expected result against the addion result */
   BNE.W _Opcode_Test_Failed     /* jump to safety handler if not equal */

   MSR   APSR, R8                /* clear all flags */
   ADDS  R4, R6, R4              /* add R4 with R5 and set status flags */
   BCC.W _Opcode_Test_Failed     /* check C = 1 */
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */
   BMI.W _Opcode_Test_Failed     /* check N = 0 */
   BVC.W _Opcode_Test_Failed     /* check V = 1 */
_FitTestAdd3 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_ADD3, R3, MOV, R4, R6 /* This FIT Test manipulates the ADD test. */
   CMP   R4, R8                  /* compare and branch on non-zero */
   BNE.W _Opcode_Test_Failed


/*--- Test of the opcode LSR -------------------------------------------------*/
   MOV   R4, #0xAAAAAAAA
   MOV   R5, #0x55555555         /* load the 2. test-pattern to R5 */
   MOV   R6, #1                  /* load the shirt value to R6 */

   MSR   APSR, R8                /* clear all flags */
   LSRS  R4, R4, R6              /* logical shift right */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
_FitTestLsr1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_LSR1, R3, MOV, R4, #0xAAAAAAAA /* This FIT Test manipulates the LSR test. */
   CMP   R4, R5                  /* compare the expected result against the addion result */
   BNE.W _Opcode_Test_Failed     /* jump to safety handler if not equal */

_FitTestLsr2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_LSR2, R3, MOV, R4, #0xAAAAAAAA /* This FIT Test manipulates the LSR test. */
   MOVW  R5, #0xAAAA             /* load new test-pattern to R5 */
   MOVT  R5, #0x2AAA
   LSRS  R4, R4, R6              /* logical shift right */
   BCC.W _Opcode_Test_Failed     /* check C = 1 */
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   CMP   R4, R5                  /* compare the expected result against the addion result */
   BNE.W _Opcode_Test_Failed     /* jump to safety handler if not equal */

   MOV   R6, #32                 /* load an new shift value to R6 */
   MSR   APSR, R8                /* clear all flags */
   LSRS  R4, R4, R6              /* logical shift right */
   BCS.W _Opcode_Test_Failed     /* check C = 0 */
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */
   CMP   R4, R8                  /* compare the expected result against the addion result */
   BNE.W _Opcode_Test_Failed     /* jump to safety handler if not equal */


/*******************************************************************************
* This section tests 2 opcodes of the group "Multiply and divide instructions"
*  defined in "Cortex-M3 Devices - Generic User Guide", Chapter 3.6.
*
* The opcodes which are tested are MUL, UDIV.
*******************************************************************************/
/*--- Test of the opcode MUL -------------------------------------------------*/
   MOVW  R4, #0x1234
   MOVW  R5, #0x5678
   MOVW  R6, #0x0060
   MOVT  R6, #0x0626
_FitTestMul1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MUL1, R3, MOV, R5, #0 /* This FIT Test manipulates the MUL test. */
   MSR   APSR, R8                /* clear all flags */

   MULS  R4, R5, R4
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   BMI.W _Opcode_Test_Failed     /* check N = 0 */
   CMP   R4, R6
   BNE.W _Opcode_Test_Failed

   MOVW  R4, #0x3333
   MOV   R5, #0x50000
   MOV   R6, #0
   MOVT  R6, #0xFFFF
_FitTestMul2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MUL2, R3, MOV, R4, #0x7333 /* This FIT Test manipulates the MUL test. */
   MSR   APSR, R8                /* clear all flags */

   MULS  R4, R5, R4
   BEQ.W _Opcode_Test_Failed     /* check Z = 0 */
   BPL.W _Opcode_Test_Failed     /* check N = 1 */
   CMP   R4, R6
   BNE.W _Opcode_Test_Failed

   MOV   R4, #0x00120000
   MOV   R5, #0x03400000
   MSR   APSR, R8                /* clear all flags */

   MULS  R4, R5, R4
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */
   BMI.W _Opcode_Test_Failed     /* check N = 0 */
   CMP   R4, R8
   BNE.W _Opcode_Test_Failed


/*--- Test of the opcode UDIV ------------------------------------------------*/
   MOVW  R4, #28000
   MOVW  R5, #400
   MOVW  R6, #70

_FitTestDiv1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_DIV1, R3, MOV, R5, #700 /* This FIT Test manipulates the DIV test. */

   UDIV  R4, R4, R5              /* R4 = R4/R5, the UDIV instruction do not change the flags. */
   CMP   R6, R4
   BNE.W _Opcode_Test_Failed

_FitTestDiv2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_DIV2, R3, MOV, R8, #1 /* This FIT Test manipulates the DIV test. */

   UDIV  R4, R4, R8              /* R4 = R4/R8, zero-division, the result of the division is 0 */
   CMP   R8, R4
   BNE.W _Opcode_Test_Failed

   MOVW  R4, #1200
   UDIV  R4, R5, R4              /* R4 = R5/R4, division is resulted in 0 */
   CMP   R8, R4
   BNE.W _Opcode_Test_Failed

   MOVW  R4, #3
   MOVW  R6, #133
   UDIV  R4, R5, R4              /* the result is rounded */
   CMP   R6, R4
   BNE.W _Opcode_Test_Failed


/*******************************************************************************
* This section tests 1 opcodes of the group "Saturating instructions"
* defined in "Cortex-M3 Devices - Generic User Guide", Chapter 3.7.
*
* The opcodes which are tested are USAT. This instruction do not affect the
* flags.
*******************************************************************************/
   MOVW  R2, #0xFFFF             /* expected value after saturation with greater value */
   MOVW  R4, #0x1234             /* value for saturation, value in range */
   MOV   R6, #0x08000000         /* expected value of APSR if Q-Flag is set */
   MSR   APSR, R8                /* clear all flags */

   USAT  R5, #16, R4             /* Saturating R4 */
   MRS   R7, APSR                /* read Status Reg to evaluate Q-Flag */
_FitTestSat1 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_SAT1, R3, MOV, R7, R6 /* This FIT Test manipulates the saturating test. */
   CMP   R7, R8                  /* check Q = 0 */
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */
   CMP   R4, R5                  /* check if result in R5 is equal to R4, no saturation */
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */

   MOV   R4, #0x10000            /* value for saturation, value greater than range */
   MSR   APSR, R8                /* clear all flags */
   USAT  R5, #16, R4             /* Saturating R4 */
   MRS   R7, APSR                /* read Status Reg to evaluate Q-Flag */
_FitTestSat2 GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_SAT2, R3, MOV, R7, #0 /* This FIT Test manipulates the saturating test. */
   CMP   R7, R6                  /* check Q = 1 */
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */
   CMP   R2, R5                  /* check if result in R5 is equal to expected value in R3, failed saturation */
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */


/*******************************************************************************
* This section tests 2 opcodes of the group "Bitfield instructions"
* defined in "Cortex-M3 Devices - Generic User Guide", Chapter 3.8.
*
* The opcodes which are tested are BFC and BFI. These instructions do not affect
* the flags.
*******************************************************************************/
   MOV   R4, #0xFFFFFFFF
   MOVW  R5, #0x00FF
   MOVT  R5, #0xFF00
   MOVW  R6, #0xF0FF
   MOVT  R6, #0xFF00

   BFC   R4, #8, #16             /* clear 16 bits at position 8  */
_FitTestBfc GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_BFC, R3, MOVT, R4, #0xFF80 /* This FIT Test manipulates the saturating test. */
   CMP   R4, R5
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */

   BFI   R4, R5, #12, #4         /* insert 4 bits at position 12 */
_FitTestBfi GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_BFI, R3, MOVT, R4, #0xFF01 /* This FIT Test manipulates the saturating test. */
   CMP   R4, R6
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */


/*******************************************************************************
* This section tests 3 opcodes of the group "Branch and control instructions"
* defined in "Cortex-M3 Devices - Generic User Guide", Chapter 3.9.
*
* The opcodes which are tested are B, BL and BX. These instructions do not
* affect the flags.
*******************************************************************************/
   MOV   R4, #0
   MOV   R5, #0x55555555

   B     _TestFunc               /* branch to the test function */
_B_Test
_FitTestB GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_B, R3, MOV, R4, #0 /* This FIT Test manipulates the branch test. */
   CMP   R4, R5                  /* check if the test function has load R5 to R4 */
   BEQ   _next_Teststep          /* branch to safety handler if not equal */

   MOV   R0, #GLOB_FAILCODE_OPCODE_TEST_ASM  /* return with error */
   LDR   R15, =globFail_SafetyHandler        /* load the address of the safety handler to the PC, branch command has an error, see SRS_359 */
   BX    lr                                  /* return */

_next_Teststep


/*******************************************************************************
* This section tests 2 opcodes of the group "Miscellaneous instructions"
* defined in "Cortex-M3 Devices - Generic User Guide", Chapter 3.10.
*
* The opcodes which are tested are MRS and MSR. These instructions do not
* affect the flags.
*******************************************************************************/
   MOV   R4, #0x40000000
   MSR   APSR, R8                /* clear all flags */

_FitTestMsr GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MSR, R3, MOV, R4, #0 /* This FIT Test manipulates the MSR test. */

   MSR   APSR, R4
   BNE.W _Opcode_Test_Failed     /* check Z = 1 */

   MSR   APSR, R8                /* clear all flags */
   MOVS  R5, #0xFFFFFFFF
   MOV   R6, #0x80000000

   MRS   R4, APSR
_FitTestMrs GLOBFIT_FitTest #GLOBFIT_CMD_OPCODE_TEST_MRS, R3, MOV, R4, #0 /* This FIT Test manipulates the MSR test. */
   CMP   R4, R6
   BNE.W _Opcode_Test_Failed     /* branch to safety handler if not equal */




/* opcode Test successful, return */
_Opcode_Test_Ok
   POP   {R2-R8}                 /* pop / restore Register content on the STACK */
   BX    lr                      /* return */


/* This test function loads R5 to R4 */
_TestFunc
   MOV   R4, R5
   B     _B_Test                 /* branch back to test, if it is not working the programm runs to the safety handler */


/* Opcode Test Failed, goto safety-Handler */
_Opcode_Test_Failed
   MOV   R0, #GLOB_FAILCODE_OPCODE_TEST_ASM  /* return with error */
   BL    globFail_SafetyHandler              /* goto the safety handler, see SRS_359 */
   BX    lr                                  /* return */



/**** end of assembler code                                                ****/
   ALIGN
   END

/**** Last line of source code                                             ****/

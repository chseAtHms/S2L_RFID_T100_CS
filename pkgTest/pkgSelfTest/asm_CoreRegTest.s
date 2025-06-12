/******************************************************************************/
/*!
 *******************************************************************************
 *       $Id: asm_CoreRegTest.s 4436 2024-05-17 14:28:50Z ankr $
 * $Revision: 4436 $
 *     $Date: 2024-05-17 16:28:50 +0200 (Fr, 17 Mai 2024) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          asm_CoreRegTest.s
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Core-Register Test
 * @description:
 * This modul containes the Core-Register Test.
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
   #include "globFit_FitTestHandler.h"
#undef __ASM__

#ifdef GLOBFIT_FITTEST_ACTIVE
#warning FITs in asm_CoreRegTest.s active!!!
#endif

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Constants (const)                                                          */
/******************************************************************************/


/***** End of:Constants *******************************************************/

/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/
   PRESERVE8
   AREA    COREREGTEST, CODE, READONLY

   EXPORT asm_CoreRegTest
   EXPORT asm_CoreRegTestStep1
   EXPORT asm_CoreRegTestStep2

   IMPORT globFail_SafetyHandler

#ifdef GLOBFIT_FITTEST_ACTIVE
   IMPORT globFit_u8FitCommand
#endif
/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/

/******************************************************************************/
/*!
@Name                Macro_FitCoreRegTest

@Description         Macro to execute the FIT tests for the CoreRegister-Test.

@Note                This is a specific macro for the CoreRegister-Test. It
                     stores the value of the workReg on the STACK and restores
                     the value at the end if no FIT execution is performed.
                     The value of the workReg have to be stored/restored on the
                     STACK because the default value is overwritten otherwise.
                     The FIT-Test macro in the globFit_FitTestHandler.h file
                     can not load the workReg value onto the STACK, because
                     this could corrupts the OC-CRC during the RAM-Test Step 2.
                     If a FIT is executed, the register value which is stored
                     onto the STACK is not written back to the register. In this
                     case the manipulated register is overwritten and the
                     manipulated value is destroyed.
@Parameter
   @param [in]       fitcmd      FIT command
   @param [in]       workReg     Working Register
   @param [in]       instr       Instruction to execute
   @param [in]       op1         Operand1 for the instruction
   @param [in]       op2         Operand2 for the instruction
   @return           void
 */
/******************************************************************************/
   MACRO
$label   Macro_FitCoreRegTest $fitcmd, $workReg, $instr, $op1, $op2
#ifdef GLOBFIT_FITTEST_ACTIVE
   PUSH     {$workReg}                       /* save workReg Value to STACK*/
   LDR.W    $workReg, =globFit_u8FitCommand  /* get the address of the FIT variable */
   LDR.W    $workReg, [$workReg]             /* get the value of the FIT variable */
   AND.W    $workReg, $workReg, #0x00FF
   CMP      $workReg, $fitcmd                /* check if the fit is activated */
   BNE.W    $label.popWorkReg                /* branch if not */

   $instr   $op1, $op2                       /* execute the instruction */
   B        $label.end                       /* branch to end */

$label.popWorkReg
   POP      {$workReg}                       /* load value of workReg from STACK */
$label.end
#endif
   MEND


/******************************************************************************/
/*!
@Name                Macro_TestRegisterStep1

@Description         Macro to execute test step 1 of the CoreRegister-Test

@Note
This Macro checks the CoreRegisters against following failures:
 - Transition faults (TF)
 - Stuck-at faults (SAF)
 - Address decoder faults (AF)

The functionality of the this test is the following:
   1. Loat test pattern 0x00000001 to R12 and $reg.
   2. Check if the value of R12 is equal to the test pattern in the test register.
   3. Shift the test pattern in R12 and $reg one position to the left. Do this
      in separate steps. Do not copy R12 to $reg. A stuck-at in R12 may be
      copied to $reg.
   4. Check if the 1 runs out of R12. If not go on with 2, else end the macro.

Register summary:
 R12 - Register with test pattern

@Parameter
   @param [in]       Register to Test, R0 - R11
   @return           void
 */
/******************************************************************************/
   MACRO
$label   Macro_TestRegisterStep1 $reg
   MOV   R12, #1                       /* write test pattern to R12 */
   MOV   $reg, #1                      /* write test pattern to test reg */
$label._FitTestStep1 Macro_FitCoreRegTest #GLOBFIT_CMD_COREREG_TEST_STEP1, $reg, MOV, $reg, #0

$label._Test_Step1_loop
   CMP   $reg, R12                     /* check if the values are equal */
   BNE.W _CoreReg_Test_Failed          /* if not branch to safety handler */

   LSLS  R12, R12, #1                  /* shift test pattern one position to the left */
   LSLS  $reg, $reg, #1                /* shift test pattern one position to the left */
   BCS   $label._Test_Step1_continue   /* if a overflow occurs in R12 (Carry = 1), the Register is tested, continue the test */
   B.W   $label._Test_Step1_loop       /* next test step */

$label._Test_Step1_continue            /* test successful, end macro */
   MEND


/******************************************************************************/
/*!
@Name                TestRegisterStep2

@Description         Macro to execute test the first part of step 2 of the
                     CoreRegister-Test (see [SRS_457]).

@Note
This Macro checks the CoreRegisters against following failures:
 - Transition faults (TF)
 - Coupling faults (CF)
 - Stuck-at faults (SAF)
 - Address decoder faults (AF)

The functionality of the this test step is the following:
   1. Set BC to 0xFFFFFFFF.
   2. Check the others Register, the value must have 0x00000000.
   3. If the value is not 0x00000000 go to the safety handler.

@Parameter
   @param [in]       Base Register, $BC
   @param [in]       Registers to test, $OC1...$OC12
   @return           void
 */
/******************************************************************************/
   MACRO
$label   TestRegisterStep2 $BC, $OC1, $OC2, $OC3, $OC4, $OC5, $OC6, $OC7, $OC8, $OC9, $OC10, $OC11, $OC12
   MOVW  $BC, #0xFFFF
   MOVT  $BC, #0xFFFF                  /* write test pattern 0xFFFFFFFF to BC */

   CMP   $OC1, #0                      /* check if the content of OC1 is 0 */
   BNE.W _CoreReg_Test_Failed          /* if not branch to the safety handler */
   CMP   $OC2, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC3, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC4, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC5, #0
   BNE.W _CoreReg_Test_Failed
$label._FitTestStep2 Macro_FitCoreRegTest #GLOBFIT_CMD_COREREG_TEST_STEP2, $OC5, MOV, $OC6, #0x100
   CMP   $OC6, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC7, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC8, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC9, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC10, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC11, #0
   BNE.W _CoreReg_Test_Failed
   CMP   $OC12, #0
   BNE.W _CoreReg_Test_Failed

   MOV   $BC, #0                       /* set BC back to 0 */
   MEND


/******************************************************************************/
/*!
@Name                TestRegisterStep2_inv

@Description         Macro to execute test the second part of step 2 of the
                     CoreRegister-Test (see [SRS_457]).

@Note
This Macro checks the CoreRegisters against following failures:
 - Transition faults (TF)
 - Coupling faults (CF)
 - Stuck-at faults (SAF)
 - Address decoder faults (AF)

The functionality of the this test step is the following:
   1. Set BC to 0x00000000.
   2. Check the others Register, the value must have 0xFFFFFFFF.
   3. If the value is not 0xFFFFFFFF go to the safety handler.

@Parameter
   @param [in]       Base Register, $BC
   @param [in]       Registers to test, $OC1...$OC12
   @return           void
 */
/******************************************************************************/
   MACRO
$label   TestRegisterStep2_inv $BC, $OC1, $OC2, $OC3, $OC4, $OC5, $OC6, $OC7, $OC8, $OC9, $OC10, $OC11, $OC12
   MOV   $BC, #0                       /* write test pattern to BC */

   CMP   $OC1, #0xFFFFFFFF             /* check if the content of OC is 0xFFFFFFFF */
   BNE.W _CoreReg_Test_Failed          /* if not branch to the safety handler */
   CMP   $OC2, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC3, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
$label._FitTestStep2Inv Macro_FitCoreRegTest #GLOBFIT_CMD_COREREG_TEST_STEP2_INV, $OC3, MOV, $OC4, #0x7FFFFFFF
   CMP   $OC4, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC5, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC6, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC7, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC8, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC9, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC10, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC11, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed
   CMP   $OC12, #0xFFFFFFFF
   BNE.W _CoreReg_Test_Failed

   MOV   $BC, #0xFFFFFFFF              /* set BC back to 0xFFFFFFFF */
   MEND

/***** End of: Macros *********************************************************/

/******************************************************************************/
/* Global Functions                                                           */
/******************************************************************************/


/******************************************************************************/
/*!
@Name                asm_CoreRegTestStep1

@Description         CoreRegister-Test Step 1

@Note
This function checks the Core-Registers R0..R12 against Stuck-At-Failures.
For further information see description of the Macro.

Register 12 is not explicit testet. During the test of R0..R11 the R12 is tested
as well.

@Parameter
   @return           void
 */
/******************************************************************************/
asm_CoreRegTest
asm_CoreRegTestStep1

   PUSH  {R0-R12} /* Push / store Register on the STACK */

/* call the macro for each Register */
_Test_Step1_R0    Macro_TestRegisterStep1 R0
_Test_Step1_R1    Macro_TestRegisterStep1 R1
_Test_Step1_R2    Macro_TestRegisterStep1 R2
_Test_Step1_R3    Macro_TestRegisterStep1 R3
_Test_Step1_R4    Macro_TestRegisterStep1 R4
_Test_Step1_R5    Macro_TestRegisterStep1 R5
_Test_Step1_R6    Macro_TestRegisterStep1 R6
_Test_Step1_R7    Macro_TestRegisterStep1 R7
_Test_Step1_R8    Macro_TestRegisterStep1 R8
_Test_Step1_R9    Macro_TestRegisterStep1 R9
_Test_Step1_R10   Macro_TestRegisterStep1 R10
_Test_Step1_R11   Macro_TestRegisterStep1 R11

   B.W      _CoreReg_Test_Ok


/******************************************************************************/
/*!
@Name                asm_CoreRegTestStep2

@Description         CoreRegister-Test Step 2

@Note
The CoreRegister-Test Step 2 checks the CoreRegisters against following
failures:
 - Transition faults (TF)
 - Coupling faults (CF)
 - Stuck-at faults (SAF)
 - Address decoder faults (AF)
For further information see description of the Macro.

@Parameter
   @return           void
 */
/******************************************************************************/
asm_CoreRegTestStep2

   PUSH  {R0-R12} /* Push / store Register on the STACK */


   /* set all registers to 0 */
   MOV   R0, #0
   MOV   R1, #0
   MOV   R2, #0
   MOV   R3, #0
   MOV   R4, #0
   MOV   R5, #0
   MOV   R6, #0
   MOV   R7, #0
   MOV   R8, #0
   MOV   R9, #0
   MOV   R10, #0
   MOV   R11, #0
   MOV   R12, #0

/* call the macro for each register */
_Test_Step2_R0       TestRegisterStep2 R0,     R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R1       TestRegisterStep2 R1, R0,     R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R2       TestRegisterStep2 R2, R0, R1,     R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R3       TestRegisterStep2 R3, R0, R1, R2,     R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R4       TestRegisterStep2 R4, R0, R1, R2, R3,     R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R5       TestRegisterStep2 R5, R0, R1, R2, R3, R4,     R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R6       TestRegisterStep2 R6, R0, R1, R2, R3, R4, R5,     R7, R8, R9, R10, R11, R12
_Test_Step2_R7       TestRegisterStep2 R7, R0, R1, R2, R3, R4, R5, R6,     R8, R9, R10, R11, R12
_Test_Step2_R8       TestRegisterStep2 R8, R0, R1, R2, R3, R4, R5, R6, R7,     R9, R10, R11, R12
_Test_Step2_R9       TestRegisterStep2 R9, R0, R1, R2, R3, R4, R5, R6, R7, R8,     R10, R11, R12
_Test_Step2_R10      TestRegisterStep2 R10, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9,     R11, R12
_Test_Step2_R11      TestRegisterStep2 R11, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,     R12
_Test_Step2_R12      TestRegisterStep2 R12, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11


   /* set all registers to 0xFFFFFFF */
   MOVW  R0, #0xFFFF
   MOVT  R0, #0xFFFF
   MOV   R1, R0
   MOV   R2, R0
   MOV   R3, R0
   MOV   R4, R0
   MOV   R5, R0
   MOV   R6, R0
   MOV   R7, R0
   MOV   R8, R0
   MOV   R9, R0
   MOV   R10, R0
   MOV   R11, R0
   MOV   R12, R0

/* call the macro for each register */
_Test_Step2_R0_inv   TestRegisterStep2_inv R0,     R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R1_inv   TestRegisterStep2_inv R1, R0,     R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R2_inv   TestRegisterStep2_inv R2, R0, R1,     R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R3_inv   TestRegisterStep2_inv R3, R0, R1, R2,     R4, R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R4_inv   TestRegisterStep2_inv R4, R0, R1, R2, R3,     R5, R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R5_inv   TestRegisterStep2_inv R5, R0, R1, R2, R3, R4,     R6, R7, R8, R9, R10, R11, R12
_Test_Step2_R6_inv   TestRegisterStep2_inv R6, R0, R1, R2, R3, R4, R5,     R7, R8, R9, R10, R11, R12
_Test_Step2_R7_inv   TestRegisterStep2_inv R7, R0, R1, R2, R3, R4, R5, R6,     R8, R9, R10, R11, R12
_Test_Step2_R8_inv   TestRegisterStep2_inv R8, R0, R1, R2, R3, R4, R5, R6, R7,     R9, R10, R11, R12
_Test_Step2_R9_inv   TestRegisterStep2_inv R9, R0, R1, R2, R3, R4, R5, R6, R7, R8,     R10, R11, R12
_Test_Step2_R10_inv  TestRegisterStep2_inv R10, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9,     R11, R12
_Test_Step2_R11_inv  TestRegisterStep2_inv R11, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,     R12
_Test_Step2_R12_inv  TestRegisterStep2_inv R12, R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11


/* CoreReg-Test was successful */
_CoreReg_Test_Ok
   POP      {R0-R12}    /* Pop / restore Register from the STACK */
   BX       lr          /* return */


/* an error occured during the RAM-Test, goto the safety handler */
_CoreReg_Test_Failed
   MOV   R0, #GLOB_FAILCODE_COREREG_TEST_ASM /* return with error */
   BL    globFail_SafetyHandler              /* goto the safety handler, see SRS_359 */
   BX    lr                                  /* return */

/***** End of:  Global Functions **********************************************/

/* End of assembly routine */
   END
/**** Last line of source code                                             ****/

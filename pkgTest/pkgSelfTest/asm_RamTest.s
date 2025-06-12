/******************************************************************************/
/*!
 *******************************************************************************
 *       $Id: asm_RamTest.s 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          asm_RamTest.s
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        RAM-Test
 * @description:
 * This module contains the RAM-Test functions (implemented is a walk path test,
 * see SRS_46).
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
#warning FITs in asm_RamTest.s active!!!
#endif

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Constants (const)                                                          */
/******************************************************************************/

CRC_REG_ADDR_HI            EQU   0x4002 /* High byte of CRC-Register address */
CRC_RESET_REG_ADDR         EQU   0x3008 /* Low byte of Reset-CRC-Register address */
CRC_DR_REG_ADDR            EQU   0x3000 /* Low byte of DR-CRC-Register address */
CRC_RESET_FLAG             EQU   0x01   /* Reset-Flag for CRC-Register */

/***** End of:Constants *******************************************************/

/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/
   PRESERVE8
   AREA    RAMTEST, CODE, READONLY

   EXPORT asm_RamTestStep1
   EXPORT asm_RamTestStep2

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
@Name                Macro_CalcOCCrc

@Description         Macro to calculate the CRC of OC

@Note
This macro calculates the CRC of the OC area.


Register summary:
 R0  - BC
 R1  - OC-area start address
 R2  - OC test size
 R3  - Working Register
 R4  - Working Register
 R6  - CRC Reset Value
 R11 - CRC-DR Register Address
 R12 - CRC-Reset Register Address

@Parameter
   @return           void
 */
/******************************************************************************/
   MACRO
$label   Macro_CalcOCCrc
   /*--- calculate the CRC of the OC-area except the BC -------------------------*/
   STRB  R6, [R12]                     /* Reset CRC */
   MOV   R3, R1                        /* copy OC start addresse to R3 */
$label.Calc_Crc_Loop
   CMP   R3, R0                        /* check if OC-Adresse = BC-Adresse */
   BEQ   $label.Jump_Over_Crc_Calc     /* jump over crc calculation, BC = OC */
   LDRB  R4, [R3]                      /* get the value of the OC address */
   STRB  R4, [R11]                     /* calculate CRC */
$label.Jump_Over_Crc_Calc
   ADD   R3, R3, #1                    /* increase OC-address */
   CMP   R3, R2                        /* check if the end of the OC-area is achieved */
   BNE   $label.Calc_Crc_Loop

   MEND

/***** End of: Macros *********************************************************/

/******************************************************************************/
/* Global Functions                                                           */
/******************************************************************************/


/******************************************************************************/
/*!
@Name                asm_RamTestStep1

@Description         RAM-Test Step 1

@Note
The RAM-test Step 1 checks the RAM against Stuck-At-Failures.

The functionality of the this test step is the following:
   1. Save content of the BC.
   2. Loat test pattern 0x00000001.
   2. Write the test pattern to the BC.
   3. Check if the value of BC is equal to the test pattern.
   4. Shift the test pattern one position to the left.
   5. Check if the 1 runs out of the BC. If not go on with 2, else with 6.
   6. Is the end of the test cycle achieved? If yes go to 7., else set BC to
      next cell and go on with 1.
   7. Load back the content of the BC.

Register summary:
 R0  - BC
 R1  - Number of test cycles
 [R3  - Working Register, only if FIT-Tests are activated]
 R6  - RAM_END value
 R7  - Test pattern
 R8  - Read back test pattern
 [R9  - Working Register, only if FIT-Tests are activated]
 R10 - Saved RAM-Value of BC

@Parameter
   @return           void
 */
/******************************************************************************/
asm_RamTestStep1

#ifdef GLOBFIT_FITTEST_ACTIVE
   PUSH  {R0,R1,R3,R6-R10}    /* Push / store Register content on the STACK */
#else
   PUSH  {R0,R1, R6-R8,R10}    /* Push / store Register content on the STACK */
#endif
   MOV   R6, #RAMTST_RAM_END_ASM_LO
   MOVT  R6, #RAMTST_RAM_END_ASM_HI /* get the RAM end address */

_Test_Step1_Start
   LDR   R10, [R0]            /* save the RAM-Value of BC to R10 */
   MOV   R7, #1               /* load the test pattern to R7 */

_Test_Step1_loop
   STR   R7, [R0]             /* store test pattern in BC */
   LDR   R8, [R0]             /* load test pattern from RAM to R8 */
_FitTestStep1 GLOBFIT_FitTest #GLOBFIT_CMD_RAM_TEST_STEP1_1, R3, MOV, R8, #0x1001 /* This FIT test manipulates the read back register of the RAM test step 1. This test simulates a stuck-at low. */
   CMP   R7, R8               /* check if R7 equals R8 */
   BNE.W _Ram_Test_Failed     /* if not equal branch to safety state */
   LSLS  R7, R7, #1           /* shift test pattern in R7 to the left */
   BCS   _Test_Step1_continue /* if a overflow occurs in R7 (Carry = 1), the BC is tested, continue the test */
   B     _Test_Step1_loop

_Test_Step1_continue
   STR   R10, [R0]            /* restore the value of the BC */
   SUBS  R1, R1, #1           /* decrement BC cycle counter */
   CBZ   R1, _Test_Step1_End  /* check if the counter is 0, then finish the test */

   ADD   R0, R0, #4           /* set next address to test */
   CMP   R0, R6               /* check if the RAM_END achieved */
   BEQ   _Test_Step1_End      /* finish the test */

#ifdef GLOBFIT_FITTEST_ACTIVE /* FIT-Code to test the last RAM-Cell */
_FitTestStep1_2 GLOBFIT_FitTest #GLOBFIT_CMD_RAM_TEST_STEP1_2, R3, CMP, R3, #GLOBFIT_CMD_RAM_TEST_STEP1_2
   BNE   _FitTestStep1_2End   /* FIT is not active, over jump the FIT */
   MOV   R9, R6               /* get the RAM end address */
   SUB   R9, R9, #4           /* set the address to the last BC */
   CMP   R0, R9               /* check if actual BC is at the last BC */
   BNE   _Test_Step1_Start    /* branch if not */
   MOV   R9, #GLOBFIT_CMD_RAM_TEST_STEP1_1   /* activate the FIT */
   LDR   R3, =globFit_u8FitCommand
   STR   R9, [R3]
_FitTestStep1_2End
#endif

   B     _Test_Step1_Start    /* start a new test cycle */

_Test_Step1_End
#ifdef GLOBFIT_FITTEST_ACTIVE
   POP   {R0,R1,R3,R6,R7-R10}    /* Pop / restore Register content on the STACK */
#else
   POP   {R0,R1,R6-R8,R10}    /* Pop / restore Register content on the STACK */
#endif
   B     _Ram_Test_Ok


/******************************************************************************/
/*!
@Name                asm_RamTestStep2

@Description         RAM-Test Step 2

@Note
The RAM-test Step 2 checks the RAM against following failures:
   - DC fault model for data and adresses
   - Static cross-over for memory cells
The functionality of the this test step is the following:
   1. Save content of the BC.
   2. Calculate CRC of the OC (without BC).
   3. Set BC to 0x00.
   4. Calculate CRC of the OC (without BC).
   5. Check if the CRCs are equal.
   6. Set BC to 0xFF.
   7. Calculate CRC of the OC (without BC).
   8. Check if the CRCs are equal.
   9. End of RAM achieved with OC? If not set OC to the next range and begin
      with 2.
  10. End of RAM achieved with BC? If not load back content of BC and set BC to
      the next cell and begin with 2.

Register summary:
 R0  - BC
 R1  - OC-area start address
 R2  - OC test size
 R3  - Working Register
 R4  - Working Register
 R5  - empty
 R6  - CRC Reset Value
 R7  - 0x00 test pattern
 R8  - 0xFF test pattern
 R9  - CRC of OC area
 R10 - Saved RAM-Value of BC
 R11 - CRC-DR Register Address
 R12 - CRC-Reset Register Address

@Parameter
   @return           void
 */
/******************************************************************************/
asm_RamTestStep2

   PUSH  {R0-R12}    /* Push / store Register content on the STACK */

/*--- init RAM Test Step 2 ---------------------------------------------------*/
   MOV   R7, #0x00
   MOV   R8, #0xFF

   MOVW  R12, #CRC_RESET_REG_ADDR
   MOVT  R12, #CRC_REG_ADDR_HI         /* copy CRC-RESET-Register Adrdess to R12 */

   MOVW  R11, #CRC_DR_REG_ADDR
   MOVT  R11, #CRC_REG_ADDR_HI         /* copy CRC-DR-Register Address to R11 */

   MOV   R6, #CRC_RESET_FLAG           /* Reset CRC */

   CMP   R2, #0                        /* check that the OC-area is not 0 */
   BEQ   _Ram_Test_Failed

   ADD   R2, R1, R2                    /* add OC length to start address to get the end address */

   LDRB  R10, [R0]                     /* load the RAM-Value of BC to R10 */


/*--- calculate the CRC of the OC-area except the BC -------------------------*/
_Init_Crc   Macro_CalcOCCrc            /* calculate the crc of the OC */
   LDR   R9, [R11]                     /* save the CRC of the OC-area to R9 */


/*--- Test with test pattern 0x00 --------------------------------------------*/
   STRB  R7, [R0]                      /* store test pattern 0x00 to BC */


/*--- calculate the CRC of the OC-area except the BC -------------------------*/
_Crc1   Macro_CalcOCCrc                /* calculate the crc of the OC after setting BC to 0x00 */
_FitTestStep2_1 GLOBFIT_FitTest #GLOBFIT_CMD_RAM_TEST_STEP2_1, R3, STRB, R3, [R11] /* This FIT test manipulates the CRC of the RAM test step 2.  */
   LDR   R3, [R11]                     /* save the CRC of the OC-area to R3 */
   CMP   R3, R9                        /* check if the new calculated crc of the OC area equals the original */
   BNE   _Ram_Test_Failed              /*  if not, branch to safety handler*/


/*--- Test with test pattern 0xFF --------------------------------------------*/
   STRB  R8, [R0]                      /* store test pattern 0xFF to BC */
_FitTestStep2_2 GLOBFIT_FitTest #GLOBFIT_CMD_RAM_TEST_STEP2_2, R3, STRB, R3, [R1] /* This FIT test manipulates the OC of the RAM test step 2.  */

/*--- calculate the CRC of the OC-area except the BC -------------------------*/
_Crc2   Macro_CalcOCCrc                /* calculate the crc of the OC after setting BC to 0xFF */
   LDR   R3, [R11]                     /* save the CRC of the OC-area to R3 */
   CMP   R3, R9                        /* check if the new calculated crc of the OC area equals the original */
   BNE   _Ram_Test_Failed              /* if not, branch to safety handler */

   STRB  R10, [R0]                     /* restore the value of the BC */

_Test_Step2_End                        /* end of RAM-Test step 2 */

   POP   {R0-R12}                      /* Pop / restore Register content on the STACK */

/* RAM-Test was successful */
_Ram_Test_Ok
   MOV   R0,   #STDEF_RET_BUSY      /* return with RET_BUSY */
   BX    lr                         /* return */


/* an error occured during the RAM-Test, goto the safety handler */
_Ram_Test_Failed
   MOV   R0, #GLOB_FAILCODE_RAM_TEST_ASM  /* return with error */
   BL    globFail_SafetyHandler           /* goto the safety handler, see SRS_359 */
   BX    lr                               /* return */

/***** End of:  Global Functions **********************************************/

/* End of assembly routine */
   ALIGN
   END
/**** Last line of source code                                             ****/

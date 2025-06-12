/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globFit_FitTestHandler.h 2992 2017-05-10 08:44:52Z klan $
 * $Revision: 2992 $
 *     $Date: 2017-05-10 10:44:52 +0200 (Mi, 10 Mai 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globFit_FitTestHandler.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Header of the FIT-Test Handler module
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef GLOBFIT_FITTESTHANDLER_H
#define GLOBFIT_FITTESTHANDLER_H


/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/
#include "globFit_FitTestHandler_Cfg.h"



/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/


#ifdef GLOBFIT_FITTEST_ACTIVE

/*** Definition of the FIT-Test Commands. *************************************
 *
 * Bit 0..14:
 *    FIT-Test Commands with a range from 0x01..0x7F.
 * Bit 15:
 *    Definition of the controller.
 *    0 - controller 1
 *    1 - controller 2
 */
 /*!< This is the default value of the FIT command. */
#define GLOBFIT_CMD_NONE                     0x00 

/* FIT Command for the selftest logical programflow control */
/*!< This command manipulates the logical program control of the selftests.*/
#define GLOBFIT_CMD_SELFTEST_LPC1            0x01 
/*!< This command manipulates the logical program control of the selftests.*/
#define GLOBFIT_CMD_SELFTEST_LPC2            0x02 
/*!< This command manipulates the logical program control of the selftests.*/
#define GLOBFIT_CMD_SELFTEST_LPC3            0x03 

/* FIT Commands for the STACK Test. */
/*!< This command manipulates the STACK overflow area. */
#define GLOBFIT_CMD_STACK_OVFLW_TEST         0x04 
/*!< This command manipulates the STACK underflow area. */
#define GLOBFIT_CMD_STACK_UNDFLW_TEST        0x05 

/* FIT Command for the safety container*/
/*!< This command manipulates the logical program flow. The RAM test is skipped. */
#define GLOBFIT_CMD_SAFETY_CONTAINER         0x06 

/* FIT Commands for the core register test. */
/*!< This command manipulates the Core Register Test Step 1. */
#define GLOBFIT_CMD_COREREG_TEST_STEP1       0x07 
/*!< This command manipulates the Core Register Test Step 2. */
#define GLOBFIT_CMD_COREREG_TEST_STEP2       0x08 
/*!< This command manipulates the Core Register Test Step 2. */
#define GLOBFIT_CMD_COREREG_TEST_STEP2_INV   0x09 

/* FIT Command for the SFR Test */
/*!< This command manipulates the SFR test. */
#define GLOBFIT_CMD_SFR_TEST                 0x0A 

/* FIT Commands for the Opcode Test. */
/*!< This command manipulates the LDR and STR test. */
#define GLOBFIT_CMD_OPCODE_TEST_LDR_STR      0x0B 
/*!< This command manipulates the PUSH and POP test. */
#define GLOBFIT_CMD_OPCODE_TEST_PUSH_POP     0x0C 
/*!< This command manipulates the MOV test. */
#define GLOBFIT_CMD_OPCODE_TEST_MOV1         0x0D 
/*!< This command manipulates the MOV test. */
#define GLOBFIT_CMD_OPCODE_TEST_MOV2         0x0E 
/*!< This command manipulates the MOV test. */
#define GLOBFIT_CMD_OPCODE_TEST_MOV3         0x0F 
/*!< This command manipulates the MOV test. */
#define GLOBFIT_CMD_OPCODE_TEST_MOV4         0x10 
/*!< This command manipulates the ADD test. */
#define GLOBFIT_CMD_OPCODE_TEST_ADD1         0x11 
/*!< This command manipulates the ADD test. */
#define GLOBFIT_CMD_OPCODE_TEST_ADD2         0x12 
/*!< This command manipulates the ADD test. */
#define GLOBFIT_CMD_OPCODE_TEST_ADD3         0x13 
/*!< This command manipulates the LSR test. */
#define GLOBFIT_CMD_OPCODE_TEST_LSR1         0x14 
/*!< This command manipulates the LSR test. */
#define GLOBFIT_CMD_OPCODE_TEST_LSR2         0x15 
/*!< This command manipulates the MUL test. */
#define GLOBFIT_CMD_OPCODE_TEST_MUL1         0x16 
/*!< This command manipulates the MUL test. */
#define GLOBFIT_CMD_OPCODE_TEST_MUL2         0x17 
/*!< This command manipulates the DIV test. */
#define GLOBFIT_CMD_OPCODE_TEST_DIV1         0x18 
/*!< This command manipulates the DIV test. */
#define GLOBFIT_CMD_OPCODE_TEST_DIV2         0x19 
/*!< This command manipulates the SAT test. */
#define GLOBFIT_CMD_OPCODE_TEST_SAT1         0x1A 
/*!< This command manipulates the SAT test. */
#define GLOBFIT_CMD_OPCODE_TEST_SAT2         0x1B 
/*!< This command manipulates the BFC test. */
#define GLOBFIT_CMD_OPCODE_TEST_BFC          0x1C 
/*!< This command manipulates the BFI test. */
#define GLOBFIT_CMD_OPCODE_TEST_BFI          0x1D 
/*!< This command manipulates the B test. */
#define GLOBFIT_CMD_OPCODE_TEST_B            0x1E 
/*!< This command manipulates the MSR test. */
#define GLOBFIT_CMD_OPCODE_TEST_MSR          0x1F 
/*!< This command manipulates the MRS test. */
#define GLOBFIT_CMD_OPCODE_TEST_MRS          0x20 

/*!< This command is unused at the moment. */
#define GLOBFIT_UNUSED_COMMAND               0x21 
/* FIT Commands for the ROM Test */
/*!< This command manipulates the ROM test cycle counter. */
#define GLOBFIT_CMD_ROM_TEST_CYCL1           0x22 
/*!< This command manipulates the ROM test CRC. */
#define GLOBFIT_CMD_ROM_TEST_CRC1            0x23 
/*!< This command manipulates the ROM test CRC. */
#define GLOBFIT_CMD_ROM_TEST_CRC2            0x24 
/*!< This command manipulates the ROM test CRC. */
#define GLOBFIT_CMD_ROM_TEST_STARTUP         0x25 

/* FIT Commands for the RAM Test. The command must have at least two bits set, to prevent an 
** accidental activation of the FIT test. */
/*!< This command manipulates the read back register of the RAM Test Step 1.  */
#define GLOBFIT_CMD_RAM_TEST_STEP1_1         0x26 
/*!< This command manipulates the read back register of the last tested RAM cell the RAM 
** Test Step 1. */
#define GLOBFIT_CMD_RAM_TEST_STEP1_2         0x27 
/*!< This command manipulates the RAM Test cycles for the RAM Test Step 1. */
#define GLOBFIT_CMD_RAM_TEST_STEP1_3         0x28 
/*!< This command manipulates the CRC of the RAM Test Step 2. */
#define GLOBFIT_CMD_RAM_TEST_STEP2_1         0x29 
/*!< This command manipulates the OC of the RAM Test Step 2. */
#define GLOBFIT_CMD_RAM_TEST_STEP2_2         0x2A 
/*!< This command manipulates the RAM Test cycles for the RAM Test Step 2. */
#define GLOBFIT_CMD_RAM_TEST_STEP2_3         0x2B 

/*!< This command manipulates the startup RAM-Test. */
#define GLOBFIT_CMD_RAMTEST_STARTUP_ERR1     0x2C 
/*!< This command manipulates the startup RAM-Test. */
#define GLOBFIT_CMD_RAMTEST_STARTUP_ERR2     0x2D 
/*!< This command manipulates the startup RAM-Test. */
#define GLOBFIT_CMD_RAMTEST_STARTUP_ERR3     0x2E 
/*!< This command manipulates the startup RAM-Test. */
#define GLOBFIT_CMD_RAMTEST_STARTUP_ERR4     0x2F 
/*!< This command manipulates the startup RAM-Test. */
#define GLOBFIT_CMD_RAMTEST_STARTUP_ERR5     0x30 

/* FIT Commands for data security. */
/*!< This command manipulates the raw UINT8 data value of a RDS variable */
#define GLOBFIT_CMD_RDS_UINT8                0x31 
/*!< This command manipulates the the inverse UINT16 data value of a RDS variable */
#define GLOBFIT_CMD_RDS_UINT16               0x32 
/*!< This command manipulates the the raw UINT32 data value of a RDS variable */
#define GLOBFIT_CMD_RDS_UINT32               0x33 
/*!< This command simulates a bit-flip in a variable which is secured by a hamming distance */
#define GLOBFIT_CMD_HAMMING_BITFLIP          0x34 

/* FIT Command for watchdog. */
/*!< This command overjumps the warchdog trigger. */
#define GLOBFIT_CMD_WATCHDOG                 0x35 

/* FIT Command for selftest timeout. */
/*!< This command tests the timeout o the selftests. */
#define GLOBFIT_NO_SELFTEST_TRIGGER          0x36 

/* FIT Command for unused interrupts. */
/*!< This command calls an unused ISR. */
#define GLOBFIT_CMD_UNUSED_INTERRUPT         0x37 

/* FIT Commands for the IPC. */
/*!< This command manipulates the IPC ID. */
#define GLOBFIT_CMD_IPC_ID                   0x38 
/*!< This command manipulates the IPC Sync Timeout. */
#define GLOBFIT_CMD_IPC_TIMEOUT_SYNC         0x39 
/*!< This command manipulates the TX data exchange. */
#define GLOBFIT_CMD_IPC_TIMEOUT_TX           0x3A 
/*!< This command manipulates the RX data exchange. */
#define GLOBFIT_CMD_IPC_TIMEOUT_RX           0x3B 
/*!< This command manipulates a value in the received IPC buffers. */
#define GLOBFIT_CMD_IPC_CRC                  0x3C 
/*!< This command manipulates the controller ID in the received and valid IPC buffer. */
#define GLOBFIT_CMD_IPC_CONTROLLER           0x3D 

/* FIT Commands for the AIC. */
/*!< This command manipulates the AIC telegram CRC. */
#define GLOBFIT_CMD_AIC_CRC                  0x3F

/* FIT Commands for the I-Parameters (IO Configuration) */
/*!< This command manipulates the IO Configuration. */
#define GLOBFIT_CMD_IPAR_CRC                 0x40 

/* FIT Commands for the CIPSafety pathflow and soft-error supervision */
/* CSS Program Flow is manipulated */
#define GLOBFIT_CMD_PFLOW                    0x43 
/* CSS Soft Error is manipulated */
#define GLOBFIT_CMD_SOFT_ERROR               0x44 

/* FIT Command for scheduler timeout check */
/* Scheduler cycle time is too high */
#define GLOBFIT_CMD_TCYC_HI                  0x45 
/* Scheduler cycle time is too low  */
#define GLOBFIT_CMD_TCYC_LO                  0x46 


/* FIT Commands for the PROFIsafe stack, see PSD_15 */
/*!< Command for the PSD FIT FI_PSD_025. */
#define GLOBFIT_CMD_FI_PSD_025               0x50 
/*!< Command for the PSD FIT FI_PSD_024. */
#define GLOBFIT_CMD_FI_PSD_024               0x51 
/*!< Command for the PSD FIT NO_PSD_RUN. */
#define GLOBFIT_CMD_NO_PSD_RUN               0x52 
/*!< Command for the PSD FIT FI_PSD_001. */
#define GLOBFIT_CMD_FI_PSD_001               0x53 
/*!< Command for the PSD FIT FI_PSD_002. */
#define GLOBFIT_CMD_FI_PSD_002               0x54 
/*!< Command for the PSD FIT FI_PSD_003. */
#define GLOBFIT_CMD_FI_PSD_003               0x55 
/*!< Command for the PSD FIT FI_PSD_005. */
#define GLOBFIT_CMD_FI_PSD_005               0x56 
/*!< Command for the PSD FIT FI_PSD_006. */
#define GLOBFIT_CMD_FI_PSD_006               0x57 
/*!< Command for the PSD FIT FI_PSD_011. */
#define GLOBFIT_CMD_FI_PSD_011               0x58 
/*!< Command for the PSD FIT FI_PSD_013. */
#define GLOBFIT_CMD_FI_PSD_013               0x59 
/*!< Command for the PSD FIT FI_PSD_017. */
#define GLOBFIT_CMD_FI_PSD_017               0x5A 
/*!< Command for the PSD FIT FI_PSD_102. */
#define GLOBFIT_CMD_FI_PSD_102               0x5B 
/*!< Command for the PSD FIT FI_PSD_104. */
#define GLOBFIT_CMD_FI_PSD_104               0x5C 

/* FIT Command to reset the uC */
/*!< This command resets the controller. */
#define GLOBFIT_CMD_RESET_MCU                0x7F 

#endif

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


#ifdef GLOBFIT_FITTEST_ACTIVE
/* FIT Tests are active */

/******************************************************************************/
/*!
@Name                GLOBFIT_FITTEST

@Description         This macro executes the FITs. After executing the instruction
                     it set the fit-command variable back to 0.

@note                This macro is for the C-Code.

@Parameter
   @param [in]       fitCmd      FIT command
   @param [in]       instr       Instruction to execute
   @return           void
 */
/******************************************************************************/
#define GLOBFIT_FITTEST(fitCmd, instr)  \
{  \
   if (fitCmd == globFit_u8FitCommand)  \
   {  \
      globFit_u8FitCommand = 0u; \
      instr;   \
   }  \
}

/******************************************************************************/
/*!
@Name                GLOBFIT_FITTEST_NOCMDRESET

@Description         This macro executes the FITs. It remains the command-variable.

@note                This macro is for the C-Code.

@Parameter
   @param [in]       fitCmd      FIT command
   @param [in]       instr       Instruction to execute
   @return           void
 */
/******************************************************************************/
#define GLOBFIT_FITTEST_NOCMDRESET(fitCmd, instr)  \
{  \
   if (fitCmd == globFit_u8FitCommand)  \
   {  \
      instr;   \
   }  \
}


/******************************************************************************/
/*!
@Name                Macro_FitTest

@Description         This macro executes the FITs.

@note                This macro is for the Assembler-Code.
                     This macro can not load the workReg value onto the STACK
                     because it could corrupts the OC-CRC in the RAM-Test
                     Step 2.

@Parameter
   @param [in]       fitCmd      FIT command
   @param [in]       workReg     Working Register
   @param [in]       instr       Instruction to execute
   @param [in]       op1         Operand 1 for the instruction
   @param [in]       op2         Operand 2 for the instruction
   @return           void
 */
/******************************************************************************/
#ifdef __ASM__
   MACRO
$label   GLOBFIT_FitTest $fitcmd, $workReg, $instr, $op1, $op2
   LDR.W    $workReg, =globFit_u8FitCommand  /* get the address of the FIT CMD variable */
   LDR.W    $workReg, [$workReg]             /* load the value of the FIT CMD variable to the reg.*/
   AND.W    $workReg, $workReg, #0x00FF
   CMP      $workReg, $fitcmd                /* check if the commands are equal */
   BNE.W    $label.end                       /* if not, branch to the end of the macro */

   $instr   $op1, $op2                       /* execute the specified instruction */

$label.end
   MEND                                      /* end of macro */
#endif /* __ASM__ */


#else /* GLOBFIT_FITTEST_ACTIVE */
/* FIT Tests are not active. */

/* Empty macros, no action when FIT Tests are disabled. */
#define GLOBFIT_FITTEST(cmd, action) ((void)0)
#define GLOBFIT_FITTEST_NOCMDRESET(cmd, action) ((void)0)

#ifdef __ASM__
   MACRO
$label   GLOBFIT_FitTest $fitcmd, $workReg, $instr, $op1, $op2
   MEND
#endif /* __ASM__ */

#endif /* GLOBFIT_FITTEST_ACTIVE */

/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/***** End of: typedef Types ********************************************//*@}*/

#if (!defined __ASM__) && (defined GLOBFIT_FITTEST_ACTIVE)
/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

extern UINT8 globFit_u8FitCommand;

/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

extern void globFit_Init (void);
extern void globFit_FitTestHandler (void);
extern UINT8 globFit_GetFitCmd (void);
extern GLOB_RETCODE_ENUM globFit_SendByte (CONST UINT8 u8Byte);


/***** End of: Function-Prototypes ********************************************/
#endif

#endif /* GLOBFIT_FITTESTHANDLER_H */

/**** Last line of source code                                             ****/

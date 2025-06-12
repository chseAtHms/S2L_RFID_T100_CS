/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globFail_Errorcodes.h
**
** $Id: globFail_Errorcodes.h 4178 2024-01-10 15:08:06Z ankr $
** $Revision: 4178 $
** $Date: 2024-01-10 16:08:06 +0100 (Mi, 10 Jan 2024) $
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
** Global return codes and failure codes.
** This header provides the return codes and the failure codes.
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

#ifndef GLOBFAIL_ERRORCODES_H
#define GLOBFAIL_ERRORCODES_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/



/*******************************************************************************
**
** Constants
**
********************************************************************************
*/



/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/


/*! Defines of failure codes for assembler modules */

/*! Failure code for assembler code, see GLOB_FAILCODE_RAM_TEST. */
#define GLOB_FAILCODE_RAM_TEST_ASM            0x84c0
/*! Failure code for assembler code, see GLOB_FAILCODE_OPCODE_TEST. */
#define GLOB_FAILCODE_OPCODE_TEST_ASM         0x87ee
/*! Failure code for assembler code, see GLOB_FAILCODE_COREREG_TEST. */
#define GLOB_FAILCODE_COREREG_TEST_ASM        0x89ef
/*! Failure code for assembler code, see GLOB_FAILCODE_NMI. */
#define GLOB_FAILCODE_NMI_ASM                 0x8e24
/*! Failure code for assembler code, see GLOB_FAILCODE_HARD_FAULT. */
#define GLOB_FAILCODE_HARD_FAULT_ASM          0x8fb3
/*! Failure code for assembler code, see GLOB_FAILCODE_MEMMANAGE_FAULT. */
#define GLOB_FAILCODE_MEMMANAGE_FAULT_ASM     0x909f
/*! Failure code for assembler code, see GLOB_FAILCODE_BUS_FAULT. */
#define GLOB_FAILCODE_BUS_FAULT_ASM           0x9108
/*! Failure code for assembler code, see GLOB_FAILCODE_USAGE_FAULT. */
#define GLOB_FAILCODE_USAGE_FAULT_ASM         0x9226
/*! Failure code for assembler code, see GLOB_FAILCODE_SVC. */
#define GLOB_FAILCODE_SVC_ASM                 0x93b1
/*! Failure code for assembler code, see GLOB_FAILCODE_DEBUGMON_FAULT. */
#define GLOB_FAILCODE_DEBUGMON_FAULT_ASM      0x947a
/*! Failure code for assembler code, see GLOB_FAILCODE_PENDSV. */
#define GLOB_FAILCODE_PENDSV_ASM              0x95ed
/*! Failure code for assembler code, see GLOB_FAILCODE_SYSTICK. */
#define GLOB_FAILCODE_SYSTICK_ASM             0x96c3

/* define used to encapsulate functionality/types/definitions not usable from
** assembler modules */
#ifndef __ASM__

/*lint -esym(769, GLOB_FAILCODE_POINTER_INVALID) global enumeration constant
**'GLOB_FAILCODE_POINTER_INVALID' not referenced. Only used for T100/PS */
/*lint -esym(769, GlOB_FAILCODE_PSD_HARD_ERR) global enumeration constant
**'GlOB_FAILCODE_PSD_HARD_ERR' not referenced. Only used for T100/PS */
/*lint -esym(769, GlOB_FAILCODE_FPAR_CRC_ERR) global enumeration constant
**'GlOB_FAILCODE_FPAR_CRC_ERR' not referenced. Only used for T100/PS */
/*lint -esym(769, GLOB_FAILCODE_NO_FAILURE) global enumeration constant
**'GLOB_FAILCODE_NO_FAILURE' not referenced. Only used for T100/PS */
/*lint -esym(769, GLOB_FAILCODE_INVALID_TRANSITION) global enumeration constant
**'GLOB_FAILCODE_INVALID_TRANSITION' not referenced. Defined for future use. */

/* following failure codes are only used in case of analog temperature sensor: */
/*lint -esym(769, GLOB_FAILCODE_ADC_CONV_TIMEOUT) */
/*lint -esym(769, GLOB_FAILCODE_ADC_CALIB_TIMEOUT) */

/* following failure codes are only used in case of digital temperature sensor: */
/*lint -esym(769, GLOB_FAILCODE_I2C_TMP102_PRESENCE) */
/*lint -esym(769, GLOB_FAILCODE_I2C_STATE_ERR) */
/*lint -esym(769, GLOB_FAILCODE_I2C_CONFIG_ERR) */
/*lint -esym(769, GLOB_FAILCODE_I2C_TEMP_RANGE_ERR) */
/*lint -esym(769, GLOB_FAILCODE_I2C_TEMP_FORM_ERR) */
/*lint -esym(769, GLOB_FAILCODE_I2C_PTR_WRITE_CFG) */
/*lint -esym(769, GLOB_FAILCODE_I2C_PTR_WRITE_TMP) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT1) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT2) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT3) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT4) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT5) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT6) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT7) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT8) */
/*lint -esym(769, GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT9) */

/*! This enum provides all global failure codes */
typedef enum
{
   /*!< ROM Test failure. */
   GLOB_FAILCODE_ROM_TEST              = 0x830bu,
   /*!< RAM Test failure. */
   GLOB_FAILCODE_RAM_TEST              = GLOB_FAILCODE_RAM_TEST_ASM,
   /*!< March C Test failure, this test is executed at startup. */
   GLOB_FAILCODE_MARCHC_TEST           = 0x8557u,
   /*!< Stack Test failure, a stack over- or underflow occurred. */
   GLOB_FAILCODE_STACK_TEST            = 0x8679u,
   /*!< Opcode Test failure. */
   GLOB_FAILCODE_OPCODE_TEST           = GLOB_FAILCODE_OPCODE_TEST_ASM,
   /*!< SFR Test failure. */
   GLOB_FAILCODE_SFR_TEST              = 0x8878u,
   /*!< Core Register Test failure. */
   GLOB_FAILCODE_COREREG_TEST          = GLOB_FAILCODE_COREREG_TEST_ASM,
   /*!< Failure in the program flow of the selftests. */
   GLOB_FAILCODE_PFLOW_SIGNATURE       = 0x8ac1u,
   /*!< Safety Container Test failure. */
   GLOB_FAILCODE_SAFCON_CRC            = 0x8b56u,
   /*!< Timeout of the Safety Handler. */
   GLOB_FAILCODE_STHAN_TIMEOUT         = 0x8c9du,
   /*!< Failure in a safety relevant variable. */
   GLOB_FAILCODE_VARIABLE_ERR          = 0x8d0au,
   /*!< NonMaskable Interrupt failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_NMI                   = GLOB_FAILCODE_NMI_ASM,
   /*!< Hard fault failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_HARD_FAULT            = GLOB_FAILCODE_HARD_FAULT_ASM,
   /*!< Memory management fault failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_MEMMANAGE_FAULT       = GLOB_FAILCODE_MEMMANAGE_FAULT_ASM,
   /*!< Bus fault failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_BUS_FAULT             = GLOB_FAILCODE_BUS_FAULT_ASM,
   /*!< Usage fault failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_USAGE_FAULT           = GLOB_FAILCODE_USAGE_FAULT_ASM,
   /*!< Supervisor call failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_SVC                   = GLOB_FAILCODE_SVC_ASM,
   /*!< Debug monitor fault failure. */
   GLOB_FAILCODE_DEBUGMON_FAULT        = GLOB_FAILCODE_DEBUGMON_FAULT_ASM,
   /*!< PendSV failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_PENDSV                = GLOB_FAILCODE_PENDSV_ASM,
   /*!< SysTick failure, see STM32F100xx_Programming_Manual.pdf chapter 2.3.2. */
   GLOB_FAILCODE_SYSTICK               = GLOB_FAILCODE_SYSTICK_ASM,
   /*!< Module is not initialized failure. */
   GLOB_FAILCODE_MODULE_NOINIT         = 0x9754u,
   /*!< IPC synchronization failure, a timeout occurred. */
   GLOB_FAILCODE_IPCS_SYNC_TIMEOUT     = 0x98c2u,
   /*!< IPC checksum failure, the checksum of the received telegram is invalid. */
   GLOB_FAILCODE_IPCX_CRC              = 0x9955u,
   /*!< IPC Controller ID failure, the received controller ID is wrong. */
   GLOB_FAILCODE_IPCX_CONTROLLER_ID    = 0x9a7bu,
   /*!< IPC ID failure, the received IPC ID is not the same as excepted. */
   GLOB_FAILCODE_IPCX_IPC_ID           = 0x9becu,
   /*!< Failure in the IPC return code. */
   GLOB_FAILCODE_IPCX_RETURN_ERR       = 0x9c27u,
   /*!< IPC Timeout, expected IPC message not received. */
   GlOB_FAILCODE_IPCX_TIMEOUT          = 0x9db0u,
   /*!< Configuration mismatch between the two controllers. */
   GLOB_FAILCODE_INVALID_CONFIG        = 0x9e9eu,
   /*!< Failure in the argument of a function. */
   GLOB_FAILCODE_INVALID_PARAM         = 0x9f09u,
   /*!< Pointer failure. */
   GLOB_FAILCODE_POINTER_INVALID       = 0xa0c6u,
   /*!< ADC conversion timeout failure. */
   GLOB_FAILCODE_ADC_CONV_TIMEOUT      = 0xa151u,
   /*!< ADC calibration timeout failure. */
   GLOB_FAILCODE_ADC_CALIB_TIMEOUT     = 0xa27fu,
   /*!< Profisafe hard error. */
   GlOB_FAILCODE_PSD_HARD_ERR          = 0xa3e8u,
   /*!< iParameter failure. */
   GlOB_FAILCODE_IPAR_CRC_ERR          = 0xa423u,
   /*!< F-Parameter failure. */
   GlOB_FAILCODE_FPAR_CRC_ERR          = 0xa5b4u,
   /*!< Temperature failure */
   GlOB_FAILCODE_TEMPERATURE_ERR       = 0xa69au,
   /*!< AIC synchronisation failure */
   GlOB_FAILCODE_AIC_SYNC_ERR          = 0xa70du,
   /*!< DO Diag Timeout */
   GlOB_FAILCODE_DODIAG_ERR            = 0xa89bu,
   /*!< DI Diag Timeout */
   GlOB_FAILCODE_DIDIAG_ERR            = 0xa90cu,
   /*!< CLK CTRL Failure */
   GlOB_FAILCODE_CLK_CTRL              = 0xaa22u,
   /*!< Scheduler timeout failure. */
   GLOB_FAILCODE_SCHEDULER_TIMEOUT     = 0xabb5u,
   /*!< Main loop was not executed within defined time. */
   GLOB_FAILCODE_MAIN_TIMEOUT          = 0xac7eu,
   /*!< No Failure */
   GLOB_FAILCODE_NO_FAILURE            = 0xade9u,

   /*!< Background task exceeded its max. delay. */
   GLOB_FAILCODE_BKGD_DELAY_ERR        = 0xaec7u,
   /*!< Interrupt context not called any more while running background task */
   GLOB_FAILCODE_BKGD_IRQ_MONITOR_ERR  = 0xaf50u,
   /*!< Critical/Fatal error of CIP Safety Software Stack */
   GLOB_FAILCODE_CSS_FATAL             = 0xb07cu,
   /*!< Soft Error during check detected */
   GLOB_FAILCODE_CSS_SOFTVAR_ERR       = 0xb1ebu,
   /*!< Mismatch in number of function calls */
   GLOB_FAILCODE_CSS_PFLOW_ERR         = 0xb2c5u,
   /*!< Application Timer exceeded the max. allowed deviation */
   GLOB_FAILCODE_APPL_TIMER_DEV_ERR    = 0xb352u,
   /*!< Error while while NV memory access */
   GLOB_FAILCODE_NV_MEMORY_ERR         = 0xb499u,
   /*!< Invalid Flash address access */
   GLOB_FAILCODE_NV_ADDRESS_ERR        = 0xb50eu,
   /*!< Error while high level NV memory access */
   GLOB_FAILCODE_NV_APPL_ERR           = 0xb620u,
   /*!< Invalid transition of global state machine */
   GLOB_FAILCODE_INVALID_TRANSITION    = 0xb7b7u,
   /*!< Max. number of events exceeded */
   GLOB_FAILCODE_EVENT_CNT_ERR         = 0xb821u,
   /*!< An unexpected program flow was detected. */
   GLOB_FAILCODE_UNEXP_PFLOW_ERR       = 0xb9b6u,
   /*!< Failure in a safety relevant variable. */
   GLOB_FAILCODE_VARIABLE_ERR_EX       = 0xba98u,
   /*!< Failure in the argument of a function. */
   GLOB_FAILCODE_INVALID_PARAM_EX      = 0xbb0fu,
   /*!< Pointer failure. */
   GLOB_FAILCODE_POINTER_INVALID_EX    = 0xbcc4u,

   /*!< I2C initialization cannot detect TMP102. */
   GLOB_FAILCODE_I2C_TMP102_PRESENCE   = 0xbd53u,
   /*!< I2C software state machine is in an invalid state */
   GLOB_FAILCODE_I2C_STATE_ERR         = 0xbe7du,
   /*!< I2C: value read from configuration register is invalid */
   GLOB_FAILCODE_I2C_CONFIG_ERR        = 0xbfeau,
   /*!< I2C: measured temperature not within valid range */
   GLOB_FAILCODE_I2C_TEMP_RANGE_ERR    = 0xc074u,
   /*!< I2C: measured temperature has invalid format */
   GLOB_FAILCODE_I2C_TEMP_FORM_ERR     = 0xc1e3u,
   /*!< I2C: Error when setting pointer Register to CFG. */
   GLOB_FAILCODE_I2C_PTR_WRITE_CFG     = 0xc2cdu,
   /*!< I2C Error when setting pointer Register to TMP. */
   GLOB_FAILCODE_I2C_PTR_WRITE_TMP     = 0xc35au,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT1  = 0xc491u,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT2  = 0xc506u,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT3  = 0xc628u,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT4  = 0xc7bfu,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT5  = 0xc829u,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT6  = 0xc9beu,
   /*!< I2C Status bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT7  = 0xca90u,
   /*!< I2C Control bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT8  = 0xcb07u,
   /*!< I2C Control bit timeout failure. */
   GLOB_FAILCODE_I2C_BITWAIT_TIMEOUT9  = 0xccccu
} GLOB_FAILCODE_ENUM;


/*! This enum provides all global return codes. */
typedef enum
{
   GLOB_RET_OK                         = 0x75fdu,   /*!< No error. */
   GLOB_RET_ERR                        = 0x76d3u,   /*!< Common error. */
   GLOB_RET_BUSY                       = 0x7744u,   /*!< Module is busy, no error. */
   GLOB_RET_TEMPERATURE_ERR            = 0x78d2u    /*!< Temperature error. */
} GLOB_RETCODE_ENUM;


#endif



/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/



#endif /* GLOBFAIL_ERRORCODES_H */

/*******************************************************************************
**
** End of globFail_Errorcodes.h
**
********************************************************************************
*/

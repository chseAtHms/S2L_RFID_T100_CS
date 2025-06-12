/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globFit_FitTestHandler.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globFit_FitTestHandler.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        This module provides the FIT-Test Handler.
 * @description:  With the FIT-Test handler module the FITs will be handled.
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
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "cfg_Config-sys.h"
#include "serial-hal.h"
#include "gpio-hal.h"
#include "sch_Scheduler-app.h"
#include "romtst_RomTest_cfg.h"

/* Header-file of module */
#include "globFit_FitTestHandler.h"
#include "globFit_FitTestHandler_loc.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

#ifdef GLOBFIT_FITTEST_ACTIVE

#warning "FIT Tests active!"

/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/


/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

/* This variables contains the FIT-Test command. It is mapped to a fixed address
 * in the RAM.
 * The variable have to be on a defined RAM address to exclude it on the startup
 * RAM test. This test overwrites all values in the RAM. So, the FIT command
 * which performs a failure in the startup RAM test will be overwritten. */
UINT8 globFit_u8FitCommand __attribute__((at(GLOBFIT_LOCATION_OF_FITCMD_VAR_RAM))) = 0u;

/*! Type definition for function pointer. */
typedef  void (*globFit_Function)(void);
/*! Function pointer to call the application. */
static globFit_Function globFit_JumpToApplStart;

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
@Name                globFit_SetMSP

@Description         Set the Main Stack Pointer.

@note                This function is a 1:1 copy from the core_cm3 module.

@Parameter
   @param[in]        u32MainStackPointer
   @return           void
 */
/******************************************************************************/
__ASM void globFit_SetMSP (uint32_t u32MainStackPointer)
{
  msr msp, r0
  bx lr
/* RSM_IGNORE_QUALITY_BEGIN Notice #48 - Function lLOC <= 0, non-operational function */
}
/* RSM_IGNORE_END */


/******************************************************************************/
/*!
@Name                globFit_Init

@Description         This function initialize the FIT-Test.

@note                The FIT-Test Handler uses the URART3 interface.

@Parameter
   @return           void
 */
/******************************************************************************/
void globFit_Init (void)
{
   /* check if reset source was the wwdg timeout or a pin reset */
   if (  ((RCC->CSR & RCC_CSR_PORRSTF) == RCC_CSR_PORRSTF)
      || ((RCC->CSR & RCC_CSR_PINRSTF) == RCC_CSR_PINRSTF))
   {
      /* clear reset source */
      RCC->CSR |= RCC_CSR_RMVF;

      /* clear FIT command */
      globFit_u8FitCommand = GLOBFIT_CMD_NONE;
   }

   /* activate clock of the GPIOB */
   RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

   /* configure the GPIO Pin PB10 for USART Tx */
   gpioHAL_ConfigureOutput( GPIOB,
                            GPIO_10,
                            OUT_CONFIG_AF_PP,
                            OUT_MODE_50MHZ );

   /* configure the GPIO Pin PB11 for USART Rx */
   gpioHAL_ConfigureInput( GPIOB,
                           GPIO_11,
                           IN_CONFIG_FLOATING );

   /* init the UART */
   serialHAL_Init( USART3,
                   SERIALHAL_115200_BAUD,
                   SERIALHAL_USART_SET_8NOP1);

}


/******************************************************************************/
/*!
@Name                globFit_FitTestHandler

@Description         This function is the FIT-Test-handler.

@note                The FIT-Test Handler checks if a new FIT-CMD has been
                     received for this controller. If that is the case the
                     FIT-CMD will execute a specified FIT-action.
                     In case of some specific FIT commands, a restart of the
                     firmware is neccasary. So, there is a jump to the start
                     address of the firmware.
                     PLEASE CONSIDER: There are problems with the window
                     watchdog. It cannot be disabled. If the wwdg is active
                     there can be a reset of the controller after restart.

@Parameter
   @return           void
 */
/******************************************************************************/
void globFit_FitTestHandler (void)
{
   UINT32 u32JumpAddress;

   if (globFit_u8FitCommand == GLOBFIT_CMD_NONE)
   {
      /* get the FIT CMD */
      globFit_u8FitCommand = globFit_GetFitCmd();
   }

   /* if the FIT command is for the startup tests or for reset, wait for the
    * wdg-reset in an infinite loop */
   if (  (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RAMTEST_STARTUP_ERR1)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RAMTEST_STARTUP_ERR2)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RAMTEST_STARTUP_ERR3)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RAMTEST_STARTUP_ERR4)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RAMTEST_STARTUP_ERR5)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_ROM_TEST_STARTUP)
      || (globFit_u8FitCommand == (UINT8)GLOBFIT_CMD_RESET_MCU))
   {
      /* waiting for wdg-reset */
      schAPP_StopScheduler();

      /* Get jump address to application */
      u32JumpAddress = *(__IO UINT32*) (ROMTST_ROM_START + 4);
      globFit_JumpToApplStart = (globFit_Function) u32JumpAddress;

      /* Initialize user application's Stack Pointer */
      globFit_SetMSP(*(__IO UINT32*) ROMTST_ROM_START);

      /* Jump to application */
      globFit_JumpToApplStart();
   }
}


/******************************************************************************/
/*!
@Name                globFit_GetFitCmd

@Description         This function looks if a new FIT CMD is available on the
                     USART interface and returns the command if it is for this
                     controller.

@note                -

@Parameter
   @return           u8Byte      Specified FIT CMD or GLOBFIT_CMD_NONE
 */
/******************************************************************************/
UINT8 globFit_GetFitCmd (void)
{
   UINT8 u8Byte = GLOBFIT_CMD_NONE;

   /* check if new data available */
   if(serialHAL_IsReadyToRead(USART3))
   {
      /* read the data from the serial interface */
      u8Byte = serialHAL_ReadFromUsartDR(USART3);

   }

   return u8Byte;
}


/******************************************************************************/
/*!
@Name                globFit_SendByte

@Description         This function send a byte over the USART interface.

@note                -

@Parameter
   @param [in]       u8Byte      Byte to send
   @return           eRet        GLOB_RET_OK    - OK
                                 GLOB_RET_BUSY  - USART is busy, TX-buffer is not empty
                                 GLOB_RET_ERR   - Error
 */
/******************************************************************************/
GLOB_RETCODE_ENUM globFit_SendByte (CONST UINT8 u8Byte)
{
   return serialHAL_WriteToUsartDR (USART3, u8Byte);
}
#endif /* GLOBFIT_FITTEST_ACTIVE */

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

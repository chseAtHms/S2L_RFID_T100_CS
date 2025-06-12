/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globFail_SafetyHandler.c
**
** $Id: globFail_SafetyHandler.c 3972 2023-03-24 14:06:49Z ankr $
** $Revision: 3972 $
** $Date: 2023-03-24 15:06:49 +0100 (Fr, 24 Mrz 2023) $
** $Author: ankr $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
** This module provides the safety handler.
** The safety handler module provides the safe state. The safe state contains
** of a function which ends in an infinite loop which triggers the watchdog.
** The only exit of this function is via a power up. Furthermore this module
** provides a SAFETY-ASSERT and a SAFETY-FAIL macro. The SAFETY-ASSERT contains
** a condition which is used for checking something. If the condition is false
** the safety handler is called with the specified fail code. The SAFETY-FAIL
** switches directly to the safety handler.
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
/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "aicMsgDef.h"
#include "aicCrc.h"
#include "aicMsgCtrlSts.h"
#include "aicMsgSis.h"

#ifdef GLOBFIT_FITTEST_ACTIVE
#include "globFit_FitTestHandler.h"
#include "cfg_Config-sys.h"
#include "gpio-hal.h"
#endif

/* Header-file of module */
#include "globFail_SafetyHandler.h"
#include "globFail_SafetyHandler_loc.h"


/* The __return_address intrinsic returns the value of the link register that is 
** used in returning from the current function.
** Since 'built-in' no header is available. So forward declaration is done 
** for PC-Lint here.
** Supress PC-Lint Warnings:
** Note 970: Use of modifier or type 'unsigned' outside of a typedef [MISRA 2004 Rule 6.3]
** Note 970: Use of modifier or type 'int' outside of a typedef [MISRA 2004 Rule 6.3]
** Note 960: Violates MISRA 2004 Required Rule 20.2, 
** Re-use of C90 identifier pattern: __return_address
** 
*/
extern unsigned int __return_address(void); /*lint !e970, !e960 */

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/* globFail_u32_AddInfo
** Additional Info. This Additional Info is also stored inside Flash
** just after the actual error code. Default 0. (see [SRS_2233])
** Format in flash:  FailCode (2 byte), AddInfo (4byte).  
*/
volatile UINT32 globFail_u32_AddInfo = 0u;

/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** globFail_SafetyHandler()
**
** Description:
** This function is the safety handler, means the realization of the
** FAIL_SAFE state, see [SRS_2001].
** If the safety handler is received, the following process is done:
** 1. Disable scheduler.
** 2. Disable all interrupts
** 3. Disable all outputs.
** ...
** n. Stay into the infinite loop and trigger the watchdog. Do nothing except
** guru meditation.
**
**------------------------------------------------------------------------------
** Inputs:
**    eFailCode      Specified failcode which should be signaled.
**
** Outputs:
**    UINT8          Return of 0, this is necessary for the RDS macros.
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
UINT8 globFail_SafetyHandler (GLOB_FAILCODE_ENUM eFailCode)
{
   AICMSGDEF_ASM_TELEGRAM_STRUCT sAicErrorTgm;
   UINT8 u8Index;
   UINT32 u32_returnAddress;

#ifndef __DEBUG__
#if (!defined(__CTC_TESTING_ON__)) && (!defined(GLOBFIT_FITTEST_ACTIVE))
   UINT32 u32_failureBlkAddr;
   UINT32 u32_wrAddr;
#endif /* (!defined(__CTC_TESTING_ON__)) && (!defined(GLOBFIT_FITTEST_ACTIVE)) */
  
#endif /* NDEBUG */

#ifdef GLOBFIT_FITTEST_ACTIVE
   CFG_CONTROLLER_ID_ENUM eControllerId;
   UINT8 *pu8Byte = (UINT8*) &eFailCode;
   UINT8 u8SendCnt = 0u;
#endif

   /* trigger watchdog */
   GLOBFAIL_TRIGGER_WWDG

   /* disable all interrupts, see [SRS_2236] */
   __disable_irq();

   /* disable Scheduler */
   TIM1->CR1 &= ((UINT16)~TIM_CR1_CEN);


   /* disable DO, see [SRS_594] */
   GPIOB->ODR &= ~GPIO_ODR_ODR5;

   /* disable discharge */
   GPIOC->ODR |= GPIO_ODR_ODR15;

   /* disable life signal, see [SRS_597] */
   GPIOB->ODR &= ~GPIO_ODR_ODR8;

   /* disable ENx_TO signal (low active), see [SRS_712] */
   GPIOC->ODR |= GPIO_ODR_ODR8;
   
   /* returns the value of the link register (lr) that is used to return from the
   ** current function: This is, of course, never done for the safety handler */
   u32_returnAddress = __return_address();
   /* Since all ARM instructions will align themselves on either a 32- or 16-bit boundary, 
   ** the LSB of the address is not used in the branch directly. 
   ** The LSB is used to differ between ARM state (LSB=0) and Thumb state (LSB=1). 
   ** This is not relevant here, so cleared */
   u32_returnAddress &= 0xFFFFFFFEu;
   /* __return_address delivers the address which should be executed after return (address
   ** of next instruction). Since we want the branch to safety handler here, we decrement by 4
   ** here */
   /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number
   ** accepted, because low level functionality */
   u32_returnAddress -= 4u;
   /* RSM_IGNORE_END */
   
   /* if no Additional Info provided, we take the return address */
   if (globFail_u32_AddInfo == 0u)
   {
     globFail_u32_AddInfo = u32_returnAddress;
   }
   /* else Additional Info already available */
   else
   {
     /* nothing to do */
   }

/*------------------------------------------------------------------------------
 * This section creates the error message and sends it out via the aic.
------------------------------------------------------------------------------*/
   /* check for last ASM/T100 fragmentation bit and toggle this bit */
   if (aicMsgCtrlSts_u8_AsmCtrlSts & AICMSGHDL_MASK_ASM_FRAGFLAG)
   {
      /* toggle ASM fragmentation bit: was 1, set to 0 */
      aicMsgCtrlSts_u8_AsmCtrlSts &= (~AICMSGHDL_MASK_ASM_FRAGFLAG);
   }
   else
   {
      /* toggle ASM fragmentation bit: was 0, set to 1 */
      aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_MASK_ASM_FRAGFLAG;
   }

   /* prepare message "FatalErrorEntry" (see [SRS_684], [SIS_066]) write telegram definitions
   ** to the buffer */
   /* Ctrl/Status, (see [SRS_2022]) */
   sAicErrorTgm.u8CtrlStatus = (aicMsgCtrlSts_u8_AsmCtrlSts | AICMSGHDL_ASM_STATE_ERROR);
   /* message header, see [SIS_053] */
   /* Msg Id, ASM shall not support multiple requests, set message ID to 0, see [SIS_028] */
   sAicErrorTgm.sAsmMsg.sAicMsgHeader.u8MsgId = 0x00u;
   /* Msg Command, includes CMD bit, [SIS_031] */
   sAicErrorTgm.sAsmMsg.sAicMsgHeader.u16MsgReqResp = AICMSGHDL_ASM_SEND_FATAL_ERR_REQ;
   /* Msg Length */
   sAicErrorTgm.sAsmMsg.sAicMsgHeader.u8MsgLength = AICMSGHDL_ASM_SEND_FATAL_ERR_REQ_LENGTH;
   /* fill Message Data Buffer with zeros */
   for (u8Index = 0u; u8Index < AICMSGDEF_ASM_MSG_DATA_SIZE; u8Index++)
   {
      sAicErrorTgm.sAsmMsg.au8AicMsgData[u8Index] = 0u;
   }
   /* set error code afterwards */
   /* Msg error code */
   sAicErrorTgm.sAsmMsg.au8AicMsgData[0] = (UINT8)(((UINT16)eFailCode) & 0x00FFu);         
   sAicErrorTgm.sAsmMsg.au8AicMsgData[1] = (UINT8)((((UINT16)eFailCode) & 0xFF00u) >> 8u);
   
   
   /* prepare SPDU part as no IO Connection is opened (see [SIS_212], [SIS_213]) */
   /* IO Data Message */
   sAicErrorTgm.sSpdu.u16_halcLen      = AICMSGDEF_CIP_IN_DATA_SIZE;
   sAicErrorTgm.sSpdu.u16_halcAddInfo  = 0xFFFFu; 
   for (u8Index = 0u; u8Index < AICMSGDEF_CIP_IN_DATA_SIZE; u8Index++)
   {
      sAicErrorTgm.sSpdu.au8_data[u8Index] = 0xFEu;
   }
   /* Time Coordination Message */
   sAicErrorTgm.sSpdu.u16_halcAddInfo2 = 0xFFFFu;
   for (u8Index = 0u; u8Index < AICMSGDEF_CIP_IN_DATA2_SIZE; u8Index++)
   {
      sAicErrorTgm.sSpdu.au8_data2[u8Index] = 0xFEu;
   }
   
   /* prepare DUI, not evaluated by ABCC, so set to fix value */
   sAicErrorTgm.sSpdu.u8_duiData  = 0u;
   sAicErrorTgm.sSpdu.u8_duiData2 = 0u;
   
   /* pepare non-safe IO part INPUT/OUTPUT */
   sAicErrorTgm.sIoInData.u8DiData       = 0u;
   sAicErrorTgm.sIoInData.u8DiQualifiers = 0u;
   sAicErrorTgm.sIoInData.u8DoQualifiers = 0u;
   sAicErrorTgm.sIoOutData.u8DoVal       = 0u;
   sAicErrorTgm.sIoOutData.u8DiErrRst    = 0u;
   sAicErrorTgm.sIoOutData.u8DoErrRst    = 0u;
   
   /* calculate crc */
   /* Lint message disabled because pointer is needed for CRC calculation */
   sAicErrorTgm.u16Crc = aicCrc_Crc16( (CONST UINT8*)&sAicErrorTgm, /*lint !e928*/
                                       (UINT32)(AICMSGDEF_ASM_TGM_SIZE - AICMSGDEF_ASM_CRC_SIZE));

   /* send tgm via AIC */
   DMA2_Channel5->CCR &= ~DMA_CCR5_EN;
   DMA2_Channel5->CNDTR = AICMSGDEF_ASM_TGM_SIZE;
   DMA2_Channel5->CMAR = (UINT32) &sAicErrorTgm;/*lint !e923*/
   DMA2_Channel5->CCR |= DMA_CCR5_EN;
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

/*------------------------------------------------------------------------------
 * This section writes the failcode to the specified flash area which is for
 * saving the failcodes. Write the failcode to the next available address.
 * Do this as long as there are some free space.
 * This section is deactive in the following modes:
 * - Debug mode
 * - FIT Tests active
 * In these modes this section is deactive to protect the flash for unneeded
 * write accesses.
-----------------------------------------------------------------------------*/
#ifndef __DEBUG__
#if (!defined(__CTC_TESTING_ON__)) && (!defined(GLOBFIT_FITTEST_ACTIVE))

   /* Unlock Flash */
   FLASH->KEYR = GLOBFAIL_FLASH_KEY1;
   FLASH->KEYR = GLOBFAIL_FLASH_KEY2;

   /* start programming */
   FLASH->CR |= FLASH_CR_PG;

   /* write the failcode to the next available address, see [SRS_2233] */
   /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number */
   for ( u32_failureBlkAddr = GLOBFAIL_FAILCODE_PAGE_STRT_ADDR;
         u32_failureBlkAddr < GLOBFAIL_FAILCODE_PAGE_END_ADDR;
         u32_failureBlkAddr += 6u) /* size of 'eFailCode' + 'globFail_u32_AddInfo' */
   /* RSM_IGNORE__END */
   {
       /* trigger watchdog to ensure no HW reset occurs during a 'long' search here */
       GLOBFAIL_TRIGGER_WWDG
     
       /* if flash location is empty */
       if (*(__IO UINT16*)u32_failureBlkAddr == 0xFFFFu)/*lint !e923*/
       {
          /* set current flash location to write */
          u32_wrAddr = u32_failureBlkAddr;
           
          /* write the failcode to the flash, see [SRS_2233] */
          /* Lint message disabled because the value of u32_wrAddr is casted to a flash address to
           * have an access to the flash for writing the failcode to the flash memory. */
          /*lint -e(923)*/
          *(__IO UINT16*)u32_wrAddr = (UINT16)eFailCode;
          
          /* the additional info (low word) is stored just after eFailCode in flash, 
           * see [SRS_2233] */
          /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number
          ** accepted, because low level functionality */
          u32_wrAddr += 2u;
          /* RSM_IGNORE_END */
          /* if still a valid flash address is available */
          if (u32_wrAddr < GLOBFAIL_FAILCODE_PAGE_END_ADDR)
          {
            /* write the low word of additional info to the flash */
            /* Lint message disabled because the value of u32_wrAddr is casted to a flash address to
             * have an access to the flash for writing the failcode to the flash memory. */
            /*lint -e(923)*/
            *(__IO UINT16*)u32_wrAddr = (UINT16)(globFail_u32_AddInfo & 0x0000FFFFu);
          }
          /* else: flash location not empty */
          else
          {
            /* empty branch */
          }
          
          /* address of high word */
          /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number
          ** accepted, because low level functionality */
          u32_wrAddr += 2u;
          /* RSM_IGNORE_END */
          /* if still a valid flash address is available */
          if (u32_wrAddr < GLOBFAIL_FAILCODE_PAGE_END_ADDR)
          {
            /* write the high word of additional info to the flash */
            /* Lint message disabled because the value of u32_wrAddr is casted to a flash address to
             * have an access to the flash for writing the failcode to the flash memory. */
            /*lint -e(923)*/
            *(__IO UINT16*)u32_wrAddr = (UINT16)((globFail_u32_AddInfo & 0xFFFF0000u) >> 16);
          }
          /* else: flash location not empty */
          else
          {
            /* empty branch */
          
          }
          /* RSM_IGNORE_QUALITY_BEGIN Notice #44 - Keyword 'break' identified outside a 'switch'
          ** structure */
          break;
          /* RSM_IGNORE_QUALITY_END */
       }
       /* else: flash location not empty */
       else
       {
         /* empty branch */
       }
   }

   /* Disable the PG Bit */
   FLASH->CR &= ~FLASH_CR_PG;

   /* Lock the flash */
   FLASH->CR |= FLASH_CR_LOCK;

#endif /* (!defined(__CTC_TESTING_ON__)) && (!defined(GLOBFIT_FITTEST_ACTIVE)) */
#endif /* NDEBUG */


#ifdef GLOBFIT_FITTEST_ACTIVE
   /* check if reset source was the wwdg timeout */
   if ((RCC->CSR & RCC_CSR_WWDGRSTF) == RCC_CSR_WWDGRSTF)
   {
      /* Initialize the FIT test with the USART interface after a system reset,
       * and get the controller ID.
       */
      gpioHAL_Init();
      cfgSYS_Init();      /* read amongst others the controller ID */
      globFit_Init();
   }
   else
   {
     /* empty branch */
   }

   eControllerId = cfgSYS_GetControllerID();

   /* send Controller ID to identify the correct uC */
   (void)globFit_SendByte((UINT8)eControllerId);

   u8SendCnt = (UINT8)sizeof(GLOB_FAILCODE_ENUM);
#endif

   /* clear counter to send error message in 4000u steps later inside the
    * Fail-Safe endless loop */
   TIM3->CNT = 0u;

/*------------------------------------------------------------------------------
 * Infinite loop (see [SRS_507]), this is the safe state.
------------------------------------------------------------------------------*/
   while (FOREVER)/*lint !e716*//* Info 716: while(1) */
   {

      /* infinite loop, do not do anything except trigger the watchdog [SRS_598]
       * and trigger sending of the DMA content cyclically */
      GLOBFAIL_TRIGGER_WWDG  /* [SRS_598] */

      /* inside the endless-loop, timer 3 (in run mode the system timer together
       * with Timer 2) is used to trigger cyclically the sending of the current
       * DMA-Buffer. Result is that every 4ms the Fail-Safe entry message is
       * repeated (see [SRS_2237]), means no standard communication any more
       * (see [SRS_2010], [SRS_2238]). */
      if ( (TIM3->CNT) >= GLOBFAIL_MSG_CYCLE_TIME)
      {
         /* clear the counter */
         TIM3->CNT = 0u;
         /* send tgm via AIC */
         DMA2_Channel5->CCR &= ~DMA_CCR5_EN;
         DMA2_Channel5->CNDTR = AICMSGDEF_ASM_TGM_SIZE;
         DMA2_Channel5->CMAR = (UINT32) &sAicErrorTgm;/*lint !e923*/
         DMA2_Channel5->CCR |= DMA_CCR5_EN;
      }
      /* else: still time until next Fail-Safe entry message shall be sent */
      else
      {
        /* empty branch */
      }

#ifdef GLOBFIT_FITTEST_ACTIVE
      if (u8SendCnt > 0u)
      {
         if (GLOB_RET_OK == globFit_SendByte(*(pu8Byte + (u8SendCnt - 1u))))
         {
            u8SendCnt--;
         }
         else
         {
           /* empty branch */
         }
      }
      else
      {
        /* empty branch */
      }

      /* check if the reset command is received */
      if(GLOBFIT_CMD_RESET_MCU == globFit_GetFitCmd())
      {
         /* go into an infinite loop without watchdog trigger, so this initiates
          * a reset */
         while (FOREVER)
         {
             ;
         }
      }
      /* else: reset command not received */
      else
      {
        /* empty branch */
      }
#endif

   } /* while (FOREVER) */

   /* Warning 527: Unreachable code at token 'return' [MISRA 2004 Rule 14.1] */
   return 0u;/*lint !e527*/
/* This function has a lot of paths. Inside safety handler no other function calls
** wanted, so complexity is accepted here. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of globFail_SafetyHandler.c
**
********************************************************************************
*/

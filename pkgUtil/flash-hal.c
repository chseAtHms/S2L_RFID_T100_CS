/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: flash-hal.c
**     Summary: This module is the flash hardware abstraction layer.
**              Sample code for the flash controller are taken from the STM32F10x Standard 
**              Peripherals Library (V3.3.0, stm32f10x_flash.c)
**   $Revision: 2040 $
**       $Date: 2017-02-07 08:40:11 +0100 (Di, 07 Feb 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: flashHAL_Unlock
**             flashHAL_Lock
**             flashHAL_ErasePage
**             flashHAL_ProgramHalfWord
**
**             GetBank1Status
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* application includes */
#include "flash-hal.h"
#include "flash-hal_cfg.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID                        32u
      
/* FLASH Keys */      
#define FLASH_KEY1                      ((UINT32)0x45670123uL)
#define FLASH_KEY2                      ((UINT32)0xCDEF89ABuL)

/* macro checks if address is word aligned */
#define IS_WORD_ALIGNED(ADDRESS)        (((ADDRESS) & 0x0000001u) == 0u)

/* macro checks if address is in valid user flash area and word aligned */
#define IS_VALID_FLASH_ADDRESS(ADDRESS) ( ((ADDRESS) >= FLASHHAL_cfg_USR_FLASH_START) && \
                                          ((ADDRESS) <= FLASHHAL_cfg_USR_FLASH_END)   && \
                                          (IS_WORD_ALIGNED(ADDRESS)) )


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC FLASHHAL_t_STATUS GetBank1Status (void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    flashHAL_Unlock

  Description:
    Unlocks the FLASH Program Erase Controller.

  See also:
    -
    
  Parameters:
    - 

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
void flashHAL_Unlock(void)
{
  /* Authorize the FPEC of Bank1 Access */
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;
}

/***************************************************************************************************
  Function:
    flashHAL_Lock

  Description:
    Locks the FLASH Program Erase Controller.

  See also:
    -
    
  Parameters:
    - 

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
void flashHAL_Lock(void)
{
  /* Set the Lock Bit to lock the FPEC and the CR of  Bank1 */
  FLASH->CR |= FLASH_CR_LOCK;
}

/***************************************************************************************************
  Function:
    flashHAL_ErasePage

  Description:
    Erases a specified FLASH page.

  See also:
    -
    
  Parameters:
    u32_pageAddress (IN)     - The page address to be erased
                               (valid: FLASHHAL_cfg_USR_FLASH_START..FLASHHAL_cfg_USR_FLASH_END 
                               and word aligned, checked)

  Return value:
    FLASHHAL_t_STATUS        - Possible values: FLASHHAL_k_BUSY, FLASHHAL_k_ERROR_PG,
                               FLASHHAL_k_ERROR_WRP, FLASHHAL_k_COMPLETE

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
FLASHHAL_t_STATUS flashHAL_ErasePage (UINT32 u32_pageAddress)
{
  FLASHHAL_t_STATUS e_flashStatus;
  
  /* check that valid flash address was passed and that the flash was unlocked previously */
  if ( !IS_VALID_FLASH_ADDRESS(u32_pageAddress) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_ADDRESS_ERR, GLOBFAIL_ADDINFO_FILE(1u));
    /* set return value (only used for unit tests) */
    e_flashStatus = FLASHHAL_k_ERROR_PG;
  }
   /* else if: flash controller not unlocked previously */
  else if ( (FLASH->CR & FLASH_CR_LOCK) != 0u )
  {
    /* set return value */
    e_flashStatus = FLASHHAL_k_ERROR_WRP;
  }
  else
  {
    /* Wait for last operation to be completed */
    e_flashStatus = GetBank1Status();
    
    /* if no flash operation is pending */
    if(e_flashStatus == FLASHHAL_k_COMPLETE)
    { 
      /* if the previous operation is completed, proceed to erase the page */
      FLASH->CR|= FLASH_CR_PER;
      FLASH->AR = u32_pageAddress; 
      FLASH->CR|= FLASH_CR_STRT;
      
      /* Wait for last operation to be completed */
      e_flashStatus = GetBank1Status();
      /* disable the PER Bit again */
      FLASH->CR &= ~FLASH_CR_PER;
    }
    /* else: previous operation not completed yet */
    else
    {
      /* empty branch, result already set in 'e_flashStatus' */
    }
  }

  /* Return the Erase Status */
  return e_flashStatus;
}

/***************************************************************************************************
  Function:
    flashHAL_ProgramHalfWord

  Description:
    Programs a half word at a specified address.

  See also:
    -
    
  Parameters:
    u32_address (IN)         - Specifies the address to be programmed.
                               (valid: FLASHHAL_cfg_USR_FLASH_START..FLASHHAL_cfg_USR_FLASH_END 
                               and word aligned, checked)
    u16_data (IN)            - Specifies the data to be programmed.
                               (valid: any, not checked)

  Return value:
    FLASHHAL_t_STATUS        - Possible values: FLASHHAL_k_BUSY, FLASHHAL_k_ERROR_PG,
                               FLASHHAL_k_ERROR_WRP, FLASHHAL_k_COMPLETE

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
FLASHHAL_t_STATUS flashHAL_ProgramHalfWord(UINT32 u32_address, UINT16 u16_data)
{
  FLASHHAL_t_STATUS e_flashStatus;
  
  /* if invalid or unaligned flash address was passed */
  if ( !IS_VALID_FLASH_ADDRESS(u32_address) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_ADDRESS_ERR, GLOBFAIL_ADDINFO_FILE(2u));
    /* set return value (only used for unit tests) */
    e_flashStatus = FLASHHAL_k_ERROR_PG;
  }
  /* else if: flash controller not unlocked previously */
  else if ( (FLASH->CR & FLASH_CR_LOCK) != 0u )
  {
    /* set return value */
    e_flashStatus = FLASHHAL_k_ERROR_WRP;
  }
  /* else: everything is fine */
  else
  {
    /* Wait for last operation to be completed */
    e_flashStatus = GetBank1Status();
    /* if no flash operation is pending */
    if(e_flashStatus == FLASHHAL_k_COMPLETE)
    {
      /* if the previous operation is completed, proceed to program the new data */
      FLASH->CR |= FLASH_CR_PG;
    
      *(__IO uint16_t*)u32_address = u16_data; /*lint !e923 : cast from unsigned int to
      pointer [Encompasses MISRA 2004 Rule 11.1], [MISRA 2004 Rule 11.3] */
      
      /* Wait for last operation to be completed */
      e_flashStatus = GetBank1Status();
      
      /* and disable the PG Bit again */
      FLASH->CR &= ~FLASH_CR_PG;
    }
    /* else: previous operation not completed yet */
    else
    {
      /* empty branch, result already set in 'e_flashStatus' */
    }
  }
  
  /* Return the Program Status */
  return e_flashStatus;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    GetBank1Status

  Description:
    This function returns the FLASH Bank1 Status. It is typically used to check the status of a 
    flash operation.

  See also:
    -
    
  Parameters:
    -

  Return value:
    FLASHHAL_t_STATUS        - Possible values: FLASHHAL_k_BUSY, FLASHHAL_k_ERROR_PG,
                               FLASHHAL_k_ERROR_WRP, FLASHHAL_k_COMPLETE

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
STATIC FLASHHAL_t_STATUS GetBank1Status (void)
{
  FLASHHAL_t_STATUS e_flashStatus;
  
  /* FLASH BANK1 Busy flag */
  if((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY) 
  {
    /** Attention: The CPU stalls until an ongoing Flash memory access is over.
    ** Since the program code is running from ROM this means that no OP code is fetched 
    ** during programming. In consequence this block (and BSY flag) shall not be reachable, 
    ** so the global safety handler is called here. */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(3u));
    
    e_flashStatus = FLASHHAL_k_BUSY;
  } /* CCT_SKIP */ /* unexpected program flow, secured by calling safety handler */
  else 
  { 
    /* FLASH BANK1 Program error flag */    
    if((FLASH->SR & FLASH_SR_PGERR) != 0u)
    { 
      e_flashStatus = FLASHHAL_k_ERROR_PG;
      /* reset PGERR (program error flag) by writing */
      FLASH->SR |= FLASH_SR_PGERR;
    }
    else 
    {
      /* FLASH BANK1 Write protected error flag */
      if((FLASH->SR & FLASH_SR_WRPRTERR) != 0u)
      {
        e_flashStatus = FLASHHAL_k_ERROR_WRP;
        /* reset WRPRTERR (write protection error) by writing */
        FLASH->SR |= FLASH_SR_WRPRTERR;
        
        /** Attention: The memory protection feature of the flash controller is not used 
        ** in this application. So the write protected error flags should be never set. */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(4u));
      }
      else
      {
        e_flashStatus = FLASHHAL_k_COMPLETE;
      }
    }
  }
  /* Return the Flash Status */
  return e_flashStatus;
}

/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: flash-hal.h
**     Summary: Header of the flash module.
**   $Revision: 2062 $
**       $Date: 2017-02-07 14:57:25 +0100 (Di, 07 Feb 2017) $
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
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef FLASH_HAL_H
#define FLASH_HAL_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/* FLASHHAL_t_STATUS
** Values:
** - FLASHHAL_k_BUSY: 
**   Busy - This indicates that a Flash operation is in progress. This is set on the beginning of a
**   Flash operation and reset when the operation finishes or when an error occurs.
** - FLASHHAL_k_ERROR_PG:
**   Programming error - Set by hardware when an address to be programmed contains a value 
**   different from '0xFFFF' before programming.
** - FLASHHAL_k_ERROR_WRP:
**   Write protection error - Set by hardware when programming a write-protected address of the 
**   Flash memory.
** - FLASHHAL_k_COMPLETE:
**   End of operation - Set by hardware when a Flash operation (programming / erase) is completed.
*/
typedef enum
{
  FLASHHAL_k_BUSY                     = 0xd893u,
  FLASHHAL_k_ERROR_PG                 = 0xd904u,
  FLASHHAL_k_ERROR_WRP                = 0xda2au,
  FLASHHAL_k_COMPLETE                 = 0xdbbdu
} FLASHHAL_t_STATUS;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
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
void flashHAL_Unlock(void);

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
void flashHAL_Lock(void);

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
FLASHHAL_t_STATUS flashHAL_ErasePage (UINT32 u32_pageAddress);

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
FLASHHAL_t_STATUS flashHAL_ProgramHalfWord(UINT32 u32_address, UINT16 u16_data);

#endif


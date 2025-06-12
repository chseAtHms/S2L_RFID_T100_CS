/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: globCtc_CtcTestHandler.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          globCtc_CtcTestHandler.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        This module handles the Storage and Restorage of the CTC Test
 *                Array.
 * @description:  The module stores the CTC_array in a separate flash page if
 *                the GPIOE Pin 2 is set from low to high. On every system start
 *                the specific flash page is read and the values are write back
 *                to the CTC_array. If the flash is not empty respectively there
 *                are stored values of the CTC_array, the flash page is erased
 *                after read back the values. So the flash page is empty for a
 *                further storing of the CTC_array.
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
#ifdef __CTC_TESTING_ON__
#warning "CTC Test is active!!!"

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

/* Module header */
#include "gpioSample.h"
#include "gpio-hal.h"

/* Header-file of module */
#include "globCtc_CtcTestHandler.h"
#include "globCtc_CtcTestHandler_loc.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/* CTC_array in which the coverage is stored. The constant GLOBCTC_ARRAY_SIZE
 * is not defined in the module, it must be defined in the uVision project
 * properties. So a change of the source code is not necessary.
 * The definition of the constant in the uVision project properties must be in
 * this way:
 * - Project --> Options for Target...
 * - Tab: C/C++
 * - In the field defines under the "Preprocessor Symbols" area must be the
 *   constant definition with the size, e.g. GLOBCTC_ARRAY_SIZE=10
 */
UINT8 CTC_array[GLOBCTC_ARRAY_SIZE] = {0};


/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Moduleglobal Variables ***************************//*@}*/


/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/
STATIC void globCtc_WriteToFlash (void);
STATIC void globCtc_ReadFromFlash (void);
STATIC void globCtc_EraseFlashPage (void);

/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Local Functions                                                            */
/********************************//*!@addtogroup locfunc Local Functions*//*@{*/

/******************************************************************************/
/*!
@Name                globCtc_WriteToFlash

@Description         This function writes the values of the CTC_array to the
                     flash.
                     Before writing the CTC_array the flash page is erased.
                     To erase and write the flash it must be unlocked. After
                     the write cycles the flash is locked again.
                     During the flash write and erase procedure all interrupts
                     are disabled and the watchdog is turned off.

@note                The CTC_array must be written 16Bit-wise to the flash. The
                     values must be inverted, for further information see
                     globCtc_ReadFromFlash.

@Parameter
   @return           void
 */
/******************************************************************************/
STATIC void globCtc_WriteToFlash (void)
{
   UINT32 u32Addr = GLOBCTC_PAGE_STRT_ADDR;
   UINT16 u16Idx;
   UINT16 *pu16Ptr;

   pu16Ptr = (UINT16 *)CTC_array;

   /* disable all interrupts */
   __disable_irq();

   /* disable watchdog */
   RCC->APB1ENR &= ~RCC_APB1RSTR_WWDGRST;

   /* Unlock Flash */
   FLASH->KEYR = GLOBCTC_FLASH_KEY1;
   FLASH->KEYR = GLOBCTC_FLASH_KEY2;

   /* set page erase bit */
   FLASH->CR |= FLASH_CR_PER;

   /* set address of page to erase */
   FLASH->AR = GLOBCTC_PAGE_STRT_ADDR;

   /* start erasing the specified flash page */
   FLASH->CR |= FLASH_CR_STRT;

   /* wait till flash erase is finished */
   while((FLASH->SR & FLASH_SR_BSY) == 1u){;}

   /* start programming */
   FLASH->CR = FLASH_CR_PG;

   /* write the values to the flash */
   for ( u16Idx = 0u;
         u16Idx < (GLOBCTC_ARRAY_SIZE/2u);
         u16Idx++)
   {
      *(__IO UINT16*)u32Addr = ~*(pu16Ptr + u16Idx);
      /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
      u32Addr+=2u;
      /* RSM_IGNORE_QUALITY_END */
   }

   /* Disable the PG Bit */
   FLASH->CR &= FLASH_CR_PG;

   /* Lock the flash */
   FLASH->CR |= FLASH_CR_LOCK;

   /* enable all interrupts */
   __enable_irq();
}


/******************************************************************************/
/*!
@Name                globCtc_ReadFromFlash

@Description         This function reads the stored values in the flash of the
                     CTC_array from the flash into the CTC_array.

@note                The values reading from the flash are inverted, so the
                     values must be stored inverted in the CTC_array. The reason
                     for this is because a deleted flash is 0xFF, all bits are
                     set to 1. Reading the CTC_array from the flash and all bits
                     are set to 1 means that all functions covered by 100%. So
                     the bits from the CTC_array must be stored inverted.

@Parameter
   @return           void
 */
/******************************************************************************/
STATIC void globCtc_ReadFromFlash (void)
{
   UINT32 u32Addr = GLOBCTC_PAGE_STRT_ADDR;
   UINT16 u16Idx;

   /* loop to read the CTC_array from the flash */
   for ( u16Idx = 0u;
         u16Idx < GLOBCTC_ARRAY_SIZE;
         u16Idx++)
   {
      CTC_array[u16Idx] |= ~*(__IO UINT8*)u32Addr;
      u32Addr++;
   }
}


/******************************************************************************/
/*!
@Name                globCtc_EraseFlashPage

@Description         This function erase the flash page where the CTC_array is
                     stored.

@note                The busy flag which indicates an ongoing flash operation
                     is ignored. Before and after an erase this flag must be
                     checked, according the datasheet, that no flash operation
                     is active at the moment. In this case it is sure that no
                     flash operation is ongoing.

@Parameter
   @return           void
 */
/******************************************************************************/
STATIC void globCtc_EraseFlashPage (void)
{
   /* Unlock Flash */
   FLASH->KEYR = GLOBCTC_FLASH_KEY1;
   FLASH->KEYR = GLOBCTC_FLASH_KEY2;

   /* set page erase bit */
   FLASH->CR |= FLASH_CR_PER;

   /* set address of page to erase */
   FLASH->AR = GLOBCTC_PAGE_STRT_ADDR;

   /* start erasing the specified flash page */
   FLASH->CR |= FLASH_CR_STRT;
}


/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/

/******************************************************************************/
/*!
@Name                globCtc_Init

@Description         This function initialize the CTC module. In the first step
                     the port GPIOE Pin 2 is configured as an input. With this
                     pin the write to a flash is initiated.
                     After the pin initialization the stored values of the
                     CTC_array are restored from the flash.

@note                -

@Parameter
   @return           void
 */
/******************************************************************************/
void globCtc_Init (void)
{
   gpioHAL_ConfigureInput( GPIOE_SW_TEST,
                           GPIO_PIN2_SW_TEST_A,
                           IN_CONFIG_PULL_DOWN);

   globCtc_ReadFromFlash();
}


/******************************************************************************/
/*!
@Name                globCtc_CtcTestHandler

@Description         This function is the CTC Test Handler, it should be called
                     from the main routine.
                     The function gets the state of the input pin from the
                     gpioSample module. This input pin is used for storing the
                     CTC values in flash. The values of the input pin must be
                     filtered. If it is definitely at HIGH state the values
                     of the CTC array are stored to the flash. Furthermore a
                     flag is set to avoid further writes to the flash.

@note                -

@Parameter
   @return           void
 */
/******************************************************************************/
void globCtc_CtcTestHandler (void)
{
   LOCAL_STATIC(,UINT8, u8HighCnt, 0u);
   LOCAL_STATIC(,BOOL, bSafeStat, FALSE);
   UINT8 i;

   /* filter for input pin GPIOE2 for saving the CTC values */
   for (i = 0; i < DISMP_BUFFDIM; i++)
   {
      /* gpioSample_au16Buffer is filled with values in the gpioSample
       * module. */
      if ((gpioSample_au16Buffer[4][i] & GPIO_PINMASK_2) == GPIO_PINMASK_2)
      {
         /* check that no overrun of the counter is done */
         if (u8HighCnt < 255u)
         {
            /* increment on every high pulse */
            u8HighCnt++;
         }
      }
      else
      {
         /* clear counter if there is no high pulse */
         u8HighCnt = 0u;
      }
   }

   /* there must be more than 2 high pulses consecutively to execute the
    * storage */
   if ((u8HighCnt > 2u) && (bSafeStat == FALSE))
   {
      globCtc_WriteToFlash();

      /* set flag to TRUE to avoid further execution */
      bSafeStat = TRUE;
   }
}


#endif


/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

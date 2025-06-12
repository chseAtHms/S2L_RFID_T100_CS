/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** serial-hal.c
**
** $Id: serial-hal.c 2048 2017-02-07 11:33:59Z klan $
** $Revision: 2048 $
** $Date: 2017-02-07 12:33:59 +0100 (Di, 07 Feb 2017) $
** $Author: klan $
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** This module provides functions to handle the USART interface. These are
** Read-, Write-, Init- and IsReadyToRead-functions.
** Furthermore this module supports the DMA1-Handling for the USART1 and the
** DMA2—Handling for UART4. The specified DMA-Handling can be enabled by calling
** the specified Init-Functions.
** The UART1 with the DMA can be handled via the send and receive functions.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
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
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* Header-file of module */
#include "serial-hal.h"
#include "serial-hal_loc.h"


/* Unique file id used to build additional info */
#define k_FILEID      30u

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



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
** serialHAL_Open()
**
** Description:
** This function enables the specified USART interface.
** If there is given a wrong UART-Interface, the Safety Handler will be called.
**
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:             UART/USART HW address:
**                      (valid range: USART1, USART3, UART4, checked)
**
**  Register setting:   USART(1,3,4)->CR1
**
** Return:
**  void
**
**  Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void serialHAL_Open (USART_TypeDef* sUsart)
{
  /* check passed parameter */
  if ( ( sUsart == USART1 ) ||
       ( sUsart == USART3 ) ||
       ( sUsart == UART4 ) )
  {
    sUsart->CR1 |= USART_CR1_UE;
  }
  /* parameter corrupted */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_POINTER_INVALID_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  

} /* end of serialHAL_Open() */


/*------------------------------------------------------------------------------
**
** serialHAL_Init()
**
** Description:
** This function initializes and opens the specified USART interface.
**
**
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:             UART/USART HW address:
**                      (valid range: USART1, USART3, UART4, checked)
**  enum eBaudrate:     Baudrate to use:
**                      (valid range: SERIALHAL_38400_BAUD, SERIALHAL_115200_BAUD,
**                      SERIALHAL_1020000_BAUD, SERIALHAL_1500000_BAUD, 
**                      SERIALHAL_2000000_BAUD, SERIALHAL_4000000_BAUD, checked)
**  enum eUsartSet:     Configuartion to use:  
**                      (valid range: SERIALHAL_USART_SET_8NOP1, SERIALHAL_USART_SET_8EVP1,
**                      SERIALHAL_USART_SET_8ODP1, checked)
**
** Return:
**  void
**
**  Remarks:
**    Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void serialHAL_Init (USART_TypeDef* sUsart,
   CONST SERIALHAL_BAUDRATE_DEF_ENUM eBaudrate,
   CONST SERIALHAL_USART_SET_ENUM eUsartSet)
{
   /* check baudrate value, if it is 0 a division by 0 occurs. */
   /* if invalid baudrate passed */
   if ( (eBaudrate != SERIALHAL_38400_BAUD)   &&
        (eBaudrate != SERIALHAL_115200_BAUD)  &&
        (eBaudrate != SERIALHAL_1020000_BAUD) &&
        (eBaudrate != SERIALHAL_1500000_BAUD) &&
        (eBaudrate != SERIALHAL_2000000_BAUD) &&
        (eBaudrate != SERIALHAL_4000000_BAUD) )
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
   }
   /* else if invalid UART address passed */
   else if ( ( sUsart != USART1 ) &&
             ( sUsart != USART3 ) &&
             ( sUsart != UART4 ) )
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_POINTER_INVALID_EX, GLOBFAIL_ADDINFO_FILE(3u));
   }
   /* else: baudrate and UART address valid */
   else
   {
     /* RCC register: enable the USARTx clock */
     serialHAL_Init_RccRegForUsartX(sUsart);
  
     /* calculate and configure baudrate, differ between USART1 and the other
      * USARTs because USART1 is clocked with max. 72MHz and the others with
      * max. 36MHz.
      * The calculation of the bautrate is used from the datasheet of the uC,
      * chapter 27.3.4 RM0008 Reference Manual Rev. 14.
      *
      * The following formula is used:
      * Tx/Rx baud = fck / (16 * USARTDIV)
      *
      * To calculate the baudrate the given formula must change to this way:
      * (16 * USARTDIV) = fck / Tx/Rx baud
      *
      * and (16 * USARTDIV) == USART_BRR
      * */
     if (sUsart == USART1)
     {
        sUsart->BRR = (UINT16) ((UINT32) SERIALHAL_SYSCLOCK / (UINT32) eBaudrate);
     }
     else
     {
        sUsart->BRR = (UINT16) ((UINT32) SERIALHAL_SYSCLOCK
           / (UINT32) ((UINT32) eBaudrate * 2u));
     }
  
     /* activate transmit and receive */
     sUsart->CR1 = (USART_CR1_RE | USART_CR1_TE);
  
     /* parity configure of the UART */
     switch (eUsartSet)
     {
        case SERIALHAL_USART_SET_8NOP1:
        {
           /* disable parity control */
           sUsart->CR1 &= ~USART_CR1_PCE;
           /* set control register 2 to the default value */
           sUsart->CR2 = SERIALHAL_REG_DFLT_VALUE;
           /* set control register 3 to the default value */
           sUsart->CR3 = SERIALHAL_REG_DFLT_VALUE;
           /* open UART */
           serialHAL_Open(sUsart);
           break;
        }
        case SERIALHAL_USART_SET_8EVP1:
        {
           /* enable even parity control */
           sUsart->CR1 |= USART_CR1_PCE;
           sUsart->CR1 &= ~USART_CR1_PS;
           /* set control register 2 to the default value */
           sUsart->CR2 = SERIALHAL_REG_DFLT_VALUE;
           /* set control register 3 to the default value */
           sUsart->CR3 = SERIALHAL_REG_DFLT_VALUE;
           /* open UART */
           serialHAL_Open(sUsart);
           break;
        }
        case SERIALHAL_USART_SET_8ODP1:
        {
           /* enable odd parity control */
           sUsart->CR1 |= (USART_CR1_PCE | USART_CR1_PS);
           /* set control register 2 to the default value */
           sUsart->CR2 = SERIALHAL_REG_DFLT_VALUE;
           /* set control register 3 to the default value */
           sUsart->CR3 = SERIALHAL_REG_DFLT_VALUE;
           /* open UART */
           serialHAL_Open(sUsart);
           break;
        }
        default:
        {
           /* wrong UART settings */
           GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
           break;
        }
  
     } /* switch( eUsartSet ) */
   }

} /* end of serialHAL_Init() */


#ifdef GLOBFIT_FITTEST_ACTIVE
/*------------------------------------------------------------------------------
**
** serialHAL_IsReadyToRead()
**
** Description:
** This function checks if new data is available.
** If there is given a wrong UART-Interface, the Safety Handler will be called.
**
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:      USART1
**               USART3
**               UART4
**
** Return:
**  BOOL eRet
**
**------------------------------------------------------------------------------
*/
BOOL serialHAL_IsReadyToRead (CONST USART_TypeDef* sUsart)
{
   BOOL bRet = FALSE;

   GLOBFAIL_SAFETY_ASSERT( ( sUsart == USART1 ) ||
                           ( sUsart == USART3 ) ||
                           ( sUsart == UART4 ),
                           GLOB_FAILCODE_POINTER_INVALID);

   /* check if (RXNE: Read data register not empty) is set. */
   if ((sUsart->SR & USART_SR_RXNE) == USART_SR_RXNE)
   {
      /*  Received data is ready to be read:
       * return TRUE, new data available
       * */
      bRet = TRUE;
   } /* CCT_SKIP Failure insertion Test */

   return bRet;

} /* end of serialHAL_IsReadyToRead() */ /* CCT_SKIP Failure insertion Test */


/*------------------------------------------------------------------------------
**
** serialHAL_ReadFromUsartDR()
**
** Description:
** This function read one byte from the RX-buffer. If there is given a wrong
** UART-Interface, the Safety Handler will be called.
**
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:      USART1
**               USART3
**               UART4
**
** Return:
**   u8Byte
**
**------------------------------------------------------------------------------
*/
UINT8 serialHAL_ReadFromUsartDR (CONST USART_TypeDef* sUsart)
{
   GLOBFAIL_SAFETY_ASSERT( ( sUsart == USART1 ) ||
                           ( sUsart == USART3 ) ||
                           ( sUsart == UART4 ),
                           GLOB_FAILCODE_POINTER_INVALID);

   return (UINT8) sUsart->DR;

} /* end of serialHAL_ReadFromUsartDR() */ /* CCT_SKIP Failure insertion Test */


/*------------------------------------------------------------------------------
**
** serialHAL_WriteToUsartDR()
**
** Description:
** This function writes one byte to the TX-buffer.
** If the buffer is not empty, the function returns BUSY, so try again later.
**
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:       USART1
**                USART3
**                UART4
**  u8Byte:       Byte to be written
**
** Return:
**  eRet        GLOB_RET_OK    - OK
**              GLOB_RET_BUSY  - USART is busy, TX-buffer is not empty
**
**------------------------------------------------------------------------------
*/
GLOB_RETCODE_ENUM serialHAL_WriteToUsartDR (USART_TypeDef* sUsart,
   CONST UINT8 u8Byte)
{
   GLOB_RETCODE_ENUM eRet = GLOB_RET_BUSY;

   GLOBFAIL_SAFETY_ASSERT( ( sUsart == USART1 ) ||
                           ( sUsart == USART3 ) ||
                           ( sUsart == UART4 ),
                           GLOB_FAILCODE_POINTER_INVALID);

   /* check the TXE: Transmit data register empty */
   if ((sUsart->SR & USART_SR_TXE) == USART_SR_TXE)
   {
      /* write byte to TX-buffer:
       * Data is transferred to the shift register)
       * */
      sUsart->DR = (UINT16) u8Byte;

      eRet = GLOB_RET_OK;
   } /* CCT_SKIP Failure insertion Test */

   return eRet;

} /* end of serialHAL_WriteToUsartDR() */ /* CCT_SKIP Failure insertion Test */
#endif /* GLOBFIT_FITTEST_ACTIVE */


/*------------------------------------------------------------------------------
**
** serialHAL_SendUsart1Dma()
**
** Description:
** The function sends data via DMA and calls initialization functions.
** This function is just for USART1 with DMA1.
**
**------------------------------------------------------------------------------
** Input:
**  u32DataLength      Length of the transmitted data. 
**                     (valid range: 0..SERIAL_USART1_MAX_DATA_LENGTH, checked)
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void serialHAL_SendUsart1Dma( CONST UINT32 u32DataLength )
{
   /* Check the length of the given data length, it must be smaller than the maximum allowed
    * number of bytes */
   if (u32DataLength <= SERIAL_USART1_MAX_DATA_LENGTH)
   {
     /* disable DMA channel (needed during settings) */
     SERIAL_USART1_TX_DMA1_CH4->CCR &= ~DMA_CCR4_EN; /* Tx */
  
     /* write DMA-size in CNTR-Register */
     SERIAL_USART1_TX_DMA1_CH4->CNDTR = u32DataLength;
  
     /* enable DMA1 channel 4 */
     SERIAL_USART1_TX_DMA1_CH4->CCR |= DMA_CCR4_EN; /* TX */
   }
   /* else: invalid parameter (length) */
   else
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(5u));
   }

} /* end of serialHAL_SendUsart1Dma() */



/* ------------------------------------------------------------------------------
**
** serialHAL_InitTxUsart1DmaCh()
**
** Description:
** This function initialize the DMA1 channel 4 for USART1 Tx
** and enables the USART-DMA-Interface for the data transfer.
** Before this function is called, the USART1 must be
** initialized via the serialHAL_Init function to ensure the
** correct communication settings.
**
**------------------------------------------------------------------------------
** Input:
**  u32TxBufferAddr    Transmit buffer address
**                     (valid range: <>NULL, not checked, only called with reference
**                     to global/static buffer)
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void serialHAL_InitTxUsart1DmaCh ( CONST UINT32 u32TxBufferAddr )
{
   /* enable DMA1 clock */
   RCC->AHBENR |= RCC_AHBENR_DMA1EN;

   /* Control register 3 (USART_CR3) */
   USART1->CR3 |= USART_CR3_DMAT; /* DMA enable transmitter for USART 1 */

   /* settings for TX DMA: */
   SERIAL_USART1_TX_DMA1_CH4->CCR = DMA_CCR4_MINC /* Memory increment mode, */
                                                  /* 1: Memory increment mode enabled  */
                                  | DMA_CCR4_PL /* Channel priority level, 11: Very high */
                                  | DMA_CCR4_DIR; /* Data transfer direction, 1: Read from memory */

   /* write the the USART1_DR address into the
    * DMA channel peripheral address register */
   SERIAL_USART1_TX_DMA1_CH4->CPAR = (UINT32) &USART1->DR;/*lint !e923*/
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

   /* write address of TxBuffer in CMAR-Register */
   SERIAL_USART1_TX_DMA1_CH4->CMAR  = u32TxBufferAddr;

} /* CCT_SKIP */ /* only register settings */ /* end of serialHAL_InitTxUsart1DmaCh() */


/*------------------------------------------------------------------------------
**
** serialHAL_InitRxUsart1DmaCh()
**
** Description:
** This function initialize the DMA1 channel 5 for USART1 Rx
** and enables the USART-DMA-Interface for the data transfer.
** Before this function is called, the USART1 must be
** initialized via the serialHAL_Init function to ensure the
** correct communication settings.
** The value has the range 0..SERIAL_USART1_MAX_DATA_LENGTH. 
** If it is out of range, the Safety Handler is called.
**
**------------------------------------------------------------------------------
** Input:
**  u32RxBufferAddr    RX buffer address.
**                     (valid range: <>NULL, not checked, only called with reference
**                     to global/static buffer)
**  u32DataLength      RX data length. 
**                     (valid range: 0..SERIAL_USART1_MAX_DATA_LENGTH, checked inside
**                     subfunction) 
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void serialHAL_InitRxUsart1DmaCh ( CONST UINT32 u32RxBufferAddr, CONST UINT32 u32DataLength )
{
   /* enable DMA1 clock */
   RCC->AHBENR |= RCC_AHBENR_DMA1EN;

   /* enable DMA transfer for USART1 */
   USART1->CR3 |= USART_CR3_DMAR; /* DMA enable receiver */

   /* settings for RX DMA:
    * Memory increment mode : 1: Memory increment mode enabled
    * Channel priority level: 11: Very high */
   SERIAL_USART1_RX_DMA1_CH5->CCR = (DMA_CCR5_MINC | DMA_CCR5_PL);

   /* write the the USART1_DR address into the DMA channel peripheral address register */
   SERIAL_USART1_RX_DMA1_CH5->CPAR = (UINT32) &USART1->DR;/*lint !e923*/
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

   /* write address of RxBuffer in CMAR-Register */
   SERIAL_USART1_RX_DMA1_CH5->CMAR = u32RxBufferAddr;

   /* enables the USART1 DMA receive */
   serialHAL_ReceiveUsart1Dma(u32DataLength);

} /* end of serialHAL_InitRxUsart1DmaCh() */

/* ------------------------------------------------------------------------------
**
** serialHAL_InitRxTxUart4Dma2Ch()
**
** Description:
** This function initialize the DMA2 for the UART4 of AIC.
** This function initialize the DMA2 Channel 5 for UART4 Tx
**
** DMA2 request: UART4 -> Channel 3: UART4_RX
**               UART4 -> Channel 5: UART4_TX
**
**------------------------------------------------------------------------------
** Input:
**  void
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void serialHAL_InitRxTxUart4Dma2Ch (void)
{
   /* enable DMA2 clock */
   RCC->AHBENR |= RCC_AHBENR_DMA2EN;

   /* Control register 3 (USART_CR3) */
   UART4->CR3 |= USART_CR3_DMAT; /* DMA enable transmitter for UART 4 */

   /* settings for Tx DMA:
    * Memory increment mode 1  : Memory increment mode enabled
    * Channel priority level 11: Very high
    * Data transfer direction 1: Read from memory */
   AIC_TX_UART_DMA2_CHANNEL5->CCR = DMA_CCR5_MINC |
                                    DMA_CCR5_PL   |
                                    DMA_CCR5_DIR;

   /* write the the USART1_DR address into the
    * DMA channel peripheral address register */
    AIC_TX_UART_DMA2_CHANNEL5->CPAR = (UINT32) &UART4->DR;/*lint !e923*/
    /* Lint message disabled because the register address is written to the
     * DMA register as integer value. */

   /*
    * Init RX for DMA
    */

   /* enable DMA transfer for USART1 */
   UART4->CR3 |= USART_CR3_DMAR; /* DMA enable receiver */

   /* settings for Rx DMA:
    * Memory increment mode : 1 : Memory increment mode enabled
    * Channel priority level: 11: Very high */
   AIC_RX_UART_DMA2_CHANNEL3->CCR = (DMA_CCR3_MINC | DMA_CCR3_PL);

   /* write the the USART1_DR address into the
    * DMA channel peripheral address register */
   AIC_RX_UART_DMA2_CHANNEL3->CPAR = (UINT32) &UART4->DR;/*lint !e923*/
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

} /* CCT_SKIP */ /* only register settings */ /* end of serialHAL_InitRxTxUart4Dma2Ch() */



/*******************************************************************************
 **
 ** Private Services
 **
 ********************************************************************************
 */
/*------------------------------------------------------------------------------
**
** serialHAL_ReceiveUsart1Dma()
**
** Description:
** This function initialize the USART1 Rx DMA1 channel 5 and enables the
** USART1 DMA receive.
**
**------------------------------------------------------------------------------
** Input:
**  u32DataLength      RX data length.  
**                     (valid range: 0..SERIAL_USART1_MAX_DATA_LENGTH, checked)
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**   Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void serialHAL_ReceiveUsart1Dma ( CONST UINT32 u32DataLength )
{
   /* Check the length of the given data length, it must be smaller than the maximum allowed
    * number of bytes */
   if (u32DataLength <= SERIAL_USART1_MAX_DATA_LENGTH)
   {                    
     /* disable DMA1 channel 5, to write data into the CNDTR and CMAR register
      * NOTE: If it is enable, the registers are write protected! */
     SERIAL_USART1_RX_DMA1_CH5->CCR &= ~DMA_CCR5_EN; /* RX */
  
     /* write DMA-size in CNTR-Register */
     SERIAL_USART1_RX_DMA1_CH5->CNDTR = u32DataLength;
  
     /* enable DMA and UART */
     SERIAL_USART1_RX_DMA1_CH5->CCR |= DMA_CCR5_EN; /* Rx */
  
     /* enable the IPC-UART */
     serialHAL_Open(USART1);
   }
   /* else: invalid parameter (length)*/
   else
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(6u));
   }

} /* end of serialHAL_ReceiveUsart1Dma() */


/*------------------------------------------------------------------------------
**
** serialHAL_Init_RccRegForUsartX()
**
** Description:
** This function enables the UART specific clock and the appropriate clock
** for the GPIO.
** If the given UART is not specified, the Safety-Handler is called.
**
**------------------------------------------------------------------------------
** Input:
**  sUsart:           UART/USART HW address:
**                    (valid range: USART1, USART3, UART4, checked)
**
**  Register setting: RCC->APB1ENR
**                    RCC->APB2ENR
**
** Return:
**  void
**
** Remarks:
**   Context: main (during initialization)
**------------------------------------------------------------------------------
*/
STATIC void serialHAL_Init_RccRegForUsartX (CONST USART_TypeDef* sUsart)
{
   /* Alternate Function I/O clock enable */
   RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

   if (USART1 == sUsart)
   {
      RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  /* enable the USART1 clock */
      RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;    /* I/O port A clock enable */
   }
   else if (USART3 == sUsart)
   {
      RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  /* enable the USART3 clock */
      RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;    /* I/O port B clock enable */
   }
   else if (UART4 == sUsart)
   {
      RCC->APB1ENR |= RCC_APB1ENR_UART4EN;   /* enable the UART4 clock */
      RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;    /* I/O port C clock enable */
   }
   else
   {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(7u)); /* error */
   } /* CCT_SKIP */ /* already checked */

} /* end of serialHAL_Init_RccRegForUsartX() */


/*******************************************************************************
**
** End of serial-hal.c
**
********************************************************************************
*/

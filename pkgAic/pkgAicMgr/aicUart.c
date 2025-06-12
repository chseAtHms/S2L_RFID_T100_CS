/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** aicUart.c
**
** $Id: aicUart.c 2387 2017-03-17 11:38:18Z klan $
** $Revision: 2387 $
** $Date: 2017-03-17 12:38:18 +0100 (Fr, 17 Mrz 2017) $
** $Author: klan $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** module containing:
** - UART/DMA interface functions for AIC
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

/* system includes */
#include "stm32f10x.h"

#include "xtypes.h"
#include "xdefs.h"

/* global and error handling includes */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* serial includes */
#include "gpio-hal.h"
#include "serial-hal.h"

/* AIC module includes  */
#include "aicMsgDef.h"
#include "aicUart.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** DMA Buffers for Rx and Tx of the AIC. These buffers are defined in a
** separate RAM section.
** The word "attribute" and the words in the brackets are compiler specific
** syntax.
**------------------------------------------------------------------------------
*/
AICMSGDEF_ASM_TELEGRAM_STRUCT aicUart_sAicTxDmaBuffer
             __attribute__( ( section( "DMA_BUFFER_SECTION" ) ) );
AICMSGDEF_AM_TELEGRAM_STRUCT aicUart_sAicRxDmaBuffer
             __attribute__( ( section( "DMA_BUFFER_SECTION" ) ) );


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Diagnostic service
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
** aicUart_UartDmaInit()
**
** Description:
**    The function initializes the ports and UART for the AIC
**    communication
**    Furthermore the Rx and Tx Buffers are set to the DMA.
**    (see [SRS_180])
**
**------------------------------------------------------------------------------
** Input:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    aicUart_UartDmaInit();
**
** Module Test:
**    - NO -
**    Reason: Only function calls, in case of an error UART would not work
**------------------------------------------------------------------------------
*/
void aicUart_UartDmaInit (void)
{
   UINT8 i;
   UINT8 *pu8 = (UINT8*) &aicUart_sAicRxDmaBuffer;/*lint !e928*/

   for (i = 0u; i < AICMSGDEF_AM_TGM_SIZE; i++)
   {
      *(pu8 + i) = 0u;/*lint !e960*/
   }
   /* Deactivated Lint messages 928 & 960 because with the pointer the buffer
    * is set to a specified value. */

   /* configure the GPIO Pin PC10 for UART Tx (see [SRS_180]) */
   gpioHAL_ConfigureOutput( GPIOC,
                            GPIO_10,
                            OUT_CONFIG_AF_PP,
                            OUT_MODE_50MHZ );

   /* configure the GPIO Pin PC11 for UART Rx (see [SRS_180]) */
   gpioHAL_ConfigureInput( GPIOC,
                           GPIO_11,
                           IN_CONFIG_FLOATING );

   /* init the UART4 (see [SRS_180], [SRS_414]) */
   serialHAL_Init( UART4_AIC,
                   SERIALHAL_1020000_BAUD,
                   SERIALHAL_USART_SET_8NOP1 );

   /* initialized Uart4 for DMA transfer (see [SRS_180], [SRS_711]) */
   serialHAL_InitRxTxUart4Dma2Ch();

   /* write address of RxBuffer in CMAR-Register */
   AIC_RX_UART_DMA2_CHANNEL3->CMAR  = (UINT32) &aicUart_sAicRxDmaBuffer;/*lint !e923*/

   /* write address of TxBuffer in CMAR-Register */
   AIC_TX_UART_DMA2_CHANNEL5->CMAR = ( UINT32 )&aicUart_sAicTxDmaBuffer;/*lint !e923*/

   /* init the DMA for Rx (see [SRS_180]) */
   aicUart_DmaInitRx();

   /* Lint message 923 deactivated because pointer address is casted to a
    * numeric value to set the DMA to the specific RAM address. */
}


/*------------------------------------------------------------------------------
**
** aicUart_TriggerSending()
**
** Description:
**    function initiates sending of Tx buffer via UART (DMA).
**    In "CNDTR" Register the number of bytes for the default message is set,
**    this is then the number of bytes which will be sent by DMA.
**
**------------------------------------------------------------------------------
** Input:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    aicUart_TriggerSending();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void aicUart_TriggerSending (void)
{
   /* clear enable to write into the CNDTR */
   AIC_TX_UART_DMA2_CHANNEL5->CCR &= ~DMA_CCR5_EN;

   /* write DMA-size in CNTR-Register */
   AIC_TX_UART_DMA2_CHANNEL5->CNDTR = AICMSGDEF_ASM_TGM_SIZE;

   /* check if the DmaSize is correct in the CNDTR register */
   GLOBFAIL_SAFETY_ASSERT( ( AIC_TX_UART_DMA2_CHANNEL5->CNDTR == AICMSGDEF_ASM_TGM_SIZE ),
                             GLOB_FAILCODE_VARIABLE_ERR);

   /* TX channel enable */
   AIC_TX_UART_DMA2_CHANNEL5->CCR |= DMA_CCR5_EN;
}


/*------------------------------------------------------------------------------
**
** aicUart_TriggerSendingStartup()
**
** Description:
**    function initiates sending of startup telegram via UART (DMA).
**    In "CNDTR" Register the number of bytes for the startup message is set,
**    this is then the number of bytes which will be sent by DMA.
**
**------------------------------------------------------------------------------
** Input:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    aicUart_TriggerSendingStartup();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void aicUart_TriggerSendingStartup (void)
{
   /* clear enable to write into the CNDTR */
   AIC_TX_UART_DMA2_CHANNEL5->CCR &= ~DMA_CCR5_EN;

   /* write DMA-size in CNTR-Register */
   AIC_TX_UART_DMA2_CHANNEL5->CNDTR = AICMSGDEF_ASM_STARTUP_TGM_SIZE;

   /* check if the DmaSize is correct in the CNDTR register */
   GLOBFAIL_SAFETY_ASSERT( ( AIC_TX_UART_DMA2_CHANNEL5->CNDTR == AICMSGDEF_ASM_STARTUP_TGM_SIZE ),
                             GLOB_FAILCODE_VARIABLE_ERR);

   /* TX channel enable */
   AIC_TX_UART_DMA2_CHANNEL5->CCR |= DMA_CCR5_EN;
}


/*------------------------------------------------------------------------------
**
** aicUart_DmaInitRx()
**
** Description:
**    initialization of reception via DMA
**
**------------------------------------------------------------------------------
** Input:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    aicUart_DmaInitRx();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void aicUart_DmaInitRx (void)
{
   /* disable DMA2 channel 3, to write data into the CNDTR and CMAR register
   * NOTE: If it is enabled, the registers are write protected! */
   AIC_RX_UART_DMA2_CHANNEL3->CCR &= ~DMA_CCR3_EN; /* RX */

   /* write DMA-size in CNTR-Register */
   AIC_RX_UART_DMA2_CHANNEL3->CNDTR = AICMSGDEF_AM_TGM_SIZE;

   /* enable DMA and UART */
   AIC_RX_UART_DMA2_CHANNEL3->CCR |= DMA_CCR3_EN; /* Rx */

   /* check that the CMAR-Register still contains the memory address of the
    * Rx and Tx Buffers. */
   GLOBFAIL_SAFETY_ASSERT( AIC_RX_UART_DMA2_CHANNEL3->CMAR == (UINT32) &aicUart_sAicRxDmaBuffer,
                           GLOB_FAILCODE_VARIABLE_ERR)/*lint !e923*/
   GLOBFAIL_SAFETY_ASSERT( AIC_TX_UART_DMA2_CHANNEL5->CMAR == (UINT32) &aicUart_sAicTxDmaBuffer,
                              GLOB_FAILCODE_VARIABLE_ERR)/*lint !e923*/
   /* Lint message 923 deactivated because pointer address is casted to a
    * numeric value to set the DMA to the specific RAM address. */

   /* enable the AIC-UART */
   serialHAL_Open( UART4_AIC );
}


/*------------------------------------------------------------------------------
**
** aicUart_NewTgmReceived()
**
** Description:
**    returns TRUE if there is a new message/telegram was received
**
**------------------------------------------------------------------------------
** Input:
**    - None -
**
** Return:
**    TRUE:  new telegram/message received
**    FALSE: no new message received
**
** Usage:
**    aicUart_NewTgmReceived();
**
** Module Test:
**    - NO -
**    Reason: easy to understand, low complexity
**------------------------------------------------------------------------------
*/
BOOL aicUart_NewTgmReceived (void)
{
   BOOL bRetVal = FALSE;

   if ( AICUART_NEW_MSG_RECEIVED == (AIC_RX_UART_DMA2_CHANNEL3->CNDTR) )
   {
      bRetVal = TRUE;
   }

   return bRetVal;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/



/*******************************************************************************
**
** End of aicUart.c
**
********************************************************************************
*/

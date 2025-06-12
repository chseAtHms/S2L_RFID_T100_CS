/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** serial-hal.h
**
** $Id: serial-hal.h 1049 2016-11-14 06:53:45Z klan $
** $Revision: 1049 $
** $Date: 2016-11-14 07:53:45 +0100 (Mo, 14 Nov 2016) $
** $Author: klan $
**
********************************************************************************

********************************************************************************
**
** Description
** -----------
** Header of modul serial-hal.
** This module provides functions to handle the USART interface. These are
** Read-, Write-, Init- and IsReadyToRead-functions.
** Furthermore this module supports the DMA1-Handling for the USART1. The
** DMA1-Handling can be enabled by calling the specified Init-Function and can
** be handled via the ReadyToRead-Function.
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
*/


#ifndef SERIALHAL_H
#define SERIALHAL_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** define for AIC communication
**------------------------------------------------------------------------------
*/
/* UART4 for AIC */
#define UART4_AIC                     ( UART4 )

/* DMA2 channel 5 for TX UART */
#define AIC_TX_UART_DMA2_CHANNEL5     ( DMA2_Channel5 )

/* DMA2 channel 3 for RX UART */
#define AIC_RX_UART_DMA2_CHANNEL3     ( DMA2_Channel3 )


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/


/*------------------------------------------------------------------------------
** typedef for structure containing the available baud rates for the USART interface.
**------------------------------------------------------------------------------
*/

/* Baudrates are assigned in enum directly */ 
/* RSM_IGNORE_QUALITY_BEGIN Notice #50   - Variable assignment to a literal number */
typedef enum
{
   SERIALHAL_38400_BAUD    = 38400u,
   SERIALHAL_115200_BAUD   = 115200u,
   SERIALHAL_1020000_BAUD  = 1020000u,
   SERIALHAL_1500000_BAUD  = 1500000u,
   SERIALHAL_2000000_BAUD  = 2000000u,
   SERIALHAL_4000000_BAUD  = 4000000u
} SERIALHAL_BAUDRATE_DEF_ENUM;
/* RSM_IGNORE_QUALITY_END */

/*------------------------------------------------------------------------------
** This enum contains the various settings for the USART interface.
**------------------------------------------------------------------------------
*/

typedef enum
{
   SERIALHAL_USART_SET_8NOP1 = 0x2b17u,  /*!< UART settings: 8 data bits, no parity, 1 stop bit */
   SERIALHAL_USART_SET_8EVP1 = 0x2cdcu,  /*!< UART settings: 8 data bits, even parity, 1 stop bit */
   SERIALHAL_USART_SET_8ODP1 = 0x2d4bu   /*!< UART settings: 8 data bits, odd parity, 1 stop bit */
} SERIALHAL_USART_SET_ENUM;



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
extern void serialHAL_SendUsart1Dma( CONST UINT32 u32DataLength );

extern void serialHAL_InitRxTxUart4Dma2Ch ( void );

#ifdef GLOBFIT_FITTEST_ACTIVE
extern BOOL serialHAL_IsReadyToRead       ( CONST USART_TypeDef* sUsart );

extern UINT8 serialHAL_ReadFromUsartDR    ( CONST USART_TypeDef* sUsart);

extern GLOB_RETCODE_ENUM serialHAL_WriteToUsartDR( USART_TypeDef* sUsart,
                                                   CONST UINT8    u8Byte );
#endif /* GLOBFIT_FITTEST_ACTIVE */

extern void serialHAL_InitTxUsart1DmaCh   ( CONST UINT32  u32TxBufferAddr );

extern void serialHAL_InitRxUsart1DmaCh   ( CONST UINT32 u32RxBufferAddr,
                                            CONST UINT32 u32DataLength );

extern void serialHAL_Init( USART_TypeDef*                    sUsart,
                            CONST SERIALHAL_BAUDRATE_DEF_ENUM eBaudrate,
                            CONST SERIALHAL_USART_SET_ENUM    eUsartSet );

extern void serialHAL_Open( USART_TypeDef* sUsart );

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of serial-hal.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** serial-hal-loc.h
**
** $Id: serial-hal_loc.h 597 2016-09-05 12:19:11Z klan $
** $Revision: 597 $
** $Date: 2016-09-05 14:19:11 +0200 (Mo, 05 Sep 2016) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**  Local header for serial-hal module.
**  Contains the local constants and type definitions of the appendent c-file,
**  see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
**
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

#ifndef SERIALHAL_LOC_H
#define SERIALHAL_LOC_H


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

/*! This constant defines the system clock for the USART usage.*/
#define SERIALHAL_SYSCLOCK          GLOBDEF_SYSTEM_CLOCK

/*! Default value for the USART Registers. */
#define SERIALHAL_REG_DFLT_VALUE    ( ( UINT16 ) 0u )

/*! DMA1 channel 4 for TX USART1 */
#define SERIAL_USART1_TX_DMA1_CH4     ( DMA1_Channel4 )

/*! DMA1 channel 5 for RX USART */
#define SERIAL_USART1_RX_DMA1_CH5     ( DMA1_Channel5 )


/* Defines the maximum number of bytes that can be transferred via USART1 (RX and TX). */
/* Here: 12 bytes payload + 2 byte header (ID, CRC), double transmission */
#ifdef RFID_ACTIVE
#define SERIAL_USART1_MAX_DATA_LENGTH  ( (12 + 4 + 2) * 2 )
#else
#define SERIAL_USART1_MAX_DATA_LENGTH  ( 28u )
#endif

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/



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
STATIC void serialHAL_Init_RccRegForUsartX (CONST USART_TypeDef* sUsart);
STATIC void serialHAL_ReceiveUsart1Dma ( CONST UINT32 u32DataLength );

#endif  /*SERIALHAL_LOC_H */

/*******************************************************************************
**
** End of serial-hal_loc.h
**
********************************************************************************
*/

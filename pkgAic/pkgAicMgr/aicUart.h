/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** aicUart.h
**
** $Id: aicUart.h 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** header file of "aicUart.c"
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

#ifndef AIC_UARTDMA_H
#define AIC_UARTDMA_H


/*******************************************************************************
**
** Constants (#define)
**
********************************************************************************
*/

/* constant for checking that new message is received */
#define AICUART_NEW_MSG_RECEIVED    ( (UINT32) 0u )

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


/*------------------------------------------------------------------------------
**  Rx and Tx DMA Buffer
**  global used for performance reasons (faster merge/split of telegram)
**------------------------------------------------------------------------------
*/
extern AICMSGDEF_ASM_TELEGRAM_STRUCT aicUart_sAicTxDmaBuffer;
extern AICMSGDEF_AM_TELEGRAM_STRUCT  aicUart_sAicRxDmaBuffer;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void aicUart_UartDmaInit(void);
extern void aicUart_DmaInitRx(void);
extern void aicUart_TriggerSending(void);
extern void aicUart_TriggerSendingStartup(void);
extern BOOL aicUart_NewTgmReceived(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of aicUart.h
**
********************************************************************************
*/


/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: ipcx_ExchangeData-sys.h
**     Summary: Header file of the IPC exchange data module.
**              This module contains all functions to send and receive data
**              over the IPC to and from the other controller.
**              The IPC is a communication over the UART interface.
**   $Revision: 2427 $
**       $Date: 2017-03-24 10:06:34 +0100 (Fr, 24 Mrz 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: ipcxSYS_Init
**             ipcxSYS_SendUINT32
**             ipcxSYS_SendBuffer
**             ipcxSYS_GetBufferinclWait
**             ipcxSYS_GetUINT32inclWait
**             ipcxSYS_ExChangeUINT32Data
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef IPCX_EXCHANGEDATA_SYS_H
#define IPCX_EXCHANGEDATA_SYS_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


/***************************************************************************************************
  Function:
    ipcxSYS_Init

  Description:
    This function initializes the IPC interface.
    The communication goes over the USART. This function initializes the specific port pins 
    PA9 and PA10 of the USART.
    
    Furthermore:
     - it initializes the DMA buffer addresses
     - initialize the UART interface for DMA.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void ipcxSYS_Init( void );

/***************************************************************************************************
  Function:
    ipcxSYS_SendUINT32

  Description:
    This function sends an UINT32 data over the IPC. This function generates the IPC telegram 
    and initiates a new IPC communication.

  See also:
    -

  Parameters:
    u8_ipcId (IN)  - ID of the IPC telegram. Since the highest bit of the transferred ID byte
                     is used to distinguish between controller1 and controller2, this bit 
                     shall not be used for an IPC ID (message ID). This must be ensured by the 
                     calling function.
                     (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)
    u32_data (IN)  - value to send
                     (valid range: any, not checked)    

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void ipcxSYS_SendUINT32( CONST UINT8  u8_ipcId, CONST UINT32 u32_data );

/***************************************************************************************************
  Function:
    ipcxSYS_SendBuffer

  Description:
    This function is used to send a variable number of bytes over the IPC to the other controller.
    The function generates the IPC telegram and initiates a new IPC communication.

  See also:
    -

  Parameters:
    u8_ipcId (IN)     - ID of the IPC telegram. Since the highest bit of the transferred ID byte
                        is used to distinguish between controller1 and controller2, this bit 
                        shall not be used for an IPC ID (message ID). This must be ensured by the 
                        calling function.
                        (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)          
    u8_dataLen (IN)   - Length of payload to send
                        (valid range: 0..IPCXSYS_SIZEOF_MAX_PAYLOAD, checked)
    pau8_buffer (IN)  - Payload to transmit over IPC
                        (valid range: <>NULL, not checked because only called by reference)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
void ipcxSYS_SendBuffer( CONST UINT8  u8_ipcId, CONST UINT8 u8_dataLen, CONST UINT8* pau8_buffer);

/***************************************************************************************************
  Function:
    ipcxSYS_GetBufferinclWait

  Description:
    This function gets an IPC telegram which should already have arrived. In some cases there might 
    be a small delay between the uCs and the time-slices, so a short wait time is implemented here.

  See also:
    -

  Parameters:
    u8_ipcId (IN)     - ID of the IPC telegram. Since the first bit of ID byte is used
                        to distinguish between controller1 and controller2, this bit shall not 
                        be used for IPC ID (message ID). This must be ensured by the calling
                        function.
                        (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)
    u8_dataLen (IN)   - Length of payload to received
                        (valid range: 0..IPCXSYS_SIZEOF_MAX_PAYLOAD, checked)
    pau8_buffer (OUT) - Buffer for reception.
                        (valid range: <>NULL, not checked because only called by reference)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
void ipcxSYS_GetBufferinclWait ( CONST UINT8 u8_ipcId, CONST UINT8 u8_dataLen, UINT8* pau8_buffer);

/***************************************************************************************************
  Function:
    ipcxSYS_GetUINT32inclWait

  Description:
    This function gets an IPC telegram which should already have arrived.
    In some cases there might be a small delay between the uCs and the time-slices, 
    so a short wait time is implemented here.

  See also:
    -

  Parameters:
    u8_ipcId (IN)  - IPC Identifier of the message. Since the first bit of ID byte is used
                     to distinguish between controller1 and controller2, this bit shall not 
                     be used for IPC ID (message ID). This must be ensured by the calling
                     function.
                     (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)

  Return value:
    UINT32         - Received IPC value

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT32 ipcxSYS_GetUINT32inclWait ( CONST UINT8 u8_ipcId );

/***************************************************************************************************
  Function:
    ipcxSYS_ExChangeUINT32Data

  Description:
    This function exchanges the given data with the other controller.
    After sending the data the function waits till the data of the other controller is received. 
    During wait time a timeout is calculated and checked. If the timeout reaches the limit, 
    the Safety Handler will be called.
    If there is an error during transmission the Safety Handler will be called as well.

  See also:
    -

  Parameters:
    u32_ipcSendVal (IN) - Value to send over IPC
                           (valid range: any, not checked)
    u8_ipcId (IN)       - IPC Identifier of the message. Since the first bit of ID byte is used
                           to distinguish between controller1 and controller2, this bit shall not 
                           be used for IPC ID (message ID). This must be ensured by the calling
                           function.
                           (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)

  Return value:
    UINT32              - Received IPC value

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
UINT32 ipcxSYS_ExChangeUINT32Data (CONST UINT32 u32_ipcSendVal, CONST UINT8 u8_ipcId);

#endif


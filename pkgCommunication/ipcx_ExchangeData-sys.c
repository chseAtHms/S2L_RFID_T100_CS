/***************************************************************************************************
**    Copyright (C) 2017-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: ipcx_ExchangeData-sys.c
**     Summary: This module contains the IPC exchange data functionality (offers functionalities 
**              required by [SRS_28]). This module contains all functions to send and receive data
**              over the IPC to and from the other controller. The IPC is a communication over the 
**              UART interface.
**   $Revision: 4224 $
**       $Date: 2024-01-23 10:39:25 +0100 (Di, 23 Jan 2024) $
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
**             Send
**             CheckCrcAnd_uC_ID_Rx
**             GetBuffer
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "timer-def.h"
#include "gpio-hal.h"
#include "serial-hal.h"
#include "crc-hal.h"
#include "timer-hal.h"
#include "cfg_Config-sys.h"
#include "ipcs_IpcSync-sys.h"
#include "globFit_FitTestHandler.h"

/* Header-file of module */
#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID                 34u

/*
** These constants defines the number of the Rx buffers.
*/
/* Constant which defines Rx buffer number 1. */
#define k_RXBUFF_IDX_0           ( ( UINT8 ) 0u )

/* Constant which defines Rx buffer number 2. */
#define k_RXBUFF_IDX_1           ( ( UINT8 ) 1u )

/* Constant which defines the index size for the Rx buffers. */
#define k_RXBUFF_IDX_SIZE        ( ( UINT8 )2u )

/* This constant defines if data is received. */
#define k_DMA_RX_RECEIVED        ( (UINT32) 0 )

/* Pin definitions for USART useage. */
#define k_GPIOA_USART            ( GPIOA )
#define k_GPIOA_USART_PIN9_TX    ( GPIO_9 )
#define k_GPIOA_USART_PIN10_RX   ( GPIO_10 )

/* Transmit buffer of the IPC. */
STATIC UINT8 au8_TxBuffer[IPCXSYS_SIZEOF_MAX_TGM];

/* Receive buffer of the IPC.
** The receive buffer are double because the IPC telegram is send twice.
*/
STATIC UINT8 au8_RxBuffer[k_RXBUFF_IDX_SIZE][IPCXSYS_SIZEOF_MAX_TGM];

/* Transmit and Receive buffer which are used from the DMA. These buffers are
** attached to the DMA buffer section in the RAM. This is a specified section
** in the RAM just for DMA buffers. It is defined in the scatter file.
**
** Subject: RAM Test
** The DMA buffer section is not under the control of the RAM Test!
*/
STATIC UINT8 au8_TxDmaBuffer[IPCXSYS_SIZEOF_MAX_TGM * 2u]
                             __attribute__((section("DMA_BUFFER_SECTION")));/*lint !e950*/

STATIC UINT8 au8_RxDmaBuffer[IPCXSYS_SIZEOF_MAX_TGM * 2u]
                             __attribute__((section("DMA_BUFFER_SECTION")));/*lint !e950*/
/* The word "attribute" and the words in the brackets are compiler specific syntax. */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC void Send( CONST UINT8 u8_dataLen );
STATIC UINT8 CheckCrcAnd_uC_ID_Rx ( CONST UINT8 u8_dataLen );
STATIC GLOB_RETCODE_ENUM GetBuffer( CONST UINT8 u8_ipcId, 
                                    CONST UINT8 u8_dataLen, 
                                    UINT8 pau8_buffer[] );

/***************************************************************************************************
**    global functions
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
void ipcxSYS_Init( void )
{
   /* configure the GPIO Pin PA9 for USART Tx */
   gpioHAL_ConfigureOutput( k_GPIOA_USART,
      k_GPIOA_USART_PIN9_TX,
      OUT_CONFIG_AF_PP,
      OUT_MODE_50MHZ );

   /* configure the GPIO Pin PA10 for USART Rx */
   gpioHAL_ConfigureInput( k_GPIOA_USART,
      k_GPIOA_USART_PIN10_RX,
      IN_CONFIG_FLOATING );

   /* initialize the IPC communication via USART interface
    * (see [SRS_155], [SRS_118]) */
   serialHAL_Init ( USART1,
                    SERIALHAL_4000000_BAUD,
                    SERIALHAL_USART_SET_8NOP1 );

   /*lint -e923*/
   /* Cast from pointer to int is OK, the function needs the address of the
    * pointer as integer value for the DMA. */
   /* initialize the UART interface for sending data via DMA (see [SRS_155]) */
   serialHAL_InitTxUsart1DmaCh((UINT32) au8_TxDmaBuffer);

   /* initialize the UART interface for reception of data via DMA (see [SRS_155]) */
   serialHAL_InitRxUsart1DmaCh((UINT32) au8_RxDmaBuffer, (IPCXSYS_SIZEOF_MAX_TGM * 2u));
   /*lint +e923*/
   
} /* end of ipcxSYS_Init() */


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
void ipcxSYS_SendUINT32( CONST UINT8  u8_ipcId, CONST UINT32 u32_data )
{
  /* forward to 'generic' send routine */ 
  /* Cast from pointer to pointer is intended here because the underlying function directly writes
  ** into the DMA buffer. 
  */
  ipcxSYS_SendBuffer(u8_ipcId, (UINT8)sizeof(UINT32), (CONST UINT8*)&u32_data); /*lint !e928 */
}

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
void ipcxSYS_SendBuffer( CONST UINT8  u8_ipcId, CONST UINT8 u8_dataLen, CONST UINT8* pau8_buffer)
{
   UINT8                   i;
   CFG_CONTROLLER_ID_ENUM  e_controllerId;
  
   /* if invalid data length */
   if (u8_dataLen > IPCXSYS_SIZEOF_MAX_PAYLOAD)
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
   }
   /* else: valid length */
   else
   {
     /* FIT to manipulate data exchange */
     /* RSM_IGNORE_BEGIN */
     /* Code only used for FIT test so RSM warnings not relevant */
     GLOBFIT_FITTEST(GLOBFIT_CMD_IPC_TIMEOUT_RX, goto _FitRx);
     /* RSM_IGNORE_END */
     
     /* reinitialize UART Rx DMA */
     /* Cast from pointer to int is OK, the function needs the address of the
      * pointer as integer value for the DMA. */
     serialHAL_InitRxUsart1DmaCh( (UINT32)au8_RxDmaBuffer,                /*lint !e923*/
                                  (2U * IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)) );

     /* It must be avoided, that one controller already sends data, but the other controller has 
     ** not yet restarted its UART Rx DMA. So an additional GPIO synchronization is done here. */
     ipcsSYS_Sync();

#ifdef GLOBFIT_FITTEST_ACTIVE
_FitRx: /* CCT_NO_PRE_WARNING, only used for FIT test */
#endif 

     /* get the controller ID (see [SRS_345]) */
     e_controllerId = cfgSYS_GetControllerID();
  
     /* in invalid controller ID */
     if ( ( e_controllerId != SAFETY_CONTROLLER_1 ) &&
          ( e_controllerId != SAFETY_CONTROLLER_2 ) )
     {
       GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
     }
     /* else: valid controller ID */
     else
     {
       /* write IPC id and controller id (see [SRS_276]) to the TX buffer (see [SRS_345],
        * [SRS_2169]) */
       au8_TxBuffer[IPCXSYS_IDX_ID] = ( u8_ipcId & IPCXSYS_IPC_ID_MASK) \
       | ((( UINT8 ) e_controllerId ) & IPCXSYS_CONTROLLER_ID_MASK );
    
       /*FIT to manipulate the telegram ID*/
       GLOBFIT_FITTEST( GLOBFIT_CMD_IPC_ID,
                        au8_TxBuffer[IPCXSYS_IDX_ID]++);
        
       /* write payload into the TX buffer (see [SRS_2169]) */
       for ( i = 0u; i < u8_dataLen; i++)
       {
          /*lint -esym(960, 17.4)*/
          /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
           * other than array indexing used
           * --> Deactivate Misra Rule 17.4, Array-Indexing is done */
          /* Info 835: A zero has been given as left argument to operator '+'
           * --> Done because of Array-Indexing */
          /* get the payload data from buffer step by step and write it to the
           * TX buffer */
          au8_TxBuffer[i + IPCXSYS_IDX_PAYLOAD] = pau8_buffer[i]; /*lint !e835 */
          /*lint +esym(960, 17.4)*/
       }
    
       /* send data to the other controller */
       Send(u8_dataLen);
     }
   }
}

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
void ipcxSYS_GetBufferinclWait ( CONST UINT8 u8_ipcId, CONST UINT8 u8_dataLen, UINT8* pau8_buffer)
{
   UINT32             u32_startTimeout;
   UINT32             u32_timeout;
   GLOB_RETCODE_ENUM  e_ipcRetVal;

   /* if invalid data length */
   if (u8_dataLen > IPCXSYS_SIZEOF_MAX_PAYLOAD)
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(3u));
   }
   /* else: valid length */
   else
   {
     e_ipcRetVal = GetBuffer(u8_ipcId, u8_dataLen, pau8_buffer);
  
     /* get the actual system time for IPC-Timeout calculation */
     u32_startTimeout = timerHAL_GetSystemTime3();
  
     /* wait time x till the IPC data received */
     while (e_ipcRetVal == GLOB_RET_BUSY)
     {
        /* IPC timeout calculation */
        u32_timeout = timerHAL_GetSystemTime3();
        u32_timeout = u32_timeout - u32_startTimeout;
       
        /* enter FS state in case of timeout (see [SRS_31]) */
        /* if timeout */
        if (u32_timeout >= IPCXSYS_WAITTIMEOUT)
        {
          GLOBFAIL_SAFETY_HANDLER(GlOB_FAILCODE_IPCX_TIMEOUT, GLOBFAIL_ADDINFO_FILE(4u));
          /* set condition to leave while-loop (onyl for unit test) */
          e_ipcRetVal = GLOB_RET_ERR;
        }
        /* else: no timeout */
        else
        {  
          /* get data from the IPC */
          e_ipcRetVal = GetBuffer(u8_ipcId, u8_dataLen, pau8_buffer);
        }
     }
   }
}

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
UINT32 ipcxSYS_GetUINT32inclWait ( CONST UINT8 u8_ipcId )
{
   /* return value */
   UINT32 u32_retVal;
  
   /* forward to 'generic' get routine */
   /* Cast from pointer to pointer is intended here because the underlying function read from the
   ** the DMA buffer. 
   */
   ipcxSYS_GetBufferinclWait (u8_ipcId, (UINT8)sizeof(UINT32), (UINT8*)&u32_retVal); /*lint !e928 */
  
   return u32_retVal;
}

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
UINT32 ipcxSYS_ExChangeUINT32Data (CONST UINT32 u32_ipcSendVal, CONST UINT8 u8_ipcId)
{
   UINT32            u32_retVal = 0u;
   UINT32            u32_startTimeout;
   UINT32            u32_timeout;
   GLOB_RETCODE_ENUM e_ipcRetVal = GLOB_RET_BUSY;
  
   /* exchange test-results */
   /* Cast from pointer to pointer is intended here because the underlying function directly writes
   ** into the DMA buffer. 
   */
   ipcxSYS_SendBuffer( u8_ipcId, 
                      (UINT8)sizeof(UINT32), 
                      (CONST UINT8*)&u32_ipcSendVal ); /*lint !e928 */

   /* get the actual system time for IPC-Timeout calculation */
   u32_startTimeout = timerHAL_GetSystemTime3();

   do
   {
      /* IPC timeout calculation */
      u32_timeout = timerHAL_GetSystemTime3();
      u32_timeout = u32_timeout - u32_startTimeout;
      /* enter FS state in case of timeout (see [SRS_31]) */
      /* if timeout */
      if (u32_timeout >= IPCXSYS_TIMEOUT)
      {
        GLOBFAIL_SAFETY_HANDLER(GlOB_FAILCODE_IPCX_TIMEOUT, GLOBFAIL_ADDINFO_FILE(5u));
        /* set condition to leave do..while-loop (onyl for unit test) */
        e_ipcRetVal = GLOB_RET_ERR;
      }
      /* else: no timeout */
      else
      {
        /* get data from the IPC */
        /* Cast from pointer to pointer is intended here because the underlying function reads from 
        ** the DMA buffer. 
        */
        e_ipcRetVal = GetBuffer(u8_ipcId, 
                                (UINT8)sizeof(UINT32), 
                                (UINT8*)&u32_retVal); /*lint !e928 */
      }

   } /* wait till the IPC data received */
   while (e_ipcRetVal == GLOB_RET_BUSY);
   
   /* if unexpected IPC result */
   if (e_ipcRetVal != GLOB_RET_OK)
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_IPCX_RETURN_ERR, GLOBFAIL_ADDINFO_FILE(6u));
   }
   /* everything fine */
   else
   {
     /* empty branch */
   }

   return u32_retVal;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    Send

  Description:
    This function sends the data over the IPC to the other controller.
    In the first steps this function calculates the crc of the data and writes it to the DMA 
    buffer for transmitting. Then a DMA transfer is initiated.

  See also:
    -

  Parameters:
    u8_dataLen (IN)   - Length of payload to send
                        (valid range: 0..IPCXSYS_SIZEOF_MAX_PAYLOAD, not checked, 
                        already checked in ipcxSYS_SendBuffer)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
STATIC void Send( CONST UINT8 u8_dataLen )
{
   UINT8 i;
   UINT8 u8_crc    = IPCXSYS_CRC_INITIAL_VALUE;

   /* copy ID and Payload to DMA buffer using for-loop  */
   for (i = 0u; i < IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen); i++)
   {
      /* calculate the checksum over the data to send (see [SRS_327]) */
      u8_crc = crcHAL_CalcCRC8(u8_crc, au8_TxBuffer[i]);

      /* copy every byte into the DMA send buffer [SRS_2169] */
      au8_TxDmaBuffer[i] = au8_TxBuffer[i];
      /* send IPC message twice, see [SRS_119] */
      au8_TxDmaBuffer[i + IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)] = au8_TxBuffer[i];
   }

   /* write the CRC to the DMA send buffer [SRS_2169] */
   au8_TxDmaBuffer[IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen)] = u8_crc;
   /* send IPC message twice, see [SRS_119] */
   au8_TxDmaBuffer[IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen)+IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)] = u8_crc;

   /* FIT to manipulate data exchange */
   /* RSM_IGNORE_BEGIN */
   /* Code only used for FIT test so RSM warnings not relevant */
   GLOBFIT_FITTEST(GLOBFIT_CMD_IPC_TIMEOUT_TX, goto _FitTx);
   /* RSM_IGNORE_END */

   /* reinitialize UART Tx DMA,this ensures that CPAR (DMA channel peripheral address register) and
   ** CMAR  (DMA channel memory address register) always contains the correct values since these 
   ** register are not included in the SFR test */
   /* Cast from pointer to int is OK, the function needs the address of the
    * pointer as integer value for the DMA. */
   serialHAL_InitTxUsart1DmaCh((UINT32) au8_TxDmaBuffer); /*lint !e923 */
   /* trigger sending of TxDMA Buffer */
   serialHAL_SendUsart1Dma(2U * IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen));

#ifdef GLOBFIT_FITTEST_ACTIVE
_FitTx: /* CCT_NO_PRE_WARNING, only used for FIT test */
   __NOP();
#endif

}


/***************************************************************************************************
  Function:
    CheckCrcAnd_uC_ID_Rx

  Description:
    This function calculates the checksum over the received IPC telegram and checks it against 
    the received. If there is an error in the first telegram the second one is checked.
    If everything is OK the specified buffer number is returned, else the Safety Handler will 
    be called.
    After checksum checking the received controller ID of the tgm is checked. If the received ID 
    equals the own ID, a failure is detected and the Safety Handler will be called.

  See also:
    -

  Parameters:
    u8_dataLen (IN)   - Length of payload to send
                        (valid range: 0..IPCXSYS_SIZEOF_MAX_PAYLOAD, not checked, 
                        already checked in ipcxSYS_SendBuffer)

  Return value:
    UINT8              - Used buffer index

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
STATIC UINT8 CheckCrcAnd_uC_ID_Rx ( CONST UINT8 u8_dataLen )
{
   UINT8                  i;
   UINT8                  u8_crc    = IPCXSYS_CRC_INITIAL_VALUE;
   UINT8                  u8_rxBuffIdx = k_RXBUFF_IDX_0;
   CFG_CONTROLLER_ID_ENUM e_controllerId;

   /* FIT to manipulate the received checksum in both buffers
    * (1st step: flips the LSB of the buffer1-crc) */
   /* RSM_IGNORE_BEGIN */
   /* Code only used for FIT test so RSM warnings not relevant */
   GLOBFIT_FITTEST_NOCMDRESET(GLOBFIT_CMD_IPC_CRC,
                   (au8_RxBuffer[k_RXBUFF_IDX_0][3] ^= 1u));
   /* RSM_IGNORE_END */

   /* calculate the CRC of first buffer */
   for ( i = 0u; i < IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen); i++ )
   {
      /* calculated the CRC of the received data (see [SRS_327]) */
      u8_crc = crcHAL_CalcCRC8 ( u8_crc, au8_RxBuffer[k_RXBUFF_IDX_0][i]);
   }

   /* check the calculated CRC with the received CRC of the RX buffer */
   if ( au8_RxBuffer[ k_RXBUFF_IDX_0 ][ IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen) ] != u8_crc )
   {
      /* the CRC of the RX buffer is wrong, try the next buffer (see [SRS_119]) */
      u8_crc = IPCXSYS_CRC_INITIAL_VALUE;

      /* FIT to manipulate the received checksum in both buffers
       * (2nd step: flips the LSB of the buffer2-crc) */
      GLOBFIT_FITTEST(GLOBFIT_CMD_IPC_CRC,
                    (au8_RxBuffer[k_RXBUFF_IDX_1][4] ^= 0x80u));

      /* calculate CRC result of second buffer (see [SRS_119], [SRS_327]) */
      for (i = 0u; i < IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen); i++)
      {
         u8_crc = crcHAL_CalcCRC8 ( u8_crc, au8_RxBuffer[k_RXBUFF_IDX_1][i]);
      }

      /* check the CRC of the second buffer */
      if ( au8_RxBuffer[ k_RXBUFF_IDX_1 ][ IPCXSYS_SIZEOF_DATA_VAR(u8_dataLen) ] != u8_crc )
      {
         /* both telegrams are wrong, go to the FS state (see [SRS_119]) */
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_IPCX_CRC, GLOBFAIL_ADDINFO_FILE(7u));
         /* set return value, only used for unit test */
         u8_rxBuffIdx = k_RXBUFF_IDX_1;
      }
      else
      {
         /* the CRC of the second buffer is OK return the number of the second buffer */
         u8_rxBuffIdx = k_RXBUFF_IDX_1;
      }
   }
   else
   {
      /* CRC of the first buffer is OK return number of the first buffer */
      u8_rxBuffIdx = k_RXBUFF_IDX_0;
   }


   /* Get the controller ID and check it against the ID in the telegram.
    * If it is the same ID, a failure is detected, e.g. the controller speaks
    * with himself. */
   e_controllerId = cfgSYS_GetControllerID();

   /* check the controller ID (see [SRS_649]) */
   /* in invalid controller ID */
   if ( ( e_controllerId != SAFETY_CONTROLLER_1 ) &&
        ( e_controllerId != SAFETY_CONTROLLER_2 ) )
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(8u));
     /* set return value, only used for unit test */
     u8_rxBuffIdx = k_RXBUFF_IDX_1;
   }
   /* else: valid controller ID */
   else
   {
     /* FIT to manipulate the the controller ID. */
     GLOBFIT_FITTEST(GLOBFIT_CMD_IPC_CONTROLLER,
                     e_controllerId =
                     (CFG_CONTROLLER_ID_ENUM)au8_RxBuffer[u8_rxBuffIdx][IPCXSYS_IDX_ID]);
  
     /* check the received ID, see [SRS_276] */
     if (    ( ( ( UINT8 ) e_controllerId ) & IPCXSYS_CONTROLLER_ID_MASK)
        == ( au8_RxBuffer[ u8_rxBuffIdx ][ IPCXSYS_IDX_ID ] & IPCXSYS_CONTROLLER_ID_MASK ) )
     {
        GLOBFAIL_SAFETY_HANDLER( GLOB_FAILCODE_IPCX_CONTROLLER_ID, GLOBFAIL_ADDINFO_FILE(9u));
        /* set return value, only used for unit test */
        u8_rxBuffIdx = k_RXBUFF_IDX_1;
     }
     /* valid ID */
     else
     {
       /* empty branch */
     }
   }

   return u8_rxBuffIdx;

}


/***************************************************************************************************
  Function:
    GetBuffer

  Description:
    This function returns the data after an IPC transfer. In the first step the received data is 
    copied out of the DMA buffer. Then a CRC and Tgm-ID check is executed. After this the received 
    data is copied into the output buffer.
    In case of a failure, the Safety Handler will be called. If the IPC transfer is still busy, 
    the return value is GLOB_RET_BUSY.

  See also:
    -

  Parameters:
    u8_ipcId (IN)     - ID of the IPC telegram. Since the highest bit of the transferred ID byte
                        is used to distinguish between controller1 and controller2, this bit 
                        shall not be used for an IPC ID (message ID). This must be ensured by the 
                        calling function.
                        (valid range: 0..IPCXSYS_IPC_ID_MASK, not checked)
    u8_dataLen (IN)   - Length of payload to receive
                        (valid range: 0..IPCXSYS_SIZEOF_MAX_PAYLOAD, not checked, must be done 
                        by calling function)
    pau8_buffer (OUT) - Buffer for reception.
                        (valid range: <>NULL, not checked because only called by reference)

  Return value:
    GLOB_RET_ERR      - Error (e.g. wrong CRC)
    GLOB_RET_BUSY     - Still waiting on bytes
    GLOB_RET_OK       - OK

  Remarks:
    Context: IRQ Scheduler
    Context: main (during initialization)

***************************************************************************************************/
STATIC GLOB_RETCODE_ENUM GetBuffer( CONST UINT8 u8_ipcId, 
                                    CONST UINT8 u8_dataLen,
                                    UINT8 pau8_buffer[] )
{
   UINT8             i;
   UINT8             u8_rxBuffIdx  = 0u;
   GLOB_RETCODE_ENUM e_retVal   = GLOB_RET_ERR;

   /*  get the CRC from TX DMA buffer first */
   if ( k_DMA_RX_RECEIVED != (DMA1_Channel5->CNDTR) )
   {
      /* still waiting for all Bytes to be received  */
      e_retVal = GLOB_RET_BUSY;
   }
   else
   {
      /* write/split the data from the DMA Rx buffer to the two Rx buffers */
      for (i = 0u; i < (IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)); i++)
      {
         /* copy the RX data from DMA Buffer to Rx buffer */
         au8_RxBuffer[k_RXBUFF_IDX_0][i] = au8_RxDmaBuffer[i];

         au8_RxBuffer[k_RXBUFF_IDX_1][i] = au8_RxDmaBuffer[i + IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)];

         /* clear the Rx DMA buffer */
         au8_RxDmaBuffer[i] = ( UINT8 ) 0u;
         au8_RxDmaBuffer[i + IPCXSYS_SIZEOF_TGM_VAR(u8_dataLen)] = ( UINT8 ) 0u;

      }

      /* check CRC and uC ID here to check if received telegram correctly received in one of the 
       * two buffers function will return a buffer index or enter safe state */
      u8_rxBuffIdx = CheckCrcAnd_uC_ID_Rx(u8_dataLen);

      /* check if received ID matches requested ID (see [SRS_649])*/
      if ( ( au8_RxBuffer[u8_rxBuffIdx][IPCXSYS_IDX_ID] & IPCXSYS_IPC_ID_MASK) != u8_ipcId )
      {
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_IPCX_IPC_ID, GLOBFAIL_ADDINFO_FILE(10u));
         /* set return value, only used for unit test */
         e_retVal = GLOB_RET_ERR;
      }
      else
      {
        /*lint -e835*/
        /* Deactivated lint message because the constant IPCXSYS_IDX_PAYLOAD
         * is made up of the IPC ID index which is 0 plus the size of the
         * IPC ID. */
        
        for (i = 0u; i < u8_dataLen; i++)
        {
          /* write the data of the valid rx-buffer to the receive buffer */
          pau8_buffer[i] = au8_RxBuffer[ u8_rxBuffIdx ][ IPCXSYS_IDX_PAYLOAD + i];
        }
        e_retVal = GLOB_RET_OK;

         /*lint +e835*//* activate lint message 835. */
      }
   }

   return e_retVal;
} 


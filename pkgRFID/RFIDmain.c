/**************************************************************************************************    
**    Copyright (C) 2016 HMS Technology Center Ravensburg GmbH, all rights reserved    
***************************************************************************************************    
**    
**        File: RFID.c    
**     Summary: This file provides the functionality to access the RFID reader    
**   $Revision: $    
**       $Date: $  
**     Author: ChSe    
**    
***************************************************************************************************    
***************************************************************************************************    
**    
** Functions:    
**    
**    
***************************************************************************************************    
**    Template Version 5    
**************************************************************************************************/    

#ifdef RFID_ACTIVE

/**************************************************************************************************
**    Include Files
**************************************************************************************************/
/* Project header */
#include "stm32f10x.h"
#include <stddef.h> // für size_t
#include "xtypes.h"
#include "xdefs.h"

#include "cfg_Config-sys.h"
#include "gpio-hal.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "RFID.h"
/**************************************************************************************************
**    global variables
**************************************************************************************************/

/**************************************************************************************************
**    static constants, types, macros, variables
**************************************************************************************************/
/* This constant defines the system clock for the USART usage */
#define RFID_SYSCLOCK       GLOBDEF_SYSTEM_CLOCK

#define RFID_DMA_CHANNEL_TX ( DMA1_Channel2 )
#define RFID_DMA_CHANNEL_RX ( DMA1_Channel3 )

/* Time measurement */
#define TIMER_START(timer)  timerStart(&s_TimeDurations.timer)
#define TIMER_STOP(timer)   timerStop(&s_TimeDurations.timer)



#define RFID_MAX_LEN 96u
#define RFID_ERROR_RES_LEN 3u

#define RFID_CMD_VE_LEN 4u
#define RFID_EXPEC_RES_VE_LEN 70u
#define RFID_RES_VE_ERROR_LEN 3u

#define RFID_CMD_SF_LEN 4u
#define RFID_EXPEC_RES_SF_LEN 11u
#define RFID_RES_SF_ERROR_LEN 3u

// Command to read Single Read Words (Record )
#define RFID_CMD_SR_LEN 10u
#define RFID_EXPEC_RES_SR_LEN 15u
#define RFID_RES_SR_ERROR_LEN 3u

// CRC-parmeter 
#define CRC_POLY 0x04C11DB7
// Only for test purposes, seed is actually a safe configuration parameter
#define CRC_SEED 0xFFFFFFFF
#define CRC_XOROUT 0xFFFFFFFF

typedef struct
{
  UINT32 u32_cur;
  UINT32 u32_max;
} t_TIME;

typedef struct
{
  t_TIME s_BootFirmware;
  t_TIME s_readUid;
  t_TIME s_readRecord;
} t_TIME_DURATIONS;

t_TIME_DURATIONS s_TimeDurations;

/* Transmit and Receive DMA buffer which are used from the DMA. These buffers are
** attached the DMA buffer section in the RAM. This is a specified section in the
** RAM just for DMA buffers. It is defined in the scatter file.
**
** Subject: RAM Test
** The DMA buffer section is not under the control of the RAM Test!
*/
UINT8 au8_rfidDmaBufferTx[255] __attribute__((section("DMA_BUFFER_SECTION")));
UINT8 au8_rfidDmaBufferRx[255] __attribute__((section("DMA_BUFFER_SECTION")));

/* RFID reader access state */

t_RFID_TAG_READ_STATE e_rfidAccessState;

t_RFID_FAILURE e_rfidLastFailure = RFID_FAIL_NONE;
UINT8 u8_rfidFailureCount = 0u;


t_RFID_TAG_DATA s_rfidTagRecordEven;
t_RFID_TAG_DATA s_rfidTagRecordOdd;

/* Store the lower 4 bytes of the RFID tag UID */
UINT32 u32_RfidInfo = 0uL;

/* RFID information */

/**************************************************************************************************
**    static function prototypes
**************************************************************************************************/



/* RFID Commands */
STATIC void RFID_BootReader(void);
STATIC void RFID_FrameTxSingleReadFixCode(void);
STATIC void RFID_FrameTxReadRecord_0(void);
STATIC void RFID_FrameTxReadRecord_1(void);

/* RFID Response verification  */
STATIC UINT8 RFID_VerifySWVersion(const UINT8 *buffer);
STATIC UINT8 RFID_VerifySingleReadFixCode(const UINT8 *buffer, t_RFID_RAW_DATA *s_rfidRawData);
STATIC UINT8 RFID_VerifySingleReadWord(const UINT8 *buffer, t_RFID_RAW_DATA *s_rfidRawData);
STATIC UINT8 RFID_VerifyChecksum(const UINT8 *buffer , UINT8 length);
STATIC UINT8 RFID_CalculateChecksum(const UINT8 * data, size_t length);
/* Tag data verification */
STATIC UINT8 RFID_ParseSingleFixCode(t_RFID_RAW_DATA *s_rfidRawData, t_RFID_TAG_DATA *s_rfidTagData);
STATIC UINT8 RFID_ParseRecord(t_RFID_RAW_DATA *s_rfidRawData, t_RFID_TAG_DATA *s_rfidTagData);
STATIC size_t RFID_ResLength(const UINT8 *buffer, size_t maxLength);
STATIC UINT32 RFID_CalculateCRC(t_RFID_TAG_DATA *s_rfidTagData);
STATIC UINT8 RFID_CheckTagRecordFields(void);
/* Utility functions */
STATIC void uartInit(void);
STATIC void uartInitDmaTx(void);
STATIC void uartInitDmaRx(void);
STATIC void RFID_FrameTxTrigger(UINT8 u8_len);
STATIC void RFID_FrameRxInit(UINT8 u8_len);
STATIC void RFID_HandleFailure(t_RFID_FAILURE e_failure);
STATIC void timerStart(t_TIME *ps_timer);
STATIC void timerStop(t_TIME *ps_timer);
STATIC UINT32 CRC32(const UINT8* data, UINT8 length);

/**************************************************************************************************
**    global functions
**************************************************************************************************/

/**************************************************************************************************
**
**  Function:
**    void RFID_Init(void)
**
**  Description:
**    This function initializes the RFID reader.
**
**  See also:
**    -
** 
**  Parameters:
**    -
**
**  Return value:
**    -
**************************************************************************************************/
void RFID_Init(void)
{
  /* Initialize the RFID reader */
  if (cfgSYS_GetControllerID() == SAFETY_CONTROLLER_1)
  {
    uartInit();
   

    uartInitDmaTx();
    uartInitDmaRx();
    e_rfidAccessState = TX_BOOT_FIRMWARE;
  }
}

/**************************************************************************************************
**
**  Function:
**    UINT32 RFID_InfoGet(void)
**
**  Description:
**    This function returns the UID of the verified RFID tag.
**
**  See also:
**    -
**  Parameters:
**    -
**  Return value:
**    u32_RfidInfo - The lower 4 bytes of the RFID tag UID
**************************************************************************************************/
UINT32 RFID_InfoGet(void)
{
  return u32_RfidInfo;
}

/**************************************************************************************************
**
**  Function:
**    void RFID_InfoSet(UINT32 u32_rfidInfo)
**
**  Description:
**    This function enables the to reset the RFID information after it was sent to 
**    the other controller.
**
**  See also:
**    -
**  Parameters:
**    u32_rfidInfo (IN) - The new RFID information to be set
**  Return value:
**    -
**************************************************************************************************/
void RFID_InfoSet(UINT32 u32_rfidInfo)
{
  u32_RfidInfo = u32_rfidInfo;
}

/**************************************************************************************************
**
**  Function:
**    void RFID_Reader_Boot(void)
**
**  Description:
**    This function processes the RFID reader.
**
**  See also:
**    -
**
**  Parameters:
**    -
**
**  Return value:
**    -
**************************************************************************************************/
void RFID_Reader_Boot(void)
{
  //STATIC UINT32 u32_funcCallCount = 0u;
  switch (e_rfidAccessState)
  {
    case TX_BOOT_FIRMWARE:    
    {
      //u32_funcCallCount = 0u;
      TIMER_START(s_BootFirmware);
      RFID_BootReader();
      RFID_FrameRxInit(RFID_EXPEC_RES_VE_LEN);
      e_rfidAccessState = RX_BOOT_FIRMWARE;
      break;
    }
    case RX_BOOT_FIRMWARE:
    { 
      //u32_funcCallCount++;
      // Function calls * t100 time slice is 100 us
      //UINT32 u32_timeElapsed = u32_funcCallCount * T100_TIME_SLICE_US;

      UINT32 u32_elapsed = timerHAL_GetSystemTime3() - s_TimeDurations.s_BootFirmware.u32_cur;
      // Wait for the DMA transfer to complete
      if (RFID_DMA_CHANNEL_RX->CNDTR == 0)
      {
        TIMER_STOP(s_BootFirmware);
        if (RFID_VerifySWVersion(au8_rfidDmaBufferRx) == RFID_OK)
        {
          e_rfidAccessState = TX_READ_UID;
        }
        else
        {
          RFID_HandleFailure(RFID_FAIL_BOOT_READER);
        }
      }
      else if (u32_elapsed > RFID_BOOT_READER_TIMEOUT)
      {
        // Timeout occurred, handle failure
        RFID_HandleFailure(RFID_FAIL_BOOT_READER_TIMEOUT);
      }
      break; 
    }
    default:
    {
      break;
    }
  }
}

/**************************************************************************************************
**
**  Function:
**    void RFID_ReadTag(void)
**
**  Description:
**    This function implements the state machine for reading the RFID tag.
**
**  See also:
**    -
**  Parameters:
**    -
**  Return value:
**    RFID_OK (0) - Response is OK
**************************************************************************************************/
 void RFID_ReadTag(void)
 {
   t_RFID_RAW_DATA s_rfidRawData;
   t_RFID_TAG_DATA s_rfidTagData;

   /* Check if failure count exceeds maximum allowed */
   if (u8_rfidFailureCount  > RFID_MAX_FAILURE_COUNT)
   {
      e_rfidAccessState = STATE_FAILURE;
   }

   switch (e_rfidAccessState)
   {
     case TX_READ_UID:
     {
        TIMER_START(s_readUid);
        RFID_FrameTxSingleReadFixCode();
        RFID_FrameRxInit(RFID_EXPEC_RES_SF_LEN);
        e_rfidAccessState = RX_READ_UID;
        break;
     }
     case RX_READ_UID:
     {
        UINT32 u32_elapsed = timerHAL_GetSystemTime3() - s_TimeDurations.s_readUid.u32_cur;
        UINT8 dma_rx_len = RFID_EXPEC_RES_SF_LEN - RFID_DMA_CHANNEL_RX->CNDTR;

        if (dma_rx_len >= RFID_ERROR_RES_LEN)
        //if (RFID_DMA_CHANNEL_RX->CNDTR == 0) 
        {
          if (au8_rfidDmaBufferRx[dma_rx_len -1u] == RFID_ETX) // Check for ETX
          {
            TIMER_STOP(s_readUid);
            // Verify the received Single Read Fix Code message
            UINT8 res = RFID_VerifySingleReadFixCode(au8_rfidDmaBufferRx, &s_rfidRawData);
            if( res == RFID_OK)
            {
              RFID_ParseSingleFixCode(&s_rfidRawData, &s_rfidTagData);
              /* Copy Fix Code and store it in both records for R_CRC calculation */
              UINT8 i;
              for (i = 0; i < RFID_UID_LEN; i++)
              {
                s_rfidTagRecordEven.au8_tag_uid[i] = s_rfidTagData.au8_tag_uid[i];
                s_rfidTagRecordOdd.au8_tag_uid[i] = s_rfidTagData.au8_tag_uid[i];
              }
              // Tag found, proceed to read the even record
              e_rfidAccessState = TX_READ_REC_EVEN;
            }
            else if (res == RFID_NO_TAG)
            {
                // No tag detected, retry reading
                e_rfidAccessState = TX_READ_UID;
                u32_RfidInfo = 0uL; // Reset RFID info
            }
            else
            {
              /* Handle failure */
              RFID_HandleFailure(RFID_FAIL_UID_VERIFY);
            } 
          }
        }
        else 
        {
          // Check for timeout
          if (u32_elapsed > RFID_READ_UID_TIMEOUT)
          {
            // Timeout occurred, go back to retry reading UID
            e_rfidAccessState = TX_READ_UID;
          }

        }
      break;
     }
     case TX_READ_REC_EVEN: 
     {  
        TIMER_START(s_readRecord);
        RFID_FrameTxReadRecord_0();
        RFID_FrameRxInit(RFID_EXPEC_RES_SR_LEN);
        e_rfidAccessState = RX_READ_REC_EVEN;
        break;  
     }
     case RX_READ_REC_EVEN:
     {
      UINT32 u32_elapsed = timerHAL_GetSystemTime3() - s_TimeDurations.s_readRecord.u32_cur;
      if (RFID_DMA_CHANNEL_RX->CNDTR == 0) 
      {
        TIMER_STOP(s_readRecord);
        if (RFID_VerifySingleReadWord(au8_rfidDmaBufferRx, &s_rfidRawData) == RFID_OK)
        { 
          if (RFID_ParseRecord(&s_rfidRawData, &s_rfidTagRecordEven) == RFID_OK)
          {
            e_rfidAccessState = CHECK_REC_EVEN;
          }
          else
          {
            RFID_HandleFailure(RFID_FAIL_EVEN_REC_VERIFY);
          }
        } 
        else
        {
          RFID_HandleFailure(RFID_FAIL_EVEN_REC_VERIFY);
        }
      }
      else
      {
        // Check for timeout
        if (u32_elapsed > RFID_READ_REC_TIMEOUT)
        {
          // Timeout occurred, go back to retry reading UID
          e_rfidAccessState = TX_READ_UID;
        }
      }
      break;
     }
     case CHECK_REC_EVEN:
     {
        if (RFID_CalculateCRC(&s_rfidTagRecordEven) == RFID_OK)
        {
          e_rfidAccessState = TX_READ_REC_ODD;
        }
        else
        {
          RFID_HandleFailure(RFID_FAIL_EVEN_REC_CRC_ERROR);
        }
         break;
     }
     case TX_READ_REC_ODD:
     {
        TIMER_START(s_readRecord);
        RFID_FrameTxReadRecord_1();
        RFID_FrameRxInit(RFID_EXPEC_RES_SR_LEN);
        e_rfidAccessState = RX_READ_REC_ODD;      
      break;
     }
     case RX_READ_REC_ODD:
     {
        UINT32 u32_elapsed = timerHAL_GetSystemTime3() - s_TimeDurations.s_readRecord.u32_cur;
        if (RFID_DMA_CHANNEL_RX->CNDTR == 0) 
        {
          TIMER_STOP(s_readRecord);
          if (RFID_VerifySingleReadWord(au8_rfidDmaBufferRx, &s_rfidRawData) == RFID_OK)
          { 
            if (RFID_ParseRecord(&s_rfidRawData, &s_rfidTagRecordOdd) == RFID_OK)
            {
              e_rfidAccessState = CHECK_REC_ODD;
            } 
            else
            {
              RFID_HandleFailure(RFID_FAIL_ODD_REC_VERIFY);
            }
          } 
          else
          {
          RFID_HandleFailure(RFID_FAIL_ODD_REC_VERIFY);
          }
        }
        else
        {
          if (u32_elapsed > RFID_READ_REC_TIMEOUT)
          {
            //Timeout occurred, go to retry reading UID
            e_rfidAccessState = TX_READ_UID;
          }
        }
        break;
     }
     case CHECK_REC_ODD:
     {
        if (RFID_CalculateCRC(&s_rfidTagRecordOdd) == RFID_OK)
        {
          e_rfidAccessState = CHECK_TAG_RECORDS;
        } 
        else
        {
          RFID_HandleFailure(RFID_FAIL_ODD_REC_CRC_ERROR);
        }
      break;
     }
     case CHECK_TAG_RECORDS:
     {
        if (RFID_CheckTagRecordFields() == RFID_OK)
        {
          e_rfidAccessState = TX_READ_UID;
        }
        else
        {
          RFID_HandleFailure(RFID_FAIL_UNKNOWN);
        }
        break;
     }
     case STATE_FAILURE:
     {
      /* Set global T100 fail safe */
     }
     default:
     {
       // Handle other states or errors
       break;
     }
    }  
  }

/**************************************************************************************************
**    static functions
**************************************************************************************************/

/**************************************************************************************************
**
**  Function:
**    void RFID_VerifySWVersion(void)
**
**  Description:
**    This function verifies the software version of the RFID reader.
**
**  See also:
**    -
**  Parameters:
**    buffer (IN) - Pointer to the buffer containing the response from the RFID reader
**
**  Return value:
**    RFID_OK (0) - Response is OK
**    RFID_SYNTAX_ERROR (4) - Syntax error
**    RFID_CHCK_ERROR (3) - Checksum error
*****************************************************************************************************/
STATIC UINT8 RFID_VerifySWVersion(const UINT8 *buffer)
{
  UINT8 au8_rfidRxBuffer[75];
  // Search for ETX in the buffer
  UINT8 msg_len = RFID_ResLength(buffer, RFID_EXPEC_RES_VE_LEN);
  // Check if the message length is valid
  if (msg_len < 3)
  {
    return RFID_SYNTAX_ERROR;
  }
  // Copy buffer to temporary working buffer
  UINT8 i;
  for (i = 0; i < RFID_EXPEC_RES_VE_LEN; i++)
  {
    au8_rfidRxBuffer[i] = buffer[i];
  }

  if(RFID_VerifyChecksum(au8_rfidRxBuffer, msg_len) != RFID_OK) 
  {
    return RFID_CHCK_ERROR;
  }
  // Check the checksum
  // UINT8 receivedChecksum = au8_rfidRxBuffer[RFID_EXPEC_RES_VE_LEN - 2];
  // UINT8 calculatedChecksum = RFID_CalculateChecksum(au8_rfidRxBuffer, RFID_EXPEC_RES_VE_LEN - 2);
  // if (receivedChecksum != calculatedChecksum)
  // {
  //   return RFID_CHCK_ERROR;
  // } 

  return RFID_OK;
}

/**************************************************************************************************
**
**  Function:
**    UINT8 RFID_VerifySingleReadFixCode(const UINT8 *buffer)
**
**  Description:
**    This function verifies the received Single Read Fix Code message and stores the data
**    in the raw data structure to prevent overwriting of the DMA buffer.
**    It checks the checksum and the length of the message.
** 
**  See also:
**    -
**
**  Parameters:
**    buffer (IN) - Pointer to the received dma buffer
**
**  Return value:
**    RFID_OK (0) - Response is OK
**    RFID_CHCK_ERROR (3) - Checksum error
**    RFID_SYNTAX_ERROR (4) - Syntax error
**************************************************************************************************/
STATIC UINT8 RFID_VerifySingleReadFixCode(const UINT8 *buffer, t_RFID_RAW_DATA *s_rfidRawData)
{
  UINT8 au8_rfidRxBuffer[75];
  // Copy buffer to temporary working buffer to prevent overwriting of DMA buffer
  UINT8 i; 
  for (i = 0; i < RFID_EXPEC_RES_SF_LEN; i++)
  {
    au8_rfidRxBuffer[i] = au8_rfidDmaBufferRx[i];
  }
  /* Calculate the received message length */ 
  UINT8 msg_len = RFID_ResLength(au8_rfidDmaBufferRx, RFID_EXPEC_RES_SF_LEN); 
  if (msg_len < 3)
  {
    return RFID_SYNTAX_ERROR;
  } 
  // Check the checksum
  if(RFID_VerifyChecksum(au8_rfidRxBuffer, msg_len) != RFID_OK) 
  {
    return RFID_CHCK_ERROR;
  }
 
   // Fill raw data structure
  s_rfidRawData->u8_status = au8_rfidRxBuffer[0];
  s_rfidRawData->u8_CHCK = au8_rfidRxBuffer[msg_len - 2];
  s_rfidRawData->u8_ETX = au8_rfidRxBuffer[msg_len - 1];
  s_rfidRawData->u8_len = msg_len - 3; // Exclude status, checksum and ETX

  // Copy the actual data bytes (between <Status> and <CHCK>)
  for (i = 0; i < RFID_RAW_MAX_LEN; i++)
  {
    if (i < s_rfidRawData->u8_len)
    {
      s_rfidRawData->au8_data[i] = au8_rfidRxBuffer[i + 1];
    }
    else
    {
      s_rfidRawData->au8_data[i] = 0xFF; // Fill with 0xFF if no data available
    }
  } 
  
  if (s_rfidRawData->u8_status == RFID_CMD_OK)
  {
    // If the status is OK, return RFID_OK
    return RFID_OK;
  }
  else if (s_rfidRawData->u8_status == RFID_PWR_ON_NOTIFICATION)
  {
    // If the status is power on notification, return RFID_POWER_ON_NOTIFICATION
    return RFID_POWER_ON_NOTIFICATION;
  }
  else if (s_rfidRawData->u8_status == RFID_CMD_SYNTAX_ERROR)
  {
    // If the status is syntax error, return RFID_SYNTAX_ERROR
    return RFID_CMD_SYNTAX_ERROR;
  }
  else if (s_rfidRawData->u8_status == RFID_NO_TAG)
  {
    // If the status is no tag, return RFID_NO_TAG
    return RFID_NO_TAG;
  }
  else if (s_rfidRawData->u8_status == RFID_HW_ERROR)
  {
    // If the status is hardware error, return RFID_HW_ERROR
    return RFID_HW_ERROR;
  }
  else
  {
    // If the status is not OK or power on notification, return RFID_SYNTAX_ERROR
    return RFID_SYNTAX_ERROR;
  }
}

/**************************************************************************************************
**
**  Function:
**    UINT8 RFID_VerifySingleReadWord(const UINT8 *buffer, t_RFID_RAW_DATA *s_rfidRawData)
**
**  Description:
**    This function verifies the response of the command  Single Read Word, 
**    and stores the data in the raw data structure to prevent overwriting of the DMA buffer.
**    It checks the checksum and the length of the message.
**
**  See also:
**    -
**  Parameters:
**    buffer (IN) - Pointer to the buffer containing the response from the RFID reader
**    s_rfidRawData (OUT) - Pointer to the raw data structure where the parsed data will be stored
**
**  Return value:
**    RFID_OK (0) - Response is OK
**    RFID_SYNTAX_ERROR (4) - Syntax error
**    RFID_CHCK_ERROR (3) - Checksum error
*****************************************************************************************************/
STATIC UINT8 RFID_VerifySingleReadWord(const UINT8 *buffer, t_RFID_RAW_DATA *s_rfidRawData)
{
  UINT8 au8_rfidRxBuffer[75];
  // Copy buffer to temporary working buffer to prevent overwriting of DMA buffer
  UINT8 i;
  for (i = 0; i < RFID_EXPEC_RES_SR_LEN; i++)
  {
    au8_rfidRxBuffer[i] = buffer[i];
  }
  
  // Find the length of the received message
  UINT8 msg_len = RFID_ResLength(au8_rfidRxBuffer, RFID_EXPEC_RES_SR_LEN);
  if (msg_len < 3)
  {
    return RFID_SYNTAX_ERROR;
  }
  
  //Check the checksum
  if (RFID_VerifyChecksum(au8_rfidRxBuffer, msg_len) != RFID_OK)
  {
    return RFID_CHCK_ERROR;
  }
  
  // //Fill raw data structure

  s_rfidRawData->u8_status = au8_rfidRxBuffer[0];
  s_rfidRawData->u8_CHCK = au8_rfidRxBuffer[msg_len - 2];
  s_rfidRawData->u8_ETX = au8_rfidRxBuffer[msg_len - 1];
  s_rfidRawData->u8_len = msg_len - 3; // Exclude status, checksum and ETX
  
  // Copy the actual data bytes (between <Status> and <CHCK>)
  for (i = 0; i < RFID_RAW_MAX_LEN; i++)
  {
    if (i < s_rfidRawData->u8_len)
    {
      s_rfidRawData->au8_data[i] = au8_rfidRxBuffer[i+1];
    }
    else
    {
      s_rfidRawData->au8_data[i] = 0xFF; // Fill with 0xFF if no data available
    }
  }

  //Clear working buffer
  // for (; i < RFID_MAX_LEN; i++)
  // {
  //   au8_rfidRxBuffer[i] = 0xFF; // Fill with 0xFF
  // }
  return RFID_OK;
}

/**************************************************************************************************
**
**  Function:
**    UINT8 RFID_VerifyChecksum(const UINT8 *buffer, UINT8 length)
**
**  Description:
**    This function verifies the checksum of the received data. It compares the received checksum
**    with the calculated checksum. The checksum is calculated from the data bytes, excluding the
**    <CHCK> and <ETX> bytes.
**
**  See also:
**    -
**  Parameters:
**    buffer (IN) - Pointer to the buffer containing the received data
**    length (IN) - Length of the received data
**
**  Return value:
**    RFID_OK (0) - Checksum is valid
**    RFID_CHCK_ERROR (3) - Checksum is invalid
**************************************************************************************************/
STATIC UINT8 RFID_VerifyChecksum(const UINT8 *buffer, UINT8 length)
{
  UINT8 receivedChecksum = buffer[length - 2];
  UINT8 calculatedChecksum = RFID_CalculateChecksum(buffer, length - 2);

  return (receivedChecksum == calculatedChecksum) ? RFID_OK : RFID_CHCK_ERROR;
}

 /**************************************************************************************************
 **
 **  Function:
  **    UINT8 RFID_CalculateChecksum(const UINT8 * data, size_t length)
 **
 **  Description:
 **    This function calculates the checksum of the given data. The checksum is calculated from 
 **    <Status> and <Date> bytes, excluding <CHCK> and <ETX>.
 **
 **  See also:
 **    -
 **  Parameters:
 **    data (IN)   - Pointer to the data
 **    length (IN) - Length of the data
 **
 **  Return value:
 **    checksum (OUT) - Checksum of the data
 **************************************************************************************************/
STATIC UINT8 RFID_CalculateChecksum(const UINT8 * data, size_t length)
{
  UINT32 checksum = 0;
  UINT8 i;
  for (i = 0; i < length; i++)
  {
    checksum += data[i];
  }
  return (UINT8)(checksum % 256);
}

/* **************************************************************************************************
**
**  Function:
**    UINT8 RFID_ParseSingleFixCode(t_RFID_RAW_DATA *s_rfidRawData, t_RFID_TAG_DATA *s_rfidTagData)
**
**  Description:
**    This function parses the Single Read Fix Code message, from the rfid raw data structure.
**    It extracts the tag UID.
**
**  See also:
**    -
**  Parameters:
**    s_rfidRawData (IN) - Pointer to the raw data structure containing the received data
**    s_rfidTagData (OUT) - Pointer to the tag data structure where the parsed data will be stored
**
**  Return value:
**    RFID_OK (0) - Response is OK
**    RFID_SYNTAX_ERROR (4) - Syntax error
**
**************************************************************************************************/
STATIC UINT8 RFID_ParseSingleFixCode(t_RFID_RAW_DATA *s_rfidRawData, t_RFID_TAG_DATA *s_rfidTagData)
{
  // Verify the length of the record
  if (s_rfidRawData->u8_len != RFID_UID_LEN)
  {
    return RFID_SYNTAX_ERROR;
  }

  // Extract the tag UID
  
  UINT8 i;
  for (i = 0; i < RFID_UID_LEN; i++)
  {
    s_rfidTagData->au8_tag_uid[i] = s_rfidRawData->au8_data[i];
  }
  
  // Clear raw data structure
  s_rfidRawData->u8_status = 0xFF;
  s_rfidRawData->u8_CHCK = 0xFF;
  s_rfidRawData->u8_ETX = 0xFF; 
  s_rfidRawData->u8_len = 0xFF;
  
  for (i = 0; i < RFID_RAW_MAX_LEN; i++)
  {
    s_rfidRawData->au8_data[i] = 0xFF; // Fill with 0xFF
  }
  return RFID_OK;
}

/**************************************************************************************************
**
**  Function:
**    UINT8 RFID_CalculateCRC(t_RFID_TAG_DATA *s_rfidTagData)
**
**  Description:
**    This function prepares the RFID input data and gives it to the CRC calculation function.
**
**  See also:
**    -
**  Parameters:
**    s_rfidTagData (IN) - Pointer to the tag data structure containing the tag UID and S2L ID
**
**  Return value:
**    The calculated CRC value
**
**************************************************************************************************/
STATIC UINT32 RFID_CalculateCRC(t_RFID_TAG_DATA *s_rfidTagData)
{       
  UINT32 u32_receivedCRC = 0x00000000;
  u32_receivedCRC |= (UINT32)s_rfidTagData->au8_r_crc[0] << 24;
  u32_receivedCRC |= (UINT32)s_rfidTagData->au8_r_crc[1] << 16;
  u32_receivedCRC |= (UINT32)s_rfidTagData->au8_r_crc[2] << 8;
  u32_receivedCRC |= (UINT32)s_rfidTagData->au8_r_crc[3];

  // Prepare the input data for CRC calculation
  UINT8 au8_reserved[RFID_RES_LEN] = {0x00, 0x00, 0x00}; // Reserved bytes
  /* UINT32 sys_auth_key = todo later as safe configuration parameter */
  UINT8 au8_crc_input[RFID_UID_LEN + RFID_SEQ_NUM_LEN + RFID_RES_LEN + RFID_S2L_ID_LEN];
  UINT8 offset = 0;
  UINT8 i;

  /* 1. UID */
  for (i = 0; i < RFID_UID_LEN; i++)
  {
    au8_crc_input[offset++] = s_rfidTagData->au8_tag_uid[i];
    //au8_crc_input[offset++] = s_rfidTagData->au8_tag_uid[RFID_UID_LEN - 1 - i]; // Reverse order
  }

  /* 2. Sequence number */
  au8_crc_input[offset++] = s_rfidTagData->u8_seq_num;

  /* 3. Reserved bytes (3 bytes 0x00) */
  for (i = 0; i < RFID_RES_LEN; i++)
  {
    au8_crc_input[offset++] = au8_reserved[i];
  }

  /* 4. S2L ID (4 bytes) */
  for (i = 0; i < RFID_S2L_ID_LEN; i++)
  {
    au8_crc_input[offset++] = s_rfidTagData->au8_s2l_id[i];
  }

  /* Calculate CRC and compare with received CRC */
  if (CRC32(au8_crc_input, offset) == u32_receivedCRC)
  {
   /* CRC is valid, proceed with the tag data */
   return RFID_OK;
  }
  else
  {
    /* CRC error, handle accordingly */
    return RFID_REC_CRC_ERROR;
  }
}

/****************************************************************************************************
**
**  Function:
**    UINT8 RFID_CheckTagRecordFields(void)
**
**  Description:
**    This function checks the plausibility of of all fields of the read records from 
**    the RFID tag. It checks the sequence number, reserved bytes and S2L ID. And compares it 
**    with the the received data from the second safety controller.
**
**  See also:
**    -
**  Parameters:
**    -
**  Return value:
**    RFID_REC_OK (0) - All fields are valid
**    RFID_REC_INVALID_LEN (1) - Record length is invalid
**    RFID_REC_INVALID_S2L_ID (4) - S2L ID is invalid
**    RFID_REC_INVALID_SEQ_NUM (5) - Sequence number is invalid
**    RFID_REC_INVALID_UID (6) - UID is invalid
*****************************************************************************************************
*/
STATIC UINT8 RFID_CheckTagRecordFields(void)
{
  u32_RfidInfo = 0uL;
  UINT8 i;
  for (i = 0; i < 4; i++)
  {
    /* switch the order of the bytes */
    u32_RfidInfo |= ((UINT32)s_rfidTagRecordEven.au8_s2l_id[i]) << (8 * i);
  }
  /* Search for new tag again */
  return RFID_OK;
}

/**************************************************************************************************
**
**  Function:
**    UINT8 RFID_ParseRecord(void)
**
**  Description:
**    This function parses the tag record, from the rfid raw data structure.
**    It extracts the Sequence number, S2L ID and CRC from the record. #
**    The record is expected to be separated in bytes, in the format:
**     <SeqNum> <Res0> <Res1> <Res2> <S2L ID> <R_CRC>
**  
**  See also:
**    -
**
**  Parameters:
**    -
**
**  Return value:
**    -
**************************************************************************************************/
STATIC UINT8 RFID_ParseRecord(t_RFID_RAW_DATA *s_rfidRawData, t_RFID_TAG_DATA *s_rfidTagData)
{

  // Verify the length of the record
  if (s_rfidRawData->u8_len != RFID_REC_TOTAL_LEN)
  {
    return RFID_REC_INVALID_LEN;
  }

   // Verify the reserved bytes
  if (s_rfidRawData->au8_data[RFID_REC_IDX_RES0] != 0x00 ||
      s_rfidRawData->au8_data[RFID_REC_IDX_RES1] != 0x00 ||
      s_rfidRawData->au8_data[RFID_REC_IDX_RES2] != 0x00)
  {
    return RFID_REC_INVALID_RES;
  }

  // Extract the data
  s_rfidTagData->u8_seq_num = s_rfidRawData->au8_data[RFID_REC_IDX_SEQ];

  UINT8 i;
  for (i = 0; i < 4; i++)
  { 
    // Copy byte 4 to 7
    s_rfidTagData->au8_s2l_id[i] = s_rfidRawData->au8_data[RFID_REC_IDX_S2L_ID + i];
    //Copy byte 8 to 11
    s_rfidTagData->au8_r_crc[i]  = s_rfidRawData->au8_data[RFID_REC_IDX_R_CRC + i];
  }
  return RFID_REC_OK;
}


 

STATIC size_t RFID_ResLength(const UINT8 *buffer, size_t maxLength)
{
  for (size_t i = 0; i < maxLength; i++)
  {
    if (buffer[i] == 0x03) // ETX
    {
      return i + 1; // Include ETX in the length
    } 
  }
  return 0; // No ETX found
}

void delay_ms(uint32_t ms)
{
  volatile uint32_t count;
  while (ms--)
  {
    count = 9000;  // für ~1 ms bei 36 MHz
    while (count--) __NOP();
  }
}





 /**************************************************************************************************
 **
 **  Function:
 **    void RFID_BootReader(void)
 **
 **  Description:
 **    This function sends the the Reader Version command after the device is booted. To verify
 **    the communication between the RFID reader and the controller.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    -
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void RFID_BootReader(void)
 {
    /* Command to read software version of the RFID reader */
   au8_rfidDmaBufferTx[0] = 0x56;
   au8_rfidDmaBufferTx[1] = 0x45;
   au8_rfidDmaBufferTx[2] = 0x9B; /* CHECKSUM */
   au8_rfidDmaBufferTx[3] = 0x03; /* ETX */
   
    /* Send the frame */
    RFID_FrameTxTrigger(RFID_CMD_VE_LEN);
 }



 /**************************************************************************************************
 **
 **  Function:
 **    void RFID_FrameTxSingleReadFixCode(void)
 **
 **  Description:
 **    This function sends the Single Read Fix Code command to the RFID reader.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    -
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void RFID_FrameTxSingleReadFixCode(void)
 {
   /* Single read fix code command */
   au8_rfidDmaBufferTx[0] = 0x53;
   au8_rfidDmaBufferTx[1] = 0x46;
   au8_rfidDmaBufferTx[2] = 0x99; /* CHECKSUM */
   au8_rfidDmaBufferTx[3] = 0x03; /* ETX */

   /* Send the frame */
   RFID_FrameTxTrigger(RFID_CMD_SF_LEN);
 }

/**************************************************************************************************
**
** Function:
**   void RFID_FrameTxReadRecord_0(void)
**
** Description:
**   This function sends the Single Read Words command to read the first record from the 
**   preprogrammed RFID tag memory. The command is used to read 3 words with a start address of 
**   record 0 (0x000000 (ASCII 0x30)).
** 
** See also:
**   -
** Parameters:
**   -
** Return value:
**   -
**************************************************************************************************/
 STATIC void RFID_FrameTxReadRecord_0(void)
 {
   /* Currenly one records -> 3 words*/
   /* Single Read Words command */
   au8_rfidDmaBufferTx[0] = 0x53;
   au8_rfidDmaBufferTx[1] = 0x52;
   /* Word address */
   au8_rfidDmaBufferTx[2] = 0x30; 
   au8_rfidDmaBufferTx[3] = 0x30; 
   au8_rfidDmaBufferTx[4] = 0x30; 
   au8_rfidDmaBufferTx[5] = 0x30; 
   /* Number of words */
   au8_rfidDmaBufferTx[6] = 0x30;
   au8_rfidDmaBufferTx[7] = 0x33;
   au8_rfidDmaBufferTx[8] = 0xC8; /* CHECKSUM */
   au8_rfidDmaBufferTx[9] = 0x03; /* ETX */

   /* Send the frame */
   RFID_FrameTxTrigger(RFID_CMD_SR_LEN);
 }

/**************************************************************************************************
**
** Function:
**   void RFID_FrameTxReadRecord_1(void)
**
** Description:
**   This function sends the Single Read Words command to read the second record from the
**   preprogrammed RFID tag memory. The command is used to read 3 words with a start address of
**   record 1 (0x000003 (ASCII 0x33)).
**
** See also:
**   -
** Parameters:
**   -
** Return value:
**   -
**************************************************************************************************/
 STATIC void RFID_FrameTxReadRecord_1(void)
 {
   /* Currenly one records -> 3 words*/
   /* Single Read Words command */
   au8_rfidDmaBufferTx[0] = 0x53;
   au8_rfidDmaBufferTx[1] = 0x52;
   /* Word address */
   au8_rfidDmaBufferTx[2] = 0x30; 
   au8_rfidDmaBufferTx[3] = 0x30; 
   au8_rfidDmaBufferTx[4] = 0x30; 
   au8_rfidDmaBufferTx[5] = 0x33; 
   /* Number of words */
   au8_rfidDmaBufferTx[6] = 0x30;
   au8_rfidDmaBufferTx[7] = 0x33;
   /* CHECKSUM */
   au8_rfidDmaBufferTx[8] = 0xCB; 
   /* ETX */
   au8_rfidDmaBufferTx[9] = 0x03;

   /* Send the frame */
   RFID_FrameTxTrigger(RFID_CMD_SR_LEN);
 }
 /**************************************************************************************************
 **
 **  Function:
 **    void RFID_FrameTxTrigger(UINT8 u8_len)
 **
 **  Description:
 **    This function sends the RFID frame to the RFID reader.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    u8_len (IN) - Length of the frame
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void RFID_FrameTxTrigger(UINT8 u8_len)
 {
   /* Disable DMA channel (needed during settings) */
   RFID_DMA_CHANNEL_TX->CCR &= ~DMA_CCR1_EN;

   /* Write DMA-size in CNTR-Register */
   RFID_DMA_CHANNEL_TX->CNDTR = u8_len;

   /* Enable DMA channel 4 */
   RFID_DMA_CHANNEL_TX->CCR |= DMA_CCR1_EN; /* TX */
 }

 /**************************************************************************************************
 **
 **  Function:
 **    void RFID_FrameRxInit(UINT8 u8_len)
 **
 **  Description:
 **    This function gets the length of the expected response frame and initializes the DMA buffer.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    u8_len (IN) - Length of the frame
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void RFID_FrameRxInit(UINT8 u8_len)
 {
   UINT8 i;

   for (i = 0; i < 100; i++)
   {
     au8_rfidDmaBufferRx[i] = 0x00u;
   }

   /* Disable DMA1 channel 5, to write data into the CNDTR and CMAR register 
   * NOTE: If it is enabled, the registers are write protected */
   RFID_DMA_CHANNEL_RX->CCR &= ~DMA_CCR5_EN; /* RX */

   /* Write DMA-size in CNTR-Register */
   RFID_DMA_CHANNEL_RX->CNDTR = u8_len;

   /* Enable DMA and UART */
   RFID_DMA_CHANNEL_RX->CCR |= DMA_CCR5_EN; /* RX */
 }


 /**************************************************************************************************
 **
 **  Function:
 **   void RFID_HandleFailure(t_RFID_FAILURE e_failure)
 **
 **  Description:
 **   This function handles the RFID reader failures. It increments the failure count and sets the
 **   last failure type. Depending on the failure type, it resets the access state to retry reading
 **   the UID and records.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **   e_failure (IN) - The type of failure that occurred
 **
 **  Return value:
 **    -
 **************************************************************************************************/
STATIC void RFID_HandleFailure(t_RFID_FAILURE e_failure)
{
  u32_RfidInfo = 0uL; // Reset RFID info
  u8_rfidFailureCount++;
  e_rfidLastFailure = e_failure;

  switch (e_failure)
  {
    case RFID_FAIL_UID_TIMEOUT:
    case RFID_FAIL_UID_VERIFY:
    case RFID_FAIL_EVEN_REC_TIMEOUT:
    case RFID_FAIL_EVEN_REC_VERIFY:
    case RFID_FAIL_EVEN_REC_CRC_ERROR:
    case RFID_FAIL_ODD_REC_TIMEOUT:
    case RFID_FAIL_ODD_REC_VERIFY:
    case RFID_FAIL_ODD_REC_CRC_ERROR:
      /* Reset the access state to retry reading the UID */
      e_rfidAccessState = TX_READ_UID;
      break;
    
    case RFID_FAIL_BOOT_READER:
    case RFID_FAIL_BOOT_READER_TIMEOUT:
    case RFID_FAIL_UNKNOWN:
  default:
    /* Global failure */
    e_rfidAccessState = STATE_FAILURE;
    break;
  }
}

 /**************************************************************************************************
 **
 **  Function:
 **    void uartInit(void)
 **  Description:
 **    This function initializes the UART for the RFID reader.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    -
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void uartInit(void)
 {
  /* RCC register: enable the USARTx clock */
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;  /* enable the USART3 clock */
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;    /* I/O port B clock enable */

  /* configure the GPIO Pin PB10 for USART Tx */
  gpioHAL_ConfigureOutput( GPIOB,
                           GPIO_10,
                           OUT_CONFIG_AF_PP,
                           OUT_MODE_50MHZ );

  /* configure the GPIO Pin PB11 for USART Rx */
  gpioHAL_ConfigureInput( GPIOB,
                          GPIO_11,
                          IN_CONFIG_FLOATING );

  /* calculate and configure baudrate, differ between USART3 and the other
  * USARTs because USART3 is clocked with max. 72MHz and the others with
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
  */
  USART3->BRR = (UINT16) ((UINT32) RFID_SYSCLOCK / (UINT32)((UINT32) RFID_38400_BAUDRATE * 2u));
  // TODO check it later sUsart->BRR = (UINT16) ((UINT32) RFID_SYSCLOCK / (UINT32) ((UINT32) RFID_115200_BAUD * 2u));

  /* activate transmit and receive */
  USART3->CR1 = (USART_CR1_RE | USART_CR1_TE);

  /* disable parity control */
  USART3->CR1 &= ~USART_CR1_PCE;
  /* set control register 2 to the default value */
  USART3->CR2 = 0u;
  /* set control register 3 to the default value */
  USART3->CR3 = 0u;
  
  /* open UART */
  USART3->CR1 |= USART_CR1_UE;
 }

 /**************************************************************************************************
 **
 **  Function:
 **    void uartInitDmaTx(void)
 **
 **  Description:
 **    This function initializes the DMA for the RFID reader.
 **
 **  See also:
 **    -
 ** 
 **  Parameters:
 **    -
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void uartInitDmaTx(void)
 {
   /* enable DMA1 clock */
   RCC->AHBENR |= RCC_AHBENR_DMA1EN;

   /* Control register 3 (USART_CR3) */
   USART3->CR3 |= USART_CR3_DMAT; /* DMA enable transmitter for USART 1 */

   /* settings for TX DMA: */
   RFID_DMA_CHANNEL_TX->CCR = DMA_CCR4_MINC /* Memory increment mode, */
                                                  /* 1: Memory increment mode enabled  */
                                  | DMA_CCR4_PL /* Channel priority level, 11: Very high */
                                  | DMA_CCR4_DIR; /* Data transfer direction, 1: Read from memory */

   /* write the the USART3_DR address into the
    * DMA channel peripheral address register */
   RFID_DMA_CHANNEL_TX->CPAR = (UINT32) &USART3->DR;/*lint !e923*/
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

   /* write address of TxBuffer in CMAR-Register */
   RFID_DMA_CHANNEL_TX->CMAR  = (UINT32)au8_rfidDmaBufferTx;
 }

 /**************************************************************************************************
 **
 **  Function:
 **    void uartInitDmaRx(void)
 **
 **  Description:
 **    This function initializes the DMA for the RFID reader.
 **
 **  See also:
 **    -
 **
 **  Parameters:
 **    -
 **
 **  Return value:
 **    -
 **************************************************************************************************/
 STATIC void uartInitDmaRx(void)
 {
   /* enable DMA1 clock */
   RCC->AHBENR |= RCC_AHBENR_DMA1EN;

   /* enable DMA transfer for USART3 */
   USART3->CR3 |= USART_CR3_DMAR; /* DMA enable receiver */

   /* settings for RX DMA:
    * Memory increment mode : 1: Memory increment mode enabled
    * Channel priority level: 11: Very high */
   RFID_DMA_CHANNEL_RX->CCR = (DMA_CCR5_MINC | DMA_CCR5_PL);

   /* write the the USART3_DR address into the DMA channel peripheral address register */
   RFID_DMA_CHANNEL_RX->CPAR = (UINT32) &USART3->DR;/*lint !e923*/
   /* Lint message disabled because the register address is written to the
    * DMA register as integer value. */

   /* write address of RxBuffer in CMAR-Register */
   RFID_DMA_CHANNEL_RX->CMAR = (UINT32)au8_rfidDmaBufferRx;

   /* enables the USART3 DMA receive */
   /* TODO serialHAL_ReceiveUSART3Dma(u32DataLength); */
 }

/**************************************************************************************************
** 
**  Function:
**    UINT32 CRC32(const UINT8* data, UINT8 length)
**
**  Description:
**    This function calculates the CRC32 checksum for the given data.
**
**  See also:
**    -
**  Parameters:
**    data (IN)   - Pointer to the data
**    length (IN) - Length of the data
**
**  Return value:
**    -
*****************************************************************************************************/
STATIC UINT32 CRC32(const UINT8 * data, UINT8 length)
{
 UINT32 crc = CRC_SEED; 

 UINT8 i;
 UINT8 j;
 for (i = 0; i< length; i++)
 {
    crc ^= (UINT32)data[i] << 24;
    for (j = 0; j <  8; j++) 
    {
      if (crc & 0x80000000)
      {
        crc = (crc << 1) ^ CRC_POLY;
      }
      else
      {
        crc <<=1;
      }
    }
  }
 crc ^= CRC_XOROUT;
 return crc;
}

/**************************************************************************************************
**
**  Function:
**    void timerStart(t_TIME *ps_timer)
**
**  Description:
**    This function starts the timer.
**
**  See also:
**    -
**
**  Parameters:
**    ps_timer (IN) - Pointer to the timer structure
**
**  Return value:
**    -
**************************************************************************************************/
STATIC void timerStart(t_TIME *ps_timer)
{
  ps_timer->u32_cur = timerHAL_GetSystemTime3();
}

/**************************************************************************************************
**
**  Function:
**    void timerStop(t_TIME *ps_timer)
**
**  Description:
**    This function stops the timer.
**
**  See also:
**    -
**
**  Parameters:
**    ps_timer (IN) - Pointer to the timer structure
**
**  Return value:
**    -
**************************************************************************************************/
STATIC void timerStop(t_TIME *ps_timer)
{
  UINT32 u32_curTime = (UINT32)(timerHAL_GetSystemTime3() - ps_timer->u32_cur);

  if (u32_curTime > ps_timer->u32_max)
  {
    ps_timer->u32_max = u32_curTime;
  }
}

#endif /* #ifdef RFID_ACTIVE */

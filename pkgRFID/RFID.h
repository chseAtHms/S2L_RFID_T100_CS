/**************************************************************************************************    
**    Copyright (C) 2016 HMS Technology Center Ravensburg GmbH, all rights reserved    
***************************************************************************************************    
**    
**        File: RFID.h    
**     Summary: External interface for the RFID reader functionality    
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
#ifndef RFID_H    
#define RFID_H    
#ifdef RFID_ACTIVE    
/**************************************************************************************************    
**    constants and macros    
**************************************************************************************************/    
/* Baudrate constants for the RFID reader */   
#define RFID_38400_BAUDRATE         38400u  
#define RFID_115200_BAUD            115200u
/* RFID Protocol Constants */

/* RFID Tag Status Codes
  * 
  * These codes are used to indicate the status of the RFID tag operations.
  * They are used in the communication between the RFID reader and the tag.
  * (see RFID Reader Manual tdoct3728d_ger.pdf chapter 6.7)
  */
#define RFID_CMD_OK                 0x30u
#define RFID_PWR_ON_NOTIFICATION    2u
#define RFID_CMD_SYNTAX_ERROR       4u
#define RFID_NO_TAG                 0x35u
#define RFID_HW_ERROR               6u

#define RFID_ETX                    0x03



#define RFID_RAW_MAX_LEN            70u
#define RFID_UID_LEN                8u
#define RFID_SEQ_NUM_LEN            1u
#define RFID_RES_LEN                3u
#define RFID_S2L_ID_LEN             4u
#define RFID_RECORD_CRC_LEN         4u


/* RFID Record Structure 
 * (see SAC_003, SAC_004, SAC_005 SAC_006)
 */
#define RFID_REC_TOTAL_LEN          12u
#define RFID_REC_IDX_SEQ            0u
#define RFID_REC_IDX_RES0           1u
#define RFID_REC_IDX_RES1           2u
#define RFID_REC_IDX_RES2           3u
#define RFID_REC_IDX_S2L_ID         4u
#define RFID_REC_IDX_R_CRC          8u

/*  Error codes for RFID operations on file and function level */
#define RFID_OK                     0u
#define RFID_POWER_ON_NOTIFICATION  2
#define RFID_TIMEOUT_ERROR          1
#define RFID_SYNTAX_ERROR           4 
#define RFID_CHCK_ERROR             3
#define RFID_TOO_SHORT              6 
#define RFID_UNKNOWN_ERROR          7 

/* RFID Record error codes */
#define RFID_REC_OK                 0u
#define RFID_REC_INVALID_LEN        1u
#define RFID_REC_INVALID_RES        2u
#define RFID_REC_SEQ_NUM_ERROR      4u
#define RFID_REC_CRC_ERROR          3u

/*  Failure count a maximum of 2 failures per hour is allowed 
 *  (see [SAC_23])
*/

#define RFID_MAX_FAILURE_COUNT      3u

/* Timeouts for RFID operations */
// t100 time slice is ca 100 us
#define T100_TIME_SLICE_US          100u // 100 us
#define RFID_BOOT_READER_TIMEOUT    30000u // 30 ms
#define RFID_READ_UID_TIMEOUT       30000u // 30 ms
#define RFID_READ_REC_TIMEOUT       50000u // 50 ms

/* Failure types for RFID operations for global error handling */
typedef enum {
  RFID_FAIL_NONE,
  RFID_FAIL_BOOT_READER,
  RFID_FAIL_BOOT_READER_TIMEOUT,
  RFID_FAIL_UID_TIMEOUT,
  RFID_FAIL_UID_VERIFY,
  RFID_FAIL_INVALID_SEQ_NUM,
  RFID_FAIL_EVEN_REC_TIMEOUT,
  RFID_FAIL_EVEN_REC_VERIFY,
  RFID_FAIL_EVEN_REC_SEQ_NUM_ERROR,
  RFID_FAIL_EVEN_REC_CRC_ERROR,
  RFID_FAIL_ODD_REC_TIMEOUT,
  RFID_FAIL_ODD_REC_VERIFY,
  RFID_FAIL_ODD_REC_SEQ_NUM_ERROR,
  RFID_FAIL_ODD_REC_CRC_ERROR,
  RFID_FAIL_S2L_ID_VERIFY,
  RFID_FAIL_UNKNOWN,
} t_RFID_FAILURE;

typedef enum {
  WF_TX_BOOT_FIRMWARE,
  WF_RX_BOOT_FIRMWARE,
  WF_RX_SINGLE_READ_FIX_CODE,
  WF_RX_READ_RECORD_0,
  WF_RX_READ_RECORD_1,
  WF_RX_READ_RECORD_2,
  WF_RX_READ_RECORD_3,
  WF_RX_READ_RECORD_4,
  WF_RX_READ_RECORD_5,
  WF_RX_READ_RECORD_6,
  WF_RX_READ_RECORD_7,
  RFID_FAILURE,
} t_RFID_ACCESS_STATE;

/* RFID Tag Read States
 * 
 * These states are used to manage the reading process of the RFID tag.
 * The states are used in a state machine to control the flow of reading
 * the UID and records from the RFID tag.
 */
typedef enum {
  TX_BOOT_FIRMWARE,
  RX_BOOT_FIRMWARE,
  TX_READ_UID,
  RX_READ_UID,
  TX_READ_REC_EVEN,
  RX_READ_REC_EVEN,
  CHECK_REC_EVEN,
  TX_READ_REC_ODD,
  RX_READ_REC_ODD,
  CHECK_REC_ODD,
  CHECK_TAG_RECORDS,
  STATE_SUCCESS,
  STATE_FAILURE,
} t_RFID_TAG_READ_STATE;


/**************************************************************************************************    
**   data types     
**************************************************************************************************/  
typedef struct {
  UINT8 u8_status;                        /* Status byte */
  UINT8 u8_CHCK;                          /* Checksum byte */
  UINT8 u8_ETX;                           /* End of text byte */
  UINT8 u8_len;                           /* Actual length of the data */
  UINT8 au8_data[RFID_RAW_MAX_LEN];       /* Data buffer large enough for all data */
} t_RFID_RAW_DATA;

typedef struct {
  UINT8 au8_tag_uid[RFID_UID_LEN];        /* Tag UID */
  UINT8 u8_seq_num;                       /* Sequence number */
  UINT8 au8_s2l_id[RFID_S2L_ID_LEN];      /* S2L ID */
  UINT8 au8_r_crc[RFID_RECORD_CRC_LEN];   /* Record CRC */
} t_RFID_TAG_DATA;




/**************************************************************************************************    
**   global variables    
**************************************************************************************************/    
/**************************************************************************************************    
**   function prototypes    
**************************************************************************************************/    
void RFID_Init(void);    
void RFID_Reader_Boot(void);
void RFID_ReadTag(void);
UINT32 RFID_InfoGet(void);
void RFID_InfoSet(UINT32 u32_info);
#endif /* #ifdef RFID_ACTIVE */    
#endif /* #ifndef RFID_H */    

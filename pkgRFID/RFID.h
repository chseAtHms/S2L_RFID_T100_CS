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
/* Baudrate */    
#define RFID_38400_BAUDRATE 38400u  
#define RFID_115200_BAUD 115200u

#define RFID_ETX 0x03

#define RFID_RAW_MAX_LEN    70u
#define RFID_UID_LEN        8u
#define RFID_SEQ_NUM_LEN    1u
#define RFID_RES_LEN        3u
#define RFID_S2L_ID_LEN     4u
#define RFID_RECORD_CRC_LEN 4u
  
// RFID Record structure
#define RFID_REC_TOTAL_LEN  12u
#define RFID_REC_IDX_SEQ    0u
#define RFID_REC_IDX_RES0   1u
#define RFID_REC_IDX_RES1   2u
#define RFID_REC_IDX_RES2   3u
#define RFID_REC_IDX_S2L_ID 4u
#define RFID_REC_IDX_R_CRC    8u

// RFID Tag Status Codes
#define RFID_CMD_OK               0x30u
#define RFID_PWR_ON_NOTIFICATION  2u
#define RFID_CMD_SYNTAX_ERROR     4u
#define RFID_NO_TAG               0x35u
#define RFID_HW_ERROR             6u

// RFID Record error codes
#define RFID_REC_OK           0u
#define RFID_REC_INVALID_LEN  1u
#define RFID_REC_INVALID_RES  2u
#define RFID_REC_SEQ_NUM_ERROR  4u
#define RFID_REC_CRC_ERROR     3u


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
  STATE_SUCCESS,
  STATE_FAILURE,
  STATE_FAILURE_UID,
  STATE_FAILURE_REC,
  STATE_FAILURE_REC_ODD,
  RFID_EVEN_REC_CRC_ERROR,
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
#endif /* #ifdef RFID_ACTIVE */    
#endif /* #ifndef RFID_H */    

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** aicMsgDef.h
**
** $Id: aicMsgDef.h 3972 2023-03-24 14:06:49Z ankr $
** $Revision: 3972 $
** $Date: 2023-03-24 15:06:49 +0100 (Fr, 24 Mrz 2023) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** contains the definitions for the AIC telegram structure
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

#ifndef AIC_MSGDEF_H
#define AIC_MSGDEF_H


/*******************************************************************************
**
** Constants (#define)
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  size of input data (from the ASM to the network/AM) unit [Bytes]
**------------------------------------------------------------------------------
*/
#ifdef RFID_ACTIVE
#define AICMSGDEF_DATA_IN_LEN         ((UINT8)3U + 4u)
#else
#define AICMSGDEF_DATA_IN_LEN         ((UINT8)3U)
#endif


/*------------------------------------------------------------------------------
**  size of output data (from the network/AM to ASM) unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_DATA_OUT_LEN        ((UINT8)3U)

/*------------------------------------------------------------------------------
**  size Non-safe Message part of the telegram, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_MSG_DATA_SIZE   ((UINT8)12u)

/*------------------------------------------------------------------------------
**  size Safety protocol data unit from the network/AM to ASM, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_SPDU_OUT_SIZE    ((UINT8)(sizeof(AICMSGDEF_SPDU_RX_STRUCT)))

/*------------------------------------------------------------------------------
**  size Safety protocol data unit from the ASM to the network/AM, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_SPDU_IN_SIZE     ((UINT8)(sizeof(AICMSGDEF_SPDU_TX_STRUCT)))

/*------------------------------------------------------------------------------
**  size of CRC used for AIC telegram, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_CRC_SIZE         ((UINT8)2u)

/*------------------------------------------------------------------------------
**  size of RX AIC telegram coming from AM to ASM, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_AM_TGM_SIZE          ( sizeof(AICMSGDEF_AM_TELEGRAM_STRUCT) )

/*------------------------------------------------------------------------------
**  size of TX AIC telegram going from ASM to AM, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_TGM_SIZE         ( sizeof(AICMSGDEF_ASM_TELEGRAM_STRUCT) )

/*------------------------------------------------------------------------------
**  size of special startup telegram, unit [Bytes]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_ASM_STARTUP_TGM_SIZE ( sizeof(AICMSGDEF_ASM_STARTUP_TELEGRAM_STRUCT) )


/*------------------------------------------------------------------------------
**  size of CIP Safety Data Message transferred in Data field. 
**  Output Assembly Inst. 0x300 (3 bytes user data), [SRS_2062]
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_OUT_DATA_ASM300_SIZE      ((UINT8)14u)
/*------------------------------------------------------------------------------
**  size of Data field inside ABCC SPDU 
**  Additionally to the Data message, the consumer number and a padding byte is
**  included in the Data field. So the following layout is given:
**  - Byte 1: consumer number 
**  - Byte 2: padding byte
**  - Byte 3-16: Data message
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_OUT_DATA_SIZE    ((UINT8)AICMSGDEF_CIP_OUT_DATA_ASM300_SIZE + 2U)


/*------------------------------------------------------------------------------
**  size of CIP Safety Time Coordination Message 
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_OUT_TCOO_SIZE             ((UINT8)6u)
/*------------------------------------------------------------------------------
**  size of Data2 field inside ABCC SPDU 
**  Additionally to the Time Coordination Message, the consumer number and a 
**  padding byte is included in the Data2 field. So the following layout is given:
**  - Byte 1: consumer number 
**  - Byte 2: padding byte
**  - Byte 3-8: Time coordination message
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_OUT_DATA2_SIZE   ((UINT8)AICMSGDEF_CIP_OUT_TCOO_SIZE + 2U)





/*------------------------------------------------------------------------------
**  size of CIP Safety Data Message transferred in Data Field.
**  Input Assembly Inst. 0x264 (3 byte user data), [SRS_2056]
**------------------------------------------------------------------------------
*/
#ifdef RFID_ACTIVE
#define AICMSGDEF_CIP_IN_DATA_ASM264_SIZE      ((UINT8)14u + (4u * 2u))
#else
#define AICMSGDEF_CIP_IN_DATA_ASM264_SIZE      ((UINT8)14u)
#endif
/*------------------------------------------------------------------------------
**  size of Data field inside T100 SPDU 
**  The following layout is given:
**  - Byte 1-14: Data message
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_IN_DATA_SIZE     (UINT8)AICMSGDEF_CIP_IN_DATA_ASM264_SIZE


/*------------------------------------------------------------------------------
**  size of CIP Safety Time Coordination Message 
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_IN_TCOO_SIZE             ((UINT8)6u)
/*------------------------------------------------------------------------------
**  size of Data2 field inside T100 SPDU 
**  The following layout is given:
**  - Byte 1-6: Time coordination message
**------------------------------------------------------------------------------
*/
#define AICMSGDEF_CIP_IN_DATA2_SIZE    (UINT8)AICMSGDEF_CIP_IN_TCOO_SIZE




/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Header of NON-SAFE message, see [SIS_053]
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_MSG_HEADER_TAG
{
   /* To keep track of requests and their corresponding response. */
   UINT8  u8MsgId;
   /* Describes what is contained in the data part of the message. */
   UINT16 u16MsgReqResp;
   /* Amount of bytes in the data field */
   UINT8  u8MsgLength;
} AICMSGDEF_MSG_HEADER_STRUCT;

/*------------------------------------------------------------------------------
** non safe message/data of AM/ASM message, see [SIS_053]
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_NON_SAFE_MSG_TAG
{
   /* header of non-safe message part */
   AICMSGDEF_MSG_HEADER_STRUCT sAicMsgHeader;
   /* data array of non-safe message part */
   UINT8   au8AicMsgData[AICMSGDEF_ASM_MSG_DATA_SIZE];
} AICMSGDEF_NON_SAFE_MSG_STRUCT;

/*------------------------------------------------------------------------------
** IO output data received via safe packet (data part only),
** see [SRS_2051], [SRS_2062] (output means here: safe packet send from
** safe PLC to safety module)
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_PS_OUT_DATA_TAG
{
   /* DO nominal value, see [SRS_2014], [SRS_2052] */
   UINT8 u8DoVal;
   /* DI Error Reset flags, see [SRS_2053] */
   UINT8 u8DiErrRst;
   /* DO Error Reset flags, see [SRS_2053] */
   UINT8 u8DoErrRst;
} AICMSGDEF_PS_OUT_DATA_STRUCT;

/*------------------------------------------------------------------------------
** IO input data send via safe packet (data part only), see [SRS_2063], [SRS_2056],
** (input means here: safe packet send from safety module to safe PLC)
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_PS_INPUT_DI_DO_TAG
{
   /* DI data, see [SRS_2012], [SRS_2013], [SRS_2054] */
   UINT8 u8DiData;
   /* DI Qualifiers, see [SRS_2012], [SRS_2013], [SRS_2055] */
   UINT8 u8DiQualifiers;
   /* DO qualifiers, [SRS_2018], [SRS_2019], [SRS_2055] */
   UINT8 u8DoQualifiers;
#ifdef RFID_ACTIVE
   /* information about the read RFID tag */
   UINT32 u32_rfidInfo;
#endif
}  AICMSGDEF_PS_INPUT_DATA_STRUCT;


/*------------------------------------------------------------------------------
**  structure for CIP Safety input packet SPDU (data from target to originator),
**  used for assembly instance 0x264
**  see [SRS_2063], [SRS_2056], [SIS_209]
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_SPDU_TX_STRUCT_TAG
{
  /**************************************************************************/
  /* For IO safety data producer                                            */
  /**************************************************************************/
  /* Length of data (n) */
  UINT16 u16_halcLen;
  
  /* Safety validator instance ID for safety IO data (only 2 bytes because
  ** Safety validator instance ID is 2 bytes inside CSS)
  */
  UINT16 u16_halcAddInfo;
  
  /* Byte 1-n: Data message */
  UINT8 au8_data[AICMSGDEF_CIP_IN_DATA_SIZE];
  
  /**************************************************************************/
  /* For IO safety data consumer                                            */
  /**************************************************************************/
  /* Safety validator instance ID for time coordination message (only 2 bytes 
  ** because Safety validator instance ID is 2 bytes inside CSS)
  */
  UINT16 u16_halcAddInfo2;
  
  /* Byte 1-6: Time coordination message */
  UINT8 au8_data2[AICMSGDEF_CIP_IN_DATA2_SIZE];
  
  /**************************************************************************/
  /* Data Update Indicator for safety data consumer and safety data producer */
  /**************************************************************************/
  /* Data Update Indicator for data (incremented whenever the data field was 
  ** updated in contrast to previous SPDU) 
  */
  UINT8 u8_duiData;
  
  /* Data Update Indicator for data2 (incremented whenever the data2 field was 
  ** updated in contrast to previous SPDU) 
  */
  UINT8 u8_duiData2;
  
} AICMSGDEF_SPDU_TX_STRUCT;


/*------------------------------------------------------------------------------
**  structure for CIP Safety output packet SPDU (data from originator to target),
**  used for assembly instance 0x64
**  see [SRS_2051], [SIS_208]
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_SPDU_RX_STRUCT_TAG
{
  /**************************************************************************/
  /* For IO safety data consumer                                            */
  /**************************************************************************/
  /* Length of data (n) */
  UINT16 u16_halcLen;
  
  /* Safety validator instance ID for safety IO data (only 2 bytes because
  ** Safety validator instance ID is 2 bytes inside CSS)
  */
  UINT16 u16_halcAddInfo;
  
  /* - Byte 1: consumer number 
  ** - Byte 2: padding byte
  ** - Byte 3-n: Data message
  */
  UINT8 au8_data[AICMSGDEF_CIP_OUT_DATA_SIZE];
  
  /**************************************************************************/
  /* For IO safety data producer                                            */
  /**************************************************************************/
  /* Safety validator instance ID for time coordination message (only 2 bytes 
  ** because Safety validator instance ID is 2 bytes inside CSS)
  */
  UINT16 u16_halcAddInfo2;
  
  /*  - Byte 1: consumer number 
  **  - Byte 2: padding byte
  **  - Byte 3-8: Time coordination message
  */
  UINT8 au8_data2[AICMSGDEF_CIP_OUT_DATA2_SIZE];
  
  /**************************************************************************/
  /* Data Update Indicator for safety data consumer and safety data producer */
  /**************************************************************************/
  /* Data Update Indicator for data (incremented whenever the data field was 
  ** updated in contrast to previous SPDU) 
  */
  UINT8 u8_duiData;
  
  /* Data Update Indicator for data2 (incremented whenever the data2 field was 
  ** updated in contrast to previous SPDU) 
  */
  UINT8 u8_duiData2;
  
} AICMSGDEF_SPDU_RX_STRUCT;


/*------------------------------------------------------------------------------
**  Telegram send from ASM to AM (see [SRS_2020])
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_ASM_TELEGRAM_TAG
{
   /* control status of ASM message */
   UINT8                            u8CtrlStatus;
   /* 16 Byte of non-safe ASM message*/
   AICMSGDEF_NON_SAFE_MSG_STRUCT    sAsmMsg;
   /* 28 Byte SPDU TX packet */
   AICMSGDEF_SPDU_TX_STRUCT         sSpdu;
   /* Input/Output data send via safety protocol as non-safe information */
   AICMSGDEF_PS_INPUT_DATA_STRUCT   sIoInData;
   /* Input/Output data received via safety protocol as non-safe information */
   AICMSGDEF_PS_OUT_DATA_STRUCT     sIoOutData;
   /* 16-bit CRC field */
   UINT16                           u16Crc;
} AICMSGDEF_ASM_TELEGRAM_STRUCT;

/*------------------------------------------------------------------------------
**  Telegram send from AM to ASM (see [SRS_2021])
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_RX_DMA_BUFFER_TAG
{
   /* control status of ASM message */
   UINT8                         u8CtrlStatus;
   /* 16 Byte of non-safe AM message*/
   AICMSGDEF_NON_SAFE_MSG_STRUCT sAmMsg;
   /* 32 Byte PDU RX packet */
   AICMSGDEF_SPDU_RX_STRUCT      sSpdu;
   /* 16-bit CRC field */
   UINT16                        u16Crc;
} AICMSGDEF_AM_TELEGRAM_STRUCT;

/*------------------------------------------------------------------------------
**  special startup telegram see [SRS_2144])
**------------------------------------------------------------------------------
*/
typedef __packed struct AICMSGDEF_ASM_STARTUP_TELEGRAM_TAG
{
   /* ASM Vendor Id: Identifies vendor of the ASM module. 0x0001  HMS  */
   UINT16   u16AsmVendorId ;
   /* ASM Module Id:  Identifies the kind of ASM */
   UINT16   u16AsmModuleId;
   /* ASM Firmware version  ASM firmware version number.
    * The version 1.12.03 corresponds to 0x01, 0x0C, 0x03. */
   UINT8    au8AsmFwVers[3];
   /* ASM serial number  32-bit unique serial number. */
   UINT32   u32AsmSN;
   /* Safe output PDU size  Size (in bytes) for the safe output PDU data
    * transferred from the IO controller to the Anybus module.  */
   UINT8    u8SafeOutputPduSize;
   /* Output data size  Amount of data from I/O controller to ASM. */
   UINT8    u8OutputDataSize;
   /* Safe input PDU size  Size (in bytes) for the safe input PDU data
    * transferred from the Anybus module to the IO controller.  */
   UINT8    u8SafeInputPduSize;
   /* Input data size  Amount of data from ASM to I/O controller. */
   UINT8    u8InputDataSize;
   /* checksum */
   UINT16   u16Crc;
} AICMSGDEF_ASM_STARTUP_TELEGRAM_STRUCT;

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


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of aicMsgDef.h
**
********************************************************************************
*/

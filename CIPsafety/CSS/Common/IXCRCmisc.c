/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRCmisc.c
**    Summary: IXCRC - CRC calculation routines
**             This module contains miscellaneous routines for calculating and
**             checking of several safety measures.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_ParityCalc
**             IXCRC_ActVsCompDataCheck
**             IXCRC_TcByte2Calc
**             IXCRC_ActVsCompDataGen
**             IXCRC_ModeByteRedundantBitsCheck
**             IXCRC_ModeByteRedundantBitsCalc
**             IXCRC_PidCidCrcS1Calc
**             IXCRC_PidCidCrcS3Calc
**             IXCRC_PidCidCrcS5Calc
**             IXCRC_PidRcCrcS3Calc
**             IXCRC_PidRcCrcS5Calc
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSCF.h"

#include "IXCRC.h"
#include "IXCRCint.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** kab_ParityTable:
    Lookup Table for parity calculation of the ACK_Byte of Time Coordination
    Section and MCast_Byte of Time Correction Section. Value of bits 0-6 is the
    index into the table. Bit 7 of the value in the table is the even parity.
    This was not part of the example code.
*/
static const CSS_t_BYTE kab_ParityTable[128] =
{
  /* 00-07 */  0x00U, 0x81U, 0x82U, 0x03U, 0x84U, 0x05U, 0x06U, 0x87U,
  /* 08-0F */  0x88U, 0x09U, 0x0AU, 0x8BU, 0x0CU, 0x8DU, 0x8EU, 0x0FU,
  /* 10-17 */  0x90U, 0x11U, 0x12U, 0x93U, 0x14U, 0x95U, 0x96U, 0x17U,
  /* 18-1F */  0x18U, 0x99U, 0x9AU, 0x1BU, 0x9CU, 0x1DU, 0x1EU, 0x9FU,
  /* 20-27 */  0xA0U, 0x21U, 0x22U, 0xA3U, 0x24U, 0xA5U, 0xA6U, 0x27U,
  /* 28-2F */  0x28U, 0xA9U, 0xAAU, 0x2BU, 0xACU, 0x2DU, 0x2EU, 0xAFU,
  /* 30-37 */  0x30U, 0xB1U, 0xB2U, 0x33U, 0xB4U, 0x35U, 0x36U, 0xB7U,
  /* 38-3F */  0xB8U, 0x39U, 0x3AU, 0xBBU, 0x3CU, 0xBDU, 0xBEU, 0x3FU,
  /* 40-47 */  0xC0U, 0x41U, 0x42U, 0xC3U, 0x44U, 0xC5U, 0xC6U, 0x47U,
  /* 48-4F */  0x48U, 0xC9U, 0xCAU, 0x4BU, 0xCCU, 0x4DU, 0x4EU, 0xCFU,
  /* 50-57 */  0x50U, 0xD1U, 0xD2U, 0x53U, 0xD4U, 0x55U, 0x56U, 0xD7U,
  /* 58-5F */  0xD8U, 0x59U, 0x5AU, 0xDBU, 0x5CU, 0xDDU, 0xDEU, 0x5FU,
  /* 60-67 */  0x60U, 0xE1U, 0xE2U, 0x63U, 0xE4U, 0x65U, 0x66U, 0xE7U,
  /* 68-6F */  0xE8U, 0x69U, 0x6AU, 0xEBU, 0x6CU, 0xEDU, 0xEEU, 0x6FU,
  /* 70-77 */  0xF0U, 0x71U, 0x72U, 0xF3U, 0x74U, 0xF5U, 0xF6U, 0x77U,
  /* 78-7F */  0x78U, 0xF9U, 0xFAU, 0x7BU, 0xFCU, 0x7DU, 0x7EU, 0xFFU,
};


/** k_PIDCID_STREAM_SIZE:
    Number of bytes for storing a PID/CID as a byte stream. Needed to calculate
    CRCs from PID/CIDs.
*/
#define k_PIDCID_STREAM_SIZE       8U


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXCRC_ParityCalc
**
** Description : This function calculates the Parity Bit of the Ack_Byte in a
**               Time Coordination Message (see FRS56) or of the Mcast_Byte of a
**               Time Correction Message (see FRS66).
**
** Parameters  : b_ackOmcastByte (IN) - Ack_Byte or Mcast_Byte
**                                      (not checked, any value allowed)
**
** Returnvalue : CSS_t_BYTE           - Calculated Ack_Byte or Mcast_Byte value
**
*******************************************************************************/
CSS_t_BYTE IXCRC_ParityCalc(CSS_t_BYTE b_ackOmcastByte)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* strip off bit 7 and look up the value from the table */
  return (kab_ParityTable[b_ackOmcastByte & 0x7FU]);
}


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_ActVsCompDataCheck
**
** Description : This function checks the Actual Data versus the Complement Data
**               of a 3 to 250 Byte Message.
**
** Parameters  : ps_dataMsg (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**
** Returnvalue : CSS_k_OK        - Complement Data match
**               <>CSS_k_OK      - Complement Data incorrect
**
*******************************************************************************/
CSS_t_UINT IXCRC_ActVsCompDataCheck(const CSS_t_DATA_MSG *ps_dataMsg)
{
  /* function's return value */
  CSS_t_UINT  w_retVal = CSS_k_OK;
  /* loop index for running through data array */
  CSS_t_USINT u8_idx;

  /* In case a mismatch is found the loop is still executed for all the data
     bytes. Finding a mismatch should be the exception. Adding further break
     conditions for the loop (or coding it as do .. while) would be less
     efficient. */

  /* for each data byte of the message ... */
  for (u8_idx = 0U; u8_idx < ps_dataMsg->u8_len; u8_idx++)
  {
    /* calculate the complement of the actual data (use a temporary variable to
       avoid compiler warnings) */
    CSS_t_BYTE b_compOfActual = ps_dataMsg->ab_aData[u8_idx] ^ 0xFFU;
    /* if the complement of Actual Data is different than the Complement Data */
    if (b_compOfActual != ps_dataMsg->ab_cData[u8_idx])
    {
      /* data mismatch detected! */
      w_retVal = IXCRC_k_ERR_ACC_VS_COMP;
    }
    else /* else: match */
    {
      /* continue */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_TcByte2Calc
**
** Description : This function calculates the Ack_Byte_2 from the Ack_Byte of a
**               Base Format Time Coordination Message or the Mcast_Byte_2 from
**               the Mcast_Byte of a Base Format Time Correction Message.
**               Ack_Byte_2   = ((Ack_B   XOR 0xFF)AND0x55)OR(Ack_B   AND 0xAA)
**               Mcast_Byte_2 = ((Mcast_B XOR 0xFF)AND0x55)OR(Mcast_B AND 0xAA)
**
** Parameters  : b_ackOmcastByte (IN) - Ack_Byte or Mcast_Byte
**                                      (not checked, any value allowed)
**
** Returnvalue : CSS_t_BYTE           - Calculated Ack_Byte or Mcast_Byte value
**
*******************************************************************************/
CSS_t_BYTE IXCRC_TcByte2Calc(CSS_t_BYTE b_ackOmcastByte)
{
  /* temporary variable for local calculation of expected Ack_Byte_2 */
  CSS_t_BYTE b_byte2 = 0x00U;

  /* calculate the expected value for Ack_Byte_2 according to Ack_Byte */
  /* respectively Mcast_Byte_2 according to Mcast_Byte */
  /* (see FRS57 and FRS67) */
  b_byte2 =   (CSS_t_BYTE)(((b_ackOmcastByte ^ 0xFFU) & 0x55U)
            | (b_ackOmcastByte & 0xAAU));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (b_byte2);
}
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_ActVsCompDataGen
**
** Description : This function generates the Complemented Data from the Actual
**               Data of a 3 to 250 Byte Message.
**
** Parameters  : ps_dataMsg (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**
** Returnvalue : -
**
*******************************************************************************/
void IXCRC_ActVsCompDataGen(CSS_t_DATA_MSG *ps_dataMsg)
{
  /* loop index for running through data array */
  CSS_t_USINT u8_idx;

  /* for each data byte of the message ... */
  for (u8_idx = 0U; u8_idx < ps_dataMsg->u8_len; u8_idx++)
  {
    /* calculate the Complemented Data from the Actual Data */
    ps_dataMsg->ab_cData[u8_idx] = ps_dataMsg->ab_aData[u8_idx] ^ 0xFFU;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}

#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
/*******************************************************************************
**
** Function    : IXCRC_ModeByteRedundantBitsCheck
**
** Description : This function checks the integrity of a Mode Byte. Complement
**               and copy bits must match their counterparts.
**
** Parameters  : b_modeByte  (IN) - Mode Byte of a message
**                                  (checked, see algorithm in Description
**                                  above)
**
** Returnvalue : CSS_k_OK         - Mode Byte is ok
**               <>CSS_k_OK       - Mode Byte is corrupted
**
*******************************************************************************/
CSS_t_UINT IXCRC_ModeByteRedundantBitsCheck(CSS_t_BYTE b_modeByte)
{
  /* function's return value */
  CSS_t_UINT  w_retVal = IXCRC_k_ERR_MODE_BYTE;

  /* right adjust N_Run_Idle, TBD_2_Bit_Copy and N_TBD_Bit */
  CSS_t_BYTE b_nBits = (CSS_t_BYTE)((b_modeByte >> 2) & 0x07U);
  /* right adjust Run_Idle, TBD_2_Bit, TBD_Bit */
  CSS_t_BYTE b_aBits = (CSS_t_BYTE)((b_modeByte >> 5) & 0x07U);

  /* if (Run_Idle == !N_Run_Idle) and (TBD_2_Bit == TBD_2_Bit_Copy) and
     (TBD_Bit == !N_TBD_Bit) (see FRS190) (due to XOR operation complemented
     bits must be 1 and equal bits must be 0, thus check for 0x05) */
  if ((b_aBits ^ b_nBits) == 0x05U)
  {
    w_retVal = CSS_k_OK;
  }
  else /* else: mismatch */
  {
    /* error return value already set */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : IXCRC_ModeByteRedundantBitsCalc
**
** Description : This function calculates the redundant bits in a Mode Byte.
**
** Parameters  : b_modeByte (IN) - the mode byte
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_BYTE      - mode byte with correctly set redundant bits
**
*******************************************************************************/
CSS_t_BYTE IXCRC_ModeByteRedundantBitsCalc(CSS_t_BYTE b_modeByte)
{
  /* (Mode_Byte AND 0xe3)OR((Mode_Byte>>3) AND 0x1c XOR 0x14), */

  CSS_t_BYTE b_retVal = (CSS_t_BYTE)(((b_modeByte) & 0xE3U)
                       | ((((b_modeByte) >> 3) & 0x1CU) ^ 0x14U));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (b_retVal);
}
#endif


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_PidCidCrcS1Calc
**
** Description : This function calculates the CRC-S1 from a PID/CID.
**
** Parameters  : ps_pidcid (IN)   - pointer to PID/CID struct
**                                  (not checked, only called with reference to
**                                  structure)
**
** Returnvalue : CSS_t_USINT      - CRC-S1 of the passed PID/CID
**
*******************************************************************************/
CSS_t_USINT IXCRC_PidCidCrcS1Calc(const CSS_t_PIDCID *ps_pidcid)
{
  /* temporary storage for PID/CID as a byte stream */
  CSS_t_BYTE   ab_stream[k_PIDCID_STREAM_SIZE];
  CSS_t_USINT  u8_crcS1;

  /* convert PID/CID struct to a byte stream */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 0U), &ps_pidcid->u16_vendId);
  CSS_H2N_CPY32(CSS_ADD_OFFSET(ab_stream, 2U), &ps_pidcid->u32_devSerNum);
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 6U), &ps_pidcid->u16_cnxnSerNum);

  u8_crcS1 = IXCRC_CrcS1compute(ab_stream, k_PIDCID_STREAM_SIZE, 0U);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_crcS1);
}
#endif


/*******************************************************************************
**
** Function    : IXCRC_PidCidCrcS3Calc
**
** Description : This function calculates the CRC-S3 from a PID/CID.
**
** Parameters  : ps_pidcid (IN)   - pointer to PID/CID struct
**                                  (not checked, only called with reference to
**                                  structure)
**
** Returnvalue : CSS_t_UINT       - CRC-S3 of the passed PID/CID
**
*******************************************************************************/
CSS_t_UINT IXCRC_PidCidCrcS3Calc(const CSS_t_PIDCID *ps_pidcid)
{
  /* temporary storage for PID/CID as a byte stream */
  CSS_t_BYTE   ab_stream[k_PIDCID_STREAM_SIZE];
  CSS_t_UINT   u16_crcS3;

  /* convert PID/CID struct to a byte stream */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 0U), &ps_pidcid->u16_vendId);
  CSS_H2N_CPY32(CSS_ADD_OFFSET(ab_stream, 2U), &ps_pidcid->u32_devSerNum);
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 6U), &ps_pidcid->u16_cnxnSerNum);

  u16_crcS3 = IXCRC_CrcS3compute(ab_stream, k_PIDCID_STREAM_SIZE, 0U);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_crcS3);
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_PidCidCrcS5Calc
**
** Description : This function calculates the CRC-S5 from a PID/CID.
**
** Parameters  : ps_pidcid (IN)   - pointer to PID/CID struct
**                                  (not checked, only called with reference to
**                                  structure)
**
** Returnvalue : CSS_t_UDINT      - CRC-S5 of the passed PID/CID
**
*******************************************************************************/
CSS_t_UDINT IXCRC_PidCidCrcS5Calc(const CSS_t_PIDCID *ps_pidcid)
{
  /* temporary storage for PID/CID as a byte stream */
  CSS_t_BYTE   ab_stream[k_PIDCID_STREAM_SIZE];
  CSS_t_UDINT  u32_crcS5;

  /* convert PID/CID struct to a byte stream */
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 0U), &ps_pidcid->u16_vendId);
  CSS_H2N_CPY32(CSS_ADD_OFFSET(ab_stream, 2U), &ps_pidcid->u32_devSerNum);
  CSS_H2N_CPY16(CSS_ADD_OFFSET(ab_stream, 6U), &ps_pidcid->u16_cnxnSerNum);

  u32_crcS5 = IXCRC_CrcS5compute(ab_stream, k_PIDCID_STREAM_SIZE, 0U);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_crcS5);
}
#endif


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
#if (    (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)  \
      || (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)        \
    )
/*******************************************************************************
**
** Function    : IXCRC_PidRcCrcS3Calc
**
** Description : This function calculates CRC-S3 from PID and Rollover Count.
**
** Parameters  : u16_rcUsedInCrc (IN) - Rollover Count value
**                                      (not checked, any value allowed)
**               u16_pidCrcS3    (IN) - CRC-S3 of the PID as input (seed) for
**                                      CRC-S3 calculation.
**                                      (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT           - CRC-S3 of the passed PID and Rollover
**                                      Count
**
*******************************************************************************/
CSS_t_UINT IXCRC_PidRcCrcS3Calc(CSS_t_UINT u16_rcUsedInCrc,
                                CSS_t_UINT u16_pidCrcS3)
{
  /* temporary storage for Rollover Count as a byte stream */
  CSS_t_BYTE  ab_stream[2U];
  CSS_t_UINT  u16_crcS3;

  /* convert Rollover Count to a byte stream */
  CSS_H2N_CPY16(ab_stream, &u16_rcUsedInCrc);

  /* calculate CRC-S3 of Rollover Count, CRC of PID is the seed */
  u16_crcS3 = IXCRC_CrcS3compute(ab_stream, 2U, u16_pidCrcS3);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_crcS3);
}
#endif  /* CSOS_cfg_LONG_FORMAT || CSOS_cfg_NUM_OF_SV_CLIENTS */


/*******************************************************************************
**
** Function    : IXCRC_PidRcCrcS5Calc
**
** Description : This function calculates CRC-S5 from PID and Rollover Count.
**
** Parameters  : u16_rcUsedInCrc (IN) - Rollover Count value
**                                      (not checked, any value allowed)
**               u32_pidCrcS5    (IN) - CRC-S5 of the PID as input (seed) for
**                                      CRC-S5 calculation.
**                                      (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT          - CRC-S5 of the passed PID and Rollover
**                                      Count
**
*******************************************************************************/
CSS_t_UDINT IXCRC_PidRcCrcS5Calc(CSS_t_UINT u16_rcUsedInCrc,
                                 CSS_t_UDINT u32_pidCrcS5)
{
  /* temporary storage for Rollover Count as a byte stream */
  CSS_t_BYTE  ab_stream[2U];
  CSS_t_UDINT u32_crcS5;

  /* convert Rollover Count to a byte stream */
  CSS_H2N_CPY16(ab_stream, &u16_rcUsedInCrc);

  /* calculate CRC-S5 of Rollover Count, CRC of PID is the seed */
  u32_crcS5 = IXCRC_CrcS5compute(ab_stream, 2U, u32_pidCrcS5);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_crcS5);
}
#endif


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


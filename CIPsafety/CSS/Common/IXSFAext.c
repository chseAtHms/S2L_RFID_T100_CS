/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSFAext.c
**    Summary: IXSFA - Safety Frame Assembly
**             This module contains the routines to convert Extended Format
**             Safety I/O message in a byte stream to a data structure as used
**             by the SVC and SVS units and vice versa.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSFA_EfsMsgRead
**             IXSFA_EflMsgRead
**             IXSFA_EfTcrMsgRead
**             IXSFA_EfTcooMsgWrite
**             IXSFA_EfTcooMsgRead
**             IXSFA_EfsMsgWrite
**             IXSFA_EflMsgWrite
**             IXSFA_EfTcrMsgWrite
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSScfg.h"
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)

#include "CSOScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"


/* Header file containing the mix macros for the configured copy method */
#if (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_ODD_EVEN_SORT)
  #include "CSSmixOES.h"
#elif (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_INTERLACE_EVEN)
  #include "CSSmixIE.h"
#elif (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_INTERLACE_ODD)
  #include "CSSmixIO.h"
#elif (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_HOMOGENEOUS_MIX)
  #include "CSSmixHM.h"
#endif

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSERapi.h"
#endif
#include "IXSCF.h"

#include "IXSFA.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSFAerr.h"
#endif


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* length of a CRC-S5 in a byte stream is 3 bytes (24 bits) */
#define k_CRC_S5_LEN  3U


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_EfsMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into an Extended Format Short Message structure (in
**               host format). The passed length (u8_expPayLen) is the length
**               of the Safety I/O payload configured for this instance. As the
**               resulting expected number of bytes is not stored (and complex
**               to calculate) we copy the message with the expected payload
**               length and compare afterwards (by the caller) if this matches
**               with the received number of bytes.
**               As we write the expected number of bytes into the message
**               structure it is sure that we don't write beyond the structure's
**               data buffer!
**
** Parameters  : pb_data (IN)      - pointer to the byte stream
**                                   (not checked, called with pointer to HALC
**                                   message data + Offset)
**               u8_expPayLen (IN) - expected payload message length
**                                   (not checked, passed value is the length
**                                   configured for this instance)
**               ps_efsMsg (OUT)   - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EfsMsgRead(const CSS_t_BYTE *pb_data,
                            CSS_t_USINT u8_expPayLen,
                            CSS_t_DATA_MSG *ps_efsMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;
  /* temporary variables needed for reading the CRC-S5 (the CRC-S5 is split   */
  /* in the msg)                                                              */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;

  /* 1 or 2 Byte Data Section, Extended Format (see FRS365) */

  /* copy the first data byte */
  CSS_N2H_CPY8(&ps_efsMsg->ab_aData[0], &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* if the message has 2 bytes copy another one */
  if (u8_expPayLen == CSS_k_SHORT_FORMAT_MAX_LENGTH)
  {
    CSS_N2H_CPY8(&ps_efsMsg->ab_aData[1], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }
  else /* one byte messages */
  {
    /* there are only 1 and 2 Byte messages */
  }

  /* copy the Mode Byte */
  CSS_N2H_CPY8(&ps_efsMsg->b_modeByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy the first part of the CRC */
  CSS_N2H_CPY16(&u16_temp, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* Copy Time Stamp */
  CSS_N2H_CPY16(&ps_efsMsg->u16_timeStamp_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy the second part of the CRC */
  CSS_N2H_CPY8(&u8_temp, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* assemble the CRC from the parts */
  ps_efsMsg->u32_crcS5 = (CSS_t_UDINT)u16_temp + ((CSS_t_UDINT)u8_temp << 16U);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_EflMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into an Extended Format Long Message structure (in host
**               format). The passed length (u8_expPayLen) is the length
**               of the Safety I/O payload configured for this instance. As the
**               resulting expected number of bytes is not stored (and complex
**               to calculate) we copy the message with the expected payload
**               length and compare afterwards (by the caller) if this matches
**               with the received number of bytes.
**               As we write the expected number of bytes into the message
**               structure it is sure that we don't write beyond the structure's
**               data buffer!
**
** Parameters  : pb_data (IN)      - pointer to the byte stream
**                                   (not checked, called with pointer to HALC
**                                   message data + Offset)
**               u8_expPayLen (IN) - expected payload message length
**                                   (not checked, passed value is the length
**                                   configured for this instance)
**               ps_eflMsg (OUT)   - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EflMsgRead(const CSS_t_BYTE *pb_data,
                            CSS_t_USINT u8_expPayLen,
                            CSS_t_DATA_MSG *ps_eflMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;
  /* temporary variables needed for reading the CRC-S5 (the CRC-S5 is split   */
  /* in the msg)                                                              */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;
  /* loop index for copying data */
  CSS_t_USINT u8_idx;

  /* 3 to 250 Byte Data Section, Extended Format (see FRS367) */

  /* copy Actual Data */
  for (u8_idx = 0U; u8_idx < u8_expPayLen; u8_idx++)
  {
    CSS_N2H_CPY8(&ps_eflMsg->ab_aData[u8_idx], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }

  /* copy Mode Byte */
  CSS_N2H_CPY8(&ps_eflMsg->b_modeByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* Copy Actual CRC */
  CSS_N2H_CPY16(&ps_eflMsg->u16_aCrcS3, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy Complemented Data */
  for (u8_idx = 0U; u8_idx < u8_expPayLen; u8_idx++)
  {
    CSS_N2H_CPY8(&ps_eflMsg->ab_cData[u8_idx], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }

  /* copy the first part of the CRC-S5 */
  CSS_N2H_CPY16(&u16_temp, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* Copy Time Stamp */
  CSS_N2H_CPY16(&ps_eflMsg->u16_timeStamp_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy the second part of the CRC-S5 */
  CSS_N2H_CPY8(&u8_temp, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* assemble the CRC-S5 from the parts */
  ps_eflMsg->u32_cCrcS5 = (CSS_t_UDINT)u16_temp + ((CSS_t_UDINT)u8_temp << 16U);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSFA_EfTcrMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into an Extended Format Time Correction Message
**               structure (in host format).
**
** Parameters  : pb_data (IN)      - pointer to the byte stream
**                                   (not checked, called with pointer to HALC
**                                   message data + Offset)
**               ps_efTcrMsg (OUT) - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EfTcrMsgRead(const CSS_t_BYTE *pb_data,
                              CSS_t_TCORR_MSG *ps_efTcrMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* copy Mcast_Byte */
  CSS_N2H_CPY8(&ps_efTcrMsg->b_mCastByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy Time_Correction_Value */
  CSS_N2H_CPY16(&ps_efTcrMsg->u16_timeCorrVal_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy CRC-S5 */
  CSS_N2H_CPY32(&ps_efTcrMsg->u32_crcS5, &pb_data[u16_cpIdx]);
  u16_cpIdx += k_CRC_S5_LEN;

  /* blank out the most significant 8 Bits as the CRC-S5 is only 24 Bits */
  ps_efTcrMsg->u32_crcS5 &= 0x00FFFFFFU;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}


/*******************************************************************************
**
** Function    : IXSFA_EfTcooMsgWrite
**
** Description : This function copies data from an Extended Format Time
**               Coordination Message structure into a byte stream.
**
** Parameters  : ps_efTcooMsg (IN) - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSFA_EfTcooMsgWrite(const CSS_t_TCOO_MSG *ps_efTcooMsg,
                          CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;
  /* temporary variables needed for writing the CRC-S5 */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;

  /* for all CSS_H2N_xxx macros: suppress lint warnings                       */
  /* - "Null statement not in line by itself"                                 */
  /* - "Boolean within 'String' always evaluates to [True/False]"             */

  /* copy Ack_Byte */
  CSS_MIX_CPY8(pb_data, &ps_efTcooMsg->b_ackByte,
               u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);          /*lint !e960 !e774 */
  u16_cpIdx++;

  /* copy Consumer_Time_Value */
  CSS_MIX_CPY16(pb_data, &ps_efTcooMsg->u16_consTimeVal_128us,
                u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);         /*lint !e960 !e774 */
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* prepare the parts of the CRC-S5 for writing it to the buffer */
  /* TMS320: Most significant 8 bits are certainly zero because u32_crcS5 has
             been calculated by IXCRC_CrcS5compute() which enforces that. */
  u16_temp = (CSS_t_UINT)(ps_efTcooMsg->u32_crcS5 & 0x0000FFFFU);
  u8_temp = (CSS_t_USINT)(ps_efTcooMsg->u32_crcS5 >> 16U);

  /* copy CRC-S5 */
  CSS_MIX_CPY16(pb_data, &u16_temp,
                u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);         /*lint !e960 !e774 */
  u16_cpIdx += CSOS_k_SIZEOF_UINT;
  CSS_MIX_CPY8(pb_data, &u8_temp, u16_cpIdx,
               CSOS_k_IO_MSGLEN_TCOO);                     /*lint !e960 !e774 */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
} /*lint !e438 Last value assigned to variable
               'u8_temp' may be unused (depending on configuration, i.e.
               CSS_cfg_H2N_COPY_METHOD) */
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : IXSFA_EfTcooMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into an Extended Format Time Coordination Message
**               structure (in host format).
**
** Parameters  : pb_data (IN)       - pointer to the byte stream
**                                    (not checked, called with pointer to HALC
**                                    message data + Offset)
**               ps_efTcooMsg (OUT) - pointer to the message structure
**                                    (not checked, only called with reference
**                                    to structure)
**
** Returnvalue : CSS_t_UINT         - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EfTcooMsgRead(const CSS_t_BYTE *pb_data,
                               CSS_t_TCOO_MSG *ps_efTcooMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* copy Ack_Byte */
  CSS_N2H_CPY8(&ps_efTcooMsg->b_ackByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy Consumer_Time_Value */
  CSS_N2H_CPY16(&ps_efTcooMsg->u16_consTimeVal_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy CRC-S5 */
  CSS_N2H_CPY32(&ps_efTcooMsg->u32_crcS5, &pb_data[u16_cpIdx]);
  u16_cpIdx += k_CRC_S5_LEN;

  /* blank out the most significant 8 Bits as the CRC-S5 is only 24 Bits */
  ps_efTcooMsg->u32_crcS5 &= 0x00FFFFFFU;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}


#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_EfsMsgWrite
**
** Description : This function copies data from an Extended Format Short Message
**               structure into a byte stream. Message length must be set in the
**               structure before calling this function.
**
** Parameters  : ps_efsMsg (IN)    - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               u16_frameLen (IN) - number of bytes that the complete frame
**                                   will have (data + time correction)
**                                   (checked, valid range:
**                                   <= (CSOS_k_IO_MSGLEN_SHORT_MAX
**                                   + CSOS_k_IO_MSGLEN_TCORR))
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EfsMsgWrite(const CSS_t_DATA_MSG *ps_efsMsg,
                             CSS_t_UINT u16_frameLen,
                             CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;
  /* temporary variables needed for reading the CRC-S5 (the CRC-S5 is split   */
  /* in the msg)                                                              */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;

  /* if passed parameter is invalid */
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_SHORT_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_EFS_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
  else /* else: parameters ok */
  {
    /* 1 or 2 Byte Data Section, Extended Format (see FRS365) */

    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy the first data byte */
    CSS_MIX_CPY8(pb_data, &ps_efsMsg->ab_aData[0],
                 u16_cpIdx, u16_frameLen);            /*lint !e960 !e774 */
    u16_cpIdx++;

    /* in case the message has 2 bytes copy another one */
    if (ps_efsMsg->u8_len == CSS_k_SHORT_FORMAT_MAX_LENGTH)
    {
      CSS_MIX_CPY8(pb_data, &ps_efsMsg->ab_aData[1],
                   u16_cpIdx, u16_frameLen);          /*lint !e960 !e774 */
      u16_cpIdx++;
    }
    else
    {
      /* nothing to do */
    }

    /* copy the Mode Byte */
    CSS_MIX_CPY8(pb_data, &ps_efsMsg->b_modeByte,
                 u16_cpIdx, u16_frameLen);            /*lint !e960 !e774 */
    u16_cpIdx++;

    /* prepare the parts of the CRC-S5 for writing it to the buffer */
    /* TMS320: Most significant 8 bits are certainly zero because u32_crcS5 has
               been calculated by IXCRC_CrcS5compute() which enforces that. */
    u16_temp = (CSS_t_UINT)(ps_efsMsg->u32_crcS5 & 0x0000FFFFU);
    u8_temp = (CSS_t_USINT)(ps_efsMsg->u32_crcS5 >> 16U);

    /* copy the first part of the CRC */
    CSS_MIX_CPY16(pb_data, &u16_temp,
                  u16_cpIdx, u16_frameLen);           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* Copy Time Stamp */
    CSS_MIX_CPY16(pb_data, &ps_efsMsg->u16_timeStamp_128us,
                  u16_cpIdx, u16_frameLen);           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* copy the second part of the CRC */
    CSS_MIX_CPY8(pb_data, &u8_temp,
                 u16_cpIdx, u16_frameLen);            /*lint !e960 !e774 */
    u16_cpIdx++;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_EflMsgWrite
**
** Description : This function copies data from an Extended Format Long Message
**               structure into a byte stream. Message length must be set in the
**               structure before calling this function.
**
** Parameters  : ps_eflMsg (IN)    - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               u16_frameLen (IN) - number of bytes that the complete frame
**                                   will have (data + time correction)
**                                   (checked, valid range:
**                                   <= (CSOS_k_IO_MSGLEN_LONG_MAX
**                                   + CSOS_k_IO_MSGLEN_TCORR))
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EflMsgWrite(const CSS_t_DATA_MSG *ps_eflMsg,
                             CSS_t_UINT u16_frameLen,
                             CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;
  /* temporary variables needed for reading the CRC-S5 (the CRC-S5 is  split  */
  /* in the msg)                                                              */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;
  /* loop index for copying data */
  CSS_t_USINT u8_idx;

  /* if passed parameter is invalid */
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_LONG_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_EFL_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
  else /* else: parameters ok */
  {
    /* 3 to 250 Byte Data Section, Extended Format (see FRS367) */

    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy Actual Data */
    for (u8_idx = 0U; u8_idx < ps_eflMsg->u8_len; u8_idx++)
    {
      CSS_MIX_CPY8(pb_data, &ps_eflMsg->ab_aData[u8_idx], u16_cpIdx,
                   u16_frameLen);                          /*lint !e960 !e774 */
      u16_cpIdx++;
    }

    /* copy Mode Byte */
    CSS_MIX_CPY8(pb_data, &ps_eflMsg->b_modeByte, u16_cpIdx,
                 u16_frameLen);                            /*lint !e960 !e774 */
    u16_cpIdx++;

    /* Copy Actual CRC */
    CSS_MIX_CPY16(pb_data, &ps_eflMsg->u16_aCrcS3, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* copy Complemented Data */
    for (u8_idx = 0U; u8_idx < ps_eflMsg->u8_len; u8_idx++)
    {
      CSS_MIX_CPY8(pb_data, &ps_eflMsg->ab_cData[u8_idx], u16_cpIdx,
                   u16_frameLen);                          /*lint !e960 !e774 */
      u16_cpIdx++;
    }

    /* prepare the parts of the CRC-S5 for writing it to the buffer */
    /* TMS320: Most significant 8 bits are certainly zero because u32_crcS5 has
               been calculated by IXCRC_CrcS5compute() which enforces that. */
    u16_temp = (CSS_t_UINT)(ps_eflMsg->u32_cCrcS5 & 0x0000FFFFU);
    u8_temp = (CSS_t_USINT)(ps_eflMsg->u32_cCrcS5 >> 16U);

    /* copy the first part of the CRC */
    CSS_MIX_CPY16(pb_data, &u16_temp, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* Copy Time Stamp */
    CSS_MIX_CPY16(pb_data, &ps_eflMsg->u16_timeStamp_128us, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* copy the second part of the CRC */
    CSS_MIX_CPY8(pb_data, &u8_temp, u16_cpIdx,
                 u16_frameLen);                            /*lint !e960 !e774 */
    u16_cpIdx++;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSFA_EfTcrMsgWrite
**
** Description : This function copies data from an Extended Format Time
**               Correction Message structure into a byte stream.
**
** Parameters  : ps_efTcrMsg (IN)  - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               u16_offset (IN)   - offset to where the message must be written
**                                   into pb_data
**               u16_frameLen (IN) - number of bytes that the complete frame
**                                   will have (data + time correction)
**                                   (checked, valid range:
**                                   if long supported:
**                                     <= (CSOS_k_IO_MSGLEN_LONG_MAX
**                                     + CSOS_k_IO_MSGLEN_TCORR)
**                                   else (only short supported):
**                                     <= (CSOS_k_IO_MSGLEN_SHORT_MAX
**                                     + CSOS_k_IO_MSGLEN_TCORR))
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_EfTcrMsgWrite(const CSS_t_TCORR_MSG *ps_efTcrMsg,
                               CSS_t_UINT u16_offset,
                               CSS_t_UINT u16_frameLen,
                               CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = u16_offset;
  /* temporary variables needed for writing the CRC-S5 */
  CSS_t_UINT  u16_temp;
  CSS_t_USINT u8_temp;

  /* if passed parameter is invalid */
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_LONG_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_EFTCR_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
#else
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_SHORT_MAX + CSOS_k_IO_MSGLEN_TCORR))

  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_EFTCR_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
#endif
  else /* else: parameters ok */
  {
    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy Mcast_Byte */
    CSS_MIX_CPY8(pb_data, &ps_efTcrMsg->b_mCastByte,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* copy Time_Correction_Value */
    CSS_MIX_CPY16(pb_data, &ps_efTcrMsg->u16_timeCorrVal_128us,
                  u16_cpIdx, u16_frameLen);                /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* prepare the parts of the CRC-S5 for writing it to the buffer */
    /* TMS320: Most significant 8 bits are certainly zero because u32_crcS5 has
               been calculated by IXCRC_CrcS5compute() which enforces that. */
    u16_temp = (CSS_t_UINT)(ps_efTcrMsg->u32_crcS5 & 0x0000FFFFU);
    u8_temp = (CSS_t_USINT)(ps_efTcrMsg->u32_crcS5 >> 16U);

    /* copy CRC-S5 */
    CSS_MIX_CPY16(pb_data, &u16_temp,
                  u16_cpIdx, u16_frameLen);                /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;
    CSS_MIX_CPY8(pb_data, &u8_temp,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


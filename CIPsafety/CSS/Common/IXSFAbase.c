/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSFAbase.c
**    Summary: IXSFA - Safety Frame Assembly
**             This module contains the routines to convert Base Format Safety
**             I/O message in a byte stream to a data structure as used by the
**             SVC and SVS units and vice versa.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSFA_BfsMsgRead
**             IXSFA_BflMsgRead
**             IXSFA_BfTcrMsgRead
**             IXSFA_BfTcooMsgWrite
**             IXSFA_BfTcooMsgRead
**             IXSFA_BfsMsgWrite
**             IXSFA_BflMsgWrite
**             IXSFA_BfTcrMsgWrite
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSScfg.h"
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)

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
** Function    : IXSFA_BfsMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into a Base Format Short Message structure (in host
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
**               ps_bfsMsg (OUT)   - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_BfsMsgRead(const CSS_t_BYTE *pb_data,
                            CSS_t_USINT u8_expPayLen,
                            CSS_t_DATA_MSG *ps_bfsMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* 1 or 2 Byte Data Section, Base Format (see FRS38) */

  /* copy the first data byte */
  CSS_N2H_CPY8(&ps_bfsMsg->ab_aData[0], &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* if the message has 2 bytes copy another one */
  if (u8_expPayLen == CSS_k_SHORT_FORMAT_MAX_LENGTH)
  {
    CSS_N2H_CPY8(&ps_bfsMsg->ab_aData[1], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }
  else /* one byte messages */
  {
    /* there are only 1 and 2 Byte messages */
  }

  /* copy the Mode Byte */
  CSS_N2H_CPY8(&ps_bfsMsg->b_modeByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* Copy Actual CRC */
  CSS_N2H_CPY8(&ps_bfsMsg->u8_aCrcS1, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* Copy Complement CRC */
  CSS_N2H_CPY8(&ps_bfsMsg->u8_cCrcS2, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* Copy Time Stamp */
  CSS_N2H_CPY16(&ps_bfsMsg->u16_timeStamp_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* Copy Time Stamp CRC */
  CSS_N2H_CPY8(&ps_bfsMsg->u8_tCrcS1,  &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_BflMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into a Base Format Long Message structure (in host
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
**               ps_bflMsg (OUT)   - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_BflMsgRead(const CSS_t_BYTE *pb_data,
                            CSS_t_USINT u8_expPayLen,
                            CSS_t_DATA_MSG *ps_bflMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;
  /* loop index for copying data */
  CSS_t_USINT u8_idx;

  /* 3 to 250 Byte Data Section, Base Format (see FRS41) */

  /* copy Actual Data */
  for (u8_idx = 0U; u8_idx < u8_expPayLen; u8_idx++)
  {
    CSS_N2H_CPY8(&ps_bflMsg->ab_aData[u8_idx], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }

  /* copy Mode Byte */
  CSS_N2H_CPY8(&ps_bflMsg->b_modeByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* Copy Actual CRC */
  CSS_N2H_CPY16(&ps_bflMsg->u16_aCrcS3, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy Complemented Data */
  for (u8_idx = 0U; u8_idx < u8_expPayLen; u8_idx++)
  {
    CSS_N2H_CPY8(&ps_bflMsg->ab_cData[u8_idx], &pb_data[u16_cpIdx]);
    u16_cpIdx++;
  }

  /* Copy Complement CRC */
  CSS_N2H_CPY16(&ps_bflMsg->u16_cCrcS3, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* Copy Time Stamp */
  CSS_N2H_CPY16(&ps_bflMsg->u16_timeStamp_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* Copy Time Stamp CRC */
  CSS_N2H_CPY8(&ps_bflMsg->u8_tCrcS1, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSFA_BfTcrMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into a Base Format Time Correction Message structure
**               (in host format).
**
** Parameters  : pb_data (IN)      - pointer to the byte stream
**                                   (not checked, called with pointer to HALC
**                                   message data + Offset)
**               ps_bfTcrMsg (OUT) - pointer to the message structure
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_BfTcrMsgRead(const CSS_t_BYTE *pb_data,
                              CSS_t_TCORR_MSG *ps_bfTcrMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* copy Mcast_Byte */
  CSS_N2H_CPY8(&ps_bfTcrMsg->b_mCastByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy Time_Correction_Value */
  CSS_N2H_CPY16(&ps_bfTcrMsg->u16_timeCorrVal_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy Mcast_Byte_2 */
  CSS_N2H_CPY8(&ps_bfTcrMsg->b_mCastByte2, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy CRC-S3 */
  CSS_N2H_CPY16(&ps_bfTcrMsg->u16_crcS3, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}


/*******************************************************************************
**
** Function    : IXSFA_BfTcooMsgWrite
**
** Description : This function copies data from a Base Format Time Coordination
**               Message structure (in host format) into a byte stream (in
**               network format).
**
** Parameters  : ps_bfTcooMsg (IN) - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSFA_BfTcooMsgWrite(const CSS_t_TCOO_MSG *ps_bfTcooMsg,
                          CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;

  /* for all CSS_H2N_xxx macros: suppress lint warnings                       */
  /* - "Null statement not in line by itself"                                 */
  /* - "Boolean within 'String' always evaluates to [True/False]"             */

  /* copy Ack_Byte */
  CSS_MIX_CPY8(pb_data, &ps_bfTcooMsg->b_ackByte,
               u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);          /*lint !e960 !e774 */
  u16_cpIdx++;

  /* copy Consumer_Time_Value */
  CSS_MIX_CPY16(pb_data, &ps_bfTcooMsg->u16_consTimeVal_128us,
                u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);         /*lint !e960 !e774 */
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy Ack_Byte_2 */
  CSS_MIX_CPY8(pb_data, &ps_bfTcooMsg->b_ackByte2,
               u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);          /*lint !e960 !e774 */
  u16_cpIdx++;

  /* copy CRC-S3 */
  CSS_MIX_CPY16(pb_data, &ps_bfTcooMsg->u16_crcS3,
                u16_cpIdx, CSOS_k_IO_MSGLEN_TCOO);         /*lint !e960 !e774 */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
/*******************************************************************************
**
** Function    : IXSFA_BfTcooMsgRead
**
** Description : This function copies data from a byte stream (in network
**               format) into a Base Format Time Coordination Message structure
**               (in host format).
**
** Parameters  : pb_data (IN)       - pointer to the byte stream
**                                    (not checked, called with pointer to HALC
**                                    message data + Offset)
**               ps_bfTcooMsg (OUT) - pointer to the message structure
**                                    (not checked, only called with reference
**                                    to structure)
**
** Returnvalue : CSS_t_UINT         - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_BfTcooMsgRead(const CSS_t_BYTE *pb_data,
                               CSS_t_TCOO_MSG *ps_bfTcooMsg)
{
  /* copy index (points to position inside pb_data[] while parsing the msg) */
  CSS_t_UINT u16_cpIdx = 0U;

  /* copy Ack_Byte */
  CSS_N2H_CPY8(&ps_bfTcooMsg->b_ackByte, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy Consumer_Time_Value */
  CSS_N2H_CPY16(&ps_bfTcooMsg->u16_consTimeVal_128us, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  /* copy Ack_Byte_2 */
  CSS_N2H_CPY8(&ps_bfTcooMsg->b_ackByte2, &pb_data[u16_cpIdx]);
  u16_cpIdx++;

  /* copy CRC-S3 */
  CSS_N2H_CPY16(&ps_bfTcooMsg->u16_crcS3, &pb_data[u16_cpIdx]);
  u16_cpIdx += CSOS_k_SIZEOF_UINT;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}


#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_BfsMsgWrite
**
** Description : This function copies data from a Base Format Short Message
**               structure (in host format) into a byte stream (in network
**               format). Message length must be set in the structure before
**               calling this function.
**
** Parameters  : ps_bfsMsg (IN)    - pointer to the message structure
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
CSS_t_UINT IXSFA_BfsMsgWrite(const CSS_t_DATA_MSG *ps_bfsMsg,
                             CSS_t_UINT u16_frameLen,
                             CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;

  /* if passed parameter is invalid */
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_SHORT_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_BFS_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
  else /* else: parameters ok */
  {
    /* 1 or 2 Byte Data Section, Base Format (see FRS38) */

    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy the first data byte */
    CSS_MIX_CPY8(pb_data, &ps_bfsMsg->ab_aData[0],
                 u16_cpIdx, u16_frameLen);            /*lint !e960 !e774 */
    u16_cpIdx++;

    /* in case the message has 2 bytes copy another one */
    if (ps_bfsMsg->u8_len == CSS_k_SHORT_FORMAT_MAX_LENGTH)
    {
      CSS_MIX_CPY8(pb_data, &ps_bfsMsg->ab_aData[1],
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
      u16_cpIdx++;
    }
    else
    {
      /* nothing to do */
    }

    /* copy the Mode Byte */
    CSS_MIX_CPY8(pb_data, &ps_bfsMsg->b_modeByte,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* Copy Actual CRC */
    CSS_MIX_CPY8(pb_data, &ps_bfsMsg->u8_aCrcS1,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* Copy Complement CRC */
    CSS_MIX_CPY8(pb_data, &ps_bfsMsg->u8_cCrcS2,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* Copy Time Stamp */
    CSS_MIX_CPY16(pb_data, &ps_bfsMsg->u16_timeStamp_128us,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* Copy Time Stamp CRC */
    CSS_MIX_CPY8(pb_data, &ps_bfsMsg->u8_tCrcS1,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSFA_BflMsgWrite
**
** Description : This function copies data from a Base Format Long Message
**               structure (in host format) into a byte stream (in network
**               format). Message length must be set in the structure before
**               calling this function.
**
** Parameters  : ps_bflMsg (IN)    - pointer to the message structure
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
CSS_t_UINT IXSFA_BflMsgWrite(const CSS_t_DATA_MSG *ps_bflMsg,
                             CSS_t_UINT u16_frameLen,
                             CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = 0U;
  /* loop index for copying data */
  CSS_t_USINT u8_idx;

  /* if passed parameter is invalid */
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_LONG_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_BFL_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
  else /* else: parameters ok */
  {
    /* 3 to 250 Byte Data Section, Base Format (see FRS41) */

    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy Actual Data */
    for (u8_idx = 0U; u8_idx < ps_bflMsg->u8_len; u8_idx++)
    {
      CSS_MIX_CPY8(pb_data, &ps_bflMsg->ab_aData[u8_idx], u16_cpIdx,
                   u16_frameLen);                          /*lint !e960 !e774 */
      u16_cpIdx++;
    }

    /* copy Mode Byte */
    CSS_MIX_CPY8(pb_data, &ps_bflMsg->b_modeByte, u16_cpIdx,
                 u16_frameLen);                            /*lint !e960 !e774 */
    u16_cpIdx++;

    /* Copy Actual CRC */
    CSS_MIX_CPY16(pb_data, &ps_bflMsg->u16_aCrcS3, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* copy Complemented Data */
    for (u8_idx = 0U; u8_idx < ps_bflMsg->u8_len; u8_idx++)
    {
      CSS_MIX_CPY8(pb_data, &ps_bflMsg->ab_cData[u8_idx], u16_cpIdx,
                   u16_frameLen);                          /*lint !e960 !e774 */
      u16_cpIdx++;
    }

    /* Copy Complement CRC */
    CSS_MIX_CPY16(pb_data, &ps_bflMsg->u16_cCrcS3, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* Copy Time Stamp */
    CSS_MIX_CPY16(pb_data, &ps_bflMsg->u16_timeStamp_128us, u16_cpIdx,
                  u16_frameLen);                           /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* Copy Time Stamp CRC */
    CSS_MIX_CPY8(pb_data, &ps_bflMsg->u8_tCrcS1, u16_cpIdx,
                 u16_frameLen);                            /*lint !e960 !e774 */
    u16_cpIdx++;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSFA_BfTcrMsgWrite
**
** Description : This function copies data from a Base Format Time Correction
**               Message structure (in host format) into a byte stream (in
**               network format).
**
** Parameters  : ps_bfTcrMsg (IN)  - pointer to the message structure
**                                   (not checked, only called with reference
**                                   to structure)
**               u16_offset (IN)   - offset to where the message must be written
**                                   into pb_data
**               u16_frameLen (IN) - number of bytes that the complete frame
**                                   will have (data + time correction)
**                                   <= (CSOS_k_IO_MSGLEN_LONG_MAX
**                                   + CSOS_k_IO_MSGLEN_TCORR))
**               pb_data (OUT)     - pointer to the byte stream
**                                   (not checked, only called with reference to
**                                   array)
**
** Returnvalue : CSS_t_UINT        - number of bytes that were copied
**
*******************************************************************************/
CSS_t_UINT IXSFA_BfTcrMsgWrite(const CSS_t_TCORR_MSG *ps_bfTcrMsg,
                               CSS_t_UINT u16_offset,
                               CSS_t_UINT u16_frameLen,
                               CSS_t_BYTE *pb_data)
{
  /* copy index (points to position inside pb_data[] while generating the     */
  /* msg)                                                                     */
  CSS_t_UINT u16_cpIdx = u16_offset;

  /* if passed parameter is invalid */
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_LONG_MAX + CSOS_k_IO_MSGLEN_TCORR))
  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_BFTCR_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
#else
  if (u16_frameLen > (CSOS_k_IO_MSGLEN_SHORT_MAX + CSOS_k_IO_MSGLEN_TCORR))

  {
    SAPL_CssErrorClbk(IXSFA_FSE_INC_BFTCR_FRAME_LEN,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_frameLen);
  }
#endif
  else /* else: parameters ok */
  {
    /* for all CSS_H2N_xxx macros: suppress lint warnings                     */
    /* - "Null statement not in line by itself"                               */
    /* - "Boolean within 'String' always evaluates to [True/False]"           */

    /* copy Mcast_Byte */
    CSS_MIX_CPY8(pb_data, &ps_bfTcrMsg->b_mCastByte,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* copy Time_Correction_Value */
    CSS_MIX_CPY16(pb_data, &ps_bfTcrMsg->u16_timeCorrVal_128us,
                  u16_cpIdx, u16_frameLen);                /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;

    /* copy Mcast_Byte_2 */
    CSS_MIX_CPY8(pb_data, &ps_bfTcrMsg->b_mCastByte2,
                 u16_cpIdx, u16_frameLen);                 /*lint !e960 !e774 */
    u16_cpIdx++;

    /* copy CRC-S3 */
    CSS_MIX_CPY16(pb_data, &ps_bfTcrMsg->u16_crcS3,
                  u16_cpIdx, u16_frameLen);                /*lint !e960 !e774 */
    u16_cpIdx += CSOS_k_SIZEOF_UINT;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_cpIdx);
}
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*******************************************************************************
**    static functions
*******************************************************************************/

#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */

/*** End Of File ***/


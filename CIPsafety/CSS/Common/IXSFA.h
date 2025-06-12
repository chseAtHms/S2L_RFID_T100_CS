/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSFA.h
**    Summary: IXSFA - Safety Frame Assembly
**             IXSFA.h is the export header file of the IXSFA unit.
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
**             IXSFA_EfsMsgRead
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

#ifndef IXSFA_H
#define IXSFA_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
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
                            CSS_t_DATA_MSG *ps_bfsMsg);
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
                            CSS_t_DATA_MSG *ps_bflMsg);
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
                              CSS_t_TCORR_MSG *ps_bfTcrMsg);


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
                          CSS_t_BYTE *pb_data);
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
                               CSS_t_TCOO_MSG *ps_bfTcooMsg);


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
                             CSS_t_BYTE *pb_data);
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
                             CSS_t_BYTE *pb_data);
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
                               CSS_t_BYTE *pb_data);
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
                            CSS_t_DATA_MSG *ps_efsMsg);
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
                            CSS_t_DATA_MSG *ps_eflMsg);
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
                              CSS_t_TCORR_MSG *ps_efTcrMsg);


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
                          CSS_t_BYTE *pb_data);
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
                               CSS_t_TCOO_MSG *ps_efTcooMsg);


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
                             CSS_t_BYTE *pb_data);
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
                             CSS_t_BYTE *pb_data);
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
                               CSS_t_BYTE *pb_data);
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


#endif /* #ifndef IXSFA_H */

/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRC.h
**    Summary: IXCRC - CRC calculation routines
**             IXCRC.h is the export header file of the IXCRC unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_CrcS4compute
**             IXCRC_BfsMsgAcrcCalc
**             IXCRC_BfsMsgCcrcCalc
**             IXCRC_BfMsgTsCrcCalc
**             IXCRC_BflMsgAcrcCalc
**             IXCRC_BflMsgCcrcCalc
**             IXCRC_BfTcooMsgCrcCalc
**             IXCRC_BfTcrMsgCrcCalc
**             IXCRC_EfsMsgCrcCalc
**             IXCRC_EflMsgCrcS3Calc
**             IXCRC_EflMsgCcrcCalc
**             IXCRC_EfTcooMsgCrcCalc
**             IXCRC_EfTcrMsgCrcCalc
**             IXCRC_ParityCalc
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

#ifndef IXCRC_H
#define IXCRC_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXCRC_k_ERR_xxx:
    Error Codes and Return Values of the functions of the IXCRC module.
*/
/*lint -esym(755, IXCRC_k_ERR_ACC_VS_COMP)        not referenced in every cfg */
/*lint -esym(755, IXCRC_k_ERR_MODE_BYTE)          not referenced in every cfg */
#define IXCRC_k_ERR_ACC_VS_COMP 0x0001U /* Act vs Comp Data mismatch */
#define IXCRC_k_ERR_MODE_BYTE   0x0002U /* Mode Byte integrity error */


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXCRC_CrcS4compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               CRC32 calculation routine for polynomial 0xedb88320 which is
**               bit reversed from the regular polynomial of 0x04c11db7 thus the
**               (crc >> 8) below is right shifted.
**               Doing incremental crc calculation is done with providing the
**               result from the previous block with preset.
**
** Parameters  : pu8_start  (IN) - Starting address to compute CRC over
**                                 (not checked, only called with references
**                                 to arrays (sometimes with offset)
**               u16_len    (IN) - Number of bytes
**                                 (not checked, any value allowed)
**               u32_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT     - Result of CRC calculation
**
*******************************************************************************/
CSS_t_UDINT IXCRC_CrcS4compute(const CSS_t_USINT *pu8_start,
                               CSS_t_UINT u16_len,
                               CSS_t_UDINT u32_preset);


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_BfsMsgAcrcCalc
**
** Description : This function calculates the Actual CRC-S1 of a Base Format
**               Short Message.
**               seed:   CRC-S1 of PID
**               source: Mode Byte and Actual Data
**
** Parameters  : ps_bfsMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u8_seed    (IN) - Seed value for CRC calculations
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT     - Calculated CRC value
**
*******************************************************************************/
CSS_t_USINT IXCRC_BfsMsgAcrcCalc(const CSS_t_DATA_MSG *ps_bfsMsg,
                                 CSS_t_USINT u8_seed);


/*******************************************************************************
**
** Function    : IXCRC_BfsMsgCcrcCalc
**
** Description : This function calculates the Complementary CRC-S2 of a Base
**               Format Short Message.
**               seed:   CRC-S1 of PID
**               source: complemented Mode Byte and complemented Data
**
** Parameters  : ps_bfsMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u8_seed    (IN) - Seed value for CRC calculations
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT     - Calculated CRC value
**
*******************************************************************************/
CSS_t_USINT IXCRC_BfsMsgCcrcCalc(const CSS_t_DATA_MSG *ps_bfsMsg,
                                 CSS_t_USINT u8_seed);
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXCRC_BfMsgTsCrcCalc
**
** Description : This function calculates the Time Stamp CRC-S1 of a Base Format
**               Message.
**               seed:   CRC-S1 of PID
**               source: Mode Byte and Time Stamp
**
** Parameters  : ps_bfMsg  (IN) - Pointer to message structure
**                                (not checked, only called with constant
**                                pointer to structure)
**               u8_seed   (IN) - Seed value for CRC calculations
**                                (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT    - Calculated CRC value
**
*******************************************************************************/
CSS_t_USINT IXCRC_BfMsgTsCrcCalc(const CSS_t_DATA_MSG *ps_bfMsg,
                                 CSS_t_USINT u8_seed);


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_BflMsgAcrcCalc
**
** Description : This function calculates the Actual CRC-S3 of a Base Format
**               Long Message.
**               seed:   CRC-S3 of PID
**               source: Mode Byte and Actual Data
**
** Parameters  : ps_bflMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u16_seed   (IN) - Seed value for CRC calculations
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Calculated CRC value
**
*******************************************************************************/
CSS_t_UINT IXCRC_BflMsgAcrcCalc(const CSS_t_DATA_MSG *ps_bflMsg,
                                CSS_t_UINT u16_seed);


/*******************************************************************************
**
** Function    : IXCRC_BflMsgCcrcCalc
**
** Description : This function calculates the Complementary CRC-S3 of a Base
**               Format Long Message is correct.
**               seed:   CRC-S3 of PID
**               source: complemented Mode Byte and complemented Data
**
** Parameters  : ps_bflMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u16_seed   (IN) - Seed value for CRC calculations
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Calculated CRC value
**
*******************************************************************************/
CSS_t_UINT IXCRC_BflMsgCcrcCalc(const CSS_t_DATA_MSG *ps_bflMsg,
                                CSS_t_UINT u16_seed);
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXCRC_BfTcooMsgCrcCalc
**
** Description : This function calculates the CRC-S3 of a Base Format Time
**               Coordination Message.
**               seed:   CRC-S3 of CID
**               source: Ack_Byte and Consumer_Time_Value
**
** Parameters  : ps_bfTcooMsg (IN) - Pointer to message structure
**                                   (not checked, only called with reference to
**                                   structure)
**               u16_seed     (IN) - Seed value for CRC calculations
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT       - Calculated CRC value
**
*******************************************************************************/
CSS_t_UINT IXCRC_BfTcooMsgCrcCalc(const CSS_t_TCOO_MSG *ps_bfTcooMsg,
                                  CSS_t_UINT u16_seed);


/*******************************************************************************
**
** Function    : IXCRC_BfTcrMsgCrcCalc
**
** Description : This function calculates the CRC-S3 of a Base Format Time
**               Correction Message.
**               seed:   CRC-S3 of PID
**               source: Mcast_Byte and Time_Correction_Value
**
** Parameters  : ps_bfTcrMsg  (IN) - Pointer to message structure
**                                   (not checked, only called with reference to
**                                   structure)
**               u16_seed     (IN) - Seed value for CRC calculations
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT       - Calculated CRC value
**
*******************************************************************************/
CSS_t_UINT IXCRC_BfTcrMsgCrcCalc(const CSS_t_TCORR_MSG *ps_bfTcrMsg,
                                 CSS_t_UINT u16_seed);
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_EfsMsgCrcCalc
**
** Description : This function calculates the CRC-S5 of an Extended Format
**               Short Message.
**               seed:   CRC-S5 of PID and Rollover Count
**               source: Mode Byte, Actual Data and Time Stamp
**
** Parameters  : ps_efsMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u32_seed   (IN) - Seed value for CRC calculation
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT     - Calculated CRC value
**
*******************************************************************************/
CSS_t_UDINT IXCRC_EfsMsgCrcCalc(const CSS_t_DATA_MSG *ps_efsMsg,
                                CSS_t_UDINT u32_seed);
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_EflMsgCrcS3Calc
**
** Description : This function calculates the Actual Data CRC-S3 of an Extended
**               Format Long Message.
**               seed:   CRC-S3 of PID and Rollover Count
**               source: Mode Byte and Actual Data
**
** Parameters  : ps_eflMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u16_seed   (IN) - Seed value for CRC calculation
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Calculated CRC value
**
*******************************************************************************/
CSS_t_UINT IXCRC_EflMsgCrcS3Calc(const CSS_t_DATA_MSG *ps_eflMsg,
                                 CSS_t_UINT u16_seed);


/*******************************************************************************
**
** Function    : IXCRC_EflMsgCcrcCalc
**
** Description : This function calculates the Complement Data CRC-S5 of an
**               Extended Format Long Message.
**               seed:   CRC-S5 of PID and Rollover Count
**               source: complemented Mode Byte, Complemented Data and
**                       Time Stamp
**
** Parameters  : ps_eflMsg  (IN) - Pointer to message structure
**                                 (not checked, only called with constant
**                                 pointer to structure)
**               u32_seed   (IN) - Seed value for CRC calculation
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT     - Calculated CRC value
**
*******************************************************************************/
CSS_t_UDINT IXCRC_EflMsgCcrcCalc(const CSS_t_DATA_MSG *ps_eflMsg,
                                 CSS_t_UDINT u32_seed);
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXCRC_EfTcooMsgCrcCalc
**
** Description : This function calculates the CRC-S5 of an Extended Format Time
**               Coordination Message.
**               seed:   CRC-S5 of CID
**               source: Ack_Byte and Consumer_Time_Value
**
** Parameters  : ps_efTcooMsg (IN) - Pointer to message structure
**                                   (not checked, only called with reference to
**                                   structure)
**               u32_seed     (IN) - Seed value for CRC calculation
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT       - Calculated CRC value
**
*******************************************************************************/
CSS_t_UDINT IXCRC_EfTcooMsgCrcCalc(const CSS_t_TCOO_MSG *ps_efTcooMsg,
                                   CSS_t_UDINT u32_seed);


/*******************************************************************************
**
** Function    : IXCRC_EfTcrMsgCrcCalc
**
** Description : This function calculates the CRC-S5 of an Extended Format Time
**               Correction Message.
**               seed:   CRC-S5 of PID
**               source: Mcast_Byte and Time_Correction_Value
**
** Parameters  : ps_efTcrMsg  (IN) - Pointer to message structure
**                                   (not checked, only called with reference to
**                                   structure)
**               u32_seed     (IN) - Seed value for CRC calculation
**                                   (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT       - Calculated CRC value
**
*******************************************************************************/
CSS_t_UDINT IXCRC_EfTcrMsgCrcCalc(const CSS_t_TCORR_MSG *ps_efTcrMsg,
                                  CSS_t_UDINT u32_seed);
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


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
CSS_t_BYTE IXCRC_ParityCalc(CSS_t_BYTE b_ackOmcastByte);


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
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
    CSS_t_UINT IXCRC_ActVsCompDataCheck(const CSS_t_DATA_MSG *ps_dataMsg);
  #endif
#endif


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
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_BYTE IXCRC_TcByte2Calc(CSS_t_BYTE b_ackOmcastByte);
#endif


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
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
    void IXCRC_ActVsCompDataGen(CSS_t_DATA_MSG *ps_dataMsg);
  #endif
#endif


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
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  CSS_t_UINT IXCRC_ModeByteRedundantBitsCheck(CSS_t_BYTE b_modeByte);
#endif


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
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  CSS_t_BYTE IXCRC_ModeByteRedundantBitsCalc(CSS_t_BYTE b_modeByte);
#endif


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
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_USINT IXCRC_PidCidCrcS1Calc(const CSS_t_PIDCID *ps_pidcid);
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
CSS_t_UINT IXCRC_PidCidCrcS3Calc(const CSS_t_PIDCID *ps_pidcid);


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
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UDINT IXCRC_PidCidCrcS5Calc(const CSS_t_PIDCID *ps_pidcid);
#endif


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
#if (    (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)  \
      || (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)        \
    )
  CSS_t_UINT IXCRC_PidRcCrcS3Calc(CSS_t_UINT u16_rcUsedInCrc,
                                  CSS_t_UINT u16_pidCrcS3);
#endif

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
                                 CSS_t_UDINT u32_pidCrcS5);
#endif


#endif /* #ifndef IXCRC_H */

/*** End Of File ***/


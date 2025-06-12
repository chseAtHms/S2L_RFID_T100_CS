/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRCs3.c
**    Summary: IXCRC - CRC computing routines
**             This module contains the calculation routines for the CRC-S3
**             presented in the example code provided in the CIP Networks
**             Library Volume 5 Appendix E-4. All safety CRCs are calculated
**             using this routines (see FRS32).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_BflMsgAcrcCalc
**             IXCRC_BflMsgCcrcCalc
**             IXCRC_BfTcooMsgCrcCalc
**             IXCRC_BfTcrMsgCrcCalc
**             IXCRC_EflMsgCrcS3Calc
**             IXCRC_CrcS3compute
**
**             CrcS3Ref1compute
**             CrcS3Ref2compute
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
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "CSSplatform.h"
  #include "CSSapi.h"
#endif
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

/** kau16_Crc16Table:
    CRCS3_POLYNOMIAL = 0x080F
    This table was generated via left shifting the MSB of each byte
*/
static const CSS_t_UINT kau16_Crc16Table[256] =
{
  0x0000U, 0x080FU, 0x101EU, 0x1811U, 0x203CU, 0x2833U, 0x3022U, 0x382DU,
  0x4078U, 0x4877U, 0x5066U, 0x5869U, 0x6044U, 0x684BU, 0x705AU, 0x7855U,
  0x80F0U, 0x88FFU, 0x90EEU, 0x98E1U, 0xA0CCU, 0xA8C3U, 0xB0D2U, 0xB8DDU,
  0xC088U, 0xC887U, 0xD096U, 0xD899U, 0xE0B4U, 0xE8BBU, 0xF0AAU, 0xF8A5U,
  0x09EFU, 0x01E0U, 0x19F1U, 0x11FEU, 0x29D3U, 0x21DCU, 0x39CDU, 0x31C2U,
  0x4997U, 0x4198U, 0x5989U, 0x5186U, 0x69ABU, 0x61A4U, 0x79B5U, 0x71BAU,
  0x891FU, 0x8110U, 0x9901U, 0x910EU, 0xA923U, 0xA12CU, 0xB93DU, 0xB132U,
  0xC967U, 0xC168U, 0xD979U, 0xD176U, 0xE95BU, 0xE154U, 0xF945U, 0xF14AU,
  0x13DEU, 0x1BD1U, 0x03C0U, 0x0BCFU, 0x33E2U, 0x3BEDU, 0x23FCU, 0x2BF3U,
  0x53A6U, 0x5BA9U, 0x43B8U, 0x4BB7U, 0x739AU, 0x7B95U, 0x6384U, 0x6B8BU,
  0x932EU, 0x9B21U, 0x8330U, 0x8B3FU, 0xB312U, 0xBB1DU, 0xA30CU, 0xAB03U,
  0xD356U, 0xDB59U, 0xC348U, 0xCB47U, 0xF36AU, 0xFB65U, 0xE374U, 0xEB7BU,
  0x1A31U, 0x123EU, 0x0A2FU, 0x0220U, 0x3A0DU, 0x3202U, 0x2A13U, 0x221CU,
  0x5A49U, 0x5246U, 0x4A57U, 0x4258U, 0x7A75U, 0x727AU, 0x6A6BU, 0x6264U,
  0x9AC1U, 0x92CEU, 0x8ADFU, 0x82D0U, 0xBAFDU, 0xB2F2U, 0xAAE3U, 0xA2ECU,
  0xDAB9U, 0xD2B6U, 0xCAA7U, 0xC2A8U, 0xFA85U, 0xF28AU, 0xEA9BU, 0xE294U,
  0x27BCU, 0x2FB3U, 0x37A2U, 0x3FADU, 0x0780U, 0x0F8FU, 0x179EU, 0x1F91U,
  0x67C4U, 0x6FCBU, 0x77DAU, 0x7FD5U, 0x47F8U, 0x4FF7U, 0x57E6U, 0x5FE9U,
  0xA74CU, 0xAF43U, 0xB752U, 0xBF5DU, 0x8770U, 0x8F7FU, 0x976EU, 0x9F61U,
  0xE734U, 0xEF3BU, 0xF72AU, 0xFF25U, 0xC708U, 0xCF07U, 0xD716U, 0xDF19U,
  0x2E53U, 0x265CU, 0x3E4DU, 0x3642U, 0x0E6FU, 0x0660U, 0x1E71U, 0x167EU,
  0x6E2BU, 0x6624U, 0x7E35U, 0x763AU, 0x4E17U, 0x4618U, 0x5E09U, 0x5606U,
  0xAEA3U, 0xA6ACU, 0xBEBDU, 0xB6B2U, 0x8E9FU, 0x8690U, 0x9E81U, 0x968EU,
  0xEEDBU, 0xE6D4U, 0xFEC5U, 0xF6CAU, 0xCEE7U, 0xC6E8U, 0xDEF9U, 0xD6F6U,
  0x3462U, 0x3C6DU, 0x247CU, 0x2C73U, 0x145EU, 0x1C51U, 0x0440U, 0x0C4FU,
  0x741AU, 0x7C15U, 0x6404U, 0x6C0BU, 0x5426U, 0x5C29U, 0x4438U, 0x4C37U,
  0xB492U, 0xBC9DU, 0xA48CU, 0xAC83U, 0x94AEU, 0x9CA1U, 0x84B0U, 0x8CBFU,
  0xF4EAU, 0xFCE5U, 0xE4F4U, 0xECFBU, 0xD4D6U, 0xDCD9U, 0xC4C8U, 0xCCC7U,
  0x3D8DU, 0x3582U, 0x2D93U, 0x259CU, 0x1DB1U, 0x15BEU, 0x0DAFU, 0x05A0U,
  0x7DF5U, 0x75FAU, 0x6DEBU, 0x65E4U, 0x5DC9U, 0x55C6U, 0x4DD7U, 0x45D8U,
  0xBD7DU, 0xB572U, 0xAD63U, 0xA56CU, 0x9D41U, 0x954EU, 0x8D5FU, 0x8550U,
  0xFD05U, 0xF50AU, 0xED1BU, 0xE514U, 0xDD39U, 0xD536U, 0xCD27U, 0xC528U
};


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  static CSS_t_UINT CrcS3Ref2compute(CSS_t_UINT u16_data,
                                     CSS_t_UINT u16_preset);
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)     \
      || (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)    \
    )
  static CSS_t_UINT CrcS3Ref1compute(CSS_t_USINT u8_data,
                                     CSS_t_UINT u16_preset);
#endif

/* provided by the example code but not used by the stack yet
  static CSS_t_UINT CrcS3Ref4compute(CSS_t_UDINT u32_data,
                                     CSS_t_UINT u16_preset);
*/



/*******************************************************************************
**    global functions
*******************************************************************************/

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
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
                                CSS_t_UINT u16_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UINT  u16_calcCrc = u16_seed;  /* PID (see FRS42-1) */
  /* mode byte must be manipulated before CRC calculation (see FRS37-2) */
  CSS_t_BYTE  b_tempModeByte = ps_bflMsg->b_modeByte & (0xE0U);

  /* calculate the CRC of the mode byte, using the seed (see FRS42-2) */
  u16_calcCrc = CrcS3Ref1compute(b_tempModeByte, u16_calcCrc);

  /* continue CRC calculation with Actual Data bytes (see FRS42-3) */
  u16_calcCrc = IXCRC_CrcS3compute(ps_bflMsg->ab_aData,
                                   (CSS_t_UINT)ps_bflMsg->u8_len,
                                   u16_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_calcCrc);
}


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
                                CSS_t_UINT u16_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UINT  u16_calcCrc = u16_seed;  /* PID (see FRS43-1) */
  /* mode byte must be manipulated before CRC calculation (see FRS37-3) */
  CSS_t_BYTE  b_tempModeByte = (ps_bflMsg->b_modeByte ^ 0xFFU) & (0xE0U);

  /* calculate the CRC of the XORed mode byte, using the seed (see FRS43-2) */
  u16_calcCrc = CrcS3Ref1compute(b_tempModeByte, u16_calcCrc);

  /* continue CRC calculation with Complemented Data bytes (see FRS43-3) */
  u16_calcCrc = IXCRC_CrcS3compute(ps_bflMsg->ab_cData,
                                   (CSS_t_UINT)ps_bflMsg->u8_len,
                                   u16_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_calcCrc);
}

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
                                  CSS_t_UINT u16_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UINT  u16_calcCrc = u16_seed;  /* CID (see FRS58-1) */

  /* calculate the CRC of the Ack_Byte, using the seed (see FRS58-2) */
  u16_calcCrc = CrcS3Ref1compute(ps_bfTcooMsg->b_ackByte, u16_calcCrc);

  /* continue CRC calculation with Consumer_Time_Value bytes (see FRS58-3) */
  u16_calcCrc = CrcS3Ref2compute(ps_bfTcooMsg->u16_consTimeVal_128us,
                                 u16_calcCrc);

  /* Ack_Byte_2 not included in CRC calculation (see FRS59) */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_calcCrc);
}


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
                                 CSS_t_UINT u16_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UINT  u16_calcCrc = u16_seed;  /* PID (see FRS68-1) */

  /* calculate the CRC of the MCast_Byte, using the seed (see FRS68-2) */
  u16_calcCrc = CrcS3Ref1compute(ps_bfTcrMsg->b_mCastByte, u16_calcCrc);

  /* continue CRC calculation with Time_Correction_Value bytes (see FRS68-3) */
  u16_calcCrc = CrcS3Ref2compute(ps_bfTcrMsg->u16_timeCorrVal_128us,
                                 u16_calcCrc);

  /* MCast_Byte_2 not included in CRC calculation (see FRS69) */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_calcCrc);
}
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
                                 CSS_t_UINT u16_seed)
{
  /* intermediate value for CRC calculations (PID and Rollover Count) */
  CSS_t_UINT  u16_calcCrc = u16_seed;  /* (see FRS368-1 and FRS368-2) */
  /* mode byte must be manipulated before CRC calculation (see FRS368) */
  CSS_t_BYTE  b_tempModeByte = ps_eflMsg->b_modeByte & (0xE0U);

  /* calculate the CRC of the mode byte, using the seed (see FRS368-3) */
  u16_calcCrc = CrcS3Ref1compute(b_tempModeByte, u16_calcCrc);

  /* continue CRC calculation with Actual Data bytes (see FRS368-4) */
  u16_calcCrc = IXCRC_CrcS3compute(ps_eflMsg->ab_aData,
                                   (CSS_t_UINT)ps_eflMsg->u8_len,
                                   u16_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_calcCrc);
}
#endif  /* (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) */
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXCRC_CrcS3compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               CRC16 calculation routine for polynomial 0x080F.
**               The kau16_Crc16Table was computed via left shift
**               operations and thus will do a (crc << 8).
**               Doing incremental crc calculation is done with providing the
**               result from the previous block with preset.
**
** Parameters  : pu8_start  (IN) - Starting address to compute CRC over
**                                 (not checked, only called with reference to
**                                 variables (sometimes with offset))
**               u16_len    (IN) - Number of bytes
**                                 (not checked, any value allowed)
**               u16_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Result of CRC calculation
**
*******************************************************************************/
CSS_t_UINT IXCRC_CrcS3compute(const CSS_t_USINT *pu8_start,
                              CSS_t_UINT u16_len,
                              CSS_t_UINT u16_preset)
{
  CSS_t_UINT u16_crc = u16_preset;
  const CSS_t_USINT *pu8_buf = pu8_start;

  while (u16_len-- != 0U)
  {
    CSS_t_USINT u8_data = *pu8_buf++;
    u16_crc =   (CSS_t_UINT)(kau16_Crc16Table[((u16_crc >> 8) ^ u8_data)]
              ^ (CSS_t_UINT)(u16_crc << 8));
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_crc);
}
/* original code from Volume 5:
unsigned short ComputeCRCS3(const void *pStart, int len, unsigned short preset)
{
unsigned short crc = preset;
unsigned char *buf = (unsigned char *)pStart;
while (len-- > 0)
{
unsigned char data = *buf++;
crc = CRC16Table[((crc >> 8) ^ data)] ^ (crc << 8);
}
return (crc);
}
*/


/*******************************************************************************
**    static functions
*******************************************************************************/

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)     \
      || (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)    \
    )
/*******************************************************************************
**
** Function    : CrcS3Ref1compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               Compute crc given 1 byte of data and a preset for the crc.
**
** Parameters  : u8_data    (IN) - Value to compute CRC on
**                                 (not checked, any value allowed)
**               u16_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Result of CRC calculation
**
*******************************************************************************/
static CSS_t_UINT CrcS3Ref1compute(CSS_t_USINT u8_data,
                                   CSS_t_UINT u16_preset)
{
  CSS_t_UINT u16_crc = u16_preset;

  u16_crc =   (CSS_t_UINT)(kau16_Crc16Table[(u16_crc >> 8) ^ u8_data]
            ^ (CSS_t_UINT)(u16_crc << 8));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return u16_crc;
}
/* original code from Volume 5:
unsigned short ComputeCRCS3Ref1(unsigned char data, unsigned short preset)
{
unsigned short crc = preset;
crc = CRC16Table[(crc >> 8) ^ data] ^ (crc << 8);
return crc;
}
*/
#endif


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : CrcS3Ref2compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               Compute crc given 2 bytes of data and a preset for the crc.
**               This assumes the input data is stored in native (little or big)
**               endian format.
**
** Parameters  : u16_data   (IN) - Value to compute CRC on
**                                 (not checked, any value allowed)
**               u16_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - Result of CRC calculation
**
*******************************************************************************/
static CSS_t_UINT CrcS3Ref2compute(CSS_t_UINT u16_data,
                                   CSS_t_UINT u16_preset)
{
  CSS_t_UINT u16_crc = u16_preset;

  u16_crc =   (CSS_t_UINT)(kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)(u16_data & 0xffU)]
                           ^ (CSS_t_UINT)(u16_crc << 8));

  u16_crc =   (CSS_t_UINT)(kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)((u16_data >> 8) & 0xffU)]
                           ^ (CSS_t_UINT)(u16_crc << 8));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return u16_crc;
}
/* original code from Volume 5:
unsigned short ComputeCRCS3Ref2(unsigned short data, unsigned short preset)
{
unsigned short crc = preset;
crc = CRC16Table[(crc >> 8) ^ (data&0xff)] ^ (crc << 8);
crc = CRC16Table[(crc >> 8) ^ ((data>>8)&0xff)] ^ (crc << 8);
return crc;
}
*/
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


/* provided by the example code but not used by the stack yet */
/*******************************************************************************
**
** Function    : CrcS3Ref4compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               Compute crc given 4 bytes of data and a preset for the crc.
**               This assumes the input data is stored in native (little or big)
**               endian format.
**
** Parameters  : u32_data   (IN) - Value to compute CRC on
**                                 (not checked, any value allowed)
**               u16_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UINT      - result of CRC calculation
**
*******************************************************************************
static CSS_t_UINT CrcS3Ref4compute(CSS_t_UDINT u32_data,
                                   CSS_t_UINT u16_preset)
{
  CSS_t_UINT u16_crc = u16_preset;

  u16_crc =   kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)(u32_data & 0xffU)]
            ^ (CSS_t_UINT)(u16_crc << 8);

  u16_crc =   kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)((u32_data >> 8) & 0xffU)]
            ^ (CSS_t_UINT)(u16_crc << 8);

  u16_crc =   kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)((u32_data >> 16) & 0xffU)]
            ^ (CSS_t_UINT)(u16_crc << 8);

  u16_crc =   kau16_Crc16Table[  (u16_crc >> 8)
                                     ^ (CSS_t_UINT)((u32_data >> 24) & 0xffU)]
            ^ (CSS_t_UINT)(u16_crc << 8);

  IXSCF_PATH_TRACK();

  return u16_crc;
}
*/
/* original code from Volume 5:
unsigned short ComputeCRCS3Ref4(unsigned long data, unsigned short preset)
{
unsigned short crc = preset;
crc = CRC16Table[(crc >> 8) ^ (data&0xff)] ^ (crc << 8);
crc = CRC16Table[(crc >> 8) ^ ((data>>8)&0xff)] ^ (crc << 8);
crc = CRC16Table[(crc >> 8) ^ ((data>>16)&0xff)] ^ (crc << 8);
crc = CRC16Table[(crc >> 8) ^ ((data>>24)&0xff)] ^ (crc << 8);
return crc;
}
*/


/*** End Of File ***/


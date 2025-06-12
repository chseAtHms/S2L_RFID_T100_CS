/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRCs1s2.c
**    Summary: IXCRC - CRC computing routines
**             This module contains the calculation routines for the CRC-S1 and
**             CRC-S2 presented in the example code provided in the CIP Networks
**             Library Volume 5 Appendix E-4. All safety CRCs are calculated
**             using this routines (see FRS32).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_BfsMsgAcrcCalc
**             IXCRC_BfsMsgCcrcCalc
**             IXCRC_BfMsgTsCrcCalc
**             IXCRC_CrcS1compute
**
**             IXCRC_CrcS2compute
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

/** kau8_Crc8Table37:
    CRCS1_POLYNOMIAL=0x37
    This table was generated via left shifting the MSB of each byte
*/
static const CSS_t_USINT kau8_Crc8Table37[256] =
{
  0x00U, 0x37U, 0x6eU, 0x59U, 0xdcU, 0xebU, 0xb2U, 0x85U,
  0x8fU, 0xb8U, 0xe1U, 0xd6U, 0x53U, 0x64U, 0x3dU, 0x0aU,
  0x29U, 0x1eU, 0x47U, 0x70U, 0xf5U, 0xc2U, 0x9bU, 0xacU,
  0xa6U, 0x91U, 0xc8U, 0xffU, 0x7aU, 0x4dU, 0x14U, 0x23U,
  0x52U, 0x65U, 0x3cU, 0x0bU, 0x8eU, 0xb9U, 0xe0U, 0xd7U,
  0xddU, 0xeaU, 0xb3U, 0x84U, 0x01U, 0x36U, 0x6fU, 0x58U,
  0x7bU, 0x4cU, 0x15U, 0x22U, 0xa7U, 0x90U, 0xc9U, 0xfeU,
  0xf4U, 0xc3U, 0x9aU, 0xadU, 0x28U, 0x1fU, 0x46U, 0x71U,
  0xa4U, 0x93U, 0xcaU, 0xfdU, 0x78U, 0x4fU, 0x16U, 0x21U,
  0x2bU, 0x1cU, 0x45U, 0x72U, 0xf7U, 0xc0U, 0x99U, 0xaeU,
  0x8dU, 0xbaU, 0xe3U, 0xd4U, 0x51U, 0x66U, 0x3fU, 0x08U,
  0x02U, 0x35U, 0x6cU, 0x5bU, 0xdeU, 0xe9U, 0xb0U, 0x87U,
  0xf6U, 0xc1U, 0x98U, 0xafU, 0x2aU, 0x1dU, 0x44U, 0x73U,
  0x79U, 0x4eU, 0x17U, 0x20U, 0xa5U, 0x92U, 0xcbU, 0xfcU,
  0xdfU, 0xe8U, 0xb1U, 0x86U, 0x03U, 0x34U, 0x6dU, 0x5aU,
  0x50U, 0x67U, 0x3eU, 0x09U, 0x8cU, 0xbbU, 0xe2U, 0xd5U,
  0x7fU, 0x48U, 0x11U, 0x26U, 0xa3U, 0x94U, 0xcdU, 0xfaU,
  0xf0U, 0xc7U, 0x9eU, 0xa9U, 0x2cU, 0x1bU, 0x42U, 0x75U,
  0x56U, 0x61U, 0x38U, 0x0fU, 0x8aU, 0xbdU, 0xe4U, 0xd3U,
  0xd9U, 0xeeU, 0xb7U, 0x80U, 0x05U, 0x32U, 0x6bU, 0x5cU,
  0x2dU, 0x1aU, 0x43U, 0x74U, 0xf1U, 0xc6U, 0x9fU, 0xa8U,
  0xa2U, 0x95U, 0xccU, 0xfbU, 0x7eU, 0x49U, 0x10U, 0x27U,
  0x04U, 0x33U, 0x6aU, 0x5dU, 0xd8U, 0xefU, 0xb6U, 0x81U,
  0x8bU, 0xbcU, 0xe5U, 0xd2U, 0x57U, 0x60U, 0x39U, 0x0eU,
  0xdbU, 0xecU, 0xb5U, 0x82U, 0x07U, 0x30U, 0x69U, 0x5eU,
  0x54U, 0x63U, 0x3aU, 0x0dU, 0x88U, 0xbfU, 0xe6U, 0xd1U,
  0xf2U, 0xc5U, 0x9cU, 0xabU, 0x2eU, 0x19U, 0x40U, 0x77U,
  0x7dU, 0x4aU, 0x13U, 0x24U, 0xa1U, 0x96U, 0xcfU, 0xf8U,
  0x89U, 0xbeU, 0xe7U, 0xd0U, 0x55U, 0x62U, 0x3bU, 0x0cU,
  0x06U, 0x31U, 0x68U, 0x5fU, 0xdaU, 0xedU, 0xb4U, 0x83U,
  0xa0U, 0x97U, 0xceU, 0xf9U, 0x7cU, 0x4bU, 0x12U, 0x25U,
  0x2fU, 0x18U, 0x41U, 0x76U, 0xf3U, 0xc4U, 0x9dU, 0xaaU,
};


#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/** kau8_Crc8Table3b:
    CRCS2_POLYNOMIAL = 0x3b
    This table was generated via left shifting the MSB of each byte
*/
static const CSS_t_USINT kau8_Crc8Table3b[256] =
{
  0x00U, 0x3bU, 0x76U, 0x4dU, 0xecU, 0xd7U, 0x9aU, 0xa1U,
  0xe3U, 0xd8U, 0x95U, 0xaeU, 0x0fU, 0x34U, 0x79U, 0x42U,
  0xfdU, 0xc6U, 0x8bU, 0xb0U, 0x11U, 0x2aU, 0x67U, 0x5cU,
  0x1eU, 0x25U, 0x68U, 0x53U, 0xf2U, 0xc9U, 0x84U, 0xbfU,
  0xc1U, 0xfaU, 0xb7U, 0x8cU, 0x2dU, 0x16U, 0x5bU, 0x60U,
  0x22U, 0x19U, 0x54U, 0x6fU, 0xceU, 0xf5U, 0xb8U, 0x83U,
  0x3cU, 0x07U, 0x4aU, 0x71U, 0xd0U, 0xebU, 0xa6U, 0x9dU,
  0xdfU, 0xe4U, 0xa9U, 0x92U, 0x33U, 0x08U, 0x45U, 0x7eU,
  0xb9U, 0x82U, 0xcfU, 0xf4U, 0x55U, 0x6eU, 0x23U, 0x18U,
  0x5aU, 0x61U, 0x2cU, 0x17U, 0xb6U, 0x8dU, 0xc0U, 0xfbU,
  0x44U, 0x7fU, 0x32U, 0x09U, 0xa8U, 0x93U, 0xdeU, 0xe5U,
  0xa7U, 0x9cU, 0xd1U, 0xeaU, 0x4bU, 0x70U, 0x3dU, 0x06U,
  0x78U, 0x43U, 0x0eU, 0x35U, 0x94U, 0xafU, 0xe2U, 0xd9U,
  0x9bU, 0xa0U, 0xedU, 0xd6U, 0x77U, 0x4cU, 0x01U, 0x3aU,
  0x85U, 0xbeU, 0xf3U, 0xc8U, 0x69U, 0x52U, 0x1fU, 0x24U,
  0x66U, 0x5dU, 0x10U, 0x2bU, 0x8aU, 0xb1U, 0xfcU, 0xc7U,
  0x49U, 0x72U, 0x3fU, 0x04U, 0xa5U, 0x9eU, 0xd3U, 0xe8U,
  0xaaU, 0x91U, 0xdcU, 0xe7U, 0x46U, 0x7dU, 0x30U, 0x0bU,
  0xb4U, 0x8fU, 0xc2U, 0xf9U, 0x58U, 0x63U, 0x2eU, 0x15U,
  0x57U, 0x6cU, 0x21U, 0x1aU, 0xbbU, 0x80U, 0xcdU, 0xf6U,
  0x88U, 0xb3U, 0xfeU, 0xc5U, 0x64U, 0x5fU, 0x12U, 0x29U,
  0x6bU, 0x50U, 0x1dU, 0x26U, 0x87U, 0xbcU, 0xf1U, 0xcaU,
  0x75U, 0x4eU, 0x03U, 0x38U, 0x99U, 0xa2U, 0xefU, 0xd4U,
  0x96U, 0xadU, 0xe0U, 0xdbU, 0x7aU, 0x41U, 0x0cU, 0x37U,
  0xf0U, 0xcbU, 0x86U, 0xbdU, 0x1cU, 0x27U, 0x6aU, 0x51U,
  0x13U, 0x28U, 0x65U, 0x5eU, 0xffU, 0xc4U, 0x89U, 0xb2U,
  0x0dU, 0x36U, 0x7bU, 0x40U, 0xe1U, 0xdaU, 0x97U, 0xacU,
  0xeeU, 0xd5U, 0x98U, 0xa3U, 0x02U, 0x39U, 0x74U, 0x4fU,
  0x31U, 0x0aU, 0x47U, 0x7cU, 0xddU, 0xe6U, 0xabU, 0x90U,
  0xd2U, 0xe9U, 0xa4U, 0x9fU, 0x3eU, 0x05U, 0x48U, 0x73U,
  0xccU, 0xf7U, 0xbaU, 0x81U, 0x20U, 0x1bU, 0x56U, 0x6dU,
  0x2fU, 0x14U, 0x59U, 0x62U, 0xc3U, 0xf8U, 0xb5U, 0x8eU,
};
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
  static CSS_t_USINT IXCRC_CrcS2compute(const CSS_t_USINT *pu8_start,
                                        CSS_t_UINT u16_len,
                                        CSS_t_USINT u8_preset);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

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
                                 CSS_t_USINT u8_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_USINT u8_calcCrc = u8_seed;  /* PID (see FRS39-1) */
  /* mode byte must be manipulated before CRC calculation (see FRS37-2) */
  CSS_t_BYTE  b_tempModeByte = (CSS_t_BYTE)(ps_bfsMsg->b_modeByte & (0xE0U));

  /* calculate the CRC of the mode byte, using the seed (see FRS39-2) */
  u8_calcCrc = IXCRC_CrcS1compute(&b_tempModeByte, 1U, u8_calcCrc);

  /* continue CRC calculation with Actual Data bytes (see FRS39-3) */
  u8_calcCrc = IXCRC_CrcS1compute(ps_bfsMsg->ab_aData,
                                  (CSS_t_UINT)ps_bfsMsg->u8_len,
                                  u8_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_calcCrc);
}


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
                                 CSS_t_USINT u8_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_USINT u8_calcCrc = u8_seed;  /* PID (see FRS40-1) */
  /* mode byte must be manipulated before CRC calculation (see FRS37-3) */
  CSS_t_BYTE  b_tempModeByte = (CSS_t_BYTE)((ps_bfsMsg->b_modeByte ^ 0xFFU)
                                            & (0xE0U));
  /* temporary storage for XORed data */
  CSS_t_BYTE  ab_cData[CSS_k_SHORT_FORMAT_MAX_LENGTH];

  /* store XORed data temporarily */
  ab_cData[0] = (CSS_t_BYTE)(ps_bfsMsg->ab_aData[0] ^ 0xFFU);
  ab_cData[1] = (CSS_t_BYTE)(ps_bfsMsg->ab_aData[1] ^ 0xFFU);

  /* calculate the CRC of the XORed mode byte, using the seed (see FRS40-2) */
  u8_calcCrc = IXCRC_CrcS2compute(&b_tempModeByte, 1U, u8_calcCrc);

  /* continue CRC calculation with XORed Actual Data bytes (see FRS40-3) */
  u8_calcCrc = IXCRC_CrcS2compute(ab_cData,
                                  (CSS_t_UINT)ps_bfsMsg->u8_len,
                                  u8_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_calcCrc);
}
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
                                 CSS_t_USINT u8_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_USINT u8_calcCrc = u8_seed;  /* PID (see FRS45-1) */
  /* mode byte must be manipulated before CRC calculation (see FRS37-4) */
  CSS_t_BYTE  b_tempModeByte = (CSS_t_BYTE)(ps_bfMsg->b_modeByte & (0x1FU));
  /* temporary storage for Time Stamp (in network byte order) */
  CSS_t_BYTE  ab_tData[2];

  /* store Time Stamp data in little endian */
  CSS_H2N_CPY16(ab_tData, &ps_bfMsg->u16_timeStamp_128us);

  /* calculate the CRC of the mode byte, using the seed (see FRS45-2) */
  u8_calcCrc = IXCRC_CrcS1compute(&b_tempModeByte, 1U, u8_calcCrc);

  /* continue CRC calculation with Time Stamp data bytes (see FRS45-3) */
  u8_calcCrc = IXCRC_CrcS1compute(ab_tData, 2U, u8_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_calcCrc);
}


/*******************************************************************************
**
** Function    : IXCRC_CrcS1compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               CRC8 calculation routine for polynomial 0x37.
**               Note that the kau8_Crc8Table37 was computed via left shift
**               operations.
**               Doing incremental crc calculation is done with providing the
**               result from the previous block with preset.
**
** Parameters  : pu8_start (IN) - Starting address to compute CRC over
**                                (not checked, only called with reference to
**                                variables (sometimes with offset))
**               u16_len   (IN) - Number of bytes
**                                (not checked, any value allowed)
**               u8_preset (IN) - Preset value for CRC
**                                (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT    - Result of CRC calculation
**
*******************************************************************************/
CSS_t_USINT IXCRC_CrcS1compute(const CSS_t_USINT *pu8_start,
                               CSS_t_UINT u16_len,
                               CSS_t_USINT u8_preset)
{
  CSS_t_USINT u8_crc = u8_preset;
  const CSS_t_USINT *pu8_buf = pu8_start;

  while (u16_len-- != 0U)
  {
    u8_crc = kau8_Crc8Table37[(u8_crc ^ *pu8_buf++)];
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_crc);
}
/* original code from Volume 5:
unsigned char ComputeCRCS1(const void *pStart, int len, unsigned char preset)
{
unsigned char crc = preset;
unsigned char *buf = (unsigned char *)pStart;
while (len-- > 0)
{
crc = CRC8Table37[(crc ^ *buf++)];
}
return (crc);
}
*/


/*******************************************************************************
**    static functions
*******************************************************************************/

#if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXCRC_CrcS2compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               CRC8 calculation routine for polynomial 0x3b.
**               Note that the kau8_Crc8Table3b was computed via left
**               shift operations.
**               Doing incremental crc calculation is done with providing the
**               result from the previous block with preset.
**
** Parameters  : pu8_start (IN) - Starting address to compute CRC over
**                                (not checked, only called with reference to
**                                variable)
**               u16_len   (IN) - Number of bytes
**                                (not checked, any value allowed)
**               u8_preset (IN) - Preset value for CRC
**                                (not checked, any value allowed)
**
** Returnvalue : CSS_t_USINT    - Result of CRC calculation
**
*******************************************************************************/
static CSS_t_USINT IXCRC_CrcS2compute(const CSS_t_USINT *pu8_start,
                                      CSS_t_UINT u16_len,
                                      CSS_t_USINT u8_preset)
{
  CSS_t_USINT u8_crc = u8_preset;
  const CSS_t_USINT *pu8_buf = pu8_start;

  while (u16_len-- != 0U)
  {
    u8_crc = kau8_Crc8Table3b[(u8_crc ^ *pu8_buf++)];
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_crc);
}
/* original code from Volume 5:
unsigned char ComputeCRCS2(const void *pStart, int len, unsigned char preset)
{
unsigned char crc = preset;
unsigned char *buf = (unsigned char *)pStart;
while (len-- > 0)
{
crc = CRC8Table3b[(crc ^ *buf++)];
}
return (crc);
}
*/
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


/*** End Of File ***/


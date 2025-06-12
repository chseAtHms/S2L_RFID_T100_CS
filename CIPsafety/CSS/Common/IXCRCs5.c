/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRCs5.c
**    Summary: IXCRC - CRC computing routines
**             This module contains the calculation routines for the CRC-S5
**             presented in the example code provided in the CIP Networks
**             Library Volume 5 Appendix E-4. All safety CRCs are calculated
**             using this routines (see FRS32).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_EfsMsgCrcCalc
**             IXCRC_EflMsgCcrcCalc
**             IXCRC_EfTcooMsgCrcCalc
**             IXCRC_EfTcrMsgCrcCalc
**             IXCRC_CrcS5compute
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

/** kau32_CrcS5Table:
    polynomial 0x5D6DCB
    This table was generated via left shifting the MSB of each byte and
    appending an additional byte to form a long word value. The MSB of the
    table value will always be zero
*/
static const CSS_t_UDINT kau32_CrcS5Table[256] =
{
  0x00000000U, 0x005d6dcbU, 0x00badb96U, 0x00e7b65dU,
  0x0028dae7U, 0x0075b72cU, 0x00920171U, 0x00cf6cbaU,
  0x0051b5ceU, 0x000cd805U, 0x00eb6e58U, 0x00b60393U,
  0x00796f29U, 0x002402e2U, 0x00c3b4bfU, 0x009ed974U,
  0x00a36b9cU, 0x00fe0657U, 0x0019b00aU, 0x0044ddc1U,
  0x008bb17bU, 0x00d6dcb0U, 0x00316aedU, 0x006c0726U,
  0x00f2de52U, 0x00afb399U, 0x004805c4U, 0x0015680fU,
  0x00da04b5U, 0x0087697eU, 0x0060df23U, 0x003db2e8U,
  0x001bbaf3U, 0x0046d738U, 0x00a16165U, 0x00fc0caeU,
  0x00336014U, 0x006e0ddfU, 0x0089bb82U, 0x00d4d649U,
  0x004a0f3dU, 0x001762f6U, 0x00f0d4abU, 0x00adb960U,
  0x0062d5daU, 0x003fb811U, 0x00d80e4cU, 0x00856387U,
  0x00b8d16fU, 0x00e5bca4U, 0x00020af9U, 0x005f6732U,
  0x00900b88U, 0x00cd6643U, 0x002ad01eU, 0x0077bdd5U,
  0x00e964a1U, 0x00b4096aU, 0x0053bf37U, 0x000ed2fcU,
  0x00c1be46U, 0x009cd38dU, 0x007b65d0U, 0x0026081bU,
  0x003775e6U, 0x006a182dU, 0x008dae70U, 0x00d0c3bbU,
  0x001faf01U, 0x0042c2caU, 0x00a57497U, 0x00f8195cU,
  0x0066c028U, 0x003bade3U, 0x00dc1bbeU, 0x00817675U,
  0x004e1acfU, 0x00137704U, 0x00f4c159U, 0x00a9ac92U,
  0x00941e7aU, 0x00c973b1U, 0x002ec5ecU, 0x0073a827U,
  0x00bcc49dU, 0x00e1a956U, 0x00061f0bU, 0x005b72c0U,
  0x00c5abb4U, 0x0098c67fU, 0x007f7022U, 0x00221de9U,
  0x00ed7153U, 0x00b01c98U, 0x0057aac5U, 0x000ac70eU,
  0x002ccf15U, 0x0071a2deU, 0x00961483U, 0x00cb7948U,
  0x000415f2U, 0x00597839U, 0x00bece64U, 0x00e3a3afU,
  0x007d7adbU, 0x00201710U, 0x00c7a14dU, 0x009acc86U,
  0x0055a03cU, 0x0008cdf7U, 0x00ef7baaU, 0x00b21661U,
  0x008fa489U, 0x00d2c942U, 0x00357f1fU, 0x006812d4U,
  0x00a77e6eU, 0x00fa13a5U, 0x001da5f8U, 0x0040c833U,
  0x00de1147U, 0x00837c8cU, 0x0064cad1U, 0x0039a71aU,
  0x00f6cba0U, 0x00aba66bU, 0x004c1036U, 0x00117dfdU,
  0x006eebccU, 0x00338607U, 0x00d4305aU, 0x00895d91U,
  0x0046312bU, 0x001b5ce0U, 0x00fceabdU, 0x00a18776U,
  0x003f5e02U, 0x006233c9U, 0x00858594U, 0x00d8e85fU,
  0x001784e5U, 0x004ae92eU, 0x00ad5f73U, 0x00f032b8U,
  0x00cd8050U, 0x0090ed9bU, 0x00775bc6U, 0x002a360dU,
  0x00e55ab7U, 0x00b8377cU, 0x005f8121U, 0x0002eceaU,
  0x009c359eU, 0x00c15855U, 0x0026ee08U, 0x007b83c3U,
  0x00b4ef79U, 0x00e982b2U, 0x000e34efU, 0x00535924U,
  0x0075513fU, 0x00283cf4U, 0x00cf8aa9U, 0x0092e762U,
  0x005d8bd8U, 0x0000e613U, 0x00e7504eU, 0x00ba3d85U,
  0x0024e4f1U, 0x0079893aU, 0x009e3f67U, 0x00c352acU,
  0x000c3e16U, 0x005153ddU, 0x00b6e580U, 0x00eb884bU,
  0x00d63aa3U, 0x008b5768U, 0x006ce135U, 0x00318cfeU,
  0x00fee044U, 0x00a38d8fU, 0x00443bd2U, 0x00195619U,
  0x00878f6dU, 0x00dae2a6U, 0x003d54fbU, 0x00603930U,
  0x00af558aU, 0x00f23841U, 0x00158e1cU, 0x0048e3d7U,
  0x00599e2aU, 0x0004f3e1U, 0x00e345bcU, 0x00be2877U,
  0x007144cdU, 0x002c2906U, 0x00cb9f5bU, 0x0096f290U,
  0x00082be4U, 0x0055462fU, 0x00b2f072U, 0x00ef9db9U,
  0x0020f103U, 0x007d9cc8U, 0x009a2a95U, 0x00c7475eU,
  0x00faf5b6U, 0x00a7987dU, 0x00402e20U, 0x001d43ebU,
  0x00d22f51U, 0x008f429aU, 0x0068f4c7U, 0x0035990cU,
  0x00ab4078U, 0x00f62db3U, 0x00119beeU, 0x004cf625U,
  0x00839a9fU, 0x00def754U, 0x00394109U, 0x00642cc2U,
  0x004224d9U, 0x001f4912U, 0x00f8ff4fU, 0x00a59284U,
  0x006afe3eU, 0x003793f5U, 0x00d025a8U, 0x008d4863U,
  0x00139117U, 0x004efcdcU, 0x00a94a81U, 0x00f4274aU,
  0x003b4bf0U, 0x0066263bU, 0x00819066U, 0x00dcfdadU,
  0x00e14f45U, 0x00bc228eU, 0x005b94d3U, 0x0006f918U,
  0x00c995a2U, 0x0094f869U, 0x00734e34U, 0x002e23ffU,
  0x00b0fa8bU, 0x00ed9740U, 0x000a211dU, 0x00574cd6U,
  0x0098206cU, 0x00c54da7U, 0x0022fbfaU, 0x007f9631U
};


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

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
                                CSS_t_UDINT u32_seed)
{
  /* intermediate value for CRC calculations (PID and Rollover Count) */
  CSS_t_UDINT u32_calcCrc = u32_seed;  /* (see FRS366-1 and FRS366-2) */
  /* mode byte must be manipulated before CRC calculation (see FRS366-3) */
  CSS_t_BYTE  b_tempModeByte = (CSS_t_BYTE)(ps_efsMsg->b_modeByte & (0xE0U));
  /* temporary storage for Time Stamp (in network byte order) */
  CSS_t_BYTE  ab_tData[2];

  /* store Time Stamp data in little endian */
  CSS_H2N_CPY16(ab_tData, &ps_efsMsg->u16_timeStamp_128us);

  /* calculate the CRC of the mode byte, using the seed (see FRS366-3) */
  u32_calcCrc = IXCRC_CrcS5compute(&b_tempModeByte, 1U, u32_calcCrc);

  /* continue CRC calculation with Actual Data bytes (see FRS366-4) */
  u32_calcCrc = IXCRC_CrcS5compute(ps_efsMsg->ab_aData,
                                   (CSS_t_UINT)ps_efsMsg->u8_len,
                                   u32_calcCrc);

  /* continue CRC calculation with Time Stamp data bytes (see FRS366-5) */
  u32_calcCrc = IXCRC_CrcS5compute(ab_tData, 2U, u32_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_calcCrc);
}
#endif  /* (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) */


#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
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
                                 CSS_t_UDINT u32_seed)
{
  /* intermediate value for CRC calculations (PID and Rollover Count) */
  CSS_t_UDINT u32_calcCrc = u32_seed; /* (see FRS369-1 and FRS369-2) */
  /* mode byte must be manipulated before CRC calculation (see FRS369-3) */
  CSS_t_BYTE  b_tempModeByte = ps_eflMsg->b_modeByte & (0x1FU);
  /* temporary storage for Time Stamp (in network byte order) */
  CSS_t_BYTE  ab_tData[2];

  /* store Time Stamp data in little endian */
  CSS_H2N_CPY16(ab_tData, &ps_eflMsg->u16_timeStamp_128us);

  /* calculate the CRC of the mode byte, using the seed (see FRS369-3) */
  u32_calcCrc = IXCRC_CrcS5compute(&b_tempModeByte, 1U, u32_calcCrc);

  /* continue CRC calculation with Complemented Data bytes (see FRS369-4) */
  u32_calcCrc = IXCRC_CrcS5compute(ps_eflMsg->ab_cData,
                                   (CSS_t_UINT)ps_eflMsg->u8_len,
                                   u32_calcCrc);

  /* continue CRC calculation with Time Stamp data bytes (see FRS369-5) */
  u32_calcCrc = IXCRC_CrcS5compute(ab_tData, 2U, u32_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_calcCrc);
}
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
                                   CSS_t_UDINT u32_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UDINT  u32_calcCrc = u32_seed;  /* CID (see FRS58-1) */
  /* temporary storage for Consumer_Time_Value (in network byte order) */
  CSS_t_BYTE  ab_tData[2];

  /* store Consumer_Time_Value data in little endian */
  CSS_H2N_CPY16(ab_tData, &ps_efTcooMsg->u16_consTimeVal_128us);

  /* calculate the CRC of the Ack_Byte, using the seed (see FRS58-2) */
  u32_calcCrc = IXCRC_CrcS5compute(&ps_efTcooMsg->b_ackByte, 1U, u32_calcCrc);

  /* continue CRC calculation with Consumer_Time_Value bytes (see FRS58-3) */
  u32_calcCrc = IXCRC_CrcS5compute(ab_tData, 2U, u32_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_calcCrc);
}


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
                                  CSS_t_UDINT u32_seed)
{
  /* intermediate value for CRC calculations */
  CSS_t_UDINT  u32_calcCrc = u32_seed;  /* PID (see FRS68-1) */
  /* temporary storage for Time_Correction_Value (in network byte order) */
  CSS_t_BYTE  ab_tData[2];

  /* store Time_Correction_Value data in little endian */
  CSS_H2N_CPY16(ab_tData, &ps_efTcrMsg->u16_timeCorrVal_128us);

  /* calculate the CRC of the MCast_Byte, using the seed (see FRS68-2) */
  u32_calcCrc = IXCRC_CrcS5compute(&ps_efTcrMsg->b_mCastByte, 1U, u32_calcCrc);

  /* continue CRC calculation with Time_Correction_Value bytes (see FRS68-3) */
  u32_calcCrc = IXCRC_CrcS5compute(ab_tData, 2U, u32_calcCrc);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_calcCrc);
}


/*******************************************************************************
**
** Function    : IXCRC_CrcS5compute
**
** Description : This function is based on example code provided in the CIP
**               Networks Library Volume 5 Appendix E-4.
**               CRCS5 calculation routine for polynomial 0x5D6DCB.
**               The kau32_CrcS5Table was computed via left shift
**               operations and thus will do a (crc << 8).
**               Doing incremental crc calculation is done with providing the
**               result from the previous block with preset.
**
** Parameters  : pu8_start (IN)  - Starting address to compute CRC over
**                                 (not checked, only called with reference to
**                                 variable)
**               u16_len    (IN) - Number of bytes
**                                 (not checked, any value allowed)
**               u32_preset (IN) - Preset value for CRC
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_t_UDINT     - Result of CRC calculation (with the MSB = 0)
**
*******************************************************************************/
CSS_t_UDINT IXCRC_CrcS5compute(const CSS_t_USINT *pu8_start,
                               CSS_t_UINT u16_len,
                               CSS_t_UDINT u32_preset)
{
  CSS_t_UDINT u32_crc = u32_preset;
  /* unsigned long temp; */
  const CSS_t_USINT *pu8_buf = pu8_start;

  while (u16_len-- != 0U)
  {
    CSS_t_USINT u8_data = *pu8_buf++;
    /* XOR data with CRC2, look up result, then XOR that with CRC; */
    u32_crc = kau32_CrcS5Table[((u32_crc >> 16) ^ u8_data) & 0xffU]
              ^ (CSS_t_UDINT)(u32_crc << 8);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_crc & 0x00ffffffU);
}
/* original code from Volume 5:
unsigned long ComputeCRCS24(const void *pStart, int len, unsigned long preset)
{
unsigned long crc = preset;
unsigned long temp;
unsigned char *buf = (unsigned char *)pStart;
while (len-- > 0)
{
unsigned char data = *buf++;
// XOR data with CRC2, look up result, then XOR that with CRC;
crc = CRCS5Table[((crc >> 16) ^ data)&0xff] ^ (crc << 8);
}
return (crc & 0x00ffffff);
}
*/


/*******************************************************************************
**    static functions
*******************************************************************************/

#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


/*** End Of File ***/



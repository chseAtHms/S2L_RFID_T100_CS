/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXCRCint.h
**    Summary: IXCRC - CRC computing routines
**             IXCRCint.h is the internal header file of the IXCRC unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXCRC_CrcS1compute
**             IXCRC_CrcS3compute
**             IXCRC_CrcS5compute
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXCRCINT_H
#define IXCRCINT_H


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
                               CSS_t_USINT u8_preset);
#endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */


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
                              CSS_t_UINT u16_preset);


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
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
                               CSS_t_UDINT u32_preset);
#endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */


#endif /* #ifndef IXCRCINT_H */

/*** End Of File ***/


/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgHalc.h
**     Summary: Header contains common HALC defines
**   $Revision: 2448 $
**       $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: -
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_HALC_H
#define AICMSG_HALC_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* This define determines the AIC Message Header Length used if an HALC Command 
** is transferred. In contrast to the AIC Message Header specified in [SIS], this 
** header contains a 16 Bit length field.
*/
#define AICMSGHALC_k_AIC_HDR_LENGTH              (UINT8)sizeof(AICMSGHALC_t_AICHDR)

/* This define determines the HALC Header Length used if an HALC Command 
** is transferred. The header length consist of the HALC Command (16 bit) and 
** the HALC addInfo (32 Bit) (see [SIS_CS]) */
#define AICMSGHALC_k_HALC_HDR_LENGTH             (UINT8)sizeof(AICMSGHALC_t_HALCHDR)

/** AICMSGHDL_k_HALC_FST_FRAG_DATA_SIZE
** Size of HALC data payload usable in the first AM/ASM fragment
*/
#define AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE     (UINT8)((sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT) - \
                                                   AICMSGHALC_k_AIC_HDR_LENGTH) - \
                                                   AICMSGHALC_k_HALC_HDR_LENGTH)
/** AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS
** Offset of the HALC data payload inside the first AM/ASM fragment
**
*/
#define AICMSGHALC_k_HALC_FST_FRAG_DATA_OFS      (UINT8)(sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT) - \
                                                   AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE)

/***************************************************************************************************
**    data types
***************************************************************************************************/

/* AIC Message Header for HALC Messages */
typedef __packed struct
{
  /* To keep track of requests and their corresponding response. */
  UINT8  u8_msgId;
  /* Describes what is contained in the data part of the message. */
  UINT16 u16_msgReqResp;
  /* Length of HALC message header and HALC data ( 6 + n bytes) */
  UINT16 u16_msgLength;
} AICMSGHALC_t_AICHDR;


/* HALC Message Header */
typedef __packed struct
{
  /* HALC command identifier */
  UINT16 u16_halcCmd;
  /* Additional information field of the HALC message */
  UINT32 u32_halcAddInfo;
} AICMSGHALC_t_HALCHDR;

/*lint -esym(768, au8_halcData) */
/* Lint message 768 is deactivated for structure member. It is defined for HALC Message definition,
** but not accessed over this structure element. */
/* HALC Message */
typedef __packed struct
{
  /* Header of the AIC message */
  AICMSGHALC_t_AICHDR  s_aicHdr;
  /* Header of the HALC message */
  AICMSGHALC_t_HALCHDR s_halcHdr; 
  /* Data of the HALC message */
  UINT8 au8_halcData[AICMSGHALC_k_HALC_FST_FRAG_DATA_SIZE];
} AICMSGHALC_t_MSG;


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


#endif


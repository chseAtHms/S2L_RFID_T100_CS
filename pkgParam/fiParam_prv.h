/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** fiParam_prv.h
**
** $Id: fiParam_prv.h 2242 2017-02-27 15:24:29Z klan $
** $Revision: 2242 $
** $Date: 2017-02-27 16:24:29 +0100 (Mo, 27 Feb 2017) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** local include file of "fiParam.c", contains static and file wide definitions
** which are not public
**
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 und STYL-2-3-20.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef FIPARAM_PRV_H_
#define FIPARAM_PRV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! This constant defines the size of the IPAR structure.*/
#define FIPARAM_SIZEOF_IPAR_STRUCT  ((UINT16)sizeof(PARAMETER_IPAR_STRUCT))

/*! This constant defines the size of the major SW version Byte*/
#define FIPARAM_SIZEOF_SW_MAJOR_VER  ((UINT16)0x01u)

/*! This constant defines the size of the HW-ID */
#define FIPARAM_SIZEOF_HW_ID         ((UINT16)0x02u)

/* This constant defines the seed for the CRC calculation.
** SCCRC is a CRC-S4 (32-bit) */
#define FIPARAM_k_CRC_SEED_VALUE     ((UINT32)0xffffffffu)

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Declaration Static Services
**
********************************************************************************
*/
STATIC UINT32 CrcS4compute(const volatile UINT8 *pu8_start, UINT16 u16_len, UINT32 u32_preset);

#endif  /* inclusion lock */


/*******************************************************************************
**
** End of fiParam_prv.h
**
********************************************************************************
*/



/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** adc_AnalogComparator-hal_loc.h
**
** $Id: adc_AnalogComparator-hal_loc.h 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** Local header file of the Analog Comparator module.
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
**
**
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/


#ifndef ADCHAL_LOC_H
#define ADCHAL_LOC_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! Properties of ADC */
/*! Number of ADC channels available on Controller */
#define ADCHALLOC_NUMBER_OF_CHANNELS   ((UINT8)18u)

/*! Constant for conversion-timeout, value is in cycles. */
#define ADCHALLOC_CONVERSION_TIMEOUT   ((UINT32)100000u)
/*! Constant for calibration-timeout, value is in cycles. */
#define ADCHALLOC_CALIBRATION_TIMEOUT  ((UINT32)2u)


/*! Macro to generate bit definition for register SMPR1 */
/*! Bit-definition for sample time of channel 10 is written to bit 0 to 2 */
#define  ADCHALLOC_SMP10(x) (((UINT32)x << 0u) & 0x00000007u)
/*! Bit-definition for sample time of channel 11 is written to bit 3 to 5 */
#define  ADCHALLOC_SMP11(x) (((UINT32)x << 3u) & 0x00000038u)
/*! Bit-definition for sample time of channel 12 is written to bit 6 to 8 */
#define  ADCHALLOC_SMP12(x) (((UINT32)x << 6u) & 0x000001C0u)
/*! Bit-definition for sample time of channel 13 is written to bit 9 to 11 */
#define  ADCHALLOC_SMP13(x) (((UINT32)x << 9u) & 0x00000E00u)
/*! Bit-definition for sample time of channel 14 is written to bit 12 to 14 */
#define  ADCHALLOC_SMP14(x) (((UINT32)x << 12u) & 0x00007000u)
/*! Bit-definition for sample time of channel 15 is written to bit 15 to 17 */
#define  ADCHALLOC_SMP15(x) (((UINT32)x << 15u) & 0x00038000u)
/*! Bit-definition for sample time of channel 16 is written to bit 18 to 20 */
#define  ADCHALLOC_SMP16(x) (((UINT32)x << 18u) & 0x001C0000u)
/*! Bit-definition for sample time of channel 17 is written to bit 21 to 23 */
#define  ADCHALLOC_SMP17(x) (((UINT32)x << 21u) & 0x00E00000u)

/*! Macro to generate bit definition for register SMPR2 */
/*! Bit-definition for sample time of channel 0 is written to bit 0 to 2 */
#define  ADCHALLOC_SMP0(x) (((UINT32)x << 0u) & 0x00000007u)
/*! Bit-definition for sample time of channel 1 is written to bit 3 to 5 */
#define  ADCHALLOC_SMP1(x) (((UINT32)x << 3u) & 0x00000038u)
/*! Bit-definition for sample time of channel 2 is written to bit 6 to 8 */
#define  ADCHALLOC_SMP2(x) (((UINT32)x << 6u) & 0x000001C0u)
/*! Bit-definition for sample time of channel 3 is written to bit 9 to 11 */
#define  ADCHALLOC_SMP3(x) (((UINT32)x << 9u) & 0x00000E00u)
/*! Bit-definition for sample time of channel 4 is written to bit 12 to 14 */
#define  ADCHALLOC_SMP4(x) (((UINT32)x << 12u) & 0x00007000u)
/*! Bit-definition for sample time of channel 5 is written to bit 15 to 17 */
#define  ADCHALLOC_SMP5(x) (((UINT32)x << 15u) & 0x00038000u)
/*! Bit-definition for sample time of channel 6 is written to bit 18 to 20 */
#define  ADCHALLOC_SMP6(x) (((UINT32)x << 18u) & 0x001C0000u)
/*! Bit-definition for sample time of channel 7 is written to bit 21 to 23 */
#define  ADCHALLOC_SMP7(x) (((UINT32)x << 21u) & 0x00E00000u)
/*! Bit-definition for sample time of channel 8 is written to bit 24 to 26 */
#define  ADCHALLOC_SMP8(x) (((UINT32)x << 24u) & 0x07000000u)
/*! Bit-definition for sample time of channel 9 is written to bit 27 to 29 */
#define  ADCHALLOC_SMP9(x) (((UINT32)x << 27u) & 0x38000000u)

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


#endif /* ADCHAL_LOC_H */

/*******************************************************************************
**
** End of adc_AnalogComparator-hal_loc.h
**
********************************************************************************
*/

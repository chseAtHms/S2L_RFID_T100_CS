/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** adc_AnalogComparator-hal.h
**
** $Id: adc_AnalogComparator-hal.h 4140 2023-12-07 09:33:41Z ankr $
** $Revision: 4140 $
** $Date: 2023-12-07 10:33:41 +0100 (Do, 07 Dez 2023) $
** $Author: ankr $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** This module is the Hardware Abstraction Layer for the ADC.
** This module handles the analog-digital converter. It provides functions to
** initialize the ADC and to read a current value from an ADC-channel.
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

#ifndef ADCHAL_H
#define ADCHAL_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/

#include "adc_AnalogComparator-hal_cfg.h"


#if (CFG_TEMPERATURE_SENSOR_DIGITAL != TRUE)

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/* Initialization function is called via this macro for fulfilling full code coverage.
   Integration/Acceptance testing will prove that the this macro calls the right function. */
#define TEMPERATURE_HAL_INIT()      adcHAL_Init()


   
/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*lint -esym(714, ADCHAL_CHANNEL_ENUM_TAG) */
/*! Identifiers for ADC-Channels (no use of hamming distance here,
 * enum-value has to be equivalent to channel number) */
typedef enum ADCHAL_CHANNEL_ENUM_TAG
{
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
   ADCHAL_CH0        = 0u,    /*!< Identifier for ADC-Channel 0 */
   ADCHAL_CH1        = 1u,    /*!< Identifier for ADC-Channel 1 */
   ADCHAL_CH2        = 2u,    /*!< Identifier for ADC-Channel 2 */
   ADCHAL_CH3        = 3u,    /*!< Identifier for ADC-Channel 3 */
   ADCHAL_CH4        = 4u,    /*!< Identifier for ADC-Channel 4 */
   ADCHAL_CH5        = 5u,    /*!< Identifier for ADC-Channel 5 */
   ADCHAL_CH6        = 6u,    /*!< Identifier for ADC-Channel 6 */
   ADCHAL_CH7        = 7u,    /*!< Identifier for ADC-Channel 7 */
   ADCHAL_CH8        = 8u,    /*!< Identifier for ADC-Channel 8 */
   ADCHAL_CH9        = 9u,    /*!< Identifier for ADC-Channel 9 */
   ADCHAL_CH10       = 10u,   /*!< Identifier for ADC-Channel 10 */
   ADCHAL_CH11       = 11u,   /*!< Identifier for ADC-Channel 11 */
   ADCHAL_CH12       = 12u,   /*!< Identifier for ADC-Channel 12 */
   ADCHAL_CH13       = 13u,   /*!< Identifier for ADC-Channel 13 */
   ADCHAL_CH14       = 14u,   /*!< Identifier for ADC-Channel 14 */
   ADCHAL_CH15       = 15u,   /*!< Identifier for ADC-Channel 15 */
   ADCHAL_CH16       = 16u,   /*!< Identifier for ADC-Channel 16 */
   ADCHAL_CH17       = 17u,   /*!< Identifier for ADC-Channel 17 */
   ADCHAL_CH_INVALID = 0xFFu  /*!< Identifier for an invalid ADC-Channel */
/* RSM_IGNORE_QUALITY_END */
} ADCHAL_CHANNEL_ENUM;

/*! Identifiers for sample time (no use of hamming distance here)*/
typedef enum ADCHAL_SAMPLE_TIME_ENUM_TAG
{
   ADCHAL_SAMPLE_TIME_1CYCLE5    =  0x00u,   /*!< Identifier for sample time: 1.5 cycles*/
   ADCHAL_SAMPLE_TIME_7CYCLE5    =  0x01u,   /*!< Identifier for sample time: 7.5 cycles*/
   ADCHAL_SAMPLE_TIME_13CYCLE5   =  0x02u,   /*!< Identifier for sample time: 13.5 cycles*/
   ADCHAL_SAMPLE_TIME_28CYCLE5   =  0x03u,   /*!< Identifier for sample time: 28.5 cycles*/
   ADCHAL_SAMPLE_TIME_41CYCLE5   =  0x04u,   /*!< Identifier for sample time: 41.5 cycles*/
   ADCHAL_SAMPLE_TIME_55CYCLE5   =  0x05u,   /*!< Identifier for sample time: 55.5 cycles*/
   ADCHAL_SAMPLE_TIME_71CYCLE5   =  0x06u,   /*!< Identifier for sample time: 71.5 cycles*/
   ADCHAL_SAMPLE_TIME_239CYCLE5  =  0x07u    /*!< Identifier for sample time: 239.5 cycles*/
} ADCHAL_SAMPLE_TIME_ENUM;

/*! Identifiers for ADC-prescaler values */
typedef enum ADCHAL_CLK_PRESCALER_ENUM_TAG
{
   ADCHAL_CLK_PRESCALER_DIV2  =  RCC_CFGR_ADCPRE_DIV2, /*!< Ident. for ADC-prescaler: divide by 2 */
   ADCHAL_CLK_PRESCALER_DIV4  =  RCC_CFGR_ADCPRE_DIV4, /*!< Ident. for ADC-prescaler: divide by 4 */
   ADCHAL_CLK_PRESCALER_DIV6  =  RCC_CFGR_ADCPRE_DIV6, /*!< Ident. for ADC-prescaler: divide by 6 */
   ADCHAL_CLK_PRESCALER_DIV8  =  RCC_CFGR_ADCPRE_DIV8  /*!< Ident. for ADC-prescaler: divide by 8 */
} ADCHAL_CLK_PRESCALER_ENUM;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void adcHAL_Init (void);
extern UINT16 adcHAL_Read (CONST ADCHAL_CHANNEL_ENUM eAdcChannel, CONST UINT8 u8Samples);

#endif  /* (CFG_TEMPERATURE_SENSOR_DIGITAL != TRUE) */

#endif /* ADCHAL_H */

/*******************************************************************************
**
** End of adc_AnalogComparator-hal.h
**
********************************************************************************
*/

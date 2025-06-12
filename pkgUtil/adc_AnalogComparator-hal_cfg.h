/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** adc_AnalogComparator-hal_cfg.h
**
** $Id: adc_AnalogComparator-hal_cfg.h 4086 2023-09-27 15:49:53Z ankr $
** $Revision: 4086 $
** $Date: 2023-09-27 17:49:53 +0200 (Mi, 27 Sep 2023) $
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
** Configuration header of the Analog Comparator module.
** This is the configuration header. It contains all configurations which
** are used in the module, e.g. constant definitions for array sizes.
** For further information see VA_C_Programmierstandard rule STYL-2-3-20 and
** LIB-0-0-10.
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

#ifndef ADCHAL_CFG_H
#define ADCHAL_CFG_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! Configuration of ADC-Prescaler */
/*! ADC-Clock = PCLK2 / 6 */
/*lint -esym(755, ADCHALCFG_ADCPRE_VALUE) */
#define  ADCHALCFG_ADCPRE_VALUE        ((ADCHAL_CLK_PRESCALER_ENUM)ADCHAL_CLK_PRESCALER_DIV6)

/*! Configuration of sample time for every ADC-channel */

/*! sample time for ADC-Channel 0: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH0     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 1: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH1     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 2: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH2     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 3: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH3     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 4: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH4     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 5: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH5     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 6: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH6     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 7: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH7     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 8: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH8     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 9: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH9     ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 10: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH10    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 11: 71.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH11    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_71CYCLE5)
/*! sample time for ADC-Channel 12: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH12    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 13: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH13    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 14: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH14    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 15: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH15    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 16: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH16    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)
/*! sample time for ADC-Channel 17: 41.5 cycles */
#define  ADCHALCFG_SAMPLE_TIME_CH17    ((ADCHAL_SAMPLE_TIME_ENUM)ADCHAL_SAMPLE_TIME_41CYCLE5)

/*! Maximum allowed number of samples converted by one single read operation */
/*lint -esym(755, ADCHALCFG_MAX_NUMBER_OF_SAMPLES) */
#define ADCHALCFG_MAX_NUMBER_OF_SAMPLES   ((UINT8)10)

/*! Reference voltage of the controller
 *  Here: Vref 2.5V  */
#define ADC_REFERENCE_VOLTAGE       2.5f
/*! Resolution of the ADC -> 2^12bit */
#define ADC_RESOLUTION              4096u

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/



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

#endif   /* <ADCHAL_CFG_H> */

/*******************************************************************************
**
** End of adc_AnalogComparator-hal_cfg.h
**
********************************************************************************
*/

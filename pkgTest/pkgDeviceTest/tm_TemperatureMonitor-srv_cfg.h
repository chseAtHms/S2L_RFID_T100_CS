/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** tm_TemperatureMonitor-srv_cfg.h
**
** $Id: tm_TemperatureMonitor-srv_cfg.h 4086 2023-09-27 15:49:53Z ankr $
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
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef  TMSRV_CFG_H
#define  TMSRV_CFG_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
/* depending on configuration not all symbols are used */
/*lint -esym(755, TMSRV_SENSOR_ADCCH) */
/*lint -esym(755, TMSRV_ADC_READ_CNT) */


/* IO configuration for temperature sensors */

/*! IO port for temperature sensor */
#define TMSRV_SENSOR_PORT           (GPIOC)
/*! IO pin for temperature sensor */
#define TMSRV_SENSOR_PIN            GPIO_1
/*! ADC channel for temperature sensor */
#define TMSRV_SENSOR_ADCCH          ADCHAL_CH11

/* ADC configurations */
/*! number of samples per read operation */
#define TMSRV_ADC_READ_CNT          ((UINT8)3u)
/*! ADC value for threshold of overheating, see [SRS_267] */
#define TMSRV_TEMP_VAL_HIGH_LEVEL   ((UINT16) (((TMSRV_V_OVERHEAT * (REAL)ADC_RESOLUTION) \
                                                / ADC_REFERENCE_VOLTAGE)) )
/*! ADC value for threshold of underheating, see [SRS_267]
 * ("+1" is added to round the value always up) */
#define TMSRV_TEMP_VAL_LOW_LEVEL    ((UINT16) ((((TMSRV_V_UNDERHEAT * (REAL)ADC_RESOLUTION) \
                                                / ADC_REFERENCE_VOLTAGE)) + 1) )

/* Voltage definitions for temperature measure */
/*! Voltage threshold for overheating, unit is volt */
#define TMSRV_V_OVERHEAT            1.3f
/*! Voltage threshold for underheating, unit is volt */
#define TMSRV_V_UNDERHEAT           0.2f

/* configuration of test-filter */
/*! Init-value of result-bytes */
#define TMSRV_RESULT_BYTE_INIT      {{(UINT8)0u},{(UINT8)~(UINT8)0u}}
/*! filter size of test-filter */
#define TMSRV_TEST_FILTER_SIZE      5u
/*! threshold of test-filter */
#define TMSRV_TEST_THRESHOLD        3u

/*! Plausibility check of the filter size. It is used for a shift operation.
 *  With this statement it is ensured that the value is not set greater than 7
 *  to avoid a buffer overrun. */
#if (TMSRV_TEST_FILTER_SIZE > 7u)
#error "Value for the test filter size is not in range, it must be 7 or less. "
#endif

/*! Plausibility check of the test threshold and the filter size. The threshold
 * should be equal or less than the filter size to ensure that a temperature
 * error is identified. */
#if (TMSRV_TEST_THRESHOLD > TMSRV_TEST_FILTER_SIZE)
#error "The value of the test threshold should be equal or less than the filter size."
#endif


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

#endif   /* <TMSRV_CFG_H> */

/*******************************************************************************
**
** End of tm_TemperatureMonitor-srv_cfg.h
**
********************************************************************************
*/

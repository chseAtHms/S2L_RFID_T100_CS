/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** adc_AnalogComparator-hal.c
**
** $Id: adc_AnalogComparator-hal.c 4086 2023-09-27 15:49:53Z ankr $
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
** This module is the Hardware Abstraction Layer for the ADC.
** This module handles the analog-digital converter. It provides functions to
** initialize the ADC and to read a current value from an ADC-channel.
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

/*******************************************************************************
**
** includes
**
********************************************************************************
*/
/* Project header */
#include "xtypes.h"
#include "cfg_Config-sys.h"

#if (CFG_TEMPERATURE_SENSOR_DIGITAL != TRUE)


#include "stm32f10x.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* Header-file of module */
#include "adc_AnalogComparator-hal.h"
#include "adc_AnalogComparator-hal_loc.h"

/*******************************************************************************
**
** Public Globals
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
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** adcHAL_Init()
**
** Description:
** This function initializes the ADC according to the _cfg.h-File.
** More precisely it activates the ADC-Clock, configures the ADC, starts it and
** calibrates it.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Register Setting:
**    RCC->CFGR
**    RCC->APB2ENR
**    ADC1->SMPR1
**    ADC1->SMPR2
**    ADC1->CR1
**    ADC1->CR2
**    ADC1->SR
**
** Return:
**    - void -
**
** Usage:
**    - None -
**
** Module Test:
**    - NO -
**    Reason: low complexity, predominantly register accesses
**------------------------------------------------------------------------------
*/
void adcHAL_Init (void)
{
   /********** local variables **********/
   UINT32 u32SMPR1Value;
   UINT32 u32Timeout;

   /********** configure ADC-Clock **********/
   RCC->CFGR |= (UINT32)ADCHALCFG_ADCPRE_VALUE;/*lint !e835*/
   /* TSL: Lint message 835 deactivated because register bit definition is
    *      zero. */

   /********** clock enable **********/
   RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

   /********** configure sample time for channels **********/
   u32SMPR1Value = ADCHALLOC_SMP11(ADCHALCFG_SAMPLE_TIME_CH11);

   ADC1->SMPR1 = u32SMPR1Value;
   ADC1->SMPR2 = 0u;

   /********** configure CR1 **********/
   ADC1->CR1 = (UINT32) 0x0u;

   /********** start ADC **********/
   ADC1->CR2 = ADC_CR2_ADON;

   /********** reset calibration **********/
   ADC1->CR2 |= ADC_CR2_RSTCAL;
   u32Timeout = 0u;
   while ( ( ADC1->SR & ADC_CR2_RSTCAL ) )
   {
      /* check if a timeout achieved */
      u32Timeout++;
      if (u32Timeout >= ADCHALLOC_CALIBRATION_TIMEOUT)
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_ADC_CALIB_TIMEOUT);
      }
   }

   /********** start calibration **********/
   ADC1->CR2 |= ADC_CR2_CAL;
   u32Timeout = 0u;
   while ( ( ADC1->SR & ADC_CR2_CAL ) )
   {
      /* check if a timeout achieved */
      u32Timeout++;
      if (u32Timeout >= ADCHALLOC_CALIBRATION_TIMEOUT)
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_ADC_CALIB_TIMEOUT);
      }
   }
}


/*------------------------------------------------------------------------------
**
** adcHAL_Read()
**
** Description:
** This function reads one Channel of the ADC multiple times and returns the
** average of the ADC-Values.
**
**------------------------------------------------------------------------------
** Inputs:
**     eAdcChannel               - Channel Selection (0..17)
**     u8Samples                 - Number of samples in one read operation
**
** Register Setting:
**    ADC1->SQR3
**    ADC1->CR2
**
** Return:
**    UINT16                     - Average ADC-Value
**
** Usage:
**    - None -
**
** Module Test:
**    - YES -
**------------------------------------------------------------------------------
*/
UINT16 adcHAL_Read ( CONST ADCHAL_CHANNEL_ENUM eAdcChannel,
                     CONST UINT8               u8Samples )
{
   UINT16 u16RetAvgAdcValue;
   UINT8  u8SampleCounter;
   UINT32 u32AdcSum  = 0u;
   UINT32 u32Timeout = 0u;

   /* test arguments on entry */
   GLOBFAIL_SAFETY_ASSERT_RET((UINT8)eAdcChannel < ADCHALLOC_NUMBER_OF_CHANNELS,
                              GLOB_FAILCODE_INVALID_PARAM, 0u);
   GLOBFAIL_SAFETY_ASSERT_RET(
      ((0u != u8Samples) && (ADCHALCFG_MAX_NUMBER_OF_SAMPLES >= u8Samples)),
      GLOB_FAILCODE_INVALID_PARAM, 0u);

   /**********                channel selection                      **********/
   ADC1->SQR3 = (ADC_SQR3_SQ1 & (UINT32) eAdcChannel);

   /**********                conversion                             **********/
   for (u8SampleCounter = 0u; u8SampleCounter < u8Samples; u8SampleCounter++)
   {
      ADC1->CR2 |= ADC_CR2_ADON; /* start conversion */
      /* wait until conversion is complete */
      while (0u == (ADC1->SR & ADC_SR_EOC))
      {
         /* check if a timeout achieved */
         u32Timeout++;
         if (u32Timeout >= ADCHALLOC_CONVERSION_TIMEOUT)
         {
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_ADC_CONV_TIMEOUT);
         }
      }
      /* summate ADC-values */
      u32AdcSum += ((ADC1->DR) & ADC_DR_DATA);
   }

   /**********              calculate average                        **********/
   /* u8Samples has been checked for zero-value on function entry */
   u16RetAvgAdcValue = (UINT16)(u32AdcSum / u8Samples); /*lint !e795 */

   return u16RetAvgAdcValue;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


#endif  /* (CFG_TEMPERATURE_SENSOR_DIGITAL != TRUE) */

/*******************************************************************************
**
** End of adc_AnalogComparator-hal.c
**
********************************************************************************
*/

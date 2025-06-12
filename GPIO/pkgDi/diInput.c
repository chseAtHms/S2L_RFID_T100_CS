/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diInput.c
**
** $Id: diInput.c 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** functions handling the input values:
** - filter the sampled input values (stored in a buffer) and store them
**   in enum with Hamming distance
** - functions to prepare and handle the IPC exchange of input values
** - functions to prepare and handle the IPC exchange of input qualifiers
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

/*******************************************************************************
**
** includes
**
********************************************************************************
*/

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

#include "cfg_Config-sys.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

/* Header-file of module */
#include "gpio-hal.h"
#include "gpio_cfg.h"
#include "diCfg.h"
#include "fiParam.h"
#include "diPortMap.h"
#include "doPortMap.h"
#include "diDoDiag.h"
#include "gpioSample.h"
#include "diErrHdl.h"

#include "diInput.h"
#include "diInput_priv.h"



/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  Structure/Variable for the filtered values (filtered results of Mikro-Read)
**  of all inputs to read at one uC
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
DIINPUT_VALUES_STRUCT diInput_sInputValues;

/*------------------------------------------------------------------------------
**  Array containing the resulting state of Digital "safe" inputs of both
**  channels (filtered values stored with enum values)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
GPIO_STATE_ENUM diInput_eDiValue[GPIOCFG_NUM_DI_TOT];

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of digital "safe" inputs as Bit
**  information (used to send values via IPC)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
UINT8 diInput_u8DiBitValues;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of digital "safe" inputs received
**  from other channel as Bit information (received via IPC)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
UINT8 diInput_u8DiBitValuesOtherCh;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of the test qualifier of digital
**  "safe" inputs as Bit information (used to send values via IPC)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
UINT8 diInput_u8DiTestQualValues = (UINT8)0xFFu;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of the test qualifiers received
**  from other channel as Bit information (received via IPC)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
UINT8 diInput_u8DiTestQualValuesOtherCh = (UINT8)0xFFu;


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** mapping of digital (save) inputs inside the buffer
**------------------------------------------------------------------------------
*/
STATIC CONST DIINPUT_BUFF_LOC_STRUCT sBuffLocationDi[GPIOCFG_NUM_DI_PER_uC] =
{
  { DIPORTMAP_BUFFER_INDEX_PORT_DI1, DIPORTMAP_PINMASK_DI1},
  { DIPORTMAP_BUFFER_INDEX_PORT_DI2, DIPORTMAP_PINMASK_DI2},
  { DIPORTMAP_BUFFER_INDEX_PORT_DI3, DIPORTMAP_PINMASK_DI3}
/* ... etc. for more output ports */
};

/*------------------------------------------------------------------------------
** mapping (inside the buffer) of read-back-inputs for digital (save) outputs
** (Test_DO signals in schematic) there is one for every DO HW-Pin, but connected
** to both uCs.
**------------------------------------------------------------------------------
*/
STATIC CONST DIINPUT_BUFF_LOC_STRUCT sBuffLocationTestDo[GPIOCFG_NUM_TESTDO] =
{
  { DIPORTMAP_BUFFER_INDEX_PORT_TEST_DOs1, PINMASK_TEST_DO1_uC1},
  { DIPORTMAP_BUFFER_INDEX_PORT_TEST_DOs2, PINMASK_TEST_DO1_uC2}
/* ... etc. for more output ports */
};

/*------------------------------------------------------------------------------
** mapping (inside the buffer) of enable-output signals coming from voltage
** supervision (EN_DO... signals in schematic)
** Currently, there is one at every uC independent from the number of outputs
** (voltage supervision enables/disables all outputs)
**------------------------------------------------------------------------------
*/
STATIC CONST DIINPUT_BUFF_LOC_STRUCT sBuffLocationEnDo[GPIOCFG_NUM_ENDO] =
{
  {DIPORTMAP_BUFFER_INDEX_PORT_EN_DO, PINMASK_EN_DO}
};


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diInput_Init()
**
** Description:
**    Get for all inputs valid and filtered values (used during startup).
**    All inputs will be read x times and filtered to have filtered values
**    after startup available:
**    - buffer will be read x times, depending of buffer dimension
**    - between every read step a fixed time of y us will be waited (debounce)
**    - then filter functions are called to filter buffer content
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diInput_Init(void)
{
   UINT8 u8Index;

   /* first set default value for all input state variables */
   /* for all digital safe inputs */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_PER_uC; u8Index++)
   {
      diInput_sInputValues.eDi1stFilt[u8Index] = DICFG_DI_INACTIVE;
      diInput_sInputValues.eDi2ndFilt[u8Index] = DICFG_DI_INACTIVE;
      diInput_eDiValue[DICFG_GET_DI_NUM_C1(u8Index)] = DICFG_DI_INACTIVE;
      diInput_eDiValue[DICFG_GET_DI_NUM_C2(u8Index)] = DICFG_DI_INACTIVE;
   }
}


/*------------------------------------------------------------------------------
**
** diInput_FilterDoRelInputs()
**
** Description:
**    calls the filter functions for all DO related inputs (read-backs)
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_FilterDoRelInputs();
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function calls
**------------------------------------------------------------------------------
*/
void diInput_FilterDoRelInputs(void)
{
   /* call function to filter Test-DOs */
   diInput_FilterTestDoValues();
   /* call function to filter EN-DOs */
   diInput_FilterEnDoValues();
}


/*------------------------------------------------------------------------------
**
** diInput_FilterDiRelInputs()
**
** Description:
**    calls the filter functions for all DI related inputs
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_FilterDiRelInputs();
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function calls
**------------------------------------------------------------------------------
*/
void diInput_FilterDiRelInputs(void)
{
   /* call function to filter DI input values */
   diInput_FilterDiValues();

   /* may be called here, but also can be called in different cycle */
   diInput_UpdateDiValuesOwnCh();
}


/*------------------------------------------------------------------------------
**
** diInput_PrepareValForIPC()
**
** Description:
**    calls functions to convert the values of input values and error qualifiers
**    of current uC to Bit Format.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_PrepareValForIPC()
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function calls
**------------------------------------------------------------------------------
*/
void diInput_PrepareValForIPC(void)
{
   /* call function to convert current DI values from enum to bit style */
   diInput_PrepareDiValuesForIpc();

   /* call function to convert current DI test qualifier from enum to bit style */
   diInput_PrepareTestQualForIpc();

}


/*------------------------------------------------------------------------------
**
** diInput_ConvValFromOtherCh()
**
** Description:
**    calls the converting functions for the input values and qualifiers
**    received from other uC
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_ConvValFromOtherCh()
**
** Module Test:
**    - NO -
**    Reason: low complexity, just function calls
**------------------------------------------------------------------------------
*/
void diInput_ConvValFromOtherCh(void)
{
   /* convert DI values from other channel from Bit to enum style */
   diInput_ConvertDiValuesFromOtherCh();

   /* convert test qualifier values from other channel from Bit to enum style */
   diInput_ConvertTestQualFromOtherCh();
}



/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diInput_filterValues()
**
** Description:
**    filters the result of "micro read" (read-result of all inputs stored in a
**    buffer) and stores the filtered result in an array which then contains the
**    filtered input values.
**    The first filter is per default set to DISMP_X_OO_Y, it may be changed with
**    corresponding defines.
**    This result is filtered by a second filter which depends on configuration/
**    parametrization of the device (default is, that this second filter is 0,
**    which means "not used").
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_filterValues();
**
** Module Test:
**    - YES -
**    Reason: complexity, not easy to understand
**------------------------------------------------------------------------------
*/
STATIC void diInput_FilterDiValues(void)
{
   LOCAL_STATIC(,RDS_UINT16, u16RdsDiStateCnt[3u], DI_DEB_HL_FILT_INIT);/*lint !e708*/
   /* PC-Lint message 708 deactivated for this statement. LOCAL_STATIC is used
    * for unit test instrumentation. Initialization is OK. */

   /* index for different ports */
   UINT8 u8DiNum;
   UINT8 u8BuffNum;
   UINT8 u8Temp;
   UINT16 u16PortVal;

   /* outer loop: do this for every input pin (digital "safe" input) */
   for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
   {
      /* reset temp. variable (used to store number of read "1" for one pin) */
      u8Temp = 0u;

      /* inner loop: do for whole buffer dimension/depth */
      for (u8BuffNum = 0u; u8BuffNum < DISMP_BUFFDIM; u8BuffNum++)
      {
         /* get the port value (of the port where the pin is located) out of the buffer */
         u16PortVal = (gpioSample_au16Buffer[sBuffLocationDi[u8DiNum].ePortBuffIndex][u8BuffNum]);

         /* mask pin value out of port value and add result to temporary variable */
         if ((u16PortVal) & (sBuffLocationDi[u8DiNum].u16PinMask))
         {
            u8Temp++;
         }
      }

      /* if pin value was more than x times of y HIGH in buffer, then set filtered value to HIGH */
      if (u8Temp >= DISMP_X_OO_Y)
      {
         diInput_sInputValues.eDi1stFilt[u8DiNum] = eGPIO_HIGH;

         /* check if DI is configured to DI-C. Hint: Macro/Array "DICFG_GET_DI_NUM_C1" can be used
          * for both uCs in this case, because configuration is the same for both
          * DIs of one dual-channel */
         if (FIPARAM_DI_INPUTTYPE_CONTACT(DICFG_GET_DI_NUM_C1(u8DiNum)))
         {
            /* second step of filter: z times the "x out of y" event, z depending on parameter 
             * (see [SRS_101]) and on additional filter cycles for Digital Contact inputs
             * HINT: UINT16 casting to avoid problems in case that the calculated sum
             * exceeds UINT8 range */
            if (((UINT16)DIINPUT_ADD_DI_C_FILTERCYCLES + (UINT16)(FIPARAM_DI_DEBFILTER(u8DiNum))) 
                   > RDS_GET(u16RdsDiStateCnt[u8DiNum]))
            {
               RDS_INC(u16RdsDiStateCnt[u8DiNum]);
            }
            else
            {
               diInput_sInputValues.eDi2ndFilt[u8DiNum] = eGPIO_HIGH;
            }
         }
         else
         {
            /* second step of filter: z times the "x out of y" event, z depending on parameter 
             * (see [SRS_101]) and on additional filter cycles for Digital Semiconductor inputs
             * HINT: UINT16 casting to avoid problems in case that the calculated sum
             * exceeds UINT8 range */
            if (((UINT16)DIINPUT_ADD_DI_S_FILTERCYCLES + (UINT16)(FIPARAM_DI_DEBFILTER(u8DiNum))) 
                   > RDS_GET(u16RdsDiStateCnt[u8DiNum]))
            {
               RDS_INC(u16RdsDiStateCnt[u8DiNum]);
             }
            else
            {
               diInput_sInputValues.eDi2ndFilt[u8DiNum] = eGPIO_HIGH;
            }
         }
      }
      else
      {
         diInput_sInputValues.eDi1stFilt[u8DiNum] = eGPIO_LOW;

         /* Note 948: Operator '==' always evaluates to True
          * Problem is in RDS_GET. */
         /* second step of filter: is there a request to have z times the "x out of y" event? */
         if (RDS_GET(u16RdsDiStateCnt[u8DiNum]) > 0u)/*lint !e948*/
         {
            RDS_DEC(u16RdsDiStateCnt[u8DiNum]);
         }
         else
         {
            diInput_sInputValues.eDi2ndFilt[u8DiNum] = eGPIO_LOW;
         }
      }
   }
}


/*------------------------------------------------------------------------------
**
** diInput_FilterTestDoValues()
**
** Description:
**    filters the result of "micro read" and stores the result to an array which then
**    contains the filtered input results. The filter is per default set to DISMP_X_OO_Y,
**    it may be changed with corresponding defines.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_FilterTestDoValues();
**
** Module Test:
**    - YES -
**    Reason: complexity
**------------------------------------------------------------------------------
*/
STATIC void diInput_FilterTestDoValues(void)
{
   /* index for different ports */
   UINT8 u8TestDoNum;
   UINT8 u8BuffNum;
   UINT8 u8Temp;
   UINT16 u16PortVal;

   /* outer loop: do this for every input pin (digital "safe" input) */
   for (u8TestDoNum = 0u; u8TestDoNum < (UINT8)GPIOCFG_NUM_TESTDO; u8TestDoNum++ )
   {
      /* reset temp. variable (used to store number of read "1" for one pin) */
      u8Temp = 0u;

      /* inner loop: do for whole buffer dimension/depth */
      for (u8BuffNum = 0u; u8BuffNum < DISMP_BUFFDIM; u8BuffNum++)
      {
         /* get the port value (of the port where the pin is located) out of the buffer */
         u16PortVal = 
           (gpioSample_au16Buffer[sBuffLocationTestDo[u8TestDoNum].ePortBuffIndex][u8BuffNum]);

         /* mask pin value out of port value and increase temporary variable, whenever pin is set */
         if ((u16PortVal) & (sBuffLocationTestDo[u8TestDoNum].u16PinMask))
         {
            u8Temp++;
         }
      }

      /* if pin value was more than x times of y HIGH in buffer, then set filtered value to HIGH */
      if (u8Temp >= DISMP_X_OO_Y)
      {
         diInput_sInputValues.eDoTest[u8TestDoNum] = eGPIO_HIGH;
      }
      else
      {
         diInput_sInputValues.eDoTest[u8TestDoNum] = eGPIO_LOW;
      }
   }
}


/*------------------------------------------------------------------------------
**
** diInput_FilterEnDoValues()
**
** Description:
**    filters the result of "micro read" and stores the result to an array which
**    then contains the filtered input results.
**    The first filter is per default set to DISMP_X_OO_Y, it may be changed
**    with corresponding defines.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    diInput_FilterEnDoValues();
**
** Module Test:
**    - YES -
**    Reason: complexity
**------------------------------------------------------------------------------
*/
STATIC void diInput_FilterEnDoValues(void)
{
   /* index for different ports */
   UINT8  u8BuffNum;
   UINT8  u8Temp;
   UINT16 u16PortVal;

   /* reset temp. variable (used to store number of read "1" for one pin) */
   u8Temp = 0u;

   /* loop: do for whole buffer dimension/depth */
   for (u8BuffNum = 0u; u8BuffNum < DISMP_BUFFDIM; u8BuffNum++)
   {
      /* get the port value (of the port where the pin is located) out of the buffer */
      u16PortVal = (gpioSample_au16Buffer[sBuffLocationEnDo[0].ePortBuffIndex][u8BuffNum]);

      /* mask pin value out of port value and increase temporary variable, whenever pin is set */
      if ((u16PortVal) & (sBuffLocationEnDo[0].u16PinMask))
      {
         u8Temp++;
      }
   }

   /* if pin value was more than x times of y HIGH in buffer, then set filtered value to HIGH */
   if (u8Temp >= DISMP_X_OO_Y)
   {
      diInput_sInputValues.eEnDo[0] = eGPIO_HIGH;
   }
   else
   {
      diInput_sInputValues.eEnDo[0] = eGPIO_LOW;
   }
}


/*------------------------------------------------------------------------------
**
** diInput_UpdateDiValuesOwnCh()
**
** Description:
**    copies the filtered DI values (only the "safe" digital inputs) of own
**    uc to the diInput_eDiValue-Array containing the values of all "safe" DIs.
**    This diInput_eDiValue-Array is then containing the "official" digital input
**    values of the safe inputs.
**    This function should be called at the end or after filter function.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_UpdateDiValuesOwnCh()
**
** Module Test:
**    - YES -
**    Reason: arithmetic
**------------------------------------------------------------------------------
*/
STATIC void diInput_UpdateDiValuesOwnCh(void)
{
   /* index for different pins */
   UINT8 u8DiNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every input pin in total (single channel digital "safe" inputs of 
         ** both channels) */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* own inputs at own uC1, to be placed in DI number 0, 2, 4 */
            diInput_eDiValue[DICFG_GET_DI_NUM_C1(u8DiNum)] = 
               (diInput_sInputValues.eDi2ndFilt[u8DiNum]);
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* loop: do this for every input pin in total (single channel digital "safe" inputs of 
         ** both channels) */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* own inputs at uC2, to be placed in DI number 1, 3, 5  */
            diInput_eDiValue[DICFG_GET_DI_NUM_C2(u8DiNum)] = 
               (diInput_sInputValues.eDi2ndFilt[u8DiNum]);
         }
         break;
      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** diInput_PrepareDiValuesForIpc()
**
** Description:
**    converts the values of digital safe inputs at the current uC to Bit Format.
**    These bits are stored in one variable which can be send via IPC later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_PrepareDiValuesForIpc()
**
** Module Test:
**    - YES -
**    Reason: shift operation
**------------------------------------------------------------------------------
*/
STATIC void diInput_PrepareDiValuesForIpc(void)
{
   /* index for different pins */
   UINT8 u8DiNum;

   /* reset all bits */
   diInput_u8DiBitValues    = 0x00u;

   /* loop: do this  for every input pin (digital "safe" input) */
   for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
   {
      /* convert enum in bit values */
      if (eGPIO_HIGH == DIINPUT_DI_OWN_SCND_FILTER(u8DiNum))
      {
        /* set Bit to 1 if input HIGH for not-inverted values*/
        diInput_u8DiBitValues |= (UINT8)(0x01u << u8DiNum);
      }
      else if (eGPIO_LOW == DIINPUT_DI_OWN_SCND_FILTER(u8DiNum))
      {
         /* nothing to do, because of initialization/reset of 
         ** "diInput_u8DiBitValues.diInput_eDiValues" and 
         ** "diInput_u8DiBitValues.diInput_eDiValuesInv" */
      }
      else
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
      }
   }

}


/*------------------------------------------------------------------------------
**
** diInput_PrepareTestQualForIpc()
**
** Description:
**    converts the values of input error qualifiers of current uC to Bit Format.
**    These bits are stored in one variable which can be send via IPC later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_PrepareTestQualForIpc()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**------------------------------------------------------------------------------
*/
STATIC void diInput_PrepareTestQualForIpc(void)
{
   /* index for different pins */
   UINT8 u8DiNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* reset all bits, test qualifier bit "1" is Ok and "0" error */
   diInput_u8DiTestQualValues    = 0x00u;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every input pin of own uC (digital "safe" input) */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            DI_ERR_ADOPT_TMP_QUALIFIER(u8DiNum*2u);

            /* convert enum in bit values, take values from input 0, 2, 4 (1,3,5 in schematic)  */
            if (DI_ERR_IS_TEST_QUAL_OK_OR_NA(u8DiNum*2u))
            {
               /* set Bit to 1 (OK) if test-bit OK or NA (not aplicable) */
               diInput_u8DiTestQualValues |= (UINT8)(0x01u << u8DiNum);
            }
         }
         break;
      case SAFETY_CONTROLLER_2:
         /* loop: do this for every input pin of own uC (digital "safe" input) */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            DI_ERR_ADOPT_TMP_QUALIFIER(((u8DiNum*2u) + 1u));

            /* convert enum in bit values, take values from input 1, 3, 5  (2,4,6 in schematic) */
            if (DI_ERR_IS_TEST_QUAL_OK_OR_NA((u8DiNum*2u) + 1u))
            {
               /* set Bit to 1 if input HIGH for not-inverted values*/
               diInput_u8DiTestQualValues |= (UINT8)(0x01u << u8DiNum);
            }
         }
         break;
      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** diInput_ConvertDiValuesFromOtherCh()
**
** Description:
**    converts the digital input values received from other channel:
**    - safe input values which are received as bit information
**      via IPC are converted to hamming-distance enum values
**    - store result in diInput_eDiValue-Array
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_ConvertDiValuesFromOtherCh()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**------------------------------------------------------------------------------
*/
STATIC void diInput_ConvertDiValuesFromOtherCh(void)
{
   /* index for different pins */
   UINT8 u8DiNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every input pin of other channel */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* is bit information from other channel "1"? */
            if ( (diInput_u8DiBitValuesOtherCh) & (UINT8)(0x01u << u8DiNum) )
            {
               /* inputs at other uC (uc2), to be placed in DI number 1, 3, 5 */
               diInput_eDiValue[DICFG_GET_DI_NUM_C2(u8DiNum)] = eGPIO_HIGH;
            }
            else
            {
               /* inputs at other uC (uc2), to be placed in DI number 1, 3, 5 */
               diInput_eDiValue[DICFG_GET_DI_NUM_C2(u8DiNum)] = eGPIO_LOW;
            }
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* loop: do this for every input pin of other channel */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* is bit information from other channel "1"? */
            if ( (diInput_u8DiBitValuesOtherCh) & (UINT8)(0x01u << u8DiNum) )
            {
               /* inputs at other uC (uC1), to be placed in DI number 0, 2, 4 */
               diInput_eDiValue[DICFG_GET_DI_NUM_C1(u8DiNum)] = eGPIO_HIGH;
            }
            else
            {
               /* inputs at other uC (uC1), to be placed in DI number 0, 2, 4 */
               diInput_eDiValue[DICFG_GET_DI_NUM_C1(u8DiNum)] = eGPIO_LOW;
            }
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
/* RSM_IGNORE_QUALITY_BEGIN Notice #2 - Function name length > 32 characters */
}
/* RSM_IGNORE_END */


/*------------------------------------------------------------------------------
**
** diInput_ConvertTestQualFromOtherCh()
**
** Description:
**    converts the test qualifier values of the inputs received from
**    other channel:
**    if the error qualifier of other channel is in error state, the corresponding
**    error flag is set.
**    This information is exchanged because of 2-channel processing, both channels
**    shall know about the state of all inputs.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    diInput_ConvertTestQualFromOtherCh()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**------------------------------------------------------------------------------
*/
STATIC void diInput_ConvertTestQualFromOtherCh(void)
{
   /* index for different pins */
   UINT8 u8DiNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every input pin of other channel */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* inputs at other uC (uc2), to be placed in DI number 1, 3, 5 */
            if ( (diInput_u8DiTestQualValuesOtherCh) & (UINT8)(0x01u << u8DiNum) )
            {
               DI_ERR_SET_TEST_QUAL( ((u8DiNum*2u) + 1u), eGPIO_DIAG_OK);
            }
            else
            {
               DI_ERR_SET_TEST_QUAL( ((u8DiNum*2u) + 1u), eGPIO_DIAG_ERR);
            }
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* loop: do this for every input pin of other channel */
         for (u8DiNum = 0u; u8DiNum < (UINT8)GPIOCFG_NUM_DI_PER_uC; u8DiNum++ )
         {
            /* inputs at other uC (uc2), to be placed in DI number 0, 2, 4 */
            if ( (diInput_u8DiTestQualValuesOtherCh) & (UINT8)(0x01u << u8DiNum) )
            {
               DI_ERR_SET_TEST_QUAL( (u8DiNum*2u), eGPIO_DIAG_OK);
            }
            else
            {
               DI_ERR_SET_TEST_QUAL( (u8DiNum*2u), eGPIO_DIAG_ERR);
            }
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
/* RSM_IGNORE_QUALITY_BEGIN Notice #2 - Function name length > 32 characters */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**
** End of diInput.c
**
********************************************************************************
*/

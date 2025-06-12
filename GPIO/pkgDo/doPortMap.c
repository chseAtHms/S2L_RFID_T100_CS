/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doPortMap.c
**
** $Id: doPortMap.c 2448 2017-03-27 13:45:16Z klan $
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
** source file for port mapping of digital outputs, contains initialization
** of output portpins and the definition (port/pin number etc) of output pins.
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

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "cfg_Config-sys.h"

/* Module header */
#include "gpio-hal.h"
#include "gpio_cfg.h"

/* Header-file of module */
#include "doPortMap.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** mapping of digital "safe" outputs (DO in schematic)
**------------------------------------------------------------------------------
*/
CONST OUT_PORT_SETTINGS_STRUCT asOutputInit[GPIOCFG_NUM_DO_PER_uC] =
{
  { PORT_DO1, PINMASK_DO1, PIN_DO1, DO1_CFG, DO1_MODE},

#if (GPIOCFG_NUM_DO_PER_uC>1u)
  { PORT_DO2, PINMASK_DO2, PIN_DO2, DO2_CFG, DO2_MODE},
#endif

#if (GPIOCFG_NUM_DO_PER_uC>2u)
  { PORT_DO3, PINMASK_DO3, PIN_DO3, DO3_CFG, DO3_MODE},
#endif

#if (GPIOCFG_NUM_DO_PER_uC>3u)
  { PORT_DO4, PINMASK_DO4, PIN_DO4, DO4_CFG, DO4_MODE},
#endif

/* ... etc. for more output ports */

};


/*------------------------------------------------------------------------------
** mapping of discharge pin
**------------------------------------------------------------------------------
*/
CONST OUT_PORT_SETTINGS_STRUCT asDisChrgInit[GPIOCFG_NUM_DO_PER_uC] =
{
  { PORT_DISCHARGE_DO1, 
    PINMASK_DISCHARGE_DO1, 
    PIN_DISCHARGE_DO1, 
    DISCHARGE_CFG_DO1, 
    DISCHARGE_MODE_DO1},

#if (GPIOCFG_NUM_DO_PER_uC>1u)
  /* just an example for future HW-Extensions */
  { PORT_DISCHARGE_DO2, 
    PINMASK_DISCHARGE_DO2, 
    PIN_DISCHARGE_DO2, 
    DISCHARGE_CFG_DO2, 
    DISCHARGE_MODE_DO2},
#endif
/* ... etc. for more discharge/output ports */
};


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/
STATIC void doPortMap_InitEnDOs(void);
STATIC void doPortMap_InitTestDOs(void);
STATIC void doPortMap_InitDischarge(void);
STATIC void doPortMap_InitLifeSignal(void);
STATIC void doPortMap_InitDOs(void);
STATIC void doPortMap_InitTest3V3(void);

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doPortMap_Init()
**
** Description:
**    function for initialization of output ports
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doPortMap_Init()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void doPortMap_Init(void)
{
   /* call initialization of discharge pins */
   doPortMap_InitDischarge();
   /* call initialization of life-signal pin */
   doPortMap_InitLifeSignal();
   /* call initialization of Test 3V3 pins */
   doPortMap_InitTest3V3();
   /* call initialization of DO pins */
   doPortMap_InitDOs();
   /* call initialization of EN-DO pins */
   doPortMap_InitEnDOs();
   /* call initialization of Test DO pins */
   doPortMap_InitTestDOs();
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doPortMap_InitDOs()
**
** Description:
**    function for initialization of digital "safe output pins
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    doPortMap_InitDOs()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitDOs(void)
{
   /* clear the output data register before setting pin to output */
   gpioHAL_ClearPin(PORT_DO1, PINMASK_DO1);
   /* set DO1 pin to output */
   gpioHAL_ConfigureOutput(PORT_DO1, PIN_DO1, DO1_CFG, DO1_MODE);
}


/*------------------------------------------------------------------------------
**
** doPortMap_InitTest3V3()
**
** Description:
**    function for initialization of Test 3V3 pins
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    doPortMap_InitTest3V3()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitTest3V3(void)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   /* clear the output data register before setting pin to output */
   gpioHAL_ClearPin(PORT_TEST_3V3, PINMASK_TEST_3V3);
   /* set TEST_3V3 pin to output */
   gpioHAL_ConfigureOutput(PORT_TEST_3V3, PIN_TEST_3V3, TEST_3V3_CFG, TEST_3V3_MODE);

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* init value: set TEST_3V3 Pin to "HIGH" at uC1 */
         gpioHAL_SetPin(PORT_TEST_3V3, PINMASK_TEST_3V3);
         break;

      case SAFETY_CONTROLLER_2:
         /* init value: set TEST_3V3 Pin to "HIGH" at uC1 */
         gpioHAL_ClearPin(PORT_TEST_3V3, PINMASK_TEST_3V3);
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
** doPortMap_InitDischarge()
**
** Description:
**    function for initialization of "discharge" pins
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    doPortMap_InitDischarge()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitDischarge(void)
{
   /* set the output data register before setting pin to output => this would
    * be the state "do not discharge" */
   gpioHAL_SetPin(PORT_DISCHARGE_DO1, PINMASK_DISCHARGE_DO1);
   /* set Discharge pin to output */
   gpioHAL_ConfigureOutput(PORT_DISCHARGE_DO1, 
                           PIN_DISCHARGE_DO1, 
                           DISCHARGE_CFG_DO1, 
                           DISCHARGE_MODE_DO1);
   /* set the output data register again ("just for sure" in case of discharge pin)
    * => this would be the state "do not discharge" */
   gpioHAL_SetPin(PORT_DISCHARGE_DO1, PINMASK_DISCHARGE_DO1);
}


/*------------------------------------------------------------------------------
**
** doPortMap_InitLifeSignal()
**
** Description:
**    function for initialization of "discharge" pins
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitLifeSignal(void)
{
   /* set the output data register before setting pin to output */
   gpioHAL_ClearPin(PORT_LIFE_SIG, PINMASK_LIFE_SIG);
   /* set LIFE pin to output */
   gpioHAL_ConfigureOutput(PORT_LIFE_SIG, PIN_LIFE_SIG, LIFE_SIG_CFG, LIFE_SIG_MODE);
}


/*------------------------------------------------------------------------------
**
** doPortMap_InitEnDOs()
**
** Description:
**    function for initialization of "enable DO" (read back signals).
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    doPortMap_InitEnDOs()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitEnDOs(void)
{
   gpioHAL_ConfigureInput( PORT_EN_DO,
                           PIN_EN_DO_uC2,
                           IN_CONFIG_FLOATING );
}


/*------------------------------------------------------------------------------
**
** doPortMap_InitTestDOs()
**
** Description:
**    function for initialization of "test DO" (read back signals).
**    calls initialization functions of uC ports
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    doPortMap_InitTestDOs()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void doPortMap_InitTestDOs(void)
{
   gpioHAL_ConfigureInput( PORT_TEST_DO1_uC1,
                           PIN_TEST_DO1_uC1,
                           IN_CONFIG_FLOATING );

   gpioHAL_ConfigureInput( PORT_TEST_DO1_uC2,
                           PIN_TEST_DO1_uC2,
                           IN_CONFIG_FLOATING );
}


/*******************************************************************************
**
** End of doPortMap.c
**
********************************************************************************
*/

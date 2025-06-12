/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diPortMap.c
**
** $Id: diPortMap.c 4086 2023-09-27 15:49:53Z ankr $
** $Revision: 4086 $
** $Date: 2023-09-27 17:49:53 +0200 (Mi, 27 Sep 2023) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** source file for port mapping of digital inputs, contains initialization
** of input portpins and the definition (port/pin number etc) of input pins.
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
#include "gpio-hal.h"
#include "gpio_cfg.h"

#include "diCfg.h"

/* Header-file of module */
#include "diPortMap.h"




/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** mapping of TI-Level diagnostic outputs (TI_Level in schematic)
**------------------------------------------------------------------------------
*/
CONST DIPORTMAP_INPT_OUT_PORT_SETTINGS_STRUCT diPortMap_sTiLevelMap[GPIOCFG_NUM_DI_PER_uC] =
{
  { DIPORTMAP_PORT_TI_LEVEL_DI1, DIPORTMAP_PINMASK_TI_LEVEL_DI1},
  { DIPORTMAP_PORT_TI_LEVEL_DI2, DIPORTMAP_PINMASK_TI_LEVEL_DI2},
  { DIPORTMAP_PORT_TI_LEVEL_DI3, DIPORTMAP_PINMASK_TI_LEVEL_DI3},

#if (GPIOCFG_NUM_DI_PER_uC > 3u)
  { PORT_TI_LEVEL_DI4, PINMASK_TI_LEVEL_DI4},
#endif
/* ... etc. for more input ports */
};


/*------------------------------------------------------------------------------
** mapping of TOs (diagnostic outputs generating pulse signal (TO in schematic))
**------------------------------------------------------------------------------
*/
CONST DIPORTMAP_INPT_OUT_PORT_SETTINGS_STRUCT diPortMap_sToMap[GPIOCFG_NUM_TO] =
{
  { DIPORTMAP_PORT_TO1, DIPORTMAP_PINMASK_TO1},
#if (GPIOCFG_NUM_TO > 1u)
  { PORT_TO2, PINMASK_TO2},
#endif
/* ... etc. for more input ports */
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
STATIC void diPortMap_InitDIs(void);
STATIC void diPortMap_InitTiLevel(void);
STATIC void diPortMap_InitTOs(void);


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diPortMap_Init()
**
** Description:
**    function for initialization of input ports.
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
**    diPortMap_Init()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void diPortMap_Init(void)
{
   /* DI for uC1 + uC2 */
   diPortMap_InitDIs();

   /* All TI_Level Low Level "PA4 ... PA6" */
   diPortMap_InitTiLevel();

   /*  ENx TO */
   diPortMap_InitTOs();
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diPortMap_InitDIs()
**
** Description:
**    function for initialization of digital "safe" inputs.
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
**    diPortMap_InitDIs()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void diPortMap_InitDIs(void)
{
   /* init DI for uC1 + uC2, see [SRS_19] */
   gpioHAL_ConfigureInput( DIPORTMAP_PORT_DI1,
                           DIPORTMAP_PIN_DI1,
                           IN_CONFIG_FLOATING );

   gpioHAL_ConfigureInput( DIPORTMAP_PORT_DI2,
                           DIPORTMAP_PIN_DI2,
                           IN_CONFIG_FLOATING );

   gpioHAL_ConfigureInput( DIPORTMAP_PORT_DI3,
                           DIPORTMAP_PIN_DI3,
                           IN_CONFIG_FLOATING );
}


/*------------------------------------------------------------------------------
**
** diPortMap_InitTiLevel()
**
** Description:
**    function for initialization of TI Level pins.
**    calls initialization functions of uC ports:
**    - set pin to default value
**    - configure pin to output
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - void -
**
** Usage:
**    diPortMap_InitTiLevel()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void diPortMap_InitTiLevel(void)
{
   /* TI-Level pins are outputs, so set the data value first to 0 */
   gpioHAL_ClearPin( DIPORTMAP_PORT_TI_LEVEL_DI1,
                     DIPORTMAP_PINMASK_TI_LEVEL_DI1);

   gpioHAL_ClearPin( DIPORTMAP_PORT_TI_LEVEL_DI2,
                     DIPORTMAP_PINMASK_TI_LEVEL_DI2);

   gpioHAL_ClearPin( DIPORTMAP_PORT_TI_LEVEL_DI3,
                     DIPORTMAP_PINMASK_TI_LEVEL_DI3);

   /*
    * All TI_Level Low Level "PA4 ... PA6"
    */
   gpioHAL_ConfigureOutput ( DIPORTMAP_PORT_TI_LEVEL_DI1,
                             DIPORTMAP_PIN_TI_LEVEL_DI1,
                             OUT_CONFIG_GPIO_PP,
                             OUT_MODE_50MHZ );

   gpioHAL_ConfigureOutput ( DIPORTMAP_PORT_TI_LEVEL_DI2,
                             DIPORTMAP_PIN_TI_LEVEL_DI2,
                             OUT_CONFIG_GPIO_PP,
                             OUT_MODE_50MHZ );

   gpioHAL_ConfigureOutput ( DIPORTMAP_PORT_TI_LEVEL_DI3,
                             DIPORTMAP_PIN_TI_LEVEL_DI3,
                             OUT_CONFIG_GPIO_PP,
                             OUT_MODE_50MHZ );
}


/*------------------------------------------------------------------------------
**
** diPortMap_InitTOs()
**
** Description:
**    function for initialization of test outputs (TO).
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
**    diPortMap_InitTOs()
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
STATIC void diPortMap_InitTOs(void)
{
   /* low active, set TO pin to not-active (inverted, HIGH: Not active)  */
   gpioHAL_SetPin( DIPORTMAP_PORT_TO1,
                   DIPORTMAP_PINMASK_TO1);

   /*  configure ENx TO to output */
   gpioHAL_ConfigureOutput( DIPORTMAP_PORT_TO1,
                            DIPORTMAP_PIN_TO1,
                            OUT_CONFIG_GPIO_OD,
                            OUT_MODE_50MHZ );
}


/*******************************************************************************
**
** End of diPortMap.c
**
********************************************************************************
*/

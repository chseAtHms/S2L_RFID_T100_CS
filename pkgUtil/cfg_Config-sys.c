/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** cfg_config-sys.c
**
** $Id: cfg_Config-sys.c 4413 2024-05-02 13:03:13Z ankr $
** $Revision: 4413 $
** $Date: 2024-05-02 15:03:13 +0200 (Do, 02 Mai 2024) $
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
** This module reads the hardware specific configuration for the application.
** The configuration is:
** - Controller ID, identification if this is controller 1 or 2
** - hardware code to identify the amount of digital in- and outputs
**
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011-2024 HMS Industrial Networks AB            **
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
/* system includes */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "gpio-hal.h"

/* Header-file of module */
#include "cfg_Config-sys_loc.h"
#include "cfg_Config-sys.h"

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/* Check if all required configuration defines are present */

#ifndef CFG_TEMPERATURE_SENSOR_DIGITAL
  #error The value for CFG_TEMPERATURE_SENSOR_DIGITAL must be defined
#endif
#if (    (CFG_TEMPERATURE_SENSOR_DIGITAL != TRUE)   \
      && (CFG_TEMPERATURE_SENSOR_DIGITAL != FALSE)    \
    )
  #error CFG_S2L_SS1T_SUPPORTED configured invalid
#endif

#ifndef CFG_S2L_SAFEBOUND_SUPPORTED
  #error The value for CFG_S2L_SAFEBOUND_SUPPORTED must be defined
#endif
#if (    (CFG_S2L_SAFEBOUND_SUPPORTED != TRUE)   \
      && (CFG_S2L_SAFEBOUND_SUPPORTED != FALSE)    \
    )
  #error CFG_S2L_SS1T_SUPPORTED configured invalid
#endif

#ifndef CFG_S2L_SS1T_SUPPORTED
  #error The value for CFG_S2L_SS1T_SUPPORTED must be defined
#endif
#if (    (CFG_S2L_SS1T_SUPPORTED != TRUE)   \
      && (CFG_S2L_SS1T_SUPPORTED != FALSE)    \
    )
  #error CFG_S2L_SS1T_SUPPORTED configured invalid
#endif


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*! This structure contains the configuration for the module. */
STATIC CFG_CONFIG_STRUCT cfgSYS_sConfiguration =
{
   SAFETY_CONTROLLER_INVALID
};

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** cfgSYS_Init()
**
** Description:
** This function initialize the config-sys module.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    cfgSYS_ReadControllerID
**
** Module Test: - NO -
**      Reason: Only one function call
**------------------------------------------------------------------------------
*/
void cfgSYS_Init (void)
{
  cfgSYS_ReadControllerID();
}


/*------------------------------------------------------------------------------
**
** cfgSYS_GetControllerID()
**
** Description:
** This function returns the controller ID.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    CFG_CONTROLLER_ID_ENUM        Controller ID.
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: This function returns only one value.
**------------------------------------------------------------------------------
*/
CFG_CONTROLLER_ID_ENUM cfgSYS_GetControllerID (void)
{
  return (cfgSYS_sConfiguration.eControllerID);
}


/*------------------------------------------------------------------------------
**
** cfgSYS_GetDiDoConfig()
**
** Description:
** This function reads the GPIO Pins which defines the amount of digital
** input and output channels (see [SRS_280]), for further information see
** HDD_HMS_ASM.doc, chapter "Micro-Controller and Periphery".
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    u8DiDoCfg        Byte with the hardware code
**
** Usage:
**    gpioHAL_GetPin
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
UINT8 cfgSYS_GetDiDoConfig (void)
{
   GPIO_PINSTATE_ENUM ePinStat;
   UINT8 u8DiDoCfg = 0u;
   UINT8 i;

   for (i = 0u; i < 8u; i++)
   {
      /* read pin state */
      ePinStat = gpioHAL_GetPin(CFG_CONTROLLER_CFG_PORT, (UINT16)((UINT32)GPIO_PINMASK_8 << i));

      if (ePinStat == IO_HIGH)
      {
         /* if the pin state is high, set the specific bit to 1 */
         u8DiDoCfg |= (UINT8)(1u << i);
      }
      else
      {
         /* check if pin state is illegal */
         if (ePinStat != IO_LOW)
         {
            GLOBFAIL_SAFETY_FAIL_RET(GLOB_FAILCODE_VARIABLE_ERR, 0u);
         }
      }
   }

   return u8DiDoCfg;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** cfgSYS_ReadControllerID()
**
** Description:
**  This function reads the state of the IO-pin to evaluate which controller
**  it is (see [SRS_276]).
**  A low state indicates that it is controller 1, a high state controller 2.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    gpioHAL_GetPin
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
STATIC void cfgSYS_ReadControllerID (void)
{
   GPIO_PINSTATE_ENUM ePinState;

   ePinState = gpioHAL_GetPin(CFG_CONTROLLER_CFG_PORT, CFG_CONTROLLER_ID_PINMASK);
   if (IO_LOW == ePinState)
   {
      cfgSYS_sConfiguration.eControllerID = SAFETY_CONTROLLER_1;
   }
   else if (IO_HIGH == ePinState)
   {
      cfgSYS_sConfiguration.eControllerID = SAFETY_CONTROLLER_2;
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }
}

/*******************************************************************************
**
** End of cfg_Config-sys.c
**
********************************************************************************
*/

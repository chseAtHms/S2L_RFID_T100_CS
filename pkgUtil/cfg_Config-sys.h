/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** cfg_config-sys.h
**
** $Id: cfg_Config-sys.h 4413 2024-05-02 13:03:13Z ankr $
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
** COPYRIGHT NOTIFICATION (c) 2024 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef  CFGSYS_H
#define  CFGSYS_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/

#include "cfg_Config-sys_cfg.h"


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*
** This configuration define determines how the temperature measurement is done
** Analog Sensor: set to FALSE
** Digital I2C Sensor: set to TRUE
** 
*/
#define CFG_TEMPERATURE_SENSOR_DIGITAL   FALSE


/*
** This configuration define determines if the Safe2Link feature "SafeBound" is
** supported (see [SRS_900]).
** Allowed values: TRUE or FALSE
*/
#define CFG_S2L_SAFEBOUND_SUPPORTED      TRUE

/*
** This configuration define determines if the Safe2Link feature "SS1-t" is
** supported (see [SRS_904]).
** Allowed values: TRUE or FALSE
*/
#define CFG_S2L_SS1T_SUPPORTED           TRUE


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*! This enum defines the controller IDs. */
typedef enum CFG_CONTROLLER_ID_ENUM_TAG
{
  SAFETY_CONTROLLER_INVALID   = 0x0Fu, /*!< Enumeration in case of an error. */
  SAFETY_CONTROLLER_1         = 0x5Au, /*!< ID of the controller 1. */
  SAFETY_CONTROLLER_2         = 0xA5u  /*!< ID of the controller 2. */
} CFG_CONTROLLER_ID_ENUM;


/*! This structure contains the configuration of the module. */
typedef struct CFG_CONFIG_STRUCT_TAG
{
  CFG_CONTROLLER_ID_ENUM              eControllerID;
} CFG_CONFIG_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*! This macro checks if the given Safety Controller ID is correct. If the
 *  value is incorrect, the Safety Handler will be called.*/
#define CFG_CONTROLLER_ID_SAFETY_ASSERT(x) \
                                          GLOBFAIL_SAFETY_ASSERT(    ( SAFETY_CONTROLLER_1 == x)  \
                                                                  || ( SAFETY_CONTROLLER_2 == x), \
                                                                  GLOB_FAILCODE_VARIABLE_ERR      )


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void cfgSYS_Init(void);
extern CFG_CONTROLLER_ID_ENUM cfgSYS_GetControllerID(void);
extern UINT8 cfgSYS_GetDiDoConfig (void);

#endif   /* CFGSYS_H */

/*******************************************************************************
**
** End of cfg_Config-sys.h
**
********************************************************************************
*/

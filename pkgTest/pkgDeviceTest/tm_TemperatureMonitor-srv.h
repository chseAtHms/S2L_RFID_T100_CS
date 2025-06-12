/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** tm_TemperatureMonitor-srv.h
**
** $Id: tm_TemperatureMonitor-srv.h 569 2016-08-31 13:08:08Z klan $
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
** Header of module tm_TemperatureMonitor-srv.
** This module is used to crosswise monitor the temperature of the safety
** microcontrollers. It provides a function to test, if the read temperature is
** within the limits specified in the corresponding cfg-header.
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


#ifndef  TMSRV_H
#define  TMSRV_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/

#include "tm_TemperatureMonitor-srv_cfg.h"


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*! This enum contains the status of the temperature test. */
typedef enum
{
   TMSRV_TEMP_TEST_PASSED = 0xC3C3, /*!< Value for a passed test of temperature at sensor. */
   TMSRV_TEMP_TEST_FAILED = 0x3C3C  /*!< Value for a failed test of temperature at sensor. */
} TMSRV_TEMP_TEST_STATUS_ENUM;


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

extern void tmSRV_Init (void);
extern void tmSRV_CheckTemperature (void);
extern void tmSRV_SyncTemperature(void);
extern TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_GetTempTestStatusOwnChannel (void);
extern TMSRV_TEMP_TEST_STATUS_ENUM tmSRV_GetTempTestStatusOtherChannel (void);
extern UINT16 tmSRV_GetAdcRawValue (void);

#endif   /* <TMSRV_H> */

/*******************************************************************************
**
** End of tm_TemperatureMonitor-srv.h
**
********************************************************************************
*/

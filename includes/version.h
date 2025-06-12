/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** version.h
**
** $Id: version.h 4694 2024-11-06 14:01:47Z ankr $
** $Revision: 4694 $
** $Date: 2024-11-06 15:01:47 +0100 (Mi, 06 Nov 2024) $
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
** This header file contains the software version and other IDs and numbers
** which are used to identify the SW and HW (e.g. Serial-Number)
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
#ifndef VERSION_H
#define VERSION_H


/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/

/* This 'version' header depends on "cfg_config-sys.h"
 * for CFG_S2L_SAFEBOUND_SUPPORTED and CFG_S2L_SS1T_SUPPORTED
 */
#include "cfg_config-sys.h"


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! These constants defines the software version. The definition is the
 *  following (see also [SRS_72]):
 *  M = major, m = minor P = patch
 *
 *  MM.mm.PP
 *
 * */
#define VERSION_SW_VERSION_MAJOR   ((UINT8)0u)
#define VERSION_SW_VERSION_MINOR   ((UINT8)5u)
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
  /* Design-In Basic (with digital temperature sensor) */
  #define VERSION_SW_VERSION_PATCH ((UINT8) 208u)
#elif ((CFG_S2L_SAFEBOUND_SUPPORTED == TRUE) || (CFG_S2L_SS1T_SUPPORTED == TRUE))
  /* Safe2Link Remote/Pure IO */
  #define VERSION_SW_VERSION_PATCH   ((UINT8)108u)
#else
  /* T100/CS */
  #define VERSION_SW_VERSION_PATCH   ((UINT8)8u)
#endif
#define VERSION_SW_VERSION         ((UINT32)((UINT32)VERSION_SW_VERSION_PATCH \
                                  + ((UINT32)VERSION_SW_VERSION_MINOR << 8u) \
                                  + ((UINT32)VERSION_SW_VERSION_MAJOR << 16u)))

/*! This constant defines the flash address of the software version. (see [SRS_2242]) */
#define VERSION_SW_VERSION_FLASH_ADDR 0x0803bff8uL


/*------------------------------------------------------------------------------
** defines for fixed numbers and IDs which identify the SW / the module
**------------------------------------------------------------------------------
*/
/* ASM Vendor Id: Identifies vendor of the ASM module. 0x0001  HMS  */
#define VERSION_ASM_VENDOR_ID       0x0001u
/* ASM Module Id:  Identifies the kind of ASM */
#define VERSION_ASM_MODULE_ID       0x0113u

/*! This constant defines the flash address of the unique serial number */
#define VERSION_SERIAL_NUM_FLASH_ADDR 0x0803F000uL

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



#endif   /* VERSION_H */

/*******************************************************************************
**
** End of version.h
**
********************************************************************************
*/

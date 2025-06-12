/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doInt.c
**
** $Id: doInit.c 4067 2023-09-18 12:08:41Z ankr $
** $Revision: 4067 $
** $Date: 2023-09-18 14:08:41 +0200 (Mo, 18 Sep 2023) $
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
** main initialization of DO-Module, calls initialization functions
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

/* Module header */

#include "cfg_Config-sys.h"

/* following include needed for the RDS declarations  */
#include "rds.h"

#include "gpio-hal.h"

#include "gpio_cfg.h"
#include "doCfg.h"
#include "diDoDiag.h"
#include "doPortMap.h"
#include "doState.h"
#include "doSetGetPin.h"
#include "doDiag.h"
#include "doErrHdl.h"
#include "doLib.h"
#include "doInit.h"


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
 ** doInit_InitDoModule()
 **
 ** Description:
 **    function for initialization/configuration of DO module:
 **    calls initialization functions
 **
 **------------------------------------------------------------------------------
 ** Inputs:
 **    - None -
 **
 ** Register Setting:
 **    - None -
 **
 ** Return:
 **    - void -
 **
 ** Usage:
 **    doInit_InitDoModule();
 **
 ** Module Test:
 **    - NO -
 **    Reason: Only function calls
 **------------------------------------------------------------------------------
 */
void doInit_InitDoModule (void)
{
   /* call initialisation of state machine */
   doState_Init();

   /* call initialization of DO-Test module */
   doDiag_Init();

   /* call initialisation of DO error handler */
   doErrHdl_Init();

   /* call initialisation of DO lib module */
   doLib_Init();
}


/*------------------------------------------------------------------------------
 **
 ** doInit_InitDoPorts()
 **
 ** Description:
 **    function for initialization/configuration of DO port/pins:
 **    calls initialization function
 **
 **------------------------------------------------------------------------------
 ** Inputs:
 **    - None -
 **
 ** Register Setting:
 **    - None -
 **
 ** Return:
 **    - void -
 **
 ** Usage:
 **    doInit_InitDoPorts();
 **
 ** Module Test:
 **    - NO -
 **    Reason: Only function calls
 **------------------------------------------------------------------------------
 */
void doInit_InitDoPorts (void)
{
   /* call initialisation DO pin module */
   doSetGetPin_Init();

   /* call initialization of Ports */
   doPortMap_Init();
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of doInit.c
**
********************************************************************************
*/

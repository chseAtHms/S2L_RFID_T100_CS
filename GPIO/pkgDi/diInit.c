/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diInit.c
**
** $Id: diInit.c 4086 2023-09-27 15:49:53Z ankr $
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
** main initialization of DI-Module, calls initialization functions
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
#include "rds.h"

/* Header-file of module */
#include "cfg_Config-sys.h"
#include "gpio_cfg.h"
#include "diCfg.h"
#include "diDoDiag.h"
#include "diPortMap.h"
#include "diState.h"
#include "diDiag.h"
#include "diErrHdl.h"
#include "diInput.h"
#include "diLib.h"

#include "diInit.h"


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
** diInit_InitDiModule()
**
** Description:
** function for initialization/configuration of DI module:
** calls initialization functions
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
**    diInit_InitDiModule();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void diInit_InitDiModule(void)
{
   /* call initialisation of state machine */
   diState_Init();

   /* call initialization of Diag/Test module */
   diDiag_Init();

   /* call initialization of error handler */
   diErrHdl_Init();

   /* get first input values */
   diInput_Init();

   /* init lib module */
   diLib_Init();
}


/*------------------------------------------------------------------------------
**
** diInit_InitDiPorts()
**
** Description:
** function for initialization/configuration of DI ports/pins:
** calls initialization function
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
**    diInit_InitDiPorts();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void diInit_InitDiPorts(void)
{
   /* call initialization of Ports */
   diPortMap_Init();
}

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of diInit.c
**
********************************************************************************
*/

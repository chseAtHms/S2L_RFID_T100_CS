/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doCfg.c
**
** $Id: doCfg.c 2448 2017-03-27 13:45:16Z klan $
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
** gets the configuration/parameterization data for outputs
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
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "gpio_cfg.h"
#include "doCfg.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** constant array to get the number of the corresponding other channel without
** calculations.
** In case of safe outputs there is in current HW only one, so intention is
** to avoid complicated calculations in future HW with more outputs.
**------------------------------------------------------------------------------
*/
const UINT8 docfg_au8DoOtherChNum[GPIOCFG_NUM_DO_TOT] =
{
   1u,   /* corresponding second channel for safe output "0" is channel "1" */
   0u    /* corresponding second channel for safe output "1" is channel "0" */
};

/*------------------------------------------------------------------------------
** translation of DI numbers to the number of dual-channel DIs
**------------------------------------------------------------------------------
*/
CONST UINT8 doCfg_u8DoDualNum[GPIOCFG_NUM_DO_TOT] =
{
   (UINT8) 0u,  /* first  DO is part of Dual-Ch-DO 0 */
   (UINT8) 0u,  /* second DO is part of Dual-Ch-DO 0 */
};

#if GPIOCFG_NUM_DO_TOT > 2u
/* RSM_IGNORE_QUALITY_BEGIN Notice #1    - Physical line length > 100 characters */
  #error "array < docfg_au8DoOtherChNum > and < diCfg_u8DiDualNum > must be adapted to new DO number!"
/* RSM_IGNORE_QUALITY_END */
#endif

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


/*******************************************************************************
**
** End of doCfg.c
**
********************************************************************************
*/

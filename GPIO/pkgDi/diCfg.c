/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diCfg.c
**
** $Id: diCfg.c 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** configuration for DI module
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


/* Header-file of module */
#include "gpio_cfg.h"
#include "diCfg.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** constant array to get the number of the other channel of a DI.
** sense: get number of other channel without time consuming
**        calculation (like " ((diNum & 0xFE)*2)+1)")
** Access is not done directly, but via macro "DICFG_GET_OTHER_CHANNEL"
**------------------------------------------------------------------------------
*/
CONST UINT8 diCfg_u8ConsDiOtherChNum[GPIOCFG_NUM_DI_TOT] =
{
   1u,
   0u,
   3u,
   2u,
   5u,
   4u
};

/*------------------------------------------------------------------------------
** translation of DI numbers to the number of dual-channel DIs
**------------------------------------------------------------------------------
*/
CONST UINT8 diCfg_u8DiDualNum[GPIOCFG_NUM_DI_TOT] =
{
   (UINT8) 0u,  /* first  DI is part of Dual-Ch-DI 0 */
   (UINT8) 0u,  /* second DI is part of Dual-Ch-DI 0 */
   (UINT8) 1u,  /* third  DI is part of Dual-Ch-DI 1 */
   (UINT8) 1u,  /* fourth DI is part of Dual-Ch-DI 1 */
   (UINT8) 2u,  /* fifth  DI is part of Dual-Ch-DI 2 */
   (UINT8) 2u,  /* sixt   DI is part of Dual-Ch-DI 2 */
};

#if GPIOCFG_NUM_DI_TOT > 6u   /* if the defined number of inputs was changed, send hint! */
  #error "array < diCfg_u8ConsDiOtherChNum > and < u8DiDualNum> must be adapted to new DI number!"
#endif


/*------------------------------------------------------------------------------
** translation of DI numbers of the pins of one uC to the DI-number used
** in whole device
**------------------------------------------------------------------------------
*/CONST UINT8 diCfg_u8DiNumC1[GPIOCFG_NUM_DI_PER_uC] =
{
   (UINT8) 0u,  /* first DI of uC1 is DI0 */
   (UINT8) 2u,  /* second DI of uC1 is DI2 */
   (UINT8) 4u,  /* third DI of uC1 is DI4 */
};

CONST UINT8 diCfg_u8DiNumC2[GPIOCFG_NUM_DI_PER_uC] =
{
   (UINT8) 1u,  /* first DI of uC2 is DI1 */
   (UINT8) 3u,  /* second DI of uC2 is DI3 */
   (UINT8) 5u,  /* third DI of uC3 is DI5 */
};

#if GPIOCFG_NUM_DI_PER_uC > 3u   /* if the defined number of inputs was changed, send hint! */
  #error "array < diCfg_u8DiNumC1 > and < diCfg_u8DiNumC2 > must be adapted to new DI number!"
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
** End of diCfg.c
**
********************************************************************************
*/

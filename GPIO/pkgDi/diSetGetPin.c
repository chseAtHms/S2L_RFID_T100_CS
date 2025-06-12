/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diSetGetPin.c
**
** $Id: diSetGetPin.c 4086 2023-09-27 15:49:53Z ankr $
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
** functions for Port-Pin access, like setting/getting diagnostic outputs/inputs
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
#include "gpio-hal.h"

/* Header-file of module */
#include "gpio_cfg.h"
#include "diCfg.h"
#include "diPortMap.h"
#include "diSetGetPin.h"



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
** diSetGet_SetTiLevelPin()
**
** Description:
**    set a the corresponding TI_Level pin of a dedicated input to HIGH or LOW.
**    DI0 located at uC1 is tested with TI0 of uC2,
**    DI1 located at uC2 is tested with TI0 of uC1,
**    DI2 located at uC1 is tested with TI1 of uC2,
**    DI3 located at uC2 is tested with TI1 of uC1,
**    and so on...
**------------------------------------------------------------------------------
** Inputs:
**    - u8DiNum: number/id of input
**    - eVal: value (IO_HIGH or IO_LOW)
**
** Outputs:
**    - None -
**
** Usage:
**    diSetGet_SetTiLevelPin(x, IO_HIGH);
**
** Module Test:
**    - NO -
**    Reason: low complexity, calculation easy to understand
**------------------------------------------------------------------------------
*/
void diSetGet_SetTiLevelPin(CONST UINT8 u8DiNum, CONST GPIO_STATE_ENUM eVal)
{
   UINT8 u8Tmp;

   /* "u8DiNum/=2" because test for u8DiNum=0 is done with TI_LEVEL0 of uC2, test of
    *  u8DiNum=1 is done with TI_LEVEL0 of uC1 and so on...  */
   u8Tmp = u8DiNum >> 1u;

   /* check parameter now (also to check later array access), parameter u8diNum
    * divided by two may not exceed "GPIOCFG_NUM_DI_PER_uC" */
   GLOBFAIL_SAFETY_ASSERT( (u8Tmp < GPIOCFG_NUM_DI_PER_uC), GLOB_FAILCODE_INVALID_PARAM);

   if (eGPIO_HIGH == eVal)
   {
      gpioHAL_SetPin(diPortMap_sTiLevelMap[u8Tmp].psPort, 
                     diPortMap_sTiLevelMap[u8Tmp].u16Pinmask);
   }
   else if (eGPIO_LOW == eVal)
   {
      gpioHAL_ClearPin(diPortMap_sTiLevelMap[u8Tmp].psPort, 
                       diPortMap_sTiLevelMap[u8Tmp].u16Pinmask);
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_INVALID_PARAM);
   }

}


/*------------------------------------------------------------------------------
**
** diSetGet_SetToEnPin()
**
** Description:
**    - set a the corresponding TO pin test signal to HIGH or LOW
**    - hint: function uses structure "diPortMap_sToMap" although there is currently only
**      one TO per uC. Only Reason is "prepare for future HW" containing several
**      TOs.
**
**------------------------------------------------------------------------------
** Inputs:
**    - eVal: value (IO_HIGH or IO_LOW)
**
** Outputs:
**    - None -
**
** Usage:
**    diSetGet_SetToEnPin(IO_HIGH);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diSetGet_SetToEnPin(CONST GPIO_STATE_ENUM eVal)
{
   if (eGPIO_HIGH == eVal)
   {
      gpioHAL_SetPin(diPortMap_sToMap[0].psPort, diPortMap_sToMap[0].u16Pinmask);
   }
   else if (eGPIO_LOW == eVal)
   {
      gpioHAL_ClearPin(diPortMap_sToMap[0].psPort, diPortMap_sToMap[0].u16Pinmask);
   }
   else
   {
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_INVALID_PARAM);
   }
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/



/*******************************************************************************
**
** End of diSetGetPin.c
**
********************************************************************************
*/

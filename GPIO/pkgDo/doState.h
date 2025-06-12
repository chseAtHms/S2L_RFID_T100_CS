/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doState.h
**
** $Id: doState.h 4436 2024-05-17 14:28:50Z ankr $
** $Revision: 4436 $
** $Date: 2024-05-17 16:28:50 +0200 (Fr, 17 Mai 2024) $
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
** header file (public) of doState.c
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013-2024 HMS Industrial Networks AB            **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef DOSTATE_H_
#define DOSTATE_H_


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

/*------------------------------------------------------------------------------
** This enum contains the possible DO states
**------------------------------------------------------------------------------
*/
typedef enum
{
   eDO_STATE_INACTIVE    = 0x468au,
   eDO_STATE_ACTIVE      = 0x471du,
   eDO_STATE_ERROR       = 0x488bu,
   eDO_STATE_DONT_USE    = 0x491cu
} DO_STATE_ENUM;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** array containing:
** requested state coming from SPDU, in case of dual channel only one bit per
** channel is valid.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "doState.c", "doLib.c" and doSafeBoundSS1t.c.
**------------------------------------------------------------------------------
*/
extern GPIO_STATE_ENUM doState_DoReqSPDU[GPIOCFG_NUM_DO_TOT];

/*------------------------------------------------------------------------------
** array containing:
** requested error reset requests coming from SPDU.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "doState.c" and "doLib.c".
**------------------------------------------------------------------------------
*/
extern TRUE_FALSE_ENUM doState_DoErrRstSPDU[GPIOCFG_NUM_DO_TOT];

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void doState_ProcessDOs(void);
extern void doState_ProcessDOsLight(void);
extern TRUE_FALSE_ENUM doState_ActiveCycTillTestReached(CONST UINT8 u8DoNum);
extern void doState_Init(void);
extern void doState_SwitchDOsOff(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doState.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doSetGetPin.h
**
** $Id: doSetGetPin.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "doSetGetPin.c", containing the interface for other SW
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

#ifndef DOSETGETSIGNAL_H_
#define DOSETGETSIGNAL_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** wait cycles to set DO and Discharge pin in dependences of each other
** Note: values affected by calling frequency / scheduler!
**------------------------------------------------------------------------------
*/
/* waiting time to set discharge Pin LOW (LOW active, LOW will discharge.
 * Discharge only allowed when DO is already LOW for a time x), Unit: 400us */
#define DOSETGETPIN_DISC_LOW_WAIT  ((UINT8)1u)

/* time that the discharge Pin will be kept in Discharge Mode "LOW"
 * (used when DO is OFF/LOW to disable Discharge Mode again and reset
 * Discharge Pin to its default value HIGH), Unit: 400us */
#define DOSETGETPIN_DISC_LOW_TIME  ((UINT8)1u)

/* waiting time to set DO HIGH (the DISC pin shall be HIGH a time y before
 * setting DO HIGH), Unit: 400us */
#define DOSETGETPIN_DO_HIGH_WAIT   ((UINT8)1u)


/*! This compiler switch deactivates the discharge pin in debug mode to
 * avoid damage on HW e.g. caused by break points when uC is stopped just when
 * the other uC enables discharge pin */
#ifdef __DEBUG__
#define USE_DISCHARGE   FALSE
#else
#define USE_DISCHARGE   TRUE
#endif


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

extern void doSetGetPin_Init(void);
extern void doSetGetPin_SetDoPin(CONST UINT8 u8doNum, CONST GPIO_STATE_ENUM eVal);
extern void doSetGetPin_SetDisChrgPin(CONST UINT8 u8doNum, CONST GPIO_STATE_ENUM eVal);
extern GPIO_STATE_ENUM doSetGetPin_GetDisChrgPin(CONST UINT8 u8doNum);
extern void doSetGetPin_SetPin(GPIO_TypeDef* sGpio, CONST UINT16 u16PinMask, 
                               CONST GPIO_STATE_ENUM eVal);
extern void doSetGetPin_SwitchDoPinOff(CONST UINT8 u8doNum);
extern void doSetGetPin_SwitchDoPinOn(CONST UINT8 u8doNum);
extern void doSetGetPin_CheckPinState(CONST UINT8 u8doNum);
extern void doSetGetPin_RstDisDo(CONST UINT8 u8doNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doSetGetSignal.h
**
********************************************************************************
*/

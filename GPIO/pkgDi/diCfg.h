/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diCfg.h
**
** $Id: diCfg.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "diCfg.c".
** Contains defines for the Digital Input module.
** Contains interface Macros and typedefs to get configuration data.
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

#ifndef DICFG_H_
#define DICFG_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** define to set digital safe input pins "active-high" or "active-low",
** see [SRS_102]
**------------------------------------------------------------------------------
*/
#define DICFG_DI_ACTIVE     (GPIO_STATE_ENUM)eGPIO_LOW
#define DICFG_DI_INACTIVE   (GPIO_STATE_ENUM)eGPIO_HIGH

/*------------------------------------------------------------------------------
** define to set TI-Level pins "active-high" or "active-low"
** (default: opposite to digital inputs)
**------------------------------------------------------------------------------
*/
#define DICFG_TILEVEL_ACTIVE     DICFG_DI_INACTIVE
#define DICFG_TILEVEL_INACTIVE   DICFG_DI_ACTIVE

/*------------------------------------------------------------------------------
** define to set TO (Test-Output) pins. TO pins are LOW-active.
**------------------------------------------------------------------------------
*/
#define DICFG_TO_ACTIVE     (GPIO_STATE_ENUM)eGPIO_LOW
#define DICFG_TO_INACTIVE   (GPIO_STATE_ENUM)eGPIO_HIGH


/*------------------------------------------------------------------------------
** constant array to get the number of the other channel of a DI.
** sense: get number of other channel without time consuming
**        calculation (like " ((diNum & 0xFE)*2)+1)")
** Access is not done directly, but via macro "DICFG_GET_OTHER_CHANNEL"
**------------------------------------------------------------------------------
*/
extern CONST UINT8 diCfg_u8ConsDiOtherChNum[GPIOCFG_NUM_DI_TOT];

/*------------------------------------------------------------------------------
** translation of DI numbers to the number of dual-channel DIs, see also [SRS_195]
**------------------------------------------------------------------------------
*/
extern CONST UINT8 diCfg_u8DiDualNum[GPIOCFG_NUM_DI_TOT];

/*------------------------------------------------------------------------------
** translation of DI numbers of the pins of one uC to the DI-number used
** in whole device, see also [SRS_195] and [SRS_592]
**------------------------------------------------------------------------------
*/
extern CONST UINT8 diCfg_u8DiNumC1[GPIOCFG_NUM_DI_PER_uC];
extern CONST UINT8 diCfg_u8DiNumC2[GPIOCFG_NUM_DI_PER_uC];


/*******************************************************************************
**
** Data Types
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
**  Macro to get / returning the number of the other channel of a dual
**  input channel.
**  Makro is used due to performance reasons(time), the input "diNum" is
**  checked inside the function using this macro.
**------------------------------------------------------------------------------
*/
#define DICFG_GET_OTHER_CHANNEL(diNum)    (diCfg_u8ConsDiOtherChNum[diNum])

/*------------------------------------------------------------------------------
**  Macro to get the number of the dual channel, where a DI belongs to
**------------------------------------------------------------------------------
*/
#define DICFG_GET_DUAL_CH_NUM(diNum)    (diCfg_u8DiDualNum[diNum])

/*------------------------------------------------------------------------------
**  Macro to get the translation of DI numbers of the pins of one uC to the
**  DI-number used in whole device
**------------------------------------------------------------------------------
*/
/* get the DI number of a DI located at uC1 with given Dual-Input number */
#define DICFG_GET_DI_NUM_C1(diNum)    (diCfg_u8DiNumC1[diNum])
/* get the DI number of a DI located at uC2 with given Dual-Input number */
#define DICFG_GET_DI_NUM_C2(diNum)    (diCfg_u8DiNumC2[diNum])

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diCfg.h
**
********************************************************************************
*/

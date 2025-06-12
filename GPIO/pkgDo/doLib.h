/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doLib.h
**
** $Id: doLib.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "doLib.c"
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

#ifndef DOLIB_H_
#define DOLIB_H_


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


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** variable containing test qualifiers to be send via IPC
** Global used to time critical IPC functionality.
**------------------------------------------------------------------------------
*/
extern UINT8 dolib_u8TestQualValues;

/*------------------------------------------------------------------------------
** variable be filled by IPC, containing test qualifiers from other channel
** Global used to time critical IPC functionality.
**------------------------------------------------------------------------------
*/
extern UINT8 dolib_u8TestQualValuesOtherCh;

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void doLib_ToggleLifeAndScCcSelect(void);

extern void doLib_PrepareTestQualForIpc(void);
extern void doLib_ConvValFromOtherCh(void);

extern void doLib_SetPSDoReq(CONST UINT8 u8DoReqByte);
extern void doLib_PassAllDOs(void);

extern void doLib_Init(void);
extern void doLib_CheckDoDiagTimeout(void);
extern void doLib_SetErrRstFlags(CONST UINT8 u8DoErrRstByte);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doLib.h
**
********************************************************************************
*/

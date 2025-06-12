/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiag.h
**
** $Id: diDiag.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "diDiag.c"
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

#ifndef DIDIAG_H_
#define DIDIAG_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
/* This constant defines the timeout for the DI diagnostic, unit microseconds. */
#define DIDIAG_TIMEOUT  ((UINT32)1800000000u) /* us --> 30 min */

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

extern void diDiag_Init(void);
extern void diDiag_HandleLevTestStates(CONST UINT8 u8DiNum);
extern void diDiag_HandleConsTest(CONST UINT8 u8DiNum);
extern void diDiag_HandleToTestExecution(void);
extern void diDiag_RstTestStates(CONST UINT8 u8DiNum);

extern BOOL diDiag_IsAnyTestOngoing(void);
extern BOOL diDiag_IsTestAtPinOngoing(CONST UINT8 u8DiNum);
extern BOOL diDiag_FirstLevelTestFinished(CONST UINT8 u8DiNum);
extern void diDiag_CheckDiagTimeout(CONST UINT8 u8DiNum);
extern void diDiag_HandleToTestState(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDiag.h
**
********************************************************************************
*/

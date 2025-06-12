/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiag.h
**
** $Id: doDiag.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "doDiag.c"
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

#ifndef DODIAG_H_
#define DODIAG_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
/* This constant defines the timeout for the DO diagnostic, unit microseconds. */
#define DODIAG_TIMEOUT  ((UINT32)1800000000u) /* us --> 30 min */

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

extern void doDiag_Init(void);
extern BOOL doDiag_IsAnyTestOngoing(void);
extern BOOL doDiag_IsVsupTestOngoing(void);
extern BOOL doDiag_IsTestAtPinOngoing(CONST UINT8 u8DoNum);

extern void doDiag_HandleDoTestStates(CONST UINT8 u8DoNum);

extern void doDiag_RstDoTestStates(CONST UINT8 u8DoNum);
extern void doDiag_CheckDiagVsupTimeout(void);
extern void doDiag_CheckDiagHSTimeout(CONST UINT8 u8DoNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doDiag.h
**
********************************************************************************
*/

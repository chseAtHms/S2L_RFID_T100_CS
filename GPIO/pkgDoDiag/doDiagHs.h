/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiagHs.h
**
** $Id: doDiagHs.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "doDiagHs.c"
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

#ifndef DODIAGHS_H_
#define DODIAGHS_H_


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
** structure containing test state and counters for the different DOs,
** HS-Test.
** This global variable is not accessed directly in other modules, but only
** via the macros "DO_DIAG_HS_TEST_ONGOING" defined below in this file.
** Safe IO is time critical, therefore this "macro"-style was chosen instead of
** functions.
**------------------------------------------------------------------------------
*/
extern DIDODIAG_DO_DIAGSTATE_STRUCT doDiagHs_sState;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

#define DO_DIAG_HS_TEST_ONGOING(doNum)  ( (eDIAG_EXEC_1ST == doDiagHs_sState.eTestState[doNum]) || \
                                          (eDIAG_EXEC_CYC == doDiagHs_sState.eTestState[doNum]) )

extern void doDiagHs_Init(void);
extern void doDiagHs_HandleTestState(CONST UINT8 u8DoNum);
extern void doDiagHs_RstTestState(CONST UINT8 u8DoNum);
extern void doDiagHs_RstTestExecutedVar(CONST UINT8 u8DoNum);
extern TRUE_FALSE_ENUM doDiagHs_WasTestExecuted(CONST UINT8 u8DoNum);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doDiagHs.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagLev.h
**
** $Id: diDiagLev.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "diDiagLev.c"
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

#ifndef DIDIAGLEV_H_
#define DIDIAGLEV_H_


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
** state structure of DI-Level-Test.
** This global variable is not accessed directly in other modules, but only
** via the macros "DI_DIAG_LEV_TEST_ONGOING" and "DI_DIAG_LEV_FIRST_TEST_FINISHED"
** below in this file. Safe IO is time critical, therefore Function-Like macros
** have been chosen instead of functions.
**-----------------------------------------------------------------------------*/
extern DIDODIAG_DI_DIAGSTATE_STRUCT diDiagLev_sState;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Macro to distinguish if level-test is currently ongoing at a dedicated
** input pin
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_TEST_ONGOING(diNum) ((eDIAG_EXEC_1ST == diDiagLev_sState.eTestState[diNum]) \
                                        || (eDIAG_EXEC_CYC == diDiagLev_sState.eTestState[diNum]))

/*------------------------------------------------------------------------------
** Macro to distinguish if first level-test is already finished,
** used for [SRS_231], [SRS_470]
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_FIRST_TEST_FINISHED(diNum) \
                                        ((eDIAG_WAIT_CYC == diDiagLev_sState.eTestState[diNum])  \
                                       || (eDIAG_REQ_CYC == diDiagLev_sState.eTestState[diNum]) \
                                       || (eDIAG_EXEC_CYC == diDiagLev_sState.eTestState[diNum]))


extern void diDiagLev_Init(void);
extern void diDiagLev_HandleTestState(CONST UINT8 u8DiNum);
extern void diDiagLev_RstTestState(CONST UINT8 u8DiNum);
extern TRUE_FALSE_ENUM diDiagLev_WasTestExecuted (CONST UINT8 u8DiNum);
extern void diDiagLev_RstTestExecutedVar(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDiagLev.h
**
********************************************************************************
*/

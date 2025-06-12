/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagTo_priv.h
**
** $Id: diDiagTo_priv.h 2448 2017-03-27 13:45:16Z klan $
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
** local include file of "diDiagTo.c", contains static and file wide definitions
** which are not public
**
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 und STYL-2-3-20.
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

#ifndef DIDIAGTO_PRIV_H_
#define DIDIAGTO_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for cyclic TO test in DI state
** "Active and valid"
** example: if "diDiag_HandleLevelTestState" called every 400us and define=50000,
**          cyclic test will be executed every 20s
**-----------------------------------------------------------------------------*/
#define DI_DIAG_TO_CYC_TOTEST_WAITTIME  (UINT32)2500

/*------------------------------------------------------------------------------
** timing definitions for the test execution of TO-Test,
** timing base is depending on calling frequency, nominal value is 400us
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** start of test, set test pulse
**------------------------------------------------------------------------------
*/
#define DI_DIAG_TO_START_TEST        DI_DIAG_TO_CYC_TOTEST_WAITTIME
/*------------------------------------------------------------------------------
** time until reset of test pulse (incl. offset which is part of iParameter,
** see [SRS_277])
**------------------------------------------------------------------------------
*/
#define DI_DIAG_TO_RESET_PULSE   ((UINT32)(DI_DIAG_TO_START_TEST + 1u + (FIPARAM_DI_TO_TEST_PARAM)))


#define DI_DIAG_TO_STOP_TEST     ((UINT32)(DI_DIAG_TO_RESET_PULSE + 1u))

/*******************************************************************************
**
** Data Types
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
** Private Services
**
********************************************************************************
*/

STATIC void diDiagTo_ExecTest(CONST UINT8 u8ChNum);
STATIC void diDiagTo_ExecTestAtOwnCh(CONST UINT8 u8ChNum);
STATIC void diDiagTo_ExecTestAtOtherCh(CONST UINT8 u8ChNum);
STATIC void diDiagTo_EvalTestResultOwnCh(CONST UINT8 u8DiNum);
STATIC void diDiagTo_SetTOsDefault(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDiagTo_priv.h
**
********************************************************************************
*/

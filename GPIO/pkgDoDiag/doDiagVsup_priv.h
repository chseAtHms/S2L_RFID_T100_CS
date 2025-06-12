/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiagVsup_priv.h
**
** $Id: doDiagVsup_priv.h 2448 2017-03-27 13:45:16Z klan $
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
** local include file of "doDiagVsup.c", contains static and file wide definitions
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

#ifndef DODIAGVSUP_PRIV_H_
#define DODIAGVSUP_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for cyclic VSUP test
** [Unit] depending on call cycle of "doDiagVsup_HandleTestState"
** example: if "doDiagVsup_HandleTestState" called every 400us and
**          define = 1250, cyclic test will be executed every 0.5s
** VSUP test shall be executed every 0.5s. Motivation: Since every diagnostic
** function shall be executed every 1s and the VSUP test consists of two test
** steps, one test step has to be executed every 0.5s in order to achieve 1s
** cycle time for the whole VSUP test.
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_CYC_TEST_WAITTIME (((UINT32)1250) - DIDODIAG_STATE_CYC_OFFSET)

/*------------------------------------------------------------------------------
** start time of VSUP test
** [Unit] depending on call cycle of "doDiagVsup_HandleTestState"
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_TEST_START \
          (UINT32)(DIDODIAG_DIDO_TEST_CNT_RESET_VAL + DIDODIAG_STATE_CYC_OFFSET)

/*------------------------------------------------------------------------------
** time to activate the discharge pins, this time should be at least 350us
** later then test start
** [Unit] depending on call cycle of "doDiagVsup_HandleTestState"
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_TEST_DISCHARGE (UINT32)(DO_DIAG_VSUP_TEST_START + 1u)

/*------------------------------------------------------------------------------
** time to read result of VSUP test, this time should be at least 50us
** later then activating the discharge pins
** [Unit] depending on call cycle of "doDiagVsup_HandleTestState"
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_TEST_READVAL \
          (UINT32)((DO_DIAG_VSUP_TEST_DISCHARGE + 1u) + FIPARAM_DO_TEST_OFFSET(0u))

/*------------------------------------------------------------------------------
** time to end VSUP test, see [SRS_640] (leave test active state), this time only
** keeps the test active for at least 300us more, reason: life signal shall be
** enabled 300us after test finished, not earlier
** [Unit] depending on call cycle of "doDiagVsup_HandleTestState"
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_TEST_END      (UINT32)(DO_DIAG_VSUP_TEST_READVAL + 1u)


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
** Private Services
**
********************************************************************************
*/

STATIC void doDiagVsup_CheckCycTestStart(void);
STATIC void doDiagVsup_ExecTest(void);

STATIC void doDiagVsup_ExecTestStep1C1(void);
STATIC void doDiagVsup_ExecTestStep1C2(void);
STATIC void doDiagVsup_ExecTestStep2C1(void);
STATIC void doDiagVsup_ExecTestStep2C2(void);

STATIC void doDiagVsup_EvalTestResStep1(void);
STATIC void doDiagVsup_EvalTestResStep2(void);

STATIC void doDiagVsup_RstTestState(void);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doDiagVsup_priv.h
**
********************************************************************************
*/

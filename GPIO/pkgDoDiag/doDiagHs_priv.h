/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiagHs_priv.h
**
** $Id: doDiagHs_priv.h 2448 2017-03-27 13:45:16Z klan $
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
** local include file of "doDiagHs.c", contains static and file wide definitions
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

#ifndef DODIAGHS_PRIV_H_
#define DODIAGHS_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for first Level test after output "Active"
**------------------------------------------------------------------------------
*/
#define DO_DIAG_HS_1ST_TEST_WAITTIME (UINT32)200

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for cyclic Level test after out "Active"
** example: if "diDiag_HandleLevelTestState" called every 400us and define=50000, 
** cyclic test will be executed every 20s
**------------------------------------------------------------------------------
*/
#define DO_DIAG_HS_CYC_TEST_WAITTIME (UINT32)2500   

/*------------------------------------------------------------------------------
** timing definitions for the test execution of Highside-Switch-Test,
** timing base is depending on calling frequency, nominal value is 200us
**------------------------------------------------------------------------------
*/
/*------------------------------------------------------------------------------
** start of test, switch output off
**-----------------------------------------------------------------------------*/
#define DO_DIAG_HS_START \
          (UINT8)(DIDODIAG_DIDO_TEST_CNT_RESET_VAL + DIDODIAG_STATE_CYC_OFFSET)
/*------------------------------------------------------------------------------
** second step of test, set discharge pin
**-----------------------------------------------------------------------------*/
#define DO_DIAG_HS_DISCHARGE  \
          (UINT8)(DO_DIAG_HS_START + 1u)
/*------------------------------------------------------------------------------
** end of test, evaluate test result
**-----------------------------------------------------------------------------*/
#define DO_DIAG_HS_READVAL(doNum) \
          (UINT8)((DO_DIAG_HS_DISCHARGE + 1u) + FIPARAM_DO_TEST_OFFSET(doNum))


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

STATIC void doDiagHs_Check1stTestStart(UINT8 u8DoNum);
STATIC void doDiagHs_CheckCycTestStart(UINT8 u8DoNum);

STATIC void doDiagHs_ExecTest(UINT8 u8DoNum);
STATIC void doDiagHs_ExecTestAtOwnCh (UINT8 u8DoNum);
STATIC void doDiagHs_ExecTestAtOtherCh (UINT8 u8DoNum);

STATIC void doDiagHs_EvalTestRes (UINT8 u8DoNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doDiagHs_priv.h
**
********************************************************************************
*/

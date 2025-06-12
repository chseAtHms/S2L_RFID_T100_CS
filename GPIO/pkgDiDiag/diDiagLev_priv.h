/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagLev_priv.h
**
** $Id: diDiagLev_priv.h 569 2016-08-31 13:08:08Z klan $
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
** local include file of "diDiagLev.c", contains static and file wide definitions
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

#ifndef DIDIAGLEV_PRIV_H_
#define DIDIAGLEV_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for first Level test after input "Active"
** Note: value affected by calling frequency / scheduler!
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_1ST_LEVTEST_WAITTIME  ((UINT32)1)

/*------------------------------------------------------------------------------
** timing (in call cycles) to wait for cyclic Level test after input "Active"
** Note: value affected by calling frequency / scheduler!
** example: if "diDiag_HandleLevelTestState" called every 200us and define=50000,
 *          cyclic test will be executed every 10
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_CYC_LEVTEST_WAITTIME  ((UINT32)2500)

/*------------------------------------------------------------------------------
** timing definitions for the test execution of Level-Test,
** timing base is depending on calling frequency, nominal value is 200us
** Note: value affected by calling frequency / scheduler!
**-----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
** start of test, set test pulse
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_SET_PULSE    ((UINT8)0u)
/*------------------------------------------------------------------------------
** evaluation of test result, read back test pulse
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEV_READBACK     ((UINT8)(DI_DIAG_LEV_SET_PULSE + 1u))
/*------------------------------------------------------------------------------
** end of test, including check for possible short to other safe inputs
**-----------------------------------------------------------------------------*/
#define DI_DIAG_LEVEL_END        ((UINT8)(DI_DIAG_LEV_READBACK + 1u))


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

STATIC void diDiagLev_Check1stTestStart(CONST UINT8 u8DiNum);
STATIC void diDiagLev_CheckCycTestStart(CONST UINT8 u8DiNum);
STATIC void diDiagLev_ExecTest(CONST UINT8 u8DiNum);
STATIC void diDiagLev_ExecTestAtOwnCh(CONST UINT8 u8DiNum);
STATIC void diDiagLev_ExecTestAtOtherCh(CONST UINT8 u8DiNum);
STATIC void diDiagLev_EvalTestResultOwnCh(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDiagLev_priv.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doDiag.h
**
** $Id: doDiagVsup.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "doDiagVsup.c"
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

#ifndef DODIAGVSUP_H_
#define DODIAGVSUP_H_


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

/*------------------------------------------------------------------------------
** This enum specifies the possible DO Voltage Supervision test states
**------------------------------------------------------------------------------
*/
typedef enum
{
   eDO_DIAG_VSUP_WAIT   = 0x4ba5u,
   eDO_DIAG_VSUP_REQ    = 0x4c6eu,
   eDO_DIAG_VSUP_EXEC   = 0x4df9u
} DO_DIAG_VSUP_STATE_ENUM;


/*------------------------------------------------------------------------------
** typedef for structure containing:
** - test-state of inputs (test states are sub-states of DI state test)
** - counter to enter test state
**------------------------------------------------------------------------------
*/
typedef struct DO_DIAG_VSUP_STATE_TAG
{
   /* state of TO (clock output) test */
   DO_DIAG_VSUP_STATE_ENUM eTestState;      
   /* timer/counter for TO (clock output) test timing (one TO for every uC, but read back at 
   ** all DI-C inputs) */
   RDS_UINT32  u32TestCnt;                           
} DO_DIAG_VSUP_STATE_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing test state and counter for VSUP test.
** This global variable is not accessed directly in other modules, but only
** via the macro "DO_DIAG_VSUP_TEST_ONGOING" defined below in this file.
** Safe IO is time critical, therefore this "macro"-style was chosen instead of
** functions.
**------------------------------------------------------------------------------
*/
extern DO_DIAG_VSUP_STATE_STRUCT doDiagVsup_sState;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Macro: TRUE if currently the VSUP test is ongoing
**------------------------------------------------------------------------------
*/
#define DO_DIAG_VSUP_TEST_ONGOING       (eDO_DIAG_VSUP_EXEC == doDiagVsup_sState.eTestState)

extern void doDiagVsup_Init(void);
extern void doDiagVsup_HandleTestState(void);
extern TRUE_FALSE_ENUM doDiagVsup_WasTestExecuted(void);
extern void doDiagVsup_RstTestExecutedVar(void);
extern void doDiagVsup_ActivateVsupTest (void);
extern void doDiagVsup_DeactivateVsupTest (void);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doDiagVsup.h
**
********************************************************************************
*/

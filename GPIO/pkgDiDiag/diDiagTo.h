/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDiagTo.h
**
** $Id: diDiagTo.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "diDiagTo.c"
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

#ifndef DIDIAGTO_H_
#define DIDIAGTO_H_


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
** typedef for structure containing:
** - test-state of inputs (test states are sub-states of DI state machine)
** - counter to enter test state
**------------------------------------------------------------------------------
*/
typedef struct DIDIAGTO_DIAGSTATE_TAG
{
   /* state of tests */
   DIDODIAG_TEST_STATE_ENUM eTestState[GPIOCFG_NUM_DI_TOT]; 
   /* test execution (used for timeout check) */
   TRUE_FALSE_ENUM  eTestExecuted[GPIOCFG_NUM_DI_TOT];      
} DIDIAGTO_DI_DIAGSTATE_STRUCT;


/*------------------------------------------------------------------------------
** typedef for structure containing:
** - counter to enter test state
** - identifier for Test executed YES/NO
**------------------------------------------------------------------------------
*/
typedef struct DI_DIAG_TO_STATE_TAG
{
   RDS_UINT32 u32TestCnt[GPIOCFG_NUM_CHANNELS];                    /* timer/counter for tests */
   TRUE_FALSE_ENUM eTestExecutionActive[GPIOCFG_NUM_CHANNELS];
} DI_DIAG_TO_STATE_STRUCT;


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

extern void diDiagTo_Init(void);
extern void diDiagTo_HandleTestState(CONST UINT8 u8DiNum);
extern void diDiagTo_RstTestState(CONST UINT8 u8DiNum);
extern void diDiagTo_HandleTestExecution(void);
extern void diDiagTo_TestExecutionCalled (CONST UINT8 u8DiNum);
extern TRUE_FALSE_ENUM diDiagTo_WasTestExecuted (CONST UINT8 u8DiNum);
extern void diDiagTo_RstTestExecutedVar(CONST UINT8 u8DiNum);
extern TRUE_FALSE_ENUM diDiagTo_IsToTestOngoing(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDiagTo.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diDoDiag.h
**
** $Id: diDoDiag.h 2448 2017-03-27 13:45:16Z klan $
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
** common definitions used for DI and DO module and diagnostic
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

#ifndef DIDODIAG_H_
#define DIDODIAG_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** offset to correct one cycle offset between setting a state and execute this
** state in next cycle, reason: by using this offset all timings can be defined
** more clearly
**------------------------------------------------------------------------------
*/
#define DIDODIAG_STATE_CYC_OFFSET         ((UINT8)1)

/*------------------------------------------------------------------------------
** reset value for test counter
**------------------------------------------------------------------------------
*/
#define DIDODIAG_DIDO_TEST_CNT_RESET_VAL  ((UINT8)0)


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  This enum specifies the possible states of "default" test state-machines
**------------------------------------------------------------------------------
*/
typedef enum
{
   eDIAG_WAIT_1ST = 0x4ed7u,   /* wait for first test */
   eDIAG_REQ_1ST  = 0x4f40u,   /* time for first test reached, but test not started yet */
   eDIAG_EXEC_1ST = 0x506cu,   /* execution of first test */
   eDIAG_WAIT_CYC = 0x51fbu,   /* waiting for next (cyclic) test (first test OK) */
   eDIAG_REQ_CYC  = 0x52d5u,   /* time for cyclic test reached, but test not started yet */
   eDIAG_EXEC_CYC = 0x5342u    /* execution of cyclic test */
} DIDODIAG_TEST_STATE_ENUM;

/*------------------------------------------------------------------------------
**  This enum contains the available value of diagnostic test qualifier
**------------------------------------------------------------------------------
*/
typedef enum
{
   eGPIO_DIAG_NA  = 0x5489u,   /* Identifier "test not executed yet" */
   eGPIO_DIAG_OK  = 0x551eu,   /* Identifier "test OK" */
   eGPIO_DIAG_ERR = 0x5630u    /* Identifier "test not OK" */
} DIDODIAG_GPIO_DIAG_ERR_ENUM;


/*------------------------------------------------------------------------------
** typedef for structure containing:
** - test-state of inputs (test states are sub-states of DI state machine)
** - counter to enter test state
**------------------------------------------------------------------------------
*/
typedef struct DI_DIAGSTATE_TAG
{
   /* state of tests */
   DIDODIAG_TEST_STATE_ENUM eTestState[GPIOCFG_NUM_DI_TOT];      
   /* timer/counter for tests */
   RDS_UINT32 u32TestCnt[GPIOCFG_NUM_DI_TOT];                    
   /* test execution (used for timeout check) */
   TRUE_FALSE_ENUM  eTestExecuted[GPIOCFG_NUM_DI_TOT];           
} DIDODIAG_DI_DIAGSTATE_STRUCT;


/*------------------------------------------------------------------------------
** typedef for structure containing:
** - test-state of outputs (test states are sub-states of DO state machine)
** - counter to enter test state
**------------------------------------------------------------------------------
*/
typedef struct DO_DIAGSTATE_TAG
{
   /* state of TO (clock output) test */
   DIDODIAG_TEST_STATE_ENUM eTestState[GPIOCFG_NUM_DO_TOT];     
   /* timer/counter for tests */
   RDS_UINT32  u32TestCnt[GPIOCFG_NUM_DO_TOT];         
   /* test execution (used for timeout check) */   
   TRUE_FALSE_ENUM  eTestExecuted[GPIOCFG_NUM_DO_TOT];          
} DIDODIAG_DO_DIAGSTATE_STRUCT;


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

/*------------------------------------------------------------------------------
** macros checking if the number of DI/DO (in function parameter) is valid.
** Fail safe mode is entered if the DI/DO is outside the valid range.
** Currently, this range is 0-5 for DIs and 0-1 for DOs
**------------------------------------------------------------------------------
*/
#define DIDODIAG_SAFETY_CHECK_DI_NUM(diNum)  \
   GLOBFAIL_SAFETY_ASSERT( (diNum < GPIOCFG_NUM_DI_TOT), GLOB_FAILCODE_INVALID_PARAM)

#define DIDODIAG_SAFETY_CHECK_DO_NUM(doNum)  \
   GLOBFAIL_SAFETY_ASSERT( (doNum < GPIOCFG_NUM_DO_TOT), GLOB_FAILCODE_INVALID_PARAM)

/*------------------------------------------------------------------------------
** macro checking if the number of dual channel DI is valid.
** Fail safe mode is entered if the input parameter is out of this range.
** Currently, this range is 0-2 for DI (6 HW-DIs result in 3 dual-channel DIs)
**------------------------------------------------------------------------------
*/
#define DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(diDualNum)  \
   GLOBFAIL_SAFETY_ASSERT( (diDualNum < GPIOCFG_NUM_DI_PER_uC), GLOB_FAILCODE_INVALID_PARAM)

/*------------------------------------------------------------------------------
** macro checking if value is an equal number (0,2,4,6,...) or not
**------------------------------------------------------------------------------
*/
#define DIDODIAG_IS_VAL_EQUAL(value) (0u == ((UINT8)(value & 0x01u)) )


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diDoDiag.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doErrHdl.h
**
** $Id: doErrHdl.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "doErrHdl.c"
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

#ifndef DOERRHDL_H_
#define DOERRHDL_H_

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Maximum number of error counters, the number of tolerated errors is (value + 1)
** Note: resulting time defined by calling frequency / scheduler!
**-----------------------------------------------------------------------------*/
#define DOERRHDL_VSUP_ERR_CNT_LIMIT       ((UINT8)  2u)
#define DOERRHDL_HS_ERR_CNT_LIMIT         ((UINT8)  2u)
#define DOERRHDL_PIN_STATE_ERR_CNT_LIMIT  ((UINT8)250u)

/*------------------------------------------------------------------------------
** steps for counter decrase in case of successful pin state check, used to
** decrease error counter in steps > 1
**------------------------------------------------------------------------------
*/
#define DOSETGETPIN_DOSTATE_ERR_CNT_DEC   ((UINT8)5u)

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** typedef for structure containing test qualifiers for outputs (see [SRS_673])
**------------------------------------------------------------------------------
*/
typedef struct DO_ERR_TAG
{
   /* VSUP test step1 result */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eVsupTestStep1Qualifier;  
   /* VSUP test step2 result */   
   DIDODIAG_GPIO_DIAG_ERR_ENUM eVsupTestStep2Qualifier;    
   /* TO test result */   
   DIDODIAG_GPIO_DIAG_ERR_ENUM eHsTestQualifier [GPIOCFG_NUM_DO_TOT]; 
   /* pin state / DO state comparison */
   DIDODIAG_GPIO_DIAG_ERR_ENUM ePinChkQualifier [GPIOCFG_NUM_DO_TOT]; 
   /* test qualifier, one for every IO */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eDoDiagQualifier [GPIOCFG_NUM_DO_TOT]; 
} DO_ERR_STRUCT;


/*------------------------------------------------------------------------------
** typedef for structure containing test error counters (see [SRS_655])
**------------------------------------------------------------------------------
*/
typedef struct DO_ERRCNT_TAG
{
   RDS_UINT8 au8VsupTestStep1ErrCnt;                /* counter for VSUP Test step1 error */
   RDS_UINT8 au8VsupTestStep2ErrCnt;                /* counter for VSUP Test step2 error */
   RDS_UINT8 au8HsTestErrCnt[GPIOCFG_NUM_DO_TOT];   /* counter for HS Test error */
   RDS_UINT8 au8PinChkErrCnt[GPIOCFG_NUM_DO_TOT];   /* counter for Pin Check Test error */
} DOERRHDL_ERRCNT_STRUCT;

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for outputs and tests
** This global structure is not accessed directly in other modules, but only
** via the macros defined below in this file.
** Safe IO is time critical, therefore this "macro"-style was chosen instead
** of function-style.
**-----------------------------------------------------------------------------*/
extern DO_ERR_STRUCT doErrHdl_sDoErrHdl;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** macro writing the error qualifiers
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DO_ERR_SET_TEST_QUAL(doNum, val) \
          (doErrHdl_sDoErrHdl.eDoDiagQualifier[doNum] = val)

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier is OK (test executed and no error found) or
** if not all tests have been executed up to now
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DO_ERR_IS_TEST_QUAL_OK_OR_NA(doNum) \
          ((eGPIO_DIAG_OK == doErrHdl_sDoErrHdl.eDoDiagQualifier[doNum]) \
        || (eGPIO_DIAG_NA == doErrHdl_sDoErrHdl.eDoDiagQualifier[doNum]))

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier is ERROR (test executed and error found)
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DO_ERR_TEST_QUAL_ERR(doNum) \
          (eGPIO_DIAG_ERR == doErrHdl_sDoErrHdl.eDoDiagQualifier[doNum])

/*------------------------------------------------------------------------------
** macro to get the VSUP-Test result
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DO_ERR_VSUP_TEST_STEP1_RESULT  (doErrHdl_sDoErrHdl.eVsupTestStep1Qualifier)
#define DO_ERR_VSUP_TEST_STEP2_RESULT  (doErrHdl_sDoErrHdl.eVsupTestStep2Qualifier)


extern void doErrHdl_Init(void);
extern void doErrHdl_RstErr (CONST UINT8 u8DoNum);
extern void doErrHdl_rstErrCnt(CONST UINT8 u8DoNum);
extern void doErrHdl_rstErrCntHsTestPinChk(CONST UINT8 u8DoNum);

extern void doErrHdl_VsupTestStep1Failed (void);
extern void doErrHdl_VsupTestStep1Ok (void);
extern void doErrHdl_VsupTestStep2Failed (void);
extern void doErrHdl_VsupTestStep2Ok (void);

extern void doErrHdl_HsTestFailed (CONST UINT8 u8DoNum);
extern void doErrHdl_HsTestOk (CONST UINT8 u8DoNum);

extern void doErrHdl_OtherChFailed (CONST UINT8 u8DoNum);

extern void doErrHdl_PinStateCheckFailed (CONST UINT8 u8DoNum);
extern void doErrHdl_PinStateCheckOk (CONST UINT8 u8DoNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doErrHdl.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diErrHdl.h
**
** $Id: diErrHdl.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "diErrHdl.c"
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

#ifndef DIERRHDL_H_
#define DIERRHDL_H_


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
#define DIERRHDL_SHORT_ERR_CNT_LIMIT    ((UINT8)2u)
#define DIERRHDL_TO_ERR_CNT_LIMIT       ((UINT8)2u)
#define DIERRHDL_LEV_ERR_CNT_LIMIT      ((UINT8)2u)


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** typedef for structure containing test qualifiers for inputs (see [SRS_672])
**------------------------------------------------------------------------------
*/
typedef struct DI_ERR_TAG
{
   /* level test result */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eLevTestQualifier[GPIOCFG_NUM_DI_TOT];   
   /* TO test result */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eToTestQualifier[GPIOCFG_NUM_DI_TOT];    
   /* consistency test result */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eConsTestQualifier[GPIOCFG_NUM_DI_TOT];  
   /* if pin receives test pulse from other pin, this error is set */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eShortTestQualifier[GPIOCFG_NUM_DI_TOT]; 
   /* "main" test qualifier, one for every IO */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eDiDiagQualifier[GPIOCFG_NUM_DI_TOT];    
   /* temporally test qualifier for every IO, these qualifiers are set during the error
   ** handler and copied to the main test qualifiers before IPC communication */
   DIDODIAG_GPIO_DIAG_ERR_ENUM eDiDiagQualifierTmp[GPIOCFG_NUM_DI_TOT]; 
} DIERRHDL_ERR_STRUCT;


/*------------------------------------------------------------------------------
** typedef for structure containing test error counters (see [SRS_652])
**------------------------------------------------------------------------------
*/
typedef struct DI_ERRCNT_TAG
{
   RDS_UINT8  au8LevTestErrCnt[GPIOCFG_NUM_DI_PER_uC];   /* counter for consistency error */
   RDS_UINT8  au8ToTestErrCnt[GPIOCFG_NUM_DI_PER_uC];    /* counter for TO error */
   RDS_UINT16 au16ConsTestErrCnt[GPIOCFG_NUM_DI_PER_uC];  /* counter for level test error */
   RDS_UINT8  au8ShortTestErrCnt[GPIOCFG_NUM_DI_PER_uC]; /* counter for short error */
} DIERRHDL_ERRCNT_STRUCT;

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for inputs and tests
** This global structure is not accessed directly in other modules, but only
** via the macros defined below in this file.
** Safe IO is time critical, therefore this "macro"-style was chosen instead
** of function-style.
**-----------------------------------------------------------------------------*/
extern DIERRHDL_ERR_STRUCT diErrHdl_sDiErrHdl;


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
#define DI_ERR_SET_TEST_QUAL(diNum, val) \
       (diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum] = val)

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier is OK (test executed and no error found)
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DI_ERR_IS_TEST_QUAL_OK(diNum) \
       (eGPIO_DIAG_OK == diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum])

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier is OK (test executed and no error found) or
** if not all tests have been executed up to now
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DI_ERR_IS_TEST_QUAL_OK_OR_NA(diNum) \
       ((eGPIO_DIAG_OK == diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum]) \
     || (eGPIO_DIAG_NA == diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum]))

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier is ERROR (test executed and error found)
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DI_ERR_IS_TEST_QUAL_ERR(diNum) \
       (eGPIO_DIAG_ERR == diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum])

/*------------------------------------------------------------------------------
** macro, TRUE if the qualifier of one channel is ERROR (test executed and error found)
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DI_ERR_IS_TEST_QUAL_CH_ERR(diNum) \
       ((eGPIO_DIAG_ERR == diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum]) \
     || (eGPIO_DIAG_ERR == diErrHdl_sDiErrHdl.eDiDiagQualifier[DICFG_GET_OTHER_CHANNEL(diNum)]))

/*------------------------------------------------------------------------------
** macro to adopt the temporally test qualifiers to the main test qualifier
** global/macro-style chosen due to performance reasons (time)
**------------------------------------------------------------------------------
*/
#define DI_ERR_ADOPT_TMP_QUALIFIER(diNum) \
       (diErrHdl_sDiErrHdl.eDiDiagQualifier[diNum] = diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[diNum])

extern void diErrHdl_Init(void);
extern void diErrHdl_rstErr(CONST UINT8 u8DiNum);
extern void diErrHdl_rstErrCnt(CONST UINT8 u8DiNum);

extern void diErrHdl_LevTestFailed(CONST UINT8 u8DiNum);
extern void diErrHdl_LevTestOk(CONST UINT8 u8DiNum);

extern void diErrHdl_ToTestFailed(CONST UINT8 u8DiNum);
extern void diErrHdl_ToTestOk(CONST UINT8 u8DiNum);

extern void diErrHdl_ShortTestFailed(CONST UINT8 u8DiNum);
extern void diErrHdl_ShortTestOk(CONST UINT8 u8DiNum);

extern void diErrHdl_ConsTestFailed(CONST UINT8 u8DualDiNum);
extern void diErrHdl_ConsTestOk(CONST UINT8 u8DualDiNum);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diErrHdl.h
**
********************************************************************************
*/

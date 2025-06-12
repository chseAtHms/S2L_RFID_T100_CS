/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diErrHdl.c
**
** $Id: diErrHdl.c 2448 2017-03-27 13:45:16Z klan $
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
** Error handler of digital input module.
** Handles and stores the qualifier bits of the "safe" inputs.
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


/*******************************************************************************
**
** includes
**
********************************************************************************
*/

/* Project header */
#include "xtypes.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "cfg_Config-sys.h"

/* Header-file of module */
#include "gpio_cfg.h"
#include "diCfg.h"
#include "diDoDiag.h"
#include "fiParam.h"
#include "diErrHdl.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for inputs and tests
**----------------------------------------------------------------------------*/
DIERRHDL_ERR_STRUCT diErrHdl_sDiErrHdl;


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for inputs and tests
**----------------------------------------------------------------------------*/
STATIC DIERRHDL_ERRCNT_STRUCT diErrHdl_sDiErrCnt;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** diErrHdl_Init()
**
** Description:
**    initialization of the error handler of the safe inputs
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
   {
      /* set default state to error flags */
      diErrHdl_rstErr(u8Index);
      /* clear error counters */
      diErrHdl_rstErrCnt(u8Index);
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_rstErr()
**
** Description:
**    reset of error states / reset of qualifiers
**
**------------------------------------------------------------------------------
** Inputs:
**    diNum: number/id of input to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_rstErr(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_rstErr(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* set all qualifiers to NA (meaning: test not executed yet, no test result available) */
   diErrHdl_sDiErrHdl.eLevTestQualifier[u8DiNum]     = eGPIO_DIAG_NA;
   diErrHdl_sDiErrHdl.eToTestQualifier[u8DiNum]      = eGPIO_DIAG_NA;
   diErrHdl_sDiErrHdl.eConsTestQualifier[u8DiNum]    = eGPIO_DIAG_NA;
   diErrHdl_sDiErrHdl.eShortTestQualifier[u8DiNum]   = eGPIO_DIAG_NA;
   diErrHdl_sDiErrHdl.eDiDiagQualifier[u8DiNum]      = eGPIO_DIAG_NA;
   diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[u8DiNum]   = eGPIO_DIAG_NA;
}


/*------------------------------------------------------------------------------
**
** diErrHdl_rstErrCnt()
**
** Description:
**    reset of error counters / reset of qualifiers
**
**------------------------------------------------------------------------------
** Inputs:
**    diNum: number/id of input to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_rstErr(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_rstErrCnt(CONST UINT8 u8DiNum)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* is u8DiNum equal number, then its an input located at uC1
          * (DI0, DI2, DI4...) */
         if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
         {
            RDS_SET(diErrHdl_sDiErrCnt.au16ConsTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
            RDS_SET(diErrHdl_sDiErrCnt.au8ToTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
            /* init Level Test error counter with limit value, to force that an error in
             * very first test enters directly the error case, reason is state switching
             * inside diState.c in combination with qualifier exchange */
            RDS_SET(diErrHdl_sDiErrCnt.au8LevTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 
                    (DIERRHDL_LEV_ERR_CNT_LIMIT));
            RDS_SET(diErrHdl_sDiErrCnt.au8ShortTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
         }
         /* is u8DiNum not equal number, then its an input located at uC2
          * (DI1, DI3, DI5...) */
         else
         {
            /* nothing to do, no error counters for pins of other uC implemented */
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* is u8DiNum equal number, then its an input located at uC1
          * (DI0, DI2, DI4...) */
         if (DIDODIAG_IS_VAL_EQUAL(u8DiNum))
         {
            /* nothing to do, no error counters for pins of other uC implemented */
         }
         /* is u8DiNum not equal number, then its an input located at uC2
          * (DI1, DI3, DI5...) */
         else
         {
            RDS_SET(diErrHdl_sDiErrCnt.au16ConsTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
            RDS_SET(diErrHdl_sDiErrCnt.au8ToTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
            /* init Level Test error counter with limit value, to force that an error in
             * very first test enters directly the error case, reason is state switching
             * inside diState.c in combination with qualifier exchange */
            RDS_SET(diErrHdl_sDiErrCnt.au8LevTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 
                    (DIERRHDL_LEV_ERR_CNT_LIMIT));
            RDS_SET(diErrHdl_sDiErrCnt.au8ShortTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
         /* no break, fall through, in case of invalid value of controller ID */
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_LevTestFailed()
**
** Description:
**    To be called when Level-Test failed, sets:
**    - The Level-Test-Qualifier Identifier to "Error"
**    - The general DI-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the error occurred
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_LevTestFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_LevTestFailed(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   if (RDS_GET(diErrHdl_sDiErrCnt.au8LevTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]) 
          < DIERRHDL_LEV_ERR_CNT_LIMIT) /*lint !e948*/
   {
      /* possible short during level test detected, increase error counter
       * (see [SRS_652]) */
      RDS_INC(diErrHdl_sDiErrCnt.au8LevTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]);
   }
   else
   {
      /* maximum number of allowed errors reached (see [SRS_652]) */

      /* set qualifier of level test */
      diErrHdl_sDiErrHdl.eLevTestQualifier[u8DiNum] = eGPIO_DIAG_ERR;

      /* set "main" qualifier of pin */
      diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[u8DiNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_LevTestOk()
**
** Description:
**    To be called when Level-Test OK (no error), sets:
**    - The Level-Test-Qualifier Identifier to "OK"
**    - The general DI-Qualifier Identifier to "OK" if there was no other error
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the test was successfully executed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_LevTestOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_LevTestOk(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* reset error counter (see [SRS_652]) */
   RDS_SET(diErrHdl_sDiErrCnt.au8LevTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);

   /* set qualifier of level test */
   diErrHdl_sDiErrHdl.eLevTestQualifier[u8DiNum] = eGPIO_DIAG_OK;

   /* set "main" qualifier of pin (only will be set if no error
    * occurred up to now */
   switch (diErrHdl_sDiErrHdl.eDiDiagQualifier[u8DiNum])
   {
      case eGPIO_DIAG_OK:
      case eGPIO_DIAG_NA:
         diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[u8DiNum] = eGPIO_DIAG_OK;
         break;
      case eGPIO_DIAG_ERR:
         /* do not change qualifier if DO is already in error */
         break;
      default:
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ToTestFailed()
**
** Description:
**    To be called when TO-Test failed, increases error counter and if counter
**    reaches limit value sets:
**    - The TO-Test-Qualifier Identifier to "Error"
**    - The general DI-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the error occurred
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_ToTestFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ToTestFailed(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   if (RDS_GET(diErrHdl_sDiErrCnt.au8ToTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]) 
          < DIERRHDL_TO_ERR_CNT_LIMIT) /*lint !e948*/
   {
      /* possible short during level test detected, increase error counter
       * (see [SRS_652]) */
      RDS_INC(diErrHdl_sDiErrCnt.au8ToTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]);
   }
   else
   {
      /* maximum allowed errors reached (see [SRS_652]) => set error flags */

      /* set qualifier of TO test */
      diErrHdl_sDiErrHdl.eToTestQualifier[u8DiNum] = eGPIO_DIAG_ERR;

      /* set "main" qualifier of pin */
      diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[u8DiNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ToTestOk()
**
** Description:
**    To be called when TO-Test OK (no error), sets:
**    - The TO-Test-Qualifier Identifier to "OK"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the test was successfully executed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_ToTestOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ToTestOk(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* reset error counter (see [SRS_652]) */
   RDS_SET(diErrHdl_sDiErrCnt.au8ToTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);
   /* set qualifier of TO test */
   diErrHdl_sDiErrHdl.eToTestQualifier[u8DiNum] = eGPIO_DIAG_OK;
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ShortTestOk()
**
** Description:
**    To be called when Short-Test is OK, sets:
**    - The Short-Test-Qualifier Identifier to "OK"
**    - The Short error counter to "0"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the test was executed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_ShortTestOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ShortTestOk(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   /* reset error counter (see [SRS_652]) */
   RDS_SET(diErrHdl_sDiErrCnt.au8ShortTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)], 0u);

   /* set qualifier of Short-Test */
   diErrHdl_sDiErrHdl.eShortTestQualifier[u8DiNum] = eGPIO_DIAG_OK;
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ShortTestFailed()
**
** Description:
**    To be called when Short-Test failed, increases error counter and if counter
**    reaches limit value sets:
**    - the Short-Test-Qualifier Identifier to "Error"
**    - the general DI-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the test was executed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_PossShortAtPin(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ShortTestFailed(CONST UINT8 u8DiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DI_NUM(u8DiNum);

   if (RDS_GET(diErrHdl_sDiErrCnt.au8ShortTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]) 
         < DIERRHDL_SHORT_ERR_CNT_LIMIT) /*lint !e948*/
   {
      /* possible short during level test detected, increase error counter
       * (see [SRS_652]) */
      RDS_INC(diErrHdl_sDiErrCnt.au8ShortTestErrCnt[DICFG_GET_DUAL_CH_NUM(u8DiNum)]);
   }
   else
   {
      /* maximum allowed error reached (see [SRS_652]) => set short error */
      /* check for short failed x times => possible short */
      /* set qualifier of Short-Test */
      diErrHdl_sDiErrHdl.eShortTestQualifier[u8DiNum] = eGPIO_DIAG_ERR;
      /* set "main" qualifier of pin */
      diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[u8DiNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ConsTestFailed()
**
** Description:
**    To be called when Consistency-Test failed, sets:
**    - The Consistency-Test-Qualifier Identifier to "Error"
**    - The general DI-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the error occurred
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_ConsTestFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ConsTestFailed(CONST UINT8 u8DualDiNum)
{
   UINT16 u16ParamConsFilt;
   UINT16 u16Temp;

   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   u16ParamConsFilt = fiParam_GetConsFilter(u8DualDiNum);
   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   u16Temp = RDS_GET(diErrHdl_sDiErrCnt.au16ConsTestErrCnt[u8DualDiNum]); /*lint !e948*/

   /* values are different, is timeout "consistency time" reached? */
   if ( u16Temp < u16ParamConsFilt)
   {
      /* channels different less time than allowed => increment counter
       * (see [SRS_652])
       * Overflow check not needed, because of comparison in "if" above */
      RDS_INC(diErrHdl_sDiErrCnt.au16ConsTestErrCnt[u8DualDiNum]);
      /* set qualifier of Consistency test, because currently it is
       * not clear if there is an error or not */
      diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C1(u8DualDiNum)] = eGPIO_DIAG_NA;
      diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C2(u8DualDiNum)] = eGPIO_DIAG_NA;
   }
   else
   {
      /* maximum allowed errors reached (see [SRS_652]) => set error flags */

      /* set qualifier of Consistency test */
      diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C1(u8DualDiNum)] = eGPIO_DIAG_ERR;
      diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C2(u8DualDiNum)] = eGPIO_DIAG_ERR;

      /* set "main" qualifier of pin */
      diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[DICFG_GET_DI_NUM_C1(u8DualDiNum)] = eGPIO_DIAG_ERR;
      diErrHdl_sDiErrHdl.eDiDiagQualifierTmp[DICFG_GET_DI_NUM_C2(u8DualDiNum)] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** diErrHdl_ConsTestOk()
**
** Description:
**    To be called when Consistency-Test OK (no error), sets:
**    - The Consistency-Test-Qualifier Identifier to "OK"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DiNum: number/id of input where the test was successfully executed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    diErrHdl_ConsTestOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void diErrHdl_ConsTestOk(CONST UINT8 u8DualDiNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DUAL_DI_NUM(u8DualDiNum);

   /* set qualifier of Consistency test */
   diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C1(u8DualDiNum)] = eGPIO_DIAG_OK;
   diErrHdl_sDiErrHdl.eConsTestQualifier[DICFG_GET_DI_NUM_C2(u8DualDiNum)] = eGPIO_DIAG_OK;

   /* reset error counter to 0 (see [SRS_652]) */
   RDS_SET(diErrHdl_sDiErrCnt.au16ConsTestErrCnt[u8DualDiNum], 0u);
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of diErrHdl.c
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doErrHdl.c
**
** $Id: doErrHdl.c 2448 2017-03-27 13:45:16Z klan $
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
** Error handler of digital output module.
** Handles and stores the qualifier bits of the "safe" outputs.
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

/* Header-file of module */
#include "gpio_cfg.h"
#include "doCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "doErrHdl.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for outputs and tests
**-----------------------------------------------------------------------------*/
DO_ERR_STRUCT doErrHdl_sDoErrHdl;


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** structure containing different qualifier bits for inputs and tests
**----------------------------------------------------------------------------*/
STATIC DOERRHDL_ERRCNT_STRUCT doErrHdl_sDoErrCnt;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doErrHdl_Init()
**
** Description:
**    initialization of the error handler of the safe outputs
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
**    doErrHdl_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_Init(void)
{
   UINT8 u8Index;

   /* for all digital outputs used */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      /* set default state to error flags */
      doErrHdl_RstErr(u8Index);
      /* clear error counters */
      doErrHdl_rstErrCnt(u8Index);
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_RstErr()
**
** Description:
**    reset of error states / reset of qualifiers
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_RstErr(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_RstErr (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   doErrHdl_sDoErrHdl.eVsupTestStep1Qualifier = eGPIO_DIAG_NA;
   doErrHdl_sDoErrHdl.eVsupTestStep2Qualifier = eGPIO_DIAG_NA;
   doErrHdl_sDoErrHdl.eHsTestQualifier[u8DoNum]  = eGPIO_DIAG_NA;
   doErrHdl_sDoErrHdl.ePinChkQualifier[u8DoNum] = eGPIO_DIAG_NA;
   doErrHdl_sDoErrHdl.eDoDiagQualifier[u8DoNum] = eGPIO_DIAG_NA;
}


/*------------------------------------------------------------------------------
**
** doErrHdl_rstErrCnt()
**
** Description:
**    reset of error counters / reset of qualifiers
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_rstErrCnt(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_rstErrCnt(CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   RDS_SET(doErrHdl_sDoErrCnt.au8VsupTestStep1ErrCnt, 0u);
   RDS_SET(doErrHdl_sDoErrCnt.au8VsupTestStep2ErrCnt, 0u);
   RDS_SET(doErrHdl_sDoErrCnt.au8HsTestErrCnt[u8DoNum], 0u);
   RDS_SET(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum], 0u);
}


/*------------------------------------------------------------------------------
**
** doErrHdl_rstErrCntHsTestPinChk()
**
** Description:
**    reset of error counters of HS test and Pin state check
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output to reset
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_rstErrCntHsTestPinChk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_rstErrCntHsTestPinChk(CONST UINT8 u8DoNum)
{
   /* check if DI number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   RDS_SET(doErrHdl_sDoErrCnt.au8HsTestErrCnt[u8DoNum], 0u);
   RDS_SET(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum], 0u);
}


/*------------------------------------------------------------------------------
**
** doErrHdl_VsupTestStep1Failed()
**
** Description:
**    To be called when step1 of VSUP-Test failed, sets:
**    - The VSUP-Test-Qualifier Identifier to "Error"
**    - The general DO-Qualifier Identifier of all outputs to "Error"
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
**    doErrHdl_VsupTestStep1Failed();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_VsupTestStep1Failed (void)
{
   UINT8 u8Index;

   /*lint -e(948)*/
   if (RDS_GET(doErrHdl_sDoErrCnt.au8VsupTestStep1ErrCnt) < DOERRHDL_VSUP_ERR_CNT_LIMIT) 
   {
      /* possible short during level test detected, increase error counter,
       * see [SRS_655] */
      RDS_INC(doErrHdl_sDoErrCnt.au8VsupTestStep1ErrCnt);
   }
   else
   {
      /* maximum allowed errors reached => set error flags, see [SRS_655] */
      doErrHdl_sDoErrHdl.eVsupTestStep1Qualifier = eGPIO_DIAG_ERR;

      for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
      {
         doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index] = eGPIO_DIAG_ERR;
      }
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_VsupTestStep1Ok()
**
** Description:
**    To be called when step1 of VSUP-Test OK (no error), sets:
**    - The VSUP-Test-Qualifier Identifier to "OK"
**    - The general DO-Qualifier Identifier to "OK" if there was no other error
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
**    doErrHdl_VsupTestStep1Ok();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_VsupTestStep1Ok (void)
{
   UINT8 u8Index;

   doErrHdl_sDoErrHdl.eVsupTestStep1Qualifier = eGPIO_DIAG_OK;

   /* reset error counter, see [SRS_655] */
   RDS_SET(doErrHdl_sDoErrCnt.au8VsupTestStep1ErrCnt, 0u);

   /* set "main" qualifier of pins to "OK" (only will be set if no error
    * occurred up to now), done to switch from init vale "NA" to value "OK" */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      switch (doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index])
      {
         case eGPIO_DIAG_OK:
         case eGPIO_DIAG_NA:
            doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index] = eGPIO_DIAG_OK;
            break;
         case eGPIO_DIAG_ERR:
            /* do not change qualifier if DO is already in error */
            break;
         default:
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;
      }
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_VsupTestStep2Failed()
**
** Description:
**    To be called when step2 of VSUP-Test failed, sets:
**    - The VSUP-Test-Qualifier Identifier to "Error"
**    - The general DO-Qualifier Identifier of all outputs to "Error"
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
**    doErrHdl_VsupTestStep2Failed();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_VsupTestStep2Failed (void)
{
   UINT8 u8Index;

   /*lint -e(948)*/
   if (RDS_GET(doErrHdl_sDoErrCnt.au8VsupTestStep2ErrCnt) < DOERRHDL_VSUP_ERR_CNT_LIMIT) 
   {
      /* possible short during level test detected, increase error counter,
       * see [SRS_655] */
      RDS_INC(doErrHdl_sDoErrCnt.au8VsupTestStep2ErrCnt);
   }
   else
   {
      /* maximum allowed errors reached => set error flags,  see [SRS_655] */
      doErrHdl_sDoErrHdl.eVsupTestStep2Qualifier = eGPIO_DIAG_ERR;

      for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
      {
         doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index] = eGPIO_DIAG_ERR;
      }
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_VsupTestStep2Ok()
**
** Description:
**    To be called when step2 of VSUP-Test OK (no error), sets:
**    - The VSUP-Test-Qualifier Identifier to "OK"
**    - The general DO-Qualifier Identifier to "OK" if there was no other error
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
**    doErrHdl_VsupTestStep2Ok();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_VsupTestStep2Ok (void)
{
   UINT8 u8Index;

   doErrHdl_sDoErrHdl.eVsupTestStep2Qualifier = eGPIO_DIAG_OK;

   /* reset error counter, see [SRS_655] */
   RDS_SET(doErrHdl_sDoErrCnt.au8VsupTestStep2ErrCnt, 0u);

   /* set "main" qualifier of pins to "OK" (only will be set if no error
    * occurred up to now), done to switch from init vale "NA" to value "OK" */
   for (u8Index=0u; u8Index<GPIOCFG_NUM_DO_TOT; u8Index++)
   {
      switch (doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index])
      {
         case eGPIO_DIAG_OK:
         case eGPIO_DIAG_NA:
            doErrHdl_sDoErrHdl.eDoDiagQualifier[u8Index] = eGPIO_DIAG_OK;
            break;
         case eGPIO_DIAG_ERR:
            /* do not change qualifier if DO is already in error */
            break;
         default:
            GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
            break;
      }
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_HsTestFailed()
**
** Description:
**    To be called when HS-Test failed, sets:
**    - The HS-Test-Qualifier Identifier to "Error"
**    - The general DO-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output where the test failed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_HsTestFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_HsTestFailed (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /*lint -e(948)*/
   if (RDS_GET(doErrHdl_sDoErrCnt.au8HsTestErrCnt[u8DoNum]) < DOERRHDL_HS_ERR_CNT_LIMIT) 
   {
      /* possible short during level test detected, increase error counter,
       * see [SRS_655] */
      RDS_INC(doErrHdl_sDoErrCnt.au8HsTestErrCnt[u8DoNum]);
   }
   else
   {
      /* maximum allowed errors reached => set error flags (see [SRS_655]) */
      doErrHdl_sDoErrHdl.eHsTestQualifier[u8DoNum] = eGPIO_DIAG_ERR;
      doErrHdl_sDoErrHdl.eDoDiagQualifier[u8DoNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_HsTestOk()
**
** Description:
**    To be called when HS-Test OK (no error), sets:
**    - The HS-Test-Qualifier Identifier to "OK"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output where the test was successful
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_HsTestOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_HsTestOk (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);
   /* reset error counter (see [SRS_655]) */
   RDS_SET(doErrHdl_sDoErrCnt.au8HsTestErrCnt[u8DoNum], 0u);
   doErrHdl_sDoErrHdl.eHsTestQualifier[u8DoNum] = eGPIO_DIAG_OK;
}


/*------------------------------------------------------------------------------
**
** doErrHdl_OtherChFailed()
**
** Description:
**    To be called when the qualifier received via IPC of the other channel
**    failed, sets:
**    - The general DO-Qualifier Identifier to "Error"
**    - Additionally, the general DO-Qualifier Identifier to "Error" of the
**      other channel in case of dual channel configuration
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output where the test failed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_OtherChFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_OtherChFailed (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   if (FIPARAM_DO_DUAL_CH(u8DoNum))
   {
      /* set qualifier of both channels to error ([SRS_153], see also [SRS_339]).
       * Reason: if one output channel is in error state, the other should also be passivated */
      doErrHdl_sDoErrHdl.eDoDiagQualifier[u8DoNum] = eGPIO_DIAG_ERR;
      doErrHdl_sDoErrHdl.eDoDiagQualifier[DOCFG_GET_OTHER_CHANNEL(u8DoNum)] = eGPIO_DIAG_ERR;
   }
   else
   {
      doErrHdl_sDoErrHdl.eDoDiagQualifier[u8DoNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_PinStateCheckFailed()
**
** Description:
**    To be called when the state of the safe output has not the expected value,
**    see [SRS_639], sets:
**    - The Pin-Check-Qualifier Identifier to "Error"
**    - The general DO-Qualifier Identifier to "Error"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output where the test failed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_PinStateCheckFailed(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_PinStateCheckFailed (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /*lint -e(948)*/
   if (RDS_GET(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum]) < DOERRHDL_PIN_STATE_ERR_CNT_LIMIT) 
   {
      /* possible short during level test detected, increase error counter,
       * see [SRS_655] */
      RDS_INC(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum]);
   }
   else
   {
      /* maximum allowed errors reached => set error flags (see [SRS_639],
       * [SRS_655]) */
      doErrHdl_sDoErrHdl.ePinChkQualifier[u8DoNum] = eGPIO_DIAG_ERR;
      doErrHdl_sDoErrHdl.eDoDiagQualifier[u8DoNum] = eGPIO_DIAG_ERR;
   }
}


/*------------------------------------------------------------------------------
**
** doErrHdl_PinStateCheckOk()
**
** Description:
**    To be called when the state of the safe output matches the expected value,
**    see [SRS_639], sets:
**    - The Pin-Check-Qualifier Identifier to "OK"
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoNum: number/id of output where the test failed
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doErrHdl_PinStateCheckOk(x);
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**------------------------------------------------------------------------------
*/
void doErrHdl_PinStateCheckOk (CONST UINT8 u8DoNum)
{
   /* check if DO number is valid */
   DIDODIAG_SAFETY_CHECK_DO_NUM(u8DoNum);

   /* Deactivated Lint Note 948: Operator '==' always evaluates to True Problem is in RDS_GET. */
   /*lint -e(948)*/
   if(RDS_GET(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum]) >= DOSETGETPIN_DOSTATE_ERR_CNT_DEC) 
   {
      /* in case of valid values decrease error counter in bigger steps  */
      RDS_SUB(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum], DOSETGETPIN_DOSTATE_ERR_CNT_DEC)
   }
   else
   {
      /* set qualifier to OK and reset error counter (see [SRS_655]) */
      doErrHdl_sDoErrHdl.ePinChkQualifier[u8DoNum] = eGPIO_DIAG_OK;
      RDS_SET(doErrHdl_sDoErrCnt.au8PinChkErrCnt[u8DoNum], 0u);/*lint !e661*/
   }
}

/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of doErrHdl.c
**
********************************************************************************
*/

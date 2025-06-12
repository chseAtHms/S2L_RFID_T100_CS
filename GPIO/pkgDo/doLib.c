/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doLib.c
**
** $Id: doLib.c 4440 2024-05-29 12:03:45Z ankr $
** $Revision: 4440 $
** $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** contains interface functions:
** - cyclic life signal toggling
** - safety protocol request for safe output state
** - Test qualifier conversion
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013-2024 HMS Industrial Networks AB            **
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
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "gpio-hal.h"

#include "cfg_Config-sys.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "gpio_cfg.h"
#include "doCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "doDiag.h"
#include "doErrHdl.h"
#include "doPortMap.h"
#include "doState.h"
#include "doSafeBoundSS1t.h"
#include "doLib.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** variable containing test qualifiers to be send via IPC
** Global used to time critical IPC functionality
**------------------------------------------------------------------------------
*/
UINT8 dolib_u8TestQualValues = (UINT8)0xFFu;

/*------------------------------------------------------------------------------
** variable be filled by IPC, containing test qualifiers from other channel
** Global used to time critical IPC functionality
**------------------------------------------------------------------------------
*/
UINT8 dolib_u8TestQualValuesOtherCh = (UINT8)0xFFu;

/*------------------------------------------------------------------------------
**  This variable contains the time stamp of the last diagnostic timeout check
**------------------------------------------------------------------------------
*/
STATIC RDS_UINT32 doLib_u32RdsDiagTimeoutCheck;



/*******************************************************************************
**
** private Services
**
********************************************************************************
*/
STATIC void doLib_ConvertTestQualFromOtherCh(void);


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doLib_Init()
**
** Description:
** function for initialization/configuration DO LIb module
** Currently, only the RDS-Variable used for Diagnostic timeout check is
** initialized
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
**    doLib_Init();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_Init(void)
{
   UINT32 u32tmp;

   u32tmp = timerHAL_GetSystemTime3();
   RDS_SET(doLib_u32RdsDiagTimeoutCheck, u32tmp);

   /* initialize sub-module for SafeBound and SS1-t functionality */
   doSafeBoundSS1t_Init();
}


/*------------------------------------------------------------------------------
**
** doLib_ToggleLifeAndScCcSelect()
**
** Description:
**    toggling of two cyclic "life signals":
**    - chip select signal, which is the cyclic signal deciding which uC is able
**      to send via AIC(UART). If this cyclic signal is send, then uC1 is able
**      to send via AIC(UART), if this signal is not send, then uC2 is able to
**      send.
**    - life signal, which is the cyclic input signal for the HW-Block
**      "Voltage Supervision"
**    Remark: Currently the function handles two signals, but this will be
**            changed in first HW-Redesign
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Usage:
**    doLib_ToggleLifeAndScCcSelect();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_ToggleLifeAndScCcSelect(void)
{
   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* toggle "life-signal" pin see [SRS_40] and [SRS_589]  */
         gpioHAL_TogglePin(PORT_LIFE_SIG, PINMASK_LIFE_SIG);
         break;

      case SAFETY_CONTROLLER_2:
         /* toggle pin only if currently no Voltage supervision test is
          * ongoing to avoid influence on test result  */
         if (FALSE == doDiag_IsVsupTestOngoing())
         {
            /* toggle "life-signal" pin see [SRS_40] and [SRS_589]  */
            gpioHAL_TogglePin(PORT_LIFE_SIG, PINMASK_LIFE_SIG);
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** doLib_SetPSDoReq()
**
** Description:
**    The requested state of the safe outputs is received inside the safe
**    packet. This function is called by the Communication/AIC module (which
**    receives the safe packet) to:
**    - set the requested value for the DO received via communication interface
**      (safety protocol)
**    - writes the resulting, required output state (after prioritization) to the
**      "reqState" (structure member).
**    (see [SRS_2007])
**-----------------------------------------------------------------------------
** Inputs:
**    doReq: requested output state byte, received via safety protocol
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doLib_SetPSDoReq(doReq);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_SetPSDoReq(CONST UINT8 u8DoReqByte)
{
   UINT8 u8DoChannel;

   /* do for all DOs */
   for (u8DoChannel = 0u; u8DoChannel < GPIOCFG_NUM_DO_TOT; u8DoChannel++)
   {
      /* is DO in dual channel mode? (see [SRS_2017], [SRS_339])*/
      if (FIPARAM_DO_DUAL_CH(u8DoChannel))
      {
         /* in dual channel mode, process only every second bit, see [SRS_2052],
          * [SRS_2141], [SRS_2142] */
         if (DIDODIAG_IS_VAL_EQUAL(u8DoChannel))
         {
            /* in dual channel case both DOs set together, see [SRS_113] */
            if (u8DoReqByte & (UINT8)(0x01u << u8DoChannel))
            {
              doSafeBoundSS1t_ReqSet(u8DoChannel, DOCFG_GET_OTHER_CHANNEL(u8DoChannel), eGPIO_HIGH);
            }
            else
            {
              doSafeBoundSS1t_ReqSet(u8DoChannel, DOCFG_GET_OTHER_CHANNEL(u8DoChannel), eGPIO_LOW);

            }
         }
      }
      /* single channel mode, see [SRS_2016] */
      else
      {
         /* process every bit of SPDU packet in single channel mode, see [SRS_2052],
          * [SRS_2140] */
         if (u8DoReqByte & (UINT8)(0x01u << u8DoChannel))
         {
           doSafeBoundSS1t_ReqSet(u8DoChannel, u8DoChannel, eGPIO_HIGH);
         }
         else
         {
           doSafeBoundSS1t_ReqSet(u8DoChannel, u8DoChannel, eGPIO_LOW);
         }
      }
   }
}

/*------------------------------------------------------------------------------
**
** doLib_PassAllDOs()
**
** Description:
**    - set the requested value for all DOs to not-active
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
**    doLib_PassAllDOs();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_PassAllDOs(void)
{
   UINT8 u8DoChannel;

   for (u8DoChannel = 0u; u8DoChannel < GPIOCFG_NUM_DO_TOT; u8DoChannel++)
   {
     /* In case of passivation: request LOW on all DOs. This also considers
        SS1-t delay time (see [SRS_906]). */
     doSafeBoundSS1t_ReqSet(u8DoChannel, u8DoChannel, eGPIO_LOW);
   }
}


/*------------------------------------------------------------------------------
**
** doLib_SetErrRstFlags()
**
** Description:
**    The requested state of the error reset flags is received inside the safe
**    packet. This function is called by the Communication/AIC module (which
**    receives the safe packet) to:
**    - set the requested value for the DO error reset flags
**    See [SRS_2015]
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DoErrRstByte: error reset Byte, received via safety protocol
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    doLib_SetErrRstFlags(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_SetErrRstFlags(CONST UINT8 u8DoErrRstByte)
{
   UINT8 u8DoChannel;

   /* do for all DOs */
   for (u8DoChannel = 0u; u8DoChannel < GPIOCFG_NUM_DO_TOT; u8DoChannel++)
   {
      /* is DO in dual channel mode? (see also see [SRS_339]) */
      if (FIPARAM_DO_DUAL_CH(u8DoChannel))
      {
         /* in dual channel mode, process only every second bit, see [SRS_2053] */
         if (DIDODIAG_IS_VAL_EQUAL(u8DoChannel))
         {
            /* is error reset flag set and also DO request "OFF" */
            if ( (0x00u != (u8DoErrRstByte & (UINT8)(0x01u << u8DoChannel))) &&
                 (eGPIO_LOW == doState_DoReqSPDU[u8DoChannel]) )
            {
               doState_DoErrRstSPDU[u8DoChannel] = eTRUE;
               doState_DoErrRstSPDU[DOCFG_GET_OTHER_CHANNEL(u8DoChannel)] = eTRUE;
            }
            else
            {
               doState_DoErrRstSPDU[u8DoChannel] = eFALSE;
               doState_DoErrRstSPDU[DOCFG_GET_OTHER_CHANNEL(u8DoChannel)] = eFALSE;
            }
         }
      }
      /* single channel mode */
      else
      {
         /* process every bit of SPDU packet in single channel mode, see [SRS_2053] */
         /* is error reset flag set and also DO request "OFF" */
         if ( (0x00u != (u8DoErrRstByte & (UINT8)(0x01u << u8DoChannel))) &&
              (eGPIO_LOW == doState_DoReqSPDU[u8DoChannel]) )
         {
            doState_DoErrRstSPDU[u8DoChannel] = eTRUE;
         }
         else
         {
            doState_DoErrRstSPDU[u8DoChannel] = eFALSE;
         }
      }
   }
}


/*------------------------------------------------------------------------------
**
** doLib_ConvValFromOtherCh()
**
** Description:
**    calls conversion functions for all DO related values/information received
**    from other uC (which are currently the qualifier bits only).
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doLib_ConvValFromOtherCh()
**
** Module Test:
**    - YES -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_ConvValFromOtherCh(void)
{
   /* call function to convert/process received bit values */
   doLib_ConvertTestQualFromOtherCh();
}


/*------------------------------------------------------------------------------
**
** doLib_PrepareTestQualForIpc()
**
** Description:
**    converts the values of output error qualifiers of current uC to Bit Format.
**    These bits are stored in one variable which can be send via IPC later.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doLib_PrepareTestQualForIpc()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void doLib_PrepareTestQualForIpc(void)
{
   /* index for different pins */
   UINT8 u8doNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* reset all bits, test qualifier bit "1" is Ok and "0" error */
   dolib_u8TestQualValues = (UINT8)0x00u;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every output pin of own uC (digital "safe" input) */
         for (u8doNum = 0u; u8doNum < GPIOCFG_NUM_DO_PER_uC; u8doNum++ )
         {
            /* convert enum in bit values, take values from output 0, 2, 4, ...   */
            if (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8doNum*2u))
            {
               /* set Bit to 1 (OK) if test-bit OK or NA (not applicable) */
               dolib_u8TestQualValues |= (UINT8)(0x01u << u8doNum);
            }
            else
            {
               /* nothing to do, because of initialization/reset of "dolib_u8TestQualValues" */
            }
         }
         break;
      case SAFETY_CONTROLLER_2:
         /* loop: do this for every output pin of own uC (digital "safe" output) */
         for (u8doNum = 0u; u8doNum < GPIOCFG_NUM_DO_PER_uC; u8doNum++)
         {
            /* convert enum in bit values, take values from input 1, 3, 5, ... */
            if (DO_ERR_IS_TEST_QUAL_OK_OR_NA((u8doNum*2u) + 1u))
            {
               /* set Bit to 1 (OK) if if test-bit OK or NA (not applicable) */
               dolib_u8TestQualValues |= (UINT8)(0x01u << u8doNum);
            }
            else
            {
               /* nothing to do, because of initialization/reset of "dolib_u8TestQualValues" */
            }
         }
         break;
      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*------------------------------------------------------------------------------
**
** doLib_CheckDoDiagTimeout()
**
** Description:
**    High-Level function to call the functions for DO diagnostic timeout
**    check:
**    Whenever a time x is expired, the functions to check the execution of
**    the diagnostic tests are called. In case of HS-Test this is only done
**    for active outputs.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doLib_CheckDoDiagTimeout()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**
** Context: main, while(FOREVER)
**------------------------------------------------------------------------------
*/
void doLib_CheckDoDiagTimeout(void)
{
   UINT8 u8Index;
   UINT32 u32SysTime;
   UINT32 u32TimeDiff;
   TRUE_FALSE_ENUM eRet;

   /* get current system time and saved "last time" of check */
   __disable_irq();
   u32SysTime = timerHAL_GetSystemTime3();
   u32TimeDiff = RDS_GET(doLib_u32RdsDiagTimeoutCheck);
   __enable_irq();

   /* check when the last diagnostic timeout check was executed */
   u32TimeDiff = u32SysTime - u32TimeDiff;

   /* time for next check reached? */
   if (u32TimeDiff >= DODIAG_TIMEOUT)
   {
      /* check for VSUP test execution only if not already in error state */
      if  ((eGPIO_DIAG_ERR != DO_ERR_VSUP_TEST_STEP1_RESULT) && \
           (eGPIO_DIAG_ERR != DO_ERR_VSUP_TEST_STEP2_RESULT))
      {
         /* call check function of VSUP-Test */
         doDiag_CheckDiagVsupTimeout();
      }

      /* call check function of HS-Test for active inputs */
      for (u8Index = 0u; u8Index < GPIOCFG_NUM_DO_TOT; u8Index++)
      {
         /* HS-Test only executed if DO active (this includes the information
          * that the DO is not in error state */
         eRet = doState_ActiveCycTillTestReached(u8Index);
         if (eTRUE == eRet)
         {
            doDiag_CheckDiagHSTimeout(u8Index);
         }
         else
         {
            /* defensive programming: Fail Safe entry in case of invalid value */
            GLOBFAIL_SAFETY_ASSERT(eRet == eFALSE, GLOB_FAILCODE_VARIABLE_ERR);
         }

      }

      /* set time of current diagnostic timeout-check for next interval */
      /* ATTENTION: RDS variable is also written by function doLib_Init()
      ** (IRQ context). Since RDS variables are not atomic, it must be ensured
      ** that this function is not called at the same time with doLib_Init(). 
      */
      RDS_SET(doLib_u32RdsDiagTimeoutCheck, u32SysTime);
   }
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** doLib_ConvertTestQualFromOtherCh()
**
** Description:
**    converts the bit values of test qualifiers received from other channel:
**    The result is stored in the qualifier array of error handler, and only for
**    the other channel.
**    Only the values of the other channel are written, therefore uC1 and uC2
**    write to different array members
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**    doLib_ConvertTestQualFromOtherCh()
**
** Module Test:
**    - YES -
**    Reason: arithmetic, shift operations
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
STATIC void doLib_ConvertTestQualFromOtherCh(void)
{
   /* index for different pins */
   UINT8 u8doNum;

   CFG_CONTROLLER_ID_ENUM eControllerID;

   /* get controller ID (uC1 or uC2) */
   eControllerID = cfgSYS_GetControllerID();

   switch (eControllerID)
   {
      case SAFETY_CONTROLLER_1:
         /* loop: do this for every output pin of other channel */
         for (u8doNum = 0u; u8doNum < GPIOCFG_NUM_DO_PER_uC; u8doNum++)
         {
            /* outputs at other uC (uc2), to be placed in DO number 1, 3, 5, ... */
            if ( dolib_u8TestQualValuesOtherCh & ((UINT8)(0x01u << u8doNum)) )
            {
               DO_ERR_SET_TEST_QUAL( ((u8doNum*2u) + 1u), eGPIO_DIAG_OK);
            }
            else
            {
               doErrHdl_OtherChFailed((UINT8)((u8doNum*2u) + 1u));
            }
         }
         break;

      case SAFETY_CONTROLLER_2:
         /* loop: do this for every output pin of other channel */
         for (u8doNum = 0u; u8doNum < GPIOCFG_NUM_DO_PER_uC; u8doNum++)
         {
            /* outputs at other uC (uc2), to be placed in DO number 0, 2, 4, ... */
            if ( dolib_u8TestQualValuesOtherCh & ((UINT8)(0x01u << u8doNum)) )
            {
               DO_ERR_SET_TEST_QUAL( (u8doNum*2u), eGPIO_DIAG_OK);
            }
            else
            {
               doErrHdl_OtherChFailed((UINT8)(u8doNum*2u));
            }
         }
         break;

      case SAFETY_CONTROLLER_INVALID:
      default:
         /* wrong controller ID */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_IPCX_CONTROLLER_ID);
         break;
   }
}


/*******************************************************************************
**
** End of doLib.c
**
********************************************************************************
*/

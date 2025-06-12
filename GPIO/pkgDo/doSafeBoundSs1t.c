/***************************************************************************************************
**    Copyright (C) 2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: doSafeBoundSs1t.c
**     Summary: This module implements the main parts of the SafeBound and SS1-t features.
**   $Revision: 4666 $
**       $Date: 2024-10-25 10:27:28 +0200 (Fr, 25 Okt 2024) $
**      Author: AnKr
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: doSafeBoundSS1t_Init
**             doSafeBoundSS1t_SetSafeBoundVal
**             doSafeBoundSS1t_ReqSet
**             doSafeBoundSS1t_ReqRampDown
**             doSafeBoundSS1t_UpdateTimers
**
**             WaitForDoRequestHighState
**             WaitForDoRequestLowState
**             WaitForDoDelayElapsedState
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

/* Module header */
#include "gpio_cfg.h"
#include "doCfg.h"
#include "fiParam.h"
#include "diDoDiag.h"
#include "doErrHdl.h"
#include "doSetGetPin.h"
#include "doState.h"
#include "doSafeBoundSs1t.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      39u

/*!< enum defintion for the DO delay state machine */
typedef enum
{
   eWF_DO_REQ_HIGH                     = 0xf9e7u,   /*!< Wait for DO request HIGH */
   eWF_DO_REQ_LOW                      = 0xfac9u,   /*!< Wait for DO request LOW */
   eWF_DO_DELAY_ELAPSED                = 0xfb5eu    /*!< Wait for DO delay elapsed */
} t_DO_SS1T_DELAY_STATE_ENUM;

/* state variables for each DO channel */
STATIC t_DO_SS1T_DELAY_STATE_ENUM ae_Ss1tState[GPIOCFG_NUM_DO_TOT];

/* delay timers for each DO channel */
STATIC RDS_UINT16 au16_RdsTimeout4msCtrs[GPIOCFG_NUM_DO_TOT];

/* DI SafeBound value for DO */
STATIC RDS_BOOL ab_RdsSafeBoundValueForDo[GPIOCFG_NUM_DO_TOT];

/* Bit masks for the SS1-t delay indicators (virtual DIs) */
#define k_BIT_MASK_VIRTUAL_DI7  (1u << 6)
#define k_BIT_MASK_VIRTUAL_DI8  (1u << 7)


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void WaitForDoRequestHighState(UINT8 u8_doChannel,
                                      UINT8 u8_doChannelOther,
                                      GPIO_STATE_ENUM e_reqState);
STATIC void WaitForDoRequestLowState(UINT8 u8_doChannel,
                                     UINT8 u8_doChannelOther,
                                     GPIO_STATE_ENUM e_reqState);
STATIC void WaitForDoDelayElapsedState(UINT8 u8_doChannel,
                                       UINT8 u8_doChannelOther,
                                       GPIO_STATE_ENUM e_reqState);


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    doSafeBoundSS1t_Init

  Description:
    This function initializes the static variables of this module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_Init(void)
{
  UINT8 u8_index;

  /* for all digital outputs used */
  for (u8_index=0u; u8_index<GPIOCFG_NUM_DO_TOT; u8_index++)
  {
    /* set default state for SS1-t state machine */
    ae_Ss1tState[u8_index] = eWF_DO_REQ_HIGH;
    /* initialize SS1-t delay timer to 'stopped' */
    RDS_SET(au16_RdsTimeout4msCtrs[u8_index], 0u);
  }
}


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_SetSafeBoundVal

  Description:
    This function sets the input value coming from the local DIs for the "AND gate" of the
    SafeBound feature. The provided value is stored and later used in the state machine.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    b_diSafeBoundValue (IN) - boolean value to be set
                              (valid range: TRUE/FALSE, not checked, only called with constant)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_SetSafeBoundVal(const UINT8 u8_doChannel,
                                     BOOL b_diSafeBoundValue)
{
  /* check if DO number is valid */
  DIDODIAG_SAFETY_CHECK_DO_NUM(u8_doChannel);

  /* store passed value into module static RDS variable */
  RDS_SET(ab_RdsSafeBoundValueForDo[u8_doChannel], b_diSafeBoundValue);
}


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_ReqSet

  Description:
    This function is the state machine for the SS1-t feature. It evaluates the request for the
    output state and considers the DI SafeBound value for the DO.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    u8_doChannelOther (IN)  - in Dual Channel mode: the other DO channel number
                              in Single Channel mode: same value as u8DoChannel
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, checked)
    e_reqState (IN)         - requested state for the output
                              (valid range: GPIO_STATE_ENUM, not checked, passed to
                               sub-functions, checked there)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_ReqSet(UINT8 u8_doChannel,
                            UINT8 u8_doChannelOther,
                            GPIO_STATE_ENUM e_reqState)
{
  /* check if DO channel numbers are valid */
  DIDODIAG_SAFETY_CHECK_DO_NUM(u8_doChannel);
  DIDODIAG_SAFETY_CHECK_DO_NUM(u8_doChannelOther);

  switch (ae_Ss1tState[u8_doChannel])
  {
    case eWF_DO_REQ_HIGH: /* DO is in safe state (LOW) */
    {
      /* let sub-function handle this state */
      WaitForDoRequestHighState(u8_doChannel, u8_doChannelOther, e_reqState);
      break;
    }

    case eWF_DO_REQ_LOW: /* DO is not in safe state (HIGH) */
    {
      /* let sub-function handle this state */
      WaitForDoRequestLowState(u8_doChannel, u8_doChannelOther, e_reqState);
      break;
    }

    case eWF_DO_DELAY_ELAPSED: /* DO is not in safe state (HIGH) */
    {
      /* let sub-function handle this state */
      WaitForDoDelayElapsedState(u8_doChannel, u8_doChannelOther, e_reqState);
      break;
    }

    default:
    {
      /* invalid state, should never enter here */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
  }

}


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_ReqRampDown

  Description:
    This function determines the states of the "virtual DIs" for the signalization of SS1-t delay.

  See also:
    -

  Parameters:
    -

  Return value:
    UINT8 - value of virtual DIs

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT8 doSafeBoundSS1t_ReqRampDown(void)
{
  UINT8 u8_retVal = 0u;

  /* if "Ramp down" shall be signaled for DO1 */
  if (ae_Ss1tState[0] == eWF_DO_DELAY_ELAPSED)
  {
    /* set virtual DI1 (see [SRS_907]) */
    u8_retVal = (u8_retVal | (UINT8)k_BIT_MASK_VIRTUAL_DI7);
  }
  else  /* else: no "Ramp down" in progress */
  {
    /* empty branch */
  }

  /* if "Ramp down" shall be signaled for DO2 */
  if (ae_Ss1tState[1] == eWF_DO_DELAY_ELAPSED)
  {
    /* set virtual DI2 (see [SRS_907]) */
    u8_retVal = (u8_retVal | (UINT8)k_BIT_MASK_VIRTUAL_DI8);
  }
  else  /* else: no "Ramp down" in progress */
  {
    /* empty branch */
  }

  return u8_retVal;
}


/***************************************************************************************************
  Function:
    doSafeBoundSS1t_UpdateTimers

  Description:
    This function needs to be called once in every scheduler cycle to update the SS1-t delay timers.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void doSafeBoundSS1t_UpdateTimers(void)
{
  UINT8 u8_doChannel;

  for (u8_doChannel = 0u; u8_doChannel < GPIOCFG_NUM_DO_TOT; u8_doChannel++)
  {
    /* if timer is running */
    if (RDS_GET(au16_RdsTimeout4msCtrs[u8_doChannel]) > 0u)
    {
      /* timer is reduced with 4ms */
      RDS_DEC(au16_RdsTimeout4msCtrs[u8_doChannel])
    }
    else  /* else: timer is inactive */
    {
      /* empty branch */
    }
  }
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    WaitForDoRequestHighState

  Description:
    This function is part of the doSafeBoundSS1t_ReqSet() state machine. It handles the state "Wait
    for DO request HIGH" state.

  See also:
    -

  Parameters:
    u8_doChannel (IN)        - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    u8_doChannelOther (IN)  - in Dual Channel mode: the other DO channel number
                              in Single Channel mode: same value as u8DoChannel
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    e_reqState (IN)         - requested state for the output
                              (valid range: GPIO_STATE_ENUM, checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void WaitForDoRequestHighState(UINT8 u8_doChannel,
                                      UINT8 u8_doChannelOther,
                                      GPIO_STATE_ENUM e_reqState)
{
  /* if HIGH state is request */
  if (eGPIO_HIGH == e_reqState)
  {
    /* if the DI "AND gate" value for the DO (SafeBound) is also HIGH/TRUE */
    /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
      *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
      *  an issue if the right hand side is not evaluated. */
    if (    (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannel]) == TRUE)
          && (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannelOther]) == TRUE)
        ) /*lint !e960*/
    {
      /* leave the DO safe state (LOW) */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_HIGH;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_HIGH;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_LOW;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_LOW;
    }
    else /* the DI "AND gate" value for the DO (SafeBound) is not HIGH/TRUE */
    {
      /* do nothing, DO safe state (LOW) must not be left
          because of the DI local SafeBound */
    }
  }
  /* else: if LOW state is requested */
  else if (eGPIO_LOW == e_reqState)
  {
    /* do nothing */
  }
  else  /* else: invalid state requested */
  {
    /* invalid state, should never enter here */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }

}


/***************************************************************************************************
  Function:
    WaitForDoRequestLowState

  Description:
    This function is part of the doSafeBoundSS1t_ReqSet() state machine. It handles the state "Wait
    for DO request LOW" state.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    u8_doChannelOther (IN)  - in Dual Channel mode: the other DO channel number
                              in Single Channel mode: same value as u8DoChannel
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    e_reqState (IN)         - requested state for the output
                              (valid range: GPIO_STATE_ENUM, checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void WaitForDoRequestLowState(UINT8 u8_doChannel,
                                     UINT8 u8_doChannelOther,
                                     GPIO_STATE_ENUM e_reqState)
{
  /* configured SS1-t delay timer value (in 4ms units) */
  UINT16 u16CfgDelay4ms = 0u;

  /* if LOW state is request */
  if (eGPIO_LOW == e_reqState)
  {
    /* get the SS1-t Delay Timer value from the configuration parameters */
    u16CfgDelay4ms = FIPARAM_DO_SS1T_DELAY_4MS(u8_doChannel);

    /* if DOx is in error state (see doState_EvalNextState) */
    if (DO_ERR_TEST_QUAL_ERR(u8_doChannel) ||
        DO_ERR_TEST_QUAL_ERR(u8_doChannelOther))
    {
      /* enter immediately the DO safe state (LOW) without delay. See [SRS_913]. */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }
    /* else if the DI "AND gate" value for the DO (SafeBound) is HIGH/TRUE and
          STO delay is configured */
    /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
      *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
      *  an issue if the right hand side is not evaluated. */
    else if (    (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannel]) == TRUE)
              && (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannelOther]) == TRUE)
              && (u16CfgDelay4ms > 0u)  /*lint !e960*/
            )
    {
      /* enter the DO safe state (LOW) after the configured STO delay
         because of the remote request (see [SRS_905]) */

      ae_Ss1tState[u8_doChannel] = eWF_DO_DELAY_ELAPSED;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_DELAY_ELAPSED;

      /* STO delay is started */
      RDS_SET(au16_RdsTimeout4msCtrs[u8_doChannel], u16CfgDelay4ms);
      RDS_SET(au16_RdsTimeout4msCtrs[u8_doChannelOther], u16CfgDelay4ms);
    }
    else /* the DI "AND gate" value for the DO (SafeBound) is not HIGH/TRUE */
    {
      /* enter immediately the DO safe state (LOW) without delay
         because of the local DI SafeBound. See [SRS_901] and [SRS_910]. */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }
  }
  /* else: if HIGH state is requested */
  else if (eGPIO_HIGH == e_reqState)
  {
    /* if the DI "and gate" value for the DO (SafeBound) is HIGH/TRUE */
    /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
      *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
      *  an issue if the right hand side is not evaluated. */
    if (    (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannel]) == TRUE)
          && (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannelOther]) == TRUE)
        )  /*lint !e960*/
    {
      /* do nothing */
    }
    else /* the DI "and gate" value for the DO (SafeBound) is not HIGH/TRUE */
    {
      /* enter immediately the DO safe state (LOW) without delay
          because of the local DI SafeBound. See [SRS_901] and [SRS_910]. */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }
  }
  else  /* else: invalid state requested */
  {
    /* invalid state, should never enter here */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
  }

}


/***************************************************************************************************
  Function:
    WaitForDoDelayElapsedState

  Description:
    This function is part of the doSafeBoundSS1t_ReqSet() state machine. It handles the state "Wait
    for DO delay elapsed" state.

  See also:
    -

  Parameters:
    u8_doChannel (IN)       - number/id of output to evaluate
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    u8_doChannelOther (IN)  - in Dual Channel mode: the other DO channel number
                              in Single Channel mode: same value as u8DoChannel
                              (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked, checked by caller)
    e_reqState (IN)         - requested state for the output
                              (valid range: GPIO_STATE_ENUM, checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void WaitForDoDelayElapsedState(UINT8 u8_doChannel,
                                       UINT8 u8_doChannelOther,
                                       GPIO_STATE_ENUM e_reqState)
{
  /* if HIGH state is request */
  if (eGPIO_HIGH == e_reqState)
  {
    /* if the DI "AND gate" value for the DO (SafeBound) is HIGH/TRUE */
    /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
      *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
      *  an issue if the right hand side is not evaluated. */
    if (    (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannel]) == TRUE)
          && (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannelOther]) == TRUE)
        )  /*lint !e960*/
    {
      doState_DoReqSPDU[u8_doChannel] = eGPIO_HIGH;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_HIGH;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_LOW;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_LOW;
    }
    else /* the DI "AND gate" value for the DO (SafeBound) is not HIGH/TRUE (see [SRS_910]) */
    {
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }

    /* delay timer is stopped */
    RDS_SET(au16_RdsTimeout4msCtrs[u8_doChannel], 0u);
    RDS_SET(au16_RdsTimeout4msCtrs[u8_doChannelOther], 0u);
  }
  /* else: if LOW state is requested */
  else if (eGPIO_LOW == e_reqState)
  {
    /* if DOx is in error state (see doState_EvalNextState) */
    if (DO_ERR_TEST_QUAL_ERR(u8_doChannel) ||
        DO_ERR_TEST_QUAL_ERR(u8_doChannelOther))
    {
      /* enter immediately the DO safe state (LOW) without delay. See [SRS_913]. */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }
    /* else if the DI "AND gate" value for the DO (SafeBound) is HIGH/TRUE */
    /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
      *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
      *  an issue if the right hand side is not evaluated. */
    else if (    (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannel]) == TRUE)
              && (RDS_GET(ab_RdsSafeBoundValueForDo[u8_doChannelOther]) == TRUE)
            )  /*lint !e960*/
    {
      /* if delay timer expired */
      /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical
        *  operator: '&&' --> Side effect of RDS_GET is checking of, the redundant copy. It's not
        *  an issue if the right hand side is not evaluated. */
      if (    (RDS_GET(au16_RdsTimeout4msCtrs[u8_doChannel]) == 0u)
            && (RDS_GET(au16_RdsTimeout4msCtrs[u8_doChannelOther]) == 0u)
         )  /*lint !e960*/
      {
        doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
        doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

        ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
        ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
      }
      else
      {
        /* do nothing */
      }
    }
    else /* the DI "AND gate" value for the DO (SafeBound) is not HIGH/TRUE */
    {
      /* enter immediately the DO safe state (LOW) without delay because of local DI SafeBound. */
      /* See [SRS_901] and [SRS_910]. */
      doState_DoReqSPDU[u8_doChannel] = eGPIO_LOW;
      doState_DoReqSPDU[u8_doChannelOther] = eGPIO_LOW;

      ae_Ss1tState[u8_doChannel] = eWF_DO_REQ_HIGH;
      ae_Ss1tState[u8_doChannelOther] = eWF_DO_REQ_HIGH;
    }
  }
  else  /* else: invalid state requested */
  {
    /* invalid state, should never enter here */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
  }
}

/*** End Of File ***/

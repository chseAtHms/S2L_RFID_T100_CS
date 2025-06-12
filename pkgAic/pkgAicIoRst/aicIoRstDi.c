/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicIoRstDi.c
**     Summary: This module handles the Automatic Error Reset handling of DIs (see [SRS_2107]).
**   $Revision: 2618 $
**       $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicIoRstDi_Init
**             aicIoRstDi_ChkAutoErrRst
**
**             AutoErrorResetDi
**             AutoErrResetProcessDi
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "rds.h"

#include "includeDiDo.h"
#include "fiParam.h"

/* application includes */
#include "aicIoRstDi.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      1u

/*!<  Data type used for the automatic error reset state machines */
typedef enum
{
  k_DI_RST_STATE_CHECK    = 0x69ffu, /*!< IO qualifiers of channel (dual/single) checked */
  k_DI_RST_STATE_ERROR    = 0x6ad1u, /*!< Error detected, error latch timer started for recovery */
  k_DI_RST_STATE_RECOVERY = 0x6b46u  /*!< Error latch timer elapsed */
} t_DI_RST_STATE;

/* state variables for each input channel */
STATIC t_DI_RST_STATE ae_DiState[GPIOCFG_NUM_DI_TOT];
/* error latch timeout counter for each input channel */
STATIC RDS_INT32 ai32_DiErrorLatchTimeout[GPIOCFG_NUM_DI_TOT];

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void AutoErrorResetDi(void);
STATIC BOOL AutoErrResetProcessDi(UINT8 u8_index);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicIoRstDi_Init

  Description:
    This function is used for module initialization. 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicIoRstDi_Init(void)
{
  UINT8 u8Index;
  
  /* if automatic error reset for DIs enabled */
  if (FIPARAM_DI_AUTOMATIC_RESET_ENABLED)
  {
    /* for each DI channel */
    for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
    {
      /* set init values */
      ae_DiState[u8Index] = k_DI_RST_STATE_CHECK;
      RDS_SET(ai32_DiErrorLatchTimeout[u8Index], 0u);
    }
  }
  /* else: automatic error reset disabled */
  else
  {
    /* empty branch */
  }
}


/***************************************************************************************************
  Function:
    aicIoRstDi_ChkAutoErrRst

  Description:
    This function has to be called once within T-cycl for the processing of the automatic error 
    reset feature. The function processes the internal state machines of the DI channels.
    The result of the calculations (error reset bits) is forwarded to the DI module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicIoRstDi_ChkAutoErrRst(void)
{
  /* if automatic error reset for DIs enabled */
  if (FIPARAM_DI_AUTOMATIC_RESET_ENABLED)
  {
    /* execute automatic error reset for all DIs */
    AutoErrorResetDi();
  }
  /* else: automatic error reset disabled */
  else
  {
    /* empty branch */
  }
  
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    AutoErrorResetDi

  Description:
    This function calls for each DI channel the automatic error reset state machine. If an error 
    reset is requested, the corresponding bit inside the error reset flag (DI) is set and passed
    to the DI module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void AutoErrorResetDi(void)
{
  UINT8 u8Index;
  UINT8 u8ErrRstFlags = 0U;
  
  /************************************************/
  /* get auto error reset state                   */
  /************************************************/
  for (u8Index = 0u; u8Index < GPIOCFG_NUM_DI_TOT; u8Index++)
  {
    /* if the DI was disabled by Parametrization */
    if (FIPARAM_DI_DISABLED(DICFG_GET_DUAL_CH_NUM(u8Index)))
    {
      /* empty branch since the DI is disabled, there is no need to check for qualifiers
      ** here */
    }
    /* else if: DI enabled and error reset requested */
    else if (AutoErrResetProcessDi(u8Index) == TRUE)
    {
      u8ErrRstFlags |= (UINT8)(0x01u << u8Index);
    }
    /* else: DI enabled but NO error reset requested */
    else
    {
      /* empty branch */
    }
  }
  
  /* pass DI Error Reset Flags to DI module */
  diLib_SetErrRstFlags(u8ErrRstFlags);
}

/***************************************************************************************************
  Function:
    AutoErrResetProcessDi

  Description:
    This function is used to handle the automatic error reset feature of a single DI channel. The
    mode of the channel (dual/single) is considered internally here.
    The function implements the state machine to handle the functionality specified in 
    [SRS_2107].

  See also:
    - 

  Parameters:
    u8_index (IN) - Channel index of DI module
                    (valid range: 0..GPIOCFG_NUM_DI_TOT-1, not checked)

  Return value:
    TRUE  - Automatic error reset of this channel shall be performed.
    FALSE - No Automatic error reset of this channel

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL AutoErrResetProcessDi(UINT8 u8_index)
{
  /* return value of this function indicates if an automatic error reset of this channel shall
  ** be performed.
  */
  BOOL b_resetReq = FALSE;

  /* evaluate state of DI channel */
  switch (ae_DiState[u8_index])
  {
    case k_DI_RST_STATE_CHECK:
    {
      /* if dual channel configuration of DI */
      if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {
          /* if both qualifiers are OK stay in state */
          if (DI_ERR_IS_TEST_QUAL_OK_OR_NA(u8_index) && \
            DI_ERR_IS_TEST_QUAL_OK_OR_NA(DICFG_GET_OTHER_CHANNEL(u8_index)))
          {
            /* empty branch, everything fine, stay in state */
          }
          /* else: at least one qualifier signalizes an error */
          else
          {
            /* enter error state */
            ae_DiState[u8_index] = k_DI_RST_STATE_ERROR;
          }
        }
        /* else: even bits shall not be processed */
        else
        {
          /* empty branch */
        }
      }
      /* single channel configuration of DI */
      else
      {
        /* if qualifier is OK stay in state */
        if (DI_ERR_IS_TEST_QUAL_OK_OR_NA(u8_index))
        {
          /* empty branch, everything fine, stay in state */
        }
        /* else: qualifier signalizes an error */
        else
        {
          /* enter error state */
          ae_DiState[u8_index] = k_DI_RST_STATE_ERROR;
        }
      }
      break;
    }

    case k_DI_RST_STATE_ERROR:
    {
      /* if dual channel configuration of DI */
      if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {
          /* if input is in safe state (inactive) */
          if ( (DI_INPUTVAL_DI(u8_index) == DICFG_DI_INACTIVE) && \
               (DI_INPUTVAL_DI(DICFG_GET_OTHER_CHANNEL(u8_index)) == DICFG_DI_INACTIVE) )
          {
            /* start Input Error Latch Time for error recovery */
            RDS_SET(ai32_DiErrorLatchTimeout[u8_index], FIPARAM_DI_LATCH_ERROR_TIME);
            /* enter next state */
            ae_DiState[u8_index] = k_DI_RST_STATE_RECOVERY;
          }
          /* else: inputs not put into safe state */
          else
          {
            /* empty branch, stay in state */
          }
        }
        /* else: not called for equal bit */
        else
        {
          /* shall not happen, because state not reachable for this channel in 
          ** dual channel mode */
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
        }
      }
      /* single channel configuration of DI */
      else
      {
        /* if input is in safe state (inactive) */
        if (DI_INPUTVAL_DI(u8_index) == DICFG_DI_INACTIVE) 
        {
          /* start Input Error Latch Time for error recovery */
          RDS_SET(ai32_DiErrorLatchTimeout[u8_index], FIPARAM_DI_LATCH_ERROR_TIME);
          /* enter next state */
          ae_DiState[u8_index] = k_DI_RST_STATE_RECOVERY;
        }
        /* else: input not put into safe state */
        else
        {
          /* stay in state */
        }
      }
      break;
    }

    case k_DI_RST_STATE_RECOVERY:
    {
      /* since function is called every ~4ms, decrement RDS variable by 4.
      ** RDS variable could be negative then if FIPARAM_DI_LATCH_ERROR_TIME is not a
      ** multiple of 4. OK, because FIPARAM_DI_LATCH_ERROR_TIME has a range of 16 bit.
      */
      RDS_DEC(ai32_DiErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DiErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DiErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DiErrorLatchTimeout[u8_index]);

      /* dual channel configuration of DI */
      if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {
          /* read input, we expect that inputs are in safe state until Input Error
          ** Latch Time is elapsed.
          ** if inputs are not in safe state (active) */
          if (  (DI_INPUTVAL_DI(u8_index) == DICFG_DI_ACTIVE) || \
                (DI_INPUTVAL_DI(DICFG_GET_OTHER_CHANNEL(u8_index)) == DICFG_DI_ACTIVE) )
          {
            /* switch back to error state */
            ae_DiState[u8_index] = k_DI_RST_STATE_ERROR;
          }
          /* else if: inputs in safe state and Input Error Latch Time elapsed */
          else if (RDS_GET(ai32_DiErrorLatchTimeout[u8_index]) <= 0)
          {
            /* go to default state */
            ae_DiState[u8_index] = k_DI_RST_STATE_CHECK;
            /* and request an error reset for this channel */
            b_resetReq = TRUE;
          }
          /* else: Input Error Latch Time not elapsed yet */
          else
          {
            /* empty branch, stay in state */
          }
        }
        /* else: not called for equal bit */
        else
        {
          /* shall not happen, because state not reachable for this channel in 
          ** dual channel mode */
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
        }
      }
      /* single channel configuration of DI */
      else
      {
        /* read input, we expect that input is kept in safe state until Input Error
        ** Latch Time is elapsed.
        ** if input is not in safe state (active) */
        if (DI_INPUTVAL_DI(u8_index) == DICFG_DI_ACTIVE) 
        {
          /* switch back to error state */
          ae_DiState[u8_index] = k_DI_RST_STATE_ERROR;
        }
        /* else if: input in safe state and Input Error Latch Time elapsed */
        else if (RDS_GET(ai32_DiErrorLatchTimeout[u8_index]) <= 0)
        {
          /* go to default state */
          ae_DiState[u8_index] = k_DI_RST_STATE_CHECK;
          /* and request an error reset for this channel (see [SRS_2107]) */
          b_resetReq = TRUE;
        }
        /* else: Input Error Latch Time not elapsed yet */
        else
        {
          /* empty branch, stay in state */
        }
      }
      break;
    }
    
    default:
    {
      /* call globFail_SafetyHandler, never return from this... */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
      break;
    }
  } /* end switch ... */

  return b_resetReq;
/* This function has a lot of paths because configuration mode of DI/DO shall be considered
** in every state here. But it makes no sense to divide the function further. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

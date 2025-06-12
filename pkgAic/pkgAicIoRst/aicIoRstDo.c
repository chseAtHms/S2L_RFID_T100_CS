/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicIoRstDo.c
**     Summary: This module handles the Automatic Error Reset handling of DOs (see [SRS_2108]).
**   $Revision: 2618 $
**       $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicIoRstDo_Init
**             aicIoRstDo_ChkAutoErrRst
**
**             AutoErrorResetDo
**             AutoErrResetProcessDo
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
#include "aicMsgDef.h"
#include "aicIoRstDo.h"

#include "SpduOutData.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      2u

/*!<  Data type used for the automatic error reset state machines */
typedef enum
{
  k_DO_RST_STATE_CHECK    = 0x6669u, /*!< IO qualifiers of channel (dual/single) checked */
  k_DO_RST_STATE_ERROR    = 0x67feu, /*!< Error detected, error latch timer started for recovery */
  k_DO_RST_STATE_RECOVERY = 0x6868u  /*!< Error latch timer elapsed */
} t_DO_RST_STATE;


/* macro checks if bit is set inside the IO Data field received from network */
#define IS_DOBIT_SET(u8_index, u8_doVal) ((u8_doVal & (UINT8)(0x01u << u8_index)) != 0u)

/* state variables for each output channel */
STATIC t_DO_RST_STATE ae_DoState[GPIOCFG_NUM_DO_TOT];
/* error latch timeout counter for each output channel */
STATIC RDS_INT32 ai32_DoErrorLatchTimeout[GPIOCFG_NUM_DO_TOT];

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void AutoErrorResetDo(void);
STATIC BOOL AutoErrResetProcessDo(UINT8 u8_index, UINT8 u8_doVal);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicIoRstDo_Init

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
void aicIoRstDo_Init(void)
{
  UINT8 u8Index;
  
  /* if automatic error reset for DOs enabled */
  if (FIPARAM_DO_AUTOMATIC_RESET_ENABLED)
  { 
    /* for each DO channel */
    for (u8Index = 0u; u8Index < GPIOCFG_NUM_DO_TOT; u8Index++)
    {
      /* set init values */
      ae_DoState[u8Index] = k_DO_RST_STATE_CHECK;
      RDS_SET(ai32_DoErrorLatchTimeout[u8Index], 0u);
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
    aicIoRstDo_ChkAutoErrRst

  Description:
    This function has to be called once within T-cycl for the processing of the automatic error 
    reset feature. The function processes the internal state machines of the DO channels.
    The result of the calculations (error reset bits) is forwarded to the DO module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicIoRstDo_ChkAutoErrRst(void)
{ 
  /* if automatic error reset for DOs enabled */
  if (FIPARAM_DO_AUTOMATIC_RESET_ENABLED)
  { 
    /* execute automatic error reset for all DOs */
    AutoErrorResetDo();
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
    AutoErrorResetDo

  Description:
    This function calls for each DO channel the automatic error reset state machine. If an error 
    reset is requested, the corresponding bit inside the error reset flag (DO) is set and passed
    to the DO module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void AutoErrorResetDo(void)
{
  UINT8 u8Index;
  UINT8 u8ErrRstFlags = 0U;
  UINT8 u8_doVal; 
  
  /* read current DO value */
  u8_doVal = spduOutData_DoGet();
  
  /************************************************/
  /* get auto error reset bits of DOs             */
  /************************************************/
  for (u8Index = 0u; u8Index < GPIOCFG_NUM_DO_TOT; u8Index++)
  {
    /* check if the DO was disabled by Parametrization */
    if (FIPARAM_DO_DISABLED(u8Index))
    {
      /* empty branch since the DO is disabled, there is no need to check for qualifiers
      ** here */
    }
    /* else if: DO enabled and error reset requested */
    else if (AutoErrResetProcessDo(u8Index, u8_doVal) == TRUE)
    {
      u8ErrRstFlags |= (UINT8)(0x01u << u8Index);
    }
    /* else: DO enabled but nor NO error reset requested */
    else
    {
      /* empty branch */
    }
  }
  
  /* pass DO Error Reset Flags to DO module */
  doLib_SetErrRstFlags(u8ErrRstFlags);
}



/***************************************************************************************************
  Function:
    AutoErrResetProcessDo

  Description:
    This function is used to handle the automatic error reset feature of a single DO channel. The
    mode of the channel (dual/single) is considered internally here.
    The function implements the state machine to handle the functionality specified in 
    [SRS_2108].

  See also:
    - 

  Parameters:
    u8_index (IN) - Channel index of DO module
                    (valid range: 0..GPIOCFG_NUM_DO_TOT-1, not checked)
    u8_doVal (IN)  - Current valid DO value
                    (valid range: any, not checked)

  Return value:
    TRUE  - Automatic error reset of this channel shall be performed.
    FALSE - No Automatic error reset of this channel

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL AutoErrResetProcessDo(UINT8 u8_index, UINT8 u8_doVal)
{
  /* return value of this function indicates if an automatic error reset of this channel shall
  ** be performed.
  */
  BOOL b_resetReq = FALSE;

  /* evaluate state of DO channel */
  switch (ae_DoState[u8_index])
  {
    case k_DO_RST_STATE_CHECK:
    {
      /* dual channel configuration of DO */
      if (FIPARAM_DO_DUAL_CH(DOCFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {  
          /* get DO qualifier and set state accordingly */
          /* if both qualifiers are OK stay in state */
          if (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8_index) && \
              DO_ERR_IS_TEST_QUAL_OK_OR_NA(DOCFG_GET_OTHER_CHANNEL(u8_index)))
          {
            /* empty branch, everything fine, stay in state */
          }
          /* else: at least one qualifier signalizes an error */
          else
          {
            /* enter error state */
            ae_DoState[u8_index] = k_DO_RST_STATE_ERROR;
          }
        }
        /* else: even bits shall not be processed */
        else
        {
          /* empty branch */
        }
      }
      /* single channel configuration of DO */
      else
      {
        /* if qualifier is OK stay in state */
        if (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8_index))
        {
          /* empty branch, everything fine, stay in state */
        }
        /* else: qualifier signalizes an error */
        else
        {
          /* enter error state */
          ae_DoState[u8_index] = k_DO_RST_STATE_ERROR;
        }
      }
      break;
    }

    case k_DO_RST_STATE_ERROR:
    {
      /* is DO in dual channel mode? */
      if (FIPARAM_DO_DUAL_CH(DOCFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {
          /* It is not possible to check if DO is OK again, because tests are only
          ** performed if DO is forced to active. So we use at least the Output Monitor 
          ** Value to ensure that DO is really put to low level (means TestResult=Pass),
          ** see OnTime Defect#7010.
          */
          
          /* if Safety Output Value is NOT forced to zero OR Output Monitor Value (pin-state) 
          ** is NOT inactive. */
          if ( (IS_DOBIT_SET(u8_index, u8_doVal)) ||
               (diInput_sInputValues.eDoTest[u8_index] != DICFG_DI_INACTIVE) || 
               (diInput_sInputValues.eDoTest[DOCFG_GET_OTHER_CHANNEL(u8_index)] 
                                                       != DICFG_DI_INACTIVE))
          {
            /* no error recovery, stay in state */
          }
          /* else: Safety Output Value zero AND Output Monitor Value (pin-state) inactive */
          else
          {
            /* start recovery sequence, start Output Error Latch Time */
            RDS_SET(ai32_DoErrorLatchTimeout[u8_index], FIPARAM_DO_LATCH_ERROR_TIME);
            ae_DoState[u8_index] = k_DO_RST_STATE_RECOVERY;
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
      /* single channel configuration of DO */
      else
      {
        /* It is not possible to check if DO is OK again, because tests are only
        ** performed if DO is forced to active. So we use at least the Output Monitor 
        ** Value to ensure that DO is really put to low level (means TestResult=Pass),
        ** see OnTime Defect#7010.
        */
        
        /* if Safety Output Value is NOT forced to zero OR Output Monitor Value (pin-state) 
        ** is NOT inactive */
        if ( (IS_DOBIT_SET(u8_index, u8_doVal)) ||
             (diInput_sInputValues.eDoTest[u8_index] != DICFG_DI_INACTIVE) )
        {
          /* no error recovery, stay in state */
        }
        /* else: Safety Output Value zero AND Output Monitor Value (pin-state) inactive */
        else
        {
          /* start recovery sequence, start Output Error Latch Time */
          RDS_SET(ai32_DoErrorLatchTimeout[u8_index], FIPARAM_DO_LATCH_ERROR_TIME);
          ae_DoState[u8_index] = k_DO_RST_STATE_RECOVERY;
        }
      }
      break;
    }

    case k_DO_RST_STATE_RECOVERY:
    {
      /* since function is called every ~4ms, decrement RDS variable by 4.
      ** RDS variable could be negative then if FIPARAM_DO_LATCH_ERROR_TIME is not a 
      ** multiple of 4. OK, because FIPARAM_DO_LATCH_ERROR_TIME has a range of 16 bit.
      */
      RDS_DEC(ai32_DoErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DoErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DoErrorLatchTimeout[u8_index]);
      RDS_DEC(ai32_DoErrorLatchTimeout[u8_index]);
      
      /* is DO in dual channel mode? */
      if (FIPARAM_DO_DUAL_CH(DOCFG_GET_DUAL_CH_NUM(u8_index)))
      {
        /* in dual channel mode, process only every second bit */
        if (DIDODIAG_IS_VAL_EQUAL(u8_index))
        {
          /* if Safety Output Value is NOT forced to zero OR Output Monitor Value (pin-state) 
          ** is NOT inactive */
          if ( (IS_DOBIT_SET(u8_index, u8_doVal)) ||
               (diInput_sInputValues.eDoTest[u8_index] != DICFG_DI_INACTIVE) || 
               (diInput_sInputValues.eDoTest[DOCFG_GET_OTHER_CHANNEL(u8_index)] 
                                                       != DICFG_DI_INACTIVE))
          {
            /* switch back to error state */
            ae_DoState[u8_index] = k_DO_RST_STATE_ERROR;
          }
          /* else if: check if Output Error Latch Time is elapsed */
          else if (RDS_GET(ai32_DoErrorLatchTimeout[u8_index]) <= 0)
          {
            /* request reset and enter default state */ 
            ae_DoState[u8_index] = k_DO_RST_STATE_CHECK;
            b_resetReq = TRUE;
          }
          /* else: Safety Output Value put to zero and Output Monitor Value is inactive but
          ** Output Error Latch Time not elapsed yet */
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
      /* single channel configuration of DO */
      else
      {
        /* if Safety Output Value is NOT forced to zero OR Output Monitor Value (pin-state) 
        ** is NOT inactive */
        if ( (IS_DOBIT_SET(u8_index, u8_doVal)) ||
             (diInput_sInputValues.eDoTest[u8_index] != DICFG_DI_INACTIVE) )
        {
          /* bit of Safety Output Value not forced to zero */
          /* switch back to error state */
          ae_DoState[u8_index] = k_DO_RST_STATE_ERROR;
        }
        /* else if: check if Output Error Latch Time is elapsed */
        else if (RDS_GET(ai32_DoErrorLatchTimeout[u8_index]) <= 0)
        {
          /* request reset and enter default state (see [SRS_2108]) */
          ae_DoState[u8_index] = k_DO_RST_STATE_CHECK;
          b_resetReq = TRUE;
        }
        /* else: Safety Output Value put to zero and Output Monitor Value is inactive but 
        ** Output Error Latch Time not elapsed yet */
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

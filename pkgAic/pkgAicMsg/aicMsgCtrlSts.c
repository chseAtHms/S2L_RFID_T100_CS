/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCtrlSts.c
**     Summary: This module implements functionality to access the control/status byte of the AM.
**              Furthermore it provides functionality to build the ASM control/status byte.
**   $Revision: 4440 $
**       $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCtrlSts_Process
**             aicMsgCtrlSts_IsNewFragRcvd
**             aicMsgCtrlSts_IsAsmFragConf
**             aicMsgCtrlSts_BuildCtrlStsByte
**
**             AbortStateRequest
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

/* module includes */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicSm.h"
#include "aicMsgSis.h"

#include "aicMsgCtrlSts.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgCtrlSts_u8_AsmCtrlSts:
** control/status of ASM message, global used due to performance reasons,
** init value fulfills also [SRS_318], see [SIS_026]
** formerly aicDataStack_u8AsmCtrlStatus
*/
UINT8 aicMsgCtrlSts_u8_AsmCtrlSts  = AICMSGHDL_ASM_STATE_BOOT;

/* aicMsgCtrlSts_u8_AmCtrlSts:
** control/status of AM message, global used due to performance reasons
** formerly aicDataStack_u8AnybusCtrlStatus
*/
/* start with zero, see [SRS_318], see [SIS_026] */
UINT8 aicMsgCtrlSts_u8_AmCtrlSts   = 0x00u;

/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      10u

/* u8_LastProcAmCtrlSts:
** This variable is the last received and processed AM control/status. Since it is possible that
** between processing AM message and sending ASM message a new telegram with (new  AM
** control/status) is received, the processed control/status (of this cycle) is saved to set the
** fragmentation bits is the correct way.
*/
STATIC UINT8 u8_LastProcAmCtrlSts  = 0x00u;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC void AbortStateRequest(void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_Process

  Description:
    The function stores the received AM control/status internally since it is possible that between
    processing AM message and sending ASM message a new telegram with (new AM control/status) is
    received.

    Furthermore the Anybus State field of the received AM Control/status byte is evaluated and
    the evaluated anybus state request is signalized to the internal state machine (see [SRS_2145], 
    [SRS_707], [SIS_068]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    -

***************************************************************************************************/
void aicMsgCtrlSts_Process (void)
{
  UINT8 u8_stateField;

  /* Since it is possible that between processing AM message and sending
  ** ASM message a new telegram with (new AM control/status) is received,
  ** the processed control/status (of this cycle) is saved to set the
  ** fragmentation bits is the correct way. */
  u8_LastProcAmCtrlSts = aicMsgCtrlSts_u8_AmCtrlSts;

  /* get anybus state field from control status byte (see [SIS_068] and [SRS_497]) */
  u8_stateField = u8_LastProcAmCtrlSts & AICMSGHDL_ANYBUS_STATE_MASK;

  /* check if state request is received inside Anybus State field */

  /* if 'no request' (see [SIS_220]) */
  if (u8_stateField == AICMSGHDL_ANYBUS_STATE_R_NO)
  {
    /* empty branch, nothing to do here */
  }
  /* else if: 'request to transit to Abort state' (see [SIS_220]) */
  else if (u8_stateField == AICMSGHDL_ANYBUS_STATE_R_ABORT)
  {
    AbortStateRequest();
  }
  /* else: unknown request */
  else
  {
    /* ignored, empty branch */
  }
}

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_IsNewFragRcvd

  Description:
    Returns TRUE if the received Anybus fragmentation bit was toggled, else FALSE.
    The principle of fragmentation is explained in Serial Interface Specification [SIS] and in 
    the Software Requirements [SRS].

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                     - the received Anybus fragmentation bit was toggled
    FALSE                    - not toggled

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCtrlSts_IsNewFragRcvd (void)
{
  BOOL b_retVal;

  /* if Anybus fragmentation bit was toggled (see [SRS_299]), set return value to TRUE */
  if ( (u8_LastProcAmCtrlSts & AICMSGHDL_MASK_ANYBUS_FRAGFLAG) != \
       (aicMsgCtrlSts_u8_AsmCtrlSts & AICMSGHDL_MASK_ANYBUS_FRAGFLAG) )
  {
    b_retVal = TRUE;
  }
  else
  {
    b_retVal = FALSE;
  }

  return b_retVal;
}

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_IsAsmFragConf

  Description:
    returns TRUE if the received ASM fragmentation bit was confirmed by ABCC/AM, else FALSE
    The principle of fragmentation is explained in Serial Interface Specification [SIS] and in 
    the Software Requirements [SRS]

  See also:
    -

  Parameters:
    -

  Return value:
    BOOL        TRUE if the received ASM fragmentation bit has the same
                value as the one send, else FALSE

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCtrlSts_IsAsmFragConf (void)
{
  BOOL b_retVal;

  /* if ASM fragmentation bit received is the same as send, set return value to TRUE */
  if ( (u8_LastProcAmCtrlSts & AICMSGHDL_MASK_ASM_FRAGFLAG) == \
       (aicMsgCtrlSts_u8_AsmCtrlSts & AICMSGHDL_MASK_ASM_FRAGFLAG) )
  {
    b_retVal = TRUE;
  }
  else
  {
    b_retVal = FALSE;
  }

  return b_retVal;
}

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_BuildCtrlStsByte

  Description:
    Builds/Prepares the Ctrl/Status Byte which is send as first Byte of every TX telegram. The 
    status of the ASM is send in lower 4 Bits of Ctrl/Status Byte, two fragmentation bits are 
    located at Bit7 and Bit6.
    Steps executed in function:
    - get the current fragmentation bits (Depending on the input parameter "b_toggleAsmFrg" the 
      ASM fragmentation bit is toggled later in function)
    - resets the "state" part of the Ctrl/Status Byte to 0
    - Set the two fragmentation bits:
      - AM fragmentation bit is set depending on function parameter (the last received
        Anybus fragmentation bit)
      - ASM fragmentation bit is set depending on function parameter (toggled)
    - sets the "state" part of the Ctrl/Status Byte according to current state
      (the "ASM state" is set depending on AIC state)

    Hint: Message fragmentation is described in Serial Interface Specification
          [SIS] and Software Requirements Specification[SRS]

  See also:
    -

  Parameters:
    b_toggleAsmFrg (IN)    - depending on this input parameter ASM fragmentation bit
                             will be toggled (see [SRS_361])
                             (valid range: TRUE, FALSE, not checked)

    b_amFragConf (IN)      - depending on this input parameter the AM fragmentation bit is set
                             (valid range: eTRUE, eFALSE, checked)
  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgCtrlSts_BuildCtrlStsByte (CONST BOOL b_toggleAsmFrg,
                                     CONST TRUE_FALSE_ENUM b_amFragConf)
{
  UINT8 u8_msgFragAnybus;
  UINT8 u8_msgFragAsm;

  /* check parameter */
  if ( (eFALSE != b_amFragConf) && (eTRUE != b_amFragConf))
  {
    /* failure in input variable */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  else
  {
    /* get last fragmentation bits, see [SRS_305] */
    u8_msgFragAnybus = u8_LastProcAmCtrlSts & AICMSGHDL_MASK_ANYBUS_FRAGFLAG;
    u8_msgFragAsm    = aicMsgCtrlSts_u8_AsmCtrlSts & AICMSGHDL_MASK_ASM_FRAGFLAG;

    /* reset ASM status/control byte except fragmentation flags */
    aicMsgCtrlSts_u8_AsmCtrlSts &= (AICMSGHDL_MASK_ANYBUS_FRAGFLAG | AICMSGHDL_MASK_ASM_FRAGFLAG);
    
    /* set AM fragmentation bit (Bit7)*/
    if (eTRUE == b_amFragConf)
    {
      /* set Anybus fragmentation bit to the same value as received,
       * see [SRS_299] */
      if (u8_msgFragAnybus)
      {
        /* set Anybus fragmentation bit to "1" */
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_MASK_ANYBUS_FRAGFLAG;
      }
      else
      {
        /* set Anybus fragmentation bit to "0" */
        aicMsgCtrlSts_u8_AsmCtrlSts &= (~AICMSGHDL_MASK_ANYBUS_FRAGFLAG);
      }
    }
    else
    {
      /* do nothing, leave AM fragmentation flag as it was in last msg send */
    }
    
    
    /* if ASM fragmentation bit (Bit6) shall be toggled */
    if (TRUE == b_toggleAsmFrg)
    {
      if (u8_msgFragAsm)
      {
        /* toggle ASM fragmentation bit: was 1, set to 0 */
        aicMsgCtrlSts_u8_AsmCtrlSts &= (~AICMSGHDL_MASK_ASM_FRAGFLAG);
      }
      else
      {
        /* toggle ASM fragmentation bit: was 0, set to 1 */
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_MASK_ASM_FRAGFLAG;
      }
    }
    else
    {
      /* nothing to do */
    }
    
    /* generate the ASM state (depending on current AIC state) (Bit0-3)
     * (see [SRS_2001], [SRS_2022]) */
    switch (aicSm_eAicState)
    {
      /* CIP Safety AIC states */
      case AICSM_AIC_STARTUP: 
      {
        /* error case, should never be here because no AIC communication */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
        break;
      }
      case AICSM_AIC_WAIT_FOR_INIT:    /* fall through */
      case AICSM_AIC_START_CSS:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_STARTUP;            /* [SRS_2216] */
        break;
      }
      case AICSM_AIC_WAIT_FOR_CONFIG:  /* fall through */
      case AICSM_AIC_PROCESS_CONFIG:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_WAIT_CONFIG;        /* [SRS_2215] */
        break;
      }
      case AICSM_AIC_INVALID_CONFIG:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_INVALID_CONFIG;     /* [SRS_2214] */
        break;
      }
      case AICSM_AIC_IDLE:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_IDLE;
        break;
      }
      case AICSM_AIC_EXEC_PROD_ONLY:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_EXEC_PROD_ONLY;     /* [SRS_2213] */
        break;
      }
      case AICSM_AIC_EXEC_CONS_ONLY:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_EXEC_CONS_ONLY;     /* [SRS_2213] */
        break;
      }
      case AICSM_AIC_EXEC_PROD_CONS:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_EXEC_PROD_AND_CONS; /* [SRS_2213] */
        break;
      }
      case AICSM_AIC_WAIT_RESET:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_WAIT_RESET;
        break;
      }
      case AICSM_AIC_ABORT:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_ABORT;
        break;
      }
      case AICSM_AIC_WAIT_TUNID:
      {
        aicMsgCtrlSts_u8_AsmCtrlSts |= AICMSGHDL_ASM_STATE_WAIT_TUNID;
        break;
      }
      default:
      {
        /* error case, should never enter here */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
        break;
      }
    }
  }
/* This function has a lot of paths because the AIC state is catched inside
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    AbortStateRequest

  Description:
    This function handles the 'request to transit to Abort state' received inside the Anybus State
    field of the Ctrl/Status byte of the received telegram (see [SIS_220]).
    Since the 'ABORT' state of the software can only be left by a Safety Reset, the 'Abort request'
    is signalized exactly once to the internal state machine.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void AbortStateRequest(void)
{
  /* variable indicates if 'Abort request' received inside Anybus State field is already signalized
  ** to internal state machine.
  */
  LOCAL_STATIC(, TRUE_FALSE_ENUM, e_AbortReqTriggered, eFALSE);

  /* if already signalized */
  if (e_AbortReqTriggered == eTRUE)
  {
    /* empty branch */
  }
  /* else if: not signalized yet */
  else if (e_AbortReqTriggered == eFALSE)
  {
    /* forward request to internal state machine depending on current AIC state */
    switch (aicSm_eAicState)
    {
      /* Abort request ignored here */
      case AICSM_AIC_STARTUP:
      case AICSM_AIC_WAIT_FOR_INIT:    /* fall through */
      case AICSM_AIC_START_CSS:        /* fall through */
      case AICSM_AIC_ABORT:            /* fall through */
      case AICSM_AIC_WAIT_RESET:       /* fall through */
      {
        break;
      }
      /* Abort request in state 'WAIT_TUNID' (see [SRS_2243]) */
      case AICSM_AIC_WAIT_TUNID: 
      /* Abort request in state 'CONFIG' (see [SRS_2244]) */
      case AICSM_AIC_WAIT_FOR_CONFIG:  /* fall through */
      case AICSM_AIC_PROCESS_CONFIG:   /* fall through */
      case AICSM_AIC_INVALID_CONFIG:   /* fall through */
      /* Abort request in state 'IDLE' (see [SRS_2245]) */
      case AICSM_AIC_IDLE:             /* fall through */
      /* Abort request in state 'EXECUTING' (see [SRS_2246]) */
      case AICSM_AIC_EXEC_PROD_ONLY:   /* fall through */
      case AICSM_AIC_EXEC_CONS_ONLY:   /* fall through */
      case AICSM_AIC_EXEC_PROD_CONS:   /* fall through */
      {
        /* forward request to internal state machine */
        aicSm_SetEvent(AICSM_k_EVT_ABORT_REQUEST);
        /* set flag so this branch is never called again */
        e_AbortReqTriggered = eTRUE;
        break;
      }
      default:
      {
        /* error case, should never enter here */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
        break;
      }
    }
  }
  /* else: variable error */
  else
  {
    /* error case, should never enter here */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
  }
/* This function has a lot of paths because the AIC state is catched inside
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */

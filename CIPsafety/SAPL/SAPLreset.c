/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLreset.c
**     Summary: This file implements the interface to perform a Safety Reset Request requested by 
**              CSS.
**   $Revision: 3841 $
**       $Date: 2023-01-13 15:04:45 +0100 (Fr, 13 Jan 2023) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_IxssoSafetyResetClbk
**             SAPL_ResetTypeGet
**
**             CheckAicState
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

#include "ipcx_ExchangeData-sys.h"

#include "aicMsgDef.h"
#include "aicMsgErr.h"
#include "aicSm.h"
#include "spduOutData.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* CSS headers */
#include "IXSSOapi.h"

/* SAPL headers */
#include "SAPLreset.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      35u

#ifndef __UNIT_TESTING_ON__
  /* Init-value (RDS_UINT8) of local Attribute Bit Map */
  #define k_ATTR_BIT_MAP_INIT         {{(UINT8)0u},{(UINT8)~(UINT8)0u}}
#else /* __UNIT_TESTING_ON__ */
  /* Init-Value used for unit tests. RDS functionality is turned off here */
  #define k_ATTR_BIT_MAP_INIT         0U 
#endif /* __UNIT_TESTING_ON__ */

/** au8_Password
** Default password e.g. used for Safety Reset Service (see [SRS_2117])
** Changing password is currently not available via CSS services.
** The following rules apply in case a password different from a Null-password
** is used.
** The password array transmitted via CIP Safety is calculated from the string entered for a
** SafetyReset request into Rockwell Logix Designer 5000 V34.00 with this algorithm:
**  - convert string to upper case unicode characters - variable length, no string termination
**  - calculate CRC-S4 with seed value 0xFDFDFDFD
**  - convert to little endian
*/
STATIC CONST CSS_t_CHAR ac8_Password[IXSSO_k_PASSWORD_SIZE] = 
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* Received Safety Reset Type */
STATIC SAPL_t_SAFETY_RESET_TYPE e_PendResetType = SAPL_k_SAFETY_RESET_NONE;

/* Received Attribute Bit Map */
/* Deactivated Lint message 708, because a Union initialization is wanted to save
** init function */
STATIC RDS_UINT8 u8_AttrBitMap = k_ATTR_BIT_MAP_INIT; /*lint !e708 */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC BOOL CheckAicState(void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/*******************************************************************************
**
** Function    : SAPL_IxssoSafetyResetClbk
**
** Description : This callback function is called when the CSS has received a
**               valid Safety_Reset service. The password parameter of the
**               Safety_Reset service is passed also. The Safety Application has
**               to store this event and must then return. The return value of
**               this callback function decides about the returned explicit
**               general status code. The Safety Application shall not call any
**               other CSS API functions from now on. The Safety Application
**               must ensure that all pending HALCS messages are delivered to
**               and executed by CSAL. Then after the response message to the
**               Safety_Reset request is sent out to the network the device must
**               be reset.
**
** Context:      IXSMR_CmdProcess()
**
** Parameters  : u8_resetType (IN) - Type of Safety Reset. Dependent on this
**                                   parameter the Safety Application has to
**                                   restore the default values of the
**                                   non-volatile parameters. Supported values:
**                                   IXSSO_k_RESET_TYPE_0, IXSSO_k_RESET_TYPE_1,
**                                   IXSSO_k_RESET_TYPE_2
**               b_attrBitMap (IN) - Only meaningful in case u8_resetType is
**                                   IXSSO_k_RESET_TYPE_2. Then this bit pattern
**                                   determines which settings shall be
**                                   preserved.
**               ac_checkPwd (IN)  - Safety_Reset password. This must be
**                                   checked by this function. If the check of
**                                   the password in the application fails the
**                                   function shall return with
**                                   CSOS_k_CGSC_PRIVILEGE_VIOLATION
**                                   (see SRS150).
**
** Returnvalue : CSOS_k_CGSC_SUCCESS - Safety_Reset will be performed
**                                     (Password matches, etc. )
**               else                - error: This is the general status code
**                                     that will be returned for this explicit
**                                     service. See the definition of the
**                                     CIP General Status Codes
**                                     (CSOS_k_CGSC_xxx) in CSOSapi.h.
**
*******************************************************************************/
CSS_t_USINT SAPL_IxssoSafetyResetClbk(CSS_t_USINT u8_resetType,
                           CSS_t_BYTE b_attrBitMap,
                           const CSS_t_CHAR ac_checkPwd[IXSSO_k_PASSWORD_SIZE])
{
  /* Callback required according to [CS_MAN], (see [3.6-7:]) */
  
  /* return value, this is the general error code returned for service */
  CSS_t_USINT u8_retVal = CSOS_k_CGSC_PRIVILEGE_VIOLATION;
  
  /* Store received Attribute Bit (only relevant for Safety Reset Type 2 
  ** processing) */
  RDS_SET(u8_AttrBitMap, b_attrBitMap);
  
  /* if Safety Reset is received in invalid AIC state */
  if (CheckAicState() == FALSE)
  {
    /* indicate device state conflict */
    u8_retVal = CSOS_k_CGSC_DEV_STATE_CONFLICT;
  }
  /* else if the password doesn't match */
  else if (CSS_MEMCMP(ac_checkPwd, ac8_Password, IXSSO_k_PASSWORD_SIZE) != 0)
  {
    /* password is wrong, return value already set */
  }
  else /* else: password ok */
  { 
    /* store Safety Reset Type in order to process request outside callback function */
    /* if Safety Reset Type 0 received */
    if (u8_resetType == IXSSO_k_RESET_TYPE_0)
    {

      /* ensure safe state of the outputs, done for more security (see [SRS_2165]) */
      __disable_irq();
      spduOutData_e_IsRunMode = eFALSE;
      __enable_irq();

      /* set pending reset type */
      e_PendResetType = SAPL_k_SAFETY_RESET_TYPE0;
      /* set event to global state machine (see [SRS_2209], [SRS_2210], [SRS_2204], 
      ** [SRS_2212]) */
      aicSm_SetEvent(AICSM_k_EVT_SAFETY_RESET);  
      /* indicate to the caller that the Safety Reset is valid (e.g. password) */
      u8_retVal = CSOS_k_CGSC_SUCCESS;
    }
    /* else if Safety Reset Type 1 received */
    else if (u8_resetType == IXSSO_k_RESET_TYPE_1)
    {
      /* ensure safe state of the outputs, done for more security (see [SRS_2165]) */
      __disable_irq();
      spduOutData_e_IsRunMode = eFALSE;
      __enable_irq();

      /* set pending reset type */
      e_PendResetType = SAPL_k_SAFETY_RESET_TYPE1;
      /* set event to global state machine (see [SRS_2209], [SRS_2210], [SRS_2204], 
      ** [SRS_2212]) */
      aicSm_SetEvent(AICSM_k_EVT_SAFETY_RESET);  
      /* indicate to the caller that the Safety Reset is valid (e.g. password) */
      u8_retVal = CSOS_k_CGSC_SUCCESS; 
    }
    /* else if Safety Reset Type 2 received */
    else if (u8_resetType == IXSSO_k_RESET_TYPE_2)
    {
      /* ensure safe state of the outputs, done for more security (see [SRS_2165]) */
      __disable_irq();
      spduOutData_e_IsRunMode = eFALSE;
      __enable_irq();

      /* set pending reset type */
      e_PendResetType = SAPL_k_SAFETY_RESET_TYPE2;
      /* set event to global state machine (see [SRS_2209], [SRS_2210], [SRS_2204], 
      ** [SRS_2212]) */
      aicSm_SetEvent(AICSM_k_EVT_SAFETY_RESET);  
      /* indicate to the caller that the Safety Reset is valid (e.g. password) */
      u8_retVal = CSOS_k_CGSC_SUCCESS; 
    }
    /* else: unsupported Safety Reset Type */
    else
    {
      /* since the CSS shall ensure that other Safety Resets Types are blocked,
      ** this section should never be reached.
      */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_UNEXP_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(1u));
    }
  }
  
  /* ATTENTION: No other CSS API function shall be called by safety application afterwards */ 
  return (u8_retVal);
}

/***************************************************************************************************
  Function:
    SAPL_ResetTypeGet

  Description:
    This function returns the current pending Safety Reset Type. Additionally the received
    Attribute Bit Map is delivered by the function.
    Note: The Attribute Bit Map is only relevant for Safety Reset Type 2.

  See also:
    -

  Parameters:
    pu8_attrBitMap (OUT) - Address to store the received 'Attribute Bit Map'
                           (valid: <>NULL, not checked, only called with reference to variable)

  Return value:
    SAPL_t_SAFETY_RESET_TYPE

  Remarks:
    Context: Background Task

***************************************************************************************************/
SAPL_t_SAFETY_RESET_TYPE SAPL_ResetTypeGet(UINT8* pu8_attrBitMap)
{
  /* Check/get value of RDS variable */
  *pu8_attrBitMap = RDS_GET(u8_AttrBitMap);
  
  return e_PendResetType;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    CheckAicState

  Description:
    This function is used to check if the Safety Reset Request is accepted in the current AIC
    state.

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                     - Application grants the permission of safety request.
    FALSE                    - Application denies the safety reset request.

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC BOOL CheckAicState(void)
{
  /* return value, indicates if SafetyReset shall be accepted */
  BOOL b_stateValid = FALSE;
  
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /* check current AIC state */ 
  switch(e_aicState)
  {
    case AICSM_AIC_IDLE:               /* fall through */
    case AICSM_AIC_WAIT_TUNID:         /* fall through */
    case AICSM_AIC_ABORT:              /* fall through */
    case AICSM_AIC_WAIT_FOR_CONFIG:    /* fall through */
    case AICSM_AIC_PROCESS_CONFIG:     /* fall through */
    case AICSM_AIC_INVALID_CONFIG:     /* fall through */
    {
      /* Safety Reset accepted, see [SRS_2204], [SRS_2209], [SRS_2210], [SRS_2212] */
      b_stateValid = TRUE;
      break;
    }
    case AICSM_AIC_EXEC_PROD_ONLY:     /* fall through */
    case AICSM_AIC_EXEC_CONS_ONLY:     /* fall through */
    case AICSM_AIC_EXEC_PROD_CONS:     /* fall through */
    case AICSM_AIC_STARTUP:            /* fall through */
    case AICSM_AIC_WAIT_FOR_INIT:      /* fall through */
    case AICSM_AIC_START_CSS:          /* fall through */
    case AICSM_AIC_WAIT_RESET:         /* fall through */
    {
      /* In these AIC states it is not expected to receive a Safety Reset Request,
      ** so blocked. */
      /* return value already set */
      break;
    }
    default:
    {
      /* invalid value of variable */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(11u));
      break;
    }
  }
  return b_stateValid;
}


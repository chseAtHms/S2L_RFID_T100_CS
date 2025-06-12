/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLfailCode.c
**     Summary: This module implements the Failure Log of the T100/CS. During startup the failure
**              codes of the flash are exchanged and stored inside RAM. This enables the user
**              to read the failure codes during runtime.
**   $Revision: 2437 $
**       $Date: 2017-03-24 13:26:24 +0100 (Fr, 24 Mrz 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_FailCodeInit
**             SAPL_FailCodeSync
**             SAPL_FailCodeLogGet
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
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"
#include "timer-sys.h"

/* CSS common headers */
#include "CSSplatform.h"

#include "SAPLfailCode.h"


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Reserved NV section used for Fail Safe failure codes (see [SRS_2242]) */
/* Flash page shall fit to page written in Safety Handler */ 
#define k_FAILURECODE_FLSH_ADDR_START    (UINT32)(0x0803f800u)
#define k_FAILURECODE_FLSH_SIZE          (UINT16)(0x0800u)

/* Size of failure code + additional info */
#define k_FAILURECODE_FLSH_BLK_SIZE       (UINT16)(sizeof(t_FAILURE))
/* Number of failure code + additional info usable in page */ 
#define k_FAILURECODE_FLSH_BLK_MAX_CNT    (UINT16)(k_FAILURECODE_FLSH_SIZE / \
                                                   k_FAILURECODE_FLSH_BLK_SIZE)

/* structure of single failure code entry */
typedef __packed struct
{
  UINT16 u16_failureCode;
  UINT32 u32_addInfo;
} t_FAILURE;

/* structure of complete failure code block */
typedef struct 
{
  t_FAILURE as_flshBlk[k_FAILURECODE_FLSH_BLK_MAX_CNT];
} t_FLSH_FAILURE_AREA;

/* const pointer to failure code block */
/* Deactivate lint, cast is necessary because a fixed address is used*/
STATIC CONST t_FLSH_FAILURE_AREA* CONST ps_FlshFailureBlock =
    (t_FLSH_FAILURE_AREA*)k_FAILURECODE_FLSH_ADDR_START; /*lint !e923*/

/* Number of Failure Codes stored inside Failure Code Block */
STATIC UINT16 u16_NumFailureCodes;

/* Failure log, containing the last SAPL_k_NUM_READABLE_FAILURES failsafe error codes of
** the own controller */
STATIC t_FAILURE as_LocalFailure[SAPL_k_NUM_READABLE_FAILURES];
    
/* Failure log, containing the last SAPL_k_NUM_READABLE_FAILURES failsafe error codes of
** both controllers */
STATIC SAPL_t_FAILURE_LOG  s_FailureLog;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    SAPL_FailCodeInit

  Description:
    This function is used to read the last SAPL_k_NUM_READABLE_FAILURES failure codes from the
    Failure Code Block of the flash.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_FailCodeInit(void)
{
  /* iterator */
  UINT16 i;
  /* number of failures found */
  UINT16 u16_failureCnt = 0u;
  /* end of Failure Code Block Found */
  BOOL b_isEndFound = FALSE;
  
  /* initialize module static variables */
  u16_NumFailureCodes = 0u;
  CSS_MEMSET(&s_FailureLog, 0u, sizeof(s_FailureLog));
  CSS_MEMSET(as_LocalFailure, 0u, sizeof(as_LocalFailure));
  
  /* search for failure code in failure code block */
  for (i=0U; (i < k_FAILURECODE_FLSH_BLK_MAX_CNT) && (b_isEndFound == FALSE); i++)
  {
    /* if failure code found */
    if (ps_FlshFailureBlock->as_flshBlk[i].u16_failureCode != 0xFFFFu)
    {
      /* increment failure counter */
      u16_failureCnt++;
    }
    /* else: no failure code found */
    else
    {
      /* set flag to leave loop */
      b_isEndFound = TRUE;
    }
  }
  
  /* store number of failure codes */
  u16_NumFailureCodes = u16_failureCnt;

  /* store failure codes */
  for (i=0u; i<SAPL_k_NUM_READABLE_FAILURES; i++)
  {
    /* if (another) failure code found */
    if (u16_failureCnt > 0u)
    {    
      /* store for further processing */
      as_LocalFailure[i].u16_failureCode = 
        ps_FlshFailureBlock->as_flshBlk[u16_failureCnt-1u].u16_failureCode;
      as_LocalFailure[i].u32_addInfo     = 
        ps_FlshFailureBlock->as_flshBlk[u16_failureCnt-1u].u32_addInfo;
      u16_failureCnt--;
    }
    /* no (more) failure code found */
    else
    {
      /* empty brach, zero for u16_failureCode and u32_addInfo already set */
    }
  }    
}


/***************************************************************************************************
  Function:
    SAPL_FailCodeSync

  Description:
    This function is used to exchange the number of stored failure code and the failure codes
    itself with the other controller.
    The number of exchanged failure codes can be configured via SAPL_k_NUM_READABLE_FAILURES.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_FailCodeSync(void)
{
  UINT32 u32_sendData;
  UINT32 u32_recvData;
  UINT16 i;
  t_FAILURE s_recvFailure;
  
  /* exchange number of stored failure codes */
  u32_sendData = u16_NumFailureCodes;
  u32_recvData = ipcxSYS_ExChangeUINT32Data(u32_sendData, IPCXSYS_IPC_ID_FAILCODE_NUM);
  
  /* store number of both controllers inside log */
  s_FailureLog.u16_numFailOwnCtrl   = u16_NumFailureCodes;
  s_FailureLog.u16_numFailOtherCtrl = (UINT16)u32_recvData;


  /* exchange failure codes itself */
  for (i=0u; i<SAPL_k_NUM_READABLE_FAILURES; i++)
  {
    /* send failure code */
    /* Cast from pointer to pointer is intended here because the underlying function directly read 
    ** from structure */
    ipcxSYS_SendBuffer(IPCXSYS_IPC_ID_FAILCODE_VAL, 
                       (UINT8)sizeof(as_LocalFailure[0]), 
                       (CONST UINT8*)&as_LocalFailure[i]); /*lint !e928*/
    
    /* wait time until DMA transfer is expected to be finished */
    timerSYS_wait(50u); /* 200 bits (10 bits per byte x 20 bytes), 4 MHz */
    /* receive failure code */
    /* Cast from pointer to pointer is intended here because the underlying function directly writes
    ** into structure */
    ipcxSYS_GetBufferinclWait(IPCXSYS_IPC_ID_FAILCODE_VAL, 
                              (UINT8)sizeof(s_recvFailure),
                              (UINT8*)&s_recvFailure); /*lint !e928*/
    
    /* store failure codes inside log */
    s_FailureLog.as_failure[i].u16_failCodeOwn   = as_LocalFailure[i].u16_failureCode;
    s_FailureLog.as_failure[i].u32_addInfoOwn    = as_LocalFailure[i].u32_addInfo;
    s_FailureLog.as_failure[i].u16_failCodeOther = s_recvFailure.u16_failureCode;
    s_FailureLog.as_failure[i].u32_addInfoOther  = s_recvFailure.u32_addInfo;
  }
}

/***************************************************************************************************
  Function:
    SAPL_FailCodeLogGet

  Description:
    This function is used to get a pointer to the Failure Code Log that was built during startup.
    The Failure Code Log contains the last SAPL_k_NUM_READABLE_FAILURES failure code.

  See also:
    -

  Parameters:
    -

  Return value:
    SAPL_t_FAILURE_LOG*  - Pointer of Failure Code Log

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST SAPL_t_FAILURE_LOG* SAPL_FailCodeLogGet(void)
{
  return &s_FailureLog;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: globErrLog.c
**     Summary: This module implements the Error Event Log that stores non-safe relevant 
**              errors/events detected by the software.
**   $Revision: 2437 $
**       $Date: 2017-03-24 13:26:24 +0100 (Fr, 24 Mrz 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: globErrLog_ErrorAdd
**             globErrLog_ErrorLogGet
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

#include "globErrLog.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* s_ErrorLog:
** Error Event log used for all non-fail-safe error. The Error Event log is
** implemented as ring buffer which means the oldest entry is overwritten by the
** newest one. (see [SRS_2163]).
*/
STATIC volatile GLOBERRLOG_t_LOG s_ErrorLog = { {0u, 0u, 0u, 0u} };

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    globErrLog_ErrorAdd

  Description:
    This function adds an new entry in the 'Error Event log'. The 'Error Event log' is implemented 
    as ring buffer which means that the oldest entry is overwritten.
    The Error Event log is able to capture k_ERROR_LOG_ENTRIES entries.
    The Error Event log can be read via AIC command AICMSGHDL_AM_GET_ASM_STATUS_REQ.

  See also:
    -

  Parameters:
    u16_errorCode (IN)     - Error Code to be stored inside Error Event log
                             (not checked, any value possible)

  Return value:
    -

  Remarks:
    Context: Background Task
    Context: IRQ Scheduler

***************************************************************************************************/
void globErrLog_ErrorAdd(UINT16 u16_errorCode)
{
  UINT8 i;
  
  /* Attention: variable is accessed from background task and IRQ scheduler, so disable IRQs
  ** temporarily */
  __disable_irq();
  
  /* Re-Organization of error codes so that the newest error code could be stored at index 0 */
  for (i = (GLOBERRLOG_k_NUM_ENTRIES-1u); i > 0u; i--)
  {
    s_ErrorLog.au16_errEntry[i] = s_ErrorLog.au16_errEntry[i-1u];
  }
  /* add entry */
  s_ErrorLog.au16_errEntry[0u] = u16_errorCode;
  
  /* enable IRQs again */
  __enable_irq();
}

/***************************************************************************************************
  Function:
    globErrLog_ErrorLogGet

  Description:
    This function is used to get a pointer to the Error Event Log of non-safe relevant error.
    The Error Event Log contains the last GLOBERRLOG_k_NUM_ENTRIES codes.

  See also:
    -

  Parameters:
    -

  Return value:
    GLOBERRLOG_t_LOG*      - Pointer of Error Event Log

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
CONST volatile GLOBERRLOG_t_LOG* globErrLog_ErrorLogGet(void)
{
  return &s_ErrorLog;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/



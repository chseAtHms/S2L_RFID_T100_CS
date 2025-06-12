/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: globErrLog.h
**     Summary: Interface header of module globErrLog.c
**   $Revision: 2242 $
**       $Date: 2017-02-27 16:24:29 +0100 (Mo, 27 Feb 2017) $
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

#ifndef GLOBERRLOG_H
#define GLOBERRLOG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* GLOBERRLOG_k_NUM_ENTRIES:
** Number of entries that can be stored in the error log
*/
#define GLOBERRLOG_k_NUM_ENTRIES      4u

/*------------------------------------------------------------------------------
**
** GLOBERRLOG_FILE
**
** Description:
** Used to build the error log with 7 bit file id and 6 bit error id.
** This requires to define k_FILEID in each module using this macro. The following
** schema is applied:
**  +---------++---------+----------+------------+--------------+
**  | bit pos ||  15-14  |    13    |   12-06    |   05-00      |
**  +---------++---------+----------+------------+--------------+
**  | meaning || errType | CSS/CSAL |   FileID   |   errId      |
**  +---------++---------+----------+------------+--------------+
**                 00         0            x             x       
**
**  Bits 15-14 are fixed set to 00 because only non-failsafe error are added into
**  error log.
**  Bit 13 is fixed set to 0 because 1 is reserved for CSS.
**  
**------------------------------------------------------------------------------
** Inputs:
**    errId      Error id used to build the additional info
**
** Outputs:
**    -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
#define GLOBERRLOG_FILE(errId)                                                   \
                       (UINT16)( (UINT16)(((UINT16)(k_FILEID) & 0x007Fu) <<  6) | \
                                 (UINT16)((UINT16)(errId) & 0x003Fu) )

/***************************************************************************************************
**    data types
***************************************************************************************************/

/* Structure of the Global (non-safe) Error Log */
typedef struct
{
  UINT16 au16_errEntry[GLOBERRLOG_k_NUM_ENTRIES];
} GLOBERRLOG_t_LOG;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
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
void globErrLog_ErrorAdd(UINT16 u16_errorCode);

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
CONST volatile GLOBERRLOG_t_LOG* globErrLog_ErrorLogGet(void);

#endif


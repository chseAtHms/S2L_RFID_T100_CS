/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCFmain.c
**    Summary: IXSCF - Safety Control Flow Monitoring
**             This unit controls the program flow of the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCF_Init
**             IXSCF_PathGetReset
**             IXSCF_PathTrack
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSStypes.h"

#include "IXSCFapi.h"
#include "IXSCF.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** u32_PathProgress:
    This module global variable records the branching of the program flow. It is
    implemented as a counter.
*/
/* Not relevant for Soft Error Checking - only a diagnostic value - additionally
   this value is cross checked in context of Control Flow Monitoring */
static CSS_t_UDINT u32_PathProgress;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCF_Init
**
** Description : This function initializes the IXSCF unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCF_Init(void)
{
  u32_PathProgress = 0U;
}


/*******************************************************************************
**
** Function    : IXSCF_PathGetReset
**
** Description : This function returns the number of function calls and
**               significant program flow branches. Precondition is that every
**               function call and every significant program flow branch was
**               tracked before with IXSCF_PATH_TRACK(). Additionally the
**               internal counter is reset to zero.
**
**               **Attention**: The Safety Application (SAPL) has to compare the
**               counter value on both safety controllers and in case of a
**               mismatch it has to set the system into a safe state
**               (see Asm.10.4.1-3).
**
** Parameters  : -
**
** Returnvalue : <number of tracks> - number of function calls/ flow branches
**
*******************************************************************************/
CSS_t_UDINT IXSCF_PathGetReset(void)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = u32_PathProgress;

  /* reset internal counter */
  u32_PathProgress = 0U;

  return (u32_retVal);
}


/*******************************************************************************
**
** Function    : IXSCF_PathTrack
**
** Description : This function registers a sub function call or a significant
**               branch in the program flow.
**
** Parameters  : -
**
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCF_PathTrack(void)
{
  u32_PathProgress++;
}


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


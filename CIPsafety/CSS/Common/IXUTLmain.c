/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXUTLmain.c
**    Summary: IXUTL - Utility Routines
**             This module contains utility functions that can be used from the
**             whole CSS.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXUTL_Init
**             IXUTL_SetSysTime
**             IXUTL_GetSysTime
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSStypes.h"

#include "IXSCF.h"

#include "IXUTLapi.h"
#include "IXUTL.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** u32_SysTime_128us:
    variable storing the system time. Accessible to other units only through
    functions.
*/
/* Not relevant for Soft Error Checking - part of the safety function but value
   is updated before each use */
static CSS_t_UDINT u32_SysTime_128us;


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXUTL_Init
**
** Description : This function initializes the IXUTL unit.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXUTL_Init(void)
{
  u32_SysTime_128us = 0U;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXUTL_SetSysTime
**
** Description : This API function must be called by the application to set the
**               system time for the CIP Safety Stack.
**
**               **Attention**:
**               This function must be called cyclically to update the system
**               time. Any delay in calling this function after an increment of
**               the real system time will directly affect the system reaction
**               time.
**               Not calling this function may result in not going into the fail
**               safe state!
**
**               **Attention**:
**               To prevent overflows in internal timing calculations the
**               parameter u32_systemTime_128us shall not progress more than
**               15625 ticks between 2 consecutive calls. In other words: this
**               function shall be called at least every 2 seconds (see FRS145
**               and FRS146).
**
**               **Attention**:
**               The time value passed to this function must be
**               identical/synchronized on both processors executing the CSS.
**
**               **Attention**:
**               The system time / clock count shall have tolerances and
**               stability within +/- 0.02% or 200PPM (see FRS244).
**
** Parameters  : u32_sysTime_128us (IN) - Current system time. System time is a
**                                        32-bit counter incremented in 128us
**                                        steps.
**                                        (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXUTL_SetSysTime(CSS_t_UDINT u32_sysTime_128us)
{
  /* copy the passed value to the module global static variable */
  u32_SysTime_128us = u32_sysTime_128us;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXUTL_GetSysTime
**
** Description : This function returns the current system time. System time is a
**               32-bit counter incremented in 128us steps.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UDINT - current system time
**
*******************************************************************************/
CSS_t_UDINT IXUTL_GetSysTime(void)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_SysTime_128us);
}


/*** End Of File ***/


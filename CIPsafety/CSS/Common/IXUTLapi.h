/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXUTLapi.h
**    Summary: IXUTL - Utility Routines
**             Interface definition of the IXUTL unit of the CSS.
**             This unit provides utility routines and definitions, e.g. setting
**             of the system time for CSS.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXUTL_SetSysTime
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXUTL_API_H
#define IXUTL_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

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
void IXUTL_SetSysTime(CSS_t_UDINT u32_sysTime_128us);


#endif /* IXUTL_API_H */

/*** End of File ***/


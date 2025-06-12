/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCFapi.h
**    Summary: IXSCF - Safety Control Flow Monitoring
**             Interface definition of the IXSCF unit of the CSS.
**             This unit provides functionality to register stack internal
**             function calls and significant program flow branches. The
**             registration of these points during program flow is made by
**             tracking these points.
**             (see Req.10.4.1-2)
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCF_PathGetReset
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCF_API_H
#define IXSCF_API_H


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

/*
**  Functions provided by the IXSCF Unit
*/

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
CSS_t_UDINT IXSCF_PathGetReset(void);


#endif /* IXSCF_API_H */

/*** End of File ***/


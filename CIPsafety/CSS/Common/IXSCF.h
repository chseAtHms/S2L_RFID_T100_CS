/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCF.h
**    Summary: IXSCF - Safety Control Flow Monitoring
**             IXSCF.h is the export header file of the IXSCF unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCF_Init
**             IXSCF_PathTrack
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCF_H
#define IXSCF_H


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
** Function    : IXSCF_Init
**
** Description : This function initializes the IXSCF unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCF_Init(void);


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
void IXSCF_PathTrack(void);


/*******************************************************************************
**
** Function    : IXSCF_PATH_TRACK
**
** Description : This macro registers a sub function call or a significant
**               branch in the program flow.
**
**               **Attention**: The macro (instead of the function) shall be
**               used for the program flow monitoring (for later optimizations
**               purposes).
**
** Parameters  : -
**
**
** Returnvalue : -
**
*******************************************************************************/
#define IXSCF_PATH_TRACK()      IXSCF_PathTrack()


#endif /* #ifndef IXSCF_H */

/*** End Of File ***/


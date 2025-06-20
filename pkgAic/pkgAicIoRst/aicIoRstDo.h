/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicIoRstDo.h
**     Summary: Interface of the aicIoRstDo module
**   $Revision: 2618 $
**       $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicIoRstDo_Init
**             aicIoRstDo_ChkAutoErrRst
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICIORSTDO_H
#define AICIORSTDO_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


/***************************************************************************************************
  Function:
    aicIoRstDo_Init

  Description:
    This function is used for module initialization. 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicIoRstDo_Init(void);

/***************************************************************************************************
  Function:
    aicIoRstDo_ChkAutoErrRst

  Description:
    This function has to be called once within T-cycl for the processing of the automatic error 
    reset feature. The function processes the internal state machines of the DO channels.
    The result of the calculations (error reset bits) is forwarded to the DO module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicIoRstDo_ChkAutoErrRst(void);

#endif


/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLipc.h
**     Summary: Interface header of module SAPLipc.c
**   $Revision: 1695 $
**       $Date: 2017-01-12 13:50:42 +0100 (Do, 12 Jan 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPLipc_SetStackVarAndPath
**             SAPLipc_SetNvCrc
**             SAPLipc_CheckNvCrc
**             SAPLipc_SendVar
**             SAPLipc_RecvAndCheckVar
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_IPC_H
#define SAPL_IPC_H

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
    SAPLipc_SetStackVarAndPath

  Description:
    This function reads the CSS variables used for soft error supervision (see [SRS_2075], [3.2-6:])
    and the program flow information of the CSS (see [SRS_2077]).

    The variables are set inside IPC structure:
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)

    The set value is exchanged with the other controller instance. For this purpose the function
    SAPLipc_SendVar shall be called.

    ATTENTION: Shall not be called if CSS is running in background task!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SetStackVarAndPath(void);

/***************************************************************************************************
  Function:
    SAPLipc_SetNvCrc

  Description:
    This function is used to set inside IPC structure:
      - the calculated CRC of the new non-volatile flash block.

    The set value is exchanged with the other controller instance. For this purpose the function
    SAPLipc_SendVar shall be called.

  See also:
    SAPLipc_CheckNvCrc

  Parameters:
    u16_nvCrc (IN)           - Calculated flash block CRC (NV CRC)
                               (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SetNvCrc(UINT16 u16_nvCrc);

/***************************************************************************************************
  Function:
    SAPLipc_CheckNvCrc

  Description:
    This function is used to compare the sent NV CRC with the one received via IPC.
    In case of mismatch, the failsafe state is entered.

    It is task of the calling function to ensure that the IPC transfer was done before this function
    is called.

  See also:
    SAPLipc_SetNvCrc

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_CheckNvCrc(void);

/***************************************************************************************************
  Function:
    SAPLipc_SendVar

  Description:
    This function is used to send via IPC to other controller
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)
      - CRC16 used for NV memory handling
   
  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SendVar(void);

/***************************************************************************************************
  Function:
    SAPLipc_RecvAndCheckVar

  Description:
    This function is used to receive via IPC from other controller
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)

    The software shall compare the received values with the sent ones. In case of any differences,
    the failsafe state is entered (see [SRS_2076], [SRS_2078]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_RecvAndCheckVar(void);

#endif


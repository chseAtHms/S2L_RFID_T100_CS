/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgInit.h
**     Summary: Interface header of module aicMsgInit.c
**   $Revision: 2387 $
**       $Date: 2017-03-17 12:38:18 +0100 (Fr, 17 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgInit_SetData
**             aicMsgInit_GetData
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_INIT_H
#define AICMSG_INIT_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/*lint -esym(768, u16_incarnationId) global struct member 'u16_incarnationId' not referenced */
/* Deactivated because the Incarnation Id is still part of the SetInitdata request but no used 
** by CSS any more */

/* Data of 'SetInitData' request received from AM during startup/init [SIS_203], [SRS_2121] */
typedef __packed struct
{
  UINT32 u32_serialNumber;
  UINT32 u32_nodeId;        /* [SRS_2121]*/
  UINT16 u16_vendorId;
  UINT16 u16_deviceType;
  UINT16 u16_productType;
  UINT16 u16_incarnationId;
  UINT8  u8_majorRevision;
  UINT8  u8_minorRevision;
} AICMSGINIT_CSSINITDATA;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgInit_SetData

  Description:
    To be called when a message request containing CSS initialization data is received.
    'SetInitData' requests are "fragmented" messages, means more than one telegram is needed to 
    transmit all data (or the whole message).
    The function will count the number of received fragments and indicate if the message is still 
    pending (not all parts/fragments received) or not (all fragments of message received).
    Hint: Message fragmentation is described in Serial Interface Specification[SIS] and Software 
    Requirements Specification[SRS]

  See also:
    -

  Parameters:
    ps_rxMsg (IN)            - Received Message from AM
                               (valid range: <>NULL, not checked, only called with reference to
                               static/global buffer)

  Return value:
    TRUE                     - SetInitData received completely
    FALSE                    - Still fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgInit_SetData (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg);

/***************************************************************************************************
  Function:
    aicMsgInit_GetData

  Description:
    This function is used to read the CSS initialization data that was received during startup 
    from ABCC.

  See also:
    -

  Parameters:
    -

  Return value:
    AICMSGINIT_CSSINITDATA*  - Pointer of CSS initialization data

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST volatile AICMSGINIT_CSSINITDATA* aicMsgInit_GetData(void);

#endif


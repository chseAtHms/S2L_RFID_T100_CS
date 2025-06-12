/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgProc.h
**     Summary: Interface header of the module aicMsgProc.c
**   $Revision: 2869 $
**       $Date: 2017-04-28 07:52:36 +0200 (Fr, 28 Apr 2017) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgProc_Init
**             aicMsgProc_ProcessMsg
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSGHDLPROCESS_H
#define AICMSGHDLPROCESS_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgProc_s_RcvdMsg
** non-safe message from AM to ASM, global used due to performance reasons
** formerly aicDataStack_sAnybusMsgRx
*/
extern AICMSGDEF_NON_SAFE_MSG_STRUCT aicMsgProc_s_RcvdMsg;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgProc_Init

  Description:
    Calls functions which init variables of modules

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void aicMsgProc_Init(void);

/***************************************************************************************************
  Function:
    aicMsgProc_ProcessMsg

  Description:
    process the incoming non-safe message in following steps:
    - process of the received ctrl/status byte
    - checks if a new fragment is received (check of Anybus-Fragmentation Bit)
    - if yes, the non-safe message will be processed:
       - if the received message is a request, the processing of a request is
         called
       - else the processing of a response is called
    - otherwise, non-safe message will be ignored

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgProc_ProcessMsg (void);

#endif


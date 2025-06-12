/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCss.h
**     Summary: Interface header of module aicMsgCss.c
**   $Revision: 2387 $
**       $Date: 2017-03-17 12:38:18 +0100 (Fr, 17 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCss_BuildReqMsg
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_CSS_H
#define AICMSG_CSS_H

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
    aicMsgCss_BuildReqMsg

  Description:
    builds/prepares a message request (request from ASM to AM/ABCC).
    Currently, there are three possible AIC requests types:
    - HalcCssMessage
    - LEDstate
    - SafetyReset
    All messages requested by SAPL or CSS are mapped to one of these request types.
    The message request is specified in the Serial Interface Specification [SIS],
    in this document the structure of the message request and the meaning of the several requests
    is defined.
    The function fulfills also [SRS_440].

  See also:
    -

  Parameters:
    ps_halcMsg (IN)          - HALCS Message located inside HALCS queue
                               (valid range: <>NULL, not checked, already checked in
                                aicMsgCss_BuildMsg)
    ps_txMsg (OUT)           - Buffer of transmit message
                               (valid range: <>NULL, not checked, only called with reference to
                               buffer)

  Return value:
    <>0                      - Request generated, Expected response ID
    0                        - Request pending (fragmented transfer, more fragments necessary)

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT16 aicMsgCss_BuildReqMsg (CONST volatile HALCS_t_AICMSG *ps_halcMsg,
                              AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_txMsg);
#endif


/***************************************************************************************************
**    Copyright (C) 2015-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: HALCSmsg.h
**     Summary: Interface of HALCS for non-safe message handling.
**   $Revision: 3418 $
**       $Date: 2020-04-07 14:31:08 +0200 (Di, 07 Apr 2020) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: HALCS_MsgInit
**             HALCS_MsgTxReqSafetyReset
**             HALCS_MsgTxReqGet
**             HALCS_MsgTxReqRelease
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef HALCS_MSG_H
#define HALCS_MSG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Definition of the unit ID. This ID is part of the HALC Command Identifier */
/* Shall not be in conflict with existing CSS or CSAL unit IDS, see CSOSapi.h */
#define HALCS_k_HALCS_ID                0x80u


/** HALCS_k_CMD_xxx:
    Command definitions:
    These defines represent the commands that pass the HALC layer.
*/
#define HALCS_k_CMD_MOD_STAT_LED_SET       (UINT16)CSOS_HalcCmdAsm(HALCS_k_HALCS_ID,  0x01U)
#define HALCS_k_CMD_NET_STAT_LED_SET       (UINT16)CSOS_HalcCmdAsm(HALCS_k_HALCS_ID,  0x02U)
#define HALCS_k_CMD_SAFETY_RESET_TYPE0     (UINT16)CSOS_HalcCmdAsm(HALCS_k_HALCS_ID,  0x03U)
#define HALCS_k_CMD_SAFETY_RESET_TYPE1     (UINT16)CSOS_HalcCmdAsm(HALCS_k_HALCS_ID,  0x04U)
#define HALCS_k_CMD_SAFETY_RESET_TYPE2     (UINT16)CSOS_HalcCmdAsm(HALCS_k_HALCS_ID,  0x05U)


/***************************************************************************************************
**    data types
***************************************************************************************************/

/** HALCS_t_AICMSG:
    This type defines the messages that are passed to AIC and sent to ABCC
*/
typedef struct
{
  UINT16  u16_cmd;                                    /* Command code */
  UINT16  u16_len;                                    /* Data length of the passed message */
  UINT32  u32_addInfo;                                /* Additional information */
  UINT8   au8_data[HALCSMSG_cfg_REQ_PAYLOAD_SIZE];    /* Data */
} HALCS_t_AICMSG;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    HALCS_MsgInit

  Description:
    This function is used to initialize the module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void HALCS_MsgInit(void);

/***************************************************************************************************
  Function:
    HALCS_MsgTxReqSafetyReset

  Description:
    This function is called from the safety application if an valid Safety Reset Request was 
    received from Originator. It is task of this function to forward this request to ABCC via
    Request Command Queue. 
    The ABCC itself will trigger a request to non-safe host then (see [SRS_2117]).
    
    Attention: The following Safety Supervisor Safety Reset Types are supported:
     - Safety Reset Type '0':
       ('Emulate as closely as possible cycling power on the device')
     - Safety Reset Type '1':
       ('Return as closely as possible to the default configuration, and then emulate cycling
        power as closely as possible')
     - Safety Reset Type '2':
       ('Return as closely as possible to the out-of-box configuration except to preserve the
        parameters indicated by the Attribute Bit Map, and then emulate cycling power as
        closely as possible.')

  See also:
    -

  Parameters:
    u16_resetCmd (IN)       - Requested Reset type
                              (valid: HALCS_k_CMD_SAFETY_RESET_TYPE0, 
                              HALCS_k_CMD_SAFETY_RESET_TYPE1, 
                              HALCS_k_CMD_SAFETY_RESET_TYPE2, checked)
    u8_attrBitMap (IN)      - Attribute Bit Map provided by CSS.
                              (valid: any value, not checked)
                                  

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void HALCS_MsgTxReqSafetyReset(UINT16 u16_resetCmd, UINT8 u8_attrBitMap);

/***************************************************************************************************
  Function:
    HALCS_MsgTxReqGet

  Description:
    This function returns a request message stored inside the request command queue.
    In case several requests are waiting, this function always returns the oldest request.
    Attention: in order to remove the request from the queue, the function HALCS_MsgTxReqRelease
    shall be called after processing.

  See also:
    HALCS_MsgTxReqRelease

  Parameters:
    -

  Return value:
    NULL            - No data available, request queue is empty
    <>NULL          - Pointer to received request message data.

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
CONST volatile HALCS_t_AICMSG* HALCS_MsgTxReqGet(void);

/***************************************************************************************************
  Function:
    HALCS_MsgTxReqRelease

  Description:
    This function releases the oldest received request message from the request message queue.

  See also:
    HALCS_MsgTxReqGet

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void HALCS_MsgTxReqRelease(void);

#endif


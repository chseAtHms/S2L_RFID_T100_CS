/***************************************************************************************************
**    Copyright (C) 2015-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: HALCSmsg.c
**     Summary: This module implements the functionality of non-safe message handling of 
**              AIC telegrams. The module is part of the HALCS implementation (see [1.4-3:]).
**   $Revision: 3418 $
**       $Date: 2020-04-07 14:31:08 +0200 (Di, 07 Apr 2020) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: HALCS_TxDataPut
**             HALCS_ModStatLedSet
**             HALCS_NetStatLedSet
**             HALCS_MsgInit
**             HALCS_MsgTxReqSafetyReset
**             HALCS_MsgTxReqGet
**             HALCS_MsgTxReqRelease
**
**             RcqPut
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "globErrLog.h"
#include "rds.h"

/* CSOS common headers */
#include "CSOSapi.h"
/* CSS common headers */
#include "CSStypes.h"
#include "CSSplatform.h"

/* HALCS headers */
#include "HALCSapi.h"
#include "HALCSmsg_cfg.h"
#include "HALCSmsg.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      13u

/* queue management variables for the "Request Command Queue" */

/* free queue entry, RDS variable 
** Attention: Accessed from background task and IRQ scheduler! */
STATIC volatile RDS_UINT16 u16_RcqNumFree;

/* queue index to read from, RDS variable */
STATIC RDS_UINT16 u16_RcqRdIdx;
/* queue index to write into, RDS variable */
STATIC RDS_UINT16 u16_RcqWrIdx;

/* memory of the "Request Command Queue" */
/* HALCS AIC Message queue */
STATIC volatile HALCS_t_AICMSG as_Rcq[HALCSMSG_cfg_RCQ_SIZE];

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC CSS_t_BOOL RcqPut(const HALCS_t_MSG *ps_msg);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/*******************************************************************************
**
** Function    : HALCS_TxDataPut
**
** Description : This function sends data from the CSS to CSAL.
**
**               **Attention**
**               The {HALCS_t_MSG} structure elements u16_cmd, u16_len,
**               u32_addInfo are provided in processor format. If the processor
**               format of HALC and HALCS is different then HAL shall ensure the
**               correct endianness. The endianness of pb_data is handled by the
**               CSAL and CSS.
**
** See Also    : HALCS_TxDataPutMix()
**
** Parameters  : ps_msg (IN)     - Pointer to the message structure to be sent,
**                                 see {HALCS_t_MSG}
**
** Returnvalue : CSS_k_TRUE      - Sending of message was successful
**               CSS_k_FALSE     - Error during message sending
**
*******************************************************************************/
CSS_t_BOOL HALCS_TxDataPut(const HALCS_t_MSG *ps_msg)
{
  /* Interface function required according to [CS_MAN], (see [1.4-3:]) */
  
  /* put request into "Request Command Queue" */
  return RcqPut(ps_msg);
  
  /* Because the command is queued, there is no feedback here available if the 
  ** request is really transferred.
  */  

}

/*******************************************************************************
**
** Function    : HALCS_ModStatLedSet
**
** Description : This function is used by the CSS to set the state of the Module
**               Status LED. CIP Safety devices are required to support a Module
**               Status LED. The state of the indicator is under control of the
**               Supervisor Object state machine. CSS will call this function
**               whenever the status of the Module Status LED changes. It is the
**               responsibility of this HALC function to implement the blink
**               patterns and to ensure that upon power on the LED is initially
**               in the "off" state.
**
** See Also    : HALCS_NetStatLedSet()
**
** Parameters  : e_msLedState (IN) - state to which the Module status shall be
**                                   set. See type definition of
**                                   {HALCS_t_LED_STATES}.
**
** Returnvalue : -
**
*******************************************************************************/
void HALCS_ModStatLedSet(HALCS_t_LED_STATES e_msLedState)
{
  /* Interface function required according to [CS_MAN], (see [1.4-3:]) */
  
  /* HALCS message triggered */ 
  HALCS_t_MSG s_msg;
  
  /* fill in message header and payload */
  s_msg.u16_cmd = HALCS_k_CMD_MOD_STAT_LED_SET;
  s_msg.u16_len = 0U; 
  s_msg.u32_addInfo = (UINT32)e_msLedState;
  s_msg.pb_data = NULL;
 
  /* From CIP Volume 5:
  ** SRS105 The User Safety Manual shall provide a warning that states 'LEDs are NOT reliable
  ** indicators and cannot be guaranteed to provide accurate information. They should ONLY be
  ** used for general diagnostics during commissioning or troubleshooting. Do not attempt to use
  ** LEDs as operational indicators'. 
  **
  ** The LED is not relevant for safety, so it can be ignored if an update can not be 
  ** transferred. 
  */
  
  /* put request into "Request Command Queue" (see [SRS_2126]) */
  /* if queuing of message to CSAL succeeded */
  if (RcqPut(&s_msg))
  {
    /* empty branch */
  }
  /* else: queue full */
  else
  {
    /* insert error into Error Event log (see [SRS_2163]) */
    globErrLog_ErrorAdd(GLOBERRLOG_FILE(1u));
  }
}


/*******************************************************************************
**
** Function    : HALCS_NetStatLedSet
**
** Description : This function is used by the CSS to set the state of the
**               Network Status LED. CIP Safety devices are required to support
**               a Network Status LED. The state of the indicator is under
**               control of the Connection Object state machine. CSS will call
**               this function whenever the status of the Network Status LED
**               changes. It is the responsibility of this HALC function to
**               implement the blink patterns and to ensure that upon power on
**               the LED is initially in the "off" state.
**
** See Also    : HALCS_ModStatLedSet()
**
** Parameters  : e_nsLedState (IN) - state to which the Network status shall be
**                                   set. See type definition of
**                                   {HALCS_t_LED_STATES}.
**
** Returnvalue : -
**
*******************************************************************************/
void HALCS_NetStatLedSet(HALCS_t_LED_STATES e_nsLedState)
{
  /* Interface function required according to [CS_MAN], (see [1.4-3:]) */
  
  /* HALCS message triggered */ 
  HALCS_t_MSG s_msg;
  
  /* fill in message header and payload */
  s_msg.u16_cmd = HALCS_k_CMD_NET_STAT_LED_SET;
  s_msg.u16_len = 0U; 
  s_msg.u32_addInfo = (UINT32)e_nsLedState;
  s_msg.pb_data = NULL;
  
  /* From CIP Volume 5:
  ** SRS105 The User Safety Manual shall provide a warning that states 'LEDs are NOT reliable
  ** indicators and cannot be guaranteed to provide accurate information. They should ONLY be
  ** used for general diagnostics during commissioning or troubleshooting. Do not attempt to use
  ** LEDs as operational indicators'. 
  **
  ** The LED is not relevant for safety, so it can be ignored if an update can not be 
  ** transferred. 
  */
  
  /* put request into "Request Command Queue" (see [SRS_2126]) */
  /* if queuing of message to CSAL succeeded */
  if (RcqPut(&s_msg))
  {
    /* empty branch */
  }
  /* else: queue full */
  else
  {
    /* insert error into Error Event log (see [SRS_2163]) */
    globErrLog_ErrorAdd(GLOBERRLOG_FILE(2u));
  }
}

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
void HALCS_MsgInit(void)
{
  
  /* Attention: Initialization is done during startup-scheduler. No access from IRQ to HALCS is
  ** done here. So disabling IRQ is not necessary here.
  */
  
  /* initialize queue management variables */
  RDS_SET(u16_RcqNumFree, HALCSMSG_cfg_RCQ_SIZE);
  RDS_SET(u16_RcqRdIdx, 0U);
  RDS_SET(u16_RcqWrIdx, 0U);
}


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
void HALCS_MsgTxReqSafetyReset(UINT16 u16_resetCmd, UINT8 u8_attrBitMap)
{
  /* HALCS message triggered */ 
  HALCS_t_MSG s_msg;
  
  /* if invalid reset type/command */
  if ( (u16_resetCmd != HALCS_k_CMD_SAFETY_RESET_TYPE0) &&
       (u16_resetCmd != HALCS_k_CMD_SAFETY_RESET_TYPE1) &&
       (u16_resetCmd != HALCS_k_CMD_SAFETY_RESET_TYPE2) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(3u));
  }
  /* else: valid reset type/command */
  else
  {
    /* fill in message header and payload */
    s_msg.u16_cmd     = u16_resetCmd;
    s_msg.u16_len     = 0U; 
    s_msg.u32_addInfo = u8_attrBitMap;
    s_msg.pb_data     = NULL;
    
    /* put request into "Request Command Queue" */
    /* if queuing of message to CSAL succeeded */
    if (RcqPut(&s_msg))
    {
      /* empty branch */
    }
    /* else: queue full */
    else
    {
      /* insert error into Error Event log (see [SRS_2163]) */
      globErrLog_ErrorAdd(GLOBERRLOG_FILE(4u));
    }
  }
}

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
CONST volatile HALCS_t_AICMSG* HALCS_MsgTxReqGet(void)
{
  /* return value of this function */
  volatile HALCS_t_AICMSG *ps_retVal = NULL;
  
  /* if queue has at least one entry */
  if ( RDS_GET(u16_RcqNumFree) < HALCSMSG_cfg_RCQ_SIZE )
  {
    /* return pointer to current queue read position */
    ps_retVal = &as_Rcq[RDS_GET(u16_RcqRdIdx)];
  }
  /* else: queue is empty */
  else
  {
    /* empty branch */
  }
  return ps_retVal;
}

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
void HALCS_MsgTxReqRelease(void)
{
  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_RcqRdIdx);
  /*lint +esym(960, 10.1)*/
  
  /* if queue has at least one entry */
  if ( RDS_GET(u16_RcqNumFree) < HALCSMSG_cfg_RCQ_SIZE )
  {
    RDS_INC(u16_RcqNumFree);
    RDS_INC(u16_RcqRdIdx);
    /* handle wraparound of read index */
    /* if wraparound */
    if ( RDS_GET_VALUE(u16_RcqRdIdx) == HALCSMSG_cfg_RCQ_SIZE )
    {
      RDS_SET(u16_RcqRdIdx, 0U);
    }
    /* else: no wraparound */
    else
    {
      /* empty branch */
    }
  }
  /* else: queue is empty */
  else
  {
    /* HALCS_MsgTxReqRelease is called although the queue is empty. Since this indicates a
    ** program flow error, the Safety Handler is called. */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(5u));
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    RcqPut

  Description:
    This function puts the passed request message into the request command queue for further 
    processing through the AIC unit.

  See also:
    -

  Parameters:
    ps_msg (IN)     - Pointer to the message structure to be sent, see {HALCS_t_MSG}

  Return value:
    CSS_k_TRUE      - Request is queued successfully
    CSS_k_FALSE     - Unable to queue request (e.g. queue full, too much payload, ...)

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC CSS_t_BOOL RcqPut(const HALCS_t_MSG *ps_msg)
{
  /* return value of this function, indicates if request is queued */
  CSS_t_BOOL b_retVal;

  /* check RDS variable once here, afterwards the variable is accessed directly */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_RcqWrIdx);
  /*lint +esym(960, 10.1)*/
  
  /* Attention: 'Request Command Queue' is accessed from background and scheduler task.
  ** The insertion of a new entry is protected by disabling irqs temporary
  */
  __disable_irq();

  /* if queue is full */
  if ( (RDS_GET(u16_RcqNumFree)) == 0U ) 
  {
    /* Attention: enable IRQ again before function is left */
    __enable_irq();
    /* set return value */
    b_retVal = CSS_k_FALSE;
  }
  /* else: if passed data is too large to be hold in a queue entry */
  else if (ps_msg->u16_len > HALCSMSG_cfg_REQ_PAYLOAD_SIZE)
  {
    /* Attention: enable IRQ again before function is left */
    __enable_irq();
    /* set return value */
    b_retVal = CSS_k_FALSE;
  }
  else
  {
    /* Attention: enable IRQ again */
    __enable_irq();

    /* map HALCS_t_MSG header elements to internal queue structure (HALCS_t_AICMSG) */
    as_Rcq[RDS_GET_VALUE(u16_RcqWrIdx)].u16_cmd = ps_msg->u16_cmd;
    as_Rcq[RDS_GET_VALUE(u16_RcqWrIdx)].u16_len = ps_msg->u16_len;
    as_Rcq[RDS_GET_VALUE(u16_RcqWrIdx)].u32_addInfo = ps_msg->u32_addInfo;

    /* if payload available */
    if (ps_msg->u16_len > 0u)
    {
      /* copy payload */
      stdlibHAL_ByteArrCopy(as_Rcq[RDS_GET_VALUE(u16_RcqWrIdx)].au8_data,
                            ps_msg->pb_data,
                            ps_msg->u16_len);
    }
    /* else: no payload */
    else
    {
      /* empty branch */
    }
    
    /* update queue management variables */
    /* Attention: variable is accessed from background task and IRQ scheduler */
    __disable_irq();
    RDS_DEC(u16_RcqNumFree);
    __enable_irq();
    
    /* increment index */
    RDS_INC(u16_RcqWrIdx);
    /* handle wraparound of write index */
    /* if wraparound */
    if (RDS_GET_VALUE(u16_RcqWrIdx) == HALCSMSG_cfg_RCQ_SIZE)
    {
      RDS_SET(u16_RcqWrIdx, 0U);
    }
    /* else: no wraparound */
    else
    {
      /* empty branch */
    }
    b_retVal = CSS_k_TRUE;
  }
  return b_retVal;
}


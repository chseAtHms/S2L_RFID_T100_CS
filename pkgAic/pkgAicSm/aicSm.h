/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicSm.h
**     Summary: Header file of "aicSm.c"
**   $Revision: 4331 $
**       $Date: 2024-03-13 08:58:56 +0100 (Mi, 13 Mrz 2024) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicSm_SetEvent
**             aicSm_Statemachine
**             aicSm_CfgOwnerToolSet
**             aicSm_CfgOwnerOriginatorGetSet
**             aicSm_CfgIsOriginatorOwned
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AIC_SM_H
#define AIC_SM_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/*!<  This enum specifies the possible states of AIC */
typedef enum
{
   AICSM_AIC_STARTUP                   = 0x59a6u,   /*!< Startup message handling */
   AICSM_AIC_WAIT_FOR_INIT             = 0x5a88u,   /*!< Waiting for 'SetInitData' request */
   AICSM_AIC_START_CSS                 = 0x5b1fu,   /*!< Starting CSS software */
   AICSM_AIC_WAIT_TUNID                = 0x5cd4u,   /*!< Waiting for TUNID */
   AICSM_AIC_WAIT_FOR_CONFIG           = 0x5d43u,   /*!< Waiting to receive the configuration */
   AICSM_AIC_PROCESS_CONFIG            = 0x5e6du,   /*!< Processing the received configuration */
   AICSM_AIC_INVALID_CONFIG            = 0x5ffau,   /*!< Invalid configuration was received */
   AICSM_AIC_IDLE                      = 0x6035u,   /*!< Idle state */
   AICSM_AIC_EXEC_PROD_ONLY            = 0x61a2u,   /*!< Executing state, Producer Only */
   AICSM_AIC_EXEC_CONS_ONLY            = 0x628cu,   /*!< Executing state, Consumer Only */
   AICSM_AIC_EXEC_PROD_CONS            = 0x631bu,   /*!< Executing state, Consumer and Producer */
   AICSM_AIC_WAIT_RESET                = 0x64d0u,   /*!< Waiting for HW reset */
   AICSM_AIC_ABORT                     = 0x6547u    /*!< Abort */
} AICSM_STATE_ENUM;


/*!< Global events triggered by IRQ scheduler or background task */
typedef enum
{
   /*!< No event */
   AICSM_k_EVT_NO_EVENT                = 0xc628u,   
  /*!< CSS initialization data received (T4) */
   AICSM_k_EVT_CSS_INIT_RECEIVED       = 0xc7bfu,   
  /*!< CSS initialized, waiting for TUNID (T5) */
   AICSM_k_EVT_CSS_INIT_WAIT_TUNID     = 0xc829u,   
  /*!< CSS initialized, waiting for config (T6) */
   AICSM_k_EVT_CSS_INIT_WAIT_CONFIG    = 0xc9beu,   
   /*!< CSS initialized (T7) */
   AICSM_k_EVT_CSS_INIT_IDLE           = 0xca90u,   
   /*!< CSS initialized, invalid NodeId (T20) */
   AICSM_k_EVT_CSS_INIT_INVLD_NODEID   = 0xcb07u,   
   /*!< TUNID set (T8) */
   AICSM_k_EVT_TUNID_RECEIVED          = 0xccccu,   
   /*!< IO Configuration Data and SCID received (T17, T18, T22) */
   AICSM_k_EVT_CONFIG_RECEIVED         = 0xcd5bu,   
   /*!< Invalid IO Configuration Data received (T21) */
   AICSM_k_EVT_INVLD_CONFIG            = 0xce75u,   
   /*!< IO configuration done (T19) */
   AICSM_k_EVT_CONFIG_DONE             = 0xcfe2u,   
   /*!< Producer Connection established (T9, T15) */
   AICSM_k_EVT_PROD_CONN_ESTABLISHED   = 0xd0ceu,   
   /*!< Producer Connection closed (T10, T16, T26, T28, T29, T30, T31, T34) */
   AICSM_k_EVT_PROD_CONN_CLOSED        = 0xd159u,   
   /*!< Producer Connection faulted (T10, T16, T34) */
   AICSM_k_EVT_PROD_CONN_FAULTED       = 0xd277u,   
   /*!< Consumer Connection established (T11, T13) */
   AICSM_k_EVT_CONS_CONN_ESTABLISHED   = 0xd3e0u,   
   /*!< Consumer Connection closed (T12, T14, T26, T27, T29, T30, T31, T34) */
   AICSM_k_EVT_CONS_CONN_CLOSED        = 0xd42bu,   
   /*!< Consumer Connection faulted (T12, T14, T34) */
   AICSM_k_EVT_CONS_CONN_FAULTED       = 0xd5bcu,   
   /*!< Safety Reset received (T23, T24) */
   AICSM_k_EVT_SAFETY_RESET            = 0xd692u,   
   /*!< Abort request received from ABCC (T32, T33) */
   AICSM_k_EVT_ABORT_REQUEST           = 0xd705u
} AICSM_t_EVENT;


/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicSm_eAicState:
** status of AIC, global used due to performance reasons
*/
extern volatile AICSM_STATE_ENUM aicSm_eAicState;

/* aicSm_eFirstMsgReceived:
** This variable indicates if the first message is received correct.
*/
extern TRUE_FALSE_ENUM aicSm_eFirstMsgReceived;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicSm_SetEvent

  Description:
    This functions is used to set a new event which shall be processed by the global state machine.
    The events are stored in a queue and processed during IRQ Scheduler. If the number of events 
    exceeds the limit 'cfg_MAX_NUM_EVENTS', a fail safe error is triggered.

  See also:
    -

  Parameters:
    e_event (IN)   - Event triggered by IRQ scheduler or Background Task.
                     (valid range: see AICSM_e_EVENT, not checked here, checked if event is
                     processed via calling aicSm_Statemachine->HandleEvent)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: Background Task

***************************************************************************************************/
void aicSm_SetEvent(AICSM_t_EVENT e_event);

/***************************************************************************************************
  Function:
    aicSm_Statemachine

  Description:
    This function contains the statemachine for the AIC handling.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicSm_Statemachine (void);

/***************************************************************************************************
  Function:
    aicSm_CfgOwnerToolSet

  Description:
    This function sets the configuration owner to the passed value.

  See also:
    -

  Parameters:
    b_isToolOwned (IN)  - TRUE:  Configuration owner is the configuration tool/host
                          FALSE: Configuration owner is Originator or there is not yet a
                                 configuration owner (un-owned)
                                 (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler (only in AIC state AICSM_AIC_IDLE, AICSM_AIC_WAIT_FOR_CONFIG
                            or AICSM_AIC_INVALID_CONFIG)
    Context: Background Task (only in AIC state AICSM_AIC_START_CSS)
    => concurrent access to this function is not possible (ruled out by AIC state machine)

***************************************************************************************************/
void aicSm_CfgOwnerToolSet(BOOL b_isToolOwned);

/***************************************************************************************************
  Function:
    aicSm_CfgOwnerOriginatorGetSet

  Description:
    This function sets the configuration owner to the passed value if possible. It is only possible
    to set the configuration owner if the device is not configured by a configuration tool/host.

  See also:
    -

  Parameters:
    b_isOriginatorOwned (IN)  - TRUE:  Configuration owner is an Originator
                                FALSE: Configuration owner is a configuration tool/host or there is
                                       not yet a configuration owner (un-owned)
                                (valid range: any, not checked)

  Return value:
    TRUE  - it was possible to set the configuration owner to the passed value
    FALSE - it was not possible to set the configuration owner to the passed value

  Remarks:
    Context: Background Task

***************************************************************************************************/
BOOL aicSm_CfgOwnerOriginatorGetSet(BOOL b_isOriginatorOwned);

/***************************************************************************************************
  Function:
    aicSm_CfgIsOriginatorOwned

  Description:
    This function checks if the device's configuration owner is an Originator (that has sent the
    configuration via Type 1 SafetyOpen). If not then the device is either configured by a
    configuration tool (means the device was configured from the host via AIC) or is not configured
    yet (un-owned).

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE  - Configuration owner is an Originator
    FALSE - Configuration owner is a configuration tool/host or the there is not yet a configuration
            owner (un-owned)

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicSm_CfgIsOriginatorOwned(void);

#endif


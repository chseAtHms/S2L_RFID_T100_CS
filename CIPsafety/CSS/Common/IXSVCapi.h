/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCapi.h
**    Summary: IXSVC - Safety Validator Client
**             Interface definition of the IXSVC unit of the CSS.
**             This unit implements the interface to the Safety Validator
**             Client.
**             IXSVCapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_InstTxFrameGenerate
**             SAPL_IxsvcEventHandlerClbk
**             IXSVC_InstInfoGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVC_API_H
#define IXSVC_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSVC_t_VALIDATOR_EVENT:
    Safety Validator Event values.
*/
typedef enum
{
/* We are defining constants here. Thus deliberately assigning numbers here */
  IXSVC_k_VE_CLIENT_CLOSE,     /* SV Client Inst has been closed */
  IXSVC_k_VE_CLIENT_OPEN,      /* SV CLient Inst has been opened */
  IXSVC_k_VE_CONSUMER_JOIN,    /* another Consumer has joined this client */
  IXSVC_k_VE_CONSUMER_LEAVE,   /* a Consumer has closed the connection */
  IXSVC_k_VE_CONSUMER_FAULTED, /* one of the Consumers faulted */
  IXSVC_k_VE_ALL_CONS_FAULTED, /* all Consumers failed or timed out */
  IXSVC_k_VE_1ST_HS_COMPLETE,  /* connection has received first TCOO msg */
  IXSVC_k_VE_CONSUMER_ACTIVE   /* Consumer now active */
} IXSVC_t_VALIDATOR_EVENT;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_InstTxFrameGenerate
**
** Description : This API function handles one Safety Validator Client Instance.
**               This function must be called cyclically to update timing
**               information and to initiate generation of Transmit Safety
**               Frames.
**
**               **Attention**:
**               This function must be called at least once per producing EPI.
**
**               **Attention**:
**               It is recommended that the calling interval of this function
**               is the connection's EPI or a multiple of it to avoid a jitter
**               in the produced messages.
**
**               **Attention**:
**               For reasons of convenience of the API this function will not
**               return an error when called with an invalid Instance ID.
**               Instead it will return without processing anything. This way
**               the user can call this function in a loop running over all
**               Instance IDs without having to distinguish between servers and
**               clients.
**
** Parameters  : u16_svcInstId (IN) - Instance Id of the Safety Validator Client
**                                    (checked, valid range:
**                                    1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                    CSOS_cfg_NUM_OF_SV_SERVERS) and
**                                    instance must be a Safety Validator
**                                    Client.
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  void IXSVC_InstTxFrameGenerate(CSS_t_UINT u16_svcInstId);
#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*
** Functions to be provided by the Safety Application
*/

/*******************************************************************************
**
** Function    : SAPL_IxsvcEventHandlerClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case the state of
**               a producing Safety Connection changes. With this function the
**               application can initiate application specific reaction to
**               producing Safety Connection events. Such events can be:
**               - opening/closing a connection
**               - consumer joined/leaved Multicast connection
**               - consumer of safety data failed
**               - consumer active
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxsvsEventHandlerClbk()
**
** Parameters  : u16_instId (IN) - instance ID of the associated Safety
**                                 Validator object
**               e_event (IN)    - Unique code describing the kind of event. See
**                                 type definition of {IXSVC_t_VALIDATOR_EVENT}.
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  void SAPL_IxsvcEventHandlerClbk(CSS_t_UINT u16_instId,
                                  IXSVC_t_VALIDATOR_EVENT e_event);
#endif


/*******************************************************************************
**
** Function    : IXSVC_InstInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Client Object Instance (e.g. state, status
**               type, etc.). See type definition of {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range:
**                                   1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS) and instance
**                                   must be a Safety Validator Client.
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  void IXSVC_InstInfoGet(CSS_t_UINT u16_instId,
                         CSS_t_VALIDATOR_INFOS *ps_svoInfos);
#endif


#endif /* IXSVC_API_H */

/*** End of File ***/


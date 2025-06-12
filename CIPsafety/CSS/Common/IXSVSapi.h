/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSapi.h
**    Summary: IXSVS - Safety Validator Server
**             Interface definition of the IXSVS unit of the CSS.
**             This unit implements the interface to the Safety Validator
**             Server.
**             IXSVSapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_ConsInstActivityMonitor
**             SAPL_IxsvsEventHandlerClbk
**             IXSVS_InstInfoGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVS_API_H
#define IXSVS_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSVS_t_VALIDATOR_EVENT:
    Safety Validator Event values.
*/
typedef enum
{
/* We are defining constants here. Thus deliberately assigning numbers here */
  IXSVS_k_VE_SERVER_CLOSE,     /* SV Server Inst has been closed */
  IXSVS_k_VE_SERVER_OPEN,      /* SV Server Inst has been opened */
  IXSVS_k_VE_1ST_DATA_REC,     /* SV Server Inst has received first data */
  IXSVS_k_VE_SERVER_FAULTED,   /* SV Server Inst failed - restart required */
  IXSVS_k_VE_SERVER_IDLE,      /* SV Server Inst is now in Idle Mode */
  IXSVS_k_VE_SERVER_RUN,       /* SV Server Inst is now in Run Mode */
  IXSVS_k_VE_SERVER_ACTIVE     /* SV Server Inst completed Time Stamp init */
} IXSVS_t_VALIDATOR_EVENT;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_ConsInstActivityMonitor
**
** Description : This API function handles the Consumer Activity Monitor of one
**               Safety Validator Server Instance. Additionally it transmits
**               the Time Coordination Messages (if a transmission is currently
**               pending).
**               This function can be called with any Safety Validator Instance
**               ID (see Demo). In case the passed instance is a Client Instance
**               or doesn't exist it will just return (and not generate an
**               error).
**
**               **Attention**:
**               This function checks if a packet has been received within
**               Network_Time_Expectation.
**               This function must be called cyclically to perform Consumer
**               Activity Monitoring. The calling interval must be fast enough
**               to meet the System Reaction Time. Worst case timeout detection
**               occurs one calling interval later.
**
** Parameters  : u16_svsInstId (IN) - Instance ID of the Safety Validator Server
**                                    (checked, valid range:
**                                    1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                    CSOS_cfg_NUM_OF_SV_SERVERS) and
**                                    instance must be a Safety Validator
**                                    Server.
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  void IXSVS_ConsInstActivityMonitor(CSS_t_UINT u16_svsInstId);
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*
** Functions to be provided by the Safety Application
*/

/*******************************************************************************
**
** Function    : SAPL_IxsvsEventHandlerClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case the state of
**               a consuming Safety Connection changes. With this function the
**               application can initiate application specific reaction to
**               consuming Safety Connection events. Such events can be:
**               - opening/closing a connection
**               - consumer of safety data failed
**               - Run/Idle Flag changed
**
**               **Attention**:
**               For the following events the application has to make sure the
**               output is set to the safe state:
**               - IXSVS_k_VE_SERVER_CLOSE
**               - IXSVS_k_VE_SERVER_FAULTED (see FRS12-3)
**               - IXSVS_k_VE_SERVER_IDLE (see FRS276)
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxsvcEventHandlerClbk()
**
** Parameters  : u16_instId (IN) - instance ID of the associated Safety
**                                 Validator object
**               e_event (IN)    - Unique code describing the kind of event. See
**                                 type definition of {IXSVS_t_VALIDATOR_EVENT}.
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  void SAPL_IxsvsEventHandlerClbk(CSS_t_UINT u16_instId,
                                  IXSVS_t_VALIDATOR_EVENT e_event);
#endif


/*******************************************************************************
**
** Function    : IXSVS_InstInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Server Object Instance (e.g. state, status,
**               type, etc.). See type definition of {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range:
**                                   1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS) and instance
**                                   must be a Safety Validator Server.
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  void IXSVS_InstInfoGet(CSS_t_UINT u16_instId,
                         CSS_t_VALIDATOR_INFOS *ps_svoInfos);
#endif


#endif /* IXSVS_API_H */

/*** End of File ***/


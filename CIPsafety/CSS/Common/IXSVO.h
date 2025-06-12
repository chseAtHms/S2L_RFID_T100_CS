/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVO.h
**    Summary: IXSVO - Safety Validator Object
**             IXSVO.h is the export header file of the IXSVO unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_Init
**             IXSVO_TargAlloc
**             IXSVO_TargClear
**             IXSVO_TargInit
**             IXSVO_CnxnFind
**             IXSVO_OrigAlloc
**             IXSVO_OrigPrepare
**             IXSVO_OrigInit
**             IXSVO_InstanceClose
**             IXSVO_InstanceStop
**             IXSVO_ExplMsgHandler
**             IXSSO_IxsvoCnxnStateTransClbk
**             IXSVO_CnxnDropAll
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVO_H
#define IXSVO_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/*
** Transport Class Triggers:
** For Safety only the following 2 Transport Class Triggers are used
*/

/* Direction: Client
** Production Trigger: Application
** Transport Class: Class 0
*/
/*lint -esym(755, IXSVO_k_TCT_CLIENT_APP_CL0)     not referenced in every cfg */
#define IXSVO_k_TCT_CLIENT_APP_CL0     0x20U

/* Direction: Server
** Production Trigger: Application
** Transport Class: Class 0
*/
/*lint -esym(755, IXSVO_k_TCT_SERVER_APP_CL0)     not referenced in every cfg */
#define IXSVO_k_TCT_SERVER_APP_CL0     0xA0U


/** IXSVO_k_CNXN_EVENT_xxx:
    These events can occur within a Safety Connection.
*/
#define IXSVO_k_CNXN_EVENT_ESTAB  1U /* connection established */
#define IXSVO_k_CNXN_EVENT_DEL    2U /* connection deleted */
#define IXSVO_k_CNXN_EVENT_FAIL   3U /* connection failed */


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVO_Init
**
** Description : This function initializes the IXSVO unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_Init(void);


/*******************************************************************************
**
** Function    : IXSVO_TargAlloc
**
** Description : This function allocates for a Target a new Safety Validator
**               (Client or Server) if necessary. A consuming Originator could
**               connect to an already existing Multi-Cast connection. In this
**               case this function returns the Instance of the existing
**               connection.
**
** Parameters  : ps_sOpenPar (IN)       - pointer to parameters received with
**                                        the Forward_Open request
**                                        (not checked, only called with
**                                        reference to structure variable)
**               pu16_svInst (OUT)      - pointer to the variable to where the
**                                        returned instance is written to (in
**                                        case of success, otherwise
**                                        CSOS_k_INVALID_INSTANCE)
**                                        (not checked, only called with
**                                        reference to variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_OK               - success
**               <>CSS_k_OK             - error
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_TargAlloc(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                           CSS_t_UINT *pu16_svInst,
                           CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
#endif


/*******************************************************************************
**
** Function    : IXSVO_TargClear
**
** Description : This function reinitializes a Safety Validator (Client or
**               Server).
**
** Parameters  : ps_sOpenPar (IN) - pointer to parameters received with
**                                  the Forward_Open request
**                                  (not checked, only called with reference
**                                  to structure variable)
**               ps_svInfo (IN)   - this struct contains Instance ID and
**                                  Consumer Number of the Safety Validator
**                                  Instance
**                                  (not checked, only called with reference
**                                  to structure variable)
**                                  - u16_instId (not checked, checked in
**                                    SafetyOpenProc3())
**                                  - u8_consNum (not checked, checked in
**                                    IXSVS_InstFind() and IXSVC_InstFind())
**                                  - u8_state (not checked, checked in
**                                    IXSVS_InstFind() and IXSVC_InstFind())
**
** Returnvalue : CSS_k_OK         - success
**               <>CSS_k_OK       - error
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_TargClear(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                           const CSS_t_SV_ID *ps_svInfo);
#endif


/*******************************************************************************
**
** Function    : IXSVO_TargInit
**
** Description : This function initializes an instance of the Safety
**               Validator Object (Client or Server) for a Connection in that we
**               are the Target.
**
** Parameters  : ps_sOpenPar (IN)    - pointer to parameters received with
**                                     the Forward_Open request
**                                     (not checked, only called with
**                                     reference to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as
**                                     the payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               u16_svInst (IN)     - Safety Validator instance ID that is
**                                     already allocated for this connection
**                                     (checked, valid range: 1..
**                                     (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                     CSOS_cfg_NUM_OF_SV_SERVERS))
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error while initializing the Safety
**                                     Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_TargInit(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                          CSS_t_USINT u8_payloadSize,
                          CSS_t_UINT u16_svInst,
                          CSS_t_SOPEN_RESP *ps_sOpenResp);
#endif


/*******************************************************************************
**
** Function    : IXSVO_CnxnFind
**
** Description : This function searches the Safety Validator Instance which
**               matches the passed Connection Triad.
**
** Parameters  : ps_cnxnTriad (IN) - pointer to the structure which contains
**                                   the description of the connection
**                                   (not checked, only called with
**                                   reference to structure variable)
**               ps_svInfo (OUT)   - pointer to the structure to where the found
**                                   Safety Validator Instance Type, Index, and
**                                   Consumer_Number will be written
**                                   (not checked, only called with
**                                   reference to structure variable)
**
** Returnvalue : CSS_k_TRUE        - connection was found (index in ps_cnxnIdx)
**               CSS_k_FALSE       - connection not found
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_BOOL IXSVO_CnxnFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo);
#endif


/*******************************************************************************
**
** Function    : IXSVO_OrigAlloc
**
** Description : This function allocates for an Originator a new Safety
**               Validator (Client or Server).
**
** Parameters  : b_cnxnType (IN) - determines the type of the connection
**                                 (checked, valid range: see {CSOS_k_CNXN_xxx})
**               u16_svInst (IN) - Safety Validator Instance ID (on the
**                                 originator the Safety Validator Instance ID
**                                 is identical to the CCO Instance ID which is
**                                 responsible for opening this connection)
**                                 (not checked, checked in IXCCO_CnxnOpen())
**
** Returnvalue : CSS_k_OK                     - success
**               IXSVO_k_FSE_INV_CNXN_TYPE_A - allocation not successful
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_OrigAlloc(CSS_t_BYTE b_cnxnType,
                           CSS_t_UINT u16_svInst);
#endif


/*******************************************************************************
**
** Function    : IXSVO_OrigPrepare
**
** Description : This function is called to prepare a just allocated Safety
**               Validator Server instance before the SafetyOpen is generated
**               by calling its Cold Start Logic.
**
** Parameters  : u16_svInst (IN)  - Safety Validator instance ID that is already
**                                  allocated for this connection
**                                  (not checked: checked in IXCCO_CnxnOpen())
**               ps_initVal (OUT) - in case of Extended Format this structure
**                                  returns the Initial Time Stamp and Initial
**                                  Rollover Value for the SafetyOpen request
**                                  (not checked, only called with reference
**                                  to structure variable)
**
** Returnvalue : CSS_k_OK         - success
**               <>CSS_k_OK       - error while preparing the Safety Validator
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
    CSS_t_WORD IXSVO_OrigPrepare(CSS_t_UINT u16_svInst,
                                 CSS_t_INIT_VAL *ps_initVal);
  #endif
#endif

/*******************************************************************************
**
** Function    : IXSVO_OrigInit
**
** Description : This function initializes an instance of the Safety
**               Validator Object (Client or Server) for a Connection in that we
**               are the Originator.
**
** Parameters  : ps_openRespRx (IN)      - pointer to parameters that were
**                                         received with the Forward_Open
**                                         response from the Target
**                                         (not checked, only called with
**                                         reference to structure variable)
**               ps_ccoSopenInitVal (IN) - pointer to parameters coming from the
**                                         CCO instance data
**                                         (not checked, only called with
**                                         reference to structure variable)
**               u16_svInst (IN)         - Safety Validator instance ID that is
**                                         already allocated for this connection
**                                         (checked, valid range: 1..
**                                         (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                         CSOS_cfg_NUM_OF_SV_SERVERS))
**
** Returnvalue : CSS_k_OK                - success
**               <>CSS_k_OK              - error while initializing the Safety
**                                         Validator Instance
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_OrigInit(const CSS_t_SOPEN_RESP_RX *ps_openRespRx,
                          const CSS_t_CCO_SOPEN_INIT_VAL *ps_ccoSopenInitVal,
                          CSS_t_UINT u16_svInst);
#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVO_InstanceClose
**
** Description : This function closes the passed Safety Validator Server or
**               Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, valid range: 1..
**                                 (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                 CSOS_cfg_NUM_OF_SV_SERVERS))
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, only called with constants)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
CSS_t_WORD IXSVO_InstanceClose(CSS_t_UINT u16_instId,
                               CSS_t_USINT u8_consNum);
#endif


/*******************************************************************************
**
** Function    : IXSVO_InstanceStop
**
** Description : This function stops/deletes the passed Safety Validator Server
**               or Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, valid range: 1..
**                                 (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                 CSOS_cfg_NUM_OF_SV_SERVERS))
**               u8_consNum (IN) - Consumer Number
**                                 (checked, valid range: 1..
**                                 CSOS_k_MAX_CONSUMER_NUM_MCAST)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_InstanceStop(CSS_t_UINT u16_instId,
                        CSS_t_USINT u8_consNum);


/*******************************************************************************
**
** Function    : IXSVO_ExplMsgHandler
**
** Description : This function handles all explicit requests addressed to the
**               Safety Validator Object. It checks if a class or Instance
**               Service is requested and calls the corresponding function to
**               serve the request.
**
** Parameters  : ps_explReq (IN)   - pointer to request message structure
**                                   (not checked, only called with reference)
**               ps_explResp (OUT) - pointer to structure describing the
**                                   response message that is to be returned
**                                   (not checked, only called with reference)
**               pb_rspData (OUT)  - pointer to the response message data that
**                                   is to be returned
**                                   (not checked, checked in ExplMsgProcess(),
**                                   valid range: <> CSS_k_NULL)
**
** Returnvalue : CSS_k_OK          - success
**               <>CSS_k_OK        - error while processing the message
**
*******************************************************************************/
CSS_t_WORD IXSVO_ExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE * const pb_rspData);


/*******************************************************************************
**
** Function    : IXSSO_IxsvoCnxnStateTransClbk
**
** Description : This callback function is called by the Safety Validator Object
**               when a connection is established, deleted or fails.
**
** Parameters  : u8_event (IN)          - Safety Connection event
**                                        (checked, allowed value see definition
**                                        of {IXSVO_k_CNXN_EVENT_xxx})
**               u16_numEstabCnxns (IN) - number of connections that are
**                                        currently in the established state
**                                        (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK   - success
**               <>CSS_k_OK - error
**
*******************************************************************************/
CSS_t_WORD IXSSO_IxsvoCnxnStateTransClbk(CSS_t_USINT u8_event,
                                         CSS_t_UINT u16_numEstabCnxns);


/*******************************************************************************
**
** Function    : IXSVO_CnxnDropAll
**
** Description : This function drops all connections at once. Needed e.g. by
**               the Safety Supervisor Object when a Configure Request service
**               is received.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_CnxnDropAll(void);


#endif /* #ifndef IXSVO_H */

/*** End Of File ***/


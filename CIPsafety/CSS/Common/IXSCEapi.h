/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEapi.h
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             Interface definition of the IXSCE unit of the CSS.
**             This unit implements the Safety Validator Connection
**             Establishment Engine as described in the CIP Safety
**             specification. Its purpose is to give the application the
**             possibility to check the connection parameters of a SafetyOpen
**             service request. Also the application shall check and accept
**             configuration data transmitted with a type 1 SafetyOpen.
**             IXSCEapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_CmdProcess
**             SAPL_IxsceCfgApplyClbk
**             SAPL_IxsceSafetyOpenValidateClbk
**             SAPL_IxsceCompElectronicKeyClbk
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCE_API_H
#define IXSCE_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/


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
** Function    : IXSCE_CmdProcess
**
** Description : This API function processes the commands received via HALC
**               which address the IXSCE unit.
**
**               **Attention**:
**               This function processes received Forward_Open and Forward_Close
**               requests and produces the responses. After reception of an
**               IXSCE command this function must be called fast enough not to
**               run into timeouts of these services on the originator.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure (checked):
**                                   valid range: <> CSS_k_NULL
**                                   - u16_cmd (checked)
**                                     valid range: CSOS_k_CMD_IXCMO_SOPEN_REQ,
**                                     CSOS_k_CMD_IXCMO_SCLOSE_REQ
**                                   - u16_len (not checked, checked in
**                                     FwdOpenHandler() and FwdCloseHandler()
**                                     valid range: 0 .. 65535)
**                                   - u32_addInfo
**                                     (not checked, any value allowed)
**                                   - pb_data (checked,
**                                     valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  void IXSCE_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg);
#endif


/*
** Functions to be provided by the Safety Application
*/

/*******************************************************************************
**
** Function    : SAPL_IxsceCfgApplyClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function during processing of
**               a Type 1 SafetyOpen. It is the job of this function
**               to save pending application configuration data to NV memory.
**
** Context       IXSCE_CmdProcess()
**
** Parameters  : -
**
** Returnvalue : CSS_k_TRUE       - Application successfully saved Config data
**                                  to NV memory
**               CSS_k_FALSE      - Applying Configuration data failed:
**                                  SafetyOpen will be rejected, Safety
**                                  Supervisor state machine will stay in
**                                  "Configuring" state.
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  CSS_t_BOOL SAPL_IxsceCfgApplyClbk(void);
#endif


/*******************************************************************************
**
** Function    : SAPL_IxsceSafetyOpenValidateClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function in case a SafetyOpen
**               service has been received and was successfully checked. By
**               applying further application specific criteria it is now up to
**               the application to decide if it wants to accept the safety
**               connection. This decision may be based on the passed parameter
**               structure and the state of the application (e.g. device is
**               configured) (see Req.5.3.1-5).
**               The application must also copy the configuration data (of a
**               Type 1 SafetyOpen) and validate it with SIL 3 integrity (see
**               SRS37), but not yet save it to NV-memory.
**
**               **Attention**:
**               CSS doesn't check the length of the safe I/O data in the
**               SafetyOpen. It is the duty of the safe application to check if
**               the requested size (u8_payloadSize) matches with the addressed
**               assembly instance (which was defined in the call of
**               IXSAI_AsmIoInstSetup()). For this comparison it must be
**               considered:
**               - The payload length of the connection may be equal or less
**                 than the assembly length.
**                 - If a connection consumes less than the length of the output
**                   assembly then the rest is filled up with zeros. If the
**                   connection consumes more than the assembly length this will
**                   lead to a fail safe error on first data consumption.
**                 - A connection may produce less than the length of the input
**                   assembly. If the connection is to produce more than the
**                   assembly length this will lead to a fail safe error on
**                   first data production.
**
**               **Attention**:
**               CSS doesn't check the Configuration Assembly Instance ID. It is
**               the duty of the safe application to restrict the
**               ps_safetyOpenParams->u16_cfgInst to valid Configuration
**               Assembly Instances.
**
** Context     : IXSCE_CmdProcess()
**
** See Also    : CSS_t_SOPEN_PARA
**
** Parameters  : ps_safetyOpenParams (IN) - pointer to structure containing the
**                                          parameters received with the
**                                          SafetyOpen service, possibly with
**                                          safety configuration data
**               u8_payloadSize (IN)      - number of bytes of safe payload data
**                                          to be transported by the safety
**                                          connection (T->O for Safety
**                                          Validator Clients, O->T for Safety
**                                          Validator Servers)
**
** Returnvalue : CSS_k_OK                 - Application grants the permission to
**                                          accept the connection request.
**               <>CSS_k_OK               - Application denies the connection
**                                          request. Returned value will be
**                                          responded in the Extended Status of
**                                          the Unsuccessful Forward_Open
**                                          Response.
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD SAPL_IxsceSafetyOpenValidateClbk(
                       const CSS_t_SOPEN_PARA *ps_safetyOpenParams,
                       CSS_t_USINT u8_payloadSize);
#endif


/*******************************************************************************
**
** Function    : SAPL_IxsceCompElectronicKeyClbk
**
** Description : This callback function is called when an explicit Message
**               request (e.g. SafetyOpen) has been received with an electronic
**               key that does not match our device but with the Compatibility
**               Bit set. A compatibility set to 1 allows the device to accept
**               any key which the device can emulate. The default behavior is
**               to reject the connection.
**
** Context     : IXSCE_CmdProcess()
**
** See Also    : -
**
** Parameters  : ps_elKey (IN) - pointer to structure containing the received
**                               electronic key
**
** Returnvalue : CSS_k_TRUE    - Key matches, accept the connection
**               CSS_k_FALSE   - Key mismatch, reject the connection
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxsceCompElectronicKeyClbk(const CSS_t_ELECTRONIC_KEY
                                           *ps_elKey);


#endif /* IXSCE_API_H */

/*** End of File ***/


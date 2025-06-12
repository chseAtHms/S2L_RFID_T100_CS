/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSMRapi.h
**    Summary: IXSMR - Safety Message Router Object
**             Interface definition of the IXSMR unit of the CSS.
**             This unit implements the safety related parts of the Message
**             Router Object. It obtains the service requests from the CSAL
**             and distributes them to the safety objects respectively to the
**             application.
**             IXSMRapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSMR_CmdProcess
**             SAPL_IxsmrObjReqClbk
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSMR_API_H
#define IXSMR_API_H


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
** Function    : IXSMR_CmdProcess
**
** Description : This API function processes the commands received via HALC
**               which address the IXSMR unit.
**
**               **Attention**:
**               This function processes received explicit request messages and
**               produces the responses. After reception of an IXSMR command
**               this function must be called fast enough not to run into
**               explicit message timeouts on the requesting client.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure (checked):
**                                   valid range: <> CSS_k_NULL
**                                   - u16_cmd (checked)
**                                     valid range: CSOS_k_CMD_IXMRO_EXPL_REQ
**                                   - u16_len (not checked, checked in
**                                     ExplMsgProcess(),
**                                     valid range: k_MIN_EXPL_REQ_LEN .. 65535)
**                                   - u32_addInfo
**                                     (not checked, any value allowed)
**                                   - pb_data (not checked, checked in
**                                     ExplMsgProcess())
**                                   **Attention**:
**                                   By processing an explicit request a
**                                   response is  generated and passed to
**                                   HALCS_TxDataPut(). If sending via
**                                   HALCS_TxDataPut() is not possible an info
**                                   error (IXSMR_k_NFSE_TXH_RSP_SEND_ERR) is
**                                   generated and the application must react
**                                   appropriately.
**
** Returnvalue : -
**
*******************************************************************************/
void IXSMR_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg);


/*
** Functions to be provided by the Safety Application
*/

/*******************************************************************************
**
** Function    : SAPL_IxsmrObjReqClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when a service request
**               to an application object was received (see Asm.5.3.1-1 and
**               Req.5.3.1-1). The application now needs to evaluate the
**               received request and provide an appropriate response.
**
**               **Attention**: If the request is a configuration message (e.g.
**               Set_Attribute_Single to an attribute that is relevant for the
**               safety device configuration) then the application object shall
**               reject the request if the device is in an operational mode
**               (e.g. has active connection or is in executing mode)
**               (see FRS116-3).
**
** Context     : IXSMR_CmdProcess()
**
** See Also    : -
**
** Parameters  : ps_request (IN)   - Structure containing the information that
**                                   was received with the explicit request.
**               ps_response (OUT) - The result of the service must be placed
**                                   into this structure.
**               pb_rspData (OUT)  - pointer to a buffer where this function
**                                   must put the response data into.
**                                   **Attention**:
**                                   This function may only write up to
**                                   CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
**                                   bytes into pb_rspData.
**                                   **Attention**:
**                                   On exit of this callback function
**                                   ps_response->u16_rspDataLen must be set
**                                   to the number of bytes of the Response_Data
**                                   or can be left unchanged in case the length
**                                   of the Response_Data shall be zero.
**
** Returnvalue : CSS_k_TRUE        - Application was able to service the request
**               CSS_k_FALSE       - Application could not service the request
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxsmrObjReqClbk(const CSS_t_EXPL_REQ *ps_request,
                                CSS_t_EXPL_RSP *ps_response,
                                CSS_t_BYTE *pb_rspData);


#endif /* IXSMR_API_H */

/*** End of File ***/


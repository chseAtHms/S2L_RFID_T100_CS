/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOapi.h
**    Summary: IXSVO - Safety Validator Object
**             Interface definition of the IXSVO unit of the CSS.
**             This unit implements the interface to the Safety Validator
**             Object (see Req.5.3-2).
**             IXSVOapi.h must be included by a safety application in order to
**             use the CIP Safety Stack.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_CmdProcess
**             IXSVO_RxDataCrossCheckError
**             IXSVO_InstanceInfoGet
**             IXSVO_FaultCountersReset
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVO_API_H
#define IXSVO_API_H


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

/*
**  Functions provided by the IXSVO Unit
*/

/*******************************************************************************
**
** Function    : IXSVO_CmdProcess
**
** Description : This API function processes the commands received via HALC
**               which address the IXSVO unit.
**
**               **Attention**:
**               This function processes the received safety IO messages.
**               After reception of an IXSVO command this function must be
**               called fast enough not to run into a consumer activity
**               monitoring timeout (see {IXSVS_ConsInstActivityMonitor()}).
**
**               **Attention**:
**               When the safety application processes the received safe data
**               based on the returned data pointer it is mandatory to evaluate
**               the callback function SAPL_IxsvsEventHandlerClbk() to determine
**               when the output data is no longer valid.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to the received HALC message
**                                   structure (checked):
**                                   valid range: <> CSS_k_NULL
**                                   - u16_cmd (checked)
**                                     valid range: CSOS_k_CMD_IXCO_IO_DATA
**                                   - u16_len (not checked, checked in
**                                     ValidatorRxProc(), valid ranges:
**                                     CSOS_k_CMD_IXCO_IO_DATA:
**                                       (2+CSOS_k_IO_MSGLEN_TCOO)
**                                       ..(2+CSOS_k_IO_MSGLEN_LONG_MAX))
**                                   - u32_addInfo
**                                     (not checked, any value allowed)
**                                   - pb_data (checked,
**                                     valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg);


/*******************************************************************************
**
** Function    : IXSVO_RxDataCrossCheckError
**
** Description : This API function must be called by the application to indicate
**               a cross checking error of received data (see FRS13 and
**               Req.5.3.1-4):
**
**               **Attention:**
**               Cross checking between Safety Controller 1 and Safety
**               Controller 2 is not done by the Safety Validator. Instead it
**               has to be done by the safety application. After reception of
**               data the safety application has to do a cross check
**               to ensure that data on Controller 1 and Controller 2 are equal
**               (see FRS130). If this cross check fails then this must be
**               indicated to the CSS by calling this function.
**
**               **Attention:**
**               The calling of this function may cause that this Safety
**               Connection will go into the Faulted state (see
**               {SAPL_IxsvsEventHandlerClbk()}. Therefore this function must be
**               called fast enough to meet the System Reaction Time.
**
**               **Attention:**
**               Volume 5 also uses the term "cross checking" for checking
**               received actual data versus complement data (only in messages
**               with a 3 to 250 Byte Data section). This is completely covered
**               by CIP Safety Software, i.e. in this case the CSS reacts
**               according to the sample code identical to this function.
**
** Parameters  : u16_svInst (IN) - The safety validator instance which had
**                                 delivered the mismatching data
**                                 (checked, valid range: 1..
**                                 (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                 CSOS_cfg_NUM_OF_SV_SERVERS))
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  void IXSVO_RxDataCrossCheckError(CSS_t_UINT u16_svInst);
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*******************************************************************************
**
** Function    : IXSVO_InstanceInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Object Instance (e.g. state, status
**               (see FRS125), type, etc.). See type definition of
**               {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range: 1..
**                                   (CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS))
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_InstanceInfoGet(CSS_t_UINT u16_instId,
                           CSS_t_VALIDATOR_INFOS *ps_svoInfos);


/*******************************************************************************
**
** Function    : IXSVO_FaultCountersReset
**
** Description : This API function resets the Fault Counters of all Safety
**               Validator instances similar like the class specific service
**               "Reset all error counters" for the Safety Validator Class.
**
**               **Attention**: According to CIP Networks Library Volume 5 the
**               Fault Counters must be reset to 0 each hour. The CSS only
**               counts the errors but doesn't reset the counters. Thus it is
**               the responsibility of the Safety Application to call this
**               function each hour.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVO_FaultCountersReset(void);


#endif /* IXSVO_API_H */

/*** End of File ***/


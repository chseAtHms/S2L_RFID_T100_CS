/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEint.h
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             IXSCEint.h is the internal header file of the IXSCE unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_FwdCloseParse
**             IXSCE_SafetyOpenProc
**             IXSCE_SafetyCloseProc
**             IXSCE_SafetyOpenProc2
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCEINT_H
#define IXSCEINT_H


/******************************************************************************
**    constants and macros
******************************************************************************/

/** k_OFS_FWDO_xxx:
    Byte offset of the fields within a Forward_Open packet
*/
#define k_OFS_FWDO_NCNXNID_OT      2U  /* Network Connection ID O->T */
#define k_OFS_FWDO_NCNXNID_TO      6U  /* Network Connection ID T->O */
#define k_OFS_FWDO_CNXN_S_NUM      10U /* Connection Serial Number */
#define k_OFS_FWDO_ORIG_V_ID       12U /* Originator Vendor ID */
#define k_OFS_FWDO_ORIG_S_NUM      14U /* Originator Serial Number */
#define k_OFS_FWDO_CNXN_TMULT      18U /* Connection Timeout Multiplier */
#define k_OFS_FWDO_RPI_OT          22U /* Requested Packet Interval O->T */
#define k_OFS_FWDO_NCNXN_PAR_OT    26U /* Network Connection Parameters O->T */
#define k_OFS_FWDO_RPI_TO          28U /* Requested Packet Interval T->O */
#define k_OFS_FWDO_NCNXN_PAR_TO    32U /* Network Connection Parameters T->O */
#define k_OFS_FWDO_TCLASS_TRIG     34U /* Transport Class Trigger */
#define k_OFS_FWDO_CNXN_P_SIZE     35U /* Connection Path Size */


#define k_CPCRC_SEED_VALUE         0xFFFFFFFFU    /* CPCRC Seed Value */

/*
** rest of the file is only relevant for Targets
*/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)

#define k_SCCRC_SEED_VALUE         0xFFFFFFFFU    /* SCCRC Seed Value */

/** k_OFS_FWDC_xxx:
    Byte offset of the fields within a Forward_Close packet
*/
#define k_OFS_FWDC_CNXN_S_NUM       2U /* Connection Serial Number */
#define k_OFS_FWDC_ORIG_V_ID        4U /* Originator Vendor ID */
#define k_OFS_FWDC_ORIG_S_NUM       6U /* Originator Serial Number */


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSCE_t_SCLOSE_RESP:
    Structure for passing the General Status Code and the Application Reply
    data for a Froward Close Response from the Safety Validator to the
    Forward_Close Response sending function.
*/
typedef struct
{
  CSS_t_MR_RESP_STATUS s_status;    /* Status Code (General and Extended) */
  /* further information needed by CSAL */
  CSS_t_UINT  u16_instId;           /* safety validator instance ID */
  CSS_t_USINT u8_consNum;           /* Consumer_Number */
} IXSCE_t_SCLOSE_RESP;

#endif   /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_FwdCloseParse
**
** Description : This function parses the received Forward_Close request (byte
**               stream) and extracts the parameters into structures.
**
** Parameters  : ps_rxHalcMsg (IN)       - pointer to the received HALC message
**                                         structure
**                                         (valid range: <> CSS_k_NULL
**                                         not checked, checked in
**                                         IXSCE_CmdProcess())
**               ps_cnxnTriad (OUT)      - pointer to the structure to where the
**                                         relevant data of the received
**                                         Forward_Close request is to be stored
**                                         (not checked, only called with
**                                         reference to structure variable)
**               ps_sCloseRespStat (OUT) - pointer to structure for returning
**                                         the status for a Forward_Open
**                                         response
**                                         (not checked, only called with
**                                         reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
void IXSCE_FwdCloseParse(const HALCS_t_MSG *ps_rxHalcMsg,
                         CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                         CSS_t_MR_RESP_STATUS *ps_sCloseRespStat);
#endif


/*******************************************************************************
**
** Function    : IXSCE_SafetyOpenProc
**
** Description : This function processes a SafetyOpen request. It checks all
**               parameters and determines if it can open the connection or not.
**
** Parameters  : ps_sOpenPar (IN)     - pointer to the structure which
**                                      contains the data that were received
**                                      in the Forward_Open request.
**                                      (not checked, only called with
**                                      reference to structure variable)
**               ps_sOpenAuxData (IN) - Auxiliary data that have been generated
**                                      when parsing the Forward_Open Request.
**                                      This information is for CSS internal
**                                      use only and will not be passed to the
**                                      application.
**                                      (not checked, only called with
**                                      reference to structure variable)
**               ps_sOpenResp (OUT)   - pointer to structure for returning the
**                                      parameters for a Forward_Open response
**                                      (not checked, only called with
**                                      reference to structure variable)
**
** Returnvalue : CSS_k_OK             - success
**               <>CSS_k_OK           - error
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  CSS_t_WORD IXSCE_SafetyOpenProc(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                  const IXSCE_t_SOPEN_AUX *ps_sOpenAuxData,
                                  CSS_t_SOPEN_RESP *ps_sOpenResp);
#endif


/*******************************************************************************
**
** Function    : IXSCE_SafetyCloseProc
**
** Description : This function searches the Safety Validator Instance which
**               matches the received Forward_Close parameters and then closes
**               this instance.
**
** Parameters  : ps_cnxnTriad (IN)   - pointer to received Forward_Close
**                                     parameters (only relevant data)
**                                     (not checked, only called with
**                                     reference to structure variable)
**               ps_sCloseResp (OUT) - pointer to structure for returning the
**                                     parameters for a Forward_Close response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
void IXSCE_SafetyCloseProc(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                                 IXSCE_t_SCLOSE_RESP *ps_sCloseResp);
#endif


/*******************************************************************************
**
** Function    : IXSCE_SafetyOpenProc2
**
** Description : This function is called from {IXSCE_SafetyOpenProc()} and
**               implements further checks and allocation and initialization
**               of a safety validator instance.
**
** Parameters  : ps_sOpenPar (IN)    - pointer to received Forward_Open
**                                     parameters
**                                     (not checked, only called with
**                                     reference to structure variable)
**               ps_svInfo (IN)      - in case the connection is already
**                                     existing this struct contains information
**                                     about it.
**                                     (not checked, only called with reference
**                                     to structure variable)
**               u8_payloadSize (IN) - number of bytes to be transported as the
**                                     payload of this safety connection
**                                     (valid range: 1..250, not checked,
**                                     checked in CnxnSizeCheck())
**               ps_sOpenResp (OUT)  - pointer to structure for returning the
**                                     parameters for a Forward_Open response
**                                     (not checked, only called with
**                                     reference to structure variable)
**
** Returnvalue : CSS_k_OK            - success
**               <>CSS_k_OK          - error
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_WORD IXSCE_SafetyOpenProc2(const CSS_t_SOPEN_PARA *ps_sOpenPar,
                                 const CSS_t_SV_ID *ps_svInfo,
                                 CSS_t_USINT u8_payloadSize,
                                 CSS_t_SOPEN_RESP *ps_sOpenResp);
#endif


#endif /* #ifndef IXSCEINT_H */

/*** End Of File ***/


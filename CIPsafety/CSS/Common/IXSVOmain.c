/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOmain.c
**    Summary: IXSVO - Safety Validator Object
**             This module contains the main routines of the IXSVO unit. It
**             contains the command handler for HALC messages and cyclic
**             handling of the IXSVO instances and an API function with which
**             SAPL can report Cross Check errors.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_CmdProcess
**             IXSVO_RxDataCrossCheckError
**
**             ValidatorRxProc
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#include "HALCSapi.h"

#include "IXSERapi.h"

#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVC.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #include "IXSVS.h"
#endif
#include "IXSSS.h"
#include "IXSCF.h"

#include "IXSVOapi.h"
#include "IXSVOerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void ValidatorRxProc(const HALCS_t_MSG *ps_rxHalcMsg);


/*******************************************************************************
**    global functions
*******************************************************************************/

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
void IXSVO_CmdProcess(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* if passed pointer is invalid */
  if (ps_rxHalcMsg == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_PTR_INV_CMDPROC,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    /* return value already set */
  }
  else /* passed pointer is valid */
  {
    /* if data pointer in passed structure is invalid */
    if (ps_rxHalcMsg->pb_data == CSS_k_NULL)
    {
      SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_PTR_INV_CMDDATA,
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      /* return value already set */
    }
    else /* data pointer is valid */
    {
      /* if CIP Safety Stack is in an improper state */
      if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
      {
        /* CIP Safety Stack is not in running state */
        SAPL_CssErrorClbk(IXSVO_k_FSE_AIS_STATE_ERR,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        /* return value already set */
      }
      else /* else: CSS is in a proper state */
      {
        /* determine which command was issued */
        switch (ps_rxHalcMsg->u16_cmd)
        {
          case CSOS_k_CMD_IXCO_IO_DATA:
          {
            /* pass message to handler function for received I/O messages */
            ValidatorRxProc(ps_rxHalcMsg);
            break;
          }

          default:
          {
            /* command unknown to this unit */
            SAPL_CssErrorClbk(IXSVO_k_NFSE_RXH_CMD_UNKNOWN,
                              ps_rxHalcMsg->u16_cmd, IXSER_k_A_NOT_USED);
            /* return value already set */
            break;
          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
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
void IXSVO_RxDataCrossCheckError(CSS_t_UINT u16_svInst)
{

  /* if CIP Safety Stack is in an improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk(IXSVO_k_FSE_AIS_STATE_ERR_DXC, IXSER_k_I_NOT_USED,
                      IXSER_k_A_NOT_USED);
  }
  else /* else: CSS is in a proper state */
  {
    /* if passed parameter is invalid */
    if (    (u16_svInst == 0U)
         || (u16_svInst > (CSOS_cfg_NUM_OF_SV_CLIENTS
                          +CSOS_cfg_NUM_OF_SV_SERVERS))
       )
    {
      SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_XCHECK_INST_INV, u16_svInst,
                        IXSER_k_A_NOT_USED);
    }
    else /* else: instance ID is valid */
    {
      /* if this is a Client instance */
      if (!IXSVD_InstIsServer(u16_svInst))
      {
        SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_XCHECK_TYPE_INV, u16_svInst,
                          IXSER_k_A_NOT_USED);
      }
      else /* else: this is a Server instance */
      {
        CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_svInst);

        /* if the returned index is invalid */
        if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
        {
          SAPL_CssErrorClbk(IXSVO_k_FSE_AIP_XCHECK_INST_NUM, u16_svInst,
                            IXSER_k_A_NOT_USED);
        }
        else /* else: valid server index */
        {
          /* pass this info to the corresponding safety validator instance */
          IXSVS_RxDataCrossCheckError(u16_svsIdx);
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : ValidatorRxProc
**
** Description : This function is called by the message handler of this unit to
**               process received Safety I/O messages.
**
** Parameters  : ps_rxHalcMsg (IN) - pointer to received HALC message structure
**                                   (not checked, checked in
**                                   IXSVO_CmdProcess())
**
** Returnvalue : -
**
*******************************************************************************/
static void ValidatorRxProc(const HALCS_t_MSG *ps_rxHalcMsg)
{
  /* if received Safety Validator Instance ID is not valid */
  if (    (ps_rxHalcMsg->u32_addInfo == 0U)
       || (ps_rxHalcMsg->u32_addInfo >
            (CSOS_cfg_NUM_OF_SV_CLIENTS+CSOS_cfg_NUM_OF_SV_SERVERS))
     )
  {
    SAPL_CssErrorClbk(IXSVO_k_NFSE_RXI_INST_RX_NUM,
                      IXSER_k_I_NOT_USED, ps_rxHalcMsg->u32_addInfo);
    /* return value already set */
  }
  /* else if received length is less than min length of a Safety I/O message */
  else if (ps_rxHalcMsg->u16_len
           < ((2U * CSOS_k_SIZEOF_BYTE) + CSOS_k_IO_MSGLEN_TCOO))
  {
    SAPL_CssErrorClbk(IXSVO_k_NFSE_RXI_TOO_SHORT,
                      (CSS_t_UINT)ps_rxHalcMsg->u32_addInfo,
                      (CSS_t_UDINT)ps_rxHalcMsg->u16_len);
    /* return value already set */
  }
  else /* else: ok */
  {
    /* safety data length (subtract Consumer Number and Pad Byte) */
    const CSS_t_UINT u16_sdataLen =
      (CSS_t_UINT)(ps_rxHalcMsg->u16_len - (2U * CSOS_k_SIZEOF_BYTE));
    /* pointer to safety data (jump over Consumer Number and Pad Byte) */
    const CSS_t_BYTE * const pb_sdata =
      CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data, (2U * CSOS_k_SIZEOF_BYTE));
    /* Get Safety Validator Instance ID from the received message */
    const CSS_t_UINT u16_svInst = (CSS_t_UINT)ps_rxHalcMsg->u32_addInfo;
    /* Get Safety Validator index (server or client) */
    const CSS_t_UINT u16_svIdx = IXSVD_SvIdxFromInstGet(u16_svInst);

    /* if this Instance is a Server */
    if (IXSVD_InstIsServer(u16_svInst))
    {
    #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
      /* if the returned index is valid */
      if (u16_svIdx < CSOS_cfg_NUM_OF_SV_SERVERS)
      {
        /* do all the data handling for received Server data */
        IXSVS_RxProc(u16_svIdx, u16_sdataLen, pb_sdata);
      }
      else /* else: index is invalid */
    #endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */
      {
        SAPL_CssErrorClbk(IXSVO_k_NFSE_RXI_INST_S_RX_NUM,
                          u16_svInst, IXSER_k_A_NOT_USED);
        /* return value already set */
      }
    }
    else /* else: this instance is a Client */
    {
    #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
      /* Get Consumer Number from the received message */
      CSS_t_USINT u8_consNum = 0U;

      /* if the returned index is valid */
      if (u16_svIdx < CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        /* Get Consumer Number from the received message */
        CSS_N2H_CPY8(&u8_consNum, ps_rxHalcMsg->pb_data);

        /* Let Safety Validator Server process the received Time Coordination */
        IXSVC_TcooRxProc(u16_svIdx, u8_consNum, u16_sdataLen, pb_sdata);
        /* return value already set */
      }
      else /* else: index is invalid */
    #endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */
      {
        SAPL_CssErrorClbk(IXSVO_k_NFSE_RXI_INST_C_RX_NUM,
                          u16_svInst, IXSER_k_A_NOT_USED);
        /* return value already set */
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*** End Of File ***/


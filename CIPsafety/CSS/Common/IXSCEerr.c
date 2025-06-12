/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEerr.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This file implements the (optional) error strings of the IXSCE
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_ErrorStrGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSScfg.h"

#if (CSS_cfg_ERROR_STRING == CSS_k_ENABLE)

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"

#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSCEerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/


/*******************************************************************************
**
** Function    : IXSCE_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSCE_k_FSE_xxx and
**                                        IXSCE_k_NFSE_xxx})
**               u16_instId (IN)        - instance Id (e.g. of the safety
**                                        validator instance that caused the
**                                        error), usage is error code specific
**                                        (not checked)
**               dw_addInfo (IN)        - additional error information.
**                                        (not checked, any value allowed)
**               u16_errStrBufSize (IN) - size of the error string buffer
**                                        (checked in IXSER_ErrorStrGet(),
**                                        valid range:
**                                        >= IXSER_k_MAX_ERROR_STRING_LEN)
**               pc_errStr (OUT)        - pointer to the error string buffer
**                                        (buffer to where this function will
**                                        write the error string)
**                                        (not checked, checked in
**                                        IXSER_ErrorStrGet())
**                                        valid range: <> CSS_k_NULL
**
** Returnvalue : 1..u16_errStrBufSize   - Success, length of the error string in
**                                        the error string buffer without '\0'
**               else                   - Error string buffer size is too small.
**                                        This is the return value of
**                                        CSS_SNPRINTF(): According to the ANSI
**                                        C99 definition the return value of
**                                        snprintf() is the number of characters
**                                        that would have been written if n had
**                                        been sufficiently large (not counting
**                                        the terminating null character).
**                                        Unfortunately in many implementations
**                                        the return value is < 0 if the
**                                        provided buffer is too small.
**
*******************************************************************************/
CSS_t_INT IXSCE_ErrorStrGet(CSS_t_WORD w_errorCode,
                            CSS_t_UINT u16_instId,
                            CSS_t_DWORD dw_addInfo,
                            CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                            CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSCE_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSCE_k_NFSE_RXH_CMD_UNKNOWN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXH_CMD_UNKNOWN - IXSCE_CmdProcess() has received an "
        "unknown Command from HALC (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_TXH_O_SUCRSP_SEND:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_TXH_O_SUCRSP_SEND - Error Sending a Foward_Open Success "
        "Response (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_TXH_C_SUCRSP_SEND:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_TXH_C_SUCRSP_SEND - Error Sending a Forward_Close "
        "Success Response (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_TXH_O_ERRRSP_SEND:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_TXH_O_ERRRSP_SEND - Error Sending a Foward_Open "
        "Unsuccessful Response (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_TXH_C_ERRRSP_SEND:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_TXH_C_ERRRSP_SEND - Error Sending a Forward_Close "
        "Unsuccessful Response (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_FSE_AIS_STATE_ERROR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_AIS_STATE_ERROR - IXSCE_CmdProcess() called in an invalid "
        "stack state"));
      break;
    }

    case IXSCE_k_FSE_AIP_PTR_INV_CMDPROC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_AIP_PTR_INV_CMDPROC - IXSCE_CmdProcess() called with "
        "an invalid pointer"));
      break;
    }

    case IXSCE_k_FSE_AIP_PTR_INV_CMDDATA:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_AIP_PTR_INV_CMDDATA - IXSCE_CmdProcess() called with "
        "an invalid pointer"));
      break;
    }

    case IXSCE_k_FSE_INC_PTR_INV_KEY_PRS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_PTR_INV_KEY_PRS - Internal function "
        "ElectronicKeyParse() called with an invalid pointer"));
      break;
    }

    case IXSCE_k_FSE_INC_PTR_INV_NSS_PRS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_PTR_INV_NSS_PRS - Internal function "
        "NetSegmentSafetyParse() called with an invalid pointer"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SC_INV_CNXN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SC_INV_CNXN - SafetyClose referenced a connection "
        "that could not be found"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_TOO_SHORT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_TOO_SHORT - SafetyOpen message was shorter than "
        "minimum length (%" CSS_PRIu32 " bytes)", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_AP_SIZE_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_AP_SIZE_1 - SafetyOpen message's application "
        "path length (%" CSS_PRIu16 " bytes) longer than remaining length "
        "(%" CSS_PRIu32 ")", u16_instId, dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_AP_SIZE_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_AP_SIZE_2 - SafetyOpen message's application "
        "path length (%" CSS_PRIu16 " bytes) shorter than remaining length "
        "(%" CSS_PRIu32 ")", u16_instId, dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SEG_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SEG_INV - SafetyOpen received with an invalid "
        "Application Path"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SC_TOO_SHORT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SC_TOO_SHORT - SafetyClose message was shorter than "
        "minimum length (%" CSS_PRIu32 " bytes)", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_CLASS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_CLASS - SafetyOpen message's application "
        "path addresses invalid class"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_INST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_INST - SafetyOpen message's application "
        "path addresses a 32-bit instance"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SSEG_SIZE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SSEG_SIZE - SafetyOpen message's Safety Segment "
        "size does not match Safety Segment type"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CPCRC_MIS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CPCRC_MIS - SafetyOpen checks: CPCRC mismatch "
        "(expected 0x%08" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_NO_TUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_NO_TUNID - SafetyOpen checks: received "
        "SafetyOpen but our TUNID is not yet set"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_TUNID_MIS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_TUNID_MIS - SafetyOpen checks: TUNID mismatch"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_DUPLICATE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_DUPLICATE - SafetyOpen checks: Duplicate "
        "Forward_Open received"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_1 - SafetyOpen checks: Electronic Key "
        "mismatch: Product Code or Vendor ID (compatibility bit = %" CSS_PRIu32
        ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_2 - SafetyOpen checks: Electronic Key "
        "mismatch: Device Type (compatibility bit = %" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_3 - SafetyOpen checks: Electronic Key "
        "mismatch: Revision (compatibility bit = %" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_MCAST_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_MCAST_LEN - SafetyOpen checks: payload data "
        "length (%" CSS_PRIu32 " bytes) less than Multicast TimeCorrection "
        "message length", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_LEN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_LEN - SafetyOpen checks: payload data "
        "length mismatch"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SS_INVNCP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SS_INVNCP - SafetyOpen checks: Network "
        "Connection Parameters OT mismatch (Server)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CS_INVNCP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CS_INVNCP - SafetyOpen checks: Network "
        "Connection Parameters OT mismatch (Client)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CM_INVNCP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CM_INVNCP - SafetyOpen checks: Network "
        "Connection Parameters TO mismatch (multi-cast Client)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TMULT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TMULT - SafetyOpen checks: TimeoutMultiplier "
        "invalid (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCT_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCT_1 - SafetyOpen checks: TransportClass/"
        "Trigger invalid (0x%02" CSS_PRIX32 "), Server not supported",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCT_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCT_2 - SafetyOpen checks: TransportClass/"
        "Trigger invalid (0x%02" CSS_PRIX32 "), Client not supported",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCT_3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCT_3 - SafetyOpen checks: TransportClass/"
        "Trigger invalid (0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCEPI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCEPI - SafetyOpen checks: TimeCorrection EPI"
        "must be zero for Single-cast"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCNCP1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCNCP1 - SafetyOpen checks: TimeCorrection "
        "Network Connection Parameters must be zero (Server)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_PIEM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_PIEM - SafetyOpen checks: Ping Interval EPI "
        "multiplier (%" CSS_PRIu32 ") not within valid range", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_PCI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_PCI - SafetyOpen checks: Ping_Count_Interval "
        "greater than 100 seconds"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCMMM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCMMM - SafetyOpen checks: Time Coordination "
        "Message Min Multiplier (%" CSS_PRIu32 ") greater than 7813",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_NTEM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_NTEM - SafetyOpen checks: Network Time "
        "Expectation Multiplier (%" CSS_PRIu32 ") greater than 45313",
        dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_STMUL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_STMUL - SafetyOpen checks: Safety Timeout "
        "Multiplier (%" CSS_PRIu32 ") invalid", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_MCNUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_MCNUM - SafetyOpen checks: Max Consumer "
        "Number invalid"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCCID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCCID - SafetyOpen checks: Time Correction "
        "Connection (%" CSS_PRIu32 ") ID invalid", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_MFNUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_MFNUM - SafetyOpen checks: Max Fault Number "
        "greater (%" CSS_PRIu32 ") than 255", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_ITSRV1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_ITSRV1 - SafetyOpen checks: Initial TimeStamp "
        "or Initial Rollover Value is invalid (Target Multicast Producer)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_CASM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_CASM - SafetyOpen checks: Application Path: "
        "Configuration Class ID (%" CSS_PRIu32 ") is invalid", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CASM_NULL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CASM_NULL - SafetyOpen checks: Application Path: "
        "Configuration Assembly Instance ID (%" CSS_PRIu16 ") shall not be "
        "NULL in Type1 SafetyOpen", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_S_PROD_NN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_S_PROD_NN - SafetyOpen checks: Application Path: "
        "For a Safety Validator Server the Produced Connection Point (%"
        CSS_PRIu16 ") must be NULL", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_S_CONS_I:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_S_CONS_I - SafetyOpen checks: Application Path: "
        "Consumed Connection Point (%" CSS_PRIu16 ") of a Safety Validator "
        "Server either not existing or is not an output", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_C_CONS_NN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_C_CONS_NN - SafetyOpen checks: Application Path: "
        "For a Safety Validator Client the Consumed Connection Point (%"
        CSS_PRIu16 ") must be NULL", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_C_PROD_I:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_C_PROD_I - SafetyOpen checks: Application Path: "
        "Produced Connection Point (%" CSS_PRIu16 ") of a Safety Validator "
        "Client either not existing or is not an input", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SAPL_DENY:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SAPL_DENY - SafetyOpen checks: request was "
        "denied by the Safety Application"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SAPL_APPL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SAPL_APPL - SafetyOpen checks: configuration "
        "data of Type1 SafetyOpen not applied by Safety Application"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_STORE_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_STORE_1 - SafetyOpen checks: error in store "
        "operation of SCID"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_STORE_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_STORE_2 - SafetyOpen checks: error in store "
        "operation of OCPUNID"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SSO_SM_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SSO_SM_1 - SafetyOpen checks: Safety Supervisor "
        "Object state machine returned an error while Type1 SafetyOpen"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_UNCONF:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_UNCONF - SafetyOpen checks: Type2 SafetyOpen "
        "received but device is unconfigured"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SCID_MIS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SCID_MIS - SafetyOpen checks: Type2 SafetyOpen "
        "received with non-matching SCID"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_OCP_NF:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_OCP_NF - SafetyOpen checks: Output Connection "
        "Point (%" CSS_PRIu16 ") not found", u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_OCPUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_OCPUNID - SafetyOpen checks: Output Connection "
        "point owned by somebody else"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SCCRC_MIS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SCCRC_MIS - SafetyOpen checks: SCCRC mismatch in "
        "Type1 SafetyOpen"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_TOOL_OWND:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_TOOL_OWND - SafetyOpen checks: Type1 SafetyOpen "
        "received but configuration is owned by tool"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CFUNID:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CFUNID - SafetyOpen checks: Type1 SafetyOpen "
        "received but configuration is owned by somebody else"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SSO_SM_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SSO_SM_2 - SafetyOpen checks: Safety Supervisor "
        "Object state machine returned an error while applying Type1 "
        "SafetyOpen data"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCNCP2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCNCP2 - SafetyOpen checks: TimeCorrection "
        "Network Connection Parameters must be zero (Single-cast Client)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_TCNCP3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_TCNCP3 - SafetyOpen checks: TimeCorrection "
        "Network Connection Parameters invalid (Multi-cast Client)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_STORE_3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_STORE_3 - SafetyOpen checks: error in store "
        "operation of CFUNID"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_TOO_LONG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_TOO_LONG - SafetyOpen checks: message was longer "
        "than maximum length (%" CSS_PRIu32 " bytes)", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_BASE_NOT_SUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_BASE_NOT_SUP - SafetyOpen checks: received request "
        "is for Base Format, but this is not supported"));
      break;
    }

    case IXSCE_k_NFSE_RXE_EXT_NOT_SUP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_EXT_NOT_SUP - SafetyOpen checks: received request "
        "is for Extended Format, but this is not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_CNXN_SIZE_CHECK:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_CNXN_SIZE_CHECK - SafetyOpen checks: inconsistency "
        "detected - tried to check multicast but not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_APP_VER_NO_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_APP_VER_NO_C - SafetyOpen checks: Unsupported "
        "Transport Trigger detected - Client not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_APP_VER_NO_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_APP_VER_NO_S - SafetyOpen checks: Unsupported "
        "Transport Trigger detected - Server not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_CPVS2_NO_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_CPVS2_NO_C - SafetyOpen checks: Unsupported "
        "Transport Trigger detected - Client not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_CPVS2_NO_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_CPVS2_NO_S - SafetyOpen checks: Unsupported "
        "Transport Trigger detected - Server not supported"));
      break;
    }

    case IXSCE_k_FSE_INC_CPVS2_XPORT_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_CPVS2_XPORT_INV - SafetyOpen checks: Invalid "
        "Transport Trigger detected - earlier check should have detected this "
        "already (trig=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_INV_ITSRV2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_INV_ITSRV2 - SafetyOpen checks: Initial TimeStamp "
        "or Initial Rollover Value is invalid (Target Singlecast Consumer)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_1:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_1 - SafetyOpen checks: Electronic Key "
        "contains wildcard: Product Code or Vendor ID"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_2 - SafetyOpen checks: Electronic Key "
        "contains wildcard: Device Type"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_3:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_EL_KEY_WC_3 - SafetyOpen checks: Electronic Key "
        "contains wildcard: Revision"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SRV_EXISTS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SRV_EXISTS - SafetyOpen checks: There is an "
        "existing connection to the same Originator but of different type "
        "(Instance %" CSS_PRIu16 ": existing Server - requested Client)",
        u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CLI_EXISTS:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CLI_EXISTS - SafetyOpen checks: There is an "
        "existing connection to the same Originator but of different type "
        "(Instance %" CSS_PRIu16 ": existing Client - requested Server)",
        u16_instId));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CLI_RPI_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CLI_RPI_INV - SafetyOpen checks: One of the "
        "RPIs is invalid for a Safety Validator Client"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SRV_RPI_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SRV_RPI_INV - SafetyOpen checks: One of the "
        "RPIs is invalid for a Safety Validator Server"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_TUNID_RX_FF:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_TUNID_RX_FF - SafetyOpen checks: Received a"
        "0xFF TUNID"));
      break;
    }

    case IXSCE_k_FSE_INC_MR_RESP_FWD_OP:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_FSE_INC_MR_RESP_FWD_OP - Forward_Open error response sending: "
        "inconsistent Message Router Response struct"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_SS_INVNCPTO:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_SS_INVNCPTO - SafetyOpen checks: Network "
        "Connection Parameters TO mismatch (Server)"));
      break;
    }

    case IXSCE_k_NFSE_RXE_SO_CS_INVNCPTO:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSCE_k_NFSE_RXE_SO_CS_INVNCPTO - SafetyOpen checks: Network "
        "Connection Parameters TO mismatch (single-cast Client)"));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSCE unit (%" CSS_PRIu16
        ") (%" CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (i_ret);
/* This function has a lot of paths, but is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSS_cfg_ERROR_STRING == CSS_k_ENABLE) */

/*** End Of File ***/



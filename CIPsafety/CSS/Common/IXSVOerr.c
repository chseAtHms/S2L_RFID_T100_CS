/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVOerr.c
**    Summary: IXSVO - Safety Validator Object
**             This file implements the (optional) error strings of the IXSVO
**             unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVO_ErrorStrGet
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


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVO_ErrorStrGet
**
** Description : This function returns an error string which describes the error
**               exactly.
**
** Parameters  : w_errorCode (IN)       - 16 bit value that holds Error Type,
**                                        Error Layer, Unit ID and Unit Error.
**                                        (checked, valid range: see
**                                        {IXSVO_k_FSE_xxx})
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
CSS_t_INT IXSVO_ErrorStrGet(CSS_t_WORD w_errorCode,
                             CSS_t_UINT u16_instId,
                             CSS_t_DWORD dw_addInfo,
                             CSS_t_SNPRINTF_BUF_SIZE u16_errStrBufSize,
                             CSS_t_SNPRINTF_CHAR *pc_errStr)
{
  /* return value of this function */
  CSS_t_INT i_ret = IXSER_k_BUF_PTR_NULL;

  switch (w_errorCode)
  {
    case IXSVO_k_FSE_INC_PRG_FLOW:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_PRG_FLOW - Unexpected Program Flow Error"));
      break;
    }

    case IXSVO_k_FSE_AIS_STATE_ERR_DXC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIS_STATE_ERR_DXC - API function called in invalid CSS "
        "state (IXSVO_RxDataCrossCheckError)"));
      break;
    }

    case IXSVO_k_FSE_AIS_STATE_ERR_IIG:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIS_STATE_ERR_IIG - API function called in invalid CSS "
        "state (IXSVO_InstanceInfoGet(%" CSS_PRIu16 "))", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RXH_CMD_UNKNOWN:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXH_CMD_UNKNOWN - unit received an unknown command (%"
        CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_CLOSE_INST_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_CLOSE_INST_INV - Safety Validator Instance ID is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_STOP_INST_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_STOP_INST_INV - Safety Validator Instance ID is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_STOP_CONS_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_STOP_CONS_NUM - Consumer Number is invalid (inst %"
        CSS_PRIu16 ") "
        "(Consumer_Num %" CSS_PRIu32, u16_instId, dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_VDATR_STAT_IDLE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_VDATR_STAT_IDLE - A safety validator is in an invalid "
        "state or reported an unexpected event to SSO (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_VDATR_STAT_ESTB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_VDATR_STAT_ESTB - A safety validator is in an invalid "
        "state or reported an unexpected event to SSO (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_VDATR_STAT_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_VDATR_STAT_FAIL - A safety validator is in an invalid "
        "state or reported an unexpected event to SSO (%" CSS_PRIu32 ")",
        dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_VDATR_STAT_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_VDATR_STAT_INV - A safety validator is in an invalid "
        "state (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_INV_CNXN_TYPE_A:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_CNXN_TYPE_A - CSAL requested an Originator "
        "connection with an invalid Cnxn Type (alloc) (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_INV_CNXN_TYPE_I:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_CNXN_TYPE_I - CSAL requested an Originator "
        "connection with an invalid Cnxn Type (init) (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_TARG_INIT_INST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_TARG_INIT_INST - Safety Validator Instance ID is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RXE_ORIG_INIT_INST:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXE_ORIG_INIT_INST - Safety Validator Instance ID is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_AIP_PTR_INV_CMDPROC:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_PTR_INV_CMDPROC - IXSVO_CmdProcess() called with "
        "an invalid pointer"));
      break;
    }

    case IXSVO_k_FSE_AIP_PTR_INV_CMDDATA:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_PTR_INV_CMDDATA - IXSVO_CmdProcess() called with "
        "an invalid pointer"));
      break;
    }

    case IXSVO_k_FSE_AIP_XCHECK_INST_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_XCHECK_INST_INV - IXSVO_RxDataCrossCheckError() was "
        "called with invalid Safety Validator Instance ID (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RXI_INST_RX_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXI_INST_RX_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_NFSE_RXI_INST_S_RX_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXI_INST_S_RX_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RXI_INST_C_RX_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXI_INST_C_RX_NUM - The passed 'instance' parameter is "
        "invalid (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RES_ALLOC_ORIG_SRV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RES_ALLOC_ORIG_SRV - Allocation of an Originator Safety "
        "Validator Server struct failed (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_AIP_XCHECK_TYPE_INV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_XCHECK_TYPE_INV - The passed 'instance' parameter is "
        "invalid (is not a server) (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_AIP_XCHECK_INST_NUM:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_XCHECK_INST_NUM - The passed 'instance' parameter is "
        "invalid (not existing) (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_AIP_PTR_INV_STATGET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_PTR_INV_STATGET - IXSVO_InstanceInfoGet() called with "
        "an invalid pointer"));
      break;
    }

    case IXSVO_k_FSE_INC_CL_WRONG_TYPE_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_CL_WRONG_TYPE_S - Tried to close a Safety Validator "
        "of an unsupported type (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_CL_WRONG_TYPE_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_CL_WRONG_TYPE_C - Tried to close a Safety Validator "
        "of an unsupported type (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_SP_WRONG_TYPE_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_SP_WRONG_TYPE_S - Tried to stop a Safety Validator of "
        "an unsupported type (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_SP_WRONG_TYPE_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_SP_WRONG_TYPE_C - Tried to stop a Safety Validator of "
        "an unsupported type (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_AIS_STATE_ERR:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIS_STATE_ERR - IXSVO_CmdProcess() called in an invalid "
        "stack state"));
      break;
    }

    case IXSVO_k_NFSE_RES_ALLOC_ORIG_CLI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RES_ALLOC_ORIG_CLI - Allocation of an Originator Safety "
        "Validator Client struct failed (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RES_ALLOC_TARG_SRV:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RES_ALLOC_TARG_SRV - Allocation of a Target Safety "
        "Validator Server Instance ID failed (%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RES_ALLOC_TARG_CLI:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RES_ALLOC_TARG_CLI - Allocation of a Target "
        "Safety Validator Client Instance ID failed (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETSTAT_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETSTAT_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETSTAT_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETSTAT_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETCP_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETCP_INST_S - Tried to get the Connection Point of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETCP_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETCP_INST_C - Tried to get the Connection Point of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETAGE_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETAGE_INST_S - Tried to get the Data Age of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_SETAGE_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_SETAGE_INST_S - Tried to set the Data Age of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETFC_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETFC_INST_S - Tried to set the Fault Counter of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETFC_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETFC_INST_C - Tried to set the Fault Counter of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_PREPARE_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_PREPARE_INST_S - Tried to prepare a Safety Validator "
        "which doesn't exist (although previous check was positive) (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_CLEAR_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_CLEAR_INST_S - Tried to clear a Safety Validator "
        "which doesn't exist (although previous check was positive) (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_CLEAR_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_CLEAR_INST_C - Tried to clear a Safety Validator "
        "which doesn't exist (although previous check was positive) (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_IIG_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_IIG_INST_S - Tried to get Instance Information for a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_IIG_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_IIG_INST_C - Tried to get Instance Information for a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_ALLOC_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ALLOC_INST_S - Tried to allocate a Safety Validator "
        "of not supported type (although previous check was positive) "
        "(trigger=0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_ALLOC_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ALLOC_INST_C - Tried to allocate a Safety Validator "
        "of not supported type (although previous check was positive) "
        "(trigger=0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_TINIT_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_TINIT_INST_S - Tried to initialize a Safety Validator "
        "of not supported type (Target) (although previous check was positive) "
        "(trigger=0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_TINIT_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_TINIT_INST_C - Tried to initialize a Safety Validator "
        "of not supported type (Target) (although previous check was positive) "
        "(trigger=0x%02" CSS_PRIX32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_INV_TRANS_IDLE:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_TRANS_IDLE - Invalid Safety Validator state "
        "transition to Idle (old state=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_INV_TRANS_INIT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_TRANS_INIT - Invalid Safety Validator state "
        "transition to Initializing (old state=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_INV_TRANS_ESTAB:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_TRANS_ESTAB - Invalid Safety Validator state "
        "transition to Established (old state=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_INV_TRANS_FAIL:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_INV_TRANS_FAIL - Invalid Safety Validator state "
        "transition to Failed (old state=%" CSS_PRIu32 ")", dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_AIP_INST_STATEGET:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_AIP_INST_STATEGET - IXSVO_InstanceInfoGet() was called "
        "with invalid Safety Validator Instance ID (%" CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_ALLOC_ORIG_NS_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ALLOC_ORIG_NS_C - Originator tries to allocate a "
        "Safety Validator Client, but clients are not supported (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_ALLOC_ORIG_NS_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ALLOC_ORIG_NS_S - Originator tries to allocate a "
        "Safety Validator Server, but servers are not supported (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_ORIG_INIT_NS_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ORIG_INIT_NS_C - Originator tries to initialize a "
        "Safety Validator Client, but clients are not supported (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_ORIG_INIT_NS_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_ORIG_INIT_NS_S - Originator tries to initialize a "
        "Safety Validator Server, but servers are not supported (inst=%"
        CSS_PRIu16 ")",
        u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETPIEM_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETPIEM_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETPIEM_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETPIEM_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTCMMM_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTCMMM_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTCMMM_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTCMMM_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETNTEM_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETNTEM_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETNTEM_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETNTEM_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTMULT_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTMULT_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTMULT_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTMULT_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETMCN_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETMCN_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETMCN_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETMCN_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETLEC_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETLEC_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETLEC_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETLEC_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_NFSE_RXI_TOO_SHORT:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_NFSE_RXI_TOO_SHORT - Received Safety I/O message is shorter "
        "than minimum Safety I/O message (inst=%" CSS_PRIu16 " len=%"
        CSS_PRIu32 ")",
        u16_instId, dw_addInfo));
      break;
    }

    case IXSVO_k_FSE_INC_EXP_INST_SRV_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_EXP_INST_SRV_S - Inconsistency detected: Explicit "
        "request addressed an instance that is supposed to be a server but "
        "servers are not supported (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_EXP_INST_SRV_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_EXP_INST_SRV_C - Inconsistency detected: Explicit "
        "request addressed an instance that is supposed to be a client but "
        "clients are not supported (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTYPE_INST_S:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTYPE_INST_S - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTYPE_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTYPE_INST_C - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETAGE_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETAGE_INST_C - Tried to get the Data Age of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_SETAGE_INST_C:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_SETAGE_INST_C - Tried to set the Data Age of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTCMMM_I_S_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTCMMM_I_S_2 - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETNTEM_I_S_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETNTEM_I_S_2 - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETTMULT_I_S_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETTMULT_I_S_2 - Tried to get an attribute of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    case IXSVO_k_FSE_INC_GETFC_INST_S_2:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "IXSVO_k_FSE_INC_GETFC_INST_S_2 - Tried to set the Fault Counter of a "
        "Safety Validator which doesn't exist (although previous check was "
        "positive) (inst=%" CSS_PRIu16 ")", u16_instId));
      break;
    }

    default:
    {
      i_ret = CSS_SNPRINTF((pc_errStr, u16_errStrBufSize,
        "Unknown error code was generated by IXSVO unit (%" CSS_PRIu16 ") (%"
        CSS_PRIu32 ") ",
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



/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSSCmain.c
**    Summary: IXSSC - Safety Stack Control
**             This unit controls the execution of the CIP Safety Stack.
**             The interface functions defined in this unit must be called by
**             the application to initialize and terminate CSS.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSSC_Init
**             IXSSC_Terminate
**             IXSSC_SoftErrByteGet
**             IXSSC_SoftErrVarGet
**
**             UnitsInit
**             InitInfoValidate
**             TerminateInfoToCsal
**             SoftErrAdvanceToNextLevel
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

/* checking of the configuration and data types */
#include "CSOScfgCheck.h"
#include "CSScfgCheck.h"

#include "IXSSS.h"
#include "IXSERapi.h"
#include "IXUTL.h"
#include "IXSVD.h"
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #include "IXSVS.h"
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #include "IXSVC.h"
#endif
#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
  #include "IXSAIapi.h"
  #include "IXSAI.h"
#endif
#include "IXSVO.h"
#include "IXSSOapi.h"
#include "IXSSO.h"
#include "IXUTLapi.h"
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #include "IXCCO.h"
#endif
#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  #include "IXSPI.h"
#endif
#include "IXSCF.h"

#include "IXSSCapi.h"
#include "IXSSCerr.h"

/* Check if configuration check is performed (also to make sure definitions   */
/* of CSOScfgCheck.h are used here so that this header file is in use.        */
#ifndef CSOS_CFG_CHECK_H
  #error Configuration not checked! (missing include of CSOScfgCheck.h)
#endif

/* Check if configuration check is performed (also to make sure definitions   */
/* of CSScfgCheck.h are used here so that this header file is in use.         */
#ifndef CSS_CFG_CHECK_H
  #error Configuration not checked! (missing include of CSScfgCheck.h)
#endif


/*******************************************************************************
**    global variables
*******************************************************************************/

/** IXSSC_kac_CSS_VERSION_STR:
    This constant represents the CSS version string. It includes the
    {CSS_k_STACK_VERSION} and the {CSS_k_STACK_STATUS} strings.
*/
const CSS_t_CHAR IXSSC_kac_CSS_VERSION_STR[] =
    "CSS VERSION: " CSS_k_STACK_VERSION " CSS STATUS: " CSS_k_STACK_STATUS;


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

#if (    (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)     \
      || (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES) \
    )
  /* counter variable for running through all CSS modules that provide static
     variables that must be protected against soft errors */
  /* Not relevant for Soft Error Checking - only a diagnostic value -
     additionally a soft error in this variable would lead to different byte
     positions being compared between the 2 controllers */
  static CSS_t_USINT u8_ModuleCnt;
#endif

#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
    /** kafp_ModuleVarGetList:
        A constant list of function pointers: these pointers reference the
        xxxSoftErrVarGet() functions of all CSS modules.
    */
  static CSS_t_UDINT (*const kafp_ModuleVarGetList[])(CSS_t_UDINT u32_varCnt,
                                                      CSS_t_BYTE *pb_var) =
  {
  #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSAI_InSoftErrByteGet,
  #endif
  #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSAI_OutSoftErrByteGet,
  #endif
    IXSSO_IdsSoftErrByteGet,
  #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSSO_ServiceSoftErrByteGet,
  #endif
    IXSSO_StateSoftErrByteGet,
    IXSSS_IdentSoftErrByteGet,
    IXSSS_MainSoftErrByteGet,
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)                 /* CCT_NO_PRE_WARNING */
    IXSVC_InitSoftErrByteGet,
    IXSVC_InitInfoSoftErrByteGet,
    IXSVC_InstSoftErrByteGet,
    IXSVC_InstIdSoftErrByteGet,
    IXSVC_InstInfoSoftErrByteGet,
    IXSVC_ProdSoftErrByteGet,
    IXSVC_QuarSoftErrByteGet,
    IXSVC_StateSoftErrByteGet,
    IXSVC_TcooSoftErrByteGet,
    IXSVC_UtilSoftErrByteGet,
  #endif
    IXSVD_LookupSoftErrByteGet,
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)                 /* CCT_NO_PRE_WARNING */
    IXSVS_ConsSoftErrByteGet,
    IXSVS_InitInfoSoftErrByteGet,
    IXSVS_InstSoftErrByteGet,
    IXSVS_InstIdSoftErrByteGet,
    IXSVS_RecvSoftErrByteGet,
    IXSVS_StateSoftErrByteGet,
    IXSVS_TcooSoftErrByteGet,
    IXSVS_TcorrSoftErrByteGet,
  #endif
  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)            /* CCT_NO_PRE_WARNING */
    IXCCO_CreateSoftErrByteGet,
    IXCCO_iAttrG1SoftErrByteGet,
    IXCCO_iAttrG2SoftErrByteGet,
    IXCCO_iAttrG3SoftErrByteGet,
    IXCCO_SoscCommonSoftErrByteGet,
    IXCCO_ValidateSoftErrByteGet,
  #endif
  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)/* CCT_NO_PRE_WARNING */
    IXSPI_MainSoftErrByteGet
  #endif
  };

  /* counter variable for running through all the bytes of static variables of
     one CSS module */
  /* Not relevant for Soft Error Checking - only a diagnostic value -
     additionally a soft error in this variable would lead to different byte
     positions being compared between the 2 controllers */
  static CSS_t_UDINT u32_VarCnt;


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  static void (*const kafp_ModuleVarGetList[])(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var) =
  {
  #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSAI_InSoftErrVarGet,
  #endif
  #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSAI_OutSoftErrVarGet,
  #endif
    IXSSO_IdsSoftErrVarGet,
  #if (CSS_cfg_SET_TUNID == CSS_k_ENABLE)
                                                        /* CCT_NO_PRE_WARNING */
    IXSSO_ServiceSoftErrVarGet,
  #endif
    IXSSO_StateSoftErrVarGet,
    IXSSS_IdentSoftErrVarGet,
    IXSSS_MainSoftErrVarGet,
  #if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)                 /* CCT_NO_PRE_WARNING */
    IXSVC_InitSoftErrVarGet,
    IXSVC_InitInfoSoftErrVarGet,
    IXSVC_InstSoftErrVarGet,
    IXSVC_InstIdSoftErrVarGet,
    IXSVC_InstInfoSoftErrVarGet,
    IXSVC_ProdSoftErrVarGet,
    IXSVC_QuarSoftErrVarGet,
    IXSVC_StateSoftErrVarGet,
    IXSVC_TcooSoftErrVarGet,
    IXSVC_UtilSoftErrVarGet,
  #endif
    IXSVD_LookupSoftErrVarGet,
  #if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)                 /* CCT_NO_PRE_WARNING */
    IXSVS_ConsSoftErrVarGet,
    IXSVS_InitInfoSoftErrVarGet,
    IXSVS_InstSoftErrVarGet,
    IXSVS_InstIdSoftErrVarGet,
    IXSVS_RecvSoftErrVarGet,
    IXSVS_StateSoftErrVarGet,
    IXSVS_TcooSoftErrVarGet,
    IXSVS_TcorrSoftErrVarGet,
  #endif
  #if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)            /* CCT_NO_PRE_WARNING */
    IXCCO_CreateSoftErrVarGet,
    IXCCO_iAttrG1SoftErrVarGet,
    IXCCO_iAttrG2SoftErrVarGet,
    IXCCO_iAttrG3SoftErrVarGet,
    IXCCO_SoscCommonSoftErrVarGet,
    IXCCO_ValidateSoftErrVarGet,
  #endif
  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)/* CCT_NO_PRE_WARNING */
    IXSPI_MainSoftErrVarGet
  #endif
  };

  /* counting the variables in the various levels */
  /* Not relevant for Soft Error Checking - only a diagnostic value -
     additionally a soft error in this variable would lead to different byte
     positions being compared between the 2 controllers */
  static CSS_t_UDINT au32_SoftErrCnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS];
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void UnitsInit(void);
static CSS_t_WORD InitInfoValidate(const CSS_t_ID_INFO *ps_initInfo);
static void TerminateInfoToCsal(void);
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  static CSS_t_BOOL SoftErrAdvanceToNextLevel(CSS_t_USINT u8_incLvl);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSSC_Init
**
** Description : This API function initializes the CSS units.
**               (see Req.3.1-3)
**
**               **Attention**:
**               It is the responsibility of the calling application
**               to ensure that at this time the HALC communications to the
**               other controller(s) is already initialized and working.
**               Otherwise it will not be possible to exchange safe I/O data.
**
**               **Attention**:
**               This API function must be called at startup to initialize the
**               stack before any other CSS function is called.
**
** See Also    : IXSSC_Terminate()
**
** Parameters  : ps_initInfo (IN)       - pointer to structure of type
**                                        {CSS_t_ID_INFO} which must contain
**                                        information that is necessary for the
**                                        stack to operate.
**                                        (checked, valid range: <> CSS_k_NULL -
**                                        all fields of the structure: not
**                                        checked, any value allowed)
**               u32_sysTime_128us (IN) - Current system time. System time is a
**                                        32-bit counter incremented in 128us
**                                        steps.
**                                        (not checked, any value allowed)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSC_Init(const CSS_t_ID_INFO *ps_initInfo,
                CSS_t_UDINT u32_sysTime_128us)
{
  /* if passed pointer is invalid */
  if (ps_initInfo == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSSC_k_FSE_AIP_PTR_INV_CMDPROC, IXSER_k_I_NOT_USED,
                      IXSER_k_A_NOT_USED);
  }
  else /* passed pointer is valid */
  {
    /* switch for the state of the IXSSC state machine */
    switch (IXSSS_StateGet())
    {
      case IXSSS_k_ST_NON_EXIST:
      {
        /* if passed initialization info is invalid */
        if (InitInfoValidate(ps_initInfo) != CSS_k_OK)
        {
          /* error: invalid Identity Information cannot continue */
        }
        else  /* else: Identity Information is ok */
        {
          /* set identity information */
          IXSSS_IdentityInfoStore(ps_initInfo);

          /* initialize all units of the CSS stack */
          UnitsInit();

          /* store the passed time as global system time */
          IXUTL_SetSysTime(u32_sysTime_128us);

          /* Initialization finished - progress to running state */
          IXSSS_StateSet(IXSSS_k_ST_RUNNING);

        #if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
          {
            /* initialize module global variables */
            u8_ModuleCnt = 0U;
            u32_VarCnt = 0U;
          }
        #elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
          {
            /* initialize module global variables */
            u8_ModuleCnt = 0U;
            CSS_MEMSET(au32_SoftErrCnt, 0, sizeof(au32_SoftErrCnt));
          }
        #endif
        }

        break;
      }

      case IXSSS_k_ST_RUNNING:
      {
        /* error - operation not allowed in current state */
        SAPL_CssErrorClbk(IXSSC_k_FSE_AIS_STATE_ERR_INIT, IXSER_k_I_NOT_USED,
                          IXSER_k_A_NOT_USED);

        break;
      }

      default:
      {
        /* invalid state */
        SAPL_CssErrorClbk(IXSSC_k_FSE_INC_STATE_INV_INIT, IXSER_k_I_NOT_USED,
                          IXSER_k_A_NOT_USED);

        break;
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSSC_Terminate
**
** Description : This API function performs the shutdown of the CIP Safety
**               Stack. The CSS cannot be used afterwards unless {IXSSC_Init()}
**               is called again.
**
**               **Attention**:
**               Before calling this function the application must switch
**               the device into the fail safe state as safety application
**               callback functions like SAPL_IxsvsEventHandlerClbk() or
**               SAPL_IxsvcEventHandlerClbk() e.g. to signal closing of a
**               connection, are not called any more!
**
**               **Attention**:
**               The Safety Application has to call this function if its
**               background diagnostics detects a failure of the communication.
**               This function will then terminate the CSS and thereby also all
**               connections (see FRS120).
**
** See Also    : IXSSC_Init()
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSSC_Terminate(void)
{
  /* switch for the state of the IXSSC state machine */
  switch (IXSSS_StateGet())
  {
    case IXSSS_k_ST_NON_EXIST:
    {
      /* error - operation not allowed in current state */
      SAPL_CssErrorClbk(IXSSC_k_FSE_AIS_STATE_ERR_TERM, IXSER_k_I_NOT_USED,
                        IXSER_k_A_NOT_USED);

      break;
    }

    case IXSSS_k_ST_RUNNING:
    {
      /* stack is terminated now - progress to Non-Exist state */
      IXSSS_StateSet(IXSSS_k_ST_NON_EXIST);

      /* Inform CSAL that CSS will shut down. */
      TerminateInfoToCsal();

      break;
    }

    default:
    {
      /* invalid state */
      SAPL_CssErrorClbk(IXSSC_k_FSE_INC_STATE_INV_TERM, IXSER_k_I_NOT_USED,
                        IXSER_k_A_NOT_USED);

      break;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSSC_SoftErrByteGet
**
** Description : This API function fills the provided buffer with data of
**               stack internal static variables which must be protected against
**               Soft Errors (see Req.5.3-17). The Safety Application can
**               implement Soft Error protection by different means (data cross
**               checking with other Safety Controller, calculating CRCs for
**               cross checking, etc.)
**
**               **Attention**:
**               The call of this API function must be synchronized with the
**               other Safety Controller to make sure CSS internal variables
**               have equal values on both controllers.
**
**               **Attention**:
**               This function should be considered deprecated in favor of
**               IXSSC_SoftErrVarGet().
**
** Parameters  : o_start (IN)             - if CSS_k_TRUE is passed then the
**                                          internal counter is reset and
**                                          the first chunk of stack internal
**                                          data is returned.
**                                        - if CSS_k_FALSE is passed consecutive
**                                          chunks are returned. This function
**                                          internally has a counter which
**                                          advances with every call.
**                                          (not checked - any value allowed)
**               pb_softErrCheckBuf (OUT) - The caller must provide a pointer to
**                                          a buffer which has at least a size
**                                          of {CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE}
**                                          bytes. After return this buffer is
**                                          filled with data to be checked.
**                                          (checked,
**                                          valid range: <> CSS_k_NULL)
**               pu8_ModuleCnt (OUT)      - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          module counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu32_VarCnt (OUT)        - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE               - The returned chunk is the last chunk
**                                          of data. The next call of this
**                                          function will return the first chunk
**                                          of data again (independent of
**                                          o_start).
**               CSS_k_FALSE              - The returned chunk is not yet the
**                                          last chunk of data.
**
*******************************************************************************/
CSS_t_BOOL IXSSC_SoftErrByteGet(CSS_t_BOOL o_start,
                                CSS_t_BYTE *pb_softErrCheckBuf,
                                CSS_t_USINT *pu8_ModuleCnt,
                                CSS_t_UDINT *pu32_VarCnt)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* counter for the number of bytes to be filled into pb_softErrCheckBuf */
  CSS_t_UINT u16_idx;
  /* return value of called functions */
  CSS_t_UDINT u32_res;

  /* if passed pointer is invalid */
  if (pb_softErrCheckBuf == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSSC_k_FSE_AIP_PTR_INV_SEVG_B,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* if static variable u8_ModuleCnt is invalid */
  else if (u8_ModuleCnt >= IXUTL_NUM_OF_ARRAY_ELEMENTS(kafp_ModuleVarGetList))
  {
    /* an invalid value in this variable would be fatal as it is used as the
       array index when calling a function from the kafp_ModuleVarGetList list
    */
    SAPL_CssErrorClbk(IXSSC_k_FSE_INC_MOD_CNT_SEVG_B,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_ModuleCnt);
    u8_ModuleCnt = 0U;
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
  {
    /* if we shall start from the beginning again */
    if (o_start)
    {
      /* beginn at the first module again */
      u8_ModuleCnt = 0U;
      /* start the variable counter at zero */
      u32_VarCnt = 0U;
    }
    else /* else: continuous call */
    {
      /* nothing to do */
    }

    u16_idx = 0U;
    /* fill the passed buffer byte by byte */
    do
    {
      /* call the function pointer of the current module list entry */
      u32_res = (*kafp_ModuleVarGetList[u8_ModuleCnt])
                   (u32_VarCnt, &pb_softErrCheckBuf[u16_idx]);

      /* if the called module indicated that it reached its end */
      if (u32_res != 0U)
      {
        /* advance to next module */
        u8_ModuleCnt++;

        /* if we have already processed the last module */
        if (u8_ModuleCnt >= IXUTL_NUM_OF_ARRAY_ELEMENTS(kafp_ModuleVarGetList))
        {
          /* beginn at the first module again */
          u8_ModuleCnt = 0U;
          /* indicate that the last chunk of data is returned */
          o_retVal = CSS_k_TRUE;
        }
        else /* else: arrived at another valid module */
        {
          /* nothing to do */
        }

        /* start the variable counter at zero for the next module */
        u32_VarCnt = 0U;
      }
      else /* else: end of module not yet reached */
      {
        /* advance to next byte */
        u32_VarCnt++;
        u16_idx++;
      }
    }
    while (u16_idx < CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE);
  }

  /* if passed pointer is NULL */
  if (pu8_ModuleCnt == CSS_k_NULL)
  {
    /* caller is not interested in diagnostic information */
  }
  else /* else: passed pointer is valid */
  {
    /* return value of internal module counter */
    *pu8_ModuleCnt = u8_ModuleCnt;
  }

  /* if passed pointer is NULL */
  if (pu32_VarCnt == CSS_k_NULL)
  {
    /* caller is not interested in diagnostic information */
  }
  else /* else: passed pointer is valid */
  {
    /* return value of internal variable counter (within current module) */
    *pu32_VarCnt = u32_VarCnt;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSSC_SoftErrVarGet
**
** Description : This API function fills the provided buffer with data of
**               stack internal static variables which must be protected against
**               Soft Errors (see Req.5.3-17). The Safety Application can
**               implement Soft Error protection by different means (data cross
**               checking with other Safety Controller, calculating CRCs for
**               cross checking, etc.)
**
**               **Attention**:
**               The call of this API function must be synchronized with the
**               other Safety Controller to make sure CSS internal variables
**               have equal values on both controllers.
**
** Parameters  : o_start (IN)             - if CSS_k_TRUE is passed then the
**                                          internal counter is reset and
**                                          the first chunk of stack internal
**                                          data is returned.
**                                        - if CSS_k_FALSE is passed consecutive
**                                          chunks are returned. This function
**                                          internally has a counter which
**                                          advances with every call.
**                                          (not checked - any value allowed)
**               pb_softErrCheckBuf (OUT) - The caller must provide a pointer to
**                                          a buffer which has at least a size
**                                          of {CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE}
**                                          bytes. After return this buffer is
**                                          filled with data to be checked.
**                                          (checked,
**                                          valid range: <> CSS_k_NULL)
**               pu16_waster (OUT)        - if <> CSS_k_NULL then this variable
**                                          will provide the number of unused
**                                          bytes in the provided buffer on
**                                          return. As the buffer is filled with
**                                          variables that can be 1, 2 or 4
**                                          bytes long and based on the fact
**                                          that variables will not get
**                                          subdivided up to 3 bytes at the end
**                                          of the buffer may remain unused.
**                                          In case the last chunk is reached
**                                          the buffer may contain even more
**                                          unused bytes.
**                                          (not checked, any value allowed)
**               pu8_ModuleCnt (OUT)      - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          module counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu8_VarLvl (OUT)         - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable level counter after the
**                                          execution of this function. In case
**                                          a Soft Error has been detected this
**                                          helps to analyze which stack
**                                          internal variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
**               pu32_VarCnt (OUT)        - if <> CSS_k_NULL then this variable
**                                          returns the value of the internal
**                                          variable counter after the execution
**                                          of this function. In case a Soft
**                                          Error has been detected this helps
**                                          to analyze which stack internal
**                                          variables are affected.
**                                          If this pointer is CSS_k_NULL then
**                                          this information will not be
**                                          returned.
**                                          (not checked, any value allowed)
** Returnvalue : CSS_k_TRUE               - The returned chunk is the last chunk
**                                          of data. The next call of this
**                                          function will return the first chunk
**                                          of data again (independent of
**                                          o_start).
**               CSS_k_FALSE              - The returned chunk is not yet the
**                                          last chunk of data.
**
*******************************************************************************/
CSS_t_BOOL IXSSC_SoftErrVarGet(CSS_t_BOOL o_start,
                               CSS_t_BYTE *pb_softErrCheckBuf,
                               CSS_t_UINT *pu16_waster,
                               CSS_t_USINT *pu8_ModuleCnt,
                               CSS_t_USINT *pu8_VarLvl,
                               CSS_t_UDINT *pu32_VarCnt)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* counter for the number of bytes to be filled into pb_softErrCheckBuf */
  CSS_t_UINT u16_idx;
  /* struct for return values of status of called modules */
  CSS_t_SOFT_ERR_RET_STAT s_retStat;

  /* make sure to start with defined values */
  CSS_MEMSET(&s_retStat, 0, sizeof(s_retStat));

  /* if passed pointer is invalid */
  if (pb_softErrCheckBuf == CSS_k_NULL)
  {
    SAPL_CssErrorClbk(IXSSC_k_FSE_AIP_PTR_INV_SEVG_V,
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  /* if static variable u8_ModuleCnt is invalid */
  else if (u8_ModuleCnt >= IXUTL_NUM_OF_ARRAY_ELEMENTS(kafp_ModuleVarGetList))
  {
    /* an invalid value in this variable would be fatal as it is used as the
       array index when calling a function from the kafp_ModuleVarGetList list
    */
    SAPL_CssErrorClbk(IXSSC_k_FSE_INC_MOD_CNT_SEVG_V,
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u8_ModuleCnt);
    u8_ModuleCnt = 0U;
  }/* CCT_SKIP */ /* unreachable code - defensive programming */
  else
  {
    /* if we shall start from the beginning again */
    if (o_start)
    {
      /* beginn at the first module again */
      u8_ModuleCnt = 0U;
      /* start at level zero and all counters zero */
      CSS_MEMSET(au32_SoftErrCnt, 0, sizeof(au32_SoftErrCnt));
    }
    else /* else: continuous call */
    {
      /* nothing to do */
    }

    u16_idx = 0U;
    /* fill the passed buffer variable by variable */
    do
    {
      /* call the function pointer of the current module list entry */
      (*kafp_ModuleVarGetList[u8_ModuleCnt])
                   (au32_SoftErrCnt,
                    &s_retStat,
                    &pb_softErrCheckBuf[u16_idx]);

      if (s_retStat.u8_incLvl >= CSS_k_SOFT_ERR_NUM_OF_LEVELS)
      {
        /* sub function returned invalid u8_incLvl */
        SAPL_CssErrorClbk(IXSSC_k_FSE_INC_INV_LVL_V_1,
                          IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);

        /* make sure this loop is left (prevent infinite loop) */
        u16_idx = CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE;
      }
      else
      {
        /* if called module indicated that end of current level is reached */
        if (s_retStat.u8_cpyLen == 0U)
        {
          o_retVal = SoftErrAdvanceToNextLevel(s_retStat.u8_incLvl);
        }
        else /* else: end of module not yet reached */
        {
          /* increment buffer index (amount of bytes written into the buffer */
          u16_idx = u16_idx + (CSS_t_UINT)s_retStat.u8_cpyLen;
          /* stay in current level and advance */
          au32_SoftErrCnt[s_retStat.u8_incLvl]++;
        }
      }
    }
    /* stop when there is no more space for at least an UDINT value */
    /* suppress lint warning: Constant expression evaluates to 0. This (4) is
       also a valid setting of CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE */
    while (    (u16_idx <= (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE
                            - CSOS_k_SIZEOF_UDINT)               /*lint !e778 */
               )
            && (!o_retVal)
          );

    /* if passed pointer is NULL */
    if (pu16_waster == CSS_k_NULL)
    {
      /* caller is not interested in diagnostic information */
    }
    else /* else: passed pointer is valid */
    {
      /* return value of internal variable counter (within current module) */
      *pu16_waster = CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE - u16_idx;
    }

    /* fill the remaining rest of the buffer (due to granularity of up to 32 bit
       values) with zero */
    while (u16_idx < CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE)
    {
      pb_softErrCheckBuf[u16_idx] = 0U;
      u16_idx++;
    }
  }

  /* if passed pointer is NULL */
  if (pu8_ModuleCnt == CSS_k_NULL)
  {
    /* caller is not interested in diagnostic information */
  }
  else /* else: passed pointer is valid */
  {
    /* return value of internal module counter */
    *pu8_ModuleCnt = u8_ModuleCnt;
  }

  /* if passed pointer is NULL */
  if (pu8_VarLvl == CSS_k_NULL)
  {
    /* caller is not interested in diagnostic information */
  }
  else /* else: passed pointer is valid */
  {
    /* return value of internal level counter (within current module) */
    *pu8_VarLvl = s_retStat.u8_incLvl;
  }

  /* if passed pointer is NULL */
  if (pu32_VarCnt == CSS_k_NULL)
  {
    /* caller is not interested in diagnostic information */
  }
  else /* else: passed pointer is valid */
  {
    /* return value of internal variable counter (within current module) */
    *pu32_VarCnt = au32_SoftErrCnt[s_retStat.u8_incLvl];
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : UnitsInit
**
** Description : This function calls the initialization functions of all CSS
**               units and thus initializes the whole stack.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
static void UnitsInit(void)
{
  /* Safety Control Flow Monitoring */
  IXSCF_Init();

  /* Utility functions */
  IXUTL_Init();

  /* Safety Validator Data */
  IXSVD_Init();

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  {
    /* Safety Validator Server */
    IXSVS_Init();
  }
#endif

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  {
    /* Safety Validator Client */
    IXSVC_Init();
  }
#endif

#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
  {
    /* Safety Assembly Object Interface */
    IXSAI_Init();
  }
#endif

  /* Safety Validator Object */
  IXSVO_Init();

  /* Safety Supervisor Object */
  IXSSO_Init();

#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  {
    /* Connection Configuration Object */
    IXCCO_Init();
  }
#endif

#if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_ENABLE)
  {
    /* Safety Process Image */
    IXSPI_Init();
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : InitInfoValidate
**
** Description : This function checks if the passed initialization information
**               is valid.
**
** Parameters  : ps_initInfo (IN) - pointer to structure of type
**                                  {CSS_t_ID_INFO} which must contain
**                                  information that is necessary for the stack
**                                  to operate.
**                                  (not checked, checked already in
**                                  IXSSC_Init())
**
** Returnvalue : CSS_k_OK         - validation was successful
**               <>CSS_k_OK       - validation raised an error
**
*******************************************************************************/
static CSS_t_WORD InitInfoValidate(const CSS_t_ID_INFO *ps_initInfo)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSSC_k_FSE_INC_PRG_FLOW;

  if (ps_initInfo->u16_vendorId == 0U)
  {
    /* invalid Vendor ID */
    w_retVal = IXSSC_k_FSE_AIP_INV_VENDOR_ID;
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)ps_initInfo->u16_vendorId);
  }
  /* device type cannot be checked - any value might be valid */
  else if (ps_initInfo->u16_prodCode == 0U)
  {
    /* invalid product code */
    w_retVal = IXSSC_k_FSE_AIP_INV_PROD_CODE;
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED,
                      (CSS_t_UDINT)ps_initInfo->u16_prodCode);
  }
  else if (    (ps_initInfo->u8_majorRev == 0U)
            || (ps_initInfo->u8_majorRev >= 0x80U)
            || (ps_initInfo->u8_minorRev == 0U)
          )
  {
    /* invalid revision */
    w_retVal = IXSSC_k_FSE_AIP_INV_REVISION;
    SAPL_CssErrorClbk(w_retVal, (CSS_t_UINT)ps_initInfo->u8_majorRev,
                      (CSS_t_UDINT)ps_initInfo->u8_minorRev);
  }
  /* serial number cannot be checked - any value might be valid */
  /* NodeID cannot be checked - any value might be valid */
  else
  {
    /* validation successful */
    w_retVal = CSS_k_OK;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : TerminateInfoToCsal
**
** Description : This function sends a command to CSAL that informs it about the
**               termination of CSS.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
static void TerminateInfoToCsal(void)
{
  /* HALC message to be transmitted to CSAL */
  HALCS_t_MSG s_halcTxMsg;

  /* completely erase structure */
  CSS_MEMSET(&s_halcTxMsg, 0, sizeof(s_halcTxMsg));

  /* prepare message */
  s_halcTxMsg.u16_cmd = CSOS_k_CMD_IXSSC_TERMINATE;
  s_halcTxMsg.u16_len = 0U;
  s_halcTxMsg.u32_addInfo = 0U;
  s_halcTxMsg.pb_data = CSS_k_NULL;

  /* if sending Device State to CSAL fails */
  if (!HALCS_TxDataPut(&s_halcTxMsg))
  {
    SAPL_CssErrorClbk((IXSSC_k_NFSE_TXH_HALC_COMM_TERM),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: no error */
  {
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : SoftErrAdvanceToNextLevel
**
** Description : This function is a sub function of IXSSC_SoftErrVarGet(). It is
**               called after a module's SoftErrVarGet function has indicated
**               that no data was copied (i.e. end of current level reached).
**               This function will then set static variables to advance
**               to the next module or next level.
**
** Parameters  : u8_incLvl (IN) - The incLvl that was returned by the recent
**                                call to a module's SoftErrVarGet function.
**                                (checked, valid range:
**                                0..(CSS_k_SOFT_ERR_NUM_OF_LEVELS-1))
**
** Returnvalue : CSS_k_TRUE     - The returned chunk is the last chunk of data.
**               CSS_k_FALSE    - The returned chunk is not yet the last chunk
**                                of data.
**
*******************************************************************************/
static CSS_t_BOOL SoftErrAdvanceToNextLevel(CSS_t_USINT u8_incLvl)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* next step depends on which level we currently are */
  switch (u8_incLvl)
  {
    /* highest level */
    case 0U:
    {
      /* module has no more data, advance to next module */
      u8_ModuleCnt++;

      /* if we have already processed the last module */
      if (u8_ModuleCnt
           >= IXUTL_NUM_OF_ARRAY_ELEMENTS(kafp_ModuleVarGetList))
      {
        /* beginn at the first module again */
        u8_ModuleCnt = 0U;
        /* indicate that the last chunk of data is returned */
        o_retVal = CSS_k_TRUE;
      }
      else /* else: arrived at another valid module */
      {
        /* nothing to do */
      }

      /* all counters zero for the next module */
      CSS_MEMSET(au32_SoftErrCnt, 0, sizeof(au32_SoftErrCnt));
      break;
    }

    case 1U:
    {
      /* increment counter of first level and start at zero of second */
      au32_SoftErrCnt[0U]++;
      au32_SoftErrCnt[1U] = 0U;
      au32_SoftErrCnt[2U] = 0U;
      au32_SoftErrCnt[3U] = 0U;
      break;
    }

    case 2U:
    {
      /* increment counter of second level and start at zero of third */
      /* level zero unchanged */
      au32_SoftErrCnt[1U]++;
      au32_SoftErrCnt[2U] = 0U;
      au32_SoftErrCnt[3U] = 0U;
      break;
    }

    case 3U:
    {
      /* increment counter of third level and start at zero of fourth */
      /* level zero unchanged */
      au32_SoftErrCnt[2U]++;
      au32_SoftErrCnt[3U] = 0U;
      break;
    }

    default:
    {
      /* sub function returned invalid u8_incLvl */
      SAPL_CssErrorClbk(IXSSC_k_FSE_INC_INV_LVL_V_2,
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      break;
    }/* CCT_SKIP */ /* unreachable code - defensive programming */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}
#endif  /* (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES) */


/*** End Of File ***/


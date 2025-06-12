/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLipc.c
**     Summary: This file implements the interface to provide access to IPC variables used by
**              SAPL.
**   $Revision: 2408 $
**       $Date: 2017-03-22 12:41:54 +0100 (Mi, 22 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPLipc_SetStackVarAndPath
**             SAPLipc_SetNvCrc
**             SAPLipc_CheckNvCrc
**             SAPLipc_SendVar
**             SAPLipc_RecvAndCheckVar
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

#include "globFit_FitTestHandler.h"

/* CSOS common headers */
#include "CSOScfg.h"

/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* CSS headers */
#include "IXSSCapi.h"
#include "IXSCFapi.h"

#include "SAPLipc.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      18u

/* Structure used for exchanging control flow path, static/global variables used
** for soft error check and CRC16 for NV memory handling.
*/
typedef __packed struct
{
  UINT16 u16_nvCrc;
  UINT16 u16_pathCnt;
  UINT8  u8_softErr;
} t_IPC_VAR;


/* s_VarLocal:
** This variable contains the current CSS control flow path, static/global 
** variables used for soft error check and the CRC16 for NV memory handling. 
** The data is sent to the other controller for supervision.
*/
STATIC t_IPC_VAR s_VarLocal;


/* s_VarRemote:
** This variable contains the received CSS control flow path, static/global 
** variables used for soft error check and the CRC16 for NV memory handling.  
** The data is received from the other controller and has to be compared with 
** local data.
*/
STATIC t_IPC_VAR s_VarRemote;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPLipc_SetStackVarAndPath

  Description:
    This function reads the CSS variables used for soft error supervision (see [SRS_2075], [3.2-6:])
    and the program flow information of the CSS (see [SRS_2077]).

    The variables are set inside IPC structure:
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)

    The set value is exchanged with the other controller instance. For this purpose the function
    SAPLipc_SendVar shall be called.

    ATTENTION: Shall not be called if CSS is running in background task!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SetStackVarAndPath(void)
{
  /* buffer for filling with data for Soft Error Checking */
  CSS_t_BYTE au8_softErrMemCheck[CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE];
  /* Control Flow Monitor Counter */
  CSS_t_UDINT u32_numOfFunctionCalls;

  /* read data for Soft Error Checking (see [SRS_2075], [3.2-6:]) */
  /* since we simple compare the returned bytes, it does not matter when all variables are read.
   * so the return value can be ignored here. */
  (void)IXSSC_SoftErrByteGet(CSS_k_FALSE,
                             au8_softErrMemCheck,
                             CSS_k_NULL,
                             CSS_k_NULL);

  /* read data of Control Flow Monitor (see [SRS_2077], [3.14-1:]) */
  u32_numOfFunctionCalls = IXSCF_PathGetReset();

  /* ensure path can be transferred in 16 bit variable */
  if (u32_numOfFunctionCalls <= (UINT32)UINT16_MAX)
  {
    /* copy data to SAPLipcVar in order to send via IPC during IRQ scheduler task */
    s_VarLocal.u16_pathCnt = (UINT16)u32_numOfFunctionCalls;
    s_VarLocal.u8_softErr  = au8_softErrMemCheck[0];
  }
  /* else: path metric to big, shall really not occur */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_CSS_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(1u));
  }
}

/***************************************************************************************************
  Function:
    SAPLipc_SetNvCrc

  Description:
    This function is used to set inside IPC structure:
      - the calculated CRC of the new non-volatile flash block.

    The set value is exchanged with the other controller instance. For this purpose the function
    SAPLipc_SendVar shall be called.

  See also:
    SAPLipc_CheckNvCrc

  Parameters:
    u16_nvCrc (IN)           - Calculated flash block CRC (NV CRC)
                               (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SetNvCrc(UINT16 u16_nvCrc)
{
  s_VarLocal.u16_nvCrc = u16_nvCrc;
}

/***************************************************************************************************
  Function:
    SAPLipc_CheckNvCrc

  Description:
    This function is used to compare the sent NV CRC with the one received via IPC.
    In case of mismatch, the failsafe state is entered.

    It is task of the calling function to ensure that the IPC transfer was done before this function
    is called.

  See also:
    SAPLipc_SetNvCrc

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_CheckNvCrc(void)
{
  /* if received NV CRC does not match to the calculated one */
  if (s_VarRemote.u16_nvCrc != s_VarLocal.u16_nvCrc)
  {
    /* call safety handler */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: NV CRC match */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    SAPLipc_SendVar

  Description:
    This function is used to send via IPC to other controller
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)
      - CRC16 used for NV memory handling
   
  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_SendVar(void)
{
  /* send data via IPC */
  /* Cast from pointer to pointer is intended here because the underlying function directly writes
  ** into the DMA buffer. */
  ipcxSYS_SendBuffer(IPCXSYS_IPC_ID_CSS_CTRL_INFO, 
                     (UINT8)sizeof(s_VarLocal), 
                     (UINT8*)&s_VarLocal); /*lint !e928*/
}

/***************************************************************************************************
  Function:
    SAPLipc_RecvAndCheckVar

  Description:
    This function is used to receive via IPC from other controller
      - number of function calls and significant program flow branches done in CSS since last call
        (16 bit value)
      - CSS stack internal variables which must be protected against Soft Errors (1 byte)

    The software shall compare the received values with the sent ones. In case of any differences,
    the failsafe state is entered (see [SRS_2076], [SRS_2078]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPLipc_RecvAndCheckVar(void)
{
  
  /* receive data via IPC */
  /* Cast from pointer to pointer is intended here because the underlying function directly reads
  ** from the DMA buffer. */
  ipcxSYS_GetBufferinclWait(IPCXSYS_IPC_ID_CSS_CTRL_INFO, 
                            (UINT8)sizeof(s_VarRemote), 
                            (UINT8*)&s_VarRemote); /*lint !e928 */
  
  /* FIT to manipulate the program flow counter*/
  /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
  GLOBFIT_FITTEST( GLOBFIT_CMD_PFLOW,
                   (s_VarRemote.u16_pathCnt += 1));
  /* RSM_IGNORE_END */
                            
  /* check CSS control flow path */
  /* if mismatch detected (->wrong program flow) */
  if (s_VarLocal.u16_pathCnt != s_VarRemote.u16_pathCnt)
  {
    /* enter global fail-safe state (see SRS_2078], [3.14-2:]) */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_CSS_PFLOW_ERR, GLOBFAIL_ADDINFO_FILE(3u));
  }
  /* else: control flow path OK */
  else
  {
    /* FIT to manipulate the soft error variable*/
    /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
    GLOBFIT_FITTEST( GLOBFIT_CMD_SOFT_ERROR,
                     (s_VarRemote.u8_softErr += 1));
    /* RSM_IGNORE_END */
    
    /* check static/global variables */
    /* if mismatch detected (->soft error) */
    if (s_VarLocal.u8_softErr != s_VarRemote.u8_softErr)
    {
      /* enter global fail-safe state (see SRS_2076]) */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_CSS_SOFTVAR_ERR, GLOBFAIL_ADDINFO_FILE(4u));
    }
    /* else: no mismatch detected */
    else
    {
      /* empty branch */
    }
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/



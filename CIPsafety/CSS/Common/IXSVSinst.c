/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSinst.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains utility functions for the implementation of
**             the Safety Validator Server functionality such as allocation,
**             stopping and closing.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_InstInitAll
**             IXSVS_InstInit
**             IXSVS_InstCloseAll
**             IXSVS_InstClose
**             IXSVS_InstStop
**             IXSVS_InstInfoGet
**             IXSVS_InstAlloc
**             IXSVS_InstFind
**             IXSVS_InstSoftErrByteGet
**             IXSVS_InstSoftErrVarGet
**
**             InstDealloc
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)

#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#include "IXSERapi.h"

#include "IXSSS.h"
#include "IXSVD.h"
#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVS.h"
#include "IXSVSint.h"
#include "IXSVSerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** ao_Allocated:
    array of flags indicating which instances are currently allocated
*/
static CSS_t_BOOL ao_Allocated[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void InstDealloc(CSS_t_UINT u16_svsIdx);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_InstInitAll
**
** Description : This function initializes all static variables of this module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstInitAll(void)
{
  CSS_t_UINT u16_svsIdx;

  for (u16_svsIdx = 0U; u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_svsIdx++)
  {
    ao_Allocated[u16_svsIdx] = CSS_k_FALSE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstInit
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the IXSVS
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_Init() and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstInit(CSS_t_UINT u16_svsIdx)
{
  /* erase local variables of other modules */
  IXSVS_InitInfoClear(u16_svsIdx);
  IXSVS_DataLocalsClear(u16_svsIdx);
  IXSVS_ConsLocalsClear(u16_svsIdx);
  IXSVS_TcooLocalsClear(u16_svsIdx);
  IXSVS_TcorrLocalsClear(u16_svsIdx);
  IXSVS_RecvLocalsClear(u16_svsIdx);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstCloseAll
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
void IXSVS_InstCloseAll(void)
{
  /* loop variable counting safety validator index */
  CSS_t_UINT u16_svsIdx = 0U;

  /* check all Safety Validator Server instances */
  for (u16_svsIdx = 0U; u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS; u16_svsIdx++)
  {
    /* if this index is allocated */
    if (ao_Allocated[u16_svsIdx])
    {
      /* close this instance - ignore return value */
      (void)IXSVS_InstClose(IXSVS_InstIdFromIdxGet(u16_svsIdx), 1U, CSS_k_TRUE);
    }
    else
    {
      /* this instance is not allocated */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstClose
**
** Description : This function closes the passed Safety Validator Server.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (not checked, checked in IXSVS_InstCloseAll()
**                                 and IXSVO_InstanceClose() and
**                                 SafetyOpenProc3())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in
**                                 IXSVS_StateUnderlayingComClose())
**               o_stop (IN)     - if CSS_k_TRUE the instance is stopped and the
**                                 event is reported to the SAPL
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
CSS_t_WORD IXSVS_InstClose(CSS_t_UINT u16_instId,
                           CSS_t_USINT u8_consNum,
                           CSS_t_BOOL o_stop)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVS_k_FSE_INC_PRG_FLOW);
  /* Safety Validator Array Index */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if instance is invalid */
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* connection is not open - so it can not be closed */
    w_retVal = CSS_k_OK;
  }
  else /* else: instance is valid */
  {
    w_retVal = IXSVS_StateUnderlayingComClose(u16_svsIdx, u8_consNum);

    /* if function parameter o_stop is set */
    if (o_stop)
    {
      /* now close the Safety Validator Instance */
      IXSVS_InstStop(u16_svsIdx);
    }
    else /* else: caller does not want to stop the connection */
    {
      /* do nothing */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVS_InstStop
**
** Description : This function stops/deletes the passed Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (checked, valid range:
**                                 0..CSOS_cfg_NUM_OF_SV_SERVERS)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstStop(CSS_t_UINT u16_svsIdx)
{
  if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* passed Instance ID is invalid */
    IXSVS_ErrorClbk(IXSVS_k_FSE_INC_IDX_IS_NUM, IXSER_k_I_NOT_USED,
                    (CSS_t_UDINT)u16_svsIdx);
  }
  else
  {
    /* if the connection is idle */
    if (IXSVS_StateGet(u16_svsIdx) == CSS_k_SV_STATE_IDLE)
    {
      /* the connection is not open - so no reason to execute the state
         machine */
    }
    else /* connection is in one of the other states */
    {
      /* execute the state machine (so that SAPL will be informed) */
      IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_SERVER_CLOSE);
    }

    /* mark instance as not allocated any more */
    InstDealloc(u16_svsIdx);
    /* stop the index/instance dependency */
    IXSVS_InstIdIdxStop(u16_svsIdx);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Server Object Instance (e.g. state, status,
**               type, etc.). See type definition of {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range:
**                                   1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS) and instance
**                                   must be a Safety Validator Server.
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_InstInfoGet(CSS_t_UINT u16_instId,
                       CSS_t_VALIDATOR_INFOS *ps_svoInfos)
{
  /* index variable for Safety Validator arrays */
  CSS_t_UINT u16_svsIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if pointer to output is invalid */
  if (ps_svoInfos == CSS_k_NULL)
  {
    IXSVS_ErrorClbk(IXSVS_k_FSE_AIP_PTR_INV_IIG,
                    u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: parameters are valid */
  {
    /* initialize output parameters */
    ps_svoInfos->u8_state             = CSS_k_SV_STATE_IDLE;
    ps_svoInfos->u8_type              = CSS_k_TYPE_SERVER | CSS_k_TYPE_SCAST;
    ps_svoInfos->u16_maxDataAge_128us = 0xFFFFU;
    ps_svoInfos->u16_cnxnPoint        = CSOS_k_INVALID_INSTANCE;

    /* if CIP Safety Stack is in an improper state */
    if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
    {
      /* CIP Safety Stack is not in running state */
      IXSVS_ErrorClbk(IXSVS_k_FSE_AIS_STATE_ERR_IIG,
                      u16_instId, IXSER_k_A_NOT_USED);
    }
    else /* else: CSS is in a proper state */
    {
      /* if instance is invalid */
      if (u16_svsIdx >= CSOS_cfg_NUM_OF_SV_SERVERS)
      {
        /* passed Instance ID is invalid */
        IXSVS_ErrorClbk(IXSVS_k_FSE_INC_INST_SIIG_NUM,
                        u16_instId, IXSER_k_A_NOT_USED);
      }
      else /* else: instance is valid */
      {
        /* return state */
        ps_svoInfos->u8_state = IXSVS_StateGet(u16_svsIdx);

        /* return max data age */
        ps_svoInfos->u16_maxDataAge_128us = IXSVS_MaxDataAgeGet(u16_svsIdx);

        /* return the connection point to which this SV server is connected */
        ps_svoInfos->u16_cnxnPoint = IXSVS_InitInfoCnxnPointGet(u16_svsIdx);

        /* return type */
        ps_svoInfos->u8_type = CSS_k_TYPE_SERVER;
        /* if this index is allocated */
        if (ao_Allocated[u16_svsIdx])
        {
          /* if is Single-cast */
          if (IXSVD_IsSingleCast(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
          {
            ps_svoInfos->u8_type |= CSS_k_TYPE_SCAST;
          }
          else /* else: then it must be Multi-cast */
          {
            ps_svoInfos->u8_type |= CSS_k_TYPE_MCAST;
          }
        }
        else /* else: not allocated */
        {
          /* unallocated = 0, but then there should be no valid u16_svsIdx
             => inconsistency */
          IXSVS_ErrorClbk(IXSVS_k_FSE_INC_SIIG_IDX_ALLOC,
                          u16_instId, IXSER_k_A_NOT_USED);
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_InstAlloc
**
** Description : This function allocates the first free Server Index of the
**               Safety Validator Object.
**
** Parameters  : -
**
** Returnvalue : >=CSOS_cfg_NUM_OF_SV_CLIENTS - error: it was not possible to
**                                              allocate an index
**               <CSOS_cfg_NUM_OF_SV_CLIENTS  - success: allocated Safety
**                                              Validator index
**
*******************************************************************************/
CSS_t_UINT IXSVS_InstAlloc(void)
{
  /* return value of this function (initialized with error) */
  CSS_t_UINT u16_allocIdx = CSOS_k_INVALID_IDX;
  /* Index for searching in Safety Validator Server Structure array */
  CSS_t_UINT u16_svsIdx = 0U;

  /* search for a free instance */
  do
  {
    /* if not yet allocated */
    if (ao_Allocated[u16_svsIdx] == CSS_k_FALSE)
    {
      /* we have found a free instance */
      IXSVS_InstInit(u16_svsIdx);
      /* mark this instance as allocated now */
      ao_Allocated[u16_svsIdx] = CSS_k_TRUE;
      /* make sure state attribute has a valid value */
      IXSVS_StateInit(u16_svsIdx);
      /* return allocated instance ID */
      u16_allocIdx = u16_svsIdx;
    }
    else /* this instance is already allocated */
    {
      /* search must continue */
    }

    u16_svsIdx++;
  }
  while (    (u16_allocIdx == CSOS_k_INVALID_IDX)
          && (u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS)
        );

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_allocIdx);
}



#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVS_InstFind
**
** Description : This function searches the Safety Validator Server Instance
**               which matches the passed Connection Triad.
**
** Parameters  : ps_cnxnTriad (IN) - pointer to the structure which contains
**                                   the description of the connection
**                                   (not checked, only called with reference to
**                                   structure)
**               ps_svInfo (OUT)   - pointer to the structure to where the found
**                                   Safety Validator Instance Type, Index, and
**                                   Consumer_Number will be written
**                                   (not checked, only called with reference to
**                                   structure)
**
** Returnvalue : CSS_k_TRUE        - connection was found (index in ps_cnxnIdx)
**               CSS_k_FALSE       - connection not found
**
*******************************************************************************/
CSS_t_BOOL IXSVS_InstFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo)
{
  /* flag to indicate that the connection that is to be closed was found */
  CSS_t_BOOL o_foundFlag = CSS_k_FALSE;
  /* index of Safety Validator Server structure */
  CSS_t_UINT u16_svsIdx = 0U;

  /* initialize output parameters */
  ps_svInfo->u16_instId    = CSOS_k_INVALID_INSTANCE;
  ps_svInfo->o_isServer    = CSS_k_TRUE;
  ps_svInfo->u8_consNum    = CSOS_k_CNUM_INVALID;
  ps_svInfo->u8_state      = CSS_k_SV_STATE_IDLE;
  ps_svInfo->u8_cnxnStatus = CSS_k_CNXN_STATUS_IDLE;
  ps_svInfo->o_multiProd   = CSS_k_FALSE;

  /* find the connection with this serial number in the Server Connections */
  do
  {
    /* if connection exists */
    if (ao_Allocated[u16_svsIdx])
    {
      /* get a pointer to the struct containing the PID of this instance */
      const IXSVS_t_INIT_CALC* const kps_pid =
        IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

      /* if connection serial number matches */
      if (ps_cnxnTriad->u16_cnxnSerNum == kps_pid->s_pid.u16_cnxnSerNum)
      {
        /* if Originator Vendor ID and Originator Serial Number match */
        if (    (ps_cnxnTriad->u16_origVendId == kps_pid->s_pid.u16_vendId)
             && (ps_cnxnTriad->u32_origSerNum == kps_pid->s_pid.u32_devSerNum)
           )
        {
          /* found matching connection */
          ps_svInfo->u16_instId    = IXSVS_InstIdFromIdxGet(u16_svsIdx);
          ps_svInfo->u8_consNum    = kps_pid->u8_Consumer_Num;
          ps_svInfo->u8_state      = IXSVS_StateGet(u16_svsIdx);
          ps_svInfo->u8_cnxnStatus = IXSVS_ConsCnxnStatusGet(u16_svsIdx);
          /* ps_svInfo->o_isServer already set */
          o_foundFlag = CSS_k_TRUE;
        }
        else
        {
          /* This request does not come from the originator which owns        */
          /* this connection.                                                 */
        }
      }
      else
      {
        /* connection serial number doesn't match, continue with next */
      }
    }
    else
    {
      /* this connection doesn't exist, continue with next */
    }

    u16_svsIdx++;
  }
  while ((!o_foundFlag) && (u16_svsIdx < CSOS_cfg_NUM_OF_SV_SERVERS));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_foundFlag);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_InstSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
CSS_t_UDINT IXSVS_InstSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(ao_Allocated))
  {
    *pb_var = *(((CSS_t_BYTE*)ao_Allocated)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(ao_Allocated);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVS_InstSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
/* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to lit. number   */
void IXSVS_InstSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_SERVERS)
  {
    /* level 0 counter at/above end */
    /* default return values already set */
  }
  else /* level 0 counter in range of array elements */
  {
    CSS_H2N_CPY8(pb_var, &ao_Allocated[au32_cnt[0U]]);
    ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BOOL;
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : InstDealloc
**
** Description : This function deallocates the addressed Safety Validator Object
**               by clearing its allocation bit.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked by callers)
**
** Returnvalue : -
**
*******************************************************************************/
static void InstDealloc(CSS_t_UINT u16_svsIdx)
{
  ao_Allocated[u16_svsIdx] = CSS_k_FALSE;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


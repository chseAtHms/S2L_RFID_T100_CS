/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCinst.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains utility functions for the implementation of
**             the Safety Validator Client functionality such as allocation,
**             stopping and closing.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_InstInitAll
**             IXSVC_InstInit
**             IXSVC_InstClose
**             IXSVC_InstCloseAll
**             IXSVC_InstConsumerStop
**             IXSVC_InstFind
**             IXSVC_InstInfoGet
**             IXSVC_InstAlloc
**             IXSVC_InstDealloc
**             IXSVC_InstSoftErrByteGet
**             IXSVC_InstSoftErrVarGet
**
**             InstProdCnxnStatusGet
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"

#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)

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

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#include "IXSVCerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** ao_Allocated:
    array of flags indicating which instances are currently allocated
*/
static CSS_t_BOOL ao_Allocated[CSOS_cfg_NUM_OF_SV_CLIENTS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  static CSS_t_USINT InstProdCnxnStatusGet(CSS_t_UINT u16_svcIdx,
                                           CSS_t_USINT u8_consIdx);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_InstInitAll
**
** Description : This function initializes all static variables of this module.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInitAll(void)
{
  CSS_t_UINT u16_svcIdx;

  for (u16_svcIdx = 0U; u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_svcIdx++)
  {
    ao_Allocated[u16_svcIdx] = CSS_k_FALSE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstInit
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the IXSVC
**               structures.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInit(CSS_t_UINT u16_svcIdx)
{
  /* erase local variables of other modules */
  IXSVC_InitInfoClear(u16_svcIdx);
  IXSVC_ProdLocalsClear(u16_svcIdx);
  IXSVC_TcooLocalsClear(u16_svcIdx);
  IXSVC_TcorrLocalsClear(u16_svcIdx);
  IXSVC_QuarLocalsClear(u16_svcIdx);
  IXSVC_InstInfoClear(u16_svcIdx);
  IXSVC_UtilLocalsClear(u16_svcIdx);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstClose
**
** Description : This function closes the passed Safety Validator Client.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID
**                                 (checked, IXSVD_SvIdxFromInstGet()
**                                 must return a valid index)
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in called function
**                                 IXSVC_StateUnderlayingComClose())
**               o_stop (IN)     - if CSS_k_TRUE the instance is stopped and the
**                                 event is reported to the SAPL
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error: connection not found
**
*******************************************************************************/
CSS_t_WORD IXSVC_InstClose(CSS_t_UINT u16_instId,
                           CSS_t_USINT u8_consNum,
                           CSS_t_BOOL o_stop)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* Safety Validator Array Index */
  CSS_t_UINT u16_svIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if passed instance Id is invalid */
  if (u16_svIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
  {
    /* connection is not open - so it can not be closed */
    w_retVal = CSS_k_OK;
  }
  else /* else: ok */
  {
    w_retVal = IXSVC_StateUnderlayingComClose(u16_svIdx, u8_consNum);

    /* errors of previous function are already reported to SAPL */
    /* continue with closing our part of the connection independently if it
       was possible to close the underlaying connection */

    /* if function parameter o_stop is set */
    if (o_stop)
    {
      /* if we stopped the last consumer */
      if (IXSVC_InstConsumerStop(u16_svIdx, u8_consNum))
      {
        /* if stopping the instance is successful */
        if (IXSVC_InstIdIdxStop(u16_svIdx))
        {
          IXSVC_InstDealloc(u16_svIdx);
        }
        else /* else: error while stopping the consumer */
        {
          /* error already reported to SAPL */
        }
      }
      else /* else: there are consumers remaining */
      {
        /* instance stays open */
      }
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
** Function    : IXSVC_InstCloseAll
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
void IXSVC_InstCloseAll(void)
{
  /* loop variable counting safety validator index client */
  CSS_t_UINT u16_svcIdx = 0U;

  /* loop variable counting consumers of a safety validator client index */
  CSS_t_USINT u8_cIdx = 0U;

  /* check all Safety Validator Client instances */
  for (u16_svcIdx = 0U; u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS; u16_svcIdx++)
  {
    /* if this instance is allocated */
    if (ao_Allocated[u16_svcIdx])
    {
      /* check every consumer */
      for (u8_cIdx = 0U;
           u8_cIdx < IXSVC_InitInfoMaxConsNumGet(u16_svcIdx);
           u8_cIdx++)
      {
        const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
          IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_cIdx);

        /* if this consumer is open */
        if (kps_iiperCons->o_Consumer_Open)
        {
          /* close this instance - ignore return value as this function doesn't
             have an own return value and because all errors in called function
             are already reported to SAPL */
          (void)IXSVC_InstClose(IXSVC_InstIdFromIdxGet(u16_svcIdx),
                                (CSS_t_USINT)(u8_cIdx + 1U), CSS_k_TRUE);
        }
        else /* this consumer is not allocated */
        {
          /* nothing to do */
        }
      }
    }
    else /* else: this instance is not allocated */
    {
      /* nothing to do */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstConsumerStop
**
** Description : This function closes the passed Consumer of the passed Safety
**               Validator Client and checks if there are further consumers of
**               this connection left.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in IXSVC_InstClose()
**                                 and IXSVO_InstanceStop())
**               u8_consNum (IN) - Consumer Number
**                                 (not checked, checked in IXSVC_InstClose()
**                                 and IXSVO_InstanceStop())
**
** Returnvalue : CSS_k_TRUE      - This function has just closed the last
**                                 remaining Consumer of this connection.
**               CSS_k_FALSE     - There are still Consumers left.
**
*******************************************************************************/
CSS_t_BOOL IXSVC_InstConsumerStop(CSS_t_UINT u16_svcIdx,
                                  CSS_t_USINT u8_consNum)
{
  /* functions return value */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* consumer index (needed for subsequent function calls) */
  CSS_t_USINT u8_consIdx = (CSS_t_USINT)(u8_consNum - 1U);

  /* if multicast */
  if (IXSVD_IsMultiCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
    /* if the connection is idle */
    if (IXSVC_StateGet(u16_svcIdx) == CSS_k_SV_STATE_IDLE)
    {
      /* the connection is not open - so no reason to execute the state
         machine */
    }
    else /* else: connection is in one of the other states */
    {
      /* update state machine */
      IXSVC_StateMachine(u16_svcIdx, u8_consNum,
                         IXSVC_k_VE_CONSUMER_LEAVE);
    }

    /* close the requested consumer */
    IXSVC_InstInfoConsumerClose(u16_svcIdx, u8_consIdx);
    /* Make sure this consumer returns to status "Idle" */
    IXSVC_ConsumerActiveIdleSet(u16_svcIdx, u8_consIdx, IXSVD_k_IDLE);
    IXSVC_ConnectionFaultFlagSet(u16_svcIdx, u8_consIdx, IXSVD_k_CNXN_OK);

    /* if no more consumers left */
    if (!IXSVC_ConsumerRemainingCheck(u16_svcIdx))
    {
      /* indicate that whole instance can be closed */
      o_retVal = CSS_k_TRUE;
    }
    else /* else: consumers left */
    {
      /* Don't close the instance (stop production) */
    }
  }
  else /* else: Single-cast */
  {
    /* The instance can definitely be closed */
    o_retVal = CSS_k_TRUE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVC_InstFind
**
** Description : This function searches the Safety Validator Client Instance
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
CSS_t_BOOL IXSVC_InstFind(const CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                          CSS_t_SV_ID *ps_svInfo)
{
  /* flag to indicate that the connection that is to be closed was found */
  CSS_t_BOOL o_foundFlag = CSS_k_FALSE;
  /* index of Safety Validator (Client or Server) structure */
  CSS_t_UINT u16_svcIdx = 0U;

  /* initialize output parameters */
  ps_svInfo->u16_instId    = CSOS_k_INVALID_INSTANCE;
  ps_svInfo->o_isServer    = CSS_k_FALSE;
  ps_svInfo->u8_consNum    = CSOS_k_CNUM_INVALID;
  ps_svInfo->u8_state      = CSS_k_SV_STATE_IDLE;
  ps_svInfo->u8_cnxnStatus = CSS_k_CNXN_STATUS_IDLE;
  ps_svInfo->o_multiProd   = CSS_k_FALSE;

  /* Let's try to find the connection with this serial number in the Client */
  /* Connections.                                                           */
  do
  {
    /* if connection exists */
    if (ao_Allocated[u16_svcIdx])
    {
      CSS_t_USINT u8_cIdx = 0U;
      /* check if this connection has a consumer with matching connection   */
      /* serial number                                                      */
      do
      {
       /* get a pointer to per consumer data */
       const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
         IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_cIdx);

        /* if connection to consumer is open */
        if (kps_iiperCons->o_Consumer_Open)
        {
          /* if connection serial number matches */
          if (ps_cnxnTriad->u16_cnxnSerNum ==
              kps_iiperCons->s_cid.u16_cnxnSerNum)
          {
            /* if Originator Vendor ID and Originator Serial Number match */
            if (    (ps_cnxnTriad->u16_origVendId ==
                     kps_iiperCons->s_cid.u16_vendId)
                 && (ps_cnxnTriad->u32_origSerNum ==
                     kps_iiperCons->s_cid.u32_devSerNum)
               )
            {
              /* found matching connection */
              ps_svInfo->u16_instId    = IXSVC_InstIdFromIdxGet(u16_svcIdx);
              ps_svInfo->u8_consNum    = (CSS_t_USINT)(u8_cIdx + 1U);
              ps_svInfo->u8_state      = IXSVC_StateGet(u16_svcIdx);
              ps_svInfo->u8_cnxnStatus = InstProdCnxnStatusGet(u16_svcIdx,
                                                               u8_cIdx);
              /* if this is a multicast SVC instance */
              if (IXSVD_IsMultiCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
              {
                /* then set the flag to indicate this */
                ps_svInfo->o_multiProd = CSS_k_TRUE;
              }
              else  /* else: singlecast SVC instance */
              {
                /* o_multiProd flag already initialized to CSS_k_FALSE */
              }

              /* ps_svInfo->o_isServer already set */
              o_foundFlag = CSS_k_TRUE;
            }
            else /* else: mismatch */
            {
              /* This request does not come from the originator which owns  */
              /* this connection.                                           */
            }
          }
          else /* connection serial number doesn't match */
          {
            /* continue with next */
          }
        }
        else /* else: consumer doesn't exist */
        {
          /* continue with next */
        }

        u8_cIdx++;
      }
      while (    (u8_cIdx < IXSVC_InitInfoMaxConsNumGet(u16_svcIdx))
              && (!o_foundFlag)
            );
    }
    else /* else: this connection doesn't exist */
    {
      /* continue with next */
    }

    u16_svcIdx++;
  }
  while ((u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS) && (!o_foundFlag));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_foundFlag);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVC_InstInfoGet
**
** Description : This API function returns information about the specified
**               Safety Validator Client Object Instance (e.g. state, status
**               type, etc.). See type definition of {CSS_t_VALIDATOR_INFOS}.
**
** See Also    : CSS_t_VALIDATOR_INFOS
**
** Parameters  : u16_instId (IN)   - instance ID of the Safety Validator Object
**                                   (checked, valid range:
**                                   1..(CSOS_cfg_NUM_OF_SV_CLIENTS+
**                                   CSOS_cfg_NUM_OF_SV_SERVERS) and instance
**                                   must be a Safety Validator Client.
**               ps_svoInfos (OUT) - pointer to a structure into which the
**                                   function stores the result
**                                   (checked - valid range: <> CSS_k_NULL)
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstInfoGet(CSS_t_UINT u16_instId,
                       CSS_t_VALIDATOR_INFOS *ps_svoInfos)
{
  /* index variable for Safety Validator Client or Server arrays */
  CSS_t_UINT u16_svcIdx = IXSVD_SvIdxFromInstGet(u16_instId);

  /* if pointer to output is invalid */
  if (ps_svoInfos == CSS_k_NULL)
  {
    IXSVC_ErrorClbk(IXSVC_k_FSE_AIP_PTR_INV_IIG,
                    u16_instId, IXSER_k_A_NOT_USED);
  }
  else /* else: parameters are valid */
  {
    /* initialize output parameters */
    ps_svoInfos->u8_state             = CSS_k_SV_STATE_IDLE;
    ps_svoInfos->u8_type              = CSS_k_TYPE_CLIENT | CSS_k_TYPE_SCAST;
    ps_svoInfos->u16_maxDataAge_128us = 0xFFFFU;
    ps_svoInfos->u16_cnxnPoint        = CSOS_k_INVALID_INSTANCE;

    /* if CIP Safety Stack is in an improper state */
    if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
    {
      /* CIP Safety Stack is not in running state */
      IXSVC_ErrorClbk(IXSVC_k_FSE_AIS_STATE_ERR_IIG,
                      u16_instId, IXSER_k_A_NOT_USED);
    }
    else
    {
      /* if passed instance Id is invalid */
      if (u16_svcIdx >= CSOS_cfg_NUM_OF_SV_CLIENTS)
      {
        /* passed Instance ID is invalid */
        IXSVC_ErrorClbk(IXSVC_k_FSE_INC_INST_CIIG_NUM,
                        u16_instId, IXSER_k_A_NOT_USED);
      }
      else /* else: ok */
      {
        /* return state */
        ps_svoInfos->u8_state = IXSVC_StateGet(u16_svcIdx);

        /* max data age exists only for Servers */
        ps_svoInfos->u16_maxDataAge_128us = 0U;

        /* return the connection point to which this SV client is connected */
        ps_svoInfos->u16_cnxnPoint = IXSVC_InitInfoCnxnPointGet(u16_svcIdx);

        /* return type */
        ps_svoInfos->u8_type = CSS_k_TYPE_CLIENT;
        /* if addressed instance is allocated */
        if (ao_Allocated[u16_svcIdx])
        {
          /* if singlecast */
          if (IXSVD_IsSingleCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
          {
            ps_svoInfos->u8_type |= CSS_k_TYPE_SCAST;
          }
          else /* else: multicast */
          {
            ps_svoInfos->u8_type |= CSS_k_TYPE_MCAST;
          }
        }
        else /* else: unallocated */
        {
          /* unallocated = 0, but then there should be no valid u16_svcIdx
             => inconsistency */
          IXSVC_ErrorClbk(IXSVC_k_FSE_INC_SIIG_IDX_ALLOC, u16_instId,
                          IXSER_k_A_NOT_USED);
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVC_InstAlloc
**
** Description : This function allocates a free Client instance number of the
**               Safety Validator Object. Dependent on the parameter
**               u16_reqSvInst this function either tries to allocate the passed
**               instance ID or tries to allocate the next free instance ID.
**
** Parameters  : -
**
** Returnvalue : >=CSOS_cfg_NUM_OF_SV_CLIENTS - error: it was not possible to
**                                              allocate an instance ID
**               <CSOS_cfg_NUM_OF_SV_CLIENTS  - success: allocated Safety
**                                              Validator instance ID
**
*******************************************************************************/
CSS_t_UINT IXSVC_InstAlloc(void)
{
  /* return value of this function (initialized with error) */
  CSS_t_UINT u16_allocIdx = CSOS_k_INVALID_IDX;
  /* Index for searching in Safety Validator Client Structure array */
  CSS_t_UINT u16_svcIdx = 0U;

  /* search for a free instance */
  do
  {
    /* if the currently tested instance is not yet allocated */
    if (ao_Allocated[u16_svcIdx] == CSS_k_FALSE)
    {
      IXSVC_InstInit(u16_svcIdx);
      /* we have found a free instance */
      /* mark this instance as allocated now */
      ao_Allocated[u16_svcIdx] = CSS_k_TRUE;
      /* make sure state attribute has a valid value */
      IXSVC_StateInit(u16_svcIdx);
      /* return allocated instance ID */
      u16_allocIdx = u16_svcIdx;
    }
    else /* else: already allocated */
    {
      /* search must continue */
    }

    u16_svcIdx++;
  }
  while (    (u16_allocIdx == CSOS_k_INVALID_IDX)
          && (u16_svcIdx < CSOS_cfg_NUM_OF_SV_CLIENTS)
        );

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_allocIdx);
}


/*******************************************************************************
**
** Function    : IXSVC_InstDealloc
**
** Description : This function deallocates the addressed Safety Validator Object
**               by clearing its allocation bit.
**
** Parameters  : u16_svcIdx (IN) - index of the Safety Validator Client instance
**                                 (not checked, checked in IXSVC_Init(),
**                                 IXSVC_OrigClientInit() and
**                                 IXSVO_InstanceStop())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_InstDealloc(CSS_t_UINT u16_svcIdx)
{
  ao_Allocated[u16_svcIdx] = CSS_k_FALSE;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVC_InstSoftErrByteGet
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
CSS_t_UDINT IXSVC_InstSoftErrByteGet(CSS_t_UDINT u32_varCnt,
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
** Function    : IXSVC_InstSoftErrVarGet
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
void IXSVC_InstSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* if level 0 counter is larger than number of array elements */
  if (au32_cnt[0U] >= CSOS_cfg_NUM_OF_SV_CLIENTS)
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

#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : InstProdCnxnStatusGet
**
** Description : This function returns the status of a producing connection
**               (i.e. Safety Validator Client) for a consumer.
**
** See Also    : CSS_k_CNXN_STATUS_xxx
**
** Parameters  : u16_svcIdx (IN)  - instance index of the Safety Validator
**                                  Client Instance
**                                  (not checked, checked in
**                                  IXSVC_InstInfoGet())
**               u8_consIdx (IN)  - index of the requested consumer
**                                  (not checked, checked in
**                                  IXSVC_InstInfoGet())
**
** Returnvalue : see {CSS_k_CNXN_STATUS_xxx}
**
*******************************************************************************/
static CSS_t_USINT InstProdCnxnStatusGet(CSS_t_UINT u16_svcIdx,
                                         CSS_t_USINT u8_consIdx)
{
  CSS_t_USINT u8_status = CSS_k_CNXN_STATUS_FAULTED;

  /* (see FRS217) */
  /* if connection is faulted */
  if (IXSVC_ConnectionIsFaulted(u16_svcIdx, u8_consIdx))
  {
    u8_status = CSS_k_CNXN_STATUS_FAULTED;
  }
  /* else: if connection is not active */
  else if (!IXSVC_ConsumerIsActive(u16_svcIdx, u8_consIdx))
  {
    /* then it must be idle */
    u8_status = CSS_k_CNXN_STATUS_IDLE;
  }
  else /* else: then it must be active */
  {
    u8_status = CSS_k_CNXN_STATUS_PROD_ACTIVE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u8_status);
}
#endif


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


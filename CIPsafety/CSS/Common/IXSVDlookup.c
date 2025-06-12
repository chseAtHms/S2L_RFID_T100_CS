/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVDlookup.c
**    Summary: IXSVD - Safety Validator Data
**             This module contains routines to look up the Safety Validator
**             Instance Id from the internal array index and vice versa.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVD_Init
**             IXSVD_NextFreeInstIdGet
**             IXSVD_SvIndexAssign
**             IXSVD_SvIndexDelete
**             IXSVD_InstIsServer
**             IXSVD_SvIdxFromInstGet
**             IXSVD_InitRcOffsetIncrementGet
**             IXSVD_LookupSoftErrByteGet
**             IXSVD_LookupSoftErrVarGet
**
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
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #include "CSSapi.h"
  #include "CSS.h"
#endif

#include "IXSERapi.h"
#include "IXSCF.h"
#include "IXUTL.h"

#include "IXSVD.h"
#include "IXSVDerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** t_INST_LOOKUP_TBL:
    Type needed to define a lookup table for the instance <-> index relation of
    the Safety Validators
*/
typedef struct
{
  CSS_t_UINT u16_idx;    /* index in the arrays of Validator Servers/Clients */
  CSS_t_BOOL o_isServer; /* CSS_k_TRUE for servers, CSS_k_FALSE -> clients */
} t_INST_LOOKUP_TBL;

/** as_InstLookupTbl:
    Array of structures containing the instance <-> index relation lookup table
*/
static t_INST_LOOKUP_TBL as_InstLookupTbl[CSOS_cfg_NUM_OF_SV_SERVERS
                                          +CSOS_cfg_NUM_OF_SV_CLIENTS];

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /** u16_Initial_RC_Offset:
      Initialization Offset for the Initial_Rollover_Value in SafetyOpen
      Requests/Responses.
      This is a device global value to make sure that multiple connections don't
      use the same starting value for the Initial_Rollover_Count. This is
      considered a random number for this purpose.
  */
  static CSS_t_UINT u16_Initial_RC_Offset;
#endif

/* size of the above variables for easing Soft Error checking */
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #define k_LOOKUP_SIZEOF_VAR1   (sizeof(as_InstLookupTbl))
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    #define k_LOOKUP_SIZEOF_VAR12  (k_LOOKUP_SIZEOF_VAR1 +         \
                                    sizeof(u16_Initial_RC_Offset))
  #else
    #define k_LOOKUP_SIZEOF_VAR12  k_LOOKUP_SIZEOF_VAR1 + 0U
  #endif
#endif


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVD_Init
**
** Description : This function initializes the IXSVD unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVD_Init(void)
{
  /* loop variable for running through the Instance lookup table */
  CSS_t_UINT u16_id;

  /* make sure the whole array of structures is initialized (also padding bytes)
     because of Soft Error Checking) */
  CSS_MEMSET(as_InstLookupTbl, 0, sizeof(as_InstLookupTbl));

  /* initialize lookup table for Safety Validator Instance - Index relation */
  for (u16_id=0U;
       u16_id < IXUTL_NUM_OF_ARRAY_ELEMENTS(as_InstLookupTbl);
       u16_id++)
  {
    as_InstLookupTbl[u16_id].o_isServer = CSS_k_FALSE;
    as_InstLookupTbl[u16_id].u16_idx = CSOS_k_INVALID_IDX;
  }

#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  {
    u16_Initial_RC_Offset = 0U;
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVD_NextFreeInstIdGet
**
** Description : This function searches for the next free Safety Validator
**               Object instance ID number - based on the passed offset.
**
** Parameters  : u16_instIdOffset (IN) - offset for the instance Id.
**                                       Search begins upwards of this value.
**                                       (not checked, only called with
**                                       constants, valid range:
**                                       1..(CSOS_cfg_NUM_OF_SV_SERVERS
**                                          +CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue :   CSOS_k_INVALID_INSTANCE - no free Instance ID found
**               <>CSOS_k_INVALID_INSTANCE - next free Safety Validator Instance
**                                           ID
**
*******************************************************************************/
CSS_t_UINT IXSVD_NextFreeInstIdGet(CSS_t_UINT u16_instIdOffset)
{
  /* found instance ID value */
  CSS_t_UINT u16_svInstId = CSOS_k_INVALID_INSTANCE;
  /* loop index */
  CSS_t_UINT u16_idx = (CSS_t_UINT)(u16_instIdOffset - 1U);

  /* check the relevant entries of the lookup table */
  do
  {
    /* check if this instance is already in use */
    if (as_InstLookupTbl[u16_idx].u16_idx == CSOS_k_INVALID_IDX)
    {
      /* found an unused instance */
      u16_svInstId = (CSS_t_UINT)(u16_idx + 1U);
    }
    else
    {
      /* instance is already in use */
    }

    u16_idx++;
  }
  /* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
  /* The following "while" was implemented in one line to avoid a wrong code
     coverage result of the gcov tool */
  while ((u16_svInstId == CSOS_k_INVALID_INSTANCE) && (u16_idx < IXUTL_NUM_OF_ARRAY_ELEMENTS(as_InstLookupTbl)));
  /* RSM_IGNORE_QUALITY_END */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_svInstId);
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**
** Function    : IXSVD_SvIndexAssign
**
** Description : This function assigns an Index of a Safety Validator
**               Client/Server to the passed Instance ID
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID number
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**               u16_idx (IN)    - index in the array of Safety Validator
**                                 clients/servers
**                                 (checked, valid range:
**                                 0..(CSOS_cfg_NUM_OF_SV_SERVERS-1)  or
**                                 0..(CSOS_cfg_NUM_OF_SV_CLIENTS-1))
**               o_isServer (IN) - must be CSS_k_TRUE for servers,
**                                 CSS_k_FALSE for clients.
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE      - assignment successful
**               CSS_k_FALSE     - error: passed parameters invalid
**
*******************************************************************************/
CSS_t_BOOL IXSVD_SvIndexAssign(CSS_t_UINT u16_instId,
                               CSS_t_UINT u16_idx,
                               CSS_t_BOOL o_isServer)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if passed parameters are invalid */
  if (    (u16_instId == 0U)
       || (u16_instId > (CSOS_cfg_NUM_OF_SV_SERVERS+CSOS_cfg_NUM_OF_SV_CLIENTS))
#if  (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)                  /* CCT_NO_PRE_WARNING */
       || (o_isServer && (u16_idx >= CSOS_cfg_NUM_OF_SV_SERVERS))
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)                   /* CCT_NO_PRE_WARNING */
       || (!o_isServer && (u16_idx >= CSOS_cfg_NUM_OF_SV_CLIENTS))
#endif
     )
  {
    /* error: return value already set */
    SAPL_CssErrorClbk(IXSVD_k_FSE_INC_IDX_MISM_ASSIGN, u16_instId,
                      (CSS_t_UDINT)u16_idx);
  }
  else
  {
    /* set member with passed values */
    as_InstLookupTbl[u16_instId - 1U].o_isServer = o_isServer;
    as_InstLookupTbl[u16_instId - 1U].u16_idx = u16_idx;
    o_retVal = CSS_k_TRUE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


/*******************************************************************************
**
** Function    : IXSVD_SvIndexDelete
**
** Description : This function deletes an entry in the Instance <-> index lookup
**               table.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID number to be
**                                 deleted.
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue : CSS_k_TRUE      - deleting successful
**               CSS_k_FALSE     - error: passed parameter invalid
**
*******************************************************************************/
CSS_t_BOOL IXSVD_SvIndexDelete(CSS_t_UINT u16_instId)
{
  /* return value of this function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;

  /* if passed parameters are invalid */
  if (    (u16_instId == 0U)
       || (u16_instId > (CSOS_cfg_NUM_OF_SV_SERVERS+CSOS_cfg_NUM_OF_SV_CLIENTS))
     )
  {
    /* error: return value already set */
    SAPL_CssErrorClbk(IXSVD_k_FSE_INC_IDX_MISM_DELETE, u16_instId,
                      IXSER_k_A_NOT_USED);
  }
  else
  {
    /* set members back to initial values */
    as_InstLookupTbl[u16_instId - 1U].o_isServer = CSS_k_FALSE;
    as_InstLookupTbl[u16_instId - 1U].u16_idx = CSOS_k_INVALID_IDX;
    o_retVal = CSS_k_TRUE;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (o_retVal);
}


/*******************************************************************************
**
** Function    : IXSVD_InstIsServer
**
** Description : This function returns CSS_k_TRUE if the passed instance ID is
**               a Safety Validator Server Instance (CSS_k_FALSE if it is a
**               Client)
**
** Parameters  : u16_instId (IN) - instance ID of the Safety Validator Server or
**                                 Client
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - instance ID is a Safety Validator Server
**               CSS_k_FALSE     - instance ID is a Safety Validator Client
**
*******************************************************************************/
CSS_t_BOOL IXSVD_InstIsServer(CSS_t_UINT u16_instId)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_InstLookupTbl[u16_instId - 1U].o_isServer);
}


/*******************************************************************************
**
** Function    : IXSVD_SvIdxFromInstGet
**
** Description : This function converts a Safety Validator Instance ID into
**               the corresponding array index (Server or Client).
**
** Parameters  : u16_instId (IN) - instance ID of the Safety Validator
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue : 0..(CSOS_cfg_NUM_OF_SV_CLIENTS-1) - array index of the Safety
**               or                                  Validator Client array
**             : 0..(CSOS_cfg_NUM_OF_SV_SERVERS-1) - array index of the Safety
**               or                                  Validator Server array
**               CSOS_k_INVALID_IDX                - the passed Instance ID is
**                                                   not existing
**
*******************************************************************************/
CSS_t_UINT IXSVD_SvIdxFromInstGet(CSS_t_UINT u16_instId)
{
  /* return value of this function */
  CSS_t_UINT u16_retVal = CSOS_k_INVALID_IDX;

  /* if passed instance id is invalid */
  if (    (u16_instId == 0U)
       || (u16_instId > (CSOS_cfg_NUM_OF_SV_SERVERS+CSOS_cfg_NUM_OF_SV_CLIENTS))
     )
  {
    /* return value already set */
  }
  else
  {
    /* get index from lookup table */
    u16_retVal = as_InstLookupTbl[u16_instId - 1U].u16_idx;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_retVal);
}


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVD_InitRcOffsetIncrementGet
**
** Description : This function increments the value of the device global
**               Initial_RC_Offset and then returns its value. The
**               Initial_RC_Offset is part of the initialization value of the
**               Initial_Rollover_Value in SafetyOpen Requests/Responses.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Initial_RC_Offset
**
*******************************************************************************/
CSS_t_UINT IXSVD_InitRcOffsetIncrementGet(void)
{
  u16_Initial_RC_Offset++;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_Initial_RC_Offset);
}
#endif


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVD_LookupSoftErrByteGet
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
CSS_t_UDINT IXSVD_LookupSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                       CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < k_LOOKUP_SIZEOF_VAR1)
  {
    *pb_var = *(((CSS_t_BYTE*)as_InstLookupTbl)+u32_varCnt);
  }
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  /* else: if counter indicates we are in the range of the second variable */
  else if (u32_varCnt < (k_LOOKUP_SIZEOF_VAR12))
  {
    *pb_var = *(((CSS_t_BYTE*)&u16_Initial_RC_Offset)
                              +(u32_varCnt - k_LOOKUP_SIZEOF_VAR1));
  }
#endif
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = k_LOOKUP_SIZEOF_VAR12;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVD_LookupSoftErrVarGet
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
void IXSVD_LookupSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var)
{
  /* default initialization of return values */
  ps_retStat->u8_cpyLen = 0U;
  ps_retStat->u8_incLvl = 0U;

  /* select the level 0 variable */
  switch (au32_cnt[0U])
  {
    case 0U:
    {
      /* indicate to caller that we are in range of level 1 */
      ps_retStat->u8_incLvl = 1U;

      /* if level 1 counter is larger than number of array elements */
      if (au32_cnt[1U] >= (CSOS_cfg_NUM_OF_SV_SERVERS + CSOS_cfg_NUM_OF_SV_CLIENTS))
      {
        /* level 1 counter at/above end */
        /* default return values already set */
      }
      else /* level 1 counter in range of array elements */
      {
        /* indicate to caller that we are in range of level 2 or below */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
          case 0U:
          {
            CSS_H2N_CPY16(pb_var, &as_InstLookupTbl[au32_cnt[1U]].u16_idx);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case 1U:
          {
            CSS_H2N_CPY8(pb_var, &as_InstLookupTbl[au32_cnt[1U]].o_isServer);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BOOL;
            break;
          }

          default:
          {
            /* level 2 counter at/above end */
            /* default return values already set */
          }
          break;
        }
      }
      break;
    }

  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    case 1U:
    {
      CSS_H2N_CPY16(pb_var, &u16_Initial_RC_Offset);
      ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
      break;
    }
  #endif

    default:
    {
      /* level 0 counter at/above end */
      /* default return values already set */
      break;
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/


/*** End Of File ***/


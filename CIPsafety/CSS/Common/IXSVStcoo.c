/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVStcoo.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains the functions to generate a Time
**             Coordination message.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_TcooLocalsClear
**             IXSVS_TcooColdStartLogic
**             IXSVS_TcooPingCountCheck
**             IXSVS_TcooTimeToSend
**             IXSVS_TcooSoftErrByteGet
**             IXSVS_TcooSoftErrVarGet
**
**             TcooSend
**             TcooMsgProd
**             TcooMsgTimeoutStart
**             TcooTxCopy
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
#include "HALCSapi.h"

#include "IXSERapi.h"
#include "IXUTL.h"
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSFA.h"
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

/* constants for initialization of several variables */
#define k_INIT_LAST_PING_COUNT    3U   /* Volume 5: binary 11 */

/** t_SVS_LOCALS_TCOO:
    type for the local variables of this module grouped into a struct
*/
typedef struct
{
  /* scheduled system time before which the next time coordination must be sent
     to adhere to the 5 seconds after ping count change rule. */
  CSS_t_UDINT       u32_tcooTimer_128us;

  /* structure for storing the data of the Time Coordination message */
  CSS_t_TCOO_MSG    s_tcooMsg;

  /* Last_Ping_Count (see Vol. 5 Chapt. 2-4.6.4.2): for the consumer being able
     to detect a change of Ping_Count.
     Legal range: 0..3 */
  CSS_t_USINT       u8_Last_Ping_Count;

  /* detects first reception, then Time Correction Message must be produced */
  CSS_t_BYTE        b_FirstReception;

  /* Time_Coordination_Count_Down (see Vol. 5 Chapt. 2-4.6.4.3): used to detect
     Multi-cast consumer has to produce a ping response based on its
     Consumer_Number.
     Legal range: 1..15 */
  CSS_t_USINT       u8_Time_Coordination_Count_Down;

  /* Flag indicating that it is time to send a Time Coordination message */
  CSS_t_BOOL        o_tcooTimeToSend;

} t_SVS_LOCALS_TCOO;

/* array of structs to store the local variables for each instance */
static t_SVS_LOCALS_TCOO as_SvsLocals[CSOS_cfg_NUM_OF_SV_SERVERS];


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  /* define IDs of the variables depending on which variables are supported */
  enum
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
     k_SOFT_ERR_OPT_VAR_TCOO_CRCS5,
  #endif
     k_SOFT_ERR_VAR_CTV,
     k_SOFT_ERR_VAR_AB
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    ,k_SOFT_ERR_OPT_VAR_AB2
    ,k_SOFT_ERR_OPT_VAR_TCOO_CRCS3
  #endif
  };
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static void TcooSend(CSS_t_UINT u16_svsIdx);
static void TcooMsgProd(CSS_t_UINT u16_svsIdx);
static void TcooMsgTimeoutStart(CSS_t_UINT u16_svsIdx);
static void TcooTxCopy(CSS_t_UINT u16_svsIdx,
                       CSS_t_BYTE *pb_data);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_TcooLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in IXSVS_Init(),
**                                  SafetyOpenProc3())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooLocalsClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Safety Validator structure */
  CSS_MEMSET(&as_SvsLocals[u16_svsIdx], 0, sizeof(as_SvsLocals[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TcooColdStartLogic
**
** Description : This function is executed at the transition of the consuming
**               connection from closed to open and initializes Time
**               Coordination specific Variables of the Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                (not checked, checked in
**                                IXSVS_TargServerInit(),
**                                IXSVS_OrigServerInit() and IXCCO_CnxnOpen())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooColdStartLogic(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCOO *const ps_svs = &as_SvsLocals[u16_svsIdx];

  /* Time_Coordination_Count_Down = 0x0000, */  /* (see FRS297) */
  ps_svs->u8_Time_Coordination_Count_Down = 0U;

  /* Last_Ping_Count = 3, */  /* (see FRS292) */
  ps_svs->u8_Last_Ping_Count = k_INIT_LAST_PING_COUNT;

  /* safety validator server is now waiting for its first data reception */
  ps_svs->b_FirstReception = IXSVD_k_FR_FIRST_REC;

  /* don't send a Time Coordination Message immediately */
  ps_svs->o_tcooTimeToSend = CSS_k_FALSE;
  ps_svs->u32_tcooTimer_128us = 0UL;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TcooPingCountCheck
**
** Description : This function is called by the consumer safety data reception
**               logic to handle ping count checking.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooPingCountCheck(CSS_t_UINT u16_svsIdx)
{
  /*
  //////////////////////////////////////////////////////////////////////////////
  // Start of ping count check function
  //////////////////////////////////////////////////////////////////////////////
  ping_count_check_in_consumer_function()
  */
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCOO *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* mode byte of the safety validator server */
  const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);
  /* get a pointer to the struct containing the Consumer_Num of this instance */
  const IXSVS_t_INIT_CALC* const kps_cnum =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);


  /*
  // Check if its time to send a Time Coordination Message
  // Multi-cast connections send a Time Coordination Message
  // (Consumer_Num - 1) receptions after the ping count change.
  IF (received Mode_Byte.Ping_Count != Last_Ping_Count),
  THEN
  */
  /* (see FRS142 and FRS291) */
  if (IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_PING_COUNT)
      != ps_svs->u8_Last_Ping_Count)
  {
    /* (see FRS71 and FRS294) */
    /* Time_Coordination_Count_Down = Consumer_Num, */
    ps_svs->u8_Time_Coordination_Count_Down = kps_cnum->u8_Consumer_Num;

    /* Increment Time_Correction_Ping_Interval_Count, */
    IXSVS_TcorrPingIntCountIncrement(u16_svsIdx);

    /* Start timer that ensures that the Time Coordination message is sent    */
    /* within the next 5 seconds.                                             */
    TcooMsgTimeoutStart(u16_svsIdx);
  }
  else
  {
    /* ping_count didn't change */
  }
  /* ENDIF */

  /*
  // For multicast-consumers connecting to an existing producer, send
  // the Time_Coordination message immediately upon the 1st reception.
  IF (It is the first data reception),
  THEN
  */
  if (ps_svs->b_FirstReception == IXSVD_k_FR_FIRST_REC)
  {
    /* Produce Time_Coordination_Message, */
    TcooMsgProd(u16_svsIdx);  /* (see FRS148) */
    /* Time_Coordination_Count_Down = 0, */
    ps_svs->u8_Time_Coordination_Count_Down = 0U;

    /* Reset flag because the first reception is now handled */
    ps_svs->b_FirstReception = IXSVD_k_FR_CONT_REC;

    IXSVS_StateMachine(u16_svsIdx, IXSVS_k_VE_1ST_DATA_REC);

    /* Start timer that ensures that the Time Coordination message is sent    */
    /* within the next 5 seconds.                                             */
    TcooMsgTimeoutStart(u16_svsIdx);
  }
  else
  {
    /* connection has already received data before */
  }
  /* ENDIF */

  /*
  // Check if a ping count change has been detected and the Time
  // Coordination message has not been sent yet.
  IF (Time_Coordination_Count_Down > 0),
  THEN
  */
  if (ps_svs->u8_Time_Coordination_Count_Down > 0U)
  {
    /* decrement Time_Coordination_Count_Down, */
    ps_svs->u8_Time_Coordination_Count_Down--;/* (see FRS295) */

    /*
    // Send the Time_Coordination_message if the decremented
    // Time_Coordination_Count_Down transitions to 0
    IF (Time_Coordination_Count_Down == 0),
    THEN
    */
    /* (see FRS296) */
    if (ps_svs->u8_Time_Coordination_Count_Down == 0U)
    {
      /* Produce Time_Coordination_Message, */
      /* Respond to a ping request (see FRS17 and FRS25 and FRS48) */
      TcooMsgProd(u16_svsIdx);
    }
    else
    {
      /* not yet time to produce a time coordination message */
    }
    /* ENDIF */
  }
  else
  {
    /* time coordination message has already been sent */
  }
  /* ENDIF */

  /*
  // Save ping count for next reception check
  Last_Ping_Count = Mode_Byte.Ping_Count,
  */
  ps_svs->u8_Last_Ping_Count =
    (CSS_t_USINT)IXUTL_BIT_GET_U8(b_modeByte, IXSVD_k_MB_PING_COUNT);

  /*
  end ping_count_check_in_consumer_function()
  //////////////////////////////////////////////////////////////////////////////
  // end of ping count check function
  //////////////////////////////////////////////////////////////////////////////
  */

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_TcooTimeToSend
**
** Description : This function checks if it is time to send a Time Coordination
**               message by checking the corresponding flag of the Safety
**               Validator Server. It also clears the flag.
**               This function also checks if the timeout for sending a Time
**               Coordination message after a change of the Ping_Count expired.
**               This is to ensure that the Time Coordination message is sent no
**               later than 5 seconds after Ping_Count change. In case of
**               timeout it sends a time coordination message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_TcooTimeToSend(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local variables of safety validator server structure */
  t_SVS_LOCALS_TCOO *const ps_svs = &as_SvsLocals[u16_svsIdx];

  /* if in current Ping_Interval the Time Coordination is not yet sent */
  if (ps_svs->u8_Time_Coordination_Count_Down > 0U)
  {
    /* if it's time to produce a time coordination message within 5 seconds   */
    /* after ping count change (see FRS332).                                  */
    if (IXUTL_IsTimeGreaterOrEqual(IXUTL_GetSysTime(),
                                   ps_svs->u32_tcooTimer_128us))
    {
      /* Produce Time_Coordination_Message */
      TcooMsgProd(u16_svsIdx);

      /* Indicate that Time Coordination Msg for current ping interval has    */
      /* been sent.                                                           */
      ps_svs->u8_Time_Coordination_Count_Down = 0U;
    }
    else
    {
      /* not yet a timeout */
    }
  }
  else
  {
    /* Time Coordination Msg in current Ping_Interval has already been sent */
  }

  /* if a Time Coordination Msg must be sent */
  if (ps_svs->o_tcooTimeToSend)
  {
    /* message will be sent (see return value) - already clear the flag */
    ps_svs->o_tcooTimeToSend = CSS_k_FALSE;

    /* Send the Time Coordination Message */
    TcooSend(u16_svsIdx);
  }
  else
  {
    /* no need to send a Time Coordination Msg */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
/* This function only has a high cyclomatic complexity because of white box
   test code */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28           - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */



#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
/*******************************************************************************
**
** Function    : IXSVS_TcooSoftErrByteGet
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
CSS_t_UDINT IXSVS_TcooSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                     CSS_t_BYTE *pb_var)
{
  /* return value of this function */
  CSS_t_UDINT u32_retVal = 0U;  /* init to default: end not reached */

  /* if counter indicates we are in the range of the first variable */
  if (u32_varCnt < sizeof(as_SvsLocals))
  {
    *pb_var = *(((CSS_t_BYTE*)as_SvsLocals)+u32_varCnt);
  }
  else /* else: counter is already beyond the end */
  {
    /* indicate in return value that the end is reached */
    u32_retVal = sizeof(as_SvsLocals);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u32_retVal);
}


#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
/*******************************************************************************
**
** Function    : IXSVS_TcooSoftErrVarGet
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
void IXSVS_TcooSoftErrVarGet(
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
    /* indicate to caller that we are in range of level 1 or below */
    ps_retStat->u8_incLvl = 1U;

    /* select the level 1 variable */
    switch (au32_cnt[1U])
    {
      case 0U:
      {
        CSS_H2N_CPY32(pb_var, &as_SvsLocals[au32_cnt[0U]].u32_tcooTimer_128us);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
        break;
      }

      case 1U:
      {
        /* indicate to caller that we are in range of level 2 */
        ps_retStat->u8_incLvl = 2U;

        /* select the level 2 variable */
        switch (au32_cnt[2U])
        {
        #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_TCOO_CRCS5:
          {
            CSS_H2N_CPY32(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcooMsg.u32_crcS5);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UDINT;
            break;
          }
        #endif

          case k_SOFT_ERR_VAR_CTV:
          {
            CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcooMsg.u16_consTimeVal_128us);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }

          case k_SOFT_ERR_VAR_AB:
          {
            CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcooMsg.b_ackByte);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            break;
          }

        #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
          case k_SOFT_ERR_OPT_VAR_AB2:
          {
            CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcooMsg.b_ackByte2);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
            break;
          }

          case k_SOFT_ERR_OPT_VAR_TCOO_CRCS3:
          {
            CSS_H2N_CPY16(pb_var, &as_SvsLocals[au32_cnt[0U]].s_tcooMsg.u16_crcS3);
            ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_UINT;
            break;
          }
        #endif

          default:
          {
            /* level 2 counter at/above end */
            ps_retStat->u8_cpyLen = 0U;
            break;
          }
        }
        break;
      }

      case 2U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].u8_Last_Ping_Count);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 3U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].b_FirstReception);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BYTE;
        break;
      }

      case 4U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].u8_Time_Coordination_Count_Down);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_USINT;
        break;
      }

      case 5U:
      {
        CSS_H2N_CPY8(pb_var, &as_SvsLocals[au32_cnt[0U]].o_tcooTimeToSend);
        ps_retStat->u8_cpyLen = CSOS_k_SIZEOF_BOOL;
        break;
      }

      default:
      {
        /* level 1 counter at/above end */
        ps_retStat->u8_cpyLen = 0U;
        break;
      }
    }
  }
}
/* RSM_IGNORE_QUALITY_END */
#endif  /* CSS_cfg_SOFT_ERR_METHOD */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : TcooSend
**
** Description : This function generates a Time Coordination message and sends
**               it.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooSend(CSS_t_UINT u16_svsIdx)
{
  /* buffer for storing the output stream */
  CSS_t_BYTE ab_tcooMsg[CSOS_k_IO_MSGLEN_TCOO];
  /* Safety Validator Server Instance ID */
  CSS_t_UINT u16_svsInstId = IXSVS_InstIdFromIdxGet(u16_svsIdx);
  /* structure for passing the Tx Frame to HALCS */
  HALCS_t_MSG  s_halcTxMsg;

  /* completely erase structure */
  CSS_MEMSET(&s_halcTxMsg, 0, sizeof(s_halcTxMsg));

  /* convert produced message to byte stream */
  TcooTxCopy(u16_svsIdx, ab_tcooMsg);

  s_halcTxMsg.u16_cmd = CSOS_k_CMD_IXSVO_IO_DATA;
  s_halcTxMsg.u16_len = CSOS_k_IO_MSGLEN_TCOO;       /* fix length = 6 */
  s_halcTxMsg.u32_addInfo = (CSS_t_UDINT)u16_svsInstId;
  s_halcTxMsg.pb_data = ab_tcooMsg;

  /* Send message considering mixing data ob both Safety Controllers */
  /* if sending the message to HALCS succeeds */
  if (HALCS_TxDataPutMix(&s_halcTxMsg))
  {
    /* success */
  }
  else /* else: error while trying to send the message to CSAL */
  {
    IXSVS_ErrorClbk(IXSVS_k_NFSE_TXH_HALCS_SEND_STS,
                    u16_svsInstId, IXSER_k_A_NOT_USED);
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : TcooMsgProd
**
** Description : This function generates all the data fields of a Time
**               Coordination message in the appropriate format (Base or
**               Extended). The resulting message data is stored in the
**               structure as_SvsLocals[u16_svsIdx].s_tcooMsg.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooMsgProd(CSS_t_UINT u16_svsIdx)
{
  /* pointer to local data of Safety Validator Server data structure */
  t_SVS_LOCALS_TCOO *const ps_svs = &as_SvsLocals[u16_svsIdx];
  /* pointer to the CRC seeds of this instance */
  const IXSVS_t_INIT_CALC* const kps_crcSeeds =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /*
  //////////////////////////////////////////////////////////////////
  //Time Coordination message production processing
  //////////////////////////////////////////////////////////////////
  */
  /* mode byte of the safety validator server */
  const CSS_t_BYTE b_modeByte = IXSVS_DataModeByteGet(u16_svsIdx);


  /* // Set the Ping_Count_Reply to the value captured at the ping request
  Ack_Byte.Ping_Count_Reply = Mode_Byte.Ping_Count, */
  /* (see FRS55) */
  IXUTL_BIT_CLR_U8(ps_svs->s_tcooMsg.b_ackByte, IXSVD_k_AB_PING_COUNT_REPLY);
  IXUTL_BIT_SET_U8(ps_svs->s_tcooMsg.b_ackByte,
                   (CSS_t_USINT)IXUTL_BIT_GET_U8(b_modeByte,
                   IXSVD_k_MB_PING_COUNT));

  /* // Set the Ping_Response bit to 1
  Ack_Byte.Ping_Response = 1, */
  /* (see FRS53 and FRS195) */
  IXUTL_BIT_SET_U8(ps_svs->s_tcooMsg.b_ackByte, IXSVD_k_AB_PING_RESPONSE);

  /* // Set the Consumer_Time_Value to the Consumer clock value
  Time_Coordination_section.Consumer_Time_Value = Consumer_Clk_Count, */
  /* (see FRS51) */
  ps_svs->s_tcooMsg.u16_consTimeVal_128us = IXSVS_ConsumerClkCountGet();

  /* // Add data integrity measures
  Set Ack_Byte.Reserved(6:4) = 0,
  Set Ack_Byte.Reserved(2) = 0, */
  /* (see FRS203) */
  IXUTL_BIT_CLR_U8(ps_svs->s_tcooMsg.b_ackByte,
                   IXSVD_k_AB_RESERVED_1 | IXSVD_k_AB_RESERVED_2);

  /* Set Ack_Byte.Parity_Even bit to the correct value, */  /* (see FRS52) */
  ps_svs->s_tcooMsg.b_ackByte = IXCRC_ParityCalc(ps_svs->s_tcooMsg.b_ackByte);

  /* if is Extended Format */
  if (IXSVD_IsExtendedFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* Calculate CRC on Time Coordination message, */
      /* use CID as seed (see FRS330) */
      ps_svs->s_tcooMsg.u32_crcS5 =
        IXCRC_EfTcooMsgCrcCalc(&ps_svs->s_tcooMsg,
                               kps_crcSeeds->u32_cidCrcS5);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_24,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else /* else: must be Base Format */
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /*
      //Ack_Byte_2 not included in ExtendedFormat
      Ack_Byte_2 !=(((Ack_Byte XOR 0xFF)AND 0x55)OR
        (Ack_Byte AND 0xAA)),
      */
      ps_svs->s_tcooMsg.b_ackByte2 =
        IXCRC_TcByte2Calc(ps_svs->s_tcooMsg.b_ackByte);

      /* Calculate CRC on Time Coordination message, */
      /* use CID as seed (see FRS330) */
      ps_svs->s_tcooMsg.u16_crcS3 =
        IXCRC_BfTcooMsgCrcCalc(&ps_svs->s_tcooMsg,
                               kps_crcSeeds->u16_cidCrcS3);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_25,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }

  /*
  //////////////////////////////////////////////////////////////////
  // end time coordination message send logic
  //////////////////////////////////////////////////////////////////
  */

  /* set flag that indicates to send a TCOO message */
  ps_svs->o_tcooTimeToSend = CSS_k_TRUE;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : TcooMsgTimeoutStart
**
** Description : This function starts a timer that would trigger sending a Time
**               Coordination message when it expires. This is to make sure that
**               a Time Coordination message is sent within the first 5 seconds
**               of a ping interval.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooMsgTimeoutStart(CSS_t_UINT u16_svsIdx)
{
  /* Schedule the timeout at now + 5 seconds */
  as_SvsLocals[u16_svsIdx].u32_tcooTimer_128us
    = IXUTL_GetSysTime() + (IXSVD_k_5_SECONDS_US / CSS_k_TICK_BASE_US);
}


/*******************************************************************************
**
** Function    : TcooTxCopy
**
** Description : This function determines the message format of a Safety
**               Validator Server Instance and copies the data to be transmitted
**               (Time Coordination message) to the passed byte stream.
**
** Parameters  : u16_svsIdx (IN) - Index of the Safety Validator Server instance
**                                 (not checked, checked in
**                                 IXSVS_ConsInstActivityMonitor())
**               pb_data (OUT)   - pointer to byte stream where the transmit
**                                 message is to be stored
**                                 (not checked, only called with reference to
**                                 array)
**
** Returnvalue : -
**
*******************************************************************************/
static void TcooTxCopy(CSS_t_UINT u16_svsIdx,
                       CSS_t_BYTE *pb_data)
{
  /* if is Base Format */
  if (IXSVD_IsBaseFormat(IXSVS_InitInfoMsgFrmtGet(u16_svsIdx)))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /* write the message structure to the transmit byte stream */
      IXSFA_BfTcooMsgWrite(&as_SvsLocals[u16_svsIdx].s_tcooMsg, pb_data);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_11,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* write the message structure to the transmit byte stream */
      IXSFA_EfTcooMsgWrite(&as_SvsLocals[u16_svsIdx].s_tcooMsg, pb_data);
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_12,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSdata.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains the data received with the Data Message and
**             functions to read/write this data.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_ConsumerClkCountGet
**             IXSVS_DataLocalsClear
**             IXSVS_DataPtrGet
**             IXSVS_DataModeByteGet
**             IXSVS_DataTimeStampGet
**             IXSVS_DataMsgPtrGet
**             IXSVS_DataRxCopy
**
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
#if (    !(CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      || !(CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)       \
      || !(CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
      || !(CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
  #include "CSOSapi.h"
#endif

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"

#if (    !(CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      || !(CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)       \
      || !(CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
      || !(CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
  #include "IXSERapi.h"
  #endif
#include "IXUTL.h"
#include "IXSVD.h"
#include "IXSFA.h"
#include "IXSCF.h"

#include "IXSVSapi.h"
#include "IXSVSint.h"
#if (    !(CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      || !(CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)       \
      || !(CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
      || !(CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
  #include "IXSVSerr.h"
#endif


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* structure for storing the data of the associated data message */
/* Not relevant for Soft Error Checking - part of the safety function but value
   is updated before each use */
static CSS_t_DATA_MSG as_DataMsg[CSOS_cfg_NUM_OF_SV_SERVERS];


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVS_ConsumerClkCountGet
**
** Description : This function returns the current value of the global timer.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - counter that is incremented by a periodic timer
**                            every 128 uSec (16 bits, see FRS249).
**
*******************************************************************************/
CSS_t_UINT IXSVS_ConsumerClkCountGet(void)
{
  /* return only the least significant 16 bits of the global system time */
  /* (see FRS248) */
  CSS_t_UINT u16_clkCount_128us
    = (CSS_t_UINT)(IXUTL_GetSysTime() & 0x0000FFFFU);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_clkCount_128us);
}


/*******************************************************************************
**
** Function    : IXSVS_DataLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Server Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svsIdx (IN)  - index of Safety Validator Server structure
**                                  (not checked, checked in SafetyOpenProc3()
**                                  and IXSVS_InstAlloc())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVS_DataLocalsClear(CSS_t_UINT u16_svsIdx)
{
  /* completely erase the Safety Validator structure */
  CSS_MEMSET(&as_DataMsg[u16_svsIdx], 0, sizeof(as_DataMsg[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**
** Function    : IXSVS_DataPtrGet
**
** Description : This function returns a pointer to the payload data and their
**               length of the requested Safety Validator instance.
**
** Parameters  : u16_svsIdx (IN)   - Index of the Safety Validator Server
**                                   Instance
**                                   (not checked, checked in IXSVS_RxProc())
**               pu8_dataLen (OUT) - outputs the length of the payload data
**                                   (not checked, only called with reference to
**                                   variable)
**
** Returnvalue : CSS_t_BYTE*       - pointer to the payload data
**
*******************************************************************************/
const CSS_t_BYTE* IXSVS_DataPtrGet(CSS_t_UINT u16_svsIdx,
                                   CSS_t_USINT *pu8_dataLen)
{
  *pu8_dataLen = as_DataMsg[u16_svsIdx].u8_len;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_DataMsg[u16_svsIdx].ab_aData);
}


/*******************************************************************************
**
** Function    : IXSVS_DataModeByteGet
**
** Description : This function returns the Mode Byte of a Safety Validator
**               Server. Thus the user of this function does not have to care
**               whether the Validator Instance refers to Base or Extended and
**               Short or Long Format.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_BYTE      - Mode Byte
**
*******************************************************************************/
CSS_t_BYTE IXSVS_DataModeByteGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_DataMsg[u16_svsIdx].b_modeByte);
}


/*******************************************************************************
**
** Function    : IXSVS_DataTimeStampGet
**
** Description : This function returns the Time Stamp of the data message of a
**               Safety Validator Server. Thus the user of this function does
**               not have to care whether the Validator Instance refers to Base
**               or Extended and Short or Long Format.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_UINT      - Time Stamp (128us ticks)
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataTimeStampGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (as_DataMsg[u16_svsIdx].u16_timeStamp_128us);
}


/*******************************************************************************
**
** Function    : IXSVS_DataMsgPtrGet
**
** Description : This function returns a pointer to the data message of a
**               Safety Validator Server.
**
** Parameters  : u16_svsIdx (IN) - index of Safety Validator Server structure
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_t_DATA_MSG  - pointer to the data message
**
*******************************************************************************/
const CSS_t_DATA_MSG* IXSVS_DataMsgPtrGet(CSS_t_UINT u16_svsIdx)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (&as_DataMsg[u16_svsIdx]);
}


/*******************************************************************************
**
** Function    : IXSVS_DataRxCopy
**
** Description : This function copies the received Safety Data I/O Message Byte-
**               Stream into the Data Message structure of this instance.
**
** Parameters  : u16_svsIdx (IN)   - index of the Safety Validator Server
**                                   instance
**                                   (not checked, checked in IXSVS_RxProc())
**               pb_data (IN)      - pointer to the Time Correction Message in
**                                   the received byte stream.
**                                   (not checked, called with offset to HALC
**                                   message buffer which is checked in
**                                   IXSVO_CmdProcess())
**               b_msgFormat (IN)  - message format byte of the addressed
**                                   instance
**                                   (checked, allowed values: bit mask
**                                   combinations of {IXSVD_k_MSG_FORMAT_xxx})
**               u8_expPayLen (IN) - number of data bytes that the Safety
**                                   Validator is configured to consume (Safety
**                                   (I/O payload)
**                                   (not checked, checked in CnxnSizeCheck(),
**                                   IXSCE_MsgLenToPayloadLen())
**
** Returnvalue : CSS_t_UINT        - number of bytes copied
**
*******************************************************************************/
CSS_t_UINT IXSVS_DataRxCopy(CSS_t_UINT u16_svsIdx,
                            const CSS_t_BYTE *pb_data,
                            CSS_t_BYTE b_msgFormat,
                            CSS_t_USINT u8_expPayLen)
{
  /* return value of this function */
  CSS_t_UINT u16_retVal = 0U;

  /* if is Base Format */
  if (IXSVD_IsBaseFormat(b_msgFormat))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    /* if is Short Message Format */
    if (IXSVD_IsShortFormat(b_msgFormat))
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* read the data from the received byte stream to the message
           structure */
        u16_retVal = IXSFA_BfsMsgRead(pb_data, u8_expPayLen,
                                      &as_DataMsg[u16_svsIdx]);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_05,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Long message Format */
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* read the data from the received byte stream to the message
           structure */
        u16_retVal = IXSFA_BflMsgRead(pb_data, u8_expPayLen,
                                      &as_DataMsg[u16_svsIdx]);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_06,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_09,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* if is Short Message Format */
    if (IXSVD_IsShortFormat(b_msgFormat))
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* read the data from the received byte stream to the message
           structure */
        u16_retVal = IXSFA_EfsMsgRead(pb_data, u8_expPayLen,
                                      &as_DataMsg[u16_svsIdx]);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_07,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Long message Format */
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* read the data from the received byte stream to the message
           structure */
        u16_retVal = IXSFA_EflMsgRead(pb_data, u8_expPayLen,
                                      &as_DataMsg[u16_svsIdx]);
      }
    #else
      {
        IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_08,
                        IXSVS_InstIdFromIdxGet(u16_svsIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      IXSVS_ErrorClbk(IXSVS_FSE_INC_MSG_FORMAT_10,
                      IXSVS_InstIdFromIdxGet(u16_svsIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
  }

  /* set the data length of the message */
  as_DataMsg[u16_svsIdx].u8_len = u8_expPayLen;

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_retVal);
}


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


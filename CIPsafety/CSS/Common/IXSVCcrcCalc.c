/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCcrcCalc.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains functions to calculate the CRCs of the
**             messages to be transmitted and to generate the complemented data.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_TxCopy
**             IXSVC_CrcCalc
**
**             CrcCalcBaseShort
**             CrcCalcBaseLong
**             CrcCalcExtShort
**             CrcCalcExtLong
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
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSFA.h"
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


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)  \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) \
    )
static void CrcCalcBaseShort(CSS_t_UINT u16_svcIdx,
                             CSS_t_DATA_MSG *ps_sMsg);
#endif
#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) \
    )
static void CrcCalcBaseLong(CSS_t_UINT u16_svcIdx,
                            CSS_t_DATA_MSG *ps_sMsg);
#endif
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
    )
static void CrcCalcExtShort(CSS_t_UDINT u32_pidRcCrcS5,
                            CSS_t_DATA_MSG *ps_sMsg);
#endif
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
static void CrcCalcExtLong(CSS_t_UINT u16_pidRcCrcS3,
                           CSS_t_UDINT u32_pidRcCrcS5,
                           CSS_t_DATA_MSG *ps_sMsg);
#endif


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_TxCopy
**
** Description : This function determines the message format of a Safety
**               Validator Client Instance and copies the data to be transmitted
**               to the passed byte stream.
**
** Parameters  : u16_svcIdx (IN) - Instance Index of the Safety Validator Client
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**               ps_dataMsg (IN) - pointer to the data message of this instance
**                                 (not checked, only called with reference to
**                                 structure variable)
**               pu16_len (OUT)  - length of the transmit message
**                                 (not checked, only called with reference to
**                                 variable)
**               pb_data (OUT)   - pointer to byte stream where the transmit
**                                 message is to be stored
**                                 (not checked, only called with reference to
**                                 array variable)
**
** Returnvalue : CSS_k_OK        - success
**               <>CSS_k_OK      - error during execution of the function
**
*******************************************************************************/
CSS_t_WORD IXSVC_TxCopy(CSS_t_UINT u16_svcIdx,
                        const CSS_t_DATA_MSG *ps_dataMsg,
                        CSS_t_UINT *pu16_len,
                        CSS_t_BYTE *pb_data)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSVC_k_FSE_INC_PRG_FLOW);
  /* pre-calculated length of the generated frame */
  CSS_t_UINT u16_frameLen;

  /* if it is a multicast connection */
  if (IXSVD_IsMultiCast(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
    /* then the overall message length will contain a Time Correction Message */
    /* (Time Correction Message is not copied by this function) */
    u16_frameLen = CSOS_k_IO_MSGLEN_TCORR;
  }
  else /* else: singlecast */
  {
    /* only a data message */
    u16_frameLen = 0U;
  }

  /* if Base Format */
  if (IXSVD_IsBaseFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    /* if Short Format */
    if (IXSVD_IsShortFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* Base Format Short Message */
        u16_frameLen = (CSS_t_UINT)((CSS_t_UINT)u16_frameLen
                     + CSOS_k_IO_MSGLEN_SHORT_OVHD
                     + ps_dataMsg->u8_len);

        /* write the data from the msg structure to the transmit byte stream */
        *pu16_len = IXSFA_BfsMsgWrite(ps_dataMsg, u16_frameLen, pb_data);
        /* success */
        w_retVal = CSS_k_OK;
      }
    #else
      {
        w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_05);
        IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Long Format */
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* Base Format Long Message */
        u16_frameLen = (CSS_t_UINT)((CSS_t_UINT)u16_frameLen
                     + CSOS_k_IO_MSGLEN_LONG_OVHD
                     + ps_dataMsg->u8_len + ps_dataMsg->u8_len);

        /* write the data from the msg structure to the transmit byte stream */
        *pu16_len = IXSFA_BflMsgWrite(ps_dataMsg, u16_frameLen, pb_data);
        /* success */
        w_retVal = CSS_k_OK;
      }
    #else
      {
        w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_06);
        IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_09);
      IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) */
  }
  else /* else: must be Extended Format */
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* if Short Format */
    if (IXSVD_IsShortFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* Extended Format Short Message */
        u16_frameLen = (CSS_t_UINT)(u16_frameLen + CSOS_k_IO_MSGLEN_SHORT_OVHD
                      + ps_dataMsg->u8_len);

        /* write the data from the msg structure to the transmit byte stream */
        *pu16_len = IXSFA_EfsMsgWrite(ps_dataMsg,
                                      u16_frameLen, pb_data);
        /* success */
        w_retVal = CSS_k_OK;
      }
    #else
      {
        w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_07);
        IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Long Format */
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* Extended Format Long Message */
        u16_frameLen = (CSS_t_UINT)(u16_frameLen + CSOS_k_IO_MSGLEN_LONG_OVHD
                     + ps_dataMsg->u8_len + ps_dataMsg->u8_len);

        /* write the data from the msg structure to the transmit byte stream */
        *pu16_len = IXSFA_EflMsgWrite(ps_dataMsg, u16_frameLen, pb_data);
        /* success */
        w_retVal = CSS_k_OK;
      }
    #else
      {
        w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_08);
        IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      w_retVal = (IXSVC_FSE_INC_MSG_FORMAT_10);
      IXSVC_ErrorClbk(w_retVal, IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif  /* (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : IXSVC_CrcCalc
**
** Description : This function calculates the complement data and all CRCs of
**               the message produced by the Safety Validator Client. Thus the
**               caller of this function does not have to care whether the
**               Validator Instance refers to Base or Extended and Short or Long
**               Format.
**
** Parameters  : u16_svcIdx (IN)     - Instance index of the Safety Validator
**                                     Client instance
**                                     (not checked, checked in
**                                     IXSVC_InstTxFrameGenerate())
**               u16_pidRcCrcS3 (IN) - value of the CRC-S3 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               u32_pidRcCrcS5 (IN) - value of the CRC-S5 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               ps_dataMsg (IN/OUT) - pointer to the data message of this
**                                     instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - ab_cData
**                                        - u32_cCrcS5
**                                        - u16_aCrcS3
**                                        - u16_cCrcS3
**                                        - u8_tCrcS1
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                     (not checked, only called with reference
**                                     to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_CrcCalc(CSS_t_UINT u16_svcIdx,
               #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
                 #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
                   CSS_t_UINT u16_pidRcCrcS3,
                 #endif
                   CSS_t_UDINT u32_pidRcCrcS5,
               #endif
                   CSS_t_DATA_MSG *ps_dataMsg)
{

  /* if Extended Format */
  if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* if Long Format */
    if (IXSVD_IsLongFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* Long Format */
        CrcCalcExtLong(u16_pidRcCrcS3, u32_pidRcCrcS5, ps_dataMsg);
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_01,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Short Format */
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* Short Format */
        CrcCalcExtShort(u32_pidRcCrcS5, ps_dataMsg);
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_02,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_17,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else /* else: must be Base Format */
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    /* if Long Format */
    if (IXSVD_IsLongFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
    {
    #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
      {
        /* Long Format */
        CrcCalcBaseLong(u16_svcIdx, ps_dataMsg);
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_03,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: must be Short Format */
    {
    #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
      {
        /* Short Format */
        CrcCalcBaseShort(u16_svcIdx, ps_dataMsg);
      }
    #else
      {
        IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_04,
                        IXSVC_InstIdFromIdxGet(u16_svcIdx),
                        IXSER_k_A_NOT_USED);
      }
    #endif
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_18,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


/*******************************************************************************
**    static functions
*******************************************************************************/

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)  \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) \
    )
/*******************************************************************************
**
** Function    : CrcCalcBaseShort
**
** Description : This function calculates the CRCs of a Base Short Format
**               message.
**
** Parameters  : u16_svcIdx (IN)  - Instance index of the Safety Validator
**                                  Client instance
**                                  (not checked, checked in
**                                  IXSVC_InstTxFrameGenerate())
**               ps_sMsg (IN/OUT) - pointer to the data message of this
**                                  instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                        - u8_tCrcS1
**                                     Not applicable for this format:
**                                        - ab_cData
**                                        - u32_cCrcS5
**                                        - u16_aCrcS3
**                                        - u16_cCrcS3
**                                  (not checked, only called with reference to
**                                  structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CrcCalcBaseShort(CSS_t_UINT u16_svcIdx,
                             CSS_t_DATA_MSG *ps_sMsg)
{
  /* pointer to the initially calculated values of this instance (u8_pidCrcS1
     needed here as seed for CRC calculation (see FRS7 and FRS155)) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);

  /* Calculation of Actual CRC */
  ps_sMsg->u8_aCrcS1 = IXCRC_BfsMsgAcrcCalc(ps_sMsg, kps_initCalc->u8_pidCrcS1);

  /* Calculation of Complementary CRC */
  ps_sMsg->u8_cCrcS2 = IXCRC_BfsMsgCcrcCalc(ps_sMsg, kps_initCalc->u8_pidCrcS1);

  /* Calculation of Time Stamp CRC */
  ps_sMsg->u8_tCrcS1 = IXCRC_BfMsgTsCrcCalc(ps_sMsg, kps_initCalc->u8_pidCrcS1);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) \
    )
/*******************************************************************************
**
** Function    : CrcCalcBaseLong
**
** Description : This function calculates the CRCs and the Complement Data of a
**               Base Long Format message.
**
** Parameters  : u16_svcIdx (IN)  - Instance index of the Safety Validator
**                                  Client instance
**                                  (not checked, checked in
**                                  IXSVC_InstTxFrameGenerate())
**               ps_sMsg (IN/OUT) - pointer to the data message of this
**                                  instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - ab_cData
**                                        - u16_aCrcS3
**                                        - u16_cCrcS3
**                                        - u8_tCrcS1
**                                     Not applicable for this format:
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                        - u32_cCrcS5
**                                  (not checked, only called with reference to
**                                  structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CrcCalcBaseLong(CSS_t_UINT u16_svcIdx,
                            CSS_t_DATA_MSG *ps_sMsg)
{
  /* pointer to the initially calculated values of this instance (u8_pidCrcS1
     and u16_pidCrcS3 are needed here as seed for CRC calculation
     (see FRS7 and FRS155)) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);

  /* Calculation of Complemented Data */
  IXCRC_ActVsCompDataGen(ps_sMsg);

  /* Calculation of Actual CRC */
  ps_sMsg->u16_aCrcS3 = IXCRC_BflMsgAcrcCalc(ps_sMsg,
                                             kps_initCalc->u16_pidCrcS3);

  /* Calculation of Complementary CRC */
  ps_sMsg->u16_cCrcS3 = IXCRC_BflMsgCcrcCalc(ps_sMsg,
                                             kps_initCalc->u16_pidCrcS3);

  /* Calculation of Time Stamp CRC */
  ps_sMsg->u8_tCrcS1 = IXCRC_BfMsgTsCrcCalc(ps_sMsg, kps_initCalc->u8_pidCrcS1);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
    )
/*******************************************************************************
**
** Function    : CrcCalcExtShort
**
** Description : This function creates an Extended Format Short data message by
**               calculating all CRCs of the Safety Validator Client.
**
** Parameters  : u32_pidRcCrcS5 (IN) - value of the CRC-S5 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               ps_sMsg (IN/OUT)    - pointer to the data message of this
**                                     instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - u32_cCrcS5
**                                     Not applicable for this format:
**                                        - ab_cData
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                        - u8_tCrcS1
**                                        - u16_aCrcS3
**                                        - u16_cCrcS3
**                                     (not checked, only called with reference
**                                     to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CrcCalcExtShort(CSS_t_UDINT u32_pidRcCrcS5,
                            CSS_t_DATA_MSG *ps_sMsg)
{
  /* Calculation of CRC - PID used as seed (see FRS7 and FRS155) */
  ps_sMsg->u32_crcS5 =
    IXCRC_EfsMsgCrcCalc(ps_sMsg, u32_pidRcCrcS5);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE) \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
/*******************************************************************************
**
** Function    : CrcCalcExtLong
**
** Description : This function creates an Extended Format Long data message by
**               calculating complement data and all CRCs of the Safety
**               Validator Client.
**
** Parameters  : u16_pidRcCrcS3 (IN) - value of the CRC-S3 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               u32_pidRcCrcS5 (IN) - value of the CRC-S5 seed (over PID and
**                                     Rollover Counter)
**                                     (not checked, any value allowed)
**               ps_sMsg (IN/OUT)    - pointer to the data message of this
**                                     instance
**                                     These fields are used as inputs:
**                                        - ab_aData
**                                        - u8_len
**                                        - b_modeByte
**                                        - u16_timeStamp_128us
**                                     These fields are used as outputs:
**                                        - ab_cData
**                                        - u16_aCrcS3
**                                        - u32_cCrcS5
**                                     Not applicable for this format:
**                                        - u8_aCrcS1
**                                        - u8_cCrcS2
**                                        - u8_tCrcS1
**                                        - u16_cCrcS3
**                                     (not checked, only called with reference
**                                     to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
static void CrcCalcExtLong(CSS_t_UINT u16_pidRcCrcS3,
                           CSS_t_UDINT u32_pidRcCrcS5,
                           CSS_t_DATA_MSG *ps_sMsg)
{
  /* Calculation of Complemented Data */
  IXCRC_ActVsCompDataGen(ps_sMsg);

  /* Calculation of Actual CRC - PID used as seed (see FRS7 and FRS155) */
  ps_sMsg->u16_aCrcS3 =
    IXCRC_EflMsgCrcS3Calc(ps_sMsg, u16_pidRcCrcS3);

  /* Calculation of Complementary CRC - PID used as seed (see FRS7 and FRS155)*/
  ps_sMsg->u32_cCrcS5 =
    IXCRC_EflMsgCcrcCalc(ps_sMsg, u32_pidRcCrcS5);

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


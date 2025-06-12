/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVCtcorr.c
**    Summary: IXSVC - Safety Validator Client
**             This module contains the functions to generate a Time Correction
**             message that is to be sent.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVC_TcorrLocalsClear
**             IXSVC_TcorrMsgProd
**             IXSVC_TcorrBaseTxCopy
**             IXSVC_TcorrExtTxCopy
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


#if (    !(CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      || !(CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)       \
    )
  #include "IXSERapi.h"
#endif
#include "IXUTL.h"
#include "IXSVD.h"
#include "IXCRC.h"
#include "IXSFA.h"
#include "IXSCF.h"

#include "IXSVCapi.h"
#include "IXSVC.h"
#include "IXSVCint.h"
#if (    !(CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      || !(CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)       \
    )
  #include "IXSVCerr.h"
#endif


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/* structures for storing the data of the associated timing messages */
/* Not relevant for Soft Error Checking - part of the safety function but value
   is updated before each use */
static CSS_t_TCORR_MSG as_TcorrMsg[CSOS_cfg_NUM_OF_SV_CLIENTS];

/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVC_TcorrLocalsClear
**
** Description : This function is to be called before initializing a Safety
**               Validator Client Instance. It clears all fields of the local
**               structure.
**
** Parameters  : u16_svcIdx (IN)  - index of Safety Validator Client structure
**                                  (not checked, checked in IXSVC_Init()
**                                  IXSVC_InstAlloc() and IXSVO_TargClear())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcorrLocalsClear(CSS_t_UINT u16_svcIdx)
{
  CSS_MEMSET(&as_TcorrMsg[u16_svcIdx], 0, sizeof(as_TcorrMsg[0]));

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}

/*******************************************************************************
**
** Function    : IXSVC_TcorrMsgProd
**
** Description : This function produces a Time Correction message. This logic
**               was part of the IXSVC_MultiCastProducerFunction() but put into
**               an own function for clarity and symmetry to the Safety
**               Validator Server.
**
** Parameters  : u16_svcIdx (IN) - Instance index of the Safety Validator Client
**                                 instance
**                                 (not checked, checked in
**                                 IXSVC_InstTxFrameGenerate())
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVC_TcorrMsgProd(CSS_t_UINT u16_svcIdx)
{
  const CSS_t_USINT u8_RR_Con_Num_Index_Pntr =
    IXSVC_RrConNumIdxPntrGet(u16_svcIdx);
  /* pointer to the initially calculated values of this instance
     (u16_pidCrcS3 and u32_pidCrcS5 are needed here) */
  const IXSVC_t_INIT_CALC* const kps_initCalc =
    IXSVC_InitInfoCalcPtrGet(u16_svcIdx);
  /* flag indicating the state of this consumer */
  CSS_t_BOOL o_isFaulted = CSS_k_FALSE;
  /* as the conditions of the following IF from the sample code had to be
     separated we need another flag for indicating when the else part must be
     executed */
  CSS_t_BOOL o_nullTcorr = CSS_k_TRUE;

  /* completely erase the Mcast_Byte. This also clears the reserved bits (see
     FRS215). All other bits will be set by the code following below */
  as_TcorrMsg[u16_svcIdx].b_mCastByte = 0x00U;

  /* Send a Time Correction message to each consumer once per Ping Interval */
  /*
  // Check if its time to send an active non-errored Time Correction
  // message
  IF ((RR_Con_Num_Index_Pntr < Max_Consumer_Number) AND
      (Consumer_Active_Idle[RR_Con_Num_Index_Pntr] == Active)), AND
      (Consumer_Open[RR_Con_Num_Index_Pntr] == Open) AND
      (S_Connection_Fault[RR_Con_Num_Index_Pntr] == OK)
  THEN
  */
  /* if not yet all consumers served */
  if (u8_RR_Con_Num_Index_Pntr < IXSVC_InitInfoMaxConsNumGet(u16_svcIdx))
  {
    /* u8_RR_Con_Num_Index_Pntr is in valid range to access arrays */
    /* get a pointer to per consumer data */
    const IXSVC_t_INST_INFO_PER_CONS *kps_iiperCons =
      IXSVC_InstInfoPerConsGet(u16_svcIdx, u8_RR_Con_Num_Index_Pntr);

    o_isFaulted = IXSVC_ConnectionIsFaulted(u16_svcIdx,
                                            u8_RR_Con_Num_Index_Pntr);

    if (    (IXSVC_ConsumerIsActive(u16_svcIdx, u8_RR_Con_Num_Index_Pntr))
         && (kps_iiperCons->o_Consumer_Open)
         && (!o_isFaulted)
       )
    {
      /* for each consumer with an active open connection this is executed    */
      /* only once per Ping_count_Interval (see FRS60 and FRS234) */

      /* //Send_Time_Correction_Message = 1, */
      /* Flag is only needed for DeviceNet (on other Networks Time Correction */
      /* is appended to Data Message)                                         */

      /* Mcast_Byte.Consumer_# = RR_Con_Num_Index_Pntr + 1, */
      /* (see FRS65 and FRS205) */
      IXUTL_BIT_SET_U8(as_TcorrMsg[u16_svcIdx].b_mCastByte,
                       (CSS_t_USINT)(u8_RR_Con_Num_Index_Pntr+1U));

      /* Mcast_Byte.Multi_Cast_Active_Idle =
         Consumer_Active_Idle[RR_Con_Num_Index_Pntr], */
      /* when we get to here then the Consumer_Active_Idle bit is definitely
         set */
      IXUTL_BIT_SET_U8(as_TcorrMsg[u16_svcIdx].b_mCastByte, IXSVD_k_MCB_MAI);

      /* Time_Correction_Section.Time_Correction_Value =
           Consumer_Time_Correction_Value[RR_Con_Num_Index_Pntr], */
      /* (see FRS27 and FRS205) */
      as_TcorrMsg[u16_svcIdx].u16_timeCorrVal_128us =
        IXSVC_ConsTimeCorrValGet(u16_svcIdx, u8_RR_Con_Num_Index_Pntr);

      /* indicate that the IF part of the sample code has been executed */
      o_nullTcorr = CSS_k_FALSE;
    }
    else
    {
      /* nothing to do */
      /* see "load null TIME_CORRECTION Section" part below */
    }
  }
  else
  {
    /* nothing to do */
    /* see "load null TIME_CORRECTION Section" part below */
  }
  /* ELSE */

  /* if the IF part of the sample code has not been executed */
  if (o_nullTcorr)
  {
    /*
    //load null TIME_CORRECTION Section,
    Mcast_Byte.Consumer_# = 0,
    */
    /* already initialized to zero at the beginning of this function */

    /* Mcast_Byte.Multi_Cast_Active_Idle = Idle, */
    /* (see FRS358) */
    /* already initialized to zero at the beginning of this function */

    /* Time_Correction_Section.Time_Correction_Value = 0, */
    as_TcorrMsg[u16_svcIdx].u16_timeCorrVal_128us = 0U;
  }
  else /* else: the IF part has been executed (all conditions fulfilled) */
  {
    /* nothing to do */
  }
  /* ENDIF */

  /* Calculation of parity, Mcast_Byte_2 and CRC is the same for the above 2 */
  /* cases thus instead of having this twice it was moved behind the 'if'. */
  /* Set Mcast_Byte.Parity_Even to the correct value, */
  /* (see FRS62) */
  as_TcorrMsg[u16_svcIdx].b_mCastByte =
    IXCRC_ParityCalc(as_TcorrMsg[u16_svcIdx].b_mCastByte);

  /* Extended or Base Format (see FRS371) */
  /* if Extended Format */
  if (IXSVD_IsExtendedFormat(IXSVC_InitInfoMsgFrmtGet(u16_svcIdx)))
  {
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    {
      /* Calculate CRC of time correction message, */
      as_TcorrMsg[u16_svcIdx].u32_crcS5 =
        IXCRC_EfTcrMsgCrcCalc(&as_TcorrMsg[u16_svcIdx],
                              kps_initCalc->u32_pidCrcS5);  /* (see FRS155) */
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_27,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }
  else /* else: must be Base Format */
  {
  #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
    {
      /* MCast_Byte_2 = ((MCast_Byte XOR 0xFF) AND 0x55) OR
           (MCast_Byte AND 0xAA) */
      as_TcorrMsg[u16_svcIdx].b_mCastByte2 =
        IXCRC_TcByte2Calc(as_TcorrMsg[u16_svcIdx].b_mCastByte);

      /* Calculate CRC of time correction message, */
      as_TcorrMsg[u16_svcIdx].u16_crcS3 =
        IXCRC_BfTcrMsgCrcCalc(&as_TcorrMsg[u16_svcIdx],
                              kps_initCalc->u16_pidCrcS3);  /* (see FRS155) */
    }
  #else
    {
      IXSVC_ErrorClbk(IXSVC_FSE_INC_MSG_FORMAT_28,
                      IXSVC_InstIdFromIdxGet(u16_svcIdx),
                      IXSER_k_A_NOT_USED);
    }
  #endif
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVC_TcorrBaseTxCopy
**
** Description : This function copies the data of a Time Correction Message
**               to be transmitted to the passed byte stream (Base Format).
**
** Parameters  : u16_svcIdx (IN)   - Instance Index of the Safety Validator
**                                   Client
**                                   (not checked, checked in
**                                   IXSVC_InstTxFrameGenerate())
**               u16_offset (IN)   - offset in the passed buffer where the Tcorr
**                                   message will start
**                                   (not checked, checked in
**                                   IXSVC_TxFrameGenerate())
**               u16_frameLen (IN) - overall length of the provided buffer
**                                   (not checked, checked in called function
**                                   IXSFA_BfTcrMsgWrite())
**               pb_data (OUT)     - pointer to byte stream where the transmit
**                                   message is to be stored
**                                   (not checked, only called with pointer to
**                                   array variable)
**
** Returnvalue : CSS_t_UINT        - number of bytes that are now in the pb_data
**                                   buffer (u16_offset + number of bytes copied
**                                   by this function
**
*******************************************************************************/
CSS_t_UINT IXSVC_TcorrBaseTxCopy(CSS_t_UINT u16_svcIdx,
                                 CSS_t_UINT u16_offset,
                                 CSS_t_UINT u16_frameLen,
                                 CSS_t_BYTE *pb_data)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* write the data from the msg structure to the transmit byte stream */
  return (IXSFA_BfTcrMsgWrite(&as_TcorrMsg[u16_svcIdx],
                              u16_offset, u16_frameLen, pb_data));
}
#endif


#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSVC_TcorrExtTxCopy
**
** Description : This function copies the data of a Time Correction Message
**               to be transmitted to the passed byte stream (Extended Format).
**
** Parameters  : u16_svcIdx (IN)   - Instance Index of the Safety Validator
**                                   Client
**                                   (not checked, checked in
**                                   IXSVC_InstTxFrameGenerate())
**               u16_offset (IN)   - offset in the passed buffer where the Tcorr
**                                   message will start
**                                   (not checked, checked in
**                                   IXSVC_TxFrameGenerate())
**               u16_frameLen (IN) - overall length of the provided buffer
**                                   (not checked, checked in called function
**                                   IXSFA_EfTcrMsgWrite())
**               pb_data (OUT)     - pointer to byte stream where the transmit
**                                   message is to be stored
**                                   (not checked, only called with pointer to
**                                   array variable)
**
** Returnvalue : CSS_t_UINT        - number of bytes that are now in the pb_data
**                                   buffer (u16_offset + number of bytes copied
**                                   by this function
**
*******************************************************************************/
CSS_t_UINT IXSVC_TcorrExtTxCopy(CSS_t_UINT u16_svcIdx,
                                CSS_t_UINT u16_offset,
                                CSS_t_UINT u16_frameLen,
                                CSS_t_BYTE *pb_data)
{
  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  /* write the data from the msg structure to the transmit byte stream */
  return (IXSFA_EfTcrMsgWrite(&as_TcorrMsg[u16_svcIdx],
                              u16_offset, u16_frameLen, pb_data));

}
#endif


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U) */


/*** End Of File ***/


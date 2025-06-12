/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVSmsgIntegCheck.c
**    Summary: IXSVS - Safety Validator Server
**             This module contains functions to check the integrity of a
**             received I/O message.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVS_MsgIntegCheckBaseShort
**             IXSVS_MsgIntegCheckBaseLong
**             IXSVS_MsgIntegCheckExtShort
**             IXSVS_MsgIntegCheckExtLong
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
#include "IXCRC.h"

#include "IXSVSapi.h"
#include "IXSVSint.h"
#include "IXSVSerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)    \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE) \
    )
/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckBaseShort
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Base Format Short
**               safety message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
CSS_t_BOOL IXSVS_MsgIntegCheckBaseShort(CSS_t_UINT u16_svsIdx)
{
  /* return value of the function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* temporary variable for CRC checking */
  CSS_t_USINT u8_expCrc = 0U;
  /* counts the number of errors found */
  CSS_t_USINT u8_errCount = 0U;
  /* get a pointer to read the Safety Validator's received data message */
  const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /*
  // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  */

  /* check format (Mode Byte) and CRCs (see FRS8-2) */

  /* check the Actual_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  u8_expCrc = IXCRC_BfsMsgAcrcCalc(ps_dataMsg,
                                   kps_initCalc->u8_pidCrcS1);
  /* if calculated CRC doesn't match received CRC */
  if (u8_expCrc != ps_dataMsg->u8_aCrcS1)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_SHRT_ACRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), (CSS_t_UDINT)u8_expCrc);

  }
  else
  {
    /* CRC is correct */
  }

  /* check the Complement_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  /* if calculated CRC doesn't match received CRC */
  u8_expCrc = IXCRC_BfsMsgCcrcCalc(ps_dataMsg,
                                   kps_initCalc->u8_pidCrcS1);
  if (u8_expCrc != ps_dataMsg->u8_cCrcS2)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_SHRT_CCRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), (CSS_t_UDINT)u8_expCrc);

  }
  else
  {
    /* CRC is correct */
  }

  /* if the Mode Byte checks indicate error (RuN_Idle, TBD_2_Bit, TBD_Bit) */
  if (IXCRC_ModeByteRedundantBitsCheck(IXSVS_DataModeByteGet(u16_svsIdx))
      != CSS_k_OK)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_SHRT_RBMB,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* redundant bits correct */
  }

  /* Base Format Short Message does not contain complementary data */

  /* if errors found */
  if (u8_errCount != 0U)
  {
    /* errors detected - return FALSE */
    o_retVal = CSS_k_FALSE;
  }
  else /* else: ok */
  {
    /* no errors found - return TRUE */
    o_retVal = CSS_k_TRUE;
  }

  return (o_retVal);
}
#endif


#if (    (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE) \
    )
/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckBaseLong
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Base Format Long
**               safety message.
**
** Parameters  : u16_svsIdx (IN) - index of the Safety Validator Server instance
**                                 (not checked, checked in IXSVS_RxProc())
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
CSS_t_BOOL IXSVS_MsgIntegCheckBaseLong(CSS_t_UINT u16_svsIdx)
{
  /* return value of the function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* temporary variable for CRC checking */
  CSS_t_UINT u16_expCrc = 0U;
  /* counts the number of errors found */
  CSS_t_USINT u8_errCount = 0U;
  /* get a pointer to read the Safety Validator's received data message */
  const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);
  /* pointer to the initially calculated values of this instance (CRC seeds,
     Max_Fault_Number, Network_Time_Expectation_Multiplier) */
  const IXSVS_t_INIT_CALC* const kps_initCalc =
    IXSVS_InitInfoCalcPtrGet(u16_svsIdx);

  /*
  // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  */

  /* check format (Mode Byte) and CRCs (see FRS8-2) */

  /* check the Actual_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  /* if calculated CRC doesn't match received CRC */
  u16_expCrc = IXCRC_BflMsgAcrcCalc(ps_dataMsg,
                                    kps_initCalc->u16_pidCrcS3);
  if (u16_expCrc != ps_dataMsg->u16_aCrcS3)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_LONG_ACRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx),
                    (CSS_t_UDINT)u16_expCrc);
  }
  else
  {
    /* CRC is correct */
  }

  /* check the Complement_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  /* if calculated CRC doesn't match received CRC */
  u16_expCrc = IXCRC_BflMsgCcrcCalc(ps_dataMsg,
                                    kps_initCalc->u16_pidCrcS3);
  if (u16_expCrc != ps_dataMsg->u16_cCrcS3)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_LONG_CCRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx),
                    (CSS_t_UDINT)u16_expCrc);
  }
  else
  {
    /* CRC is correct */
  }

  /* if the Mode Byte checks indicate error (RuN_Idle, TBD_2_Bit, TBD_Bit) */
  if (IXCRC_ModeByteRedundantBitsCheck(IXSVS_DataModeByteGet(u16_svsIdx))
      != CSS_k_OK)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_LONG_RBMB,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* redundant bits correct */
  }

  /* if Acutal_Data versus Complement_Data check indicates error (see FRS8-1) */
  if (IXCRC_ActVsCompDataCheck(ps_dataMsg) != CSS_k_OK)
  {
    u8_errCount++;  /* (see FRS6 and FRS130) */
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_BASE_LONG_AVSC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* cross-check passed */
  }

  /* if errors found */
  if (u8_errCount != 0U)
  {
    /* errors detected - return FALSE */
    o_retVal = CSS_k_FALSE;
  }
  else /* else: ok */
  {
    /* no errors found - return TRUE */
    o_retVal = CSS_k_TRUE;
  }

  return (o_retVal);
}
#endif


#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)    \
    )
/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckExtShort
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Extended Format
**               Short safety message.
**
** Parameters  : u16_svsIdx (IN)     - index of the Safety Validator Server
**                                     instance
**                                     (not checked, checked in IXSVS_RxProc())
**               u32_pidRcCrcS5 (IN) - CRC-S5 over Producer Identifier
**                                     (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
CSS_t_BOOL IXSVS_MsgIntegCheckExtShort(CSS_t_UINT u16_svsIdx,
                                       CSS_t_UDINT u32_pidRcCrcS5)
{
  /* return value of the function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* temporary variable for CRC checking */
  CSS_t_UDINT u32_expCrc = 0U;
  /* counts the number of errors found */
  CSS_t_USINT u8_errCount = 0U;
  /* get a pointer to read the Safety Validator's received data message */
  const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);

  /*
  // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  */

  /* check format (Mode Byte) and CRCs (see FRS8-2) */

  /* check the CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  /* if calculated CRC doesn't match received CRC */
  u32_expCrc = IXCRC_EfsMsgCrcCalc(ps_dataMsg, u32_pidRcCrcS5);
  if (u32_expCrc != ps_dataMsg->u32_crcS5)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_SHORT_CRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), u32_expCrc);
  }
  else
  {
    /* CRC is correct */
  }

  /* Extended Format Short Message does not contain complementary CRC */

  /* if the Mode Byte checks indicate error (RuN_Idle, TBD_2_Bit, TBD_Bit) */
  if (IXCRC_ModeByteRedundantBitsCheck(IXSVS_DataModeByteGet(u16_svsIdx))
      != CSS_k_OK)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_SHORT_RBMB,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* redundant bits correct */
  }

  /* Extended Format Short Message does not contain complementary data */

  /* if errors found */
  if (u8_errCount != 0U)
  {
    /* errors detected - return FALSE */
    o_retVal = CSS_k_FALSE;
  }
  else /* else: ok */
  {
    /* no errors found - return TRUE */
    o_retVal = CSS_k_TRUE;
  }

  return (o_retVal);
}
#endif


#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)   \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)     \
    )
/*******************************************************************************
**
** Function    : IXSVS_MsgIntegCheckExtLong
**
** Description : This function is an auxiliary function for the consumer safety
**               data reception logic. It checks the received Extended Format
**               Long safety message.
**
** Parameters  : u16_svsIdx (IN)     - index of the Safety Validator Server
**                                     instance
**                                     (not checked, checked in IXSVS_RxProc())
**               u32_pidRcCrcS5 (IN) - CRC-S5 over Producer Identifier
**                                     (not checked, any value allowed)
**               u16_pidRcCrcS3 (IN) - CRC-S3 over Producer Identifier
**                                     (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE  - All message integrity checks were successful
**               CSS_k_FALSE - Message integrity checks failed
**
*******************************************************************************/
CSS_t_BOOL IXSVS_MsgIntegCheckExtLong(CSS_t_UINT u16_svsIdx,
                                      CSS_t_UDINT u32_pidRcCrcS5,
                                      CSS_t_UINT u16_pidRcCrcS3)
{
  /* return value of the function */
  CSS_t_BOOL o_retVal = CSS_k_FALSE;
  /* temporary variables for CRC checking */
  CSS_t_UINT u16_expCrc = 0U;
  CSS_t_UDINT u32_expCrc = 0U;
  /* counts the number of errors found */
  CSS_t_USINT u8_errCount = 0U;
  /* get a pointer to read the Safety Validator's received data message */
  const CSS_t_DATA_MSG * const ps_dataMsg = IXSVS_DataMsgPtrGet(u16_svsIdx);

  /*
  // check for data integrity faults
  IF ((CRC Error based on format used)
       OR (Complement_Data_Section CRC != OK)
       OR (Mode_Byte.Run_Idle != not Mode_Byte.N_Run_Idle)
       OR (Mode_Byte. TBD_2_Bit !=
           Mode_Byte. TBD_2_Bit _Copy)
       OR (Mode_Byte.TBD_Bit != not Mode_Byte.N_TBD_Bit)
       OR (Actual_Data != not Complement_Data) [for > 2 bytes only]
     ),
  */

  /* check format (Mode Byte) and CRCs (see FRS8-2) */

  /* check the Actual_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  u16_expCrc = IXCRC_EflMsgCrcS3Calc(ps_dataMsg, u16_pidRcCrcS3);
  /* if calculated CRC doesn't match received CRC */
  if (u16_expCrc != ps_dataMsg->u16_aCrcS3)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_LONG_ACRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx),
                    (CSS_t_UDINT)u16_expCrc);
  }
  else
  {
    /* CRC is correct */
  }

  /* check the Complement_CRC using PID as seed (see FRS7 and FRS156) */
  /* (see FRS8-3) */
  u32_expCrc = IXCRC_EflMsgCcrcCalc(ps_dataMsg, u32_pidRcCrcS5);
  /* if calculated CRC doesn't match received CRC */
  if (u32_expCrc != ps_dataMsg->u32_cCrcS5)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_LONG_CCRC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), u32_expCrc);
  }
  else
  {
    /* CRC is correct */
  }

  /* if the Mode Byte checks indicate error (RuN_Idle, TBD_2_Bit, TBD_Bit) */
  if (IXCRC_ModeByteRedundantBitsCheck(IXSVS_DataModeByteGet(u16_svsIdx))
      != CSS_k_OK)
  {
    u8_errCount++;
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_LONG_RBMB,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* redundant bits correct */
  }

  /* if Actual_Data versus Complement_Data check indicates error (see FRS8-1) */
  if (IXCRC_ActVsCompDataCheck(ps_dataMsg) != CSS_k_OK)
  {
    u8_errCount++;  /* (see FRS6 and FRS130) */
    IXSVS_ErrorClbk(IXSVS_k_NFSE_RXI_EXT_LONG_AVSC,
                    IXSVS_InstIdFromIdxGet(u16_svsIdx), IXSER_k_A_NOT_USED);
  }
  else
  {
    /* cross-check passed */
  }

  /* if errors found */
  if (u8_errCount != 0U)
  {
    /* errors detected - return FALSE */
    o_retVal = CSS_k_FALSE;
  }
  else /* else: ok */
  {
    /* no errors found - return TRUE */
    o_retVal = CSS_k_TRUE;
  }

  return (o_retVal);
}
#endif


/*******************************************************************************
**    static functions
*******************************************************************************/


#endif  /* (CSOS_cfg_NUM_OF_SV_SERVERS > 0U) */


/*** End Of File ***/


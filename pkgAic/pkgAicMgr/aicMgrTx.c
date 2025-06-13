/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMgrTx.c
**     Summary: AIC-Manager Module containing the interface functions and high level functions
**              of the AIC communication module for AIC transmission (TX)
**              The module is also responsible to merge several parts to one telegram to send via
**              UART:
**              - Ctrl/Status-Byte of ASM
**              - Non-safe Message part (TX)
**              - Safe Message part (SPDU, safe In-Packet)
**              - Safe IO-Data in extra non-safe part
**              - CRC of complete UART telegram
**   $Revision: 4440 $
**       $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMgrTx_SysInit
**             aicMgrTx_BuildAsmTgm
**             aicMgrTx_MergeSendAsmTgm
**             aicMgrTx_MergeSendStartupTgm
**             aicMgrTx_SampleIoData
**             aicMgrTx_ResetIoData
**
**             MergeBuffer
**             BuildStartupTgm
**             SampleDiStates
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
#include "rds.h"
#include "version.h"

#include "globPreChk.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "timer-def.h"
#include "timer-hal.h"

#include "includeDiDo.h"
#include "fiParam.h"

/* AIC module includes */
#include "aicMsgDef.h"
#include "aicMsgBld.h"
#include "aicMsgProc.h"
#include "aicMsgCtrlSts.h"
#include "aicCrc.h"
#include "aicUart.h"

#include "aicMgrTx.h"

#include "aicSm.h"

#include "spduIn.h"
#include "spduOutIo.h"
#include "spduOutTcoo.h"
#include "spduOutData.h"

#ifdef RFID_ACTIVE
  #include "RFID.h"
#endif


/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMgrTx_s_IoVal:
** structure to store the data to send via safe protocol (see [SRS_2012], [SRS_2013],
** input data only, not the safe packet), global used due to performance reasons
** formerly aicDataStackSpdu_sTxSpduInData
*/
#ifdef RFID_ACTIVE
volatile AICMSGDEF_PS_INPUT_DATA_STRUCT aicMgrTx_s_IoVal = {0x00u, 0x00u, 0x00u, 0x00uL};
#else
volatile AICMSGDEF_PS_INPUT_DATA_STRUCT aicMgrTx_s_IoVal = {0x00u, 0x00u, 0x00u};
#endif
/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      4u

/*------------------------------------------------------------------------------
**  definitions for time delays between AIC telegrams during startup,
**  unit 1us + 0,8%   see [SIS_044]
**------------------------------------------------------------------------------
*/
#define k_STARTUP_TELEGRAM_DELAY  ((UINT32)24801u)  /* 24801 = about 25ms */


/* This variable contains the last time to calculate the next aic message at
 * startup. */
STATIC UINT32 u32_LastTime = (UINT32)0u;


/** PRE-PROCESSOR CHECK **/
/* Ensure that startup telegram fits into the AIC message buffer */
/* Deactivated Lint Note 948: Operator '==' always evaluates to True */
IXX_TASSERT(sizeof(AICMSGDEF_ASM_STARTUP_TELEGRAM_STRUCT)  <=
            sizeof (aicUart_sAicTxDmaBuffer), aicMgrTx_chk1) /*lint !e948 */

/* Mute lint warning type not referenced. These types are just defined for    */
/* being able to check the size of the elementary types.                      */
/*lint -esym(751, IXX_DUMMY_aicMgrTx_chk1) not referenced - just for checks */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void MergeBuffer (void);
STATIC void BuildStartupTgm (void);
STATIC void SampleDiStates(void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    aicMgrTx_SysInit

  Description:
    This function initializes the aic modules (including DMA transfer)

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void aicMgrTx_SysInit (void)
{
  /* init modules of package 'Spdu' */
  spduIn_Init();
  spduOutData_Reset(); /* set default */
  spduOutIo_Init();
  spduOutTcoo_Init();
  
  /* init non-safe message part */
  aicMsgProc_Init();

  /* initialization of UART/DMA HW */
  aicUart_UartDmaInit();

  /* initialize time variable with current system time  */
  u32_LastTime = timerHAL_GetSystemTime3();

} /* end of aic_SysInit() */




/***************************************************************************************************
  Function:
    aicMgrTx_BuildAsmTgm

  Description:
    High Level function to trigger/call the build functions for:
    - build non-safe part of TGM (Message part)
    - build safe part of TGM (SPDU)

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrTx_BuildAsmTgm (void)
{
  switch (aicSm_eAicState)
  {
    case AICSM_AIC_EXEC_PROD_CONS:  /* fall through */
    case AICSM_AIC_EXEC_PROD_ONLY:  /* fall through */
    case AICSM_AIC_EXEC_CONS_ONLY:  /* fall through */
    {
      /* build the non-safe message response/request */
      aicMsgBld_BuildMsg();
      /* build packet containing IO Data Message and Time Coordination Message
       * (see [SRS_2009], [SIS_016])
       */
      spduIn_BuildTgm();
      break;
    }
    
    case AICSM_AIC_WAIT_FOR_INIT:   /* fall through */
    case AICSM_AIC_START_CSS:       /* fall through */
    case AICSM_AIC_WAIT_TUNID:      /* fall through */
    case AICSM_AIC_WAIT_FOR_CONFIG: /* fall through */
    case AICSM_AIC_PROCESS_CONFIG:  /* fall through */
    case AICSM_AIC_INVALID_CONFIG:  /* fall through */
    case AICSM_AIC_IDLE:            /* fall through */
    case AICSM_AIC_WAIT_RESET:      /* fall through */
    case AICSM_AIC_ABORT:           /* fall through */
    {
      /* build the non-safe message response/request */
      aicMsgBld_BuildMsg();
      /* just build dummy (empty) packet, see [SRS_2026], [SIS_016] */
      /* Because no SPDU is built, no CIP Safety Communication is possible here
       * (see [SRS_2002], [SRS_2197], [SRS_2199], [SRS_2047], [SRS_2205],
       * [SRS_2217])
       */
      spduIn_BuildTgmDummy();
      break;
    }
    
    /* function shall not be called in AIC state AICSM_AIC_STARTUP,
    ** the startup telegram is build in aicMgrTx_MergeSendStartupTgm
    ** directly */
    case AICSM_AIC_STARTUP:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
      break;
    }
    default:
    {
       GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
       break;
    }
  }
}

/***************************************************************************************************
  Function:
    aicMgrTx_MergeSendAsmTgm

  Description:
    This function merges the AIC telegram together and send it out.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrTx_MergeSendAsmTgm (void)
{
  /* merge/join the several packets to the DMA Buffer */
  MergeBuffer();
  /* trigger sending message */
  aicUart_TriggerSending();
}

/***************************************************************************************************
  Function:
    aicMgrTx_MergeSendStartupTgm

  Description:
    In STARTUP mode this function initiates a build of the startup telegram
    and sends it in the specified cycle.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrTx_MergeSendStartupTgm (void)
{
  UINT32 u32Time;
  UINT32 u32TimeDiff;

  /* calculate the elapsed time, for the calculation an underflow of the
   * UINT32 is wanted */
  u32Time = timerHAL_GetSystemTime3();
  u32TimeDiff = (UINT32)(u32Time - u32_LastTime);

  /* if time for new startup telegram */
  if (u32TimeDiff >= k_STARTUP_TELEGRAM_DELAY)
  {
    /* build special startup telegram (see [SRS_568], [SRS_426]) */
    BuildStartupTgm();
    /* send special startup telegram */
    aicUart_TriggerSendingStartup();

    u32_LastTime = u32Time;
  }
  /* else: still time to wait */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    aicMgrTx_SampleIoData

  Description:
    High Level function to trigger/call the build functions for:
    - get IO data from DI/DO module
    - store the information in module according to the "AIC IO data layout"
    Description of "AIC IO data layout" can be found in SRS.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrTx_SampleIoData (void)
{
  UINT8 u8_idx;

  /* default init: set IO data variables to fail-safe values */
  /* all DIs to not active (safe state) */
  aicMgrTx_s_IoVal.u8DiData = (UINT8)0u;
  /* all DI qualifiers to "0" (error state) */
  aicMgrTx_s_IoVal.u8DiQualifiers = (UINT8)0u;
  /* all DO qualifiers to "0" (error state) */
  aicMgrTx_s_IoVal.u8DoQualifiers = (UINT8)0u;
#ifdef RFID_ACTIVE
  aicMgrTx_s_IoVal.u32_rfidInfo = RFID_InfoGet();
#endif
  /************************************************/
  /* get DI state, see [SRS_2025] */
  /************************************************/
  SampleDiStates();

  /************************************************/
  /* get DI qualifier, see [SRS_2025] */
  /************************************************/
  for (u8_idx = 0u; u8_idx < GPIOCFG_NUM_DI_TOT; u8_idx++)
  {
    /********************************************************************************/
    /* get DI qualifier and set value to send accordingly, see [SRS_2011], [SRS_672] */
    /********************************************************************************/
    /* dual channel configuration of DI */
    if (FIPARAM_DI_DUAL_CH(DICFG_GET_DUAL_CH_NUM(u8_idx)))
    {
      /* get DI qualifier and set value to be sent accordingly
       * (see [SRS_2055], [SRS_2056], [SRS_2013], [SRS_2139]) */
      if (DI_ERR_IS_TEST_QUAL_OK_OR_NA(u8_idx) && \
          DI_ERR_IS_TEST_QUAL_OK_OR_NA(DICFG_GET_OTHER_CHANNEL(u8_idx)))
      {
        aicMgrTx_s_IoVal.u8DiQualifiers |= (UINT8)(0x01u << u8_idx);
      }
      else
      {
        /* empty branch */
      }
    }
    /* single channel configuration of DI */
    else
    {
      /* get DI qualifier and set value to be sent accordingly
       * (see [SRS_2012], [SRS_2055], [SRS_2056], [SRS_2138]) */
      if (DI_ERR_IS_TEST_QUAL_OK_OR_NA(u8_idx))
      {
        aicMgrTx_s_IoVal.u8DiQualifiers |= (UINT8)(0x01u << u8_idx);
      }
      else
      {
        /* empty branch */
      }
    }
  }

  /****************************************************************************/
  /* get DO qualifier and set value to send accordingly, see [SRS_2015], [SRS_673],
   * [SRS_2025], [SRS_2056] */
  /****************************************************************************/
  for (u8_idx = 0u; u8_idx < GPIOCFG_NUM_DO_TOT; u8_idx++)
  {
    /* dual channel configuration of DO (see [SRS_339], [SRS_153]) */
    if (FIPARAM_DO_DUAL_CH(u8_idx))
    {
      /* check DO qualifier (see [SRS_2019]) */
      if (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8_idx) && \
          DO_ERR_IS_TEST_QUAL_OK_OR_NA(DOCFG_GET_OTHER_CHANNEL(u8_idx)))
      {
        aicMgrTx_s_IoVal.u8DoQualifiers |= (UINT8)(0x01u << u8_idx);
      }
      else
      {
        /* empty branch */
      }
    }
    /* single channel configuration of DO */
    else
    {
      /* check DO qualifier (see [SRS_2018]) */
      if (DO_ERR_IS_TEST_QUAL_OK_OR_NA(u8_idx))
      {
        aicMgrTx_s_IoVal.u8DoQualifiers |= (UINT8)(0x01u << u8_idx);
      }
      else
      {
        /* empty branch */
      }
    }
  }
}

/***************************************************************************************************
  Function:
    aicMgrTx_ResetIoData

  Description:
    This function sets the IO Data variable to fail-safe values:
      - all DIs to not active (safe state)
      - all DI qualifiers to "0" (error state)
      - all DO qualifiers to "0" (error state)

    This function shall be called if DI/DO modules are not active!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrTx_ResetIoData (void)
{
  /* default init: set IO data variables to fail-safe values */
  /* all DIs to not active (safe state) */
  aicMgrTx_s_IoVal.u8DiData = (UINT8)0u;
  /* all DI qualifiers to "0" (error state) */
  aicMgrTx_s_IoVal.u8DiQualifiers = (UINT8)0u;
  /* all DO qualifiers to "0" (error state) */
  aicMgrTx_s_IoVal.u8DoQualifiers = (UINT8)0u;
#ifdef RFID_ACTIVE
  aicMgrTx_s_IoVal.u32_rfidInfo = 0uL;
#endif

  /* all SafeBound values to "FALSE" */
  doSafeBoundSS1t_SetSafeBoundVal((UINT8)0u, (BOOL)FALSE);
  doSafeBoundSS1t_SetSafeBoundVal((UINT8)1u, (BOOL)FALSE);
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    MergeBuffer

  Description:
    merges the different parts of the TX-Telegram (see [SRS_2020]) into the
    TX-DMA buffer
    (after this step the DMA buffer is "ready to be sent"):
    - Ctrl/Status-Byte of ASM
    - Non-safe Message part (TX)
    - Safe Message part (SPDU, safe In-Packet)
    - Safe IO-Data in extra non-safe part
    - CRC of complete UART telegram
    The endian format of the telegram shall be little endian (see [SIS_052]).

    Former T100/PS function aicSplitMerge_MergeTxBuffer()

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void MergeBuffer (void)
{
  /* Ctrl/Status Byte of ASM */
  aicUart_sAicTxDmaBuffer.u8CtrlStatus = aicMsgCtrlSts_u8_AsmCtrlSts;
  /* Non-Safe message part of ASM */
  aicUart_sAicTxDmaBuffer.sAsmMsg = aicMsgBld_s_SendMsg;
  /* SPDU, safe input packet */
  aicUart_sAicTxDmaBuffer.sSpdu = spduIn_s_Paket;
  /* Safe IO input and output data in additional non-safe part, see
   * [SRS_2032], [SRS_2033] */
  aicUart_sAicTxDmaBuffer.sIoInData  = aicMgrTx_s_IoVal;

  /* get the values for the virtual DIs (SS1-t delay signalization) */
  aicUart_sAicTxDmaBuffer.sIoInData.u8DiData |= doSafeBoundSS1t_ReqRampDown();

  aicUart_sAicTxDmaBuffer.sIoOutData = spduOutData_Get();
  /* CRC of whole telegram, see [SRS_418], [SRS_420], [SIS_019] */
  /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
  aicUart_sAicTxDmaBuffer.u16Crc = aicCrc_Crc16( (UINT8*)(&aicUart_sAicTxDmaBuffer),/*lint !e928*/
                                        (UINT32)(AICMSGDEF_ASM_TGM_SIZE - AICMSGDEF_ASM_CRC_SIZE));
}

/***************************************************************************************************
  Function:
    BuildStartupTgm

  Description:
    builds the special startup telegram (see [SRS_568]) and stores it to
    DMA-TX-Buffer.
    This startup telegram is a fixed telegram specified in SIS (see [SRS_427])
    which will be sent frequently during startup until a valid telegram is
    received from the other side.
    The endian format of the telegram shall be little endian (see [SIS_064]).

    Former T100/PS function aicSplitMerge_BuildStartupTgm()

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void BuildStartupTgm (void)
{
  AICMSGDEF_ASM_STARTUP_TELEGRAM_STRUCT* psTxDmaBuff;
  UINT32* pu32SerialNumInFlash;

  /* get the (fixed) flash address of the serial number */
  pu32SerialNumInFlash = (UINT32*)VERSION_SERIAL_NUM_FLASH_ADDR;
  
  /* set pointer to DMA-Buffer address */
  /* Cast of Pointer is OK */
  psTxDmaBuff = 
    (AICMSGDEF_ASM_STARTUP_TELEGRAM_STRUCT*)(&aicUart_sAicTxDmaBuffer);/*lint !e740 !e929*/

  psTxDmaBuff->u16AsmVendorId = VERSION_ASM_VENDOR_ID;  /* ASM Vendor Id: Identifies vendor  */
                                                        /* of the ASM module. 0x0001  HMS  */
  psTxDmaBuff->u16AsmModuleId = VERSION_ASM_MODULE_ID;  /* ASM Module Id:  Identifies the */
                                                        /* kind of ASM */

  psTxDmaBuff->au8AsmFwVers[0] = VERSION_SW_VERSION_PATCH; /* SW version first byte */
  psTxDmaBuff->au8AsmFwVers[1] = VERSION_SW_VERSION_MINOR; /* SW version second byte */
  psTxDmaBuff->au8AsmFwVers[2] = VERSION_SW_VERSION_MAJOR; /* SW version third byte */

  psTxDmaBuff->u32AsmSN = *pu32SerialNumInFlash; /* ASM serial number  */
                                                 /* 32-bit unique serial number. */

  psTxDmaBuff->u8SafeOutputPduSize = AICMSGDEF_ASM_SPDU_OUT_SIZE;  /* see [SRS_2144] */
  psTxDmaBuff->u8OutputDataSize    = AICMSGDEF_DATA_OUT_LEN;       /* see [SRS_2144] */

  psTxDmaBuff->u8SafeInputPduSize = AICMSGDEF_ASM_SPDU_IN_SIZE;    /* see [SRS_2144] */
  psTxDmaBuff->u8InputDataSize    = AICMSGDEF_DATA_IN_LEN;         /* see [SRS_2144] */

  /* calculate CRC */
  /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
  psTxDmaBuff->u16Crc = aicCrc_Crc16( ((UINT8*)(psTxDmaBuff)),/*lint !e928*/
                               ((UINT32)(AICMSGDEF_ASM_STARTUP_TGM_SIZE-AICMSGDEF_ASM_CRC_SIZE)) );
}

/***************************************************************************************************
  Function:
    SampleDiStates

  Description:
    Sub-function to trigger/call the build functions for:
    - get DI state data from DI/DO module
    - store the information in module according to the "AIC IO data layout"
    Description of "AIC IO data layout" can be found in SRS.

  See also:
    aicMgrTx_SampleIoData()

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC void SampleDiStates(void)
{
  UINT8 u8_idx;

  /* temporary set all SafeBound values to "TRUE" (might be corrected in the next steps) */
  doSafeBoundSS1t_SetSafeBoundVal((UINT8)0u, (BOOL)TRUE);
  doSafeBoundSS1t_SetSafeBoundVal((UINT8)1u, (BOOL)TRUE);

  /************************************************/
  /* get DI state, see [SRS_2025] */
  /************************************************/
  for (u8_idx = 0u; u8_idx < GPIOCFG_NUM_DI_TOT; u8_idx++)
  {
    /* ATTENTION: In contrast to the T100/PS implementation here the DIs are
    ** valid even if 'passivation' is requested. So the DIs can be read even if
    ** the Output Connection (Originator --> Target) is not established. */

    /* get state from DI module and set value to be sent accordingly
     * (see [SRS_2009], [SRS_2012], [SRS_2013], [SRS_273], [SRS_2054], [SRS_2056],
     * [SRS_2138], [SRS_2139]) */
    if (DICFG_DI_ACTIVE == diState_GetDiState(u8_idx))
    {
      aicMgrTx_s_IoVal.u8DiData |= (UINT8)(0x01u << u8_idx);
    }
    else
    {
      /* if SafeBound is configured */
      if (FIPARAM_DI_SAFEBOUND_ENABLE(u8_idx)) /*lint !e960*/
      {
        /* if one of the inputs is zero (not active) then the whole value for the SafeBound group is
           zero. That is the logical "AND" of the SafeBound function. See [SRS_901]. */
        /* if DI1 or DI3 or DI5 */
        if (    (u8_idx == 0u)
             || (u8_idx == 2u)
             || (u8_idx == 4u)
           )
        {
          /* DO1 */
          doSafeBoundSS1t_SetSafeBoundVal((UINT8)0u, (BOOL)FALSE);
        }
        else /* DI2 or DI5 or DI6 */
        {
          /* DO2 */
          doSafeBoundSS1t_SetSafeBoundVal((UINT8)1u, (BOOL)FALSE);
        }
      }
      else /* SafeBound is not configured */
      {
        /* no action is necessary */
      }
    }
  }
}

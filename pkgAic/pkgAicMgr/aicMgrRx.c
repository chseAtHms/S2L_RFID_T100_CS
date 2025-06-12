/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMgrRx.c
**     Summary: AIC-Manager Module containing the interface functions and high level functions
**              of the AIC communication module for AIC reception (RX)
**              The module is also responsible to split the received UART telegram in its parts:
**              - Ctrl/Status-Byte of AM/ABCC
**              - Non-safe Message part (RX)
**              - Safe Message part (SPDU, safe Out-Packet)
**              - CRC of complete UART telegram
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMgrRx_CheckForNewTgm
**             aicMgrRx_PrepAicSync
**             aicMgrRx_DoAicSync
**             aicMgrRx_ProcessTgm
**             aicMgrRx_GetOutputData
**
**             SplitAndCheckBuffer
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

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "includeDiDo.h"

#include "aicMsgDef.h"
#include "aicCrc.h"
#include "aicUart.h"
#include "globFit_FitTestHandler.h"

#include "aicMsgProc.h"
#include "aicMsgErr.h"
#include "aicMsgCtrlSts.h"

#include "aicMgrRx.h"

#include "aicSm.h"
#include "aicIoRstDi.h"
#include "aicIoRstDo.h"

#include "spduOutIo.h"
#include "spduOutTcoo.h"
#include "spduOutData.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      3u

/* counter for number of telegrams received, Overflow handled in SW */
STATIC UINT8 u8_NumTgm = (UINT8)0u;


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC BOOL SplitAndCheckBuffer (void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/


/***************************************************************************************************
  Function:
    aicMgrRx_CheckForNewTgm

  Description:
    calls functions to check if there was a new RX telegram received.
    If yes, functions to check (CRC) and split the message are called.
    In case of an error in CRC, function to handle an "invalid" telegram
    is called

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_CheckForNewTgm (void)
{
  /* if new telegram received */
  if (TRUE == aicUart_NewTgmReceived())
  {
    /* function call returns FALSE in case of a CRC error */
    if (FALSE == SplitAndCheckBuffer())
    {
      /* call err-function to handle invalid telegram, see [SRS_284], [SIS_021] */
      aicMsgErr_SerialError();
      /* Hint: Counter "u8_NumTgm" is not incremented here because
       * counter is counting the valid telegrams */
      /* The telegram is ignored, see [SIS_020], [SIS_022] */
    }
    else
    {
      /* indicator, that a valid message from AM (ABCC) was received */
      aicSm_eFirstMsgReceived = eTRUE;
      /* increase counter for correct telegram reception, overflow handled
       * in function "aicMgrRx_DoAicSync" */
      u8_NumTgm++;
    }
  }
  /* else: no new telegram received */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    aicMgrRx_PrepAicSync

  Description:
    Calls function to transfer the current number of received telegrams to
    the module which will send the value via IPC.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_PrepAicSync (void)
{
  /* call function to hand over IPC transmission value */
  gpio_SetIpcAicSyncTx((UINT8)u8_NumTgm);
}

/***************************************************************************************************
  Function:
    aicMgrRx_DoAicSync

  Description:
    Gets the number of received telegrams of the other channel (received
    via IPC).
    If the other channel received one telegram more, than the AIC-RX buffer
    is again checked for a telegram.
    Else if both channels received the same number of telegrams, everything
    is fine.
    Else if the difference of the number of received telegrams is bigger than
    1, the fail safe handler is called because one uC missed more than one
    telegram.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_DoAicSync (void)
{
  UINT8 u8_numTgmRxIpcEx;

  /* get AIC sync info from IPC RX buffer */
  u8_numTgmRxIpcEx = gpio_GetIpcAicSyncRx();

  /* if both uCs have same number of telegrams received */
  if (u8_NumTgm == u8_numTgmRxIpcEx)
  {
    /* standard case, nothing to do... */
  }
  /* if other uC received one telegram less, note: casting important to handle
   * overflow (so that "0xFF+1" equals "0x00" and not "0x100") */
  else if (u8_NumTgm == (UINT8)(u8_numTgmRxIpcEx + (UINT8)1u))
  {
    /* do nothing, because other uC is one RX telegram "behind" */
  }
  /* other uC has received one telegram more, note: casting important to handle
   * overflow (so that "0xFF+1" equals "0x00" and not "0x100") */
  else if ((UINT8)(u8_NumTgm + (UINT8)1u) == u8_numTgmRxIpcEx)
  {
    /* "behind" other uC, try to catch Telegram again */
    aicMgrRx_CheckForNewTgm();

    /* is the number of received telegrams still different? */
    if (u8_NumTgm != u8_numTgmRxIpcEx)
    {
      /* error: uCs are working with different AIC-messages  */
      GLOBFAIL_SAFETY_HANDLER(GlOB_FAILCODE_AIC_SYNC_ERR, GLOBFAIL_ADDINFO_FILE(1u));
    }
    /* else: number OK now */
    else
    {
      /* empty branch */
    }
  }
  else
  {
    /* all other constellations should never occur */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }
}

/***************************************************************************************************
  Function:
    aicMgrRx_ProcessTgm

  Description:
    calls the necessary functions to process an incoming message

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_ProcessTgm (void)
{
  /* synchronize/update application buffers accessed by safety application */
  spduOutIo_Sync();
  spduOutTcoo_Sync();

  /* call function to process non-safe part of received telegram */
  aicMsgProc_ProcessMsg();
}

/***************************************************************************************************
  Function:
    aicMgrRx_GetOutputData

  Description:
    High Level function to process the received data if device is in EXCHANGE mode.
    Additionally automatic error reset feature is handled here.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMgrRx_GetOutputData (void)
{

  /* handle automatic error reset for the DI/DO module (see [SRS_2011], [SRS_2015]).
  ** If DI/DO is configured for the automatic error reset feature, the received error reset flags
  ** inside the SPDU are ignored in function spduOutData_Process() (see [SRS_2223], [SRS_2224]).
  */  
  aicIoRstDi_ChkAutoErrRst();
  aicIoRstDo_ChkAutoErrRst();

  /* process output packet only in EXECUTING state, see [SRS_2049], [SIS_017].
  ** Consuming Connection is established, so get the data received via CIP Safety packet.
  ** Note: The Anybus state 'R_RUN' (mentioned in [SIS_017], used for PROFINET) corresponds here
  ** directly to 'EXEC'. */
  /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical 
   *  operator: '||' --> no side effects because state only changed from IRQ context so 
   * reading twice is safe */
  if ( (AICSM_AIC_EXEC_CONS_ONLY == aicSm_eAicState) ||
       (AICSM_AIC_EXEC_PROD_CONS == aicSm_eAicState) ) /*lint !e960 */
  {
    /* process output data depending on RUN/IDLE flag */
    spduOutData_Process();
  }
  /* else: in other AIC state the outputs are disabled */
  else
  {
    /* Attention: aicMgrRx_GetOutputData() is called in RUN-Scheduler and so in
    ** IDLE state. Here it is ensured that the DOs are disabled (see [SRS_2005]).
    */
    /* request disable of outputs */
    doLib_PassAllDOs();
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SplitAndCheckBuffer

  Description:
    - checks if the telegram received is correct (CRC check)
    - If the CRC is correct, the RX-Telegram is splitted into the different
      parts (see [SRS_2021]) and stored on the data stack (global variables/structs):
       - Ctrl/Status-Byte of AM/ABCC
       - Non-safe Message part (RX)
       - Safe Message part (SPDU, safe Out-Packet)
    - calls DMA-RX-Init function to prepare reception of next telegram

    Former T100/PS function aicSplitMerge_SplitAndCheckRxBuffer()

  See also:
    -

  Parameters:
    -

  Return value:
     bRetVal -           TRUE:  Telegram reception is correct
                         FALSE: Telegram is receipt not correct

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
STATIC BOOL SplitAndCheckBuffer (void)
{
  UINT16  u16CalcCRC;
  BOOL    bRetVal = FALSE;

  /* FIT to manipulate the AIC telegram to get a crc error */
  GLOBFIT_FITTEST(GLOBFIT_CMD_AIC_CRC, aicUart_sAicRxDmaBuffer.sAmMsg.au8AicMsgData[0u] ^= 0x01u);

  /* check the received messages CRC, see [SRS_418], [SRS_420], [SIS_019] */
  /* Deactivated PC-Lint Message 928 because cast from pointer to pointer is OK. */
  u16CalcCRC = aicCrc_Crc16( (UINT8*)(&aicUart_sAicRxDmaBuffer),/*lint !e928*/
                          (UINT32)(AICMSGDEF_AM_TGM_SIZE - AICMSGDEF_ASM_CRC_SIZE));

  if (u16CalcCRC == aicUart_sAicRxDmaBuffer.u16Crc)
  {
    bRetVal = TRUE;

    /* CRC check successful, transfer/copy received data to data stack */
    /* store CTRL/STatus Byte of AIC */
    aicMsgCtrlSts_u8_AmCtrlSts = aicUart_sAicRxDmaBuffer.u8CtrlStatus;
    /* store non-safe message part of AIC */
    aicMsgProc_s_RcvdMsg = aicUart_sAicRxDmaBuffer.sAmMsg;
    /* store safe message part of AIC */
    spduOut_s_Paket = aicUart_sAicRxDmaBuffer.sSpdu;
  }
  else
  {
    bRetVal = FALSE;
  }

  /* re-initialize DMA reading for next cycle */
  aicUart_DmaInitRx();

  return bRetVal;
}

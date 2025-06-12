/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMgrTx.h
**     Summary: Interface header of module 'aicMgrTx.c'
**   $Revision: 4453 $
**       $Date: 2024-06-05 16:00:00 +0200 (Mi, 05 Jun 2024) $
**      Author:
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
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMGRTX_H
#define AICMGRTX_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMgrTx_s_IoVal:
** structure to store the data to send via safe protocol (see [SRS_2012], [SRS_2013],
** input data only, not the safe packet), global used due to performance reasons
** formerly aicDataStackSpdu_sTxSpduInData
*/
extern volatile AICMSGDEF_PS_INPUT_DATA_STRUCT aicMgrTx_s_IoVal;

/***************************************************************************************************
**    function prototypes
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
void aicMgrTx_SysInit (void);

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
void aicMgrTx_BuildAsmTgm (void);

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
void aicMgrTx_MergeSendAsmTgm (void);

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
void aicMgrTx_MergeSendStartupTgm (void);

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
void aicMgrTx_SampleIoData (void);

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
void aicMgrTx_ResetIoData (void);

#endif


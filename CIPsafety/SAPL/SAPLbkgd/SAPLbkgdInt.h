/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdInt.h
**     Summary: Internal Interface for synchronization of the Background Task.
**   $Revision: 2870 $
**       $Date: 2017-04-28 08:30:24 +0200 (Fr, 28 Apr 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskInit
**             SAPL_BkgdTaskExecuting
**             SAPL_BkgdTaskIdle
**             SAPL_BkgdTaskConfig
**             SAPL_BkgdTaskTunid
**             SAPL_BkgdTaskReset
**             SAPL_BkgdTaskAbort
**             SAPL_BkgdComIoCfgApply
**             SAPL_BkgdComActivityMonitor
**             SAPL_BkgdComSetSysTime
**             SAPL_BkgdComInitFaultCnt
**             SAPL_BkgdComTxFrameGenerate
**             SAPL_BkgdComRxProcessHalcMsg
**             SAPL_BkgdComDiscardHalcMsg
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_BKGD_INT_H
#define SAPL_BKGD_INT_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskInit

  Description:
    This function is used to initialize the IXXAT CIP Safety Target Stack. It is required that the
    CSS initialization data were already received via 'SetInitData' request from AM.
    This means, the AIC state AICSM_AIC_START_CSS is expected here.
  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskInit(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskExecuting

  Description:
    This function is used to execute the functionality in Executing state

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskExecuting(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskIdle

  Description:
    This function is used to execute the functionality in Idle state

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskIdle(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskConfig

  Description:
    This function is used to execute the functionality in configuration mode

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskConfig(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskTunid

  Description:
    This function is used to execute the functionality in wait tunid 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskTunid(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskReset

  Description:
    This function is used to execute the functionality in 'WAIT_RESET'. 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskReset(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdTaskAbort

  Description:
    This function is used to execute the functionality in 'ABORT'

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdTaskAbort(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComIoCfgApply

  Description:
    This function is used to apply a valid IO Configuration. For this purpose the function:
      - reads the current IO Configuration Data + SCID from non-volatile memory
      - calculates the SCCRC over IO Configuration Data 
      - compares calculated SCCRC with SCCRC stored inside non-volatile memory
      - sets IO Configuration Data inside pkgParam 
      - sets SCCRC inside pkgParam (for cyclic check)

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComIoCfgApply(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComActivityMonitor

  Description:
    This function performs the Consumer Activity Monitoring [3.9-2:]. This checks the reception
    of IO Data Messages. Furthermore the generation of Time Coordination Messages are initiated
    here.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComActivityMonitor(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComSetSysTime

  Description:
    This function sets the current system time of the CSS stack [3.15-1:] and handles the
    reset of the fault counters [3.7-2:].

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComSetSysTime(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComInitFaultCnt

  Description:
    This function initializes the timer used to reset the fault timers. This function is typically
    called once within startup.

  See also:
    -

  Parameters:
    u32_sysTime (IN)         - Current system time (128us ticks)
                               (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComInitFaultCnt(UINT32 u32_sysTime);

/***************************************************************************************************
  Function:
    SAPL_BkgdComTxFrameGenerate

  Description:
    This function is used to trigger the generation of new IO Data Messages. Therefore the
    Input Assembly is updated additionally here.
    Furthermore the reception of Time Coordination Messages are checked inside.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComTxFrameGenerate(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComRxProcessHalcMsg

  Description:
    This function checks if new HALC message was received. If so, the message is passed to the 
    proper module of the CSS.
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComRxProcessHalcMsg(void);

/***************************************************************************************************
  Function:
    SAPL_BkgdComDiscardHalcMsg

  Description:
    This function discards a pending HALCS Message 
 

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_BkgdComDiscardHalcMsg(void);

#endif


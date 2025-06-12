/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdIdle.c
**     Summary: This module implements the background task functionality in the state 'IDLE'.
**   $Revision: 2412 $
**       $Date: 2017-03-23 08:32:18 +0100 (Do, 23 Mrz 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskIdle
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "xtypes.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicMsgDef.h"
#include "spduOutIo.h"
#include "spduOutTcoo.h"

/* SAPL headers */
#include "SAPLbkgdInt.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

  
/***************************************************************************************************
**    global functions
***************************************************************************************************/


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
void SAPL_BkgdTaskIdle(void)
{
  /* This background task is called once within T-cyc:asm (4ms). This means the CSS API
  ** is triggered as fast to achieve RPI of 4ms (see [SRS_2146], [SRS_2130]).
  ** Attention: No processing of IO Data Messages and Time Coordination Messages from
  ** SPDU is done here, means no cyclic CIP Safety communication possible (see [SRS_2047]).
  */

  
  /* set current system time of CSS and handle reset of the fault counters */
  SAPL_BkgdComSetSysTime();
  
  /* The data buffers, containing the last received IO Data Message and Time Coordination 
  ** Message, are updated regardless of the AIC state if the DUI/DUI2 has changed (in 
  ** spduOutIo_Sync(), spduOutTcoo_Sync()). To avoid that 'old' Messages are processed 
  ** later when entering 'EXECUTING' state, the Messages are discarded (marked as consumed) 
  ** here actively by calling spduOutIo_NewMsgGet() respectively spduOutTcoo_NewMsgGet().
  ** This keeps the DUI/DUI2 inside aicSpdu up-to-date. 
  */
  /* Discard/Ignore IO Data Message (inside SPDU part) */
  (void)spduOutIo_NewMsgGet();
  /* Discard/Ignore Time Coordination Message (inside SPDU part) */
  (void)spduOutTcoo_NewMsgGet();

  /* IXSVS_ConsInstActivityMonitor() shall be called cyclically (see [3.9-2:]),
  ** so done here (inside SAPL_BkgdComActivityMonitor()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComActivityMonitor();

  /* IXSVC_InstTxFrameGenerate() shall be called cyclically (see [3.8-1:]),
  ** so done here (inside SAPL_BkgdComTxFrameGenerate()) although no SPDU is processed 
  ** in this state. */
  SAPL_BkgdComTxFrameGenerate();

  /******************************************************************************/
  /* Processing of HALC (CSAL) Message                                          */
  /******************************************************************************/
  /* Process HALC message, only a single message is processed here */
  SAPL_BkgdComRxProcessHalcMsg();
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

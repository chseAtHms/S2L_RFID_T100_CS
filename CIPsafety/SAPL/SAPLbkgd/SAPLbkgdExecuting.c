/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLbkgdExecuting.c
**     Summary: This module implements the background task functionality in the state 'EXECUTING'.
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_BkgdTaskExecuting
**
**             BkgdTaskRxIoFrameProcess
**             BkgdTaskRxTcooFrameProcess
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
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicMsgDef.h"

#include "aicSm.h"

#include "spduOutIo.h"
#include "spduOutTcoo.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* HALCS headers */
#include "HALCSapi.h"

/* CSS headers */
#include "IXSVOapi.h"

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

STATIC void BkgdTaskRxIoFrameProcess(void);
STATIC void BkgdTaskRxTcooFrameProcess(void);
  
/***************************************************************************************************
**    global functions
***************************************************************************************************/


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
void SAPL_BkgdTaskExecuting(void)
{
  /* This background task is called once within T-cyc:asm (4ms). This means the CSS API
  ** is triggered as fast to achieve RPI of 4ms (see [SRS_2146], [SRS_2134], [SRS_2135]).
  */

  /* set current system time of CSS and handle reset of the fault counters */
  SAPL_BkgdComSetSysTime();
  
  /*****************************************************************************/
  /* Trigger RX part (processing IO Data Message, Time Coordination Message)   */
  /*****************************************************************************/
  /* Reception of IO Data Message */
  BkgdTaskRxIoFrameProcess();
  /* reception of Time Coordination Message */
  BkgdTaskRxTcooFrameProcess();
  
  /******************************************************************************/
  /* Trigger TX part (generation of IO Data Message, Time Coordination Message) */
  /* and RX check part                                                          */
  /******************************************************************************/
  /* Check reception of IO Data Messages and initiate generation of
  ** Time Coordination Messages
  */
  SAPL_BkgdComActivityMonitor();

  /* Initiate generation of IO Data Messages and check reception of
  ** Time Coordination Messages
  */
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


/***************************************************************************************************
  Function:
    BkgdTaskRxIoFrameProcess

  Description:
    This function is used to process new IO Data Messages received from ABCC for the 
    Consuming Connection.
    
  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskRxIoFrameProcess(void)
{ 
  /* IO Data Message */
  volatile SPDUOUTIO_t_MSG* ps_ioDataMsg;
  /* current AIC state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /*******************************/
  /* For IO safety data consumer */
  /*******************************/
  /* Check for new IO Data Message, (see [2.1-6:]). Done even if Consumer Connection is not active
  ** to keep the Data Update Indicator inside aicSpdu up-to-date */
  ps_ioDataMsg = spduOutIo_NewMsgGet();
  
  /* if Consumer Connection is active */ 
  if ( (AICSM_AIC_EXEC_PROD_CONS == e_aicState) ||
       (AICSM_AIC_EXEC_CONS_ONLY == e_aicState) )
  {  
    /* if new IO Data Message received */
    if (ps_ioDataMsg != NULL)
    { 
      /* HALCS message which shall be processed by CSS */
      HALCS_t_MSG s_rxHalcMsg;
      
      /* HALC messages sent from ABCC to T100 (IO Data Message, Time Coordination Message)
      ** are sent directly within the SPDU (see [SIS_201]). Because the type of the HALC 
      ** command is known by the SPDU location, the identifier CSOS_k_CMD_IXCO_IO_DATA 
      ** is not needed */
      /* ATTENTION: The T100 does not check here if the received IO Data Message is valid 
      ** (e.g. open connection, valid data (see [SIS_210], [SIS_211]). It is task of the CSS to 
      ** interpret the data. */   
      
      /* prepare HALC message header for CSS API call */
      s_rxHalcMsg.u16_cmd     = CSOS_k_CMD_IXCO_IO_DATA; /* known because of location in SPDU */
      s_rxHalcMsg.u16_len     = ps_ioDataMsg->u16_len;
      s_rxHalcMsg.u32_addInfo = ps_ioDataMsg->u32_addInfo;
      /* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away const/volatile
      ** from a pointer or reference 
      ** Note 926: cast from pointer to pointer [MISRA 2004 Rule 11.4]
      ** -->OK because data in au8_data is set via volatile access function stdlibHAL_ByteArrCopy,
      ** so data must be located in buffer */
      s_rxHalcMsg.pb_data     = (const CSS_t_BYTE*)ps_ioDataMsg->au8_data; /*lint !e960, !e926 */
      
      /* pass IO data to IXSVO_CmdProcess() (see [3.7-1:]) */
      /* data is accessed by callback function 'SAPL_IxsaiIoDataRxClbk' */
      IXSVO_CmdProcess(&s_rxHalcMsg);
    }
    /* else: no new IO Data Message received */
    else
    {
      /* empty branch */
    }
  }
  /* else: Consumer Connection not active */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    BkgdTaskRxTcooFrameProcess

  Description:
    This function is used to process new Time Coordination Messages received from ABCC for
    the Producing Connection.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC void BkgdTaskRxTcooFrameProcess(void)
{ 
  /* Time Coordination Message */
  volatile SPDUOUTTCOO_t_MSG* ps_tcooMsg;
  /* current AIC state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /*******************************/
  /* For IO safety data producer */
  /*******************************/
  /* Check for new Time Coordination Message, (see [2.1-6:]). Done even if Producer Connection is
  ** not active to keep the Data Update Indicator inside aicSpdu up-to-date */
  ps_tcooMsg = spduOutTcoo_NewMsgGet();
  
  /* if Producer Connection is active */ 
  if ( (AICSM_AIC_EXEC_PROD_CONS == e_aicState) ||
       (AICSM_AIC_EXEC_PROD_ONLY == e_aicState) )
  {
    /* if new Time Coordination Message received */
    if (ps_tcooMsg != NULL)
    {
      /* HALCS message which shall be processed by CSS */
      HALCS_t_MSG s_rxHalcMsg;
      
      /* HALC messages sent from ABCC to T100 (IO Data Message, Time Coordination Message)
      ** are sent directly within the SPDU (see [SIS_201]). Because the type of the HALC 
      ** command is known by the SPDU location, the identifier CSOS_k_CMD_IXCO_IO_DATA 
      ** is not needed */
      /* ATTENTION: The T100 does not check here if the received Time Coordination Message is valid 
      ** (e.g. open connection, valid data (see [SIS_210], [SIS_211]). It is task of the CSS to 
      ** interpret the data. */   
      
      /* prepare HALC message header for CSS API call */
      s_rxHalcMsg.u16_cmd     = CSOS_k_CMD_IXCO_IO_DATA; /* known because of location in SPDU */
      s_rxHalcMsg.u16_len     = ps_tcooMsg->u16_len; 
      s_rxHalcMsg.u32_addInfo = ps_tcooMsg->u32_addInfo;
      /* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away const/volatile
      ** from a pointer or reference 
      ** Note 926: cast from pointer to pointer [MISRA 2004 Rule 11.4]
      ** -->OK because data in au8_data is set via volatile access function stdlibHAL_ByteArrCopy,
      ** so data must be located in buffer */
      s_rxHalcMsg.pb_data     = (const CSS_t_BYTE*)ps_tcooMsg->au8_data; /*lint !e960, !e926*/
      
      /* pass Time Coordination Message to IXSVO_CmdProcess() (see [3.7-1:]) */
      IXSVO_CmdProcess(&s_rxHalcMsg);
    }
    /* else: no new Time Coordination Message received */
    else
    {
      /* empty branch */
    }
  }
  /* else: Producer Connection is not active */
  else
  {
    /* empty branch */
  }
}

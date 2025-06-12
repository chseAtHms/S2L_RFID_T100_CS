/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutTcoo.c
**     Summary: The module is used to synchronize the received SPDU (as part of the AIC telegram) 
**              with the background task.
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: spduOutTcoo_Init
**             spduOutTcoo_Sync
**             spduOutTcoo_NewMsgGet
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

#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "stdlib-hal.h"

/* application includes */
#include "aicMsgDef.h"
#include "spduOutIo.h"
#include "spduOutTcoo.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* New Time Coordination Message received */
#define k_TCOO_NEW        (UINT8)0x5Au
/* Data consumed by the application */
#define k_TCOO_CONSUMED   (UINT8)0xA5u

/* u8_LastTcooMsgDui:
** variable indicates the Update Indicator of the last received 
** Time Coordination Message
*/
STATIC RDS_UINT8 u8_LastTcooMsgDui;

/* s_LastTcooMsg:
** variable contains the last received Time Coordination Message
*/
STATIC volatile SPDUOUTTCOO_t_MSG  s_LastTcooMsg;

/* u8_TcooMsgStatus:
** variable contains the current status of the Time Coordination Message.
** k_TCOO_NEW means that a new Time Coordination Message was received (not 
** consumed by application yet, k_TCOO_CONSUMED mean that the application 
** has consumed the data. */
STATIC volatile RDS_UINT8 u8_TcooMsgStatus;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    spduOutTcoo_Init

  Description:
    This function is used to initialize the module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void spduOutTcoo_Init(void)
{
  /* init RDS variables of the module */
  RDS_SET(u8_LastTcooMsgDui, 0u);
  RDS_SET(u8_TcooMsgStatus, k_TCOO_CONSUMED);
}



/***************************************************************************************************
  Function:
    spduOutTcoo_Sync

  Description:
    This function is used to update/synchronize the data buffer which is accessed by the safety
    application to pass the Time Coordination Message to the CIP Safety Stack. 
    For this purpose the function checks the Data Update Indicator (DUI2) of the SPDU and 
    compares them with the DUI2 of the last read Time Coordination Message.
    If different, the data buffer is updated. AFTERWARDS the safety application can access the 
    corresponding data buffer.

    ATTENTION: It must be ensured, that the safety application has finished its access to the data
    buffers until next call to this function!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduOutTcoo_Sync(void)
{
  /* if new 'Time Coordination Message' was received (means Data Update Indicator updated) */
  if (spduOut_s_Paket.u8_duiData2 != RDS_GET(u8_LastTcooMsgDui))
  {
    /* get Time Coordination Message header from SPDU */
    s_LastTcooMsg.u16_len      = AICMSGDEF_CIP_OUT_DATA2_SIZE;
    s_LastTcooMsg.u32_addInfo  = spduOut_s_Paket.u16_halcAddInfo2;
    /* get Time Coordination Message payload from SPDU */
    stdlibHAL_ByteArrCopy(s_LastTcooMsg.au8_data, 
                          spduOut_s_Paket.au8_data2, 
                          (UINT16)sizeof(s_LastTcooMsg.au8_data));
    /* Update DUI value of module */
    RDS_SET(u8_LastTcooMsgDui, spduOut_s_Paket.u8_duiData2);
    /* mark data as updated */
    RDS_SET(u8_TcooMsgStatus, k_TCOO_NEW);
  }
  /* no new data */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    spduOutTcoo_NewMsgGet

  Description:
    This function is used to get a new Time Coordination Message from SPDU (if available), 
    (see [2.1-6:]).

    ATTENTION: It must be ensured, that the safety application has finished its access to the 
    received data buffers until next call to spduOutTcoo_Sync!

  See also:
    -

  Parameters:
    -

  Return value:
    NULL        - No new Time Coordination Message received since last call
    <>NULL      - Pointer to SPDUOUTTCOO_t_MSG which contains the Time Coordination Message.

  Remarks:
    Context: Background Task

***************************************************************************************************/
volatile SPDUOUTTCOO_t_MSG* spduOutTcoo_NewMsgGet(void)
{
  /* return value */
  volatile SPDUOUTTCOO_t_MSG* ps_tcooMsg;
  
  /* ATTENTION: Variables accessed from IRQ and Background Task. Variables written only written 
  ** (from IRQ context) by calling spduOutTcoo_Sync(). This is only done when the Background Task is
  ** not active. So it is not necessary to disable IRQs here. */
  
  /* if new 'Time Coordination Message' was received */
  if (k_TCOO_NEW == RDS_GET(u8_TcooMsgStatus) )
  {
    /* assign HALC Time Coordination Message */
    ps_tcooMsg = &s_LastTcooMsg;
    /* clear update flag */
    RDS_SET(u8_TcooMsgStatus, k_TCOO_CONSUMED);
  }
  /* no new Time Coordination Message */
  else
  {
    /* assign NULL which means no new data available */
    ps_tcooMsg = NULL;
  }
  return ps_tcooMsg;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

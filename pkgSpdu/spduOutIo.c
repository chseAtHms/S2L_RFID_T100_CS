/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutIo.c
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
**  Functions: spduOutIo_Init
**             spduOutIo_Sync
**             spduOutIo_NewMsgGet
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

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* spduOut_s_Paket:
** location/array to store the incoming safe packet (host to device),
** global used due to performance reasons
** formerly aicDataStackSpdu_sRxSpduOutPaket
*/
AICMSGDEF_SPDU_RX_STRUCT spduOut_s_Paket;

/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* New IO Data Message received */
#define k_IODATA_NEW        (UINT8)0x5Au
/* Data consumed by the application */
#define k_IODATA_CONSUMED   (UINT8)0xA5u

/* u8_LastIoMsgDui:
** variable indicates the Update Indicator of the last received 
** IO Data Message
*/
STATIC RDS_UINT8 u8_LastIoMsgDui;

/* s_LastIoMsg:
** variable contains the last received IO Data Message
*/
STATIC volatile SPDUOUTIO_t_MSG s_LastIoMsg;

/* u8_IoMsgStatus:
** variable contains the current status of the IO Data Message.
** k_IODATA_NEW means that a new IO Data Message was received (not 
** consumed by application yet, k_IODATA_CONSUMED mean that the application 
** has consumed the data. */
STATIC volatile RDS_UINT8 u8_IoMsgStatus;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    spduOutIo_Init

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
void spduOutIo_Init(void)
{
  /* init RDS variables of the module */
  RDS_SET(u8_LastIoMsgDui, 0u);
  RDS_SET(u8_IoMsgStatus, k_IODATA_CONSUMED);
}


/***************************************************************************************************
  Function:
    spduOutIo_Sync

  Description:
    This function is used to update/synchronize the data buffer which is accessed by the safety
    application to pass the IO Data Message to the CIP Safety Stack. 
    For this purpose the function checks the Data Update Indicator (DUI) of the SPDU and 
    compares them with the DUI of the last read IO Data Message.
    If different, the corresponding data buffer is updated. AFTERWARDS the safety application can 
    access the data buffer.

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
void spduOutIo_Sync(void)
{
  /* if new 'Data Message' was received (means Data Update Indicator updated) */
  if (spduOut_s_Paket.u8_duiData != RDS_GET(u8_LastIoMsgDui))
  {
    /* get IO Data Message Header from SPDU */
    s_LastIoMsg.u16_len      = spduOut_s_Paket.u16_halcLen; /* shall be 16 bytes (2 + 14), */
                                                                /* see [SIS_CS] */
    s_LastIoMsg.u32_addInfo  = spduOut_s_Paket.u16_halcAddInfo;
    /* get IO Data Message payload from SPDU */
    stdlibHAL_ByteArrCopy(s_LastIoMsg.au8_data, 
                          spduOut_s_Paket.au8_data, 
                          (UINT16)sizeof(s_LastIoMsg.au8_data));
    /* Update DUI value of module */
    RDS_SET(u8_LastIoMsgDui, spduOut_s_Paket.u8_duiData);
    /* mark data as updated */
    RDS_SET(u8_IoMsgStatus, k_IODATA_NEW);
  }
  /* no new data */
  else
  {
    /* empty branch */
  }
}

/***************************************************************************************************
  Function:
    spduOutIo_NewMsgGet

  Description:
    This function is used to get a new IO Data Message from SPDU (if available), (see [2.1-6:]).

    ATTENTION: It must be ensured, that the safety application has finished its access to the 
    received data buffers until next call to spduOutIo_Sync!

  See also:
    -

  Parameters:
    -

  Return value:
    NULL        - No new IO Data Message received since last call
    <>NULL      - Pointer to SPDUOUTIO_t_MSG which contains the IO Data Message.

  Remarks:
    Context: Background Task

***************************************************************************************************/
volatile SPDUOUTIO_t_MSG* spduOutIo_NewMsgGet(void)
{
  /* return value */
  volatile SPDUOUTIO_t_MSG* ps_ioDataMsg;
  
  /* ATTENTION: Variables accessed from IRQ and Background Task. Variables written only written 
  ** (from IRQ context) by calling spduOutIo_Sync(). This is only done when the Background Task is
  ** not active. So it is not necessary to disable IRQs here. */
  
  /* if new 'IO Data Message' was received */
  if ( k_IODATA_NEW == RDS_GET(u8_IoMsgStatus) )
  {
    /* assign IO Data Message */
    ps_ioDataMsg = &s_LastIoMsg;
    /* clear update flag */
    RDS_SET(u8_IoMsgStatus, k_IODATA_CONSUMED);
  }
  /* no new IO Data Message */
  else
  {
    /* assign NULL which means no new data available */
    ps_ioDataMsg = NULL;
  }
  return ps_ioDataMsg;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

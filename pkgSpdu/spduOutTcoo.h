/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutTcoo.h
**     Summary: Interface header of the module 'spduOutTcoo.c'
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

#ifndef SPDUOUTTCOO_H
#define SPDUOUTTCOO_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/* Data Type used to transfer the Time Coordination Message from the received SPDU */
typedef struct
{
  UINT16 u16_len;
  UINT32 u32_addInfo;
  UINT8  au8_data[AICMSGDEF_CIP_OUT_DATA2_SIZE];
} SPDUOUTTCOO_t_MSG;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
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
void spduOutTcoo_Init(void);

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
void spduOutTcoo_Sync(void);

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
volatile SPDUOUTTCOO_t_MSG* spduOutTcoo_NewMsgGet(void);

#endif


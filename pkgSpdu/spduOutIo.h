/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutIo.h
**     Summary: Interface header of the module 'spduOutIo.c'
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

#ifndef SPDUOUTIO_H
#define SPDUOUTIO_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/

/* Data Type used to transfer the IO Data Message from the received SPDU */
typedef struct
{
  UINT16 u16_len;
  UINT32 u32_addInfo;
  UINT8  au8_data[AICMSGDEF_CIP_OUT_DATA_SIZE];
} SPDUOUTIO_t_MSG;

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* spduOut_s_Paket:
** location/array to store the incoming safe packet (host to device),
** global used due to performance reasons
** formerly aicDataStackSpdu_sRxSpduOutPaket
*/
extern AICMSGDEF_SPDU_RX_STRUCT spduOut_s_Paket;

/***************************************************************************************************
**    function prototypes
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
void spduOutIo_Init(void);

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
void spduOutIo_Sync(void);

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
volatile SPDUOUTIO_t_MSG* spduOutIo_NewMsgGet(void);

#endif


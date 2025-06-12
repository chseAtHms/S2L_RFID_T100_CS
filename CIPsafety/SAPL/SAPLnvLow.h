/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLnvLow.h
**     Summary: Interface for low level NV memory functionality required by the high level.
**   $Revision: 4226 $
**       $Date: 2024-01-24 11:34:11 +0100 (Mi, 24 Jan 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_NvLowAcquire
**             SAPL_NvLowRelease
**             SAPL_NvLowInit
**             SAPL_NvLowCrcSync
**             SAPL_NvLowCurrBlkRead
**             SAPL_NvLowHandler
**             SAPL_NvLowIsBusy
**             SAPL_NvLowFreeBlkGet
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_NVLOW_H
#define SAPL_NVLOW_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Storage Data Size of the Configuration Data (IO) */
#define SAPL_k_SDS_CONF_DATA_IO      (UINT16)AICMSGHDL_IOCONFIGDATA_LEN
/* Storage Data Size of the Output Cnxn Point Owners Attribute, 
** (must be big enough to hold also the padding bytes of the corresponding CSS structure) */
#define SAPL_k_SDS_OCPUNID           (UINT16)12U
/* Storage Data Size of the Safety Configuration ID Attribute,
** (must be big enough to hold also the padding bytes of the corresponding CSS structure) */
#define SAPL_k_SDS_SCID              (UINT16)12U
/* Storage Data Size of the Target's Unique Network ID Attribute,
** (must be big enough to hold also the padding bytes of the corresponding CSS structure) */
#define SAPL_k_SID_TUNID             (UINT16)12U
/* Storage Data Size of the Configuration UNID Attribute,
** (must be big enough to hold also the padding bytes of the corresponding CSS structure) */
#define SAPL_k_SID_CFUNID            (UINT16)12U

/***************************************************************************************************
**    data types
***************************************************************************************************/

/* Structure of all variables stored inside of the non-volatile memory of the device.
** (see [SRS_2185]).
** Please consider that padding bytes could be added. */
/*lint -esym(768, u8_res1) global struct member  
**'u8_res1' not referenced. Only used to prevent padding 
*/
typedef struct
{
  BOOL   b_alEnable;                              /* Alarm Enable */
  BOOL   b_wnEnable;                              /* Warning Enable */
  UINT8  au8_ocpunid[SAPL_k_SDS_OCPUNID];         /* OCPUNID */
  UINT8  au8_scid[SAPL_k_SDS_SCID];               /* SCID */
  UINT8  au8_tunid[SAPL_k_SID_TUNID];             /* TUNID */
  UINT8  au8_cfunid[SAPL_k_SID_CFUNID];           /* CFUNID */
  UINT8  au8_cfgDataIo[SAPL_k_SDS_CONF_DATA_IO];  /* Configuration Data IO */
  UINT8  u8_res1;
} SAPL_t_NVLOW_BLKDATA;

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* Default data that should be stored in NV memory in case of
** - a factory reset
** - if NV memory seems to be corrupted
** - first power-cycle (production test does not store default data)
*/
extern CONST SAPL_t_NVLOW_BLKDATA SAPLnvLow_DefaultData;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_NvLowAcquire

  Description:
    This function is used to block the access from the NV low level flash handler to the high level
    buffer. This means the application can write data to the buffer until the function 
    SAPL_NvLowRelease is called.

    ATTENTION: This function shall not be called if it is possible that SAPL_NvLowHandler() is
    called by IRQ scheduler!

  See also:
    SAPL_NvLowRelease

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvLowAcquire(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowRelease

  Description:
    This function is used to signalize, that new data was written into the high level buffer. This 
    means that the low level flash handler can now copy the data and write the data physically into
    nv memory.

    ATTENTION: This function shall not be called if it is possible that SAPL_NvLowHandler() is
    called by IRQ scheduler!

  See also:
    SAPL_NvLowAcquire

  Parameters:
    ps_blkData (IN)          - Block data that shall be written to the 'Parameters and 
                               Configuration' on-chip memory.
                               (valid range: <>NULL, not checked, only called by reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvLowRelease(CONST SAPL_t_NVLOW_BLKDATA* ps_blkData);

/***************************************************************************************************
  Function:
    SAPL_NvLowInit

  Description:
    This function is used to initialize the low level flash management of the device.
    Therefore the software checks the whole 'Parameters and Configuration' on-chip memory
    if a valid flash block was found. Valid means that the corresponding block status is set to 
    'valid' and additionally the CRC16 is correct.
    If no valid flash block is found, the functions stores the default data inside NV memory.
    
    The function implements furthermore the Flash Block Threshold management. If less than 
    k_PARAM_CONF_FLSH_BLK_THRESH blocks are free, the whole 'Parameters and Configuration' on-chip 
    memory is erased and the current flash block is re-stored at the beginning of this reserved 
    area.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_NvLowInit(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowCrcSync

  Description:
    This function is used to exchange the CRC16 of the current flash block with the other 
    controller.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_NvLowCrcSync(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowCurrBlkRead

  Description:
    This function is used to read the current (active) flash block of the 'Parameters and
    Configuration' on-chip memory.
    The software:
      - checks the block status of the flash block 
      - checks the data against the stored CRC16 (see [SRS_2177])

    ATTENTION: This function shall not be called if it is possible that SAPL_NvLowHandler() is
    called by IRQ scheduler!

  See also:
    -

  Parameters:
    -

  Return value:
    NULL           - Error, flash seems to be corrupted.
    <>NULL         - Data of current flash block

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST SAPL_t_NVLOW_BLKDATA* SAPL_NvLowCurrBlkRead(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowHandler

  Description:
    This function is the handler function to program new data into NV memory physically.
    The function:
      - checks if new data shall be written and makes a copy to the internal buffer
      - invalidates the old flash block
      - programs data 16-bit granular into NV memory
      - exchanges the CRC16 for validation

   Attention: It takes several calls to this function until all data is written into NV memory.

   Attention: This function shall only be called in time slices where no NV Memory access 
              through background task is possible!!!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void SAPL_NvLowHandler(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowIsBusy

  Description:
    This function delivers the state of the low level flash driver.
    It is not permitted to call this function if the flash driver is not initialized yet.

    ATTENTION: This function shall not be called if it is possible that SAPL_NvLowHandler() is
    called by IRQ scheduler!

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                             - Flash busy (pending flash access)
    FALSE                            - Flash ready (no pending flash access)

  Remarks:
    Context: Background Task

***************************************************************************************************/
BOOL SAPL_NvLowIsBusy(void);

/***************************************************************************************************
  Function:
    SAPL_NvLowFreeBlkGet

  Description:
    This function returns the number of free flash blocks until a restart of the device is
    necessary.

    ATTENTION: This function shall not be called if it is possible that SAPL_NvLowHandler() is
    called by IRQ scheduler!

  See also:
    -

  Parameters:
    -

  Return value:
    UINT16 - Number of free flash blocks until a restart of the device is necessary.

  Remarks:
    Context: Background Task

***************************************************************************************************/
UINT16 SAPL_NvLowFreeBlkGet(void);

#endif


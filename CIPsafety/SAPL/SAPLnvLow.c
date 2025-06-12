/***************************************************************************************************
**    Copyright (C) 2015-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLnvLow.c
**     Summary: This module implements the low level functions for the NV memory handling.
**   $Revision: 4413 $
**       $Date: 2024-05-02 15:03:13 +0200 (Do, 02 Mai 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_NvLowAcquire
**             SAPL_NvLowRelease
**             SAPL_NvLowCurrBlkRead
**             SAPL_NvLowInit
**             SAPL_NvLowCrcSync
**             SAPL_NvLowHandler
**             SAPL_NvLowIsBusy
**             SAPL_NvLowFreeBlkGet
**
**             FirstBlkWrite
**             NvParamConfSectionErase
**             CRC16Get
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
#include "globPreChk.h"
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "rds.h"

#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicCrc.h"

#include "flash-hal.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* CSS headers */
#include "IXSSOapi.h"

#include "SAPLipc.h"
#include "SAPLnvLow.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      20u

/* Reserved NV section used for 'Parameters and Configuration' */
#define k_PARAM_CONF_FLSH_ADDR_START     (UINT32)(0x0803c000u) /* see [SRS_2174]) */
#define k_PARAM_CONF_FLSH_PAGE_SIZE      (UINT16)(0x0800u)
#define k_PARAM_CONF_FLSH_SIZE           (UINT16)(0x2800u)
#define k_PARAM_CONF_FLSH_ADDR_END       (UINT32)(0x0803e7ffu)

/* size of single configuration block */
#define k_PARAM_CONF_FLSH_BLK_SIZE       (UINT16)(sizeof(t_FLSH_BLK))
/* Number of configuration blocks usable in single page */ 
#define k_PARAM_CONF_FLSH_BLK_MAX_CNT   (UINT8)(k_PARAM_CONF_FLSH_SIZE / k_PARAM_CONF_FLSH_BLK_SIZE)
/* Flash Block Threshold during startup, less than 20%, (see [SRS_2191]) */
#define k_PARAM_CONF_FLSH_BLK_THRESH     (UINT8)((k_PARAM_CONF_FLSH_BLK_MAX_CNT * 80u) / 100u)

/* Defines the number of flash cycles necessary to write a whole Flash Block (incl. CRC16) */
#define k_PARAM_CONF_FLSH_BLK_WR_CYCLES  (UINT16)((k_PARAM_CONF_FLSH_BLK_SIZE) / sizeof(UINT16))

/* used to mark free (unprogrammed) flash blocks */
#define k_FLSH_BLK_STATUS_FREE           (UINT16)(0xffff)
/* used to mark valid (programmed) flash blocks */
#define k_FLSH_BLK_STATUS_VALID          (UINT16)(0xabcd)
/* used to mark invalid (expired) flash blocks */
#define k_FLSH_BLK_STATUS_INVALID        (UINT16)(0x0000)

/*!<  FSM states */
typedef enum
{
  k_FLSH_UNINITIALIZED                = 0x6c8du,
  k_FLSH_IDLE                         = 0x6d1au,
  k_FLSH_EXCH_CRC_COMP                = 0x6e34u,
  k_FLSH_WRITE_BLK                    = 0x6fa3u,
  k_FLSH_INVLD_OLD_BLK                = 0x708fu
} t_FLSH_FSM;

/* Structure of Flash Block */
typedef struct
{
  UINT16 u16_blkSts;
  UINT16 u16_blkIdx;    /* (see [SRS_2176]) */
  SAPL_t_NVLOW_BLKDATA s_blkData;
  UINT16 u16_blkCrc;
} t_FLSH_BLK;

/* structure of user flash area */
typedef struct 
{
  t_FLSH_BLK as_flshBlk[k_PARAM_CONF_FLSH_BLK_MAX_CNT];
} t_FLSH_USER_AREA;


/* Default data that should be stored in NV memory in case of
** - a factory reset
** - if NV memory seems to be corrupted
** - first power-cycle (production test does not store default data)
*/
CONST SAPL_t_NVLOW_BLKDATA SAPLnvLow_DefaultData =
{
  /* Alarm Enable (Default = 1) */
  TRUE,

  /* Warning Enable (Default = 1) */
  TRUE,

  /* OCPUNID, incl. 2 padding bytes (Default = 0)*/
  { 0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Date (Little Endian) */
    0x00u,   /* SNN, Date (Little Endian) */
    0x00u,   /* Padding (part of the underlying CSS structure) */
    0x00u }, /* Padding (part of the underlying CSS structure) */

  /* SCID, incl. 2 padding bytes (Default = 0) */
  { 0x00u,   /* SCCRC (Little Endian) */
    0x00u,   /* SCCRC (Little Endian) */
    0x00u,   /* SCCRC (Little Endian) */
    0x00u,   /* SCCRC (Little Endian) */
    0x00u,   /* SCTS, Time (Little Endian) */
    0x00u,   /* SCTS, Time (Little Endian) */
    0x00u,   /* SCTS, Time (Little Endian) */
    0x00u,   /* SCTS, Time (Little Endian) */
    0x00u,   /* SCTS, Date (Little Endian) */
    0x00u,   /* SCTS, Date (Little Endian) */
    0x00u,   /* Padding (part of the underlying CSS structure) */
    0x00u }, /* Padding (part of the underlying CSS structure) */

  /* TUNID, incl. 2 padding bytes (Default SNN = 0xFF, NodeId = Device Specific) */
  { 0xFFu,   /* Node ID (Little Endian) */
    0xFFu,   /* Node ID (Little Endian) */
    0xFFu,   /* Node ID (Little Endian) */
    0xFFu,   /* Node ID (Little Endian) */
    0xFFu,   /* SNN, Time (Little Endian) */
    0xFFu,   /* SNN, Time (Little Endian) */
    0xFFu,   /* SNN, Time (Little Endian) */
    0xFFu,   /* SNN, Time (Little Endian) */
    0xFFu,   /* SNN, Date (Little Endian) */
    0xFFu,   /* SNN, Date (Little Endian) */
    0x00u,   /* Padding (part of the underlying CSS structure) */
    0x00u }, /* Padding (part of the underlying CSS structure) */

  /* CFUNID, incl. 2 padding bytes (Default 0 - un-owned, accept any owner) */
  { 0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* Node ID (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Time (Little Endian) */
    0x00u,   /* SNN, Date (Little Endian) */
    0x00u,   /* SNN, Date (Little Endian) */
    0x00u,   /* Padding (part of the underlying CSS structure) */
    0x00u }, /* Padding (part of the underlying CSS structure) */

  /* Configuration Data IO, shall match to 'fiParam_sIParam' */
  { 0x03u,   /* Number of Dual Channel Inputs */
    0x01u,   /* Number of Dual Channel Outputs */
    0x00u,   /* DI1/2: Enabled, Channel Mode, Type, Reset Type (see [SRS_2044]) */
    0x00u,   /* DI1/2: Debounce Filter Time (see [SRS_2044]) */
    0x00u,   /* DI1/2: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DI1/2: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DI3/4: Enabled, Channel Mode, Type, Reset Type (see [SRS_2044]) */
    0x00u,   /* DI3/4: Debounce Filter Time (see [SRS_2044]) */
    0x00u,   /* DI3/4: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DI3/4: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DI5/6: Enabled, Channel Mode, Type, Reset Type (see [SRS_2044]) */
    0x00u,   /* DI5/6: Debounce Filter Time (see [SRS_2044]) */
    0x00u,   /* DI5/6: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DI5/6: Consistency Filter Time (Little Endian) (see [SRS_2044]) */
    0x00u,   /* DO1/2: Enabled, Channel Mode, Reset Type, Test Offset (see [SRS_2045]) */
    0xE8u,   /* Latch Input Error Time=1000ms (Little Endian) (see [SRS_2109]) */
    0x03u,   /* Latch Input Error Time=1000ms (Little Endian) (see [SRS_2109]) */
    0xE8u,   /* Latch Output Error Time=1000ms (Little Endian) (see [SRS_2109]) */
    0x03u,   /* Latch Output Error Time=1000ms (Little Endian) (see [SRS_2109]) */
    0x00u,   /* DO safe state delay in multiples of 4ms (Little Endian) (see [SRS_908]) */
    0x00u,   /* DO safe state delay in multiples of 4ms (Little Endian) (see [SRS_908]) */
    0x00u,   /* SafeBound Enable (see [SRS_902]) */
    0x00u,   /* Pad Byte (see [SRS_2254]) */
  },
  0x00u    /* Padding->u8_res1 */
};

/** u32_PendingBlkData
**  Address of next block data to program into NV memory
*/
STATIC volatile RDS_UINT32 u32_PendingBlkData;

/** e_FlshFsmState
** State of FlashHandler
*/
STATIC volatile t_FLSH_FSM e_FlshFsmState = k_FLSH_UNINITIALIZED;

/** s_WrFlshBlk
** Internal used flash block. The data of this block is programmed inside NV memory
*/
STATIC t_FLSH_BLK  s_WrFlshBlk;

/* u16_RdFlshBlkIdx
** Current Flash Block Index used for Read Access
** valid 0..(k_PARAM_CONF_FLSH_BLK_MAX_CNT-1)
*/
STATIC volatile RDS_UINT16 u16_RdFlshBlkIdx;

/* variable indicates how many half words of the flash block have been written to the NV memory
** ATTENTION: Half word is considered as 16 Bit */
STATIC RDS_UINT16 u16_WrHalfWordIdx;

/* const pointer to flash user area used for NV variables */
/* Deactivate lint, cast is necessary because a fixed address is used*/
STATIC t_FLSH_USER_AREA* CONST ps_FlshUsrArea =
    (t_FLSH_USER_AREA*)k_PARAM_CONF_FLSH_ADDR_START; /*lint !e923*/


/** PRE-PROCESSOR CHECK **/
/* Ensure buffers match to CSS definitions */
/* Deactivated Lint Note 948: Operator '==' always evaluates to True */
IXX_TASSERT( (SAPL_k_SDS_OCPUNID  == IXSSO_k_SDS_OCPUNID), nvlow_chk1 ) /*lint !e948 */
IXX_TASSERT( (SAPL_k_SDS_SCID  == IXSSO_k_SDS_SCID), nvlow_chk2 )       /*lint !e948 */
IXX_TASSERT( (SAPL_k_SID_TUNID  == IXSSO_k_SDS_TUNID), nvlow_chk3 )     /*lint !e948 */

/* Mute lint warning type not referenced. These types are just defined for
** being able to check the size of the elementary types. */
/*lint -esym(751, IXX_DUMMY_nvlow_chk1 ) not referenced - just for checks */
/*lint -esym(751, IXX_DUMMY_nvlow_chk2 ) not referenced - just for checks */
/*lint -esym(751, IXX_DUMMY_nvlow_chk3 ) not referenced - just for checks */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void FirstBlkWrite(CONST SAPL_t_NVLOW_BLKDATA* ps_blkData);
STATIC void NvParamConfSectionErase(void);
STATIC UINT16 CRC16Get(CONST t_FLSH_BLK* ps_flshBlk);

/***************************************************************************************************
**    global functions
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
void SAPL_NvLowAcquire(void)
{
  /* clear pending again */
  RDS_SET(u32_PendingBlkData, 0);
}

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
void SAPL_NvLowRelease(CONST SAPL_t_NVLOW_BLKDATA* ps_blkData)
{
  /* set pending block address */
  RDS_SET(u32_PendingBlkData, (UINT32)ps_blkData);
}

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
CONST SAPL_t_NVLOW_BLKDATA* SAPL_NvLowCurrBlkRead(void)
{
  /* return value of this function, pointer to block data */
  SAPL_t_NVLOW_BLKDATA* ps_retVal = NULL;
  /* local calculated CRC */
  UINT16 u16_crc;
  
  /* check Read Block Index once */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_RdFlshBlkIdx);
  /*lint +esym(960, 10.1)*/
  
  /* if NV memory not initialized at all */
  if (e_FlshFsmState == k_FLSH_UNINITIALIZED)
  {
    /* enter fail-safe */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: if 'u16_RdFlshBlkIdx' out of range */
  else if (RDS_GET_VALUE(u16_RdFlshBlkIdx) >= k_PARAM_CONF_FLSH_BLK_MAX_CNT)
  {
    /* enter fail-safe */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: everything fine */
  else
  {  
    /* if flash block status not marked as valid */
    if (ps_FlshUsrArea->as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)].u16_blkSts 
          != k_FLSH_BLK_STATUS_VALID)
    {
      /* enter fail-safe */
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(3u));
    }
    /* else: flash block status is valid */
    else
    {
      /* calculation of CRC16 */
      u16_crc = CRC16Get(&ps_FlshUsrArea->as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)]);
      /* if CRC16 is correct (see [SRS_2177]) */
      if (u16_crc == ps_FlshUsrArea->as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)].u16_blkCrc)
      {
        ps_retVal = &ps_FlshUsrArea->as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)].s_blkData;
      }
      /* else: CRC error */
      else
      {
        /* enter fail-safe */
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(4u));
      }
    }
  }
  return ps_retVal;
}


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
void SAPL_NvLowInit(void)
{
  /* iterator */
  UINT16 i;
  /* number of valid flash blocks */
  UINT16 u16_validBlkCnt = 0u;
  /* local calculated CRC16 */
  UINT16 u16_crc16;
  
  /* initialize module static variables */
  CSS_MEMSET(&s_WrFlshBlk, 0u, sizeof(s_WrFlshBlk));

  /* initialize RDS variables */
  RDS_SET(u16_RdFlshBlkIdx, 0U);
  RDS_SET(u16_WrHalfWordIdx, 0U);
  RDS_SET(u32_PendingBlkData, 0U);
  
  /* search for valid blocks in ENTIRE user area */
  for (i=0U; i < k_PARAM_CONF_FLSH_BLK_MAX_CNT; i++)
  {
    /* check block status of current flash block */
    if (ps_FlshUsrArea->as_flshBlk[i].u16_blkSts == k_FLSH_BLK_STATUS_VALID)
    {
      /* check CRC */
      u16_crc16 = CRC16Get(&ps_FlshUsrArea->as_flshBlk[i]);
      /* if valid CRC found */
      if (u16_crc16 == ps_FlshUsrArea->as_flshBlk[i].u16_blkCrc)
      {
        /* assign current block index */
        RDS_SET(u16_RdFlshBlkIdx, i);
        /* and increment block counter */
        u16_validBlkCnt++;
      }
      /* else: a 'valid' flash block was found, but the Block CRC is invalid (means corrupted) */
      else
      {
        /* This can only happen if the flash is corrupted. In order to handle this situation,
        ** the flash is erased completely and the default data is written (see [SRS_2225]).
        ** Afterwards the failsafe state is entered */
        FirstBlkWrite(&SAPLnvLow_DefaultData);
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(5u));
      }
    }
    /* else if : flash block invalid or free (unprogrammed) */
    else if ( (ps_FlshUsrArea->as_flshBlk[i].u16_blkSts == k_FLSH_BLK_STATUS_INVALID) ||
              (ps_FlshUsrArea->as_flshBlk[i].u16_blkSts == k_FLSH_BLK_STATUS_FREE) )
    {
      /* check next flash block */
    }
    /* else: invalid status */
    else
    {
      /* This can only happen if the flash is corrupted (e.g. power lost during flash).
      ** In order to handle this situation, the flash is erased completely and the default data is
      ** written. Afterwards the failsafe state is entered */
      FirstBlkWrite(&SAPLnvLow_DefaultData);
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(6u));
    }
  }
  
  /* if more than one valid flash block found  */
  if ( u16_validBlkCnt > 1u )
  {
    /* This can only happen if the flash is corrupted (e.g. power lost during flash).
    ** In order to handle this situation, the flash is erased completely and the default data is
    ** written. Afterwards the failsafe state is entered */
    FirstBlkWrite(&SAPLnvLow_DefaultData);
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(7u));
  }
  /* else if no valid flash block was found (see [SRS_2194]) */
  else if ( u16_validBlkCnt == 0u )
  {
    /* Write the default data into first block of flash (see [SRS_2194]) */
    FirstBlkWrite(&SAPLnvLow_DefaultData);
  }
  /* else: if flash block threshold is exceeded (see [SRS_2191]) */
  else if ( RDS_GET_VALUE(u16_RdFlshBlkIdx) > k_PARAM_CONF_FLSH_BLK_THRESH )
  {
    /* Write the current block data into first block of flash */
    FirstBlkWrite(&ps_FlshUsrArea->as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)].s_blkData);
  }
  /* else: valid flash content found */
  else
  {
    /* empty branch */
  }

  /* set FSM state of FlashHandler to IDLE */
  e_FlshFsmState = k_FLSH_IDLE;
}

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
void SAPL_NvLowCrcSync(void)
{
  UINT32 u32_sendCfgCrcData;
  UINT32 u32_recvCfgCrcData;
  
  /* exchange of CRC to ensure the same configuration is used on both uC */
  u32_sendCfgCrcData =
        ps_FlshUsrArea->as_flshBlk[RDS_GET(u16_RdFlshBlkIdx)].u16_blkCrc;
  
  u32_recvCfgCrcData = ipcxSYS_ExChangeUINT32Data(u32_sendCfgCrcData, IPCXSYS_IPC_ID_NV_CRC);
  
  /* check if the received data is equal to the own data, if not go to Safety Handler and write the
  ** default data to NV memory (see [SRS_2189]) */
  if (u32_recvCfgCrcData != u32_sendCfgCrcData)
  {
    /* write the default values into first block of flash (see [SRS_2189]) */
    FirstBlkWrite(&SAPLnvLow_DefaultData);
    /* call global safety handler for error signalization to user */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_CONFIG, GLOBFAIL_ADDINFO_FILE(8u));
  }
  /* else: CRC equal on both controllers */
  else
  {
    /* empty branch */
  }
}

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
void SAPL_NvLowHandler(void)
{  
  FLASHHAL_t_STATUS e_flashStatus;
  UINT16 u16_wrFlshBlkIdx;
  
  /* check RDS variable once here */
  /*lint -esym(960, 10.1)*/
  /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_RdFlshBlkIdx);
  /*lint +esym(960, 10.1)*/
  
  /* set write block index, always u16_RdFlshBlkIdx + 1, (see [SRS_2186]) */
  u16_wrFlshBlkIdx = RDS_GET_VALUE(u16_RdFlshBlkIdx) + 1U;

  switch (e_FlshFsmState)
  {
    /* k_FLSH_UNINITIALIZED: Not initialized */
    case k_FLSH_UNINITIALIZED:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(9u));
      break;
    }
    /* k_FLSH_IDLE: Waiting for new request */
    case k_FLSH_IDLE:
    { 
      /* check RDS variable once here */
      /*lint -esym(960, 10.1)*/
      /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
      * complex integer expression. */
      RDS_CHECK_VARIABLE(u32_PendingBlkData);
      /*lint +esym(960, 10.1)*/
      
      /* if new data available */
      if ( RDS_GET_VALUE(u32_PendingBlkData) != 0U )
      {    
        /* if the max. number of flash blocks reached, there is no resources to flash 
        ** available.
        */
        if (u16_wrFlshBlkIdx >= k_PARAM_CONF_FLSH_BLK_MAX_CNT)
        {
          /* enter global failsafe (see [SRS_2187]) */ 
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(10u));
        }
        /* else: if Block Status is not 'Free' */
        else if (ps_FlshUsrArea->as_flshBlk[u16_wrFlshBlkIdx].u16_blkSts != k_FLSH_BLK_STATUS_FREE)
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(11u));
        }
        /* else: valid new flash block detected */
        else
        {
          /* Prepare new flash Block */
          s_WrFlshBlk.u16_blkSts = k_FLSH_BLK_STATUS_VALID;
          s_WrFlshBlk.u16_blkIdx = u16_wrFlshBlkIdx;
          /* copy content to internal memory buffer, since passed address to buffer is accessed from
          ** background, a cast to 'volatile' is done here */
          s_WrFlshBlk.s_blkData  = 
             *(volatile SAPL_t_NVLOW_BLKDATA*)RDS_GET_VALUE(u32_PendingBlkData);
          /* calculate new CRC and store in flash block (RAM) */
          s_WrFlshBlk.u16_blkCrc = CRC16Get(&s_WrFlshBlk);
          /* reset pending 'flag' since data is already copied into internal memory buffer */
          RDS_SET(u32_PendingBlkData, 0U);
          
          /* reinit word index */
          RDS_SET(u16_WrHalfWordIdx, 0U);
          
          /* set NV CRC for IPC transfer */
          SAPLipc_SetNvCrc(s_WrFlshBlk.u16_blkCrc);

          /* enter next state */
          e_FlshFsmState = k_FLSH_EXCH_CRC_COMP;
        }
      }
      /* else: no new data to write requested */
      else 
      {
        /* empty branch */
      }        
      break;
    }

    /* k_FLSH_EXCH_CRC_COMP: Compare calculated with received CRC */
    case k_FLSH_EXCH_CRC_COMP:
    {
      SAPLipc_CheckNvCrc();
      /* enter next state */
      e_FlshFsmState = k_FLSH_WRITE_BLK;
      break;
    }
    
    /* k_FLSH_WRITE_BLK: Writing single (half) word into flash memory until the whole block
    ** is written (see [SRS_2175]) */
    case k_FLSH_WRITE_BLK:
    {
      /* Current Flash Address to write */
      UINT32 u32_currWrFlshAddr;
      /* Current Block Address (RAM) to read from */
      UINT16* pu16_currWrDataAddr;
      
      /* check RDS variable once here */
      /*lint -esym(960, 10.1)*/
      /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
      * complex integer expression. */
      RDS_CHECK_VARIABLE(u16_WrHalfWordIdx);
      /*lint +esym(960, 10.1)*/

      /* check ranges of 'Write Block Index' and 'Word Counter' */
      /* if invalid range detected */
      if ( (u16_wrFlshBlkIdx >= k_PARAM_CONF_FLSH_BLK_MAX_CNT) ||
           (RDS_GET_VALUE(u16_WrHalfWordIdx) >= k_PARAM_CONF_FLSH_BLK_WR_CYCLES) )
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(12u));
      }
      /* else: ranges OK */
      else
      {
        /* calc current flash address depending on already written words */
        /* Deactivate lint, cast is necessary to build flash address (UINT32) */
        u32_currWrFlshAddr = 
           (UINT32)&ps_FlshUsrArea->as_flshBlk[u16_wrFlshBlkIdx]
            + (RDS_GET_VALUE(u16_WrHalfWordIdx) * sizeof(UINT16)); /*lint !e923*/

        /* Deactivate lint, cast is necessary to access structure word by word, pointer arithmetic
        ** is wanted here */
        /* Info 740: Unusual pointer cast (incompatible indirect types) [MISRA 2004 Rule 1.2] */
        /*lint -esym(960, 17.1) -esym(960, 17.4) */
        /*calc next block address to read depending on already read words */
        pu16_currWrDataAddr = 
          (UINT16*)&s_WrFlshBlk + RDS_GET_VALUE(u16_WrHalfWordIdx); /*lint !e929, !e740 */
        /*lint +esym(960, 17.1) +esym(960, 17.4) */

        /* Unlock Flash Memory */
        flashHAL_Unlock();
        /* programming half word */
        e_flashStatus = flashHAL_ProgramHalfWord(u32_currWrFlshAddr, *pu16_currWrDataAddr);
        /* Lock the flash */
        flashHAL_Lock();
        /* if half word was not written */
        if (FLASHHAL_k_COMPLETE != e_flashStatus)
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(13u));
        }
        /* else: half word written */
        else
        {
          /* update half word index */
          RDS_INC(u16_WrHalfWordIdx);

          /* if all half words are written */
          if (RDS_GET_VALUE(u16_WrHalfWordIdx) == k_PARAM_CONF_FLSH_BLK_WR_CYCLES)
          {
            /* enter next state */
            e_FlshFsmState = k_FLSH_INVLD_OLD_BLK;
          }
          /* else: still data to write */
          else
          {
            /* empty branch */
          }
        }
      }
      break;
    }

    /* k_FLSH_INVLD_OLD_BLK: Invalidate Old Flash Block (see [SRS_2175])*/
    case k_FLSH_INVLD_OLD_BLK:
    {  
      /* Current Flash Address to write */
      UINT32 u32_currWrFlshAddr;
      
      /* if invalid range of Read Block Index, 'last block' shall be never invalidated... */
      if ( RDS_GET_VALUE(u16_RdFlshBlkIdx) >= (k_PARAM_CONF_FLSH_BLK_MAX_CNT-1u) )
      {
        GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(14u));
      }
      /* else: range of Read Block Index OK */
      else
      {
        /* set flash address of Block Status of current Read Block */
        /* Deactivate lint, cast is necessary to build flash address (UINT32) */
        u32_currWrFlshAddr =
            (UINT32)&ps_FlshUsrArea->
                      as_flshBlk[RDS_GET_VALUE(u16_RdFlshBlkIdx)].u16_blkSts; /*lint !e923 */
  
        /* Flash Block 'Valid' written so update Read Block Index */
        RDS_SET(u16_RdFlshBlkIdx, u16_wrFlshBlkIdx);
  
        /* Invalidate 'Old Flash Block' */
        /* Unlock Flash Memory */
        flashHAL_Unlock();
        /* programming half word */
        e_flashStatus = flashHAL_ProgramHalfWord(u32_currWrFlshAddr, k_FLSH_BLK_STATUS_INVALID);
        /* Lock the flash */
        flashHAL_Lock();
        /* if half word was not written */
        if (FLASHHAL_k_COMPLETE != e_flashStatus)
        {
          GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(15u));
        }
        /* else: half word written */
        else
        {
          /* flash cycle finished, switch back to 'IDLE' */
          e_FlshFsmState = k_FLSH_IDLE;
        }
      }
      break;
    }
    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(16u));
      break;
    }
  }
}

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
BOOL SAPL_NvLowIsBusy(void)
{
  /* return value of this function */
  BOOL b_retVal;
  
  switch (e_FlshFsmState)
  {
    case k_FLSH_UNINITIALIZED:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(17u));
      /* set return value (only used for unit test) */
      b_retVal = TRUE;
      break;
    }
    case k_FLSH_IDLE:
    {
      /* if no write request is triggered by application */
      if ((UINT32)RDS_GET(u32_PendingBlkData) == 0U)
      {
        b_retVal = FALSE;
      }
      /* else: state IDLE, but already a write request is triggered by application */
      else
      {
        b_retVal = TRUE;
      }
      break;
    }
    case k_FLSH_INVLD_OLD_BLK:     /* fall through */
    case k_FLSH_WRITE_BLK:         /* fall through */
    case k_FLSH_EXCH_CRC_COMP:     /* fall through */
    {
      b_retVal = TRUE;
      break;
    }
    default:
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(18u));
      /* set return value (only used for unit test) */
      b_retVal = TRUE;
      break;
    }
  }
  return b_retVal;
}

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
UINT16 SAPL_NvLowFreeBlkGet(void)
{

  /* number of free flash blocks */
  UINT16 u16_freeBlkCnt;
  
  /* check RDS variables once here */
  /*lint -esym(960, 10.1)*/
  /* Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
   * complex integer expression. */
  RDS_CHECK_VARIABLE(u16_RdFlshBlkIdx);
  /*lint +esym(960, 10.1)*/
  
  /* u16_RdFlshBlkIdx: 0..k_PARAM_CONF_FLSH_BLK_MAX_CNT-1;
  ** u16_freeBlkCnt:   0..k_PARAM_CONF_FLSH_BLK_MAX_CNT
  */
  
  /* if still free blocks available */
  if (k_PARAM_CONF_FLSH_BLK_MAX_CNT > RDS_GET_VALUE(u16_RdFlshBlkIdx))
  {
    u16_freeBlkCnt = ((UINT16)k_PARAM_CONF_FLSH_BLK_MAX_CNT -
                      (RDS_GET_VALUE(u16_RdFlshBlkIdx) + 1u));
  }
  /* else no more free blocks */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(19u));
    /* set return value (only used for unit tests) */
    u16_freeBlkCnt = 0u;
  }
  return u16_freeBlkCnt;
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    FirstBlkWrite

  Description:
    The function is used to write data to the first flash block of the 'Parameters and
    Configuration' on-chip memory.
    Therefore the software:
      - erases the whole 'Parameters and Configuration' on-chip memory
      - writes the passed block data to the first flash block
      - sets the first flash block as active one
    In contrast to the FlashHandler, the FlashBlock is written at once completely by this function.
    Since no CRC16 exchange between the two controllers is done here, this check has to performed
    afterwards.

  See also:
    -

  Parameters:
    ps_blkData (IN)          - Block data that shall be written to the first flash block of the
                               'Parameters and Configuration' on-chip memory.
                               (valid range: <>NULL, not checked, only called by reference)

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
STATIC void FirstBlkWrite(CONST SAPL_t_NVLOW_BLKDATA* ps_blkData)
{
  /* iterator */
  UINT16 u16_wrIdx;
  /* flash status */
  FLASHHAL_t_STATUS e_flashStatus;
  /* flash block that should be written to flash memory */
  t_FLSH_BLK  s_flshBlk;
  /* Current Flash Address to write */
  UINT32 u32_currWrFlshAddr;
  /* Current RdWr Block Address */
  UINT16* pu16_currWrDataAddr;

  /* Prepare flash Block */
  s_flshBlk.u16_blkSts = k_FLSH_BLK_STATUS_VALID;
  s_flshBlk.u16_blkIdx = 0u; /* first block */
  /* copy content to internal memory buffer */
  s_flshBlk.s_blkData  = *ps_blkData;
  /* calc CRC16 */
  s_flshBlk.u16_blkCrc = CRC16Get(&s_flshBlk);

  /* erase the complete 'Parameters and Configuration' section */
  NvParamConfSectionErase();

  /* Unlock Flash Memory */
  flashHAL_Unlock();
  /* write until the whole flash block is written */
  for ( u16_wrIdx = 0U;
        u16_wrIdx < k_PARAM_CONF_FLSH_BLK_WR_CYCLES;
        u16_wrIdx++ )
  {
    /* Deactivate lint, cast is necessary to build flash address (UINT32) */
    u32_currWrFlshAddr = 
      (UINT32)&ps_FlshUsrArea->as_flshBlk[0] + (u16_wrIdx * sizeof(UINT16)); /*lint !e923*/
    /* Deactivate lint, cast is necessary to access structure word by word, pointer arithmetic
    ** is wanted here */
    /* Info 740: Unusual pointer cast (incompatible indirect types) [MISRA 2004 Rule 1.2] */
    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    pu16_currWrDataAddr = (UINT16*)&s_flshBlk + u16_wrIdx; /*lint !e929, !e740 */
    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    /* programming half word */
    e_flashStatus = flashHAL_ProgramHalfWord(u32_currWrFlshAddr, *pu16_currWrDataAddr);
    /* if half word was not written */
    if (FLASHHAL_k_COMPLETE != e_flashStatus)
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(20u));
      /* Keyword break is used only AFTER global safety handler is called. So this
       * is only relevant for unit test */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #44   -Keyword 'break' identified */
      break;
      /* RSM_IGNORE_QUALITY_END */
    }
    /* else half word written */
    else
    {
      /* empty branch */
    }
  }
  
  /* Lock the flash */
  flashHAL_Lock();
  /* assign block '0' as current valid Read block */
  RDS_SET(u16_RdFlshBlkIdx, 0U);
}


/***************************************************************************************************
  Function:
    NvParamConfSectionErase

  Description:
    This function erases the complete 'Parameter and Configuration' section of the onchip memory.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
STATIC void NvParamConfSectionErase(void)
{
  UINT32 u32_pageAddr;
  /* flash status */
  FLASHHAL_t_STATUS e_flashStatus;
  
  /* Unlock Flash */
  flashHAL_Unlock();
  
  for ( u32_pageAddr = k_PARAM_CONF_FLSH_ADDR_START; 
        u32_pageAddr < k_PARAM_CONF_FLSH_ADDR_END;
        u32_pageAddr += k_PARAM_CONF_FLSH_PAGE_SIZE )
  {
    /* erase page */
    e_flashStatus = flashHAL_ErasePage(u32_pageAddr);
    /* if page is not erased */
    if (FLASHHAL_k_COMPLETE != e_flashStatus)
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(21u));
      /* Keyword break is used only AFTER global safety handler is called. So this
       * is only relevant for unit test */
      /* RSM_IGNORE_QUALITY_BEGIN Notice #44   -Keyword 'break' identified */
      break;
      /* RSM_IGNORE_QUALITY_END */
    }
    /* else: page erased */
    else
    {
      /* empty branch */
    }
  }
  
  /* Lock the flash */
  flashHAL_Lock();
}

/***************************************************************************************************
  Function:
    CRC16Get

  Description:
    This function is used to calculate the CRC16 of a flash block (see [SRS_2176]). Since the 
    flash block also includes the current block index, this value is also included into the
    CRC check. This ensures that both controllers always access the same flash block.

  See also:
    -

  Parameters:
    ps_flshBlk (IN)         - Pointer to flash block
                              (valid range: <>NULL, not checked, only called by reference) 

  Return value:
    UINT16                   - CRC16

  Remarks:
    Context: main (during initialization)
    Context: Background Task

***************************************************************************************************/
STATIC UINT16 CRC16Get(CONST t_FLSH_BLK* ps_flshBlk)
{
  /* simply call existing function CRC function of aic module. The polynominal 0xa001 is used 
  ** there (see [SRS_2176]). */  
  /* Deactivate lint, cast required to call CRC routine (based on byte access) */
  return (aicCrc_Crc16((CONST UINT8*)ps_flshBlk,  /*lint !e928*/
                        k_PARAM_CONF_FLSH_BLK_SIZE-sizeof(UINT16))); 
}


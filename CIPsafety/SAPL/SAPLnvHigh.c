/***************************************************************************************************
**    Copyright (C) 2015-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLnvHigh.c
**     Summary: This module implements the high level functions for the NV memory handling.
**              High level functions are typically used to access NV variables.  
**   $Revision: 4337 $
**       $Date: 2024-03-15 13:23:28 +0100 (Fr, 15 Mrz 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_IxssoNvAttribStoreClbk
**             SAPL_IxssoNvAttribRestoreClbk
**             SAPL_NvHighInit
**             SAPL_NvHighConfigStore
**             SAPL_NvHighConfigRestore
**             SAPL_NvHighDefaultStore
**             SAPL_NvHighResetType2Store
**
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
#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicCrc.h"

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

#include "SAPLnvlow.h"
#include "SAPLnvHigh.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      19u

/* Write Buffer used by the background task to write variables to non-volatile memory.
** Since this buffer is infrequently updated, the content shall be protected against soft-error
** by a CRC16. Every time BEFORE the background task writes into the buffer, this check must be 
** performed. */
STATIC SAPL_t_NVLOW_BLKDATA s_WrBufBgrd;

/* CRC of Write Buffer. Used to protect s_WrBufBgrd against soft-errors */
STATIC UINT16 u16_WrBufCrc = 0u;

/* values of Attribute_Bit_Map, See VOL5, Table 5-4.21 */
/* Some values currently not used but kept for future use and completeness */
/*lint -esym(750, k_ATTR_BIT_MAP_PRESERVE_MACID) local macro not referenced */
/*lint -esym(750, k_ATTR_BIT_MAP_PRESERVE_BAUDRATE) local macro not referenced */
/*lint -esym(750, k_ATTR_BIT_MAP_PRESERVE_PASSWORD) local macro not referenced */
/*lint -esym(750, k_ATTR_BIT_MAP_RESERVED) local macro not referenced */
/*lint -esym(750, k_ATTR_BIT_MAP_USE_EXTENDED_MAP) local macro not referenced */
#define k_ATTR_BIT_MAP_PRESERVE_MACID        0x01U
#define k_ATTR_BIT_MAP_PRESERVE_BAUDRATE     0x02U
#define k_ATTR_BIT_MAP_PRESERVE_TUNID        0x04U
#define k_ATTR_BIT_MAP_PRESERVE_PASSWORD     0x08U
#define k_ATTR_BIT_MAP_PRESERVE_CFUNID       0x10U
#define k_ATTR_BIT_MAP_PRESERVE_OCPUNID      0x20U
#define k_ATTR_BIT_MAP_RESERVED              0x40U
#define k_ATTR_BIT_MAP_USE_EXTENDED_MAP      0x80U

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
STATIC UINT16 CRC16Get(void);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/*******************************************************************************
**
** Function    : SAPL_IxssoNvAttribStoreClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when it wants to write
**               the values for the Safety Supervisor Object Attributes to
**               non-volatile memory.
**
**               **Attention**:
**               Storing and restoring of non-volatile parameters shall be done
**               with safety integrity.
**
**               **Attention**:
**               The data passed in pb_storeData is stored/restored in processor
**               format. This means that endianness and possibly padding bytes
**               within structures (e.g. IXSSO_k_SID_OCPUNID = array of
**               CSS_t_UNID) is exactly the same like the data is stored in the
**               variables used by CSS.
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxssoNvAttribRestoreClbk()
**
** Parameters  : e_storeId (IN)    - Unique store identifier to be able to
**                                   reference the stored data later in the
**                                   {SAPL_IxssoNvAttribRestoreClbk()}. If a
**                                   store identifier was already used then the
**                                   stored data for this identifier will be
**                                   overwritten. In this case the
**                                   ``u32_dataSize`` must be the same.
**               u16_dataSize (IN) - size of the data in byte to be stored
**               pb_storeData (IN) - pointer to the data to be stored
**
** Returnvalue : CSS_k_TRUE    - Application was able to store the requested
**                               values
**               CSS_k_FALSE   - Application encountered an error while trying
**                               to write the NV values. The values are not
**                               saved!
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoNvAttribStoreClbk(IXSSO_t_STORAGE_ID e_storeId,
                                       CSS_t_UINT u16_dataSize,
                                       const CSS_t_BYTE * pb_storeData)
{
  /* Callback required according to [CS_MAN], (see [3.6-5:]) */
  
  /* return value of this function */
  CSS_t_BOOL b_retVal = CSS_k_FALSE;
  
  /* check content inside write buffer to ensure data is not destroyed by any kind of 
  ** soft-errors. */
  UINT16 u16_crc = CRC16Get();
  /* if application buffer invalid (soft error?) */
  if (u16_crc != u16_WrBufCrc)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: application buffer valid */
  else
  {
    /* switch: select variable to modify, only CSS variables are considered here */
    switch (e_storeId)
    {
      /* Alarm Enable (see [SRS_2179]) */
      case IXSSO_k_SID_AL_ENABLE:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.b_alEnable))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(&s_WrBufBgrd.b_alEnable, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
          /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* Warning Enable (see [SRS_2179]) */
      case IXSSO_k_SID_WN_ENABLE:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.b_wnEnable))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(&s_WrBufBgrd.b_wnEnable, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* OCPUNID (see [SRS_2178]) */
      case IXSSO_k_SID_OCPUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.au8_ocpunid))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(s_WrBufBgrd.au8_ocpunid, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
    
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* SCID */
      case IXSSO_k_SID_SCID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.au8_scid))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(s_WrBufBgrd.au8_scid, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* TUNID (see [SRS_2123]) */
      case IXSSO_k_SID_TUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.au8_tunid))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(s_WrBufBgrd.au8_tunid, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* CFUNID (see [SRS_2253]) */
      case IXSSO_k_SID_CFUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(s_WrBufBgrd.au8_cfunid))
        {
          /* Lock Flash Access */ 
          SAPL_NvLowAcquire();
          /* copy data */
          CSS_MEMCPY(s_WrBufBgrd.au8_cfunid, pb_storeData, u16_dataSize);
          /* Unlock Flash Access */
          SAPL_NvLowRelease(&s_WrBufBgrd);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      default:
      {
        /* unknown Storage ID */
        /* empty branch, return value already set to 'FALSE' */
        break;
      }
    }
    
    /* update CRC16 */
    u16_WrBufCrc = CRC16Get();
  }
  
  return (b_retVal);
}

/*******************************************************************************
**
** Function    : SAPL_IxssoNvAttribRestoreClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when it wants to read
**               the stored values for the Safety Supervisor Object Attributes.
**               In case of an unconfigured device default values must be
**               returned:
**               - Alarm Enable = 1
**               - Warning enable = 1
**               - CFUNID = 0 (see SRS134) (Target only)
**               - OCPUNIDs = 0 (see SRS201) (consuming Target only)
**               - SCID = 0 (see SRS184) (Target only)
**               - TUNID.SNN: A device in Manufacturers default state shall have
**                 an invalid UNID value in its NVS
**                 (e.g. FF FF FF FF FF FF) (see SRS117)
**               - TUNID.NodeID: device specific default value (see SRS115)
**
**               One way of initializing the NV memory of an unconfigured device
**               (or to restore the factory default settings) is to assign the
**               default value to a variable and call
**               {SAPL_IxssoNvAttribStoreClbk()} to store it. See the CSS demo
**               for details.
**
**               **Attention**:
**               Storing and restoring of non-volatile parameters shall be done
**               with safety integrity.
**
**               **Attention**:
**               The data passed in pb_storeData is stored/restored in processor
**               format. This means that endianness and possibly padding bytes
**               within structures (e.g. IXSSO_k_SID_OCPUNID = array of
**               CSS_t_UNID) is exactly the same like the data is stored in the
**               variables used by CSS.
**
** Context     : IXSCE_CmdProcess(), IXSVO_CmdProcess(), IXSMR_CmdProcess(),
**               IXSSC_Init(), IXSSO_ExceptionStatusSet(),
**               IXSVC_InstTxFrameGenerate(), IXSVS_ConsInstActivityMonitor(),
**               IXSSO_SafetyDeviceMajorFault()
**
** See Also    : SAPL_IxssoNvAttribStoreClbk()
**
** Parameters  : e_storeId (IN)     - unique store identifier that was passed at
**                                    the call of
**                                    {SAPL_IxssoNvAttribStoreClbk()}
**               u16_dataSize (IN)  - size of the data in byte to be restored
**               pb_storeData (OUT) - pointer to the memory space where the
**                                    stored data is restored
**
** Returnvalue : CSS_k_TRUE     - Application was able to provide the requested
**                                values
**               CSS_k_FALSE    - Application encountered an error while trying
**                                to read the NV values. Thus this function
**                                returns the default values for all
**                                attributes.
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxssoNvAttribRestoreClbk(IXSSO_t_STORAGE_ID e_storeId,
                                         CSS_t_UINT u16_dataSize,
                                         CSS_t_BYTE * pb_storeData)
{
  /* Callback required according to [CS_MAN], (see [3.6-6:]) */
  
  /* return value of this function */
  CSS_t_BOOL b_retVal = CSS_k_FALSE;
  /* pointer to current flash data block */
  CONST SAPL_t_NVLOW_BLKDATA* ps_rdFlshBlk;
  
  /* get pointer of current flash data block, this also evaluates the CRC16 inside NV-memory */
  ps_rdFlshBlk = SAPL_NvLowCurrBlkRead();
  /* if error occurred */
  if (ps_rdFlshBlk == NULL)
  {
    /* empty branch, return value already set to 'FALSE' */
  }
  /* else: no error */
  else
  {
    switch (e_storeId)
    {
      /* Alarm Enable (see [SRS_2179]) */
      case IXSSO_k_SID_AL_ENABLE:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->b_alEnable))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, &ps_rdFlshBlk->b_alEnable, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
          /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* Warning Enable (see [SRS_2179]) */
      case IXSSO_k_SID_WN_ENABLE:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->b_wnEnable))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, &ps_rdFlshBlk->b_wnEnable, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* OCPUNID (see [SRS_2178]) */
      case IXSSO_k_SID_OCPUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->au8_ocpunid))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, ps_rdFlshBlk->au8_ocpunid, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* SCID */
      case IXSSO_k_SID_SCID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->au8_scid))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, ps_rdFlshBlk->au8_scid, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* TUNID (see [SRS_2123]) */
      case IXSSO_k_SID_TUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->au8_tunid))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, ps_rdFlshBlk->au8_tunid, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      /* Configuration UNID (see [SRS_2253]) */
      case IXSSO_k_SID_CFUNID:
      {
        /* if passed data size matches to the expected one */ 
        if (u16_dataSize == sizeof(ps_rdFlshBlk->au8_cfunid))
        {
          /* copy data */
          CSS_MEMCPY(pb_storeData, ps_rdFlshBlk->au8_cfunid, u16_dataSize);
          b_retVal = CSS_k_TRUE;
        }
        /* else: mismatch in data size */
        else
        {
           /* empty branch, return value already set to 'FALSE' */
        }
        break;
      }
      default:
      {
        /* unknown/unexpected Storage ID */
        /* empty branch, return value already set to 'FALSE' */
        break;
      }
    }
  }
  return (b_retVal);
/* This function has a lot of paths because all possible types are catched inside the
** switch/case. But de facto the function is not really complex. */
/* RSM_IGNORE_QUALITY_BEGIN Notice #28   - Cyclomatic complexity > 15 */
}
/* RSM_IGNORE_QUALITY_END */


/***************************************************************************************************
  Function:
    SAPL_NvHighInit

  Description:
    This function is used to initialize the application specific part of the NV memory handling.
    The function has to be called, BEFORE the IRQ scheduler is started!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_NvHighInit(void)
{
  /* pointer to current flash data block */
  CONST SAPL_t_NVLOW_BLKDATA* ps_rdData;
  
  /* init the NV memory management within the software */
  SAPL_NvLowInit();
 
  /* read back data from flash, this also evaluates the CRC16 inside NV-memory */
  ps_rdData = SAPL_NvLowCurrBlkRead();
  /* if valid data was found inside flash */
  if (ps_rdData != NULL)
  {
    /* content of NV-memory is stored inside Write Buffer */
    s_WrBufBgrd = *ps_rdData;
    /* calculate CRC16 to protect buffer against soft-error */
    u16_WrBufCrc = CRC16Get();
  }
  /* else: no valid data inside flash */
  else
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_MEMORY_ERR, GLOBFAIL_ADDINFO_FILE(2u));
  }
}

/***************************************************************************************************
  Function:
    SAPL_NvHighConfigStore

  Description:
    This function is used to store the Configuration Data (IO) inside NV memory (see [SRS_2143]).
    For this purpose the passed values are set inside the Write Buffer of the application. The 
    FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    pb_storeData (IN)  - pointer to the memory space where the data is stored
                         (valid range: <>NULL, not checked, only passed with reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighConfigStore(CONST UINT8* pb_storeData)
{
  /* check content inside write buffer to ensure data is not destroyed by any kind of
  ** soft-errors. */
  UINT16 u16_crc = CRC16Get();
  /* if application buffer invalid (soft error?) */
  if (u16_crc != u16_WrBufCrc)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(3u));
  }
  /* else: application buffer valid */
  else
  {
    /* Lock Flash Access */ 
    SAPL_NvLowAcquire();
    /* copy data of Configuration Data (IO) */
    CSS_MEMCPY(s_WrBufBgrd.au8_cfgDataIo, pb_storeData, SAPL_k_SDS_CONF_DATA_IO);
    /* Unlock Flash Access */
    SAPL_NvLowRelease(&s_WrBufBgrd);
    
    /* update CRC */
    u16_WrBufCrc = CRC16Get();
  }
}

/***************************************************************************************************
  Function:
    SAPL_NvHighConfigRestore

  Description:
    This function is used to read the Configuration Data (IO) from NV memory.

  See also:
    -

  Parameters:
    pb_storeData (OUT) - pointer to the memory space where the stored data is restored
                         (valid range: <>NULL, not checked, only passed with reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighConfigRestore(UINT8* pb_storeData)
{
  /* pointer to current flash data block */
  CONST SAPL_t_NVLOW_BLKDATA* ps_rdFlshBlk;
  
  /* get pointer of current flash data block, this also evaluates the CRC16 inside NV-memory */
  ps_rdFlshBlk = SAPL_NvLowCurrBlkRead();
  /* if error occurred */
  if (ps_rdFlshBlk == NULL)
  {
    /* NV memory error */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_NV_APPL_ERR, GLOBFAIL_ADDINFO_FILE(5u));
  }
  /* else: no error */
  else
  {
    /* copy data */
    CSS_MEMCPY(pb_storeData, ps_rdFlshBlk->au8_cfgDataIo, SAPL_k_SDS_CONF_DATA_IO);
  }
}

/***************************************************************************************************
  Function:
    SAPL_NvHighDefaultStore

  Description:
    This function is used to set the NV variables to their default values. 
    For this purpose the default values are set inside the Write Buffer of the application. The 
    FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighDefaultStore(void)
{
  /* Lock Flash Access */ 
  SAPL_NvLowAcquire();
  /* set default data inside application buffer */
  s_WrBufBgrd = SAPLnvLow_DefaultData;
  /* Unlock Flash Access */
  SAPL_NvLowRelease(&s_WrBufBgrd);
  /* update CRC */
  u16_WrBufCrc = CRC16Get();
}


/***************************************************************************************************
  Function:
    SAPL_NvHighResetType2Store

  Description:
    This function is used to process the Attribute Bit Map received within a Safety Reset Type 2.
    Depending on the Attribute Bit Map some or all parameters of the NV memory will be set to 
    default values by the application. See [SRS_2171].
    The FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    u8_attrBitMap (IN) - The Attribute Bit Map is part of the Safety Reset Type 2 and indicates
                         which parameters shall be PRESERVED.

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighResetType2Store(CONST UINT8 u8_attrBitMap)
{
  /* check content inside write buffer to ensure data is not destroyed by any kind of
  ** soft-errors. */
  UINT16 u16_crc = CRC16Get();
  /* if application buffer invalid (soft error?) */
  if (u16_crc != u16_WrBufCrc)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(6u));
  }
  /* else: application buffer valid */
  else
  {
    /* Lock Flash Access */ 
    SAPL_NvLowAcquire();

    /* set default values of data which can't be preserved */
    s_WrBufBgrd.b_alEnable = SAPLnvLow_DefaultData.b_alEnable;
    s_WrBufBgrd.b_wnEnable = SAPLnvLow_DefaultData.b_wnEnable;
    s_WrBufBgrd.u8_res1    = SAPLnvLow_DefaultData.u8_res1;
    CSS_MEMCPY(&s_WrBufBgrd.au8_scid[0], 
               &SAPLnvLow_DefaultData.au8_scid[0], 
               SAPL_k_SDS_SCID);
    CSS_MEMCPY(&s_WrBufBgrd.au8_cfgDataIo[0], 
               &SAPLnvLow_DefaultData.au8_cfgDataIo[0], 
               SAPL_k_SDS_CONF_DATA_IO);

    /* if TUNID shall be preserved */
    if (u8_attrBitMap & k_ATTR_BIT_MAP_PRESERVE_TUNID)
    {
      /* empty branch, nothing to do here because data shall be preserved */
    }
    else
    {
      /* set to default */
      CSS_MEMCPY(&s_WrBufBgrd.au8_tunid[0], 
                 &SAPLnvLow_DefaultData.au8_tunid[0], 
                 SAPL_k_SID_TUNID);
    }

    /* if CFUNID shall be preserved */
    if (u8_attrBitMap & k_ATTR_BIT_MAP_PRESERVE_CFUNID)
    {
      /* empty branch, nothing to do here because data shall be preserved */
    }
    else
    {
      /* set CFUNID to default */
      CSS_MEMCPY(&s_WrBufBgrd.au8_cfunid[0], 
                 &SAPLnvLow_DefaultData.au8_cfunid[0], 
                 SAPL_k_SID_CFUNID);
    }

    /* if OCPUNID shall be preserved */
    if (u8_attrBitMap & k_ATTR_BIT_MAP_PRESERVE_OCPUNID)
    {
      /* empty branch, nothing to do here because data shall be preserved */
    }
    else
    {
      /* set to default */
      CSS_MEMCPY(&s_WrBufBgrd.au8_ocpunid[0], 
               &SAPLnvLow_DefaultData.au8_ocpunid[0], 
               SAPL_k_SDS_OCPUNID);
    }
    
    /* Unlock Flash Access */
    SAPL_NvLowRelease(&s_WrBufBgrd);
    
    /* update CRC */
    u16_WrBufCrc = CRC16Get();
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    CRC16Get

  Description:
    This function is used to calculate the CRC16 of the Write Buffer used by the background task 

  See also:
    -

  Parameters:
    -

  Return value:
    UINT16                  - CRC16

  Remarks:
    Context: Background Task

***************************************************************************************************/
STATIC UINT16 CRC16Get(void)
{
  /* Deactivate lint, cast required to call CRC routine (based on byte access) */
  return (aicCrc_Crc16((CONST UINT8*)&s_WrBufBgrd, sizeof(s_WrBufBgrd))); /*lint !e928*/
}

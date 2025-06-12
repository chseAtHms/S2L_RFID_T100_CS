/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLappObj.c
**     Summary: This module implements the application specific objects/functionalities of the 
**              T100/CS.
**   $Revision: 2408 $
**       $Date: 2017-03-22 12:41:54 +0100 (Mi, 22 Mrz 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_AppObjDiagMsgHandler
**             SAPL_AppObjDiagMaxTimeSliceSet
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

#include "tm_TemperatureMonitor-srv.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

#include "SAPLappObj.h"
#include "SAPLnvLow.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/*************************************************************/
/* Diagnostic Object, Class ID 0x0064                        */
/*************************************************************/
/* Attribute IDs used for class attributes */

/* Attribute IDs used for instance attributes [SRS_2125] */

/* Temperature */
#define k_DIAG_INST_ATTR_ID_TEMP                     (UINT16)0x0003 
/* Temp. Status own Channel */
#define k_DIAG_INST_ATTR_ID_STS_OWN_CH               (UINT16)0x0004 
/* Temp. Status other Channel */
#define k_DIAG_INST_ATTR_ID_STS_OTH_CH               (UINT16)0x0005 
/* Available NV memory block until power-cycle */
#define k_DIAG_INST_ATTR_AVAILABLE_FLASH_BLKS        (UINT16)0x000A 
/* Max. timeslice touched by background task */
#define k_DIAG_INST_ATTR_MAX_TIMESLICE               (UINT16)0x0014 

/* Defines the instance used for accessing the temperature diagnosis data */
#define k_DIAG_INST_ID                               (UINT16)1U     /* Instance used for Diag. */


/* Max. time slice reached by background task */
STATIC UINT8 u8_MaxTimesliceReached = 0u;

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_AppObjDiagMsgHandler

  Description:
    This function handles all explicit requests addressed to the vendor-specific Diagnostic
    object (Class ID 0x64) (see [SRS_2124]). The function checks if the requested services and 
    instances are implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Temperature (Instance Attribute)
     - Temperature Status own channel (Instance Attribute)
     - Temperature Status other channel (Instance Attribute)
     - Available NV data flash blocks (Instance Attribute)
     - Highest Time Slice touched by background task (Instance Attribute)

  See also:
    -

  Parameters:
    ps_explReq (IN)    - Structure containing the information that was received with the 
                         explicit request.
                         (not checked, administrated by CSS)
    ps_explResp (OUT)  - The result of the service must be placed into this structure.
                         (not checked, administrated by CSS)
    pau8_rspData (OUT) - Pointer to a buffer where this function must put the response data into.
                         **Attention**:
                         This function may only write up to CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
                         bytes into pb_rspData.
                         **Attention**:
                         On exit of this callback function ps_response->u16_rspDataLen must be set
                         to the number of bytes of the Response_Data or can be left unchanged in 
                         case the length of the Response_Data shall be zero.
                         (not checked, administrated by CSS)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_AppObjDiagMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                               CSS_t_EXPL_RSP *ps_explResp,
                               CSS_t_BYTE *pau8_rspData)
{
  /* if 'Class Service' (Instance=0) */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    /* Process Class Service */  
    
    /* Currently no Class Services are supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else 'Instance Service' */
  else
  {
    /* Process Instance Service */

    /* if addressed instance is not available */
    if (ps_explReq->u32_instance != k_DIAG_INST_ID)
    {
      /* Object does not exist */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
    /* else valid instance addressed */
    else
    {
      /* select service requested by originator */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* determine requested attribute */
          
          /* if attribute 'Temperature', see [SRS_2125] */
          if (ps_explReq->u16_attribute == k_DIAG_INST_ATTR_ID_TEMP)
          {
            UINT16 u16TempVal;
            
            /* get temperature values */
            __disable_irq();
            u16TempVal = tmSRV_GetAdcRawValue();
            __enable_irq();
            /* and write them to data array ("little-endian") */
            CSS_H2N_CPY16(pau8_rspData, &u16TempVal);
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }
          /* if attribute 'Temperature Status own Channel', see [SRS_2125] */
          else if (ps_explReq->u16_attribute == k_DIAG_INST_ATTR_ID_STS_OWN_CH)
          {
            
            /* the result of the temperature test (UINT16, value 0x3C3C or 0xC3C3)
            * is transmitted by one Byte only (value 0x3C or 0xC3) */
            __disable_irq();
            /* set response payload */
            /* Deactivate lint since pointer arithmetic is not really done here */
            /*lint -esym(960, 17.4) */
            pau8_rspData[0u] = (UINT8)((UINT16)(tmSRV_GetTempTestStatusOwnChannel()) & 0x00FFu);
            /*lint +esym(960, 17.4) */
            __enable_irq();
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }
          /* if attribute 'Temperature Status other Channel', see [SRS_2125] */
          else if (ps_explReq->u16_attribute == k_DIAG_INST_ATTR_ID_STS_OTH_CH)
          {
            /* the result of the temperature test (UINT16, value 0x3C3C or 0xC3C3)
            * is transmitted by one Byte only (value 0x3C or 0xC3) */
            __disable_irq();
            /* set response payload */
            /* Deactivate lint since pointer arithmetic is not really done here */
            /*lint -esym(960, 17.4) */
            pau8_rspData[0u] = (UINT8)((UINT16)(tmSRV_GetTempTestStatusOtherChannel()) & 0x00FFu);
            /*lint +esym(960, 17.4) */
            __enable_irq();
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }
          
          /* if attribute 'Available NV blocks ', see [SRS_2125] */
          else if (ps_explReq->u16_attribute == k_DIAG_INST_ATTR_AVAILABLE_FLASH_BLKS)
          {
            UINT16 u16_freeBlk = SAPL_NvLowFreeBlkGet();
            
            /* set response data ("little-endian") */
            CSS_H2N_CPY16(pau8_rspData, &u16_freeBlk);
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }

          /* if attribute 'Max time slice touched by background task' */
          else if (ps_explReq->u16_attribute == k_DIAG_INST_ATTR_MAX_TIMESLICE)
          {
            /* set response data ("little-endian") */
            CSS_H2N_CPY8(pau8_rspData, &u8_MaxTimesliceReached);

            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_USINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          }

          /* else attribute unknown/unsupported */
          else
          {
            /* attribute unknown/unsupported */
            ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
          } 
          break;
        }
        
        /* Unknown/unsupported service */
        default:
        {
          /* Instance Service not supported */
          ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
          break;
        }
      }
    }
  }
}

/***************************************************************************************************
  Function:
    SAPL_AppObjDiagMaxTimeSliceSet

  Description:
    This function is called to set the new max value of the reached time slice.
    It is task of the calling function to detect if a new max is reached.

  See also:
    -

  Parameters:
    u8_maxTimeSlice (IN) - new attribute value
                           (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: main

***************************************************************************************************/
void SAPL_AppObjDiagMaxTimeSliceSet(UINT8 u8_maxTimeSlice)
{
  u8_MaxTimesliceReached =u8_maxTimeSlice;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/



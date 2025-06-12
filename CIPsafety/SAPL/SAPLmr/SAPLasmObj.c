/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLasmObj.c
**     Summary: This module contains the implementation of the explicit messaging access to the 
**              assembly object for the Safety Application of the T100/CS.
**   $Revision: 2400 $
**       $Date: 2017-03-20 15:17:21 +0100 (Mo, 20 Mrz 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_AsmObjExplMsgHandler
**
**             AsmObjSrvcClass
**             AsmObjSrvcOutInst
**             AsmObjSrvcInInst
**             AsmObjSrvcCfgInst
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

#include "aicMsgDef.h"
#include "aicMgrTx.h"

#include "SpduOutData.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

#include "SAPLcfg.h"
#include "SAPLasmObj.h"

#include "gpio_cfg.h"
#include "fiparam.h"
#include "version.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* revision value of the assembly object class definition */
#define k_ASM_CLASS_REVISION_VALUE  2U

/* Attribute IDs of the Assembly Object Attributes (see [SRS_2182]) */
#define k_ASM_INST_ATTR_DATA        3U 
#define k_ASM_INST_ATTR_SIZE        4U


/** PRE-PROCESSOR CHECK **/
/* Ensure internal CSS buffer is configured big enough to hold the longest Explicit Messaging
** response (Configuration Assembly Object: SW Major version, HW ID, parameterization) */
/* Deactivated Lint Note 948: Operator '==' always evaluates to True */
/*lint -e(948) */
IXX_TASSERT( CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE >= SAPL_IO_CFG_ASSEMBLY_LEN, asmObj_chk1 )

/* Mute lint warning type not referenced. These types are just defined for
** being able to check the size of the elementary types. */
/*lint -esym(751, IXX_DUMMY_asmObj_chk1 ) not referenced - just for checks */

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void AsmObjSrvcClass(const CSS_t_EXPL_REQ *ps_explReq,
                            CSS_t_EXPL_RSP *ps_explResp,
                            CSS_t_BYTE *pau8_rspData);
STATIC void AsmObjSrvcOutInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData);
STATIC void AsmObjSrvcInInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData);
STATIC void AsmObjSrvcCfgInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_AsmObjExplMsgHandler

  Description:
    This function handles all explicit requests addressed to the assembly object (Class ID 0x04). 
    The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Data, Assembly Instance 0x300 (Instance Attribute)
     - Data, Assembly Instance 0x264 (Instance Attribute)
     - Data, Configuration Assembly Instance 0x340 (Instance Attribute)

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
void SAPL_AsmObjExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                               CSS_t_EXPL_RSP *ps_explResp,
                               CSS_t_BYTE *pau8_rspData)
{
  /* if Instance ID is Class designator */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    AsmObjSrvcClass(ps_explReq, ps_explResp, pau8_rspData);
  }
  /* else 'Instance Service' */
  else
  {
    /* if output assembly addressed */
    if (ps_explReq->u32_instance == SAPL_IO_OUT_ASSEMBLY_ID)
    {
      AsmObjSrvcOutInst(ps_explReq, ps_explResp, pau8_rspData);
    }
    /* else if input assembly addressed */
    else if (ps_explReq->u32_instance == SAPL_IO_IN_ASSEMBLY_ID)
    {
      AsmObjSrvcInInst(ps_explReq, ps_explResp, pau8_rspData);
    }
    /* else if configuration assembly addressed */
    else if (ps_explReq->u32_instance == SAPL_IO_CFG_ASSEMBLY_ID)
    {
      AsmObjSrvcCfgInst(ps_explReq, ps_explResp, pau8_rspData);
    }
    /* else invalid/unknown instance */
    else
    {
      /* object does not exist  */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
  }
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    AsmObjSrvcClass

  Description:
    This function provides the services for the assembly object class.

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
STATIC void AsmObjSrvcClass(const CSS_t_EXPL_REQ *ps_explReq,
                            CSS_t_EXPL_RSP *ps_explResp,
                            CSS_t_BYTE *pau8_rspData)
{
  /* temporary variable for copying attribute data to the explicit response */
  CSS_t_UINT u16_temp = 0U;

  /* if unsupported service, currently only Get_Attribute_Single supported */
  if (ps_explReq->u8_service != CSOS_k_CCSC_GET_ATTR_SINGLE)
  {
    /* Instance Service not supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else: Get_Attribute_Single */
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Class Revision */
      case CSOS_k_CCCA_REVISION:
      {
        u16_temp = k_ASM_CLASS_REVISION_VALUE;
        CSS_H2N_CPY16(pau8_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }
}


/***************************************************************************************************
  Function:
    AsmObjSrvcOutInst

  Description:
    This function provides the services for the OUTPUT assembly object instance.

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
STATIC void AsmObjSrvcOutInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData)
{
  /* if unsupported service, currently only Get_Attribute_Single supported */
  if (ps_explReq->u8_service != CSOS_k_CCSC_GET_ATTR_SINGLE)
  {
    /* Instance Service not supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else: Get_Attribute_Single */
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Attribute: Data (see [SRS_2182])*/
      case k_ASM_INST_ATTR_DATA:
      {
        AICMSGDEF_PS_OUT_DATA_STRUCT s_outIoVal;
        
        /** Attention: Data structure accessed from IRQ scheduler and background task. */
        __disable_irq();
        s_outIoVal = spduOutData_Get();
        __enable_irq();
        /* copy data and set size */
        /* Note 928: cast from pointer to pointer [MISRA 2004 Rule 11.4], OK because used 
         * for low level copy function */ 
        stdlibHAL_ByteArrCopy(pau8_rspData, 
                              (const UINT8*)&s_outIoVal, /*lint !e928 */
                              (UINT16)sizeof(s_outIoVal));
        ps_explResp->u16_rspDataLen = SAPL_IO_OUT_ASSEMBLY_LEN;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute Size (see [SRS_2182]) */
      case k_ASM_INST_ATTR_SIZE:
      {
        UINT16 u16_temp = SAPL_IO_OUT_ASSEMBLY_LEN;
        
        /* set size of object */
        CSS_H2N_CPY16(pau8_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }
}
/***************************************************************************************************
  Function:
    AsmObjSrvcInInst

  Description:
    This function provides the services for the INPUT assembly object instance.

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
STATIC void AsmObjSrvcInInst(const CSS_t_EXPL_REQ *ps_explReq,
                             CSS_t_EXPL_RSP *ps_explResp,
                             CSS_t_BYTE *pau8_rspData)
{
  /* if unsupported service, currently only Get_Attribute_Single supported */
  if (ps_explReq->u8_service != CSOS_k_CCSC_GET_ATTR_SINGLE)
  {
    /* Instance Service not supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else: Get_Attribute_Single */
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Attribute: Data (see [SRS_2181])*/
      case k_ASM_INST_ATTR_DATA:
      {
        /** Attention: Data structure accessed from IRQ scheduler and background task. */
        __disable_irq();
        /* Note 928: cast from pointer to pointer [MISRA 2004 Rule 11.4], OK because used 
         * for low level copy function */ 
        stdlibHAL_ByteArrCopy(pau8_rspData, 
                              (const volatile UINT8*)&aicMgrTx_s_IoVal, /*lint !e928 */
                              (UINT16)sizeof(aicMgrTx_s_IoVal));
        __enable_irq();

        ps_explResp->u16_rspDataLen = SAPL_IO_IN_ASSEMBLY_LEN;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute Size (see [SRS_2181]) */
      case k_ASM_INST_ATTR_SIZE:
      {
        UINT16 u16_temp = SAPL_IO_IN_ASSEMBLY_LEN;

        /* set size of objects */
        CSS_H2N_CPY16(pau8_rspData, &u16_temp);
        
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }
}

/***************************************************************************************************
  Function:
    AsmObjSrvcCfgInst

  Description:
    This function provides the services for the CONFIGURATION assembly object instance.

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
STATIC void AsmObjSrvcCfgInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData)
{
  /* if unsupported service, currently only Get_Attribute_Single supported */
  if (ps_explReq->u8_service != CSOS_k_CCSC_GET_ATTR_SINGLE)
  {
    /* Instance Service not supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else: Get_Attribute_Single */
  else
  {
    switch (ps_explReq->u16_attribute)
    {
      /* Attribute: Data (see [SRS_2120])*/
      case k_ASM_INST_ATTR_DATA:
      {
        UINT8   u8SwMajorVersion = VERSION_SW_VERSION_MAJOR;
        UINT16  u16HwId = VERSION_ASM_MODULE_ID;

        CSS_H2N_CPY8(&pau8_rspData[0], &u8SwMajorVersion);
        CSS_H2N_CPY16(&pau8_rspData[1], &u16HwId);

        /* Read out current parametrization from 'pkgParam' */
        __disable_irq();
        /* Deactivate lint since pointer is wanted here */
        /*lint -esym(960, 17.4) */
        /* Note 928: cast from pointer to pointer [MISRA 2004 Rule 11.4], OK because used 
         * for low level copy function */ 
        stdlibHAL_ByteArrCopy(&pau8_rspData[3], 
                              (const volatile UINT8*)&fiParam_sIParam, /*lint !e928*/
                              (UINT16)sizeof(fiParam_sIParam));
        /*lint +esym(960, 17.4) */
        __enable_irq();

        ps_explResp->u16_rspDataLen = SAPL_IO_CFG_ASSEMBLY_LEN;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute Size (see [SRS_2120]) */
      case k_ASM_INST_ATTR_SIZE:
      {
        UINT16 u16_temp = SAPL_IO_CFG_ASSEMBLY_LEN;
          
        /* set size of objects */
        CSS_H2N_CPY16(pau8_rspData, &u16_temp);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      default:
      {
        /* attribute not supported */
        ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
        break;
      }
    }
  }
}

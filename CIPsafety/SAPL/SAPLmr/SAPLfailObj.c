/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLfailObj.c
**     Summary: This module contains the implementation of the explicit messaging access to the 
**              failure code object for the Safety Application of the T100/CS.
**   $Revision: 2048 $
**       $Date: 2017-02-07 12:33:59 +0100 (Di, 07 Feb 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_FailObjExplMsgHandler
**
**             FailObjSrvcClass
**             FailObjSrvcInst
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

#include "aicMsgDef.h"
#include "aicMgrTx.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

#include "SAPLfailObj.h"
#include "SAPLfailCode.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/


/* Class Attribute ID of the Failure Code Object (see [SRS_2227]) */
#define k_FAIL_CLASS_ATTR_NUM_FAIL_OWN       8u
#define k_FAIL_CLASS_ATTR_NUM_FAIL_OTHER     9u

/* Instance Attribute IDs of the Failure Code Object (see [SRS_2228]) */
#define k_FAIL_INST_ATTR_FAILCODE_OWN        1U
#define k_FAIL_INST_ATTR_ADDINFO_OWN         2U
#define k_FAIL_INST_ATTR_FAILCODE_OTHER      3U
#define k_FAIL_INST_ATTR_ADDINFO_OTHER       4U

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/

STATIC void FailObjSrvcClass(const CSS_t_EXPL_REQ *ps_explReq,
                            CSS_t_EXPL_RSP *ps_explResp,
                            CSS_t_BYTE *pau8_rspData);
STATIC void FailObjSrvcInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData);

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_FailObjExplMsgHandler

  Description:
    This function handles all explicit requests addressed to the failure code object (Class ID 0x65)
    The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Number Failures Own Controller, (Class Attribute)
     - Number Failures Other Controller, (Class Attribute)

     - Failure Code Own Controller, (Instance Attribute)
     - Additional Info Own Controller, (Instance Attribute)
     - Failure Code Other Controller, (Instance Attribute)
     - Additional Info Other Controller, (Instance Attribute)

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
void SAPL_FailObjExplMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE *pau8_rspData)
{
  /* if Instance ID is Class designator */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    FailObjSrvcClass(ps_explReq, ps_explResp, pau8_rspData);
  }
  /* else 'Instance Service' */
  else
  {
    /* if valid Failure Code instance addressed, (see [SRS_2229])  */
    if (ps_explReq->u32_instance <=  SAPL_k_NUM_READABLE_FAILURES)
    {
      FailObjSrvcInst(ps_explReq, ps_explResp, pau8_rspData);
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
    FailObjSrvcClass

  Description:
    This function provides the services for the Failure Code class.

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
STATIC void FailObjSrvcClass(const CSS_t_EXPL_REQ *ps_explReq,
                            CSS_t_EXPL_RSP *ps_explResp,
                            CSS_t_BYTE *pau8_rspData)
{
  /* get pointer to Failure Log */
  CONST SAPL_t_FAILURE_LOG* ps_failureLog = SAPL_FailCodeLogGet();

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
      /* Number of Failure Codes own controller (see [SRS_2227]) */
      case k_FAIL_CLASS_ATTR_NUM_FAIL_OWN:
      {
        CSS_H2N_CPY16(pau8_rspData, &ps_failureLog->u16_numFailOwnCtrl);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }

      /* Number of Failure Codes own controller (see [SRS_2227]) */
      case k_FAIL_CLASS_ATTR_NUM_FAIL_OTHER:
      {
        CSS_H2N_CPY16(pau8_rspData, &ps_failureLog->u16_numFailOtherCtrl);
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
    FailObjSrvcInst

  Description:
    This function provides the services for the Failure Code object instances.

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
STATIC void FailObjSrvcInst(const CSS_t_EXPL_REQ *ps_explReq,
                              CSS_t_EXPL_RSP *ps_explResp,
                              CSS_t_BYTE *pau8_rspData)
{
  /* get pointer to Failure Log */
  CONST SAPL_t_FAILURE_LOG* ps_failureLog = SAPL_FailCodeLogGet();

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
      /* Attribute: Failure Code Own Controller (see [SRS_2228])*/
      case k_FAIL_INST_ATTR_FAILCODE_OWN:
      {
        CSS_H2N_CPY16(pau8_rspData, 
                      &ps_failureLog->as_failure[ps_explReq->u32_instance-1u].u16_failCodeOwn);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute: Additional Info Own Controller (see [SRS_2228])*/
      case k_FAIL_INST_ATTR_ADDINFO_OWN:
      {
        CSS_H2N_CPY32(pau8_rspData, 
                      &ps_failureLog->as_failure[ps_explReq->u32_instance-1u].u32_addInfoOwn);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UDINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute: Failure Code Other Controller (see [SRS_2228])*/
      case k_FAIL_INST_ATTR_FAILCODE_OTHER:
      {
        CSS_H2N_CPY16(pau8_rspData, 
                      &ps_failureLog->as_failure[ps_explReq->u32_instance-1u].u16_failCodeOther);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
        ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
        break;
      }
      /* Attribute: Additional Info Other Controller (see [SRS_2228])*/
      case k_FAIL_INST_ATTR_ADDINFO_OTHER:
      {
        CSS_H2N_CPY32(pau8_rspData, 
                      &ps_failureLog->as_failure[ps_explReq->u32_instance-1u].u32_addInfoOther);
        ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UDINT;
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

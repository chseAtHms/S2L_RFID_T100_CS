/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLmrObj.c
**     Summary: This module implements the Message Router object of the T100/CS.
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_IxsmrObjReqClbk
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

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

/* HALCS headers */
#include "HALCSapi.h"

/* CSS headers */
#include "IXSMRapi.h"

/* SAPL Message router headers */
#include "SAPLasmObj.h"
#include "SAPLprofObj.h"
#include "SAPLappObj.h"
#include "SAPLfailObj.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/*******************************************************************************
**
** Function    : SAPL_IxsmrObjReqClbk
**
** Description : This callback function must be implemented by the application.
**               The CIP Safety Stack calls this function when a service request
**               to an application object was received (see Asm.5.3.1-1 and
**               Req.5.3.1-1). The application now needs to evaluate the
**               received request and provide an appropriate response.
**
**               **Attention**: If the request is a configuration message (e.g.
**               Set_Attribute_Single to an attribute that is relevant for the
**               safety device configuration) then the application object shall
**               reject the request if the device is in an operational mode
**               (e.g. has active connection or is in executing mode)
**               (see FRS116-3).
**
** Context     : IXSMR_CmdProcess()
**
** See Also    : -
**
** Parameters  : ps_request (IN)   - Structure containing the information that
**                                   was received with the explicit request.
**               ps_response (OUT) - The result of the service must be placed
**                                   into this structure.
**               pb_rspData (OUT)  - pointer to a buffer where this function
**                                   must put the response data into.
**                                   **Attention**:
**                                   This function may only write up to
**                                   CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
**                                   bytes into pb_rspData.
**                                   **Attention**:
**                                   On exit of this callback function
**                                   ps_response->u16_rspDataLen must be set
**                                   to the number of bytes of the Response_Data
**                                   or can be left unchanged in case the length
**                                   of the Response_Data shall be zero.
**
** Returnvalue : CSS_k_TRUE        - Application was able to service the request
**               CSS_k_FALSE       - Application could not service the request
**
*******************************************************************************/
CSS_t_BOOL SAPL_IxsmrObjReqClbk(const CSS_t_EXPL_REQ *ps_request,
                                CSS_t_EXPL_RSP *ps_response,
                                CSS_t_BYTE *pb_rspData)
{
  /* Callback required according to [CS_MAN], (see [3.5-2:]) */

  /* return value of this function */
  CSS_t_BOOL b_retVal = CSS_k_FALSE;

  /* call proper function to support Explicit Messaging (see [SRS_2079]) */

  /* route the request to the given Class ID */
  switch (ps_request->u16_class)
  {

    /*******************************************************/
    /***** Assembly object/class ***************************/
    /*******************************************************/
    case CSOS_k_CCC_ASSEMBLY_OBJ:
    {
      SAPL_AsmObjExplMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }

    /*******************************************************/
    /***** Profile specific objects/classes, see [1.4-2:] **/
    /*******************************************************/
    case SAPL_PROFOBJ_k_SDIP:
    {
      /* route to Safety Discrete Input Point handler */
      SAPL_ProfObjSdipMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }
    case SAPL_PROFOBJ_k_SDOP:
    {
      /* route to Safety Discrete Output Point handler */
      SAPL_ProfObjSdopMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }
    case SAPL_PROFOBJ_k_SDIG:
    {
      /* route to Safety Discrete Input Group handler */
      SAPL_ProfObjSdigMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }
    case SAPL_PROFOBJ_k_SDCO:
    {
      /* route to Safety Dual Channel Output handler */
      SAPL_ProfObjSdcoMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }

    /*******************************************************/
    /***** Application/Vendor specific objects/classes *****/
    /*******************************************************/
    case SAPL_APPOBJ_k_DIAG:
    {
      /* route to vendor-specific diagnostic handler */
      SAPL_AppObjDiagMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }

    case SAPL_APPOBJ_k_FAILURECODE:
    {
      /* route to vendor-specific failure code handler */
      SAPL_FailObjExplMsgHandler(ps_request, ps_response, pb_rspData);
      /* indicate that this request has been serviced */
      b_retVal = CSS_k_TRUE;
      break;
    }

    /* unknown ClassId */
    default:
    {
      /* Error: object does not exist. This can only occur if the ABCC forwards a 
      ** request to a ClassId that was not registered via GetClassId message (see [SIS_CS]).
      ** Because CSS_k_FALSE is returned, an entry is added to the 'Error Event Log' */
      ps_response->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
  }

  return b_retVal;
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/



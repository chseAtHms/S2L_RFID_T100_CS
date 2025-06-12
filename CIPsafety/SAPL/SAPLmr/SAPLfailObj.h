/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLfailObj.h
**     Summary: Interface for the access of the Failure Code object located on the T100.
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
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_FAILOBJ_H
#define SAPL_FAILOBJ_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Failure Code object Class ID (see [SRS_2226]) */
#define SAPL_APPOBJ_k_FAILURECODE      (UINT16)0x0065

/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
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
                                CSS_t_BYTE *pau8_rspData);

#endif


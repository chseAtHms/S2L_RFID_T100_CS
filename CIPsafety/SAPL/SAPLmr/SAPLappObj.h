/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLappObj.h
**     Summary: Interface for application specific (vendor-specific) objects located on the T100.
**   $Revision: 2400 $
**       $Date: 2017-03-20 15:17:21 +0100 (Mo, 20 Mrz 2017) $
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

#ifndef SAPL_APPOBJ_H
#define SAPL_APPOBJ_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/**********************************************************************************/
/* Definition of objects/classes used by the 'Safety Discrete I/O Device' profile */
/**********************************************************************************/

/* Vendor-specific Diagnostic object */
#define SAPL_APPOBJ_k_DIAG          (UINT16)0x0064

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
                               CSS_t_BYTE *pau8_rspData);

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
void SAPL_AppObjDiagMaxTimeSliceSet(UINT8 u8_maxTimeSlice);

#endif


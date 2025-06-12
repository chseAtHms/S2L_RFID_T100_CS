/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLprofObj.h
**     Summary: Interface for profile specific objects located on the T100.
**   $Revision: 1695 $
**       $Date: 2017-01-12 13:50:42 +0100 (Do, 12 Jan 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_ProfObjSdipMsgHandler
**             SAPL_ProfObjSdopMsgHandler
**             SAPL_ProfObjSdigMsgHandler
**             SAPL_ProfObjSdcoMsgHandler
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_PROFOBJ_H
#define SAPL_PROFOBJ_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/**********************************************************************************/
/* Definition of objects/classes used by the 'Safety Discrete I/O Device' profile */
/**********************************************************************************/

/* Safety Discrete Input Point (SDIG) */
#define SAPL_PROFOBJ_k_SDIP          (UINT16)0x003D
/* Safety Discrete Output Point (SDOP) */
#define SAPL_PROFOBJ_k_SDOP          (UINT16)0x003B
/* Safety Discrete Input Group (SDIG) */
#define SAPL_PROFOBJ_k_SDIG          (UINT16)0x003E
/* Safety Dual Channel Output Object (SDCO) */
#define SAPL_PROFOBJ_k_SDCO          (UINT16)0x003F

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
    SAPL_ProfObjSdipMsgHandler

  Description:
    This function handles all explicit requests addressed to the Safety Discrete Input Point 
    object (Class ID 0x3D). The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Latch Input Error Time (Class Attribute)
     - Safety Status (Instance Attribute)
     - Safety Input Logical Value (Instance Attribute)     

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
void SAPL_ProfObjSdipMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE *pau8_rspData);
                                      
/***************************************************************************************************
  Function:
    SAPL_ProfObjSdopMsgHandler

  Description:
    This function handles all explicit requests addressed to the Safety Discrete Output Point 
    object (Class ID 0x3B). The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Latch Output Error Time (Class Attribute)
     - Safety Output Value (Instance Attribute)
     - Output Monitor Value (Instance Attribute)
     - Safety Status (Instance Attribute)

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
void SAPL_ProfObjSdopMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE *pau8_rspData);                              
                                     
/***************************************************************************************************
  Function:
    SAPL_ProfObjSdigMsgHandler

  Description:
    This function handles all explicit requests addressed to the Safety Discrete Input Group 
    object (Class ID 0x3E). The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Safety Status (Instance Attribute)

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
void SAPL_ProfObjSdigMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE *pau8_rspData);                               
                                      
/***************************************************************************************************
  Function:
    SAPL_ProfObjSdcoMsgHandler

  Description:
    This function handles all explicit requests addressed to the Safety Dual Channel Output
    object (Class ID 0x3F). The function checks if the requested services and instances are 
    implemented. It is task of the function to generate a proper response.
    
    Currently the following services are implemented:
     - Get_Attribute_Single
     
     Currently the following class/instance attributes are implemented:
     - Dual Channel Mode (Instance Attribute)

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
void SAPL_ProfObjSdcoMsgHandler(const CSS_t_EXPL_REQ *ps_explReq,
                                CSS_t_EXPL_RSP *ps_explResp,
                                CSS_t_BYTE *pau8_rspData);

#endif


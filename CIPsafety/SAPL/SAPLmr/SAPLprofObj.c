/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLprofObj.c
**     Summary: This module implements the profile specific objects/functionalities of the 
**              T100/CS according to the 'Safety Discrete I/O Device' profile. [SRS_2085], [1.4-2:]
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
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
#include "rds.h"

#include "includeDiDo.h"
#include "fiParam.h"

#include "aicMsgDef.h"
#include "aicSm.h"

#include "SpduOutData.h"

/* CSOS common headers */
#include "CSOScfg.h"
#include "CSOSapi.h"
/* CSS common headers */
#include "CSScfg.h"
#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"

#include "SAPLprofObj.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Macro is used to check if DI/DO processing is done in current state, 
** DI/DO processing is only done in RUN scheduler means in AICSM_AIC_IDLE,
** AICSM_AIC_EXEC_PROD_CONS, AICSM_AIC_EXEC_PROD_ONLY, AICSM_AIC_EXEC_CONS_ONLY
** (see [SRS_2231], [SRS_2232])
*/
#define DIDO_ACTIVE(state)     ( ((state) == AICSM_AIC_IDLE) || \
                                 ((state) == AICSM_AIC_EXEC_PROD_CONS) || \
                                 ((state) == AICSM_AIC_EXEC_PROD_ONLY) || \
                                 ((state) == AICSM_AIC_EXEC_CONS_ONLY) )

/* Macros used to get the internal used index from the passed instance id */
#define SINGLE_CH_IDX(x)       ( (x)-1u )

/* Macros used to get the internal used Dual Channel DI index from the passed instance id
** (first controller, C1), (Inst0 is not valid) */
#define DUAL_CH_DI_IDX_C1(x)   ( DICFG_GET_DI_NUM_C1((x)-1u) )
/* Macros used to get the internal used Dual Channel DI index from the passed instance id
** (second controller, C2) (Inst0 is not valid) */
#define DUAL_CH_DI_IDX_C2(x)   ( DICFG_GET_DI_NUM_C2((x)-1u) )

/*  Macros used to get the internal used index from the passed instance id (first controller, C1)
** e.g. Inst1 -> DO0, Inst2 -> DO2, Inst3 -> DO4, ..., (Inst0 is not valid)
** The macro always delivers the first assigned DO (of controller 1) */
#define DUAL_CH_DO_IDX_C1(x)   ( ((x)*2u)-2u )

/*************************************************************/
/* Safety Discrete Input Point (SDIP), Class ID 0x003D       */
/*************************************************************/
/* Attribute IDs used for class attributes [SRS_2088] */
#define k_SDIP_CLASS_ATTR_ID_LTCH_INP_ERR_TIME       (UINT16)0x0008 /* Latch Input Time Error */
/* Attribute IDs used for instance attributes [SRS_2089] */
#define k_SDIP_INST_ATTR_ID_SAFETY_STS               (UINT16)0x0004 /* Safety Status */
#define k_SDIP_INST_ATTR_ID_SAFETY_INP_LOG_VAL       (UINT16)0x0007 /* Safety Input Logical Value */
/* Number of SDIP instances. Each input is represented by an SDIP instance. This means the current 
** T100/CS implements 6 SDIP instances [SRS_2087] */
#define k_SDIP_NUM_OF_INST                           0x06u    /* SDIP instances */


/*************************************************************/
/* Safety Discrete Output Point (SDOP), Class ID 0x003B      */
/*************************************************************/
/* Attribute IDs used for class attributes [SRS_2095] */
#define k_SDOP_CLASS_ATTR_ID_LTCH_OUTP_ERR_TIME      (UINT16)0x0008 /* Latch Output Time Error */
/* Attribute IDs used for instance attributes [SRS_2096] */
#define k_SDOP_INST_ATTR_ID_SAFETY_OUTP_VAL          (UINT16)0x0003 /* Safety Output Value */
#define k_SDOP_INST_ATTR_ID_SAFETY_OUTP_MON_VAL      (UINT16)0x0004 /* Safety Output Monitor Value*/
#define k_SDOP_INST_ATTR_ID_SAFETY_STS               (UINT16)0x0005 /* Safety Status */
/* Number of SDOP instances. Each output is represented by an SDOP instance. This means the current 
** T100/CS implements 2 SDOP instances [SRS_2094] */
#define k_SDOP_NUM_OF_INST                           0x02u    /* SDOP instances */


/*************************************************************/
/* Safety Discrete Input Group (SDIG), Class ID 0x003E       */
/*************************************************************/
/* Attribute IDs used for class attributes */

/* Attribute IDs used for instance attributes [SRS_2091] */
#define k_SDIG_INST_ATTR_ID_SAFETY_STS               (UINT16)0x0005 /* Safety Status */
/* Number of SDIG instances. Each pair of SDIP is represented by an SDIG instance. This means the
**  current  T100/CS implements 3 SDIG instances [SRS_2090] */
#define k_SDIG_NUM_OF_INST                           0x03u    /* SDIG instances */


/*************************************************************/
/* Safety Dual Channel Output Object (SDCO), Class ID 0x003F */
/*************************************************************/
/* Attribute IDs used for class attributes */

/* Attribute IDs used for instance attributes [SRS_2098] */
#define k_SDCO_INST_ATTR_ID_DUAL_CH_MODE             (UINT16)0x0003 /* Dual Channel Mode */
/* Number of SDCO instances. Each pair of SDOP is represented by an SDCO instance. This means the 
** current  T100/CS implements 1 SDCO instance [SRS_2097] */
#define k_SDCO_NUM_OF_INST                           0x01u    /* SDCO instances */

/* Semantics of Value used for Safety Discrete I/O Profile */
#define k_SEMANTICS_VALUE_OFF                        (UINT8)0
#define k_SEMANTICS_VALUE_ON                         (UINT8)1
#define k_SEMANTICS_VALUE_ALARM                      (UINT8)0 
#define k_SEMANTICS_VALUE_OK                         (UINT8)1
#define k_SEMANTICS_VALUE_SINGLE_CH                  (UINT8)0
#define k_SEMANTICS_VALUE_DUAL_CH                    (UINT8)1


#if (k_SDIP_NUM_OF_INST != GPIOCFG_NUM_DI_TOT)
  #error "SDIP Object shall be adapted to new value for GPIOCFG_NUM_DI_TOT"
#endif
#if (k_SDOP_NUM_OF_INST != GPIOCFG_NUM_DO_TOT)
  #error "SDOP Object shall be adapted to new value for GPIOCFG_NUM_DO_TOT"
#endif
#if (k_SDIG_NUM_OF_INST != GPIOCFG_NUM_DI_PER_uC)
  #error "SDIG Object shall be adapted to new value for GPIOCFG_NUM_DI_PER_uC"
#endif
#if (k_SDCO_NUM_OF_INST != GPIOCFG_NUM_DO_PER_uC)
  #error "SDCO Object shall be adapted to new value for GPIOCFG_NUM_DO_PER_uC"
#endif

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
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
                                CSS_t_BYTE *pau8_rspData)
{
  /* temporary variable for copying attribute data to the response buffer */
  UINT16 u16_tmpVal;
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
   
  /* if 'Class Service' (Instance=0) */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    /* if no DI/DO processing active, */
    if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2231] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else: valid state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* Latch Input Error Time, see [SRS_2088] */ 
          if (ps_explReq->u16_attribute == k_SDIP_CLASS_ATTR_ID_LTCH_INP_ERR_TIME)
          {
            /* set payload */
            __disable_irq();
            u16_tmpVal = FIPARAM_DI_LATCH_ERROR_TIME;
            __enable_irq();
            CSS_H2N_CPY16(pau8_rspData, &u16_tmpVal);
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          }
          else
          {
            /* attribute unknown/unsupported */
            ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
          }
          break;
        }
        default:
        {
          /* Class Service not supported */
          ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
          break;
        }
      }
    }
  }
  /* else 'Instance Service' */
  else
  {
    /* if addressed instance is not available */
    if (ps_explReq->u32_instance > k_SDIP_NUM_OF_INST)
    {
      /* Object does not exist */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
    /* else if: no DI/DO processing active, */
    else if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2231] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else valid instance/state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* determine requested attribute */
          
          /* if attribute 'Safety Status', see [SRS_2089] */
          if (ps_explReq->u16_attribute == k_SDIP_INST_ATTR_ID_SAFETY_STS)
          {
            /* access to diErrHdl_sDiErrHdl is done via volatile pointer because data is modified
            ** in IRQ scheduler */
            volatile DIERRHDL_ERR_STRUCT* ps_diErrHdl = &diErrHdl_sDiErrHdl;

            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
            /* if 'test OK' or 'test not executed' means 'OK, '1' */
            /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand 
             * of logical operator: '||' --> no side effects because IRQs disabled here */
            if ( (ps_diErrHdl->eDiDiagQualifier[SINGLE_CH_IDX(ps_explReq->u32_instance)] 
                    == eGPIO_DIAG_OK) ||
                 (ps_diErrHdl->eDiDiagQualifier[SINGLE_CH_IDX(ps_explReq->u32_instance)] 
                    == eGPIO_DIAG_NA) ) /*lint !e960 */
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OK;
              /*lint +esym(960, 17.4) */
              
            }
            /* else means 'ALARM', '0' */
            else
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ALARM;
              /*lint +esym(960, 17.4) */
            }
            __enable_irq();
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          }
      
          /* if attribute 'Safety Input Logical value', see [SRS_2089] */
          else if (ps_explReq->u16_attribute == k_SDIP_INST_ATTR_ID_SAFETY_INP_LOG_VAL)
          {
            GPIO_STATE_ENUM e_diState; 
            
            /** Attention: Data structures used inside diState_GetDiState() are accessed from 
            ** IRQ scheduler. So it is required to disable scheduler temporarily here.
            */
            __disable_irq(); 
            /* get DI state */
            e_diState = diState_GetDiState((UINT8)SINGLE_CH_IDX(ps_explReq->u32_instance));
            /* enable IRQ scheduler again */
            __enable_irq();
            
            /* if DI is 'active', means 'ON, '1' */
            if ( e_diState == DICFG_DI_ACTIVE)
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ON;
              /*lint +esym(960, 17.4) */
            }
            /* else means 'OFF', '0' */
            else
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OFF;
              /*lint +esym(960, 17.4) */
            }
            
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
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
                                CSS_t_BYTE *pau8_rspData)
{
  /* temporary variable for copying attribute data to the response buffer */
  UINT16 u16_tmpVal;
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /* if 'Class Service' (Instance=0) */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    /* if no DI/DO processing active, */
    if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2232] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else: valid state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* Latch Output Error Time, see [SRS_2095] */ 
          if (ps_explReq->u16_attribute == k_SDOP_CLASS_ATTR_ID_LTCH_OUTP_ERR_TIME)
          {
            /* set payload */
            __disable_irq();
            u16_tmpVal = FIPARAM_DO_LATCH_ERROR_TIME;
            __enable_irq();
            CSS_H2N_CPY16(pau8_rspData, &u16_tmpVal);
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_UINT;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
          }
          else
          {
            /* attribute unknown/unsupported */
            ps_explResp->u8_genStat = CSOS_k_CGSC_ATTR_NOT_SUP;
          }
          break;
        }
        default:
        {
          /* Class Service not supported */
          ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
          break;
        }
      }
    }
  }
  /* else 'Instance Service' */
  else
  {
    /* if addressed instance is not available */
    if (ps_explReq->u32_instance > k_SDOP_NUM_OF_INST)
    {
      /* Object does not exist */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
    /* else if no DI/DO processing active, */
    else if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2232] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else valid instance/state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* determine requested attribute */
          
          /* if attribute 'Safety Output Value', see [SRS_2096] */
          if (ps_explReq->u16_attribute == k_SDOP_INST_ATTR_ID_SAFETY_OUTP_VAL)
          {
            AICMSGDEF_PS_OUT_DATA_STRUCT s_outIoVal;
            
            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
             s_outIoVal = spduOutData_Get();
            __enable_irq();
            
            /* if bit of requested channel/instance is set inside bit field, means 'ON, '1' */
            if (s_outIoVal.u8DoVal & ((UINT8)(0x01u << (SINGLE_CH_IDX(ps_explReq->u32_instance)))))
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ON;
              /*lint +esym(960, 17.4) */
            }
            /* else bit not set, means 'OFF', '0' */
            else
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OFF;
              /*lint +esym(960, 17.4) */
            }
            
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }
          /* if attribute 'Output Monitor Value', see [SRS_2096] */
          else if (ps_explReq->u16_attribute == k_SDOP_INST_ATTR_ID_SAFETY_OUTP_MON_VAL)
          {
            /* access to diInput_sInputValues is done via volatile pointer because data is modified
            ** in IRQ scheduler */
            volatile DIINPUT_VALUES_STRUCT* ps_diInput = &diInput_sInputValues;
            
            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
            /* if 'low pin-state' means 'ON, '1' */
            if (ps_diInput->eDoTest[SINGLE_CH_IDX(ps_explReq->u32_instance)] ==  eGPIO_LOW)
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ON;
              /*lint +esym(960, 17.4) */
             
            }
            /* else bit not set, means 'OFF', '0' */
            else
            {
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OFF;
              /*lint +esym(960, 17.4) */
            }
            __enable_irq();
            
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
            ps_explResp->u8_genStat = CSOS_k_CGSC_SUCCESS;
            
          }
          /* if attribute 'Safety Status', see [SRS_2096] */
          else if (ps_explReq->u16_attribute == k_SDOP_INST_ATTR_ID_SAFETY_STS)
          {
            /* access to doErrHdl_sDoErrHdl is done via volatile pointer because data is modified
            ** in IRQ scheduler */
            volatile DO_ERR_STRUCT* ps_doErrHdl = &doErrHdl_sDoErrHdl;
            
            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
            /* if 'test OK' or 'test not executed' means 'OK, '1' */
            /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand 
             * of logical operator: '||' --> no side effects because IRQs disabled here */
            if ( (ps_doErrHdl->eDoDiagQualifier[SINGLE_CH_IDX(ps_explReq->u32_instance)] 
                    == eGPIO_DIAG_OK) ||
                 (ps_doErrHdl->eDoDiagQualifier[SINGLE_CH_IDX(ps_explReq->u32_instance)] 
                    == eGPIO_DIAG_NA) ) /*lint !e960 */
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OK;
              /*lint +esym(960, 17.4) */
              
            }
            /* else bit not set, means 'ALARM', '0' */
            else
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ALARM;
              /*lint +esym(960, 17.4) */
            }
            __enable_irq();
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
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
                                CSS_t_BYTE *pau8_rspData)
{
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /* if 'Class Service' (Instance=0) */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    
    /* Currently no Class Services are supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else 'Instance Service' */
  else
  {
    /* if addressed instance is not available */
    if (ps_explReq->u32_instance > k_SDIG_NUM_OF_INST)
    {
      /* Object does not exist */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
    /* if no DI/DO processing active, */
    else if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2231] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else valid instance/state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* determine requested attribute */
          
          /* if attribute 'Safety Status', see [SRS_2091] */
          if (ps_explReq->u16_attribute == k_SDIG_INST_ATTR_ID_SAFETY_STS)
          {
            /* access to diErrHdl_sDiErrHdl is done via volatile pointer because data is modified
            ** in IRQ scheduler */
            volatile DIERRHDL_ERR_STRUCT* ps_diErrHdl = &diErrHdl_sDiErrHdl;
            
            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
            /* if 'test OK' or 'test not executed' means 'OK, '1' */
            /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand  
             * of logical operator: '||' or '&&' --> no side effects because IRQs disabled here */
            if ( ((ps_diErrHdl->eDiDiagQualifier[DUAL_CH_DI_IDX_C1(ps_explReq->u32_instance)] 
                     == eGPIO_DIAG_OK) ||
                  (ps_diErrHdl->eDiDiagQualifier[DUAL_CH_DI_IDX_C1(ps_explReq->u32_instance)] 
                     == eGPIO_DIAG_NA) ) && /*lint !e960 */
                 ((ps_diErrHdl->eDiDiagQualifier[DUAL_CH_DI_IDX_C2(ps_explReq->u32_instance)] 
                     == eGPIO_DIAG_OK) ||   
                  (ps_diErrHdl->eDiDiagQualifier[DUAL_CH_DI_IDX_C2(ps_explReq->u32_instance)] 
                     == eGPIO_DIAG_NA) ) )  /*lint !e960 */
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_OK;
              /*lint +esym(960, 17.4) */
              
            }
            /* else bit not set, means 'ALARM', '0' */
            else
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_ALARM;
              /*lint +esym(960, 17.4) */
            }
            __enable_irq();
         
            /* set response header */
            ps_explResp->u16_rspDataLen = CSOS_k_SIZEOF_BOOL;
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
                                CSS_t_BYTE *pau8_rspData)
{
  /* Current ASM state, variable is accessed by IRQ Scheduler Task but read access is 
  ** considered as 'atomic' here */
  AICSM_STATE_ENUM e_aicState = aicSm_eAicState;
  
  /* if 'Class Service' (Instance=0) */
  if (ps_explReq->u32_instance == CSOS_k_CLASS_DESIGNATOR)
  {
    
    /* Currently no Class Services are supported */
    ps_explResp->u8_genStat = CSOS_k_CGSC_SERVICE_NOT_SUP;
  }
  /* else 'Instance Service' */
  else
  {
    /* if addressed instance is not available */
    if (ps_explReq->u32_instance > k_SDCO_NUM_OF_INST)
    {
      /* Object does not exist */
      ps_explResp->u8_genStat = CSOS_k_CGSC_OBJ_DOES_NOT_EXIST;
    }
    /* else if no DI/DO processing active, */
    else if (!DIDO_ACTIVE(e_aicState))
    {
      /* Device State Conflict (no DI/DO processing), see [SRS_2232] */
      ps_explResp->u8_genStat = CSOS_k_CGSC_DEV_STATE_CONFLICT;
    }
    /* else valid instance/state */
    else
    {
      /* select service */
      switch (ps_explReq->u8_service)
      {
        /* 'Get_Attribute_Single' Service */
        case CSOS_k_CCSC_GET_ATTR_SINGLE:
        {
          /* determine requested attribute */
          
          /* if attribute 'Dual Channel Mode' */
          if (ps_explReq->u16_attribute == k_SDCO_INST_ATTR_ID_DUAL_CH_MODE)
          {
            /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
            ** required to disable scheduler temporarily here.
            */
            __disable_irq();
            /* if dual channel configuration of DO  means dual channel, '1' */
            if (FIPARAM_DO_DUAL_CH(DUAL_CH_DO_IDX_C1(ps_explReq->u32_instance)))
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_DUAL_CH;
              /*lint +esym(960, 17.4) */
              
            }
            /* else bit not set, means single channel', '0' */
            else
            {
              /* set response payload */
              /* Deactivate lint since pointer arithmetic is not really done here */
              /*lint -esym(960, 17.4) */
              pau8_rspData[0] = k_SEMANTICS_VALUE_SINGLE_CH;
              /*lint +esym(960, 17.4) */
            }
            __enable_irq();
          
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
**    static functions
***************************************************************************************************/



/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSAImain.c
**    Summary: IXSAI - Safety Assembly Object Interface
**             This module contains the main routines of the implementation of
**             the Assembly Object interface.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSAI_AsmIoInstSetup
**             IXSAI_Init
**             IXSAI_AsmInstTypeGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#if (    (CSOS_cfg_LONG_FORMAT != CSOS_k_ENABLE)                  \
      || (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)  \
    )
  #include "CSS.h"
#endif

#include "IXSSS.h"
#include "IXSERapi.h"
#include "IXSCF.h"

#include "IXSAIapi.h"
#include "IXSAI.h"
#include "IXSAIint.h"
#include "IXSAIerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSAI_AsmIoInstSetup
**
** Description : This API function must be called by the application to set up
**               an I/O Assembly instance.
**
** Parameters  : u16_instId (IN)    - Assembly Instance ID that is to be
**                                    assigned
**                                    (checked, valid range: 1..0xFFFE
**                                    and <> CSS_cfg_ASM_NULL_INST_ID
**                                    if (CSS_cfg_IO_ASM_LOOKUP_TABLE ==
**                                    CSS_k_ENABLE) then range is limited to
**                                    1..255)
**               u16_length (IN)    - Data length that is to be assigned
**                                    (checked, valid range:
**                                    1..IXSAI_k_ASM_IO_MAX_LEN)
**                                    IXSAI_AsmInputInstSetup() and
**                                    IXSAI_AsmOutputInstSetup())
**               e_type (IN)        - type that is to be assigned (input or
**                                    output, see type definition of
**                                    {IXSAI_t_ASM_TYPE}
**                                    (checked, valid range: IXSAI_k_ASM_INPUT
**                                    or IXSAI_k_ASM_OUTPUT)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSAI_AsmIoInstSetup(CSS_t_UINT u16_instId,
                          CSS_t_UINT u16_length,
                          IXSAI_t_ASM_TYPE e_type)
{
  /* if CIP Safety Stack is in improper state */
  if (IXSSS_StateGet() != IXSSS_k_ST_RUNNING)
  {
    /* CIP Safety Stack is not in running state */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIS_STATE_ERR_IOS,
                      u16_instId, IXSER_k_A_NOT_USED);
  }
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSS_k_ENABLE)
  /* else: if passed parameter instance ID is invalid */
  else if (    (u16_instId == 0U)                         /* zero */
            || (u16_instId > CSOS_k_MAX_USINT)            /* > lookup table */
            || (u16_instId == CSS_cfg_ASM_NULL_INST_ID)   /* NULL instance */
          )
  {
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_IO_INST_SETUP,
                      u16_instId, IXSER_k_A_NOT_USED);
  }
#else
  /* else: if passed parameter instance ID is invalid */
  else if (    (u16_instId == 0U)
            || (u16_instId == CSOS_k_INVALID_INSTANCE)
            || (u16_instId == CSS_cfg_ASM_NULL_INST_ID)
          )
  {
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_IO_INST_SETUP,
                      u16_instId, IXSER_k_A_NOT_USED);
  }
#endif
  /* else: if length is invalid */
  else if ((u16_length == 0U) || (u16_length > IXSAI_k_ASM_IO_MAX_LEN))
  {
    /* error: length invalid */
    SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_IO_LEN_SETUP,
                      u16_instId, (CSS_t_UDINT)u16_length);
  }
  else /* else: ok */
  {
    /* if the passed assembly type indicates "input" */
    if (e_type == IXSAI_k_ASM_INPUT)
    {
    #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
      {
        /* set up an input assembly instance */
        (void)IXSAI_AsmInputInstSetup(u16_instId, u16_length);
      }
    #else
      {
        SAPL_CssErrorClbk(IXSAI_k_FSE_INC_IN_NOT_SUP, u16_instId,
                          IXSER_k_A_NOT_USED);
      }
    #endif
    }
    /* else: if the passed assembly type indicates "output" */
    else if (e_type == IXSAI_k_ASM_OUTPUT)
    {
    #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
      {
        /* set up an output assembly instance */
        (void)IXSAI_AsmOutputInstSetup(u16_instId, u16_length);
      }
    #else
      {
        SAPL_CssErrorClbk(IXSAI_k_FSE_INC_OUT_NOT_SUP, u16_instId,
                          IXSER_k_A_NOT_USED);
      }
    #endif
    }
    else /* else: passed assembly type is something else */
    {
      /* error: parameter e_type invalid */
      SAPL_CssErrorClbk(IXSAI_k_FSE_AIP_ASM_TYPE_IOS, u16_instId,
                        IXSER_k_A_NOT_USED);
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}


#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
/*******************************************************************************
**
** Function    : IXSAI_Init
**
** Description : This function initializes the IXSAI unit.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSAI_Init(void)
{
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  {
    IXSAI_InputInit();
  }
#endif
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  {
    IXSAI_OutputInit();
  }
#endif

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif


#if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
      || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
    )
/*******************************************************************************
**
** Function    : IXSAI_AsmInstTypeGet
**
** Description : This function can be used to determine the type of the
**               specified assembly instance.
**
** See Also    : -
**
** Parameters  : u16_asmInstId (IN)  - Assembly Instance ID
**                                     (not checked, any value allowed)
**
** Returnvalue : IXSAI_k_ASM_INPUT   - Input Assembly Object
**               IXSAI_k_ASM_OUTPUT  - Output Assembly Object
**               IXSAI_k_ASM_INVALID - invalid instance ID
**
*******************************************************************************/
IXSAI_t_ASM_TYPE IXSAI_AsmInstTypeGet(CSS_t_UINT u16_asmInstId)
{
  /* return value of this function */
  IXSAI_t_ASM_TYPE e_type = IXSAI_k_ASM_INVALID;

  /* if the passed instance ID is equal to the NULL instance ID */
  if (u16_asmInstId == CSS_cfg_ASM_NULL_INST_ID)
  {
    e_type = IXSAI_k_ASM_INVALID;
  }
  /* else: instance ID is ok */
#if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
  /* if the passed instance ID is found in the Input assembly instances */
  else if (IXSAI_InputInstCheckExists(u16_asmInstId) != CSS_k_FALSE)
  {
    e_type = IXSAI_k_ASM_INPUT;
  }
#endif
#if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
  /* if the passed instance ID is found in the Output assembly instances */
  else if (IXSAI_OutputInstCheckExists(u16_asmInstId) != CSS_k_FALSE)
  {
    e_type = IXSAI_k_ASM_OUTPUT;
  }
#endif
  else
  {
    /* return value already initialized */
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (e_type);
}
#endif


/*******************************************************************************
**    static functions
*******************************************************************************/


#else
  /* just to avoid lint warning "CSOScfg.h not used" */
  #if (CSOS_k_NOT_APPLICABLE == CSOS_k_NOT_APPLICABLE)
  #endif
#endif /* (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
            || (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE) \
          ) */

/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSScfgCheck.h
**    Summary: CSS configuration file checking
**             CSScfgCheck.h is responsible for checking if the configuration
**             defines in CSScfg.h were set to valid values (see Req.10.5.1.4-2
**             and Req.10.5.1.4-3).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: -
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef CSS_CFG_CHECK_H
#define CSS_CFG_CHECK_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSS_k_CONFIGURATION_STRING
*/
#ifndef CSS_k_CONFIGURATION_STRING
  #error configuration string missing: CSS_k_CONFIGURATION_STRING
#endif

/** CSS_k_ENABLE:
*/
#ifndef CSS_k_ENABLE
  #error configuration define missing: CSS_k_ENABLE
#endif


/** CSS_k_DISABLE:
*/
#ifndef CSS_k_DISABLE
  #error configuration define missing: CSS_k_DISABLE
#endif
#if (CSS_k_DISABLE == CSS_k_ENABLE)
  #error CSS_k_ENABLE and CSS_k_DISABLE must be defined differently
#endif


/** CSS_k_NOT_APPLICABLE:
*/
#ifndef CSS_k_NOT_APPLICABLE
  #error configuration define missing: CSS_k_NOT_APPLICABLE
#endif
#if (CSS_k_NOT_APPLICABLE != 0xFFFFFFFFU)
  #error CSS_k_NOT_APPLICABLE has an invalid value
#endif


/** CSS_H2N_CPY_MTD_ODD_EVEN_SORT:
*/
#ifndef CSS_H2N_CPY_MTD_ODD_EVEN_SORT
  #error configuration define missing: CSS_H2N_CPY_MTD_ODD_EVEN_SORT
#endif

/**CSS_H2N_CPY_MTD_INTERLACE_EVEN:
*/
#ifndef CSS_H2N_CPY_MTD_INTERLACE_EVEN
  #error configuration define missing: CSS_H2N_CPY_MTD_INTERLACE_EVEN
#endif

/**CSS_H2N_CPY_MTD_INTERLACE_ODD:
*/
#ifndef CSS_H2N_CPY_MTD_INTERLACE_ODD
  #error configuration define missing: CSS_H2N_CPY_MTD_INTERLACE_ODD
#endif


/** CSS_cfg_ERROR_STRING:
*/
#ifndef CSS_cfg_ERROR_STRING
  #error configuration define missing: CSS_cfg_ERROR_STRING
#endif
#if (    (CSS_cfg_ERROR_STRING != CSS_k_DISABLE)   \
      && (CSS_cfg_ERROR_STRING != CSS_k_ENABLE)    \
    )
  #error CSS_cfg_ERROR_STRING configured invalid
#endif


/** CSS_cfg_BASE_FORMAT:
*/
#ifndef CSS_cfg_BASE_FORMAT
  #error configuration define missing: CSS_cfg_BASE_FORMAT
#endif
#if (    (CSS_cfg_BASE_FORMAT != CSS_k_DISABLE)   \
      && (CSS_cfg_BASE_FORMAT != CSS_k_ENABLE)    \
    )
  /* CSS_k_DISABLE is not allowed according to Volume 5 edition 2.5 but       */
  /* needed for HMS internal tests. If Base Format is disabled then the       */
  /* Conformance Test will fail!                                              */
  #error CSS_cfg_BASE_FORMAT configured invalid
#endif


/** CSS_cfg_EXTENDED_FORMAT:
*/
#ifndef CSS_cfg_EXTENDED_FORMAT
  #error configuration define missing: CSS_cfg_EXTENDED_FORMAT
#endif
#if (    (CSS_cfg_EXTENDED_FORMAT != CSS_k_DISABLE)   \
      && (CSS_cfg_EXTENDED_FORMAT != CSS_k_ENABLE)    \
    )
  /* CSS_k_DISABLE is not allowed according to Volume 5 edition 2.5 but       */
  /* needed for HMS internal tests. If Extended Format is disabled then the   */
  /* Conformance Test will fail!                                              */
  #error CSS_cfg_EXTENDED_FORMAT configured invalid
#endif

#if (    (CSS_cfg_BASE_FORMAT == CSS_k_DISABLE)     \
      && (CSS_cfg_EXTENDED_FORMAT == CSS_k_DISABLE) \
    )
  #error At least one of Base or Extended Format must be supported
#endif


/** CSS_cfg_IO_ASM_LOOKUP_TABLE:
*/
#ifndef CSS_cfg_IO_ASM_LOOKUP_TABLE
  #error configuration define missing: CSS_cfg_IO_ASM_LOOKUP_TABLE
#endif
#if (    (CSS_cfg_IO_ASM_LOOKUP_TABLE != CSS_k_DISABLE)   \
      && (CSS_cfg_IO_ASM_LOOKUP_TABLE != CSS_k_ENABLE)    \
    )
  #error CSS_cfg_IO_ASM_LOOKUP_TABLE configured invalid
#endif


/** CSS_cfg_NUM_OF_ASM_IN_INSTS:
*/
#ifndef CSS_cfg_NUM_OF_ASM_IN_INSTS
  #error configuration define missing: CSS_cfg_NUM_OF_ASM_IN_INSTS
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)
  #if (    (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_DISABLE)  \
        || (CSOS_cfg_TARGET == CSOS_k_ENABLE)                   \
      )
    #if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS == 0U)     \
          || (CSS_cfg_NUM_OF_ASM_IN_INSTS > 65534U)  \
        )
      #error CSS_cfg_NUM_OF_ASM_IN_INSTS configured invalid
    #endif
  #else
    #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
      #error CSS_cfg_NUM_OF_ASM_IN_INSTS configured invalid
    #endif
  #endif
#else
  #if (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)
    #error Input Assembly Instances cannot be supported without SV Clients
  #endif
#endif


/** CSS_cfg_NUM_OF_ASM_OUT_INSTS:
*/
#ifndef CSS_cfg_NUM_OF_ASM_OUT_INSTS
  #error configuration define missing: CSS_cfg_NUM_OF_ASM_OUT_INSTS
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 0U)
  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_DISABLE)
    #if  (    (CSS_cfg_NUM_OF_ASM_OUT_INSTS < CSOS_cfg_NUM_OF_SV_SERVERS)    \
           || (CSS_cfg_NUM_OF_ASM_OUT_INSTS > 65534U) \
         )
      #error CSS_cfg_NUM_OF_ASM_OUT_INSTS configured invalid
    #endif
  #elif (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    #if  (    (CSS_cfg_NUM_OF_ASM_OUT_INSTS == 0U)    \
           || (CSS_cfg_NUM_OF_ASM_OUT_INSTS > 65534U) \
         )
      #error CSS_cfg_NUM_OF_ASM_OUT_INSTS configured invalid
    #endif
  #else
    #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
      #error CSS_cfg_NUM_OF_ASM_OUT_INSTS configured invalid
    #endif
  #endif
#else
  #if (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
    #error Output Assembly Instances cannot be supported without SV Servers
  #endif
#endif


/** CSS_cfg_NUM_OF_T_ASM_OUT_INSTS:
*/
#ifndef CSS_cfg_NUM_OF_T_ASM_OUT_INSTS
  #error configuration define missing: CSS_cfg_NUM_OF_T_ASM_OUT_INSTS
#endif
#if (    (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)               \
      && (CSOS_cfg_TARGET == CSOS_k_ENABLE)                   \
      && (CSS_cfg_ORIGINATOR_PROCESS_IMAGE == CSS_k_DISABLE)  \
    )
  #if (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS > CSS_cfg_NUM_OF_ASM_OUT_INSTS)
    #error CSS_cfg_NUM_OF_T_ASM_OUT_INSTS configured invalid
  #endif
#else
  #if (CSS_cfg_NUM_OF_T_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)
    #error preconditions for supporting CSS_cfg_NUM_OF_T_ASM_OUT_INSTS not met
  #endif
#endif


/* Consistency check for Assembly settings */
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  #if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS == CSS_k_NOT_APPLICABLE)  \
        && (CSS_cfg_NUM_OF_ASM_OUT_INSTS == CSS_k_NOT_APPLICABLE) \
      )
    #error When CSOS_cfg_TARGET is enabled then In/Out-Assemblies are required
  #endif
#endif
#if (CSS_cfg_IO_ASM_LOOKUP_TABLE == CSOS_k_ENABLE)
  #if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS == CSS_k_NOT_APPLICABLE)  \
        && (CSS_cfg_NUM_OF_ASM_OUT_INSTS == CSS_k_NOT_APPLICABLE) \
      )
    #error CSS_cfg_IO_ASM_LOOKUP_TABLE is enabled without Assemblies
  #endif
  #if (    (CSS_cfg_NUM_OF_ASM_IN_INSTS != CSS_k_NOT_APPLICABLE)  \
        && (CSS_cfg_NUM_OF_ASM_IN_INSTS > 255U)                   \
      )
    #error Assembly lookup table is limited to 255 Assembly instances
  #endif
  #if (    (CSS_cfg_NUM_OF_ASM_OUT_INSTS != CSS_k_NOT_APPLICABLE)  \
        && (CSS_cfg_NUM_OF_ASM_OUT_INSTS > 255U)                   \
      )
    #error Assembly lookup table is limited to 255 Assembly instances
  #endif
#endif


/** CSS_cfg_ASM_NULL_INST_ID:
*/
#ifndef CSS_cfg_ASM_NULL_INST_ID
  #error configuration define missing: CSS_cfg_ASM_NULL_INST_ID
#endif
#if (     (CSS_cfg_ASM_NULL_INST_ID == 0U)       \
      ||  (CSS_cfg_ASM_NULL_INST_ID == 0xFFFFU)  \
    )
  #error CSS_cfg_ASM_NULL_INST_ID configured invalid
#endif


/** CSS_cfg_SET_TUNID:
*/
#ifndef CSS_cfg_SET_TUNID
  #error CSS_cfg_SET_TUNID ist not defined
#endif
#if (    (CSS_cfg_SET_TUNID != CSS_k_DISABLE)   \
      && (CSS_cfg_SET_TUNID != CSS_k_ENABLE)    \
    )
  #error CSS_cfg_SET_TUNID configured invalid
#endif


/** CSS_cfg_NUM_OF_CCO_INSTS:
*/
#ifndef CSS_cfg_NUM_OF_CCO_INSTS
  #error CSS_cfg_NUM_OF_CCO_INSTS is not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)
  #if (CSS_cfg_NUM_OF_CCO_INSTS != CSS_k_NOT_APPLICABLE)
    #error CSS_cfg_NUM_OF_CCO_INSTS configured invalid
  #endif
#else
  #if (    (CSS_cfg_NUM_OF_CCO_INSTS < 1U)   \
        || (CSS_cfg_NUM_OF_CCO_INSTS > 65533U) \
      )
    #error CSS_cfg_NUM_OF_CCO_INSTS configured invalid
  #endif
  /* Consistency check */
  #if (CSS_cfg_NUM_OF_CCO_INSTS >                                  \
        (CSOS_cfg_NUM_OF_SV_SERVERS + CSOS_cfg_NUM_OF_SV_CLIENTS))
    #error More CCO instances than Safety Validator instances
  #endif
  #if (CSOS_cfg_TARGET == CSOS_k_DISABLE)
    #if (    (CSS_cfg_NUM_OF_CCO_INSTS < CSOS_cfg_NUM_OF_SV_SERVERS) \
          || (CSS_cfg_NUM_OF_CCO_INSTS < CSOS_cfg_NUM_OF_SV_CLIENTS) \
        )
      #error More Safety Validator instances than CCO instances
    #endif
  #endif
#endif


/** CSS_cfg_ORIGINATOR_PROCESS_IMAGE:
*/
#ifndef CSS_cfg_ORIGINATOR_PROCESS_IMAGE
  #error CSS_cfg_ORIGINATOR_PROCESS_IMAGE ist not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  #if (    (CSS_cfg_ORIGINATOR_PROCESS_IMAGE != CSS_k_DISABLE)   \
        && (CSS_cfg_ORIGINATOR_PROCESS_IMAGE != CSS_k_ENABLE)    \
      )
    #error CSS_cfg_ORIGINATOR_PROCESS_IMAGE configured invalid
  #endif
#else
  #if (CSS_cfg_ORIGINATOR_PROCESS_IMAGE != CSS_k_DISABLE)
    #error Process Image is only applicable for Originators
  #endif
#endif


/** CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W:
*/
#ifndef CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W
  #error CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W is not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)
  #if (CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W != CSS_k_NOT_APPLICABLE)
    #error CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W configured invalid
  #endif
#else
  #if (    (CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W < 1U)   \
        || (CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W > 126U) \
      )
    #error CSS_cfg_CCO_CNXN_NAME_MAX_LEN_W configured invalid
  #endif
#endif


/** CSS_cfg_H2N_COPY_METHOD:
*/
#ifndef CSS_cfg_H2N_COPY_METHOD
  #error CSS_cfg_H2N_COPY_METHOD is not defined
#endif
#if (    (CSS_cfg_H2N_COPY_METHOD != CSS_H2N_CPY_MTD_ODD_EVEN_SORT)   \
      && (CSS_cfg_H2N_COPY_METHOD != CSS_H2N_CPY_MTD_INTERLACE_EVEN)  \
      && (CSS_cfg_H2N_COPY_METHOD != CSS_H2N_CPY_MTD_INTERLACE_ODD)   \
      && (CSS_cfg_H2N_COPY_METHOD != CSS_H2N_CPY_MTD_HOMOGENEOUS_MIX) \
    )
  #error CSS_cfg_H2N_COPY_METHOD configured invalid
#endif


/** CSS_cfg_SOFT_ERR_METHOD:
*/
#ifndef CSS_cfg_SOFT_ERR_METHOD
  #error CSS_cfg_SOFT_ERR_METHOD is not defined
#endif
#if (    (CSS_cfg_SOFT_ERR_METHOD != CSS_SOFT_ERR_MTD_OFF)       \
      && (CSS_cfg_SOFT_ERR_METHOD != CSS_SOFT_ERR_MTD_BYTES)     \
      && (CSS_cfg_SOFT_ERR_METHOD != CSS_SOFT_ERR_MTD_VARIABLES) \
    )
  #error CSS_cfg_SOFT_ERR_METHOD configured invalid
#endif


/** CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE:
*/
#ifndef CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE
  #error CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE is not defined
#endif
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_OFF)
  #if (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE != CSS_k_NOT_APPLICABLE)
    #error CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE configured invalid
  #endif
#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  #if (    (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE == 0U)              \
        || (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE > 65535U)           \
      )
    #error CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE configured invalid
  #endif
#elif (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  #if (    (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE < 4U)               \
        || (CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE > 65535U)           \
      )
    #error CSS_cfg_SOFT_ERR_MEM_CHUNK_SIZE configured invalid
  #endif
#endif


/** CSS_cfg_CCO_CLASS_ID:
*/
/* see Volume 1, chapter 4-9.1 "CIP Object Address Ranges" */
#ifndef CSS_cfg_CCO_CLASS_ID
  #error CSS_cfg_CCO_CLASS_ID is not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)
  #if (CSS_cfg_CCO_CLASS_ID != CSS_k_NOT_APPLICABLE)
    #error CSS_cfg_CCO_CLASS_ID configured invalid
  #endif
#else
  #if (    (     (CSS_cfg_CCO_CLASS_ID >= 100U)  /* range 0x64.. */            \
              && (CSS_cfg_CCO_CLASS_ID <= 199U)  /* ..0xC7: vendor specific */ \
           )                                                                   \
        ||       (CSS_cfg_CCO_CLASS_ID == 243U)  /* 0xF3: CCO class ID */      \
        || (     (CSS_cfg_CCO_CLASS_ID >= 768U)  /* range 0x300.. */           \
              && (CSS_cfg_CCO_CLASS_ID <= 1279U) /* ..0x4FF: vendor specific */\
           )                                                                   \
      )
    /* The configured value for the CCO Class ID is ok */
  #else
    #error CSS_cfg_CCO_CLASS_ID configured invalid
  #endif
#endif


#ifndef CSS_cfg_SAFETY_RESET
  #error CSS_cfg_SAFETY_RESET is not defined
#endif
#if (    (CSS_cfg_SAFETY_RESET != CSS_k_DISABLE)   \
      && (CSS_cfg_SAFETY_RESET != CSS_k_ENABLE)    \
    )
  #error CSS_cfg_SAFETY_RESET configured invalid
#endif

/** CSS_cfg_ALARM_WARNING_ENABLE:
*/
#ifndef CSS_cfg_ALARM_WARNING_ENABLE
  #error CSS_cfg_ALARM_WARNING_ENABLE is not defined
#endif
#if (    (CSS_cfg_ALARM_WARNING_ENABLE != CSS_k_DISABLE)   \
      && (CSS_cfg_ALARM_WARNING_ENABLE != CSS_k_ENABLE)    \
    )
  #error CSS_cfg_ALARM_WARNING_ENABLE configured invalid
#endif


/** CSS_k_PLATFORM_FACTOR:
*/
#ifndef CSS_k_PLATFORM_FACTOR
  #error platform define missing: CSS_k_PLATFORM_FACTOR
#endif
#if ((CSS_k_PLATFORM_FACTOR != 1) && \
     (CSS_k_PLATFORM_FACTOR != 2))
  #error CSS_k_PLATFORM_FACTOR shall be set to either 1 or 2
#endif


/*******************************************************************************
**    data types
*******************************************************************************/

/*
** Checking the size of the elementary data types: in case the user has
** misconfigured the types the compiler will generate an error
** (e.g. "negative subscript" or "size of array is negative",
** "Illegal index value", etc.)
*/
#ifndef CSS_TYPE_ASSERT
  /*lint -save -e961                   mute lint warning '#/##' operator used */
  /*lint --e{506} error 506: (Warning -- Constant value Boolean [MISRA 2004
                                         Rules 13.7 and 14.1]) */
  #define CSS_TYPE_ASSERT(exp, name) typedef int dummy_##name [(exp) ? 1 : -1];
  /*lint -restore */
#endif

#if (CSS_k_PLATFORM_FACTOR == 1)
  CSS_TYPE_ASSERT(sizeof(CSS_t_BOOL)  == CSOS_k_SIZEOF_BOOL,  css_o___1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_SINT)  == CSOS_k_SIZEOF_SINT,  css_i8__1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_INT)   == CSOS_k_SIZEOF_INT,   css_i16_2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_DINT)  == CSOS_k_SIZEOF_DINT,  css_i32_4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_USINT) == CSOS_k_SIZEOF_USINT, css_u8__1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_UINT)  == CSOS_k_SIZEOF_UINT,  css_u16_2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_UDINT) == CSOS_k_SIZEOF_UDINT, css_u32_4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_BYTE)  == CSOS_k_SIZEOF_BYTE,  css_b___1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_WORD)  == CSOS_k_SIZEOF_WORD,  css_w___2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_DWORD) == CSOS_k_SIZEOF_DWORD, css_dw__4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_CHAR)  == 1U,                  css_c___1_byte)
#elif (CSS_k_PLATFORM_FACTOR == 2)
  CSS_TYPE_ASSERT(sizeof(CSS_t_BOOL)  == 1U, css_o___1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_SINT)  == 1U, css_i8__1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_INT)   == 1U, css_i16_2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_DINT)  == 2U, css_i32_4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_USINT) == 1U, css_u8__1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_UINT)  == 1U, css_u16_2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_UDINT) == 2U, css_u32_4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_BYTE)  == 1U, css_b___1_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_WORD)  == 1U, css_w___2_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_DWORD) == 2U, css_dw__4_byte)
  CSS_TYPE_ASSERT(sizeof(CSS_t_CHAR)  == 1U, css_c___1_byte)
#endif

/* Mute lint warning type not referenced. These types are just defined for    */
/* being able to check the size of the elementary types.                      */
/*lint -esym(756, dummy_css_o___1_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_i8__1_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_i16_2_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_i32_4_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_u8__1_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_u16_2_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_u32_4_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_b___1_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_w___2_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_dw__4_byte) not referenced - just for type checks */
/*lint -esym(756, dummy_css_c___1_byte) not referenced - just for type checks */


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/


#endif /* CSS_CFG_CHECK_H */

/*** End of File ***/


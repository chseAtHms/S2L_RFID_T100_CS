/*******************************************************************************
**    Copyright (C) 2009-2020 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSOScfgCheck.h
**    Summary: CSOS configuration file checking
**             CSOScfgCheck.h contains is responsible for checking if the
**             configuration defines in CSOScfg.h were set to valid values
**             (see Req.10.5.1.4-2 and Req.10.5.1.4-3).
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

#ifndef CSOS_CFG_CHECK_H
#define CSOS_CFG_CHECK_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSOS_k_ENABLE:
*/
#ifndef CSOS_k_ENABLE
  #error configuration define missing: CSOS_k_ENABLE
#endif


/** CSOS_k_DISABLE:
*/
#ifndef CSOS_k_DISABLE
  #error configuration define missing: CSOS_k_DISABLE
#endif
#if (CSOS_k_DISABLE == CSOS_k_ENABLE)
  #error CSOS_k_ENABLE and CSOS_k_DISABLE must be defined differently
#endif


/** CSOS_k_NOT_APPLICABLE:
*/
#ifndef CSOS_k_NOT_APPLICABLE
  #error configuration define missing: CSOS_k_NOT_APPLICABLE
#endif
#if (CSOS_k_NOT_APPLICABLE != 0xFFFFFFFFU)
  #error CSOS_k_NOT_APPLICABLE has an invalid value
#endif


/** CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE:
*/
#ifndef CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
  #error configuration define missing: CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE
#endif
#if (    (CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE < 31U)     \
      || (CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE > 65500U)  \
    )
  #error CSOS_cfg_EXPL_RSP_DATA_BUF_SIZE configured invalid
#else
  /* Further static checks will be performed in the SSO, SVO and CCO at the
     places where the maximum explicit response size of these objects is
     known. */
#endif


/** CSOS_cfg_ADD_STAT_SIZE_MAX:
*/
#ifndef CSOS_cfg_ADD_STAT_SIZE_MAX
  #error configuration define missing: CSOS_cfg_ADD_STAT_SIZE_MAX
#endif
#if (    (CSOS_cfg_ADD_STAT_SIZE_MAX < 1U)         \
      || (CSOS_cfg_ADD_STAT_SIZE_MAX > 255U)       \
    )
  #error CSOS_cfg_ADD_STAT_SIZE_MAX configured invalid
#endif


/** CSOS_cfg_ORIGINATOR:
*/
#ifndef CSOS_cfg_ORIGINATOR
  #error configuration define missing: CSOS_cfg_ORIGINATOR
#endif
#if (    (CSOS_cfg_ORIGINATOR != CSOS_k_DISABLE)   \
      && (CSOS_cfg_ORIGINATOR != CSOS_k_ENABLE)    \
    )
  #error CSOS_cfg_ORIGINATOR configured invalid
#endif


/** CSOS_cfg_TARGET:
*/
#ifndef CSOS_cfg_TARGET
  #error configuration define missing: CSOS_cfg_TARGET
#endif
#if (    (CSOS_cfg_TARGET != CSOS_k_DISABLE)   \
      && (CSOS_cfg_TARGET != CSOS_k_ENABLE)    \
    )
  #error CSOS_cfg_TARGET configured invalid
#endif

/* At least one must be supported: Originator or Target (both is also ok) */
#if (    (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)  \
      && (CSOS_cfg_TARGET == CSOS_k_DISABLE)      \
    )
  #error One of CSOS_cfg_ORIGINATOR or CSOS_cfg_TARGET must be supported
#endif


/** CSOS_cfg_SHORT_FORMAT:
*/
#ifndef CSOS_cfg_SHORT_FORMAT
  #error configuration define missing: CSOS_cfg_SHORT_FORMAT
#endif
#if (    (CSOS_cfg_SHORT_FORMAT != CSOS_k_DISABLE)   \
      && (CSOS_cfg_SHORT_FORMAT != CSOS_k_ENABLE)    \
    )
  #error CSOS_cfg_SHORT_FORMAT configured invalid
#endif


/** CSOS_cfg_LONG_FORMAT:
*/
#ifndef CSOS_cfg_LONG_FORMAT
  #error configuration define missing: CSOS_cfg_LONG_FORMAT
#endif
#if (    (CSOS_cfg_LONG_FORMAT != CSOS_k_DISABLE)   \
      && (CSOS_cfg_LONG_FORMAT != CSOS_k_ENABLE)    \
    )
  #error CSOS_cfg_LONG_FORMAT configured invalid
#endif

#if (    (CSOS_cfg_SHORT_FORMAT == CSOS_k_DISABLE)  \
      && (CSOS_cfg_LONG_FORMAT == CSOS_k_DISABLE)   \
    )
  #error At least one of Short or Long Format must be supported
#endif


/** CSOS_cfg_LONG_FORMAT_MAX_LENGTH:
*/
#ifndef CSOS_cfg_LONG_FORMAT_MAX_LENGTH
  #error configuration define missing: CSOS_cfg_LONG_FORMAT_MAX_LENGTH
#endif
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  #if (    (CSOS_cfg_LONG_FORMAT_MAX_LENGTH < 3U)   \
        || (CSOS_cfg_LONG_FORMAT_MAX_LENGTH > 250U) \
      )
    #error CSOS_cfg_LONG_FORMAT_MAX_LENGTH configured invalid
  #endif
#else
  #if (CSOS_cfg_LONG_FORMAT_MAX_LENGTH != CSOS_k_NOT_APPLICABLE)
    #error CSOS_cfg_LONG_FORMAT_MAX_LENGTH configured invalid
  #endif
#endif


/** CSOS_cfg_NUM_OF_SV_CLIENTS:
*/
#ifndef CSOS_cfg_NUM_OF_SV_CLIENTS
  #error configuration define missing: CSOS_cfg_NUM_OF_SV_CLIENTS
#endif
#if (CSOS_cfg_NUM_OF_SV_CLIENTS > 65534U)
  #error CSOS_cfg_NUM_OF_SV_CLIENTS configured invalid
#endif


/** CSOS_cfg_NUM_OF_SV_SERVERS:
*/
#ifndef CSOS_cfg_NUM_OF_SV_SERVERS
  #error configuration define missing: CSOS_cfg_NUM_OF_SV_SERVERS
#endif
#if (CSOS_cfg_NUM_OF_SV_SERVERS > 65534U)
  #error CSOS_cfg_NUM_OF_SV_SERVERS configured invalid
#endif

#if (    (CSOS_cfg_NUM_OF_SV_CLIENTS == 0U)  \
      && (CSOS_cfg_NUM_OF_SV_SERVERS == 0U)  \
    )
  #error At least one of Safety Validator Client or Server must be supported
#endif

#if ((CSOS_cfg_NUM_OF_SV_CLIENTS + CSOS_cfg_NUM_OF_SV_SERVERS) > 65534U)
  #error The sum of Safety Validator Clients and Servers shall not exceed 65534
#endif


/** CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W:
*/
#ifndef CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W
  #error CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W is not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)
  #if (CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W != CSOS_k_NOT_APPLICABLE)
    #error CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W configured invalid
  #endif
#else
  #if (    (CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W < 1U)   \
        || (CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W > 125U) \
      )
    #error CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W configured invalid
  #endif
#endif


/** CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W:
*/
#ifndef CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W
  #error CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W is not defined
#endif
#if (CSOS_cfg_ORIGINATOR == CSOS_k_DISABLE)
  #if (CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W != CSOS_k_NOT_APPLICABLE)
    #error CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W configured invalid
  #endif
#else
  #if (    (CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W < 1U)   \
        || (CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W > 125U) \
      )
    #error CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W configured invalid
  #endif
#endif


/** CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W + CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W:
*/
#if (CSOS_cfg_ORIGINATOR != CSOS_k_DISABLE)
  #if ((   CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W              \
         + CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W) > 126U)
    #error The sum of (CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W+ \
           CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W) shall not exceed 126
  #endif
#endif


/** CSOS_cfg_NUM_OF_SAFETY_PORTS:
*/
#ifndef CSOS_cfg_NUM_OF_SAFETY_PORTS
  #error CSOS_cfg_NUM_OF_SAFETY_PORTS is not defined
#endif
#if (    (CSOS_cfg_NUM_OF_SAFETY_PORTS == 0U)    \
      || (CSOS_cfg_NUM_OF_SAFETY_PORTS > 254U)     \
    )
  #error CSOS_cfg_NUM_OF_SAFETY_PORTS configured invalid
#endif


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/


#endif /* CSOS_CFG_CHECK_H */

/*** End of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSOS.h
**    Summary: CSOS
**             General constant definitions for usage of the CSOS Stack.
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


#ifndef CSOS_H
#define CSOS_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/
/*
** data lengths of the parts of Forward_Open/Forward_Close messages
*/
#define CSOS_k_MSG_ROUTER_REQ_LEN  (  CSOS_k_SIZEOF_USINT /* service */        \
                                    + CSOS_k_SIZEOF_USINT /* path size */      \
                                    + CSOS_k_SIZEOF_USINT /* log seg: class */ \
                                    + CSOS_k_SIZEOF_USINT /* Cnxn Manager */   \
                                    + CSOS_k_SIZEOF_USINT /* log seg: Inst */  \
                                    + CSOS_k_SIZEOF_USINT)/* Inst 1 */

#define CSOS_k_TIMEOUT_INFO_LEN    (  CSOS_k_SIZEOF_BYTE  /* prio/time_tick */ \
                                    + CSOS_k_SIZEOF_USINT)/* Time-out_ticks */

#define CSOS_k_NET_CNXN_ID_LEN     (  CSOS_k_SIZEOF_UDINT /* O->T */           \
                                    + CSOS_k_SIZEOF_UDINT)/* T->O */

#define CSOS_k_CNXN_TRIAD_LEN      (  CSOS_k_SIZEOF_UINT  /* Cnxn SerNum */    \
                                    + CSOS_k_SIZEOF_UINT  /* Orig Vendor ID */ \
                                    + CSOS_k_SIZEOF_UDINT)/* Orig SerNum */

#define CSOS_k_NET_CNXN_PARAMS_LEN (  CSOS_k_SIZEOF_USINT /* Cnxn ToutMult */  \
                                    + CSOS_k_SIZEOF_BYTE  /* reserved */       \
                                    + CSOS_k_SIZEOF_BYTE  /* reserved */       \
                                    + CSOS_k_SIZEOF_BYTE  /* reserved */       \
                                    + CSOS_k_SIZEOF_UDINT /* O->T RPI */       \
                                    + CSOS_k_SIZEOF_WORD  /* O->T NetCnxnPar */\
                                    + CSOS_k_SIZEOF_UDINT /* T->O RPI */       \
                                    + CSOS_k_SIZEOF_WORD  /* T->O NetCnxnPar */\
                                    + CSOS_k_SIZEOF_BYTE) /* XportType/Trig */

#define CSOS_k_ELECTRONIC_KEY_LEN  (  CSOS_k_SIZEOF_USINT /* Log seg: El Key */\
                                    + CSOS_k_SIZEOF_USINT /* Key Format */     \
                                    + CSOS_k_SIZEOF_UINT  /* Vendor ID */      \
                                    + CSOS_k_SIZEOF_UINT  /* Device Type */    \
                                    + CSOS_k_SIZEOF_UINT  /* Prod Code */      \
                                    + CSOS_k_SIZEOF_BYTE  /* Major Revision */ \
                                    + CSOS_k_SIZEOF_USINT)/* Minor Revision */

/*
** data lengths of the Safety Segments (Base/Extended)
*/
#define CSOS_k_INIT_VAL_LEN        (  CSOS_k_SIZEOF_UINT  /* Init Timestamp */ \
                                    + CSOS_k_SIZEOF_UINT) /* Init Rollover */

#define CSOS_k_SEG_SIZE_EXT        (  CSOS_k_SIZEOF_USINT /* SftyNetSeg Form */\
                                    + CSOS_k_SIZEOF_BYTE  /* reserved */       \
                                    + CSOS_k_SIZEOF_UDINT /* SCCRC */          \
                                    + CSOS_k_SIZEOF_UINT  /* SCTS: date */     \
                                    + CSOS_k_SIZEOF_UDINT /* SCTS: time */     \
                                    + CSOS_k_SIZEOF_UDINT /* Tcorr EPI */      \
                                    + CSOS_k_SIZEOF_WORD  /* Tcorr NCP */      \
                                    + CSOS_k_SIZEOF_UNID  /* TUNID */          \
                                    + CSOS_k_SIZEOF_UNID  /* OUNID */          \
                                    + CSOS_k_SIZEOF_UINT  /* PingIntEpiMult */ \
                                    + CSOS_k_SIZEOF_UINT  /* TCMMM */          \
                                    + CSOS_k_SIZEOF_UINT  /* NTEM */           \
                                    + CSOS_k_SIZEOF_USINT /* Timeout Mult */   \
                                    + CSOS_k_SIZEOF_USINT /* Max_Cons_Num */   \
                                    + CSOS_k_SIZEOF_UINT  /* Max_Fault_Num */  \
                                    + CSOS_k_SIZEOF_UDINT /* CPCRC */          \
                                    + CSOS_k_SIZEOF_UDINT /* Tcorr Cnxn ID */  \
                                    + CSOS_k_INIT_VAL_LEN)/* InitTstamp/Rollo */

#define CSOS_k_SAFETY_SEG_EXT_LEN  (  CSOS_k_SIZEOF_BYTE  /* Segment Type */   \
                                    + CSOS_k_SIZEOF_BYTE  /* Segment Length */ \
                                    + CSOS_k_SEG_SIZE_EXT)/* SafetyNetSeg Ext */

/*lint -esym(755, CSOS_k_SEG_SIZE_BASE)          not referenced in every cfg */
 #define CSOS_k_SEG_SIZE_BASE      (  CSOS_k_SIZEOF_USINT /* SftyNetSeg Form */\
                                    + CSOS_k_SIZEOF_BYTE  /* reserved */       \
                                    + CSOS_k_SIZEOF_UDINT /* SCCRC */          \
                                    + CSOS_k_SIZEOF_UINT  /* SCTS: date */     \
                                    + CSOS_k_SIZEOF_UDINT /* SCTS: time */     \
                                    + CSOS_k_SIZEOF_UDINT /* Tcorr EPI */      \
                                    + CSOS_k_SIZEOF_WORD  /* Tcorr NCP */      \
                                    + CSOS_k_SIZEOF_UNID  /* TUNID */          \
                                    + CSOS_k_SIZEOF_UNID  /* OUNID */          \
                                    + CSOS_k_SIZEOF_UINT  /* PingIntEpiMult */ \
                                    + CSOS_k_SIZEOF_UINT  /* TCMMM */          \
                                    + CSOS_k_SIZEOF_UINT  /* NTEM */           \
                                    + CSOS_k_SIZEOF_USINT /* Timeout Mult */   \
                                    + CSOS_k_SIZEOF_USINT /* Max_Cons_Num */   \
                                    + CSOS_k_SIZEOF_UDINT /* CPCRC */          \
                                    + CSOS_k_SIZEOF_UDINT)/* Tcorr Cnxn ID */

/*lint -esym(755, CSOS_k_SAFETY_SEG_BASE_LEN)    not referenced in every cfg */
#define CSOS_k_SAFETY_SEG_BASE_LEN (  CSOS_k_SIZEOF_BYTE  /* Segment Type */   \
                                    + CSOS_k_SIZEOF_BYTE  /* Segment Length */ \
                                    + CSOS_k_SEG_SIZE_BASE)/* SafetyNetSeg */


/** CSOS_k_MAX_HALCS_FWDOPEN_LEN:
    Maximum length of a Forward_Open message transmitted via HALCS to CSAL. This
    is a Type 1 Forward Open Request and includes HALCS Header Information. But
    this doesn't include the configuration data (device parameters) as these are
    inserted by CSAL. Only the offset where CSAL has to insert this data is
    contained.
*/
#define CSOS_k_MAX_HALCS_FWDOPEN_LEN                                           \
  (                                                                            \
      CSOS_k_SIZEOF_UDINT              /* application reference */             \
    + CSOS_k_SIZEOF_UINT               /* configuration data offset */         \
    + CSOS_k_MSG_ROUTER_REQ_LEN        /* Message Router Request Format */     \
    + CSOS_k_TIMEOUT_INFO_LEN                                                  \
    + CSOS_k_NET_CNXN_ID_LEN                                                   \
    + CSOS_k_CNXN_TRIAD_LEN                                                    \
    + CSOS_k_NET_CNXN_PARAMS_LEN                                               \
    + CSOS_k_SIZEOF_BYTE               /* Connection Path Size */              \
    + CSOS_k_ELECTRONIC_KEY_LEN                                                \
    + (2U * (CSOS_cfg_CCO_CNXN_RPATH_MAX_LEN_W                                 \
            +CSOS_cfg_CCO_CNXN_TPATH_MAX_LEN_W))  /* application path */       \
    + CSOS_k_SIZEOF_BYTE               /* Data Segment Type: Simple */         \
    + CSOS_k_SIZEOF_BYTE               /* Segment Length (in WORDs) */         \
    /* use Extended Format Safety Segment as it is longer */                   \
    + CSOS_k_SAFETY_SEG_EXT_LEN                                                \
  )  /* Config Data Segment not of interest here as it is inserted by CSAL */


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/


#endif /* CSOS_H */

/*** End of File ***/


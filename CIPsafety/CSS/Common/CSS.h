/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSS.h
**    Summary: CSS - stack global constants and data type
**             CSS macros, definitions and data types which are used
**             throughout the CIP Safety Stack.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: -
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef CSS_H
#define CSS_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSS_k_RESP_CONS_NUM_SCAST:
    Consumer number for Single-cast connections in a SafetyOpen Response.
*/
#define CSS_k_RESP_CONS_NUM_SCAST       0xFFFFU


/* maximum value for Ping_Interval_EPI_Multiplier */
#define CSS_k_MAX_PIEM                  999U       /* less than 1000 */
/* maximum value for Time_Coord_Msg_Min_Multiplier */
#define CSS_k_MAX_TCOOM_MIN_MULT_128US  7813U      /* == 1 second */
/* maximum value for Network_Time_Expectation_Multiplier */
#define CSS_k_MAX_NTE_MULT_128US        45313U     /* == 5.8 seconds */
/* minimum value for Timeout_Multiplier.PI */
#define CSS_k_MIN_TMULT_PI              1U
/* maximum value for Timeout_Multiplier.PI */
#define CSS_k_MAX_TMULT_PI              4U


/*
** Network Connection Parameters
*/
/*lint -esym(755, CSS_k_NCP_CNXN_TYPE_PTP)        not referenced in every cfg */
/*lint -esym(755, CSS_k_NCP_NOT_USED)             not referenced in every cfg */
/*lint -esym(755, CSS_k_NCP_MC_HP_FIX)            not referenced in every cfg */
/*lint -esym(755, CSS_k_NCP_MC_SP_FIX)            not referenced in every cfg */
/*lint -esym(755, CSS_k_NCP_MC_HP_FIX_TMSG_LEN)   not referenced in every cfg */
/*lint -esym(755, CSS_k_NCP_MC_SP_FIX_TMSG_LEN)   not referenced in every cfg */
/*lint -esym(755, CSS_IsPointToPoint)             not referenced in every cfg */
/* Connection Type: Point To Point */
#define CSS_k_NCP_CNXN_TYPE_PTP       0x4000U
/* Connection Type: MultiCast */
#define CSS_k_NCP_CNXN_TYPE_MCAST     0x2000U
/* Connection Size (0..511) */
#define CSS_k_NCP_CNXN_SIZE           0x01FFU
/* The other bits of the Network Connection Parameters are combined in the    */
/* following masks for simpler checking:                                      */
/* Point to Point, High Priority, Fixed Length 0 Bytes (not specified here) */
#define CSS_k_NCP_PTP_HP_FIX          0x4400U
/* Point to Point, Scheduled Prio, Fixed Length 0 Bytes (not specified here) */
#define CSS_k_NCP_PTP_SP_FIX          0x4800U
/* Point to Point, High Priority, Fixed Length 6 Bytes */
#define CSS_k_NCP_PTP_HP_FIX_TMSG_LEN 0x4406U
/* Point to Point, Scheduled Priority, Fixed Length 6 Bytes */
#define CSS_k_NCP_PTP_SP_FIX_TMSG_LEN 0x4806U
/* Multi-cast, High Priority, Fixed Length 0 Bytes (not specified here) */
#define CSS_k_NCP_MC_HP_FIX           0x2400U
/* Multi-cast, Scheduled Priority, Fixed Length 0 Bytes (not specified here) */
#define CSS_k_NCP_MC_SP_FIX           0x2800U
/* Multi-cast, High Priority, Fixed Length 6 Bytes*/
#define CSS_k_NCP_MC_HP_FIX_TMSG_LEN  0x2406U
/* Multi-cast, Scheduled Priority, Fixed Length 6 Bytes*/
#define CSS_k_NCP_MC_SP_FIX_TMSG_LEN  0x2806U
/* Connection not used */
#define CSS_k_NCP_NOT_USED            0x0000U


/* macro for quickly finding out if a connection is Single-cast */
#define CSS_IsPointToPoint(w_netCnxnPar)                                   \
          (  ((w_netCnxnPar) & CSS_k_NCP_CNXN_TYPE_PTP) ==                 \
             CSS_k_NCP_CNXN_TYPE_PTP                                       \
          )

/* macro for quickly finding out if a connection is Multi-cast */
#define CSS_IsMultiCast(w_netCnxnPar)                                      \
          (  ((w_netCnxnPar) & CSS_k_NCP_CNXN_TYPE_MCAST) ==               \
             CSS_k_NCP_CNXN_TYPE_MCAST                                     \
          )


/** CSS_k_DMF_xxx:
    Description of the Data Map Formats (Attribute 9 of the Connection
    Configuration Object).
*/
/*lint -esym(755, CSS_k_DMF_WORD_IDX)             not referenced in every cfg */
/*lint -esym(755, CSS_k_DMF_BYTE_IDX)             not referenced in every cfg */
/*lint -esym(755, CSS_k_DMF_VEND_SPEC_ASM)        not referenced in every cfg */
#define CSS_k_DMF_WORD_IDX        0U /* Open Scanner Format: Word Offsets */
#define CSS_k_DMF_BYTE_IDX        1U /* Byte-Index Scanner Format */
#define CSS_k_DMF_VEND_SPEC_ASM 100U /* Vendor specific Form.: Assemblies */


/*
** CIP Additional Status Codes of the Connection Manager
*/
/*lint -esym(755, CSS_k_CMEXTSTAT_RPI_NOT_SUP)    not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_NO_MORE_CNXN)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CFG_PATH)       not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CFGOP_NOT_AL)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CNXN_NOT_FND)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CPCRC)          not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_DUP_FWD_OPEN)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MISCELLANEOUS)  not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MAX_CONS_NUM)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_NET_CNXN_PAR)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_OUNID_CFG)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_OUNID_OUT)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_PIEM)           not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_DEV_TYPE)       not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_REVISION)       not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_P_OR_C_PATH)    not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_SCID)           not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TCCID)          not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TCCP)           not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TCMMM)          not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TCT)            not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TEXP_MULT)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TOUT_MULT)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TUNID_NOT_SET)  not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_TUNID_MISM)     not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_VID_OR_PRODC)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CM_EXSTAT_REQ_TIME_OUT)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MC_TCORR_RPI)   not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CONFIG_PATH)    not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_CONS_PATH)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_PROD_PATH)      not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MC_PIEM)        not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MC_MCN)         not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_MC_SNST)        not referenced in every cfg */
/*lint -esym(755, CSS_k_CMEXTSTAT_RPI_NOT_ACCPT)  not referenced in every cfg */
#define CSS_k_CMEXTSTAT_UNDEFINED     0xFFFFU /* no other matching EXTSTAT
                                                 code */
#define CSS_k_CMEXTSTAT_DUP_FWD_OPEN  0x0100U /* Connection in use or Duplicate
                                                 Forward_Open */
#define CSS_k_CMEXTSTAT_TCT           0x0103U /* Transport Class and Trigger
                                                 combination not supported */
#define CSS_k_CMEXTSTAT_OUNID_CFG     0x0105U /* Configuration is already owned
                                                 by another originator */
#define CSS_k_CMEXTSTAT_OUNID_OUT     0x0106U /* Output is already owned by
                                                 another originator */
#define CSS_k_CMEXTSTAT_CNXN_NOT_FND  0x0107U /* Target Connection not found
                                                 (Forward_Close) */
#define CSS_k_CMEXTSTAT_NET_CNXN_PAR  0x0108U /* Invalid Network Cnxn Param */
/* #define CSS_k_CMEXTSTAT_CNXN_SIZE  0x0109U    Invalid Cnxn Size */
#define CSS_k_CMEXTSTAT_DEV_NOT_CFG   0x0110U /* Device not configured */
#define CSS_k_CMEXTSTAT_RPI_NOT_SUP   0x0111U /* O_to_T RPI, T_to_O RPI or Time
                                                 Correction RPI not supported */
#define CSS_k_CMEXTSTAT_RPI_NOT_ACCPT 0x0112U /* O_to_T RPI or T_to_O RPI not
                                                 acceptable */
#define CSS_k_CMEXTSTAT_NO_MORE_CNXN  0x0113U /* All Safety Validator Instances
                                                 are being used */
#define CSS_k_CMEXTSTAT_VID_OR_PRODC  0x0114U /* Vendor ID or Product Code
                                                 specified in the electronic key
                                                 does not match */
#define CSS_k_CMEXTSTAT_DEV_TYPE      0x0115U /* Product Type specified in the
                                                 electronic key does not
                                                 match */
#define CSS_k_CMEXTSTAT_REVISION      0x0116U /* Major or Minor Revision
                                                 specified in the electronic key
                                                 does not match */
#define CSS_k_CMEXTSTAT_P_OR_C_PATH   0x0117U /* Invalid Produced or Consumed
                                                 Application Path. Deprecated:
                                                 use 0x12A or 0x12B */
#define CSS_k_CMEXTSTAT_CFG_PATH      0x0118U /* Invalid or inconsistent
                                                 Configuration Application
                                                 Path. Deprecated: use 0x129 */
#define CSS_k_CMEXTSTAT_CONFIG_PATH   0x0129U /* Invalid Configturation
                                                 Application Path
                                                 (replaces 0x0118) */
#define CSS_k_CMEXTSTAT_CONS_PATH     0x012AU /* Invalid Consuming Application
                                                 Path (replaces 0x0117) */
#define CSS_k_CMEXTSTAT_PROD_PATH     0x012BU /* Invalid Producing Application
                                                 Path (replaces 0x0117) */


#define CSS_k_CM_EXSTAT_REQ_TIME_OUT  0x0204U /* Target did not respond on
                                                 Unconnected Request */
#define CSS_k_CMEXTSTAT_PARAM_ERR     0x0205U /* Parameter Error in Safety
                                                 Open */
#define CSS_k_CMEXTSTAT_SEGMENT       0x0315U /* Invalid Segment Type in Cnxn
                                                 Path */
#define CSS_k_CMEXTSTAT_MISCELLANEOUS 0x031CU /* no other extended status code
                                                 applies */

/* Safety Parameter Errors */
#define CSS_k_CMEXTSTAT_MC_TCORR_RPI  0x0801U /* Time Correction RPI invalid on
                                                 Multicast join */
#define CSS_k_CMEXTSTAT_SCNXN_SIZE    0x0802U /* Invalid Safety Cnxn Size */
#define CSS_k_CMEXTSTAT_SCNXN_FORMAT  0x0803U /* Invalid Safety Cnxn Format */
#define CSS_k_CMEXTSTAT_TCCP          0x0804U /* Invalid Time Correction
                                                 Connection Parameters */
#define CSS_k_CMEXTSTAT_PIEM          0x0805U /* Invalid Ping Interval EPI
                                                 Multiplier */
#define CSS_k_CMEXTSTAT_TCMMM         0x0806U /* Invalid Time Coordination Msg
                                                 Min Multiplier */
#define CSS_k_CMEXTSTAT_TEXP_MULT     0x0807U /* Invalid Time Expect. Mult. */
#define CSS_k_CMEXTSTAT_TOUT_MULT     0x0808U /* Invalid Timeout Multiplier */
#define CSS_k_CMEXTSTAT_MAX_CONS_NUM  0x0809U /* Invalid Max Consumer Number */
#define CSS_k_CMEXTSTAT_CPCRC         0x080AU /* Invalid CPCRC */
#define CSS_k_CMEXTSTAT_TCCID         0x080BU /* Time Correction Cnxn ID
                                                 invalid */
#define CSS_k_CMEXTSTAT_SCID          0x080CU /* SCID mismatch */
#define CSS_k_CMEXTSTAT_TUNID_NOT_SET 0x080DU /* TUNID not set */
#define CSS_k_CMEXTSTAT_TUNID_MISM    0x080EU /* TUNID mismatch */
#define CSS_k_CMEXTSTAT_CFGOP_NOT_AL  0x080FU /* Configuration operation not
                                                 allowed */
#define CSS_k_CMEXTSTAT_MC_PIEM       0x0815U /* Invalid Ping Interval EPI
                                                 Multiplier on Multicast join */
#define CSS_k_CMEXTSTAT_MC_MCN        0x0816U /* Invalid Multicast Max Consumer
                                                 Number on Multicast join */
#define CSS_k_CMEXTSTAT_MC_SNST       0x0817U /* Invalid Safety Network Segment
                                                 Type on Multicast join */


/** CSS_k_TICK_BASE_US:
    Base timer tick of the CSS in micro seconds.
*/
#define CSS_k_TICK_BASE_US            128U


/** CSS_k_RIF_xxx:
    This defines the possible values for the Run Idle Flag in an Assembly
    structure / Process Image.
*/
#define CSS_k_RIF_IDLE                0x00U
#define CSS_k_RIF_RUN                 0xAAU


/* Connection Manager Class specific Service Codes */
#define CSS_k_SRVC_FORWARD_OPEN       0x54U  /* Forward_Open service */
#define CSS_k_SRVC_FORWARD_CLOSE      0x4EU  /* Forward_Close service */


/* Minimum/Maximum payload length of a Short/Long Safety I/O messages */
/*lint -esym(755, CSS_k_SHORT_FORMAT_MIN_LENGTH)  not referenced in every cfg */
/*lint -esym(755, CSS_k_LONG_FORMAT_MIN_LENGTH)   not referenced in every cfg */
#define CSS_k_SHORT_FORMAT_MIN_LENGTH 1U
#define CSS_k_SHORT_FORMAT_MAX_LENGTH 2U
#define CSS_k_LONG_FORMAT_MIN_LENGTH  3U
/* Long Format max length is configured by CSOS_cfg_LONG_FORMAT_MAX_LENGTH */


/*******************************************************************************
**    data types
*******************************************************************************/

/** CSS_t_DATA_MSG:
    Groups the possible data messages of a Safety Validator Instance. Can be
    either Base or Extended Format and Short or Long Format.
*/
typedef struct
{
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
    CSS_t_UDINT u32_crcS5;                              /* CRC-S5 */
  #endif
  #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
    CSS_t_UDINT u32_cCrcS5;                             /* Compl. CRC-S5 */
  #endif
#endif
  CSS_t_UINT  u16_timeStamp_128us;                      /* Time Stamp */

  /* if long format is supported then reserve a long buffer (can then be used
     for long and short messages/ */
#if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
  CSS_t_UINT  u16_aCrcS3;                               /* Actual CRC-S3 */
  /* 3 to 250 Byte Data Section, Base Format */
  CSS_t_BYTE  ab_aData[CSOS_cfg_LONG_FORMAT_MAX_LENGTH];/* Actual Data (long) */
  CSS_t_BYTE  ab_cData[CSOS_cfg_LONG_FORMAT_MAX_LENGTH];/* Compl. Data */
#else
  /* 1 or 2 Byte Data Section, Base Format */
  CSS_t_BYTE  ab_aData[CSS_k_SHORT_FORMAT_MAX_LENGTH];  /* Actual Data (short)*/
#endif
  CSS_t_USINT u8_len;                                   /* num of Data Bytes */
  CSS_t_BYTE  b_modeByte;                               /* Mode Byte */

#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  #if (CSOS_cfg_LONG_FORMAT == CSOS_k_ENABLE)
    CSS_t_UINT  u16_cCrcS3;                             /* Compl. CRC-S3 */
  #endif
  CSS_t_USINT u8_tCrcS1;                                /* Time Stamp CRC-S1 */
  #if (CSOS_cfg_SHORT_FORMAT == CSOS_k_ENABLE)
    CSS_t_USINT u8_aCrcS1;                              /* Actual CRC-S1 */
    CSS_t_USINT u8_cCrcS2;                              /* Complement CRC-S2 */
  #endif
#endif

} CSS_t_DATA_MSG;


/** CSS_t_TCOO_MSG:
    Time Coordination Message
*/
typedef struct
{
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UDINT u32_crcS5;                   /* CRC-S5 */
#endif
  CSS_t_UINT  u16_consTimeVal_128us;       /* Consumer_Time_Value */
  CSS_t_BYTE  b_ackByte;                   /* Ack_Byte */
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_BYTE  b_ackByte2;                  /* Ack_Byte_2 */
  CSS_t_UINT  u16_crcS3;                   /* CRC-S3 */
#endif
} CSS_t_TCOO_MSG;


/** CSS_t_TCR_MSG:
    Time Correction Message
*/
typedef struct
{
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UDINT u32_crcS5;                   /* CRC-S5 */
#endif
  CSS_t_UINT  u16_timeCorrVal_128us;       /* Time_Correction_Value */
  CSS_t_BYTE  b_mCastByte;                 /* MCast_Byte */
#if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
  CSS_t_BYTE  b_mCastByte2;                /* MCast_Byte_2 */
  CSS_t_UINT  u16_crcS3;                   /* CRC-S3 */
#endif
} CSS_t_TCORR_MSG;


/** CSS_t_PIDCID:
    This type is used for storing PID (Producer Identifier) or CID (Consumer
    Identifier) information.
*/
typedef struct
{
  CSS_t_UDINT u32_devSerNum;    /* Device Serial Number */
  CSS_t_UINT  u16_vendId;       /* Vendor ID */
  CSS_t_UINT  u16_cnxnSerNum;   /* Connection Serial Number */
} CSS_t_PIDCID;


/** CSS_k_MAX_NUM_EXT_STATUS_WORDS:
    Maximum number of additional/extended status words supported.
*/
#define CSS_k_MAX_NUM_EXT_STATUS_WORDS   5U

/** CSS_t_MR_RESP_STATUS:
    Structure for passing the CIP service status in the Message Router
    Response format message. This structure can pass more than one Extended
    Status words.
*/
/*lint -esym(768, au16_moreExt)                   not referenced in every cfg */
/*lint -esym(768, u8_numMoreExt)                  not referenced in every cfg */
typedef struct
{
  CSS_t_UINT  u16_ext;          /* Extended Status Code (at some places in the
                                   CIP Specs also called "Additional Status").
                                   This is the first word. In case there are
                                   more extended status words then these are
                                   placed in au16_moreExt. */
  CSS_t_UINT au16_moreExt[CSS_k_MAX_NUM_EXT_STATUS_WORDS];
                                /* More Extended Status Code words to be
                                   appended after u16_ext. */
  CSS_t_USINT u8_numMoreExt;    /* Number of bytes used in the array
                                   au16_moreExt[]. */
  CSS_t_USINT u8_gen;           /* General Status Code */
} CSS_t_MR_RESP_STATUS;


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /** CSS_t_APP_REPLY:
      Structure for passing the Application Reply data for a Forward_Open
      Success Response from the Safety Validator to the Forward_Open Response
      sending function.
  */
  typedef struct
  {
    CSS_t_UDINT   u32_trgDevSerNum;   /* Target's Device Serial Number */
    CSS_t_UINT    u16_consNum;        /* Consumer_Number (encoded as UINT) */
    CSS_t_UINT    u16_trgVendId;      /* Target's Vendor ID */
    CSS_t_UINT    u16_trgCnxnSerNum;  /* Target Connection Serial Number */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    CSS_t_UINT    u16_initialTS;      /* Initial Time Stamp (128 us ticks) */
    CSS_t_UINT    u16_initialRV;      /* Initial Rollover Value */
  #endif
    CSS_t_UINT    u16_instId;         /* safety validator instance ID */
    CSS_t_BYTE    b_cnxnType;         /* type of this connection - see
                                         {CSOS_k_CNXN_xxx} */
  } CSS_t_APP_REPLY;


  /** CSS_t_SOPEN_RESP:
      Structure for passing the General Status Code and the Application Reply
      data for a Froward Open Response from the Safety Validator to the
      Forward_Open Response sending function.
  */
  typedef struct
  {
    CSS_t_MR_RESP_STATUS s_status;    /* Status Code (General and
                                         optionally (several) Extended
                                         Status Words) */
    /* following fields are only of interest if the General Status Code is ok */
    CSS_t_APP_REPLY s_appReply;
  } CSS_t_SOPEN_RESP;
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)

  /** CSS_t_SOPEN_RESP_RX:
      Structure that contains the received SafetyOpen Response from the Target
      plus additional information needed by the originator to establish its
      part of the Safety Connection.
  */
  typedef struct
  {
    /* from successful Forward_Open Reply */
    CSS_t_UDINT   u32_netCnxnId_OT; /* CIP Consumed Connection ID */
    CSS_t_UDINT   u32_netCnxnId_TO; /* CIP Producing Connection ID */
    CSS_t_CNXN_TRIAD s_cnxnTriad;   /* Connection Triad: Combination of:
                                       Connection Serial Number
                                       Originator Vendor ID
                                       Originator Serial Number */
    CSS_t_UDINT   u32_apiOT_us;     /* O_to_T_API */
    CSS_t_UDINT   u32_apiTO_us;     /* T_to_O_API */
    /* from CIP Safety Target Application Reply */
    CSS_t_UDINT   u32_trgDevSerNum; /* Target's Device Serial Number */
    CSS_t_UINT    u16_consNum;      /* Consumer_Number (encoded as UINT) */
    CSS_t_UINT    u16_trgVendId;    /* Target's Vendor ID */
    CSS_t_UINT    u16_trgCnxnSerNum;/* Target Connection Serial Number */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* information only needed for Extended Format */
    CSS_t_UINT    u16_initialTs;    /* Initial Time Stamp */
    CSS_t_UINT    u16_initialRv;    /* Initial Rollover Value */
  #endif
    /* Status Codes */
    CSS_t_UINT    u16_extStat;      /* Extended Status Code */
    CSS_t_USINT   u8_genStat;       /* General Status Code */
  } CSS_t_SOPEN_RESP_RX;

  /** CSS_t_INIT_VAL:
      Structure that is used to pass the values of Initial Time Stamp and
      Initial Rollover value for the SafetyOpen Request/Response between
      functions.
  */
  typedef struct
  {
    CSS_t_UINT u16_initialTs;
    CSS_t_UINT u16_initialRv;
  } CSS_t_INIT_VAL;


  /** CSS_t_CCO_SOPEN_INIT_VAL:
      This structure type is needed for passing information from the CCO to the
      SVCEE wehen a SafetyOpen response is received.
  */
  typedef struct
  {
    /* application reference                                                  */
    /* (must be passed to CSAL when this connection is opened)                */
    CSS_t_UDINT   u32_appRef;
    /* additional info needed by the Originator CSS to establish Safety Cnxns */
    CSS_t_UINT    u16_dataOffset;   /* Server: Offset into Output Data Table */
                                    /* Client: Offset into Input Data Table */
    CSS_t_UINT    u16_pingIntEpiMult; /* Ping Interval EPI Multiplier */
    CSS_t_UINT    u16_tcooMsgMinMult_128us; /* Time Coord Msg Min Multiplier */
    CSS_t_UINT    u16_NetTimeExpMult_128us; /* Network Time Expectation Mult */
  #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
    /* information only needed for Extended Format */
    CSS_t_UINT    u16_MaxFault_Number;  /* Max Fault Number */
  #endif
    CSS_t_BYTE    b_cnxnType;       /* producer/consumer - single-/multicast */
    CSS_t_USINT   u8_cnxnTmult;     /* Connection Timeout Multiplier */
    CSS_t_USINT   u8_format;        /* Safety Network Segment Format */
    CSS_t_USINT   u8_dataLen;       /* produced/consumed data length */
    CSS_t_USINT   u8_timeoutMult;   /* (Safety) Timeout Multiplier */
    CSS_t_USINT   u8_maxConsNum;    /* Max Consumer Number */
  } CSS_t_CCO_SOPEN_INIT_VAL;

#endif  /* (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE) */


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  /** CSS_t_SV_ID:
      Structure describing a Safety Connection by Safety Validator Instance ID
      and Consumer Number.
  */
  typedef struct
  {
    CSS_t_UINT  u16_instId;     /* Safety Validator Instance ID */
    CSS_t_BOOL  o_isServer;     /* CSS_k_TRUE if this is a Server instance */
    CSS_t_USINT u8_consNum;     /* Consumer Number */
    CSS_t_USINT u8_state;       /* Safety Validator State */
    CSS_t_USINT u8_cnxnStatus;  /* Producer/Producing Connection Status (Client)
                                   or Consuming Connection Status (Server) */
    CSS_t_BOOL  o_multiProd;    /* CSS_k_TRUE if this produces multicast */
  } CSS_t_SV_ID;
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  /** CSS_t_SOFT_ERR_RET_STAT:
      Structure describing the return values of the soft error detection sub-
      functions.
  */
  typedef struct
  {
    CSS_t_USINT u8_cpyLen;    /* length (number of bytes) of returned data in
                                 pb_var array */
    CSS_t_USINT u8_incLvl;    /* level counter that must be incremented to get
                                 to next variable */
  } CSS_t_SOFT_ERR_RET_STAT;

  /* number of levels in the soft error detection Variables get functions */
  #define CSS_k_SOFT_ERR_NUM_OF_LEVELS      4U

#endif


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/


#endif /* #ifndef CSS_H */


/*** End of File ***/



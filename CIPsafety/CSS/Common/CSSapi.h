/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSSapi.h
**    Summary: CSS API
**             General Macros and defines necessary to use the CSS. Also the
**             macros to serialize/ de-serialize CIP frames (See Req.4.1-3) are
**             defined here.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: SAPL_CssProduceEvenBytesClbk
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef CSS_API_H
#define CSS_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSS_k_STACK_VERSION:
    This symbol represents the version of the CIP Safety Stack and unittest.
*/
#define CSS_k_STACK_VERSION     "1.8.0"

/** CSS_k_STACK_STATUS:
    This symbol represents the status of the CSOS stack and unittest.
*/
#define CSS_k_STACK_STATUS      "Release"


/** CSS_k_OK:
    Generally used return value of functions indicating a successful execution.
*/
#define CSS_k_OK     0x0000U


/** CSS_k_SV_STATE_xxx:
    Safety Validator State Attribute Values (instance attribute 1 and in
    {CSS_t_VALIDATOR_INFOS})
*/
/* Unallocated or Idle (CIP Volume 5 uses both terms) */
#define CSS_k_SV_STATE_IDLE              0U  /* Idle */
#define CSS_k_SV_STATE_INITIALIZING      1U  /* Initializing */
#define CSS_k_SV_STATE_ESTABLISHED       2U  /* Established */
#define CSS_k_SV_STATE_FAILED            3U  /* Connection Failed */

/** CSS_k_TYPE_xxx:
    Defines (bit masks) for assembling Safety Validator Type (instance
    attribute 2 and in {CSS_t_VALIDATOR_INFOS})
*/
/*lint -esym(755, CSS_k_TYPE_SERVER)              not referenced in every cfg */
#define CSS_k_TYPE_SERVER                0x80U /* Bit 7 [1]: Consumer(server) */
#define CSS_k_TYPE_CLIENT                0x00U /* Bit 7 [0]: Producer(client) */
#define CSS_k_TYPE_SCAST                 0x01U /* Bit 0: Single-cast */
#define CSS_k_TYPE_MCAST                 0x02U /* Bit 1: Multi-cast */

/** CSS_k_CNXN_STATUS_xxx:
    Status of a Safety Validator Instance (in {CSS_t_VALIDATOR_INFOS}
*/
/*lint -esym(755, CSS_k_CNXN_STATUS_CONS_RUN)     not referenced in every cfg */
/*lint -esym(755, CSS_k_CNXN_STATUS_PROD_ACTIVE)  not referenced in every cfg */
#define CSS_k_CNXN_STATUS_FAULTED        0U  /* Faulted */
#define CSS_k_CNXN_STATUS_IDLE           1U  /* Idle */
#define CSS_k_CNXN_STATUS_PROD_ACTIVE    2U  /* Producer Active */
#define CSS_k_CNXN_STATUS_CONS_RUN       3U  /* Consumer Run */


#if (CSS_k_PLATFORM_FACTOR == 2)
  /** CSS_MASK_HIGH_B:
      This macro masks the high byte of the given word at the given offset.
  */
  #define CSS_MASK_HIGH_B(pv_src, ofs) \
                                 ((*(((CSS_t_USINT*)(pv_src)) + ofs)) & 0x00FFU)
#endif


/*******************************************************************************
**    data types
*******************************************************************************/

/**
    CIP Safety Data Types:

    The following section contains the definition of data types specified by
    CIP specification (CIP Networks Library Vol.5).
*/

/** CSS_t_DATE_AND_TIME:
    Implements the standard CIP type DATE_AND_TIME (DT). Can encode all
    dates and times from 1972 to about 2150.
*/
typedef struct
{
  CSS_t_UDINT  u32_time;
  CSS_t_UINT   u16_date;
} CSS_t_DATE_AND_TIME;


/** CSS_t_SNN:
    This structure represents a "Safety Network Number". The type is equal to
    the type DATE_AND_TIME.
*/
typedef CSS_t_DATE_AND_TIME CSS_t_SNN;

/** CSS_t_UNID:
    Unique Network Identifier - Used in the Safety Stack to uniquely identify a
    device among several networks.
*/
typedef struct
{
  CSS_t_UDINT u32_nodeId;  /* NodeID
                              - For SERCOS III Safety Devices this is the SDID
                              - For EtherNet/IP Safety Devices this is the
                                IP-address
                              - For DeviceNet Safety Devices this is the MAC ID
                            */
  CSS_t_SNN   s_snn;     /* Safety Network Number */
} CSS_t_UNID;


/** CSS_t_SCID:
    This structure represents an "SCID" (Safety Configuration Identifier)
*/
typedef struct
{
  CSS_t_UDINT  u32_crc;   /* Safety Configuration CRC (SCCRC) */
  CSS_t_UDINT  u32_time;  /* Safety Configuration Time Stamp (SCTS), Time */
  CSS_t_UINT   u16_date;  /* Safety Configuration Time Stamp (SCTS), Date */
} CSS_t_SCID;


/** CSS_t_CNXN_TRIAD:
    Structure describing a Safety connection by the combination of Connection
    Serial Number, Originator Vendor ID and Originator Serial Number
    parameters.
*/
typedef struct
{
  CSS_t_UDINT u32_origSerNum;
  CSS_t_UINT  u16_cnxnSerNum;
  CSS_t_UINT  u16_origVendId;
} CSS_t_CNXN_TRIAD;


/** CSS_t_ELECTRONIC_KEY:
    Structure for storing the electronic key segment received with a
    Forward_Open request.
*/
typedef struct
{
  CSS_t_UINT  u16_vendId;         /* Vendor ID */
  CSS_t_UINT  u16_devType;        /* Device Type (at some places in the CIP
                                     specs also called "Product Type") */
  CSS_t_UINT  u16_prodCode;       /* Product Code */
  CSS_t_BYTE  b_compMajorRev;     /* Compatibility / Major Revision
                                     (Major Revision is an USINT but it is
                                     combined with the Compatibility Bit) */
  CSS_t_USINT u8_minorRev;        /* Minor Revision */
} CSS_t_ELECTRONIC_KEY;


/** CSS_t_NET_SEG_SAFETY_T_FRMT:
    Structure containing the parameters from the Network Segment Safety (part
    of the data of a SafetyOpen service). This structure covers Target (=Base)
    Format as well as Extended Format.
*/
typedef struct
{
  CSS_t_UDINT         u32_sccrc;          /* Configuration CRC (SCCRC) */
  CSS_t_UDINT         u32_cpcrc;          /* Connection Param CRC (CPCRC) */
  CSS_t_UDINT         u32_tcorrCnxnId;    /* Time Correction Connection ID
                                             - not used (DeviceNet only) */
  CSS_t_UDINT         u32_tcorrEpi_us;    /* Time Correction EPI */
  CSS_t_DATE_AND_TIME s_scts;             /* Configuration Timestamp (SCTS) */
  CSS_t_UINT          u16_pingIntEpiMult; /* Ping_Interval_EPI_Multiplier */
  CSS_t_UINT          u16_tcooMsgMinMult_128us; /* Time_Coord_Msg_Min_Mult */
  CSS_t_UINT          u16_NetTimeExpMult_128us; /* Network_Time_Expect_Mult */
  CSS_t_WORD          w_tcorrNetCnxnPar;  /* Time Corr Network Cnxn Params */
  CSS_t_UNID          s_tunid;            /* Target_UNID (TUNID) */
  CSS_t_UNID          s_ounid;            /* Originator_UNID (OUNID) */
  /* Additional parameters for extended format */
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UINT          u16_maxFaultNum;    /* Max_Fault_Number */
  CSS_t_UINT          u16_initalTs;       /* Initial Time Stamp */
  CSS_t_UINT          u16_initialRv;      /* Initial Rollover Value */
#endif
  CSS_t_USINT         u8_format;          /* Safety Network Segment Format */
  CSS_t_USINT         u8_timeoutMult;     /* Timeout_Multiplier */
  CSS_t_USINT         u8_maxConsNum;      /* Max_Conumer_Number */
} CSS_t_NET_SEG_SAFETY_T_FRMT;


/** CSS_t_EXPL_REQ:
    Structure containing the service parameters of an explicit request.
*/
typedef struct
{
  CSS_t_UDINT  u32_reqId;        /* identifier for request/response matching */
  CSS_t_UDINT  u32_instance;     /* Instance ID */
  CSS_t_UDINT  u32_member;       /* Member ID */
  const CSS_t_BYTE *pb_reqData;  /* pointer to service specific data */
  CSS_t_UINT   u16_class;        /* Class ID */
  CSS_t_UINT   u16_attribute;    /* Attribute ID */
  CSS_t_UINT   u16_reqDataLen;   /* length of service specific data */
  CSS_t_USINT  u8_service;       /* Service Code */
} CSS_t_EXPL_REQ;


/** CSS_t_EXPL_RSP:
    Structure containing the response to an explicit request.
*/
typedef struct
{
  CSS_t_UINT   u16_rspDataLen;   /* length of data in the response buffer */
  CSS_t_UINT   au16_addStat[CSOS_cfg_ADD_STAT_SIZE_MAX]; /* Add Status Codes */
  CSS_t_USINT  u8_service;       /* Reply Service Code (set by the stack) */
  CSS_t_USINT  u8_genStat;       /* General Status Code */
  CSS_t_USINT  u8_addStatSize;   /* Num of Words in Additional Status array */
#if (CSOS_cfg_ORIGINATOR == CSOS_k_ENABLE)
  CSS_t_BOOL   o_csalInsert;     /* flag indicating that the CSAL has to insert
                                    data into the Explicit Response message */
#endif
} CSS_t_EXPL_RSP;


/** CSS_t_VALIDATOR_INFOS:
    Structure used to request info about a specific Safety Validator Instance.
*/
typedef struct
{
  CSS_t_UINT  u16_maxDataAge_128us; /* Max Data Age attribute of the instance */
  CSS_t_UINT  u16_cnxnPoint;        /* Cnxn Point associated to this Cnxn */
  CSS_t_USINT u8_state;             /* State attribute of the instance */
  CSS_t_USINT u8_type;              /* Type attribute of the instance */
} CSS_t_VALIDATOR_INFOS;


/** CSS_t_ID_INFO:
    Structure type for passing initialization information to the CSS.
*/
typedef struct
{
  CSS_t_UDINT u32_serNum;        /* Serial Number */
  CSS_t_UDINT au32_nodeId[CSOS_cfg_NUM_OF_SAFETY_PORTS];   /* NodeIDs (one for
                                      each port)
                                    - For SERCOS III Safety Devices this is the
                                      SDID
                                    - For EtherNet/IP Safety Devices this is the
                                      IP-address
                                    - For DeviceNet Safety Devices this is the
                                      MAC ID
                                    A safety device shall have a single physical
                                    address that is unique on the device's
                                    network segment (see FRS14).
                                    See also the remarks about the Node ID for
                                    Multiple Safety Port Devices in the
                                    description of the define
                                    CSOS_cfg_NUM_OF_SAFETY_PORTS. */
  CSS_t_UINT  u16_vendorId;      /* Vendor ID */
  /*lint -esym(768, u16_devType)                  not referenced in every cfg */
  CSS_t_UINT  u16_devType;       /* Device Type (at some places in the CIP
                                    specs also called "Product Type") */
  CSS_t_UINT  u16_prodCode;      /* Product Code */
  CSS_t_USINT u8_majorRev;       /* Major Revision */
  CSS_t_USINT u8_minorRev;       /* Minor Revision */
#if (CSOS_cfg_NUM_OF_SAFETY_PORTS > 1U)
                                 /* Array of Port Numbers */
  CSS_t_UINT au16_portNumbers[CSOS_cfg_NUM_OF_SAFETY_PORTS];
#endif
} CSS_t_ID_INFO;


/** CSS_t_SOPEN_PARA:
    Structure containing the parameters of a SafetyOpen service.
*/
typedef struct
{
  const CSS_t_BYTE  *pb_cfgData;          /* Pointer to Config Data */
  CSS_t_UDINT u32_netCnxnId_OT;           /* CIP Consumed Connection ID */
  CSS_t_UDINT u32_netCnxnId_TO;           /* CIP Producing Connection ID */
  CSS_t_UDINT u32_rpiOT_us;               /* O_to_T_RPI */
  CSS_t_UDINT u32_rpiTO_us;               /* T_to_O_RPI */
  CSS_t_CNXN_TRIAD s_cnxnTriad;           /* Connection Triad: Combination of:
                                             Connection Serial Number
                                             Originator Vendor ID
                                             Originator Serial Number */
  /* Originator to Target Network Connection Parameters (16 bits, bit coded):
     0..8:   Connection Size (in bytes) - size of the whole safety container
     9:      Fixed/Variable             - for safety only "fixed" (0) is allowed
     10..11: Priority                   - for safety allowed values are
                                          "High" (01) and "Scheduled" (10)
     12:     Reserved                   - (0)
     13..14: Connection Type            - for O->T direction only Point to Point
                                          (10) is allowed
     15:     Redundant Owner            - for safety there is no redundancy (0)
  */
  CSS_t_WORD  w_netCnxnPar_OT;            /* O_to_T_Network Cnxn Parameters */
  /* Target to Originator Network Connection Parameters (16 bits, bit coded):
     0..8:   Connection Size (in bytes) - size of the whole safety container
     9:      Fixed/Variable             - for safety only "fixed" (0) is allowed
     10..11: Priority                   - for safety allowed values are
                                          "High" (01) and "Scheduled" (10)
     12:     Reserved                   - (0)
     13..14: Connection Type            - Point to Point (10) or Multicast (10)
     15:     Redundant Owner            - for safety there is no redundancy (0)
  */
  CSS_t_WORD  w_netCnxnPar_TO;            /* T_to_O_Network Cnxn Parameters */
  CSS_t_UINT  u16_cfgClass;               /* Application Path 1: Class */
  CSS_t_UINT  u16_cfgInst;                /* Application Path 1: Instance */
  CSS_t_UINT  u16_cnxnPointProd;          /* Application Path 2: Cnxn Point */
  CSS_t_UINT  u16_cnxnPointCons;          /* Application Path 3: Cnxn Point */
  CSS_t_ELECTRONIC_KEY        s_elKey;    /* Electronic Key */
  CSS_t_NET_SEG_SAFETY_T_FRMT s_nsd;      /* Network Safety Data */
  CSS_t_USINT u8_cnxnTimeoutMultiplier;   /* Connection Timeout Multiplier */
  CSS_t_BYTE  b_transportTypeTrigger;     /* Transport Type Trigger */
  CSS_t_BYTE  u8_cfgDataSizeW;            /* Config Data Size (num of words) */
} CSS_t_SOPEN_PARA;


/*******************************************************************************
**    global variables
*******************************************************************************/

/* check if platform defines are present */
#ifndef CSS_k_ENDIAN
  #error CSS_k_ENDIAN is undefined!
#endif


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : CSS_N2H_CPY8
**
** Description : This function copies the data of any 8 bit type from network
**               (N) format to host (H) format.
**
** Parameters  : pv_dst (IN) - destination pointer to the data in host (H)
**                             format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer to the data in network (N) format
**                             (not checked), valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_k_PLATFORM_FACTOR == 1)
  #define CSS_N2H_CPY8(pv_dst, pv_src) \
          ((*((CSS_t_USINT*)(pv_dst))) = (*((CSS_t_USINT*)(pv_src))))
#elif (CSS_k_PLATFORM_FACTOR == 2)
  #define CSS_N2H_CPY8(pv_dst, pv_src) \
          ((*((CSS_t_USINT*)(pv_dst))) = (*((CSS_t_USINT*)(pv_src))))
#endif


/*******************************************************************************
**
** Function    : CSS_N2H_CPY16
**
** Description : This function copies the data of any 16 bit type from network
**               (N) format to host (H) format.
**
** Parameters  : pv_dst (IN) - destination pointer to the data in host (H)
**                             format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer to the data in network (N) format
**                             (not checked), valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_N2H_CPY16(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) +0U)) = (*(((CSS_t_USINT*)(pv_src)) +1U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +1U)) = (*(((CSS_t_USINT*)(pv_src)) +0U)))
  #else
    #define CSS_N2H_CPY16(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) +0U)) = (*(((CSS_t_USINT*)(pv_src)) +0U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +1U)) = (*(((CSS_t_USINT*)(pv_src)) +1U)))
  #endif
#elif (CSS_k_PLATFORM_FACTOR == 2)
    #define CSS_N2H_CPY16(pv_dst, pv_src)                                      \
      (*((CSS_t_UINT*)(pv_dst))) =                                             \
        (((CSS_t_UINT)(((CSS_t_UINT)CSS_MASK_HIGH_B(pv_src, 0)) << 0)) |       \
         ((CSS_t_UINT)(((CSS_t_UINT)CSS_MASK_HIGH_B(pv_src, 1)) << 8)))
#endif


/*lint -esym(755, CSS_N2H_CPY32)                  not referenced in every cfg */
/*******************************************************************************
**
** Function    : CSS_N2H_CPY32
**
** Description : This function copies the data of any 32 bit type from network
**               (N) format to host (H) format.
**
** Parameters  : pv_dst (IN) - destination pointer to the data in host (H)
**                             format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer to the data in network (N) format
**                             (not checked), valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_N2H_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) +0U)) = (*(((CSS_t_USINT*)(pv_src)) +3U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +1U)) = (*(((CSS_t_USINT*)(pv_src)) +2U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +2U)) = (*(((CSS_t_USINT*)(pv_src)) +1U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +3U)) = (*(((CSS_t_USINT*)(pv_src)) +0U)))
  #else
    #define CSS_N2H_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) +0U)) = (*(((CSS_t_USINT*)(pv_src)) +0U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +1U)) = (*(((CSS_t_USINT*)(pv_src)) +1U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +2U)) = (*(((CSS_t_USINT*)(pv_src)) +2U))); \
      ((*(((CSS_t_USINT*)(pv_dst)) +3U)) = (*(((CSS_t_USINT*)(pv_src)) +3U)))
  #endif
#elif (CSS_k_PLATFORM_FACTOR == 2)
    #define CSS_N2H_CPY32(pv_dst, pv_src)                                      \
      (*((CSS_t_UDINT*)(pv_dst))) =                                            \
       (  ((CSS_t_UDINT)(((CSS_t_UDINT)CSS_MASK_HIGH_B(pv_src, 0)) <<  0)) |   \
          ((CSS_t_UDINT)(((CSS_t_UDINT)CSS_MASK_HIGH_B(pv_src, 1)) <<  8)) |   \
          ((CSS_t_UDINT)(((CSS_t_UDINT)CSS_MASK_HIGH_B(pv_src, 2)) << 16)) |   \
          ((CSS_t_UDINT)(((CSS_t_UDINT)CSS_MASK_HIGH_B(pv_src, 3)) << 24))  )
#endif


/*******************************************************************************
**
** Function    : CSS_H2N_CPY8
**
** Description : This function copies the data of any 8 bit type from the
**               source pointer to the destination pointer.
**
** Parameters  : pv_dst (IN) - destination pointer (not checked),
**                             valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer (not checked),
**                             valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#define CSS_H2N_CPY8(pv_dst, pv_src)                                           \
        ((*((CSS_t_USINT*)(pv_dst))) = (*((CSS_t_USINT*)(pv_src))))


/*******************************************************************************
**
** Function    : CSS_H2N_CPY16
**
** Description : This macro swaps the 16 bit data if it is necessary and
**               copies. The data will be copied from the source pointer to the
**               destination pointer.
**
** Parameters  : pv_dst (IN) - destination pointer (not checked),
**                             valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer (not checked),
**                             valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_H2N_CPY16(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 1U)));                                 \
      ((*(((CSS_t_USINT*)(pv_dst)) + 1U)) = (*(((CSS_t_USINT*)(pv_src)) + 0U)))
  #else
    #define CSS_H2N_CPY16(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 0U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 1U)) = (*(((CSS_t_USINT*)(pv_src)) + 1U)))
  #endif
#else
    #define CSS_H2N_CPY16(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src)))) & 0x00FFU));                \
      ((*(((CSS_t_UINT*)(pv_dst)) + 1U)) =                                     \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src)))) >> 8U));

#endif

/*******************************************************************************
**
** Function    : CSS_H2N_CPY32
**
** Description : This function swaps the 32 bit data if it is necessary and
**               copies. The data will be copied from the source pointer to the
**               destination pointer.
**
** Parameters  : pv_dst (IN) - destination pointer (not checked),
**                             valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer (not checked),
**                             valid range: != CSS_k_NULL
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_H2N_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 3U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 1U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 2U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 2U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 1U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 3U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 0U)))
  #else
    #define CSS_H2N_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 0U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 1U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 1U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 2U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 2U)));                                   \
      ((*(((CSS_t_USINT*)(pv_dst)) + 3U)) =                                    \
        (*(((CSS_t_USINT*)(pv_src)) + 3U)))
  #endif
#else
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_H2N_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU));             \
      ((*(((CSS_t_UINT*)(pv_dst)) + 1U)) =                                     \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U));                 \
      ((*(((CSS_t_USINT*)(pv_dst)) + 2U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU));             \
      ((*(((CSS_t_USINT*)(pv_dst)) + 3U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U))

  #else
    #define CSS_H2N_CPY32(pv_dst, pv_src)                                      \
      ((*(((CSS_t_USINT*)(pv_dst)) + 0U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU));             \
      ((*(((CSS_t_UINT*)(pv_dst)) + 1U)) =                                     \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U));                 \
      ((*(((CSS_t_USINT*)(pv_dst)) + 2U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU));             \
      ((*(((CSS_t_USINT*)(pv_dst)) + 3U)) =                                    \
        (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U))
  #endif
#endif


/*lint -esym(755, CSS_H2N_CPY)                    not referenced in every cfg */
/*******************************************************************************
**
** Function    : CSS_H2N_CPY
**
** Description : This function copies a byte stream from the host processor to
**               the network
**
** Parameters  : pv_dst (IN) - destination pointer (not checked),
**                             valid range: != CSS_k_NULL
**               pv_src (IN) - source pointer (not checked),
**                             valid range: != CSS_k_NULL
**               len (IN)    - length of the data to be copied (not checked),
**                             valid range: any value allowed
**
** Returnvalue : -
**
*******************************************************************************/
#define CSS_H2N_CPY(p_dst, p_src, len) CSS_MEMCPY((p_dst), (p_src), (len))


/*******************************************************************************
**
** Function    : SAPL_CssProduceEvenBytesClbk
**
** Description : This callback function must be implemented by the application.
**               It is called by CSS to identify if this Safety Controller
**               produces the even or the odd bytes.
**
** Parameters  : -
**
** Returnvalue : CSS_k_FALSE - Safety Controller produces the odd bytes
**               CSS_k_TRUE  - Safety Controller produces the even bytes
**
*******************************************************************************/
#if (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_HOMOGENEOUS_MIX)
  CSS_t_BOOL SAPL_CssProduceEvenBytesClbk(void);
#endif


#endif /* #ifndef CSOS_API_H */


/*** End Of File ***/


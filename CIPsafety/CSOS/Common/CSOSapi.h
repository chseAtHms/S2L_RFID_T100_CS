/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSOSapi.h
**    Summary: CSOS API
**             General definitions for usage of the CSOS Stack e.g. CIP Service
**             Codes, Status Codes, etc..
**             Also the identifiers for the units of the CSOS project are
**             defined here.
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

#ifndef CSOS_API_H
#define CSOS_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSOS_k_MAX_CONSUMER_NUM_xxx:
    This constant represents the maximum number of consumers that a producer can
    have.
*/
/*lint -esym(755, COSS_k_MAX_CONSUMER_NUM_SCAST)  not referenced in every cfg */
#define CSOS_k_MAX_CONSUMER_NUM_MCAST  15U
#define CSOS_k_MAX_CONSUMER_NUM_SCAST  1U


/** CSOS_k_IO_MSGLEN_xxx:
    These constants represent the lengths of the different IO messages.
*/
/*lint -esym(755, CSOS_k_IO_MSGLEN_TCOO)          not referenced in every cfg */
#define CSOS_k_IO_MSGLEN_SHORT_OVHD 6U /* Short Msgs has this much overhead */
#define CSOS_k_IO_MSGLEN_LONG_OVHD  8U /* Long Msgs has this much overhead */
#define CSOS_k_IO_MSGLEN_SHORT_MIN  7U /* 6 Bytes overhead + 1 Byte payload */
#define CSOS_k_IO_MSGLEN_SHORT_MAX  8U /* 6 Bytes overhead + 2 Byte payload */
#define CSOS_k_IO_MSGLEN_LONG_MIN  14U /* 8 Bytes overhead + 2*3 Byte payload */
#define CSOS_k_IO_MSGLEN_LONG_MAX   ((2U * CSOS_cfg_LONG_FORMAT_MAX_LENGTH) + \
                                     CSOS_k_IO_MSGLEN_LONG_OVHD)
#define CSOS_k_IO_MSGLEN_TCORR      6U /* Time Correction Msg has fixed len */
#define CSOS_k_IO_MSGLEN_TCOO       6U /* Time Coordination Msg has fixed len */


/** CSOS_k_CSS_IXxxx, CSOS_k_CSAL_IXxxx:
    Definition of the unit identifiers. These identifiers make up a part
    of the error codes. Thus the error unit can easily refer to the unit
    that has caused an error.
    The field in the error code that encodes the unit identifier is just 5 bits.
    Thus the valid range for unit identifiers is from 0x00 to 0x1F.
*/
/* units of CSS */
#define CSOS_k_CSS_IXSSC   0x01    /* safety stack control */
#define CSOS_k_CSS_IXSAI   0x02    /* safety assembly object interface */
#define CSOS_k_CSS_IXSCE   0x03    /* safety validator connection */
                                   /* establishment engine */
#define CSOS_k_CSS_IXSMR   0x04    /* safety message router object */
#define CSOS_k_CSS_IXSSO   0x05    /* safety supervisor object */
#define CSOS_k_CSS_IXSVO   0x06    /* safety validator object */
#define CSOS_k_CSS_IXSVC   0x07    /* safety validator client */
#define CSOS_k_CSS_IXSVS   0x08    /* safety validator server */
#define CSOS_k_CSS_IXCRC   0x09    /* cyclic redundancy check */
#define CSOS_k_CSS_IXSVD   0x0A    /* safety validator data */
#define CSOS_k_CSS_IXSER   0x0B    /* safety error unit */
#define CSOS_k_CSS_IXSFA   0x0C    /* safety frame assembling unit */
#define CSOS_k_CSS_IXUTL   0x0D    /* utility unit */
#define CSOS_k_CSS_IXEPP   0x0E    /* EPATH parsing (CSS) */
#define CSOS_k_CSS_IXSCF   0x0F    /* safety control flow monitoring unit */
#define CSOS_k_CSS_IXSSS   0x10    /* safety stack status */
#define CSOS_k_CSS_IXCCO   0x11    /* connection configuration object */
#define CSOS_k_CSS_IXSPI   0x12    /* safety process image */
/* units of CSAL */
#define CSOS_k_CSAL_IXALS  0x13    /* adaptation Layer Status */
#define CSOS_k_CSAL_IXERR  0x14    /* error unit */
#define CSOS_k_CSAL_IXALC  0x15    /* adaptation layer control */
#define CSOS_k_CSAL_IXMRO  0x16    /* message router object */
#define CSOS_k_CSAL_IXIDO  0x17    /* identity object */
#define CSOS_k_CSAL_IXCMO  0x18    /* connection manager object */
#define CSOS_k_CSAL_IXCO   0x19    /* connection object */
#define CSOS_k_CSAL_IXEPA  0x1A    /* EPATH parsing (CSAL) */
#define CSOS_k_CSAL_IXEMH  0x1B    /* explicit message handler */
#define CSOS_k_CSAL_IXEMS  0x1C    /* explicit message server */
#define CSOS_k_CSAL_IXEMC  0x1D    /* explicit message client */
#define CSOS_k_CSAL_IXLOS  0x1E    /* Link object */
#define CSOS_k_CSAL_IXHCH  0x1F    /* HALC Command Handle */


/** CSOS_k_CCSC_xxx
    Definition of the CIP Common Service Codes according to Volume 1 Appendix A
*/
#define CSOS_k_CCSC_GET_ATTR_ALL         0x01U  /* Get_Attributes_All */
#define CSOS_k_CCSC_SET_ATTR_ALL         0x02U  /* Set_Attributes_All */
#define CSOS_k_CCSC_GET_ATTR_LIST        0x03U  /* Get_Attribute_List */
#define CSOS_k_CCSC_SET_ATTR_LIST        0x04U  /* Set_Attribute_List */
#define CSOS_k_CCSC_RESET                0x05U  /* Reset */
#define CSOS_k_CCSC_START                0x06U  /* Start */
#define CSOS_k_CCSC_STOP                 0x07U  /* Stop */
#define CSOS_k_CCSC_CREATE               0x08U  /* Create */
#define CSOS_k_CCSC_DELETE               0x09U  /* Delete */
                                   /* Multiple_Service_Packet - not supported */
/* #define CSOS_k_CCSC_MULTIPLE_SERVICE  0x0AU */
#define CSOS_k_CCSC_APPLY_ATTR           0x0DU  /* Apply_Attributes */
#define CSOS_k_CCSC_GET_ATTR_SINGLE      0x0EU  /* Get_Attribute_Sinlge */
#define CSOS_k_CCSC_SET_ATTR_SINGLE      0x10U  /* Set_Attribute_Single */
#define CSOS_k_CCSC_FIND_NEXT_OBJ_INST   0x11U  /* Find_Next_Object_Instance */
#define CSOS_k_CCSC_RESTORE              0x15U  /* Restore */
#define CSOS_k_CCSC_SAVE                 0x16U  /* Save */
#define CSOS_k_CCSC_NOP                  0x17U  /* No Operation */
#define CSOS_k_CCSC_GET_MEMBER           0x18U  /* Get_Member */
#define CSOS_k_CCSC_SET_MEMBER           0x19U  /* Set_Member */
#define CSOS_k_CCSC_INSERT_MEMBER        0x1AU  /* Insert_Member */
#define CSOS_k_CCSC_REMOVE_MEMBER        0x1BU  /* Remove_Member */
#define CSOS_k_CCSC_GROUP_SYNC           0x1CU  /* GroupSync */
/* CSOS specific value for initialization of variables  */
#define CSOS_k_CCSC_INVALID_SERVICE      0xFFU /* invalid srvc code value */

/** CSOS_k_BIT_REPLY_SERVICE_CODE:
    This define is used to generate the Reply Service Code from the CIP Common
    Service Codes {CSOS_k_CCSC_xxx}.
*/
#define CSOS_k_BIT_REPLY_SERVICE_CODE    0x80U


/** CSOS_k_CGSC_xxx:
    Definition of the CIP General Status Codes according to Volume 1 Appendix B
*/
#define CSOS_k_CGSC_SUCCESS              0x00U  /* Success */
#define CSOS_k_CGSC_CNXN_FAILURE         0x01U  /* Connection failure */
#define CSOS_k_CGSC_RES_UNAVAIL          0x02U  /* Resource unavailable */
#define CSOS_k_CGSC_INVALID_PARAM_VALUE  0x03U  /* Invalid parameter value */
#define CSOS_k_CGSC_PATH_SEG_ERROR       0x04U  /* Path segment error */
#define CSOS_k_CGSC_PATH_DST_UNKNOWN     0x05U  /* Path destination unknown */
#define CSOS_k_CGSC_SERVICE_NOT_SUP      0x08U  /* Service not supported */
#define CSOS_k_CGSC_INVALID_ATTR_VAL     0x09U  /* Invalid attr value */
#define CSOS_k_CGSC_ALREADY_IN_REQ_MODE  0x0BU  /* Already in requested mode */
#define CSOS_k_CGSC_OBJ_STATE_CONFLICT   0x0CU  /* Object state conflict */
#define CSOS_k_CGSC_OBJ_ALREADY_EXISTS   0x0DU  /* Object already exists */
#define CSOS_k_CGSC_ATTR_NOT_SETTABLE    0x0EU  /* Attribute not settable */
#define CSOS_k_CGSC_PRIVILEGE_VIOLATION  0x0FU  /* Privilege violation */
#define CSOS_k_CGSC_DEV_STATE_CONFLICT   0x10U  /* Device state conflict */
#define CSOS_k_CGSC_REPLY_DATA_TOO_LARGE 0x11U  /* Reply data too large */
#define CSOS_k_CGSC_NOT_ENOUGH_DATA      0x13U  /* Not enough data */
#define CSOS_k_CGSC_ATTR_NOT_SUP         0x14U  /* Attribute not supported */
#define CSOS_k_CGSC_TOO_MUCH_DATA        0x15U  /* Too much data */
#define CSOS_k_CGSC_OBJ_DOES_NOT_EXIST   0x16U  /* Object does not exist */
#define CSOS_k_CGSC_NO_STORED_ATTR_DATA  0x18U  /* No stored attribute data */
#define CSOS_k_CGSC_STORE_OP_FAILURE     0x19U  /* Store operation failure */
#define CSOS_k_CGSC_VENDOR_SPEC_ERROR    0x1FU  /* Vendor specific error */
#define CSOS_k_CGSC_INVALID_PARAM        0x20U  /* Invalid parameter */
#define CSOS_k_CGSC_KEY_FAILURE_IN_PATH  0x25U  /* Key Failure in path */
#define CSOS_k_CGSC_PATH_SIZE_INVALID    0x26U  /* Path Size Invalid */
#define CSOS_k_CGSC_MEMBER_NOT_SETTABLE  0x29U  /* Member not settable */
/* CSOS specific value for initialization of variables  */
#define CSOS_k_CGSC_INVALID_STATUS_CODE  0xFFU  /* invalid status code value */


/** CSOS_k_SNS_FORMAT_xxx:
    Safety Network Segment Format definition according to CIP Networks Library
    Volume 5 Appendix C.
*/
#define CSOS_k_SNS_FORMAT_TARGET_BASE    0U     /* Target Format (Base) */
/* #define CSOS_k_SNS_FORMAT_ROUTER      1U     Router Format (Base/Extended) */
#define CSOS_k_SNS_FORMAT_TARGET_EXT     2U     /* Extended Format (EF) */
/* all other formats are reserved by CIP */

/** CSOS_k_SIZE_NET_SEG_DATA_xxx:
    Network Segment Safety Data Size (number of WORDs) definition according to
    CIP Networks Library Volume 5 Appendix C.
*/
#define CSOS_k_SIZE_NET_SEG_DATA_BASE    27U    /* number of WORDs */
#define CSOS_k_SIZE_NET_SEG_DATA_EXT     30U    /* number of WORDs */

/** CSOS_k_CCC_xxx:
    Definition of the CIP Class Codes according to Volume 1 Chapter 5-1
*/
#define CSOS_k_CCC_IDENTITY_OBJ          0x0001U  /* Identity Object */
#define CSOS_k_CCC_MSG_ROUTER_OBJ        0x0002U  /* Message Router Object */
#define CSOS_k_CCC_ASSEMBLY_OBJ          0x0004U  /* Assembly Object */
#define CSOS_k_CCC_CNXN_OBJ              0x0005U  /* Connection Object */
#define CSOS_k_CCC_CNXN_MGR_OBJ          0x0006U  /* Connection Manager Obj */
#define CSOS_k_CCC_S_SUPERVISOR_OBJ      0x0039U  /* Safety Supervisor Object */
#define CSOS_k_CCC_S_VALIDATOR_OBJ       0x003AU  /* Safety Validator Object */
#define CSOS_k_CCC_SERCOS_III_OBJ        0x004CU  /* Sercos III Link Ojbect */
#define CSOS_k_CCC_CNXN_CFG_OBJ          0x00F3U  /* Connection Config Object */
/* CSOS specific value for initialization of variables  */
#define CSOS_k_CCC_INVALID_CLASS         0xFFFFU  /* invalid Class code value */

/** CSOS_k_CCCA_xxx:
    Definition of the CIP Common Class Attributes according to Volume 1 Chapter
    4-4.1
*/
#define CSOS_k_CCCA_REVISION             1U  /* Revision */
#define CSOS_k_CCCA_MAX_INST             2U  /* Max Instance */
#define CSOS_k_CCCA_NUM_OF_INST          3U  /* Number of Instances */
#define CSOS_k_CCCA_OPT_ATTR_LIST        4U  /* Optional Attribute list */
#define CSOS_k_CCCA_OPT_SRVC_LIST        5U  /* Optional Service list */
#define CSOS_k_CCCA_MAX_CLASS_ATTR       6U  /* Maximum Number Class Attrs */
#define CSOS_k_CCCA_MAX_INST_ATTR        7U  /* Maximum Number Instance Attrs */


/* CSOS specific value for initialization of variables  */
#define CSOS_k_INVALID_INSTANCE          0xFFFFU /* invalid Instance ID */
#define CSOS_k_INVALID_INSTANCE_32       0xFFFFFFFFU  /* inv Inst ID (32 bit) */
#define CSOS_k_INVALID_ATTRIBUTE         0xFFFFU /* invalid Attribute ID */
#define CSOS_k_INVALID_MEMBER_32         0xFFFFFFFFU /* invalid Member ID */
#define CSOS_k_INVALID_ADD_STATUS        0xFFFFU /* invalid Additional Status */
#define CSOS_k_CNUM_INVALID              0xFFU   /* invalid consumer number */

/** CSOS_k_CLASS_DESIGNATOR:
    This define specifies the instance ID that addresses the Class - not its
    instances.
*/
#define CSOS_k_CLASS_DESIGNATOR          0U  /* class designator (inst ID 0) */


/** CSOS_k_INVALID_IDX:
    This define specifies an invalid value for several variables that refer to
    array indexes.
*/
#define CSOS_k_INVALID_IDX               0xFFFFU  /* invalid index */


/** CSOS_k_EXPL_RESP_HDR_MAX_SIZE:
    This define determines the maximum size of the Message Router Response
    Format header of an explicit response message.
    Header consists of: Reply Service, Reserved octet, General Status and
    Size of Additional Status (see Volume 1 Edition 3.10 Section 2-4.2)
*/
#define CSOS_k_EXPL_RESP_HDR_MAX_SIZE    ((4U * CSOS_k_SIZEOF_USINT) +  \
                                         (CSOS_cfg_ADD_STAT_SIZE_MAX    \
                                         * CSOS_k_SIZEOF_WORD))


/** CSOS_k_SIZEOF_xxx
    Determine the data length of variables of the respective type in a data
    packet on the CIP network (number of bytes).
*/
#define CSOS_k_SIZEOF_BOOL               1U  /* size of Boolean */
#define CSOS_k_SIZEOF_SINT               1U  /* size of Short Integer */
#define CSOS_k_SIZEOF_INT                2U  /* size of Integer */
#define CSOS_k_SIZEOF_DINT               4U  /* size of Double Integer */
#define CSOS_k_SIZEOF_USINT              1U  /* size of Unsigned Short Int */
#define CSOS_k_SIZEOF_UINT               2U  /* size of Unsigned Integer */
#define CSOS_k_SIZEOF_UDINT              4U  /* size of Unsigned double Int */
#define CSOS_k_SIZEOF_BYTE               1U  /* size of bit string - 8 bits */
#define CSOS_k_SIZEOF_WORD               2U  /* size of bit string - 16 bits */
#define CSOS_k_SIZEOF_DWORD              4U  /* size of bit string - 32 bits */
/* size of a Unique Network Identifier */
#define CSOS_k_SIZEOF_UNID               (CSOS_k_SIZEOF_UDINT + \
                                          CSOS_k_SIZEOF_UINT +  \
                                          CSOS_k_SIZEOF_UDINT)
/* size of a Safety Configuration Identifier */
#define CSOS_k_SIZEOF_SCID               (CSOS_k_SIZEOF_UDINT + \
                                          CSOS_k_SIZEOF_UDINT + \
                                          CSOS_k_SIZEOF_UINT)

/** CSOS_k_MAX_xxx:
    Maximum data value that can be stored in the respective data type.
*/
#define CSOS_k_MAX_USINT                 255U    /* max Unsigned Short Int */
#define CSOS_k_MAX_UINT                  65535U  /* max Unsigned Integer */
#define CSOS_k_MAX_INT                   32767   /* max Integer */


/** CSOS_k_SEG_TYPE_xxx:
    These defines are used to encode the segment Type.

     Segment Type/Format byte encoding:
      - Bit 0..4:  Segment Format
      - Bit 5..7:  Segment Type (see table below)

      +--------------+----------------+------------------+
      | Segment Type | Segment Format | Segment name     |
      +--------------+----------------+------------------+
      |      000     |       XXXXX    | Port segment     |
      +--------------+----------------+------------------+
      |      001     |       XXXXX    | Logical segment  |
      +--------------+----------------+------------------+
      |      100     |       XXXXX    | Data segment     |
      +--------------+----------------+------------------+
      |      010     |       XXXXX    | Network segment  |
      +--------------+----------------+------------------+
      .
*/
#define CSOS_k_SEG_TYPE_PORT            0x00U  /* Port segment    000X | XXXX */
#define CSOS_k_SEG_TYPE_LOGICAL         0x20U  /* Logical segment 001X | XXXX */
#define CSOS_k_SEG_TYPE_DATA            0x80U  /* Data segment    100X | XXXX */
#define CSOS_k_SEG_TYPE_NETWORK         0x40U  /* Network segment 010X | XXXX */


/** CSOS_SEG_TYPE_GET:
    Returns the segment type from the passed byte containing a segment
    descriptor.
*/
#define CSOS_SEG_TYPE_GET(u8_segTypeFormat)         (u8_segTypeFormat & 0xE0U)


/** CSOS_k_LOG_SEG_xxx:
    These defines are used to encode the segment Format if the segment type is
    logical segment (CSOS_k_SEG_TYPE_LOGICAL).

     Segment Type/Format byte encoding:
      - Bit 0..1:  Logical Format (see below)
      - Bit 2..4:  Logical Type (see below)
      - Bit 5..7:  Segment Type
        (001 == CSOS_k_SEG_TYPE_LOGICAL == Logical Segment)

      +----------------+-----------------------+
      | Logical Format |                       |
      +----------------+-----------------------+
      |     00         |8-bit logical address  |
      +----------------+-----------------------+
      |     01         |16-bit logical address |
      +----------------+-----------------------+
      |     10         |32-bit logical address |
      +----------------+-----------------------+
      |     11         |Reserved               |
      +----------------+-----------------------+
      .

      +----------------+-----------------------+
      | Logical Type   |                       |
      +----------------+-----------------------+
      |     000        |Class ID               |
      +----------------+-----------------------+
      |     001        |Instance ID            |
      +----------------+-----------------------+
      |     010        |Member ID              |
      +----------------+-----------------------+
      |     011        |Connection Point       |
      +----------------+-----------------------+
      |     100        |Attribute ID           |
      +----------------+-----------------------+
      |     others     |not supported          |
      +----------------+-----------------------+
      .
      According to Volume 1 not all combinations of Logical Format and
      Logical Type are allowed (particularly the 32-bit formats)!
*/
#define CSOS_k_LOG_SEG_CLASS_ID_8       0x20U /* 001 | 000 | 00 */
#define CSOS_k_LOG_SEG_CLASS_ID_16      0x21U /* 001 | 000 | 01 */
#define CSOS_k_LOG_SEG_INSTANCE_ID_8    0x24U /* 001 | 001 | 00 */
#define CSOS_k_LOG_SEG_INSTANCE_ID_16   0x25U /* 001 | 001 | 01 */
#define CSOS_k_LOG_SEG_INSTANCE_ID_32   0x26U /* 001 | 001 | 10 */
#define CSOS_k_LOG_SEG_MEMBER_ID_8      0x28U /* 001 | 010 | 00 */
#define CSOS_k_LOG_SEG_MEMBER_ID_16     0x29U /* 001 | 010 | 01 */
#define CSOS_k_LOG_SEG_MEMBER_ID_32     0x2AU /* 001 | 010 | 10 */
#define CSOS_k_LOG_SEG_CNXN_POINT_8     0x2CU /* 001 | 011 | 00 */
#define CSOS_k_LOG_SEG_CNXN_POINT_16    0x2DU /* 001 | 011 | 01 */
#define CSOS_k_LOG_SEG_CNXN_POINT_32    0x2EU /* 001 | 011 | 10 */
#define CSOS_k_LOG_SEG_ATTRIBUTE_ID_8   0x30U /* 001 | 100 | 00 */
#define CSOS_k_LOG_SEG_ATTRIBUTE_ID_16  0x31U /* 001 | 100 | 01 */
#define CSOS_k_LOG_SEG_ELECTRONIC_KEY   0x34U /* 001 | 101 | 00 */

/** CSOS_k_KEY_FORMAT_TABLE:
    This symbol represents the Key Format in a Logical the Electronic Key
    Segment.
*/
#define CSOS_k_KEY_FORMAT_TABLE         0x04U /* Key Format Table */

/** CSOS_k_DATA_SEG_SIMPLE:
    This symbol represents a Simple Data Segment
*/
#define CSOS_k_DATA_SEG_SIMPLE          (CSOS_k_SEG_TYPE_DATA | 0x00U)

/** CSOS_k_NET_SEG_SAFETY:
    This symbol represents a Safety Network Segment
*/
#define CSOS_k_NET_SEG_SAFETY           (CSOS_k_SEG_TYPE_NETWORK | 0x10U)


/*******************************************************************************
**    HALC Command Codes
*******************************************************************************/

/** CSOS_UNIT_HANDLE_GET:
    This macro is used to decide which unit has to process the received command.
*/
#define CSOS_UNIT_HANDLE_GET(u16_unitHdl) (u16_unitHdl & 0x00FFU)


/** Unit handle shift macro definitions:
    This macro assembles a HALC command code to shift the unit handles for the
    Command definitions ({CSOS_k_CMD_xxx}).
*/
/*lint -emacro( (960), CSOS_HalcCmdAsm )
  (Note -- Violates MISRA 2004 Required Rule 10.5, Operators '~' and '<<'
           require recasting to underlying type for sub-integers)
  (Note -- Violates MISRA 2004 Required Rule 12.7, Bitwise operator applied to
           signed underlying type: |)
  (Note -- Violates MISRA 2004 Required Rule 10.1, Prohibited Implicit
           Conversion: Signed versus Unsigned)
*/
#define CSOS_HalcCmdAsm(UnitHdl, CmdIdx)  (((CmdIdx) << 8) | (UnitHdl))


/** CSOS_k_APL:
    This define is used as the unit ID for commands that are directed to the
    application instead of CSAL or CSS.
*/
#define CSOS_k_APL                       0x00U


/** CSOS_k_OFS_xxx:
    These defines specify the offsets of information added by CSAL/CSS within
    an HALCS_t_MSG in the data section (referenced by pb_data) for the
    CSOS_k_CMD_IXSCE_SOPEN_RES and CSOS_k_CMD_IXSCE_SCLOSE_RES commands.
*/
#define CSOS_k_OFS_SV_INST_ID            0U  /* Safety Validator Instance ID */
#define CSOS_k_OFS_CONS_NUM              2U  /* Consumer Number */
#define CSOS_k_OFS_CNXN_TYPE             3U  /* Connection Type */
#define CSOS_k_OFS_OPEN_CNXN_POINT       4U  /* Connection Point */
#define CSOS_k_OFS_OPEN_MRR_RESP_DATA    6U  /* Response Data (open command) */
#define CSOS_k_OFS_CLOSE_MRR_RESP_DATA   4U  /* Response Data (close command) */
/* size of the SafetyOpen Response header data */
#define CSOS_k_SIZEOF_OPEN_CNXN_HDR      CSOS_k_OFS_OPEN_MRR_RESP_DATA
/* size of the SafetyClose Response header data */
#define CSOS_k_SIZEOF_CLOSE_CNXN_HDR     CSOS_k_OFS_CLOSE_MRR_RESP_DATA


/** CSOS_k_CNXN_xxx:
    These defines specify the type of a connection as needed by CSAL to handle
    the data of a connection.
*/
#define CSOS_k_CNXN_CONS_SINGLE          0x00U   /* Single-cast consumer */
#define CSOS_k_CNXN_CONS_MULTI           0x03U   /* Multi-cast consumer */
#define CSOS_k_CNXN_PROD_SINGLE          0x0CU   /* Single-cast producer */
#define CSOS_k_CNXN_PROD_MULTI           0x0FU   /* Multi-cast producer */
#define CSOS_k_CNXN_FIELD_NOT_USED       0xFFU   /* invalid value */


/** CSOS_k_CMD_xxx:
    Command definitions:
    These defines represent the commands that pass the HALC layer.
    The source Unit is encoded in the name of the command whereas the
    destination unit is encoded in the value.
*/
#define CSOS_k_CMD_IXCO_IO_DATA        CSOS_HalcCmdAsm(CSOS_k_CSS_IXSVO,  0x01U)
#define CSOS_k_CMD_IXSVO_IO_DATA       CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCO,  0x02U)
#define CSOS_k_CMD_IXCMO_SOPEN_REQ     CSOS_HalcCmdAsm(CSOS_k_CSS_IXSCE,  0x11U)
#define CSOS_k_CMD_IXSCE_SOPEN_RES     CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x12U)
#define CSOS_k_CMD_IXCMO_SCLOSE_REQ    CSOS_HalcCmdAsm(CSOS_k_CSS_IXSCE,  0x13U)
#define CSOS_k_CMD_IXSCE_SCLOSE_RES    CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x14U)
#define CSOS_k_CMD_IXCCO_SOPEN1_REQ    CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x21U)
#define CSOS_k_CMD_IXCCO_SOPEN2_REQ    CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x22U)
#define CSOS_k_CMD_IXCCO_SCLOSE_REQ    CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x23U)
#define CSOS_k_CMD_IXCMO_SOPEN_RES     CSOS_HalcCmdAsm(CSOS_k_CSS_IXCCO,  0x31U)
#define CSOS_k_CMD_IXCMO_SCLOSE_RES    CSOS_HalcCmdAsm(CSOS_k_CSS_IXCCO,  0x32U)
#define CSOS_k_CMD_IXSCE_CNXN_OPEN     CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x41U)
#define CSOS_k_CMD_IXSCE_CNXN_CLOSE    CSOS_HalcCmdAsm(CSOS_k_CSAL_IXCMO, 0x42U)
/* CSOS_k_CMD_IXMRO_EXPL_REQ is used by IXCMO on modular devices */
#define CSOS_k_CMD_IXMRO_EXPL_REQ      CSOS_HalcCmdAsm(CSOS_k_CSS_IXSMR,  0x51U)
#define CSOS_k_CMD_IXSMR_EXPL_RES      CSOS_HalcCmdAsm(CSOS_k_CSAL_IXMRO, 0x52U)
#define CSOS_k_CMD_IXSMR_EXPL_RES_INS  CSOS_HalcCmdAsm(CSOS_k_CSAL_IXMRO, 0x53U)
#define CSOS_k_CMD_IXSSO_DEV_STATE     CSOS_HalcCmdAsm(CSOS_k_CSAL_IXIDO, 0x61U)
#define CSOS_k_CMD_IXSSO_SNN           CSOS_HalcCmdAsm(CSOS_k_CSAL_IXLOS, 0x62U)
#define CSOS_k_CMD_IXSSC_TERMINATE     CSOS_HalcCmdAsm(CSOS_k_CSAL_IXALC, 0x81U)


/*******************************************************************************
**    data types
*******************************************************************************/

/** CSOS_t_SSO_DEV_STATUS:
    Device Status Attribute State Values of the Safety Supervisor Object.
*/
typedef enum
{
  CSOS_k_SSO_DS_UNDEFINED    = 0x00U,   /* Undefined */
  CSOS_k_SSO_DS_SELF_TESTING = 0x01U,   /* Self-Testing */
  CSOS_k_SSO_DS_IDLE         = 0x02U,   /* Idle */
  CSOS_k_SSO_DS_SELF_TEST_EX = 0x03U,   /* Self-Test Exception */
  CSOS_k_SSO_DS_EXECUTING    = 0x04U,   /* Executing */
  CSOS_k_SSO_DS_ABORT        = 0x05U,   /* Abort */
  CSOS_k_SSO_DS_CRIT_FAULT   = 0x06U,   /* Critical Fault */
  CSOS_k_SSO_DS_CONFIGURING  = 0x07U,   /* Configuring */
  CSOS_k_SSO_DS_WAIT_TUNID   = 0x08U    /* Waiting for TUNID */
  /* 9   - 50 */                        /* Reserved by CIP */
  /* 51  - 99 */                        /* Device Specific */
  /* 100 - 255 */                       /* Vendor Specific */
} CSOS_t_SSO_DEV_STATUS;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/


#endif /* CSOS_API_H */

/*** End of File ***/


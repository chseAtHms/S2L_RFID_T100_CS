/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXEPP.h
**    Summary: IXEPP - Encoded Path Parser
**             IXEPP.h is the export header file of the IXEPP unit.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXEPP_EPathParse
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXEPP_H
#define IXEPP_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXEPP_k_SFB_xxx:
    EPATH valid bits: Bit masks for indication of which segments have been found
    in an EPATH.
*/
#define IXEPP_k_SFB_NO_VALID_FIELD    0x00000000UL
#define IXEPP_k_SFB_CLASS_ID_1        0x00000001UL
#define IXEPP_k_SFB_INST_ID_1         0x00000002UL
#define IXEPP_k_SFB_ATTR_ID           0x00000004UL
#define IXEPP_k_SFB_MEMB_ID           0x00000008UL
#define IXEPP_k_SFB_ELECTRONIC_KEY    0x00000010UL
#define IXEPP_k_SFB_CNXN_POINT_1      0x00000020UL
#define IXEPP_k_SFB_CNXN_POINT_2      0x00000040UL
#define IXEPP_k_SFB_DATA_SEG          0x00000080UL
#define IXEPP_k_SFB_SAFETY_NTWK_SEG   0x00000100UL
#define IXEPP_k_SFB_CLASS_ID_2        0x00000200UL
#define IXEPP_k_SFB_INST_ID_2         0x00000400UL
#define IXEPP_k_SFB_CLASS_ID_3        0x00000800UL
#define IXEPP_k_SFB_INST_ID_3         0x00001000UL


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXEPP_t_FIELD_LOG_SEG:
    Structure type for storing logical segments.
*/
typedef struct
{
  /* Class IDs */
  CSS_t_UINT  u16_classId1;       /* 1st Class ID */
  CSS_t_UINT  u16_classId2;       /* 2nd Class ID */
  CSS_t_UINT  u16_classId3;       /* 3rd Class ID */

  CSS_t_UINT  u16_attrId;         /* Attribute ID */
  CSS_t_UDINT u32_membId;         /* Member ID */

  /* Instance IDs */
  CSS_t_UDINT u32_instId1;        /* 1st Instance ID */
  CSS_t_UDINT u32_instId2;        /* 2nd Instance ID */
  CSS_t_UDINT u32_instId3;        /* 3rd Instance ID */
  /* Connection points */
  CSS_t_UDINT u32_cpId1;          /* 1st connection point */
  CSS_t_UDINT u32_cpId2;          /* 2nd connection point */

  const CSS_t_BYTE *pba_elKey;    /* pointer to Electronic Key */
} IXEPP_t_FIELD_LOG_SEG;

/** IXEPP_t_FIELD_DATA_SEG:
    Structure type for storing a data segment.
*/
typedef struct
{
  const CSS_t_BYTE *pba_cfgData;  /* pointer to Configuration Data */
  CSS_t_USINT u8_cfgSizeW;        /* number of Words in Data Segment */
} IXEPP_t_FIELD_DATA_SEG;

/** IXEPP_t_FIELD_NET_SEG:
    Structure type for storing a network segment.
*/
typedef struct
{
  const CSS_t_BYTE *pba_nsd;      /* pointer to Network Segment Safety */
  CSS_t_USINT u8_nsdSize;         /* number of Words in Net Segment Safety */
} IXEPP_t_FIELD_NET_SEG;


/** IXEPP_t_FIELDS:
    Structure type for storing the results of parsing an EPATH.
*/
typedef struct
{
  IXEPP_t_FIELD_LOG_SEG s_logSeg;      /* logical segment */

  IXEPP_t_FIELD_DATA_SEG s_dataSeg;    /* data segment */
  IXEPP_t_FIELD_NET_SEG s_netSeg;      /* network segment */

  CSS_t_UINT u16_elKeyAndAppPathSize;  /* number of Bytes of Electronic Key
                                          and Application Paths (= size of the
                                          logical segment) */
} IXEPP_t_FIELDS;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXEPP_EPathParse
**
** Description : This function parses the passed EPATH string. A bit field
**               indicates what segments have been found (and parsed) in the
**               EPATH and the parsed values are placed into a structure.
**
** Parameters  : pb_ePath (IN)          - pointer to the EPATH (checked,
**                                        valid range: <> CSS_k_NULL)
**               o_padded (IN)          - CSS_k_TRUE  : EPATH is padded
**                                        CSS_k_FALSE : EPATH is packed
**                                        (not checked, any value allowed)
**               u16_reqPathSize (IN)   - size of the Request_Path (number of
**                                        bytes)
**                                        (not checked, any value allowed)
**               pdw_valid (OUT)        - bit string to signal what has been
**                                        found in the EPATH. See the bit
**                                        defines {IXEPP_k_SFB_xxx}
**                                        (not checked, only called with
**                                        reference to variable)
**               ps_ePathFields (OUT)   - pointer to structure that contains the
**                                        parsed EPATH field values.
**                                        (not checked, only called with
**                                        reference to struct)
**
** Returnvalue : CSS_t_USINT            - CIP General Status code
**               ==CSOS_k_CGSC_SUCCESS  - success
**               !=CSOS_k_CGSC_SUCCESS  - protocol failure
**
*******************************************************************************/
CSS_t_USINT IXEPP_EPathParse(const CSS_t_BYTE *pb_ePath,
                             CSS_t_BOOL o_padded,
                             CSS_t_UINT u16_reqPathSize,
                             CSS_t_DWORD *pdw_valid,
                             IXEPP_t_FIELDS *ps_ePathFields);


#endif /* #ifndef IXEPP_H */

/*** End of File ***/


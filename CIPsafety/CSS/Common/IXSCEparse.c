/*******************************************************************************
**    Copyright (C) 2009-2019 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCEparse.c
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             This module is responsible for parsing Forward_Open and Forward
**             Close requests and storing the parameters into structures.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_FwdOpenParse
**             IXSCE_FwdCloseParse
**
**             FwdOpenParse2
**             NetSegmentSafetyParse
**             EpathSegmentCheck
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


/*******************************************************************************
**    include-files
*******************************************************************************/

#include "CSOScfg.h"
#include "CSScfg.h"

#include "CSOSapi.h"

#include "CSStypes.h"
#include "CSSplatform.h"
#include "CSSapi.h"
#include "CSS.h"
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  #include "HALCSapi.h"
#endif

#include "IXSERapi.h"

#include "IXEPP.h"
#include "IXSCF.h"

#include "IXSCE.h"
#include "IXSCEint.h"
#include "IXSCEerr.h"


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    static constants, types, macros, variables
*******************************************************************************/

/** k_FWDO_FIELDS_SIZE:
    Byte length of the fixed size fields at the beginning of a Forward_Open
    Message. This includes all fields until Connection_Path_Size */
#define k_FWDO_FIELDS_SIZE       36U


/** k_FWDC_FIELDS_SIZE:
    Byte length of the fixed size fields at the beginning of a Forward_Close
    Message. This includes all fields until Connection_Path_Size */
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
  #define k_FWDC_FIELDS_SIZE     12U
#endif

/* Maximum length of a Forward_Open request. This contains the fixed size fields
   as defined above plus maximum Connection_Path. The Connection_Path is limited
   by the Connection_Path_Length field. The Connection_Path_Size is an 8 bit
   value specifying the path length in number of words.  */
#define k_FWDO_MAX_SIZE     (k_FWDO_FIELDS_SIZE + (2U * CSOS_k_MAX_USINT))

/* Relative byte offsets of the fields in a Network Segment Safety. */
/* First those fields that have equal offsets in Target and Extended Format */
#define k_OFS_NSD_SIZE            1U    /* Segment Size */
#define k_OFS_NSD_FORMAT          2U    /* Segment Format */
#define k_OFS_NSD_SCCRC           4U    /* Safety Config CRC (SCCRC) */
#define k_OFS_NSD_SCTS_TIME       8U    /* Safety Config Time Stamp (Time) */
#define k_OFS_NSD_SCTS_DATE      12U    /* Safety Config Time Stamp (Date) */
#define k_OFS_NSD_TCORR_EPI      14U    /* Time Correction EPI */
#define k_OFS_NSD_TCORR_NCP      18U    /* Time Correction Net Cnxn Params */
#define k_OFS_NSD_TUNID_SNN_TIME 20U    /* Target_UNID (TUNID) (SNN.Time) */
#define k_OFS_NSD_TUNID_SNN_DATE 24U    /* Target_UNID (TUNID) (SNN.Date) */
#define k_OFS_NSD_TUNID_NODEID   26U    /* Target_UNID (TUNID) (NodeID) */
#define k_OFS_NSD_OUNID_SNN_TIME 30U    /* Originator_UNID (OUNID) (SNN.Time) */
#define k_OFS_NSD_OUNID_SNN_DATE 34U    /* Originator_UNID (OUNID) (SNN.Date) */
#define k_OFS_NSD_OUNID_NODEID   36U    /* Originator_UNID (OUNID) (NodeID) */
#define k_OFS_NSD_PIEM           40U    /* Ping Interval EPI Multiplier */
#define k_OFS_NSD_TCMMM          42U    /* Time_Coord_Msg_Min_Multiplier */
#define k_OFS_NSD_NTEM           44U    /* Network_Time_Expectation_Multipl. */
#define k_OFS_NSD_TMUL           46U    /* Timeout_Multiplier */
#define k_OFS_NSD_MAX_CONS       47U    /* Max_Consumer_Number */

/* The following fields have special offsets in Target Format (Base Format) */
/*lint -esym(750, k_OFS_NSD_CPCRC_BASE)        not referenced in every config */
/*lint -esym(750, k_OFS_NSD_TCORR_CID_BASE)    not referenced in every config */
#define k_OFS_NSD_CPCRC_BASE     48U    /* Connection Parameters CRC (CPCRC) */
#define k_OFS_NSD_TCORR_CID_BASE 52U    /* Time Correction Connection ID */

/* The following fields have special offsets in Extended Format */
/*lint -esym(750, k_OFS_NSD_MAX_FAULT_EXT)     not referenced in every config */
/*lint -esym(750, k_OFS_NSD_CPCRC_EXT)         not referenced in every config */
/*lint -esym(750, k_OFS_NSD_TCORR_CID_EXT)     not referenced in every config */
/*lint -esym(750, k_OFS_NSD_INITIAL_TS_EXT)    not referenced in every config */
/*lint -esym(750, k_OFS_NSD_INITIAL_RV_EXT)    not referenced in every config */
#define k_OFS_NSD_MAX_FAULT_EXT  48U    /* Max_Fault_Number */
#define k_OFS_NSD_CPCRC_EXT      50U    /* Connection Parameters CRC (CPCRC) */
#define k_OFS_NSD_TCORR_CID_EXT  54U    /* Time Correction Connection ID */
#define k_OFS_NSD_INITIAL_TS_EXT 58U    /* Initial Time Stamp */
#define k_OFS_NSD_INITIAL_RV_EXT 60U    /* Initial Rollover Value */


/** k_SOPEN_COMPR:
    These segments must be present in a SafetyOpen with compressed application
    path.
*/
#define k_SOPEN_COMPR_REQ (IXEPP_k_SFB_ELECTRONIC_KEY  | \
                           IXEPP_k_SFB_SAFETY_NTWK_SEG | \
                           IXEPP_k_SFB_CLASS_ID_1      | \
                           IXEPP_k_SFB_INST_ID_1       | \
                           IXEPP_k_SFB_CNXN_POINT_1    | \
                           IXEPP_k_SFB_CNXN_POINT_2    )

/** k_SOPEN_LONG:
    These segments must be present in a SafetyOpen with long application path.
*/
#define k_SOPEN_LONG_REQ  (IXEPP_k_SFB_ELECTRONIC_KEY  | \
                           IXEPP_k_SFB_SAFETY_NTWK_SEG | \
                           IXEPP_k_SFB_CLASS_ID_1      | \
                           IXEPP_k_SFB_INST_ID_1       | \
                           IXEPP_k_SFB_CLASS_ID_2      | \
                           IXEPP_k_SFB_INST_ID_2       | \
                           IXEPP_k_SFB_CLASS_ID_3      | \
                           IXEPP_k_SFB_INST_ID_3       )

/** k_SOPEN_COMPR_INST_REQ:
    For Originators only: Support sending a path construct that is not
    mentioned in Volume 5: compressed path with instances (instead of
    connection points). (This is only supported to ease connecting to some 3rd
    party devices that use such non-standard paths) (see Req.5.3-20).
*/
#define k_SOPEN_COMPR_INST_REQ   (IXEPP_k_SFB_ELECTRONIC_KEY  | \
                                  IXEPP_k_SFB_SAFETY_NTWK_SEG | \
                                  IXEPP_k_SFB_CLASS_ID_1      | \
                                  IXEPP_k_SFB_INST_ID_1       | \
                                  IXEPP_k_SFB_INST_ID_2       | \
                                  IXEPP_k_SFB_INST_ID_3       )

/** k_SOPEN_OPT:
    These segments may optionally be present in a SafetyOpen application path.
*/
#define k_SOPEN_OPT        (IXEPP_k_SFB_DATA_SEG)


/*******************************************************************************
**    static function-prototypes
*******************************************************************************/

static CSS_t_WORD FwdOpenParse2(const IXEPP_t_FIELDS *ps_ePathFields,
                                CSS_t_UDINT dw_valid,
                                CSS_t_SOPEN_PARA *ps_sOpenPar,
                                IXSCE_t_SOPEN_AUX *ps_sOpenAuxData,
                                CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static CSS_t_WORD NetSegmentSafetyParse(const CSS_t_BYTE *pba_nsd,
                                        CSS_t_NET_SEG_SAFETY_T_FRMT *ps_nsd,
                                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);
static CSS_t_UINT EpathSegmentCheck(CSS_t_UDINT u32_testBits,
                                    CSS_t_UDINT u32_reqBits,
                                    CSS_t_UDINT u32_optBits);


/*******************************************************************************
**    global functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSCE_FwdOpenParse
**
** Description : This function parses the received Forward_Open request (byte
**               stream) and extracts the parameters into structures.
**
** Parameters  : pb_msgBuf (IN)         - pointer to the buffer containing the
**                                        data of the received HALC message
**                                        (valid range: <> CSS_k_NULL
**                                        not checked, checked in
**                                        IXSCE_CmdProcess())
**               u16_msgLen (IN)        - length of the data in the buffer
**                                        (checked, valid range:
**                                        k_FWDO_FIELDS_SIZE..k_FWDO_MAX_SIZE)
**               o_originator (IN)      - determines from which context this
**                                        function was called:
**                                        CSS_k_TRUE:  called from Originator
**                                                     (CCO Validate)
**                                        CSS_k_FALSE: called from Target
**                                                     (Forward_Open checking)
**                                        (not checked, only called with
**                                        constants)
**               ps_sOpenPar (OUT)      - pointer to the structure to where the
**                                        data of the received Forward_Open
**                                        request is to be stored. The caller
**                                        can also pass CSS_k_NULL if he is not
**                                        interested in this data.
**                                        (checked, CSS_k_NULL or !=CSS_k_NULL)
**               ps_sOpenAuxData (OUT)  - pointer to structure to where
**                                        additional information concerning the
**                                        received Forward_Open request is to be
**                                        stored
**                                        (not checked, only called with
**                                        reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_OK               - success
**               <>CSS_k_OK             - Extended Status Code to be returned in
**                                        Unsuccessful Forward_Open Response
**
*******************************************************************************/
CSS_t_WORD IXSCE_FwdOpenParse(const CSS_t_BYTE *pb_msgBuf,
                              CSS_t_UINT u16_msgLen,
                              CSS_t_BOOL o_originator,
                              CSS_t_SOPEN_PARA *ps_sOpenPar,
                              IXSCE_t_SOPEN_AUX *ps_sOpenAuxData,
                              CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);
  /* temporary variable for parsing structures */
  CSS_t_USINT u8_temp = 0U;
  /* temporary variable for length checking of the connection path */
  CSS_t_UINT u16_cnxnPathSize = 0U;
  /* structure type of parse function result */
  IXEPP_t_FIELDS  s_ePathFields;
  /* flags for storing EPATH parse result */
  CSS_t_UDINT dw_valid = IXEPP_k_SFB_NO_VALID_FIELD;

  /* EPATH field structure is cleared */
  CSS_MEMSET(&s_ePathFields, 0, sizeof(s_ePathFields));

  /* if length is too long */
  if (u16_msgLen > k_FWDO_MAX_SIZE)
  {
    ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_TOO_MUCH_DATA;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_TOO_LONG),
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_msgLen);
    w_retVal = CSS_k_OK;
  }
  /* else: if length is too short */
  else if (u16_msgLen < k_FWDO_FIELDS_SIZE)
  {
    /* received packet is too short to be a valid Forward_Open */
    ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_NOT_ENOUGH_DATA;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_TOO_SHORT),
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)u16_msgLen);
    w_retVal = CSS_k_OK;
  }
  else /* else: message has at least minimum length */
  {
    /* if caller doesn't want to have this data returned */
    if (ps_sOpenPar == CSS_k_NULL)
    {
      /* ps_sOpenPar indicates that the caller is not interested in this data */
    }
    else /* else: copy data from Forward_Open message */
    {
      /* Extract the common fields of a Forward_Open (without connection path */
      CSS_N2H_CPY32(&ps_sOpenPar->u32_netCnxnId_OT,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_NCNXNID_OT));
      CSS_N2H_CPY32(&ps_sOpenPar->u32_netCnxnId_TO,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_NCNXNID_TO));
      CSS_N2H_CPY16(&ps_sOpenPar->s_cnxnTriad.u16_cnxnSerNum,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_CNXN_S_NUM));
      CSS_N2H_CPY16(&ps_sOpenPar->s_cnxnTriad.u16_origVendId,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_ORIG_V_ID));
      CSS_N2H_CPY32(&ps_sOpenPar->s_cnxnTriad.u32_origSerNum,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_ORIG_S_NUM));
      CSS_N2H_CPY8(&ps_sOpenPar->u8_cnxnTimeoutMultiplier,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_CNXN_TMULT));
      /* 3 reserved bytes are following. Value not checked. */
      CSS_N2H_CPY32(&ps_sOpenPar->u32_rpiOT_us,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_RPI_OT));
      CSS_N2H_CPY16(&ps_sOpenPar->w_netCnxnPar_OT,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_NCNXN_PAR_OT));
      CSS_N2H_CPY32(&ps_sOpenPar->u32_rpiTO_us,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_RPI_TO));
      CSS_N2H_CPY16(&ps_sOpenPar->w_netCnxnPar_TO,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_NCNXN_PAR_TO));
      CSS_N2H_CPY8(&ps_sOpenPar->b_transportTypeTrigger,
                    CSS_ADD_OFFSET(pb_msgBuf,
                                   k_OFS_FWDO_TCLASS_TRIG));
    }

    CSS_N2H_CPY8(&u8_temp,
                 CSS_ADD_OFFSET(pb_msgBuf,
                                k_OFS_FWDO_CNXN_P_SIZE));

    /* convert from number of WORDs to number of BYTEs */
    u16_cnxnPathSize = (CSS_t_UINT)((CSS_t_UINT)u8_temp * 2U);

    /* if the remaining size of the Fwd_Open is larger than the Cnxn Path */
    if ((u16_msgLen - k_FWDO_FIELDS_SIZE) > u16_cnxnPathSize)
    {
      ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_TOO_MUCH_DATA;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_AP_SIZE_1),
                      (CSS_t_UINT)(u16_msgLen - k_FWDO_FIELDS_SIZE),
                      (CSS_t_UDINT)u16_cnxnPathSize);
      w_retVal = CSS_k_OK;
    }
    /* else if the remaining size of the Fwd_Open is less than the Cnxn Path */
    else if ((u16_msgLen - k_FWDO_FIELDS_SIZE) < u16_cnxnPathSize)
    {
      ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_NOT_ENOUGH_DATA;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_AP_SIZE_2),
                      (CSS_t_UINT)(u16_msgLen - k_FWDO_FIELDS_SIZE),
                      (CSS_t_UDINT)u16_cnxnPathSize);
      w_retVal = CSS_k_OK;
    }
    else /* else: Connection Path size is consistent */
    {
      /* Let IXEPP parse the Connection Path */
      ps_sOpenRespStat->u8_gen =
        IXEPP_EPathParse(CSS_ADD_OFFSET(pb_msgBuf,
                                        k_FWDO_FIELDS_SIZE),          /* path */
                         CSS_k_TRUE,                                /* padded */
                         u16_cnxnPathSize,      /* length of the Path (bytes) */
                         &dw_valid,             /* found segments */
                         &s_ePathFields);       /* struct to store result */

      /* if previous function returned an error */
      if (ps_sOpenRespStat->u8_gen != CSOS_k_CGSC_SUCCESS)
      {
        /* EPATH parse function reported an error */
        /* if the error indicates Path/Segment Error */
        if (ps_sOpenRespStat->u8_gen == CSOS_k_CGSC_PATH_SEG_ERROR)
        {
          /* inside a Connection path of Forward_Open general parsing errors
             (e.g. invalid or unexpected segments found) shall be indicated
             with General Status 1 and Extended Status 0x315 */
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SEGMENT;
        }
        else /* it is some other error */
        {
          /* general status code already set */
          /* make sure extended status code is not set */
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
        }

        /* abort processing */
        /* EPP has already reported a precise error info to the application */
        w_retVal = CSS_k_OK;
      }
      else /* else: no error */
      {
        /* check if required fields were contained in the path. This also     */
        /* includes the check for a safety network segment (see FRS153).      */
        /* Application path can be long or compressed, first check for long   */
        ps_sOpenRespStat->u16_ext = EpathSegmentCheck(dw_valid,
                                                      k_SOPEN_LONG_REQ,
                                                      k_SOPEN_OPT);
        /* if it is an EPATH with a valid long Application Path */
        if (ps_sOpenRespStat->u16_ext == CSOS_k_INVALID_ADD_STATUS)
        {
          /* ok, all required fields set */
        }
        else  /* else: then it can still be a valid EPATH with a compressed
               Application Path */
        {
          ps_sOpenRespStat->u16_ext = EpathSegmentCheck(dw_valid,
                                                        k_SOPEN_COMPR_REQ,
                                                        k_SOPEN_OPT);

          /* if it is an EPATH with a valid compressed Application Path */
          if (ps_sOpenRespStat->u16_ext == CSOS_k_INVALID_ADD_STATUS)
          {
            /* Set classId2 and classId3 same as classId. Then all further
               processing can be done independently of Application Path
               format. */
            s_ePathFields.s_logSeg.u16_classId2 =
              s_ePathFields.s_logSeg.u16_classId1;
            s_ePathFields.s_logSeg.u16_classId3 =
              s_ePathFields.s_logSeg.u16_classId1;
            s_ePathFields.s_logSeg.u32_instId2 =
              s_ePathFields.s_logSeg.u32_cpId1;
            s_ePathFields.s_logSeg.u32_instId3 =
              s_ePathFields.s_logSeg.u32_cpId2;
          }
          else  /* else: check for a compressed path with instance IDs */
          {
            /* if it is the CCO Validate that called this function */
            if (o_originator)
            {
              /* if none of the previous path constructs matched then we can
                 try another one: compressed path with instances */
              ps_sOpenRespStat->u16_ext =
                EpathSegmentCheck(dw_valid, k_SOPEN_COMPR_INST_REQ,
                                  k_SOPEN_OPT);

              /* if it is a valid compressed EPATH with Instance IDs */
              if (ps_sOpenRespStat->u16_ext == CSOS_k_INVALID_ADD_STATUS)
              {
                /* For this kind of compressed path we also have to set the
                   missing parts (classId2 and classId3 same as classId). */
                s_ePathFields.s_logSeg.u16_classId2 =
                  s_ePathFields.s_logSeg.u16_classId1;
                s_ePathFields.s_logSeg.u16_classId3 =
                  s_ePathFields.s_logSeg.u16_classId1;
              }
              else  /* else: none of the path types that we tried matched */
              {
                /* Error, ps_sOpenRespStat->u16_ext is set to something else
                   than CSOS_k_INVALID_ADD_STATUS */
              }
            }
            else  /* else: was called from Target (Forward_Open checking) */
            {
              /* Target does not support the compressed path with instances,
                 thus none of the path types supported by Target has matched. */
              /* ps_sOpenRespStat->u16_ext already set to something else than
                 CSOS_k_INVALID_ADD_STATUS */
            }
          }
        }

        /* if none of the known/supported path types has matched */
        if (ps_sOpenRespStat->u16_ext != CSOS_k_INVALID_ADD_STATUS)
        {
          ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
          /* extended status already set */
          SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SEG_INV),
                            IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
          w_retVal = CSS_k_OK;
        }
        else /* else: EPATH is valid */
        {
          /* store pointer to electronic key in Forward_Open stream */
          ps_sOpenAuxData->pba_elKey = s_ePathFields.s_logSeg.pba_elKey;

          /* if caller doesn't want to have this data returned */
          if (ps_sOpenPar == CSS_k_NULL)
          {
            /* ps_sOpenPar indicates that the caller is not interested in   */
            /* this data                                                    */
            w_retVal = CSS_k_OK;
          }
          else /* else: copy data from Forward_Open message */
          {
            /* Call function to extract the fields of the Electronic Key    */
            /* Segment. Electronic Key segment must be present in all       */
            /* SafetyOpen (see FRS341) */
            w_retVal = IXSCE_ElectronicKeyParse(ps_sOpenAuxData->pba_elKey,
                                                &ps_sOpenPar->s_elKey);
          }

          /* if previous function returned an error */
          if (w_retVal != CSS_k_OK)
          {
            /* w_retVal is already set, do not continue further parsing */
          }
          else /* else: no error */
          {
            /* as this function gets to complex let a sub-function continue */
            w_retVal =  FwdOpenParse2(&s_ePathFields, dw_valid, ps_sOpenPar,
                                      ps_sOpenAuxData, ps_sOpenRespStat);

          }
        }
      }
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
/*******************************************************************************
**
** Function    : IXSCE_FwdCloseParse
**
** Description : This function parses the received Forward_Close request (byte
**               stream) and extracts the parameters into structures.
**
** Parameters  : ps_rxHalcMsg (IN)       - pointer to the received HALC message
**                                         structure
**                                         (valid range: <> CSS_k_NULL
**                                         not checked, checked in
**                                         IXSCE_CmdProcess())
**               ps_cnxnTriad (OUT)      - pointer to the structure to where the
**                                         relevant data of the received
**                                         Forward_Close request is to be stored
**                                         (not checked, only called with
**                                         reference to structure variable)
**               ps_sCloseRespStat (OUT) - pointer to structure for returning
**                                         the status for a Forward_Open
**                                         response
**                                         (not checked, only called with
**                                         reference to structure variable)
**
** Returnvalue : -
**
*******************************************************************************/
void IXSCE_FwdCloseParse(const HALCS_t_MSG *ps_rxHalcMsg,
                         CSS_t_CNXN_TRIAD *ps_cnxnTriad,
                         CSS_t_MR_RESP_STATUS *ps_sCloseRespStat)
{
  /* if length is too short */
  if (ps_rxHalcMsg->u16_len < k_FWDC_FIELDS_SIZE)
  {
    /* received packet is too short to be a valid Forward_Open */
    ps_sCloseRespStat->u8_gen = CSOS_k_CGSC_NOT_ENOUGH_DATA;
    ps_sCloseRespStat->u16_ext = CSS_k_CMEXTSTAT_UNDEFINED;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SC_TOO_SHORT),
                      IXSER_k_I_NOT_USED, (CSS_t_UDINT)ps_rxHalcMsg->u16_len);
  }
  else /* else: message has at least minimum length */
  {
    /* read Connection Serial Number, Originator Vendor ID and Serial Number */
    CSS_N2H_CPY16(&ps_cnxnTriad->u16_cnxnSerNum,
                  CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                 k_OFS_FWDC_CNXN_S_NUM));
    CSS_N2H_CPY16(&ps_cnxnTriad->u16_origVendId,
                  CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                 k_OFS_FWDC_ORIG_V_ID));
    CSS_N2H_CPY32(&ps_cnxnTriad->u32_origSerNum,
                  CSS_ADD_OFFSET(ps_rxHalcMsg->pb_data,
                                 k_OFS_FWDC_ORIG_S_NUM));

    /* ConnectionPath Size (USINT) is following. Value not checked. */
    /* 1 reserved byte is following. Value not checked. */

    /* The Connection_Path is not of interest for SafetyOpen (must be ignored!
       See FRS337) */

    /* return success */
    ps_sCloseRespStat->u8_gen  = CSOS_k_CGSC_SUCCESS;
    ps_sCloseRespStat->u16_ext = CSOS_k_INVALID_ADD_STATUS;
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */
}
#endif  /* (CSOS_cfg_TARGET == CSOS_k_ENABLE) */


/*******************************************************************************
**    static functions
*******************************************************************************/

/*******************************************************************************
**
** Function    : FwdOpenParse2
**
** Description : This function is a sub-function of IXSCE_FwdOpenParse() and
**               continues the parsing of a received Forward_Open request (byte
**               stream) and extracts the parameters into structures.
**
** Parameters  : ps_ePathFields (IN)   - pointer to the structure describing
**                                       the values parsed from the Connection
**                                       Path (EPATH)
**                                       (not checked, only called with
**                                       reference to structure variable)
**               dw_valid (IN)         - bit field that indicates which fields
**                                       inside ps_ePathFields structure are
**                                       valid
**                                       (not checked, continues work of
**                                       IXSCE_FwdOpenParse())
**               ps_sOpenPar (OUT)     - pointer to the structure to where the
**                                       data of the received Forward_Open
**                                        request is to be stored. The caller
**                                        can also pass CSS_k_NULL if he is not
**                                       interested in this data.
**                                       (any value allowed, not checked)
**               ps_sOpenAuxData (OUT) - pointer to structure to where
**                                       additional information concerning the
**                                       received Forward_Open request is to be
**                                       stored
**                                       (not checked, only called with
**                                       reference to structure variable)
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                       (not checked, only called with
**                                       reference to structure variable)
**
** Returnvalue : CSS_k_OK              - success
**               <>CSS_k_OK            - Extended Status Code to be returned in
**                                       Unsuccessful Forward_Open Response
**
*******************************************************************************/
static CSS_t_WORD FwdOpenParse2(const IXEPP_t_FIELDS *ps_ePathFields,
                                CSS_t_UDINT dw_valid,
                                CSS_t_SOPEN_PARA *ps_sOpenPar,
                                IXSCE_t_SOPEN_AUX *ps_sOpenAuxData,
                                CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = (IXSCE_k_FSE_INC_PRG_FLOW);

  /* if classes don't point to the assembly object */
  if (    (ps_ePathFields->s_logSeg.u16_classId1 != CSOS_k_CCC_ASSEMBLY_OBJ)
       || (ps_ePathFields->s_logSeg.u16_classId2 != CSOS_k_CCC_ASSEMBLY_OBJ)
       || (ps_ePathFields->s_logSeg.u16_classId3 != CSOS_k_CCC_ASSEMBLY_OBJ)
     )
  {
    ps_sOpenRespStat->u8_gen = CSOS_k_CGSC_CNXN_FAILURE;
    ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SEGMENT;
    SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_CLASS),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
    w_retVal = CSS_k_OK;
  }
  else /* else: classes are set correctly */
  {
    /* if instances are too large */
    if (    (ps_ePathFields->s_logSeg.u32_instId1 > CSOS_k_MAX_UINT)
         || (ps_ePathFields->s_logSeg.u32_instId2 > CSOS_k_MAX_UINT) /* cpId1 */
         || (ps_ePathFields->s_logSeg.u32_instId3 > CSOS_k_MAX_UINT) /* cpId1 */
       )
    {
      ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SEGMENT;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_INV_INST),
                      IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      w_retVal = CSS_k_OK;
    }
    else /* else: instances are valid (only 16 bit) */
    {
      /* store the size of the application path */
      ps_sOpenAuxData->u16_elKeyAndAppPathSize =
        ps_ePathFields->u16_elKeyAndAppPathSize;
      /* store pointer to Network Segment Safety */
      ps_sOpenAuxData->pba_nsd = ps_ePathFields->s_netSeg.pba_nsd;

      /* if caller doesn't want to have this data returned */
      if (ps_sOpenPar == CSS_k_NULL)
      {
        /* ps_sOpenPar indicates that the caller is not           */
        /* interested in this data                                */
        /* set success response code */
        ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_SUCCESS;
        ps_sOpenRespStat->u16_ext = CSOS_k_INVALID_ADD_STATUS;
        w_retVal = CSS_k_OK;
      }
      else /* else: copy data from Forward_Open message */
      {
        /* copy the application paths */
        ps_sOpenPar->u16_cfgClass      = CSOS_k_CCC_ASSEMBLY_OBJ;
        ps_sOpenPar->u16_cfgInst       =
          (CSS_t_UINT)ps_ePathFields->s_logSeg.u32_instId1;
        ps_sOpenPar->u16_cnxnPointCons =
          (CSS_t_UINT)ps_ePathFields->s_logSeg.u32_instId2;
        ps_sOpenPar->u16_cnxnPointProd =
          (CSS_t_UINT)ps_ePathFields->s_logSeg.u32_instId3;

        /* if no config data is contained in Forward_Open */
        if ((dw_valid & IXEPP_k_SFB_DATA_SEG)
             != IXEPP_k_SFB_DATA_SEG)
        {
          /* Type 2 SafetyOpen */
          ps_sOpenPar->pb_cfgData = CSS_k_NULL;
          ps_sOpenPar->u8_cfgDataSizeW = 0U;
        }
        else /* else: there is configuration data in Forward_Open */
        {
          /* Type 1 SafetyOpen */
          ps_sOpenPar->pb_cfgData =
            ps_ePathFields->s_dataSeg.pba_cfgData;
          ps_sOpenPar->u8_cfgDataSizeW =
            ps_ePathFields->s_dataSeg.u8_cfgSizeW;
        }

        /* extract the fields of the Network Segment Safety */
        w_retVal =
          NetSegmentSafetyParse(ps_ePathFields->s_netSeg.pba_nsd,
                                &ps_sOpenPar->s_nsd, ps_sOpenRespStat);
      }
    }
  }


  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : NetSegmentSafetyParse
**
** Description : This function parses the fields of the Network Segment Safety
**               in a SafetyOpen message and stores the values in the Safety
**               Open Parameters structure.
**
** Parameters  : pba_nsd (IN)           - pointer to the received SafetyOpen
**                                        message (byte stream)
**                                        (valid range: <> CSS_k_NULL, checked)
**               ps_nsd (OUT)           - pointer to the structure to where the
**                                        data of the received SafetyOpen
**                                        request is to be stored
**                                        (not checked, checked in
**                                        IXSCE_FwdOpenParse())
**               ps_sOpenRespStat (OUT) - pointer to structure for returning the
**                                        status for a Forward_Open response
**                                        (not checked, only called with
**                                        reference to structure variable)
**
** Returnvalue : CSS_k_OK               - success
**               <>CSS_k_OK             - Extended Status Code to be returned in
**                                        Unsuccessful Forward_Open Response
**
*******************************************************************************/
static CSS_t_WORD NetSegmentSafetyParse(const CSS_t_BYTE *pba_nsd,
                                        CSS_t_NET_SEG_SAFETY_T_FRMT *ps_nsd,
                                        CSS_t_MR_RESP_STATUS *ps_sOpenRespStat)
{
  /* return value of this function */
  CSS_t_WORD w_retVal = IXSCE_k_FSE_INC_PRG_FLOW;
  /* temporary variable for checking the segment size */
  CSS_t_USINT u8_segSize = 0U;

  /* if passed pointer is invalid */
  if (pba_nsd == CSS_k_NULL)
  {
    w_retVal = (IXSCE_k_FSE_INC_PTR_INV_NSS_PRS);
    SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
  }
  else /* else: pointer is valid */
  {
    /* extract Segment Size and Segment Format */
    CSS_N2H_CPY8(&u8_segSize, CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_SIZE));
    CSS_N2H_CPY8(&ps_nsd->u8_format,
                 CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_FORMAT));

    /* check segment format vs size */
    /* if base format and size as expected */
    if (    (u8_segSize == CSOS_k_SIZE_NET_SEG_DATA_BASE)
         && (ps_nsd->u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
       )
    {
      /* this is a valid request in Target Format (Base) */
    }
    /* else if extended format and size as expected */
    else if (    (u8_segSize == CSOS_k_SIZE_NET_SEG_DATA_EXT)
              && (ps_nsd->u8_format == CSOS_k_SNS_FORMAT_TARGET_EXT)
            )
    {
      /* this is a valid request in EF Format (Extended) */
    }
    else /* else: format/size mismatch */
    {
      /* error: format and size information does not match */
      ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_CNXN_FAILURE;
      ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCNXN_FORMAT;
      SAPL_CssErrorClbk((IXSCE_k_NFSE_RXE_SO_SSEG_SIZE),
                        IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
      w_retVal = CSS_k_OK;
    }

    /* if already found an error */
    if (w_retVal != IXSCE_k_FSE_INC_PRG_FLOW)
    {
      /* do not continue with further parsing */
    }
    else /* else: no error */
    {
      /* continue with parsing the parts that are common to Base and Extended */
      /* Format                                                               */
      CSS_N2H_CPY32(&ps_nsd->u32_sccrc,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_SCCRC));
      CSS_N2H_CPY32(&ps_nsd->s_scts.u32_time,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_SCTS_TIME));
      CSS_N2H_CPY16(&ps_nsd->s_scts.u16_date,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_SCTS_DATE));
      CSS_N2H_CPY32(&ps_nsd->u32_tcorrEpi_us,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TCORR_EPI));
      CSS_N2H_CPY16(&ps_nsd->w_tcorrNetCnxnPar,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TCORR_NCP));
      CSS_N2H_CPY32(&ps_nsd->s_tunid.s_snn.u32_time,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TUNID_SNN_TIME));
      CSS_N2H_CPY16(&ps_nsd->s_tunid.s_snn.u16_date,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TUNID_SNN_DATE));
      CSS_N2H_CPY32(&ps_nsd->s_tunid.u32_nodeId,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TUNID_NODEID));
      CSS_N2H_CPY32(&ps_nsd->s_ounid.s_snn.u32_time,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_OUNID_SNN_TIME));
      CSS_N2H_CPY16(&ps_nsd->s_ounid.s_snn.u16_date,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_OUNID_SNN_DATE));
      CSS_N2H_CPY32(&ps_nsd->s_ounid.u32_nodeId,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_OUNID_NODEID));
      CSS_N2H_CPY16(&ps_nsd->u16_pingIntEpiMult,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_PIEM));
      CSS_N2H_CPY16(&ps_nsd->u16_tcooMsgMinMult_128us,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TCMMM));
      CSS_N2H_CPY16(&ps_nsd->u16_NetTimeExpMult_128us,
                    CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_NTEM));
      CSS_N2H_CPY8(&ps_nsd->u8_timeoutMult,
                   CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TMUL));
      CSS_N2H_CPY8(&ps_nsd->u8_maxConsNum,
                   CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_MAX_CONS));

      /* if base format */
      if (ps_nsd->u8_format == CSOS_k_SNS_FORMAT_TARGET_BASE)
      {
      #if (CSS_cfg_BASE_FORMAT == CSS_k_ENABLE)
        {
          /* parse the fields that are special for Base Format */
          CSS_N2H_CPY32(&ps_nsd->u32_cpcrc,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_CPCRC_BASE));
          CSS_N2H_CPY32(&ps_nsd->u32_tcorrCnxnId,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TCORR_CID_BASE));
          /* parsing was successful, set success response code */
          ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_SUCCESS;
          ps_sOpenRespStat->u16_ext = CSOS_k_INVALID_ADD_STATUS;
        }
      #else
        {
          ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCNXN_FORMAT;
          w_retVal = (IXSCE_k_NFSE_RXE_BASE_NOT_SUP);
          SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        }
      #endif
      }
      else /* else: extended format */
      {
      #if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
        {
          /* parse the fields that are special for Extended Format
             (see FRS373) */
          CSS_N2H_CPY16(&ps_nsd->u16_maxFaultNum,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_MAX_FAULT_EXT));
          CSS_N2H_CPY32(&ps_nsd->u32_cpcrc,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_CPCRC_EXT));
          CSS_N2H_CPY32(&ps_nsd->u32_tcorrCnxnId,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_TCORR_CID_EXT));
          CSS_N2H_CPY16(&ps_nsd->u16_initalTs,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_INITIAL_TS_EXT));
          CSS_N2H_CPY16(&ps_nsd->u16_initialRv,
                        CSS_ADD_OFFSET(pba_nsd, k_OFS_NSD_INITIAL_RV_EXT));
          /* parsing was successful, set success response code */
          ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_SUCCESS;
          ps_sOpenRespStat->u16_ext = CSOS_k_INVALID_ADD_STATUS;
        }
      #else
        {
          ps_sOpenRespStat->u8_gen  = CSOS_k_CGSC_CNXN_FAILURE;
          ps_sOpenRespStat->u16_ext = CSS_k_CMEXTSTAT_SCNXN_FORMAT;
          w_retVal = (IXSCE_k_NFSE_RXE_EXT_NOT_SUP);
          SAPL_CssErrorClbk(w_retVal, IXSER_k_I_NOT_USED, IXSER_k_A_NOT_USED);
        }
      #endif
      }

      w_retVal = CSS_k_OK;
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (w_retVal);
}


/*******************************************************************************
**
** Function    : EpathSegmentCheck
**
** Description : This function checks the passed bit string (u32_testBits) if
**               all the required bits (u32_reqBits) are set. Additionally only
**               the optional bits (u32_optBits) may be set. No other bits.
**               The return value can be used as the additional status code for
**               a connection failure error.
**
** Parameters  : u32_testBits (IN) - the bit string that is to be tested
**                                   (not checked, any value allowed)
**               u32_reqBits (IN)  - bit mask of required bits. If a bit in this
**                                   mask is set then it must also be set in the
**                                   u32_testBits.
**                                   (not checked, any value allowed)
**               u32_optBits (IN)  - bit mask of the optional bits. If a bit in
**                                   this mask is set then it may be optionally
**                                   set in u32_testBits. If in u32_testBits is
**                                   set that is not set in u32_reqBits or
**                                   u32_optBits then an error is returned
**
** Returnvalue : CSOS_k_INVALID_ADD_STATUS - success: all required bits are set
**                                           maybe some optional bits, but no
**                                           others
**               CSS_k_CMEXTSTAT_PARAM_ERR - error: required bit(s) missing
**               CSS_k_CMEXTSTAT_SEGMENT   - error: invalid bit(s) set
**
*******************************************************************************/
static CSS_t_UINT EpathSegmentCheck(CSS_t_UDINT u32_testBits,
                                    CSS_t_UDINT u32_reqBits,
                                    CSS_t_UDINT u32_optBits)
{
  CSS_t_UINT u16_retVal = CSOS_k_INVALID_ADD_STATUS;

  /* if a required bit is missing */
  if ((u32_testBits & u32_reqBits) != u32_reqBits)
  {
    u16_retVal = CSS_k_CMEXTSTAT_PARAM_ERR;
  }
  else /* else: all required bits are set */
  {
    /* if more than the required and optional bits are set */
    if ((u32_testBits & (~(u32_reqBits | u32_optBits))) != 0UL)
    {
      u16_retVal = CSS_k_CMEXTSTAT_SEGMENT;
    }
    else /* else: only required and optional bits are set */
    {
      /* ok, return value already set */
    }
  }

  IXSCF_PATH_TRACK();  /* Control Flow Monitoring */

  return (u16_retVal);
}


/*** End Of File ***/


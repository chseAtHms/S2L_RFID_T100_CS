/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSCE.h
**    Summary: IXSCE - Safety Validator Connection Establishment Engine
**             IXSCE.h is the export header file of the IXSCE unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSCE_MsgLenToPayloadLen
**             IXSCE_CpcrcCalc
**             IXSCE_FwdOpenParse
**             IXSCE_ElectronicKeyParse
**             IXSCE_ElectronicKeyMatchCheck
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSCE_H
#define IXSCE_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** k_CRC_SIZE_xxx:
    Sizes (number of bytes) of the Safety Network Segment for calculation of the
    CPCRC.
*/
#define IXSCE_k_CRC_SIZE_NSEG_SAFETY_BASE 48U  /* Base Format */
#define IXSCE_k_CRC_SIZE_NSEG_SAFETY_EXT  50U  /* Extended Format */

/* Some min/max values for checking RPI ranges */
/*lint -esym(755, IXSCE_k_MAX_RPI_DATA_BASE_US)   not referenced in every cfg */
/*lint -esym(755, IXSCE_k_MAX_RPI_DATA_EXT_US)    not referenced in every cfg */
#define IXSCE_k_MIN_RPI_TCOO_US         100U         /* 100us */
#define IXSCE_k_MAX_RPI_TCOO_US         100000000U   /* 100s */
#define IXSCE_k_MIN_RPI_DATA_US         100U         /* 100us */
#define IXSCE_k_MAX_RPI_DATA_BASE_US    100000U      /* 100ms */
#define IXSCE_k_MAX_RPI_DATA_EXT_US     1000000U     /* 1000ms */


/*******************************************************************************
**    data types
*******************************************************************************/

/** IXSCE_t_SOPEN_AUX:
    Auxiliary data structure for storing data that are important for being able
    to parse the SafetyOpen (and calculate CPCRC), but are not of interest for
    the application
*/
typedef struct
{
  const CSS_t_BYTE *pba_elKey;  /* pointer to Electronic Key Segment */
  const CSS_t_BYTE *pba_nsd;    /* pointer to Network Safety Data Segment */
  #if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
    CSS_t_UDINT  u32_cpcrcCalc; /* CPCRC calculated by us */
  #endif
  CSS_t_UINT  u16_elKeyAndAppPathSize;  /* Size of the Electronic Key and */
                                        /* the Application Paths (1-3) */
} IXSCE_t_SOPEN_AUX;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSCE_MsgLenToPayloadLen) */
/*lint -esym(765, IXSCE_MsgLenToPayloadLen) */
/*******************************************************************************
**
** Function    : IXSCE_MsgLenToPayloadLen
**
** Description : This function calculates the length of the payload data of a
**               Safety Connection from the absolute length transported over the
**               connection - considering if this connection is single- or
**               multicast.
**
** Parameters  : u16_msgLen (IN)    - number of bytes transported by the
**                                    connection
**                                    (valid range: CSOS_k_IO_MSGLEN_SHORT_MIN..
**                                    (CSOS_k_IO_MSGLEN_LONG_MAX +
**                                    CSOS_k_IO_MSGLEN_TCORR), checked)
**               o_isMulticast (IN) - CSS_k_TRUE if this checks a multicast
**                                    connection, CSS_k_FALSE in case of
**                                    singlecast
**
** Returnvalue : 0   - error (this length is not valid for a safety connection)
**               >0  - length of the payload data
**
*******************************************************************************/
CSS_t_USINT IXSCE_MsgLenToPayloadLen(CSS_t_UINT u16_msgLen,
                                     CSS_t_BOOL o_isMulticast);


/* mute lint warnings: depending on configuration this function is used from  */
/* other units or only internally in this unit. Thus lint suggests to make    */
/* this function static (warning is dependent of configuration defines)       */
/*lint -esym(759, IXSCE_CpcrcCalc) */
/*lint -esym(765, IXSCE_CpcrcCalc) */
/*******************************************************************************
**
** Function    : IXSCE_CpcrcCalc
**
** Description : This function calculates the CPCRC from the passed Safety
**               Open.
**
** Parameters  : pb_msgBuf (IN)          - pointer to the buffer which contains
**                                         the received Foward_Open message
**                                         (not checked, only called with
**                                         checked pointers or references to
**                                         variables)
**               o_isExtendedFormat (IN) - CSS_k_TRUE if it is an Extended
**                                         Format Connection
**                                         (not checked, any value allowed)
**               ps_sOpenAuxData (IN)    - pointer to structure with additional
**                                         information concerning the received
**                                         Forward_Open request
**                                         (not checked, only called with
**                                         reference to structure variables)
**
** Returnvalue : CSS_t_UDINT             - Calculated CPCRC value
**
*******************************************************************************/
CSS_t_UDINT IXSCE_CpcrcCalc(const CSS_t_BYTE *pb_msgBuf,
                            CSS_t_BOOL o_isExtendedFormat,
                            const IXSCE_t_SOPEN_AUX *ps_sOpenAuxData);


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
                              CSS_t_MR_RESP_STATUS *ps_sOpenRespStat);


/*******************************************************************************
**
** Function    : IXSCE_ElectronicKeyParse
**
** Description : This function parses the fields of an Electronic Key in an
**               Explicit message and stores the values in the passed Electronic
**               Key structure.
**
** Parameters  : pba_em (IN)    - pointer to the received explicit
**                                message (byte stream)
**                                (valid range: <> CSS_k_NULL, checked)
**               ps_elKey (OUT) - pointer to the structure to where the
**                                data of the electronic key segment is to be
**                                stored
**                                (not checked, only called with reference to
**                                structure variable)
**
** Returnvalue : CSS_k_OK       - success
**               <>CSS_k_OK     - error while processing the message
**
*******************************************************************************/
CSS_t_WORD IXSCE_ElectronicKeyParse(const CSS_t_BYTE *pba_em,
                                    CSS_t_ELECTRONIC_KEY *ps_elKey);


/*******************************************************************************
**
** Function    : IXSCE_ElectronicKeyMatchCheck
**
** Description : This function checks if the Electronic Key in a received
**               Explicit Request matches ours.
**
** Parameters  : ps_elKey (IN)      - pointer to Electronic Key structure of
**                                    received parameters
**                                    (not checked, only called with
**                                    reference to structure variable)
**               pu8_genStat (OUT)  - pointer to the general Status Code to be
**                                    returned in the response message.
**                                    (not checked, only called with reference
**                                    to variable)
**               pu16_extStat (OUT) - pointer to the extended Status Code to be
**                                    returned in the response message.
**                                    Extended Status Codes concerning
**                                    Electronic Key checking of the Connection
**                                    Manager Object are Identical with the
**                                    Message Router Object.
**                                    (not checked, only called with reference
**                                    to variable)
**
** Returnvalue : CSS_k_TRUE         - Electronic Key matches
**               CSS_k_FALSE        - Electronic Key does not match
**
*******************************************************************************/
CSS_t_BOOL IXSCE_ElectronicKeyMatchCheck(const CSS_t_ELECTRONIC_KEY *ps_elKey,
                                         CSS_t_USINT *pu8_genStat,
                                         CSS_t_UINT *pu16_extStat);


#endif /* #ifndef IXSCE_H */

/*** End Of File ***/


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXSVD.h
**    Summary: IXSVD - Safety Validator Data
**             IXSVD.h is the export header file of the IXSVD unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXSVD_Init
**             IXSVD_SvIdxFromInstGet
**             IXSVD_InstIsServer
**             IXSVD_NextFreeInstIdGet
**             IXSVD_SvIndexAssign
**             IXSVD_SvIndexDelete
**             IXSVD_InitRcOffsetIncrementGet
**             IXSVD_LookupSoftErrByteGet
**             IXSVD_LookupSoftErrVarGet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXSVD_H
#define IXSVD_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/* possible values for Consumer_Active_Idle
   Run_Idle (see FRS179) and Multi_Cast_Active_Idle (see FRS209) */
#define IXSVD_k_IDLE                     0U
#define IXSVD_k_ACTIVE                   1U

/* possible values for S_Run_Idle_Out */
/*lint -esym(755, IXSVD_k_RUN)                    not referenced in every cfg */
/* #define IXSVD_k_IDLE                  0U */
#define IXSVD_k_RUN                      1U

/* possible values for S_Connection_Fault and S_Con_Flt_C_Out */
#define IXSVD_k_CNXN_OK                  0U
#define IXSVD_k_CNXN_FAULTED             1U

/** IXSVD_k_MSG_FORMAT_xxx:
    Bit definitions used in Safety Validator Client and Server structures field
    b_msgFormat to determine what kind of safety connection the Safety Validator
    refers to.
*/
/*lint -esym(755, IXSVD_k_MSG_FORMAT_ORIGINATOR)  not referenced in every cfg */
#define IXSVD_k_MSG_FORMAT_LONG          0x01U
#define IXSVD_k_MSG_FORMAT_EXTENDED      0x02U
#define IXSVD_k_MSG_FORMAT_MCAST         0x04U
#define IXSVD_k_MSG_FORMAT_ORIGINATOR    0x08U
/* according to the definitions above a mask to separate only base/extended and
   short/long information from the message format byte */
/*lint -esym(755, IXSVD_k_MSG_FORMAT_MASK)        not referenced in every cfg */
#define IXSVD_k_MSG_FORMAT_MASK          0x03U
/* according to the definitions above a mask to separate all possible
   combinations of base/extended and short/long from the message format byte */
/*lint -esym(755, IXSVD_k_MSG_FORMAT_BFS)         not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MSG_FORMAT_BFL)         not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MSG_FORMAT_EFS)         not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MSG_FORMAT_EFL)         not referenced in every cfg */
#define IXSVD_k_MSG_FORMAT_BFS           0x00U
#define IXSVD_k_MSG_FORMAT_BFL           0x01U
#define IXSVD_k_MSG_FORMAT_EFS           0x02U
#define IXSVD_k_MSG_FORMAT_EFL           0x03U


/* macros to check format of a message */
/*lint -esym(755, IXSVD_IsLongFormat)             not referenced in every cfg */
#define IXSVD_IsBaseFormat(b_msgFormat)                                        \
          (((b_msgFormat) & IXSVD_k_MSG_FORMAT_EXTENDED) == 0x00U)
#define IXSVD_IsExtendedFormat(b_msgFormat)                                    \
          (  ((b_msgFormat) & IXSVD_k_MSG_FORMAT_EXTENDED) ==                  \
             IXSVD_k_MSG_FORMAT_EXTENDED                                       \
          )
#define IXSVD_IsShortFormat(b_msgFormat)                                       \
          ((b_msgFormat & IXSVD_k_MSG_FORMAT_LONG) == 0x00U)
#define IXSVD_IsLongFormat(b_msgFormat)                                        \
          (  ((b_msgFormat) & IXSVD_k_MSG_FORMAT_LONG) ==                      \
             IXSVD_k_MSG_FORMAT_LONG                                           \
          )
#define IXSVD_IsSingleCast(b_msgFormat)                                        \
          (((b_msgFormat) & IXSVD_k_MSG_FORMAT_MCAST) == 0x00U)
#define IXSVD_IsMultiCast(b_msgFormat)                                         \
          (  ((b_msgFormat) & IXSVD_k_MSG_FORMAT_MCAST) ==                     \
             IXSVD_k_MSG_FORMAT_MCAST                                          \
          )
#if (    (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)                             \
      && (CSOS_cfg_NUM_OF_SV_CLIENTS > 0U)                                     \
    )
  #define IXSVD_IsTarget(b_msgFormat)                                          \
            (((b_msgFormat) & IXSVD_k_MSG_FORMAT_ORIGINATOR) == 0x00U)
  #define IXSVD_IsOriginator(b_msgFormat)                                      \
            (  ((b_msgFormat) & IXSVD_k_MSG_FORMAT_ORIGINATOR) ==              \
               IXSVD_k_MSG_FORMAT_ORIGINATOR                                   \
            )
#endif

/* Bit definitions of the Mode Byte */
/*lint -esym(755, IXSVD_k_MB_TBD_BIT)             not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MB_TBD_2_BIT)           not referenced in every cfg */
#define IXSVD_k_MB_PING_COUNT            0x03U
/* #define IXSVD_k_MB_N_TBD_BIT          0x04U */
/* #define IXSVD_k_MB_TBD_2_BIT_COPY     0x08U */
/* #define IXSVD_k_MB_N_RUN_IDLE         0x10U */
#define IXSVD_k_MB_TBD_BIT               0x20U
#define IXSVD_k_MB_TBD_2_BIT             0x40U
#define IXSVD_k_MB_RUN_IDLE              0x80U

/* Bit definitions of the Ack_Byte (in Time Coordination Messages) */
/*lint -esym(755, IXSVD_k_AB_RESERVED_1)          not referenced in every cfg */
/*lint -esym(755, IXSVD_k_AB_RESERVED_2)          not referenced in every cfg */
#define IXSVD_k_AB_PING_COUNT_REPLY      0x03U
#define IXSVD_k_AB_RESERVED_1            0x04U
#define IXSVD_k_AB_PING_RESPONSE         0x08U
#define IXSVD_k_AB_RESERVED_2            0x70U
/* #define IXSVD_k_AB_PARITY             0x80U */

/* Bit definitions of the Mcast_Byte (in Time Correction Messages) */
/*lint -esym(755, IXSVD_k_MCB_CONS_NUM)           not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MCB_RESERVED_1)         not referenced in every cfg */
/*lint -esym(755, IXSVD_k_MCB_RESERVED_2)         not referenced in every cfg */
#define IXSVD_k_MCB_CONS_NUM             0x0FU
#define IXSVD_k_MCB_RESERVED_1           0x10U
#define IXSVD_k_MCB_MAI                  0x20U
#define IXSVD_k_MCB_RESERVED_2           0x40U
/* #define IXSVD_k_MCB_PARITY            0x80U */

/* possible values for Server's first reception flag */
/*lint -esym(755, IXSVD_k_FR_FIRST_REC)           not referenced in every cfg */
/*lint -esym(755, IXSVD_k_FR_CONT_REC)            not referenced in every cfg */
#define IXSVD_k_FR_FIRST_REC             0x01U
#define IXSVD_k_FR_CONT_REC              0x00U

/* Constant: 5 seconds expressed in micro seconds */
#define IXSVD_k_5_SECONDS_US             5000000UL


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXSVD_Init
**
** Description : This function initializes the IXSVD unit.
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXSVD_Init(void);


/*******************************************************************************
**
** Function    : IXSVD_SvIdxFromInstGet
**
** Description : This function converts a Safety Validator Instance ID into
**               the corresponding array index (Server or Client).
**
** Parameters  : u16_instId (IN) - instance ID of the Safety Validator
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue : 0..(CSOS_cfg_NUM_OF_SV_CLIENTS-1) - array index of the Safety
**               or                                  Validator Client array
**             : 0..(CSOS_cfg_NUM_OF_SV_SERVERS-1) - array index of the Safety
**               or                                  Validator Server array
**               CSOS_k_INVALID_IDX                - the passed Instance ID is
**                                                   not existing
**
*******************************************************************************/
CSS_t_UINT IXSVD_SvIdxFromInstGet(CSS_t_UINT u16_instId);


/*******************************************************************************
**
** Function    : IXSVD_InstIsServer
**
** Description : This function returns CSS_k_TRUE if the passed instance ID is
**               a Safety Validator Server Instance (CSS_k_FALSE if it is a
**               Client)
**
** Parameters  : u16_instId (IN) - instance ID of the Safety Validator Server or
**                                 Client
**                                 (not checked, checked by callers)
**
** Returnvalue : CSS_k_TRUE      - instance ID is a Safety Validator Server
**               CSS_k_FALSE     - instance ID is a Safety Validator Client
**
*******************************************************************************/
CSS_t_BOOL IXSVD_InstIsServer(CSS_t_UINT u16_instId);


/*******************************************************************************
**
** Function    : IXSVD_NextFreeInstIdGet
**
** Description : This function searches for the next free Safety Validator
**               Object instance ID number - based on the passed offset.
**
** Parameters  : u16_instIdOffset (IN) - offset for the instance Id.
**                                       Search begins upwards of this value.
**                                       (not checked, only called with
**                                       constants, valid range:
**                                       1..(CSOS_cfg_NUM_OF_SV_SERVERS
**                                          +CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue :   CSOS_k_INVALID_INSTANCE - no free Instance ID found
**               <>CSOS_k_INVALID_INSTANCE - next free Safety Validator Instance
**                                           ID
**
*******************************************************************************/
#if (CSOS_cfg_TARGET == CSOS_k_ENABLE)
CSS_t_UINT IXSVD_NextFreeInstIdGet(CSS_t_UINT u16_instIdOffset);
#endif


/*******************************************************************************
**
** Function    : IXSVD_SvIndexAssign
**
** Description : This function assigns an Index of a Safety Validator
**               Client/Server to the passed Instance ID
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID number
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**               u16_idx (IN)    - index in the array of Safety Validator
**                                 clients/servers
**                                 (checked, valid range:
**                                 0..(CSOS_cfg_NUM_OF_SV_SERVERS-1)  or
**                                 0..(CSOS_cfg_NUM_OF_SV_CLIENTS-1))
**               o_isServer (IN) - must be CSS_k_TRUE for servers,
**                                 CSS_k_FALSE for clients.
**                                 (not checked, any value allowed)
**
** Returnvalue : CSS_k_TRUE      - assignment successful
**               CSS_k_FALSE     - error: passed parameters invalid
**
*******************************************************************************/
CSS_t_BOOL IXSVD_SvIndexAssign(CSS_t_UINT u16_instId,
                               CSS_t_UINT u16_idx,
                               CSS_t_BOOL o_isServer);


/*******************************************************************************
**
** Function    : IXSVD_SvIndexDelete
**
** Description : This function deletes an entry in the Instance <-> index lookup
**               table.
**
** Parameters  : u16_instId (IN) - Safety Validator Instance ID number to be
**                                 deleted.
**                                 (checked, valid range:
**                                 1..(CSOS_cfg_NUM_OF_SV_SERVERS+
**                                 CSOS_cfg_NUM_OF_SV_CLIENTS))
**
** Returnvalue : CSS_k_TRUE      - deleting successful
**               CSS_k_FALSE     - error: passed parameter invalid
**
*******************************************************************************/
CSS_t_BOOL IXSVD_SvIndexDelete(CSS_t_UINT u16_instId);


/*******************************************************************************
**
** Function    : IXSVD_InitRcOffsetIncrementGet
**
** Description : This function increments the value of the device global
**               Initial_RC_Offset and then returns its value. The
**               Initial_RC_Offset is part of the initialization value of the
**               Initial_Rollover_Value in SafetyOpen Requests/Responses.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UINT - Initial_RC_Offset
**
*******************************************************************************/
#if (CSS_cfg_EXTENDED_FORMAT == CSS_k_ENABLE)
  CSS_t_UINT IXSVD_InitRcOffsetIncrementGet(void);
#endif


/*******************************************************************************
**
** Function    : IXSVD_LookupSoftErrByteGet
**
** Description : This function returns one byte of data for Soft Error checking
**
** See Also    : IXSSC_SoftErrByteGet()
**
** Parameters  : u32_varCnt (IN) - addresses one byte of the data that must be
**                                 checked against Soft Errors
**                                 (not checked, any value allowed)
**               pb_var (OUT)    - pointer to a byte that returns the requested
**                                 data
**                                 (not checked, only called with reference to
**                                 variable)
**
** Returnvalue : <>0             - u32_varCnt is greater than the number of
**                                 bytes of soft error protected static
**                                 variables of this module. Returned value is
**                                 the number of soft error protected bytes.
**               0               - addresses a valid byte in the soft error
**                                 protected static variables of this module.
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_BYTES)
  CSS_t_UDINT IXSVD_LookupSoftErrByteGet(CSS_t_UDINT u32_varCnt,
                                         CSS_t_BYTE *pb_var);
#endif


/*******************************************************************************
**
** Function    : IXSVD_LookupSoftErrVarGet
**
** Description : This function returns the data of one variable for Soft Error
**               checking
**
** See Also    : IXSSC_SoftErrVarGet()
**
** Parameters  : au32_cnt (IN)    - addressing information of requested
**                                  variable: each element of the array is the
**                                  counter for one level.
**                                  (not checked, any value allowed)
**               ps_retStat (OUT) - pointer to a struct that returns the status
**                                  of this operation:
**                                  - length of returned data in pb_var array
**                                  - level counter that must be incremented to
**                                    get to next variable
**               pb_var (OUT)     - pointer to a byte that returns the requested
**                                  data
**                                  (not checked, only called with reference to
**                                  variable)
**
** Returnvalue : -
**
*******************************************************************************/
#if (CSS_cfg_SOFT_ERR_METHOD == CSS_SOFT_ERR_MTD_VARIABLES)
  void IXSVD_LookupSoftErrVarGet(
                       const CSS_t_UDINT au32_cnt[CSS_k_SOFT_ERR_NUM_OF_LEVELS],
                       CSS_t_SOFT_ERR_RET_STAT *ps_retStat,
                       CSS_t_BYTE *pb_var);
#endif


#endif /* #ifndef IXSVD_H */

/*** End Of File ***/


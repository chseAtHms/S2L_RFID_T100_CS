/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCfg.h
**     Summary: Interface header of module aicMsgCfg.c
**   $Revision: 4413 $
**       $Date: 2024-05-02 15:03:13 +0200 (Do, 02 Mai 2024) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCfg_SetData
**             aicMsgCfg_GetData
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_CFG_H
#define AICMSG_CFG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
**  Length of IO Configuration data transferred via SetConfigString command (see [SRS_2153])
**-------------------------------------------------------------------------------------------------
*/
#define AICMSGHDL_IOCONFIGDATA_LEN                   23u

/***************************************************************************************************
**    data types
***************************************************************************************************/

/*lint -esym(768, u8_swMajVer) global struct member 'u8_swMajVer' not referenced */
/*lint -esym(768, u16_hwId) global struct member 'u16_hwId' not referenced */
/* Deactivated because the the Software Major Version and Hardware ID are transferred only 
** to ensure that the SCCRC is also calculated over these values. 
** The struct members are not accessed because the software itself uses the hard coded values. 
*/

/* Data structure/stream of 'SetConfigString' request received from non-safe host via AM 
** (see [SRS_2153], [SRS_2200]). 
** Shall be packed to avoid padding bytes.
*/
typedef __packed struct
{
  /* Software Major version */
  UINT8  u8_swMajVer;
  
  /* Hardware ID T100/CS */
  UINT16 u16_hwId;
  
  /* IO Configuration Data set */ 
  UINT8  au8_ioConfigData[AICMSGHDL_IOCONFIGDATA_LEN];
  
  /* SCID (Safety Configuration CRC) (see [SRS_2166]) */
  UINT32 u32_sccrc;
  
  /* SCID (Safety Configuration Time Stamp) (see [SRS_2166]) */
  UINT32 u32_scts_time;
  
  /* SCID (Safety Configuration Time Stamp) (see [SRS_2166])*/
  UINT16 u16_scts_date;
} AICMSGCFG_SETCONFIGSTRDATA;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgCfg_SetData

  Description:
    To be called when a message request containing IO Configuration Data + SCID (SetConfigString) 
    is received (see [SRS_2048]).
    'SetConfigString' requests are "fragmented" messages, means more than one telegram is needed to
    transmit all data (or the whole message).
    The function will count the number of received fragments and indicate if the message is still 
    pending (not all parts/fragments received) or not (all fragments of message received).
    Hint: Message fragmentation is described in Serial Interface Specification[SIS] and Software 
    Requirements Specification[SRS]

  See also:
    -

  Parameters:
    ps_rxMsg (IN)            - Received Message from AM
                               (valid range: <>NULL, not checked, only called with reference to
                               static/global buffer)

  Return value:
    TRUE                     - SetConfigString received completely
    FALSE                    - Still fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCfg_SetData (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg);

/***************************************************************************************************
  Function:
    aicMsgCfg_GetData

  Description:
    This function is used to read the data from the SetConfigString request that was received 
    from ABCC.

  See also:
    -

  Parameters:
    -

  Return value:
    AICMSGCFG_SETCONFIGSTRDATA* - Pointer of SetConfigString data

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST volatile AICMSGCFG_SETCONFIGSTRDATA* aicMsgCfg_GetData(void);


#endif


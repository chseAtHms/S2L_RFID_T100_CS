/***************************************************************************************************
**    Copyright (C) 2015-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: HALCSspdu.c
**     Summary: This module implements the functionality of safe message handling of AIC telegrams.
**              The module is part of the HALCS implementation (see [1.4-3:]).
**   $Revision: 3418 $
**       $Date: 2020-04-07 14:31:08 +0200 (Di, 07 Apr 2020) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: HALCS_TxDataPutMix
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* Project header */
#include "xtypes.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "aicMsgDef.h"
#include "spduIn.h"

/* CSS common headers */
#include "CSStypes.h"

/* HALCS headers */
#include "HALCSapi.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
/*lint -esym(750, k_FILEID) local macro 'k_FILEID' not referenced
** Used previously to generate unique additional info */
#define k_FILEID      14u


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/*******************************************************************************
**
** Function    : HALCS_TxDataPutMix
**
** Description : This function sends data from the CSS to CSAL with respect to
**               mixing the data from Controller 1 and Controller 2. Mixing is
**               required for the HALC commands CSOS_k_CMD_IXSVO_IO_DATA,
**               CSOS_k_CMD_IXCCO_SOPEN1_REQ and CSOS_k_CMD_IXCCO_SOPEN2_REQ.
**               The way how the bytes are arranged and how they have to be
**               mixed is dependent on CSS_cfg_H2N_COPY_METHOD.
**
**               The example below shows how a message which is 10 bytes long
**               (B0, B1 ... B9) is encoded in the different copy methods:
**
**               CSS_H2N_CPY_MTD_ODD_EVEN_SORT:
**               - B9 B7 B5 B3 B1 B0 B2 B4 B6 B8
**
**               CSS_H2N_CPY_MTD_INTERLACE_EVEN:
**               - B0 0 B2 0 B4 0 B6 0 B8 0
**
**               CSS_H2N_CPY_MTD_INTERLACE_ODD:
**               - 0 B1 0 B3 0 B5 0 B7 0 B9
**
**               **Attention**
**               The {HALCS_t_MSG} structure elements u16_cmd, u16_len,
**               u32_addInfo are provided in processor format. If the processor
**               format of HALC and HALCS is different then HAL shall ensure the
**               correct endianness. The endianness of pb_data is handled by the
**               CSAL and CSS.
**
** See Also    : HALCS_TxDataPut()
**
** Parameters  : ps_msg (IN)     - Pointer to the message structure to be sent,
**                                 see {HALCS_t_MSG}
**
** Returnvalue : CSS_k_TRUE      - Sending of message was successful
**               CSS_k_FALSE     - Error during message sending
**
*******************************************************************************/
CSS_t_BOOL HALCS_TxDataPutMix(const HALCS_t_MSG *ps_msg)
{
  /* Interface function required according to [CS_MAN], (see [1.4-3:]) */
  
  /* return value of this function */
  CSS_t_BOOL b_retVal;
  
  /* if Time Coordination Message */
  if (ps_msg->u16_len == AICMSGDEF_CIP_IN_DATA2_SIZE)
  { 
    /* Forward to HALCS message to SpduIn */ 
    spduIn_PrepIpcTcoo((UINT32)ps_msg->u32_addInfo, ps_msg->pb_data);
   
    b_retVal = CSS_k_TRUE;
  }
  /* if else: IO Data Message */
  else if (ps_msg->u16_len == AICMSGDEF_CIP_IN_DATA_SIZE)
  {
    /* Forward to HALCS message to SpduIn */ 
    spduIn_PrepIpcIo((UINT32)ps_msg->u32_addInfo, ps_msg->pb_data);
    
    b_retVal = CSS_k_TRUE;
  }
  /* else: invalid/unsupported Message Type or Length */
  else
  {
    /* unable to send because request does not fit into SPDU design,
    ** it is task of CSS to decide if this is handled as safe or non-safe
    ** error.
    */
    b_retVal = CSS_k_FALSE;
  }
  
  return b_retVal;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/



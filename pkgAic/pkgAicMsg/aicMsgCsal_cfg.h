/***************************************************************************************************
**    Copyright (C) 2015-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCsal_cfg.h
**     Summary: Configuration header of CSAL messages for non-safe message handling.
**   $Revision: 2767 $
**       $Date: 2017-04-20 07:45:26 +0200 (Do, 20 Apr 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: -
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSGCSAL_CFG_H
#define AICMSGCSAL_CFG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/** AICMSGCSAL_cfg_UCQ_SIZE
** Number of Queue Entries of the "Request Command Queue" 
** ATTENTION: According to the design, the SW shall process the HALC Message within the same cycle.
** So a queue is not really necessary here. To be able to uncouple this (future use) the queue is
** still available here. Value set to 2 (one element for writing, one element for reading)
*/
#define AICMSGCSAL_cfg_UCQ_SIZE                2u

/** AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE:
** Payload of request data storable in one single queue entry (in octets) 
** The buffer shall be big enough to hold all HALC Commands received from CSAL (over AIC). 
** Max. payload length of  
** CSOS_k_CMD_IXCMO_SOPEN_REQ     = 150 bytes (see [Con-Safe] #423)
** CSOS_k_CMD_IXCMO_SCLOSE_REQ    = 54 Bytes  (see [Con-Safe] #423)
** CSOS_k_CMD_IXMRO_EXPL_REQ      = variable, so limited here to CSOS_k_CMD_IXCMO_SOPEN_REQ
** CSOS_k_CMD_IXLOS_LINK_FAILURE  = 0 Bytes (see EIP Porting Guide)
** CSOS_k_CMD_IXCO_IO_DATA        = not transferred here
** 
** Since the ABCC module handles HALC messages with a length up to 500 bytes (according to 
** [SIS_CS]), the payload size is set to this value.
*/
#define AICMSGCSAL_cfg_REQ_PAYLOAD_SIZE        500u

/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

#endif


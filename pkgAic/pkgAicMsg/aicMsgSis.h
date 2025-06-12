/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgSis.h
**     Summary: This header contains the definitions of the Serial Interface Specification [SIS].
**   $Revision: 4413 $
**       $Date: 2024-05-02 15:03:13 +0200 (Do, 02 Mai 2024) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: -
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSG_SIS_H
#define AICMSG_SIS_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/*------------------------------------------------------------------------------
** mask to differentiate a request from a response, see [SIS_032], [SIS_033]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_REQ_MASK       0x4000u


/*------------------------------------------------------------------------------
** limit / max. values for serial error counters
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_ERRCNT_LIMIT   0xFFFFu

/*------------------------------------------------------------------------------
** Request message ID
** Hint: ASM uses only one dummy ID, because there is never a request be sent 
**       while any other request (or the response to this request) is pending.
**       See [SIS_028].
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_DEF_REQ_ID         0u

/* Mask to get ABCC state out from Ctrl/Status Byte */
#define AICMSGHDL_ANYBUS_STATE_MASK       0x07u

/* ABCC State, No request (see [SIS_220]) */
#define AICMSGHDL_ANYBUS_STATE_R_NO       0x00u
/* ABCC State, Request to transit to Abort state. Transition is required in case
** of an IP switch setting change according to FRS364 of VOL5 (see [SIS_220]).
 */
#define AICMSGHDL_ANYBUS_STATE_R_ABORT    0x01u

/*------------------------------------------------------------------------------
** bitmask for fragmentation flags
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_MASK_ANYBUS_FRAGFLAG    0x80u
#define AICMSGHDL_MASK_ASM_FRAGFLAG       0x40u

/*------------------------------------------------------------------------------
** mask to get AM/ABCC function out from Req/Resp Message Header
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_FUNC_MASK               0x00FFu

/*------------------------------------------------------------------------------
** error response message definitions
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_ERROR_RESPONSE_ID_MASK1    ( ( UINT16 ) 0x8000u )
#define AICMSGHDL_ERROR_RESPONSE_ID_MASK2    ( ( UINT16 ) 0x80FFu )
#define AICMSGHDL_ERROR_RESPONSE_LENGTH      0x02u

/*------------------------------------------------------------------------------
** definitions used in error response message as error codes or "message state"
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_MSG_VALID                   ( (UINT16) 0x0000u )
#define AICMSGHDL_MSG_UNSUPPORTED_FUNCTION    ( (UINT16) 0x0001u )
#define AICMSGHDL_MSG_INVALID_REQUEST_LENGTH  ( (UINT16) 0x0002u )
#define AICMSGHDL_MSG_INVALID_DATA            ( (UINT16) 0x0003u )
#define AICMSGHDL_MSG_INVALID_STATE           ( (UINT16) 0x0004u )
#define AICMSGHDL_MSG_OUT_OF_RES              ( (UINT16) 0x0005u )

/*------------------------------------------------------------------------------
**  GetASMStatus message definitions [SIS_055]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_GET_ASM_STATUS_REQ         ( ( UINT16 ) 0x4000u )
#define AICMSGHDL_AM_GET_ASM_STATUS_RESP        ( ( UINT16 ) 0x0000u )
#define AICMSGHDL_AM_GET_ASM_STATUS_REQ_LENGTH  0u
#define AICMSGHDL_AM_GET_ASM_STATUS_RES_LENGTH  0x0Cu


/*------------------------------------------------------------------------------
** SetConfigString message definitions [SIS_056]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_SET_CONFIG_STRING_REQ         ( ( UINT16 ) 0x4001u )
#define AICMSGHDL_AM_SET_CONFIG_STRING_REQ_LENGTH  ( ( UINT8  ) 0x24u )

#define AICMSGHDL_AM_SET_CONFIG_STRING_RESP        ( ( UINT16 ) 0x0001u )
#define AICMSGHDL_AM_SET_CONFIG_STRING_RESP_LENGTH ( ( UINT8  ) 0x00u )

/*------------------------------------------------------------------------------
** Error Confirmation message definitions [SIS_057]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_ERROR_CONFIRM_REQ          0x4002u
#define AICMSGHDL_AM_ERROR_CONFIRM_RESP         0x0002u
#define AICMSGHDL_AM_ERROR_CONFIRM_LENGTH       0u

/*------------------------------------------------------------------------------
** HalcCsalMessage message definitions [SIS_202]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_HALC_CSAL_MSG_REQ          ( ( UINT16 ) 0x4080u )
/* Length of request depends on transferred data */
/* Attention: The length field is 2 bytes long instead of 1 byte as defined in [SIS_053] */ 

#define AICMSGHDL_AM_HALC_CSAL_MSG_RESP         ( ( UINT16 ) 0x0080u )
#define AICMSGHDL_AM_HALC_CSAL_MSG_RESP_LENGTH  ( ( UINT8 )  0x00u )


/*------------------------------------------------------------------------------
** SetInitData message definitions [SIS_203]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_SET_INIT_DATA_REQ          ( ( UINT16 ) 0x4081u )
#define AICMSGHDL_AM_SET_INIT_DATA_REQ_LENGTH   ( ( UINT8  ) 0x12u )

#define AICMSGHDL_AM_SET_INIT_DATA_RESP         ( ( UINT16 ) 0x0081u )
#define AICMSGHDL_AM_SET_INIT_DATA_RESP_LENGTH  ( ( UINT8  ) 0x00u )

/*------------------------------------------------------------------------------
** GetClassId message definitions [SIS_214]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_GET_CLASS_ID_REQ           ( ( UINT16 ) 0x4082u )
#define AICMSGHDL_AM_GET_CLASS_ID_REQ_LENGTH    ( ( UINT8  ) 0x00u )

#define AICMSGHDL_AM_GET_CLASS_ID_RESP          ( ( UINT16 ) 0x0082u )
/* Length of response depends on transferred class ids */

/*------------------------------------------------------------------------------
** LinkStatus message definitions [SIS_216]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_LINK_STATUS_REQ            ( ( UINT16 ) 0x4083u )
#define AICMSGHDL_AM_LINK_STATUS_REQ_LENGTH     ( ( UINT8  ) 0x01u )

#define AICMSGHDL_AM_LINK_STATUS_RESP           ( ( UINT16 ) 0x0083u )
#define AICMSGHDL_AM_LINK_STATUS_RESP_LENGTH    ( ( UINT8  ) 0x00u )

/*------------------------------------------------------------------------------
** GetAssemblyInstIDs message definitions [SIS_217]
**------------------------------------------------------------------------------
*/
#define AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_REQ           ( ( UINT16 ) 0x4084u )
#define AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_REQ_LENGTH    ( ( UINT8  ) 0x00u )

#define AICMSGHDL_AM_GET_ASSEMBLY_INST_ID_RESP          ( ( UINT16 ) 0x0084u )
/* Length of response depends on transferred assembly instance ids */


/*------------------------------------------------------------------------------
** HalcCssMessage message definitions [SIS_206]
**------------------------------------------------------------------------------
*/
/*lint -esym(755, AICMSGHDL_ASM_HALC_CSS_MSG_RESP_LENGTH)*/
/* These constants are defined but not used at the moment, for future use.
 * Lint Message deactivated. */
#define AICMSGHDL_ASM_HALC_CSS_MSG_REQ           ( ( UINT16 ) 0x4080u )
/* Length of request depends on transferred data */
/* Attention: The length field is 2 bytes long instead of 1 byte as defined in [SIS_053] */ 

#define AICMSGHDL_ASM_HALC_CSS_MSG_RESP          ( ( UINT16 ) 0x0080u )
/* Attention: The length field is 2 bytes long instead of 1 byte as defined in [SIS_053] */ 
#define AICMSGHDL_ASM_HALC_CSS_MSG_RESP_LENGTH   ( ( UINT16 ) 0x0000u )

/*------------------------------------------------------------------------------
** LEDstate message definitions [SIS_207]
**------------------------------------------------------------------------------
*/
/*lint -esym(755, AICMSGHDL_ASM_LED_STATE_RESP_LENGTH)*/
/* These constants are defined but not used at the moment, for future use.
 * Lint Message deactivated. */
#define AICMSGHDL_ASM_LED_STATE_REQ              ( ( UINT16 ) 0x4081u )
#define AICMSGHDL_ASM_LED_STATE_REQ_LENGTH       ( ( UINT8  ) 0x02u )

#define AICMSGHDL_ASM_LED_STATE_RESP             ( ( UINT16 ) 0x0081u )
#define AICMSGHDL_ASM_LED_STATE_RESP_LENGTH      ( ( UINT8  ) 0x00u )

/*------------------------------------------------------------------------------
** SafetyReset message definitions [SIS_219]
**------------------------------------------------------------------------------
*/
/*lint -esym(755, AICMSGHDL_ASM_SAFETY_RESET_RESP_LENGTH)*/
/* These constants are defined but not used at the moment, for future use.
 * Lint Message deactivated. */
#define AICMSGHDL_ASM_SAFETY_RESET_REQ           ( ( UINT16 ) 0x4082u )
#define AICMSGHDL_ASM_SAFETY_RESET_REQ_LENGTH    ( ( UINT8  ) 0x02u )

#define AICMSGHDL_ASM_SAFETY_RESET_RESP          ( ( UINT16 ) 0x0082u )
#define AICMSGHDL_ASM_SAFETY_RESET_RESP_LENGTH   ( ( UINT8  ) 0x00u )

#define AICMSGHDL_ASM_SAFETY_RESET_TYPE0         ( (UINT8   ) 0x00u )
#define AICMSGHDL_ASM_SAFETY_RESET_TYPE1         ( (UINT8   ) 0x01u )
#define AICMSGHDL_ASM_SAFETY_RESET_TYPE2         ( (UINT8   ) 0x02u )

/*------------------------------------------------------------------------------
**  FatalErrorEntry message definitions, see [SIS_066]
**------------------------------------------------------------------------------
*/
/* includes CMD bit, see [SIS_031] */
#define AICMSGHDL_ASM_SEND_FATAL_ERR_REQ         ( ( UINT16 ) 0x4000u )
#define AICMSGHDL_ASM_SEND_FATAL_ERR_REQ_LENGTH  ( ( UINT8  ) 0x02u )



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


/*******************************************************************************
**    Copyright (C) 2009-2018 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: HALCSapi.h
**    Summary: HALCS - Hardware Abstraction Layer - Safety (Communication)
**             HALCS is on Controller1/Controller2 the interface of the HALC
**             and CSS. HALCSapi.h defines the interface of the HALC to CSS.
**             The HALCS also controls the physical ports of the Module and
**             Network Status LEDs.
**             HALCSapi.h specifies the interface of HALCS and must be included
**             by a safety application in order to use the CIP Safety Stack
**             (see Req.4.3-1 and Req.5.5-1).
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: HALCS_TxDataPut
**             HALCS_TxDataPutMix
**             HALCS_ModStatLedSet
**             HALCS_NetStatLedSet
**
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef HALCS_API_H
#define HALCS_API_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** HALCS_t_LED_STATES:
    This enumeration type encodes the states of the Module Status LED and
    Network Status LED.
*/
typedef enum
{
/*lint -esym(769, HALCS_k_LED_OFF)     macro not referenced: usage of this macro
                                       by SAPL is optional */
  HALCS_k_LED_OFF,             /* LED is off */
  HALCS_k_LED_GREEN,           /* LED is solid green */
  HALCS_k_LED_FLASH_GREEN,     /* LED blinks continuously green */
  HALCS_k_LED_FLASH_RED,       /* LED blinks continuously red */
  HALCS_k_LED_RED,             /* LED is solid red */
  HALCS_k_LED_FLASH_RED_GREEN  /* LED blinks continuously alternating red
                                  and green */
} HALCS_t_LED_STATES;


/*******************************************************************************
**    data types
*******************************************************************************/

/** HALCS_t_MSG:
    This type defines the messages that are passed from CSS to CSAL and
    from CSAL to CSS.
*/
typedef struct
{
  CSS_t_UINT  u16_cmd;          /* Command code (see CSOSapi.h) */
  CSS_t_UINT  u16_len;          /* Data length of the passed message */
  CSS_t_UDINT u32_addInfo;      /* Additional information */
  const CSS_t_BYTE *pb_data;    /* Pointer to passed data */
} HALCS_t_MSG;


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : HALCS_TxDataPut
**
** Description : This function sends data from the CSS to CSAL.
**
**               **Attention**
**               The {HALCS_t_MSG} structure elements u16_cmd, u16_len,
**               u32_addInfo are provided in processor format. If the processor
**               format of HALC and HALCS is different then HAL shall ensure the
**               correct endianness. The endianness of pb_data is handled by the
**               CSAL and CSS.
**
** See Also    : HALCS_TxDataPutMix()
**
** Parameters  : ps_msg (IN)     - Pointer to the message structure to be sent,
**                                 see {HALCS_t_MSG}
**
** Returnvalue : CSS_k_TRUE      - Sending of message was successful
**               CSS_k_FALSE     - Error during message sending
**
*******************************************************************************/
CSS_t_BOOL HALCS_TxDataPut(const HALCS_t_MSG *ps_msg);


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
CSS_t_BOOL HALCS_TxDataPutMix(const HALCS_t_MSG *ps_msg);


/*******************************************************************************
**
** Function    : HALCS_ModStatLedSet
**
** Description : This function is used by the CSS to set the state of the Module
**               Status LED. CIP Safety devices are required to support a Module
**               Status LED. The state of the indicator is under control of the
**               Supervisor Object state machine. CSS will call this function
**               whenever the status of the Module Status LED changes. It is the
**               responsibility of this HALC function to implement the blink
**               patterns and to ensure that upon power on the LED is initially
**               in the "off" state.
**
** See Also    : HALCS_NetStatLedSet()
**
** Parameters  : e_msLedState (IN) - state to which the Module status shall be
**                                   set. See type definition of
**                                   {HALCS_t_LED_STATES}.
**
** Returnvalue : -
**
*******************************************************************************/
void HALCS_ModStatLedSet(HALCS_t_LED_STATES e_msLedState);


/*******************************************************************************
**
** Function    : HALCS_NetStatLedSet
**
** Description : This function is used by the CSS to set the state of the
**               Network Status LED. CIP Safety devices are required to support
**               a Network Status LED. The state of the indicator is under
**               control of the Connection Object state machine. CSS will call
**               this function whenever the status of the Network Status LED
**               changes. It is the responsibility of this HALC function to
**               implement the blink patterns and to ensure that upon power on
**               the LED is initially in the "off" state.
**
** See Also    : HALCS_ModStatLedSet()
**
** Parameters  : e_nsLedState (IN) - state to which the Network status shall be
**                                   set. See type definition of
**                                   {HALCS_t_LED_STATES}.
**
** Returnvalue : -
**
*******************************************************************************/
void HALCS_NetStatLedSet(HALCS_t_LED_STATES e_nsLedState);


#endif /* HALCS_API_H */

/*** End of File ***/


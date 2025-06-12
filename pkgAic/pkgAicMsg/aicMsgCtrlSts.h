/***************************************************************************************************
**    Copyright (C) 2016-2023 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCtrlSts.h
**     Summary: Interface header of module aicMsgCtrlSts.c
**   $Revision: 4087 $
**       $Date: 2023-09-27 17:51:15 +0200 (Mi, 27 Sep 2023) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgCtrlSts_Process
**             aicMsgCtrlSts_IsNewFragRcvd
**             aicMsgCtrlSts_IsAsmFragConf
**             aicMsgCtrlSts_BuildCtrlStsByte
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef AICMSGHDLCTRLSTS_H
#define AICMSGHDLCTRLSTS_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/*  states of ASM to be send via AIC. These states are the "main" states of
**  the ASM, but only used in AIC-part of SW and therefore defined here
**  see [SRS_2022]
*/
#define AICMSGHDL_ASM_STATE_BOOT               (UINT8)0x00u
/* 0x01 - reserved */
#define AICMSGHDL_ASM_STATE_STARTUP            (UINT8)0x02u
#define AICMSGHDL_ASM_STATE_WAIT_TUNID         (UINT8)0x03u
#define AICMSGHDL_ASM_STATE_WAIT_CONFIG        (UINT8)0x04u
#define AICMSGHDL_ASM_STATE_INVALID_CONFIG     (UINT8)0x05u
#define AICMSGHDL_ASM_STATE_IDLE               (UINT8)0x06u
/* 0x07 - reserved */
#define AICMSGHDL_ASM_STATE_EXEC_PROD_ONLY     (UINT8)0x08u
#define AICMSGHDL_ASM_STATE_EXEC_CONS_ONLY     (UINT8)0x09u
#define AICMSGHDL_ASM_STATE_EXEC_PROD_AND_CONS (UINT8)0x0Au
/* 0x0B - reserved */
#define AICMSGHDL_ASM_STATE_WAIT_RESET         (UINT8)0x0Cu
#define AICMSGHDL_ASM_STATE_ABORT              (UINT8)0x0Du
/* 0x0E - reserved */
#define AICMSGHDL_ASM_STATE_ERROR              (UINT8)0x0Fu

/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* aicMsgCtrlSts_u8_AsmCtrlSts:
** control/status of ASM message, global used due to performance reasons,
** init value fulfills also [SRS_318], see [SIS_026]
** formerly aicDataStack_u8AsmCtrlStatus
*/
extern UINT8 aicMsgCtrlSts_u8_AsmCtrlSts;

/* aicMsgCtrlSts_u8_AmCtrlSts:
** control/status of AM message, global used due to performance reasons
** formerly aicDataStack_u8AnybusCtrlStatus
*/
/* start with zero, see [SRS_318], see [SIS_026] */
extern UINT8 aicMsgCtrlSts_u8_AmCtrlSts;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_Process

  Description:
    The function stores the received AM control/status internally since it is possible that between
    processing AM message and sending ASM message a new telegram with (new AM control/status) is
    received.

    Furthermore the Anybus State field of the received AM Control/status byte is evaluated and
    the evaluated anybus state request is signalized to the internal state machine (see [SRS_2145], 
    [SRS_707], [SIS_068]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    -

***************************************************************************************************/
void aicMsgCtrlSts_Process (void);

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_IsNewFragRcvd

  Description:
    Returns TRUE if the received Anybus fragmentation bit was toggled, else FALSE.
    The principle of fragmentation is explained in Serial Interface Specification [SIS] and in 
    the Software Requirements [SRS].

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE                     - the received Anybus fragmentation bit was toggled
    FALSE                    - not toggled

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCtrlSts_IsNewFragRcvd (void);

/***************************************************************************************************
  Function:
    aicMsgCtrlSts_IsAsmFragConf

  Description:
    returns TRUE if the received ASM fragmentation bit was confirmed by ABCC/AM, else FALSE
    The principle of fragmentation is explained in Serial Interface Specification [SIS] and in 
    the Software Requirements [SRS]

  See also:
    -

  Parameters:
    -

  Return value:
    BOOL        TRUE if the received ASM fragmentation bit has the same
                value as the one send, else FALSE

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgCtrlSts_IsAsmFragConf (void);


/***************************************************************************************************
  Function:
    aicMsgCtrlSts_BuildCtrlStsByte

  Description:
    Builds/Prepares the Ctrl/Status Byte which is send as first Byte of every TX telegram. The 
    status of the ASM is send in lower 4 Bits of Ctrl/Status Byte, two fragmentation bits are 
    located at Bit7 and Bit6.
    Steps executed in function:
    - get the current fragmentation bits (Depending on the input parameter "b_toggleAsmFrg" the 
      ASM fragmentation bit is toggled later in function)
    - resets the "state" part of the Ctrl/Status Byte to 0
    - Set the two fragmentation bits:
      - AM fragmentation bit is set depending on function parameter (the last received
        Anybus fragmentation bit)
      - ASM fragmentation bit is set depending on function parameter (toggled)
    - sets the "state" part of the Ctrl/Status Byte according to current state
      (the "ASM state" is set depending on AIC state)

    Hint: Message fragmentation is described in Serial Interface Specification
          [SIS] and Software Requirements Specification[SRS]

  See also:
    -

  Parameters:
    b_toggleAsmFrg (IN)    - depending on this input parameter ASM fragmentation bit
                             will be toggled (see [SRS_361])
                             (valid range: TRUE, FALSE, not checked)

    b_amFragConf (IN)      - depending on this input parameter the AM fragmentation bit is set
                             (valid range: eTRUE, eFALSE, checked)
  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void aicMsgCtrlSts_BuildCtrlStsByte (CONST BOOL b_toggleAsmFrg,
                                     CONST TRUE_FALSE_ENUM b_amFragConf);
#endif


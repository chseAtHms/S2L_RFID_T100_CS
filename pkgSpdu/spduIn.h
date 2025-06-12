/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduIn.h
**     Summary: Interface header of the module 'spduIn'
**   $Revision: 3166 $
**       $Date: 2017-05-31 14:41:15 +0200 (Mi, 31 Mai 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: spduIn_Init
**             spduIn_BuildTgm
**             spduIn_BuildTgmDummy
**             spduIn_ResetTgmIo
**             spduIn_ResetTgmTcco
**             spduIn_SendSafeTgm
**             spduIn_RecvSafeTgm
**             spduIn_PrepIpcIo
**             spduIn_PrepIpcTcoo
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SPDUIN_H
#define SPDUIN_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* spduIn_s_Paket:
** location/array to store the outgoing safe packet (safety module to safe PLC),
** global used due to performance reasons,
** Formerly aicDataStackSpdu_sTxSpduInPaket
*/
extern AICMSGDEF_SPDU_TX_STRUCT spduIn_s_Paket;

/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    spduIn_Init

  Description:
    This function is used to initialize the module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void spduIn_Init(void);

/***************************************************************************************************
  Function:
    spduIn_BuildTgm

  Description:
    The main function is to build the safe packet according to the safety protocol specification.
    For this purpose the functions takes the safe fragment build by the own controller and the 
    fragment build by the other controller and generates the safety packet. So it is ensured that
    a safety packet is never build by only one controller (see [SRS_2030]). The fragments are 
    exchanged via IPC previously. 
    Because it is possible, that one controller was able to build a fragment but the other 
    controller not, the DUI (Data Update Indicator) is used to detect if both controllers have 
    exchanged the same frame fragment. If the DUI is different, the safe packet is NOT build. 
    That means in that case the old data is transferred again to AM.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_BuildTgm (void);

/***************************************************************************************************
  Function:
    spduIn_BuildTgmDummy

  Description:
    Build an 'empty', not valid safe packet according to [SIS_CS].
    This function is used only when the SW is in a condition where a complete Telegram via AIC has
    to be send, but without "real" and valid safety communication (means no CIP Safety Connection 
    established).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_BuildTgmDummy (void);

/***************************************************************************************************
  Function:
    spduIn_ResetTgmIo

  Description:
    This function is used to set the IO Data Message part of the T100 SPDU to default 
    values according to [SIS_CS]. This means
      - HALC length is set to AICMSGDEF_CIP_IN_DATA_SIZE (14 bytes)
      - HALC addInfo is set to 0xFFFF
      - data is filled with 0xFE

    This function is called whenever the Producing Connection is closed or faulted.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_ResetTgmIo (void);

/***************************************************************************************************
  Function:
    spduIn_ResetTgmTcco

  Description:
    This function is used to set the Time Coordination Message part of the T100 SPDU to default 
    values according to [SIS_CS]. This means
      - HALC addInfo2 is set to 0xFFFF
      - data2 is filled with 0xFE

    This function is called whenever the Consuming Connection is closed or faulted.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_ResetTgmTcco (void);

/***************************************************************************************************
  Function:
    spduIn_SendSafeTgm

  Description:
    This function is used to send the subframes generated by the IXXAT CIP Safety Stack to the 
    other controller (via IPC) in order to build a complete frame (see [SRS_2150], [2.1-10:], 
    [2.1-5:]). Depending on the controller id either the even or the odd bytes are sent to the 
    other uc.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_SendSafeTgm(void);

/***************************************************************************************************
  Function:
    spduIn_RecvSafeTgm

  Description:
    This function is used to receive the subframes generated by the IXXAT CIP Safety Stack from the 
    other controller (via IPC) in order to build a complete frame (see [SRS_2150], [2.1-10:], 
    [2.1-5:]).

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduIn_RecvSafeTgm(void);

/***************************************************************************************************
  Function:
    spduIn_PrepIpcIo

  Description:
    This function is used to prepare the IPC structure (IO Data Message part) after a new IO Data 
    Message was generated by CSS. The structure is used to exchange the sub frames with the other 
    controller. The IPC transfer itself is triggered via spduIn_SendSafeTgm.

  See also:
    spduIn_SendSafeTgm

  Parameters:
    u32_addInfo (IN) - Additional info generated by CSS
                       (valid range: any, not checked)
    au8_ioData (IN)  - Subframe buffer (incomplete message) generated by CSS
                       (valid range: <>NULL, not checked, buffer provided by CSS)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void spduIn_PrepIpcIo (UINT32 u32_addInfo, const UINT8* au8_ioData);

/***************************************************************************************************
  Function:
    spduIn_PrepIpcTcoo

  Description:
    This function is used to prepare the IPC structure (Time Coordination Message part) after a new
    Time Coordination Message was generated by CSS. The structure is used to exchange the sub frames
    with the other controller. The IPC transfer itself is triggered via spduIn_SendSafeTgm.

  See also:
    spduIn_SendSafeTgm

  Parameters:
    u32_addInfo (IN) - Additional info generated by CSS
                       (valid range: any, not checked)
    au8_tcoo (IN)    - Subframe buffer (incomplete message) generated by CSS
                       (valid range: <>NULL, not checked, buffer provided by CSS)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void spduIn_PrepIpcTcoo (UINT32 u32_addInfo, const UINT8* au8_tcoo);

#endif


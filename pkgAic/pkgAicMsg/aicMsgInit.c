/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgInit.c
**     Summary: This module handles the SetInitData requests received via AIC.
**   $Revision: 2387 $
**       $Date: 2017-03-17 12:38:18 +0100 (Fr, 17 Mrz 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: aicMsgInit_SetData
**             aicMsgInit_GetData
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"

/* module includes */
#include "globPreChk.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "rds.h"

#include "stdlib-hal.h"

#include "aicMsgDef.h"
#include "aicMsgInit.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      7u

/* Init-value (RDS_UINT8) of local message counter */
#define k_MSGCNT_SETINITDATA_INIT           {{(UINT8)0u},{(UINT8)~(UINT8)0u}}

/* Sizes of SetConfigString fragments:
** first  fragment: 12 bytes payload
** second fragment:  6 bytes payload
*/
/* 2nd and last fragment */
#define k_CSSINITDATA_LAST_FRAG_LEN         (6u)

/** PRE-PROCESSOR CHECK **/ 
/* Ensure that First and Second (Last) fragment sizes match to the global Message definitions
** and to the used structure definition 
*/
/* Deactivated Lint Note 948: Operator '==' always evaluates to True */
/*lint -e(948) */
IXX_TASSERT(sizeof(AICMSGINIT_CSSINITDATA)  ==
            (AICMSGDEF_ASM_MSG_DATA_SIZE + \
            k_CSSINITDATA_LAST_FRAG_LEN), aicMsgInit_chk1)

/* Mute lint warning type not referenced. These types are just defined for    */
/* being able to check the size of the elementary types.                      */
/*lint -esym(751, IXX_DUMMY_aicMsgInit_chk1) not referenced - just for checks */



/* CSS initialization data received during 'SetInitData' request */
STATIC volatile AICMSGINIT_CSSINITDATA s_CssInitData =
  {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/
  

/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    aicMsgInit_SetData

  Description:
    To be called when a message request containing CSS initialization data is received.
    'SetInitData' requests are "fragmented" messages, means more than one telegram is needed to 
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
    TRUE                     - SetInitData received completely
    FALSE                    - Still fragments to receive

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
BOOL aicMsgInit_SetData (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg)
{
  /* RDS union initializations */
  /* Deactivated Lint message 708, because a Union initialisation is wanted to save
  ** init function */
  LOCAL_STATIC(, RDS_UINT8,  u8_FragCntInitData, k_MSGCNT_SETINITDATA_INIT); /*lint !e708*/

  BOOL b_initDataRcvd = FALSE;

  /* if first fragment */
  if (0u == RDS_GET(u8_FragCntInitData) )
  {
    /* first of two fragments (SetInitData fixed length of 22 Bytes (incl. header) in total,
     * see [SIS_CS])
     * In the first fragment, the header is NOT used for the payload */
    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* lint Note 928 deactivated, pointer casting is OK! */
    stdlibHAL_ByteArrCopy((volatile UINT8*)&s_CssInitData,       /*lint !e928*/
                          &ps_rxMsg->au8AicMsgData[0],
                          (UINT16)AICMSGDEF_ASM_MSG_DATA_SIZE);
    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    /* increase number of received fragments */
    RDS_INC(u8_FragCntInitData);

    /* 'SetInitData' nor completely received, still fragment missing */
    /* return value already set */
  }
  /* else if : second fragment */
  else if ( 1u == RDS_GET(u8_FragCntInitData) )
  {
    /* second and last fragment (SetInitData fixed length of 22 Bytes (incl. header) in total),
     * now the "data only" buffer is used, because in this case it was a
     * message without header */
    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* lint Note 928 deactivated, pointer casting is OK! */
    stdlibHAL_ByteArrCopy((volatile UINT8*)&s_CssInitData +      /*lint !e928*/
                             AICMSGDEF_ASM_MSG_DATA_SIZE, 
                          (CONST UINT8*)ps_rxMsg,                /*lint !e928*/
                          (UINT16)k_CSSINITDATA_LAST_FRAG_LEN);
    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    
    /* reset fragment counter variable */
    RDS_SET(u8_FragCntInitData, 0u);
    
    /* 'SetInitData' completely received */
    b_initDataRcvd = TRUE;
  }
  /* else: invalid fragment */
  else
  {
    /* should not enter here, because only 2 fragments expected */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  return b_initDataRcvd;
}

/***************************************************************************************************
  Function:
    aicMsgInit_GetData

  Description:
    This function is used to read the CSS initialization data that was received during startup 
    from ABCC.

  See also:
    -

  Parameters:
    -

  Return value:
    AICMSGINIT_CSSINITDATA*  - Pointer of CSS initialization data

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST volatile AICMSGINIT_CSSINITDATA* aicMsgInit_GetData(void)
{
  return &s_CssInitData;
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/


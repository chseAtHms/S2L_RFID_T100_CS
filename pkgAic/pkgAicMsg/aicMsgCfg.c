/***************************************************************************************************
**    Copyright (C) 2016-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: aicMsgCfg.c
**     Summary: This module handles the SetConfigString requests received via AIC.
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
#include "aicMsgCfg.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      8u

/* Init-value (RDS_UINT8) of local message counter */
#define k_MSGCNT_SETCFGSTR_INIT               {{(UINT8)0u},{(UINT8)~(UINT8)0u}}

/* size of last SetConfigString fragment
** first  fragment: 12 bytes payload
** second fragment: 16 bytes payload 
** third/last fragment:  8 bytes payload */
#define k_CFGSTRDATA_LAST_FRAG_LEN            (8u)

/** PRE-PROCESSOR CHECK **/
/* Ensure that the fragment sizes match to the global Message definitions and to the used structure
** definition.
*/
/* Deactivated Lint Note 948: Operator '==' always evaluates to True */
/*lint -e(948) */
IXX_TASSERT(sizeof(AICMSGCFG_SETCONFIGSTRDATA) == \
            (AICMSGDEF_ASM_MSG_DATA_SIZE + \
            sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT) + \
            k_CFGSTRDATA_LAST_FRAG_LEN), aicMsgCfg_chk1)

/* Mute lint warning type not referenced. These types are just defined for    */
/* being able to check the size of the elementary types.                      */
/*lint -esym(751, IXX_DUMMY_aicMsgCfg_chk1) not referenced - just for checks */


/* Received SetConfigString data */
STATIC volatile AICMSGCFG_SETCONFIGSTRDATA s_CfgStrData =
{
  /* Software Major version */
  0u,
  
  /* Hardware ID T100/CS */
  0u,
  
  /* IO Configuration Data set */
  { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
    0u, 0u, 0u, 0u,
  },

  /* SCID (Safety Configuration CRC) */
  0u,

  /* SCID (Safety Configuration Time Stamp) */
  0u,

   /* SCID (Safety Configuration Time Stamp) */
  0u
};

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
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
BOOL aicMsgCfg_SetData (CONST AICMSGDEF_NON_SAFE_MSG_STRUCT* ps_rxMsg)
{
  /* RDS union initializations */
  /* Deactivated Lint message 708, because a Union initialisation is wanted to save
  ** init function */
  LOCAL_STATIC(, RDS_UINT8,  u8_FragCntCfgStr, k_MSGCNT_SETCFGSTR_INIT); /*lint !e708*/

  BOOL b_cfgDataRcvd = FALSE;

  /* if first fragment */
  if (0u == RDS_GET(u8_FragCntCfgStr) )
  {
    /* first of three fragments (SetConfigString has fixed length of 40 Bytes
     * (incl. header) in total, see [SIS])
     * In the first fragment, the header is NOT used for the payload,
     * so only 12 bytes are usable */
    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* lint Note 928 deactivated, pointer casting is OK! */
    stdlibHAL_ByteArrCopy((volatile UINT8*)&s_CfgStrData,     /*lint !e928*/
                          &ps_rxMsg->au8AicMsgData[0],
                          (UINT16)(AICMSGDEF_ASM_MSG_DATA_SIZE));
    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    
    /* increase number of received fragments */
    RDS_INC(u8_FragCntCfgStr);
    
    /* Configuration Data not completely received, still fragments missing */
    /* return value already set */
  }
  else if (1u == RDS_GET(u8_FragCntCfgStr) )
  {
    /* second of three fragments (SetConfigString has fixed length of 40 Bytes
     * (incl. header) in total, see [SIS])
     * now the "data only" buffer is used, because in this case it was a message
     * without header, so 16 bytes are usable */

    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* lint Note 928 deactivated, pointer casting is OK! */
    stdlibHAL_ByteArrCopy((volatile UINT8*)&s_CfgStrData +    /*lint !e928*/
                            AICMSGDEF_ASM_MSG_DATA_SIZE,
                          (CONST UINT8*)ps_rxMsg,             /*lint !e928*/
                          (UINT16)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT));
    /*lint +esym(960, 17.1) +esym(960, 17.4) */

    /* increase number of received fragments */
    RDS_INC(u8_FragCntCfgStr);
    
    /* Configuration Data not completely received, still fragments missing */
    /* return value already set */
  }
  else if (2u == RDS_GET(u8_FragCntCfgStr) )
  {
    /* third and last fragment (SetConfigString fixed length of 40 Bytes
     * incl. header) in total),
     * now the "data only" buffer is used, because in this case it was a
     * message without header */

    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* lint Note 928 deactivated, pointer casting is OK! */
    stdlibHAL_ByteArrCopy((volatile UINT8*)&s_CfgStrData +      /*lint !e928*/
                           AICMSGDEF_ASM_MSG_DATA_SIZE +
                           (UINT8)sizeof(AICMSGDEF_NON_SAFE_MSG_STRUCT),
                          (CONST UINT8*)ps_rxMsg,               /*lint !e928*/
                          (UINT16)k_CFGSTRDATA_LAST_FRAG_LEN);
    /*lint +esym(960, 17.1) +esym(960, 17.4) */

    /* reset fragment counter variable */
    RDS_SET(u8_FragCntCfgStr, 0u);
    
    /* Configuration Data completely received */
    b_cfgDataRcvd = TRUE;
  }
  else
  {
    /* should not enter here, because only 3 fragments expected */
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  return b_cfgDataRcvd;
}

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
CONST volatile AICMSGCFG_SETCONFIGSTRDATA* aicMsgCfg_GetData(void)
{
  return &s_CfgStrData;
}


/***************************************************************************************************
**    static functions
***************************************************************************************************/

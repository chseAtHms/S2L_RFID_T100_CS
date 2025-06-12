/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doState_priv.h
**
** $Id: doState_priv.h 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
** $Author: klan $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** local include file of "doState.c", contains static and file wide definitions
** which are not public
**
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 und STYL-2-3-20.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef DOSTATE_PRIV_H_
#define DOSTATE_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
#define DOSTATE_ERR_RESET_INIT {  eFALSE , \
                                  eFALSE  }
#if GPIOCFG_NUM_DO_TOT != 2u
  #error "define DOSTATE_ERR_RESET_INIT must be adapted to new DO number!"
#endif

/*------------------------------------------------------------------------------
** maximum active cycles until an active DO shall have been tested (used for
** timeout check of HS test)
**-----------------------------------------------------------------------------*/
#define DOSTATE_MAX_ACTIVE_CYC_TILL_TEST 75000u /* 75000 = 30s with a 400us call cycle*/

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** typedef for structure containing:
** - state of outputs
**------------------------------------------------------------------------------
*/
typedef struct DO_STATE_TAG
{
   DO_STATE_ENUM eState[GPIOCFG_NUM_DO_TOT];            /* state of output  */
   RDS_UINT32    u32ActiveStateCnt[GPIOCFG_NUM_DO_TOT]; /* timer/counter for active state */
} DO_STATE_STRUCT;


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

STATIC void doState_ActiveState(CONST UINT8 u8DoNum);
STATIC void doState_InactiveState(CONST UINT8 u8DoNum);
STATIC void doState_ErrState(CONST UINT8 u8DoNum);
STATIC void doState_EvalNextState(CONST UINT8 u8DoNum);
STATIC void doState_IncrActiveStateCnt(CONST UINT8 u8DoNum);
STATIC void doState_RstActiveStateCnt(CONST UINT8 u8DoNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doState_priv.h
**
********************************************************************************
*/

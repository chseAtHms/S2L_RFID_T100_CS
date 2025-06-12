/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diState_priv.h
**
** $Id: diState_priv.h 569 2016-08-31 13:08:08Z klan $
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
** local include file of "diState.c", contains static and file wide definitions
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

#ifndef DISTATE_PRIV_H_
#define DISTATE_PRIV_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

#define DISTATE_ERR_RESET_INIT { eFALSE, \
                                 eFALSE, \
                                 eFALSE, \
                                 eFALSE, \
                                 eFALSE, \
                                 eFALSE}
#if GPIOCFG_NUM_DI_TOT != 6u
  #error "define DISTATE_ERR_RESET_INIT must be adapted to new DI number!"
#endif

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** typedef for structure containing:
** - state of inputs
**------------------------------------------------------------------------------
*/
typedef struct DI_STATE_TAG
{
   DI_STATE_ENUM eState[GPIOCFG_NUM_DI_TOT];     /* contains state of every input  */
} DI_STATE_STRUCT;


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

STATIC void diState_ActiveState(CONST UINT8 u8DiNum);
STATIC void diState_InactiveState(CONST UINT8 u8DiNum);
STATIC void diState_ErrState(CONST UINT8 u8DiNum);
STATIC void diState_ActiveWaitTestState(CONST UINT8 u8DiNum);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diState_priv.h
**
********************************************************************************
*/

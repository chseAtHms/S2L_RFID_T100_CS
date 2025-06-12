/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diState.h
**
** $Id: diState.h 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
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
** header file (public) of diState.c
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

#ifndef DISTATE_H_
#define DISTATE_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/



/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  enum specifying the possible DI states
**------------------------------------------------------------------------------
*/
typedef enum
{
   /*!< input inactive */
   eDI_STATE_INACTIVE                  = 0x3f48u,   
   /*!< input read as active, but not tested (and not compared in dual channel) */
   eDI_STATE_ACTIVE_WAIT_TEST          = 0x40d6u,   
   /*!< input active and result tested (and compared to other channel) */
   eDI_STATE_ACTIVE_TESTED_AND_VALID   = 0x4141u,   
   /*!< internal error, DI-tests found an error at input */
   eDI_STATE_ERROR                     = 0x43f8u,   
   /*!< DI not used at all */
   eDI_STATE_DONT_USE                  = 0x4433u    
} DI_STATE_ENUM;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** array containing:
** requested error reset requests coming from SPDU.
** Global variable is used due to performance issues (function style would
** take more time and additional parameter checks would be necessary)
** Access to this variable only in "diState.c" and "diLib.c".
**------------------------------------------------------------------------------
*/
extern TRUE_FALSE_ENUM diState_DiErrRstSPDU[GPIOCFG_NUM_DI_TOT];


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void diState_ProcessDIs(void);
extern void diState_Init(void);
extern GPIO_STATE_ENUM diState_GetDiState(CONST UINT8 u8DiNum);
extern void diState_HandleToTestExecution(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diState.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diLib.h
**
** $Id: diLib.h 569 2016-08-31 13:08:08Z klan $
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
** header file of "diLib.c"
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

#ifndef DILIB_H_
#define DILIB_H_


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


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void diLib_Init(void);
extern void diLib_CheckDiDiagTimeout(void);
extern void diLib_SetErrRstFlags(CONST UINT8 u8DiErrRstByte);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diLib.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diInput_priv.h
**
** $Id: diInput_priv.h 2448 2017-03-27 13:45:16Z klan $
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
** local include file of "diInput.c", contains static and file wide definitions
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

#ifndef DIINPUT_PRIV_H_
#define DIINPUT_PRIV_H_


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
** type-definition of structure of one buffer element
**------------------------------------------------------------------------------
*/
typedef struct IN_BUFF_LOC_TAG
{
   GPIOSAMPLE_DI_BUFFER_INDEX_PORT  ePortBuffIndex; /* index of port inside the input read buffer */
   UINT16 u16PinMask;                               /* pinmask of input inside a port */
} DIINPUT_BUFF_LOC_STRUCT;




/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/
STATIC void diInput_UpdateDiValuesOwnCh(void);

STATIC void diInput_FilterDiValues(void);
STATIC void diInput_FilterEnDoValues(void);
STATIC void diInput_FilterTestDoValues(void);

STATIC void diInput_PrepareDiValuesForIpc(void);
STATIC void diInput_PrepareTestQualForIpc(void);

STATIC void diInput_ConvertDiValuesFromOtherCh(void);
STATIC void diInput_ConvertTestQualFromOtherCh(void);


#endif  /* inclusion lock */


/*******************************************************************************
**
** End of diInput_priv.h
**
********************************************************************************
*/

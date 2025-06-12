/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diInput.h
**
** $Id: diInput.h 2448 2017-03-27 13:45:16Z klan $
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
** include file of diInput.c
** contains functions/macros to access the input values
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

#ifndef DIINPUT_H_
#define DIINPUT_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

#define DIINPUT_ADD_DI_C_FILTERCYCLES   ((UINT8)(2u))

#define DIINPUT_ADD_DI_S_FILTERCYCLES   ((UINT8)(2u))

/*------------------------------------------------------------------------------
**  init value used for local static high level filter
**------------------------------------------------------------------------------
*/
#define DI_DEB_HL_FILT_INIT  { {{(UINT16)0u}, {(UINT16)~(UINT16)0u}}, \
                               {{(UINT16)0u}, {(UINT16)~(UINT16)0u}}, \
                               {{(UINT16)0u}, {(UINT16)~(UINT16)0u}} }
#if GPIOCFG_NUM_DI_TOT > 6u
/* RSM_IGNORE_QUALITY_BEGIN Notice #1    - Physical line length > 100 characters */
  #error "define DI_DEB_HL_FILT_INIT must be adapted to new DI number! (init values for RDS-Variable!)"
/* RSM_IGNORE_QUALITY_END */
#endif

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  type-def for structure for the filtered values (filtered results of Mikro-Read)
**  of all inputs to read at one uC
**------------------------------------------------------------------------------
*/
typedef struct DIINPUT_VALUES_TAG
{
   GPIO_STATE_ENUM eDi1stFilt[GPIOCFG_NUM_DI_PER_uC];   /* DI result of first 2oo3 filter */
   GPIO_STATE_ENUM eDi2ndFilt[GPIOCFG_NUM_DI_PER_uC];   /* DI result of x 2oo3 filter */
   GPIO_STATE_ENUM eDoTest[GPIOCFG_NUM_TESTDO];         /* filter result of DO-Test pin */
   GPIO_STATE_ENUM eEnDo[GPIOCFG_NUM_ENDO];             /* filter result of EN_DO pin */
} DIINPUT_VALUES_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of digital "safe" inputs as Bit
**  information (used to send values via IPC).
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern UINT8 diInput_u8DiBitValues;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of digital "safe" inputs received
**  from other channel as Bit information (received via IPC)
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern UINT8 diInput_u8DiBitValuesOtherCh;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of the test qualifier of digital
**  "safe" inputs as Bit information (used to send values via IPC).
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern UINT8 diInput_u8DiTestQualValues;

/*------------------------------------------------------------------------------
**  Structure/Variable to store the values of the test qualifiers received
**  from other channel as Bit information (received via IPC).
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern UINT8 diInput_u8DiTestQualValuesOtherCh;

/*------------------------------------------------------------------------------
**  Structure/Variable for the filtered values (filtered results of Mikro-Read)
**  of all inputs to read at one uC.
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern DIINPUT_VALUES_STRUCT diInput_sInputValues;

/*------------------------------------------------------------------------------
**  Array containing the resulting state of Digital "safe" inputs of both
**  channels (filtered values stored with enum values).
**  Global used due to performance reasons.
**------------------------------------------------------------------------------
*/
extern GPIO_STATE_ENUM diInput_eDiValue[GPIOCFG_NUM_DI_TOT];



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Macros to get the filtered input values directly from array and variables,
** done via macro instead of function due to performance reasons (time).
** Parameter shall be checked or safe in function where macros are used!
**------------------------------------------------------------------------------
*/
#define DI_INPUTVAL_DI(diNum)                \
  ((GPIO_STATE_ENUM)(diInput_eDiValue[diNum]))
#define DIINPUT_DI_OWN_FIRST_FILTER(diNum)   \
  ((GPIO_STATE_ENUM)(diInput_sInputValues.eDi1stFilt[diNum]))
#define DIINPUT_DI_OWN_SCND_FILTER(diNum)    \
  ((GPIO_STATE_ENUM)(diInput_sInputValues.eDi2ndFilt[diNum]))
#define DI_INPUTVAL_TESTDO(testDoNum)        \
  ((GPIO_STATE_ENUM)(diInput_sInputValues.eDoTest[testDoNum]))


extern void diInput_Init(void);

extern void diInput_FilterDoRelInputs(void);
extern void diInput_FilterDiRelInputs(void);

extern void diInput_PrepareValForIPC(void);
extern void diInput_ConvValFromOtherCh(void);




#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diInput.h
**
********************************************************************************
*/

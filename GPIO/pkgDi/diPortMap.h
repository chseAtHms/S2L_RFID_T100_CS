/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** diPortMap.h
**
** $Id: diPortMap.h 4067 2023-09-18 12:08:41Z ankr $
** $Revision: 4067 $
** $Date: 2023-09-18 14:08:41 +0200 (Mo, 18 Sep 2023) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** header file of "diPortMap.c", contains defines for all input ports
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

#ifndef DIPORTMAP_H_
#define DIPORTMAP_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** definition/assignment of pins to read
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** mapping of digital (save) inputs
**------------------------------------------------------------------------------
*/
/* input1 / DI 1 */
#define DIPORTMAP_PORT_DI1                (GPIOA)
#define DIPORTMAP_BUFFER_INDEX_PORT_DI1   eBUFFER_INDEX_PORTA
#define DIPORTMAP_PINMASK_DI1             GPIO_PINMASK_0
#define DIPORTMAP_PIN_DI1                 GPIO_0
/* input2 / DI 2 */
#define DIPORTMAP_PORT_DI2                (GPIOA)
#define DIPORTMAP_BUFFER_INDEX_PORT_DI2   eBUFFER_INDEX_PORTA
#define DIPORTMAP_PINMASK_DI2             GPIO_PINMASK_1
#define DIPORTMAP_PIN_DI2                 GPIO_1
/* input3 / DI 3 */
#define DIPORTMAP_PORT_DI3                (GPIOA)
#define DIPORTMAP_BUFFER_INDEX_PORT_DI3   eBUFFER_INDEX_PORTA
#define DIPORTMAP_PINMASK_DI3             GPIO_PINMASK_2
#define DIPORTMAP_PIN_DI3                 GPIO_2

/*------------------------------------------------------------------------------
** definition/assignment of pins to read (2.: read back pins of outputs)
** mapping of read-back-inputs for digital (save) outputs (Test_DO signals in schematic)
** can be found in doPortMap, here only the location inside the input buffer is
** defined
**------------------------------------------------------------------------------
*/
/* Test_DO1 */
#define DIPORTMAP_BUFFER_INDEX_PORT_TEST_DOs1   eBUFFER_INDEX_PORTB
/* Test_DO2 */
#define DIPORTMAP_BUFFER_INDEX_PORT_TEST_DOs2   eBUFFER_INDEX_PORTB


/*------------------------------------------------------------------------------
** mapping of enable-output signals coming from voltage supervision
** (EN_DO... signals in schematic)
** mapping can be found in doPortMap, here only the location inside the input buffer is
** defined
**------------------------------------------------------------------------------
*/
/* EN_DO */
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
  #define DIPORTMAP_BUFFER_INDEX_PORT_EN_DO   eBUFFER_INDEX_PORTC
#else
  #define DIPORTMAP_BUFFER_INDEX_PORT_EN_DO   eBUFFER_INDEX_PORTB
#endif


/*------------------------------------------------------------------------------
** mapping of TO diagnosis read-back signal (Diag_TO in schematic)
**------------------------------------------------------------------------------
*/



/*------------------------------------------------------------------------------
** definition/assignment of pins used for diagnostics (outputs)
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** mapping of TI_Level pins
**------------------------------------------------------------------------------
*/
#define DIPORTMAP_PORT_TI_LEVEL_DI1      (GPIOA)
#define DIPORTMAP_PINMASK_TI_LEVEL_DI1   GPIO_PINMASK_4
#define DIPORTMAP_PIN_TI_LEVEL_DI1       GPIO_4

#define DIPORTMAP_PORT_TI_LEVEL_DI2      (GPIOA)
#define DIPORTMAP_PINMASK_TI_LEVEL_DI2   GPIO_PINMASK_5
#define DIPORTMAP_PIN_TI_LEVEL_DI2       GPIO_5

#define DIPORTMAP_PORT_TI_LEVEL_DI3      (GPIOA)
#define DIPORTMAP_PINMASK_TI_LEVEL_DI3   GPIO_PINMASK_6
#define DIPORTMAP_PIN_TI_LEVEL_DI3       GPIO_6


/*------------------------------------------------------------------------------
** mapping of TO (Test Output) pins
**------------------------------------------------------------------------------
*/
#define DIPORTMAP_PORT_TO1       (GPIOC)
#define DIPORTMAP_PINMASK_TO1    GPIO_PINMASK_8
#define DIPORTMAP_PIN_TO1        GPIO_8


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/


/*------------------------------------------------------------------------------
** typedef for output properties (outputs used to set test signals inside the
** input test functions)
**------------------------------------------------------------------------------
*/
typedef struct INPT_OUT_PORT_SETTINGS_TAG
{
   GPIO_TypeDef* psPort;    /* port of pin */
   UINT16 u16Pinmask;        /* bit location inside port of pin */
} DIPORTMAP_INPT_OUT_PORT_SETTINGS_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** CONST mapping of TI-Level diagnostic outputs (TI_Level in schematic).
** Global const used due to performance reasons
**------------------------------------------------------------------------------
*/
extern CONST DIPORTMAP_INPT_OUT_PORT_SETTINGS_STRUCT diPortMap_sTiLevelMap[GPIOCFG_NUM_DI_PER_uC];

/*------------------------------------------------------------------------------
** CONST mapping of TOs (diagnostic outputs generating pulse signal,
** "TO" in schematic))
** Global const used due to performance reasons
**------------------------------------------------------------------------------
*/
extern CONST DIPORTMAP_INPT_OUT_PORT_SETTINGS_STRUCT diPortMap_sToMap[GPIOCFG_NUM_TO];


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void diPortMap_Init(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of diPortMap.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** doPortMap.h
**
** $Id: doPortMap.h 4067 2023-09-18 12:08:41Z ankr $
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
** header file of "doPortMap.c"
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

#ifndef DOPORTMAP_H
#define DOPORTMAP_H


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** definition/assignment of port pins used
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** mapping of life signal
**------------------------------------------------------------------------------
*/
#define PORT_LIFE_SIG       GPIOB
#define PINMASK_LIFE_SIG    GPIO_PINMASK_8
#define PIN_LIFE_SIG        GPIO_8
#define LIFE_SIG_CFG        OUT_CONFIG_GPIO_PP
#define LIFE_SIG_MODE       OUT_MODE_50MHZ

/*------------------------------------------------------------------------------
** mapping of Test 3V3 signal
**------------------------------------------------------------------------------
*/
#define PORT_TEST_3V3       GPIOD
#define PINMASK_TEST_3V3    GPIO_PINMASK_2
#define PIN_TEST_3V3        GPIO_2
#define TEST_3V3_CFG        OUT_CONFIG_GPIO_PP
#define TEST_3V3_MODE       OUT_MODE_2MHZ


/*------------------------------------------------------------------------------
** definition/assignment of output pins
**------------------------------------------------------------------------------
*/

/*------------------------------------------------------------------------------
** mapping of digital (save) outputs
**------------------------------------------------------------------------------
*/
/* output1 / DO 1 */
#define PORT_DO1            GPIOB
#define PINMASK_DO1         GPIO_PINMASK_5
#define PIN_DO1             GPIO_5
#define DO1_CFG             OUT_CONFIG_GPIO_PP
#define DO1_MODE            OUT_MODE_50MHZ

/* for future HW */
#if (GPIOCFG_NUM_DO_PER_uC>1u)
  /* output2 / DO2 */
  #define PORT_DO2
  #define PINMASK_DO2
  #define PIN_DO2
  #define DO2_CFG           OUT_CONFIG_GPIO_PP
  #define DO2_MODE          OUT_MODE_50MHZ
#endif

/* for future HW */
#if (GPIOCFG_NUM_DO_PER_uC>2u)
  /* output3 / DO3 */
  #define PORT_DO3
  #define PINMASK_DO3
  #define PIN_DO3
  #define DO3_CFG           OUT_CONFIG_GPIO_PP
  #define DO3_MODE          OUT_MODE_50MHZ
#endif

/*------------------------------------------------------------------------------
** mapping of discharge outputs
**------------------------------------------------------------------------------
*/
#define PORT_DISCHARGE_DO1     GPIOC
#define PINMASK_DISCHARGE_DO1  GPIO_PINMASK_15
#define PIN_DISCHARGE_DO1      GPIO_15
#define DISCHARGE_CFG_DO1      OUT_CONFIG_GPIO_PP
#define DISCHARGE_MODE_DO1     OUT_MODE_50MHZ

/*------------------------------------------------------------------------------
** mapping of Test DO inputs (Test_DO signals in schematic)
** read-back-inputs for digital (save) outputs
** there is one for every DO HW-Pin, but connected to both uCs.
**------------------------------------------------------------------------------
*/
#define PORT_TEST_DO1_uC1      GPIOB
#define PINMASK_TEST_DO1_uC1   GPIO_PINMASK_0
#define PIN_TEST_DO1_uC1       GPIO_0

#define PORT_TEST_DO1_uC2      GPIOB
#define PINMASK_TEST_DO1_uC2   GPIO_PINMASK_1
#define PIN_TEST_DO1_uC2       GPIO_1

/*------------------------------------------------------------------------------
** mapping of enable DO inputs (read back from voltage supervision)
**------------------------------------------------------------------------------
*/

#ifndef CFG_TEMPERATURE_SENSOR_DIGITAL
  #error The value for CFG_TEMPERATURE_SENSOR_DIGITAL must be defined
#endif

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
  /* This is the hardware with the digital temperature measurement sensor */
  #define PORT_EN_DO       GPIOC
  #define PINMASK_EN_DO    GPIO_PINMASK_6
  #define PIN_EN_DO_uC2    GPIO_6
#else
  /* This is the primary hardware with analog temperature measurement sensor */
  #define PORT_EN_DO       GPIOB
  #define PINMASK_EN_DO    GPIO_PINMASK_7
  #define PIN_EN_DO_uC2    GPIO_7
#endif

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** typedef for output portpin properties
**------------------------------------------------------------------------------
*/
/*lint -esym(768, OUT_PORT_SETTINGS_TAG::pin)
       -esym(768, OUT_PORT_SETTINGS_TAG::outCfg)
       -esym(768, OUT_PORT_SETTINGS_TAG::outMode)*/
/* Lint message 768 deactivated because the struct members are defined but not
 * used, for future purpose. */
typedef struct OUT_PORT_SETTINGS_TAG
{
   GPIO_TypeDef* port;    /*  */
   UINT16 pinmask;    /*  */
   GPIO_PIN_ENUM  pin;          /* */
   GPIO_OUTPUT_CONFIG_ENUM outCfg;
   GPIO_OUTPUT_MODE_ENUM outMode;
} OUT_PORT_SETTINGS_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** constant mapping of digital "safe" outputs (DO in schematic)
**------------------------------------------------------------------------------
*/
extern CONST OUT_PORT_SETTINGS_STRUCT asOutputInit[GPIOCFG_NUM_DO_PER_uC];

/*------------------------------------------------------------------------------
** constant mapping of discharge pin (DISCHARGE... in schematic)
**------------------------------------------------------------------------------
*/
extern CONST OUT_PORT_SETTINGS_STRUCT asDisChrgInit[GPIOCFG_NUM_DO_PER_uC];


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void doPortMap_Init(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of doPortMap.h
**
********************************************************************************
*/

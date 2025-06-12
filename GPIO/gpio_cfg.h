/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio_cfg.h
**
** $Id: gpio_cfg.h 569 2016-08-31 13:08:08Z klan $
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
** global definitions / configurations which are be used by input and output
** module (not dedicated for input or output module).
** These definitions may also be used by other SW.
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

#ifndef GPIOCFG_H_
#define GPIOCFG_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** define for the number of outputs per uC (that is the number of available dual
** channel outputs of the ASM) and in total (usually twice the value per uC),
** see [SRS_195]
** Type: Output
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_DO_PER_uC   1u                           /* see [SRS_213] */
#define GPIOCFG_NUM_DO_TOT      (2u*GPIOCFG_NUM_DO_PER_uC)   /* see [SRS_213] */

/*------------------------------------------------------------------------------
** define for the number read-back-inputs for digital (save) outputs
** (Test_DO signals in schematic)
** Type: Input
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_TESTDO       GPIOCFG_NUM_DO_TOT

/*------------------------------------------------------------------------------
** define for the number of inputs per uC (that is the number of available dual
** channel inputs of the ASM), and the total number of Inputs (usually twice
** the value per uC).
** In current project, every uC has 3 digital safe inputs. So all in all there are
** 6 digital safe inputs which lead to 3 "dual channel" digital safe inputs.
** One "dual channel" digital safe input always uses one input of uC1 and the
** other of uC2, see [SRS_195]
** Type: Input
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_DI_PER_uC    3u                         /* see [SRS_213] */
#define GPIOCFG_NUM_DI_TOT      (2u*GPIOCFG_NUM_DI_PER_uC)  /* see [SRS_213] and
                                                               [SRS_592] */

/*------------------------------------------------------------------------------
** define for number of enable-output signals coming from voltage supervision
** (EN_DO... signals in schematic)
** There is only one at every uC (voltage supervision enables/disables all outputs),
** means even in case of increasing number of outputs this value may stay 1
** (Depending on HW-change)
** Type: Input
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_ENDO     1u

/*------------------------------------------------------------------------------
** define for the number of Test Outputs per uC (used to generate a test pulse
** which is running through devices connected outside and coming back to the input,
** e.g. can be used to test the availability of an emergency button), see [SRS_195]
** Type: Output
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_TO       1u

/*------------------------------------------------------------------------------
** define for the number of Diagnostic/Read-Back inputs of Test Outputs
** per uC
** (used for diagnostic, if Pulse send at TO is not read back at input:
** if also not read back at Diag_TO, then internal error else external error)
** Type: Input
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_DIAG_TO  1u

/*------------------------------------------------------------------------------
** define for the number of channels (same as number of uCs)
**------------------------------------------------------------------------------
*/
#define GPIOCFG_NUM_CHANNELS  2u

/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** This enum contains the available states of an IO Pin
**------------------------------------------------------------------------------
*/
typedef enum
{
   eGPIO_LOW   = 0x57a7u,   /* Identifier for low pin-state */
   eGPIO_HIGH  = 0x5831u    /* Identifier for high pin-state */
} GPIO_STATE_ENUM;



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


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of gpio_cfg.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** clkCtrl_cfg.h
**
** $Id: clkCtrl_cfg.h 569 2016-08-31 13:08:08Z klan $
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
** This is the configuration header. It contains all configurations which
** are used in the module, e.g. constant definitions for array sizes.
** For further information see VA_C_Programmierstandard rule STYL-2-3-20 and
** LIB-0-0-10.
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

#ifndef CLKCTRL_CFG_H
#define CLKCTRL_CFG_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
/* This constant defines the time cycle for the Clock diagnostic, the time is in
 * microseconds. */
#define CLKCTRL_TIMEDIFF  ((UINT32)30000000u) /* us --> 30 sec */

/* This macro calculates the specified deviation of the given value.
 * For the current use a deviation of 0.8% is needed. So the calculation is:
 * dev_result = value * 0.8/100  or  dev_result = value * 8/1000
 *
 * The division 8/1000 is similar than 8/1024 which can be handled easier for
 * the controller. So, the division can be executed as a shift operation, right
 * shift of 7 bits.
 * The deviation is then 0.78% instead of 0.8%.
 * */
#define CLKCTRL_CALC_DEVIATION(value) ((UINT32)((UINT32)value >> 7u))


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

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of clkCtrl_cfg.h
**
********************************************************************************
*/



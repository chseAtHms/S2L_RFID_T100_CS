/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globFail_SafetyHandler_loc.h
**
** $Id: globFail_SafetyHandler_loc.h 2437 2017-03-24 12:26:24Z klan $
** $Revision: 2437 $
** $Date: 2017-03-24 13:26:24 +0100 (Fr, 24 Mrz 2017) $
** $Author: klan $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Local header file of the Safety Handler module.
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
**
**
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/


#ifndef GLOBFAIL_SAFETYHANDLER_LOC_H
#define GLOBFAIL_SAFETYHANDLER_LOC_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! Constants to define the failcode page start and end address. */

/*!< Start address of the failcode page, (see [SRS_2242]). */
#define GLOBFAIL_FAILCODE_PAGE_STRT_ADDR  0x0803F800uL 
/*!< End address of the failcode page. */
/* Since each failure code contains 6 bytes, the page (2048 bytes) can not be
** filled entirely (341 x 6 bytes = 2046). This means the last two bytes of 
** the page are not usable.
*/
#define GLOBFAIL_FAILCODE_PAGE_END_ADDR   0x0803FFFDuL 

/*! Constants to define the FLASH keys. */
#define GLOBFAIL_FLASH_KEY1   0x45670123uL /*!< Value of the FLASH key 1. */
#define GLOBFAIL_FLASH_KEY2   0xCDEF89ABuL /*!< Value of the FLASH key 2. */

/*! Constant to define the cycle time (base is Timer 3) for message send in
 * Fail-Safe Mode, unit is 1us+0.8% = 1.008us
 * 4000us / 1.008us ~ 3969 (see [SRS_2237]) */
#define GLOBFAIL_MSG_CYCLE_TIME   (UINT32)3969u

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*------------------------------------------------------------------------------
**
** GLOBFAIL_TRIGGER_WWDG
**
** Description:
** Macro to trigger the window watchdog in the Safety Handler.
** This macro checks if the wwdg can be triggered or not. Is the counter value
** in the allowed area then the wwdg can be triggered. For further information
** see the STM32F100xxx_Reference_Manual.pdf.
** PC-Lint message 931 deactivated for this macro because there are no side
** effects.
**
**------------------------------------------------------------------------------
** Inputs:
**    -
**
** Outputs:
**    -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
/*lint -emacro(931, GLOBFAIL_TRIGGER_WWDG)*/
#define GLOBFAIL_TRIGGER_WWDG  \
{  \
   if ((WWDG->CR & (UINT32)WWDG_CR_T) < (WWDG->CFR & (UINT32)WWDG_CFR_W)) \
   { \
      WWDG->CR = (UINT32)(WWDG_CR_T0 + WWDG_CR_T1 + WWDG_CR_T6);\
   } \
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


#endif /* GLOBFAIL_SAFETYHANDLER_LOC_H */

/*******************************************************************************
**
** End of globFail_SafetyHandler_loc.h
**
********************************************************************************
*/

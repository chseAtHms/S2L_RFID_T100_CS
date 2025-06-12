/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** stHan_SelftestHandler-srv.h
**
** $Id: stHan_SelftestHandler-srv.h 569 2016-08-31 13:08:08Z klan $
** $Revision: 569 $
** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
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
** This module contains the selftest handler.
** The selftest handler executes consecutively the following selftests:
** 1. Safety-Container Check
** 2. ROM-Test
** 3. STACK-Test
** 4. CPU-Test
**    a. Opcode Test
**    b. SFR Test
**    c. CoreRegister Test
** 5. RAM-Test
** 6. Program-Flow Check of the selftest handler
**
** To execute the selftests the DoSelfTest-function has to be called.
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

#ifndef STHAN_SELFTESTHANDLER_SRV_H
#define STHAN_SELFTESTHANDLER_SRV_H

/*******************************************************************************
**
** includes (#include)
**
********************************************************************************
*/



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

extern void stHan_DoSelfTests (void);
extern void stHan_InitSelfTests (void);
extern void stHan_CheckSelfTestTimeout (void);


#endif /* STHAN_SELFTESTHANDLER_SRV_H */

/*******************************************************************************
**
** End of stHan_SelftestHandler-srv.h
**
********************************************************************************
*/

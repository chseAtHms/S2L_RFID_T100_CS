/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** stHan_SelftestHandler-srv_loc.h
**
** $Id: stHan_SelftestHandler-srv_loc.h 4393 2024-04-22 13:59:26Z ankr $
** $Revision: 4393 $
** $Date: 2024-04-22 15:59:26 +0200 (Mo, 22 Apr 2024) $
** $Author: ankr $
**
** Original Author: T. Langenbacher
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Module local Header of the selftest handler.
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

#ifndef STHAN_SELFTESTHANDLER_SRV_LOC_H
#define STHAN_SELFTESTHANDLER_SRV_LOC_H

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*! This constant defines the number of testcycles which is used during the
 *  selftest execution.  */
/* testcycles depend on which peripherals are used */
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
  #define STHANLOC_NUMBER_OF_TESTCYCLES  ((UINT32)0x0006977Bu)
#else
  #define STHANLOC_NUMBER_OF_TESTCYCLES  ((UINT32)0x0006977Cu)
#endif

/*! This constant defines the timeout for the selftests, the time is in
 *  milliseconds. */
#define STHANLOC_TIMEOUT               ((UINT32)3600000000u) /* ms --> 60 min */


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*! This enum contains the states of the selftest handler. The values of states
 * have a hamming distance (see [SRS_467]). */
typedef enum
{
   STHAN_RAM_TEST_ENUM     = 0x103du,   /*!< State of RAM-Test */
   STHAN_SAFECON_TEST_ENUM = 0x11aau,   /*!< State of Safety-Container */
   STHAN_ROM_TEST_ENUM     = 0x1284u,   /*!< State of ROM-Test */
   STHAN_STACK_TEST_ENUM   = 0x1313u,   /*!< State of STACK-Test */
   STHAN_CPU_TEST_ENUM     = 0x14d8u,   /*!< State of CPU-test */
   STHAN_CHECK_PFLOW_ENUM  = 0x154fu    /*!< State of Program-Flow */
} STHAN_SELF_TEST_TASK_ENUM;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/
STATIC void stHan_TriggerSelfTestTimeout (void);


#endif /* STHAN_SELFTESTHANDLER_SRV_LOC_H */

/*******************************************************************************
**
** End of stHan_SelftestHandler-srv_loc.h
**
********************************************************************************
*/

/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globFail_SafetyHandler.h
**
** $Id: globFail_SafetyHandler.h 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
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
** Header of the safety handler.
** The safety handler module provides the safe state. The safe state contains
** of a function which ends in an infinite loop which triggers the watchdog.
** The only exit of this function is via a power up.
** Furthermore this module provides a SAFETY-ASSERT and a SAFETY-FAIL macro.
** The SAFETY-ASSERT contains a condition which is used for checking something.
** If the condition is false the safety handler is called with the specified
** fail code. The SAFETY-FAIL switches directly to the safety handler.
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

#ifndef GLOBFAIL_SAFETYHANDLER_H
#define GLOBFAIL_SAFETYHANDLER_H

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

#ifndef __ASM__
/* globFail_u32_AddInfo
** Additional Info. This Additional Info is also stored inside Flash
** just after the actual error code. Default 0. (see [SRS_2233])
** Format in flash:  FailCode (2 byte), AddInfo (4byte).  
*/
extern volatile UINT32 globFail_u32_AddInfo;
#endif

#ifdef __CLASSIFIED_COMPLEXITY__

/*! If the compiler switch __CLASSIFIED_COMPLEXITY__ is defined, then the
 *  macros are used for calculating the complexity without the SAFETY-ASSERT
 *  and SAFETY-FAIL logic. */
#define GLOBFAIL_SAFETY_ASSERT(condition, failcode) ((void)0);
#define GLOBFAIL_SAFETY_ASSERT_RET(condition, failcode, retVal) ((void)0);
#define GLOBFAIL_SAFETY_FAIL(failcode) ((void)0);
#define GLOBFAIL_SAFETY_FAIL_RET(failcode, retVal) ((void)0);
#define GLOBFAIL_SAFETY_FAIL_RDS(failcode) ((void)0);

#else /*  __GET_CORRECT_METRICS__ */


/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_ASSERT
**
** Description:
** Switches to the safe state if the condition is not true.
**
**------------------------------------------------------------------------------
** Inputs:
**    condition    Condition which leads to the safety state if not fulfilled.
**    failcode     Global error code.
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
#ifndef __UNIT_TESTING_ON__

#define GLOBFAIL_SAFETY_ASSERT(condition, failcode) \
{ \
   if (!(condition)) \
   { \
      (void) globFail_SafetyHandler(failcode); \
   } \
}

#else

#define GLOBFAIL_SAFETY_ASSERT(condition, failcode) \
{ \
   if (!(condition)) \
   { \
      (void) globFail_SafetyHandler(failcode); \
      return; \
   } \
}

#endif /* __UNIT_TESTING_ON__ */


/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_ASSERT_RET
**
** Description:
** Switches to the safe state if the condition is not true. Additional the
** function allows to define a return value, just for the unit tests
**
**------------------------------------------------------------------------------
** Inputs:
**    condition    Condition which leads to the safety state if not fulfilled.
**    failcode     Global error code.
**    retVal       return value of calling function (used only by module tests
**                 with Cantata)
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
#ifndef __UNIT_TESTING_ON__

#define GLOBFAIL_SAFETY_ASSERT_RET(condition, failcode, retVal) \
{ \
   if (!(condition)) \
   { \
      (void) globFail_SafetyHandler(failcode); \
   } \
}

#else

#define GLOBFAIL_SAFETY_ASSERT_RET(condition, failcode, retVal) \
{ \
   if (!(condition)) \
   { \
      (void) globFail_SafetyHandler(failcode); \
      return retVal; \
   } \
}

#endif /* __UNIT_TESTING_ON__ */


/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_FAIL
**
** Description:
** Switches directly to the safe state.
**
**------------------------------------------------------------------------------
** Inputs:
**    failcode     Global error code.
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
#ifndef __UNIT_TESTING_ON__

#define GLOBFAIL_SAFETY_FAIL(failcode) globFail_SafetyHandler(failcode)

#else

#define GLOBFAIL_SAFETY_FAIL(failcode) globFail_SafetyHandler(failcode); return

#endif /* __UNIT_TESTING_ON__ */

/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_FAIL_RET
**
** Description:
** Switches directly to the safe state. Additional the function allows to
** define a return value, just for the unit test.
**
**------------------------------------------------------------------------------
** Inputs:
**    failcode     Global error code.
**    retVal       return value of calling function (used only by module tests
**                 with Cantata)
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
#ifndef __UNIT_TESTING_ON__
   #define GLOBFAIL_SAFETY_FAIL_RET(failcode, retVal) globFail_SafetyHandler(failcode)
#else
   #define GLOBFAIL_SAFETY_FAIL_RET(failcode, retVal) globFail_SafetyHandler(failcode); \
                                                      return retVal

#endif /* __UNIT_TESTING_ON__ */

/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_FAIL_RDS
**
** Description:
** This macro is called only from the RDS module and switches directly to the
** safe state.
**
**------------------------------------------------------------------------------
** Inputs:
**    failcode     Global error code.
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
#define GLOBFAIL_SAFETY_FAIL_RDS(failcode)  globFail_SafetyHandler(failcode)


/*------------------------------------------------------------------------------
**
** GLOBFAIL_ADDINFO_FILE
**
** Description:
** Used to build the additional info containing of 7 bit file id and 6 bit error id.
** This requires to define k_FILEID in each module using this macro. The following
** schema is applied:
**  +---------+----------+---------+----------+------------+--------------+
**  | bit pos | 31-16    |  15-14  |    13    |   12-06    |   05-00      |
**  +---------+----------+---------+----------+------------+--------------+
**  | meaning | unused   | errType | CSS/CSAL |   FileID   |   errId      |
**  +---------+----------+---------+----------+------------+--------------+
**                 0         11         0            x             x       
**
**  Bits 15-14 are fixed set to 11 because only the additional info for
**  failsafe error are set here.
**  Bit 13 is fixed set to 0 because 1 is reserved for CSS.
**------------------------------------------------------------------------------
** Inputs:
**    errId      Error id used to build the additional info
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
#define GLOBFAIL_ADDINFO_FILE(errId)                                        \
                           (UINT32)((UINT16)((UINT16)(3u)       << 14) |    \
                                    (UINT16)(((UINT16)(k_FILEID) & 0x007Fu) <<  6) | \
                                    (UINT16)((UINT16)(errId) & 0x003Fu) )

/*------------------------------------------------------------------------------
**
** GLOBFAIL_SAFETY_HANDLER
**
** Description:
** Switches directly to the safe state. An extension to the global failure code
** (additional info) is used here. To ensure the additional info matches to the
** global failure code, the IRQs are disabled here although done in safety
** handler later.
** The additional info consist of 7 bit file id and 6 bit error id. This
** requires the define k_FILEID in each module using this macro.
**------------------------------------------------------------------------------
** Inputs:
**    failcode     Global failure code.
**    addInfo      Error id used to build the additional info
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
#define GLOBFAIL_SAFETY_HANDLER(failcode, addInfo) \
{ \
  __disable_irq(); \
  globFail_u32_AddInfo = addInfo; \
  globFail_SafetyHandler(failcode); \
}

#endif /* __GET_CORRECT_METRICS__ */

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
#ifndef __ASM__
extern UINT8 globFail_SafetyHandler (GLOB_FAILCODE_ENUM eFailCode);
#endif


#endif /* GLOBFAIL_SAFETYHANDLER_H */

/*******************************************************************************
**
** End of globFail_SafetyHandler.h
**
********************************************************************************
*/

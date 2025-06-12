/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLfailCode.h
**     Summary: Interface for Failure Log.
**   $Revision: 2014 $
**       $Date: 2017-02-03 16:16:04 +0100 (Fr, 03 Feb 2017) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_FailCodeInit
**             SAPL_FailCodeSync
**             SAPL_FailCodeLogGet
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_FAILURECODE_H
#define SAPL_FAILURECODE_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Number of readable failure codes, (see [SRS_2229]) */
#define SAPL_k_NUM_READABLE_FAILURES    2u

/***************************************************************************************************
**    data types
***************************************************************************************************/

/* Single Failure entry of the Failure Log */
typedef struct
{
  UINT16 u16_failCodeOwn;      /* Failure Code own controller */
  UINT32 u32_addInfoOwn;       /* Additional Info own controller */
  UINT16 u16_failCodeOther;    /* Failure Code other controller */
  UINT32 u32_addInfoOther;     /* Additional Info other controller */
} SAPL_t_FAILURECODE;

/* Structure of the Failure Log */
typedef struct
{
  UINT16 u16_numFailOwnCtrl;    /* number of failure codes own controller */
  UINT16 u16_numFailOtherCtrl;  /* number of failure codes other controller */
  SAPL_t_FAILURECODE as_failure[SAPL_k_NUM_READABLE_FAILURES];
} SAPL_t_FAILURE_LOG;

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/

/***************************************************************************************************
  Function:
    SAPL_FailCodeInit

  Description:
    This function is used to read the last SAPL_k_NUM_READABLE_FAILURES failure codes from the
    Failure Code Block of the flash.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_FailCodeInit(void);

/***************************************************************************************************
  Function:
    SAPL_FailCodeSync

  Description:
    This function is used to exchange the number of stored failure code and the failure codes
    itself with the other controller.
    The number of exchanged failure codes can be configured via SAPL_k_NUM_READABLE_FAILURES.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_FailCodeSync(void);

/***************************************************************************************************
  Function:
    SAPL_FailCodeLogGet

  Description:
    This function is used to get a pointer to the Failure Code Log that was built during startup.
    The Failure Code Log contains the last SAPL_k_NUM_READABLE_FAILURES failure code.

  See also:
    -

  Parameters:
    -

  Return value:
    SAPL_t_FAILURE_LOG*  - Pointer of Failure Code Log

  Remarks:
    Context: Background Task

***************************************************************************************************/
CONST SAPL_t_FAILURE_LOG* SAPL_FailCodeLogGet(void);

#endif


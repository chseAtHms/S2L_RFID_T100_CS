/***************************************************************************************************
**    Copyright (C) 2016-2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: spduOutData.c
**     Summary: The module implements functionality to take the received Output values
**              (DO value, DI Error Reset Flags, DI Error Reset Flags) and forwards them to
**              the DI/DO module.
**   $Revision: 2618 $
**       $Date: 2017-04-06 13:58:52 +0200 (Do, 06 Apr 2017) $
**      Author: K.Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: spduOutData_Reset
**             spduOutData_Process
**             spduOutData_Set
**             spduOutData_Get
**             spduOutData_DoGet
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "includeDiDo.h"
#include "fiParam.h"

/* application includes */
#include "aicMsgDef.h"
#include "spduOutData.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/

/* spduOutData_e_IsRunMode
** Status (Idle/Run) of the output data received by CSS.
** eFALSE: IDLE, eTRUE: RUN
*/
volatile TRUE_FALSE_ENUM spduOutData_e_IsRunMode = eFALSE;

/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      28u

/* u8_RdsRxIoValues:
** safe variable to store the DO request (DO ON/Off bits received vis protocol)
*/
STATIC volatile RDS_UINT8(u8_RdsRxDoValues);

/* u8_RdsDiErrRstFlags:
** safe variable to store the DI Error Reset request received via protocol
*/
STATIC volatile RDS_UINT8(u8_RdsDiErrRstFlags);

/* u8_RdsDoErrRstFlags:
** safe variable to store the DO Error Reset request received via protocol
*/
STATIC volatile RDS_UINT8(u8_RdsDoErrRstFlags);

/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
***************************************************************************************************/

/***************************************************************************************************
  Function:
    spduOutData_Reset

  Description:
    This function resets the received IO values of global IO value structure and sets the
    default values inside module.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)
    Context: IRQ Scheduler

***************************************************************************************************/
void spduOutData_Reset (void)
{
   /* reset the IO values, this also ensures that the copy inside the non-safe IO block of the SPDU
   ** is set to default values (see [SRS_2032] */
   RDS_SET(u8_RdsRxDoValues, 0x00u);
   RDS_SET(u8_RdsDiErrRstFlags, 0x00u);
   RDS_SET(u8_RdsDoErrRstFlags, 0x00u);
}

/***************************************************************************************************
  Function:
    spduOutData_Process

  Description:
    This function is used to process the IO Data payload received via CIP Safety IO Data Message.
    Depending on the received RUN/IDLE bit the software:
      - passes the IO Data, DI Reset Flags, DO Reset Flags to the DI/DO Module (-->RUN)
      - requests the passivation on the DOs (-->IDLE)

    Attention: This function shall be called only, if the Consuming Connection is established!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
void spduOutData_Process(void)
{ 
  /* if RUN bit set inside received IO Data Message */
  if (spduOutData_e_IsRunMode == eTRUE)
  { 
    /* get DO (Output) request byte and pass it to DO module, see [SRS_2007], [SRS_2014],
     * [SRS_2016], [SRS_2017] */
    doLib_SetPSDoReq( RDS_GET(u8_RdsRxDoValues) );
    
    /* if automatic error reset feature for DIs enabled */
    if ( FIPARAM_DI_AUTOMATIC_RESET_ENABLED )
    {
      /* ignore the DI Error Reset flags inside SPDU (see [SRS_2223]) */
    }
    /* else: manual error reset */
    else
    {
      /* get DI (Output) Reset Flags (in one Byte) requested by PLC and pass it to 
      ** DI module, see [SRS_2053], [SRS_2011] */
      diLib_SetErrRstFlags( RDS_GET(u8_RdsDiErrRstFlags) );
    }
    
    /* if automatic error reset feature for DOs enabled */
    if ( FIPARAM_DO_AUTOMATIC_RESET_ENABLED )
    {
      /* ignore the DO Error Reset flags inside SPDU (see [SRS_2224]) */
    }
    /* else: manual error reset */
    else
    {
      /* get DO (Output) Reset Flags (in one Byte) requested by PLC and pass it 
      to DO module, see [SRS_2053], [SRS_2015] */
      doLib_SetErrRstFlags( RDS_GET(u8_RdsDoErrRstFlags) );
    }
  }
  /* else if: IDLE bit set */
  else if (spduOutData_e_IsRunMode == eFALSE)
  {
    /* request passivation of outputs */
    doLib_PassAllDOs();
  }
  /* else: defensive programming, check value of global */
  else
  {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
}

/***************************************************************************************************
  Function:
    spduOutData_Set

  Description:
    This function is used to store the received output data (DO Value, DI Error Reset Flags,
    DO Error Reset Flags) inside the module.
    The passed output data keeps valid until a new output data is passed or the function  
    spduOutData_Reset() is called after AIC state has changed.

    This function shall be called if an new IO Data Message was received.

  See also:
    -

  Parameters:
    ps_ioVal (IN)            - Pointer to received output data  (DO Value, DI Error Reset Flags,
                               DO Error Reset Flag)
                               (valid: <>NULL, not checked, already checked in 
                               SAPL_IxsaiIoDataRxClbk())

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void spduOutData_Set(CONST AICMSGDEF_PS_OUT_DATA_STRUCT* ps_ioVal)
{
  /** Attention: Data structure accessed from IRQ scheduler and background task. So it is
  ** required to disable scheduler during copy process
  */
  __disable_irq();
  
  /* copy data of received CIP Safety packet to global output buffer (see [SRS_2014]) */
  /* store DO Value, DI Erros Restet Flags and DO Error Reset Flags in RDS variables */
  RDS_SET(u8_RdsRxDoValues, ps_ioVal->u8DoVal);
  RDS_SET(u8_RdsDiErrRstFlags, ps_ioVal->u8DiErrRst);
  RDS_SET(u8_RdsDoErrRstFlags, ps_ioVal->u8DoErrRst);
  
  /* enable IRQs again */
  __enable_irq();
}

/***************************************************************************************************
  Function:
    spduOutData_Get

  Description:
    This function returns the last received output data (DO Value, DI Error Reset Flags,
    DO Error Reset Flags).

    Because no RDS checks are done inside this function, the values shall be used for non-safe 
    functionality only.

    Attention: IRQs not disabled because function called from Background Task and IRQ Scheduler.
    This is task of the caller function.

  See also:
    -

  Parameters:
    -

  Return value:
    AICMSGDEF_PS_OUT_DATA_STRUCT - (DO Value, DI Error Reset Flags, DO Error Reset Flags)

  Remarks:
    Context: Background Task
    Context: IRQ Scheduler

***************************************************************************************************/
AICMSGDEF_PS_OUT_DATA_STRUCT spduOutData_Get(void)
{
  AICMSGDEF_PS_OUT_DATA_STRUCT s_ioVal;
 
  /* Attention: Because this data is used only for non-safe functionality, we do not perform
  ** RDS check because of performance reasons. */
  s_ioVal.u8DoVal = RDS_GET_VALUE(u8_RdsRxDoValues);
  s_ioVal.u8DiErrRst = RDS_GET_VALUE(u8_RdsDiErrRstFlags);
  s_ioVal.u8DoErrRst = RDS_GET_VALUE(u8_RdsDoErrRstFlags);
  
  return s_ioVal;
}

/***************************************************************************************************
  Function:
    spduOutData_DoGet

  Description:
    This function return the current secured (RDS) DO Value.

  See also:
    -

  Parameters:
    -

  Return value:
    DO Value

  Remarks:
    Context: IRQ Scheduler

***************************************************************************************************/
UINT8 spduOutData_DoGet(void)
{
  return ( RDS_GET(u8_RdsRxDoValues) );
}

/***************************************************************************************************
**    static functions
***************************************************************************************************/

/*******************************************************************************
 ********************************************************************************
 **
 ** File Name
 ** ---------
 **
 ** ipcs_IpcSync-sys.c
 **
 ** $Id: ipcs_IpcSync-sys.c 569 2016-08-31 13:08:08Z klan $
 ** $Revision: 569 $
 ** $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 ** $Author: klan $
 **
 ********************************************************************************
 ********************************************************************************
 **
 ** Description
 ** -----------
 **
 ** IPC synchronization module (offers functionalities required by [SRS_28],
 ** [SRS_317]).
 **
 ** This module contains the functions to synchronize the IPC and
 ** the two controllers.
 ** If the program switches into the sync function the sync pin
 ** PA12 is set to the excepted state. Now the state of the sync pin PA11
 ** of the other controller is checked. The program waits till the
 ** sync pin PA11 is set to the expected state. Then the other controller
 ** reaches the sync state as well. After this the program flow continues
 ** and the sync pin PA12 is set to the inverse state state which is used as the
 ** next expected state.
 ** If the other controller never reaches the sync state, the
 ** program gets after a specified time a timeout and switches to
 ** the safe state.
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

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "timer-def.h"
#include "timer-hal.h"
#include "gpio-hal.h"

#include "globFit_FitTestHandler.h"

/* Header-file of module */
#include "ipcs_IpcSync-sys.h"
#include "ipcs_IpcSync-sys_loc.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

/*******************************************************************************
 **
 ** Public Globals
 **
 ********************************************************************************
 */

/*******************************************************************************
 **
 ** Private Globals
 **
 ********************************************************************************
 */

/*******************************************************************************
 **
 ** Public Services
 **
 ********************************************************************************
 */

/*------------------------------------------------------------------------------
 **
 ** ipcsSYS_Init()
 **
 ** Description:
 ** This function initialize the sync module.
 ** The pin PA11 is configured as input and pin PA12 as output.
 ** These pins are used for the synchronization.
 ** Furthermore the output pin is set to low, this is the idle state.
 **
 **------------------------------------------------------------------------------
 ** Input:
 **  void
 **  Register settings: GPIOA Pin 11 + 12
 **
 ** Return:
 **  void
 **
 ** Usage:
 **  gpioHAL_ConfigureInput
 **  gpioHAL_ConfigureOutput
 **  gpioHAL_ClearPin
 **
 ** Module Test: - NO -
 **              Reason: Only function calls and port settings
 **------------------------------------------------------------------------------
 */
void ipcsSYS_Init (void)
{
   /*
    *  PA11 (uC1)input <- PA12 (uC2)output
    */

   /* set PA11 to input */
   gpioHAL_ConfigureInput ( IPC_SYNC_GPIOA,
      GPIO_11,
      IN_CONFIG_FLOATING);

   /* set PA12 to output */
   gpioHAL_ConfigureOutput ( IPC_SYNC_GPIOA,
      GPIO_12,
      OUT_CONFIG_GPIO_OD,
      OUT_MODE_10MHZ);

   /* clear the output PA12 */
   gpioHAL_ClearPin ( IPC_SYNC_GPIOA,
      GPIO_PINMASK_12 );

} /* end of ipcsSYS_Init() */

/*------------------------------------------------------------------------------
 **
 ** ipcsSYS_SyncStartup()
 **
 ** Description:
 ** This function is used to synchronize the two controllers at startup (see
 ** [SRS_76]).
 ** This function sets the controller specific sync signal to
 ** high and waits till the sync signal of the other controller
 ** is high as well.
 ** During the waiting a timeout is measured. If it is reached, the safety
 ** handler will be called.
 **
 **
 **------------------------------------------------------------------------------
 ** Input:
 **  void
 **  Register settings: GPIOA Pin 11 / 12
 **
 ** Return:
 **  void
 **
 ** Usage:
 **  gpioHAL_SetPin
 **  gpioHAL_GetPin
 **
 ** Module Test: - NO -
 **      Reason: function has low complexity
 **------------------------------------------------------------------------------
 */
void ipcsSYS_SyncStartup (void)
{
   UINT32 u32SysTime;
   UINT32 u32Timeout;

   GPIO_PINSTATE_ENUM ePinState;

   /* get actual system time for timeout calculation */
   u32SysTime = timerHAL_GetSystemTime3();

   /* set sync signal to high */
   gpioHAL_SetPin ( IPC_SYNC_GPIOA,
      GPIO_PINMASK_12 );

   /* get pin state of sync line */
   ePinState = gpioHAL_GetPin( IPC_SYNC_GPIOA,
      GPIO_PINMASK_11 );

   /* check the state of Pin: low or high */
   GPIO_PINSTATE_SAFETY_ASSERT( ePinState);

   /* wait for other controller */
   while (IO_HIGH != ePinState)
   {
      /* timeout calculation via macro */
      u32Timeout = TIMEOUT_SYNC( u32SysTime );

      /* check if a timeout achieved, FS in error case, see [SRS_76] */
      GLOBFAIL_SAFETY_ASSERT(IPCSSYS_IPCSYNC_TIMEOUT_STARTUP > u32Timeout,
         GLOB_FAILCODE_IPCS_SYNC_TIMEOUT);

      /* get pin state of sync line */
      ePinState = gpioHAL_GetPin( IPC_SYNC_GPIOA,
         GPIO_PINMASK_11 );

      /* check the state of Pin: low or high */
      GPIO_PINSTATE_SAFETY_ASSERT(ePinState);
   }
} /* end of ipcsSYS_SyncStartup() */


/*------------------------------------------------------------------------------
 **
 ** ipcsSYS_Sync()
 **
 ** Description:
 ** This function is used to synchronize the two controllers.
 ** This function sets the controller specific sync signal to
 ** to the expected state and waits till the sync signal of the other controller
 ** is at the expected state. Then the controller specific sync signal is set
 ** to the inverse state.
 ** During the waiting a timeout is measured. If it is reached, the safety
 ** handler will be called.
 **
 **------------------------------------------------------------------------------
 ** Input:
 **  void
 **  Register settings: GPIOA Pin 11 / 12
 **
 ** Return:
 **  void
 **
 ** Usage:
 **  timerHAL_GetSystemTime3
 **  gpioHAL_SetPin
 **  gpioHAL_ClearPin
 **
 ** Module Test: - NO -
 **              Reason: low complexity
 **------------------------------------------------------------------------------
 */
void ipcsSYS_Sync( void )
{
   UINT32 u32SysTime;
   UINT32 u32Timeout;

   LOCAL_STATIC (, GPIO_PINSTATE_ENUM,
                   ePinExpectedState,
                   IO_HIGH);

   GPIO_PINSTATE_ENUM ePinState;

   /* get actual system time for timeout calculation */
   u32SysTime = timerHAL_GetSystemTime3();

   if(IO_HIGH == ePinExpectedState)
   {  /* set sync signal to high */
      gpioHAL_SetPin ( IPC_SYNC_GPIOA,
         GPIO_PINMASK_12 );
   }
   else if (IO_LOW == ePinExpectedState)
   {
      gpioHAL_ClearPin ( IPC_SYNC_GPIOA,
         GPIO_PINMASK_12 );
   }
   else
   {  /* error: invalid value of variable */
      GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
   }

   /* get pin state of sync line */
   ePinState = gpioHAL_GetPin( IPC_SYNC_GPIOA,
      GPIO_PINMASK_11 );

   /* check the state of Pin: low or high */
   GPIO_PINSTATE_SAFETY_ASSERT( ePinState );

   /* wait for other controller */
   while (ePinExpectedState != ePinState)
   {
      /* timeout calculation via macro */
      u32Timeout = TIMEOUT_SYNC( u32SysTime );


      /*FIT to manipulate the the timeout variable*/
      GLOBFIT_FITTEST(GLOBFIT_CMD_IPC_TIMEOUT_SYNC,
         (u32Timeout = (IPCSSYS_IPCSYNC_TIMEOUT + 1u)));

      /* check if a timeout achieved */
      GLOBFAIL_SAFETY_ASSERT( IPCSSYS_IPCSYNC_TIMEOUT > u32Timeout,
                              GLOB_FAILCODE_IPCS_SYNC_TIMEOUT);

      /* get pin state of sync line */
      ePinState = gpioHAL_GetPin( IPC_SYNC_GPIOA,
                                  GPIO_PINMASK_11 );

      /* check the state of Pin: low or high */
      GPIO_PINSTATE_SAFETY_ASSERT(ePinState);
   }

   /* Check the expected pin state and set it to the opposite pin state.
    * A check against an invalid value of the expected pin state is not
    * necessary because an indirect check is in the while loop above. The only
    * abortion of the while loop is when the expected pin state equals the
    * current pin state. If they are not equal, e.g. in case of a soft-error,
    * the while never ends and a timeout is detected which ends in the Safety
    * Handler. */
   if(IO_HIGH == ePinExpectedState)
   {
      ePinExpectedState = IO_LOW;
   }
   else
   {
      ePinExpectedState = IO_HIGH;
   }

} /* end of ipcsSYS_Sync() */



/*******************************************************************************
 **
 ** End of ipcs_IpcSync-sys.c
 **
 ********************************************************************************
 */

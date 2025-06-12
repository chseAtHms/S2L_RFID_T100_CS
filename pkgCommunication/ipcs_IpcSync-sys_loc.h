/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** ipcs_IpcSync-sys_loc.h
**
** $Id: ipcs_IpcSync-sys_loc.h 2287 2017-03-03 13:39:21Z klan $
** $Revision: 2287 $
** $Date: 2017-03-03 14:39:21 +0100 (Fr, 03 Mrz 2017) $
** $Author: klan $
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Local header file of the IPC-Sync module.
**
** Contains the local constants and type definitions of the appendent c-file,
** see VA_C_Programmierstandard rule TSTB-2-0-10 und STYL-2-3-20.
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

#ifndef IPCS_IPCSYNC_LOC_H

#define IPCS_IPCSYNC_LOC_H
/*******************************************************************************
**
** Switches (#define)
**
********************************************************************************
*/

/*! This compiler switch sets the ipc sync timeout to a debug friendly value. */
#ifdef __DEBUG__
#define IPCS_TIMEOUT_FOR_DEBUG_USAGE_ACTIVE  TRUE
#else
#define IPCS_TIMEOUT_FOR_DEBUG_USAGE_ACTIVE  FALSE
#endif

/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/

/*******************************************************************************
**
** Constants
**
********************************************************************************
*/
#if (IPCS_TIMEOUT_FOR_DEBUG_USAGE_ACTIVE == TRUE)
   /*! This constant defines the timeout of the synchronization between the
    * controllers. The time unit is in microseconds. */
   /* In debug mode this value can be changed. */
   #define IPCSSYS_IPCSYNC_TIMEOUT           ((UINT32)80)      
   /* In debug mode this value can be changed. */
   #define IPCSSYS_IPCSYNC_TIMEOUT_STARTUP   ((UINT32)2000000)

   /* if the compiler switch is activated, the compiler signals a warning */
   #warning "IPC-Sync-Timeout changed for debug usage"
#else
   /*! This constant defines the timeout of the synchronization between the
    * controllers. The time unit is in microseconds. */
   #define IPCSSYS_IPCSYNC_TIMEOUT           ((UINT32)80)
   #define IPCSSYS_IPCSYNC_TIMEOUT_STARTUP   ((UINT32)2000000)
#endif

/*! This constant defines the IPC-Sync GPIO. */
#define IPC_SYNC_GPIOA          (GPIOA)

/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/

/*******************************************************************************
**
** Macros
**
********************************************************************************
*/
/*! This macro calculates the timeout for the IPC sync. */
#define TIMEOUT_SYNC( x )   ( timerHAL_GetSystemTime( TIMERDEF_SYNC ) - ( UINT32 )( x ) )


/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
**
**------------------------------------------------------------------------------
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/


#endif /* inclusion lock */

/*******************************************************************************
**
** End of ipcs_IpcSync-sys_loc.h
**
********************************************************************************
*/

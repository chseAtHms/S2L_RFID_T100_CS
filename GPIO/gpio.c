/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio.c
**
** $Id: gpio.c 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
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
** Interface between higher level SW (e.g. Scheduler) and DI/DO module.
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

/*******************************************************************************
**
** includes
**
********************************************************************************
*/

/* Project header */
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

#include "gpio_cfg.h"

#include "doInit.h"
#include "doState.h"
#include "doLib.h"

#include "diInit.h"
#include "diState.h"
#include "diInput.h"
#include "diLib.h"

#include "gpioSample.h"

/* Header-file of module */
#include "gpio.h"

#ifdef RFID_ACTIVE
  #include "cfg_Config-sys.h"
  #include "RFID.h"
#endif


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

STATIC UINT8 gpio_u8IpcAicSyncSend = (UINT8)0x00u;
STATIC UINT8 gpio_u8IpcAicSyncReceive = (UINT8)0x00u;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** gpio_InitPorts()
**
** Description:
** function for initialization/configuration of DO and DI ports:
** calls initialization functions
**
**------------------------------------------------------------------------------
** Inputs:
**    eReInit:
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_InitPorts();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**
** Context: main (during initialization)
**------------------------------------------------------------------------------
*/
void gpio_InitPorts(void)
{
   diInit_InitDiPorts();
   doInit_InitDoPorts();
}


/*------------------------------------------------------------------------------
**
** gpio_InitModules()
**
** Description:
** Initialization of DI/DO modules without port/pin initialization.
** Can be used for re-initialization of DI/DO module. Calls functions to reset
** the default/reset values except the ports.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_InitModules();
**
** Module Test:
**    - NO -
**    Reason: Only function call
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_InitModules(void)
{
   diInit_InitDiModule();
   doInit_InitDoModule();
}


/*------------------------------------------------------------------------------
**
** gpio_FilterDiRelValues()
**
** Description:
**    function to call the filter function which filters the input-read buffer
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_filterInputValues();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_FilterDiRelValues(void)
{
   diInput_FilterDiRelInputs();
}


/*------------------------------------------------------------------------------
**
** gpio_ProcessDIs()
**
** Description:
**    function triggers processing of digital inputs (DIs)
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_ProcessDIs();
**
** Module Test: - NO -
**              Reason: Only function calls
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_ProcessDIs(void)
{
   diState_ProcessDIs();
}


/*------------------------------------------------------------------------------
**
** gpio_ProcessDOs()
**
** Description:
**    function triggers processing of digital outputs (DOs):
**    - calls function to filter DO related input values
**    - calls DO processing function
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_ProcessDOs();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_ProcessDOs(void)
{
   diInput_FilterDoRelInputs();
   doState_ProcessDOs();
}


/*------------------------------------------------------------------------------
**
** gpio_SwitchOutputsOff()
**
** Description:
**    calls function to switch off outputs, specially used during init/startup,
**    see [SRS_75]
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_SwitchOutputsOff();
**
** Module Test:
**    - NO -
**    Reason: Only function call
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_SwitchOutputsOff(void)
{
   doState_SwitchDOsOff();
}

/*------------------------------------------------------------------------------
**
** gpio_SendIOvaluesViaIPC()
**
** Description:
**    function handles the IPC transmission of input output values to other uC:
**    - call functions to prepare values
**    - fill transmission structure
**    - trigger IPC sending
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_SendIOvaluesViaIPC();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_SendIOvaluesViaIPC(void)
{
   INPT_IPC_IO_DATA_UNION uIpcIoData;

   /* prepare DO related values */
   doLib_PrepareTestQualForIpc();

   /* prepare DI related values */
   diInput_PrepareValForIPC();

   /* copy values to transmission struct */
   /* prepare DO qualifier for other channel, see [SRS_686] */
   uIpcIoData.sIoStruct.u8SafeDoQual = dolib_u8TestQualValues;
   /* prepare DI qualifier for other channel, see [SRS_685] */
   uIpcIoData.sIoStruct.u8SafeDiQual = diInput_u8DiTestQualValues;

   /* copy DI values to IPC struct, see [SRS_671] */
   uIpcIoData.sIoStruct.u8SafeDiValues = diInput_u8DiBitValues;

   uIpcIoData.sIoStruct.u8AicSyncCnt = gpio_u8IpcAicSyncSend;

#ifdef RFID_ACTIVE
  if (cfgSYS_GetControllerID() == SAFETY_CONTROLLER_1)
  {
    uIpcIoData.sIoStruct.u32RfidInfo = RFID_InfoGet();
  }

  ipcxSYS_SendBuffer(IPCXSYS_IPC_ID_IO, (UINT8)sizeof(INPT_IPC_IO_DATA_STRUCT),
                     (CONST UINT8*)&uIpcIoData.sIoStruct);
#else
   /* call function to initiate IPC transmission, see [SRS_671] */
   ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_IO, (uIpcIoData.u32IpcPacket));
#endif /* RFID_ACTIVE */
}


/*------------------------------------------------------------------------------
**
** gpio_GetIOvaluesFromIPC()
**
** Description:
**    function gets the received input / output values (via IPC-Buffer) from other
**    channel/uC.
**    If there was no correct IPC message received, fail-safe mode will be entered.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_GetIOvaluesFromIPC();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_GetIOvaluesFromIPC(void)
{
   INPT_IPC_IO_DATA_UNION uIpcIoDataRx;

#ifdef RFID_ACTIVE
 ipcxSYS_GetBufferinclWait(IPCXSYS_IPC_ID_IO, (UINT8)sizeof(INPT_IPC_IO_DATA_STRUCT),
                            (UINT8*)&uIpcIoDataRx.sIoStruct);
  
  if (cfgSYS_GetControllerID() == SAFETY_CONTROLLER_2)
  {
    RFID_InfoSet(uIpcIoDataRx.sIoStruct.u32RfidInfo);
  }
#else
   uIpcIoDataRx.u32IpcPacket = ipcxSYS_GetUINT32inclWait( IPCXSYS_IPC_ID_IO );
#endif /* RFID_ACTIVE */
   dolib_u8TestQualValuesOtherCh = uIpcIoDataRx.sIoStruct.u8SafeDoQual;
   diInput_u8DiTestQualValuesOtherCh = uIpcIoDataRx.sIoStruct.u8SafeDiQual;
   diInput_u8DiBitValuesOtherCh = uIpcIoDataRx.sIoStruct.u8SafeDiValues;
   gpio_u8IpcAicSyncReceive = uIpcIoDataRx.sIoStruct.u8AicSyncCnt;

   /* convert values received from other channel */
   diInput_ConvValFromOtherCh();
   doLib_ConvValFromOtherCh();
}


/*------------------------------------------------------------------------------
**
** gpio_SendAicSyncViaIPC()
**
** Description:
**    function handles the IPC transmission of the AIC sync value. This value
**    is usually send together with the IO data, but in initialization/
**    parameterization there are no IO values to exchange, so the AIC sync
**    value is send alone
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_SendAicSyncViaIPC();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_SendAicSyncViaIPC(void)
{
   INPT_IPC_IO_DATA_UNION uIpcIoData;

   /* copy values to transmission struct */
   uIpcIoData.sIoStruct.u8SafeDoQual = 0x00u;

   uIpcIoData.sIoStruct.u8SafeDiQual = 0x00u;

   uIpcIoData.sIoStruct.u8SafeDiValues = 0x00u;

   uIpcIoData.sIoStruct.u8AicSyncCnt = gpio_u8IpcAicSyncSend;

#ifdef RFID_ACTIVE
 if (cfgSYS_GetControllerID() == SAFETY_CONTROLLER_1)
  {
    uIpcIoData.sIoStruct.u32RfidInfo = RFID_InfoGet();
  }

  ipcxSYS_SendBuffer(IPCXSYS_IPC_ID_IO, (UINT8)sizeof(INPT_IPC_IO_DATA_STRUCT),
                     (CONST UINT8*)&uIpcIoData.sIoStruct);
#else
   /* call function to initiate IPC transmission */
   ipcxSYS_SendUINT32(IPCXSYS_IPC_ID_IO, (uIpcIoData.u32IpcPacket));
#endif

}


/*------------------------------------------------------------------------------
**
** gpio_GetAicSyncFromIPC()
**
** Description:
**    function handles the IPC reception of the AIC sync value. This value
**    is usually received together with the IO data, but in initialization/
**    parameterization there are no IO values to exchange, so the AIC sync
**    value is received alone
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_GetAicSyncFromIPC();
**
** Module Test:
**    - NO -
**    Reason: low complexity, easy to understand
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_GetAicSyncFromIPC(void)
{
   INPT_IPC_IO_DATA_UNION uIpcIoDataRx;

#ifdef RFID_ACTIVE
  ipcxSYS_GetBufferinclWait(IPCXSYS_IPC_ID_IO, (UINT8)sizeof(INPT_IPC_IO_DATA_STRUCT),
                            (UINT8*)&uIpcIoDataRx.sIoStruct);
  
  if (cfgSYS_GetControllerID() == SAFETY_CONTROLLER_2)
  {
    RFID_InfoSet(uIpcIoDataRx.sIoStruct.u32RfidInfo);
  }
#else
   uIpcIoDataRx.u32IpcPacket = ipcxSYS_GetUINT32inclWait( IPCXSYS_IPC_ID_IO );
#endif

   dolib_u8TestQualValuesOtherCh = 0x00u;
   diInput_u8DiTestQualValuesOtherCh = 0x00u;
   diInput_u8DiBitValuesOtherCh = 0x00u;
   gpio_u8IpcAicSyncReceive = uIpcIoDataRx.sIoStruct.u8AicSyncCnt;

}


/*------------------------------------------------------------------------------
**
** gpio_SamplePorts()
**
** Description:
**    function call to function that reads out all ports and stores them to a buffer.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_SamplePorts();
**
** Module Test:
**    - NO -
**    Reason: Only function call
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_SamplePorts(void)
{
   gpioSample_AllPorts(); /* see [SRS_337] */
}


/*------------------------------------------------------------------------------
**
** gpio_CyclicSignals()
**
** Description:
**    function call to toggle the life signals (used to trigger the voltage supervision
**    block of HW and the AIC HW block)
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    gpio_CyclicSignals();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_CyclicSignals(void)
{
   doLib_ToggleLifeAndScCcSelect(); /* see [SRS_40] */
}


/*------------------------------------------------------------------------------
**
** gpio_GetIpcAicSyncRx()
**
** Description:
**    returns the AIC sync value which received together with IO values in
**    one IPC message
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    UINT8: IPC sync value for AIC
**
** Usage:
**    x = gpio_GetIpcAicSyncRx();
**
** Module Test:
**    - NO -
**    Reason: easy to understand, low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
UINT8 gpio_GetIpcAicSyncRx(void)
{
   return gpio_u8IpcAicSyncReceive;
}


/*------------------------------------------------------------------------------
**
** gpio_SetIpcAicSyncTx()
**
** Description:
**    sets the AIC sync value. This value is send via IPC together with the IO
**    information via IPC in another function
**
**------------------------------------------------------------------------------
** Inputs:
**    UINT8 data: Value to be set
**
** Register Setting:
**    - None -
**
** Return:
**    UINT8: IPC sync value for AIC
**
** Usage:
**    gpio_SetIpcAicSyncTx(x);
**
** Module Test:
**    - NO -
**    Reason: easy to understand, low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_SetIpcAicSyncTx(CONST UINT8 data)
{
   /* Data may have all values therefore there is no input check required */
   gpio_u8IpcAicSyncSend = data;
}


/*------------------------------------------------------------------------------
**
** gpio_CheckDiDoDiagTimeout()
**
** Description:
**    calls functions to check the timeout of diagnostics
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
**
** Module Test:
**    - NO -
**    Reason: easy to understand, low complexity
**
** Context: main, while(FOREVER)
**------------------------------------------------------------------------------
*/
void gpio_CheckDiDoDiagTimeout(void)
{
   doLib_CheckDoDiagTimeout();
   diLib_CheckDiDiagTimeout();
}


/*------------------------------------------------------------------------------
**
** gpio_HandleToTestExecution()
**
** Description:
**    calls function to handle the TO-Test execution
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
**
** Module Test:
**    - NO -
**    Reason: easy to understand, low complexity
**
** Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void gpio_HandleToTestExecution(void)
{
   /* call function to execute TO-Test */
   diState_HandleToTestExecution();
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of gpio.c
**
********************************************************************************
*/

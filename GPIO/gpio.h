/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpio.h
**
** $Id: gpio.h 569 2016-08-31 13:08:08Z klan $
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
** Header file of module gpio.
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

#ifndef GPIO_H_
#define GPIO_H_


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

/*------------------------------------------------------------------------------
**  type-def for structure send via IPC containing the IO data and qualifier
**------------------------------------------------------------------------------
*/
typedef struct IPC_IO_DATA_TAG
{
   UINT8 u8SafeDiValues;  /* values of safe inputs */
   UINT8 u8SafeDiQual;    /* values of qualifier for safe inputs */
   UINT8 u8SafeDoQual;    /* values of qualifier for safe outputs */
   UINT8 u8AicSyncCnt;    /* sync value for AIC */
#ifdef RFID_ACTIVE
   UINT32 u32RfidInfo;     /* RFID information */
#endif /* RFID_ACTIVE */
} INPT_IPC_IO_DATA_STRUCT;


/*------------------------------------------------------------------------------
**  type-def for union used to make IO information send via IPC available
**  as UINT32 or as structure-parts. Helpful in send/read functions to get
**  an easier and faster access
**------------------------------------------------------------------------------
*/
/*lint -esym(960, 18.4)*/
typedef union IPC_IO_DATA
{
   INPT_IPC_IO_DATA_STRUCT sIoStruct;
   UINT32 u32IpcPacket;
} INPT_IPC_IO_DATA_UNION;
/*lint +esym(960, 18.4)*/

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

extern void  gpio_InitPorts(void);
extern void  gpio_InitModules(void);
extern void  gpio_SendIOvaluesViaIPC(void);
extern void  gpio_GetIOvaluesFromIPC(void);
extern void  gpio_GetAicSyncFromIPC(void);
extern void  gpio_SendAicSyncViaIPC(void);
extern void  gpio_ProcessDIs(void);
extern void  gpio_ProcessDOs(void);
extern void  gpio_SwitchOutputsOff(void);
extern void  gpio_SamplePorts(void);
extern void  gpio_CyclicSignals(void);
extern void  gpio_FilterDiRelValues(void);
extern UINT8 gpio_GetIpcAicSyncRx(void);
extern void  gpio_SetIpcAicSyncTx(CONST UINT8 data);
extern void  gpio_CheckDiDoDiagTimeout(void);
extern void  gpio_HandleToTestExecution(void);


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of gpio.h
**
********************************************************************************
*/

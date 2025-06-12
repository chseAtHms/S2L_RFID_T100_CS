/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** main.c
**
** $Id: main.c 4693 2024-11-06 10:18:57Z ankr $
** $Revision: 4693 $
** $Date: 2024-11-06 11:18:57 +0100 (Mi, 06 Nov 2024) $
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
** Application main routine
** This module contains the main routine for the program. The main-routine
** implements the system initialization as well as an endless loop for command
** interpretation.
** Additionally this module contains two functions (main_SystemInit and
** main_SetSysClockTo72) which are directly called by the startup-file and set
** the system clock settings. Therefore these routines are already executed when
** the main is called.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2011-2024 HMS Industrial Networks AB            **
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
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"
#include "version.h"

/* module definitions */
#include "globFail_Errorcodes.h"
#include "gpio.h"
#include "timer-def.h"


/* Module header */
#include "gpio-hal.h"
#include "timer-hal.h"
#include "timer-sys.h"
/* lint: usage of following files depends on configuration */
#include "adc_AnalogComparator-hal.h"   /*lint -efile(766, adc_AnalogComparator-hal.h)*/
#include "i2c_DigitalComparator-hal.h"  /*lint -efile(766, i2c_DigitalComparator-hal.h)*/

#include "ipcs_IpcSync-sys.h"
#include "ipcx_ExchangeData-sys_def.h"
#include "ipcx_ExchangeData-sys.h"

#include "stHan_SelftestHandler-srv.h"
#include "tm_TemperatureMonitor-srv.h"
#include "globFail_SafetyHandler.h"

#include "aicMsgDef.h"
#include "aicMsgCfg.h"
#include "aicMgrRx.h"
#include "aicMgrTx.h"
#include "aicSm.h"
#include "sch_Scheduler-app.h"
#include "clkCtrl.h"

#ifdef GLOBFIT_FITTEST_ACTIVE
   #include "globFit_FitTestHandler.h"
#endif

#ifdef __CTC_TESTING_ON__
#include "globCtc_CtcTestHandler.h"
#endif

/* Header-file of module */
#include "main_loc.h"
#include "main.h"

/* RSM_IGNORE_BEGIN Notice #1    - Physical line length > 100 characters */
#ifdef __DEBUG__
#warning "Debug build active. To activate the release build please undefine symbol __DEBUG__ in the compiler options."
#endif
/* RSM_IGNORE_END */

#include "SAPLbkgdMain.h"
#include "SAPLnvHigh.h"
#include "SAPLnvLow.h"
#include "SAPLfailCode.h"

/* HALCS headers */
#include "HALCSmsg_cfg.h"
#include "HALCSmsg.h"

/* TIM header */
#include "TIM.h"

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

/* This structure specifies the system timer. */
STATIC TIMERDEF_TIMER_CNT_CONFIG_STRUCT main_sSysTimConfig =
{
   TIMERDEF_TIMER3,        /*!< Timer 3 is the system timer. */
   (MAIN_SYS_CLOCK-1u),    /*!< Configure the timer to a system tick of 1 microsecond. */
   &main_sSysTimConfig     /*!< Pointer to itself. */
};



/* Calculation of prescaler value for TIMER4
* PLLCLK = HSE * 9 = 7,936 MHz * 9 = 71,424 MHz
* Since APB1 prescaler = 2, TIMxCLK = 71,424 MHz 
* Requested counter clock = 1/128*10-6s = 7812,5 Hz
* Prescaler = (71,424 MHz / 7812,5 Hz) - 1
* Prescaler = ~9141
*/
#define PRESCALER_TIMER4_128US_BASE     9141U


/* This structure specifies the timer/counter used for the 128us base period. */
STATIC TIMERDEF_TIMER_CNT_CONFIG_STRUCT main_s128usTimConfig =
{
   TIMERDEF_TIMER4,              /*!< Timer 4 is the system timer. */
   PRESCALER_TIMER4_128US_BASE,  /*!< Configure the timer to a period of ~128 microseconds. */
   &main_s128usTimConfig          /*!< Pointer to itself. */
};


/* This variable contains the software version and it is mapped to a specified
 * flash address (see also [SRS_72]). */
/* Version number can contain zeros. */
STATIC CONST UINT32 main_u32SwVersion __attribute__((at(VERSION_SW_VERSION_FLASH_ADDR))) 
  = VERSION_SW_VERSION;/*lint !e835 !e845*/


/* Ensure the correct ARM compiler version is used (see [SRS_2149]) */
/* The format of __ARMCC_VERSION is PVVbbbb where:
** - P is the major version
** - VV is the minor version
** - bbbb is the build number.
** The value can be obtained by calling armcc.exe --version_number
*/
#ifndef __arm__
  #warning "Invalid compiler used."
#elif (__ARMCC_VERSION != 5020028)
  #warning "Files compiled with invalid compiler version."
#endif

/* Unique file id used to build additional info */
#define k_FILEID      36u

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/


/*------------------------------------------------------------------------------
**
** main_Init()
**
** Description:
** This function initialize all modules.
**
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: Only register settings
**------------------------------------------------------------------------------
*/
STATIC void main_Init (void)
{
  /*  initialize modules of the HAL */
  timerHAL_InitCounter (&main_sSysTimConfig);
  /*  setup 128us base period used by CIP Safety */
  timerHAL_InitCounter (&main_s128usTimConfig);

  /* IO port (x) clock enable  */
  gpioHAL_Init();

  cfgSYS_Init();      /* read amongst others the controller ID */

  /* call initialisation of gpio module including DI, DO and Port/Pin initialisation */
#ifdef __DEBUG__
  gpioHAL_InitSwTestPort(); /* CCT_NO_PRE_WARNING, only used for Debug */
#endif /* __DEBUG__ */

#ifdef __CTC_TESTING_ON__
  globCtc_Init(); /* CCT_NO_PRE_WARNING, only used for CTC */
#endif

  /* call initialization of ports, covers [SRS_75] */
  gpio_InitPorts();

  /* call initialization of temperature measurement sensor */
  TEMPERATURE_HAL_INIT();

  schAPP_Init();

  /*
   * init the UART for the IPC and synch line
   */
  ipcxSYS_Init();     /* IPC UART      */
  ipcsSYS_Init();     /* synch line */

  /* configure the GPIO Pin PC10 for USART Tx */
  gpioHAL_ConfigureOutput( GPIOC,
                           GPIO_10,
                           OUT_CONFIG_GPIO_PP,
                           OUT_MODE_50MHZ );

  /* initialize modules of  the service layer */
  tmSRV_Init();

  /* initialize AIC and SPDU package */
  aicMgrTx_SysInit();

#ifdef GLOBFIT_FITTEST_ACTIVE
  globFit_Init(); /* CCT_NO_PRE_WARNING, only used for FIT test */
#endif

  /* initialize selftests, includes execution of startup tests (ROM, CPU),
   * see [SRS_689] */
  stHan_InitSelfTests();

  /* initialize clock control module */
  clkCtrl_Init();
}



/*------------------------------------------------------------------------------
**
** main()
**
** Description:
** main routine
** This is the main routine of the program. After the system initialization the
** program initializes the various modules and the whole application is started.
**
**
**------------------------------------------------------------------------------
** Inputs:
**    int
**
** Outputs:
**    - void -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: Only register settings
**------------------------------------------------------------------------------
*/
int main (void) /*lint !e970 */ /* Use of modifier or type 'int' outside of a typedef */
{
  /* just entered from bootloader, no CIP (Safety) communication available of course
  ** (see [SRS_2239]).
  */

  /* call initialization of modules */
  main_Init();

  /* just finished selftests, no CIP (Safety) communication available of course
  ** (see [SRS_2240]).
  */

  /* init NV (non-volatile) data handling
  ** Attention: shall be called before uCs are synchronized because a big drift
  ** is possible (e.g. flash erase).
  */
  SAPL_NvHighInit();
  
  /* init Failure Log */
  SAPL_FailCodeInit();
  
  /* check/compare configuration (incl. HW-ID) of both uCs */
  /* the uCs are synchronized first here */
  main_Config();
  
  /* check/compare CRC16 of flash block of both uCs.
  ** Attention: shall be done after uCs are synchronized to prevent IPC timeout.
  */
  SAPL_NvLowCrcSync();
  
  /* exchange Failure Code of controllers
  ** Attention: shall be done after uCs are synchronized to prevent IPC timeout.
  */
  SAPL_FailCodeSync();
  
  /* init HALCS modules and prepare message queue before starting CSS (calling IXSCC_Init) 
  ** (see [3.2-2:]). The queue ensures all messages generated during startup are stored 
  ** and forwarded (see [3.2-3:])
  ** Attention: shall be done before IRQ scheduler is enabled, because functionality
  ** accessed from IRQ scheduler.
  ** */
  HALCS_MsgInit();

  /* init timer unit (128us CSS timer)
  ** Attention: shall be done before IRQ scheduler is enabled, because functionality
  ** accessed from IRQ scheduler.
  */
  TIM_Init();
  
  /* start main scheduler */
  main_Online();    /* start of TIM1_UP_IRQHandler() in sch_scheduler_app.c  */

  do /* endless main loop */
  {
    /* current AIC state */
    AICSM_STATE_ENUM e_aicState;
    
    /********************************/
    /* execution of background task */
    /********************************/
    SAPL_BkgdTaskExec(); 

    /* Measure max. time slice touched */
    schAPP_TriggerCycleStatistic();

    /* check the timeout of the selftests, see [SRS_665] */
    stHan_CheckSelfTestTimeout();
    
    /* make temp. copy because AIC state is changed from IRQ context, read access is 
    ** considered as 'atomic' here */
    e_aicState = aicSm_eAicState;
    
    /* Attention: Check of IO diagnostic is also done in 'IDLE' state, because
    ** RUN-Scheduler (incl. IO processing) is already executed here.
    */

    if ( (AICSM_AIC_EXEC_PROD_CONS == e_aicState) ||
         (AICSM_AIC_EXEC_PROD_ONLY == e_aicState) ||
         (AICSM_AIC_EXEC_CONS_ONLY == e_aicState) ||
         (AICSM_AIC_IDLE == e_aicState) )
    {
      /* check timeout of safe IO diagnostics */
      /* ATTENTION: The functions diLib_CheckDiDiagTimeout() and doLib_CheckDoDiagTimeout() called
      ** inside gpio_CheckDiDoDiagTimeout() have access to RDS variables which are also written 
      ** from IRQ context via doLib_Init() respectively diLib_Init(). To avoid any conflicts, these
      ** functions shall not be called at the same time when gpio_CheckDiDoDiagTimeout() is 
      ** executed. This ensured here, because diLib_Init() and doLib_Init() are always called 
      ** (inside gpio_InitModules()) BEFORE AIC state AICSM_AIC_IDLE is entered!
      */
      gpio_CheckDiDoDiagTimeout();
    }
    else
    {
      /* empty branch */
    }

   #ifdef __CTC_TESTING_ON__
    globCtc_CtcTestHandler(); /* CCT_NO_PRE_WARNING, only used for FIT test */
   #endif

   #ifdef GLOBFIT_FITTEST_ACTIVE
    globFit_FitTestHandler(); /* CCT_NO_PRE_WARNING, only used for FIT test */

    GLOBFIT_FITTEST(GLOBFIT_CMD_UNUSED_INTERRUPT,
       EXTI15_10_IRQHandler());
   #endif

    /* this shall be the last line in main loop, trigger execution of main */
    schAPP_TriggerMainExecTime();
    
  } while(FOREVER); /*lint !e506, 'Constant value Boolean', OK because endless main loop */  
  
}


/*------------------------------------------------------------------------------
**
** main_SystemInit()
**
** Description:
** This function initializes the microcontroller system.
** This function performs the setup of the microcontroller system. It
** initialize the PLL and updates the SystemCoreClock variable. This function
** is directly called from the startup file.
**
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: Only register settings
**------------------------------------------------------------------------------
*/
void main_SystemInit (void)
{
  /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
  /* Set HSION bit */
  RCC->CR |= (UINT32) 0x00000001uL;

  /* Make sure the LSION bit is cleared (internal low speed oscillator disabled) */
  RCC->CSR &= (UINT32) 0xFFFFFFFEuL;

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CFGR &= (UINT32) 0xF0FF0000uL;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (UINT32) 0xFEF6FFFFuL;

  /* Reset HSEBYP bit */
  RCC->CR &= (UINT32) 0xFFFBFFFFuL;

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= (UINT32) 0xFF80FFFFuL;

  /* Disable all interrupts and clear pending bits  */
  /* For devices of the 'High-density performance line' like the STM32F103ZCH6, 
  ** Bits 22:21 of RCC_CIR are reserved and must be kept at reset value.
  ** This is also done inside reference code of STM32F10x_StdPeriph_Template
  ** (V3.5.0), file 'system_stm32f10x.c'.
  ** For 'High-density performance line' devices the define STM32F10X_HD must be
  ** set. This is done in the uVision project here. 
  */
  RCC->CIR = 0x009F0000uL;

  /* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
  /* Configure the Flash Latency cycles and enable prefetch buffer */
  main_SetSysClockTo72();
  
  /* In the reference code of STM32F10x_StdPeriph_Template (V3.5.0), file 
  ** 'system_stm32f10x.c' the Vector Table Offset Register (SCB->VTOR) is configured
  ** at this place. This is not done in this application, because VTOR is already 
  ** configured by bootloader SW.
  */
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** main_SetSysClockTo72()
**
** Description:
** This function sets the system clock frequency to 24MHz and configures HCLK,
** PCLK2 and PCLK1 prescalers.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    -
**
** Module Test: - NO -
**      Reason: Only register settings
**------------------------------------------------------------------------------
*/
STATIC void main_SetSysClockTo72(void)
{

  __IO UINT32 StartUpCounter = 0u;
  __IO UINT32 HSEStatus = 0u;

  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
  /* Enable HSE */
  RCC->CR |= ((UINT32)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  }
  /* Note 960: Violates MISRA 2004 Required Rule 12.4, side effects on right hand of logical 
   *  operator: '&&' --> no side effects. */
  while( (StartUpCounter != (UINT32)HSE_STARTUP_TIMEOUT) && (HSEStatus == 0u ) );/*lint !e960*/

  if ((RCC->CR & RCC_CR_HSERDY) != (UINT32)RESET)
  {
    HSEStatus = (UINT32)0x01;
  }
  else
  {
    HSEStatus = (UINT32)0x00;
  } /* CCT_SKIP */ /* HSE start-up error, leads in endless loop afterwards */

  if (HSEStatus == (UINT32)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH->ACR |= (UINT32)FLASH_ACR_PRFTBE;

    /* Flash 2 wait state */
    FLASH->ACR &= (UINT32)(~(UINT32)FLASH_ACR_LATENCY);
    FLASH->ACR |= (UINT32)FLASH_ACR_LATENCY_2;

    /* Lint message 835 deactivated because register bit definition is zero. */
    
    /* HCLK = SYSCLK */
    RCC->CFGR |= (UINT32)RCC_CFGR_HPRE_DIV1;/*lint !e835*/

    /* PCLK2 = HCLK */
    RCC->CFGR |= (UINT32)RCC_CFGR_PPRE2_DIV1;/*lint !e835*/

    

    /* PCLK1 = HCLK / 2 */
    RCC->CFGR |= (UINT32)RCC_CFGR_PPRE1_DIV2;

    /*  PLL configuration: PLLCLK = HSE * 9 = 72 MHz */
    RCC->CFGR &= (UINT32)((UINT32)~( RCC_CFGR_PLLSRC
                                       | RCC_CFGR_PLLXTPRE
                                       | RCC_CFGR_PLLMULL ) );

    RCC->CFGR |= (UINT32)( RCC_CFGR_PLLSRC
                           | RCC_CFGR_PLLMULL9 );

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0u)
    {
    }

    /* Select PLL as system clock source */
    RCC->CFGR &= ( UINT32 )( ( UINT32 )~( RCC_CFGR_SW ) );
    RCC->CFGR |= ( UINT32 )RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (UINT32)RCC_CFGR_SWS) != (UINT32)0x08)
    {
    } /* CCT_SKIP */ /* wait time */
  }
  else
  { 
    /* If HSE fails to start-up, the application will have wrong clock configuration.
    ** It seems to be the safest reaction here to 'do nothing' and enter an endless loop.
    ** Because this, the other controller enters failsafe state due to missing synchronization.   
    */
    
    /* Lint message 716 deactivated because while(1) loop wanted here */
    while(FOREVER) /*lint !e716*/
    {
      ;
    } /* CCT_SKIP */ /* endless loop in case of HSE start-up error */
  } /* CCT_SKIP */ /* HSE start-up error, endless loop */
}


/*------------------------------------------------------------------------------
**
** main_online()
**
** Description:
** This function starts the scheduler.
**
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    schAPP_StartScheduler
**
** Module Test: - NO -
**      Reason: Only register settings
**------------------------------------------------------------------------------
*/
STATIC void main_Online (void)
{
  schAPP_StartScheduler();
}


/*------------------------------------------------------------------------------
**
** main_Config()
**
** Description:
** This function reads the configuration and the software version and sends the
** data to the other controller. The received data of the other controller is
** checked and if it is not equal the Safety Handler will be called.
**
**------------------------------------------------------------------------------
** Inputs:
**    - void -
**
** Outputs:
**    - void -
**
** Usage:
**    cfgSYS_GetDiDoConfig()
**    ipcx_ExChangeData()
**
**
** Module Test: - NO -
**      Reason: low complexity
**------------------------------------------------------------------------------
*/
STATIC void main_Config (void)
{
  UINT8    u8DiDoCfg;
  UINT32   u32SendCfgData;
  UINT32   u32RecvCfgData;

  /* get the DI and DO config see [SRS_280] */
  u8DiDoCfg = cfgSYS_GetDiDoConfig();

  /* check of DI and DO config against the Module ID, both must be on the same
   * value to ensure that the firmware matches with the hardware, see [SRS_280] */
  
  /* if mismatch detected */
  if (u8DiDoCfg != (UINT8)(VERSION_ASM_MODULE_ID & 0x00FFu))
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_CONFIG, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: OK */
  else
  {
    /* get the software version and set it with the DI and DO config to the IPC
     * telegram for exchange with the other controller (see also [SRS_520], [SRS_280]) */
    u32SendCfgData = (UINT32)(main_u32SwVersion << 8u) + (UINT32)u8DiDoCfg;
  
    /* inserted wait time because of initialization of IPC, in some cases it is
     * not ready */
    timerSYS_wait(2000u);
  
    /* sync the controllers the first time, see [SRS_76] */
    ipcsSYS_SyncStartup();
  
    /* exchange the data (see also [SRS_520], [SRS_280]) */
    u32RecvCfgData = ipcxSYS_ExChangeUINT32Data(u32SendCfgData, IPCXSYS_IPC_ID_CONFIG);
  
    /* check if the received data is equal to the own data, if not go to Safety
     * Handler (see also [SRS_520], [SRS_280]) */
    
    /* if mismatch between sent and received data */
    if (u32RecvCfgData != u32SendCfgData)
    {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_CONFIG, GLOBFAIL_ADDINFO_FILE(2u));
    }
    /* else: OK */
    else
    {
      /* empty branch */
    }
  }
}


/*******************************************************************************
**
** End of main.c
**
********************************************************************************
*/

/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SfrTest_cfg.c 4109 2023-11-21 16:30:41Z ankr $
 * $Revision: 4109 $
 *     $Date: 2023-11-21 17:30:41 +0100 (Di, 21 Nov 2023) $
 *   $Author: ankr $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SfrTest_cfg.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Config file for the sfr test.
 * @description:  This module contains the configuration of the SFR-Test which
 *                contains the SFRs to test and the specified masks to get the
 *                correct value for calculating the checksum.
 *                Furthermore the specified checksum (or checksums) for
 *                calculating the checksum over the SFRs are also defined in
 *                this module.
 *                The SFRs and the specified masks are defined in the document
 *                SFR-Map_HMS_ASM.xls.
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"
#include "globDef_GlobalDefinitions.h"
#include "cfg_config-sys.h"

/* Module header */
#include "SFRCRC_typ.h"

/* Header-file of module */
#include "SfrTest_cfg.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

/*lint -save -e708*/

/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! Const CRC for the Config Registers of uC1 and uC2. The CRC is calculated
 * before compiling the firmware. */
/* SFR Test CRC depends on which peripherals are used */
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
   #ifdef GLOBFIT_FITTEST_ACTIVE
      #ifdef GLOBFIT_FITTEST_STARTUP_ACTIVE
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0x92C0CC75uL; /*!< CRC1 of the uC1. */
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0x92C0CC75uL; /*!< CRC1 of the uC2. */
      #else
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0xBC9D2F79uL; /*!< CRC1 of the uC1. */
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0xBC9D2F79uL; /*!< CRC1 of the uC2. */
      #endif
   #else
      STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0xC6449F4BuL; /*!< CRC1 of the uC1, see [SRS_459].*/
      STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0xC6449F4BuL; /*!< CRC1 of the uC2, see [SRS_459].*/
   #endif
#else
   #ifdef GLOBFIT_FITTEST_ACTIVE
      #ifdef GLOBFIT_FITTEST_STARTUP_ACTIVE
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0x0C4B5D69uL; /*!< CRC1 of the uC1. */
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0x0C4B5D69uL; /*!< CRC1 of the uC2. */
      #else
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0xA10EADF6uL; /*!< CRC1 of the uC1. */
         STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0xA10EADF6uL; /*!< CRC1 of the uC2. */
      #endif
   #else
      STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC1 = 0x755D2D47uL; /*!< CRC1 of the uC1, see [SRS_459].*/
      STATIC CONST SFRCRC sfrTstCfg_CfgCrcuC2 = 0x755D2D47uL; /*!< CRC1 of the uC2, see [SRS_459].*/
   #endif
#endif

/*! CRC for the Param Registers, the CRC will be calculated at runtime. */
/*STATIC SFRCRC sfrTstCfg_ParamCrc = 0x00000000uL;*//*lint !e843*//* Avoid lint
message because value is changed during initialization of the firmware. */

/*----------------------------------------------------------------------------*/
/* Config Mask Arrays for both uCs                                            */
/*----------------------------------------------------------------------------*/

/*! Mask-Array for Register WWDG */
STATIC CONST SFR32 sfrTstCfg_au32WwdgMask [2] =
{
   0x00000080u,/* [0] WWDG_CR */
   0x000003FFu /* [1] WWDG_CFR */
               /* [2] WWDG_SR */
};

/*! Mask-Array for Register AFIO */
STATIC CONST SFR32 sfrTstCfg_au32AfioMask [7] =
{
   0x000000FFu,/* [0] AFIO_EVCR */
   0x001FFFFFu,/* [1] AFIO_MAPR */
   0x0000FFFFu,/* [2] AFIO_EXTICR1 */
   0x0000FFFFu,/* [3] AFIO_EXTICR2 */
   0x0000FFFFu,/* [4] AFIO_EXTICR3 */
   0x0000FFFFu,/* [5] AFIO_EXTICR4 */
               /* [6] not used     */
   0x000007E0u /* [7] AFIO_MAPR2 */
};

/*! Mask-Array for Register GPIOA */
STATIC CONST SFR32 sfrTstCfg_au32GpioAMask [1] =
{
               /* [0] GPIOA_CRL  */
               /* [1] GPIOA_CRH  */
               /* [2] GPIOA_IDR  */
   0xFFFFEF8Fu /* [3] GPIOA_ODR  */
               /* [4] GPIOA_BSRR */
               /* [5] GPIOA_BRR  */
               /* [6] GPIOA_LCKR */
};

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
   /*! Mask-Array for Register GPIOB */
   STATIC CONST SFR32 sfrTstCfg_au32GpioBMask [1] =
   {
                  /* [0] GPIOB_CRL  */
                  /* [1] GPIOB_CRH  */
                  /* [2] GPIOB_IDR  */
      0xFFFFFE1Fu /* [3] GPIOB_ODR  */
                  /* [4] GPIOB_BSRR */
                  /* [5] GPIOB_BRR  */
                  /* [6] GPIOB_LCKR */
   };
#else
   /*! Mask-Array for Register GPIOB */
   STATIC CONST SFR32 sfrTstCfg_au32GpioBMask [1] =
   {
                  /* [0] GPIOB_CRL  */
                  /* [1] GPIOB_CRH  */
                  /* [2] GPIOB_IDR  */
      0xFFFFFEDFu /* [3] GPIOB_ODR  */
                  /* [4] GPIOB_BSRR */
                  /* [5] GPIOB_BRR  */
                  /* [6] GPIOB_LCKR */
   };
#endif

/*! Mask-Array for Register GPIOC */
STATIC CONST SFR32 sfrTstCfg_au32GpioCMask [1] =
{
               /* [0] GPIOC_CRL  */
               /* [1] GPIOC_CRH  */
               /* [2] GPIOC_IDR  */
   0xFFFF7EFFu /* [3] GPIOC_ODR  */
               /* [4] GPIOC_BSRR */
               /* [5] GPIOC_BRR  */
               /* [6] GPIOC_LCKR */
};

/*! Mask-Array for Register GPIOD */
STATIC CONST SFR32 sfrTstCfg_au32GpioDMask [1] =
{
               /* [0] GPIOD_CRL  */
               /* [1] GPIOD_CRH  */
               /* [2] GPIOD_IDR  */
   0xFFFFFFFBu /* [3] GPIOD_ODR  */
               /* [4] GPIOD_BSRR */
               /* [5] GPIOD_BRR  */
               /* [6] GPIOD_LCKR */
};

#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
  /*! Mask-Array for Register I2C1 */
  STATIC CONST SFR32 sfrTstCfg_au32I2c1Mask [1] =
  {
    0xFFFFF0FFu  /* [0] I2C_CR1   */
                 /* [1] I2C_CR2   */
                 /* [2] I2C_OAR1   */
                 /* [3] I2C_OAR2 */
                 /* [4] I2C_DR */
                 /* [5] I2C_SR1 */
                 /* [6] I2C_SR2 */
                 /* [7] I2C_CCR */
                 /* [8] I2C_TRISE */
   };
#else
  /*! Mask-Array for Register ADC1 */
  STATIC CONST SFR32 sfrTstCfg_au32Adc1Mask [1] =
  {
                 /* [0] ADC_SR    */
                 /* [1] ADC_CR1   */
     0xFF9FFFF3u /* [2] ADC_CR2   */
                 /* [3] ADC_SMPR1 */
                 /* [4] ADC_SMPR2 */
                 /* [5] ADC_JOFR1 */
                 /* [6] ADC_JOFR2 */
                 /* [7] ADC_JOFR3 */
                 /* [8] ADC_JOFR4 */
                 /* [9] ADC_HTR   */
                 /* [10] ADC_LTR   */
                 /* [11] ADC_SQR1  */
                 /* [12] ADC_SQR2  */
                 /* [13] ADC_SQR3  */
                 /* [14] ADC_JSQR  */
                 /* [15] ADC_JDR1  */
                 /* [16] ADC_JDR2  */
                 /* [17] ADC_JDR3  */
                 /* [18] ADC_JDR4  */
                 /* [19] ADC_DR    */
  };
#endif

/*! Mask-Array for Register DMA1 */
STATIC CONST SFR32 sfrTstCfg_au32Dma1Mask [2] =
{
               /* [0] DMA_ISR    */
               /* [1] DMA_IFCR   */
               /* [2] DMA_CCR1   */
               /* [3] DMA_CNDTR1 */
               /* [4] DMA_CPAR1  */
               /* [5] DMA_CMAR1  */
               /* [6] reserved   */
               /* [7] DMA_CCR2   */
               /* [8] DMA_CNDTR2 */
               /* [9] DMA_CPAR2  */
               /* [10] DMA_CMAR2  */
               /* [11] reserved  */
               /* [12] DMA_CCR3   */
               /* [13] DMA_CNDTR3 */
               /* [14] DMA_CPAR3  */
               /* [15] DMA_CMAR3  */
               /* [16] reserved   */
   0xFFFFFFFEu,/* [17] DMA_CCR4   */
               /* [18] DMA_CNDTR4 */
               /* [19] DMA_CPAR4  */
               /* [20] DMA_CMAR4  */
               /* [21] reserved   */
   0xFFFFFFFEu /* [22] DMA_CCR5   */
               /* [23] DMA_CNDTR5 */
               /* [24] DMA_CPAR5  */
               /* [25] DMA_CMAR5  */
               /* [26] reserved   */
               /* [27] DMA_CCR6   */
               /* [28] DMA_CNDTR6 */
               /* [29] DMA_CPAR6  */
               /* [30] DMA_CMAR6  */
               /* [31] reserved   */
};

/*! Mask-Array for Register DMA2 */
STATIC CONST SFR32 sfrTstCfg_au32Dma2Mask [2] =
{
               /* [0] DMA_ISR    */
               /* [1] DMA_IFCR   */
               /* [2] DMA_CCR1   */
               /* [3] DMA_CNDTR1 */
               /* [4] DMA_CPAR1  */
               /* [5] DMA_CMAR1  */
               /* [6] reserved   */
               /* [7] DMA_CCR2   */
               /* [8] DMA_CNDTR2 */
               /* [9] DMA_CPAR2  */
               /* [10] DMA_CMAR2  */
               /* [11] reserved  */
   0xFFFFFFFEu,/* [12] DMA_CCR3   */
               /* [13] DMA_CNDTR3 */
               /* [14] DMA_CPAR3  */
               /* [15] DMA_CMAR3  */
               /* [16] reserved   */
               /* [17] DMA_CCR4   */
               /* [18] DMA_CNDTR4 */
               /* [19] DMA_CPAR4  */
               /* [20] DMA_CMAR4  */
               /* [21] reserved   */
   0xFFFFFFFEu /* [22] DMA_CCR5   */
               /* [23] DMA_CNDTR5 */
               /* [24] DMA_CPAR5  */
               /* [25] DMA_CMAR5  */
               /* [26] reserved   */
               /* [27] DMA_CCR6   */
               /* [28] DMA_CNDTR6 */
               /* [29] DMA_CPAR6  */
               /* [30] DMA_CMAR6  */
               /* [31] reserved   */
};

STATIC CONST SFR32 sfrTstCfg_au32RccMask [4] =
{
   0xFDFD00FDu,/* [0] RCC_CR       */
   0xFFFFFFF3u,/* [1] RCC_CFGR     */
   0xFF00FF00u,/* [2] RCC_CIR      */
               /* [3] RCC_APB2RSTR */
               /* [4] RCC_APB1RSTR */
               /* [5] RCC_AHBENR   */
               /* [6] RCC_APB2ENR  */
               /* [7] RCC_APB1ENR  */
               /* [8] RCC_BDCR     */
   0x00000001u /* [9] RCC_CSR      */
};

/*----------------------------------------------------------------------------*/
/* Config Block Array for both uCs                                            */
/*----------------------------------------------------------------------------*/
/* Array with the Config SFRs. *//*lint -e928 -e960*/
/* Note 928: cast from pointer to pointer [possibly violation MISRA 2004 Rule
 * 11.4], [MISRA 2004 Rule 11.4]
 * --> Cast of the Register to the variable is wanted.
 * Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away
 * const/volatile from a pointer or reference
 * --> Explizit wanted. */
STATIC SFRTEST_BLOCKDESCRIPTOR_STRUCT sfrTstCfg_asMapConfigBlock [SFRTEST_MAX_SIZEOF_CFG_BLOCKS] =
{
   {/* [0] === TIM2 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x000DEDCFu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8*) TIM2},                     /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [1] === TIM3 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x000DEDCFu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8*) TIM3},                     /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [2] === TIM4 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x000DEDCFu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) TIM4},                    /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [3] === TIM5 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x000DEDCFu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) TIM5},                    /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [4] === WWDG ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      3u,                                 /* u32RegCount    */
      2u,                                 /* u32MaskCount   */
      0x00000003u,                        /* u32RegSel      */
      0x00000003u,                        /* u32MaskSel     */
      {(SFR8 *) WWDG},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32WwdgMask}   /* pu32Mask       */
   },
   {/* [5] === UART4 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000007Cu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) UART4},                   /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
   {/* [6] === I2C1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      9u,                                 /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0000018Fu,                        /* u32RegSel      */
      0x00000001u,                        /* u32MaskSel     */
      {(SFR8 *) I2C1},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32I2c1Mask}   /* pu32Mask       */
   },
#else
   /* I2C1 not used */
#endif
   {/* [6] or [7] === AFIO ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      8u,                                 /* u32RegCount    */
      7u,                                 /* u32MaskCount   */
      0x000000BFu,                        /* u32RegSel      */
      0x000000BFu,                        /* u32MaskSel     */
      {(SFR8 *) AFIO},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32AfioMask}   /* pu32Mask       */
   },
   {/* [7] or [8] === GPIOA ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000008u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOA},                   /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32GpioAMask}  /* pu32Mask       */
   },
   {/* [8] or [9] === GPIOB ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000008u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOB},                   /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32GpioBMask}  /* pu32Mask       */
   },
   {/* [9] or [10] === GPIOC ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000008u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOC},                   /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32GpioCMask}  /* pu32Mask       */
   },
   {/* [10] or [11] === GPIOD ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000008u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOD},                   /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32GpioDMask}  /* pu32Mask       */
   },
   {/* [11] or [12] === GPIOE ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOE},                   /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [12] or [13] === GPIOF ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOF},                   /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [13] or [14] === GPIOG ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000004Bu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) GPIOG},                   /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
#if (CFG_TEMPERATURE_SENSOR_DIGITAL == TRUE)
   /* ADC1 not used */
#else
   {/* [14] === ADC1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      1u,                                 /* u32MaskCount   */
      0x0007FFFEu,                        /* u32RegSel      */
      0x00000004u,                        /* u32MaskSel     */
      {(SFR8 *) ADC1},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32Adc1Mask}   /* pu32Mask       */
   },
#endif
   {/* [15] === TIM1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      20u,                                /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x000FFDCFu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) TIM1},                    /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [16] === USART1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      7u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000007Cu,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) USART1},                  /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   },
   {/* [17] === DMA1 - 1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      32u,                                /* u32RegCount    */
      2u,                                 /* u32MaskCount   */
      0x6842D6B4u,                        /* u32RegSel      */
      0x00420000u,                        /* u32MaskSel     */
      {(SFR8 *) DMA1},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32Dma1Mask}   /* pu32Mask       */
   },
   {/* [18] === DMA1 - 2 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      4u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x0000000Du,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) DMA1_Channel7_BASE},      /* pu32Reg        */
      {NULL}   /* pu32Mask       */
   },
   {/* [19] === DMA2 - 1 ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      26u,                                /* u32RegCount    */
      2u,                                 /* u32MaskCount   */
      0x014A56B4u,                        /* u32RegSel      */
      0x00401000u,                        /* u32MaskSel     */
      {(SFR8 *) DMA2},                    /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32Dma2Mask}   /* pu32Mask       */
   },
   {/* [20] === RCC ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      10u,                                /* u32RegCount    */
      4u,                                 /* u32MaskCount   */
      0x000003FFu,                        /* u32RegSel      */
      0x00000207u,                        /* u32MaskSel     */
      {(SFR8 *) RCC},                     /* pu32Reg        */
      {(SFR8 *) sfrTstCfg_au32RccMask}    /* pu32Mask       */
   },
   {/* [21] === FLASH ===  */
      SFR_REGSIZE_32BIT,                  /* u16RegSize     */
      9u,                                 /* u32RegCount    */
      0u,                                 /* u32MaskCount   */
      0x00000001u,                        /* u32RegSel      */
      0x00000000u,                        /* u32MaskSel     */
      {(SFR8 *) FLASH},                   /* pu32Reg        */
      {NULL}                              /* pu32Mask       */
   }
};
/* activate the specified lint messages. *//*lint +e928 +e960*/

/*lint -e960*/
/* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away
 * const/volatile from a pointer or reference
 * --> Explizit wanted. */
/*! Array with the config and param SFRs, it is used for the CRC calculation. */
CONST SFRTEST_REGISTERMAP_STRUCT sfrTstCfg_asMap[SFRTEST_CFG_DIM] =
{
   {/* Config SFR for uC1 */
      SFRTEST_MAX_SIZEOF_CFG_BLOCKS,            /* u32BlockCount    */
      sfrTstCfg_asMapConfigBlock,               /* pBlock         */
      TRUE,                                     /* bIsConstCrc    */
      &sfrTstCfg_CfgCrcuC1,                     /* puSfrCrc       */
   },
   {/* Config SFR for uC2 */
      SFRTEST_MAX_SIZEOF_CFG_BLOCKS,            /* u32BlockCount    */
      sfrTstCfg_asMapConfigBlock,               /* pBlock         */
      TRUE,                                     /* bIsConstCrc    */
      &sfrTstCfg_CfgCrcuC2,                     /* puSfrCrc       */
   }
};
/* activate the specified lint messages. *//*lint +e960*/

/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/



/***** End of: globvar Moduleglobal Variables ***************************//*@}*/


/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Local Functions                                                            */
/********************************//*!@addtogroup locfunc Local Functions*//*@{*/



/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/

/*lint -restore*/

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

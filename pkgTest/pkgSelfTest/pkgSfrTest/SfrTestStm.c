/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SfrTestStm.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SfrTestStm.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Contains the state machine of the SFR test.
 * @description:  -
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

/* Module header */
#include "stDef_SelftestDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "SFRCRC_typ.h"  /*sfr type definitions*/
#include "SfrTest_cfg.h" /*sfr config file*/
#include "sfrerrhnd_Interface.h"  /*interface for errorhandler*/
#include "sfrcrc_Interface.h"

/* Header-file of module */
#include "SfrTestStm.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/


/***** End of: moduleglobvar Moduleglobal Variables *********************//*@}*/

/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

/*! If this compiler switch is set to TRUE, the SFR CRC is logged block by block
 *  into the specified array. */
#if SFRTEST_ENABLE_BLOCK_CRC_LOG_FOR_DEBUG == TRUE
#warning "Compiler switch SFRTEST_ENABLE_BLOCK_CRC_LOG_FOR_DEBUG active!!!"
STATIC UINT32 sfrTestStm_au32CrcLog[SFRTEST_MAX_SIZEOF_CFG_BLOCKS];
#endif

/***** End of: globvar Moduleglobal Variables ***************************//*@}*/

/******************************************************************************/
/* Function-Prototypes                                                        */
/******************************************************************************/
STATIC void sfrTestStm_CalcUint8Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_CalcUint16Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_CalcUint32Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC SFRTEST_STATE_ENUM sfrTestStm_onExecute ( CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_onConfirm ( CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_onReset ( CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_onInit ( CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_onResetRef ( CONST SFRTEST_CFG_STRUCT * SfrCfg);
STATIC void sfrTestStm_StmStateNotInitialized (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                               CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC void sfrTestStm_StmStateSamplingRef (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                            CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC void sfrTestStm_StmStateRefDone (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                        CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC void sfrTestStm_StmStateRefConfirmed (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                             CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC void sfrTestStm_StmStateSampling (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                         CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC void sfrTestStm_StmStateDone (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                     CONST SFRTEST_EVENT_ENUM SfrEvent);
STATIC SFRTEST_STATE_ENUM sfrTestStm_Statemachine (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                                   CONST SFRTEST_EVENT_ENUM SfrEvent);

/***** End of: Function-Prototypes ********************************************/

/******************************************************************************/
/* Local Functions                                                            */
/********************************//*!@addtogroup locfunc Local Functions*//*@{*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_CalcUint8Crc

 @Description         This function calculates the CRC8 to a given 8 Bit
                      Register value.

 @note                This function fetches the specified register value from
                      the memory. After that it is check if there is defined a
                      mask for this register. If there is one, the unused bits
                      in the register are masked out. So they are not used for
                      the CRC calculation. Then the value is given to the
                      CRC calculation.

 @Parameter
 @param [in]          SfrCfg      the config data for the sfr test
 @return              void
 */
/******************************************************************************/
STATIC void sfrTestStm_CalcUint8Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   UINT8 u8Value;

   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960, 17.4)*/
   /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    * other than array indexing used
    * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */

   /*get the specified register value*/
   u8Value = SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
      .Reg.pu8Reg [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)];

   /*check if the selected register is masked*/
   if ( SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u32MaskSel
      & (1uL << RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)))
   {
      u8Value &= SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
         .Mask.pu8Mask [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsMaskIndex)];
      RDS_INC(SfrCfg->pTestData->u32RdsMaskIndex);
   }

   /*give the value to the CRC calculation*/
   SfrCfg->pTestData->uCrc = SFRCRC_AddU8(SfrCfg->pTestData->uCrc, u8Value);

   /*lint +esym(960, 17.4)*//* activate Misra Rule 17.4 */
}/*end of function sfrTestStm_CalcUint8Crc*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_CalcUint16Crc

 @Description         This function calculates the CRC16 to a given 16 Bit
                      Register value.

 @note                This function fetches the specified register value from
                      the memory. After that it is check if there is defined a
                      mask for this register. If there is one, the unused bits
                      in the register are masked out. So they are not used for
                      the CRC calculation. Then the value is given to the
                      CRC calculation.

 @Parameter
 @param [in]          SfrCfg      the config data for the sfr test
 @return              void
 */
/******************************************************************************/
STATIC void sfrTestStm_CalcUint16Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   UINT16 u16Value;

   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960, 17.4)*/
   /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    * other than array indexing used
    * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */

   /*get the specified register value*/
   u16Value = SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
      .Reg.pu16Reg [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)];

   /*check if the selected register is masked*/
   if (SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u32MaskSel
      & (1uL << RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)))
   {
      u16Value &= SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
         .Mask.pu16Mask [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsMaskIndex)];
      RDS_INC(SfrCfg->pTestData->u32RdsMaskIndex);
   }

   /*give the value to the CRC calculation*/
   SfrCfg->pTestData->uCrc = SFRCRC_AddU16(SfrCfg->pTestData->uCrc, u16Value);

   /*lint +esym(960, 17.4)*//* activate Misra Rule 17.4 */
}/*end of function sfrTestStm_CalcUint16Crc*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_CalcUint32Crc

 @Description         This function calculates the CRC32 to a given 32 Bit
                      Register value.

 @note                This function fetches the specified register value from
                      the memory. After that it is check if there is defined a
                      mask for this register. If there is one, the unused bits
                      in the register are masked out. So they are not used for
                      the CRC calculation. Then the value is given to the
                      CRC calculation.

 @Parameter
 @param [in]          SfrCfg      the config data for the sfr test
 @return              void
 */
/******************************************************************************/
STATIC void sfrTestStm_CalcUint32Crc (CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   UINT32 u32Value;

   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960, 17.4)*/
   /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    * other than array indexing used
    * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */

   /*get the specified register value*/
   u32Value = SfrCfg->pSfrMap->pBlock[RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
      .Reg.pu32Reg[RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)];

   /*check if the selected register is masked*/
   if ( SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u32MaskSel
      & (1uL << RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)))
   {
      u32Value &= SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)] \
         .Mask.pu32Mask [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsMaskIndex)];

      RDS_INC(SfrCfg->pTestData->u32RdsMaskIndex);
   }

   /*give the value to the CRC calculation*/
   SfrCfg->pTestData->uCrc = SFRCRC_AddU32(SfrCfg->pTestData->uCrc, u32Value);

   /*lint +esym(960, 17.4)*//* activate Misra Rule 17.4 */
}/*end of function sfrTestStm_CalcUint32Crc*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_onExecute

 @Description         Initializes current CRC with start value and the reference
                      CRC with the inverted start value. If test is complete,
                      either SFRTEST_STATE_DONE or SFRTEST_STATE_ERROR is
                      returned, else SFRTEST_STATE_SAMPLING.

 @note                -

 @Parameter
 @param [in]          SfrCfg      the config data for the sfr test
 @return              SFRTEST_STATE_ENUM SfrStmCurrentState returns the current
                      state of the sfr test
 */
/******************************************************************************/
STATIC SFRTEST_STATE_ENUM sfrTestStm_onExecute ( CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   SFRTEST_STATE_ENUM SfrStmCurrentState;
   UINT16 u16RegCnt;

   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960,17.4) */
   /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
       * other than array indexing used
       * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */

   SfrStmCurrentState = SfrCfg->pTestData->eSfrTestState;

   /*lint -esym(960, 10.1)*/
   /*Note 960: Violates MISRA 2004 Required Rule 10.1, Implicit conversion of
    * complex integer expression. */
   RDS_CHECK_VARIABLE(SfrCfg->pTestData->u32RdsRegIndex);
   RDS_CHECK_VARIABLE(SfrCfg->pTestData->u32RdsMaskIndex);
   RDS_CHECK_VARIABLE(SfrCfg->pTestData->u32RdsBlockIndex);
   /*lint +esym(960, 10.1)*/

   /*add uRegsPerCycle register to the CRC*/
   /*calculate CRC for uRegsPerCycle register*/
   for (u16RegCnt = 0u; u16RegCnt < SfrCfg->uRegsPerCycle; u16RegCnt++)
   {
      /*check if the register index exceeds the register count*/
      if (RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex) <
         SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u32RegCount)
      {
         /*check if the register is selected*/
         if (SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u32RegSel
            & (1uL << RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)))
         {
            /*add the selected register in dependency of its size to the CRC*/
            /* RSM_IGNORE_QUALITY_BEGIN Notice #1 - Physical line length > 100 characters */
            switch (SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)].u16RegSize)
            /* RSM_IGNORE_QUALITY_END */
            {
               case SFR_REGSIZE_8BIT:
               {
                  sfrTestStm_CalcUint8Crc (SfrCfg);

                  break;
               }
               case SFR_REGSIZE_16BIT:
               {
                  sfrTestStm_CalcUint16Crc (SfrCfg);

                  break;
               }
               case SFR_REGSIZE_32BIT:
               {
                  sfrTestStm_CalcUint32Crc (SfrCfg);

                  break;
               }
               default:
               {
                  /* fatal error */
                  SfrStmCurrentState = SFRTEST_STATE_ERROR;
                  SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_DATA;
                  SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
                  SFRERRHND_ReportError(SFRTEST_ERRSTATE_DATA);
                  break;
               }
            }/*end of switch u16RegSize*/
         }/*end if register is selected*/

         /*increment the register index to select the next register*/
         RDS_INC(SfrCfg->pTestData->u32RdsRegIndex);

      } /* end if check if the register index exceeds the register count*/

      /*if the register index is equal to the register count, the CRC calculation is done*/
      /* RSM_IGNORE_QUALITY_BEGIN Notice #1 - Physical line length > 100 characters */
      if (RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)
         == SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE( SfrCfg->pTestData->u32RdsBlockIndex)].u32RegCount)
      /* RSM_IGNORE_QUALITY_END */
      {
         /*reset the register index and register mask index*/
         RDS_SET(SfrCfg->pTestData->u32RdsRegIndex, 0u);

         RDS_SET(SfrCfg->pTestData->u32RdsMaskIndex, 0u);

#if SFRTEST_ENABLE_BLOCK_CRC_LOG_FOR_DEBUG == TRUE
         sfrTestStm_au32CrcLog[RDS_GET_VALUE( SfrCfg->pTestData->u32RdsBlockIndex)] 
           = SfrCfg->pTestData->uCrc;
#endif
         /*check if crc calculation is done*/
         if (RDS_GET_VALUE(SfrCfg->pTestData->u32RdsBlockIndex)
            == (SfrCfg->pSfrMap->u32BlockCount - 1u))
         {

            RDS_SET(SfrCfg->pTestData->u32RdsBlockIndex, 0u);
            SfrStmCurrentState = SFRTEST_STATE_DONE;
         }
         else
         {/*crc calculation is not done increment blockindex*/

            RDS_INC(SfrCfg->pTestData->u32RdsBlockIndex);
         }
      }/*end if SFR Test cycle done*/

      /*is current RegIndex out of bounds*/
      else
      {
         /* RSM_IGNORE_QUALITY_BEGIN Notice #1 - Physical line length > 100 characters */
         if (RDS_GET_VALUE(SfrCfg->pTestData->u32RdsRegIndex)
            > SfrCfg->pSfrMap->pBlock [RDS_GET_VALUE( SfrCfg->pTestData->u32RdsBlockIndex)].u32RegCount)
         /* RSM_IGNORE_QUALITY_END */
         {
            SfrStmCurrentState = SFRTEST_STATE_ERROR;
            SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_DATA;
            SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
            SFRERRHND_ReportError(SFRTEST_ERRSTATE_DATA);
         }/*end if register counter has an error*/
      }

      if (  (SfrStmCurrentState == SFRTEST_STATE_ERROR)
         || (SfrStmCurrentState == SFRTEST_STATE_DONE))
      {
         /* RSM_IGNORE_QUALITY_BEGIN Notice #44   - Keyword 'break' identified outside a 'switch' 
         ** structure */
         break; /* exit for loop */
         /* RSM_IGNORE_QUALITY_END */
      }
   }/*end of for uiRegCnt*/

   /*lint +esym(960, 17.4)*//* activate Misra Rule 17.4 */

   return SfrStmCurrentState;
}/*end of function sfrTestStm_onExecute*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_onConfirm

 @Description      Copies the current CRC to the reference CRC, then inverts
 current CRC

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_onConfirm ( CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960, 11.5)*//* deactivate Misra Rule 11.5*/
   /*save calculated CRC in the reference CRC*/
   *(SFRCRC *) SfrCfg->pSfrMap->puSfrCrc = SfrCfg->pTestData->uCrc;/*lint !e929*/
   /* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away
    * const/volatile from a pointer or reference
    * --> Rule accepted because the pointer is CONST, the value of the CRC
    *     is copied from one variable to the other.
    * Note 929: cast from pointer to pointer [possibly violation MISRA 2004
    * Rule 11.4], [MISRA 2004 Rule 11.4]
    * --> Cast from pointer to pointer is wanted.
    */
   /*lint +esym(960, 11.5)*//* activate Misra Rule 11.5 */

   /*invert calculated CRC*/
   SfrCfg->pTestData->uCrc = ~SfrCfg->pTestData->uCrc;
}/*end of function sfrTestStm_onConfirm*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_onReset

 @Description         Sets current CRC to start value and resets block index,
                      register and mask index to 0

 @note                -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_onReset ( CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*set current CRC to start value*/
   SfrCfg->pTestData->uCrc = STARTCRC;

   /*set block index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsBlockIndex, 0u);

   /*set register index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsRegIndex, 0u);

   /*set mask index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsMaskIndex, 0u);
} /* end of function mg_sfrTestStm_onReset*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_onInit

 @Description         Sets current CRC to start value and init block index,
                      register and mask index to 0

 @note                -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_onInit ( CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*initialize CRC with the start value*/
   SfrCfg->pTestData->uCrc = STARTCRC;

   /*set block index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsBlockIndex, 0u);

   /*set register index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsRegIndex, 0u);

   /*set mask index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsMaskIndex, 0u);
}/*end of function sfrTestStm_onInit*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_onResetRef

 @Description      Resets the reference CRC, resets current CRC to start
                   value and init block index, register and mask index to 0.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_onResetRef ( CONST SFRTEST_CFG_STRUCT * SfrCfg)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*lint -esym(960, 11.5)*//* deactivate Misra Rule 11.5*/
   /*reset reference CRC*/
   *(SFRCRC *) SfrCfg->pSfrMap->puSfrCrc = STARTCRC;/*lint !e929*/
   /* Note 960: Violates MISRA 2004 Required Rule 11.5, attempt to cast away
    * const/volatile from a pointer or reference
    * --> Rule accepted because the pointer is CONST, the value of the CRC
    *     is copied from one variable to the other.
    * Note 929: cast from pointer to pointer [possibly violation MISRA 2004
    * Rule 11.4], [MISRA 2004 Rule 11.4]
    * --> Cast from pointer to pointer is wanted.
    */
   /*lint +esym(960, 11.5)*//* activate Misra Rule 11.5 */

   /*set current CRC to start value*/
   SfrCfg->pTestData->uCrc = STARTCRC;

   /*set block index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsBlockIndex, 0u);

   /*set register index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsRegIndex, 0u);

   /*set mask index to start index*/
   RDS_SET(SfrCfg->pTestData->u32RdsMaskIndex, 0u);
}/*end of function sfrTestStm_onResetRef*/


/******************************************************************************/
/*!
 @Name                sfrTestStm_StmStateNotInitialized

 @Description         This function handles the state _NOTINITIALIZED of the
                      SFR Test Statemachine.
                      In case of an invalid given SfrEvent the function for
                      handling the SFR Test Statemachine Errors is called.

 @note                -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateNotInitialized (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                               CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_INIT:
      {
         /*only param crc's should be initialized*/
         if (FALSE == SfrCfg->pSfrMap->bIsConstCrc)
         {
            sfrTestStm_onInit(SfrCfg);/*lint !e522*/
            /* Warning 522: Highest operation, function '...', lacks
             * side-effects [MISRA 2004 Rule 14.2]
             * --> Function has no side-effects. */

            SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_SAMPLINGREF;
         }
         else /* No check of TRUE and an illegal value because both errors ends
                 in the same function with the same parameters. */
         {
            SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
            SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
            SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         }

         break;
      }
      case SFRTEST_EV_RESET:
      {
         /*only constant crc's can be reset in this state*/
         if (TRUE == SfrCfg->pSfrMap->bIsConstCrc)
         {
            sfrTestStm_onInit(SfrCfg);/*lint !e522*/
            /* Warning 522: Highest operation, function '...', lacks
             * side-effects [MISRA 2004 Rule 14.2]
             * --> Function has no side-effects. */

            SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_SAMPLING;
         }
         else /* No check of FALSE and an illegal value because both errors ends
                 in the same function with the same parameters. */
         {
            SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
            SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
            SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         }
         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_CONFIRM:   /* fall through to error handling */
      case SFRTEST_EV_RESETREF:  /* fall through to error handling */
      case SFRTEST_EV_EXECUTE:   /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
/* RSM_IGNORE_QUALITY_BEGIN Notice #2 - Function name length > 32 characters */
}/*end of function sfrTestStm_StmStateNotInitialized*/
/* RSM_IGNORE_QUALITY_END */


/******************************************************************************/
/*!
 @Name             sfrTestStm_StmStateSamplingRef

 @Description      This function handles the state _SAMPLINGREF of the
                   SFR Test Statemachine.
                   In case of an invalid given SfrEvent the function for
                   handling the SFR Test Statemachine Errors is called.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateSamplingRef (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                            CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_EXECUTE:
      {
         /*check if sampling is finished and set new state*/
         switch (sfrTestStm_onExecute(SfrCfg))
         {
            case SFRTEST_STATE_DONE:
            {
               SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_REFDONE;
               break;
            }
            case SFRTEST_STATE_SAMPLINGREF:
            {
               /*state didn't change -> no need to do anything*/
               break;
            }
            case SFRTEST_STATE_NOTINITIALIZED:  /* fall through to error handling */
            case SFRTEST_STATE_SAMPLING:        /* fall through to error handling */
            case SFRTEST_STATE_REFDONE:         /* fall through to error handling */
            case SFRTEST_STATE_REFCONFIRMED:    /* fall through to error handling */
            case SFRTEST_STATE_ERROR:           /* fall through to error handling */
            default:
            {
               SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
               SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
               SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
               break;
            }
         }
         break;
      }
      case SFRTEST_EV_RESETREF:
      {
         sfrTestStm_onResetRef(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_NOTINITIALIZED;

         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_RESET:     /* fall through to error handling */
      case SFRTEST_EV_CONFIRM:   /* fall through to error handling */
      case SFRTEST_EV_INIT:      /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
}/*end of function sfrTestStm_StmStateSamplingRef*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_StmStateRefDone

 @Description      This function handles the state _REFDONE of the
                   SFR Test Statemachine.
                   In case of an invalid given SfrEvent the function for
                   handling the SFR Test Statemachine Errors is called.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateRefDone (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                        CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_CONFIRM:
      {
         sfrTestStm_onConfirm(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_REFCONFIRMED;

         break;
      }
      case SFRTEST_EV_RESETREF:
      {
         sfrTestStm_onResetRef(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_NOTINITIALIZED;
         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_INIT:      /* fall through to error handling */
      case SFRTEST_EV_EXECUTE:   /* fall through to error handling */
      case SFRTEST_EV_RESET:     /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
}/*end of function sfrTestStm_StmStateRefDone*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_StmStateRefConfirmed

 @Description      This function handles the state _REFCONFIRMED of the
                   SFR Test Statemachine.
                   In case of an invalid given SfrEvent the function for
                   handling the SFR Test Statemachine Errors is called.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateRefConfirmed (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                             CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_RESET:
      {
         sfrTestStm_onReset(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_SAMPLING;

         /* Reset CRC after calculating */
         STDEF_MCU_CRC_RESET;

         break;
      }
      case SFRTEST_EV_RESETREF:
      {
         sfrTestStm_onResetRef(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_NOTINITIALIZED;
         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_EXECUTE: /* fall through to error handling */
      case SFRTEST_EV_CONFIRM: /* fall through to error handling */
      case SFRTEST_EV_INIT:    /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
}/*end of function sfrTestStm_StmStateRefConfirmed*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_StmStateSampling

 @Description      This function handles the state _SAMPLING of the
                   SFR Test Statemachine.
                   In case of an invalid given SfrEvent the function for
                   handling the SFR Test Statemachine Errors is called.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateSampling (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                         CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_EXECUTE:
      {
         /*check if sampling is finished and set new state*/
         switch (sfrTestStm_onExecute(SfrCfg))
         {
            case SFRTEST_STATE_DONE:
            {
               /* check the calculated CRC, see [SRS_461] */
               if (SfrCfg->pTestData->uCrc == *SfrCfg->pSfrMap->puSfrCrc)
               {
                  /*CRC check OK*/
                  SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_DONE;
               }
               else
               {
#if SFRTEST_SET_SFRCRCCHECK_ALWAYS_TRUE == TRUE
#warning "SFR Test CRC check is always TRUE!!!"
#warning "Set compiler switch SFRTEST_SET_SFRCRCCHECK_ALWAYS_TRUE to FALSE to enable CRC check."

                  /*CRC check OK*/
                  SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_DONE;
#else
                  /*CRC check not OK*/
                  SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_REG;
                  SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
                  SFRERRHND_ReportError(SFRTEST_ERRSTATE_REG);
#endif
               }
               break;
            }
            case SFRTEST_STATE_SAMPLING:
            {
               /*continue sampling of crc, no change of the state machine state*/
               break;
            }
            case SFRTEST_STATE_ERROR:           /* fall through to error handling */
            case SFRTEST_STATE_NOTINITIALIZED:  /* fall through to error handling */
            case SFRTEST_STATE_SAMPLINGREF:     /* fall through to error handling */
            case SFRTEST_STATE_REFDONE:         /* fall through to error handling */
            case SFRTEST_STATE_REFCONFIRMED:    /* fall through to error handling */
            default:
            {
               SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
               SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
               SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
               break;
            }
         }/*end of switch sfrEvent*/
         break;
      }/*end of case SFRTEST_EV_EXECUTE*/
      case SFRTEST_EV_RESETREF:
      {
         sfrTestStm_onResetRef(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_NOTINITIALIZED;

         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_RESET:   /* fall through to error handling */
      case SFRTEST_EV_CONFIRM: /* fall through to error handling */
      case SFRTEST_EV_INIT:    /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
}/*end of function sfrTestStm_StmStateSampling*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_StmStateDone

 @Description      This function handles the state _DONE of the
                   SFR Test Statemachine.
                   In case of an invalid given SfrEvent the function for
                   handling the SFR Test Statemachine Errors is called.

 @note             -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC void sfrTestStm_StmStateDone (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                     CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   switch (SfrEvent)
   {
      case SFRTEST_EV_RESET:
      {
         sfrTestStm_onReset(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_SAMPLING;

         /* Reset CRC after calculating */
         STDEF_MCU_CRC_RESET;

         break;
      }
      case SFRTEST_EV_RESETREF:
      {
         sfrTestStm_onResetRef(SfrCfg);/*lint !e522*/
         /* Warning 522: Highest operation, function '...', lacks
          * side-effects [MISRA 2004 Rule 14.2]
          * --> Function has no side-effects. */

         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_NOTINITIALIZED;

         break;
      }
      /*not allowed events*/
      case SFRTEST_EV_EXECUTE: /* fall through to error handling */
      case SFRTEST_EV_CONFIRM: /* fall through to error handling */
      case SFRTEST_EV_INIT:    /* fall through to error handling */
      default:
      {
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end of switch sfrEvent*/
}/*end of function sfrTestStm_StmStateDone*/


/******************************************************************************/
/*!
 @Name             sfrTestStm_Statemachine

 @Description      This function contains the state machine for the SFR Test.

 @note             Every given event causes an other reaction. If there is an
                   invalid event, the error handling function is called.

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           void
 */
/******************************************************************************/
STATIC SFRTEST_STATE_ENUM sfrTestStm_Statemachine (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                                   CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   /* The check of the pointer SfrCfg is done in the interface functions of the
    * module only. The check is not done in this private functions again,
    * because the pointer may get corrupted by stack failure only.
    */

   /*do event depending on the current state of the sfr test*/
   switch (SfrCfg->pTestData->eSfrTestState)
   {
      case SFRTEST_STATE_NOTINITIALIZED:
      {
         sfrTestStm_StmStateNotInitialized(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_NOTINITIALIZED*/
      case SFRTEST_STATE_SAMPLINGREF:
      {
         sfrTestStm_StmStateSamplingRef(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_SAMPLINGREF*/
      case SFRTEST_STATE_REFDONE:
      {
         sfrTestStm_StmStateRefDone(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_REFDONE*/
      case SFRTEST_STATE_REFCONFIRMED:
      {
         sfrTestStm_StmStateRefConfirmed(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_REFCONFIRMED*/
      case SFRTEST_STATE_SAMPLING:
      {
         sfrTestStm_StmStateSampling(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_SAMPLING*/
      case SFRTEST_STATE_DONE:
      {
         sfrTestStm_StmStateDone(SfrCfg, SfrEvent);

         break;
      }/*end of case SFRTEST_STATE_DONE*/
      case SFRTEST_STATE_ERROR:
      {
         /*do nothing because we are in an error state*/
         break;
      }/*end of case SFRTEST_STATE_ERROR*/
      default:
      {
         /*unkown State -> throw error*/
         SfrCfg->pTestData->eErrorState = SFRTEST_ERRSTATE_STM;
         SfrCfg->pTestData->eSfrTestState = SFRTEST_STATE_ERROR;
         SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
         break;
      }
   }/*end switch*/

   return SfrCfg->pTestData->eSfrTestState;
}/*end of function sfrTestStm_Statemachine*/



/***** End of: locfunc Local Functions **********************************//*@}*/

/******************************************************************************/
/* Global Functions                                                           */
/*******************************//*!@addtogroup glbfunc Global Functions*//*@{*/

/******************************************************************************/
/*!
 @Name                sfrTestStm_Trigger

 @Description         Triggers the SFR-Test state machine, and executes the
                      actions assigned to transitions. Unexpected events always
                      lead to the state SFRTEST_STATE_ERROR. Before the state
                      machine is triggered, it is checked whether the test data
                      is consistent, if not, a transition to SFRTEST_STATE_ERROR
                      is initiated. The new state is returned to the calling
                      function.

 @note                -

 @Parameter
 @param [in]       SfrCfg      the config data for the sfr test
 @param [in]       SfrEvent    the event that should be handled by the state
                               machine
 @return           eSfrTestState returns the state of the sfr test after the
                   event that occured has been handled
 */
/******************************************************************************/
SFRTEST_STATE_ENUM sfrTestStm_Trigger (CONST SFRTEST_CFG_STRUCT * SfrCfg,
                                       CONST SFRTEST_EVENT_ENUM SfrEvent)
{
   SFRTEST_STATE_ENUM eRet;

   /* Check the pointer of the config data for the sfr test. */
   if ((SfrCfg == NULL) || (SfrCfg != SfrCfg->pThis))
   {
      SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
      eRet = SFRTEST_STATE_ERROR; /*lint !e904 LINTDEV_RETURN_AFTER_HALT*/
   }

   else
   {
      eRet = sfrTestStm_Statemachine(SfrCfg, SfrEvent);
   }

   return eRet;
}/*end of function sfrTestStm_Trigger*/

/***** End of: glbfunc Global Functions *********************************//*@}*/

/**** Last line of source code                                             ****/

/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SFRCRC_typ.h 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SFRCRC_typ.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Contains all structs and enums needed for the SFR test.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef SFRCRC_TYP_H
#define SFRCRC_TYP_H

/******************************************************************************/
/* includes of config header (#include)                                      */
/******************************************************************************/

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/

/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*!This constant defines the data type of the CRC. */
#define SFRCRC UINT32

/*! If NULL is not defined, define it for the SFR-Test. */
#ifndef NULL
/*lint -e(960)*//*Definition of NULL is only if it is not defined. */
#define NULL 0
#endif

/*! These constants defines the data type for the SFRs. */
#define SFR8   UINT8    /*!< Data type for SFR with 8 Bit. */
#define SFR16  UINT16   /*!< Data type for SFR with 16 Bit. */
#define SFR32  UINT32   /*!< Data type for SFR with 32 Bit. */

/*! These constants defines the size of the Registers in bit. */
#define SFR_REGSIZE_8BIT   8u    /*!< Register size of 8 Bit. */
#define SFR_REGSIZE_16BIT  16u   /*!< Register size of 16 Bit. */
#define SFR_REGSIZE_32BIT  32u   /*!< Register size of 32 Bit. */

/***** End of: define Constants *****************************************//*@}*/

/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/



/***** End of: macros Macros ********************************************//*@}*/

/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/

/*!Defines the data type of the SAFEINDEX. */
typedef UINT32 SAFEINDEX;

/*! This enumeration defines the events that trigger the transitions of the
 * special function register test state machine.
 *  With a hamming distance >=4 */
typedef enum
{
   SFRTEST_EV_EXECUTE   = 0x000F,
   SFRTEST_EV_RESET     = 0x00F0,
   SFRTEST_EV_CONFIRM   = 0x0F00,
   SFRTEST_EV_INIT      = 0xF000,
   SFRTEST_EV_RESETREF  = 0xFFF0
} SFRTEST_EVENT_ENUM;

/*! This enumeration defines the states of a SFR-Test.
 *  Note: The states SAMPLINGREF, REFDONE and REFCONFIRMED are applicable only
 *  if the bIsConstCrc flag in the SFR_REGISTERMAP_STRUCT is set to false.
 */
typedef enum
{
   SFRTEST_STATE_NOTINITIALIZED  = 0x00FF,
   SFRTEST_STATE_SAMPLINGREF     = 0x0F0F,
   SFRTEST_STATE_REFDONE         = 0xF00F,
   SFRTEST_STATE_REFCONFIRMED    = 0x0FF0,
   SFRTEST_STATE_SAMPLING        = 0xF0F0,
   SFRTEST_STATE_DONE            = 0xFF00,
   SFRTEST_STATE_ERROR           = 0xF0FF
} SFRTEST_STATE_ENUM;

/*! The error state should be mapped to global error states that can be passed
 * to the global error handler.
 */
typedef enum
{
   SFRTEST_ERRSTATE_NONE   = 0xAF0F,
   SFRTEST_ERRSTATE_REG    = 0xFFAF,
   SFRTEST_ERRSTATE_DATA   = 0x0FAA,
   SFRTEST_ERRSTATE_STM    = 0xA0A0
} SFRTEST_ERROR_STATE;

/*! Structure for the definition of a block of register holding the information
 *  which registers shall be included in the test
 */
/*lint -esym(768, SFRTEST_BLOCKDESCRIPTOR_STRUCT_tag::u32MaskCount) LINTDEV_INDIVIDUAL is used in 
**config file 
**Info 768: global struct member 'SFRTEST_BLOCKDESCRIPTOR_STRUCT_tag::ulMaskCount' not referenced*/
/*lint -esym(768, pu16Reg) LINTDEV_INDIVIDUAL union member should be used later 
**Info 768: global struct member 'pui16Reg' not referenced*/
/*lint -esym(768, pu32Reg) LINTDEV_INDIVIDUAL union member should be used later 
**Info 768: global struct member 'pui16Reg' not referenced*/
/*lint -esym(768, pu16Mask) LINTDEV_INDIVIDUAL union member should be used later 
**Info 768: global struct member 'pui16Mask' not referenced*/
/*lint -esym(768, pu32Mask) LINTDEV_INDIVIDUAL union member should be used later 
**Info 768: global struct member 'pui32Mask' not referenced*/
typedef struct SFRTEST_BLOCKDESCRIPTOR_STRUCT_tag
{
   /*!< Width of the register in the block in number of bits. */
   UINT16 u16RegSize; 
   /*!< Count of registers in the block(max. 32!). */
   UINT32 u32RegCount; 
   /*!< Count of registers that need to be masked for testing. */
   UINT32 u32MaskCount; 
   /*!< Contains the selection bits for each register in the block if the register shall be included
   ** in the test. */
   UINT32 u32RegSel; 
   /*!< Indicates whether the register needs to be masked for testing. */
   UINT32 u32MaskSel; 
   union /*!< This union defines the type of the Register, 8 / 16 / 32 Bit. */
   {
      CONST SFR8 * pu8Reg;
      CONST SFR16 * pu16Reg;
      CONST SFR32 * pu32Reg;
   /* RSM_IGNORE_QUALITY_BEGIN Notice #35 - Class specification contains public data */
   } Reg; /*lint !e960  Declaration of union type or object of union type */
   /* RSM_IGNORE_QUALITY_END */
   union /*!< This union defines the type of the Register-Mask, 8 / 16 / 32 Bit. */
   {
      CONST SFR8 * pu8Mask;
      CONST SFR16 * pu16Mask;
      CONST SFR32 * pu32Mask;
   /* RSM_IGNORE_QUALITY_BEGIN Notice #35 - Class specification contains public data */
   } Mask; /*lint !e960  Declaration of union type or object of union type */
   /* RSM_IGNORE_QUALITY_END */
} SFRTEST_BLOCKDESCRIPTOR_STRUCT;

/*! Structure for the definition of the registers that shall be included in the
 * test.
 */
typedef struct SFRTEST_REGISTERMAP_STRUCT_tag
{
   /*!< Number of register block definitions. */
   UINT32 u32BlockCount; 
   /*!< Pointer to an array of register block definitions */
   SFRTEST_BLOCKDESCRIPTOR_STRUCT * pBlock; 
   /*!< flag to indicate wether the reference crc is a constant crc or a crc calculated at 
   ** runtime. */
   BOOL bIsConstCrc; 
   /*!< Pointer to the reference crc.*/
   SFRCRC const * puSfrCrc; 
} SFRTEST_REGISTERMAP_STRUCT;

/*! This struct contains all the data to the current SFR-Test state*/
/*lint -esym(768, SFRTEST_DATA_STRUCT_tag::pThis) LINTDEV_INDIVIDUAL union member should be 
**used later Info 768: global struct member 'SFRTEST_DATA_STRUCT_tag::pThis' not referenced*/
typedef struct SFRTEST_DATA_STRUCT_tag
{
   RDS_UINT32 u32RdsBlockIndex;
   RDS_UINT32 u32RdsRegIndex;
   RDS_UINT32 u32RdsMaskIndex;
   SFRCRC uCrc;
   SFRTEST_STATE_ENUM eSfrTestState;
   SFRTEST_ERROR_STATE eErrorState;

   const struct SFRTEST_DATA_STRUCT_tag * pThis; /*!< This pointer for safety reasons */
} SFRTEST_DATA_STRUCT;

/*! Root structure for the definition of a special function register test
 * instance.
 */
typedef struct SFRTEST_CFG_STRUCT_tag
{
   /*!< Pointer to the register map object that shall be used for the test. */
   const SFRTEST_REGISTERMAP_STRUCT * pSfrMap; 
   /*!< Pointer to the test data object. */
   SFRTEST_DATA_STRUCT * pTestData; 
   /*!< Number of registers that shall be processed during one test step. */
   UINT16 uRegsPerCycle; 
   /*!< This-pointer for safety reasons. */
   void *pThis; 
} SFRTEST_CFG_STRUCT;

/***** End of: typedef Types ********************************************//*@}*/

/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

/***** End of: globvar Global Variables *********************************//*@}*/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/

#endif /* SFRCRC_TYP_H */

/**** Last line of source code                                             ****/

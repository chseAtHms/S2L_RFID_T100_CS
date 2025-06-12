/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: xtypes.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          xtypes.h
 * @origAuthor:   mdischinger / embeX GmbH
 *
 * @brief:        Compiler switch for embeX-Types.
 * @description:
 * This header file defines all standard variables of specified (Byte-) size.
 * This definitions are dependent on the used architecture and will be defined
 * in seperate sections, selectable by Compiler Switches, for each possible
 * architecture. The adequate Compiler Switch must be set in each project
 * (e.g. Compiler Option -dCOMPILER_...).
 *
 * The following switches are already defined:
 * - COMPILER_KEIL_ARM        for Keil-Compiler for ARM
 *
 */
/************************************************************************//*@}*/

#ifndef  XTYPES_H
#define  XTYPES_H


/******************************************************************************/
/* includes of specific header (#include)                                     */
/******************************************************************************/
#include "stdint.h" /* for the datatypes uint8_t, uint16_t, ... */


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/* Definition of TRUE and FALSE */
#define FALSE  0u
#define TRUE   1u

/* Definition of minimum and maximum values for the specified datatypes. */
#define  INT8_MIN    (-128)            /*!< Minimum 8 Bit signed value.    */
#define  INT8_MAX    127               /*!< Maximum 8 Bit signed value.    */
#define  UINT8_MAX   255               /*!< Maximum 8 Bit unsigned value.  */

#define  INT16_MIN   (-32768)          /*!< Minimum 16 Bit signed value.   */
#define  INT16_MAX   32767             /*!< Maximum 16 Bit signed value.   */
#define  UINT16_MAX  65535             /*!< Maximum 16 Bit unsigned value. */

#define  INT32_MIN   (-2147483648u)    /*!< Minimum 32 Bit signed value.   */
#define  INT32_MAX   2147483647        /*!< Maximum 32 Bit signed value.   */
#define  UINT32_MAX  4294967295u       /*!< Maximum 32 Bit unsigned value. */

#define  BYTE_MAX    255               /*!< Maximum Byte value.            */
#define  WORD_MAX    65535             /*!< Maximum Word value.            */
#define  DWORD_MAX   4294967295u       /*!< Maximum DWort value.           */

/* Definition of specific data values. */
#define  ZERO        0u                /*!< Zero value. */

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/******************************************//*!@addtogroup macros Macros*//*@{*/

/*! This macro checks a variable if it has a boolean value. If it is not
 *  TRUE or FALSE the Safety-Handler is called.
 *
 *  @param[in] x     Boolean variable
 *  */
#define BOOL_SAFETY_ASSERT(x) \
   GLOBFAIL_SAFETY_ASSERT((TRUE == x) || (FALSE == x),  GLOB_FAILCODE_VARIABLE_ERR)

/*! This macro checks a variable if it has a boolean value. If it is not
 *  TRUE or FALSE the Safety-Handler is called.
 *  Furthermore this macro has a return value which is used for the unit tests.
 *
 *  @param[in] x     Boolean variable
 *  @return          Return value
 *  */
#define BOOL_SAFETY_ASSERT_RET(x, ret) \
   GLOBFAIL_SAFETY_ASSERT_RET((TRUE == x) || (FALSE == x),  GLOB_FAILCODE_VARIABLE_ERR, ret)

/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/*! Std-Types for Keil-Compiler (ARM) */
#define COMPILER_KEIL_ARM
#ifdef COMPILER_KEIL_ARM

   /* Types */
   typedef  float          REAL;    /*!<@brief floating-point  */
   typedef  double         DREAL;   /*!<@brief floating-point  */

   typedef  uint8_t        UINT8;   /*!<@brief unsigned 8-Bit-Integer   */
   typedef  uint16_t       UINT16;  /*!<@brief unsigned 16-Bit-Integer  */
   typedef  uint32_t       UINT32;  /*!<@brief unsigned 32-Bit-Integer  */
   typedef  int8_t         INT8;    /*!<@brief signed 8-Bit-Integer     */
   typedef  int16_t        INT16;   /*!<@brief signed 16-Bit-Integer    */
   typedef  int32_t        INT32;   /*!<@brief signed 32-Bit-Integer    */

   typedef  uint8_t        BYTE;    /*!<@brief Byte         */
   typedef  uint16_t       WORD;    /*!<@brief Word         */
   typedef  uint32_t       DWORD;   /*!<@brief Double Word  */

   typedef  uint8_t        BOOL;    /*!<@brief logical Variable (TRUE, FALSE)*/

#endif   /* COMPILER_KEIL_ARM */


/*------------------------------------------------------------------------------
** This enum contains SAFE TRUE/FALSE with hamming distance
**------------------------------------------------------------------------------
*/
typedef enum
{
   eTRUE   = 0x2839u,   /*!< Identifier for safe true */
   eFALSE  = 0x29aeu    /*!< Identifier for safe false */
} TRUE_FALSE_ENUM;


/***** End of: typedef Types ********************************************//*@}*/

//*****************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

/***** End of: globvar Global Variables *********************************//*@}*/


#endif   /* XTYPES_H */

/**** Last line of source code                                              ****/

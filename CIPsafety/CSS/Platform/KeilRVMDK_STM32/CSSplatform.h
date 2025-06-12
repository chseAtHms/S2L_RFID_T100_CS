/*******************************************************************************
**    Copyright (C) 2009-2020 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSSplatform.h
**    Summary: CSS platform specific settings
**             CSSplatform.h contains definitions that are dependent on the
**             hardware platform. This must be ported to the used compiler/micro
**             controller.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: -
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef CSS_PLATFORM_H
#define CSS_PLATFORM_H


/*******************************************************************************
**    platform include-files
*******************************************************************************/

#ifndef XTYPES_H
  #include "xtypes.h"
#endif

#include "stdlib-hal.h"    /* include string.h from standard C library, see below */

#ifndef GLOBDEF_GLOBALDEFINITIONS_H
  #include "globDef_GlobalDefinitions.h"
#endif

/*! If the compiler switch __UNIT_TESTING_ON__ is defined, the following includes
 *  are used for unit tests. 
 */
#ifdef __UNIT_TESTING_ON__
  /* (This include is only used in unit tests!) */
  #include <stdio.h>
#endif

/*******************************************************************************
**    constants
*******************************************************************************/

/** CSS_k_PLATFORM_STRING:
    This symbol identifies the Target System (Hardware, Development Tool-chain,
    etc.)(see Req.9.1-1 and Req.9.1-2 and Req.9.2-1)
*/
#define CSS_k_PLATFORM_STRING          "IXXAT Safe T100 CS"

/** CSS_k_BIG_ENDIAN:
    This define is used to set {CSS_k_ENDIAN} constant, if the low byte is on
    the high address and the high byte is on the low address.
*/
#define CSS_k_BIG_ENDIAN               1

/** CSS_k_LITTLE_ENDIAN:
    This define is used to set {CSS_k_ENDIAN} constant, if the low byte is on
    the low address and the high byte is on the high address.
*/
#define CSS_k_LITTLE_ENDIAN            2


/*******************************************************************************
**    platform settings
*******************************************************************************/

/** CSS_k_ENDIAN:
    This define specifies, whether the platform is big endian
    (see {CSS_k_BIG_ENDIAN}) or little endian (see {CSS_k_LITTLE_ENDIAN}).
    allowed values : CSS_k_BIG_ENDIAN, CSS_k_LITTLE_ENDIAN
*/
#define CSS_k_ENDIAN                   CSS_k_LITTLE_ENDIAN

/** CSS_k_PLATFORM_FACTOR:
    This define specifies the smallest data type.
       1 - smallest data type is 8 bit (one byte)
       2 - smallest data type is 16 bit (two bytes)
*/
#define CSS_k_PLATFORM_FACTOR          1

/** CSS_ADD_OFFSET:
    This macro is used to increment the address of the given pointer with
    the given offset.
*/
#define CSS_ADD_OFFSET(ptr, offset)    (((CSS_t_BYTE *)(ptr)) + (offset))


/*******************************************************************************
**    library functions
*******************************************************************************/

/* place character c into first len characters of p_dst, return p_dst */
#define CSS_MEMSET(p_dst, c, len)      \
    stdlibHAL_ByteArrSet((UINT8*)(p_dst), (UINT8)(c), (UINT16)(len))

/* copy len characters from src to dst, and return p_dst */
#define CSS_MEMCPY(p_dst, p_src, len)  \
    stdlibHAL_ByteArrCopy((UINT8*)(p_dst), (const UINT8*)(p_src), (UINT16)(len))

/* compare len characters from p_1 with p_2, return zero if equal */
#define CSS_MEMCMP(p_1, p_2, len)      \
    stdlibHAL_ByteArrComp((const UINT8*)(p_1), (const UINT8*)(p_2), (UINT16)(len))


/*! If the compiler switch __UNIT_TESTING_ON__ is defined, the following macros
 *  are used for unit tests. 
 */
#ifdef __UNIT_TESTING_ON__

  /* determine the length of a string (number of bytes)
     (This macro is only used in unit tests!) */
  #define CSS_STRLEN(p_str)              \
      strlen(p_str)

  /* CSS_SNPRINTF is needed for the Get Error String mechanism and unit tests   */
  /* We need to define a char data type for typecasting the pointer passed to   */
  /* snprintf.                                                                  */
  /* (This macro is only used in unit tests!) */
  typedef char CSS_t_SNPRINTF_CHAR;
  
  /* Definition for the buffer size type */
  /* (This macro is only used in unit tests!) */
  #define CSS_t_SNPRINTF_BUF_SIZE        size_t
  
  /** CSS_SNPRINTF:
      This macro realizes the standard I/O function snprintf with different
      number of parameters and checks whether the error string fits into the
      given buffer. If the macro returns CSS_k_TRUE then the error string was
      copied into the buffer otherwise return CSS_k_FALSE.
  */
  /* (This macro is only used in unit tests!) */
  #define CSS_SNPRINTF(param)            (INT32)(snprintf param)

  /** CSS_FPRINTFx:
      These macros realize the standard I/O function fprintf with different number
      of parameters and different I/O functions.
      Use these macros for UART printing for unittests.
  */
  /* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
  #define CSS_FPRINTF0(file, format)                  fprintf((file), (format))
  #define CSS_FPRINTF1(file, format, a1)              fprintf((file), (format), (a1))
  #define CSS_FPRINTF2(file, format, a1, a2)          fprintf((file), (format), (a1), (a2))
  #define CSS_FPRINTF4(file, format, a1, a2, a3, a4)  fprintf((file), (format), (a1), (a2), (a3), (a4))
  /* RSM_IGNORE_QUALITY_END */

#endif /* __UNIT_TESTING_ON__ */

#endif /* #ifndef CSS_PLATFORM_H */

/*** End of File ***/

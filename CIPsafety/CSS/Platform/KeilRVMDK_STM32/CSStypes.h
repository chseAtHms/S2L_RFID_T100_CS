/*******************************************************************************
**    Copyright (C) 2009-2022 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSStypes.h
**    Summary: CSS data type definitions
**             Definition of the basic data types that are used inside the CIP
**             Safety Stack.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: -
**
********************************************************************************
**    Template Version 3
*******************************************************************************/


#ifndef CSS_TYPES_H
#define CSS_TYPES_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** CSS_k_TRUE:
    Boolean value representing TRUE
*/
#ifndef CSS_k_TRUE
  #define CSS_k_TRUE    ((CSS_t_BOOL)(1==1))
#endif

/** CSS_k_FALSE:
    Boolean value representing FALSE
*/
#ifndef CSS_k_FALSE
  #define CSS_k_FALSE   ((CSS_t_BOOL)(1==0))
#endif

/** CSS_k_NULL:
    Value for tagging pointers invalid
*/
#ifndef CSS_k_NULL
  #define CSS_k_NULL (void *)0U
#endif


/*******************************************************************************
**    data types
*******************************************************************************/

/**
    CIP Data Types:

    The following section contains the definition of standard data types
    according to the CIP specifications (Vol.1, Appendix C-2).
*/


/** CSS_t_BOOL:
    Boolean: possible values 0 and 1, corresponding to CSS_k_FALSE and
    CSS_k_TRUE.
    Prefix for variables of this type is: o
*/
typedef unsigned char       CSS_t_BOOL;


/** CSS_t_SINT:
    Short Integer, Range -128 .. 127.
    Prefix for variables of this type is: i8
*/
typedef signed char         CSS_t_SINT;


/** CSS_t_INT:
    Integer, Range -32768 .. 32767.
    Prefix for variables of this type is: i16
*/
typedef signed short int    CSS_t_INT;


/** CSS_t_DINT:
    Double Integer, Range -2^31 .. (2^31)-1.
    Prefix for variables of this type is: i32
*/
typedef signed long int     CSS_t_DINT;


/** CSS_t_USINT:
    Unsigned Short Integer, Range 0 .. 255.
    Prefix for variables of this type is: u8
*/
typedef unsigned char       CSS_t_USINT;


/** CSS_t_UINT:
    Unsigned Integer, Range 0 .. 65535.
    Prefix for variables of this type is: u16
*/
typedef unsigned short int  CSS_t_UINT;


/** CSS_t_UDINT:
    Unsigned Integer, Range 0 .. (2^32)-1.
    Prefix for variables of this type is: u32
*/
typedef unsigned long int   CSS_t_UDINT;


/** CSS_t_BYTE:
    bit string - 8 bits.
    Prefix for variables of this type is: b
*/
typedef unsigned char       CSS_t_BYTE;


/** CSS_t_WORD:
    bit string - 16 bits.
    Prefix for variables of this type is: w
*/
typedef unsigned short int  CSS_t_WORD;


/** CSS_t_DWORD:
    bit string - 32 bits.
    Prefix for variables of this type is: dw
*/
typedef unsigned long int   CSS_t_DWORD;


/*
   Other Data Types used throughout the whole stack
    ------------------------------------------------

*/

/** CSS_t_CHAR:
    This type defines the character data type. This type is usually
    needed for variables of type STRING.
    Prefix for variables of this type is: c
*/
typedef signed char         CSS_t_CHAR;


/** CSS_t_VOID:
    This type defines the "void" data type. It is just used for variables
    (e.g. pointer to void). Function return values are just encoded with
    a plain "void".
    Prefix for variables of this type is: v
*/
typedef void                CSS_t_VOID;


#endif /* #ifndef CSS_TYPES_H */

/*** End of File ***/



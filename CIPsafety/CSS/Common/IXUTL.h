/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: IXUTL.h
**    Summary: IXUTL - Utility Routines
**             IXUTL.h is the export header file of the IXUTL unit.
**
**     Author: A.Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: IXUTL_Init
**             IXUTL_GetSysTime
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef IXUTL_H
#define IXUTL_H


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/** IXUTL_IsTimeGreaterOrEqual:
    Macro for comparing 2 times. If t1 is greater or equal than t2 then
    CSS_k_TRUE is returned. Can be used for timeout checking etc.
    (result of the macro is not casted to CSS_t_BOOL as this leads to lint
    warning: "Prohibited cast of complex integer expression: Signed versus
    Unsigned")
*/
#define IXUTL_IsTimeGreaterOrEqual(u32_t1, u32_t2)  \
                           ((CSS_t_UDINT)((u32_t1) - (u32_t2)) < 0x80000000UL)


/** IXUTL_NUM_OF_ARRAY_ELEMENTS:
    This macro returns the number of elements of an array.
*/
#define IXUTL_NUM_OF_ARRAY_ELEMENTS(x)   (sizeof(x) / sizeof((x)[0]))


/** IXUTL_BIT_CLR_U8:
    This macro will clear (set to zero) those bits which are 1 in the provided
    mask. This macro can be used on CSS_t_USINT and CSS_t_BYTE variables.
*/
#define IXUTL_BIT_CLR_U8(u8_val, u8_mask)  ((u8_val) &=                        \
                                           (CSS_t_BYTE)~(CSS_t_BYTE)(u8_mask))

/** IXUTL_BIT_SET_U8:
    This macro will set (set to one) those bits which are 1 in the provided
    mask. This macro can be used on CSS_t_USINT and CSS_t_BYTE variables.
*/
#define IXUTL_BIT_SET_U8(u8_val, u8_mask)  ((u8_val) |= (u8_mask))

/** IXUTL_BIT_GET_U8:
    This macro will return the value of those bits which are 1 in the provided
    mask. This macro can be used on CSS_t_USINT and CSS_t_BYTE variables.
*/
#define IXUTL_BIT_GET_U8(u8_val, u8_mask)  ((u8_val) & (u8_mask))


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

/*******************************************************************************
**
** Function    : IXUTL_Init
**
** Description : This function initializes the IXUTL unit.
**
** See Also    : -
**
** Parameters  : -
**
** Returnvalue : -
**
*******************************************************************************/
void IXUTL_Init(void);


/*******************************************************************************
**
** Function    : IXUTL_GetSysTime
**
** Description : This function returns the current system time. System time is a
**               32-bit counter incremented in 128us steps.
**
** Parameters  : -
**
** Returnvalue : CSS_t_UDINT - current system time
**
*******************************************************************************/
CSS_t_UDINT IXUTL_GetSysTime(void);


#endif /* #ifndef IXUTL_H */


/*** End Of File ***/


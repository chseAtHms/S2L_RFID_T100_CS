/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: rds.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          rds.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Redundant Data Security (RDS)
 * @description:  This module supports the Redundant Data Security. It provides
 *                safety data types and macros which are handles these.
 *                The safety data types are specific data types of the standard
 *                data types.
 *                See [SRS_48].
 */
/************************************************************************//*@}*/
#ifndef RDS_H
#define RDS_H


/******************************************************************************/
/* includes of config header (#include)                                       */
/******************************************************************************/


/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/


/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/


/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/

/*lint -save */
/*lint -esym(961,19.7) */
/* Note 961: Violates MISRA 2004 Advisory Rule 19.7, Function-like macro defined
 * Lint message deactivated because of the RDS macros. Function-like macros
 * are OK. Lint message for the RDS module deactivated. */

/*! These constants are used for the checking of the safety variables. */
#define RDS_UINT8_VARIABLES_EQUAL   0x000000FFuL
#define RDS_UINT16_VARIABLES_EQUAL  0x0000FFFFuL
#define RDS_UINT32_VARIABLES_EQUAL  0xFFFFFFFFuL


#if defined(__UNIT_TESTING_ON__) || defined(__CLASSIFIED_COMPLEXITY__)

/*! If the compiler switch __UNIT_TESTING_ON__ is defined, the following macros
 *  are used for unit test instrumentation. The RDS functionality is
 *  turned off because the RDS functionality must not be checked during the
 *  unit tests.
 *
 *  If the compiler switch __CLASSIFIED_COMPLEXITY__ is defined, then the
 *  macros are used for calculating the complexity without the RDS logic.
 *
 *  A detailed description of the specific RDS macro can be found in the else
 *  branch.
*/

#define RDS_TYPEDEF(name, typeVal, typeRaw) typedef typeVal name;
#define RDS_SET(self, value)     {(self) = (value);}
#define RDS_CHECK(self)          (1u)
#define RDS_CHECK_VARIABLE(self) (void)(0u)
#define RDS_GET_VALUE(self)      (self)
#define RDS_GET(self)            (self)
#define RDS_INC(self)            {(self)++;}
#define RDS_DEC(self)            {(self)--;}
#define RDS_ADD(self, value)     {(self) += (value);}
#define RDS_SUB(self, value)     {(self) -= (value);}
#define RDS_CLRBIT(self, value)  {(self) &= ~(value);}
#define RDS_SETBIT(self, value)  {(self) |= (value);}

#elif defined(__CTC_TESTING_ON__)

#define RDS_TYPEDEF(name, typeVal, typeRaw) \
   typedef struct \
   {  \
      union { typeVal val; typeRaw raw; } val; \
      union { typeVal val; typeRaw raw; } inv; \
   }  \
   name;

#define RDS_SET(self, value)     {(self.val.val) = (value);}
#define RDS_CHECK(self)          (1u)
#define RDS_CHECK_VARIABLE(self) (void)(0u)
#define RDS_GET_VALUE(self)      (self.val.val)
#define RDS_GET(self)            (self.val.val)
#define RDS_INC(self)            {(self.val.val)++;}
#define RDS_DEC(self)            {(self.val.val)--;}
#define RDS_ADD(self, value)     {(self.val.val) += (value);}
#define RDS_SUB(self, value)     {(self.val.val) -= (value);}
#define RDS_CLRBIT(self, value)  {(self.val.val) &= ~(value);}
#define RDS_SETBIT(self, value)  {(self.val.val) |= (value);}

#else /* __UNIT_TESTING_ON__ */


/*! This macro defines the RDS data structure.
 @param[in] name     Type name of the new data structure
 @param[in] typeVal  Basic data type, e.g. INT8
 @param[in] typeRaw  Raw value of the basic data type, the raw value defines the
                     unsigned int value in the memory. For example the typeval
                     is INT8 and the value is -2, then the typeRaw is UINT8 and
                     the value is 0xFE.
 */
#define RDS_TYPEDEF(name, typeVal, typeRaw) \
   typedef struct \
   {  \
      union { typeVal val; typeRaw raw; } val; \
      union { typeVal val; typeRaw raw; } inv; \
   }  \
   name;


/*! This macro sets the value of the RDS-Variable.
 @param[in] self     RDS structure
 @param[in] value    Value for the RDS-Variable
 */
#define RDS_SET(self, value) \
   { \
      (self).val.val = (value); \
      (self).inv.val = (value); \
      (self).inv.raw = ~(self).inv.raw; \
   }


/*! Check the data consistency of the RDS-Variable.

 @param[in] self  RDS structure

 @note The size of the self.val.raw value is checked to get the correct
       comparison value. The reason is that the C compiler always casts the char
       and short bit operations per "integer promotion" to int.
       With the statement ((sizeof(self.val.raw).......) the correct comparison
       value is determined:
       U8:  0x000000FFu (RDS_UINT8_VARIABLES_EQUAL)
       U16: 0x0000FFFFu (RDS_UINT16_VARIABLES_EQUAL)
       U32: 0xFFFFFFFFu (RDS_UINT32_VARIABLES_EQUAL)
       The macro checks the data consistency of the RDS-Variable. If the
       comparison is TRUE the value is valid, if it is FALSE the value is
       invalid.
       It works for all RDS data types.
 */
#define RDS_CHECK(self) (((self).val.raw ^ (self).inv.raw) ==  \
    ((sizeof((self).val.raw)==1u) ? (RDS_UINT8_VARIABLES_EQUAL) : \
    ((sizeof((self).val.raw)==2u) ? (RDS_UINT16_VARIABLES_EQUAL) : (RDS_UINT32_VARIABLES_EQUAL))))


/*! This macro checks the value of the RDS.
 @param[in] self  RDS structure
 @return    If the data is valid, a 0 is returned. If the data is invalid, the
            SAFETY_ASSERT will be called.
 */
#define RDS_CHECK_VARIABLE(self) \
   (RDS_CHECK(self) ? 0u : GLOBFAIL_SAFETY_FAIL_RDS(GLOB_FAILCODE_VARIABLE_ERR))


/*! This macro returns the value of the RDS-Variable without checking the value.
 @param[in] self  RDS structure
 */
#define RDS_GET_VALUE(self) \
   ((self).val.val)


/*! This macro returns the value of the RDS-Variable with a check of the value.
 @param[in] self  RDS structure
 @note: If the data is invalid, the SAFETY_ASSERT will be called.
 */
#define RDS_GET(self) \
   ((RDS_CHECK(self) ? 0u : GLOBFAIL_SAFETY_FAIL_RDS(GLOB_FAILCODE_VARIABLE_ERR)), (self).val.val)


/*! This macro increments the value of the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It works for pointers !
 * It doesn't work for the float data types

 @param[in] self  RDS structure
 */
#define RDS_INC(self) \
   { \
      (self).val.val++; \
      (self).inv.val--; \
   }


/*! This macro decrements the value of the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It works for pointers !
 * It doesn't work for the float data types

 @param[in] self  RDS structure
 */
#define RDS_DEC(self) \
   { \
      (self).val.val--; \
      (self).inv.val++; \
   }


/*! This macro adds a value to the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It works for pointers !
 * It doesn't work for the float data types

 @param[in] self     RDS structure
 @param[in] value    Value to add
 */
#define RDS_ADD(self, value) \
   { \
      (self).val.val += (value); \
      (self).inv.val -= (value); \
   }


/*! This macro subtracts a value of the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It works for pointers !
 * It doesn't work for the float data types

 @param[in] self     RDS structure
 @param[in] value    Value to subtract
 */
#define RDS_SUB(self, value) \
   { \
      (self).val.val -= (value); \
      (self).inv.val += (value); \
   }


/*! This macro clears one or more bits in the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It doesn't work for the float data types and pointers

 @param[in] self     RDS structure
 @param[in] value    Bit mask to clearing bits
 */
#define RDS_CLRBIT(self, value) \
   { \
      (self).val.val &= ~(value); \
      (self).inv.val |= (value); \
   }


/*! This macro sets one or more bits in the RDS-Variable.
 @note
 * It works for all basic data types UINT8, UINT16, UINT32, INT8, INT16, INT32
 * It doesn't work for the float data types and pointers

 @param[in] self     RDS structure
 @param[in] value    Bit mask to set bits
 */
#define RDS_SETBIT(self, value) \
   { \
      (self).val.val |= (value); \
      (self).inv.val &= ~(value); \
   }

#endif /*__UNIT_TESTING_ON__ */

/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/*! RDS types of the standard data types. */
RDS_TYPEDEF(RDS_BOOL,   BOOL,   UINT8);
RDS_TYPEDEF(RDS_UINT8,  UINT8,  UINT8);
RDS_TYPEDEF(RDS_UINT16, UINT16, UINT16);
RDS_TYPEDEF(RDS_UINT32, UINT32, UINT32);

RDS_TYPEDEF(RDS_INT8,  INT8,  UINT8);
RDS_TYPEDEF(RDS_INT16, INT16, UINT16);
RDS_TYPEDEF(RDS_INT32, INT32, UINT32);

RDS_TYPEDEF(RDS_FLOAT, REAL, UINT32);

/* RDS types of the standard pointer data types. */
RDS_TYPEDEF(RDS_PTR_UINT8,  UINT8*,  UINT32);
RDS_TYPEDEF(RDS_PTR_UINT16, UINT16*, UINT32);
RDS_TYPEDEF(RDS_PTR_UINT32, UINT32*, UINT32);

RDS_TYPEDEF(RDS_PTR_INT8,  INT8*,  UINT32);
RDS_TYPEDEF(RDS_PTR_INT16, INT16*, UINT32);
RDS_TYPEDEF(RDS_PTR_INT32, INT32*, UINT32);

RDS_TYPEDEF(RDS_PTR_FLOAT_T, REAL*, UINT32);

/*lint -restore*/
/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* RDS_H */

/**** Last line of source code                                             ****/

/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: xdefs.h 1129 2016-11-21 09:43:02Z klan $
 * $Revision: 1129 $
 *     $Date: 2016-11-21 10:43:02 +0100 (Mo, 21 Nov 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          xdefs.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Redefinitions for module tests.
 * @description:  Redefinitions of keywords that need to be modified in order
 *                to be able to perform proper module tests.
 */
/************************************************************************//*@}*/

#ifndef XDEFS_H
#define XDEFS_H

/******************************************************************************/
/* includes of config header (#include)                                      */
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

#ifdef __UNIT_TESTING_ON__
   #include "xtypes.h"
   #define STATIC
   #define LOCAL_STATIC(optmod, type, name, init) extern type name
   #define CONST
   #define FOREVER ((UINT8)0u)
   #define MAKE_RAM_PTR(typ,addr) ((typ*)(addr)); manipulate_pointer((typ*)addr);
#else

   /* These are embeX standard macros and the lint messages are accepted for
    * these macros.
    */
   #define STATIC static
   #define CONST const
   #define FOREVER ((UINT8)1u)

   /*! This macro is used for declaration of static variables in functions.
    *  For further details see "VA_C_Programmierstandard.pdf" TSTB-1-0-10.
    *
    *  @param[in] optmod   Optional parameter, e.g. CONST, ...
    *  @param[in] type     Data type
    *  @param[in] name     Name of the variable
    *  @param[in] init     Initialization value
    *   */
   #define LOCAL_STATIC(optmod, type, name, init) static optmod type name = init

   /*! This macro is used for making a pointer to a specific RAM address.
    *  For further details see "VA_C_Programmierstandard.pdf" TSTB-1-0-10.
    *
    *  @param[in] typ      Data type
    *  @param[in] addr     RAM address
    *   */
   #define MAKE_RAM_PTR(typ,addr) ((typ*)(addr))

#endif

/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/

#ifdef __UNIT_TESTING_ON__
   extern void manipulate_pointer(uint8_t * address);
   extern uint8_t MAKE_RAM_PTR_counter_value;
   extern uint8_t manipulate_pointer_ACTIVATE;
   extern uint8_t manipulation_time;
   extern uint8_t manipulation_value;
#endif /* __UNIT_TESTING_ON__ */

/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* XDEFS_H */

/**** Last line of source code                                             ****/

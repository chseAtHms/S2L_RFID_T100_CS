/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: cputst_CpuTest_loc.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          cputst_CpuTest_loc.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief         Local header of the CPU-Test module.
 * @description:
 * Contains the local constants and type definitions of the appendent c-file,
 * see VA_C_Programmierstandard rule TSTB-2-0-10 and STYL-2-3-20.
 */
/************************************************************************//*@}*/

#ifndef CPUTST_CPUTEST_LOC_H
#define CPUTST_CPUTEST_LOC_H

/******************************************************************************/
/* Switches (#define)                                                         */
/******************************************************************************/



/******************************************************************************/
/* Constants (#define)                                                        */
/***************************************//*!@addtogroup define Constants*//*@{*/

/*! This constant defines the maximal numbers of SFR-Test Configurations of
 *  the controller.*/
#define CPUTEST_MAXNUMOF_SFRTEST_CFGS 1u

/*! This constant defines the init-value for the SFR-Test list index counter.*/
#define CPUTEST_SFRTSTLSTIDXCNT_INIT  {{(UINT8)0u},{(UINT8)~(UINT8)0u}}

/***** End of: define Constants *****************************************//*@}*/


/******************************************************************************/
/* Macro Definitions (#define)                                                */
/*****************************************//**!@addtogroup macros Macros*//*@{*/


/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/

/*! This enum contains the states of the CPU-Test manager. */
typedef enum
{
   CPUTEST_E_OPCODE_TEST   = 0x1661u,
   CPUTEST_E_SFR_TEST      = 0x17f6u,
   CPUTEST_E_CORE_TEST     = 0x1860u
} CPUTEST_E_TASK;


/*! This enum contains the states of the CoreRegister Test. */
typedef enum
{
   CPUTEST_E_COREREGTEST_STEP1   = 0x0Fu,
   CPUTEST_E_COREREGTEST_STEP2   = 0xF0u
} CPUTEST_E_COREREGTEST_STEPS;


/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/***** End of: Function-Prototypes ********************************************/


#endif /* CPUTST_CPUTEST_LOC_H */

/**** Last line of source code                                             ****/

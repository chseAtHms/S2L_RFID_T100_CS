/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: sfrerrhnd_Interface.c 2448 2017-03-27 13:45:16Z klan $
 * $Revision: 2448 $
 *     $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          sfrerrhnd_Interface.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        The interface of the error handler.
 * @description:  -
 */
/************************************************************************//*@}*/

/******************************************************************************/
/* includes (#include)                                                        */
/******************************************************************************/
/* Project header */
#include "xtypes.h"
#include "xdefs.h"
#include "rds.h"

/* Module header */
#include "SFRCRC_typ.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* Header-file of module */
#include "sfrerrhnd_Interface.h"

/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/

/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

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

/******************************************************************************/
/*!
 @Name                SFRERRHND_ReportError

 @Description         This function is called from the SFR-Test state machine
                      in case of an error.

 @note                This function can be used to handle errors of the SFR-Test
                      state machine, e.g. to call the error handler.

 @Parameter
   @param [in]        errState      Error state.
   @return            void
 */
/******************************************************************************/
void SFRERRHND_ReportError (CONST SFRTEST_ERROR_STATE errState)
{
      /* evaluate the error state */
   switch (errState)
   {
      case SFRTEST_ERRSTATE_REG:
      {
         /* enter FS state, see [SRS_359] */
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_SFR_TEST);
         break;
      }
      case SFRTEST_ERRSTATE_DATA:
      case SFRTEST_ERRSTATE_STM:   /* fall through */
      case SFRTEST_ERRSTATE_NONE:  /* fall through */
      default:
      {
         GLOBFAIL_SAFETY_FAIL(GLOB_FAILCODE_VARIABLE_ERR);
         break;
      }
   }
}



/***** End of: glbfunc Global Functions *********************************//*@}*/

/**** Last line of source code                                             ****/

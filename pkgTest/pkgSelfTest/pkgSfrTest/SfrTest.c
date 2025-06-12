/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: SfrTest.c 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          SfrTest.c
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Contains the service interface for the special function
 *                register test.
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
#include "SfrTest_cfg.h"
#include "SfrTestStm.h"   /*sfr test state machine*/
#include "sfrerrhnd_Interface.h"  /*interface for errorhandler*/
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

/* Header-file of module */
#include "SfrTest.h"


/********** NO FURTHER INCLUDES AFTER INCLUDE OF MODULE HEADER **********/



/******************************************************************************/
/* Module global Variables or Constants (const)                               */
/*******************//*!@addtogroup moduleglobvar Moduleglobal Variables*//*@{*/

/*! the data for the sfr test */
SFRTEST_DATA_STRUCT sfrTest_asData[SFRTEST_CFG_DIM] = SFRTEST_DATA_INIT;/*lint !e708*/
/* Info 708: union initialization
 * The struct array contains of different RDS-Variables, all of these are
 * initialized with a defined value. */
/*! all sfr test configurations */
SFRTEST_CFG_STRUCT sfrTest_asCfg[SFRTEST_CFG_DIM] = SFRTEST_CFG_INIT;

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


#if SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE
/******************************************************************************/
/*!
@Name                sfrTest_InitReference

@Description         This function triggers the initialization of the
                     calculation of the reference value(s) for the special
                     function register test. It shall be called when the
                     reference value(s) are determined at run time and not
                     stored in read-only memory.
                     If the Stm-Trigger function returns an error, the SFR-Test
                     Error Handler is called.
                     If the given Safe-Index is not correct, the Safety Handler
                     is called.

@note                -

@Parameter
   @param [in]       SAFEINDEX   Safeindex of the config
   @return           void
 */
/******************************************************************************/
void sfrTest_InitReference (CONST SAFEINDEX si)
{
   SFRTEST_STATE_ENUM eResult;

   SFRTEST_SAFEINDEX_SAFETY_ASSERT(si);

   eResult =  sfrTestStm_Trigger(&sfrTest_asCfg[(UINT32)(si&0xFFFFuL)],
                                SFRTEST_EV_INIT);

   if (eResult == SFRTEST_STATE_ERROR)
   {
      SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
   }
}
#endif /* SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE */


/******************************************************************************/
/*!
@Name                sfrTest_Reset

@Description         This function triggers the reset of the special function
                     register test.
                     If the Stm-Trigger function returns an error, the SFR-Test
                     Error Handler is called.
                     If the given Safe-Index is not correct, the Safety Handler
                     is called.

@note                -

@Parameter
   @param [in]       SAFEINDEX   Safeindex of the config
   @return           void
 */
/******************************************************************************/
void sfrTest_Reset (CONST SAFEINDEX si)
{
   SFRTEST_STATE_ENUM eResult;

   SFRTEST_SAFEINDEX_SAFETY_ASSERT(si);

   eResult = sfrTestStm_Trigger(&sfrTest_asCfg[(UINT32)(si&0xFFFFuL)],
                             SFRTEST_EV_RESET);

   if (eResult == SFRTEST_STATE_ERROR)
   {
      SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
   }
}


#if SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE
/******************************************************************************/
/*!
@Name                sfrTest_ConfirmReference

@Description         This function shall be called to confirm that the data
                     from which the reference value(s) have been determined is
                     still valid and consistent. Testing can start only if this
                     confirmation is done.
                     If the Stm-Trigger function returns an error, the SFR-Test
                     Error Handler is called.
                     If the given Safe-Index is not correct, the Safety Handler
                     is called.

@note                It is the caller's responsibility to ensure the validity
                     and consistency of the data that builds the base of the
                     reference value(s). It should be ensured by code inspection
                     that this verification step is properly performed.

@Parameter
   @param [in]       SAFEINDEX   Safeindex of the config
   @return           void
 */
/******************************************************************************/
void sfrTest_ConfirmReference (CONST SAFEINDEX si)
{
   SFRTEST_STATE_ENUM eResult;

   SFRTEST_SAFEINDEX_SAFETY_ASSERT(si);

   eResult = sfrTestStm_Trigger(&sfrTest_asCfg[(UINT32)(si&0xFFFFuL)],
                                SFRTEST_EV_CONFIRM);

   if (eResult == SFRTEST_STATE_ERROR)
   {
      SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
   }
}
#endif /* SFRTEST_ENABLE_PARAM_CRC_CALC == TRUE */


/******************************************************************************/
/*!
@Name                sfrTest_Execute

@Description         This function executes one step of the special function
                     register test and returns the resulting state. The return
                     value must be evaluated by the caller, since the test needs
                     to be reset when it is done before the next test cycle can
                     be started, and the calling program part should also
                     implement a watchdog functionality to ensure that the test
                     is completed within a given time interval.
                     If the given Safe-Index is not correct, the Safety Handler
                     is called.

@note                -

@Parameter
   @param [in]       SAFEINDEX   Safeindex of the config
   @return           SFRTEST_STATE_ENUM - the current state of the sfr test
                                          after the event was handled
 */
/******************************************************************************/
SFRTEST_STATE_ENUM sfrTest_Execute (CONST SAFEINDEX si)
{
   SFRTEST_STATE_ENUM eResult;

   SFRTEST_SAFEINDEX_SAFETY_ASSERT(si);

   eResult = sfrTestStm_Trigger(&sfrTest_asCfg[(UINT32)(si&0xFFFFuL)],
                                SFRTEST_EV_EXECUTE);

   if (eResult == SFRTEST_STATE_ERROR)
   {
      SFRERRHND_ReportError(SFRTEST_ERRSTATE_STM);
   }

   return eResult;
}

/***** End of: glbfunc Global Functions *********************************//*@}*/


/**** Last line of source code                                             ****/

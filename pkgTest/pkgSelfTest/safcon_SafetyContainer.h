/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: safcon_SafetyContainer.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file:         safcon_SafetyContainer.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        Header of the Safety-Container.
 * @description:
 * The module Safety container is responsible for backing up the configuration
 * parameters. The configuration parameters are data that will never change
 * during runtime. They are for example read at startup from the EEPROM and
 * then kept in RAM.
 * To protect the data against corruption a checksum is calculated cyclically
 * over the values of the parameters in the RAM.
 *
 * At startup an initial checksum is calculated over the parameters. This
 * checksum is the basic value and it is used to check against the cyclically
 * calculated checksum. If the checksums not equal at the checking time the
 * error handler will be called.
*/
/************************************************************************//*@}*/

#ifndef SAFCON_SAFETYCONTAINER_H
#define SAFCON_SAFETYCONTAINER_H

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


/***** End of: macros Macros ********************************************//*@}*/


/******************************************************************************/
/* Elementary Types (typedef)                                                 */
/******************************************//*!@addtogroup typedef Types*//*@{*/


/***** End of: typedef Types ********************************************//*@}*/


/******************************************************************************/
/* Global Variables or Constants (const)                                      */
/*******************************//*!@addtogroup globvar Global Variables*//*@{*/


/***** End of: globvar Global Variables *********************************//*@}*/


/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

extern void safcon_Init (void);
extern void safcon_CheckCrc (void);

/***** End of: Function-Prototypes ********************************************/


#endif /* SAFCON_SAFETYCONTAINER_H */

/**** Last line of source code                                             ****/

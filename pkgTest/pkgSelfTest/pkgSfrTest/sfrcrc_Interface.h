/*********************************************//*!@addtogroup file Files*//*@{*/
/*!
 *******************************************************************************
 *       $Id: sfrcrc_Interface.h 569 2016-08-31 13:08:08Z klan $
 * $Revision: 569 $
 *     $Date: 2016-08-31 15:08:08 +0200 (Mi, 31 Aug 2016) $
 *   $Author: klan $
 *******************************************************************************
 *
 * @project:      HMS_ASM
 * @customer:     HMS Industrial Networks GmbH
 *
 * @file          sfrcrc_Interface.h
 * @origAuthor:   tlangenbacher / embeX GmbH
 *
 * @brief:        The interface for the crc calculation mapping.
 * @description:  -
 */
/************************************************************************//*@}*/

#ifndef SFRCRC_INTERFACE
#define SFRCRC_INTERFACE

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
extern SFRCRC crc_AddU8(CONST SFRCRC uCrc, CONST UINT8 u8Value);
extern SFRCRC crc_AddU16(CONST SFRCRC uCrc, CONST UINT16 u16Value);
extern SFRCRC crc_AddU32(CONST SFRCRC uCrc, CONST UINT32 u32Value);

/***** End of: Function-Prototypes ********************************************/


#endif /* SFRCRC_INTERFACE */

/**** Last line of source code                                             ****/

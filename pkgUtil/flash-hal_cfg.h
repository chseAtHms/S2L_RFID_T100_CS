/***************************************************************************************************
**    Copyright (C) 2016 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: flash-hal_cfg.h
**     Summary: Configuration header of the Flash module.
**   $Revision: 1077 $
**       $Date: 2016-11-15 11:47:14 +0100 (Di, 15 Nov 2016) $
**      Author: K. Angele
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions:
**
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef FLASHHAL_CFG_H
#define FLASHHAL_CFG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Start address of accessible flash area (see [SRS_2174]) */ 
#define FLASHHAL_cfg_USR_FLASH_START     (uint32_t)(0x0803c000)
/* End address of accessible flash area (see [SRS_2174]) */ 
#define FLASHHAL_cfg_USR_FLASH_END       (uint32_t)(0x0803e7fe)

/***************************************************************************************************
**    data types
***************************************************************************************************/

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


#endif


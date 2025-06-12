/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** globErrHandler.c
**
** $Id: globErrHandler.c 3972 2023-03-24 14:06:49Z ankr $
** $Revision: 3972 $
** $Date: 2023-03-24 15:06:49 +0100 (Fr, 24 Mrz 2023) $
** $Author: ankr $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** This module contains the global error handling for the DIs and DOs.
** The DI and DO module get the information from this module if there is an
** error or not.
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013 HMS Industrial Networks AB                 **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

/*******************************************************************************
**
** includes
**
********************************************************************************
*/
#include "xtypes.h"
#include "xdefs.h"

/* Module header */
#include "globErrHandler.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/


/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/
/*------------------------------------------------------------------------------
**  Variable to store a possible error reset request
**------------------------------------------------------------------------------
*/
STATIC TRUE_FALSE_ENUM globErrHandler_eDiErrRstReq = eFALSE;
STATIC TRUE_FALSE_ENUM globErrHandler_eDoErrRstReq = eFALSE;

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/


/*------------------------------------------------------------------------------
**
** globErrReset()
**
** Description:
** external error reset requested (can be done by AIC message (see [SRS_493]),
** [SIS_034] or Portpin)
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    globErrReset();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void globErrHandler_ErrReset(void)
{
   globErrHandler_eDiErrRstReq = eTRUE;
   globErrHandler_eDoErrRstReq = eTRUE;
}


/*------------------------------------------------------------------------------
**
** globErrHandler_GetDiRstReq()
**
** Description:
** returns current value of DI error reset request, used by diState to fulfill
** [SRS_2011]
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    TRUE_FALSE_ENUM eTRUE or eFALSE
**
** Usage:
**    x = globErrHandler_GetDiRstReq();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM globErrHandler_GetDiRstReq(void)
{
   return globErrHandler_eDiErrRstReq;
}


/*------------------------------------------------------------------------------
**
** globErrHandler_GetDoRstReq()
**
** Description:
** returns current value of DO error reset request, used to fulfill [SRS_2015]
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    TRUE_FALSE_ENUM eTRUE or eFALSE
**
** Usage:
**    x = globErrHandler_GetDoRstReq();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
TRUE_FALSE_ENUM globErrHandler_GetDoRstReq(void)
{
   return globErrHandler_eDoErrRstReq;
}


/*------------------------------------------------------------------------------
**
** globErrHandler_DiRstConf()
**
** Description:
** function to confirm that the DI module tried to reset any present DI error
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    globErrHandler_DiRstConf();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void globErrHandler_DiRstConf(void)
{
   globErrHandler_eDiErrRstReq = eFALSE;
}


/*------------------------------------------------------------------------------
**
** globErrHandler_DoRstConf()
**
** Description:
** function to confirm that the DO module tried to reset any present DO error
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Usage:
**    globErrHandler_DoRstConf();
**
** Module Test:
**    - NO -
**    Reason: Only function calls
**------------------------------------------------------------------------------
*/
void globErrHandler_DoRstConf(void)
{
   globErrHandler_eDoErrRstReq = eFALSE;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of globErrHandler.c
**
********************************************************************************
*/

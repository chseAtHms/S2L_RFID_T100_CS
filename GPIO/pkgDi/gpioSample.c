/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpioSample.c
**
** $Id: gpioSample.c 2448 2017-03-27 13:45:16Z klan $
** $Revision: 2448 $
** $Date: 2017-03-27 15:45:16 +0200 (Mo, 27 Mrz 2017) $
** $Author: klan $
**
** Original Author: S. Arzdorf
**
********************************************************************************
********************************************************************************
**
** Description
** -----------
**
** Digital Input sample module. This module offers functions to read out
** all inputs and to store them.
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

/* Project header */
#include "stm32f10x.h"
#include "xtypes.h"
#include "xdefs.h"


/* Header-file of module */
#include "gpioSample.h"


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  input-buffer:
**  structure containing the Port-Input values of the last x (dimension)
**  read cycles (see [SRS_338]).
**  Global variable used instead of function, because this sample
**  data is used in every time slice and processing is very time critical.
**------------------------------------------------------------------------------
*/
UINT16 gpioSample_au16Buffer[DISMP_PORTDIM][DISMP_BUFFDIM];

/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  constant pointer to constant array containing the Register addresses of Ports
 *  to read
**------------------------------------------------------------------------------
*/
STATIC CONST GPIO_TypeDef* CONST gpioSample_Ports[DISMP_PORTDIM] =
{
   (GPIOA),
   (GPIOB),
   (GPIOC),
   (GPIOD),
   (GPIOE),
   (GPIOF),
   (GPIOG)
};


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** gpioSample_AllPorts()
**
** Description:
**    function gets all port values by reading the IDR (Input Data Register), and
**    saves the result of several read cycles and a time stamp to a buffer. Note:
**    - IDR register contains the pin value only for GPIOs initialized as
**      "input" or "output open drain"
**    - IDR register is read out regardless of current portpin configuration
**
**    => functions using the result of this function must know about port-
**       mapping and configuration, because result is usable only for input and
**       open drain portpins.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Outputs:
**    - None -
**
** Usage:
**     gpioSample_AllPorts()
**
** Module Test:
**    - YES -
**    Reason: error in write access of array might overwrite other RAM
**------------------------------------------------------------------------------
*/
void gpioSample_AllPorts(void)
{
   /* static variable for current buffer index */
   LOCAL_STATIC(, UINT8, u8BuffIndex, 0u);

   /* index for different ports */
   UINT8 u8portIndex;

   /* reset buffer index if buffer size end reached, this is done at the beginning
    * to check automatically the range of variable */
   if (DISMP_BUFFDIM <= u8BuffIndex)
   {
      u8BuffIndex = 0u;
   }

   /* read port values: the value of the whole port will be read out by reading
    * the complete IDR register note: the IDR register contains the pin value
    * only for GPIOs initialized as "input" or "output open drain" */
   for (u8portIndex = 0u; u8portIndex < DISMP_PORTDIM; u8portIndex++ )
   {
      /* reading of DI input portpin, see also [SRS_337] */
      gpioSample_au16Buffer[u8portIndex][u8BuffIndex] = 
          (UINT16)((gpioSample_Ports[u8portIndex]->IDR) & 0xFFFFu);
   }

   /* increase buffer index */
   u8BuffIndex++;
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/


/*******************************************************************************
**
** End of gpioSample.c
**
********************************************************************************
*/

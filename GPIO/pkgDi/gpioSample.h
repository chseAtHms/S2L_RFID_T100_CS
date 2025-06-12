/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** gpioSample.h
**
** $Id: gpioSample.h 2448 2017-03-27 13:45:16Z klan $
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
** header file of "gpioSample.c"
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

#ifndef GPIOSAMPLE_H_
#define GPIOSAMPLE_H_


/*******************************************************************************
**
** Constants
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** number of ports to read/sample
**------------------------------------------------------------------------------
*/
#define DISMP_PORTDIM   ((UINT8) 7)

/*------------------------------------------------------------------------------
** number of buffer dimension (means: how many times a port value is saved).
** In "X out of Y", this is the "Y"
**------------------------------------------------------------------------------
*/
#define DISMP_BUFFDIM   ((UINT8) 3)

/*------------------------------------------------------------------------------
** X out of Y: how many of the buffer entries must have value "active" before
** value is seen as active
** In "X out of Y", this is the "X"
**------------------------------------------------------------------------------
*/
#define DISMP_X_OO_Y    ((UINT8)(DISMP_BUFFDIM-1u))


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** enum specifies the positions of the read values of the several ports in the
** read buffer. Note: ORDER MUST BE THE SAME LIKE IN "gpioSample_Ports"
** no hamming distance here, because the values are indicating buffer positions
**------------------------------------------------------------------------------
*/
typedef enum
{
   /* RSM_IGNORE_QUALITY_BEGIN Notice #50 - Variable assignment to a literal number */
   eBUFFER_INDEX_PORTA = 0u,
   eBUFFER_INDEX_PORTB = 1u,
   eBUFFER_INDEX_PORTC = 2u,
   eBUFFER_INDEX_PORTD = 3u,
   eBUFFER_INDEX_PORTE = 4u,
   eBUFFER_INDEX_PORTF = 5u,
   eBUFFER_INDEX_PORTG = 6u
   /* RSM_IGNORE_QUALITY_END */
} GPIOSAMPLE_DI_BUFFER_INDEX_PORT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**  input-buffer:
**  structure containing the Port-Input values of the last x (dimension)
**  read cycles. Global variable used instead of function, because this sample
**  data is used in every time slice and processing is very time critical.
**------------------------------------------------------------------------------
*/
extern UINT16 gpioSample_au16Buffer[DISMP_PORTDIM][DISMP_BUFFDIM];

/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

extern void gpioSample_AllPorts(void);

#endif  /* inclusion lock */

/*******************************************************************************
**
** End of gpioSample.h
**
********************************************************************************
*/

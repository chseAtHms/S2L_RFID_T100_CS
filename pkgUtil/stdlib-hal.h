/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: stdlib-hal.h
**     Summary: Interface of wrapper function for C Standard Library functionality
**   $Revision: 2427 $
**       $Date: 2017-03-24 10:06:34 +0100 (Fr, 24 Mrz 2017) $
**      Author: KlAn
** Responsible: KlAn
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: stdlibHAL_ByteArrCopy
**             stdlibHAL_ByteArrSet
**             stdlibHAL_ByteArrComp
**
**    History:
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef STDLIB_HAL_H
#define STDLIB_HAL_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/


/***************************************************************************************************
**    data types
***************************************************************************************************/


/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    function prototypes
***************************************************************************************************/


/***************************************************************************************************
  Function:
    stdlibHAL_ByteArrCopy

  Description: 
     The function copies u16_len bytes from the byte array pointed to by pu8_src into the
     byte array pointed to pu8_dst. It is task of the calling function to ensure that the arrays
     are big enough.

     This interface differs from the memcpy interface defined in the C Standard Library.

  See also:
    -

  Parameters:   
    pu8_dst (OUT)    - This is pointer to the destination array where the content is to be copied.
                       (valid range: <>NULL, checked)
    pu8_src (IN)     - This is pointer to the source of data to be copied.
                       valid range: <>NULL, checked)
    u16_len (IN)     - This is the number of bytes to be copied.
                       (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler
    Context: Background Task

***************************************************************************************************/
void stdlibHAL_ByteArrCopy(volatile UINT8* pu8_dst, const volatile UINT8* pu8_src, UINT16 u16_len);


/***************************************************************************************************
  Function:
    stdlibHAL_ByteArrSet

  Description: 
     The function copies the value of u8_val (unsigned char) into each of the first u16_len 
     bytes of the byte array pointed to by pu8_dst.

     This interface differs from the memset interface defined in the C Standard Library.

  See also:
    -

  Parameters:   
    pu8_dst (OUT)    - This is the pointer to the byte array to fill.
                       (valid range <>NULL, checked)
    u8_val (IN)      - This is the value to be set.
                       (valid range: any, not checked)
    u16_len (IN)     - This is the number of bytes to be set to the value.
                       (valid range: any, not checked)

  Return value:
    -

  Remarks: 
    Context: IRQ Scheduler
    Context: Background Task

***************************************************************************************************/
void stdlibHAL_ByteArrSet(UINT8 pu8_dst[], UINT8 u8_val, UINT16 u16_len);

/***************************************************************************************************
  Function:
    stdlibHAL_ByteArrComp

  Description: 
     This function compares the first u16_len bytes of byte array pu8_arr1 and byte array
     pu8_arr2.

     This interface differs from the memcmp interface defined in the C Standard Library.

  See also:
    -

  Parameters:   
    pu8_arr1 (IN)    - This is the pointer to a byte array.
                       (valid range: <>NULL, checked)
    pu8_arr2 (IN)    - This is the pointer to a byte array.
                       (valid range: <>NULL, checked)
    u16_len (IN)     - This is the number of bytes to be compared.
                       (valid range: any, not checked)

  Return value:
    - if < 0 then it indicates pu8_arr1 is less than pu8_arr2
    - if > 0 then it indicates pu8_arr2 is less than pu8_arr1.
    - if = 0 then it indicates pu8_arr1 is equal to pu8_arr2.

  Remarks: 
    Context: Background Task

***************************************************************************************************/
INT16 stdlibHAL_ByteArrComp(const UINT8 pu8_arr1[], const UINT8 pu8_arr2[], UINT16 u16_len);

#endif


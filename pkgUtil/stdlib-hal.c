/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: stdlib-hal.c
**     Summary: Implementation of wrapper function for C Standard Library functionality
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


/***************************************************************************************************
**    include-files
***************************************************************************************************/

/* system includes */
#include "xtypes.h"

#include "globDef_GlobalDefinitions.h"
#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"

#include "stdlib-hal.h"

/***************************************************************************************************
**    global variables
***************************************************************************************************/


/***************************************************************************************************
**    static constants, types, macros, variables
***************************************************************************************************/

/* Unique file id used to build additional info */
#define k_FILEID      33u


/***************************************************************************************************
**    static function-prototypes
***************************************************************************************************/


/***************************************************************************************************
**    global functions
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
void stdlibHAL_ByteArrCopy(volatile UINT8* pu8_dst, const volatile UINT8* pu8_src, UINT16 u16_len)
{
  /* ATTENTION:
  ** 'packed pointers' are used to force the compiler to generate code to support unaligned accesses
  ** e.g. LDM/STM instructions are avoided.
  */
  __packed volatile UINT32* pu32_longDst;
  __packed const volatile UINT32* pu32_longSrc;
  
  volatile UINT8* pu8_byteDst;
  const volatile UINT8* pu8_byteSrc;
  
  /* copy iterator */
  UINT16 i = 0u;
  /* number of longs (32 bit) to copy */
  UINT16 u16_longCpyNum = u16_len / (UINT8)sizeof(UINT32);
  /* number of remaining bytes (8 bit) to copy */
  UINT16 u16_byteCpyNum  = u16_len & 0x0003u;

  /* if pointers invalid (NULL) */
  if ( (pu8_dst == NULL) || (pu8_src == NULL) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_POINTER_INVALID_EX, GLOBFAIL_ADDINFO_FILE(1u));
  }
  /* else: pointers valid */
  else 
  {
    /* convert byte addressing to long addressing */
    /* Note 927: cast from pointer to pointer [MISRA 2004 Rule 11.4] */
    /* Info 826: Suspicious pointer-to-pointer conversion (area too small) [possibly violation
    ** MISRA 2004 Rule 11.4] */
    /* --> Deactivate warnings, access (even unaligned is considered as safe because
    ** 'packed pointers' are used */
    pu32_longDst = (volatile UINT32*)pu8_dst;       /*lint !e927, !e826*/
    pu32_longSrc = (const volatile UINT32*)pu8_src; /*lint !e927, !e826*/
    
    /* copy long values */
    for (i = 0u; i < u16_longCpyNum; i++)
    {
      /*lint -esym(960, 17.4)*/
      /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
      * other than array indexing used
      * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
      pu32_longDst[i] = pu32_longSrc[i];
      /*lint +esym(960, 17.4)*/
    }
    
    
    /* convert back to byte addressing (next long address) */
    /*lint -esym(960, 17.4)*/
    /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    ** other than array indexing used
    ** Info 826: Suspicious pointer-to-pointer conversion (area too small) [possibly violation
    ** MISRA 2004 Rule 11.4] 
    ** Note 928: cast from pointer to pointer [possibly violation MISRA 2004 Rule 11.4]
    ** --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
    pu8_byteDst = (volatile UINT8*)&pu32_longDst[i];        /*lint !e826, !e928 */
    pu8_byteSrc = (const volatile UINT8*)&pu32_longSrc[i];  /*lint !e826, !e928 */
    /*lint +esym(960, 17.4)*/
    
    /* copy trailing bytes byte-by-byte */
    for (i = 0u; i < u16_byteCpyNum; i++)
    {
      /*lint -esym(960, 17.4)*/
      /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
      * other than array indexing used
      * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
      pu8_byteDst[i] = pu8_byteSrc[i];
      /*lint +esym(960, 17.4)*/
    }
  }
}


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
void stdlibHAL_ByteArrSet(UINT8* pu8_dst, UINT8 u8_val, UINT16 u16_len)
{ 
  /* ATTENTION:
   ** 'packed pointers' are used to force the compiler to generate code to support unaligned 
   ** accesses e.g. LDM/STM instructions are avoided.
   */
  __packed UINT32* pu32_longDst;
  
  UINT8* pu8_byteDst;

  /* copy iterator */
  UINT16 i = 0u;
  /* number of longs (32 bit) to copy */
  UINT16 u16_longCpyNum = u16_len / (UINT8)sizeof(UINT32);
  /* number of remaining bytes (8 bit) to copy */
  UINT16 u16_byteCpyNum  = u16_len & 0x0003u;

   /* value used for 4 byte accesses */
  UINT32 u32_longVal = ( (UINT32)u8_val
                         + (UINT32)((UINT32)u8_val << 8)
                         + (UINT32)((UINT32)u8_val << 16)
                         + (UINT32)((UINT32)u8_val << 24) );
  
  /* if pointer invalid (NULL) */
  if (pu8_dst == NULL)
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_POINTER_INVALID_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: pointers valid */
  else
  {
    /* convert byte addressing to long addressing */
    /* Info 826: Suspicious pointer-to-pointer conversion (area too small) [possibly violation
    ** MISRA 2004 Rule 11.4] */
    /* --> Deactivate warnings, access (even unaligned is considered as safe because
    ** 'packed pointers' are used */
    pu32_longDst = (UINT32*)pu8_dst; /*lint !e927, !e826*/
    
    /* set long values */
    for (i = 0u; i < u16_longCpyNum; i++)
    {
      /*lint -esym(960, 17.4)*/
      /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
      * other than array indexing used
      * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
      pu32_longDst[i] = u32_longVal;
      /*lint +esym(960, 17.4)*/
    }

    /* convert back to byte addressing (next long address) */
    /*lint -esym(960, 17.4)*/
    /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
    ** other than array indexing used
    ** Info 826: Suspicious pointer-to-pointer conversion (area too small) [possibly violation
    ** MISRA 2004 Rule 11.4] 
    ** Note 928: cast from pointer to pointer [possibly violation MISRA 2004 Rule 11.4]
    ** --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
    pu8_byteDst = (UINT8*)&pu32_longDst[i]; /*lint !e826, !e928 */
    /*lint +esym(960, 17.4)*/
    
    /* set trailing bytes byte-by-byte */
    for (i = 0u; i < u16_byteCpyNum; i++)
    {
      /*lint -esym(960, 17.4)*/
      /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
      * other than array indexing used
      * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
      pu8_byteDst[i] = u8_val;
      /*lint +esym(960, 17.4)*/
    }
  }
}

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
INT16 stdlibHAL_ByteArrComp(const UINT8 pu8_arr1[], const UINT8 pu8_arr2[], UINT16 u16_len)
{
  INT16 i16_ret = 0;
  UINT16 i;
  
  /* if pointers invalid (NULL) */
  if ( (pu8_arr1 == NULL) || (pu8_arr2 == NULL) )
  {
    GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_POINTER_INVALID_EX, GLOBFAIL_ADDINFO_FILE(3u));
  }
  /* else: pointers valid */
  else 
  {
    /* compare copy byte per byte, 
    ** if two byte are unequal, the loop is left */
    for (i = 0u; ( (i < u16_len) && (i16_ret == 0) ); i++)
    {
      i16_ret = (INT16)pu8_arr1[i] - (INT16)pu8_arr2[i];
    }
  }
  return i16_ret;
}



/***************************************************************************************************
**    static functions
***************************************************************************************************/



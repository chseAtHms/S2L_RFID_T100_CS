/***************************************************************************************************
**    Copyright (C) 2016-2020 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLnvHigh.h
**     Summary: Interface for high level NV memory functionality required e.g. by CSS.
**   $Revision: 3418 $
**       $Date: 2020-04-07 14:31:08 +0200 (Di, 07 Apr 2020) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_NvHighInit
**             SAPL_NvHighConfigStore
**             SAPL_NvHighConfigRestore
**             SAPL_NvHighDefaultStore
**             SAPL_NvHighResetType2Store
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_NVHIGH_H
#define SAPL_NVHIGH_H

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
    SAPL_NvHighInit

  Description:
    This function is used to initialize the application specific part of the NV memory handling.
    The function has to be called, BEFORE the IRQ scheduler is started!

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: main (during initialization)

***************************************************************************************************/
void SAPL_NvHighInit(void);

/***************************************************************************************************
  Function:
    SAPL_NvHighConfigStore

  Description:
    This function is used to store the Configuration Data (IO) inside NV memory (see [SRS_2143]).
    For this purpose the passed values are set inside the Write Buffer of the application. The 
    FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    pb_storeData (IN)  - pointer to the memory space where the data is stored
                         (valid range: <>NULL, not checked, only passed with reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighConfigStore(CONST UINT8* pb_storeData);

/***************************************************************************************************
  Function:
    SAPL_NvHighConfigRestore

  Description:
    This function is used to read the Configuration Data (IO) from NV memory.

  See also:
    -

  Parameters:
    pb_storeData (OUT) - pointer to the memory space where the stored data is restored
                         (valid range: <>NULL, not checked, only passed with reference)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighConfigRestore(UINT8* pb_storeData);
                                 
/***************************************************************************************************
  Function:
    SAPL_NvHighDefaultStore

  Description:
    This function is used to set the NV variables to their default values. 
    For this purpose the default values are set inside the Write Buffer of the application. The 
    FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighDefaultStore(void);

/***************************************************************************************************
  Function:
    SAPL_NvHighResetType2Store

  Description:
    This function is used to process the Attribute Bit Map received within a Safety Reset Type 2.
    Depending on the Attribute Bit Map some or all parameters of the NV memory will be set to 
    default values by the application. See [SRS_2171].
    The FlashHandler will program the data in the NV memory concurrently then.

  See also:
    -

  Parameters:
    u8_attrBitMap (IN) - The Attribute Bit Map is part of the Safety Reset Type 2 and indicates
                         which parameters shall be PRESERVED.

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void SAPL_NvHighResetType2Store(CONST UINT8 u8_attrBitMap);

#endif


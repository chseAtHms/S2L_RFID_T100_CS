/***************************************************************************************************
**    Copyright (C) 2015-2024 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: SAPLcfg.h
**     Summary: Configuration header of SAPL
**   $Revision: 4440 $
**       $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
**      Author: KlAn
** Responsible: (optional)
**
****************************************************************************************************
****************************************************************************************************
**
**  Functions: SAPL_ConfigStringCheck
**
**    History: -
**
****************************************************************************************************
**    Template Version 5
***************************************************************************************************/

#ifndef SAPL_CFG_H
#define SAPL_CFG_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/* Parameters of output assembly instance (see [SRS_2102]) */
#define SAPL_IO_OUT_ASSEMBLY_ID      (CSS_t_UINT)0x300u
/* payload in octets (see [SRS_2051], [SRS_2062]) */
#define SAPL_IO_OUT_ASSEMBLY_LEN     (CSS_t_UINT)(sizeof(AICMSGDEF_PS_OUT_DATA_STRUCT))

/* Parameters of input assembly instance (see [SRS_2093]) */
#define SAPL_IO_IN_ASSEMBLY_ID       (CSS_t_UINT)0x264u
/* payload in octets (see [SRS_2063], [SRS_2056]) */
#define SAPL_IO_IN_ASSEMBLY_LEN      (CSS_t_UINT)(sizeof(AICMSGDEF_PS_INPUT_DATA_STRUCT))

/* Parameters of configuration assembly instance (see [SRS_2120]) */
#define SAPL_IO_CFG_ASSEMBLY_ID      (CSS_t_UINT)0x340u
/* payload in octets */
#define SAPL_IO_CFG_ASSEMBLY_LEN     (CSS_t_UINT)(sizeof(UINT8) +  \
                                                  sizeof(UINT16) + \
                                                  sizeof(fiParam_sIParam))

/* Min. Supported RPI for O_to_T RPI and T_to_O RPI (see [SRS_2147])*/ 
#define SAPL_cfg_MIN_RPI_SUPPORTED    (CSS_t_UDINT)4000u /* in [us] */


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
    SAPL_ConfigStringCheck

  Description:
    This function performs several value and consistency checks on the received configuration data
    string.

  See also:
    -

  Parameters:
    ab_cfgData (IN)  - array with received configuration data (without header). The length was
                       already checked by the caller.

  Return value:
    CSS_k_OK         - configuration string successfully checked.
    <>CSS_k_OK       - deny the connection request. Returned value will be responded in
                       the Extended Status of the Unsuccessful Forward_Open Response.

  Remarks:
    Context: Background Task

***************************************************************************************************/
UINT16 SAPL_ConfigStringCheck(const CSS_t_BYTE ab_cfgData[]);

#endif /* SAPL_CFG_H */


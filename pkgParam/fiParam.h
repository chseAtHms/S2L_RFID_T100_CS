/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** fiParam.h
**
** $Id: fiParam.h 4440 2024-05-29 12:03:45Z ankr $
** $Revision: 4440 $
** $Date: 2024-05-29 14:03:45 +0200 (Mi, 29 Mai 2024) $
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
** include file of fiParam.c
**
********************************************************************************
********************************************************************************
**                                                                            **
** COPYRIGHT NOTIFICATION (c) 2013-2024 HMS Industrial Networks AB            **
**                                                                            **
** This program is the property of HMS Industrial Networks AB.                **
** It may not be reproduced, distributed, or used without permission          **
** of an authorised company official.                                         **
**                                                                            **
********************************************************************************
********************************************************************************
*/

#ifndef FIPARAM_H_
#define FIPARAM_H_

/*******************************************************************************
**
** Switches (#define)
**
********************************************************************************
*/


/*******************************************************************************
**
** Constants (#define)
**
********************************************************************************
*/
/*** Macros to read data from the configuration string - Byte Offsets ***/
/* The following defines might not be referenced (depends on support of SS1-t and SafeBound
    feature) */
/*lint -esym(755, FIPARAM_k_OFS_SS1T_DLY_B1) */
/*lint -esym(755, FIPARAM_k_OFS_SS1T_DLY_B2) */
/*lint -esym(755, FIPARAM_k_OFS_SAFEBOUND_B1) */
#define FIPARAM_k_OFS_DI12_PARAMS     2u  /* byte offset of number of DI12 configuration */
#define FIPARAM_k_OFS_DI34_PARAMS     6u  /* byte offset of number of DI34 configuration */
#define FIPARAM_k_OFS_DI56_PARAMS    10u  /* byte offset of number of DI56 configuration */
#define FIPARAM_k_OFS_DO_PARAMS      14u  /* byte offset of number of DO12 configuration */
#define FIPARAM_k_OFS_SS1T_DLY_B1    19u  /* byte offset of first config byte of SS1-t delay */
#define FIPARAM_k_OFS_SS1T_DLY_B2    20u  /* byte offset of second config byte of SS1-t delay */
#define FIPARAM_k_OFS_SAFEBOUND_B1   21u  /* byte offset of first config byte of SafeBound Enable */

/*** Macros to read data from the configuration string - Bit Masks ***/
#define FIPARAM_k_MASK_DI_ENABLED    0x01u  /* mask for DI Enable bit */
#define FIPARAM_k_MASK_DO_ENABLED    0x01u  /* mask for DO Enable bit */
#define FIPARAM_k_MASK_SAFEBOUND_B12 0x03u  /* mask for SafeBound Enable bit 1+2 */
#define FIPARAM_k_MASK_SAFEBOUND_B34 0x0Cu  /* mask for SafeBound Enable bit 3+4 */
#define FIPARAM_k_MASK_SAFEBOUND_B56 0x30u  /* mask for SafeBound Enable bit 5+6 */


/*******************************************************************************
**
** Data Types
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** This enum contains the available parameterization values of TO test
**------------------------------------------------------------------------------
*/
/*lint -esym(769, FIPARAM_E_DICFG_TO_400us)
       -esym(769, FIPARAM_E_DICFG_TO_800us)
       -esym(769, FIPARAM_E_DICFG_TO_1200us)
       -esym(769, FIPARAM_E_DICFG_TO_1600us)
       -esym(769, FIPARAM_E_DICFG_TO_2000us)
       -esym(769, FIPARAM_E_DICFG_TO_2400us)
       -esym(756, FIPARAM_DI_CFG_TO_ENUM)*/
/* Lint message deactivated because the enum values are defined but not used at
 * the moment, for future use. */
 /* RSM_IGNORE_BEGIN Notice #50   - Variable assignment to a literal number
 ** accepted because existing definition */
typedef enum
{   /* no hamming distance here */
   FIPARAM_E_DICFG_TO_400us      = 0u, /* TO test pulse 400us */
   FIPARAM_E_DICFG_TO_800us      = 1u, /* TO test pulse 800us */
   FIPARAM_E_DICFG_TO_1200us     = 2u, /* TO test pulse 1200us */
   FIPARAM_E_DICFG_TO_1600us     = 3u, /* TO test pulse 160us */
   FIPARAM_E_DICFG_TO_2000us     = 4u, /* TO test pulse 200us */
   FIPARAM_E_DICFG_TO_2400us     = 5u, /* TO test pulse 2400us */
   FIPARAM_E_DICFG_TO_OFF        = 6u, /* TO always LOW, shall not be used */
   FIPARAM_E_DICFG_TO_ON         = 7u  /* TO always HIGH, shall not be used */
} FIPARAM_DI_CFG_TO_ENUM;
/* RSM_IGNORE_END */

/*------------------------------------------------------------------------------
** typedef for structure containing the setting of one output
**------------------------------------------------------------------------------
*/
typedef __packed struct FIPARAM_DI_PARAM_TAG
{
   UINT8  u8Properties;
   UINT8  u8DebFilter;           /* debounce filter */
   UINT16 u16ConsFilter;         /* consistency filter */
} FIPARAM_DI_PARAM_STRUCT;


/*------------------------------------------------------------------------------
** typedef for structure containing the setting of one output
**------------------------------------------------------------------------------
*/
typedef __packed struct FIPARAM_DO_PARAM_TAG
{
   UINT8 u8Properties;
} FIPARAM_DO_PARAM_STRUCT;

/*------------------------------------------------------------------------------
**  iParameters for DI/DO configuration
**------------------------------------------------------------------------------
*/
/*lint -esym(768, PARAMETER_IPAR_TAG::numOfDualChDIs)*/
/*lint -esym(768, PARAMETER_IPAR_TAG::numOfDualChDOs)*/
/*lint -esym(768, PARAMETER_IPAR_TAG::u8_safeBoundPadByte)*/
/* Lint message 768 is deactivated for some structure members. They are defined
 * but not used, for future purpose. */
typedef __packed struct PARAMETER_IPAR_TAG
{
   UINT8 numOfDualChDIs;
   UINT8 numOfDualChDOs;
   FIPARAM_DI_PARAM_STRUCT diParam[GPIOCFG_NUM_DI_PER_uC];  /* array of structures containing */
                                                            /* the properties of the DOs */
   FIPARAM_DO_PARAM_STRUCT doParam[GPIOCFG_NUM_DO_PER_uC];  /* array of structures containing */
                                                            /* the properties of the DOs */
   UINT16 u16_latchInputErrTime;
   UINT16 u16_latchOutputErrTime;

  /* SS1-t: DO safe state delay in multiples of 4ms. This parameter exists only once for all DOs
     (see [SRS_908]). In case of Single-channel DO this parameter applies to both channels.
     The configuration value for SS1-t is contained in the configuration data independent
     of CFG_S2L_SS1T_SUPPORTED. Thus, the configuration string always has the same
     length. In case the feature is deactivated the value is checked to be zero (see [SRS_911]). */
   UINT16 u16_ss1tDelay4ms;

  /* SafeBound: Bit-coded per digital Inputs: Bit 0 (LSB) = DI1. Two uppermost bits are unused. See
     [SRS_902]. The configuration value for SafeBound is contained in the configuration data
     independent of CFG_S2L_SAFEBOUND_SUPPORTED. Thus, the configuration string always has the same
     length. In case the feature is deactivated the value is checked to be zero (see [SRS_903]). */
   UINT8 u8_safeBoundPerDiEnable;
   /* pad byte to make the whole config string even length (see [SRS_2254]) */
   UINT8 u8_safeBoundPadByte;

}PARAMETER_IPAR_STRUCT;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Module wide accessible input properties, global used due to performance
** reasons
**------------------------------------------------------------------------------
*/
extern volatile PARAMETER_IPAR_STRUCT fiParam_sIParam;


/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/



/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/
extern void fiParam_setSafConCheckActive (TRUE_FALSE_ENUM eSafConAct);
extern void fiParam_setIParam(CONST UINT8* pu8Src, CONST UINT8 u8Quantity);
extern void fiParam_setCandidateIParam(CONST UINT8* pu8Src, CONST UINT8 u8Quantity,
                                       CONST UINT32 u32sccrc);
extern UINT8* fiParam_getCandidateIParam(UINT32 *pu32Sccrc);
extern BOOL fiParam_checkCandidateIParam(void);
extern void fiParam_clearCandidateIParam(void);
extern void fiParam_CalcFiParamSafeConCrc(void);
extern UINT16 fiParam_GetConsFilter (CONST UINT8 u8DualDiNum);
extern void fiParam_setIParCrc(UINT32 u32_sccrc);
extern UINT32 fiParam_SccrcCompute(const volatile UINT8 *pu8_start, UINT16 u16_len);

/*******************************************************************************
********************************************************************************
**
** MACROS FOR DIs
**
********************************************************************************
*******************************************************************************/

/*------------------------------------------------------------------------------
**  Macros to get enable/disable configuration of the DI (see [SRS_234], [SRS_2044])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_DISABLED(diNum)      \
  ( 0u == ((fiParam_sIParam.diParam[diNum].u8Properties) & 0x01u) )

/*------------------------------------------------------------------------------
**  Macros to get the single/dual channel configuration (see [SRS_234], [SRS_2044])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_CH_PARAM(diNum)      ( (fiParam_sIParam.diParam[diNum].u8Properties) & 0x02u)
#define FIPARAM_DI_DUAL_CH(diNum)       ( 0u != FIPARAM_DI_CH_PARAM(diNum) )

/*------------------------------------------------------------------------------
**  Macros to get the DI type configuration (see [SRS_54], [SRS_2044]):
**  - semiconductor or active input
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_TYPE_PARAM(diNum)           \
  ((UINT8)(fiParam_sIParam.diParam[diNum].u8Properties) & 0x04u)
#define FIPARAM_DI_INPUTTYPE_CONTACT(diNum)    \
  (0u != FIPARAM_DI_TYPE_PARAM(DICFG_GET_DUAL_CH_NUM(diNum)))

/*------------------------------------------------------------------------------
**  Macros to get the TO behavior (see [SRS_277], [SRS_2044]):
**  - different time for TO (one of 6 possible values)
**  - TO always LOW
**  - TO always HIGH
**  Hint: Currently, there is only one valid TO parameter, which is the first
**        one (because there is only one TO line per channel), therefore always
**        the array member "0u" is read out
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_TO_TEST_PARAM   \
  (((UINT8)(fiParam_sIParam.diParam[0u].u8Properties) & 0x70u) >> 4u)

/*------------------------------------------------------------------------------
**  Macro returning the debounce filter value for high-level Input filter
**  (see [SRS_101], [SRS_2044]).
**  All safe DIs are filtered in two steps:
**  First by a "XooY", which is defined during compiletime.
**  Second by a high-level filter (configurable here) which defines how many
**  times the "XooY" must result in the same value.
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_DEBFILTER(diNum)  ((UINT8)(fiParam_sIParam.diParam[diNum].u8DebFilter))

/*------------------------------------------------------------------------------
**  Macros to get the reset type configuration (see [SRS_2220], [SRS_2044])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_RESET_TYPE_PARAM(diNum)     \
  ((UINT8)(fiParam_sIParam.diParam[diNum].u8Properties) & 0x08u)
#define FIPARAM_DI_AUTOMATIC_RESET(diNum)      \
  (0u != FIPARAM_DI_RESET_TYPE_PARAM(DICFG_GET_DUAL_CH_NUM(diNum)))
/* The automatic reset feature is enabled/disabled globally by setting the corresponding bit 
** inside CH1/2 see [SRS_2221] */
#define FIPARAM_DI_AUTOMATIC_RESET_ENABLED     FIPARAM_DI_AUTOMATIC_RESET(0u)

/*------------------------------------------------------------------------------
** Macro returning the Latch Input Error Time 
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_LATCH_ERROR_TIME            ((UINT16)(fiParam_sIParam.u16_latchInputErrTime))


/*------------------------------------------------------------------------------
** Macro returning the SafeBound per DI enable
**------------------------------------------------------------------------------
*/
#define FIPARAM_DI_SAFEBOUND_EN_PARAM(diNum)     \
        (((fiParam_sIParam.u8_safeBoundPerDiEnable) & (UINT8)(1u << (diNum))) & 0x3Fu)

#define FIPARAM_DI_SAFEBOUND_ENABLE(diNum)      \
        ((0u != FIPARAM_DI_SAFEBOUND_EN_PARAM(diNum)) \
          && (!FIPARAM_DI_DISABLED(DICFG_GET_DUAL_CH_NUM(diNum))))


/*******************************************************************************
********************************************************************************
**
** MACROS FOR DOs
**
********************************************************************************
*******************************************************************************/

/*------------------------------------------------------------------------------
** Macro returning if DO is disabled or not (see [SRS_634], [SRS_2045])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DO_DISABLED(doNum)     \
  (0u == ( (fiParam_sIParam.doParam[DOCFG_GET_DUAL_CH_NUM(doNum)].u8Properties) & 0x01u) )

/*------------------------------------------------------------------------------
** Macro returning if DO is in dual or single channel configuration (see [SRS_634],
** [SRS_2045])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DO_CH(doNum)           \
  ((UINT8)(fiParam_sIParam.doParam[doNum].u8Properties) & 0x02u)
#define FIPARAM_DO_DUAL_CH(doNum)      \
  (0u != FIPARAM_DO_CH(DOCFG_GET_DUAL_CH_NUM(doNum)))

/*------------------------------------------------------------------------------
** Macro returning the offset for DO test (see [SRS_636], [SRS_2045])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DO_TEST_OFFSET(doNum)  \
  ((UINT8)((fiParam_sIParam.doParam[DOCFG_GET_DUAL_CH_NUM(doNum)].u8Properties) & 0xF0u) >> 4u)

/*------------------------------------------------------------------------------
**  Macros to get the reset type configuration (see [SRS_2222], [SRS_2045])
**------------------------------------------------------------------------------
*/
#define FIPARAM_DO_RESET_TYPE_PARAM(doNum)     \
  ((UINT8)(fiParam_sIParam.doParam[doNum].u8Properties) & 0x04u)
#define FIPARAM_DO_AUTOMATIC_RESET(doNum)      \
  (0u != FIPARAM_DO_RESET_TYPE_PARAM(DOCFG_GET_DUAL_CH_NUM(doNum)))
/* The automatic reset feature is enabled/disabled globally by setting the 
** corresponding bit inside CH1/2 */
#define FIPARAM_DO_AUTOMATIC_RESET_ENABLED     FIPARAM_DO_AUTOMATIC_RESET(0u)

/*------------------------------------------------------------------------------
** Macro returning the Latch Output Error Time 
**------------------------------------------------------------------------------
*/
#define FIPARAM_DO_LATCH_ERROR_TIME            ((UINT16)(fiParam_sIParam.u16_latchOutputErrTime))


/*------------------------------------------------------------------------------
** Macro returning the SS1-t Delay Time
**------------------------------------------------------------------------------
*/
/* macro parameter doNum ignored as this setting exists only once for all DOs. */
#define FIPARAM_DO_SS1T_DELAY_4MS(doNum)         ((UINT16)fiParam_sIParam.u16_ss1tDelay4ms)


#endif  /* inclusion lock */

/*******************************************************************************
**
** End of fiParam.h
**
********************************************************************************
*/



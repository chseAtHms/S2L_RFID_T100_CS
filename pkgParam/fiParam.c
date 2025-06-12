/*******************************************************************************
********************************************************************************
**
** File Name
** ---------
**
** fiParam.c
**
** $Id: fiParam.c 4440 2024-05-29 12:03:45Z ankr $
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
** This module handles/hold the 'IO Configuration Data' used to configure the 
** GPIO module. Since the module is adopted from the T100/PS project,
** the PROFIsafe names for the IO Configuration Data (iParameters) is kept in 
** function names, variables, comments etc.
** Data comparable with the F-Parameters in T100/PS does not exist any more, 
** so these functions, variables have been removed from the module.
** 
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

/* system includes */
#include "xtypes.h"
#include "xdefs.h"
#include "version.h"

#include "globFail_Errorcodes.h"
#include "globFail_SafetyHandler.h"
#include "globFit_FitTestHandler.h"

#include "stdlib-hal.h"

#include "gpio_cfg.h"

#include "fiParam.h"
#include "fiParam_prv.h"

/*******************************************************************************
**
** Public Globals
**
********************************************************************************
*/

/* Unique file id used to build additional info */
#define k_FILEID      16u

/*------------------------------------------------------------------------------
**  iParameters (for DI/DO configuration)
**  This struct is filled, when parameters are received, global used due to
**  performance reasons
**------------------------------------------------------------------------------
*/
/*lint -esym(843, fiParam_sIParam) *//* Lint message deactivated because the
 * variable must be in the RAM and not in Flash because there is a write access
 * to this variable over a pointer. */
volatile PARAMETER_IPAR_STRUCT fiParam_sIParam =
{
   3u,     /* Fixed: Number of DIs */
   1u,     /* Fixed: Number of DOs */
   {
      {0u, 0u, 0u},
      {0u, 0u, 0u},
      {0u, 0u, 0u}
   },
   {
      {0u}
   },
   1000u, /* Default: 1000ms (see [SRS_2109]) */
   1000u, /* Default: 1000ms (see [SRS_2109]) */
   0u,    /* Default: 0ms - this means that Safe State Delay is off. (see [SRS_909]) */
   0x00u, /* Default: 0x00 - this means that SafeBound is off for all channels. (see [SRS_902]) */
   0x00u, /* pad byte */

};

/*! Variable to indicate that the F- and I-Parameters can be checked by the
 * Safety-Container. */
STATIC TRUE_FALSE_ENUM fiParam_eSafConCheckActive = eFALSE;

/* Active SCCRC */
STATIC volatile UINT32 fiParam_u32_Sccrc = 0U;

/*******************************************************************************
**
** Private Globals
**
********************************************************************************
*/

/** kau32_Crc32Table:
    CRC32 (CRC-S4)
    polynomial 0xedb88320 = standard CRC-32 bit reversed from the regular
    published polynomial of 0x04c11db7. This table should be used to
    replicate standard Ethernet CRC hardware algorithm. Thus the software
    implementation of the Ethernet CRC should produce the same result as the
    hardware. This table was generated via right shifting the LSB of each
    byte
*/
STATIC CONST UINT32 kau32_Crc32Table[256] =
{
  0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU,
  0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
  0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U,
  0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
  0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,
  0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
  0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU,
  0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
  0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U,
  0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
  0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U,
  0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
  0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U,
  0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
  0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,
  0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
  0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU,
  0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
  0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U,
  0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
  0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU,
  0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
  0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU,
  0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
  0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,
  0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
  0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U,
  0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
  0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U,
  0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
  0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U,
  0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
  0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU,
  0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
  0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,
  0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
  0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU,
  0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
  0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU,
  0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
  0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U,
  0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
  0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U,
  0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
  0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,
  0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
  0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U,
  0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
  0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU,
  0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
  0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U,
  0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
  0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU,
  0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
  0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,
  0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
  0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U,
  0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
  0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U,
  0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
  0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U,
  0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
  0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U,
  0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU
};


/* Temporary I-Parameter struct to be written from background (Type 1 SafetyOpen
   reception) until copied to active I-Parameters in the IRQ-scheduler (ASM
   state machine - Process Config) */
STATIC PARAMETER_IPAR_STRUCT sIParamCandidate =
{
   0u,
   0u,
   {
      {0u, 0u, 0u},
      {0u, 0u, 0u},
      {0u, 0u, 0u}
   },
   {
      {0u}
   },
   0u,
   0u,
   0u,
   0x00u,
   0x00u,
};

/* Sccrc calculated over sIParamCandidate (zero means currently no candidate
   I-Parameters stored) */
STATIC UINT32 u32CandidateSccrc = 0x00000000UL;


/*******************************************************************************
**
** Public Services
**
********************************************************************************
*/

/*------------------------------------------------------------------------------
**
** fiParam_setSafConCheckActive()
**
** Description:
** This function sets the flag fiParam_eSafConCheckActive to the parameter value.
** With this the Safety Container with the Parameter check is active / inactive.
**
**------------------------------------------------------------------------------
** Inputs:
**    eSafConAct: Determines if the Safety Container with the Parameter check
**                shall be active (eTRUE) or inactive (eFALSE).
**                (valid values: eTRUE, eFALSE, checked)
**
** Register Setting:
**    - None -
**
** Return:
**    - void -
**
** Remarks:
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void fiParam_setSafConCheckActive (TRUE_FALSE_ENUM eSafConAct)
{
   switch (eSafConAct)
   {
      case eTRUE:
      case eFALSE:
      {
         fiParam_eSafConCheckActive = eSafConAct;
         break;
      }
      default:
      {
         /* invalid values of parameter! */
         GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(1u));
         break;
      }
   }
}

/*------------------------------------------------------------------------------
**
** fiParam_setIParam()
**
** Description:
**    function to write the I-Parameter. I-Parameter are received via AIC
**    Communication interface split in several parts/messages but this function is 
**    only called if the I-Parameters are received completely.
**    Total size of I-Parameters depends on the number of safe IOs, in how many parts
**    these parameters are transmitted is defined in the message description.
**    The given size is checked, so that no buffer overrun can occur.
**    If the parameters are invalid the Safety Handler is called.
**
**------------------------------------------------------------------------------
** Inputs:
**    pu8Src:     pointer to the data
**                (valid range: <>NULL, not checked, only called with reference to structure)
**    u8Quantity: number of bytes to write
**                (valid range: FIPARAM_SIZEOF_IPAR_STRUCT, checked)
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Remarks:
**    Context: Background Task
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void fiParam_setIParam(CONST UINT8* pu8Src, CONST UINT8 u8Quantity)
{
  volatile UINT8* pu8TmpPoint;
  UINT8 i;

  /* parameter check: if u8Quantity lead to overrun of Parameters, then parameter error */
  if ( (UINT16)u8Quantity != FIPARAM_SIZEOF_IPAR_STRUCT )
  {
     /* invalid values of parameters! */
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(2u));
  }
  /* else: parameters are fine */
  else
  {
    __disable_irq();
    /*lint -esym(960, 17.1) -esym(960, 17.4) */
    /* set pointer to location inside the Parameter Struct */
    /* lint e928 disabled because casting is OK  */
    pu8TmpPoint = (volatile UINT8*)(&fiParam_sIParam); /*lint !e928*/
  
    for (i = 0u; i < u8Quantity; i++)
    {
       /* copy byte wise to Parameter Struct */
       *(pu8TmpPoint+i) = *(pu8Src+i);
    }
    
    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    __enable_irq();
  }
}

/*------------------------------------------------------------------------------
**
** fiParam_CalcFiParamSafeConCrc()
**
** Description:
** This function is called from the Safety Container module to calculate the
** checksum over the safety relevant data of the fiParam module (see [SRS_2050]).
** If there is a mismatch between the calculated and the expected CRC (SCCRC),
** the Safety Handler is called.
**
**------------------------------------------------------------------------------
** Inputs:
**    - None -
**
** Register Setting:
**    - None -
**
** Return:
**    - None -
**
** Remarks:
**    Context: main (during initialization)
**    Context: IRQ Scheduler
**------------------------------------------------------------------------------
*/
void fiParam_CalcFiParamSafeConCrc(void)
{

   if (eTRUE == fiParam_eSafConCheckActive)
   {
      UINT32 u32Crc;
  
      /* This FIT manipulates the IO Configuration. */
      GLOBFIT_FITTEST(GLOBFIT_CMD_IPAR_CRC, fiParam_sIParam.numOfDualChDIs ^= 0x08);
      
      /* calculate checksum of iParameters (cyclic check as "safety container", see [SRS_464]) */
      /* The SW major revision and the Hardware ID are 
      ** considered in fiParam_SccrcCompute directly, see [SRS_2041] */
      /* lint e928 disabled because casting is OK to get byte array for CRC routine */  
      u32Crc = fiParam_SccrcCompute((const volatile UINT8*)&fiParam_sIParam, /*lint !e928*/
                                    FIPARAM_SIZEOF_IPAR_STRUCT); 
      /* if CRC is correct */
      if (u32Crc == fiParam_u32_Sccrc)
      {
        /* nothing to do, CRC check OK */
      }
      /* else: CRC not correct */
      else
      {
        /* wrong result */
        GLOBFAIL_SAFETY_HANDLER(GlOB_FAILCODE_IPAR_CRC_ERR, GLOBFAIL_ADDINFO_FILE(3u));  
      }
   }
   else if (eFALSE == fiParam_eSafConCheckActive)
   {
      /* nothing to do */
   }
   else
   {
      GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_VARIABLE_ERR_EX, GLOBFAIL_ADDINFO_FILE(4u));
   }
}

/*------------------------------------------------------------------------------
**
** fiParam_GetConsFilter()
**
** Description:
**    returns the parametrization value of the consistency filter, see [SRS_609]
**
**------------------------------------------------------------------------------
** Inputs:
**    u8DualDiNum: Number of Dual-Channel-DI
**                 (valid range: 0..(GPIOCFG_NUM_DI_PER_uC-1), checked)
**
** Register Setting:
**    - None -
**
** Return:
**    UINT16: value of the consistency filter (parameter)
**
** Remarks:
**    Context: IRQ Scheduler
**    Context: main, while(FOREVER)
**
**------------------------------------------------------------------------------
*/
UINT16 fiParam_GetConsFilter (CONST UINT8 u8DualDiNum)
{
   UINT16 u16Tmp;

  /* check if DI number is valid */
   if (u8DualDiNum < GPIOCFG_NUM_DI_PER_uC)
   {
     /* get consistency filter time from iParameter, see [SRS_2044] */
     /* 'fiParam_sIParam' is only written via function fiParam_setIParam() from Background Task.
     ** So reading from 'IRQ Scheduler' and 'main, while(FOREVER)' is OK, means no IRQ disabling 
     ** necessary */
     u16Tmp = ((UINT16)fiParam_sIParam.diParam[u8DualDiNum].u16ConsFilter);
    
     u16Tmp -= (u16Tmp >> 7u); /* reduce ConsFilter by about 0.8% (Oscillator) */
   }
   /* else invalid parameter */
   else
   {
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(5u));
     /* set return value (only used for unit tests) */
     u16Tmp = 0u;
   }

   return (u16Tmp);
}

/***************************************************************************************************
  Function:
    fiParam_setIParCrc

  Description:
    This function sets the SCCRC in the parameter module. This means, the local IO
    Configuration Data is cyclically verified against this CRC if 'fiParam_eSafConCheckActive'
    is set to eTRUE.

  See also:
    -

  Parameters:
    u32_sccrc (IN)     - CRC32 which should be used
                         (valid range: any, not checked)

  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void fiParam_setIParCrc(UINT32 u32_sccrc)
{
  __disable_irq();
  /* store SCCRC */
  fiParam_u32_Sccrc = u32_sccrc;
  __enable_irq();
}

/***************************************************************************************************
  Function:
    fiParam_SccrcCompute

  Description:
    This function is used to calculate the SCCRC (CRC-S4) of the passed data.
    The CRC calculation includes:
      - SW major revision
      - Hardware ID
      - IO Configuration Data

  See also:
    -

  Parameters:
    pu8_start (IN)           - Address of source buffer for the CRC calculation.
                               (valid range: <>NULL, not checked, only called with reference)
    u16_len (IN)             - Length of source buffer
                               (valid range: any, not checked)

  Return value:
    UINT32   - Calculated SCCRC

  Remarks:
    Context: IRQ Scheduler
    Context: Background Task

***************************************************************************************************/
UINT32 fiParam_SccrcCompute(const volatile UINT8 *pu8_start, UINT16 u16_len)
{
  UINT32 u32Crc = FIPARAM_k_CRC_SEED_VALUE; /* starting seed */

  UINT8   u8SwMajorVersion = (UINT8)VERSION_SW_VERSION_MAJOR;
  UINT16  u16HwId = (UINT16)VERSION_ASM_MODULE_ID;
 
  /** ATTENTION: We don't take the SW Major Revision and Hardware ID received from host via
  ** 'SetConfigString'. Instead of that, we take the local values for the CRC check here.
  ** A mismatch of SW Major Revision and Hardware ID lead to a mismatch here.
  ** The SW Major Revision and Hardware ID are transferred via 'SetConfigString' because we
  ** don't want consider 'implicit' numbers inside the CRC calculation (done by host/PLC). 
  ** In fact they are not necessary.
  */
  
  /* Start calculation of SCCRC (CRC-S4) from VOL_5 (see [SRS_2040], [SRS_2119]) for the
  ** transferred data
  */
  /* Step 1: Start CRC with SW major version, see [SRS_2041] */
  u32Crc = CrcS4compute(&u8SwMajorVersion, FIPARAM_SIZEOF_SW_MAJOR_VER, u32Crc);
  /* Step 2: take now HW-ID into CRC, see [SRS_2041] */
  /* set pointer to location of Hardware ID */
  /* lint e928 disabled because casting is OK  */
  u32Crc = CrcS4compute((UINT8*)&u16HwId, FIPARAM_SIZEOF_HW_ID, u32Crc); /*lint !e928 */
  /* Step 3: the io configuration under test (see [SRS_2041], [SRS_2039])*/
  u32Crc = CrcS4compute(pu8_start, u16_len, u32Crc);
  
  /* return the result */
  return u32Crc;
}

/***************************************************************************************************
  Function:
    fiParam_setCandidateIParam

  Description:
    This function stores temporarily the I-Parameters that will later be set as the active
    I-Parameters.
    The given size is checked, so that no buffer overrun can occur.
    If the parameters are invalid the Safety Handler is called.

  See also:
    -

  Parameters:
    pu8Src  (IN)    - pointer to the data
                      (valid range: <>NULL, not checked, only called with reference to structure)
    u8Quantity (IN) - number of bytes to write
                      (valid range: FIPARAM_SIZEOF_IPAR_STRUCT, checked)
    u32sccrc (IN)   - SCCRC of the I-Parameters
                      (valid range: any, not checked)
  Return value:
    -

  Remarks:
    Context: Background Task

***************************************************************************************************/
void fiParam_setCandidateIParam(CONST UINT8* pu8Src, CONST UINT8 u8Quantity,
                                CONST UINT32 u32sccrc)
{
  /* parameter check: if u8Quantity lead to overrun of Parameters, then parameter error */
  if ( (UINT16)u8Quantity != FIPARAM_SIZEOF_IPAR_STRUCT )
  {
     /* invalid values of parameters! */
     GLOBFAIL_SAFETY_HANDLER(GLOB_FAILCODE_INVALID_PARAM_EX, GLOBFAIL_ADDINFO_FILE(6u));
  }
  /* else: parameters are fine */
  else
  {
    __disable_irq();
    /*lint -esym(960, 17.1) -esym(960, 17.4) */

    /* store the candidate i-Parameter SCCRC */
    u32CandidateSccrc = u32sccrc;
    /* store the candidate i-Parameter data */
    /* lint e928 disabled because casting is OK  */
    stdlibHAL_ByteArrCopy((UINT8*)&sIParamCandidate, pu8Src, (UINT16)u8Quantity); /*lint !e928*/

    /*lint +esym(960, 17.1) +esym(960, 17.4) */
    __enable_irq();
  }
}

/***************************************************************************************************
  Function:
    fiParam_getCandidateIParam

  Description:
    This function returns the temporarily stored I-Parameters and the SCCRC that belongs to it.

  See also:
    -

  Parameters:
    pu32Sccrc (OUT) - pointer to variable with returned SCCRC value
                      (valid range: <>NULL, not checked, only called with reference)

  Return value:
    pointer to candidate I-Parameters structure

  Remarks:
    Context: IRQ Scheduler (only in aicSm_Statemachine() in AIC state AICSM_AIC_PROCESS_CONFIG)
    Context: Background Task (only in SAPL_IxsceCfgApplyClbk())
    => concurrent access to this function is not possible (ruled out by SAPL_BkgdTaskCheck())

***************************************************************************************************/
UINT8* fiParam_getCandidateIParam(UINT32 *pu32Sccrc)
{
  /* return value of temporary stored candidate data */
  *pu32Sccrc = u32CandidateSccrc;
  /* lint e928 disabled because casting is OK  */
  return (UINT8*)(&sIParamCandidate); /*lint !e928*/
}

/***************************************************************************************************
  Function:
    fiParam_checkCandidateIParam

  Description:
    This function calculates the SCCRC over the temporary candidate IParameters and checks against
    the stored temporary candidate SCCRC.

  See also:
    -

  Parameters:
    -

  Return value:
    TRUE  - if the SCCRC is correct
    FALSE - if the SCCRC is not correct

  Remarks:
    Context: IRQ Scheduler (only in aicSm_Statemachine() in AIC state AICSM_AIC_PROCESS_CONFIG)
    Context: Background Task (only in SAPL_IxsceCfgApplyClbk())
    => concurrent access to this function is not possible (ruled out by SAPL_BkgdTaskCheck())

***************************************************************************************************/
BOOL fiParam_checkCandidateIParam(void)
{
  BOOL bResult = FALSE;
  /* lint e928 disabled because casting is OK  */
  UINT32 u32Sccrc = fiParam_SccrcCompute((UINT8*)(&sIParamCandidate), /*lint !e928*/
                                          (UINT16)sizeof(sIParamCandidate));

  if (u32Sccrc == u32CandidateSccrc)
  {
    bResult = TRUE;
  }
  else
  {
    /* return value already set */
  }

  return bResult;
}

/***************************************************************************************************
  Function:
    fiParam_clearCandidateIParam

  Description:
    This function clears the temporary candidate IParameters and SCCRC by setting them to zero.

  See also:
    -

  Parameters:
    -

  Return value:
    -

  Remarks:
    Context: IRQ Scheduler (only in aicSm_Statemachine() in AIC state AICSM_AIC_PROCESS_CONFIG)

***************************************************************************************************/
void fiParam_clearCandidateIParam(void)
{
  UINT8* pu8TmpPoint;
  UINT8 i;

  u32CandidateSccrc = 0x00000000UL;

  /*lint -esym(960, 17.1) -esym(960, 17.4) */

  /* set pointer to location inside the Parameter Struct */
  /* lint e928 disabled because casting is OK  */
  pu8TmpPoint = (UINT8*)(&sIParamCandidate); /*lint !e928*/

  for (i = 0u; i < sizeof(sIParamCandidate); i++)
  {
      /* copy byte wise to Parameter Struct */
      *(pu8TmpPoint+i) = 0u;
  }

  /*lint +esym(960, 17.1) +esym(960, 17.4) */
}


/*******************************************************************************
**
** Private Services
**
********************************************************************************
*/

/***************************************************************************************************
  Function:
    CrcS4compute

  Description:
    This function is based on example code provided in the CIP Networks Library Volume 5 
    Appendix E-4, (see [SRS_2040], [SRS_2119]).
    CRC32 calculation routine for polynomial 0xedb88320 which is bit reversed from the regular 
    polynomial of 0x04c11db7 thus the (crc >> 8) below is right shifted.
    Doing incremental crc calculation is done with providing the result from the previous block 
    with preset.

  See also:
    -

  Parameters:
    pu8_start  (IN) - Starting address to compute CRC over
                      (not checked, only called with references to arrays (sometimes with offset)
    u16_len    (IN) - Number of bytes
                      (not checked, any value allowed)
    u32_preset (IN) - Preset value for CRC
                      (not checked, any value allowed)

  Return value:
    UINT32          - Result of CRC calculation

  Remarks:
    Context: IRQ Scheduler
    Context: Background Task

***************************************************************************************************/
STATIC UINT32 CrcS4compute(const volatile UINT8 *pu8_start,
                           UINT16 u16_len,
                           UINT32 u32_preset)
{
  UINT16 i;
  UINT32 u32_crc = u32_preset;
  
  /*lint -esym(960, 17.4)*/
  /* Note 960: Violates MISRA 2004 Required Rule 17.4, pointer arithmetic
   * other than array indexing used
   * --> Deactivate Misra Rule 17.4, Array-Indexing is wanted. */
  for ( i = 0U; i < u16_len; i++) 
  {
    u32_crc =  kau32_Crc32Table[(u32_crc ^ pu8_start[i]) & 0xffU] ^ (u32_crc >> 8);
  }
  /*lint +esym(960, 17.4)*/
  
  return u32_crc;
}


/*******************************************************************************
**
** End of fiParam.c
**
********************************************************************************
*/

/***************************************************************************************************
**    Copyright (C) 2017 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: ipcx_ExchangeData-sys_def.h
**     Summary: Header file with definitions of the IPCX module.
**              This header file contains various definitions of the IPCX module.
**   $Revision: 2769 $
**       $Date: 2017-04-20 08:52:47 +0200 (Do, 20 Apr 2017) $
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

#ifndef IPCX_EXCHANGEDATA_SYS_DEF_H
#define IPCX_EXCHANGEDATA_SYS_DEF_H

/***************************************************************************************************
**    constants and macros
***************************************************************************************************/

/*! Initialization value for the CRC8 of the IPC. */
#define IPCXSYS_CRC_INITIAL_VALUE   ((UINT8) 0xFFu)

/*! Various constants for the different sizes for the IPC telegram. */
/*!< Size of the ID (see [SRS_2169]) */
#define IPCXSYS_SIZEOF_ID           ( ( UINT8 ) 1u )                 
/*!< Size of the IPC-Tgm-Header, the header consists of the ID. */
#define IPCXSYS_SIZEOF_HEADER       IPCXSYS_SIZEOF_ID                
/*!< Maximal size of the payload of the telegram. (see [SRS_2169]) */
#ifdef RFID_ACTIVE
#define IPCXSYS_SIZEOF_MAX_PAYLOAD  ( ( UINT8 ) 12u + 4u)
#else
#define IPCXSYS_SIZEOF_MAX_PAYLOAD  ( ( UINT8 ) 12u )                
#endif               
/*!< Size of the CRC (see [SRS_2169]). */
#define IPCXSYS_SIZEOF_CRC          ( ( UINT8 ) 1u )                 
/*!< Size of the Footer, it consists of the CRC. */
#define IPCXSYS_SIZEOF_FOOTER       IPCXSYS_SIZEOF_CRC               
/*!< Max. Size of the IPC telegram. (see [SRS_2169]) */
#define IPCXSYS_SIZEOF_MAX_TGM      (UINT32)( IPCXSYS_SIZEOF_ID + \
                                              IPCXSYS_SIZEOF_MAX_PAYLOAD + \
                                              IPCXSYS_SIZEOF_CRC )   

/* Macro is used to calculate the size of the data part of IPC telegram (header + payload) */
#define IPCXSYS_SIZEOF_DATA_VAR(x)  ( (UINT32)IPCXSYS_SIZEOF_HEADER + (x) )
/* Macro is used to calculate the size of the complete IPC telegram (header + payload + crc) */
#define IPCXSYS_SIZEOF_TGM_VAR(x)   ( (UINT32)IPCXSYS_SIZEOF_ID + (x) + IPCXSYS_SIZEOF_CRC )

/*! These constants defines the index of the ID and the payload in the IPC telegram. */
#define IPCXSYS_IDX_ID              ((UINT8) 0u)                           /*!< Index of the ID. */
#define IPCXSYS_IDX_PAYLOAD         ( IPCXSYS_IDX_ID   \
   + IPCXSYS_SIZEOF_ID)   /*!< Index of the payload. */

/*! This constants defines the mask for the ID handling of the IPC telegram. */
#define IPCXSYS_CONTROLLER_ID_MASK     ( ( UINT8 ) 0x80u ) /*!< Mask for the controller ID. */
#define IPCXSYS_IPC_ID_MASK            ( ( UINT8 ) 0x7Fu ) /*!< Mask for the IPC ID. */

/*! These constants defines the IPC ID (see [SRS_345]). If the IPC-IDs are not
 * enough, feel free to add more. */
/* Attention: Since the uC ID is also part of the transferred 'ID byte' (highest bit), the valid 
 * range of '0..IPCXSYS_IPC_ID_MASK' shall be considered for the IPC IDs. */

/* Common IPC IDs used by different safety protocols, shall not be changed */
#define IPCXSYS_IPC_ID_TEMP_MESS     ( ( UINT8 ) 0x03u )
#define IPCXSYS_IPC_ID_CONFIG        ( ( UINT8 ) 0x04u )  /* see [SRS_520], [SRS_280] */
#define IPCXSYS_IPC_ID_IO            ( ( UINT8 ) 0x05u )  /* see [SRS_671], [SRS_685], [SRS_686] */
#define IPCXSYS_IPC_ID_CLKCTRL       ( ( UINT8 ) 0x06u )

/* Protocol specific IPC IDs */
#define IPCXSYS_IPC_ID_APPCLKCTRL    ( ( UINT8 ) 0x07u )
#define IPCXSYS_IPC_ID_SAFETGM       ( ( UINT8 ) 0x08u )
#define IPCXSYS_IPC_ID_CSS_CTRL_INFO ( ( UINT8 ) 0x09u )
#define IPCXSYS_IPC_ID_NV_CRC        ( ( UINT8 ) 0x0Au )  /* see [SRS_2189] */
#define IPCXSYS_IPC_ID_FAILCODE_NUM  ( ( UINT8 ) 0x0Bu )  /* see [SRS_2230] */
#define IPCXSYS_IPC_ID_FAILCODE_VAL  ( ( UINT8 ) 0x0Cu )  /* see [SRS_2230] */

/*! Timeout for the IPC communication */
#define IPCXSYS_TIMEOUT              ( ( UINT32 ) 100u ) /*!< Timeout for the IPC in usec. */
#ifdef RFID_ACTIVE
  #define IPCXSYS_WAITTIMEOUT          ( ( UINT32 )  50u ) /*!< Timeout for the IPC in usec. */
#else
  #define IPCXSYS_WAITTIMEOUT          ( ( UINT32 )  30u ) /*!< Timeout for the IPC in usec. */
#endif

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

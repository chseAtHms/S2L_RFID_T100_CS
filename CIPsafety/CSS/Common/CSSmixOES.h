/*******************************************************************************
**    Copyright (C) 2009-2017 HMS Technology Center Ravensburg GmbH
********************************************************************************
**
**   Workfile: CSSmixOES.h
**    Summary: host to net copy macros for method CSS_H2N_CPY_MTD_ODD_EVEN_SORT
**             CSS macros for copying data from host to network considering
**             mixing data from both Safety Controllers.
**
**     Author: A. Kramer
**
********************************************************************************
********************************************************************************
**
**  Functions: -
**
********************************************************************************
**    Template Version 3
*******************************************************************************/

#ifndef CSS_MIX_EOS_H
#define CSS_MIX_EOS_H


/* make sure that the required headers have been included before this file */
#ifndef CSS_cfg_H2N_COPY_METHOD
  #error CSS_cfg_H2N_COPY_METHOD undefined!
#endif

#if (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_ODD_EVEN_SORT)


/*******************************************************************************
**    constants and macros
*******************************************************************************/

/* Mute lint warning because this macro is used for generating output frames
   and if position of an output byte is fixed then even/odd check has a constant
   result. */
/*lint -emacro(845, CSS_MIX_CPY8) left argument to operator is certain to .. */
/*******************************************************************************
**
** Function    : CSS_MIX_CPY8
**
** Description : This function copies the data of any 8 bit type from host
**               (H) format to network (N) format.
**
**               **Attention**:
**               This macro must consider that the transmit telegrams must be
**               made up of data from Safety Controller 1 and Safety Controller
**               2. At no point in time a complete transmit safety telegram may
**               exist in the memory of just one Safety Controller!
**               Thus the parts of the telegram produced by this macro must be
**               combined with data from the other Safety Controller inside
**               HALCS (see Req.2.5.2.1-1).
**
** Parameters  : pv_dst (IN)  - destination pointer to the data in network (N)
**                              format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN)  - source pointer to the data in host (H) format
**                              (not checked), valid range: != CSS_k_NULL
**               u16_idx (IN) - offset where the data will be placed inside the
**                              buffer pv_dst (not checked),
**                              valid range: < u16_len
**               u16_len (IN) - length of the buffer pv_dst (number of bytes),
**                              (not checked), valid range: >= 1
**
** Returnvalue : -
**
*******************************************************************************/
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
#if (CSS_k_PLATFORM_FACTOR == 1)
  #define CSS_MIX_CPY8(pv_dst, pv_src, u16_idx, u16_len)                                                 \
    if ((u16_idx) & 0x0001U)                                                                             \
    {                                                                                                    \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*((CSS_t_USINT*)(pv_src)));  \
    }                                                                                                    \
    else                                                                                                 \
    {                                                                                                    \
      ((*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))     = (*((CSS_t_USINT*)(pv_src)))); \
    }
#elif (CSS_k_PLATFORM_FACTOR == 2)
  #define CSS_MIX_CPY8(pv_dst, pv_src, u16_idx, u16_len)                                                 \
    if ((u16_idx) & 0x0001U)                                                                             \
    {                                                                                                    \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*((CSS_t_USINT*)(pv_src)));  \
    }                                                                                                    \
    else                                                                                                 \
    {                                                                                                    \
      ((*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))     = (*((CSS_t_USINT*)(pv_src)))); \
    }

#endif
/* RSM_IGNORE_QUALITY_END */


/* Mute lint warning because this macro is used for generating output frames
   and if position of an output byte is fixed then even/odd check has a constant
   result. */
/*lint -emacro(845, CSS_MIX_CPY16) left argument to operator is certain to .. */
/*******************************************************************************
**
** Function    : CSS_MIX_CPY16
**
** Description : This function copies the data of any 16 bit type from host
**               (H) format to network (N) format.
**
**               **Attention**:
**               This macro must consider that the transmit telegrams must be
**               made up of data from Safety Controller 1 and Safety Controller
**               2. At no point in time a complete transmit safety telegram may
**               exist in the memory of just one Safety Controller!
**               Thus the parts of the telegram produced by this macro must be
**               combined with data from the other Safety Controller inside
**               HALCS (see Req.2.5.2.1-1).
**
** Parameters  : pv_dst (IN)  - destination pointer to the data in network (N)
**                              format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN)  - source pointer to the data in host (H) format
**                              (not checked), valid range: != CSS_k_NULL
**               u16_idx (IN) - offset where the data will be placed inside the
**                              buffer pv_dst (not checked),
**                              valid range: < u16_len
**               u16_len (IN) - length of the buffer pv_dst (number of bytes),
**                              (not checked), valid range: >= 2
**
** Returnvalue : -
**
*******************************************************************************/
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_MIX_CPY16(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))      = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
      }
  #else
    #define CSS_MIX_CPY16(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))      = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
      }
  #endif
#elif (CSS_k_PLATFORM_FACTOR == 2)
  #define CSS_MIX_CPY16(pv_dst, pv_src, u16_idx, u16_len)                                                        \
    if ((u16_idx) & 0x0001U)                                                                                     \
    {                                                                                                            \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*(CSS_t_UINT*)(pv_src)) & 0x00FFU);       \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (CSS_t_USINT)((*(CSS_t_UINT*)(pv_src)) >> 8U);   \
    }                                                                                                            \
    else                                                                                                         \
    {                                                                                                            \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+0U))) = (CSS_t_USINT)((*(CSS_t_UINT*)(pv_src)) & 0x00FFU);    \
      (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*(CSS_t_UINT*)(pv_src)) >> 8U); \
    }
#endif
/* RSM_IGNORE_QUALITY_END */


/*lint -esym(755, CSS_MIX_CPY32)                  not referenced in every cfg */
/* Mute lint warning because this macro is used for generating output frames
   and if position of an output byte is fixed then even/odd check has a constant
   result. */
/*lint -emacro(845, CSS_MIX_CPY32) left argument to operator is certain to .. */
/*******************************************************************************
**
** Function    : CSS_MIX_CPY32
**
** Description : This function copies the data of any 32 bit type from host
**               (H) format to network (N) format.
**
**               **Attention**:
**               This macro must consider that the transmit telegrams must be
**               made up of data from Safety Controller 1 and Safety Controller
**               2. At no point in time a complete transmit safety telegram may
**               exist in the memory of just one Safety Controller!
**               Thus the parts of the telegram produced by this macro must be
**               combined with data from the other Safety Controller inside
**               HALCS (see Req.2.5.2.1-1).
**
** Parameters  : pv_dst (IN)  - destination pointer to the data in network (N)
**                              format (not checked), valid range: != CSS_k_NULL
**               pv_src (IN)  - source pointer to the data in host (H) format
**                              (not checked), valid range: != CSS_k_NULL
**               u16_idx (IN) - offset where the data will be placed inside the
**                              buffer pv_dst (not checked),
**                              valid range: < u16_len
**               u16_len (IN) - length of the buffer pv_dst (number of bytes),
**                              (not checked), valid range: >= 4
**
** Returnvalue : -
**
*******************************************************************************/
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
#if (CSS_k_PLATFORM_FACTOR == 1)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_MIX_CPY32(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 3U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 2U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+2U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))      = (*(((CSS_t_USINT*)(pv_src)) + 3U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 2U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
      }
  #else
    #define CSS_MIX_CPY32(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (*(((CSS_t_USINT*)(pv_src)) + 2U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+2U))) = (*(((CSS_t_USINT*)(pv_src)) + 3U)); \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+(((u16_len)/2U)+((u16_idx)/2U))))      = (*(((CSS_t_USINT*)(pv_src)) + 0U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + 1U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (*(((CSS_t_USINT*)(pv_src)) + 2U)); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (*(((CSS_t_USINT*)(pv_src)) + 3U)); \
      }
  #endif
#elif (CSS_k_PLATFORM_FACTOR == 2)
  #if (CSS_k_ENDIAN == CSS_k_BIG_ENDIAN)
    #define CSS_MIX_CPY32(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU);    \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U);        \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U);     \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+0U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU);    \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U);        \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U);     \
      }
  #else
    #define CSS_MIX_CPY32(pv_dst, pv_src, u16_idx, u16_len)                                                      \
      if ((u16_idx) & 0x0001U)                                                                                   \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU);    \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U);        \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U);     \
      }                                                                                                          \
      else                                                                                                       \
      {                                                                                                          \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+0U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) & 0x00FFU);    \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+0U)) >> 8U);        \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((u16_idx)/2U))+1U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) & 0x00FFU); \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-((u16_idx)/2U))-2U))) = (CSS_t_USINT)((*((CSS_t_UINT*)((pv_src))+1U)) >> 8U);     \
      }
  #endif
#endif
/* RSM_IGNORE_QUALITY_END */


/*lint -esym(755, CSS_MIX_CPY)                    not referenced in every cfg */
/*******************************************************************************
**
** Function    : CSS_MIX_CPY
**
** Description : This function copies a byte stream from the host processor to
**               the network.
**
**               **Attention**:
**               This macro must consider that the transmit telegrams must be
**               made up of data from Safety Controller 1 and Safety Controller
**               2. At no point in time a complete transmit safety telegram may
**               exist in the memory of just one Safety Controller!
**               Thus the parts of the telegram produced by this macro must be
**               combined with data from the other Safety Controller inside
**               HALCS (see Req.2.5.2.1-1).
**
** Parameters  : pv_dst (IN)   - destination pointer (not checked),
**                               valid range: != CSS_k_NULL
**               pv_src (IN)   - source pointer (not checked),
**                               valid range: != CSS_k_NULL
**               u16_size (IN) - length of the data to be copied (not checked),
**                               valid range: any value allowed
**               u16_idx (IN)  - offset where the data will be placed inside the
**                               buffer pv_dst (not checked),
**                               valid range: < u16_len
**               u16_len (IN)  - length of the buffer pv_dst (number of bytes),
**                               (not checked), valid range: any value allowed
**
** Returnvalue : -
**
*******************************************************************************/
/* RSM_IGNORE_QUALITY_BEGIN  Notice #1 - Physical line length > 80 characters */
#define CSS_MIX_CPY(pv_dst, pv_src, u16_size, u16_idx, u16_len)                                                              \
  {                                                                                                                          \
    CSS_t_UINT u16_i;                                                                                                        \
    for (u16_i = 0U; u16_i < (u16_size); u16_i++)                                                                            \
    {                                                                                                                        \
      if (((u16_idx)+u16_i) & 0x0001U)                                                                                       \
      {                                                                                                                      \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)-(((u16_idx)+u16_i)/2U))-1U))) = (*(((CSS_t_USINT*)(pv_src)) + u16_i)); \
      }                                                                                                                      \
      else                                                                                                                   \
      {                                                                                                                      \
        (*(((CSS_t_USINT*)(pv_dst))+((((u16_len)/2U)+((((u16_idx)+u16_i))/2U))))) = (*(((CSS_t_USINT*)(pv_src)) + u16_i));  \
      }                                                                                                                      \
    }                                                                                                                        \
  }
/* RSM_IGNORE_QUALITY_END */


#else
  #error This file is usable only for copy method CSS_H2N_CPY_MTD_ODD_EVEN_SORT
#endif  /*  (CSS_cfg_H2N_COPY_METHOD == CSS_H2N_CPY_MTD_ODD_EVEN_SORT) */


/*******************************************************************************
**    data types
*******************************************************************************/


/*******************************************************************************
**    global variables
*******************************************************************************/


/*******************************************************************************
**    function prototypes
*******************************************************************************/

#endif /* #ifndef CSS_MIX_DIRECT_H */


/*** End Of File ***/


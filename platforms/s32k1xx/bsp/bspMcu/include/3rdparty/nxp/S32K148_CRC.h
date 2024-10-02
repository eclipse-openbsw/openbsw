/*
** ###################################################################
**     Processor:           S32K148_M4
**     Compiler:            Keil ARM C/C++ Compiler
**     Reference manual:    S32K1XX RM Rev.14
**     Version:             rev. 1.1a, 2024-05-21
**     Build:               b220202
**
**     Abstract:
**         Peripheral Access Layer for S32K148_M4
**
**     Copyright 1997-2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2024 NXP
**
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     1. Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     2. Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     3. Neither the name of the copyright holder nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/

/*!
 * @file S32K148_CRC.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_CRC
 *
 * This file contains register definitions and macros for easy access to their
 * bit fields.
 *
 * This file assumes LITTLE endian system.
 */

/**
* @page misra_violations MISRA-C:2012 violations
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.3, local typedef not referenced
* The SoC header defines typedef for all modules.
*
* @section [global]
* Violates MISRA 2012 Advisory Rule 2.5, local macro not referenced
* The SoC header defines macros for all modules and registers.
*
* @section [global]
* Violates MISRA 2012 Advisory Directive 4.9, Function-like macro
* These are generated macros used for accessing the bit-fields from registers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.1, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.2, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.4, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 5.5, identifier clash
* The supported compilers use more than 31 significant characters for identifiers.
*
* @section [global]
* Violates MISRA 2012 Required Rule 21.1, defined macro '__I' is reserved to the compiler
* This type qualifier is needed to ensure correct I/O access and addressing.
*/

/* Prevention from multiple including the same memory map */
#if !defined(S32K148_CRC_H_)  /* Check if memory map has not been already included */
#define S32K148_CRC_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- CRC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CRC_Peripheral_Access_Layer CRC Peripheral Access Layer
 * @{
 */

/** CRC - Register Layout Typedef */
typedef struct {
  union {                                          /* offset: 0x0 */
    struct {                                         /* offset: 0x0 */
      __IO uint8_t LL;                             /**< CRC_LL register, offset: 0x0 */
      __IO uint8_t LU;                             /**< CRC_LU register, offset: 0x1 */
      __IO uint8_t HL;                             /**< CRC_HL register, offset: 0x2 */
      __IO uint8_t HU;                             /**< CRC_HU register, offset: 0x3 */
    } DATA_8;
    struct {                                         /* offset: 0x0 */
      __IO uint16_t L;                             /**< CRC_L register, offset: 0x0 */
      __IO uint16_t H;                             /**< CRC_H register, offset: 0x2 */
    } DATA_16;
    __IO uint32_t DATA;                              /**< CRC Data register, offset: 0x0 */
  } DATAu;
  __IO uint32_t GPOLY;                             /**< CRC Polynomial register, offset: 0x4 */
  __IO uint32_t CTRL;                              /**< CRC Control register, offset: 0x8 */
} CRC_Type, *CRC_MemMapPtr;

/** Number of instances of the CRC module. */
#define CRC_INSTANCE_COUNT                       (1u)

/* CRC - Peripheral instance base addresses */
/** Peripheral CRC base address */
#define IP_CRC_BASE                              (0x40032000u)
/** Peripheral CRC base pointer */
#define IP_CRC                                   ((CRC_Type *)IP_CRC_BASE)
/** Array initializer of CRC peripheral base addresses */
#define IP_CRC_BASE_ADDRS                        { IP_CRC_BASE }
/** Array initializer of CRC peripheral base pointers */
#define IP_CRC_BASE_PTRS                         { IP_CRC }

/* ----------------------------------------------------------------------------
   -- CRC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup CRC_Register_Masks CRC Register Masks
 * @{
 */

/*! @name DATA_8_LL - CRC_LL register */
/*! @{ */
#define CRC_DATAu_DATA_8_LL_DATALL_MASK      (0xFFU)
#define CRC_DATAu_DATA_8_LL_DATALL_SHIFT     (0U)
#define CRC_DATAu_DATA_8_LL_DATALL_WIDTH     (8U)
#define CRC_DATAu_DATA_8_LL_DATALL(x)        (((uint8_t)(((uint8_t)(x)) << CRC_DATAu_DATA_8_LL_DATALL_SHIFT)) & CRC_DATAu_DATA_8_LL_DATALL_MASK)
/*! @} */

/*! @name DATA_8_LU - CRC_LU register */
/*! @{ */
#define CRC_DATAu_DATA_8_LU_DATALU_MASK      (0xFFU)
#define CRC_DATAu_DATA_8_LU_DATALU_SHIFT     (0U)
#define CRC_DATAu_DATA_8_LU_DATALU_WIDTH     (8U)
#define CRC_DATAu_DATA_8_LU_DATALU(x)        (((uint8_t)(((uint8_t)(x)) << CRC_DATAu_DATA_8_LU_DATALU_SHIFT)) & CRC_DATAu_DATA_8_LU_DATALU_MASK)
/*! @} */

/*! @name DATA_8_HL - CRC_HL register */
/*! @{ */
#define CRC_DATAu_DATA_8_HL_DATAHL_MASK      (0xFFU)
#define CRC_DATAu_DATA_8_HL_DATAHL_SHIFT     (0U)
#define CRC_DATAu_DATA_8_HL_DATAHL_WIDTH     (8U)
#define CRC_DATAu_DATA_8_HL_DATAHL(x)        (((uint8_t)(((uint8_t)(x)) << CRC_DATAu_DATA_8_HL_DATAHL_SHIFT)) & CRC_DATAu_DATA_8_HL_DATAHL_MASK)
/*! @} */

/*! @name DATA_8_HU - CRC_HU register */
/*! @{ */
#define CRC_DATAu_DATA_8_HU_DATAHU_MASK      (0xFFU)
#define CRC_DATAu_DATA_8_HU_DATAHU_SHIFT     (0U)
#define CRC_DATAu_DATA_8_HU_DATAHU_WIDTH     (8U)
#define CRC_DATAu_DATA_8_HU_DATAHU(x)        (((uint8_t)(((uint8_t)(x)) << CRC_DATAu_DATA_8_HU_DATAHU_SHIFT)) & CRC_DATAu_DATA_8_HU_DATAHU_MASK)
/*! @} */

/*! @name DATA_16_L - CRC_L register */
/*! @{ */
#define CRC_DATAu_DATA_16_L_DATAL_MASK       (0xFFFFU)
#define CRC_DATAu_DATA_16_L_DATAL_SHIFT      (0U)
#define CRC_DATAu_DATA_16_L_DATAL_WIDTH      (16U)
#define CRC_DATAu_DATA_16_L_DATAL(x)         (((uint16_t)(((uint16_t)(x)) << CRC_DATAu_DATA_16_L_DATAL_SHIFT)) & CRC_DATAu_DATA_16_L_DATAL_MASK)
/*! @} */

/*! @name DATA_16_H - CRC_H register */
/*! @{ */
#define CRC_DATAu_DATA_16_H_DATAH_MASK       (0xFFFFU)
#define CRC_DATAu_DATA_16_H_DATAH_SHIFT      (0U)
#define CRC_DATAu_DATA_16_H_DATAH_WIDTH      (16U)
#define CRC_DATAu_DATA_16_H_DATAH(x)         (((uint16_t)(((uint16_t)(x)) << CRC_DATAu_DATA_16_H_DATAH_SHIFT)) & CRC_DATAu_DATA_16_H_DATAH_MASK)
/*! @} */

/*! @name DATA - CRC Data register */
/*! @{ */

#define CRC_DATAu_DATA_LL_MASK                   (0xFFU)
#define CRC_DATAu_DATA_LL_SHIFT                  (0U)
#define CRC_DATAu_DATA_LL_WIDTH                  (8U)
#define CRC_DATAu_DATA_LL(x)                     (((uint32_t)(((uint32_t)(x)) << CRC_DATAu_DATA_LL_SHIFT)) & CRC_DATAu_DATA_LL_MASK)

#define CRC_DATAu_DATA_LU_MASK                   (0xFF00U)
#define CRC_DATAu_DATA_LU_SHIFT                  (8U)
#define CRC_DATAu_DATA_LU_WIDTH                  (8U)
#define CRC_DATAu_DATA_LU(x)                     (((uint32_t)(((uint32_t)(x)) << CRC_DATAu_DATA_LU_SHIFT)) & CRC_DATAu_DATA_LU_MASK)

#define CRC_DATAu_DATA_HL_MASK                   (0xFF0000U)
#define CRC_DATAu_DATA_HL_SHIFT                  (16U)
#define CRC_DATAu_DATA_HL_WIDTH                  (8U)
#define CRC_DATAu_DATA_HL(x)                     (((uint32_t)(((uint32_t)(x)) << CRC_DATAu_DATA_HL_SHIFT)) & CRC_DATAu_DATA_HL_MASK)

#define CRC_DATAu_DATA_HU_MASK                   (0xFF000000U)
#define CRC_DATAu_DATA_HU_SHIFT                  (24U)
#define CRC_DATAu_DATA_HU_WIDTH                  (8U)
#define CRC_DATAu_DATA_HU(x)                     (((uint32_t)(((uint32_t)(x)) << CRC_DATAu_DATA_HU_SHIFT)) & CRC_DATAu_DATA_HU_MASK)
/*! @} */

/*! @name GPOLY - CRC Polynomial register */
/*! @{ */

#define CRC_GPOLY_LOW_MASK                       (0xFFFFU)
#define CRC_GPOLY_LOW_SHIFT                      (0U)
#define CRC_GPOLY_LOW_WIDTH                      (16U)
#define CRC_GPOLY_LOW(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_GPOLY_LOW_SHIFT)) & CRC_GPOLY_LOW_MASK)

#define CRC_GPOLY_HIGH_MASK                      (0xFFFF0000U)
#define CRC_GPOLY_HIGH_SHIFT                     (16U)
#define CRC_GPOLY_HIGH_WIDTH                     (16U)
#define CRC_GPOLY_HIGH(x)                        (((uint32_t)(((uint32_t)(x)) << CRC_GPOLY_HIGH_SHIFT)) & CRC_GPOLY_HIGH_MASK)
/*! @} */

/*! @name CTRL - CRC Control register */
/*! @{ */

#define CRC_CTRL_TCRC_MASK                       (0x1000000U)
#define CRC_CTRL_TCRC_SHIFT                      (24U)
#define CRC_CTRL_TCRC_WIDTH                      (1U)
#define CRC_CTRL_TCRC(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_TCRC_SHIFT)) & CRC_CTRL_TCRC_MASK)

#define CRC_CTRL_WAS_MASK                        (0x2000000U)
#define CRC_CTRL_WAS_SHIFT                       (25U)
#define CRC_CTRL_WAS_WIDTH                       (1U)
#define CRC_CTRL_WAS(x)                          (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_WAS_SHIFT)) & CRC_CTRL_WAS_MASK)

#define CRC_CTRL_FXOR_MASK                       (0x4000000U)
#define CRC_CTRL_FXOR_SHIFT                      (26U)
#define CRC_CTRL_FXOR_WIDTH                      (1U)
#define CRC_CTRL_FXOR(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_FXOR_SHIFT)) & CRC_CTRL_FXOR_MASK)

#define CRC_CTRL_TOTR_MASK                       (0x30000000U)
#define CRC_CTRL_TOTR_SHIFT                      (28U)
#define CRC_CTRL_TOTR_WIDTH                      (2U)
#define CRC_CTRL_TOTR(x)                         (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_TOTR_SHIFT)) & CRC_CTRL_TOTR_MASK)

#define CRC_CTRL_TOT_MASK                        (0xC0000000U)
#define CRC_CTRL_TOT_SHIFT                       (30U)
#define CRC_CTRL_TOT_WIDTH                       (2U)
#define CRC_CTRL_TOT(x)                          (((uint32_t)(((uint32_t)(x)) << CRC_CTRL_TOT_SHIFT)) & CRC_CTRL_TOT_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group CRC_Register_Masks */

/*!
 * @}
 */ /* end of group CRC_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_CRC_H_) */

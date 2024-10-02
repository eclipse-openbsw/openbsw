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
 * @file S32K148_AIPS.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_AIPS
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
#if !defined(S32K148_AIPS_H_)  /* Check if memory map has not been already included */
#define S32K148_AIPS_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- AIPS Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AIPS_Peripheral_Access_Layer AIPS Peripheral Access Layer
 * @{
 */

/** AIPS - Size of Registers Arrays */
#define AIPS_OPACR_COUNT                          12u

/** AIPS - Register Layout Typedef */
typedef struct {
  __IO uint32_t MPRA;                              /**< Master Privilege Register A, offset: 0x0 */
  uint8_t RESERVED_0[28];
  __IO uint32_t PACRA;                             /**< Peripheral Access Control Register, offset: 0x20 */
  __IO uint32_t PACRB;                             /**< Peripheral Access Control Register, offset: 0x24 */
  uint8_t RESERVED_1[4];
  __IO uint32_t PACRD;                             /**< Peripheral Access Control Register, offset: 0x2C */
  uint8_t RESERVED_2[16];
  __IO uint32_t OPACR[AIPS_OPACR_COUNT];           /**< Off-Platform Peripheral Access Control Register, array offset: 0x40, array step: 0x4 */
} AIPS_Type, *AIPS_MemMapPtr;

/** Number of instances of the AIPS module. */
#define AIPS_INSTANCE_COUNT                      (1u)

/* AIPS - Peripheral instance base addresses */
/** Peripheral AIPS base address */
#define IP_AIPS_BASE                             (0x40000000u)
/** Peripheral AIPS base pointer */
#define IP_AIPS                                  ((AIPS_Type *)IP_AIPS_BASE)
/** Array initializer of AIPS peripheral base addresses */
#define IP_AIPS_BASE_ADDRS                       { IP_AIPS_BASE }
/** Array initializer of AIPS peripheral base pointers */
#define IP_AIPS_BASE_PTRS                        { IP_AIPS }

/* ----------------------------------------------------------------------------
   -- AIPS Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup AIPS_Register_Masks AIPS Register Masks
 * @{
 */

/*! @name MPRA - Master Privilege Register A */
/*! @{ */

#define AIPS_MPRA_MPL3_MASK                      (0x10000U)
#define AIPS_MPRA_MPL3_SHIFT                     (16U)
#define AIPS_MPRA_MPL3_WIDTH                     (1U)
#define AIPS_MPRA_MPL3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MPL3_SHIFT)) & AIPS_MPRA_MPL3_MASK)

#define AIPS_MPRA_MTW3_MASK                      (0x20000U)
#define AIPS_MPRA_MTW3_SHIFT                     (17U)
#define AIPS_MPRA_MTW3_WIDTH                     (1U)
#define AIPS_MPRA_MTW3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTW3_SHIFT)) & AIPS_MPRA_MTW3_MASK)

#define AIPS_MPRA_MTR3_MASK                      (0x40000U)
#define AIPS_MPRA_MTR3_SHIFT                     (18U)
#define AIPS_MPRA_MTR3_WIDTH                     (1U)
#define AIPS_MPRA_MTR3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTR3_SHIFT)) & AIPS_MPRA_MTR3_MASK)

#define AIPS_MPRA_MPL2_MASK                      (0x100000U)
#define AIPS_MPRA_MPL2_SHIFT                     (20U)
#define AIPS_MPRA_MPL2_WIDTH                     (1U)
#define AIPS_MPRA_MPL2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MPL2_SHIFT)) & AIPS_MPRA_MPL2_MASK)

#define AIPS_MPRA_MTW2_MASK                      (0x200000U)
#define AIPS_MPRA_MTW2_SHIFT                     (21U)
#define AIPS_MPRA_MTW2_WIDTH                     (1U)
#define AIPS_MPRA_MTW2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTW2_SHIFT)) & AIPS_MPRA_MTW2_MASK)

#define AIPS_MPRA_MTR2_MASK                      (0x400000U)
#define AIPS_MPRA_MTR2_SHIFT                     (22U)
#define AIPS_MPRA_MTR2_WIDTH                     (1U)
#define AIPS_MPRA_MTR2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTR2_SHIFT)) & AIPS_MPRA_MTR2_MASK)

#define AIPS_MPRA_MPL1_MASK                      (0x1000000U)
#define AIPS_MPRA_MPL1_SHIFT                     (24U)
#define AIPS_MPRA_MPL1_WIDTH                     (1U)
#define AIPS_MPRA_MPL1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MPL1_SHIFT)) & AIPS_MPRA_MPL1_MASK)

#define AIPS_MPRA_MTW1_MASK                      (0x2000000U)
#define AIPS_MPRA_MTW1_SHIFT                     (25U)
#define AIPS_MPRA_MTW1_WIDTH                     (1U)
#define AIPS_MPRA_MTW1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTW1_SHIFT)) & AIPS_MPRA_MTW1_MASK)

#define AIPS_MPRA_MTR1_MASK                      (0x4000000U)
#define AIPS_MPRA_MTR1_SHIFT                     (26U)
#define AIPS_MPRA_MTR1_WIDTH                     (1U)
#define AIPS_MPRA_MTR1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTR1_SHIFT)) & AIPS_MPRA_MTR1_MASK)

#define AIPS_MPRA_MPL0_MASK                      (0x10000000U)
#define AIPS_MPRA_MPL0_SHIFT                     (28U)
#define AIPS_MPRA_MPL0_WIDTH                     (1U)
#define AIPS_MPRA_MPL0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MPL0_SHIFT)) & AIPS_MPRA_MPL0_MASK)

#define AIPS_MPRA_MTW0_MASK                      (0x20000000U)
#define AIPS_MPRA_MTW0_SHIFT                     (29U)
#define AIPS_MPRA_MTW0_WIDTH                     (1U)
#define AIPS_MPRA_MTW0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTW0_SHIFT)) & AIPS_MPRA_MTW0_MASK)

#define AIPS_MPRA_MTR0_MASK                      (0x40000000U)
#define AIPS_MPRA_MTR0_SHIFT                     (30U)
#define AIPS_MPRA_MTR0_WIDTH                     (1U)
#define AIPS_MPRA_MTR0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_MPRA_MTR0_SHIFT)) & AIPS_MPRA_MTR0_MASK)
/*! @} */

/*! @name PACRA - Peripheral Access Control Register */
/*! @{ */

#define AIPS_PACRA_TP1_MASK                      (0x1000000U)
#define AIPS_PACRA_TP1_SHIFT                     (24U)
#define AIPS_PACRA_TP1_WIDTH                     (1U)
#define AIPS_PACRA_TP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_TP1_SHIFT)) & AIPS_PACRA_TP1_MASK)

#define AIPS_PACRA_WP1_MASK                      (0x2000000U)
#define AIPS_PACRA_WP1_SHIFT                     (25U)
#define AIPS_PACRA_WP1_WIDTH                     (1U)
#define AIPS_PACRA_WP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_WP1_SHIFT)) & AIPS_PACRA_WP1_MASK)

#define AIPS_PACRA_SP1_MASK                      (0x4000000U)
#define AIPS_PACRA_SP1_SHIFT                     (26U)
#define AIPS_PACRA_SP1_WIDTH                     (1U)
#define AIPS_PACRA_SP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_SP1_SHIFT)) & AIPS_PACRA_SP1_MASK)

#define AIPS_PACRA_TP0_MASK                      (0x10000000U)
#define AIPS_PACRA_TP0_SHIFT                     (28U)
#define AIPS_PACRA_TP0_WIDTH                     (1U)
#define AIPS_PACRA_TP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_TP0_SHIFT)) & AIPS_PACRA_TP0_MASK)

#define AIPS_PACRA_WP0_MASK                      (0x20000000U)
#define AIPS_PACRA_WP0_SHIFT                     (29U)
#define AIPS_PACRA_WP0_WIDTH                     (1U)
#define AIPS_PACRA_WP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_WP0_SHIFT)) & AIPS_PACRA_WP0_MASK)

#define AIPS_PACRA_SP0_MASK                      (0x40000000U)
#define AIPS_PACRA_SP0_SHIFT                     (30U)
#define AIPS_PACRA_SP0_WIDTH                     (1U)
#define AIPS_PACRA_SP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRA_SP0_SHIFT)) & AIPS_PACRA_SP0_MASK)
/*! @} */

/*! @name PACRB - Peripheral Access Control Register */
/*! @{ */

#define AIPS_PACRB_TP5_MASK                      (0x100U)
#define AIPS_PACRB_TP5_SHIFT                     (8U)
#define AIPS_PACRB_TP5_WIDTH                     (1U)
#define AIPS_PACRB_TP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_TP5_SHIFT)) & AIPS_PACRB_TP5_MASK)

#define AIPS_PACRB_WP5_MASK                      (0x200U)
#define AIPS_PACRB_WP5_SHIFT                     (9U)
#define AIPS_PACRB_WP5_WIDTH                     (1U)
#define AIPS_PACRB_WP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_WP5_SHIFT)) & AIPS_PACRB_WP5_MASK)

#define AIPS_PACRB_SP5_MASK                      (0x400U)
#define AIPS_PACRB_SP5_SHIFT                     (10U)
#define AIPS_PACRB_SP5_WIDTH                     (1U)
#define AIPS_PACRB_SP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_SP5_SHIFT)) & AIPS_PACRB_SP5_MASK)

#define AIPS_PACRB_TP1_MASK                      (0x1000000U)
#define AIPS_PACRB_TP1_SHIFT                     (24U)
#define AIPS_PACRB_TP1_WIDTH                     (1U)
#define AIPS_PACRB_TP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_TP1_SHIFT)) & AIPS_PACRB_TP1_MASK)

#define AIPS_PACRB_WP1_MASK                      (0x2000000U)
#define AIPS_PACRB_WP1_SHIFT                     (25U)
#define AIPS_PACRB_WP1_WIDTH                     (1U)
#define AIPS_PACRB_WP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_WP1_SHIFT)) & AIPS_PACRB_WP1_MASK)

#define AIPS_PACRB_SP1_MASK                      (0x4000000U)
#define AIPS_PACRB_SP1_SHIFT                     (26U)
#define AIPS_PACRB_SP1_WIDTH                     (1U)
#define AIPS_PACRB_SP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_SP1_SHIFT)) & AIPS_PACRB_SP1_MASK)

#define AIPS_PACRB_TP0_MASK                      (0x10000000U)
#define AIPS_PACRB_TP0_SHIFT                     (28U)
#define AIPS_PACRB_TP0_WIDTH                     (1U)
#define AIPS_PACRB_TP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_TP0_SHIFT)) & AIPS_PACRB_TP0_MASK)

#define AIPS_PACRB_WP0_MASK                      (0x20000000U)
#define AIPS_PACRB_WP0_SHIFT                     (29U)
#define AIPS_PACRB_WP0_WIDTH                     (1U)
#define AIPS_PACRB_WP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_WP0_SHIFT)) & AIPS_PACRB_WP0_MASK)

#define AIPS_PACRB_SP0_MASK                      (0x40000000U)
#define AIPS_PACRB_SP0_SHIFT                     (30U)
#define AIPS_PACRB_SP0_WIDTH                     (1U)
#define AIPS_PACRB_SP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRB_SP0_SHIFT)) & AIPS_PACRB_SP0_MASK)
/*! @} */

/*! @name PACRD - Peripheral Access Control Register */
/*! @{ */

#define AIPS_PACRD_TP1_MASK                      (0x1000000U)
#define AIPS_PACRD_TP1_SHIFT                     (24U)
#define AIPS_PACRD_TP1_WIDTH                     (1U)
#define AIPS_PACRD_TP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_TP1_SHIFT)) & AIPS_PACRD_TP1_MASK)

#define AIPS_PACRD_WP1_MASK                      (0x2000000U)
#define AIPS_PACRD_WP1_SHIFT                     (25U)
#define AIPS_PACRD_WP1_WIDTH                     (1U)
#define AIPS_PACRD_WP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_WP1_SHIFT)) & AIPS_PACRD_WP1_MASK)

#define AIPS_PACRD_SP1_MASK                      (0x4000000U)
#define AIPS_PACRD_SP1_SHIFT                     (26U)
#define AIPS_PACRD_SP1_WIDTH                     (1U)
#define AIPS_PACRD_SP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_SP1_SHIFT)) & AIPS_PACRD_SP1_MASK)

#define AIPS_PACRD_TP0_MASK                      (0x10000000U)
#define AIPS_PACRD_TP0_SHIFT                     (28U)
#define AIPS_PACRD_TP0_WIDTH                     (1U)
#define AIPS_PACRD_TP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_TP0_SHIFT)) & AIPS_PACRD_TP0_MASK)

#define AIPS_PACRD_WP0_MASK                      (0x20000000U)
#define AIPS_PACRD_WP0_SHIFT                     (29U)
#define AIPS_PACRD_WP0_WIDTH                     (1U)
#define AIPS_PACRD_WP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_WP0_SHIFT)) & AIPS_PACRD_WP0_MASK)

#define AIPS_PACRD_SP0_MASK                      (0x40000000U)
#define AIPS_PACRD_SP0_SHIFT                     (30U)
#define AIPS_PACRD_SP0_WIDTH                     (1U)
#define AIPS_PACRD_SP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_PACRD_SP0_SHIFT)) & AIPS_PACRD_SP0_MASK)
/*! @} */

/*! @name OPACR - Off-Platform Peripheral Access Control Register */
/*! @{ */

#define AIPS_OPACR_TP7_MASK                      (0x1U)
#define AIPS_OPACR_TP7_SHIFT                     (0U)
#define AIPS_OPACR_TP7_WIDTH                     (1U)
#define AIPS_OPACR_TP7(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP7_SHIFT)) & AIPS_OPACR_TP7_MASK)

#define AIPS_OPACR_WP7_MASK                      (0x2U)
#define AIPS_OPACR_WP7_SHIFT                     (1U)
#define AIPS_OPACR_WP7_WIDTH                     (1U)
#define AIPS_OPACR_WP7(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP7_SHIFT)) & AIPS_OPACR_WP7_MASK)

#define AIPS_OPACR_SP7_MASK                      (0x4U)
#define AIPS_OPACR_SP7_SHIFT                     (2U)
#define AIPS_OPACR_SP7_WIDTH                     (1U)
#define AIPS_OPACR_SP7(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP7_SHIFT)) & AIPS_OPACR_SP7_MASK)

#define AIPS_OPACR_TP6_MASK                      (0x10U)
#define AIPS_OPACR_TP6_SHIFT                     (4U)
#define AIPS_OPACR_TP6_WIDTH                     (1U)
#define AIPS_OPACR_TP6(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP6_SHIFT)) & AIPS_OPACR_TP6_MASK)

#define AIPS_OPACR_WP6_MASK                      (0x20U)
#define AIPS_OPACR_WP6_SHIFT                     (5U)
#define AIPS_OPACR_WP6_WIDTH                     (1U)
#define AIPS_OPACR_WP6(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP6_SHIFT)) & AIPS_OPACR_WP6_MASK)

#define AIPS_OPACR_SP6_MASK                      (0x40U)
#define AIPS_OPACR_SP6_SHIFT                     (6U)
#define AIPS_OPACR_SP6_WIDTH                     (1U)
#define AIPS_OPACR_SP6(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP6_SHIFT)) & AIPS_OPACR_SP6_MASK)

#define AIPS_OPACR_TP5_MASK                      (0x100U)
#define AIPS_OPACR_TP5_SHIFT                     (8U)
#define AIPS_OPACR_TP5_WIDTH                     (1U)
#define AIPS_OPACR_TP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP5_SHIFT)) & AIPS_OPACR_TP5_MASK)

#define AIPS_OPACR_WP5_MASK                      (0x200U)
#define AIPS_OPACR_WP5_SHIFT                     (9U)
#define AIPS_OPACR_WP5_WIDTH                     (1U)
#define AIPS_OPACR_WP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP5_SHIFT)) & AIPS_OPACR_WP5_MASK)

#define AIPS_OPACR_SP5_MASK                      (0x400U)
#define AIPS_OPACR_SP5_SHIFT                     (10U)
#define AIPS_OPACR_SP5_WIDTH                     (1U)
#define AIPS_OPACR_SP5(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP5_SHIFT)) & AIPS_OPACR_SP5_MASK)

#define AIPS_OPACR_TP4_MASK                      (0x1000U)
#define AIPS_OPACR_TP4_SHIFT                     (12U)
#define AIPS_OPACR_TP4_WIDTH                     (1U)
#define AIPS_OPACR_TP4(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP4_SHIFT)) & AIPS_OPACR_TP4_MASK)

#define AIPS_OPACR_WP4_MASK                      (0x2000U)
#define AIPS_OPACR_WP4_SHIFT                     (13U)
#define AIPS_OPACR_WP4_WIDTH                     (1U)
#define AIPS_OPACR_WP4(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP4_SHIFT)) & AIPS_OPACR_WP4_MASK)

#define AIPS_OPACR_SP4_MASK                      (0x4000U)
#define AIPS_OPACR_SP4_SHIFT                     (14U)
#define AIPS_OPACR_SP4_WIDTH                     (1U)
#define AIPS_OPACR_SP4(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP4_SHIFT)) & AIPS_OPACR_SP4_MASK)

#define AIPS_OPACR_TP3_MASK                      (0x10000U)
#define AIPS_OPACR_TP3_SHIFT                     (16U)
#define AIPS_OPACR_TP3_WIDTH                     (1U)
#define AIPS_OPACR_TP3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP3_SHIFT)) & AIPS_OPACR_TP3_MASK)

#define AIPS_OPACR_WP3_MASK                      (0x20000U)
#define AIPS_OPACR_WP3_SHIFT                     (17U)
#define AIPS_OPACR_WP3_WIDTH                     (1U)
#define AIPS_OPACR_WP3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP3_SHIFT)) & AIPS_OPACR_WP3_MASK)

#define AIPS_OPACR_SP3_MASK                      (0x40000U)
#define AIPS_OPACR_SP3_SHIFT                     (18U)
#define AIPS_OPACR_SP3_WIDTH                     (1U)
#define AIPS_OPACR_SP3(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP3_SHIFT)) & AIPS_OPACR_SP3_MASK)

#define AIPS_OPACR_TP2_MASK                      (0x100000U)
#define AIPS_OPACR_TP2_SHIFT                     (20U)
#define AIPS_OPACR_TP2_WIDTH                     (1U)
#define AIPS_OPACR_TP2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP2_SHIFT)) & AIPS_OPACR_TP2_MASK)

#define AIPS_OPACR_WP2_MASK                      (0x200000U)
#define AIPS_OPACR_WP2_SHIFT                     (21U)
#define AIPS_OPACR_WP2_WIDTH                     (1U)
#define AIPS_OPACR_WP2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP2_SHIFT)) & AIPS_OPACR_WP2_MASK)

#define AIPS_OPACR_SP2_MASK                      (0x400000U)
#define AIPS_OPACR_SP2_SHIFT                     (22U)
#define AIPS_OPACR_SP2_WIDTH                     (1U)
#define AIPS_OPACR_SP2(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP2_SHIFT)) & AIPS_OPACR_SP2_MASK)

#define AIPS_OPACR_TP1_MASK                      (0x1000000U)
#define AIPS_OPACR_TP1_SHIFT                     (24U)
#define AIPS_OPACR_TP1_WIDTH                     (1U)
#define AIPS_OPACR_TP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP1_SHIFT)) & AIPS_OPACR_TP1_MASK)

#define AIPS_OPACR_WP1_MASK                      (0x2000000U)
#define AIPS_OPACR_WP1_SHIFT                     (25U)
#define AIPS_OPACR_WP1_WIDTH                     (1U)
#define AIPS_OPACR_WP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP1_SHIFT)) & AIPS_OPACR_WP1_MASK)

#define AIPS_OPACR_SP1_MASK                      (0x4000000U)
#define AIPS_OPACR_SP1_SHIFT                     (26U)
#define AIPS_OPACR_SP1_WIDTH                     (1U)
#define AIPS_OPACR_SP1(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP1_SHIFT)) & AIPS_OPACR_SP1_MASK)

#define AIPS_OPACR_TP0_MASK                      (0x10000000U)
#define AIPS_OPACR_TP0_SHIFT                     (28U)
#define AIPS_OPACR_TP0_WIDTH                     (1U)
#define AIPS_OPACR_TP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_TP0_SHIFT)) & AIPS_OPACR_TP0_MASK)

#define AIPS_OPACR_WP0_MASK                      (0x20000000U)
#define AIPS_OPACR_WP0_SHIFT                     (29U)
#define AIPS_OPACR_WP0_WIDTH                     (1U)
#define AIPS_OPACR_WP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_WP0_SHIFT)) & AIPS_OPACR_WP0_MASK)

#define AIPS_OPACR_SP0_MASK                      (0x40000000U)
#define AIPS_OPACR_SP0_SHIFT                     (30U)
#define AIPS_OPACR_SP0_WIDTH                     (1U)
#define AIPS_OPACR_SP0(x)                        (((uint32_t)(((uint32_t)(x)) << AIPS_OPACR_SP0_SHIFT)) & AIPS_OPACR_SP0_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group AIPS_Register_Masks */

/*!
 * @}
 */ /* end of group AIPS_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_AIPS_H_) */

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
 * @file S32K148_TRGMUX.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_TRGMUX
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
#if !defined(S32K148_TRGMUX_H_)  /* Check if memory map has not been already included */
#define S32K148_TRGMUX_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- TRGMUX Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup TRGMUX_Peripheral_Access_Layer TRGMUX Peripheral Access Layer
 * @{
 */

/** TRGMUX - Size of Registers Arrays */
#define TRGMUX_TRGMUXn_COUNT                      32u

/** TRGMUX - Register Layout Typedef */
typedef struct {
  __IO uint32_t TRGMUXn[TRGMUX_TRGMUXn_COUNT];     /**< TRGMUX DMAMUX0 Register..TRGMUX FTM7 Register, array offset: 0x0, array step: 0x4 */
} TRGMUX_Type, *TRGMUX_MemMapPtr;

/** Number of instances of the TRGMUX module. */
#define TRGMUX_INSTANCE_COUNT                    (1u)

/* TRGMUX - Peripheral instance base addresses */
/** Peripheral TRGMUX base address */
#define IP_TRGMUX_BASE                           (0x40063000u)
/** Peripheral TRGMUX base pointer */
#define IP_TRGMUX                                ((TRGMUX_Type *)IP_TRGMUX_BASE)
/** Array initializer of TRGMUX peripheral base addresses */
#define IP_TRGMUX_BASE_ADDRS                     { IP_TRGMUX_BASE }
/** Array initializer of TRGMUX peripheral base pointers */
#define IP_TRGMUX_BASE_PTRS                      { IP_TRGMUX }

/* ----------------------------------------------------------------------------
   -- TRGMUX Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup TRGMUX_Register_Masks TRGMUX Register Masks
 * @{
 */

/*! @name TRGMUXn - TRGMUX DMAMUX0 Register..TRGMUX FTM7 Register */
/*! @{ */

#define TRGMUX_TRGMUXn_SEL0_MASK                 (0x7FU)
#define TRGMUX_TRGMUXn_SEL0_SHIFT                (0U)
#define TRGMUX_TRGMUXn_SEL0_WIDTH                (7U)
#define TRGMUX_TRGMUXn_SEL0(x)                   (((uint32_t)(((uint32_t)(x)) << TRGMUX_TRGMUXn_SEL0_SHIFT)) & TRGMUX_TRGMUXn_SEL0_MASK)

#define TRGMUX_TRGMUXn_SEL1_MASK                 (0x7F00U)
#define TRGMUX_TRGMUXn_SEL1_SHIFT                (8U)
#define TRGMUX_TRGMUXn_SEL1_WIDTH                (7U)
#define TRGMUX_TRGMUXn_SEL1(x)                   (((uint32_t)(((uint32_t)(x)) << TRGMUX_TRGMUXn_SEL1_SHIFT)) & TRGMUX_TRGMUXn_SEL1_MASK)

#define TRGMUX_TRGMUXn_SEL2_MASK                 (0x7F0000U)
#define TRGMUX_TRGMUXn_SEL2_SHIFT                (16U)
#define TRGMUX_TRGMUXn_SEL2_WIDTH                (7U)
#define TRGMUX_TRGMUXn_SEL2(x)                   (((uint32_t)(((uint32_t)(x)) << TRGMUX_TRGMUXn_SEL2_SHIFT)) & TRGMUX_TRGMUXn_SEL2_MASK)

#define TRGMUX_TRGMUXn_SEL3_MASK                 (0x7F000000U)
#define TRGMUX_TRGMUXn_SEL3_SHIFT                (24U)
#define TRGMUX_TRGMUXn_SEL3_WIDTH                (7U)
#define TRGMUX_TRGMUXn_SEL3(x)                   (((uint32_t)(((uint32_t)(x)) << TRGMUX_TRGMUXn_SEL3_SHIFT)) & TRGMUX_TRGMUXn_SEL3_MASK)

#define TRGMUX_TRGMUXn_LK_MASK                   (0x80000000U)
#define TRGMUX_TRGMUXn_LK_SHIFT                  (31U)
#define TRGMUX_TRGMUXn_LK_WIDTH                  (1U)
#define TRGMUX_TRGMUXn_LK(x)                     (((uint32_t)(((uint32_t)(x)) << TRGMUX_TRGMUXn_LK_SHIFT)) & TRGMUX_TRGMUXn_LK_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group TRGMUX_Register_Masks */
#define TRGMUX_DMAMUX0_INDEX 0
#define TRGMUX_EXTOUT0_INDEX 1
#define TRGMUX_EXTOUT1_INDEX 2
#define TRGMUX_ADC0_INDEX 3
#define TRGMUX_ADC1_INDEX 4
#define TRGMUX_CMP0_INDEX 7
#define TRGMUX_FTM0_INDEX 10
#define TRGMUX_FTM1_INDEX 11
#define TRGMUX_FTM2_INDEX 12
#define TRGMUX_FTM3_INDEX 13
#define TRGMUX_PDB0_INDEX 14
#define TRGMUX_PDB1_INDEX 15
#define TRGMUX_FLEXIO_INDEX 17
#define TRGMUX_LPIT0_INDEX 18
#define TRGMUX_LPUART0_INDEX 19
#define TRGMUX_LPUART1_INDEX 20
#define TRGMUX_LPI2C0_INDEX 21
#define TRGMUX_LPSPI0_INDEX 23
#define TRGMUX_LPSPI1_INDEX 24
#define TRGMUX_LPTMR0_INDEX 25
#define TRGMUX_LPI2C1_INDEX 27
#define TRGMUX_FTM4_INDEX 28
#define TRGMUX_FTM5_INDEX 29
#define TRGMUX_FTM6_INDEX 30
#define TRGMUX_FTM7_INDEX 31


/*!
 * @}
 */ /* end of group TRGMUX_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_TRGMUX_H_) */

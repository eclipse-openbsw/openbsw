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
 * @file S32K148_PCC.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_PCC
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
#if !defined(S32K148_PCC_H_)  /* Check if memory map has not been already included */
#define S32K148_PCC_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- PCC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PCC_Peripheral_Access_Layer PCC Peripheral Access Layer
 * @{
 */

/** PCC - Size of Registers Arrays */
#define PCC_PCCn_COUNT                            122u

/** PCC - Register Layout Typedef */
typedef struct {
  __IO uint32_t PCCn[PCC_PCCn_COUNT];              /**< PCC FTFC Register..PCC ENET Register, array offset: 0x0, array step: 0x4 */
} PCC_Type, *PCC_MemMapPtr;

/** Number of instances of the PCC module. */
#define PCC_INSTANCE_COUNT                       (1u)

/* PCC - Peripheral instance base addresses */
/** Peripheral PCC base address */
#define IP_PCC_BASE                              (0x40065000u)
/** Peripheral PCC base pointer */
#define IP_PCC                                   ((PCC_Type *)IP_PCC_BASE)
/** Array initializer of PCC peripheral base addresses */
#define IP_PCC_BASE_ADDRS                        { IP_PCC_BASE }
/** Array initializer of PCC peripheral base pointers */
#define IP_PCC_BASE_PTRS                         { IP_PCC }

/* ----------------------------------------------------------------------------
   -- PCC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PCC_Register_Masks PCC Register Masks
 * @{
 */

/*! @name PCCn - PCC FTFC Register..PCC ENET Register */
/*! @{ */

#define PCC_PCCn_PCD_MASK                        (0x7U)
#define PCC_PCCn_PCD_SHIFT                       (0U)
#define PCC_PCCn_PCD_WIDTH                       (3U)
#define PCC_PCCn_PCD(x)                          (((uint32_t)(((uint32_t)(x)) << PCC_PCCn_PCD_SHIFT)) & PCC_PCCn_PCD_MASK)

#define PCC_PCCn_FRAC_MASK                       (0x8U)
#define PCC_PCCn_FRAC_SHIFT                      (3U)
#define PCC_PCCn_FRAC_WIDTH                      (1U)
#define PCC_PCCn_FRAC(x)                         (((uint32_t)(((uint32_t)(x)) << PCC_PCCn_FRAC_SHIFT)) & PCC_PCCn_FRAC_MASK)

#define PCC_PCCn_PCS_MASK                        (0x7000000U)
#define PCC_PCCn_PCS_SHIFT                       (24U)
#define PCC_PCCn_PCS_WIDTH                       (3U)
#define PCC_PCCn_PCS(x)                          (((uint32_t)(((uint32_t)(x)) << PCC_PCCn_PCS_SHIFT)) & PCC_PCCn_PCS_MASK)

#define PCC_PCCn_CGC_MASK                        (0x40000000U)
#define PCC_PCCn_CGC_SHIFT                       (30U)
#define PCC_PCCn_CGC_WIDTH                       (1U)
#define PCC_PCCn_CGC(x)                          (((uint32_t)(((uint32_t)(x)) << PCC_PCCn_CGC_SHIFT)) & PCC_PCCn_CGC_MASK)

#define PCC_PCCn_PR_MASK                         (0x80000000U)
#define PCC_PCCn_PR_SHIFT                        (31U)
#define PCC_PCCn_PR_WIDTH                        (1U)
#define PCC_PCCn_PR(x)                           (((uint32_t)(((uint32_t)(x)) << PCC_PCCn_PR_SHIFT)) & PCC_PCCn_PR_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group PCC_Register_Masks */
#define PCC_FTFC_INDEX 32
#define PCC_DMAMUX_INDEX 33
#define PCC_FlexCAN0_INDEX 36
#define PCC_FlexCAN1_INDEX 37
#define PCC_FTM3_INDEX 38
#define PCC_ADC1_INDEX 39
#define PCC_FlexCAN2_INDEX 43
#define PCC_LPSPI0_INDEX 44
#define PCC_LPSPI1_INDEX 45
#define PCC_LPSPI2_INDEX 46
#define PCC_PDB1_INDEX 49
#define PCC_CRC_INDEX 50
#define PCC_PDB0_INDEX 54
#define PCC_LPIT_INDEX 55
#define PCC_FTM0_INDEX 56
#define PCC_FTM1_INDEX 57
#define PCC_FTM2_INDEX 58
#define PCC_ADC0_INDEX 59
#define PCC_RTC_INDEX 61
#define PCC_LPTMR0_INDEX 64
#define PCC_PORTA_INDEX 73
#define PCC_PORTB_INDEX 74
#define PCC_PORTC_INDEX 75
#define PCC_PORTD_INDEX 76
#define PCC_PORTE_INDEX 77
#define PCC_SAI0_INDEX 84
#define PCC_SAI1_INDEX 85
#define PCC_FlexIO_INDEX 90
#define PCC_EWM_INDEX 97
#define PCC_LPI2C0_INDEX 102
#define PCC_LPI2C1_INDEX 103
#define PCC_LPUART0_INDEX 106
#define PCC_LPUART1_INDEX 107
#define PCC_LPUART2_INDEX 108
#define PCC_FTM4_INDEX 110
#define PCC_FTM5_INDEX 111
#define PCC_FTM6_INDEX 112
#define PCC_FTM7_INDEX 113
#define PCC_CMP0_INDEX 115
#define PCC_QSPI_INDEX 118
#define PCC_ENET_INDEX 121


/*!
 * @}
 */ /* end of group PCC_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_PCC_H_) */

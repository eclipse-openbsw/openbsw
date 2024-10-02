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
 * @file S32K148_MSCM.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_MSCM
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
#if !defined(S32K148_MSCM_H_)  /* Check if memory map has not been already included */
#define S32K148_MSCM_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- MSCM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MSCM_Peripheral_Access_Layer MSCM Peripheral Access Layer
 * @{
 */

/** MSCM - Size of Registers Arrays */
#define MSCM_OCMDR_COUNT                          3u

/** MSCM - Register Layout Typedef */
typedef struct {
  __I  uint32_t CPxTYPE;                           /**< Processor X Type Register, offset: 0x0 */
  __I  uint32_t CPxNUM;                            /**< Processor X Number Register, offset: 0x4 */
  __I  uint32_t CPxMASTER;                         /**< Processor X Master Register, offset: 0x8 */
  __I  uint32_t CPxCOUNT;                          /**< Processor X Count Register, offset: 0xC */
  __I  uint32_t CPxCFG0;                           /**< Processor X Configuration Register 0, offset: 0x10 */
  __I  uint32_t CPxCFG1;                           /**< Processor X Configuration Register 1, offset: 0x14 */
  __I  uint32_t CPxCFG2;                           /**< Processor X Configuration Register 2, offset: 0x18 */
  __I  uint32_t CPxCFG3;                           /**< Processor X Configuration Register 3, offset: 0x1C */
  __I  uint32_t CP0TYPE;                           /**< Processor 0 Type Register, offset: 0x20 */
  __I  uint32_t CP0NUM;                            /**< Processor 0 Number Register, offset: 0x24 */
  __I  uint32_t CP0MASTER;                         /**< Processor 0 Master Register, offset: 0x28 */
  __I  uint32_t CP0COUNT;                          /**< Processor 0 Count Register, offset: 0x2C */
  __I  uint32_t CP0CFG0;                           /**< Processor 0 Configuration Register 0, offset: 0x30 */
  __I  uint32_t CP0CFG1;                           /**< Processor 0 Configuration Register 1, offset: 0x34 */
  __I  uint32_t CP0CFG2;                           /**< Processor 0 Configuration Register 2, offset: 0x38 */
  __I  uint32_t CP0CFG3;                           /**< Processor 0 Configuration Register 3, offset: 0x3C */
  uint8_t RESERVED_0[960];
  __IO uint32_t OCMDR[MSCM_OCMDR_COUNT];           /**< On-Chip Memory Descriptor Register, array offset: 0x400, array step: 0x4 */
} MSCM_Type, *MSCM_MemMapPtr;

/** Number of instances of the MSCM module. */
#define MSCM_INSTANCE_COUNT                      (1u)

/* MSCM - Peripheral instance base addresses */
/** Peripheral MSCM base address */
#define IP_MSCM_BASE                             (0x40001000u)
/** Peripheral MSCM base pointer */
#define IP_MSCM                                  ((MSCM_Type *)IP_MSCM_BASE)
/** Array initializer of MSCM peripheral base addresses */
#define IP_MSCM_BASE_ADDRS                       { IP_MSCM_BASE }
/** Array initializer of MSCM peripheral base pointers */
#define IP_MSCM_BASE_PTRS                        { IP_MSCM }

/* ----------------------------------------------------------------------------
   -- MSCM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MSCM_Register_Masks MSCM Register Masks
 * @{
 */

/*! @name CPxTYPE - Processor X Type Register */
/*! @{ */

#define MSCM_CPxTYPE_RYPZ_MASK                   (0xFFU)
#define MSCM_CPxTYPE_RYPZ_SHIFT                  (0U)
#define MSCM_CPxTYPE_RYPZ_WIDTH                  (8U)
#define MSCM_CPxTYPE_RYPZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxTYPE_RYPZ_SHIFT)) & MSCM_CPxTYPE_RYPZ_MASK)

#define MSCM_CPxTYPE_PERSONALITY_MASK            (0xFFFFFF00U)
#define MSCM_CPxTYPE_PERSONALITY_SHIFT           (8U)
#define MSCM_CPxTYPE_PERSONALITY_WIDTH           (24U)
#define MSCM_CPxTYPE_PERSONALITY(x)              (((uint32_t)(((uint32_t)(x)) << MSCM_CPxTYPE_PERSONALITY_SHIFT)) & MSCM_CPxTYPE_PERSONALITY_MASK)
/*! @} */

/*! @name CPxNUM - Processor X Number Register */
/*! @{ */

#define MSCM_CPxNUM_CPN_MASK                     (0x1U)
#define MSCM_CPxNUM_CPN_SHIFT                    (0U)
#define MSCM_CPxNUM_CPN_WIDTH                    (1U)
#define MSCM_CPxNUM_CPN(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CPxNUM_CPN_SHIFT)) & MSCM_CPxNUM_CPN_MASK)
/*! @} */

/*! @name CPxMASTER - Processor X Master Register */
/*! @{ */

#define MSCM_CPxMASTER_PPMN_MASK                 (0x3FU)
#define MSCM_CPxMASTER_PPMN_SHIFT                (0U)
#define MSCM_CPxMASTER_PPMN_WIDTH                (6U)
#define MSCM_CPxMASTER_PPMN(x)                   (((uint32_t)(((uint32_t)(x)) << MSCM_CPxMASTER_PPMN_SHIFT)) & MSCM_CPxMASTER_PPMN_MASK)
/*! @} */

/*! @name CPxCOUNT - Processor X Count Register */
/*! @{ */

#define MSCM_CPxCOUNT_PCNT_MASK                  (0x3U)
#define MSCM_CPxCOUNT_PCNT_SHIFT                 (0U)
#define MSCM_CPxCOUNT_PCNT_WIDTH                 (2U)
#define MSCM_CPxCOUNT_PCNT(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCOUNT_PCNT_SHIFT)) & MSCM_CPxCOUNT_PCNT_MASK)
/*! @} */

/*! @name CPxCFG0 - Processor X Configuration Register 0 */
/*! @{ */

#define MSCM_CPxCFG0_DCWY_MASK                   (0xFFU)
#define MSCM_CPxCFG0_DCWY_SHIFT                  (0U)
#define MSCM_CPxCFG0_DCWY_WIDTH                  (8U)
#define MSCM_CPxCFG0_DCWY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG0_DCWY_SHIFT)) & MSCM_CPxCFG0_DCWY_MASK)

#define MSCM_CPxCFG0_DCSZ_MASK                   (0xFF00U)
#define MSCM_CPxCFG0_DCSZ_SHIFT                  (8U)
#define MSCM_CPxCFG0_DCSZ_WIDTH                  (8U)
#define MSCM_CPxCFG0_DCSZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG0_DCSZ_SHIFT)) & MSCM_CPxCFG0_DCSZ_MASK)

#define MSCM_CPxCFG0_ICWY_MASK                   (0xFF0000U)
#define MSCM_CPxCFG0_ICWY_SHIFT                  (16U)
#define MSCM_CPxCFG0_ICWY_WIDTH                  (8U)
#define MSCM_CPxCFG0_ICWY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG0_ICWY_SHIFT)) & MSCM_CPxCFG0_ICWY_MASK)

#define MSCM_CPxCFG0_ICSZ_MASK                   (0xFF000000U)
#define MSCM_CPxCFG0_ICSZ_SHIFT                  (24U)
#define MSCM_CPxCFG0_ICSZ_WIDTH                  (8U)
#define MSCM_CPxCFG0_ICSZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG0_ICSZ_SHIFT)) & MSCM_CPxCFG0_ICSZ_MASK)
/*! @} */

/*! @name CPxCFG1 - Processor X Configuration Register 1 */
/*! @{ */

#define MSCM_CPxCFG1_L2WY_MASK                   (0xFF0000U)
#define MSCM_CPxCFG1_L2WY_SHIFT                  (16U)
#define MSCM_CPxCFG1_L2WY_WIDTH                  (8U)
#define MSCM_CPxCFG1_L2WY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG1_L2WY_SHIFT)) & MSCM_CPxCFG1_L2WY_MASK)

#define MSCM_CPxCFG1_L2SZ_MASK                   (0xFF000000U)
#define MSCM_CPxCFG1_L2SZ_SHIFT                  (24U)
#define MSCM_CPxCFG1_L2SZ_WIDTH                  (8U)
#define MSCM_CPxCFG1_L2SZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG1_L2SZ_SHIFT)) & MSCM_CPxCFG1_L2SZ_MASK)
/*! @} */

/*! @name CPxCFG2 - Processor X Configuration Register 2 */
/*! @{ */

#define MSCM_CPxCFG2_TMUSZ_MASK                  (0xFF00U)
#define MSCM_CPxCFG2_TMUSZ_SHIFT                 (8U)
#define MSCM_CPxCFG2_TMUSZ_WIDTH                 (8U)
#define MSCM_CPxCFG2_TMUSZ(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG2_TMUSZ_SHIFT)) & MSCM_CPxCFG2_TMUSZ_MASK)

#define MSCM_CPxCFG2_TMLSZ_MASK                  (0xFF000000U)
#define MSCM_CPxCFG2_TMLSZ_SHIFT                 (24U)
#define MSCM_CPxCFG2_TMLSZ_WIDTH                 (8U)
#define MSCM_CPxCFG2_TMLSZ(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG2_TMLSZ_SHIFT)) & MSCM_CPxCFG2_TMLSZ_MASK)
/*! @} */

/*! @name CPxCFG3 - Processor X Configuration Register 3 */
/*! @{ */

#define MSCM_CPxCFG3_FPU_MASK                    (0x1U)
#define MSCM_CPxCFG3_FPU_SHIFT                   (0U)
#define MSCM_CPxCFG3_FPU_WIDTH                   (1U)
#define MSCM_CPxCFG3_FPU(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_FPU_SHIFT)) & MSCM_CPxCFG3_FPU_MASK)

#define MSCM_CPxCFG3_SIMD_MASK                   (0x2U)
#define MSCM_CPxCFG3_SIMD_SHIFT                  (1U)
#define MSCM_CPxCFG3_SIMD_WIDTH                  (1U)
#define MSCM_CPxCFG3_SIMD(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_SIMD_SHIFT)) & MSCM_CPxCFG3_SIMD_MASK)

#define MSCM_CPxCFG3_JAZ_MASK                    (0x4U)
#define MSCM_CPxCFG3_JAZ_SHIFT                   (2U)
#define MSCM_CPxCFG3_JAZ_WIDTH                   (1U)
#define MSCM_CPxCFG3_JAZ(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_JAZ_SHIFT)) & MSCM_CPxCFG3_JAZ_MASK)

#define MSCM_CPxCFG3_MMU_MASK                    (0x8U)
#define MSCM_CPxCFG3_MMU_SHIFT                   (3U)
#define MSCM_CPxCFG3_MMU_WIDTH                   (1U)
#define MSCM_CPxCFG3_MMU(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_MMU_SHIFT)) & MSCM_CPxCFG3_MMU_MASK)

#define MSCM_CPxCFG3_TZ_MASK                     (0x10U)
#define MSCM_CPxCFG3_TZ_SHIFT                    (4U)
#define MSCM_CPxCFG3_TZ_WIDTH                    (1U)
#define MSCM_CPxCFG3_TZ(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_TZ_SHIFT)) & MSCM_CPxCFG3_TZ_MASK)

#define MSCM_CPxCFG3_CMP_MASK                    (0x20U)
#define MSCM_CPxCFG3_CMP_SHIFT                   (5U)
#define MSCM_CPxCFG3_CMP_WIDTH                   (1U)
#define MSCM_CPxCFG3_CMP(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_CMP_SHIFT)) & MSCM_CPxCFG3_CMP_MASK)

#define MSCM_CPxCFG3_BB_MASK                     (0x40U)
#define MSCM_CPxCFG3_BB_SHIFT                    (6U)
#define MSCM_CPxCFG3_BB_WIDTH                    (1U)
#define MSCM_CPxCFG3_BB(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_BB_SHIFT)) & MSCM_CPxCFG3_BB_MASK)

#define MSCM_CPxCFG3_SBP_MASK                    (0x300U)
#define MSCM_CPxCFG3_SBP_SHIFT                   (8U)
#define MSCM_CPxCFG3_SBP_WIDTH                   (2U)
#define MSCM_CPxCFG3_SBP(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CPxCFG3_SBP_SHIFT)) & MSCM_CPxCFG3_SBP_MASK)
/*! @} */

/*! @name CP0TYPE - Processor 0 Type Register */
/*! @{ */

#define MSCM_CP0TYPE_RYPZ_MASK                   (0xFFU)
#define MSCM_CP0TYPE_RYPZ_SHIFT                  (0U)
#define MSCM_CP0TYPE_RYPZ_WIDTH                  (8U)
#define MSCM_CP0TYPE_RYPZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0TYPE_RYPZ_SHIFT)) & MSCM_CP0TYPE_RYPZ_MASK)

#define MSCM_CP0TYPE_PERSONALITY_MASK            (0xFFFFFF00U)
#define MSCM_CP0TYPE_PERSONALITY_SHIFT           (8U)
#define MSCM_CP0TYPE_PERSONALITY_WIDTH           (24U)
#define MSCM_CP0TYPE_PERSONALITY(x)              (((uint32_t)(((uint32_t)(x)) << MSCM_CP0TYPE_PERSONALITY_SHIFT)) & MSCM_CP0TYPE_PERSONALITY_MASK)
/*! @} */

/*! @name CP0NUM - Processor 0 Number Register */
/*! @{ */

#define MSCM_CP0NUM_CPN_MASK                     (0x1U)
#define MSCM_CP0NUM_CPN_SHIFT                    (0U)
#define MSCM_CP0NUM_CPN_WIDTH                    (1U)
#define MSCM_CP0NUM_CPN(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CP0NUM_CPN_SHIFT)) & MSCM_CP0NUM_CPN_MASK)
/*! @} */

/*! @name CP0MASTER - Processor 0 Master Register */
/*! @{ */

#define MSCM_CP0MASTER_PPMN_MASK                 (0x3FU)
#define MSCM_CP0MASTER_PPMN_SHIFT                (0U)
#define MSCM_CP0MASTER_PPMN_WIDTH                (6U)
#define MSCM_CP0MASTER_PPMN(x)                   (((uint32_t)(((uint32_t)(x)) << MSCM_CP0MASTER_PPMN_SHIFT)) & MSCM_CP0MASTER_PPMN_MASK)
/*! @} */

/*! @name CP0COUNT - Processor 0 Count Register */
/*! @{ */

#define MSCM_CP0COUNT_PCNT_MASK                  (0x3U)
#define MSCM_CP0COUNT_PCNT_SHIFT                 (0U)
#define MSCM_CP0COUNT_PCNT_WIDTH                 (2U)
#define MSCM_CP0COUNT_PCNT(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CP0COUNT_PCNT_SHIFT)) & MSCM_CP0COUNT_PCNT_MASK)
/*! @} */

/*! @name CP0CFG0 - Processor 0 Configuration Register 0 */
/*! @{ */

#define MSCM_CP0CFG0_DCWY_MASK                   (0xFFU)
#define MSCM_CP0CFG0_DCWY_SHIFT                  (0U)
#define MSCM_CP0CFG0_DCWY_WIDTH                  (8U)
#define MSCM_CP0CFG0_DCWY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG0_DCWY_SHIFT)) & MSCM_CP0CFG0_DCWY_MASK)

#define MSCM_CP0CFG0_DCSZ_MASK                   (0xFF00U)
#define MSCM_CP0CFG0_DCSZ_SHIFT                  (8U)
#define MSCM_CP0CFG0_DCSZ_WIDTH                  (8U)
#define MSCM_CP0CFG0_DCSZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG0_DCSZ_SHIFT)) & MSCM_CP0CFG0_DCSZ_MASK)

#define MSCM_CP0CFG0_ICWY_MASK                   (0xFF0000U)
#define MSCM_CP0CFG0_ICWY_SHIFT                  (16U)
#define MSCM_CP0CFG0_ICWY_WIDTH                  (8U)
#define MSCM_CP0CFG0_ICWY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG0_ICWY_SHIFT)) & MSCM_CP0CFG0_ICWY_MASK)

#define MSCM_CP0CFG0_ICSZ_MASK                   (0xFF000000U)
#define MSCM_CP0CFG0_ICSZ_SHIFT                  (24U)
#define MSCM_CP0CFG0_ICSZ_WIDTH                  (8U)
#define MSCM_CP0CFG0_ICSZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG0_ICSZ_SHIFT)) & MSCM_CP0CFG0_ICSZ_MASK)
/*! @} */

/*! @name CP0CFG1 - Processor 0 Configuration Register 1 */
/*! @{ */

#define MSCM_CP0CFG1_L2WY_MASK                   (0xFF0000U)
#define MSCM_CP0CFG1_L2WY_SHIFT                  (16U)
#define MSCM_CP0CFG1_L2WY_WIDTH                  (8U)
#define MSCM_CP0CFG1_L2WY(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG1_L2WY_SHIFT)) & MSCM_CP0CFG1_L2WY_MASK)

#define MSCM_CP0CFG1_L2SZ_MASK                   (0xFF000000U)
#define MSCM_CP0CFG1_L2SZ_SHIFT                  (24U)
#define MSCM_CP0CFG1_L2SZ_WIDTH                  (8U)
#define MSCM_CP0CFG1_L2SZ(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG1_L2SZ_SHIFT)) & MSCM_CP0CFG1_L2SZ_MASK)
/*! @} */

/*! @name CP0CFG2 - Processor 0 Configuration Register 2 */
/*! @{ */

#define MSCM_CP0CFG2_TMUSZ_MASK                  (0xFF00U)
#define MSCM_CP0CFG2_TMUSZ_SHIFT                 (8U)
#define MSCM_CP0CFG2_TMUSZ_WIDTH                 (8U)
#define MSCM_CP0CFG2_TMUSZ(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG2_TMUSZ_SHIFT)) & MSCM_CP0CFG2_TMUSZ_MASK)

#define MSCM_CP0CFG2_TMLSZ_MASK                  (0xFF000000U)
#define MSCM_CP0CFG2_TMLSZ_SHIFT                 (24U)
#define MSCM_CP0CFG2_TMLSZ_WIDTH                 (8U)
#define MSCM_CP0CFG2_TMLSZ(x)                    (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG2_TMLSZ_SHIFT)) & MSCM_CP0CFG2_TMLSZ_MASK)
/*! @} */

/*! @name CP0CFG3 - Processor 0 Configuration Register 3 */
/*! @{ */

#define MSCM_CP0CFG3_FPU_MASK                    (0x1U)
#define MSCM_CP0CFG3_FPU_SHIFT                   (0U)
#define MSCM_CP0CFG3_FPU_WIDTH                   (1U)
#define MSCM_CP0CFG3_FPU(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_FPU_SHIFT)) & MSCM_CP0CFG3_FPU_MASK)

#define MSCM_CP0CFG3_SIMD_MASK                   (0x2U)
#define MSCM_CP0CFG3_SIMD_SHIFT                  (1U)
#define MSCM_CP0CFG3_SIMD_WIDTH                  (1U)
#define MSCM_CP0CFG3_SIMD(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_SIMD_SHIFT)) & MSCM_CP0CFG3_SIMD_MASK)

#define MSCM_CP0CFG3_JAZ_MASK                    (0x4U)
#define MSCM_CP0CFG3_JAZ_SHIFT                   (2U)
#define MSCM_CP0CFG3_JAZ_WIDTH                   (1U)
#define MSCM_CP0CFG3_JAZ(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_JAZ_SHIFT)) & MSCM_CP0CFG3_JAZ_MASK)

#define MSCM_CP0CFG3_MMU_MASK                    (0x8U)
#define MSCM_CP0CFG3_MMU_SHIFT                   (3U)
#define MSCM_CP0CFG3_MMU_WIDTH                   (1U)
#define MSCM_CP0CFG3_MMU(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_MMU_SHIFT)) & MSCM_CP0CFG3_MMU_MASK)

#define MSCM_CP0CFG3_TZ_MASK                     (0x10U)
#define MSCM_CP0CFG3_TZ_SHIFT                    (4U)
#define MSCM_CP0CFG3_TZ_WIDTH                    (1U)
#define MSCM_CP0CFG3_TZ(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_TZ_SHIFT)) & MSCM_CP0CFG3_TZ_MASK)

#define MSCM_CP0CFG3_CMP_MASK                    (0x20U)
#define MSCM_CP0CFG3_CMP_SHIFT                   (5U)
#define MSCM_CP0CFG3_CMP_WIDTH                   (1U)
#define MSCM_CP0CFG3_CMP(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_CMP_SHIFT)) & MSCM_CP0CFG3_CMP_MASK)

#define MSCM_CP0CFG3_BB_MASK                     (0x40U)
#define MSCM_CP0CFG3_BB_SHIFT                    (6U)
#define MSCM_CP0CFG3_BB_WIDTH                    (1U)
#define MSCM_CP0CFG3_BB(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_BB_SHIFT)) & MSCM_CP0CFG3_BB_MASK)

#define MSCM_CP0CFG3_SBP_MASK                    (0x300U)
#define MSCM_CP0CFG3_SBP_SHIFT                   (8U)
#define MSCM_CP0CFG3_SBP_WIDTH                   (2U)
#define MSCM_CP0CFG3_SBP(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_CP0CFG3_SBP_SHIFT)) & MSCM_CP0CFG3_SBP_MASK)
/*! @} */

/*! @name OCMDR - On-Chip Memory Descriptor Register */
/*! @{ */

#define MSCM_OCMDR_OCM1_MASK                     (0x30U)
#define MSCM_OCMDR_OCM1_SHIFT                    (4U)
#define MSCM_OCMDR_OCM1_WIDTH                    (2U)
#define MSCM_OCMDR_OCM1(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCM1_SHIFT)) & MSCM_OCMDR_OCM1_MASK)

#define MSCM_OCMDR_OCMPU_MASK                    (0x1000U)
#define MSCM_OCMDR_OCMPU_SHIFT                   (12U)
#define MSCM_OCMDR_OCMPU_WIDTH                   (1U)
#define MSCM_OCMDR_OCMPU(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCMPU_SHIFT)) & MSCM_OCMDR_OCMPU_MASK)

#define MSCM_OCMDR_OCMT_MASK                     (0xE000U)
#define MSCM_OCMDR_OCMT_SHIFT                    (13U)
#define MSCM_OCMDR_OCMT_WIDTH                    (3U)
#define MSCM_OCMDR_OCMT(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCMT_SHIFT)) & MSCM_OCMDR_OCMT_MASK)

#define MSCM_OCMDR_RO_MASK                       (0x10000U)
#define MSCM_OCMDR_RO_SHIFT                      (16U)
#define MSCM_OCMDR_RO_WIDTH                      (1U)
#define MSCM_OCMDR_RO(x)                         (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_RO_SHIFT)) & MSCM_OCMDR_RO_MASK)

#define MSCM_OCMDR_OCMW_MASK                     (0xE0000U)
#define MSCM_OCMDR_OCMW_SHIFT                    (17U)
#define MSCM_OCMDR_OCMW_WIDTH                    (3U)
#define MSCM_OCMDR_OCMW(x)                       (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCMW_SHIFT)) & MSCM_OCMDR_OCMW_MASK)

#define MSCM_OCMDR_OCMSZ_MASK                    (0xF000000U)
#define MSCM_OCMDR_OCMSZ_SHIFT                   (24U)
#define MSCM_OCMDR_OCMSZ_WIDTH                   (4U)
#define MSCM_OCMDR_OCMSZ(x)                      (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCMSZ_SHIFT)) & MSCM_OCMDR_OCMSZ_MASK)

#define MSCM_OCMDR_OCMSZH_MASK                   (0x10000000U)
#define MSCM_OCMDR_OCMSZH_SHIFT                  (28U)
#define MSCM_OCMDR_OCMSZH_WIDTH                  (1U)
#define MSCM_OCMDR_OCMSZH(x)                     (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_OCMSZH_SHIFT)) & MSCM_OCMDR_OCMSZH_MASK)

#define MSCM_OCMDR_V_MASK                        (0x80000000U)
#define MSCM_OCMDR_V_SHIFT                       (31U)
#define MSCM_OCMDR_V_WIDTH                       (1U)
#define MSCM_OCMDR_V(x)                          (((uint32_t)(((uint32_t)(x)) << MSCM_OCMDR_V_SHIFT)) & MSCM_OCMDR_V_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group MSCM_Register_Masks */

/*!
 * @}
 */ /* end of group MSCM_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_MSCM_H_) */

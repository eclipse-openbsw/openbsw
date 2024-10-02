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
 * @file S32K148_ADC.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_ADC
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
#if !defined(S32K148_ADC_H_)  /* Check if memory map has not been already included */
#define S32K148_ADC_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- ADC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ADC_Peripheral_Access_Layer ADC Peripheral Access Layer
 * @{
 */

/** ADC - Size of Registers Arrays */
#define ADC_SC1_COUNT                             16u
#define ADC_R_COUNT                               16u
#define ADC_CV_COUNT                              2u
#define ADC_SC1ZZ_COUNT                           16u
#define ADC_RZZ_COUNT                             16u

/** ADC - Register Layout Typedef */
/**
 * Patch Accenture: SC1 and R array field names were used for extended SC1 and R registers,
 *                  the first (primary) register sets were represented as single fields.
 *                  Turned single fields into SC1 and R array fields and renamed
 *                  original SC1 and R fields into zzSC1 and zzR.
 *                  Added respective count definitions above.
*/
typedef struct {
  __IO uint32_t SC1[ADC_SC1_COUNT];                /**< ADC Status and Control Register 1, array offset: 0x0, array step: 0x4 */
  __IO uint32_t CFG1;                              /**< ADC Configuration Register 1, offset: 0x40 */
  __IO uint32_t CFG2;                              /**< ADC Configuration Register 2, offset: 0x44 */
  __I  uint32_t R[ADC_R_COUNT];                    /**< ADC Data Result Registers, array offset: 0x48, array step: 0x4 */
  __IO uint32_t CV[ADC_CV_COUNT];                  /**< Compare Value Registers, array offset: 0x88, array step: 0x4 */
  __IO uint32_t SC2;                               /**< Status and Control Register 2, offset: 0x90 */
  __IO uint32_t SC3;                               /**< Status and Control Register 3, offset: 0x94 */
  __IO uint32_t BASE_OFS;                          /**< BASE Offset Register, offset: 0x98 */
  __IO uint32_t OFS;                               /**< ADC Offset Correction Register, offset: 0x9C */
  __IO uint32_t USR_OFS;                           /**< USER Offset Correction Register, offset: 0xA0 */
  __IO uint32_t XOFS;                              /**< ADC X Offset Correction Register, offset: 0xA4 */
  __IO uint32_t YOFS;                              /**< ADC Y Offset Correction Register, offset: 0xA8 */
  __IO uint32_t G;                                 /**< ADC Gain Register, offset: 0xAC */
  __IO uint32_t UG;                                /**< ADC User Gain Register, offset: 0xB0 */
  __IO uint32_t CLPS;                              /**< ADC General Calibration Value Register S, offset: 0xB4 */
  __IO uint32_t CLP3;                              /**< ADC Plus-Side General Calibration Value Register 3, offset: 0xB8 */
  __IO uint32_t CLP2;                              /**< ADC Plus-Side General Calibration Value Register 2, offset: 0xBC */
  __IO uint32_t CLP1;                              /**< ADC Plus-Side General Calibration Value Register 1, offset: 0xC0 */
  __IO uint32_t CLP0;                              /**< ADC Plus-Side General Calibration Value Register 0, offset: 0xC4 */
  __IO uint32_t CLPX;                              /**< ADC Plus-Side General Calibration Value Register X, offset: 0xC8 */
  __IO uint32_t CLP9;                              /**< ADC Plus-Side General Calibration Value Register 9, offset: 0xCC */
  __IO uint32_t CLPS_OFS;                          /**< ADC General Calibration Offset Value Register S, offset: 0xD0 */
  __IO uint32_t CLP3_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register 3, offset: 0xD4 */
  __IO uint32_t CLP2_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register 2, offset: 0xD8 */
  __IO uint32_t CLP1_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register 1, offset: 0xDC */
  __IO uint32_t CLP0_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register 0, offset: 0xE0 */
  __IO uint32_t CLPX_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register X, offset: 0xE4 */
  __IO uint32_t CLP9_OFS;                          /**< ADC Plus-Side General Calibration Offset Value Register 9, offset: 0xE8 */
  uint8_t RESERVED_0[28];
  __IO uint32_t ASC1A;                             /**< ADC Status and Control Register 1, offset: 0x108 */
  __IO uint32_t ASC1B;                             /**< ADC Status and Control Register 1, offset: 0x10C */
  __IO uint32_t ASC1C;                             /**< ADC Status and Control Register 1, offset: 0x110 */
  __IO uint32_t ASC1D;                             /**< ADC Status and Control Register 1, offset: 0x114 */
  __IO uint32_t ASC1E;                             /**< ADC Status and Control Register 1, offset: 0x118 */
  __IO uint32_t ASC1F;                             /**< ADC Status and Control Register 1, offset: 0x11C */
  __IO uint32_t ASC1G;                             /**< ADC Status and Control Register 1, offset: 0x120 */
  __IO uint32_t ASC1H;                             /**< ADC Status and Control Register 1, offset: 0x124 */
  __IO uint32_t ASC1I;                             /**< ADC Status and Control Register 1, offset: 0x128 */
  __IO uint32_t ASC1J;                             /**< ADC Status and Control Register 1, offset: 0x12C */
  __IO uint32_t ASC1K;                             /**< ADC Status and Control Register 1, offset: 0x130 */
  __IO uint32_t ASC1L;                             /**< ADC Status and Control Register 1, offset: 0x134 */
  __IO uint32_t ASC1M;                             /**< ADC Status and Control Register 1, offset: 0x138 */
  __IO uint32_t ASC1N;                             /**< ADC Status and Control Register 1, offset: 0x13C */
  __IO uint32_t ASC1O;                             /**< ADC Status and Control Register 1, offset: 0x140 */
  __IO uint32_t ASC1P;                             /**< ADC Status and Control Register 1, offset: 0x144 */
  __IO uint32_t zzSC1[ADC_SC1ZZ_COUNT];            /**< ADC Status and Control Register 1, array offset: 0x148, array step: 0x4 */
  __I  uint32_t ARA;                               /**< ADC Data Result Registers, offset: 0x188 */
  __I  uint32_t ARB;                               /**< ADC Data Result Registers, offset: 0x18C */
  __I  uint32_t ARC;                               /**< ADC Data Result Registers, offset: 0x190 */
  __I  uint32_t ARD;                               /**< ADC Data Result Registers, offset: 0x194 */
  __I  uint32_t ARE;                               /**< ADC Data Result Registers, offset: 0x198 */
  __I  uint32_t ARF;                               /**< ADC Data Result Registers, offset: 0x19C */
  __I  uint32_t ARG;                               /**< ADC Data Result Registers, offset: 0x1A0 */
  __I  uint32_t ARH;                               /**< ADC Data Result Registers, offset: 0x1A4 */
  __I  uint32_t ARI;                               /**< ADC Data Result Registers, offset: 0x1A8 */
  __I  uint32_t ARJ;                               /**< ADC Data Result Registers, offset: 0x1AC */
  __I  uint32_t ARK;                               /**< ADC Data Result Registers, offset: 0x1B0 */
  __I  uint32_t ARL;                               /**< ADC Data Result Registers, offset: 0x1B4 */
  __I  uint32_t ARM;                               /**< ADC Data Result Registers, offset: 0x1B8 */
  __I  uint32_t ARN;                               /**< ADC Data Result Registers, offset: 0x1BC */
  __I  uint32_t ARO;                               /**< ADC Data Result Registers, offset: 0x1C0 */
  __I  uint32_t ARP;                               /**< ADC Data Result Registers, offset: 0x1C4 */
  __I  uint32_t zzR[ADC_RZZ_COUNT];                /**< ADC Data Result Registers, array offset: 0x1C8, array step: 0x4 */
} ADC_Type, *ADC_MemMapPtr;

/** Number of instances of the ADC module. */
#define ADC_INSTANCE_COUNT                       (2u)

/* ADC - Peripheral instance base addresses */
/** Peripheral ADC0 base address */
#define IP_ADC0_BASE                             (0x4003B000u)
/** Peripheral ADC0 base pointer */
#define IP_ADC0                                  ((ADC_Type *)IP_ADC0_BASE)
/** Peripheral ADC1 base address */
#define IP_ADC1_BASE                             (0x40027000u)
/** Peripheral ADC1 base pointer */
#define IP_ADC1                                  ((ADC_Type *)IP_ADC1_BASE)
/** Array initializer of ADC peripheral base addresses */
#define IP_ADC_BASE_ADDRS                        { IP_ADC0_BASE, IP_ADC1_BASE }
/** Array initializer of ADC peripheral base pointers */
#define IP_ADC_BASE_PTRS                         { IP_ADC0, IP_ADC1 }

/* ----------------------------------------------------------------------------
   -- ADC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ADC_Register_Masks ADC Register Masks
 * @{
 */

/*! @name SC1A - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1A_ADCH_MASK                       (0x3FU)
#define ADC_SC1A_ADCH_SHIFT                      (0U)
#define ADC_SC1A_ADCH_WIDTH                      (6U)
#define ADC_SC1A_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1A_ADCH_SHIFT)) & ADC_SC1A_ADCH_MASK)

#define ADC_SC1A_AIEN_MASK                       (0x40U)
#define ADC_SC1A_AIEN_SHIFT                      (6U)
#define ADC_SC1A_AIEN_WIDTH                      (1U)
#define ADC_SC1A_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1A_AIEN_SHIFT)) & ADC_SC1A_AIEN_MASK)

#define ADC_SC1A_COCO_MASK                       (0x80U)
#define ADC_SC1A_COCO_SHIFT                      (7U)
#define ADC_SC1A_COCO_WIDTH                      (1U)
#define ADC_SC1A_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1A_COCO_SHIFT)) & ADC_SC1A_COCO_MASK)
/*! @} */

/*! @name SC1B - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1B_ADCH_MASK                       (0x3FU)
#define ADC_SC1B_ADCH_SHIFT                      (0U)
#define ADC_SC1B_ADCH_WIDTH                      (6U)
#define ADC_SC1B_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1B_ADCH_SHIFT)) & ADC_SC1B_ADCH_MASK)

#define ADC_SC1B_AIEN_MASK                       (0x40U)
#define ADC_SC1B_AIEN_SHIFT                      (6U)
#define ADC_SC1B_AIEN_WIDTH                      (1U)
#define ADC_SC1B_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1B_AIEN_SHIFT)) & ADC_SC1B_AIEN_MASK)

#define ADC_SC1B_COCO_MASK                       (0x80U)
#define ADC_SC1B_COCO_SHIFT                      (7U)
#define ADC_SC1B_COCO_WIDTH                      (1U)
#define ADC_SC1B_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1B_COCO_SHIFT)) & ADC_SC1B_COCO_MASK)
/*! @} */

/*! @name SC1C - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1C_ADCH_MASK                       (0x3FU)
#define ADC_SC1C_ADCH_SHIFT                      (0U)
#define ADC_SC1C_ADCH_WIDTH                      (6U)
#define ADC_SC1C_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1C_ADCH_SHIFT)) & ADC_SC1C_ADCH_MASK)

#define ADC_SC1C_AIEN_MASK                       (0x40U)
#define ADC_SC1C_AIEN_SHIFT                      (6U)
#define ADC_SC1C_AIEN_WIDTH                      (1U)
#define ADC_SC1C_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1C_AIEN_SHIFT)) & ADC_SC1C_AIEN_MASK)

#define ADC_SC1C_COCO_MASK                       (0x80U)
#define ADC_SC1C_COCO_SHIFT                      (7U)
#define ADC_SC1C_COCO_WIDTH                      (1U)
#define ADC_SC1C_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1C_COCO_SHIFT)) & ADC_SC1C_COCO_MASK)
/*! @} */

/*! @name SC1D - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1D_ADCH_MASK                       (0x3FU)
#define ADC_SC1D_ADCH_SHIFT                      (0U)
#define ADC_SC1D_ADCH_WIDTH                      (6U)
#define ADC_SC1D_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1D_ADCH_SHIFT)) & ADC_SC1D_ADCH_MASK)

#define ADC_SC1D_AIEN_MASK                       (0x40U)
#define ADC_SC1D_AIEN_SHIFT                      (6U)
#define ADC_SC1D_AIEN_WIDTH                      (1U)
#define ADC_SC1D_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1D_AIEN_SHIFT)) & ADC_SC1D_AIEN_MASK)

#define ADC_SC1D_COCO_MASK                       (0x80U)
#define ADC_SC1D_COCO_SHIFT                      (7U)
#define ADC_SC1D_COCO_WIDTH                      (1U)
#define ADC_SC1D_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1D_COCO_SHIFT)) & ADC_SC1D_COCO_MASK)
/*! @} */

/*! @name SC1E - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1E_ADCH_MASK                       (0x3FU)
#define ADC_SC1E_ADCH_SHIFT                      (0U)
#define ADC_SC1E_ADCH_WIDTH                      (6U)
#define ADC_SC1E_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1E_ADCH_SHIFT)) & ADC_SC1E_ADCH_MASK)

#define ADC_SC1E_AIEN_MASK                       (0x40U)
#define ADC_SC1E_AIEN_SHIFT                      (6U)
#define ADC_SC1E_AIEN_WIDTH                      (1U)
#define ADC_SC1E_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1E_AIEN_SHIFT)) & ADC_SC1E_AIEN_MASK)

#define ADC_SC1E_COCO_MASK                       (0x80U)
#define ADC_SC1E_COCO_SHIFT                      (7U)
#define ADC_SC1E_COCO_WIDTH                      (1U)
#define ADC_SC1E_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1E_COCO_SHIFT)) & ADC_SC1E_COCO_MASK)
/*! @} */

/*! @name SC1F - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1F_ADCH_MASK                       (0x3FU)
#define ADC_SC1F_ADCH_SHIFT                      (0U)
#define ADC_SC1F_ADCH_WIDTH                      (6U)
#define ADC_SC1F_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1F_ADCH_SHIFT)) & ADC_SC1F_ADCH_MASK)

#define ADC_SC1F_AIEN_MASK                       (0x40U)
#define ADC_SC1F_AIEN_SHIFT                      (6U)
#define ADC_SC1F_AIEN_WIDTH                      (1U)
#define ADC_SC1F_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1F_AIEN_SHIFT)) & ADC_SC1F_AIEN_MASK)

#define ADC_SC1F_COCO_MASK                       (0x80U)
#define ADC_SC1F_COCO_SHIFT                      (7U)
#define ADC_SC1F_COCO_WIDTH                      (1U)
#define ADC_SC1F_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1F_COCO_SHIFT)) & ADC_SC1F_COCO_MASK)
/*! @} */

/*! @name SC1G - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1G_ADCH_MASK                       (0x3FU)
#define ADC_SC1G_ADCH_SHIFT                      (0U)
#define ADC_SC1G_ADCH_WIDTH                      (6U)
#define ADC_SC1G_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1G_ADCH_SHIFT)) & ADC_SC1G_ADCH_MASK)

#define ADC_SC1G_AIEN_MASK                       (0x40U)
#define ADC_SC1G_AIEN_SHIFT                      (6U)
#define ADC_SC1G_AIEN_WIDTH                      (1U)
#define ADC_SC1G_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1G_AIEN_SHIFT)) & ADC_SC1G_AIEN_MASK)

#define ADC_SC1G_COCO_MASK                       (0x80U)
#define ADC_SC1G_COCO_SHIFT                      (7U)
#define ADC_SC1G_COCO_WIDTH                      (1U)
#define ADC_SC1G_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1G_COCO_SHIFT)) & ADC_SC1G_COCO_MASK)
/*! @} */

/*! @name SC1H - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1H_ADCH_MASK                       (0x3FU)
#define ADC_SC1H_ADCH_SHIFT                      (0U)
#define ADC_SC1H_ADCH_WIDTH                      (6U)
#define ADC_SC1H_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1H_ADCH_SHIFT)) & ADC_SC1H_ADCH_MASK)

#define ADC_SC1H_AIEN_MASK                       (0x40U)
#define ADC_SC1H_AIEN_SHIFT                      (6U)
#define ADC_SC1H_AIEN_WIDTH                      (1U)
#define ADC_SC1H_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1H_AIEN_SHIFT)) & ADC_SC1H_AIEN_MASK)

#define ADC_SC1H_COCO_MASK                       (0x80U)
#define ADC_SC1H_COCO_SHIFT                      (7U)
#define ADC_SC1H_COCO_WIDTH                      (1U)
#define ADC_SC1H_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1H_COCO_SHIFT)) & ADC_SC1H_COCO_MASK)
/*! @} */

/*! @name SC1I - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1I_ADCH_MASK                       (0x3FU)
#define ADC_SC1I_ADCH_SHIFT                      (0U)
#define ADC_SC1I_ADCH_WIDTH                      (6U)
#define ADC_SC1I_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1I_ADCH_SHIFT)) & ADC_SC1I_ADCH_MASK)

#define ADC_SC1I_AIEN_MASK                       (0x40U)
#define ADC_SC1I_AIEN_SHIFT                      (6U)
#define ADC_SC1I_AIEN_WIDTH                      (1U)
#define ADC_SC1I_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1I_AIEN_SHIFT)) & ADC_SC1I_AIEN_MASK)

#define ADC_SC1I_COCO_MASK                       (0x80U)
#define ADC_SC1I_COCO_SHIFT                      (7U)
#define ADC_SC1I_COCO_WIDTH                      (1U)
#define ADC_SC1I_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1I_COCO_SHIFT)) & ADC_SC1I_COCO_MASK)
/*! @} */

/*! @name SC1J - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1J_ADCH_MASK                       (0x3FU)
#define ADC_SC1J_ADCH_SHIFT                      (0U)
#define ADC_SC1J_ADCH_WIDTH                      (6U)
#define ADC_SC1J_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1J_ADCH_SHIFT)) & ADC_SC1J_ADCH_MASK)

#define ADC_SC1J_AIEN_MASK                       (0x40U)
#define ADC_SC1J_AIEN_SHIFT                      (6U)
#define ADC_SC1J_AIEN_WIDTH                      (1U)
#define ADC_SC1J_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1J_AIEN_SHIFT)) & ADC_SC1J_AIEN_MASK)

#define ADC_SC1J_COCO_MASK                       (0x80U)
#define ADC_SC1J_COCO_SHIFT                      (7U)
#define ADC_SC1J_COCO_WIDTH                      (1U)
#define ADC_SC1J_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1J_COCO_SHIFT)) & ADC_SC1J_COCO_MASK)
/*! @} */

/*! @name SC1K - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1K_ADCH_MASK                       (0x3FU)
#define ADC_SC1K_ADCH_SHIFT                      (0U)
#define ADC_SC1K_ADCH_WIDTH                      (6U)
#define ADC_SC1K_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1K_ADCH_SHIFT)) & ADC_SC1K_ADCH_MASK)

#define ADC_SC1K_AIEN_MASK                       (0x40U)
#define ADC_SC1K_AIEN_SHIFT                      (6U)
#define ADC_SC1K_AIEN_WIDTH                      (1U)
#define ADC_SC1K_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1K_AIEN_SHIFT)) & ADC_SC1K_AIEN_MASK)

#define ADC_SC1K_COCO_MASK                       (0x80U)
#define ADC_SC1K_COCO_SHIFT                      (7U)
#define ADC_SC1K_COCO_WIDTH                      (1U)
#define ADC_SC1K_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1K_COCO_SHIFT)) & ADC_SC1K_COCO_MASK)
/*! @} */

/*! @name SC1L - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1L_ADCH_MASK                       (0x3FU)
#define ADC_SC1L_ADCH_SHIFT                      (0U)
#define ADC_SC1L_ADCH_WIDTH                      (6U)
#define ADC_SC1L_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1L_ADCH_SHIFT)) & ADC_SC1L_ADCH_MASK)

#define ADC_SC1L_AIEN_MASK                       (0x40U)
#define ADC_SC1L_AIEN_SHIFT                      (6U)
#define ADC_SC1L_AIEN_WIDTH                      (1U)
#define ADC_SC1L_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1L_AIEN_SHIFT)) & ADC_SC1L_AIEN_MASK)

#define ADC_SC1L_COCO_MASK                       (0x80U)
#define ADC_SC1L_COCO_SHIFT                      (7U)
#define ADC_SC1L_COCO_WIDTH                      (1U)
#define ADC_SC1L_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1L_COCO_SHIFT)) & ADC_SC1L_COCO_MASK)
/*! @} */

/*! @name SC1M - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1M_ADCH_MASK                       (0x3FU)
#define ADC_SC1M_ADCH_SHIFT                      (0U)
#define ADC_SC1M_ADCH_WIDTH                      (6U)
#define ADC_SC1M_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1M_ADCH_SHIFT)) & ADC_SC1M_ADCH_MASK)

#define ADC_SC1M_AIEN_MASK                       (0x40U)
#define ADC_SC1M_AIEN_SHIFT                      (6U)
#define ADC_SC1M_AIEN_WIDTH                      (1U)
#define ADC_SC1M_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1M_AIEN_SHIFT)) & ADC_SC1M_AIEN_MASK)

#define ADC_SC1M_COCO_MASK                       (0x80U)
#define ADC_SC1M_COCO_SHIFT                      (7U)
#define ADC_SC1M_COCO_WIDTH                      (1U)
#define ADC_SC1M_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1M_COCO_SHIFT)) & ADC_SC1M_COCO_MASK)
/*! @} */

/*! @name SC1N - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1N_ADCH_MASK                       (0x3FU)
#define ADC_SC1N_ADCH_SHIFT                      (0U)
#define ADC_SC1N_ADCH_WIDTH                      (6U)
#define ADC_SC1N_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1N_ADCH_SHIFT)) & ADC_SC1N_ADCH_MASK)

#define ADC_SC1N_AIEN_MASK                       (0x40U)
#define ADC_SC1N_AIEN_SHIFT                      (6U)
#define ADC_SC1N_AIEN_WIDTH                      (1U)
#define ADC_SC1N_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1N_AIEN_SHIFT)) & ADC_SC1N_AIEN_MASK)

#define ADC_SC1N_COCO_MASK                       (0x80U)
#define ADC_SC1N_COCO_SHIFT                      (7U)
#define ADC_SC1N_COCO_WIDTH                      (1U)
#define ADC_SC1N_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1N_COCO_SHIFT)) & ADC_SC1N_COCO_MASK)
/*! @} */

/*! @name SC1O - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1O_ADCH_MASK                       (0x3FU)
#define ADC_SC1O_ADCH_SHIFT                      (0U)
#define ADC_SC1O_ADCH_WIDTH                      (6U)
#define ADC_SC1O_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1O_ADCH_SHIFT)) & ADC_SC1O_ADCH_MASK)

#define ADC_SC1O_AIEN_MASK                       (0x40U)
#define ADC_SC1O_AIEN_SHIFT                      (6U)
#define ADC_SC1O_AIEN_WIDTH                      (1U)
#define ADC_SC1O_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1O_AIEN_SHIFT)) & ADC_SC1O_AIEN_MASK)

#define ADC_SC1O_COCO_MASK                       (0x80U)
#define ADC_SC1O_COCO_SHIFT                      (7U)
#define ADC_SC1O_COCO_WIDTH                      (1U)
#define ADC_SC1O_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1O_COCO_SHIFT)) & ADC_SC1O_COCO_MASK)
/*! @} */

/*! @name SC1P - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1P_ADCH_MASK                       (0x3FU)
#define ADC_SC1P_ADCH_SHIFT                      (0U)
#define ADC_SC1P_ADCH_WIDTH                      (6U)
#define ADC_SC1P_ADCH(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1P_ADCH_SHIFT)) & ADC_SC1P_ADCH_MASK)

#define ADC_SC1P_AIEN_MASK                       (0x40U)
#define ADC_SC1P_AIEN_SHIFT                      (6U)
#define ADC_SC1P_AIEN_WIDTH                      (1U)
#define ADC_SC1P_AIEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1P_AIEN_SHIFT)) & ADC_SC1P_AIEN_MASK)

#define ADC_SC1P_COCO_MASK                       (0x80U)
#define ADC_SC1P_COCO_SHIFT                      (7U)
#define ADC_SC1P_COCO_WIDTH                      (1U)
#define ADC_SC1P_COCO(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC1P_COCO_SHIFT)) & ADC_SC1P_COCO_MASK)
/*! @} */

/*! @name CFG1 - ADC Configuration Register 1 */
/*! @{ */

#define ADC_CFG1_ADICLK_MASK                     (0x3U)
#define ADC_CFG1_ADICLK_SHIFT                    (0U)
#define ADC_CFG1_ADICLK_WIDTH                    (2U)
#define ADC_CFG1_ADICLK(x)                       (((uint32_t)(((uint32_t)(x)) << ADC_CFG1_ADICLK_SHIFT)) & ADC_CFG1_ADICLK_MASK)

#define ADC_CFG1_MODE_MASK                       (0xCU)
#define ADC_CFG1_MODE_SHIFT                      (2U)
#define ADC_CFG1_MODE_WIDTH                      (2U)
#define ADC_CFG1_MODE(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CFG1_MODE_SHIFT)) & ADC_CFG1_MODE_MASK)

#define ADC_CFG1_ADIV_MASK                       (0x60U)
#define ADC_CFG1_ADIV_SHIFT                      (5U)
#define ADC_CFG1_ADIV_WIDTH                      (2U)
#define ADC_CFG1_ADIV(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CFG1_ADIV_SHIFT)) & ADC_CFG1_ADIV_MASK)

#define ADC_CFG1_CLRLTRG_MASK                    (0x100U)
#define ADC_CFG1_CLRLTRG_SHIFT                   (8U)
#define ADC_CFG1_CLRLTRG_WIDTH                   (1U)
#define ADC_CFG1_CLRLTRG(x)                      (((uint32_t)(((uint32_t)(x)) << ADC_CFG1_CLRLTRG_SHIFT)) & ADC_CFG1_CLRLTRG_MASK)
/*! @} */

/*! @name CFG2 - ADC Configuration Register 2 */
/*! @{ */

#define ADC_CFG2_SMPLTS_MASK                     (0xFFU)
#define ADC_CFG2_SMPLTS_SHIFT                    (0U)
#define ADC_CFG2_SMPLTS_WIDTH                    (8U)
#define ADC_CFG2_SMPLTS(x)                       (((uint32_t)(((uint32_t)(x)) << ADC_CFG2_SMPLTS_SHIFT)) & ADC_CFG2_SMPLTS_MASK)
/*! @} */

/*! @name RA - ADC Data Result Registers */
/*! @{ */

#define ADC_RA_D_MASK                            (0xFFFU)
#define ADC_RA_D_SHIFT                           (0U)
#define ADC_RA_D_WIDTH                           (12U)
#define ADC_RA_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RA_D_SHIFT)) & ADC_RA_D_MASK)
/*! @} */

/*! @name RB - ADC Data Result Registers */
/*! @{ */

#define ADC_RB_D_MASK                            (0xFFFU)
#define ADC_RB_D_SHIFT                           (0U)
#define ADC_RB_D_WIDTH                           (12U)
#define ADC_RB_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RB_D_SHIFT)) & ADC_RB_D_MASK)
/*! @} */

/*! @name RC - ADC Data Result Registers */
/*! @{ */

#define ADC_RC_D_MASK                            (0xFFFU)
#define ADC_RC_D_SHIFT                           (0U)
#define ADC_RC_D_WIDTH                           (12U)
#define ADC_RC_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RC_D_SHIFT)) & ADC_RC_D_MASK)
/*! @} */

/*! @name RD - ADC Data Result Registers */
/*! @{ */

#define ADC_RD_D_MASK                            (0xFFFU)
#define ADC_RD_D_SHIFT                           (0U)
#define ADC_RD_D_WIDTH                           (12U)
#define ADC_RD_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RD_D_SHIFT)) & ADC_RD_D_MASK)
/*! @} */

/*! @name RE - ADC Data Result Registers */
/*! @{ */

#define ADC_RE_D_MASK                            (0xFFFU)
#define ADC_RE_D_SHIFT                           (0U)
#define ADC_RE_D_WIDTH                           (12U)
#define ADC_RE_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RE_D_SHIFT)) & ADC_RE_D_MASK)
/*! @} */

/*! @name RF - ADC Data Result Registers */
/*! @{ */

#define ADC_RF_D_MASK                            (0xFFFU)
#define ADC_RF_D_SHIFT                           (0U)
#define ADC_RF_D_WIDTH                           (12U)
#define ADC_RF_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RF_D_SHIFT)) & ADC_RF_D_MASK)
/*! @} */

/*! @name RG - ADC Data Result Registers */
/*! @{ */

#define ADC_RG_D_MASK                            (0xFFFU)
#define ADC_RG_D_SHIFT                           (0U)
#define ADC_RG_D_WIDTH                           (12U)
#define ADC_RG_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RG_D_SHIFT)) & ADC_RG_D_MASK)
/*! @} */

/*! @name RH - ADC Data Result Registers */
/*! @{ */

#define ADC_RH_D_MASK                            (0xFFFU)
#define ADC_RH_D_SHIFT                           (0U)
#define ADC_RH_D_WIDTH                           (12U)
#define ADC_RH_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RH_D_SHIFT)) & ADC_RH_D_MASK)
/*! @} */

/*! @name RI - ADC Data Result Registers */
/*! @{ */

#define ADC_RI_D_MASK                            (0xFFFU)
#define ADC_RI_D_SHIFT                           (0U)
#define ADC_RI_D_WIDTH                           (12U)
#define ADC_RI_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RI_D_SHIFT)) & ADC_RI_D_MASK)
/*! @} */

/*! @name RJ - ADC Data Result Registers */
/*! @{ */

#define ADC_RJ_D_MASK                            (0xFFFU)
#define ADC_RJ_D_SHIFT                           (0U)
#define ADC_RJ_D_WIDTH                           (12U)
#define ADC_RJ_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RJ_D_SHIFT)) & ADC_RJ_D_MASK)
/*! @} */

/*! @name RK - ADC Data Result Registers */
/*! @{ */

#define ADC_RK_D_MASK                            (0xFFFU)
#define ADC_RK_D_SHIFT                           (0U)
#define ADC_RK_D_WIDTH                           (12U)
#define ADC_RK_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RK_D_SHIFT)) & ADC_RK_D_MASK)
/*! @} */

/*! @name RL - ADC Data Result Registers */
/*! @{ */

#define ADC_RL_D_MASK                            (0xFFFU)
#define ADC_RL_D_SHIFT                           (0U)
#define ADC_RL_D_WIDTH                           (12U)
#define ADC_RL_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RL_D_SHIFT)) & ADC_RL_D_MASK)
/*! @} */

/*! @name RM - ADC Data Result Registers */
/*! @{ */

#define ADC_RM_D_MASK                            (0xFFFU)
#define ADC_RM_D_SHIFT                           (0U)
#define ADC_RM_D_WIDTH                           (12U)
#define ADC_RM_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RM_D_SHIFT)) & ADC_RM_D_MASK)
/*! @} */

/*! @name RN - ADC Data Result Registers */
/*! @{ */

#define ADC_RN_D_MASK                            (0xFFFU)
#define ADC_RN_D_SHIFT                           (0U)
#define ADC_RN_D_WIDTH                           (12U)
#define ADC_RN_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RN_D_SHIFT)) & ADC_RN_D_MASK)
/*! @} */

/*! @name RO - ADC Data Result Registers */
/*! @{ */

#define ADC_RO_D_MASK                            (0xFFFU)
#define ADC_RO_D_SHIFT                           (0U)
#define ADC_RO_D_WIDTH                           (12U)
#define ADC_RO_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RO_D_SHIFT)) & ADC_RO_D_MASK)
/*! @} */

/*! @name RP - ADC Data Result Registers */
/*! @{ */

#define ADC_RP_D_MASK                            (0xFFFU)
#define ADC_RP_D_SHIFT                           (0U)
#define ADC_RP_D_WIDTH                           (12U)
#define ADC_RP_D(x)                              (((uint32_t)(((uint32_t)(x)) << ADC_RP_D_SHIFT)) & ADC_RP_D_MASK)
/*! @} */

/*! @name CV - Compare Value Registers */
/*! @{ */

#define ADC_CV_CV_MASK                           (0xFFFFU)
#define ADC_CV_CV_SHIFT                          (0U)
#define ADC_CV_CV_WIDTH                          (16U)
#define ADC_CV_CV(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_CV_CV_SHIFT)) & ADC_CV_CV_MASK)
/*! @} */

/*! @name SC2 - Status and Control Register 2 */
/*! @{ */

#define ADC_SC2_REFSEL_MASK                      (0x3U)
#define ADC_SC2_REFSEL_SHIFT                     (0U)
#define ADC_SC2_REFSEL_WIDTH                     (2U)
#define ADC_SC2_REFSEL(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_SC2_REFSEL_SHIFT)) & ADC_SC2_REFSEL_MASK)

#define ADC_SC2_DMAEN_MASK                       (0x4U)
#define ADC_SC2_DMAEN_SHIFT                      (2U)
#define ADC_SC2_DMAEN_WIDTH                      (1U)
#define ADC_SC2_DMAEN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC2_DMAEN_SHIFT)) & ADC_SC2_DMAEN_MASK)

#define ADC_SC2_ACREN_MASK                       (0x8U)
#define ADC_SC2_ACREN_SHIFT                      (3U)
#define ADC_SC2_ACREN_WIDTH                      (1U)
#define ADC_SC2_ACREN(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC2_ACREN_SHIFT)) & ADC_SC2_ACREN_MASK)

#define ADC_SC2_ACFGT_MASK                       (0x10U)
#define ADC_SC2_ACFGT_SHIFT                      (4U)
#define ADC_SC2_ACFGT_WIDTH                      (1U)
#define ADC_SC2_ACFGT(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC2_ACFGT_SHIFT)) & ADC_SC2_ACFGT_MASK)

#define ADC_SC2_ACFE_MASK                        (0x20U)
#define ADC_SC2_ACFE_SHIFT                       (5U)
#define ADC_SC2_ACFE_WIDTH                       (1U)
#define ADC_SC2_ACFE(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC2_ACFE_SHIFT)) & ADC_SC2_ACFE_MASK)

#define ADC_SC2_ADTRG_MASK                       (0x40U)
#define ADC_SC2_ADTRG_SHIFT                      (6U)
#define ADC_SC2_ADTRG_WIDTH                      (1U)
#define ADC_SC2_ADTRG(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC2_ADTRG_SHIFT)) & ADC_SC2_ADTRG_MASK)

#define ADC_SC2_ADACT_MASK                       (0x80U)
#define ADC_SC2_ADACT_SHIFT                      (7U)
#define ADC_SC2_ADACT_WIDTH                      (1U)
#define ADC_SC2_ADACT(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_SC2_ADACT_SHIFT)) & ADC_SC2_ADACT_MASK)

#define ADC_SC2_TRGPRNUM_MASK                    (0x6000U)
#define ADC_SC2_TRGPRNUM_SHIFT                   (13U)
#define ADC_SC2_TRGPRNUM_WIDTH                   (2U)
#define ADC_SC2_TRGPRNUM(x)                      (((uint32_t)(((uint32_t)(x)) << ADC_SC2_TRGPRNUM_SHIFT)) & ADC_SC2_TRGPRNUM_MASK)

#define ADC_SC2_TRGSTLAT_MASK                    (0xF0000U)
#define ADC_SC2_TRGSTLAT_SHIFT                   (16U)
#define ADC_SC2_TRGSTLAT_WIDTH                   (4U)
#define ADC_SC2_TRGSTLAT(x)                      (((uint32_t)(((uint32_t)(x)) << ADC_SC2_TRGSTLAT_SHIFT)) & ADC_SC2_TRGSTLAT_MASK)

#define ADC_SC2_TRGSTERR_MASK                    (0xF000000U)
#define ADC_SC2_TRGSTERR_SHIFT                   (24U)
#define ADC_SC2_TRGSTERR_WIDTH                   (4U)
#define ADC_SC2_TRGSTERR(x)                      (((uint32_t)(((uint32_t)(x)) << ADC_SC2_TRGSTERR_SHIFT)) & ADC_SC2_TRGSTERR_MASK)
/*! @} */

/*! @name SC3 - Status and Control Register 3 */
/*! @{ */

#define ADC_SC3_AVGS_MASK                        (0x3U)
#define ADC_SC3_AVGS_SHIFT                       (0U)
#define ADC_SC3_AVGS_WIDTH                       (2U)
#define ADC_SC3_AVGS(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC3_AVGS_SHIFT)) & ADC_SC3_AVGS_MASK)

#define ADC_SC3_AVGE_MASK                        (0x4U)
#define ADC_SC3_AVGE_SHIFT                       (2U)
#define ADC_SC3_AVGE_WIDTH                       (1U)
#define ADC_SC3_AVGE(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC3_AVGE_SHIFT)) & ADC_SC3_AVGE_MASK)

#define ADC_SC3_ADCO_MASK                        (0x8U)
#define ADC_SC3_ADCO_SHIFT                       (3U)
#define ADC_SC3_ADCO_WIDTH                       (1U)
#define ADC_SC3_ADCO(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC3_ADCO_SHIFT)) & ADC_SC3_ADCO_MASK)

#define ADC_SC3_CAL_MASK                         (0x80U)
#define ADC_SC3_CAL_SHIFT                        (7U)
#define ADC_SC3_CAL_WIDTH                        (1U)
#define ADC_SC3_CAL(x)                           (((uint32_t)(((uint32_t)(x)) << ADC_SC3_CAL_SHIFT)) & ADC_SC3_CAL_MASK)
/*! @} */

/*! @name BASE_OFS - BASE Offset Register */
/*! @{ */

#define ADC_BASE_OFS_BA_OFS_MASK                 (0xFFU)
#define ADC_BASE_OFS_BA_OFS_SHIFT                (0U)
#define ADC_BASE_OFS_BA_OFS_WIDTH                (8U)
#define ADC_BASE_OFS_BA_OFS(x)                   (((uint32_t)(((uint32_t)(x)) << ADC_BASE_OFS_BA_OFS_SHIFT)) & ADC_BASE_OFS_BA_OFS_MASK)
/*! @} */

/*! @name OFS - ADC Offset Correction Register */
/*! @{ */

#define ADC_OFS_OFS_MASK                         (0xFFFFU)
#define ADC_OFS_OFS_SHIFT                        (0U)
#define ADC_OFS_OFS_WIDTH                        (16U)
#define ADC_OFS_OFS(x)                           (((uint32_t)(((uint32_t)(x)) << ADC_OFS_OFS_SHIFT)) & ADC_OFS_OFS_MASK)
/*! @} */

/*! @name USR_OFS - USER Offset Correction Register */
/*! @{ */

#define ADC_USR_OFS_USR_OFS_MASK                 (0xFFU)
#define ADC_USR_OFS_USR_OFS_SHIFT                (0U)
#define ADC_USR_OFS_USR_OFS_WIDTH                (8U)
#define ADC_USR_OFS_USR_OFS(x)                   (((uint32_t)(((uint32_t)(x)) << ADC_USR_OFS_USR_OFS_SHIFT)) & ADC_USR_OFS_USR_OFS_MASK)
/*! @} */

/*! @name XOFS - ADC X Offset Correction Register */
/*! @{ */

#define ADC_XOFS_XOFS_MASK                       (0x3FU)
#define ADC_XOFS_XOFS_SHIFT                      (0U)
#define ADC_XOFS_XOFS_WIDTH                      (6U)
#define ADC_XOFS_XOFS(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_XOFS_XOFS_SHIFT)) & ADC_XOFS_XOFS_MASK)
/*! @} */

/*! @name YOFS - ADC Y Offset Correction Register */
/*! @{ */

#define ADC_YOFS_YOFS_MASK                       (0xFFU)
#define ADC_YOFS_YOFS_SHIFT                      (0U)
#define ADC_YOFS_YOFS_WIDTH                      (8U)
#define ADC_YOFS_YOFS(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_YOFS_YOFS_SHIFT)) & ADC_YOFS_YOFS_MASK)
/*! @} */

/*! @name G - ADC Gain Register */
/*! @{ */

#define ADC_G_G_MASK                             (0x7FFU)
#define ADC_G_G_SHIFT                            (0U)
#define ADC_G_G_WIDTH                            (11U)
#define ADC_G_G(x)                               (((uint32_t)(((uint32_t)(x)) << ADC_G_G_SHIFT)) & ADC_G_G_MASK)
/*! @} */

/*! @name UG - ADC User Gain Register */
/*! @{ */

#define ADC_UG_UG_MASK                           (0x3FFU)
#define ADC_UG_UG_SHIFT                          (0U)
#define ADC_UG_UG_WIDTH                          (10U)
#define ADC_UG_UG(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_UG_UG_SHIFT)) & ADC_UG_UG_MASK)
/*! @} */

/*! @name CLPS - ADC General Calibration Value Register S */
/*! @{ */

#define ADC_CLPS_CLPS_MASK                       (0x7FU)
#define ADC_CLPS_CLPS_SHIFT                      (0U)
#define ADC_CLPS_CLPS_WIDTH                      (7U)
#define ADC_CLPS_CLPS(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLPS_CLPS_SHIFT)) & ADC_CLPS_CLPS_MASK)
/*! @} */

/*! @name CLP3 - ADC Plus-Side General Calibration Value Register 3 */
/*! @{ */

#define ADC_CLP3_CLP3_MASK                       (0x3FFU)
#define ADC_CLP3_CLP3_SHIFT                      (0U)
#define ADC_CLP3_CLP3_WIDTH                      (10U)
#define ADC_CLP3_CLP3(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLP3_CLP3_SHIFT)) & ADC_CLP3_CLP3_MASK)
/*! @} */

/*! @name CLP2 - ADC Plus-Side General Calibration Value Register 2 */
/*! @{ */

#define ADC_CLP2_CLP2_MASK                       (0x3FFU)
#define ADC_CLP2_CLP2_SHIFT                      (0U)
#define ADC_CLP2_CLP2_WIDTH                      (10U)
#define ADC_CLP2_CLP2(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLP2_CLP2_SHIFT)) & ADC_CLP2_CLP2_MASK)
/*! @} */

/*! @name CLP1 - ADC Plus-Side General Calibration Value Register 1 */
/*! @{ */

#define ADC_CLP1_CLP1_MASK                       (0x1FFU)
#define ADC_CLP1_CLP1_SHIFT                      (0U)
#define ADC_CLP1_CLP1_WIDTH                      (9U)
#define ADC_CLP1_CLP1(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLP1_CLP1_SHIFT)) & ADC_CLP1_CLP1_MASK)
/*! @} */

/*! @name CLP0 - ADC Plus-Side General Calibration Value Register 0 */
/*! @{ */

#define ADC_CLP0_CLP0_MASK                       (0xFFU)
#define ADC_CLP0_CLP0_SHIFT                      (0U)
#define ADC_CLP0_CLP0_WIDTH                      (8U)
#define ADC_CLP0_CLP0(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLP0_CLP0_SHIFT)) & ADC_CLP0_CLP0_MASK)
/*! @} */

/*! @name CLPX - ADC Plus-Side General Calibration Value Register X */
/*! @{ */

#define ADC_CLPX_CLPX_MASK                       (0x7FU)
#define ADC_CLPX_CLPX_SHIFT                      (0U)
#define ADC_CLPX_CLPX_WIDTH                      (7U)
#define ADC_CLPX_CLPX(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLPX_CLPX_SHIFT)) & ADC_CLPX_CLPX_MASK)
/*! @} */

/*! @name CLP9 - ADC Plus-Side General Calibration Value Register 9 */
/*! @{ */

#define ADC_CLP9_CLP9_MASK                       (0x7FU)
#define ADC_CLP9_CLP9_SHIFT                      (0U)
#define ADC_CLP9_CLP9_WIDTH                      (7U)
#define ADC_CLP9_CLP9(x)                         (((uint32_t)(((uint32_t)(x)) << ADC_CLP9_CLP9_SHIFT)) & ADC_CLP9_CLP9_MASK)
/*! @} */

/*! @name CLPS_OFS - ADC General Calibration Offset Value Register S */
/*! @{ */

#define ADC_CLPS_OFS_CLPS_OFS_MASK               (0xFU)
#define ADC_CLPS_OFS_CLPS_OFS_SHIFT              (0U)
#define ADC_CLPS_OFS_CLPS_OFS_WIDTH              (4U)
#define ADC_CLPS_OFS_CLPS_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLPS_OFS_CLPS_OFS_SHIFT)) & ADC_CLPS_OFS_CLPS_OFS_MASK)
/*! @} */

/*! @name CLP3_OFS - ADC Plus-Side General Calibration Offset Value Register 3 */
/*! @{ */

#define ADC_CLP3_OFS_CLP3_OFS_MASK               (0xFU)
#define ADC_CLP3_OFS_CLP3_OFS_SHIFT              (0U)
#define ADC_CLP3_OFS_CLP3_OFS_WIDTH              (4U)
#define ADC_CLP3_OFS_CLP3_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLP3_OFS_CLP3_OFS_SHIFT)) & ADC_CLP3_OFS_CLP3_OFS_MASK)
/*! @} */

/*! @name CLP2_OFS - ADC Plus-Side General Calibration Offset Value Register 2 */
/*! @{ */

#define ADC_CLP2_OFS_CLP2_OFS_MASK               (0xFU)
#define ADC_CLP2_OFS_CLP2_OFS_SHIFT              (0U)
#define ADC_CLP2_OFS_CLP2_OFS_WIDTH              (4U)
#define ADC_CLP2_OFS_CLP2_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLP2_OFS_CLP2_OFS_SHIFT)) & ADC_CLP2_OFS_CLP2_OFS_MASK)
/*! @} */

/*! @name CLP1_OFS - ADC Plus-Side General Calibration Offset Value Register 1 */
/*! @{ */

#define ADC_CLP1_OFS_CLP1_OFS_MASK               (0xFU)
#define ADC_CLP1_OFS_CLP1_OFS_SHIFT              (0U)
#define ADC_CLP1_OFS_CLP1_OFS_WIDTH              (4U)
#define ADC_CLP1_OFS_CLP1_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLP1_OFS_CLP1_OFS_SHIFT)) & ADC_CLP1_OFS_CLP1_OFS_MASK)
/*! @} */

/*! @name CLP0_OFS - ADC Plus-Side General Calibration Offset Value Register 0 */
/*! @{ */

#define ADC_CLP0_OFS_CLP0_OFS_MASK               (0xFU)
#define ADC_CLP0_OFS_CLP0_OFS_SHIFT              (0U)
#define ADC_CLP0_OFS_CLP0_OFS_WIDTH              (4U)
#define ADC_CLP0_OFS_CLP0_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLP0_OFS_CLP0_OFS_SHIFT)) & ADC_CLP0_OFS_CLP0_OFS_MASK)
/*! @} */

/*! @name CLPX_OFS - ADC Plus-Side General Calibration Offset Value Register X */
/*! @{ */

#define ADC_CLPX_OFS_CLPX_OFS_MASK               (0xFFFU)
#define ADC_CLPX_OFS_CLPX_OFS_SHIFT              (0U)
#define ADC_CLPX_OFS_CLPX_OFS_WIDTH              (12U)
#define ADC_CLPX_OFS_CLPX_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLPX_OFS_CLPX_OFS_SHIFT)) & ADC_CLPX_OFS_CLPX_OFS_MASK)
/*! @} */

/*! @name CLP9_OFS - ADC Plus-Side General Calibration Offset Value Register 9 */
/*! @{ */

#define ADC_CLP9_OFS_CLP9_OFS_MASK               (0xFFFU)
#define ADC_CLP9_OFS_CLP9_OFS_SHIFT              (0U)
#define ADC_CLP9_OFS_CLP9_OFS_WIDTH              (12U)
#define ADC_CLP9_OFS_CLP9_OFS(x)                 (((uint32_t)(((uint32_t)(x)) << ADC_CLP9_OFS_CLP9_OFS_SHIFT)) & ADC_CLP9_OFS_CLP9_OFS_MASK)
/*! @} */

/*! @name ASC1A - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1A_ADCH_MASK                      (0x3FU)
#define ADC_ASC1A_ADCH_SHIFT                     (0U)
#define ADC_ASC1A_ADCH_WIDTH                     (6U)
#define ADC_ASC1A_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1A_ADCH_SHIFT)) & ADC_ASC1A_ADCH_MASK)

#define ADC_ASC1A_AIEN_MASK                      (0x40U)
#define ADC_ASC1A_AIEN_SHIFT                     (6U)
#define ADC_ASC1A_AIEN_WIDTH                     (1U)
#define ADC_ASC1A_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1A_AIEN_SHIFT)) & ADC_ASC1A_AIEN_MASK)

#define ADC_ASC1A_COCO_MASK                      (0x80U)
#define ADC_ASC1A_COCO_SHIFT                     (7U)
#define ADC_ASC1A_COCO_WIDTH                     (1U)
#define ADC_ASC1A_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1A_COCO_SHIFT)) & ADC_ASC1A_COCO_MASK)
/*! @} */

/*! @name ASC1B - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1B_ADCH_MASK                      (0x3FU)
#define ADC_ASC1B_ADCH_SHIFT                     (0U)
#define ADC_ASC1B_ADCH_WIDTH                     (6U)
#define ADC_ASC1B_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1B_ADCH_SHIFT)) & ADC_ASC1B_ADCH_MASK)

#define ADC_ASC1B_AIEN_MASK                      (0x40U)
#define ADC_ASC1B_AIEN_SHIFT                     (6U)
#define ADC_ASC1B_AIEN_WIDTH                     (1U)
#define ADC_ASC1B_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1B_AIEN_SHIFT)) & ADC_ASC1B_AIEN_MASK)

#define ADC_ASC1B_COCO_MASK                      (0x80U)
#define ADC_ASC1B_COCO_SHIFT                     (7U)
#define ADC_ASC1B_COCO_WIDTH                     (1U)
#define ADC_ASC1B_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1B_COCO_SHIFT)) & ADC_ASC1B_COCO_MASK)
/*! @} */

/*! @name ASC1C - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1C_ADCH_MASK                      (0x3FU)
#define ADC_ASC1C_ADCH_SHIFT                     (0U)
#define ADC_ASC1C_ADCH_WIDTH                     (6U)
#define ADC_ASC1C_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1C_ADCH_SHIFT)) & ADC_ASC1C_ADCH_MASK)

#define ADC_ASC1C_AIEN_MASK                      (0x40U)
#define ADC_ASC1C_AIEN_SHIFT                     (6U)
#define ADC_ASC1C_AIEN_WIDTH                     (1U)
#define ADC_ASC1C_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1C_AIEN_SHIFT)) & ADC_ASC1C_AIEN_MASK)

#define ADC_ASC1C_COCO_MASK                      (0x80U)
#define ADC_ASC1C_COCO_SHIFT                     (7U)
#define ADC_ASC1C_COCO_WIDTH                     (1U)
#define ADC_ASC1C_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1C_COCO_SHIFT)) & ADC_ASC1C_COCO_MASK)
/*! @} */

/*! @name ASC1D - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1D_ADCH_MASK                      (0x3FU)
#define ADC_ASC1D_ADCH_SHIFT                     (0U)
#define ADC_ASC1D_ADCH_WIDTH                     (6U)
#define ADC_ASC1D_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1D_ADCH_SHIFT)) & ADC_ASC1D_ADCH_MASK)

#define ADC_ASC1D_AIEN_MASK                      (0x40U)
#define ADC_ASC1D_AIEN_SHIFT                     (6U)
#define ADC_ASC1D_AIEN_WIDTH                     (1U)
#define ADC_ASC1D_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1D_AIEN_SHIFT)) & ADC_ASC1D_AIEN_MASK)

#define ADC_ASC1D_COCO_MASK                      (0x80U)
#define ADC_ASC1D_COCO_SHIFT                     (7U)
#define ADC_ASC1D_COCO_WIDTH                     (1U)
#define ADC_ASC1D_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1D_COCO_SHIFT)) & ADC_ASC1D_COCO_MASK)
/*! @} */

/*! @name ASC1E - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1E_ADCH_MASK                      (0x3FU)
#define ADC_ASC1E_ADCH_SHIFT                     (0U)
#define ADC_ASC1E_ADCH_WIDTH                     (6U)
#define ADC_ASC1E_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1E_ADCH_SHIFT)) & ADC_ASC1E_ADCH_MASK)

#define ADC_ASC1E_AIEN_MASK                      (0x40U)
#define ADC_ASC1E_AIEN_SHIFT                     (6U)
#define ADC_ASC1E_AIEN_WIDTH                     (1U)
#define ADC_ASC1E_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1E_AIEN_SHIFT)) & ADC_ASC1E_AIEN_MASK)

#define ADC_ASC1E_COCO_MASK                      (0x80U)
#define ADC_ASC1E_COCO_SHIFT                     (7U)
#define ADC_ASC1E_COCO_WIDTH                     (1U)
#define ADC_ASC1E_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1E_COCO_SHIFT)) & ADC_ASC1E_COCO_MASK)
/*! @} */

/*! @name ASC1F - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1F_ADCH_MASK                      (0x3FU)
#define ADC_ASC1F_ADCH_SHIFT                     (0U)
#define ADC_ASC1F_ADCH_WIDTH                     (6U)
#define ADC_ASC1F_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1F_ADCH_SHIFT)) & ADC_ASC1F_ADCH_MASK)

#define ADC_ASC1F_AIEN_MASK                      (0x40U)
#define ADC_ASC1F_AIEN_SHIFT                     (6U)
#define ADC_ASC1F_AIEN_WIDTH                     (1U)
#define ADC_ASC1F_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1F_AIEN_SHIFT)) & ADC_ASC1F_AIEN_MASK)

#define ADC_ASC1F_COCO_MASK                      (0x80U)
#define ADC_ASC1F_COCO_SHIFT                     (7U)
#define ADC_ASC1F_COCO_WIDTH                     (1U)
#define ADC_ASC1F_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1F_COCO_SHIFT)) & ADC_ASC1F_COCO_MASK)
/*! @} */

/*! @name ASC1G - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1G_ADCH_MASK                      (0x3FU)
#define ADC_ASC1G_ADCH_SHIFT                     (0U)
#define ADC_ASC1G_ADCH_WIDTH                     (6U)
#define ADC_ASC1G_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1G_ADCH_SHIFT)) & ADC_ASC1G_ADCH_MASK)

#define ADC_ASC1G_AIEN_MASK                      (0x40U)
#define ADC_ASC1G_AIEN_SHIFT                     (6U)
#define ADC_ASC1G_AIEN_WIDTH                     (1U)
#define ADC_ASC1G_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1G_AIEN_SHIFT)) & ADC_ASC1G_AIEN_MASK)

#define ADC_ASC1G_COCO_MASK                      (0x80U)
#define ADC_ASC1G_COCO_SHIFT                     (7U)
#define ADC_ASC1G_COCO_WIDTH                     (1U)
#define ADC_ASC1G_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1G_COCO_SHIFT)) & ADC_ASC1G_COCO_MASK)
/*! @} */

/*! @name ASC1H - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1H_ADCH_MASK                      (0x3FU)
#define ADC_ASC1H_ADCH_SHIFT                     (0U)
#define ADC_ASC1H_ADCH_WIDTH                     (6U)
#define ADC_ASC1H_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1H_ADCH_SHIFT)) & ADC_ASC1H_ADCH_MASK)

#define ADC_ASC1H_AIEN_MASK                      (0x40U)
#define ADC_ASC1H_AIEN_SHIFT                     (6U)
#define ADC_ASC1H_AIEN_WIDTH                     (1U)
#define ADC_ASC1H_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1H_AIEN_SHIFT)) & ADC_ASC1H_AIEN_MASK)

#define ADC_ASC1H_COCO_MASK                      (0x80U)
#define ADC_ASC1H_COCO_SHIFT                     (7U)
#define ADC_ASC1H_COCO_WIDTH                     (1U)
#define ADC_ASC1H_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1H_COCO_SHIFT)) & ADC_ASC1H_COCO_MASK)
/*! @} */

/*! @name ASC1I - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1I_ADCH_MASK                      (0x3FU)
#define ADC_ASC1I_ADCH_SHIFT                     (0U)
#define ADC_ASC1I_ADCH_WIDTH                     (6U)
#define ADC_ASC1I_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1I_ADCH_SHIFT)) & ADC_ASC1I_ADCH_MASK)

#define ADC_ASC1I_AIEN_MASK                      (0x40U)
#define ADC_ASC1I_AIEN_SHIFT                     (6U)
#define ADC_ASC1I_AIEN_WIDTH                     (1U)
#define ADC_ASC1I_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1I_AIEN_SHIFT)) & ADC_ASC1I_AIEN_MASK)

#define ADC_ASC1I_COCO_MASK                      (0x80U)
#define ADC_ASC1I_COCO_SHIFT                     (7U)
#define ADC_ASC1I_COCO_WIDTH                     (1U)
#define ADC_ASC1I_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1I_COCO_SHIFT)) & ADC_ASC1I_COCO_MASK)
/*! @} */

/*! @name ASC1J - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1J_ADCH_MASK                      (0x3FU)
#define ADC_ASC1J_ADCH_SHIFT                     (0U)
#define ADC_ASC1J_ADCH_WIDTH                     (6U)
#define ADC_ASC1J_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1J_ADCH_SHIFT)) & ADC_ASC1J_ADCH_MASK)

#define ADC_ASC1J_AIEN_MASK                      (0x40U)
#define ADC_ASC1J_AIEN_SHIFT                     (6U)
#define ADC_ASC1J_AIEN_WIDTH                     (1U)
#define ADC_ASC1J_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1J_AIEN_SHIFT)) & ADC_ASC1J_AIEN_MASK)

#define ADC_ASC1J_COCO_MASK                      (0x80U)
#define ADC_ASC1J_COCO_SHIFT                     (7U)
#define ADC_ASC1J_COCO_WIDTH                     (1U)
#define ADC_ASC1J_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1J_COCO_SHIFT)) & ADC_ASC1J_COCO_MASK)
/*! @} */

/*! @name ASC1K - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1K_ADCH_MASK                      (0x3FU)
#define ADC_ASC1K_ADCH_SHIFT                     (0U)
#define ADC_ASC1K_ADCH_WIDTH                     (6U)
#define ADC_ASC1K_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1K_ADCH_SHIFT)) & ADC_ASC1K_ADCH_MASK)

#define ADC_ASC1K_AIEN_MASK                      (0x40U)
#define ADC_ASC1K_AIEN_SHIFT                     (6U)
#define ADC_ASC1K_AIEN_WIDTH                     (1U)
#define ADC_ASC1K_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1K_AIEN_SHIFT)) & ADC_ASC1K_AIEN_MASK)

#define ADC_ASC1K_COCO_MASK                      (0x80U)
#define ADC_ASC1K_COCO_SHIFT                     (7U)
#define ADC_ASC1K_COCO_WIDTH                     (1U)
#define ADC_ASC1K_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1K_COCO_SHIFT)) & ADC_ASC1K_COCO_MASK)
/*! @} */

/*! @name ASC1L - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1L_ADCH_MASK                      (0x3FU)
#define ADC_ASC1L_ADCH_SHIFT                     (0U)
#define ADC_ASC1L_ADCH_WIDTH                     (6U)
#define ADC_ASC1L_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1L_ADCH_SHIFT)) & ADC_ASC1L_ADCH_MASK)

#define ADC_ASC1L_AIEN_MASK                      (0x40U)
#define ADC_ASC1L_AIEN_SHIFT                     (6U)
#define ADC_ASC1L_AIEN_WIDTH                     (1U)
#define ADC_ASC1L_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1L_AIEN_SHIFT)) & ADC_ASC1L_AIEN_MASK)

#define ADC_ASC1L_COCO_MASK                      (0x80U)
#define ADC_ASC1L_COCO_SHIFT                     (7U)
#define ADC_ASC1L_COCO_WIDTH                     (1U)
#define ADC_ASC1L_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1L_COCO_SHIFT)) & ADC_ASC1L_COCO_MASK)
/*! @} */

/*! @name ASC1M - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1M_ADCH_MASK                      (0x3FU)
#define ADC_ASC1M_ADCH_SHIFT                     (0U)
#define ADC_ASC1M_ADCH_WIDTH                     (6U)
#define ADC_ASC1M_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1M_ADCH_SHIFT)) & ADC_ASC1M_ADCH_MASK)

#define ADC_ASC1M_AIEN_MASK                      (0x40U)
#define ADC_ASC1M_AIEN_SHIFT                     (6U)
#define ADC_ASC1M_AIEN_WIDTH                     (1U)
#define ADC_ASC1M_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1M_AIEN_SHIFT)) & ADC_ASC1M_AIEN_MASK)

#define ADC_ASC1M_COCO_MASK                      (0x80U)
#define ADC_ASC1M_COCO_SHIFT                     (7U)
#define ADC_ASC1M_COCO_WIDTH                     (1U)
#define ADC_ASC1M_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1M_COCO_SHIFT)) & ADC_ASC1M_COCO_MASK)
/*! @} */

/*! @name ASC1N - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1N_ADCH_MASK                      (0x3FU)
#define ADC_ASC1N_ADCH_SHIFT                     (0U)
#define ADC_ASC1N_ADCH_WIDTH                     (6U)
#define ADC_ASC1N_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1N_ADCH_SHIFT)) & ADC_ASC1N_ADCH_MASK)

#define ADC_ASC1N_AIEN_MASK                      (0x40U)
#define ADC_ASC1N_AIEN_SHIFT                     (6U)
#define ADC_ASC1N_AIEN_WIDTH                     (1U)
#define ADC_ASC1N_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1N_AIEN_SHIFT)) & ADC_ASC1N_AIEN_MASK)

#define ADC_ASC1N_COCO_MASK                      (0x80U)
#define ADC_ASC1N_COCO_SHIFT                     (7U)
#define ADC_ASC1N_COCO_WIDTH                     (1U)
#define ADC_ASC1N_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1N_COCO_SHIFT)) & ADC_ASC1N_COCO_MASK)
/*! @} */

/*! @name ASC1O - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1O_ADCH_MASK                      (0x3FU)
#define ADC_ASC1O_ADCH_SHIFT                     (0U)
#define ADC_ASC1O_ADCH_WIDTH                     (6U)
#define ADC_ASC1O_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1O_ADCH_SHIFT)) & ADC_ASC1O_ADCH_MASK)

#define ADC_ASC1O_AIEN_MASK                      (0x40U)
#define ADC_ASC1O_AIEN_SHIFT                     (6U)
#define ADC_ASC1O_AIEN_WIDTH                     (1U)
#define ADC_ASC1O_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1O_AIEN_SHIFT)) & ADC_ASC1O_AIEN_MASK)

#define ADC_ASC1O_COCO_MASK                      (0x80U)
#define ADC_ASC1O_COCO_SHIFT                     (7U)
#define ADC_ASC1O_COCO_WIDTH                     (1U)
#define ADC_ASC1O_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1O_COCO_SHIFT)) & ADC_ASC1O_COCO_MASK)
/*! @} */

/*! @name ASC1P - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_ASC1P_ADCH_MASK                      (0x3FU)
#define ADC_ASC1P_ADCH_SHIFT                     (0U)
#define ADC_ASC1P_ADCH_WIDTH                     (6U)
#define ADC_ASC1P_ADCH(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1P_ADCH_SHIFT)) & ADC_ASC1P_ADCH_MASK)

#define ADC_ASC1P_AIEN_MASK                      (0x40U)
#define ADC_ASC1P_AIEN_SHIFT                     (6U)
#define ADC_ASC1P_AIEN_WIDTH                     (1U)
#define ADC_ASC1P_AIEN(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1P_AIEN_SHIFT)) & ADC_ASC1P_AIEN_MASK)

#define ADC_ASC1P_COCO_MASK                      (0x80U)
#define ADC_ASC1P_COCO_SHIFT                     (7U)
#define ADC_ASC1P_COCO_WIDTH                     (1U)
#define ADC_ASC1P_COCO(x)                        (((uint32_t)(((uint32_t)(x)) << ADC_ASC1P_COCO_SHIFT)) & ADC_ASC1P_COCO_MASK)
/*! @} */

/*! @name SC1 - ADC Status and Control Register 1 */
/*! @{ */

#define ADC_SC1_ADCH_MASK                        (0x3FU)
#define ADC_SC1_ADCH_SHIFT                       (0U)
#define ADC_SC1_ADCH_WIDTH                       (6U)
#define ADC_SC1_ADCH(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC1_ADCH_SHIFT)) & ADC_SC1_ADCH_MASK)

#define ADC_SC1_AIEN_MASK                        (0x40U)
#define ADC_SC1_AIEN_SHIFT                       (6U)
#define ADC_SC1_AIEN_WIDTH                       (1U)
#define ADC_SC1_AIEN(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC1_AIEN_SHIFT)) & ADC_SC1_AIEN_MASK)

#define ADC_SC1_COCO_MASK                        (0x80U)
#define ADC_SC1_COCO_SHIFT                       (7U)
#define ADC_SC1_COCO_WIDTH                       (1U)
#define ADC_SC1_COCO(x)                          (((uint32_t)(((uint32_t)(x)) << ADC_SC1_COCO_SHIFT)) & ADC_SC1_COCO_MASK)
/*! @} */

/*! @name ARA - ADC Data Result Registers */
/*! @{ */

#define ADC_ARA_D_MASK                           (0xFFFU)
#define ADC_ARA_D_SHIFT                          (0U)
#define ADC_ARA_D_WIDTH                          (12U)
#define ADC_ARA_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARA_D_SHIFT)) & ADC_ARA_D_MASK)
/*! @} */

/*! @name ARB - ADC Data Result Registers */
/*! @{ */

#define ADC_ARB_D_MASK                           (0xFFFU)
#define ADC_ARB_D_SHIFT                          (0U)
#define ADC_ARB_D_WIDTH                          (12U)
#define ADC_ARB_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARB_D_SHIFT)) & ADC_ARB_D_MASK)
/*! @} */

/*! @name ARC - ADC Data Result Registers */
/*! @{ */

#define ADC_ARC_D_MASK                           (0xFFFU)
#define ADC_ARC_D_SHIFT                          (0U)
#define ADC_ARC_D_WIDTH                          (12U)
#define ADC_ARC_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARC_D_SHIFT)) & ADC_ARC_D_MASK)
/*! @} */

/*! @name ARD - ADC Data Result Registers */
/*! @{ */

#define ADC_ARD_D_MASK                           (0xFFFU)
#define ADC_ARD_D_SHIFT                          (0U)
#define ADC_ARD_D_WIDTH                          (12U)
#define ADC_ARD_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARD_D_SHIFT)) & ADC_ARD_D_MASK)
/*! @} */

/*! @name ARE - ADC Data Result Registers */
/*! @{ */

#define ADC_ARE_D_MASK                           (0xFFFU)
#define ADC_ARE_D_SHIFT                          (0U)
#define ADC_ARE_D_WIDTH                          (12U)
#define ADC_ARE_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARE_D_SHIFT)) & ADC_ARE_D_MASK)
/*! @} */

/*! @name ARF - ADC Data Result Registers */
/*! @{ */

#define ADC_ARF_D_MASK                           (0xFFFU)
#define ADC_ARF_D_SHIFT                          (0U)
#define ADC_ARF_D_WIDTH                          (12U)
#define ADC_ARF_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARF_D_SHIFT)) & ADC_ARF_D_MASK)
/*! @} */

/*! @name ARG - ADC Data Result Registers */
/*! @{ */

#define ADC_ARG_D_MASK                           (0xFFFU)
#define ADC_ARG_D_SHIFT                          (0U)
#define ADC_ARG_D_WIDTH                          (12U)
#define ADC_ARG_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARG_D_SHIFT)) & ADC_ARG_D_MASK)
/*! @} */

/*! @name ARH - ADC Data Result Registers */
/*! @{ */

#define ADC_ARH_D_MASK                           (0xFFFU)
#define ADC_ARH_D_SHIFT                          (0U)
#define ADC_ARH_D_WIDTH                          (12U)
#define ADC_ARH_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARH_D_SHIFT)) & ADC_ARH_D_MASK)
/*! @} */

/*! @name ARI - ADC Data Result Registers */
/*! @{ */

#define ADC_ARI_D_MASK                           (0xFFFU)
#define ADC_ARI_D_SHIFT                          (0U)
#define ADC_ARI_D_WIDTH                          (12U)
#define ADC_ARI_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARI_D_SHIFT)) & ADC_ARI_D_MASK)
/*! @} */

/*! @name ARJ - ADC Data Result Registers */
/*! @{ */

#define ADC_ARJ_D_MASK                           (0xFFFU)
#define ADC_ARJ_D_SHIFT                          (0U)
#define ADC_ARJ_D_WIDTH                          (12U)
#define ADC_ARJ_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARJ_D_SHIFT)) & ADC_ARJ_D_MASK)
/*! @} */

/*! @name ARK - ADC Data Result Registers */
/*! @{ */

#define ADC_ARK_D_MASK                           (0xFFFU)
#define ADC_ARK_D_SHIFT                          (0U)
#define ADC_ARK_D_WIDTH                          (12U)
#define ADC_ARK_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARK_D_SHIFT)) & ADC_ARK_D_MASK)
/*! @} */

/*! @name ARL - ADC Data Result Registers */
/*! @{ */

#define ADC_ARL_D_MASK                           (0xFFFU)
#define ADC_ARL_D_SHIFT                          (0U)
#define ADC_ARL_D_WIDTH                          (12U)
#define ADC_ARL_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARL_D_SHIFT)) & ADC_ARL_D_MASK)
/*! @} */

/*! @name ARM - ADC Data Result Registers */
/*! @{ */

#define ADC_ARM_D_MASK                           (0xFFFU)
#define ADC_ARM_D_SHIFT                          (0U)
#define ADC_ARM_D_WIDTH                          (12U)
#define ADC_ARM_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARM_D_SHIFT)) & ADC_ARM_D_MASK)
/*! @} */

/*! @name ARN - ADC Data Result Registers */
/*! @{ */

#define ADC_ARN_D_MASK                           (0xFFFU)
#define ADC_ARN_D_SHIFT                          (0U)
#define ADC_ARN_D_WIDTH                          (12U)
#define ADC_ARN_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARN_D_SHIFT)) & ADC_ARN_D_MASK)
/*! @} */

/*! @name ARO - ADC Data Result Registers */
/*! @{ */

#define ADC_ARO_D_MASK                           (0xFFFU)
#define ADC_ARO_D_SHIFT                          (0U)
#define ADC_ARO_D_WIDTH                          (12U)
#define ADC_ARO_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARO_D_SHIFT)) & ADC_ARO_D_MASK)
/*! @} */

/*! @name ARP - ADC Data Result Registers */
/*! @{ */

#define ADC_ARP_D_MASK                           (0xFFFU)
#define ADC_ARP_D_SHIFT                          (0U)
#define ADC_ARP_D_WIDTH                          (12U)
#define ADC_ARP_D(x)                             (((uint32_t)(((uint32_t)(x)) << ADC_ARP_D_SHIFT)) & ADC_ARP_D_MASK)
/*! @} */

/*! @name R - ADC Data Result Registers */
/*! @{ */

#define ADC_R_D_MASK                             (0xFFFU)
#define ADC_R_D_SHIFT                            (0U)
#define ADC_R_D_WIDTH                            (12U)
#define ADC_R_D(x)                               (((uint32_t)(((uint32_t)(x)) << ADC_R_D_SHIFT)) & ADC_R_D_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group ADC_Register_Masks */

/*!
 * @}
 */ /* end of group ADC_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_ADC_H_) */

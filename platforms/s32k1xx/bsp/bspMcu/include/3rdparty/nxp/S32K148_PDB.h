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
 * @file S32K148_PDB.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_PDB
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
#if !defined(S32K148_PDB_H_)  /* Check if memory map has not been already included */
#define S32K148_PDB_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- PDB Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PDB_Peripheral_Access_Layer PDB Peripheral Access Layer
 * @{
 */

/** PDB - Size of Registers Arrays */
#define PDB_DLY_COUNT                          8u
#define PDB_CH_COUNT                              4u
#define PDB_POnDLY_COUNT                          1u

/** PDB - Register Layout Typedef */
typedef struct {
  __IO uint32_t SC;                                /**< Status and Control register, offset: 0x0 */
  __IO uint32_t MOD;                               /**< Modulus register, offset: 0x4 */
  __I  uint32_t CNT;                               /**< Counter register, offset: 0x8 */
  __IO uint32_t IDLY;                              /**< Interrupt Delay register, offset: 0xC */
  struct {                                         /* offset: 0x10, array step: 0x28 */
    __IO uint32_t C1;                                /**< Channel n Control register 1, array offset: 0x10, array step: 0x28 */
    __IO uint32_t S;                                 /**< Channel n Status register, array offset: 0x14, array step: 0x28 */
    __IO uint32_t DLY[PDB_DLY_COUNT];             /**< Channel n Delay 0 register..Channel n Delay 7 register, array offset: 0x18, array step: index*0x28, index2*0x4 */
  } CH[PDB_CH_COUNT];
  uint8_t RESERVED_0[224];
  __IO uint32_t POEN;                              /**< Pulse-Out n Enable register, offset: 0x190 */
  union {                                          /* offset: 0x194 */
    struct {                                         /* offset: 0x194 */
      __IO uint16_t DLY2;                            /**< PDB_DLY2 register, offset: 0x194 */
      __IO uint16_t DLY1;                            /**< PDB_DLY1 register, offset: 0x196 */
    } ACCESS16BIT;
    __IO uint32_t PODLY;           /**< Pulse-Out n Delay register, offset: 0x194 */
  } POnDLY[PDB_POnDLY_COUNT];
} PDB_Type, *PDB_MemMapPtr;

/** Number of instances of the PDB module. */
#define PDB_INSTANCE_COUNT                       (2u)

/* PDB - Peripheral instance base addresses */
/** Peripheral PDB0 base address */
#define IP_PDB0_BASE                             (0x40036000u)
/** Peripheral PDB0 base pointer */
#define IP_PDB0                                  ((PDB_Type *)IP_PDB0_BASE)
/** Peripheral PDB1 base address */
#define IP_PDB1_BASE                             (0x40031000u)
/** Peripheral PDB1 base pointer */
#define IP_PDB1                                  ((PDB_Type *)IP_PDB1_BASE)
/** Array initializer of PDB peripheral base addresses */
#define IP_PDB_BASE_ADDRS                        { IP_PDB0_BASE, IP_PDB1_BASE }
/** Array initializer of PDB peripheral base pointers */
#define IP_PDB_BASE_PTRS                         { IP_PDB0, IP_PDB1 }

/* ----------------------------------------------------------------------------
   -- PDB Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup PDB_Register_Masks PDB Register Masks
 * @{
 */

/*! @name SC - Status and Control register */
/*! @{ */

#define PDB_SC_LDOK_MASK                         (0x1U)
#define PDB_SC_LDOK_SHIFT                        (0U)
#define PDB_SC_LDOK_WIDTH                        (1U)
#define PDB_SC_LDOK(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_SC_LDOK_SHIFT)) & PDB_SC_LDOK_MASK)

#define PDB_SC_CONT_MASK                         (0x2U)
#define PDB_SC_CONT_SHIFT                        (1U)
#define PDB_SC_CONT_WIDTH                        (1U)
#define PDB_SC_CONT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_SC_CONT_SHIFT)) & PDB_SC_CONT_MASK)

#define PDB_SC_MULT_MASK                         (0xCU)
#define PDB_SC_MULT_SHIFT                        (2U)
#define PDB_SC_MULT_WIDTH                        (2U)
#define PDB_SC_MULT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_SC_MULT_SHIFT)) & PDB_SC_MULT_MASK)

#define PDB_SC_PDBIE_MASK                        (0x20U)
#define PDB_SC_PDBIE_SHIFT                       (5U)
#define PDB_SC_PDBIE_WIDTH                       (1U)
#define PDB_SC_PDBIE(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_PDBIE_SHIFT)) & PDB_SC_PDBIE_MASK)

#define PDB_SC_PDBIF_MASK                        (0x40U)
#define PDB_SC_PDBIF_SHIFT                       (6U)
#define PDB_SC_PDBIF_WIDTH                       (1U)
#define PDB_SC_PDBIF(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_PDBIF_SHIFT)) & PDB_SC_PDBIF_MASK)

#define PDB_SC_PDBEN_MASK                        (0x80U)
#define PDB_SC_PDBEN_SHIFT                       (7U)
#define PDB_SC_PDBEN_WIDTH                       (1U)
#define PDB_SC_PDBEN(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_PDBEN_SHIFT)) & PDB_SC_PDBEN_MASK)

#define PDB_SC_TRGSEL_MASK                       (0xF00U)
#define PDB_SC_TRGSEL_SHIFT                      (8U)
#define PDB_SC_TRGSEL_WIDTH                      (4U)
#define PDB_SC_TRGSEL(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_SC_TRGSEL_SHIFT)) & PDB_SC_TRGSEL_MASK)

#define PDB_SC_PRESCALER_MASK                    (0x7000U)
#define PDB_SC_PRESCALER_SHIFT                   (12U)
#define PDB_SC_PRESCALER_WIDTH                   (3U)
#define PDB_SC_PRESCALER(x)                      (((uint32_t)(((uint32_t)(x)) << PDB_SC_PRESCALER_SHIFT)) & PDB_SC_PRESCALER_MASK)

#define PDB_SC_DMAEN_MASK                        (0x8000U)
#define PDB_SC_DMAEN_SHIFT                       (15U)
#define PDB_SC_DMAEN_WIDTH                       (1U)
#define PDB_SC_DMAEN(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_DMAEN_SHIFT)) & PDB_SC_DMAEN_MASK)

#define PDB_SC_SWTRIG_MASK                       (0x10000U)
#define PDB_SC_SWTRIG_SHIFT                      (16U)
#define PDB_SC_SWTRIG_WIDTH                      (1U)
#define PDB_SC_SWTRIG(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_SC_SWTRIG_SHIFT)) & PDB_SC_SWTRIG_MASK)

#define PDB_SC_PDBEIE_MASK                       (0x20000U)
#define PDB_SC_PDBEIE_SHIFT                      (17U)
#define PDB_SC_PDBEIE_WIDTH                      (1U)
#define PDB_SC_PDBEIE(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_SC_PDBEIE_SHIFT)) & PDB_SC_PDBEIE_MASK)

#define PDB_SC_LDMOD_MASK                        (0xC0000U)
#define PDB_SC_LDMOD_SHIFT                       (18U)
#define PDB_SC_LDMOD_WIDTH                       (2U)
#define PDB_SC_LDMOD(x)                          (((uint32_t)(((uint32_t)(x)) << PDB_SC_LDMOD_SHIFT)) & PDB_SC_LDMOD_MASK)
/*! @} */

/*! @name MOD - Modulus register */
/*! @{ */

#define PDB_MOD_MOD_MASK                         (0xFFFFU)
#define PDB_MOD_MOD_SHIFT                        (0U)
#define PDB_MOD_MOD_WIDTH                        (16U)
#define PDB_MOD_MOD(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_MOD_MOD_SHIFT)) & PDB_MOD_MOD_MASK)
/*! @} */

/*! @name CNT - Counter register */
/*! @{ */

#define PDB_CNT_CNT_MASK                         (0xFFFFU)
#define PDB_CNT_CNT_SHIFT                        (0U)
#define PDB_CNT_CNT_WIDTH                        (16U)
#define PDB_CNT_CNT(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_CNT_CNT_SHIFT)) & PDB_CNT_CNT_MASK)
/*! @} */

/*! @name IDLY - Interrupt Delay register */
/*! @{ */

#define PDB_IDLY_IDLY_MASK                       (0xFFFFU)
#define PDB_IDLY_IDLY_SHIFT                      (0U)
#define PDB_IDLY_IDLY_WIDTH                      (16U)
#define PDB_IDLY_IDLY(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_IDLY_IDLY_SHIFT)) & PDB_IDLY_IDLY_MASK)
/*! @} */

/*! @name C1 - Channel n Control register 1 */
/*! @{ */

#define PDB_C1_EN_MASK                           (0xFFU)
#define PDB_C1_EN_SHIFT                          (0U)
#define PDB_C1_EN_WIDTH                          (8U)
#define PDB_C1_EN(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_C1_EN_SHIFT)) & PDB_C1_EN_MASK)

#define PDB_C1_TOS_MASK                          (0xFF00U)
#define PDB_C1_TOS_SHIFT                         (8U)
#define PDB_C1_TOS_WIDTH                         (8U)
#define PDB_C1_TOS(x)                            (((uint32_t)(((uint32_t)(x)) << PDB_C1_TOS_SHIFT)) & PDB_C1_TOS_MASK)

#define PDB_C1_BB_MASK                           (0xFF0000U)
#define PDB_C1_BB_SHIFT                          (16U)
#define PDB_C1_BB_WIDTH                          (8U)
#define PDB_C1_BB(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_C1_BB_SHIFT)) & PDB_C1_BB_MASK)
/*! @} */

/*! @name S - Channel n Status register */
/*! @{ */

#define PDB_S_ERR_MASK                           (0xFFU)
#define PDB_S_ERR_SHIFT                          (0U)
#define PDB_S_ERR_WIDTH                          (8U)
#define PDB_S_ERR(x)                             (((uint32_t)(((uint32_t)(x)) << PDB_S_ERR_SHIFT)) & PDB_S_ERR_MASK)

#define PDB_S_CF_MASK                            (0xFF0000U)
#define PDB_S_CF_SHIFT                           (16U)
#define PDB_S_CF_WIDTH                           (8U)
#define PDB_S_CF(x)                              (((uint32_t)(((uint32_t)(x)) << PDB_S_CF_SHIFT)) & PDB_S_CF_MASK)
/*! @} */

/*! @name DLY - Channel n Delay 0 register..Channel n Delay 7 register */
/*! @{ */

#define PDB_DLY_DLY_MASK                         (0xFFFFU)
#define PDB_DLY_DLY_SHIFT                        (0U)
#define PDB_DLY_DLY_WIDTH                        (16U)
#define PDB_DLY_DLY(x)                           (((uint32_t)(((uint32_t)(x)) << PDB_DLY_DLY_SHIFT)) & PDB_DLY_DLY_MASK)
/*! @} */

/*! @name POEN - Pulse-Out n Enable register */
/*! @{ */

#define PDB_POEN_POEN_MASK                       (0xFFU)
#define PDB_POEN_POEN_SHIFT                      (0U)
#define PDB_POEN_POEN_WIDTH                      (8U)
#define PDB_POEN_POEN(x)                         (((uint32_t)(((uint32_t)(x)) << PDB_POEN_POEN_SHIFT)) & PDB_POEN_POEN_MASK)
/*! @} */

/*! @name POnDLY_PODLY - Pulse-Out n Delay register */
/*! @{ */

#define PDB_POnDLY_PODLY_DLY2_MASK               (0xFFFFU)
#define PDB_POnDLY_PODLY_DLY2_SHIFT              (0U)
#define PDB_POnDLY_PODLY_DLY2_WIDTH              (16U)
#define PDB_POnDLY_PODLY_DLY2(x)                 (((uint32_t)(((uint32_t)(x)) << PDB_POnDLY_PODLY_DLY2_SHIFT)) & PDB_POnDLY_PODLY_DLY2_MASK)

#define PDB_POnDLY_PODLY_DLY1_MASK               (0xFFFF0000U)
#define PDB_POnDLY_PODLY_DLY1_SHIFT              (16U)
#define PDB_POnDLY_PODLY_DLY1_WIDTH              (16U)
#define PDB_POnDLY_PODLY_DLY1(x)                 (((uint32_t)(((uint32_t)(x)) << PDB_POnDLY_PODLY_DLY1_SHIFT)) & PDB_POnDLY_PODLY_DLY1_MASK)
/*! @} */

/*! @name POnDLY_ACCESS16BIT_DLY2 - PDB_DLY2 register */
/*! @{ */
#define PDB_POnDLY_ACCESS16BIT_DLY2_DLY2_MASK (0xFFFFU)
#define PDB_POnDLY_ACCESS16BIT_DLY2_DLY2_SHIFT (0U)
#define PDB_POnDLY_ACCESS16BIT_DLY2_DLY2_WIDTH (16U)
#define PDB_POnDLY_ACCESS16BIT_DLY2_DLY2(x)  (((uint16_t)(((uint16_t)(x)) << PDB_POnDLY_ACCESS16BIT_DLY2_DLY2_SHIFT)) & PDB_POnDLY_ACCESS16BIT_DLY2_DLY2_MASK)
/*! @} */

/*! @name POnDLY_ACCESS16BIT_DLY1 - PDB_DLY1 register */
/*! @{ */
#define PDB_POnDLY_ACCESS16BIT_DLY1_DLY1_MASK (0xFFFFU)
#define PDB_POnDLY_ACCESS16BIT_DLY1_DLY1_SHIFT (0U)
#define PDB_POnDLY_ACCESS16BIT_DLY1_DLY1_WIDTH (16U)
#define PDB_POnDLY_ACCESS16BIT_DLY1_DLY1(x)  (((uint16_t)(((uint16_t)(x)) << PDB_POnDLY_ACCESS16BIT_DLY1_DLY1_SHIFT)) & PDB_POnDLY_ACCESS16BIT_DLY1_DLY1_MASK)
/*! @} */



/*!
 * @}
 */ /* end of group PDB_Register_Masks */

/*!
 * @}
 */ /* end of group PDB_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_PDB_H_) */

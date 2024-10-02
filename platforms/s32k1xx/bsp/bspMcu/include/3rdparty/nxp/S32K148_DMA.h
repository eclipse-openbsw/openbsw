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
 * @file S32K148_DMA.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_DMA
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
#if !defined(S32K148_DMA_H_)  /* Check if memory map has not been already included */
#define S32K148_DMA_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- DMA Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Peripheral_Access_Layer DMA Peripheral Access Layer
 * @{
 */

/** DMA - Size of Registers Arrays */
#define DMA_DCHPRI_COUNT                          16u
#define DMA_TCD_COUNT                             16u

/** DMA - Register Layout Typedef */
typedef struct {
  __IO uint32_t CR;                                /**< Control, offset: 0x0 */
  __I  uint32_t ES;                                /**< Error Status, offset: 0x4 */
  uint8_t RESERVED_0[4];
  __IO uint32_t ERQ;                               /**< Enable Request, offset: 0xC */
  uint8_t RESERVED_1[4];
  __IO uint32_t EEI;                               /**< Enable Error Interrupt, offset: 0x14 */
  __O  uint8_t CEEI;                               /**< Clear Enable Error Interrupt, offset: 0x18 */
  __O  uint8_t SEEI;                               /**< Set Enable Error Interrupt, offset: 0x19 */
  __O  uint8_t CERQ;                               /**< Clear Enable Request, offset: 0x1A */
  __O  uint8_t SERQ;                               /**< Set Enable Request, offset: 0x1B */
  __O  uint8_t CDNE;                               /**< Clear DONE Status Bit, offset: 0x1C */
  __O  uint8_t SSRT;                               /**< Set START Bit, offset: 0x1D */
  __O  uint8_t CERR;                               /**< Clear Error, offset: 0x1E */
  __O  uint8_t CINT;                               /**< Clear Interrupt Request, offset: 0x1F */
  uint8_t RESERVED_2[4];
  __IO uint32_t INT;                               /**< Interrupt Request, offset: 0x24 */
  uint8_t RESERVED_3[4];
  __IO uint32_t ERR;                               /**< Error, offset: 0x2C */
  uint8_t RESERVED_4[4];
  __I  uint32_t HRS;                               /**< Hardware Request Status, offset: 0x34 */
  uint8_t RESERVED_5[12];
  __IO uint32_t EARS;                              /**< Enable Asynchronous Request in Stop, offset: 0x44 */
  uint8_t RESERVED_6[184];
  __IO uint8_t DCHPRI[DMA_DCHPRI_COUNT];           /**< Channel Priority, array offset: 0x100, array step: 0x1 */
  uint8_t RESERVED_7[3824];
  struct {                                         /* offset: 0x1000, array step: 0x20 */
    __IO uint32_t SADDR;                             /**< TCD Source Address, array offset: 0x1000, array step: 0x20 */
    __IO uint16_t SOFF;                              /**< TCD Signed Source Address Offset, array offset: 0x1004, array step: 0x20 */
    __IO uint16_t ATTR;                              /**< TCD Transfer Attributes, array offset: 0x1006, array step: 0x20 */
    union {                                          /* offset: 0x1008, array step: 0x20 */
      __IO uint32_t MLNO;                              /**< TCD Minor Byte Count (Minor Loop Mapping Disabled), array offset: 0x1008, array step: 0x20 */
      __IO uint32_t MLOFFNO;                           /**< TCD Signed Minor Loop Offset (Minor Loop Mapping Enabled and Offset Disabled), array offset: 0x1008, array step: 0x20 */
      __IO uint32_t MLOFFYES;                          /**< TCD Signed Minor Loop Offset (Minor Loop Mapping and Offset Enabled), array offset: 0x1008, array step: 0x20 */
    } NBYTES;
    __IO uint32_t SLAST;                             /**< TCD Last Source Address Adjustment, array offset: 0x100C, array step: 0x20 */
    __IO uint32_t DADDR;                             /**< TCD Destination Address, array offset: 0x1010, array step: 0x20 */
    __IO uint16_t DOFF;                              /**< TCD Signed Destination Address Offset, array offset: 0x1014, array step: 0x20 */
    union {                                          /* offset: 0x1016, array step: 0x20 */
      __IO uint16_t ELINKNO;                           /**< TCD Current Minor Loop Link, Major Loop Count (Channel Linking Disabled), array offset: 0x1016, array step: 0x20 */
      __IO uint16_t ELINKYES;                          /**< TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled), array offset: 0x1016, array step: 0x20 */
    } CITER;
    __IO uint32_t DLASTSGA;                          /**< TCD Last Destination Address Adjustment/Scatter Gather Address, array offset: 0x1018, array step: 0x20 */
    __IO uint16_t CSR;                               /**< TCD Control and Status, array offset: 0x101C, array step: 0x20 */
    union {                                          /* offset: 0x101E, array step: 0x20 */
      __IO uint16_t ELINKNO;                           /**< TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled), array offset: 0x101E, array step: 0x20 */
      __IO uint16_t ELINKYES;                          /**< TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled), array offset: 0x101E, array step: 0x20 */
    } BITER;
  } TCD[DMA_TCD_COUNT];
} DMA_Type, *DMA_MemMapPtr;

/** Number of instances of the DMA module. */
#define DMA_INSTANCE_COUNT                       (1u)

/* DMA - Peripheral instance base addresses */
/** Peripheral DMA base address */
#define IP_DMA_BASE                              (0x40008000u)
/** Peripheral DMA base pointer */
#define IP_DMA                                   ((DMA_Type *)IP_DMA_BASE)
/** Array initializer of DMA peripheral base addresses */
#define IP_DMA_BASE_ADDRS                        { IP_DMA_BASE }
/** Array initializer of DMA peripheral base pointers */
#define IP_DMA_BASE_PTRS                         { IP_DMA }

/* ----------------------------------------------------------------------------
   -- DMA Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup DMA_Register_Masks DMA Register Masks
 * @{
 */

/*! @name CR - Control */
/*! @{ */

#define DMA_CR_EDBG_MASK                         (0x2U)
#define DMA_CR_EDBG_SHIFT                        (1U)
#define DMA_CR_EDBG_WIDTH                        (1U)
#define DMA_CR_EDBG(x)                           (((uint32_t)(((uint32_t)(x)) << DMA_CR_EDBG_SHIFT)) & DMA_CR_EDBG_MASK)

#define DMA_CR_ERCA_MASK                         (0x4U)
#define DMA_CR_ERCA_SHIFT                        (2U)
#define DMA_CR_ERCA_WIDTH                        (1U)
#define DMA_CR_ERCA(x)                           (((uint32_t)(((uint32_t)(x)) << DMA_CR_ERCA_SHIFT)) & DMA_CR_ERCA_MASK)

#define DMA_CR_HOE_MASK                          (0x10U)
#define DMA_CR_HOE_SHIFT                         (4U)
#define DMA_CR_HOE_WIDTH                         (1U)
#define DMA_CR_HOE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_CR_HOE_SHIFT)) & DMA_CR_HOE_MASK)

#define DMA_CR_HALT_MASK                         (0x20U)
#define DMA_CR_HALT_SHIFT                        (5U)
#define DMA_CR_HALT_WIDTH                        (1U)
#define DMA_CR_HALT(x)                           (((uint32_t)(((uint32_t)(x)) << DMA_CR_HALT_SHIFT)) & DMA_CR_HALT_MASK)

#define DMA_CR_CLM_MASK                          (0x40U)
#define DMA_CR_CLM_SHIFT                         (6U)
#define DMA_CR_CLM_WIDTH                         (1U)
#define DMA_CR_CLM(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_CR_CLM_SHIFT)) & DMA_CR_CLM_MASK)

#define DMA_CR_EMLM_MASK                         (0x80U)
#define DMA_CR_EMLM_SHIFT                        (7U)
#define DMA_CR_EMLM_WIDTH                        (1U)
#define DMA_CR_EMLM(x)                           (((uint32_t)(((uint32_t)(x)) << DMA_CR_EMLM_SHIFT)) & DMA_CR_EMLM_MASK)

#define DMA_CR_ECX_MASK                          (0x10000U)
#define DMA_CR_ECX_SHIFT                         (16U)
#define DMA_CR_ECX_WIDTH                         (1U)
#define DMA_CR_ECX(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_CR_ECX_SHIFT)) & DMA_CR_ECX_MASK)

#define DMA_CR_CX_MASK                           (0x20000U)
#define DMA_CR_CX_SHIFT                          (17U)
#define DMA_CR_CX_WIDTH                          (1U)
#define DMA_CR_CX(x)                             (((uint32_t)(((uint32_t)(x)) << DMA_CR_CX_SHIFT)) & DMA_CR_CX_MASK)

#define DMA_CR_ACTIVE_MASK                       (0x80000000U)
#define DMA_CR_ACTIVE_SHIFT                      (31U)
#define DMA_CR_ACTIVE_WIDTH                      (1U)
#define DMA_CR_ACTIVE(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_CR_ACTIVE_SHIFT)) & DMA_CR_ACTIVE_MASK)
/*! @} */

/*! @name ES - Error Status */
/*! @{ */

#define DMA_ES_DBE_MASK                          (0x1U)
#define DMA_ES_DBE_SHIFT                         (0U)
#define DMA_ES_DBE_WIDTH                         (1U)
#define DMA_ES_DBE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_DBE_SHIFT)) & DMA_ES_DBE_MASK)

#define DMA_ES_SBE_MASK                          (0x2U)
#define DMA_ES_SBE_SHIFT                         (1U)
#define DMA_ES_SBE_WIDTH                         (1U)
#define DMA_ES_SBE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_SBE_SHIFT)) & DMA_ES_SBE_MASK)

#define DMA_ES_SGE_MASK                          (0x4U)
#define DMA_ES_SGE_SHIFT                         (2U)
#define DMA_ES_SGE_WIDTH                         (1U)
#define DMA_ES_SGE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_SGE_SHIFT)) & DMA_ES_SGE_MASK)

#define DMA_ES_NCE_MASK                          (0x8U)
#define DMA_ES_NCE_SHIFT                         (3U)
#define DMA_ES_NCE_WIDTH                         (1U)
#define DMA_ES_NCE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_NCE_SHIFT)) & DMA_ES_NCE_MASK)

#define DMA_ES_DOE_MASK                          (0x10U)
#define DMA_ES_DOE_SHIFT                         (4U)
#define DMA_ES_DOE_WIDTH                         (1U)
#define DMA_ES_DOE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_DOE_SHIFT)) & DMA_ES_DOE_MASK)

#define DMA_ES_DAE_MASK                          (0x20U)
#define DMA_ES_DAE_SHIFT                         (5U)
#define DMA_ES_DAE_WIDTH                         (1U)
#define DMA_ES_DAE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_DAE_SHIFT)) & DMA_ES_DAE_MASK)

#define DMA_ES_SOE_MASK                          (0x40U)
#define DMA_ES_SOE_SHIFT                         (6U)
#define DMA_ES_SOE_WIDTH                         (1U)
#define DMA_ES_SOE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_SOE_SHIFT)) & DMA_ES_SOE_MASK)

#define DMA_ES_SAE_MASK                          (0x80U)
#define DMA_ES_SAE_SHIFT                         (7U)
#define DMA_ES_SAE_WIDTH                         (1U)
#define DMA_ES_SAE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_SAE_SHIFT)) & DMA_ES_SAE_MASK)

#define DMA_ES_ERRCHN_MASK                       (0xF00U)
#define DMA_ES_ERRCHN_SHIFT                      (8U)
#define DMA_ES_ERRCHN_WIDTH                      (4U)
#define DMA_ES_ERRCHN(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ES_ERRCHN_SHIFT)) & DMA_ES_ERRCHN_MASK)

#define DMA_ES_CPE_MASK                          (0x4000U)
#define DMA_ES_CPE_SHIFT                         (14U)
#define DMA_ES_CPE_WIDTH                         (1U)
#define DMA_ES_CPE(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_CPE_SHIFT)) & DMA_ES_CPE_MASK)

#define DMA_ES_ECX_MASK                          (0x10000U)
#define DMA_ES_ECX_SHIFT                         (16U)
#define DMA_ES_ECX_WIDTH                         (1U)
#define DMA_ES_ECX(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_ECX_SHIFT)) & DMA_ES_ECX_MASK)

#define DMA_ES_VLD_MASK                          (0x80000000U)
#define DMA_ES_VLD_SHIFT                         (31U)
#define DMA_ES_VLD_WIDTH                         (1U)
#define DMA_ES_VLD(x)                            (((uint32_t)(((uint32_t)(x)) << DMA_ES_VLD_SHIFT)) & DMA_ES_VLD_MASK)
/*! @} */

/*! @name ERQ - Enable Request */
/*! @{ */

#define DMA_ERQ_ERQ0_MASK                        (0x1U)
#define DMA_ERQ_ERQ0_SHIFT                       (0U)
#define DMA_ERQ_ERQ0_WIDTH                       (1U)
#define DMA_ERQ_ERQ0(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ0_SHIFT)) & DMA_ERQ_ERQ0_MASK)

#define DMA_ERQ_ERQ1_MASK                        (0x2U)
#define DMA_ERQ_ERQ1_SHIFT                       (1U)
#define DMA_ERQ_ERQ1_WIDTH                       (1U)
#define DMA_ERQ_ERQ1(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ1_SHIFT)) & DMA_ERQ_ERQ1_MASK)

#define DMA_ERQ_ERQ2_MASK                        (0x4U)
#define DMA_ERQ_ERQ2_SHIFT                       (2U)
#define DMA_ERQ_ERQ2_WIDTH                       (1U)
#define DMA_ERQ_ERQ2(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ2_SHIFT)) & DMA_ERQ_ERQ2_MASK)

#define DMA_ERQ_ERQ3_MASK                        (0x8U)
#define DMA_ERQ_ERQ3_SHIFT                       (3U)
#define DMA_ERQ_ERQ3_WIDTH                       (1U)
#define DMA_ERQ_ERQ3(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ3_SHIFT)) & DMA_ERQ_ERQ3_MASK)

#define DMA_ERQ_ERQ4_MASK                        (0x10U)
#define DMA_ERQ_ERQ4_SHIFT                       (4U)
#define DMA_ERQ_ERQ4_WIDTH                       (1U)
#define DMA_ERQ_ERQ4(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ4_SHIFT)) & DMA_ERQ_ERQ4_MASK)

#define DMA_ERQ_ERQ5_MASK                        (0x20U)
#define DMA_ERQ_ERQ5_SHIFT                       (5U)
#define DMA_ERQ_ERQ5_WIDTH                       (1U)
#define DMA_ERQ_ERQ5(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ5_SHIFT)) & DMA_ERQ_ERQ5_MASK)

#define DMA_ERQ_ERQ6_MASK                        (0x40U)
#define DMA_ERQ_ERQ6_SHIFT                       (6U)
#define DMA_ERQ_ERQ6_WIDTH                       (1U)
#define DMA_ERQ_ERQ6(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ6_SHIFT)) & DMA_ERQ_ERQ6_MASK)

#define DMA_ERQ_ERQ7_MASK                        (0x80U)
#define DMA_ERQ_ERQ7_SHIFT                       (7U)
#define DMA_ERQ_ERQ7_WIDTH                       (1U)
#define DMA_ERQ_ERQ7(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ7_SHIFT)) & DMA_ERQ_ERQ7_MASK)

#define DMA_ERQ_ERQ8_MASK                        (0x100U)
#define DMA_ERQ_ERQ8_SHIFT                       (8U)
#define DMA_ERQ_ERQ8_WIDTH                       (1U)
#define DMA_ERQ_ERQ8(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ8_SHIFT)) & DMA_ERQ_ERQ8_MASK)

#define DMA_ERQ_ERQ9_MASK                        (0x200U)
#define DMA_ERQ_ERQ9_SHIFT                       (9U)
#define DMA_ERQ_ERQ9_WIDTH                       (1U)
#define DMA_ERQ_ERQ9(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ9_SHIFT)) & DMA_ERQ_ERQ9_MASK)

#define DMA_ERQ_ERQ10_MASK                       (0x400U)
#define DMA_ERQ_ERQ10_SHIFT                      (10U)
#define DMA_ERQ_ERQ10_WIDTH                      (1U)
#define DMA_ERQ_ERQ10(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ10_SHIFT)) & DMA_ERQ_ERQ10_MASK)

#define DMA_ERQ_ERQ11_MASK                       (0x800U)
#define DMA_ERQ_ERQ11_SHIFT                      (11U)
#define DMA_ERQ_ERQ11_WIDTH                      (1U)
#define DMA_ERQ_ERQ11(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ11_SHIFT)) & DMA_ERQ_ERQ11_MASK)

#define DMA_ERQ_ERQ12_MASK                       (0x1000U)
#define DMA_ERQ_ERQ12_SHIFT                      (12U)
#define DMA_ERQ_ERQ12_WIDTH                      (1U)
#define DMA_ERQ_ERQ12(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ12_SHIFT)) & DMA_ERQ_ERQ12_MASK)

#define DMA_ERQ_ERQ13_MASK                       (0x2000U)
#define DMA_ERQ_ERQ13_SHIFT                      (13U)
#define DMA_ERQ_ERQ13_WIDTH                      (1U)
#define DMA_ERQ_ERQ13(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ13_SHIFT)) & DMA_ERQ_ERQ13_MASK)

#define DMA_ERQ_ERQ14_MASK                       (0x4000U)
#define DMA_ERQ_ERQ14_SHIFT                      (14U)
#define DMA_ERQ_ERQ14_WIDTH                      (1U)
#define DMA_ERQ_ERQ14(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ14_SHIFT)) & DMA_ERQ_ERQ14_MASK)

#define DMA_ERQ_ERQ15_MASK                       (0x8000U)
#define DMA_ERQ_ERQ15_SHIFT                      (15U)
#define DMA_ERQ_ERQ15_WIDTH                      (1U)
#define DMA_ERQ_ERQ15(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERQ_ERQ15_SHIFT)) & DMA_ERQ_ERQ15_MASK)
/*! @} */

/*! @name EEI - Enable Error Interrupt */
/*! @{ */

#define DMA_EEI_EEI0_MASK                        (0x1U)
#define DMA_EEI_EEI0_SHIFT                       (0U)
#define DMA_EEI_EEI0_WIDTH                       (1U)
#define DMA_EEI_EEI0(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI0_SHIFT)) & DMA_EEI_EEI0_MASK)

#define DMA_EEI_EEI1_MASK                        (0x2U)
#define DMA_EEI_EEI1_SHIFT                       (1U)
#define DMA_EEI_EEI1_WIDTH                       (1U)
#define DMA_EEI_EEI1(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI1_SHIFT)) & DMA_EEI_EEI1_MASK)

#define DMA_EEI_EEI2_MASK                        (0x4U)
#define DMA_EEI_EEI2_SHIFT                       (2U)
#define DMA_EEI_EEI2_WIDTH                       (1U)
#define DMA_EEI_EEI2(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI2_SHIFT)) & DMA_EEI_EEI2_MASK)

#define DMA_EEI_EEI3_MASK                        (0x8U)
#define DMA_EEI_EEI3_SHIFT                       (3U)
#define DMA_EEI_EEI3_WIDTH                       (1U)
#define DMA_EEI_EEI3(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI3_SHIFT)) & DMA_EEI_EEI3_MASK)

#define DMA_EEI_EEI4_MASK                        (0x10U)
#define DMA_EEI_EEI4_SHIFT                       (4U)
#define DMA_EEI_EEI4_WIDTH                       (1U)
#define DMA_EEI_EEI4(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI4_SHIFT)) & DMA_EEI_EEI4_MASK)

#define DMA_EEI_EEI5_MASK                        (0x20U)
#define DMA_EEI_EEI5_SHIFT                       (5U)
#define DMA_EEI_EEI5_WIDTH                       (1U)
#define DMA_EEI_EEI5(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI5_SHIFT)) & DMA_EEI_EEI5_MASK)

#define DMA_EEI_EEI6_MASK                        (0x40U)
#define DMA_EEI_EEI6_SHIFT                       (6U)
#define DMA_EEI_EEI6_WIDTH                       (1U)
#define DMA_EEI_EEI6(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI6_SHIFT)) & DMA_EEI_EEI6_MASK)

#define DMA_EEI_EEI7_MASK                        (0x80U)
#define DMA_EEI_EEI7_SHIFT                       (7U)
#define DMA_EEI_EEI7_WIDTH                       (1U)
#define DMA_EEI_EEI7(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI7_SHIFT)) & DMA_EEI_EEI7_MASK)

#define DMA_EEI_EEI8_MASK                        (0x100U)
#define DMA_EEI_EEI8_SHIFT                       (8U)
#define DMA_EEI_EEI8_WIDTH                       (1U)
#define DMA_EEI_EEI8(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI8_SHIFT)) & DMA_EEI_EEI8_MASK)

#define DMA_EEI_EEI9_MASK                        (0x200U)
#define DMA_EEI_EEI9_SHIFT                       (9U)
#define DMA_EEI_EEI9_WIDTH                       (1U)
#define DMA_EEI_EEI9(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI9_SHIFT)) & DMA_EEI_EEI9_MASK)

#define DMA_EEI_EEI10_MASK                       (0x400U)
#define DMA_EEI_EEI10_SHIFT                      (10U)
#define DMA_EEI_EEI10_WIDTH                      (1U)
#define DMA_EEI_EEI10(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI10_SHIFT)) & DMA_EEI_EEI10_MASK)

#define DMA_EEI_EEI11_MASK                       (0x800U)
#define DMA_EEI_EEI11_SHIFT                      (11U)
#define DMA_EEI_EEI11_WIDTH                      (1U)
#define DMA_EEI_EEI11(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI11_SHIFT)) & DMA_EEI_EEI11_MASK)

#define DMA_EEI_EEI12_MASK                       (0x1000U)
#define DMA_EEI_EEI12_SHIFT                      (12U)
#define DMA_EEI_EEI12_WIDTH                      (1U)
#define DMA_EEI_EEI12(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI12_SHIFT)) & DMA_EEI_EEI12_MASK)

#define DMA_EEI_EEI13_MASK                       (0x2000U)
#define DMA_EEI_EEI13_SHIFT                      (13U)
#define DMA_EEI_EEI13_WIDTH                      (1U)
#define DMA_EEI_EEI13(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI13_SHIFT)) & DMA_EEI_EEI13_MASK)

#define DMA_EEI_EEI14_MASK                       (0x4000U)
#define DMA_EEI_EEI14_SHIFT                      (14U)
#define DMA_EEI_EEI14_WIDTH                      (1U)
#define DMA_EEI_EEI14(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI14_SHIFT)) & DMA_EEI_EEI14_MASK)

#define DMA_EEI_EEI15_MASK                       (0x8000U)
#define DMA_EEI_EEI15_SHIFT                      (15U)
#define DMA_EEI_EEI15_WIDTH                      (1U)
#define DMA_EEI_EEI15(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_EEI_EEI15_SHIFT)) & DMA_EEI_EEI15_MASK)
/*! @} */

/*! @name CEEI - Clear Enable Error Interrupt */
/*! @{ */

#define DMA_CEEI_CEEI_MASK                       (0xFU)
#define DMA_CEEI_CEEI_SHIFT                      (0U)
#define DMA_CEEI_CEEI_WIDTH                      (4U)
#define DMA_CEEI_CEEI(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CEEI_CEEI_SHIFT)) & DMA_CEEI_CEEI_MASK)

#define DMA_CEEI_CAEE_MASK                       (0x40U)
#define DMA_CEEI_CAEE_SHIFT                      (6U)
#define DMA_CEEI_CAEE_WIDTH                      (1U)
#define DMA_CEEI_CAEE(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CEEI_CAEE_SHIFT)) & DMA_CEEI_CAEE_MASK)

#define DMA_CEEI_NOP_MASK                        (0x80U)
#define DMA_CEEI_NOP_SHIFT                       (7U)
#define DMA_CEEI_NOP_WIDTH                       (1U)
#define DMA_CEEI_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_CEEI_NOP_SHIFT)) & DMA_CEEI_NOP_MASK)
/*! @} */

/*! @name SEEI - Set Enable Error Interrupt */
/*! @{ */

#define DMA_SEEI_SEEI_MASK                       (0xFU)
#define DMA_SEEI_SEEI_SHIFT                      (0U)
#define DMA_SEEI_SEEI_WIDTH                      (4U)
#define DMA_SEEI_SEEI(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SEEI_SEEI_SHIFT)) & DMA_SEEI_SEEI_MASK)

#define DMA_SEEI_SAEE_MASK                       (0x40U)
#define DMA_SEEI_SAEE_SHIFT                      (6U)
#define DMA_SEEI_SAEE_WIDTH                      (1U)
#define DMA_SEEI_SAEE(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SEEI_SAEE_SHIFT)) & DMA_SEEI_SAEE_MASK)

#define DMA_SEEI_NOP_MASK                        (0x80U)
#define DMA_SEEI_NOP_SHIFT                       (7U)
#define DMA_SEEI_NOP_WIDTH                       (1U)
#define DMA_SEEI_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_SEEI_NOP_SHIFT)) & DMA_SEEI_NOP_MASK)
/*! @} */

/*! @name CERQ - Clear Enable Request */
/*! @{ */

#define DMA_CERQ_CERQ_MASK                       (0xFU)
#define DMA_CERQ_CERQ_SHIFT                      (0U)
#define DMA_CERQ_CERQ_WIDTH                      (4U)
#define DMA_CERQ_CERQ(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERQ_CERQ_SHIFT)) & DMA_CERQ_CERQ_MASK)

#define DMA_CERQ_CAER_MASK                       (0x40U)
#define DMA_CERQ_CAER_SHIFT                      (6U)
#define DMA_CERQ_CAER_WIDTH                      (1U)
#define DMA_CERQ_CAER(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERQ_CAER_SHIFT)) & DMA_CERQ_CAER_MASK)

#define DMA_CERQ_NOP_MASK                        (0x80U)
#define DMA_CERQ_NOP_SHIFT                       (7U)
#define DMA_CERQ_NOP_WIDTH                       (1U)
#define DMA_CERQ_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_CERQ_NOP_SHIFT)) & DMA_CERQ_NOP_MASK)
/*! @} */

/*! @name SERQ - Set Enable Request */
/*! @{ */

#define DMA_SERQ_SERQ_MASK                       (0xFU)
#define DMA_SERQ_SERQ_SHIFT                      (0U)
#define DMA_SERQ_SERQ_WIDTH                      (4U)
#define DMA_SERQ_SERQ(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SERQ_SERQ_SHIFT)) & DMA_SERQ_SERQ_MASK)

#define DMA_SERQ_SAER_MASK                       (0x40U)
#define DMA_SERQ_SAER_SHIFT                      (6U)
#define DMA_SERQ_SAER_WIDTH                      (1U)
#define DMA_SERQ_SAER(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SERQ_SAER_SHIFT)) & DMA_SERQ_SAER_MASK)

#define DMA_SERQ_NOP_MASK                        (0x80U)
#define DMA_SERQ_NOP_SHIFT                       (7U)
#define DMA_SERQ_NOP_WIDTH                       (1U)
#define DMA_SERQ_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_SERQ_NOP_SHIFT)) & DMA_SERQ_NOP_MASK)
/*! @} */

/*! @name CDNE - Clear DONE Status Bit */
/*! @{ */

#define DMA_CDNE_CDNE_MASK                       (0xFU)
#define DMA_CDNE_CDNE_SHIFT                      (0U)
#define DMA_CDNE_CDNE_WIDTH                      (4U)
#define DMA_CDNE_CDNE(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CDNE_CDNE_SHIFT)) & DMA_CDNE_CDNE_MASK)

#define DMA_CDNE_CADN_MASK                       (0x40U)
#define DMA_CDNE_CADN_SHIFT                      (6U)
#define DMA_CDNE_CADN_WIDTH                      (1U)
#define DMA_CDNE_CADN(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CDNE_CADN_SHIFT)) & DMA_CDNE_CADN_MASK)

#define DMA_CDNE_NOP_MASK                        (0x80U)
#define DMA_CDNE_NOP_SHIFT                       (7U)
#define DMA_CDNE_NOP_WIDTH                       (1U)
#define DMA_CDNE_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_CDNE_NOP_SHIFT)) & DMA_CDNE_NOP_MASK)
/*! @} */

/*! @name SSRT - Set START Bit */
/*! @{ */

#define DMA_SSRT_SSRT_MASK                       (0xFU)
#define DMA_SSRT_SSRT_SHIFT                      (0U)
#define DMA_SSRT_SSRT_WIDTH                      (4U)
#define DMA_SSRT_SSRT(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SSRT_SSRT_SHIFT)) & DMA_SSRT_SSRT_MASK)

#define DMA_SSRT_SAST_MASK                       (0x40U)
#define DMA_SSRT_SAST_SHIFT                      (6U)
#define DMA_SSRT_SAST_WIDTH                      (1U)
#define DMA_SSRT_SAST(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_SSRT_SAST_SHIFT)) & DMA_SSRT_SAST_MASK)

#define DMA_SSRT_NOP_MASK                        (0x80U)
#define DMA_SSRT_NOP_SHIFT                       (7U)
#define DMA_SSRT_NOP_WIDTH                       (1U)
#define DMA_SSRT_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_SSRT_NOP_SHIFT)) & DMA_SSRT_NOP_MASK)
/*! @} */

/*! @name CERR - Clear Error */
/*! @{ */

#define DMA_CERR_CERR_MASK                       (0xFU)
#define DMA_CERR_CERR_SHIFT                      (0U)
#define DMA_CERR_CERR_WIDTH                      (4U)
#define DMA_CERR_CERR(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERR_CERR_SHIFT)) & DMA_CERR_CERR_MASK)

#define DMA_CERR_CAEI_MASK                       (0x40U)
#define DMA_CERR_CAEI_SHIFT                      (6U)
#define DMA_CERR_CAEI_WIDTH                      (1U)
#define DMA_CERR_CAEI(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CERR_CAEI_SHIFT)) & DMA_CERR_CAEI_MASK)

#define DMA_CERR_NOP_MASK                        (0x80U)
#define DMA_CERR_NOP_SHIFT                       (7U)
#define DMA_CERR_NOP_WIDTH                       (1U)
#define DMA_CERR_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_CERR_NOP_SHIFT)) & DMA_CERR_NOP_MASK)
/*! @} */

/*! @name CINT - Clear Interrupt Request */
/*! @{ */

#define DMA_CINT_CINT_MASK                       (0xFU)
#define DMA_CINT_CINT_SHIFT                      (0U)
#define DMA_CINT_CINT_WIDTH                      (4U)
#define DMA_CINT_CINT(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CINT_CINT_SHIFT)) & DMA_CINT_CINT_MASK)

#define DMA_CINT_CAIR_MASK                       (0x40U)
#define DMA_CINT_CAIR_SHIFT                      (6U)
#define DMA_CINT_CAIR_WIDTH                      (1U)
#define DMA_CINT_CAIR(x)                         (((uint8_t)(((uint8_t)(x)) << DMA_CINT_CAIR_SHIFT)) & DMA_CINT_CAIR_MASK)

#define DMA_CINT_NOP_MASK                        (0x80U)
#define DMA_CINT_NOP_SHIFT                       (7U)
#define DMA_CINT_NOP_WIDTH                       (1U)
#define DMA_CINT_NOP(x)                          (((uint8_t)(((uint8_t)(x)) << DMA_CINT_NOP_SHIFT)) & DMA_CINT_NOP_MASK)
/*! @} */

/*! @name INT - Interrupt Request */
/*! @{ */

#define DMA_INT_INT0_MASK                        (0x1U)
#define DMA_INT_INT0_SHIFT                       (0U)
#define DMA_INT_INT0_WIDTH                       (1U)
#define DMA_INT_INT0(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT0_SHIFT)) & DMA_INT_INT0_MASK)

#define DMA_INT_INT1_MASK                        (0x2U)
#define DMA_INT_INT1_SHIFT                       (1U)
#define DMA_INT_INT1_WIDTH                       (1U)
#define DMA_INT_INT1(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT1_SHIFT)) & DMA_INT_INT1_MASK)

#define DMA_INT_INT2_MASK                        (0x4U)
#define DMA_INT_INT2_SHIFT                       (2U)
#define DMA_INT_INT2_WIDTH                       (1U)
#define DMA_INT_INT2(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT2_SHIFT)) & DMA_INT_INT2_MASK)

#define DMA_INT_INT3_MASK                        (0x8U)
#define DMA_INT_INT3_SHIFT                       (3U)
#define DMA_INT_INT3_WIDTH                       (1U)
#define DMA_INT_INT3(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT3_SHIFT)) & DMA_INT_INT3_MASK)

#define DMA_INT_INT4_MASK                        (0x10U)
#define DMA_INT_INT4_SHIFT                       (4U)
#define DMA_INT_INT4_WIDTH                       (1U)
#define DMA_INT_INT4(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT4_SHIFT)) & DMA_INT_INT4_MASK)

#define DMA_INT_INT5_MASK                        (0x20U)
#define DMA_INT_INT5_SHIFT                       (5U)
#define DMA_INT_INT5_WIDTH                       (1U)
#define DMA_INT_INT5(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT5_SHIFT)) & DMA_INT_INT5_MASK)

#define DMA_INT_INT6_MASK                        (0x40U)
#define DMA_INT_INT6_SHIFT                       (6U)
#define DMA_INT_INT6_WIDTH                       (1U)
#define DMA_INT_INT6(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT6_SHIFT)) & DMA_INT_INT6_MASK)

#define DMA_INT_INT7_MASK                        (0x80U)
#define DMA_INT_INT7_SHIFT                       (7U)
#define DMA_INT_INT7_WIDTH                       (1U)
#define DMA_INT_INT7(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT7_SHIFT)) & DMA_INT_INT7_MASK)

#define DMA_INT_INT8_MASK                        (0x100U)
#define DMA_INT_INT8_SHIFT                       (8U)
#define DMA_INT_INT8_WIDTH                       (1U)
#define DMA_INT_INT8(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT8_SHIFT)) & DMA_INT_INT8_MASK)

#define DMA_INT_INT9_MASK                        (0x200U)
#define DMA_INT_INT9_SHIFT                       (9U)
#define DMA_INT_INT9_WIDTH                       (1U)
#define DMA_INT_INT9(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT9_SHIFT)) & DMA_INT_INT9_MASK)

#define DMA_INT_INT10_MASK                       (0x400U)
#define DMA_INT_INT10_SHIFT                      (10U)
#define DMA_INT_INT10_WIDTH                      (1U)
#define DMA_INT_INT10(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT10_SHIFT)) & DMA_INT_INT10_MASK)

#define DMA_INT_INT11_MASK                       (0x800U)
#define DMA_INT_INT11_SHIFT                      (11U)
#define DMA_INT_INT11_WIDTH                      (1U)
#define DMA_INT_INT11(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT11_SHIFT)) & DMA_INT_INT11_MASK)

#define DMA_INT_INT12_MASK                       (0x1000U)
#define DMA_INT_INT12_SHIFT                      (12U)
#define DMA_INT_INT12_WIDTH                      (1U)
#define DMA_INT_INT12(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT12_SHIFT)) & DMA_INT_INT12_MASK)

#define DMA_INT_INT13_MASK                       (0x2000U)
#define DMA_INT_INT13_SHIFT                      (13U)
#define DMA_INT_INT13_WIDTH                      (1U)
#define DMA_INT_INT13(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT13_SHIFT)) & DMA_INT_INT13_MASK)

#define DMA_INT_INT14_MASK                       (0x4000U)
#define DMA_INT_INT14_SHIFT                      (14U)
#define DMA_INT_INT14_WIDTH                      (1U)
#define DMA_INT_INT14(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT14_SHIFT)) & DMA_INT_INT14_MASK)

#define DMA_INT_INT15_MASK                       (0x8000U)
#define DMA_INT_INT15_SHIFT                      (15U)
#define DMA_INT_INT15_WIDTH                      (1U)
#define DMA_INT_INT15(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_INT_INT15_SHIFT)) & DMA_INT_INT15_MASK)
/*! @} */

/*! @name ERR - Error */
/*! @{ */

#define DMA_ERR_ERR0_MASK                        (0x1U)
#define DMA_ERR_ERR0_SHIFT                       (0U)
#define DMA_ERR_ERR0_WIDTH                       (1U)
#define DMA_ERR_ERR0(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR0_SHIFT)) & DMA_ERR_ERR0_MASK)

#define DMA_ERR_ERR1_MASK                        (0x2U)
#define DMA_ERR_ERR1_SHIFT                       (1U)
#define DMA_ERR_ERR1_WIDTH                       (1U)
#define DMA_ERR_ERR1(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR1_SHIFT)) & DMA_ERR_ERR1_MASK)

#define DMA_ERR_ERR2_MASK                        (0x4U)
#define DMA_ERR_ERR2_SHIFT                       (2U)
#define DMA_ERR_ERR2_WIDTH                       (1U)
#define DMA_ERR_ERR2(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR2_SHIFT)) & DMA_ERR_ERR2_MASK)

#define DMA_ERR_ERR3_MASK                        (0x8U)
#define DMA_ERR_ERR3_SHIFT                       (3U)
#define DMA_ERR_ERR3_WIDTH                       (1U)
#define DMA_ERR_ERR3(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR3_SHIFT)) & DMA_ERR_ERR3_MASK)

#define DMA_ERR_ERR4_MASK                        (0x10U)
#define DMA_ERR_ERR4_SHIFT                       (4U)
#define DMA_ERR_ERR4_WIDTH                       (1U)
#define DMA_ERR_ERR4(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR4_SHIFT)) & DMA_ERR_ERR4_MASK)

#define DMA_ERR_ERR5_MASK                        (0x20U)
#define DMA_ERR_ERR5_SHIFT                       (5U)
#define DMA_ERR_ERR5_WIDTH                       (1U)
#define DMA_ERR_ERR5(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR5_SHIFT)) & DMA_ERR_ERR5_MASK)

#define DMA_ERR_ERR6_MASK                        (0x40U)
#define DMA_ERR_ERR6_SHIFT                       (6U)
#define DMA_ERR_ERR6_WIDTH                       (1U)
#define DMA_ERR_ERR6(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR6_SHIFT)) & DMA_ERR_ERR6_MASK)

#define DMA_ERR_ERR7_MASK                        (0x80U)
#define DMA_ERR_ERR7_SHIFT                       (7U)
#define DMA_ERR_ERR7_WIDTH                       (1U)
#define DMA_ERR_ERR7(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR7_SHIFT)) & DMA_ERR_ERR7_MASK)

#define DMA_ERR_ERR8_MASK                        (0x100U)
#define DMA_ERR_ERR8_SHIFT                       (8U)
#define DMA_ERR_ERR8_WIDTH                       (1U)
#define DMA_ERR_ERR8(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR8_SHIFT)) & DMA_ERR_ERR8_MASK)

#define DMA_ERR_ERR9_MASK                        (0x200U)
#define DMA_ERR_ERR9_SHIFT                       (9U)
#define DMA_ERR_ERR9_WIDTH                       (1U)
#define DMA_ERR_ERR9(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR9_SHIFT)) & DMA_ERR_ERR9_MASK)

#define DMA_ERR_ERR10_MASK                       (0x400U)
#define DMA_ERR_ERR10_SHIFT                      (10U)
#define DMA_ERR_ERR10_WIDTH                      (1U)
#define DMA_ERR_ERR10(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR10_SHIFT)) & DMA_ERR_ERR10_MASK)

#define DMA_ERR_ERR11_MASK                       (0x800U)
#define DMA_ERR_ERR11_SHIFT                      (11U)
#define DMA_ERR_ERR11_WIDTH                      (1U)
#define DMA_ERR_ERR11(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR11_SHIFT)) & DMA_ERR_ERR11_MASK)

#define DMA_ERR_ERR12_MASK                       (0x1000U)
#define DMA_ERR_ERR12_SHIFT                      (12U)
#define DMA_ERR_ERR12_WIDTH                      (1U)
#define DMA_ERR_ERR12(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR12_SHIFT)) & DMA_ERR_ERR12_MASK)

#define DMA_ERR_ERR13_MASK                       (0x2000U)
#define DMA_ERR_ERR13_SHIFT                      (13U)
#define DMA_ERR_ERR13_WIDTH                      (1U)
#define DMA_ERR_ERR13(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR13_SHIFT)) & DMA_ERR_ERR13_MASK)

#define DMA_ERR_ERR14_MASK                       (0x4000U)
#define DMA_ERR_ERR14_SHIFT                      (14U)
#define DMA_ERR_ERR14_WIDTH                      (1U)
#define DMA_ERR_ERR14(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR14_SHIFT)) & DMA_ERR_ERR14_MASK)

#define DMA_ERR_ERR15_MASK                       (0x8000U)
#define DMA_ERR_ERR15_SHIFT                      (15U)
#define DMA_ERR_ERR15_WIDTH                      (1U)
#define DMA_ERR_ERR15(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_ERR_ERR15_SHIFT)) & DMA_ERR_ERR15_MASK)
/*! @} */

/*! @name HRS - Hardware Request Status */
/*! @{ */

#define DMA_HRS_HRS0_MASK                        (0x1U)
#define DMA_HRS_HRS0_SHIFT                       (0U)
#define DMA_HRS_HRS0_WIDTH                       (1U)
#define DMA_HRS_HRS0(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS0_SHIFT)) & DMA_HRS_HRS0_MASK)

#define DMA_HRS_HRS1_MASK                        (0x2U)
#define DMA_HRS_HRS1_SHIFT                       (1U)
#define DMA_HRS_HRS1_WIDTH                       (1U)
#define DMA_HRS_HRS1(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS1_SHIFT)) & DMA_HRS_HRS1_MASK)

#define DMA_HRS_HRS2_MASK                        (0x4U)
#define DMA_HRS_HRS2_SHIFT                       (2U)
#define DMA_HRS_HRS2_WIDTH                       (1U)
#define DMA_HRS_HRS2(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS2_SHIFT)) & DMA_HRS_HRS2_MASK)

#define DMA_HRS_HRS3_MASK                        (0x8U)
#define DMA_HRS_HRS3_SHIFT                       (3U)
#define DMA_HRS_HRS3_WIDTH                       (1U)
#define DMA_HRS_HRS3(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS3_SHIFT)) & DMA_HRS_HRS3_MASK)

#define DMA_HRS_HRS4_MASK                        (0x10U)
#define DMA_HRS_HRS4_SHIFT                       (4U)
#define DMA_HRS_HRS4_WIDTH                       (1U)
#define DMA_HRS_HRS4(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS4_SHIFT)) & DMA_HRS_HRS4_MASK)

#define DMA_HRS_HRS5_MASK                        (0x20U)
#define DMA_HRS_HRS5_SHIFT                       (5U)
#define DMA_HRS_HRS5_WIDTH                       (1U)
#define DMA_HRS_HRS5(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS5_SHIFT)) & DMA_HRS_HRS5_MASK)

#define DMA_HRS_HRS6_MASK                        (0x40U)
#define DMA_HRS_HRS6_SHIFT                       (6U)
#define DMA_HRS_HRS6_WIDTH                       (1U)
#define DMA_HRS_HRS6(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS6_SHIFT)) & DMA_HRS_HRS6_MASK)

#define DMA_HRS_HRS7_MASK                        (0x80U)
#define DMA_HRS_HRS7_SHIFT                       (7U)
#define DMA_HRS_HRS7_WIDTH                       (1U)
#define DMA_HRS_HRS7(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS7_SHIFT)) & DMA_HRS_HRS7_MASK)

#define DMA_HRS_HRS8_MASK                        (0x100U)
#define DMA_HRS_HRS8_SHIFT                       (8U)
#define DMA_HRS_HRS8_WIDTH                       (1U)
#define DMA_HRS_HRS8(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS8_SHIFT)) & DMA_HRS_HRS8_MASK)

#define DMA_HRS_HRS9_MASK                        (0x200U)
#define DMA_HRS_HRS9_SHIFT                       (9U)
#define DMA_HRS_HRS9_WIDTH                       (1U)
#define DMA_HRS_HRS9(x)                          (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS9_SHIFT)) & DMA_HRS_HRS9_MASK)

#define DMA_HRS_HRS10_MASK                       (0x400U)
#define DMA_HRS_HRS10_SHIFT                      (10U)
#define DMA_HRS_HRS10_WIDTH                      (1U)
#define DMA_HRS_HRS10(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS10_SHIFT)) & DMA_HRS_HRS10_MASK)

#define DMA_HRS_HRS11_MASK                       (0x800U)
#define DMA_HRS_HRS11_SHIFT                      (11U)
#define DMA_HRS_HRS11_WIDTH                      (1U)
#define DMA_HRS_HRS11(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS11_SHIFT)) & DMA_HRS_HRS11_MASK)

#define DMA_HRS_HRS12_MASK                       (0x1000U)
#define DMA_HRS_HRS12_SHIFT                      (12U)
#define DMA_HRS_HRS12_WIDTH                      (1U)
#define DMA_HRS_HRS12(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS12_SHIFT)) & DMA_HRS_HRS12_MASK)

#define DMA_HRS_HRS13_MASK                       (0x2000U)
#define DMA_HRS_HRS13_SHIFT                      (13U)
#define DMA_HRS_HRS13_WIDTH                      (1U)
#define DMA_HRS_HRS13(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS13_SHIFT)) & DMA_HRS_HRS13_MASK)

#define DMA_HRS_HRS14_MASK                       (0x4000U)
#define DMA_HRS_HRS14_SHIFT                      (14U)
#define DMA_HRS_HRS14_WIDTH                      (1U)
#define DMA_HRS_HRS14(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS14_SHIFT)) & DMA_HRS_HRS14_MASK)

#define DMA_HRS_HRS15_MASK                       (0x8000U)
#define DMA_HRS_HRS15_SHIFT                      (15U)
#define DMA_HRS_HRS15_WIDTH                      (1U)
#define DMA_HRS_HRS15(x)                         (((uint32_t)(((uint32_t)(x)) << DMA_HRS_HRS15_SHIFT)) & DMA_HRS_HRS15_MASK)
/*! @} */

/*! @name EARS - Enable Asynchronous Request in Stop */
/*! @{ */

#define DMA_EARS_EDREQ_0_MASK                    (0x1U)
#define DMA_EARS_EDREQ_0_SHIFT                   (0U)
#define DMA_EARS_EDREQ_0_WIDTH                   (1U)
#define DMA_EARS_EDREQ_0(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_0_SHIFT)) & DMA_EARS_EDREQ_0_MASK)

#define DMA_EARS_EDREQ_1_MASK                    (0x2U)
#define DMA_EARS_EDREQ_1_SHIFT                   (1U)
#define DMA_EARS_EDREQ_1_WIDTH                   (1U)
#define DMA_EARS_EDREQ_1(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_1_SHIFT)) & DMA_EARS_EDREQ_1_MASK)

#define DMA_EARS_EDREQ_2_MASK                    (0x4U)
#define DMA_EARS_EDREQ_2_SHIFT                   (2U)
#define DMA_EARS_EDREQ_2_WIDTH                   (1U)
#define DMA_EARS_EDREQ_2(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_2_SHIFT)) & DMA_EARS_EDREQ_2_MASK)

#define DMA_EARS_EDREQ_3_MASK                    (0x8U)
#define DMA_EARS_EDREQ_3_SHIFT                   (3U)
#define DMA_EARS_EDREQ_3_WIDTH                   (1U)
#define DMA_EARS_EDREQ_3(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_3_SHIFT)) & DMA_EARS_EDREQ_3_MASK)

#define DMA_EARS_EDREQ_4_MASK                    (0x10U)
#define DMA_EARS_EDREQ_4_SHIFT                   (4U)
#define DMA_EARS_EDREQ_4_WIDTH                   (1U)
#define DMA_EARS_EDREQ_4(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_4_SHIFT)) & DMA_EARS_EDREQ_4_MASK)

#define DMA_EARS_EDREQ_5_MASK                    (0x20U)
#define DMA_EARS_EDREQ_5_SHIFT                   (5U)
#define DMA_EARS_EDREQ_5_WIDTH                   (1U)
#define DMA_EARS_EDREQ_5(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_5_SHIFT)) & DMA_EARS_EDREQ_5_MASK)

#define DMA_EARS_EDREQ_6_MASK                    (0x40U)
#define DMA_EARS_EDREQ_6_SHIFT                   (6U)
#define DMA_EARS_EDREQ_6_WIDTH                   (1U)
#define DMA_EARS_EDREQ_6(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_6_SHIFT)) & DMA_EARS_EDREQ_6_MASK)

#define DMA_EARS_EDREQ_7_MASK                    (0x80U)
#define DMA_EARS_EDREQ_7_SHIFT                   (7U)
#define DMA_EARS_EDREQ_7_WIDTH                   (1U)
#define DMA_EARS_EDREQ_7(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_7_SHIFT)) & DMA_EARS_EDREQ_7_MASK)

#define DMA_EARS_EDREQ_8_MASK                    (0x100U)
#define DMA_EARS_EDREQ_8_SHIFT                   (8U)
#define DMA_EARS_EDREQ_8_WIDTH                   (1U)
#define DMA_EARS_EDREQ_8(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_8_SHIFT)) & DMA_EARS_EDREQ_8_MASK)

#define DMA_EARS_EDREQ_9_MASK                    (0x200U)
#define DMA_EARS_EDREQ_9_SHIFT                   (9U)
#define DMA_EARS_EDREQ_9_WIDTH                   (1U)
#define DMA_EARS_EDREQ_9(x)                      (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_9_SHIFT)) & DMA_EARS_EDREQ_9_MASK)

#define DMA_EARS_EDREQ_10_MASK                   (0x400U)
#define DMA_EARS_EDREQ_10_SHIFT                  (10U)
#define DMA_EARS_EDREQ_10_WIDTH                  (1U)
#define DMA_EARS_EDREQ_10(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_10_SHIFT)) & DMA_EARS_EDREQ_10_MASK)

#define DMA_EARS_EDREQ_11_MASK                   (0x800U)
#define DMA_EARS_EDREQ_11_SHIFT                  (11U)
#define DMA_EARS_EDREQ_11_WIDTH                  (1U)
#define DMA_EARS_EDREQ_11(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_11_SHIFT)) & DMA_EARS_EDREQ_11_MASK)

#define DMA_EARS_EDREQ_12_MASK                   (0x1000U)
#define DMA_EARS_EDREQ_12_SHIFT                  (12U)
#define DMA_EARS_EDREQ_12_WIDTH                  (1U)
#define DMA_EARS_EDREQ_12(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_12_SHIFT)) & DMA_EARS_EDREQ_12_MASK)

#define DMA_EARS_EDREQ_13_MASK                   (0x2000U)
#define DMA_EARS_EDREQ_13_SHIFT                  (13U)
#define DMA_EARS_EDREQ_13_WIDTH                  (1U)
#define DMA_EARS_EDREQ_13(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_13_SHIFT)) & DMA_EARS_EDREQ_13_MASK)

#define DMA_EARS_EDREQ_14_MASK                   (0x4000U)
#define DMA_EARS_EDREQ_14_SHIFT                  (14U)
#define DMA_EARS_EDREQ_14_WIDTH                  (1U)
#define DMA_EARS_EDREQ_14(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_14_SHIFT)) & DMA_EARS_EDREQ_14_MASK)

#define DMA_EARS_EDREQ_15_MASK                   (0x8000U)
#define DMA_EARS_EDREQ_15_SHIFT                  (15U)
#define DMA_EARS_EDREQ_15_WIDTH                  (1U)
#define DMA_EARS_EDREQ_15(x)                     (((uint32_t)(((uint32_t)(x)) << DMA_EARS_EDREQ_15_SHIFT)) & DMA_EARS_EDREQ_15_MASK)
/*! @} */

/*! @name DCHPRI - Channel Priority */
/*! @{ */

#define DMA_DCHPRI_CHPRI_MASK                    (0xFU)
#define DMA_DCHPRI_CHPRI_SHIFT                   (0U)
#define DMA_DCHPRI_CHPRI_WIDTH                   (4U)
#define DMA_DCHPRI_CHPRI(x)                      (((uint8_t)(((uint8_t)(x)) << DMA_DCHPRI_CHPRI_SHIFT)) & DMA_DCHPRI_CHPRI_MASK)

#define DMA_DCHPRI_DPA_MASK                      (0x40U)
#define DMA_DCHPRI_DPA_SHIFT                     (6U)
#define DMA_DCHPRI_DPA_WIDTH                     (1U)
#define DMA_DCHPRI_DPA(x)                        (((uint8_t)(((uint8_t)(x)) << DMA_DCHPRI_DPA_SHIFT)) & DMA_DCHPRI_DPA_MASK)

#define DMA_DCHPRI_ECP_MASK                      (0x80U)
#define DMA_DCHPRI_ECP_SHIFT                     (7U)
#define DMA_DCHPRI_ECP_WIDTH                     (1U)
#define DMA_DCHPRI_ECP(x)                        (((uint8_t)(((uint8_t)(x)) << DMA_DCHPRI_ECP_SHIFT)) & DMA_DCHPRI_ECP_MASK)
/*! @} */

/*! @name TCD_SADDR - TCD Source Address */
/*! @{ */

#define DMA_TCD_SADDR_SADDR_MASK                 (0xFFFFFFFFU)
#define DMA_TCD_SADDR_SADDR_SHIFT                (0U)
#define DMA_TCD_SADDR_SADDR_WIDTH                (32U)
#define DMA_TCD_SADDR_SADDR(x)                   (((uint32_t)(((uint32_t)(x)) << DMA_TCD_SADDR_SADDR_SHIFT)) & DMA_TCD_SADDR_SADDR_MASK)
/*! @} */

/*! @name TCD_SOFF - TCD Signed Source Address Offset */
/*! @{ */

#define DMA_TCD_SOFF_SOFF_MASK                   (0xFFFFU)
#define DMA_TCD_SOFF_SOFF_SHIFT                  (0U)
#define DMA_TCD_SOFF_SOFF_WIDTH                  (16U)
#define DMA_TCD_SOFF_SOFF(x)                     (((uint16_t)(((uint16_t)(x)) << DMA_TCD_SOFF_SOFF_SHIFT)) & DMA_TCD_SOFF_SOFF_MASK)
/*! @} */

/*! @name TCD_ATTR - TCD Transfer Attributes */
/*! @{ */

#define DMA_TCD_ATTR_DSIZE_MASK                  (0x7U)
#define DMA_TCD_ATTR_DSIZE_SHIFT                 (0U)
#define DMA_TCD_ATTR_DSIZE_WIDTH                 (3U)
#define DMA_TCD_ATTR_DSIZE(x)                    (((uint16_t)(((uint16_t)(x)) << DMA_TCD_ATTR_DSIZE_SHIFT)) & DMA_TCD_ATTR_DSIZE_MASK)

#define DMA_TCD_ATTR_DMOD_MASK                   (0xF8U)
#define DMA_TCD_ATTR_DMOD_SHIFT                  (3U)
#define DMA_TCD_ATTR_DMOD_WIDTH                  (5U)
#define DMA_TCD_ATTR_DMOD(x)                     (((uint16_t)(((uint16_t)(x)) << DMA_TCD_ATTR_DMOD_SHIFT)) & DMA_TCD_ATTR_DMOD_MASK)

#define DMA_TCD_ATTR_SSIZE_MASK                  (0x700U)
#define DMA_TCD_ATTR_SSIZE_SHIFT                 (8U)
#define DMA_TCD_ATTR_SSIZE_WIDTH                 (3U)
#define DMA_TCD_ATTR_SSIZE(x)                    (((uint16_t)(((uint16_t)(x)) << DMA_TCD_ATTR_SSIZE_SHIFT)) & DMA_TCD_ATTR_SSIZE_MASK)

#define DMA_TCD_ATTR_SMOD_MASK                   (0xF800U)
#define DMA_TCD_ATTR_SMOD_SHIFT                  (11U)
#define DMA_TCD_ATTR_SMOD_WIDTH                  (5U)
#define DMA_TCD_ATTR_SMOD(x)                     (((uint16_t)(((uint16_t)(x)) << DMA_TCD_ATTR_SMOD_SHIFT)) & DMA_TCD_ATTR_SMOD_MASK)
/*! @} */

/*! @name TCD_NBYTES_MLNO - TCD Minor Byte Count (Minor Loop Mapping Disabled) */
/*! @{ */

#define DMA_TCD_NBYTES_MLNO_NBYTES_MASK          (0xFFFFFFFFU)
#define DMA_TCD_NBYTES_MLNO_NBYTES_SHIFT         (0U)
#define DMA_TCD_NBYTES_MLNO_NBYTES_WIDTH         (32U)
#define DMA_TCD_NBYTES_MLNO_NBYTES(x)            (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLNO_NBYTES_SHIFT)) & DMA_TCD_NBYTES_MLNO_NBYTES_MASK)
/*! @} */

/*! @name TCD_NBYTES_MLOFFNO - TCD Signed Minor Loop Offset (Minor Loop Mapping Enabled and Offset Disabled) */
/*! @{ */

#define DMA_TCD_NBYTES_MLOFFNO_NBYTES_MASK       (0x3FFFFFFFU)
#define DMA_TCD_NBYTES_MLOFFNO_NBYTES_SHIFT      (0U)
#define DMA_TCD_NBYTES_MLOFFNO_NBYTES_WIDTH      (30U)
#define DMA_TCD_NBYTES_MLOFFNO_NBYTES(x)         (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFNO_NBYTES_SHIFT)) & DMA_TCD_NBYTES_MLOFFNO_NBYTES_MASK)

#define DMA_TCD_NBYTES_MLOFFNO_DMLOE_MASK        (0x40000000U)
#define DMA_TCD_NBYTES_MLOFFNO_DMLOE_SHIFT       (30U)
#define DMA_TCD_NBYTES_MLOFFNO_DMLOE_WIDTH       (1U)
#define DMA_TCD_NBYTES_MLOFFNO_DMLOE(x)          (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFNO_DMLOE_SHIFT)) & DMA_TCD_NBYTES_MLOFFNO_DMLOE_MASK)

#define DMA_TCD_NBYTES_MLOFFNO_SMLOE_MASK        (0x80000000U)
#define DMA_TCD_NBYTES_MLOFFNO_SMLOE_SHIFT       (31U)
#define DMA_TCD_NBYTES_MLOFFNO_SMLOE_WIDTH       (1U)
#define DMA_TCD_NBYTES_MLOFFNO_SMLOE(x)          (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFNO_SMLOE_SHIFT)) & DMA_TCD_NBYTES_MLOFFNO_SMLOE_MASK)
/*! @} */

/*! @name TCD_NBYTES_MLOFFYES - TCD Signed Minor Loop Offset (Minor Loop Mapping and Offset Enabled) */
/*! @{ */

#define DMA_TCD_NBYTES_MLOFFYES_NBYTES_MASK      (0x3FFU)
#define DMA_TCD_NBYTES_MLOFFYES_NBYTES_SHIFT     (0U)
#define DMA_TCD_NBYTES_MLOFFYES_NBYTES_WIDTH     (10U)
#define DMA_TCD_NBYTES_MLOFFYES_NBYTES(x)        (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFYES_NBYTES_SHIFT)) & DMA_TCD_NBYTES_MLOFFYES_NBYTES_MASK)

#define DMA_TCD_NBYTES_MLOFFYES_MLOFF_MASK       (0x3FFFFC00U)
#define DMA_TCD_NBYTES_MLOFFYES_MLOFF_SHIFT      (10U)
#define DMA_TCD_NBYTES_MLOFFYES_MLOFF_WIDTH      (20U)
#define DMA_TCD_NBYTES_MLOFFYES_MLOFF(x)         (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFYES_MLOFF_SHIFT)) & DMA_TCD_NBYTES_MLOFFYES_MLOFF_MASK)

#define DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK       (0x40000000U)
#define DMA_TCD_NBYTES_MLOFFYES_DMLOE_SHIFT      (30U)
#define DMA_TCD_NBYTES_MLOFFYES_DMLOE_WIDTH      (1U)
#define DMA_TCD_NBYTES_MLOFFYES_DMLOE(x)         (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFYES_DMLOE_SHIFT)) & DMA_TCD_NBYTES_MLOFFYES_DMLOE_MASK)

#define DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK       (0x80000000U)
#define DMA_TCD_NBYTES_MLOFFYES_SMLOE_SHIFT      (31U)
#define DMA_TCD_NBYTES_MLOFFYES_SMLOE_WIDTH      (1U)
#define DMA_TCD_NBYTES_MLOFFYES_SMLOE(x)         (((uint32_t)(((uint32_t)(x)) << DMA_TCD_NBYTES_MLOFFYES_SMLOE_SHIFT)) & DMA_TCD_NBYTES_MLOFFYES_SMLOE_MASK)
/*! @} */

/*! @name TCD_SLAST - TCD Last Source Address Adjustment */
/*! @{ */

#define DMA_TCD_SLAST_SLAST_MASK                 (0xFFFFFFFFU)
#define DMA_TCD_SLAST_SLAST_SHIFT                (0U)
#define DMA_TCD_SLAST_SLAST_WIDTH                (32U)
#define DMA_TCD_SLAST_SLAST(x)                   (((uint32_t)(((uint32_t)(x)) << DMA_TCD_SLAST_SLAST_SHIFT)) & DMA_TCD_SLAST_SLAST_MASK)
/*! @} */

/*! @name TCD_DADDR - TCD Destination Address */
/*! @{ */

#define DMA_TCD_DADDR_DADDR_MASK                 (0xFFFFFFFFU)
#define DMA_TCD_DADDR_DADDR_SHIFT                (0U)
#define DMA_TCD_DADDR_DADDR_WIDTH                (32U)
#define DMA_TCD_DADDR_DADDR(x)                   (((uint32_t)(((uint32_t)(x)) << DMA_TCD_DADDR_DADDR_SHIFT)) & DMA_TCD_DADDR_DADDR_MASK)
/*! @} */

/*! @name TCD_DOFF - TCD Signed Destination Address Offset */
/*! @{ */

#define DMA_TCD_DOFF_DOFF_MASK                   (0xFFFFU)
#define DMA_TCD_DOFF_DOFF_SHIFT                  (0U)
#define DMA_TCD_DOFF_DOFF_WIDTH                  (16U)
#define DMA_TCD_DOFF_DOFF(x)                     (((uint16_t)(((uint16_t)(x)) << DMA_TCD_DOFF_DOFF_SHIFT)) & DMA_TCD_DOFF_DOFF_MASK)
/*! @} */

/*! @name TCD_CITER_ELINKNO - TCD Current Minor Loop Link, Major Loop Count (Channel Linking Disabled) */
/*! @{ */

#define DMA_TCD_CITER_ELINKNO_CITER_MASK         (0x7FFFU)
#define DMA_TCD_CITER_ELINKNO_CITER_SHIFT        (0U)
#define DMA_TCD_CITER_ELINKNO_CITER_WIDTH        (15U)
#define DMA_TCD_CITER_ELINKNO_CITER(x)           (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CITER_ELINKNO_CITER_SHIFT)) & DMA_TCD_CITER_ELINKNO_CITER_MASK)

#define DMA_TCD_CITER_ELINKNO_ELINK_MASK         (0x8000U)
#define DMA_TCD_CITER_ELINKNO_ELINK_SHIFT        (15U)
#define DMA_TCD_CITER_ELINKNO_ELINK_WIDTH        (1U)
#define DMA_TCD_CITER_ELINKNO_ELINK(x)           (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CITER_ELINKNO_ELINK_SHIFT)) & DMA_TCD_CITER_ELINKNO_ELINK_MASK)
/*! @} */

/*! @name TCD_CITER_ELINKYES - TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled) */
/*! @{ */

#define DMA_TCD_CITER_ELINKYES_CITER_MASK        (0x1FFU)
#define DMA_TCD_CITER_ELINKYES_CITER_SHIFT       (0U)
#define DMA_TCD_CITER_ELINKYES_CITER_WIDTH       (9U)
#define DMA_TCD_CITER_ELINKYES_CITER(x)          (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CITER_ELINKYES_CITER_SHIFT)) & DMA_TCD_CITER_ELINKYES_CITER_MASK)

#define DMA_TCD_CITER_ELINKYES_LINKCH_MASK       (0x1E00U)
#define DMA_TCD_CITER_ELINKYES_LINKCH_SHIFT      (9U)
#define DMA_TCD_CITER_ELINKYES_LINKCH_WIDTH      (4U)
#define DMA_TCD_CITER_ELINKYES_LINKCH(x)         (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CITER_ELINKYES_LINKCH_SHIFT)) & DMA_TCD_CITER_ELINKYES_LINKCH_MASK)

#define DMA_TCD_CITER_ELINKYES_ELINK_MASK        (0x8000U)
#define DMA_TCD_CITER_ELINKYES_ELINK_SHIFT       (15U)
#define DMA_TCD_CITER_ELINKYES_ELINK_WIDTH       (1U)
#define DMA_TCD_CITER_ELINKYES_ELINK(x)          (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CITER_ELINKYES_ELINK_SHIFT)) & DMA_TCD_CITER_ELINKYES_ELINK_MASK)
/*! @} */

/*! @name TCD_DLASTSGA - TCD Last Destination Address Adjustment/Scatter Gather Address */
/*! @{ */

#define DMA_TCD_DLASTSGA_DLASTSGA_MASK           (0xFFFFFFFFU)
#define DMA_TCD_DLASTSGA_DLASTSGA_SHIFT          (0U)
#define DMA_TCD_DLASTSGA_DLASTSGA_WIDTH          (32U)
#define DMA_TCD_DLASTSGA_DLASTSGA(x)             (((uint32_t)(((uint32_t)(x)) << DMA_TCD_DLASTSGA_DLASTSGA_SHIFT)) & DMA_TCD_DLASTSGA_DLASTSGA_MASK)
/*! @} */

/*! @name TCD_CSR - TCD Control and Status */
/*! @{ */

#define DMA_TCD_CSR_START_MASK                   (0x1U)
#define DMA_TCD_CSR_START_SHIFT                  (0U)
#define DMA_TCD_CSR_START_WIDTH                  (1U)
#define DMA_TCD_CSR_START(x)                     (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_START_SHIFT)) & DMA_TCD_CSR_START_MASK)

#define DMA_TCD_CSR_INTMAJOR_MASK                (0x2U)
#define DMA_TCD_CSR_INTMAJOR_SHIFT               (1U)
#define DMA_TCD_CSR_INTMAJOR_WIDTH               (1U)
#define DMA_TCD_CSR_INTMAJOR(x)                  (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_INTMAJOR_SHIFT)) & DMA_TCD_CSR_INTMAJOR_MASK)

#define DMA_TCD_CSR_INTHALF_MASK                 (0x4U)
#define DMA_TCD_CSR_INTHALF_SHIFT                (2U)
#define DMA_TCD_CSR_INTHALF_WIDTH                (1U)
#define DMA_TCD_CSR_INTHALF(x)                   (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_INTHALF_SHIFT)) & DMA_TCD_CSR_INTHALF_MASK)

#define DMA_TCD_CSR_DREQ_MASK                    (0x8U)
#define DMA_TCD_CSR_DREQ_SHIFT                   (3U)
#define DMA_TCD_CSR_DREQ_WIDTH                   (1U)
#define DMA_TCD_CSR_DREQ(x)                      (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_DREQ_SHIFT)) & DMA_TCD_CSR_DREQ_MASK)

#define DMA_TCD_CSR_ESG_MASK                     (0x10U)
#define DMA_TCD_CSR_ESG_SHIFT                    (4U)
#define DMA_TCD_CSR_ESG_WIDTH                    (1U)
#define DMA_TCD_CSR_ESG(x)                       (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_ESG_SHIFT)) & DMA_TCD_CSR_ESG_MASK)

#define DMA_TCD_CSR_MAJORELINK_MASK              (0x20U)
#define DMA_TCD_CSR_MAJORELINK_SHIFT             (5U)
#define DMA_TCD_CSR_MAJORELINK_WIDTH             (1U)
#define DMA_TCD_CSR_MAJORELINK(x)                (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_MAJORELINK_SHIFT)) & DMA_TCD_CSR_MAJORELINK_MASK)

#define DMA_TCD_CSR_ACTIVE_MASK                  (0x40U)
#define DMA_TCD_CSR_ACTIVE_SHIFT                 (6U)
#define DMA_TCD_CSR_ACTIVE_WIDTH                 (1U)
#define DMA_TCD_CSR_ACTIVE(x)                    (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_ACTIVE_SHIFT)) & DMA_TCD_CSR_ACTIVE_MASK)

#define DMA_TCD_CSR_DONE_MASK                    (0x80U)
#define DMA_TCD_CSR_DONE_SHIFT                   (7U)
#define DMA_TCD_CSR_DONE_WIDTH                   (1U)
#define DMA_TCD_CSR_DONE(x)                      (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_DONE_SHIFT)) & DMA_TCD_CSR_DONE_MASK)

#define DMA_TCD_CSR_MAJORLINKCH_MASK             (0xF00U)
#define DMA_TCD_CSR_MAJORLINKCH_SHIFT            (8U)
#define DMA_TCD_CSR_MAJORLINKCH_WIDTH            (4U)
#define DMA_TCD_CSR_MAJORLINKCH(x)               (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_MAJORLINKCH_SHIFT)) & DMA_TCD_CSR_MAJORLINKCH_MASK)

#define DMA_TCD_CSR_BWC_MASK                     (0xC000U)
#define DMA_TCD_CSR_BWC_SHIFT                    (14U)
#define DMA_TCD_CSR_BWC_WIDTH                    (2U)
#define DMA_TCD_CSR_BWC(x)                       (((uint16_t)(((uint16_t)(x)) << DMA_TCD_CSR_BWC_SHIFT)) & DMA_TCD_CSR_BWC_MASK)
/*! @} */

/*! @name TCD_BITER_ELINKNO - TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) */
/*! @{ */

#define DMA_TCD_BITER_ELINKNO_BITER_MASK         (0x7FFFU)
#define DMA_TCD_BITER_ELINKNO_BITER_SHIFT        (0U)
#define DMA_TCD_BITER_ELINKNO_BITER_WIDTH        (15U)
#define DMA_TCD_BITER_ELINKNO_BITER(x)           (((uint16_t)(((uint16_t)(x)) << DMA_TCD_BITER_ELINKNO_BITER_SHIFT)) & DMA_TCD_BITER_ELINKNO_BITER_MASK)

#define DMA_TCD_BITER_ELINKNO_ELINK_MASK         (0x8000U)
#define DMA_TCD_BITER_ELINKNO_ELINK_SHIFT        (15U)
#define DMA_TCD_BITER_ELINKNO_ELINK_WIDTH        (1U)
#define DMA_TCD_BITER_ELINKNO_ELINK(x)           (((uint16_t)(((uint16_t)(x)) << DMA_TCD_BITER_ELINKNO_ELINK_SHIFT)) & DMA_TCD_BITER_ELINKNO_ELINK_MASK)
/*! @} */

/*! @name TCD_BITER_ELINKYES - TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) */
/*! @{ */

#define DMA_TCD_BITER_ELINKYES_BITER_MASK        (0x1FFU)
#define DMA_TCD_BITER_ELINKYES_BITER_SHIFT       (0U)
#define DMA_TCD_BITER_ELINKYES_BITER_WIDTH       (9U)
#define DMA_TCD_BITER_ELINKYES_BITER(x)          (((uint16_t)(((uint16_t)(x)) << DMA_TCD_BITER_ELINKYES_BITER_SHIFT)) & DMA_TCD_BITER_ELINKYES_BITER_MASK)

#define DMA_TCD_BITER_ELINKYES_LINKCH_MASK       (0x1E00U)
#define DMA_TCD_BITER_ELINKYES_LINKCH_SHIFT      (9U)
#define DMA_TCD_BITER_ELINKYES_LINKCH_WIDTH      (4U)
#define DMA_TCD_BITER_ELINKYES_LINKCH(x)         (((uint16_t)(((uint16_t)(x)) << DMA_TCD_BITER_ELINKYES_LINKCH_SHIFT)) & DMA_TCD_BITER_ELINKYES_LINKCH_MASK)

#define DMA_TCD_BITER_ELINKYES_ELINK_MASK        (0x8000U)
#define DMA_TCD_BITER_ELINKYES_ELINK_SHIFT       (15U)
#define DMA_TCD_BITER_ELINKYES_ELINK_WIDTH       (1U)
#define DMA_TCD_BITER_ELINKYES_ELINK(x)          (((uint16_t)(((uint16_t)(x)) << DMA_TCD_BITER_ELINKYES_ELINK_SHIFT)) & DMA_TCD_BITER_ELINKYES_ELINK_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group DMA_Register_Masks */

/*!
 * @}
 */ /* end of group DMA_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_DMA_H_) */

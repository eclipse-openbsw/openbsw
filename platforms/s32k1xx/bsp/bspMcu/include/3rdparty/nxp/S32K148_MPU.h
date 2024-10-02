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
 * @file S32K148_MPU.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_MPU
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
#if !defined(S32K148_MPU_H_)  /* Check if memory map has not been already included */
#define S32K148_MPU_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- MPU Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MPU_Peripheral_Access_Layer MPU Peripheral Access Layer
 * @{
 */

/** MPU - Size of Registers Arrays */
#define MPU_EAR_EDR_COUNT                         5u
#define MPU_RGD_COUNT                             16u
#define MPU_RGDAAC_COUNT                          16u

/** MPU - Register Layout Typedef */
typedef struct {
  __IO uint32_t CESR;                              /**< Control/Error Status Register, offset: 0x0 */
  uint8_t RESERVED_0[12];
  struct {                                         /* offset: 0x10, array step: 0x8 */
    __I  uint32_t EAR;                               /**< Error Address Register, slave port 0..Error Address Register, slave port 4, array offset: 0x10, array step: 0x8 */
    __I  uint32_t EDR;                               /**< Error Detail Register, slave port 0..Error Detail Register, slave port 4, array offset: 0x14, array step: 0x8 */
  } EAR_EDR[MPU_EAR_EDR_COUNT];
  uint8_t RESERVED_1[968];
  struct {                                         /* offset: 0x400, array step: 0x10 */
    __IO uint32_t WORD0;                             /**< Region Descriptor 0, Word 0..Region Descriptor 15, Word 0, array offset: 0x400, array step: 0x10 */
    __IO uint32_t WORD1;                             /**< Region Descriptor 0, Word 1..Region Descriptor 15, Word 1, array offset: 0x404, array step: 0x10 */
    __IO uint32_t WORD2;                             /**< Region Descriptor 0, Word 2..Region Descriptor 15, Word 2, array offset: 0x408, array step: 0x10 */
    __IO uint32_t WORD3;                             /**< Region Descriptor 0, Word 3..Region Descriptor 15, Word 3, array offset: 0x40C, array step: 0x10 */
  } RGD[MPU_RGD_COUNT];
  uint8_t RESERVED_2[768];
  __IO uint32_t RGDAAC[MPU_RGDAAC_COUNT];          /**< Region Descriptor Alternate Access Control 0..Region Descriptor Alternate Access Control 15, array offset: 0x800, array step: 0x4 */
} MPU_Type, *MPU_MemMapPtr;

/** Number of instances of the MPU module. */
#define MPU_INSTANCE_COUNT                       (1u)

/* MPU - Peripheral instance base addresses */
/** Peripheral MPU base address */
#define IP_MPU_BASE                              (0x4000D000u)
/** Peripheral MPU base pointer */
#define IP_MPU                                   ((MPU_Type *)IP_MPU_BASE)
/** Array initializer of MPU peripheral base addresses */
#define IP_MPU_BASE_ADDRS                        { IP_MPU_BASE }
/** Array initializer of MPU peripheral base pointers */
#define IP_MPU_BASE_PTRS                         { IP_MPU }

/* ----------------------------------------------------------------------------
   -- MPU Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MPU_Register_Masks MPU Register Masks
 * @{
 */

/*! @name CESR - Control/Error Status Register */
/*! @{ */

#define MPU_CESR_VLD_MASK                        (0x1U)
#define MPU_CESR_VLD_SHIFT                       (0U)
#define MPU_CESR_VLD_WIDTH                       (1U)
#define MPU_CESR_VLD(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_CESR_VLD_SHIFT)) & MPU_CESR_VLD_MASK)

#define MPU_CESR_NRGD_MASK                       (0xF00U)
#define MPU_CESR_NRGD_SHIFT                      (8U)
#define MPU_CESR_NRGD_WIDTH                      (4U)
#define MPU_CESR_NRGD(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_CESR_NRGD_SHIFT)) & MPU_CESR_NRGD_MASK)

#define MPU_CESR_NSP_MASK                        (0xF000U)
#define MPU_CESR_NSP_SHIFT                       (12U)
#define MPU_CESR_NSP_WIDTH                       (4U)
#define MPU_CESR_NSP(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_CESR_NSP_SHIFT)) & MPU_CESR_NSP_MASK)

#define MPU_CESR_HRL_MASK                        (0xF0000U)
#define MPU_CESR_HRL_SHIFT                       (16U)
#define MPU_CESR_HRL_WIDTH                       (4U)
#define MPU_CESR_HRL(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_CESR_HRL_SHIFT)) & MPU_CESR_HRL_MASK)

#define MPU_CESR_SPERR4_MASK                     (0x8000000U)
#define MPU_CESR_SPERR4_SHIFT                    (27U)
#define MPU_CESR_SPERR4_WIDTH                    (1U)
#define MPU_CESR_SPERR4(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR4_SHIFT)) & MPU_CESR_SPERR4_MASK)

#define MPU_CESR_SPERR3_MASK                     (0x10000000U)
#define MPU_CESR_SPERR3_SHIFT                    (28U)
#define MPU_CESR_SPERR3_WIDTH                    (1U)
#define MPU_CESR_SPERR3(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR3_SHIFT)) & MPU_CESR_SPERR3_MASK)

#define MPU_CESR_SPERR2_MASK                     (0x20000000U)
#define MPU_CESR_SPERR2_SHIFT                    (29U)
#define MPU_CESR_SPERR2_WIDTH                    (1U)
#define MPU_CESR_SPERR2(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR2_SHIFT)) & MPU_CESR_SPERR2_MASK)

#define MPU_CESR_SPERR1_MASK                     (0x40000000U)
#define MPU_CESR_SPERR1_SHIFT                    (30U)
#define MPU_CESR_SPERR1_WIDTH                    (1U)
#define MPU_CESR_SPERR1(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR1_SHIFT)) & MPU_CESR_SPERR1_MASK)

#define MPU_CESR_SPERR0_MASK                     (0x80000000U)
#define MPU_CESR_SPERR0_SHIFT                    (31U)
#define MPU_CESR_SPERR0_WIDTH                    (1U)
#define MPU_CESR_SPERR0(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_CESR_SPERR0_SHIFT)) & MPU_CESR_SPERR0_MASK)
/*! @} */

/*! @name EAR - Error Address Register, slave port 0..Error Address Register, slave port 4 */
/*! @{ */

#define MPU_EAR_EADDR_MASK                       (0xFFFFFFFFU)
#define MPU_EAR_EADDR_SHIFT                      (0U)
#define MPU_EAR_EADDR_WIDTH                      (32U)
#define MPU_EAR_EADDR(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_EAR_EADDR_SHIFT)) & MPU_EAR_EADDR_MASK)
/*! @} */

/*! @name EDR - Error Detail Register, slave port 0..Error Detail Register, slave port 4 */
/*! @{ */

#define MPU_EDR_ERW_MASK                         (0x1U)
#define MPU_EDR_ERW_SHIFT                        (0U)
#define MPU_EDR_ERW_WIDTH                        (1U)
#define MPU_EDR_ERW(x)                           (((uint32_t)(((uint32_t)(x)) << MPU_EDR_ERW_SHIFT)) & MPU_EDR_ERW_MASK)

#define MPU_EDR_EATTR_MASK                       (0xEU)
#define MPU_EDR_EATTR_SHIFT                      (1U)
#define MPU_EDR_EATTR_WIDTH                      (3U)
#define MPU_EDR_EATTR(x)                         (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EATTR_SHIFT)) & MPU_EDR_EATTR_MASK)

#define MPU_EDR_EMN_MASK                         (0xF0U)
#define MPU_EDR_EMN_SHIFT                        (4U)
#define MPU_EDR_EMN_WIDTH                        (4U)
#define MPU_EDR_EMN(x)                           (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EMN_SHIFT)) & MPU_EDR_EMN_MASK)

#define MPU_EDR_EPID_MASK                        (0xFF00U)
#define MPU_EDR_EPID_SHIFT                       (8U)
#define MPU_EDR_EPID_WIDTH                       (8U)
#define MPU_EDR_EPID(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EPID_SHIFT)) & MPU_EDR_EPID_MASK)

#define MPU_EDR_EACD_MASK                        (0xFFFF0000U)
#define MPU_EDR_EACD_SHIFT                       (16U)
#define MPU_EDR_EACD_WIDTH                       (16U)
#define MPU_EDR_EACD(x)                          (((uint32_t)(((uint32_t)(x)) << MPU_EDR_EACD_SHIFT)) & MPU_EDR_EACD_MASK)
/*! @} */

/*! @name RGD_WORD0 - Region Descriptor 0, Word 0..Region Descriptor 15, Word 0 */
/*! @{ */

#define MPU_RGD_WORD0_SRTADDR_MASK               (0xFFFFFFE0U)
#define MPU_RGD_WORD0_SRTADDR_SHIFT              (5U)
#define MPU_RGD_WORD0_SRTADDR_WIDTH              (27U)
#define MPU_RGD_WORD0_SRTADDR(x)                 (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD0_SRTADDR_SHIFT)) & MPU_RGD_WORD0_SRTADDR_MASK)
/*! @} */

/*! @name RGD_WORD1 - Region Descriptor 0, Word 1..Region Descriptor 15, Word 1 */
/*! @{ */

#define MPU_RGD_WORD1_ENDADDR_MASK               (0xFFFFFFE0U)
#define MPU_RGD_WORD1_ENDADDR_SHIFT              (5U)
#define MPU_RGD_WORD1_ENDADDR_WIDTH              (27U)
#define MPU_RGD_WORD1_ENDADDR(x)                 (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD1_ENDADDR_SHIFT)) & MPU_RGD_WORD1_ENDADDR_MASK)
/*! @} */

/*! @name RGD_WORD2 - Region Descriptor 0, Word 2..Region Descriptor 15, Word 2 */
/*! @{ */

#define MPU_RGD_WORD2_M0UM_MASK                  (0x7U)
#define MPU_RGD_WORD2_M0UM_SHIFT                 (0U)
#define MPU_RGD_WORD2_M0UM_WIDTH                 (3U)
#define MPU_RGD_WORD2_M0UM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M0UM_SHIFT)) & MPU_RGD_WORD2_M0UM_MASK)

#define MPU_RGD_WORD2_M0SM_MASK                  (0x18U)
#define MPU_RGD_WORD2_M0SM_SHIFT                 (3U)
#define MPU_RGD_WORD2_M0SM_WIDTH                 (2U)
#define MPU_RGD_WORD2_M0SM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M0SM_SHIFT)) & MPU_RGD_WORD2_M0SM_MASK)

#define MPU_RGD_WORD2_M0PE_MASK                  (0x20U)
#define MPU_RGD_WORD2_M0PE_SHIFT                 (5U)
#define MPU_RGD_WORD2_M0PE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M0PE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M0PE_SHIFT)) & MPU_RGD_WORD2_M0PE_MASK)

#define MPU_RGD_WORD2_M1UM_MASK                  (0x1C0U)
#define MPU_RGD_WORD2_M1UM_SHIFT                 (6U)
#define MPU_RGD_WORD2_M1UM_WIDTH                 (3U)
#define MPU_RGD_WORD2_M1UM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M1UM_SHIFT)) & MPU_RGD_WORD2_M1UM_MASK)

#define MPU_RGD_WORD2_M1SM_MASK                  (0x600U)
#define MPU_RGD_WORD2_M1SM_SHIFT                 (9U)
#define MPU_RGD_WORD2_M1SM_WIDTH                 (2U)
#define MPU_RGD_WORD2_M1SM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M1SM_SHIFT)) & MPU_RGD_WORD2_M1SM_MASK)

#define MPU_RGD_WORD2_M1PE_MASK                  (0x800U)
#define MPU_RGD_WORD2_M1PE_SHIFT                 (11U)
#define MPU_RGD_WORD2_M1PE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M1PE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M1PE_SHIFT)) & MPU_RGD_WORD2_M1PE_MASK)

#define MPU_RGD_WORD2_M2UM_MASK                  (0x7000U)
#define MPU_RGD_WORD2_M2UM_SHIFT                 (12U)
#define MPU_RGD_WORD2_M2UM_WIDTH                 (3U)
#define MPU_RGD_WORD2_M2UM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M2UM_SHIFT)) & MPU_RGD_WORD2_M2UM_MASK)

#define MPU_RGD_WORD2_M2SM_MASK                  (0x18000U)
#define MPU_RGD_WORD2_M2SM_SHIFT                 (15U)
#define MPU_RGD_WORD2_M2SM_WIDTH                 (2U)
#define MPU_RGD_WORD2_M2SM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M2SM_SHIFT)) & MPU_RGD_WORD2_M2SM_MASK)

#define MPU_RGD_WORD2_M3UM_MASK                  (0x1C0000U)
#define MPU_RGD_WORD2_M3UM_SHIFT                 (18U)
#define MPU_RGD_WORD2_M3UM_WIDTH                 (3U)
#define MPU_RGD_WORD2_M3UM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M3UM_SHIFT)) & MPU_RGD_WORD2_M3UM_MASK)

#define MPU_RGD_WORD2_M3SM_MASK                  (0x600000U)
#define MPU_RGD_WORD2_M3SM_SHIFT                 (21U)
#define MPU_RGD_WORD2_M3SM_WIDTH                 (2U)
#define MPU_RGD_WORD2_M3SM(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M3SM_SHIFT)) & MPU_RGD_WORD2_M3SM_MASK)

#define MPU_RGD_WORD2_M4WE_MASK                  (0x1000000U)
#define MPU_RGD_WORD2_M4WE_SHIFT                 (24U)
#define MPU_RGD_WORD2_M4WE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M4WE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M4WE_SHIFT)) & MPU_RGD_WORD2_M4WE_MASK)

#define MPU_RGD_WORD2_M4RE_MASK                  (0x2000000U)
#define MPU_RGD_WORD2_M4RE_SHIFT                 (25U)
#define MPU_RGD_WORD2_M4RE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M4RE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M4RE_SHIFT)) & MPU_RGD_WORD2_M4RE_MASK)

#define MPU_RGD_WORD2_M5WE_MASK                  (0x4000000U)
#define MPU_RGD_WORD2_M5WE_SHIFT                 (26U)
#define MPU_RGD_WORD2_M5WE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M5WE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M5WE_SHIFT)) & MPU_RGD_WORD2_M5WE_MASK)

#define MPU_RGD_WORD2_M5RE_MASK                  (0x8000000U)
#define MPU_RGD_WORD2_M5RE_SHIFT                 (27U)
#define MPU_RGD_WORD2_M5RE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M5RE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M5RE_SHIFT)) & MPU_RGD_WORD2_M5RE_MASK)

#define MPU_RGD_WORD2_M6WE_MASK                  (0x10000000U)
#define MPU_RGD_WORD2_M6WE_SHIFT                 (28U)
#define MPU_RGD_WORD2_M6WE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M6WE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M6WE_SHIFT)) & MPU_RGD_WORD2_M6WE_MASK)

#define MPU_RGD_WORD2_M6RE_MASK                  (0x20000000U)
#define MPU_RGD_WORD2_M6RE_SHIFT                 (29U)
#define MPU_RGD_WORD2_M6RE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M6RE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M6RE_SHIFT)) & MPU_RGD_WORD2_M6RE_MASK)

#define MPU_RGD_WORD2_M7WE_MASK                  (0x40000000U)
#define MPU_RGD_WORD2_M7WE_SHIFT                 (30U)
#define MPU_RGD_WORD2_M7WE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M7WE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M7WE_SHIFT)) & MPU_RGD_WORD2_M7WE_MASK)

#define MPU_RGD_WORD2_M7RE_MASK                  (0x80000000U)
#define MPU_RGD_WORD2_M7RE_SHIFT                 (31U)
#define MPU_RGD_WORD2_M7RE_WIDTH                 (1U)
#define MPU_RGD_WORD2_M7RE(x)                    (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD2_M7RE_SHIFT)) & MPU_RGD_WORD2_M7RE_MASK)
/*! @} */

/*! @name RGD_WORD3 - Region Descriptor 0, Word 3..Region Descriptor 15, Word 3 */
/*! @{ */

#define MPU_RGD_WORD3_VLD_MASK                   (0x1U)
#define MPU_RGD_WORD3_VLD_SHIFT                  (0U)
#define MPU_RGD_WORD3_VLD_WIDTH                  (1U)
#define MPU_RGD_WORD3_VLD(x)                     (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD3_VLD_SHIFT)) & MPU_RGD_WORD3_VLD_MASK)

#define MPU_RGD_WORD3_PIDMASK_MASK               (0xFF0000U)
#define MPU_RGD_WORD3_PIDMASK_SHIFT              (16U)
#define MPU_RGD_WORD3_PIDMASK_WIDTH              (8U)
#define MPU_RGD_WORD3_PIDMASK(x)                 (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD3_PIDMASK_SHIFT)) & MPU_RGD_WORD3_PIDMASK_MASK)

#define MPU_RGD_WORD3_PID_MASK                   (0xFF000000U)
#define MPU_RGD_WORD3_PID_SHIFT                  (24U)
#define MPU_RGD_WORD3_PID_WIDTH                  (8U)
#define MPU_RGD_WORD3_PID(x)                     (((uint32_t)(((uint32_t)(x)) << MPU_RGD_WORD3_PID_SHIFT)) & MPU_RGD_WORD3_PID_MASK)
/*! @} */

/*! @name RGDAAC - Region Descriptor Alternate Access Control 0..Region Descriptor Alternate Access Control 15 */
/*! @{ */

#define MPU_RGDAAC_M0UM_MASK                     (0x7U)
#define MPU_RGDAAC_M0UM_SHIFT                    (0U)
#define MPU_RGDAAC_M0UM_WIDTH                    (3U)
#define MPU_RGDAAC_M0UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M0UM_SHIFT)) & MPU_RGDAAC_M0UM_MASK)

#define MPU_RGDAAC_M0SM_MASK                     (0x18U)
#define MPU_RGDAAC_M0SM_SHIFT                    (3U)
#define MPU_RGDAAC_M0SM_WIDTH                    (2U)
#define MPU_RGDAAC_M0SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M0SM_SHIFT)) & MPU_RGDAAC_M0SM_MASK)

#define MPU_RGDAAC_M0PE_MASK                     (0x20U)
#define MPU_RGDAAC_M0PE_SHIFT                    (5U)
#define MPU_RGDAAC_M0PE_WIDTH                    (1U)
#define MPU_RGDAAC_M0PE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M0PE_SHIFT)) & MPU_RGDAAC_M0PE_MASK)

#define MPU_RGDAAC_M1UM_MASK                     (0x1C0U)
#define MPU_RGDAAC_M1UM_SHIFT                    (6U)
#define MPU_RGDAAC_M1UM_WIDTH                    (3U)
#define MPU_RGDAAC_M1UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M1UM_SHIFT)) & MPU_RGDAAC_M1UM_MASK)

#define MPU_RGDAAC_M1SM_MASK                     (0x600U)
#define MPU_RGDAAC_M1SM_SHIFT                    (9U)
#define MPU_RGDAAC_M1SM_WIDTH                    (2U)
#define MPU_RGDAAC_M1SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M1SM_SHIFT)) & MPU_RGDAAC_M1SM_MASK)

#define MPU_RGDAAC_M1PE_MASK                     (0x800U)
#define MPU_RGDAAC_M1PE_SHIFT                    (11U)
#define MPU_RGDAAC_M1PE_WIDTH                    (1U)
#define MPU_RGDAAC_M1PE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M1PE_SHIFT)) & MPU_RGDAAC_M1PE_MASK)

#define MPU_RGDAAC_M2UM_MASK                     (0x7000U)
#define MPU_RGDAAC_M2UM_SHIFT                    (12U)
#define MPU_RGDAAC_M2UM_WIDTH                    (3U)
#define MPU_RGDAAC_M2UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M2UM_SHIFT)) & MPU_RGDAAC_M2UM_MASK)

#define MPU_RGDAAC_M2SM_MASK                     (0x18000U)
#define MPU_RGDAAC_M2SM_SHIFT                    (15U)
#define MPU_RGDAAC_M2SM_WIDTH                    (2U)
#define MPU_RGDAAC_M2SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M2SM_SHIFT)) & MPU_RGDAAC_M2SM_MASK)

#define MPU_RGDAAC_M3UM_MASK                     (0x1C0000U)
#define MPU_RGDAAC_M3UM_SHIFT                    (18U)
#define MPU_RGDAAC_M3UM_WIDTH                    (3U)
#define MPU_RGDAAC_M3UM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M3UM_SHIFT)) & MPU_RGDAAC_M3UM_MASK)

#define MPU_RGDAAC_M3SM_MASK                     (0x600000U)
#define MPU_RGDAAC_M3SM_SHIFT                    (21U)
#define MPU_RGDAAC_M3SM_WIDTH                    (2U)
#define MPU_RGDAAC_M3SM(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M3SM_SHIFT)) & MPU_RGDAAC_M3SM_MASK)

#define MPU_RGDAAC_M4WE_MASK                     (0x1000000U)
#define MPU_RGDAAC_M4WE_SHIFT                    (24U)
#define MPU_RGDAAC_M4WE_WIDTH                    (1U)
#define MPU_RGDAAC_M4WE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M4WE_SHIFT)) & MPU_RGDAAC_M4WE_MASK)

#define MPU_RGDAAC_M4RE_MASK                     (0x2000000U)
#define MPU_RGDAAC_M4RE_SHIFT                    (25U)
#define MPU_RGDAAC_M4RE_WIDTH                    (1U)
#define MPU_RGDAAC_M4RE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M4RE_SHIFT)) & MPU_RGDAAC_M4RE_MASK)

#define MPU_RGDAAC_M5WE_MASK                     (0x4000000U)
#define MPU_RGDAAC_M5WE_SHIFT                    (26U)
#define MPU_RGDAAC_M5WE_WIDTH                    (1U)
#define MPU_RGDAAC_M5WE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M5WE_SHIFT)) & MPU_RGDAAC_M5WE_MASK)

#define MPU_RGDAAC_M5RE_MASK                     (0x8000000U)
#define MPU_RGDAAC_M5RE_SHIFT                    (27U)
#define MPU_RGDAAC_M5RE_WIDTH                    (1U)
#define MPU_RGDAAC_M5RE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M5RE_SHIFT)) & MPU_RGDAAC_M5RE_MASK)

#define MPU_RGDAAC_M6WE_MASK                     (0x10000000U)
#define MPU_RGDAAC_M6WE_SHIFT                    (28U)
#define MPU_RGDAAC_M6WE_WIDTH                    (1U)
#define MPU_RGDAAC_M6WE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M6WE_SHIFT)) & MPU_RGDAAC_M6WE_MASK)

#define MPU_RGDAAC_M6RE_MASK                     (0x20000000U)
#define MPU_RGDAAC_M6RE_SHIFT                    (29U)
#define MPU_RGDAAC_M6RE_WIDTH                    (1U)
#define MPU_RGDAAC_M6RE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M6RE_SHIFT)) & MPU_RGDAAC_M6RE_MASK)

#define MPU_RGDAAC_M7WE_MASK                     (0x40000000U)
#define MPU_RGDAAC_M7WE_SHIFT                    (30U)
#define MPU_RGDAAC_M7WE_WIDTH                    (1U)
#define MPU_RGDAAC_M7WE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M7WE_SHIFT)) & MPU_RGDAAC_M7WE_MASK)

#define MPU_RGDAAC_M7RE_MASK                     (0x80000000U)
#define MPU_RGDAAC_M7RE_SHIFT                    (31U)
#define MPU_RGDAAC_M7RE_WIDTH                    (1U)
#define MPU_RGDAAC_M7RE(x)                       (((uint32_t)(((uint32_t)(x)) << MPU_RGDAAC_M7RE_SHIFT)) & MPU_RGDAAC_M7RE_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group MPU_Register_Masks */

/*!
 * @}
 */ /* end of group MPU_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_MPU_H_) */

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
 * @file S32K148_RCM.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_RCM
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
#if !defined(S32K148_RCM_H_)  /* Check if memory map has not been already included */
#define S32K148_RCM_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- RCM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RCM_Peripheral_Access_Layer RCM Peripheral Access Layer
 * @{
 */

/** RCM - Register Layout Typedef */
typedef struct {
  __I  uint32_t VERID;                             /**< Version ID Register, offset: 0x0 */
  __I  uint32_t PARAM;                             /**< Parameter Register, offset: 0x4 */
  __I  uint32_t SRS;                               /**< System Reset Status Register, offset: 0x8 */
  __IO uint32_t RPC;                               /**< Reset Pin Control register, offset: 0xC */
  uint8_t RESERVED_0[8];
  __IO uint32_t SSRS;                              /**< Sticky System Reset Status Register, offset: 0x18 */
  __IO uint32_t SRIE;                              /**< System Reset Interrupt Enable Register, offset: 0x1C */
} RCM_Type, *RCM_MemMapPtr;

/** Number of instances of the RCM module. */
#define RCM_INSTANCE_COUNT                       (1u)

/* RCM - Peripheral instance base addresses */
/** Peripheral RCM base address */
#define IP_RCM_BASE                              (0x4007F000u)
/** Peripheral RCM base pointer */
#define IP_RCM                                   ((RCM_Type *)IP_RCM_BASE)
/** Array initializer of RCM peripheral base addresses */
#define IP_RCM_BASE_ADDRS                        { IP_RCM_BASE }
/** Array initializer of RCM peripheral base pointers */
#define IP_RCM_BASE_PTRS                         { IP_RCM }

/* ----------------------------------------------------------------------------
   -- RCM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup RCM_Register_Masks RCM Register Masks
 * @{
 */

/*! @name VERID - Version ID Register */
/*! @{ */

#define RCM_VERID_FEATURE_MASK                   (0xFFFFU)
#define RCM_VERID_FEATURE_SHIFT                  (0U)
#define RCM_VERID_FEATURE_WIDTH                  (16U)
#define RCM_VERID_FEATURE(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_VERID_FEATURE_SHIFT)) & RCM_VERID_FEATURE_MASK)

#define RCM_VERID_MINOR_MASK                     (0xFF0000U)
#define RCM_VERID_MINOR_SHIFT                    (16U)
#define RCM_VERID_MINOR_WIDTH                    (8U)
#define RCM_VERID_MINOR(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_VERID_MINOR_SHIFT)) & RCM_VERID_MINOR_MASK)

#define RCM_VERID_MAJOR_MASK                     (0xFF000000U)
#define RCM_VERID_MAJOR_SHIFT                    (24U)
#define RCM_VERID_MAJOR_WIDTH                    (8U)
#define RCM_VERID_MAJOR(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_VERID_MAJOR_SHIFT)) & RCM_VERID_MAJOR_MASK)
/*! @} */

/*! @name PARAM - Parameter Register */
/*! @{ */

#define RCM_PARAM_EWAKEUP_MASK                   (0x1U)
#define RCM_PARAM_EWAKEUP_SHIFT                  (0U)
#define RCM_PARAM_EWAKEUP_WIDTH                  (1U)
#define RCM_PARAM_EWAKEUP(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EWAKEUP_SHIFT)) & RCM_PARAM_EWAKEUP_MASK)

#define RCM_PARAM_ELVD_MASK                      (0x2U)
#define RCM_PARAM_ELVD_SHIFT                     (1U)
#define RCM_PARAM_ELVD_WIDTH                     (1U)
#define RCM_PARAM_ELVD(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ELVD_SHIFT)) & RCM_PARAM_ELVD_MASK)

#define RCM_PARAM_ELOC_MASK                      (0x4U)
#define RCM_PARAM_ELOC_SHIFT                     (2U)
#define RCM_PARAM_ELOC_WIDTH                     (1U)
#define RCM_PARAM_ELOC(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ELOC_SHIFT)) & RCM_PARAM_ELOC_MASK)

#define RCM_PARAM_ELOL_MASK                      (0x8U)
#define RCM_PARAM_ELOL_SHIFT                     (3U)
#define RCM_PARAM_ELOL_WIDTH                     (1U)
#define RCM_PARAM_ELOL(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ELOL_SHIFT)) & RCM_PARAM_ELOL_MASK)

#define RCM_PARAM_ECMU_LOC_MASK                  (0x10U)
#define RCM_PARAM_ECMU_LOC_SHIFT                 (4U)
#define RCM_PARAM_ECMU_LOC_WIDTH                 (1U)
#define RCM_PARAM_ECMU_LOC(x)                    (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ECMU_LOC_SHIFT)) & RCM_PARAM_ECMU_LOC_MASK)

#define RCM_PARAM_EWDOG_MASK                     (0x20U)
#define RCM_PARAM_EWDOG_SHIFT                    (5U)
#define RCM_PARAM_EWDOG_WIDTH                    (1U)
#define RCM_PARAM_EWDOG(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EWDOG_SHIFT)) & RCM_PARAM_EWDOG_MASK)

#define RCM_PARAM_EPIN_MASK                      (0x40U)
#define RCM_PARAM_EPIN_SHIFT                     (6U)
#define RCM_PARAM_EPIN_WIDTH                     (1U)
#define RCM_PARAM_EPIN(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EPIN_SHIFT)) & RCM_PARAM_EPIN_MASK)

#define RCM_PARAM_EPOR_MASK                      (0x80U)
#define RCM_PARAM_EPOR_SHIFT                     (7U)
#define RCM_PARAM_EPOR_WIDTH                     (1U)
#define RCM_PARAM_EPOR(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EPOR_SHIFT)) & RCM_PARAM_EPOR_MASK)

#define RCM_PARAM_EJTAG_MASK                     (0x100U)
#define RCM_PARAM_EJTAG_SHIFT                    (8U)
#define RCM_PARAM_EJTAG_WIDTH                    (1U)
#define RCM_PARAM_EJTAG(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EJTAG_SHIFT)) & RCM_PARAM_EJTAG_MASK)

#define RCM_PARAM_ELOCKUP_MASK                   (0x200U)
#define RCM_PARAM_ELOCKUP_SHIFT                  (9U)
#define RCM_PARAM_ELOCKUP_WIDTH                  (1U)
#define RCM_PARAM_ELOCKUP(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ELOCKUP_SHIFT)) & RCM_PARAM_ELOCKUP_MASK)

#define RCM_PARAM_ESW_MASK                       (0x400U)
#define RCM_PARAM_ESW_SHIFT                      (10U)
#define RCM_PARAM_ESW_WIDTH                      (1U)
#define RCM_PARAM_ESW(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ESW_SHIFT)) & RCM_PARAM_ESW_MASK)

#define RCM_PARAM_EMDM_AP_MASK                   (0x800U)
#define RCM_PARAM_EMDM_AP_SHIFT                  (11U)
#define RCM_PARAM_EMDM_AP_WIDTH                  (1U)
#define RCM_PARAM_EMDM_AP(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_EMDM_AP_SHIFT)) & RCM_PARAM_EMDM_AP_MASK)

#define RCM_PARAM_ESACKERR_MASK                  (0x2000U)
#define RCM_PARAM_ESACKERR_SHIFT                 (13U)
#define RCM_PARAM_ESACKERR_WIDTH                 (1U)
#define RCM_PARAM_ESACKERR(x)                    (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ESACKERR_SHIFT)) & RCM_PARAM_ESACKERR_MASK)

#define RCM_PARAM_ETAMPER_MASK                   (0x8000U)
#define RCM_PARAM_ETAMPER_SHIFT                  (15U)
#define RCM_PARAM_ETAMPER_WIDTH                  (1U)
#define RCM_PARAM_ETAMPER(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ETAMPER_SHIFT)) & RCM_PARAM_ETAMPER_MASK)

#define RCM_PARAM_ECORE1_MASK                    (0x10000U)
#define RCM_PARAM_ECORE1_SHIFT                   (16U)
#define RCM_PARAM_ECORE1_WIDTH                   (1U)
#define RCM_PARAM_ECORE1(x)                      (((uint32_t)(((uint32_t)(x)) << RCM_PARAM_ECORE1_SHIFT)) & RCM_PARAM_ECORE1_MASK)
/*! @} */

/*! @name SRS - System Reset Status Register */
/*! @{ */

#define RCM_SRS_LVD_MASK                         (0x2U)
#define RCM_SRS_LVD_SHIFT                        (1U)
#define RCM_SRS_LVD_WIDTH                        (1U)
#define RCM_SRS_LVD(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRS_LVD_SHIFT)) & RCM_SRS_LVD_MASK)

#define RCM_SRS_LOC_MASK                         (0x4U)
#define RCM_SRS_LOC_SHIFT                        (2U)
#define RCM_SRS_LOC_WIDTH                        (1U)
#define RCM_SRS_LOC(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRS_LOC_SHIFT)) & RCM_SRS_LOC_MASK)

#define RCM_SRS_LOL_MASK                         (0x8U)
#define RCM_SRS_LOL_SHIFT                        (3U)
#define RCM_SRS_LOL_WIDTH                        (1U)
#define RCM_SRS_LOL(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRS_LOL_SHIFT)) & RCM_SRS_LOL_MASK)

#define RCM_SRS_WDOG_MASK                        (0x20U)
#define RCM_SRS_WDOG_SHIFT                       (5U)
#define RCM_SRS_WDOG_WIDTH                       (1U)
#define RCM_SRS_WDOG(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRS_WDOG_SHIFT)) & RCM_SRS_WDOG_MASK)

#define RCM_SRS_PIN_MASK                         (0x40U)
#define RCM_SRS_PIN_SHIFT                        (6U)
#define RCM_SRS_PIN_WIDTH                        (1U)
#define RCM_SRS_PIN(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRS_PIN_SHIFT)) & RCM_SRS_PIN_MASK)

#define RCM_SRS_POR_MASK                         (0x80U)
#define RCM_SRS_POR_SHIFT                        (7U)
#define RCM_SRS_POR_WIDTH                        (1U)
#define RCM_SRS_POR(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRS_POR_SHIFT)) & RCM_SRS_POR_MASK)

#define RCM_SRS_JTAG_MASK                        (0x100U)
#define RCM_SRS_JTAG_SHIFT                       (8U)
#define RCM_SRS_JTAG_WIDTH                       (1U)
#define RCM_SRS_JTAG(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRS_JTAG_SHIFT)) & RCM_SRS_JTAG_MASK)

#define RCM_SRS_LOCKUP_MASK                      (0x200U)
#define RCM_SRS_LOCKUP_SHIFT                     (9U)
#define RCM_SRS_LOCKUP_WIDTH                     (1U)
#define RCM_SRS_LOCKUP(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_SRS_LOCKUP_SHIFT)) & RCM_SRS_LOCKUP_MASK)

#define RCM_SRS_SW_MASK                          (0x400U)
#define RCM_SRS_SW_SHIFT                         (10U)
#define RCM_SRS_SW_WIDTH                         (1U)
#define RCM_SRS_SW(x)                            (((uint32_t)(((uint32_t)(x)) << RCM_SRS_SW_SHIFT)) & RCM_SRS_SW_MASK)

#define RCM_SRS_MDM_AP_MASK                      (0x800U)
#define RCM_SRS_MDM_AP_SHIFT                     (11U)
#define RCM_SRS_MDM_AP_WIDTH                     (1U)
#define RCM_SRS_MDM_AP(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_SRS_MDM_AP_SHIFT)) & RCM_SRS_MDM_AP_MASK)

#define RCM_SRS_SACKERR_MASK                     (0x2000U)
#define RCM_SRS_SACKERR_SHIFT                    (13U)
#define RCM_SRS_SACKERR_WIDTH                    (1U)
#define RCM_SRS_SACKERR(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_SRS_SACKERR_SHIFT)) & RCM_SRS_SACKERR_MASK)
/*! @} */

/*! @name RPC - Reset Pin Control register */
/*! @{ */

#define RCM_RPC_RSTFLTSRW_MASK                   (0x3U)
#define RCM_RPC_RSTFLTSRW_SHIFT                  (0U)
#define RCM_RPC_RSTFLTSRW_WIDTH                  (2U)
#define RCM_RPC_RSTFLTSRW(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_RPC_RSTFLTSRW_SHIFT)) & RCM_RPC_RSTFLTSRW_MASK)

#define RCM_RPC_RSTFLTSS_MASK                    (0x4U)
#define RCM_RPC_RSTFLTSS_SHIFT                   (2U)
#define RCM_RPC_RSTFLTSS_WIDTH                   (1U)
#define RCM_RPC_RSTFLTSS(x)                      (((uint32_t)(((uint32_t)(x)) << RCM_RPC_RSTFLTSS_SHIFT)) & RCM_RPC_RSTFLTSS_MASK)

#define RCM_RPC_RSTFLTSEL_MASK                   (0x1F00U)
#define RCM_RPC_RSTFLTSEL_SHIFT                  (8U)
#define RCM_RPC_RSTFLTSEL_WIDTH                  (5U)
#define RCM_RPC_RSTFLTSEL(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_RPC_RSTFLTSEL_SHIFT)) & RCM_RPC_RSTFLTSEL_MASK)
/*! @} */

/*! @name SSRS - Sticky System Reset Status Register */
/*! @{ */

#define RCM_SSRS_SLVD_MASK                       (0x2U)
#define RCM_SSRS_SLVD_SHIFT                      (1U)
#define RCM_SSRS_SLVD_WIDTH                      (1U)
#define RCM_SSRS_SLVD(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SLVD_SHIFT)) & RCM_SSRS_SLVD_MASK)

#define RCM_SSRS_SLOC_MASK                       (0x4U)
#define RCM_SSRS_SLOC_SHIFT                      (2U)
#define RCM_SSRS_SLOC_WIDTH                      (1U)
#define RCM_SSRS_SLOC(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SLOC_SHIFT)) & RCM_SSRS_SLOC_MASK)

#define RCM_SSRS_SLOL_MASK                       (0x8U)
#define RCM_SSRS_SLOL_SHIFT                      (3U)
#define RCM_SSRS_SLOL_WIDTH                      (1U)
#define RCM_SSRS_SLOL(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SLOL_SHIFT)) & RCM_SSRS_SLOL_MASK)

#define RCM_SSRS_SWDOG_MASK                      (0x20U)
#define RCM_SSRS_SWDOG_SHIFT                     (5U)
#define RCM_SSRS_SWDOG_WIDTH                     (1U)
#define RCM_SSRS_SWDOG(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SWDOG_SHIFT)) & RCM_SSRS_SWDOG_MASK)

#define RCM_SSRS_SPIN_MASK                       (0x40U)
#define RCM_SSRS_SPIN_SHIFT                      (6U)
#define RCM_SSRS_SPIN_WIDTH                      (1U)
#define RCM_SSRS_SPIN(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SPIN_SHIFT)) & RCM_SSRS_SPIN_MASK)

#define RCM_SSRS_SPOR_MASK                       (0x80U)
#define RCM_SSRS_SPOR_SHIFT                      (7U)
#define RCM_SSRS_SPOR_WIDTH                      (1U)
#define RCM_SSRS_SPOR(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SPOR_SHIFT)) & RCM_SSRS_SPOR_MASK)

#define RCM_SSRS_SJTAG_MASK                      (0x100U)
#define RCM_SSRS_SJTAG_SHIFT                     (8U)
#define RCM_SSRS_SJTAG_WIDTH                     (1U)
#define RCM_SSRS_SJTAG(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SJTAG_SHIFT)) & RCM_SSRS_SJTAG_MASK)

#define RCM_SSRS_SLOCKUP_MASK                    (0x200U)
#define RCM_SSRS_SLOCKUP_SHIFT                   (9U)
#define RCM_SSRS_SLOCKUP_WIDTH                   (1U)
#define RCM_SSRS_SLOCKUP(x)                      (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SLOCKUP_SHIFT)) & RCM_SSRS_SLOCKUP_MASK)

#define RCM_SSRS_SSW_MASK                        (0x400U)
#define RCM_SSRS_SSW_SHIFT                       (10U)
#define RCM_SSRS_SSW_WIDTH                       (1U)
#define RCM_SSRS_SSW(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SSW_SHIFT)) & RCM_SSRS_SSW_MASK)

#define RCM_SSRS_SMDM_AP_MASK                    (0x800U)
#define RCM_SSRS_SMDM_AP_SHIFT                   (11U)
#define RCM_SSRS_SMDM_AP_WIDTH                   (1U)
#define RCM_SSRS_SMDM_AP(x)                      (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SMDM_AP_SHIFT)) & RCM_SSRS_SMDM_AP_MASK)

#define RCM_SSRS_SSACKERR_MASK                   (0x2000U)
#define RCM_SSRS_SSACKERR_SHIFT                  (13U)
#define RCM_SSRS_SSACKERR_WIDTH                  (1U)
#define RCM_SSRS_SSACKERR(x)                     (((uint32_t)(((uint32_t)(x)) << RCM_SSRS_SSACKERR_SHIFT)) & RCM_SSRS_SSACKERR_MASK)
/*! @} */

/*! @name SRIE - System Reset Interrupt Enable Register */
/*! @{ */

#define RCM_SRIE_DELAY_MASK                      (0x3U)
#define RCM_SRIE_DELAY_SHIFT                     (0U)
#define RCM_SRIE_DELAY_WIDTH                     (2U)
#define RCM_SRIE_DELAY(x)                        (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_DELAY_SHIFT)) & RCM_SRIE_DELAY_MASK)

#define RCM_SRIE_LOC_MASK                        (0x4U)
#define RCM_SRIE_LOC_SHIFT                       (2U)
#define RCM_SRIE_LOC_WIDTH                       (1U)
#define RCM_SRIE_LOC(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_LOC_SHIFT)) & RCM_SRIE_LOC_MASK)

#define RCM_SRIE_LOL_MASK                        (0x8U)
#define RCM_SRIE_LOL_SHIFT                       (3U)
#define RCM_SRIE_LOL_WIDTH                       (1U)
#define RCM_SRIE_LOL(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_LOL_SHIFT)) & RCM_SRIE_LOL_MASK)

#define RCM_SRIE_WDOG_MASK                       (0x20U)
#define RCM_SRIE_WDOG_SHIFT                      (5U)
#define RCM_SRIE_WDOG_WIDTH                      (1U)
#define RCM_SRIE_WDOG(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_WDOG_SHIFT)) & RCM_SRIE_WDOG_MASK)

#define RCM_SRIE_PIN_MASK                        (0x40U)
#define RCM_SRIE_PIN_SHIFT                       (6U)
#define RCM_SRIE_PIN_WIDTH                       (1U)
#define RCM_SRIE_PIN(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_PIN_SHIFT)) & RCM_SRIE_PIN_MASK)

#define RCM_SRIE_GIE_MASK                        (0x80U)
#define RCM_SRIE_GIE_SHIFT                       (7U)
#define RCM_SRIE_GIE_WIDTH                       (1U)
#define RCM_SRIE_GIE(x)                          (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_GIE_SHIFT)) & RCM_SRIE_GIE_MASK)

#define RCM_SRIE_JTAG_MASK                       (0x100U)
#define RCM_SRIE_JTAG_SHIFT                      (8U)
#define RCM_SRIE_JTAG_WIDTH                      (1U)
#define RCM_SRIE_JTAG(x)                         (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_JTAG_SHIFT)) & RCM_SRIE_JTAG_MASK)

#define RCM_SRIE_LOCKUP_MASK                     (0x200U)
#define RCM_SRIE_LOCKUP_SHIFT                    (9U)
#define RCM_SRIE_LOCKUP_WIDTH                    (1U)
#define RCM_SRIE_LOCKUP(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_LOCKUP_SHIFT)) & RCM_SRIE_LOCKUP_MASK)

#define RCM_SRIE_SW_MASK                         (0x400U)
#define RCM_SRIE_SW_SHIFT                        (10U)
#define RCM_SRIE_SW_WIDTH                        (1U)
#define RCM_SRIE_SW(x)                           (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_SW_SHIFT)) & RCM_SRIE_SW_MASK)

#define RCM_SRIE_MDM_AP_MASK                     (0x800U)
#define RCM_SRIE_MDM_AP_SHIFT                    (11U)
#define RCM_SRIE_MDM_AP_WIDTH                    (1U)
#define RCM_SRIE_MDM_AP(x)                       (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_MDM_AP_SHIFT)) & RCM_SRIE_MDM_AP_MASK)

#define RCM_SRIE_SACKERR_MASK                    (0x2000U)
#define RCM_SRIE_SACKERR_SHIFT                   (13U)
#define RCM_SRIE_SACKERR_WIDTH                   (1U)
#define RCM_SRIE_SACKERR(x)                      (((uint32_t)(((uint32_t)(x)) << RCM_SRIE_SACKERR_SHIFT)) & RCM_SRIE_SACKERR_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group RCM_Register_Masks */

/*!
 * @}
 */ /* end of group RCM_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_RCM_H_) */

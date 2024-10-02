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
 * @file S32K148_MCM.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_MCM
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
#if !defined(S32K148_MCM_H_)  /* Check if memory map has not been already included */
#define S32K148_MCM_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- MCM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCM_Peripheral_Access_Layer MCM Peripheral Access Layer
 * @{
 */

/** MCM - Size of Registers Arrays */
#define MCM_LMDR_COUNT                            2u

/** MCM - Register Layout Typedef */
typedef struct {
  uint8_t RESERVED_0[8];
  __I  uint16_t PLASC;                             /**< Crossbar Switch (AXBS) Slave Configuration, offset: 0x8 */
  __I  uint16_t PLAMC;                             /**< Crossbar Switch (AXBS) Master Configuration, offset: 0xA */
  __IO uint32_t CPCR;                              /**< Core Platform Control Register, offset: 0xC */
  __IO uint32_t ISCR;                              /**< Interrupt Status and Control Register, offset: 0x10 */
  uint8_t RESERVED_1[28];
  __IO uint32_t PID;                               /**< Process ID Register, offset: 0x30 */
  uint8_t RESERVED_2[12];
  __IO uint32_t CPO;                               /**< Compute Operation Control Register, offset: 0x40 */
  uint8_t RESERVED_3[956];
  __IO uint32_t LMDR[MCM_LMDR_COUNT];              /**< Local Memory Descriptor Register, array offset: 0x400, array step: 0x4 */
  __IO uint32_t LMDR2;                             /**< Local Memory Descriptor Register2, offset: 0x408 */
  uint8_t RESERVED_4[116];
  __IO uint32_t LMPECR;                            /**< LMEM Parity and ECC Control Register, offset: 0x480 */
  uint8_t RESERVED_5[4];
  __IO uint32_t LMPEIR;                            /**< LMEM Parity and ECC Interrupt Register, offset: 0x488 */
  uint8_t RESERVED_6[4];
  __I  uint32_t LMFAR;                             /**< LMEM Fault Address Register, offset: 0x490 */
  __I  uint32_t LMFATR;                            /**< LMEM Fault Attribute Register, offset: 0x494 */
  uint8_t RESERVED_7[8];
  __I  uint32_t LMFDHR;                            /**< LMEM Fault Data High Register, offset: 0x4A0 */
  __I  uint32_t LMFDLR;                            /**< LMEM Fault Data Low Register, offset: 0x4A4 */
} MCM_Type, *MCM_MemMapPtr;

/** Number of instances of the MCM module. */
#define MCM_INSTANCE_COUNT                       (1u)

/* MCM - Peripheral instance base addresses */
/** Peripheral MCM base address */
#define IP_MCM_BASE                              (0xE0080000u)
/** Peripheral MCM base pointer */
#define IP_MCM                                   ((MCM_Type *)IP_MCM_BASE)
/** Array initializer of MCM peripheral base addresses */
#define IP_MCM_BASE_ADDRS                        { IP_MCM_BASE }
/** Array initializer of MCM peripheral base pointers */
#define IP_MCM_BASE_PTRS                         { IP_MCM }

/* ----------------------------------------------------------------------------
   -- MCM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCM_Register_Masks MCM Register Masks
 * @{
 */

/*! @name PLASC - Crossbar Switch (AXBS) Slave Configuration */
/*! @{ */

#define MCM_PLASC_ASC_MASK                       (0xFFU)
#define MCM_PLASC_ASC_SHIFT                      (0U)
#define MCM_PLASC_ASC_WIDTH                      (8U)
#define MCM_PLASC_ASC(x)                         (((uint16_t)(((uint16_t)(x)) << MCM_PLASC_ASC_SHIFT)) & MCM_PLASC_ASC_MASK)
/*! @} */

/*! @name PLAMC - Crossbar Switch (AXBS) Master Configuration */
/*! @{ */

#define MCM_PLAMC_AMC_MASK                       (0xFFU)
#define MCM_PLAMC_AMC_SHIFT                      (0U)
#define MCM_PLAMC_AMC_WIDTH                      (8U)
#define MCM_PLAMC_AMC(x)                         (((uint16_t)(((uint16_t)(x)) << MCM_PLAMC_AMC_SHIFT)) & MCM_PLAMC_AMC_MASK)
/*! @} */

/*! @name CPCR - Core Platform Control Register */
/*! @{ */

#define MCM_CPCR_HLT_FSM_ST_MASK                 (0x3U)
#define MCM_CPCR_HLT_FSM_ST_SHIFT                (0U)
#define MCM_CPCR_HLT_FSM_ST_WIDTH                (2U)
#define MCM_CPCR_HLT_FSM_ST(x)                   (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_HLT_FSM_ST_SHIFT)) & MCM_CPCR_HLT_FSM_ST_MASK)

#define MCM_CPCR_AXBS_HLT_REQ_MASK               (0x4U)
#define MCM_CPCR_AXBS_HLT_REQ_SHIFT              (2U)
#define MCM_CPCR_AXBS_HLT_REQ_WIDTH              (1U)
#define MCM_CPCR_AXBS_HLT_REQ(x)                 (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_AXBS_HLT_REQ_SHIFT)) & MCM_CPCR_AXBS_HLT_REQ_MASK)

#define MCM_CPCR_AXBS_HLTD_MASK                  (0x8U)
#define MCM_CPCR_AXBS_HLTD_SHIFT                 (3U)
#define MCM_CPCR_AXBS_HLTD_WIDTH                 (1U)
#define MCM_CPCR_AXBS_HLTD(x)                    (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_AXBS_HLTD_SHIFT)) & MCM_CPCR_AXBS_HLTD_MASK)

#define MCM_CPCR_FMC_PF_IDLE_MASK                (0x10U)
#define MCM_CPCR_FMC_PF_IDLE_SHIFT               (4U)
#define MCM_CPCR_FMC_PF_IDLE_WIDTH               (1U)
#define MCM_CPCR_FMC_PF_IDLE(x)                  (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_FMC_PF_IDLE_SHIFT)) & MCM_CPCR_FMC_PF_IDLE_MASK)

#define MCM_CPCR_PBRIDGE_IDLE_MASK               (0x40U)
#define MCM_CPCR_PBRIDGE_IDLE_SHIFT              (6U)
#define MCM_CPCR_PBRIDGE_IDLE_WIDTH              (1U)
#define MCM_CPCR_PBRIDGE_IDLE(x)                 (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_PBRIDGE_IDLE_SHIFT)) & MCM_CPCR_PBRIDGE_IDLE_MASK)

#define MCM_CPCR_CBRR_MASK                       (0x200U)
#define MCM_CPCR_CBRR_SHIFT                      (9U)
#define MCM_CPCR_CBRR_WIDTH                      (1U)
#define MCM_CPCR_CBRR(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_CBRR_SHIFT)) & MCM_CPCR_CBRR_MASK)

#define MCM_CPCR_SRAMUAP_MASK                    (0x3000000U)
#define MCM_CPCR_SRAMUAP_SHIFT                   (24U)
#define MCM_CPCR_SRAMUAP_WIDTH                   (2U)
#define MCM_CPCR_SRAMUAP(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_SRAMUAP_SHIFT)) & MCM_CPCR_SRAMUAP_MASK)

#define MCM_CPCR_SRAMUWP_MASK                    (0x4000000U)
#define MCM_CPCR_SRAMUWP_SHIFT                   (26U)
#define MCM_CPCR_SRAMUWP_WIDTH                   (1U)
#define MCM_CPCR_SRAMUWP(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_SRAMUWP_SHIFT)) & MCM_CPCR_SRAMUWP_MASK)

#define MCM_CPCR_SRAMLAP_MASK                    (0x30000000U)
#define MCM_CPCR_SRAMLAP_SHIFT                   (28U)
#define MCM_CPCR_SRAMLAP_WIDTH                   (2U)
#define MCM_CPCR_SRAMLAP(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_SRAMLAP_SHIFT)) & MCM_CPCR_SRAMLAP_MASK)

#define MCM_CPCR_SRAMLWP_MASK                    (0x40000000U)
#define MCM_CPCR_SRAMLWP_SHIFT                   (30U)
#define MCM_CPCR_SRAMLWP_WIDTH                   (1U)
#define MCM_CPCR_SRAMLWP(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_CPCR_SRAMLWP_SHIFT)) & MCM_CPCR_SRAMLWP_MASK)
/*! @} */

/*! @name ISCR - Interrupt Status and Control Register */
/*! @{ */

#define MCM_ISCR_FIOC_MASK                       (0x100U)
#define MCM_ISCR_FIOC_SHIFT                      (8U)
#define MCM_ISCR_FIOC_WIDTH                      (1U)
#define MCM_ISCR_FIOC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIOC_SHIFT)) & MCM_ISCR_FIOC_MASK)

#define MCM_ISCR_FDZC_MASK                       (0x200U)
#define MCM_ISCR_FDZC_SHIFT                      (9U)
#define MCM_ISCR_FDZC_WIDTH                      (1U)
#define MCM_ISCR_FDZC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FDZC_SHIFT)) & MCM_ISCR_FDZC_MASK)

#define MCM_ISCR_FOFC_MASK                       (0x400U)
#define MCM_ISCR_FOFC_SHIFT                      (10U)
#define MCM_ISCR_FOFC_WIDTH                      (1U)
#define MCM_ISCR_FOFC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FOFC_SHIFT)) & MCM_ISCR_FOFC_MASK)

#define MCM_ISCR_FUFC_MASK                       (0x800U)
#define MCM_ISCR_FUFC_SHIFT                      (11U)
#define MCM_ISCR_FUFC_WIDTH                      (1U)
#define MCM_ISCR_FUFC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FUFC_SHIFT)) & MCM_ISCR_FUFC_MASK)

#define MCM_ISCR_FIXC_MASK                       (0x1000U)
#define MCM_ISCR_FIXC_SHIFT                      (12U)
#define MCM_ISCR_FIXC_WIDTH                      (1U)
#define MCM_ISCR_FIXC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIXC_SHIFT)) & MCM_ISCR_FIXC_MASK)

#define MCM_ISCR_FIDC_MASK                       (0x8000U)
#define MCM_ISCR_FIDC_SHIFT                      (15U)
#define MCM_ISCR_FIDC_WIDTH                      (1U)
#define MCM_ISCR_FIDC(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIDC_SHIFT)) & MCM_ISCR_FIDC_MASK)

#define MCM_ISCR_FIOCE_MASK                      (0x1000000U)
#define MCM_ISCR_FIOCE_SHIFT                     (24U)
#define MCM_ISCR_FIOCE_WIDTH                     (1U)
#define MCM_ISCR_FIOCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIOCE_SHIFT)) & MCM_ISCR_FIOCE_MASK)

#define MCM_ISCR_FDZCE_MASK                      (0x2000000U)
#define MCM_ISCR_FDZCE_SHIFT                     (25U)
#define MCM_ISCR_FDZCE_WIDTH                     (1U)
#define MCM_ISCR_FDZCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FDZCE_SHIFT)) & MCM_ISCR_FDZCE_MASK)

#define MCM_ISCR_FOFCE_MASK                      (0x4000000U)
#define MCM_ISCR_FOFCE_SHIFT                     (26U)
#define MCM_ISCR_FOFCE_WIDTH                     (1U)
#define MCM_ISCR_FOFCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FOFCE_SHIFT)) & MCM_ISCR_FOFCE_MASK)

#define MCM_ISCR_FUFCE_MASK                      (0x8000000U)
#define MCM_ISCR_FUFCE_SHIFT                     (27U)
#define MCM_ISCR_FUFCE_WIDTH                     (1U)
#define MCM_ISCR_FUFCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FUFCE_SHIFT)) & MCM_ISCR_FUFCE_MASK)

#define MCM_ISCR_FIXCE_MASK                      (0x10000000U)
#define MCM_ISCR_FIXCE_SHIFT                     (28U)
#define MCM_ISCR_FIXCE_WIDTH                     (1U)
#define MCM_ISCR_FIXCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIXCE_SHIFT)) & MCM_ISCR_FIXCE_MASK)

#define MCM_ISCR_FIDCE_MASK                      (0x80000000U)
#define MCM_ISCR_FIDCE_SHIFT                     (31U)
#define MCM_ISCR_FIDCE_WIDTH                     (1U)
#define MCM_ISCR_FIDCE(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_ISCR_FIDCE_SHIFT)) & MCM_ISCR_FIDCE_MASK)
/*! @} */

/*! @name PID - Process ID Register */
/*! @{ */

#define MCM_PID_PID_MASK                         (0xFFU)
#define MCM_PID_PID_SHIFT                        (0U)
#define MCM_PID_PID_WIDTH                        (8U)
#define MCM_PID_PID(x)                           (((uint32_t)(((uint32_t)(x)) << MCM_PID_PID_SHIFT)) & MCM_PID_PID_MASK)
/*! @} */

/*! @name CPO - Compute Operation Control Register */
/*! @{ */

#define MCM_CPO_CPOREQ_MASK                      (0x1U)
#define MCM_CPO_CPOREQ_SHIFT                     (0U)
#define MCM_CPO_CPOREQ_WIDTH                     (1U)
#define MCM_CPO_CPOREQ(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_CPO_CPOREQ_SHIFT)) & MCM_CPO_CPOREQ_MASK)

#define MCM_CPO_CPOACK_MASK                      (0x2U)
#define MCM_CPO_CPOACK_SHIFT                     (1U)
#define MCM_CPO_CPOACK_WIDTH                     (1U)
#define MCM_CPO_CPOACK(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_CPO_CPOACK_SHIFT)) & MCM_CPO_CPOACK_MASK)

#define MCM_CPO_CPOWOI_MASK                      (0x4U)
#define MCM_CPO_CPOWOI_SHIFT                     (2U)
#define MCM_CPO_CPOWOI_WIDTH                     (1U)
#define MCM_CPO_CPOWOI(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_CPO_CPOWOI_SHIFT)) & MCM_CPO_CPOWOI_MASK)
/*! @} */

/*! @name LMDR - Local Memory Descriptor Register */
/*! @{ */

#define MCM_LMDR_CF0_MASK                        (0xFU)
#define MCM_LMDR_CF0_SHIFT                       (0U)
#define MCM_LMDR_CF0_WIDTH                       (4U)
#define MCM_LMDR_CF0(x)                          (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_CF0_SHIFT)) & MCM_LMDR_CF0_MASK)

#define MCM_LMDR_MT_MASK                         (0xE000U)
#define MCM_LMDR_MT_SHIFT                        (13U)
#define MCM_LMDR_MT_WIDTH                        (3U)
#define MCM_LMDR_MT(x)                           (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_MT_SHIFT)) & MCM_LMDR_MT_MASK)

#define MCM_LMDR_LOCK_MASK                       (0x10000U)
#define MCM_LMDR_LOCK_SHIFT                      (16U)
#define MCM_LMDR_LOCK_WIDTH                      (1U)
#define MCM_LMDR_LOCK(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_LOCK_SHIFT)) & MCM_LMDR_LOCK_MASK)

#define MCM_LMDR_DPW_MASK                        (0xE0000U)
#define MCM_LMDR_DPW_SHIFT                       (17U)
#define MCM_LMDR_DPW_WIDTH                       (3U)
#define MCM_LMDR_DPW(x)                          (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_DPW_SHIFT)) & MCM_LMDR_DPW_MASK)

#define MCM_LMDR_WY_MASK                         (0xF00000U)
#define MCM_LMDR_WY_SHIFT                        (20U)
#define MCM_LMDR_WY_WIDTH                        (4U)
#define MCM_LMDR_WY(x)                           (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_WY_SHIFT)) & MCM_LMDR_WY_MASK)

#define MCM_LMDR_LMSZ_MASK                       (0xF000000U)
#define MCM_LMDR_LMSZ_SHIFT                      (24U)
#define MCM_LMDR_LMSZ_WIDTH                      (4U)
#define MCM_LMDR_LMSZ(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_LMSZ_SHIFT)) & MCM_LMDR_LMSZ_MASK)

#define MCM_LMDR_LMSZH_MASK                      (0x10000000U)
#define MCM_LMDR_LMSZH_SHIFT                     (28U)
#define MCM_LMDR_LMSZH_WIDTH                     (1U)
#define MCM_LMDR_LMSZH(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_LMSZH_SHIFT)) & MCM_LMDR_LMSZH_MASK)

#define MCM_LMDR_V_MASK                          (0x80000000U)
#define MCM_LMDR_V_SHIFT                         (31U)
#define MCM_LMDR_V_WIDTH                         (1U)
#define MCM_LMDR_V(x)                            (((uint32_t)(((uint32_t)(x)) << MCM_LMDR_V_SHIFT)) & MCM_LMDR_V_MASK)
/*! @} */

/*! @name LMDR2 - Local Memory Descriptor Register2 */
/*! @{ */

#define MCM_LMDR2_CF1_MASK                       (0xF0U)
#define MCM_LMDR2_CF1_SHIFT                      (4U)
#define MCM_LMDR2_CF1_WIDTH                      (4U)
#define MCM_LMDR2_CF1(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_CF1_SHIFT)) & MCM_LMDR2_CF1_MASK)

#define MCM_LMDR2_MT_MASK                        (0xE000U)
#define MCM_LMDR2_MT_SHIFT                       (13U)
#define MCM_LMDR2_MT_WIDTH                       (3U)
#define MCM_LMDR2_MT(x)                          (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_MT_SHIFT)) & MCM_LMDR2_MT_MASK)

#define MCM_LMDR2_LOCK_MASK                      (0x10000U)
#define MCM_LMDR2_LOCK_SHIFT                     (16U)
#define MCM_LMDR2_LOCK_WIDTH                     (1U)
#define MCM_LMDR2_LOCK(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_LOCK_SHIFT)) & MCM_LMDR2_LOCK_MASK)

#define MCM_LMDR2_DPW_MASK                       (0xE0000U)
#define MCM_LMDR2_DPW_SHIFT                      (17U)
#define MCM_LMDR2_DPW_WIDTH                      (3U)
#define MCM_LMDR2_DPW(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_DPW_SHIFT)) & MCM_LMDR2_DPW_MASK)

#define MCM_LMDR2_WY_MASK                        (0xF00000U)
#define MCM_LMDR2_WY_SHIFT                       (20U)
#define MCM_LMDR2_WY_WIDTH                       (4U)
#define MCM_LMDR2_WY(x)                          (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_WY_SHIFT)) & MCM_LMDR2_WY_MASK)

#define MCM_LMDR2_LMSZ_MASK                      (0xF000000U)
#define MCM_LMDR2_LMSZ_SHIFT                     (24U)
#define MCM_LMDR2_LMSZ_WIDTH                     (4U)
#define MCM_LMDR2_LMSZ(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_LMSZ_SHIFT)) & MCM_LMDR2_LMSZ_MASK)

#define MCM_LMDR2_LMSZH_MASK                     (0x10000000U)
#define MCM_LMDR2_LMSZH_SHIFT                    (28U)
#define MCM_LMDR2_LMSZH_WIDTH                    (1U)
#define MCM_LMDR2_LMSZH(x)                       (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_LMSZH_SHIFT)) & MCM_LMDR2_LMSZH_MASK)

#define MCM_LMDR2_V_MASK                         (0x80000000U)
#define MCM_LMDR2_V_SHIFT                        (31U)
#define MCM_LMDR2_V_WIDTH                        (1U)
#define MCM_LMDR2_V(x)                           (((uint32_t)(((uint32_t)(x)) << MCM_LMDR2_V_SHIFT)) & MCM_LMDR2_V_MASK)
/*! @} */

/*! @name LMPECR - LMEM Parity and ECC Control Register */
/*! @{ */

#define MCM_LMPECR_ERNCR_MASK                    (0x1U)
#define MCM_LMPECR_ERNCR_SHIFT                   (0U)
#define MCM_LMPECR_ERNCR_WIDTH                   (1U)
#define MCM_LMPECR_ERNCR(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_LMPECR_ERNCR_SHIFT)) & MCM_LMPECR_ERNCR_MASK)

#define MCM_LMPECR_ER1BR_MASK                    (0x100U)
#define MCM_LMPECR_ER1BR_SHIFT                   (8U)
#define MCM_LMPECR_ER1BR_WIDTH                   (1U)
#define MCM_LMPECR_ER1BR(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_LMPECR_ER1BR_SHIFT)) & MCM_LMPECR_ER1BR_MASK)

#define MCM_LMPECR_ECPR_MASK                     (0x100000U)
#define MCM_LMPECR_ECPR_SHIFT                    (20U)
#define MCM_LMPECR_ECPR_WIDTH                    (1U)
#define MCM_LMPECR_ECPR(x)                       (((uint32_t)(((uint32_t)(x)) << MCM_LMPECR_ECPR_SHIFT)) & MCM_LMPECR_ECPR_MASK)
/*! @} */

/*! @name LMPEIR - LMEM Parity and ECC Interrupt Register */
/*! @{ */

#define MCM_LMPEIR_ENC_MASK                      (0xFFU)
#define MCM_LMPEIR_ENC_SHIFT                     (0U)
#define MCM_LMPEIR_ENC_WIDTH                     (8U)
#define MCM_LMPEIR_ENC(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMPEIR_ENC_SHIFT)) & MCM_LMPEIR_ENC_MASK)

#define MCM_LMPEIR_E1B_MASK                      (0xFF00U)
#define MCM_LMPEIR_E1B_SHIFT                     (8U)
#define MCM_LMPEIR_E1B_WIDTH                     (8U)
#define MCM_LMPEIR_E1B(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMPEIR_E1B_SHIFT)) & MCM_LMPEIR_E1B_MASK)

#define MCM_LMPEIR_PE_MASK                       (0xFF0000U)
#define MCM_LMPEIR_PE_SHIFT                      (16U)
#define MCM_LMPEIR_PE_WIDTH                      (8U)
#define MCM_LMPEIR_PE(x)                         (((uint32_t)(((uint32_t)(x)) << MCM_LMPEIR_PE_SHIFT)) & MCM_LMPEIR_PE_MASK)

#define MCM_LMPEIR_PEELOC_MASK                   (0x1F000000U)
#define MCM_LMPEIR_PEELOC_SHIFT                  (24U)
#define MCM_LMPEIR_PEELOC_WIDTH                  (5U)
#define MCM_LMPEIR_PEELOC(x)                     (((uint32_t)(((uint32_t)(x)) << MCM_LMPEIR_PEELOC_SHIFT)) & MCM_LMPEIR_PEELOC_MASK)

#define MCM_LMPEIR_V_MASK                        (0x80000000U)
#define MCM_LMPEIR_V_SHIFT                       (31U)
#define MCM_LMPEIR_V_WIDTH                       (1U)
#define MCM_LMPEIR_V(x)                          (((uint32_t)(((uint32_t)(x)) << MCM_LMPEIR_V_SHIFT)) & MCM_LMPEIR_V_MASK)
/*! @} */

/*! @name LMFAR - LMEM Fault Address Register */
/*! @{ */

#define MCM_LMFAR_EFADD_MASK                     (0xFFFFFFFFU)
#define MCM_LMFAR_EFADD_SHIFT                    (0U)
#define MCM_LMFAR_EFADD_WIDTH                    (32U)
#define MCM_LMFAR_EFADD(x)                       (((uint32_t)(((uint32_t)(x)) << MCM_LMFAR_EFADD_SHIFT)) & MCM_LMFAR_EFADD_MASK)
/*! @} */

/*! @name LMFATR - LMEM Fault Attribute Register */
/*! @{ */

#define MCM_LMFATR_PEFPRT_MASK                   (0xFU)
#define MCM_LMFATR_PEFPRT_SHIFT                  (0U)
#define MCM_LMFATR_PEFPRT_WIDTH                  (4U)
#define MCM_LMFATR_PEFPRT(x)                     (((uint32_t)(((uint32_t)(x)) << MCM_LMFATR_PEFPRT_SHIFT)) & MCM_LMFATR_PEFPRT_MASK)

#define MCM_LMFATR_PEFSIZE_MASK                  (0x70U)
#define MCM_LMFATR_PEFSIZE_SHIFT                 (4U)
#define MCM_LMFATR_PEFSIZE_WIDTH                 (3U)
#define MCM_LMFATR_PEFSIZE(x)                    (((uint32_t)(((uint32_t)(x)) << MCM_LMFATR_PEFSIZE_SHIFT)) & MCM_LMFATR_PEFSIZE_MASK)

#define MCM_LMFATR_PEFW_MASK                     (0x80U)
#define MCM_LMFATR_PEFW_SHIFT                    (7U)
#define MCM_LMFATR_PEFW_WIDTH                    (1U)
#define MCM_LMFATR_PEFW(x)                       (((uint32_t)(((uint32_t)(x)) << MCM_LMFATR_PEFW_SHIFT)) & MCM_LMFATR_PEFW_MASK)

#define MCM_LMFATR_PEFMST_MASK                   (0xFF00U)
#define MCM_LMFATR_PEFMST_SHIFT                  (8U)
#define MCM_LMFATR_PEFMST_WIDTH                  (8U)
#define MCM_LMFATR_PEFMST(x)                     (((uint32_t)(((uint32_t)(x)) << MCM_LMFATR_PEFMST_SHIFT)) & MCM_LMFATR_PEFMST_MASK)

#define MCM_LMFATR_OVR_MASK                      (0x80000000U)
#define MCM_LMFATR_OVR_SHIFT                     (31U)
#define MCM_LMFATR_OVR_WIDTH                     (1U)
#define MCM_LMFATR_OVR(x)                        (((uint32_t)(((uint32_t)(x)) << MCM_LMFATR_OVR_SHIFT)) & MCM_LMFATR_OVR_MASK)
/*! @} */

/*! @name LMFDHR - LMEM Fault Data High Register */
/*! @{ */

#define MCM_LMFDHR_PEFDH_MASK                    (0xFFFFFFFFU)
#define MCM_LMFDHR_PEFDH_SHIFT                   (0U)
#define MCM_LMFDHR_PEFDH_WIDTH                   (32U)
#define MCM_LMFDHR_PEFDH(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_LMFDHR_PEFDH_SHIFT)) & MCM_LMFDHR_PEFDH_MASK)
/*! @} */

/*! @name LMFDLR - LMEM Fault Data Low Register */
/*! @{ */

#define MCM_LMFDLR_PEFDL_MASK                    (0xFFFFFFFFU)
#define MCM_LMFDLR_PEFDL_SHIFT                   (0U)
#define MCM_LMFDLR_PEFDL_WIDTH                   (32U)
#define MCM_LMFDLR_PEFDL(x)                      (((uint32_t)(((uint32_t)(x)) << MCM_LMFDLR_PEFDL_SHIFT)) & MCM_LMFDLR_PEFDL_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group MCM_Register_Masks */

/*!
 * @}
 */ /* end of group MCM_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_MCM_H_) */

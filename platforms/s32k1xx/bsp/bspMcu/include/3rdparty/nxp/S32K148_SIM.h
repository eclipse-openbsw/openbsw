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
 * @file S32K148_SIM.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_SIM
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
#if !defined(S32K148_SIM_H_)  /* Check if memory map has not been already included */
#define S32K148_SIM_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- SIM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SIM_Peripheral_Access_Layer SIM Peripheral Access Layer
 * @{
 */

/** SIM - Register Layout Typedef */
typedef struct {
  uint8_t RESERVED_0[4];
  __IO uint32_t CHIPCTL;                           /**< Chip Control register, offset: 0x4 */
  uint8_t RESERVED_1[4];
  __IO uint32_t FTMOPT0;                           /**< FTM Option Register 0, offset: 0xC */
  __IO uint32_t LPOCLKS;                           /**< LPO Clock Select Register, offset: 0x10 */
  uint8_t RESERVED_2[4];
  __IO uint32_t ADCOPT;                            /**< ADC Options Register, offset: 0x18 */
  __IO uint32_t FTMOPT1;                           /**< FTM Option Register 1, offset: 0x1C */
  __IO uint32_t MISCTRL0;                          /**< Miscellaneous control register 0, offset: 0x20 */
  __I  uint32_t SDID;                              /**< System Device Identification Register, offset: 0x24 */
  uint8_t RESERVED_3[24];
  __IO uint32_t PLATCGC;                           /**< Platform Clock Gating Control Register, offset: 0x40 */
  uint8_t RESERVED_4[8];
  __I  uint32_t FCFG1;                             /**< Flash Configuration Register 1, offset: 0x4C */
  uint8_t RESERVED_5[4];
  __I  uint32_t UIDH;                              /**< Unique Identification Register High, offset: 0x54 */
  __I  uint32_t UIDMH;                             /**< Unique Identification Register Mid-High, offset: 0x58 */
  __I  uint32_t UIDML;                             /**< Unique Identification Register Mid Low, offset: 0x5C */
  __I  uint32_t UIDL;                              /**< Unique Identification Register Low, offset: 0x60 */
  uint8_t RESERVED_6[4];
  __IO uint32_t CLKDIV4;                           /**< System Clock Divider Register 4, offset: 0x68 */
  __IO uint32_t MISCTRL1;                          /**< Miscellaneous Control register 1, offset: 0x6C */
} SIM_Type, *SIM_MemMapPtr;

/** Number of instances of the SIM module. */
#define SIM_INSTANCE_COUNT                       (1u)

/* SIM - Peripheral instance base addresses */
/** Peripheral SIM base address */
#define IP_SIM_BASE                              (0x40048000u)
/** Peripheral SIM base pointer */
#define IP_SIM                                   ((SIM_Type *)IP_SIM_BASE)
/** Array initializer of SIM peripheral base addresses */
#define IP_SIM_BASE_ADDRS                        { IP_SIM_BASE }
/** Array initializer of SIM peripheral base pointers */
#define IP_SIM_BASE_PTRS                         { IP_SIM }

/* ----------------------------------------------------------------------------
   -- SIM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup SIM_Register_Masks SIM Register Masks
 * @{
 */

/*! @name CHIPCTL - Chip Control register */
/*! @{ */

#define SIM_CHIPCTL_ADC_INTERLEAVE_EN_MASK       (0xFU)
#define SIM_CHIPCTL_ADC_INTERLEAVE_EN_SHIFT      (0U)
#define SIM_CHIPCTL_ADC_INTERLEAVE_EN_WIDTH      (4U)
#define SIM_CHIPCTL_ADC_INTERLEAVE_EN(x)         (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_ADC_INTERLEAVE_EN_SHIFT)) & SIM_CHIPCTL_ADC_INTERLEAVE_EN_MASK)

#define SIM_CHIPCTL_CLKOUTSEL_MASK               (0xF0U)
#define SIM_CHIPCTL_CLKOUTSEL_SHIFT              (4U)
#define SIM_CHIPCTL_CLKOUTSEL_WIDTH              (4U)
#define SIM_CHIPCTL_CLKOUTSEL(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_CLKOUTSEL_SHIFT)) & SIM_CHIPCTL_CLKOUTSEL_MASK)

#define SIM_CHIPCTL_CLKOUTDIV_MASK               (0x700U)
#define SIM_CHIPCTL_CLKOUTDIV_SHIFT              (8U)
#define SIM_CHIPCTL_CLKOUTDIV_WIDTH              (3U)
#define SIM_CHIPCTL_CLKOUTDIV(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_CLKOUTDIV_SHIFT)) & SIM_CHIPCTL_CLKOUTDIV_MASK)

#define SIM_CHIPCTL_CLKOUTEN_MASK                (0x800U)
#define SIM_CHIPCTL_CLKOUTEN_SHIFT               (11U)
#define SIM_CHIPCTL_CLKOUTEN_WIDTH               (1U)
#define SIM_CHIPCTL_CLKOUTEN(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_CLKOUTEN_SHIFT)) & SIM_CHIPCTL_CLKOUTEN_MASK)

#define SIM_CHIPCTL_TRACECLK_SEL_MASK            (0x1000U)
#define SIM_CHIPCTL_TRACECLK_SEL_SHIFT           (12U)
#define SIM_CHIPCTL_TRACECLK_SEL_WIDTH           (1U)
#define SIM_CHIPCTL_TRACECLK_SEL(x)              (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_TRACECLK_SEL_SHIFT)) & SIM_CHIPCTL_TRACECLK_SEL_MASK)

#define SIM_CHIPCTL_PDB_BB_SEL_MASK              (0x2000U)
#define SIM_CHIPCTL_PDB_BB_SEL_SHIFT             (13U)
#define SIM_CHIPCTL_PDB_BB_SEL_WIDTH             (1U)
#define SIM_CHIPCTL_PDB_BB_SEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_PDB_BB_SEL_SHIFT)) & SIM_CHIPCTL_PDB_BB_SEL_MASK)

#define SIM_CHIPCTL_ADC_SUPPLY_MASK              (0x70000U)
#define SIM_CHIPCTL_ADC_SUPPLY_SHIFT             (16U)
#define SIM_CHIPCTL_ADC_SUPPLY_WIDTH             (3U)
#define SIM_CHIPCTL_ADC_SUPPLY(x)                (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_ADC_SUPPLY_SHIFT)) & SIM_CHIPCTL_ADC_SUPPLY_MASK)

#define SIM_CHIPCTL_ADC_SUPPLYEN_MASK            (0x80000U)
#define SIM_CHIPCTL_ADC_SUPPLYEN_SHIFT           (19U)
#define SIM_CHIPCTL_ADC_SUPPLYEN_WIDTH           (1U)
#define SIM_CHIPCTL_ADC_SUPPLYEN(x)              (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_ADC_SUPPLYEN_SHIFT)) & SIM_CHIPCTL_ADC_SUPPLYEN_MASK)

#define SIM_CHIPCTL_SRAMU_RETEN_MASK             (0x100000U)
#define SIM_CHIPCTL_SRAMU_RETEN_SHIFT            (20U)
#define SIM_CHIPCTL_SRAMU_RETEN_WIDTH            (1U)
#define SIM_CHIPCTL_SRAMU_RETEN(x)               (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_SRAMU_RETEN_SHIFT)) & SIM_CHIPCTL_SRAMU_RETEN_MASK)

#define SIM_CHIPCTL_SRAML_RETEN_MASK             (0x200000U)
#define SIM_CHIPCTL_SRAML_RETEN_SHIFT            (21U)
#define SIM_CHIPCTL_SRAML_RETEN_WIDTH            (1U)
#define SIM_CHIPCTL_SRAML_RETEN(x)               (((uint32_t)(((uint32_t)(x)) << SIM_CHIPCTL_SRAML_RETEN_SHIFT)) & SIM_CHIPCTL_SRAML_RETEN_MASK)
/*! @} */

/*! @name FTMOPT0 - FTM Option Register 0 */
/*! @{ */

#define SIM_FTMOPT0_FTM0FLTxSEL_MASK             (0x7U)
#define SIM_FTMOPT0_FTM0FLTxSEL_SHIFT            (0U)
#define SIM_FTMOPT0_FTM0FLTxSEL_WIDTH            (3U)
#define SIM_FTMOPT0_FTM0FLTxSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM0FLTxSEL_SHIFT)) & SIM_FTMOPT0_FTM0FLTxSEL_MASK)

#define SIM_FTMOPT0_FTM1FLTxSEL_MASK             (0x70U)
#define SIM_FTMOPT0_FTM1FLTxSEL_SHIFT            (4U)
#define SIM_FTMOPT0_FTM1FLTxSEL_WIDTH            (3U)
#define SIM_FTMOPT0_FTM1FLTxSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM1FLTxSEL_SHIFT)) & SIM_FTMOPT0_FTM1FLTxSEL_MASK)

#define SIM_FTMOPT0_FTM2FLTxSEL_MASK             (0x700U)
#define SIM_FTMOPT0_FTM2FLTxSEL_SHIFT            (8U)
#define SIM_FTMOPT0_FTM2FLTxSEL_WIDTH            (3U)
#define SIM_FTMOPT0_FTM2FLTxSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM2FLTxSEL_SHIFT)) & SIM_FTMOPT0_FTM2FLTxSEL_MASK)

#define SIM_FTMOPT0_FTM3FLTxSEL_MASK             (0x7000U)
#define SIM_FTMOPT0_FTM3FLTxSEL_SHIFT            (12U)
#define SIM_FTMOPT0_FTM3FLTxSEL_WIDTH            (3U)
#define SIM_FTMOPT0_FTM3FLTxSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM3FLTxSEL_SHIFT)) & SIM_FTMOPT0_FTM3FLTxSEL_MASK)

#define SIM_FTMOPT0_FTM4CLKSEL_MASK              (0x30000U)
#define SIM_FTMOPT0_FTM4CLKSEL_SHIFT             (16U)
#define SIM_FTMOPT0_FTM4CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM4CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM4CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM4CLKSEL_MASK)

#define SIM_FTMOPT0_FTM5CLKSEL_MASK              (0xC0000U)
#define SIM_FTMOPT0_FTM5CLKSEL_SHIFT             (18U)
#define SIM_FTMOPT0_FTM5CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM5CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM5CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM5CLKSEL_MASK)

#define SIM_FTMOPT0_FTM6CLKSEL_MASK              (0x300000U)
#define SIM_FTMOPT0_FTM6CLKSEL_SHIFT             (20U)
#define SIM_FTMOPT0_FTM6CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM6CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM6CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM6CLKSEL_MASK)

#define SIM_FTMOPT0_FTM7CLKSEL_MASK              (0xC00000U)
#define SIM_FTMOPT0_FTM7CLKSEL_SHIFT             (22U)
#define SIM_FTMOPT0_FTM7CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM7CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM7CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM7CLKSEL_MASK)

#define SIM_FTMOPT0_FTM0CLKSEL_MASK              (0x3000000U)
#define SIM_FTMOPT0_FTM0CLKSEL_SHIFT             (24U)
#define SIM_FTMOPT0_FTM0CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM0CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM0CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM0CLKSEL_MASK)

#define SIM_FTMOPT0_FTM1CLKSEL_MASK              (0xC000000U)
#define SIM_FTMOPT0_FTM1CLKSEL_SHIFT             (26U)
#define SIM_FTMOPT0_FTM1CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM1CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM1CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM1CLKSEL_MASK)

#define SIM_FTMOPT0_FTM2CLKSEL_MASK              (0x30000000U)
#define SIM_FTMOPT0_FTM2CLKSEL_SHIFT             (28U)
#define SIM_FTMOPT0_FTM2CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM2CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM2CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM2CLKSEL_MASK)

#define SIM_FTMOPT0_FTM3CLKSEL_MASK              (0xC0000000U)
#define SIM_FTMOPT0_FTM3CLKSEL_SHIFT             (30U)
#define SIM_FTMOPT0_FTM3CLKSEL_WIDTH             (2U)
#define SIM_FTMOPT0_FTM3CLKSEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT0_FTM3CLKSEL_SHIFT)) & SIM_FTMOPT0_FTM3CLKSEL_MASK)
/*! @} */

/*! @name LPOCLKS - LPO Clock Select Register */
/*! @{ */

#define SIM_LPOCLKS_LPO1KCLKEN_MASK              (0x1U)
#define SIM_LPOCLKS_LPO1KCLKEN_SHIFT             (0U)
#define SIM_LPOCLKS_LPO1KCLKEN_WIDTH             (1U)
#define SIM_LPOCLKS_LPO1KCLKEN(x)                (((uint32_t)(((uint32_t)(x)) << SIM_LPOCLKS_LPO1KCLKEN_SHIFT)) & SIM_LPOCLKS_LPO1KCLKEN_MASK)

#define SIM_LPOCLKS_LPO32KCLKEN_MASK             (0x2U)
#define SIM_LPOCLKS_LPO32KCLKEN_SHIFT            (1U)
#define SIM_LPOCLKS_LPO32KCLKEN_WIDTH            (1U)
#define SIM_LPOCLKS_LPO32KCLKEN(x)               (((uint32_t)(((uint32_t)(x)) << SIM_LPOCLKS_LPO32KCLKEN_SHIFT)) & SIM_LPOCLKS_LPO32KCLKEN_MASK)

#define SIM_LPOCLKS_LPOCLKSEL_MASK               (0xCU)
#define SIM_LPOCLKS_LPOCLKSEL_SHIFT              (2U)
#define SIM_LPOCLKS_LPOCLKSEL_WIDTH              (2U)
#define SIM_LPOCLKS_LPOCLKSEL(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_LPOCLKS_LPOCLKSEL_SHIFT)) & SIM_LPOCLKS_LPOCLKSEL_MASK)

#define SIM_LPOCLKS_RTCCLKSEL_MASK               (0x30U)
#define SIM_LPOCLKS_RTCCLKSEL_SHIFT              (4U)
#define SIM_LPOCLKS_RTCCLKSEL_WIDTH              (2U)
#define SIM_LPOCLKS_RTCCLKSEL(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_LPOCLKS_RTCCLKSEL_SHIFT)) & SIM_LPOCLKS_RTCCLKSEL_MASK)
/*! @} */

/*! @name ADCOPT - ADC Options Register */
/*! @{ */

#define SIM_ADCOPT_ADC0TRGSEL_MASK               (0x1U)
#define SIM_ADCOPT_ADC0TRGSEL_SHIFT              (0U)
#define SIM_ADCOPT_ADC0TRGSEL_WIDTH              (1U)
#define SIM_ADCOPT_ADC0TRGSEL(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC0TRGSEL_SHIFT)) & SIM_ADCOPT_ADC0TRGSEL_MASK)

#define SIM_ADCOPT_ADC0SWPRETRG_MASK             (0xEU)
#define SIM_ADCOPT_ADC0SWPRETRG_SHIFT            (1U)
#define SIM_ADCOPT_ADC0SWPRETRG_WIDTH            (3U)
#define SIM_ADCOPT_ADC0SWPRETRG(x)               (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC0SWPRETRG_SHIFT)) & SIM_ADCOPT_ADC0SWPRETRG_MASK)

#define SIM_ADCOPT_ADC0PRETRGSEL_MASK            (0x30U)
#define SIM_ADCOPT_ADC0PRETRGSEL_SHIFT           (4U)
#define SIM_ADCOPT_ADC0PRETRGSEL_WIDTH           (2U)
#define SIM_ADCOPT_ADC0PRETRGSEL(x)              (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC0PRETRGSEL_SHIFT)) & SIM_ADCOPT_ADC0PRETRGSEL_MASK)

#define SIM_ADCOPT_ADC1TRGSEL_MASK               (0x100U)
#define SIM_ADCOPT_ADC1TRGSEL_SHIFT              (8U)
#define SIM_ADCOPT_ADC1TRGSEL_WIDTH              (1U)
#define SIM_ADCOPT_ADC1TRGSEL(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC1TRGSEL_SHIFT)) & SIM_ADCOPT_ADC1TRGSEL_MASK)

#define SIM_ADCOPT_ADC1SWPRETRG_MASK             (0xE00U)
#define SIM_ADCOPT_ADC1SWPRETRG_SHIFT            (9U)
#define SIM_ADCOPT_ADC1SWPRETRG_WIDTH            (3U)
#define SIM_ADCOPT_ADC1SWPRETRG(x)               (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC1SWPRETRG_SHIFT)) & SIM_ADCOPT_ADC1SWPRETRG_MASK)

#define SIM_ADCOPT_ADC1PRETRGSEL_MASK            (0x3000U)
#define SIM_ADCOPT_ADC1PRETRGSEL_SHIFT           (12U)
#define SIM_ADCOPT_ADC1PRETRGSEL_WIDTH           (2U)
#define SIM_ADCOPT_ADC1PRETRGSEL(x)              (((uint32_t)(((uint32_t)(x)) << SIM_ADCOPT_ADC1PRETRGSEL_SHIFT)) & SIM_ADCOPT_ADC1PRETRGSEL_MASK)
/*! @} */

/*! @name FTMOPT1 - FTM Option Register 1 */
/*! @{ */

#define SIM_FTMOPT1_FTM0SYNCBIT_MASK             (0x1U)
#define SIM_FTMOPT1_FTM0SYNCBIT_SHIFT            (0U)
#define SIM_FTMOPT1_FTM0SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM0SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM0SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM0SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM1SYNCBIT_MASK             (0x2U)
#define SIM_FTMOPT1_FTM1SYNCBIT_SHIFT            (1U)
#define SIM_FTMOPT1_FTM1SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM1SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM1SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM1SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM2SYNCBIT_MASK             (0x4U)
#define SIM_FTMOPT1_FTM2SYNCBIT_SHIFT            (2U)
#define SIM_FTMOPT1_FTM2SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM2SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM2SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM2SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM3SYNCBIT_MASK             (0x8U)
#define SIM_FTMOPT1_FTM3SYNCBIT_SHIFT            (3U)
#define SIM_FTMOPT1_FTM3SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM3SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM3SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM3SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM1CH0SEL_MASK              (0x30U)
#define SIM_FTMOPT1_FTM1CH0SEL_SHIFT             (4U)
#define SIM_FTMOPT1_FTM1CH0SEL_WIDTH             (2U)
#define SIM_FTMOPT1_FTM1CH0SEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM1CH0SEL_SHIFT)) & SIM_FTMOPT1_FTM1CH0SEL_MASK)

#define SIM_FTMOPT1_FTM2CH0SEL_MASK              (0xC0U)
#define SIM_FTMOPT1_FTM2CH0SEL_SHIFT             (6U)
#define SIM_FTMOPT1_FTM2CH0SEL_WIDTH             (2U)
#define SIM_FTMOPT1_FTM2CH0SEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM2CH0SEL_SHIFT)) & SIM_FTMOPT1_FTM2CH0SEL_MASK)

#define SIM_FTMOPT1_FTM2CH1SEL_MASK              (0x100U)
#define SIM_FTMOPT1_FTM2CH1SEL_SHIFT             (8U)
#define SIM_FTMOPT1_FTM2CH1SEL_WIDTH             (1U)
#define SIM_FTMOPT1_FTM2CH1SEL(x)                (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM2CH1SEL_SHIFT)) & SIM_FTMOPT1_FTM2CH1SEL_MASK)

#define SIM_FTMOPT1_FTM4SYNCBIT_MASK             (0x800U)
#define SIM_FTMOPT1_FTM4SYNCBIT_SHIFT            (11U)
#define SIM_FTMOPT1_FTM4SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM4SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM4SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM4SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM5SYNCBIT_MASK             (0x1000U)
#define SIM_FTMOPT1_FTM5SYNCBIT_SHIFT            (12U)
#define SIM_FTMOPT1_FTM5SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM5SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM5SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM5SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM6SYNCBIT_MASK             (0x2000U)
#define SIM_FTMOPT1_FTM6SYNCBIT_SHIFT            (13U)
#define SIM_FTMOPT1_FTM6SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM6SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM6SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM6SYNCBIT_MASK)

#define SIM_FTMOPT1_FTM7SYNCBIT_MASK             (0x4000U)
#define SIM_FTMOPT1_FTM7SYNCBIT_SHIFT            (14U)
#define SIM_FTMOPT1_FTM7SYNCBIT_WIDTH            (1U)
#define SIM_FTMOPT1_FTM7SYNCBIT(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM7SYNCBIT_SHIFT)) & SIM_FTMOPT1_FTM7SYNCBIT_MASK)

#define SIM_FTMOPT1_FTMGLDOK_MASK                (0x8000U)
#define SIM_FTMOPT1_FTMGLDOK_SHIFT               (15U)
#define SIM_FTMOPT1_FTMGLDOK_WIDTH               (1U)
#define SIM_FTMOPT1_FTMGLDOK(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTMGLDOK_SHIFT)) & SIM_FTMOPT1_FTMGLDOK_MASK)

#define SIM_FTMOPT1_FTM0_OUTSEL_MASK             (0xFF0000U)
#define SIM_FTMOPT1_FTM0_OUTSEL_SHIFT            (16U)
#define SIM_FTMOPT1_FTM0_OUTSEL_WIDTH            (8U)
#define SIM_FTMOPT1_FTM0_OUTSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM0_OUTSEL_SHIFT)) & SIM_FTMOPT1_FTM0_OUTSEL_MASK)

#define SIM_FTMOPT1_FTM3_OUTSEL_MASK             (0xFF000000U)
#define SIM_FTMOPT1_FTM3_OUTSEL_SHIFT            (24U)
#define SIM_FTMOPT1_FTM3_OUTSEL_WIDTH            (8U)
#define SIM_FTMOPT1_FTM3_OUTSEL(x)               (((uint32_t)(((uint32_t)(x)) << SIM_FTMOPT1_FTM3_OUTSEL_SHIFT)) & SIM_FTMOPT1_FTM3_OUTSEL_MASK)
/*! @} */

/*! @name MISCTRL0 - Miscellaneous control register 0 */
/*! @{ */

#define SIM_MISCTRL0_STOP1_MONITOR_MASK          (0x200U)
#define SIM_MISCTRL0_STOP1_MONITOR_SHIFT         (9U)
#define SIM_MISCTRL0_STOP1_MONITOR_WIDTH         (1U)
#define SIM_MISCTRL0_STOP1_MONITOR(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_STOP1_MONITOR_SHIFT)) & SIM_MISCTRL0_STOP1_MONITOR_MASK)

#define SIM_MISCTRL0_STOP2_MONITOR_MASK          (0x400U)
#define SIM_MISCTRL0_STOP2_MONITOR_SHIFT         (10U)
#define SIM_MISCTRL0_STOP2_MONITOR_WIDTH         (1U)
#define SIM_MISCTRL0_STOP2_MONITOR(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_STOP2_MONITOR_SHIFT)) & SIM_MISCTRL0_STOP2_MONITOR_MASK)

#define SIM_MISCTRL0_FTM_GTB_SPLIT_EN_MASK       (0x4000U)
#define SIM_MISCTRL0_FTM_GTB_SPLIT_EN_SHIFT      (14U)
#define SIM_MISCTRL0_FTM_GTB_SPLIT_EN_WIDTH      (1U)
#define SIM_MISCTRL0_FTM_GTB_SPLIT_EN(x)         (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM_GTB_SPLIT_EN_SHIFT)) & SIM_MISCTRL0_FTM_GTB_SPLIT_EN_MASK)

#define SIM_MISCTRL0_FTM0_OBE_CTRL_MASK          (0x10000U)
#define SIM_MISCTRL0_FTM0_OBE_CTRL_SHIFT         (16U)
#define SIM_MISCTRL0_FTM0_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM0_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM0_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM0_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM1_OBE_CTRL_MASK          (0x20000U)
#define SIM_MISCTRL0_FTM1_OBE_CTRL_SHIFT         (17U)
#define SIM_MISCTRL0_FTM1_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM1_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM1_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM1_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM2_OBE_CTRL_MASK          (0x40000U)
#define SIM_MISCTRL0_FTM2_OBE_CTRL_SHIFT         (18U)
#define SIM_MISCTRL0_FTM2_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM2_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM2_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM2_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM3_OBE_CTRL_MASK          (0x80000U)
#define SIM_MISCTRL0_FTM3_OBE_CTRL_SHIFT         (19U)
#define SIM_MISCTRL0_FTM3_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM3_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM3_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM3_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM4_OBE_CTRL_MASK          (0x100000U)
#define SIM_MISCTRL0_FTM4_OBE_CTRL_SHIFT         (20U)
#define SIM_MISCTRL0_FTM4_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM4_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM4_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM4_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM5_OBE_CTRL_MASK          (0x200000U)
#define SIM_MISCTRL0_FTM5_OBE_CTRL_SHIFT         (21U)
#define SIM_MISCTRL0_FTM5_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM5_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM5_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM5_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM6_OBE_CTRL_MASK          (0x400000U)
#define SIM_MISCTRL0_FTM6_OBE_CTRL_SHIFT         (22U)
#define SIM_MISCTRL0_FTM6_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM6_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM6_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM6_OBE_CTRL_MASK)

#define SIM_MISCTRL0_FTM7_OBE_CTRL_MASK          (0x800000U)
#define SIM_MISCTRL0_FTM7_OBE_CTRL_SHIFT         (23U)
#define SIM_MISCTRL0_FTM7_OBE_CTRL_WIDTH         (1U)
#define SIM_MISCTRL0_FTM7_OBE_CTRL(x)            (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_FTM7_OBE_CTRL_SHIFT)) & SIM_MISCTRL0_FTM7_OBE_CTRL_MASK)

#define SIM_MISCTRL0_RMII_CLK_OBE_MASK           (0x1000000U)
#define SIM_MISCTRL0_RMII_CLK_OBE_SHIFT          (24U)
#define SIM_MISCTRL0_RMII_CLK_OBE_WIDTH          (1U)
#define SIM_MISCTRL0_RMII_CLK_OBE(x)             (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_RMII_CLK_OBE_SHIFT)) & SIM_MISCTRL0_RMII_CLK_OBE_MASK)

#define SIM_MISCTRL0_RMII_CLK_SEL_MASK           (0x2000000U)
#define SIM_MISCTRL0_RMII_CLK_SEL_SHIFT          (25U)
#define SIM_MISCTRL0_RMII_CLK_SEL_WIDTH          (1U)
#define SIM_MISCTRL0_RMII_CLK_SEL(x)             (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_RMII_CLK_SEL_SHIFT)) & SIM_MISCTRL0_RMII_CLK_SEL_MASK)

#define SIM_MISCTRL0_QSPI_CLK_SEL_MASK           (0x4000000U)
#define SIM_MISCTRL0_QSPI_CLK_SEL_SHIFT          (26U)
#define SIM_MISCTRL0_QSPI_CLK_SEL_WIDTH          (1U)
#define SIM_MISCTRL0_QSPI_CLK_SEL(x)             (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL0_QSPI_CLK_SEL_SHIFT)) & SIM_MISCTRL0_QSPI_CLK_SEL_MASK)
/*! @} */

/*! @name SDID - System Device Identification Register */
/*! @{ */

#define SIM_SDID_FEATURES_MASK                   (0xFFU)
#define SIM_SDID_FEATURES_SHIFT                  (0U)
#define SIM_SDID_FEATURES_WIDTH                  (8U)
#define SIM_SDID_FEATURES(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SDID_FEATURES_SHIFT)) & SIM_SDID_FEATURES_MASK)

#define SIM_SDID_PACKAGE_MASK                    (0xF00U)
#define SIM_SDID_PACKAGE_SHIFT                   (8U)
#define SIM_SDID_PACKAGE_WIDTH                   (4U)
#define SIM_SDID_PACKAGE(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_SDID_PACKAGE_SHIFT)) & SIM_SDID_PACKAGE_MASK)

#define SIM_SDID_REVID_MASK                      (0xF000U)
#define SIM_SDID_REVID_SHIFT                     (12U)
#define SIM_SDID_REVID_WIDTH                     (4U)
#define SIM_SDID_REVID(x)                        (((uint32_t)(((uint32_t)(x)) << SIM_SDID_REVID_SHIFT)) & SIM_SDID_REVID_MASK)

#define SIM_SDID_RAMSIZE_MASK                    (0xF0000U)
#define SIM_SDID_RAMSIZE_SHIFT                   (16U)
#define SIM_SDID_RAMSIZE_WIDTH                   (4U)
#define SIM_SDID_RAMSIZE(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_SDID_RAMSIZE_SHIFT)) & SIM_SDID_RAMSIZE_MASK)

#define SIM_SDID_DERIVATE_MASK                   (0xF00000U)
#define SIM_SDID_DERIVATE_SHIFT                  (20U)
#define SIM_SDID_DERIVATE_WIDTH                  (4U)
#define SIM_SDID_DERIVATE(x)                     (((uint32_t)(((uint32_t)(x)) << SIM_SDID_DERIVATE_SHIFT)) & SIM_SDID_DERIVATE_MASK)

#define SIM_SDID_SUBSERIES_MASK                  (0xF000000U)
#define SIM_SDID_SUBSERIES_SHIFT                 (24U)
#define SIM_SDID_SUBSERIES_WIDTH                 (4U)
#define SIM_SDID_SUBSERIES(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_SDID_SUBSERIES_SHIFT)) & SIM_SDID_SUBSERIES_MASK)

#define SIM_SDID_GENERATION_MASK                 (0xF0000000U)
#define SIM_SDID_GENERATION_SHIFT                (28U)
#define SIM_SDID_GENERATION_WIDTH                (4U)
#define SIM_SDID_GENERATION(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_SDID_GENERATION_SHIFT)) & SIM_SDID_GENERATION_MASK)
/*! @} */

/*! @name PLATCGC - Platform Clock Gating Control Register */
/*! @{ */

#define SIM_PLATCGC_CGCMSCM_MASK                 (0x1U)
#define SIM_PLATCGC_CGCMSCM_SHIFT                (0U)
#define SIM_PLATCGC_CGCMSCM_WIDTH                (1U)
#define SIM_PLATCGC_CGCMSCM(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_PLATCGC_CGCMSCM_SHIFT)) & SIM_PLATCGC_CGCMSCM_MASK)

#define SIM_PLATCGC_CGCMPU_MASK                  (0x2U)
#define SIM_PLATCGC_CGCMPU_SHIFT                 (1U)
#define SIM_PLATCGC_CGCMPU_WIDTH                 (1U)
#define SIM_PLATCGC_CGCMPU(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_PLATCGC_CGCMPU_SHIFT)) & SIM_PLATCGC_CGCMPU_MASK)

#define SIM_PLATCGC_CGCDMA_MASK                  (0x4U)
#define SIM_PLATCGC_CGCDMA_SHIFT                 (2U)
#define SIM_PLATCGC_CGCDMA_WIDTH                 (1U)
#define SIM_PLATCGC_CGCDMA(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_PLATCGC_CGCDMA_SHIFT)) & SIM_PLATCGC_CGCDMA_MASK)

#define SIM_PLATCGC_CGCERM_MASK                  (0x8U)
#define SIM_PLATCGC_CGCERM_SHIFT                 (3U)
#define SIM_PLATCGC_CGCERM_WIDTH                 (1U)
#define SIM_PLATCGC_CGCERM(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_PLATCGC_CGCERM_SHIFT)) & SIM_PLATCGC_CGCERM_MASK)

#define SIM_PLATCGC_CGCEIM_MASK                  (0x10U)
#define SIM_PLATCGC_CGCEIM_SHIFT                 (4U)
#define SIM_PLATCGC_CGCEIM_WIDTH                 (1U)
#define SIM_PLATCGC_CGCEIM(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_PLATCGC_CGCEIM_SHIFT)) & SIM_PLATCGC_CGCEIM_MASK)
/*! @} */

/*! @name FCFG1 - Flash Configuration Register 1 */
/*! @{ */

#define SIM_FCFG1_DEPART_MASK                    (0xF000U)
#define SIM_FCFG1_DEPART_SHIFT                   (12U)
#define SIM_FCFG1_DEPART_WIDTH                   (4U)
#define SIM_FCFG1_DEPART(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_DEPART_SHIFT)) & SIM_FCFG1_DEPART_MASK)

#define SIM_FCFG1_EEERAMSIZE_MASK                (0xF0000U)
#define SIM_FCFG1_EEERAMSIZE_SHIFT               (16U)
#define SIM_FCFG1_EEERAMSIZE_WIDTH               (4U)
#define SIM_FCFG1_EEERAMSIZE(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_FCFG1_EEERAMSIZE_SHIFT)) & SIM_FCFG1_EEERAMSIZE_MASK)
/*! @} */

/*! @name UIDH - Unique Identification Register High */
/*! @{ */

#define SIM_UIDH_UID127_96_MASK                  (0xFFFFFFFFU)
#define SIM_UIDH_UID127_96_SHIFT                 (0U)
#define SIM_UIDH_UID127_96_WIDTH                 (32U)
#define SIM_UIDH_UID127_96(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_UIDH_UID127_96_SHIFT)) & SIM_UIDH_UID127_96_MASK)
/*! @} */

/*! @name UIDMH - Unique Identification Register Mid-High */
/*! @{ */

#define SIM_UIDMH_UID95_64_MASK                  (0xFFFFFFFFU)
#define SIM_UIDMH_UID95_64_SHIFT                 (0U)
#define SIM_UIDMH_UID95_64_WIDTH                 (32U)
#define SIM_UIDMH_UID95_64(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_UIDMH_UID95_64_SHIFT)) & SIM_UIDMH_UID95_64_MASK)
/*! @} */

/*! @name UIDML - Unique Identification Register Mid Low */
/*! @{ */

#define SIM_UIDML_UID63_32_MASK                  (0xFFFFFFFFU)
#define SIM_UIDML_UID63_32_SHIFT                 (0U)
#define SIM_UIDML_UID63_32_WIDTH                 (32U)
#define SIM_UIDML_UID63_32(x)                    (((uint32_t)(((uint32_t)(x)) << SIM_UIDML_UID63_32_SHIFT)) & SIM_UIDML_UID63_32_MASK)
/*! @} */

/*! @name UIDL - Unique Identification Register Low */
/*! @{ */

#define SIM_UIDL_UID31_0_MASK                    (0xFFFFFFFFU)
#define SIM_UIDL_UID31_0_SHIFT                   (0U)
#define SIM_UIDL_UID31_0_WIDTH                   (32U)
#define SIM_UIDL_UID31_0(x)                      (((uint32_t)(((uint32_t)(x)) << SIM_UIDL_UID31_0_SHIFT)) & SIM_UIDL_UID31_0_MASK)
/*! @} */

/*! @name CLKDIV4 - System Clock Divider Register 4 */
/*! @{ */

#define SIM_CLKDIV4_TRACEFRAC_MASK               (0x1U)
#define SIM_CLKDIV4_TRACEFRAC_SHIFT              (0U)
#define SIM_CLKDIV4_TRACEFRAC_WIDTH              (1U)
#define SIM_CLKDIV4_TRACEFRAC(x)                 (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV4_TRACEFRAC_SHIFT)) & SIM_CLKDIV4_TRACEFRAC_MASK)

#define SIM_CLKDIV4_TRACEDIV_MASK                (0xEU)
#define SIM_CLKDIV4_TRACEDIV_SHIFT               (1U)
#define SIM_CLKDIV4_TRACEDIV_WIDTH               (3U)
#define SIM_CLKDIV4_TRACEDIV(x)                  (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV4_TRACEDIV_SHIFT)) & SIM_CLKDIV4_TRACEDIV_MASK)

#define SIM_CLKDIV4_TRACEDIVEN_MASK              (0x10000000U)
#define SIM_CLKDIV4_TRACEDIVEN_SHIFT             (28U)
#define SIM_CLKDIV4_TRACEDIVEN_WIDTH             (1U)
#define SIM_CLKDIV4_TRACEDIVEN(x)                (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV4_TRACEDIVEN_SHIFT)) & SIM_CLKDIV4_TRACEDIVEN_MASK)
/*! @} */

/*! @name MISCTRL1 - Miscellaneous Control register 1 */
/*! @{ */

#define SIM_MISCTRL1_SW_TRG_MASK                 (0x1U)
#define SIM_MISCTRL1_SW_TRG_SHIFT                (0U)
#define SIM_MISCTRL1_SW_TRG_WIDTH                (1U)
#define SIM_MISCTRL1_SW_TRG(x)                   (((uint32_t)(((uint32_t)(x)) << SIM_MISCTRL1_SW_TRG_SHIFT)) & SIM_MISCTRL1_SW_TRG_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group SIM_Register_Masks */

/*!
 * @}
 */ /* end of group SIM_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_SIM_H_) */

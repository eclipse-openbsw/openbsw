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
 * @file S32K148_QuadSPI.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_QuadSPI
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
#if !defined(S32K148_QuadSPI_H_)  /* Check if memory map has not been already included */
#define S32K148_QuadSPI_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- QuadSPI Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup QuadSPI_Peripheral_Access_Layer QuadSPI Peripheral Access Layer
 * @{
 */

/** QuadSPI - Size of Registers Arrays */
#define QuadSPI_RBDR_COUNT                        32u
#define QuadSPI_LUT_COUNT                         64u

/** QuadSPI - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCR;                               /**< Module Configuration Register, offset: 0x0 */
  uint8_t RESERVED_0[4];
  __IO uint32_t IPCR;                              /**< IP Configuration Register, offset: 0x8 */
  __IO uint32_t FLSHCR;                            /**< Flash Configuration Register, offset: 0xC */
  __IO uint32_t BUF0CR;                            /**< Buffer0 Configuration Register, offset: 0x10 */
  __IO uint32_t BUF1CR;                            /**< Buffer1 Configuration Register, offset: 0x14 */
  __IO uint32_t BUF2CR;                            /**< Buffer2 Configuration Register, offset: 0x18 */
  __IO uint32_t BUF3CR;                            /**< Buffer3 Configuration Register, offset: 0x1C */
  __IO uint32_t BFGENCR;                           /**< Buffer Generic Configuration Register, offset: 0x20 */
  __IO uint32_t SOCCR;                             /**< SOC Configuration Register, offset: 0x24 */
  uint8_t RESERVED_1[8];
  __IO uint32_t BUF0IND;                           /**< Buffer0 Top Index Register, offset: 0x30 */
  __IO uint32_t BUF1IND;                           /**< Buffer1 Top Index Register, offset: 0x34 */
  __IO uint32_t BUF2IND;                           /**< Buffer2 Top Index Register, offset: 0x38 */
  uint8_t RESERVED_2[196];
  __IO uint32_t SFAR;                              /**< Serial Flash Address Register, offset: 0x100 */
  __IO uint32_t SFACR;                             /**< Serial Flash Address Configuration Register, offset: 0x104 */
  __IO uint32_t SMPR;                              /**< Sampling Register, offset: 0x108 */
  __I  uint32_t RBSR;                              /**< RX Buffer Status Register, offset: 0x10C */
  __IO uint32_t RBCT;                              /**< RX Buffer Control Register, offset: 0x110 */
  uint8_t RESERVED_3[60];
  __I  uint32_t TBSR;                              /**< TX Buffer Status Register, offset: 0x150 */
  __IO uint32_t TBDR;                              /**< TX Buffer Data Register, offset: 0x154 */
  __IO uint32_t TBCT;                              /**< Tx Buffer Control Register, offset: 0x158 */
  __I  uint32_t SR;                                /**< Status Register, offset: 0x15C */
  __IO uint32_t FR;                                /**< Flag Register, offset: 0x160 */
  __IO uint32_t RSER;                              /**< Interrupt and DMA Request Select and Enable Register, offset: 0x164 */
  __I  uint32_t SPNDST;                            /**< Sequence Suspend Status Register, offset: 0x168 */
  __O  uint32_t SPTRCLR;                           /**< Sequence Pointer Clear Register, offset: 0x16C */
  uint8_t RESERVED_4[16];
  __IO uint32_t SFA1AD;                            /**< Serial Flash A1 Top Address, offset: 0x180 */
  __IO uint32_t SFA2AD;                            /**< Serial Flash A2 Top Address, offset: 0x184 */
  __IO uint32_t SFB1AD;                            /**< Serial Flash B1 Top Address, offset: 0x188 */
  __IO uint32_t SFB2AD;                            /**< Serial Flash B2 Top Address, offset: 0x18C */
  uint8_t RESERVED_5[112];
  __I  uint32_t RBDR[QuadSPI_RBDR_COUNT];          /**< RX Buffer Data Register, array offset: 0x200, array step: 0x4 */
  uint8_t RESERVED_6[128];
  __IO uint32_t LUTKEY;                            /**< LUT Key Register, offset: 0x300 */
  __IO uint32_t LCKCR;                             /**< LUT Lock Configuration Register, offset: 0x304 */
  uint8_t RESERVED_7[8];
  __IO uint32_t LUT[QuadSPI_LUT_COUNT];            /**< Look-up Table register, array offset: 0x310, array step: 0x4 */
} QuadSPI_Type, *QuadSPI_MemMapPtr;

/** Number of instances of the QuadSPI module. */
#define QuadSPI_INSTANCE_COUNT                   (1u)

/* QuadSPI - Peripheral instance base addresses */
/** Peripheral QuadSPI base address */
#define IP_QuadSPI_BASE                          (0x40076000u)
/** Peripheral QuadSPI base pointer */
#define IP_QuadSPI                               ((QuadSPI_Type *)IP_QuadSPI_BASE)
/** Array initializer of QuadSPI peripheral base addresses */
#define IP_QuadSPI_BASE_ADDRS                    { IP_QuadSPI_BASE }
/** Array initializer of QuadSPI peripheral base pointers */
#define IP_QuadSPI_BASE_PTRS                     { IP_QuadSPI }

/* ----------------------------------------------------------------------------
   -- QuadSPI Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup QuadSPI_Register_Masks QuadSPI Register Masks
 * @{
 */

/*! @name MCR - Module Configuration Register */
/*! @{ */

#define QuadSPI_MCR_SWRSTSD_MASK                 (0x1U)
#define QuadSPI_MCR_SWRSTSD_SHIFT                (0U)
#define QuadSPI_MCR_SWRSTSD_WIDTH                (1U)
#define QuadSPI_MCR_SWRSTSD(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_SWRSTSD_SHIFT)) & QuadSPI_MCR_SWRSTSD_MASK)

#define QuadSPI_MCR_SWRSTHD_MASK                 (0x2U)
#define QuadSPI_MCR_SWRSTHD_SHIFT                (1U)
#define QuadSPI_MCR_SWRSTHD_WIDTH                (1U)
#define QuadSPI_MCR_SWRSTHD(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_SWRSTHD_SHIFT)) & QuadSPI_MCR_SWRSTHD_MASK)

#define QuadSPI_MCR_END_CFG_MASK                 (0xCU)
#define QuadSPI_MCR_END_CFG_SHIFT                (2U)
#define QuadSPI_MCR_END_CFG_WIDTH                (2U)
#define QuadSPI_MCR_END_CFG(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_END_CFG_SHIFT)) & QuadSPI_MCR_END_CFG_MASK)

#define QuadSPI_MCR_DQS_OUT_EN_MASK              (0x10U)
#define QuadSPI_MCR_DQS_OUT_EN_SHIFT             (4U)
#define QuadSPI_MCR_DQS_OUT_EN_WIDTH             (1U)
#define QuadSPI_MCR_DQS_OUT_EN(x)                (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_DQS_OUT_EN_SHIFT)) & QuadSPI_MCR_DQS_OUT_EN_MASK)

#define QuadSPI_MCR_DQS_LAT_EN_MASK              (0x20U)
#define QuadSPI_MCR_DQS_LAT_EN_SHIFT             (5U)
#define QuadSPI_MCR_DQS_LAT_EN_WIDTH             (1U)
#define QuadSPI_MCR_DQS_LAT_EN(x)                (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_DQS_LAT_EN_SHIFT)) & QuadSPI_MCR_DQS_LAT_EN_MASK)

#define QuadSPI_MCR_DQS_EN_MASK                  (0x40U)
#define QuadSPI_MCR_DQS_EN_SHIFT                 (6U)
#define QuadSPI_MCR_DQS_EN_WIDTH                 (1U)
#define QuadSPI_MCR_DQS_EN(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_DQS_EN_SHIFT)) & QuadSPI_MCR_DQS_EN_MASK)

#define QuadSPI_MCR_DDR_EN_MASK                  (0x80U)
#define QuadSPI_MCR_DDR_EN_SHIFT                 (7U)
#define QuadSPI_MCR_DDR_EN_WIDTH                 (1U)
#define QuadSPI_MCR_DDR_EN(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_DDR_EN_SHIFT)) & QuadSPI_MCR_DDR_EN_MASK)

#define QuadSPI_MCR_VAR_LAT_EN_MASK              (0x100U)
#define QuadSPI_MCR_VAR_LAT_EN_SHIFT             (8U)
#define QuadSPI_MCR_VAR_LAT_EN_WIDTH             (1U)
#define QuadSPI_MCR_VAR_LAT_EN(x)                (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_VAR_LAT_EN_SHIFT)) & QuadSPI_MCR_VAR_LAT_EN_MASK)

#define QuadSPI_MCR_CLR_RXF_MASK                 (0x400U)
#define QuadSPI_MCR_CLR_RXF_SHIFT                (10U)
#define QuadSPI_MCR_CLR_RXF_WIDTH                (1U)
#define QuadSPI_MCR_CLR_RXF(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_CLR_RXF_SHIFT)) & QuadSPI_MCR_CLR_RXF_MASK)

#define QuadSPI_MCR_CLR_TXF_MASK                 (0x800U)
#define QuadSPI_MCR_CLR_TXF_SHIFT                (11U)
#define QuadSPI_MCR_CLR_TXF_WIDTH                (1U)
#define QuadSPI_MCR_CLR_TXF(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_CLR_TXF_SHIFT)) & QuadSPI_MCR_CLR_TXF_MASK)

#define QuadSPI_MCR_MDIS_MASK                    (0x4000U)
#define QuadSPI_MCR_MDIS_SHIFT                   (14U)
#define QuadSPI_MCR_MDIS_WIDTH                   (1U)
#define QuadSPI_MCR_MDIS(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_MDIS_SHIFT)) & QuadSPI_MCR_MDIS_MASK)

#define QuadSPI_MCR_DOZE_MASK                    (0x8000U)
#define QuadSPI_MCR_DOZE_SHIFT                   (15U)
#define QuadSPI_MCR_DOZE_WIDTH                   (1U)
#define QuadSPI_MCR_DOZE(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_DOZE_SHIFT)) & QuadSPI_MCR_DOZE_MASK)

#define QuadSPI_MCR_ISD2FA_MASK                  (0x10000U)
#define QuadSPI_MCR_ISD2FA_SHIFT                 (16U)
#define QuadSPI_MCR_ISD2FA_WIDTH                 (1U)
#define QuadSPI_MCR_ISD2FA(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_ISD2FA_SHIFT)) & QuadSPI_MCR_ISD2FA_MASK)

#define QuadSPI_MCR_ISD3FA_MASK                  (0x20000U)
#define QuadSPI_MCR_ISD3FA_SHIFT                 (17U)
#define QuadSPI_MCR_ISD3FA_WIDTH                 (1U)
#define QuadSPI_MCR_ISD3FA(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_ISD3FA_SHIFT)) & QuadSPI_MCR_ISD3FA_MASK)

#define QuadSPI_MCR_ISD2FB_MASK                  (0x40000U)
#define QuadSPI_MCR_ISD2FB_SHIFT                 (18U)
#define QuadSPI_MCR_ISD2FB_WIDTH                 (1U)
#define QuadSPI_MCR_ISD2FB(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_ISD2FB_SHIFT)) & QuadSPI_MCR_ISD2FB_MASK)

#define QuadSPI_MCR_ISD3FB_MASK                  (0x80000U)
#define QuadSPI_MCR_ISD3FB_SHIFT                 (19U)
#define QuadSPI_MCR_ISD3FB_WIDTH                 (1U)
#define QuadSPI_MCR_ISD3FB(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_ISD3FB_SHIFT)) & QuadSPI_MCR_ISD3FB_MASK)

#define QuadSPI_MCR_SCLKCFG_MASK                 (0xFF000000U)
#define QuadSPI_MCR_SCLKCFG_SHIFT                (24U)
#define QuadSPI_MCR_SCLKCFG_WIDTH                (8U)
#define QuadSPI_MCR_SCLKCFG(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_MCR_SCLKCFG_SHIFT)) & QuadSPI_MCR_SCLKCFG_MASK)
/*! @} */

/*! @name IPCR - IP Configuration Register */
/*! @{ */

#define QuadSPI_IPCR_IDATSZ_MASK                 (0xFFFFU)
#define QuadSPI_IPCR_IDATSZ_SHIFT                (0U)
#define QuadSPI_IPCR_IDATSZ_WIDTH                (16U)
#define QuadSPI_IPCR_IDATSZ(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_IPCR_IDATSZ_SHIFT)) & QuadSPI_IPCR_IDATSZ_MASK)

#define QuadSPI_IPCR_SEQID_MASK                  (0xF000000U)
#define QuadSPI_IPCR_SEQID_SHIFT                 (24U)
#define QuadSPI_IPCR_SEQID_WIDTH                 (4U)
#define QuadSPI_IPCR_SEQID(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_IPCR_SEQID_SHIFT)) & QuadSPI_IPCR_SEQID_MASK)
/*! @} */

/*! @name FLSHCR - Flash Configuration Register */
/*! @{ */

#define QuadSPI_FLSHCR_TCSS_MASK                 (0xFU)
#define QuadSPI_FLSHCR_TCSS_SHIFT                (0U)
#define QuadSPI_FLSHCR_TCSS_WIDTH                (4U)
#define QuadSPI_FLSHCR_TCSS(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_FLSHCR_TCSS_SHIFT)) & QuadSPI_FLSHCR_TCSS_MASK)

#define QuadSPI_FLSHCR_TCSH_MASK                 (0xF00U)
#define QuadSPI_FLSHCR_TCSH_SHIFT                (8U)
#define QuadSPI_FLSHCR_TCSH_WIDTH                (4U)
#define QuadSPI_FLSHCR_TCSH(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_FLSHCR_TCSH_SHIFT)) & QuadSPI_FLSHCR_TCSH_MASK)

#define QuadSPI_FLSHCR_TDH_MASK                  (0x30000U)
#define QuadSPI_FLSHCR_TDH_SHIFT                 (16U)
#define QuadSPI_FLSHCR_TDH_WIDTH                 (2U)
#define QuadSPI_FLSHCR_TDH(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_FLSHCR_TDH_SHIFT)) & QuadSPI_FLSHCR_TDH_MASK)
/*! @} */

/*! @name BUF0CR - Buffer0 Configuration Register */
/*! @{ */

#define QuadSPI_BUF0CR_MSTRID_MASK               (0xFU)
#define QuadSPI_BUF0CR_MSTRID_SHIFT              (0U)
#define QuadSPI_BUF0CR_MSTRID_WIDTH              (4U)
#define QuadSPI_BUF0CR_MSTRID(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF0CR_MSTRID_SHIFT)) & QuadSPI_BUF0CR_MSTRID_MASK)

#define QuadSPI_BUF0CR_ADATSZ_MASK               (0xFF00U)
#define QuadSPI_BUF0CR_ADATSZ_SHIFT              (8U)
#define QuadSPI_BUF0CR_ADATSZ_WIDTH              (8U)
#define QuadSPI_BUF0CR_ADATSZ(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF0CR_ADATSZ_SHIFT)) & QuadSPI_BUF0CR_ADATSZ_MASK)

#define QuadSPI_BUF0CR_HP_EN_MASK                (0x80000000U)
#define QuadSPI_BUF0CR_HP_EN_SHIFT               (31U)
#define QuadSPI_BUF0CR_HP_EN_WIDTH               (1U)
#define QuadSPI_BUF0CR_HP_EN(x)                  (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF0CR_HP_EN_SHIFT)) & QuadSPI_BUF0CR_HP_EN_MASK)
/*! @} */

/*! @name BUF1CR - Buffer1 Configuration Register */
/*! @{ */

#define QuadSPI_BUF1CR_MSTRID_MASK               (0xFU)
#define QuadSPI_BUF1CR_MSTRID_SHIFT              (0U)
#define QuadSPI_BUF1CR_MSTRID_WIDTH              (4U)
#define QuadSPI_BUF1CR_MSTRID(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF1CR_MSTRID_SHIFT)) & QuadSPI_BUF1CR_MSTRID_MASK)

#define QuadSPI_BUF1CR_ADATSZ_MASK               (0xFF00U)
#define QuadSPI_BUF1CR_ADATSZ_SHIFT              (8U)
#define QuadSPI_BUF1CR_ADATSZ_WIDTH              (8U)
#define QuadSPI_BUF1CR_ADATSZ(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF1CR_ADATSZ_SHIFT)) & QuadSPI_BUF1CR_ADATSZ_MASK)
/*! @} */

/*! @name BUF2CR - Buffer2 Configuration Register */
/*! @{ */

#define QuadSPI_BUF2CR_MSTRID_MASK               (0xFU)
#define QuadSPI_BUF2CR_MSTRID_SHIFT              (0U)
#define QuadSPI_BUF2CR_MSTRID_WIDTH              (4U)
#define QuadSPI_BUF2CR_MSTRID(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF2CR_MSTRID_SHIFT)) & QuadSPI_BUF2CR_MSTRID_MASK)

#define QuadSPI_BUF2CR_ADATSZ_MASK               (0xFF00U)
#define QuadSPI_BUF2CR_ADATSZ_SHIFT              (8U)
#define QuadSPI_BUF2CR_ADATSZ_WIDTH              (8U)
#define QuadSPI_BUF2CR_ADATSZ(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF2CR_ADATSZ_SHIFT)) & QuadSPI_BUF2CR_ADATSZ_MASK)
/*! @} */

/*! @name BUF3CR - Buffer3 Configuration Register */
/*! @{ */

#define QuadSPI_BUF3CR_MSTRID_MASK               (0xFU)
#define QuadSPI_BUF3CR_MSTRID_SHIFT              (0U)
#define QuadSPI_BUF3CR_MSTRID_WIDTH              (4U)
#define QuadSPI_BUF3CR_MSTRID(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF3CR_MSTRID_SHIFT)) & QuadSPI_BUF3CR_MSTRID_MASK)

#define QuadSPI_BUF3CR_ADATSZ_MASK               (0xFF00U)
#define QuadSPI_BUF3CR_ADATSZ_SHIFT              (8U)
#define QuadSPI_BUF3CR_ADATSZ_WIDTH              (8U)
#define QuadSPI_BUF3CR_ADATSZ(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF3CR_ADATSZ_SHIFT)) & QuadSPI_BUF3CR_ADATSZ_MASK)

#define QuadSPI_BUF3CR_ALLMST_MASK               (0x80000000U)
#define QuadSPI_BUF3CR_ALLMST_SHIFT              (31U)
#define QuadSPI_BUF3CR_ALLMST_WIDTH              (1U)
#define QuadSPI_BUF3CR_ALLMST(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF3CR_ALLMST_SHIFT)) & QuadSPI_BUF3CR_ALLMST_MASK)
/*! @} */

/*! @name BFGENCR - Buffer Generic Configuration Register */
/*! @{ */

#define QuadSPI_BFGENCR_SEQID_MASK               (0xF000U)
#define QuadSPI_BFGENCR_SEQID_SHIFT              (12U)
#define QuadSPI_BFGENCR_SEQID_WIDTH              (4U)
#define QuadSPI_BFGENCR_SEQID(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_BFGENCR_SEQID_SHIFT)) & QuadSPI_BFGENCR_SEQID_MASK)
/*! @} */

/*! @name SOCCR - SOC Configuration Register */
/*! @{ */

#define QuadSPI_SOCCR_SOCCFG_MASK                (0xFFFFFFFFU)
#define QuadSPI_SOCCR_SOCCFG_SHIFT               (0U)
#define QuadSPI_SOCCR_SOCCFG_WIDTH               (32U)
#define QuadSPI_SOCCR_SOCCFG(x)                  (((uint32_t)(((uint32_t)(x)) << QuadSPI_SOCCR_SOCCFG_SHIFT)) & QuadSPI_SOCCR_SOCCFG_MASK)
/*! @} */

/*! @name BUF0IND - Buffer0 Top Index Register */
/*! @{ */

#define QuadSPI_BUF0IND_TPINDX0_MASK             (0xFFFFFFF8U)
#define QuadSPI_BUF0IND_TPINDX0_SHIFT            (3U)
#define QuadSPI_BUF0IND_TPINDX0_WIDTH            (29U)
#define QuadSPI_BUF0IND_TPINDX0(x)               (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF0IND_TPINDX0_SHIFT)) & QuadSPI_BUF0IND_TPINDX0_MASK)
/*! @} */

/*! @name BUF1IND - Buffer1 Top Index Register */
/*! @{ */

#define QuadSPI_BUF1IND_TPINDX1_MASK             (0xFFFFFFF8U)
#define QuadSPI_BUF1IND_TPINDX1_SHIFT            (3U)
#define QuadSPI_BUF1IND_TPINDX1_WIDTH            (29U)
#define QuadSPI_BUF1IND_TPINDX1(x)               (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF1IND_TPINDX1_SHIFT)) & QuadSPI_BUF1IND_TPINDX1_MASK)
/*! @} */

/*! @name BUF2IND - Buffer2 Top Index Register */
/*! @{ */

#define QuadSPI_BUF2IND_TPINDX2_MASK             (0xFFFFFFF8U)
#define QuadSPI_BUF2IND_TPINDX2_SHIFT            (3U)
#define QuadSPI_BUF2IND_TPINDX2_WIDTH            (29U)
#define QuadSPI_BUF2IND_TPINDX2(x)               (((uint32_t)(((uint32_t)(x)) << QuadSPI_BUF2IND_TPINDX2_SHIFT)) & QuadSPI_BUF2IND_TPINDX2_MASK)
/*! @} */

/*! @name SFAR - Serial Flash Address Register */
/*! @{ */

#define QuadSPI_SFAR_SFADR_MASK                  (0xFFFFFFFFU)
#define QuadSPI_SFAR_SFADR_SHIFT                 (0U)
#define QuadSPI_SFAR_SFADR_WIDTH                 (32U)
#define QuadSPI_SFAR_SFADR(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFAR_SFADR_SHIFT)) & QuadSPI_SFAR_SFADR_MASK)
/*! @} */

/*! @name SFACR - Serial Flash Address Configuration Register */
/*! @{ */

#define QuadSPI_SFACR_CAS_MASK                   (0xFU)
#define QuadSPI_SFACR_CAS_SHIFT                  (0U)
#define QuadSPI_SFACR_CAS_WIDTH                  (4U)
#define QuadSPI_SFACR_CAS(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFACR_CAS_SHIFT)) & QuadSPI_SFACR_CAS_MASK)

#define QuadSPI_SFACR_WA_MASK                    (0x10000U)
#define QuadSPI_SFACR_WA_SHIFT                   (16U)
#define QuadSPI_SFACR_WA_WIDTH                   (1U)
#define QuadSPI_SFACR_WA(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFACR_WA_SHIFT)) & QuadSPI_SFACR_WA_MASK)
/*! @} */

/*! @name SMPR - Sampling Register */
/*! @{ */

#define QuadSPI_SMPR_FSPHS_MASK                  (0x20U)
#define QuadSPI_SMPR_FSPHS_SHIFT                 (5U)
#define QuadSPI_SMPR_FSPHS_WIDTH                 (1U)
#define QuadSPI_SMPR_FSPHS(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SMPR_FSPHS_SHIFT)) & QuadSPI_SMPR_FSPHS_MASK)

#define QuadSPI_SMPR_FSDLY_MASK                  (0x40U)
#define QuadSPI_SMPR_FSDLY_SHIFT                 (6U)
#define QuadSPI_SMPR_FSDLY_WIDTH                 (1U)
#define QuadSPI_SMPR_FSDLY(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SMPR_FSDLY_SHIFT)) & QuadSPI_SMPR_FSDLY_MASK)
/*! @} */

/*! @name RBSR - RX Buffer Status Register */
/*! @{ */

#define QuadSPI_RBSR_RDBFL_MASK                  (0x3F00U)
#define QuadSPI_RBSR_RDBFL_SHIFT                 (8U)
#define QuadSPI_RBSR_RDBFL_WIDTH                 (6U)
#define QuadSPI_RBSR_RDBFL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RBSR_RDBFL_SHIFT)) & QuadSPI_RBSR_RDBFL_MASK)

#define QuadSPI_RBSR_RDCTR_MASK                  (0xFFFF0000U)
#define QuadSPI_RBSR_RDCTR_SHIFT                 (16U)
#define QuadSPI_RBSR_RDCTR_WIDTH                 (16U)
#define QuadSPI_RBSR_RDCTR(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RBSR_RDCTR_SHIFT)) & QuadSPI_RBSR_RDCTR_MASK)
/*! @} */

/*! @name RBCT - RX Buffer Control Register */
/*! @{ */

#define QuadSPI_RBCT_WMRK_MASK                   (0x1FU)
#define QuadSPI_RBCT_WMRK_SHIFT                  (0U)
#define QuadSPI_RBCT_WMRK_WIDTH                  (5U)
#define QuadSPI_RBCT_WMRK(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_RBCT_WMRK_SHIFT)) & QuadSPI_RBCT_WMRK_MASK)

#define QuadSPI_RBCT_RXBRD_MASK                  (0x100U)
#define QuadSPI_RBCT_RXBRD_SHIFT                 (8U)
#define QuadSPI_RBCT_RXBRD_WIDTH                 (1U)
#define QuadSPI_RBCT_RXBRD(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RBCT_RXBRD_SHIFT)) & QuadSPI_RBCT_RXBRD_MASK)
/*! @} */

/*! @name TBSR - TX Buffer Status Register */
/*! @{ */

#define QuadSPI_TBSR_TRBFL_MASK                  (0x3F00U)
#define QuadSPI_TBSR_TRBFL_SHIFT                 (8U)
#define QuadSPI_TBSR_TRBFL_WIDTH                 (6U)
#define QuadSPI_TBSR_TRBFL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_TBSR_TRBFL_SHIFT)) & QuadSPI_TBSR_TRBFL_MASK)

#define QuadSPI_TBSR_TRCTR_MASK                  (0xFFFF0000U)
#define QuadSPI_TBSR_TRCTR_SHIFT                 (16U)
#define QuadSPI_TBSR_TRCTR_WIDTH                 (16U)
#define QuadSPI_TBSR_TRCTR(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_TBSR_TRCTR_SHIFT)) & QuadSPI_TBSR_TRCTR_MASK)
/*! @} */

/*! @name TBDR - TX Buffer Data Register */
/*! @{ */

#define QuadSPI_TBDR_TXDATA_MASK                 (0xFFFFFFFFU)
#define QuadSPI_TBDR_TXDATA_SHIFT                (0U)
#define QuadSPI_TBDR_TXDATA_WIDTH                (32U)
#define QuadSPI_TBDR_TXDATA(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_TBDR_TXDATA_SHIFT)) & QuadSPI_TBDR_TXDATA_MASK)
/*! @} */

/*! @name TBCT - Tx Buffer Control Register */
/*! @{ */

#define QuadSPI_TBCT_WMRK_MASK                   (0x1FU)
#define QuadSPI_TBCT_WMRK_SHIFT                  (0U)
#define QuadSPI_TBCT_WMRK_WIDTH                  (5U)
#define QuadSPI_TBCT_WMRK(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_TBCT_WMRK_SHIFT)) & QuadSPI_TBCT_WMRK_MASK)
/*! @} */

/*! @name SR - Status Register */
/*! @{ */

#define QuadSPI_SR_BUSY_MASK                     (0x1U)
#define QuadSPI_SR_BUSY_SHIFT                    (0U)
#define QuadSPI_SR_BUSY_WIDTH                    (1U)
#define QuadSPI_SR_BUSY(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_BUSY_SHIFT)) & QuadSPI_SR_BUSY_MASK)

#define QuadSPI_SR_IP_ACC_MASK                   (0x2U)
#define QuadSPI_SR_IP_ACC_SHIFT                  (1U)
#define QuadSPI_SR_IP_ACC_WIDTH                  (1U)
#define QuadSPI_SR_IP_ACC(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_IP_ACC_SHIFT)) & QuadSPI_SR_IP_ACC_MASK)

#define QuadSPI_SR_AHB_ACC_MASK                  (0x4U)
#define QuadSPI_SR_AHB_ACC_SHIFT                 (2U)
#define QuadSPI_SR_AHB_ACC_WIDTH                 (1U)
#define QuadSPI_SR_AHB_ACC(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB_ACC_SHIFT)) & QuadSPI_SR_AHB_ACC_MASK)

#define QuadSPI_SR_AHBTRN_MASK                   (0x40U)
#define QuadSPI_SR_AHBTRN_SHIFT                  (6U)
#define QuadSPI_SR_AHBTRN_WIDTH                  (1U)
#define QuadSPI_SR_AHBTRN(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHBTRN_SHIFT)) & QuadSPI_SR_AHBTRN_MASK)

#define QuadSPI_SR_AHB0NE_MASK                   (0x80U)
#define QuadSPI_SR_AHB0NE_SHIFT                  (7U)
#define QuadSPI_SR_AHB0NE_WIDTH                  (1U)
#define QuadSPI_SR_AHB0NE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB0NE_SHIFT)) & QuadSPI_SR_AHB0NE_MASK)

#define QuadSPI_SR_AHB1NE_MASK                   (0x100U)
#define QuadSPI_SR_AHB1NE_SHIFT                  (8U)
#define QuadSPI_SR_AHB1NE_WIDTH                  (1U)
#define QuadSPI_SR_AHB1NE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB1NE_SHIFT)) & QuadSPI_SR_AHB1NE_MASK)

#define QuadSPI_SR_AHB2NE_MASK                   (0x200U)
#define QuadSPI_SR_AHB2NE_SHIFT                  (9U)
#define QuadSPI_SR_AHB2NE_WIDTH                  (1U)
#define QuadSPI_SR_AHB2NE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB2NE_SHIFT)) & QuadSPI_SR_AHB2NE_MASK)

#define QuadSPI_SR_AHB3NE_MASK                   (0x400U)
#define QuadSPI_SR_AHB3NE_SHIFT                  (10U)
#define QuadSPI_SR_AHB3NE_WIDTH                  (1U)
#define QuadSPI_SR_AHB3NE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB3NE_SHIFT)) & QuadSPI_SR_AHB3NE_MASK)

#define QuadSPI_SR_AHB0FUL_MASK                  (0x800U)
#define QuadSPI_SR_AHB0FUL_SHIFT                 (11U)
#define QuadSPI_SR_AHB0FUL_WIDTH                 (1U)
#define QuadSPI_SR_AHB0FUL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB0FUL_SHIFT)) & QuadSPI_SR_AHB0FUL_MASK)

#define QuadSPI_SR_AHB1FUL_MASK                  (0x1000U)
#define QuadSPI_SR_AHB1FUL_SHIFT                 (12U)
#define QuadSPI_SR_AHB1FUL_WIDTH                 (1U)
#define QuadSPI_SR_AHB1FUL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB1FUL_SHIFT)) & QuadSPI_SR_AHB1FUL_MASK)

#define QuadSPI_SR_AHB2FUL_MASK                  (0x2000U)
#define QuadSPI_SR_AHB2FUL_SHIFT                 (13U)
#define QuadSPI_SR_AHB2FUL_WIDTH                 (1U)
#define QuadSPI_SR_AHB2FUL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB2FUL_SHIFT)) & QuadSPI_SR_AHB2FUL_MASK)

#define QuadSPI_SR_AHB3FUL_MASK                  (0x4000U)
#define QuadSPI_SR_AHB3FUL_SHIFT                 (14U)
#define QuadSPI_SR_AHB3FUL_WIDTH                 (1U)
#define QuadSPI_SR_AHB3FUL(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_AHB3FUL_SHIFT)) & QuadSPI_SR_AHB3FUL_MASK)

#define QuadSPI_SR_RXWE_MASK                     (0x10000U)
#define QuadSPI_SR_RXWE_SHIFT                    (16U)
#define QuadSPI_SR_RXWE_WIDTH                    (1U)
#define QuadSPI_SR_RXWE(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_RXWE_SHIFT)) & QuadSPI_SR_RXWE_MASK)

#define QuadSPI_SR_RXFULL_MASK                   (0x80000U)
#define QuadSPI_SR_RXFULL_SHIFT                  (19U)
#define QuadSPI_SR_RXFULL_WIDTH                  (1U)
#define QuadSPI_SR_RXFULL(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_RXFULL_SHIFT)) & QuadSPI_SR_RXFULL_MASK)

#define QuadSPI_SR_RXDMA_MASK                    (0x800000U)
#define QuadSPI_SR_RXDMA_SHIFT                   (23U)
#define QuadSPI_SR_RXDMA_WIDTH                   (1U)
#define QuadSPI_SR_RXDMA(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_RXDMA_SHIFT)) & QuadSPI_SR_RXDMA_MASK)

#define QuadSPI_SR_TXEDA_MASK                    (0x1000000U)
#define QuadSPI_SR_TXEDA_SHIFT                   (24U)
#define QuadSPI_SR_TXEDA_WIDTH                   (1U)
#define QuadSPI_SR_TXEDA(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_TXEDA_SHIFT)) & QuadSPI_SR_TXEDA_MASK)

#define QuadSPI_SR_TXWA_MASK                     (0x2000000U)
#define QuadSPI_SR_TXWA_SHIFT                    (25U)
#define QuadSPI_SR_TXWA_WIDTH                    (1U)
#define QuadSPI_SR_TXWA(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_TXWA_SHIFT)) & QuadSPI_SR_TXWA_MASK)

#define QuadSPI_SR_TXDMA_MASK                    (0x4000000U)
#define QuadSPI_SR_TXDMA_SHIFT                   (26U)
#define QuadSPI_SR_TXDMA_WIDTH                   (1U)
#define QuadSPI_SR_TXDMA(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_TXDMA_SHIFT)) & QuadSPI_SR_TXDMA_MASK)

#define QuadSPI_SR_TXFULL_MASK                   (0x8000000U)
#define QuadSPI_SR_TXFULL_SHIFT                  (27U)
#define QuadSPI_SR_TXFULL_WIDTH                  (1U)
#define QuadSPI_SR_TXFULL(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_SR_TXFULL_SHIFT)) & QuadSPI_SR_TXFULL_MASK)
/*! @} */

/*! @name FR - Flag Register */
/*! @{ */

#define QuadSPI_FR_TFF_MASK                      (0x1U)
#define QuadSPI_FR_TFF_SHIFT                     (0U)
#define QuadSPI_FR_TFF_WIDTH                     (1U)
#define QuadSPI_FR_TFF(x)                        (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_TFF_SHIFT)) & QuadSPI_FR_TFF_MASK)

#define QuadSPI_FR_IPIEF_MASK                    (0x40U)
#define QuadSPI_FR_IPIEF_SHIFT                   (6U)
#define QuadSPI_FR_IPIEF_WIDTH                   (1U)
#define QuadSPI_FR_IPIEF(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_IPIEF_SHIFT)) & QuadSPI_FR_IPIEF_MASK)

#define QuadSPI_FR_IPAEF_MASK                    (0x80U)
#define QuadSPI_FR_IPAEF_SHIFT                   (7U)
#define QuadSPI_FR_IPAEF_WIDTH                   (1U)
#define QuadSPI_FR_IPAEF(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_IPAEF_SHIFT)) & QuadSPI_FR_IPAEF_MASK)

#define QuadSPI_FR_ABOF_MASK                     (0x1000U)
#define QuadSPI_FR_ABOF_SHIFT                    (12U)
#define QuadSPI_FR_ABOF_WIDTH                    (1U)
#define QuadSPI_FR_ABOF(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_ABOF_SHIFT)) & QuadSPI_FR_ABOF_MASK)

#define QuadSPI_FR_AIBSEF_MASK                   (0x2000U)
#define QuadSPI_FR_AIBSEF_SHIFT                  (13U)
#define QuadSPI_FR_AIBSEF_WIDTH                  (1U)
#define QuadSPI_FR_AIBSEF(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_AIBSEF_SHIFT)) & QuadSPI_FR_AIBSEF_MASK)

#define QuadSPI_FR_AITEF_MASK                    (0x4000U)
#define QuadSPI_FR_AITEF_SHIFT                   (14U)
#define QuadSPI_FR_AITEF_WIDTH                   (1U)
#define QuadSPI_FR_AITEF(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_AITEF_SHIFT)) & QuadSPI_FR_AITEF_MASK)

#define QuadSPI_FR_ABSEF_MASK                    (0x8000U)
#define QuadSPI_FR_ABSEF_SHIFT                   (15U)
#define QuadSPI_FR_ABSEF_WIDTH                   (1U)
#define QuadSPI_FR_ABSEF(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_ABSEF_SHIFT)) & QuadSPI_FR_ABSEF_MASK)

#define QuadSPI_FR_RBDF_MASK                     (0x10000U)
#define QuadSPI_FR_RBDF_SHIFT                    (16U)
#define QuadSPI_FR_RBDF_WIDTH                    (1U)
#define QuadSPI_FR_RBDF(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_RBDF_SHIFT)) & QuadSPI_FR_RBDF_MASK)

#define QuadSPI_FR_RBOF_MASK                     (0x20000U)
#define QuadSPI_FR_RBOF_SHIFT                    (17U)
#define QuadSPI_FR_RBOF_WIDTH                    (1U)
#define QuadSPI_FR_RBOF(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_RBOF_SHIFT)) & QuadSPI_FR_RBOF_MASK)

#define QuadSPI_FR_ILLINE_MASK                   (0x800000U)
#define QuadSPI_FR_ILLINE_SHIFT                  (23U)
#define QuadSPI_FR_ILLINE_WIDTH                  (1U)
#define QuadSPI_FR_ILLINE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_ILLINE_SHIFT)) & QuadSPI_FR_ILLINE_MASK)

#define QuadSPI_FR_TBUF_MASK                     (0x4000000U)
#define QuadSPI_FR_TBUF_SHIFT                    (26U)
#define QuadSPI_FR_TBUF_WIDTH                    (1U)
#define QuadSPI_FR_TBUF(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_TBUF_SHIFT)) & QuadSPI_FR_TBUF_MASK)

#define QuadSPI_FR_TBFF_MASK                     (0x8000000U)
#define QuadSPI_FR_TBFF_SHIFT                    (27U)
#define QuadSPI_FR_TBFF_WIDTH                    (1U)
#define QuadSPI_FR_TBFF(x)                       (((uint32_t)(((uint32_t)(x)) << QuadSPI_FR_TBFF_SHIFT)) & QuadSPI_FR_TBFF_MASK)
/*! @} */

/*! @name RSER - Interrupt and DMA Request Select and Enable Register */
/*! @{ */

#define QuadSPI_RSER_TFIE_MASK                   (0x1U)
#define QuadSPI_RSER_TFIE_SHIFT                  (0U)
#define QuadSPI_RSER_TFIE_WIDTH                  (1U)
#define QuadSPI_RSER_TFIE(x)                     (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_TFIE_SHIFT)) & QuadSPI_RSER_TFIE_MASK)

#define QuadSPI_RSER_IPIEIE_MASK                 (0x40U)
#define QuadSPI_RSER_IPIEIE_SHIFT                (6U)
#define QuadSPI_RSER_IPIEIE_WIDTH                (1U)
#define QuadSPI_RSER_IPIEIE(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_IPIEIE_SHIFT)) & QuadSPI_RSER_IPIEIE_MASK)

#define QuadSPI_RSER_IPAEIE_MASK                 (0x80U)
#define QuadSPI_RSER_IPAEIE_SHIFT                (7U)
#define QuadSPI_RSER_IPAEIE_WIDTH                (1U)
#define QuadSPI_RSER_IPAEIE(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_IPAEIE_SHIFT)) & QuadSPI_RSER_IPAEIE_MASK)

#define QuadSPI_RSER_ABOIE_MASK                  (0x1000U)
#define QuadSPI_RSER_ABOIE_SHIFT                 (12U)
#define QuadSPI_RSER_ABOIE_WIDTH                 (1U)
#define QuadSPI_RSER_ABOIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_ABOIE_SHIFT)) & QuadSPI_RSER_ABOIE_MASK)

#define QuadSPI_RSER_AIBSIE_MASK                 (0x2000U)
#define QuadSPI_RSER_AIBSIE_SHIFT                (13U)
#define QuadSPI_RSER_AIBSIE_WIDTH                (1U)
#define QuadSPI_RSER_AIBSIE(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_AIBSIE_SHIFT)) & QuadSPI_RSER_AIBSIE_MASK)

#define QuadSPI_RSER_AITIE_MASK                  (0x4000U)
#define QuadSPI_RSER_AITIE_SHIFT                 (14U)
#define QuadSPI_RSER_AITIE_WIDTH                 (1U)
#define QuadSPI_RSER_AITIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_AITIE_SHIFT)) & QuadSPI_RSER_AITIE_MASK)

#define QuadSPI_RSER_ABSEIE_MASK                 (0x8000U)
#define QuadSPI_RSER_ABSEIE_SHIFT                (15U)
#define QuadSPI_RSER_ABSEIE_WIDTH                (1U)
#define QuadSPI_RSER_ABSEIE(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_ABSEIE_SHIFT)) & QuadSPI_RSER_ABSEIE_MASK)

#define QuadSPI_RSER_RBDIE_MASK                  (0x10000U)
#define QuadSPI_RSER_RBDIE_SHIFT                 (16U)
#define QuadSPI_RSER_RBDIE_WIDTH                 (1U)
#define QuadSPI_RSER_RBDIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_RBDIE_SHIFT)) & QuadSPI_RSER_RBDIE_MASK)

#define QuadSPI_RSER_RBOIE_MASK                  (0x20000U)
#define QuadSPI_RSER_RBOIE_SHIFT                 (17U)
#define QuadSPI_RSER_RBOIE_WIDTH                 (1U)
#define QuadSPI_RSER_RBOIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_RBOIE_SHIFT)) & QuadSPI_RSER_RBOIE_MASK)

#define QuadSPI_RSER_RBDDE_MASK                  (0x200000U)
#define QuadSPI_RSER_RBDDE_SHIFT                 (21U)
#define QuadSPI_RSER_RBDDE_WIDTH                 (1U)
#define QuadSPI_RSER_RBDDE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_RBDDE_SHIFT)) & QuadSPI_RSER_RBDDE_MASK)

#define QuadSPI_RSER_ILLINIE_MASK                (0x800000U)
#define QuadSPI_RSER_ILLINIE_SHIFT               (23U)
#define QuadSPI_RSER_ILLINIE_WIDTH               (1U)
#define QuadSPI_RSER_ILLINIE(x)                  (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_ILLINIE_SHIFT)) & QuadSPI_RSER_ILLINIE_MASK)

#define QuadSPI_RSER_TBFDE_MASK                  (0x2000000U)
#define QuadSPI_RSER_TBFDE_SHIFT                 (25U)
#define QuadSPI_RSER_TBFDE_WIDTH                 (1U)
#define QuadSPI_RSER_TBFDE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_TBFDE_SHIFT)) & QuadSPI_RSER_TBFDE_MASK)

#define QuadSPI_RSER_TBUIE_MASK                  (0x4000000U)
#define QuadSPI_RSER_TBUIE_SHIFT                 (26U)
#define QuadSPI_RSER_TBUIE_WIDTH                 (1U)
#define QuadSPI_RSER_TBUIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_TBUIE_SHIFT)) & QuadSPI_RSER_TBUIE_MASK)

#define QuadSPI_RSER_TBFIE_MASK                  (0x8000000U)
#define QuadSPI_RSER_TBFIE_SHIFT                 (27U)
#define QuadSPI_RSER_TBFIE_WIDTH                 (1U)
#define QuadSPI_RSER_TBFIE(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_RSER_TBFIE_SHIFT)) & QuadSPI_RSER_TBFIE_MASK)
/*! @} */

/*! @name SPNDST - Sequence Suspend Status Register */
/*! @{ */

#define QuadSPI_SPNDST_SUSPND_MASK               (0x1U)
#define QuadSPI_SPNDST_SUSPND_SHIFT              (0U)
#define QuadSPI_SPNDST_SUSPND_WIDTH              (1U)
#define QuadSPI_SPNDST_SUSPND(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SPNDST_SUSPND_SHIFT)) & QuadSPI_SPNDST_SUSPND_MASK)

#define QuadSPI_SPNDST_SPDBUF_MASK               (0xC0U)
#define QuadSPI_SPNDST_SPDBUF_SHIFT              (6U)
#define QuadSPI_SPNDST_SPDBUF_WIDTH              (2U)
#define QuadSPI_SPNDST_SPDBUF(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SPNDST_SPDBUF_SHIFT)) & QuadSPI_SPNDST_SPDBUF_MASK)

#define QuadSPI_SPNDST_DATLFT_MASK               (0xFE00U)
#define QuadSPI_SPNDST_DATLFT_SHIFT              (9U)
#define QuadSPI_SPNDST_DATLFT_WIDTH              (7U)
#define QuadSPI_SPNDST_DATLFT(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SPNDST_DATLFT_SHIFT)) & QuadSPI_SPNDST_DATLFT_MASK)
/*! @} */

/*! @name SPTRCLR - Sequence Pointer Clear Register */
/*! @{ */

#define QuadSPI_SPTRCLR_BFPTRC_MASK              (0x1U)
#define QuadSPI_SPTRCLR_BFPTRC_SHIFT             (0U)
#define QuadSPI_SPTRCLR_BFPTRC_WIDTH             (1U)
#define QuadSPI_SPTRCLR_BFPTRC(x)                (((uint32_t)(((uint32_t)(x)) << QuadSPI_SPTRCLR_BFPTRC_SHIFT)) & QuadSPI_SPTRCLR_BFPTRC_MASK)

#define QuadSPI_SPTRCLR_IPPTRC_MASK              (0x100U)
#define QuadSPI_SPTRCLR_IPPTRC_SHIFT             (8U)
#define QuadSPI_SPTRCLR_IPPTRC_WIDTH             (1U)
#define QuadSPI_SPTRCLR_IPPTRC(x)                (((uint32_t)(((uint32_t)(x)) << QuadSPI_SPTRCLR_IPPTRC_SHIFT)) & QuadSPI_SPTRCLR_IPPTRC_MASK)
/*! @} */

/*! @name SFA1AD - Serial Flash A1 Top Address */
/*! @{ */

#define QuadSPI_SFA1AD_TPADA1_MASK               (0xFFFFFC00U)
#define QuadSPI_SFA1AD_TPADA1_SHIFT              (10U)
#define QuadSPI_SFA1AD_TPADA1_WIDTH              (22U)
#define QuadSPI_SFA1AD_TPADA1(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFA1AD_TPADA1_SHIFT)) & QuadSPI_SFA1AD_TPADA1_MASK)
/*! @} */

/*! @name SFA2AD - Serial Flash A2 Top Address */
/*! @{ */

#define QuadSPI_SFA2AD_TPADA2_MASK               (0xFFFFFC00U)
#define QuadSPI_SFA2AD_TPADA2_SHIFT              (10U)
#define QuadSPI_SFA2AD_TPADA2_WIDTH              (22U)
#define QuadSPI_SFA2AD_TPADA2(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFA2AD_TPADA2_SHIFT)) & QuadSPI_SFA2AD_TPADA2_MASK)
/*! @} */

/*! @name SFB1AD - Serial Flash B1 Top Address */
/*! @{ */

#define QuadSPI_SFB1AD_TPADB1_MASK               (0xFFFFFC00U)
#define QuadSPI_SFB1AD_TPADB1_SHIFT              (10U)
#define QuadSPI_SFB1AD_TPADB1_WIDTH              (22U)
#define QuadSPI_SFB1AD_TPADB1(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFB1AD_TPADB1_SHIFT)) & QuadSPI_SFB1AD_TPADB1_MASK)
/*! @} */

/*! @name SFB2AD - Serial Flash B2 Top Address */
/*! @{ */

#define QuadSPI_SFB2AD_TPADB2_MASK               (0xFFFFFC00U)
#define QuadSPI_SFB2AD_TPADB2_SHIFT              (10U)
#define QuadSPI_SFB2AD_TPADB2_WIDTH              (22U)
#define QuadSPI_SFB2AD_TPADB2(x)                 (((uint32_t)(((uint32_t)(x)) << QuadSPI_SFB2AD_TPADB2_SHIFT)) & QuadSPI_SFB2AD_TPADB2_MASK)
/*! @} */

/*! @name RBDR - RX Buffer Data Register */
/*! @{ */

#define QuadSPI_RBDR_RXDATA_MASK                 (0xFFFFFFFFU)
#define QuadSPI_RBDR_RXDATA_SHIFT                (0U)
#define QuadSPI_RBDR_RXDATA_WIDTH                (32U)
#define QuadSPI_RBDR_RXDATA(x)                   (((uint32_t)(((uint32_t)(x)) << QuadSPI_RBDR_RXDATA_SHIFT)) & QuadSPI_RBDR_RXDATA_MASK)
/*! @} */

/*! @name LUTKEY - LUT Key Register */
/*! @{ */

#define QuadSPI_LUTKEY_KEY_MASK                  (0xFFFFFFFFU)
#define QuadSPI_LUTKEY_KEY_SHIFT                 (0U)
#define QuadSPI_LUTKEY_KEY_WIDTH                 (32U)
#define QuadSPI_LUTKEY_KEY(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUTKEY_KEY_SHIFT)) & QuadSPI_LUTKEY_KEY_MASK)
/*! @} */

/*! @name LCKCR - LUT Lock Configuration Register */
/*! @{ */

#define QuadSPI_LCKCR_LOCK_MASK                  (0x1U)
#define QuadSPI_LCKCR_LOCK_SHIFT                 (0U)
#define QuadSPI_LCKCR_LOCK_WIDTH                 (1U)
#define QuadSPI_LCKCR_LOCK(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LCKCR_LOCK_SHIFT)) & QuadSPI_LCKCR_LOCK_MASK)

#define QuadSPI_LCKCR_UNLOCK_MASK                (0x2U)
#define QuadSPI_LCKCR_UNLOCK_SHIFT               (1U)
#define QuadSPI_LCKCR_UNLOCK_WIDTH               (1U)
#define QuadSPI_LCKCR_UNLOCK(x)                  (((uint32_t)(((uint32_t)(x)) << QuadSPI_LCKCR_UNLOCK_SHIFT)) & QuadSPI_LCKCR_UNLOCK_MASK)
/*! @} */

/*! @name LUT - Look-up Table register */
/*! @{ */

#define QuadSPI_LUT_OPRND0_MASK                  (0xFFU)
#define QuadSPI_LUT_OPRND0_SHIFT                 (0U)
#define QuadSPI_LUT_OPRND0_WIDTH                 (8U)
#define QuadSPI_LUT_OPRND0(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_OPRND0_SHIFT)) & QuadSPI_LUT_OPRND0_MASK)

#define QuadSPI_LUT_PAD0_MASK                    (0x300U)
#define QuadSPI_LUT_PAD0_SHIFT                   (8U)
#define QuadSPI_LUT_PAD0_WIDTH                   (2U)
#define QuadSPI_LUT_PAD0(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_PAD0_SHIFT)) & QuadSPI_LUT_PAD0_MASK)

#define QuadSPI_LUT_INSTR0_MASK                  (0xFC00U)
#define QuadSPI_LUT_INSTR0_SHIFT                 (10U)
#define QuadSPI_LUT_INSTR0_WIDTH                 (6U)
#define QuadSPI_LUT_INSTR0(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_INSTR0_SHIFT)) & QuadSPI_LUT_INSTR0_MASK)

#define QuadSPI_LUT_OPRND1_MASK                  (0xFF0000U)
#define QuadSPI_LUT_OPRND1_SHIFT                 (16U)
#define QuadSPI_LUT_OPRND1_WIDTH                 (8U)
#define QuadSPI_LUT_OPRND1(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_OPRND1_SHIFT)) & QuadSPI_LUT_OPRND1_MASK)

#define QuadSPI_LUT_PAD1_MASK                    (0x3000000U)
#define QuadSPI_LUT_PAD1_SHIFT                   (24U)
#define QuadSPI_LUT_PAD1_WIDTH                   (2U)
#define QuadSPI_LUT_PAD1(x)                      (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_PAD1_SHIFT)) & QuadSPI_LUT_PAD1_MASK)

#define QuadSPI_LUT_INSTR1_MASK                  (0xFC000000U)
#define QuadSPI_LUT_INSTR1_SHIFT                 (26U)
#define QuadSPI_LUT_INSTR1_WIDTH                 (6U)
#define QuadSPI_LUT_INSTR1(x)                    (((uint32_t)(((uint32_t)(x)) << QuadSPI_LUT_INSTR1_SHIFT)) & QuadSPI_LUT_INSTR1_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group QuadSPI_Register_Masks */

/*!
 * @}
 */ /* end of group QuadSPI_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_QuadSPI_H_) */

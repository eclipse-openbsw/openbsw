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
 * @file S32K148_FLEXCAN.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_FLEXCAN
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
#if !defined(S32K148_FLEXCAN_H_)  /* Check if memory map has not been already included */
#define S32K148_FLEXCAN_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- FLEXCAN Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FLEXCAN_Peripheral_Access_Layer FLEXCAN Peripheral Access Layer
 * @{
 */

/** FLEXCAN - Size of Registers Arrays */
#define FLEXCAN_RAMn_COUNT                        128u
#define FLEXCAN_RXIMR_COUNT                       32u
#define FLEXCAN_WMB_COUNT                         4u

/** FLEXCAN - Register Layout Typedef */
typedef struct {
  __IO uint32_t MCR;                               /**< Module Configuration Register, offset: 0x0 */
  __IO uint32_t CTRL1;                             /**< Control 1 Register, offset: 0x4 */
  __IO uint32_t TIMER;                             /**< Free Running Timer, offset: 0x8 */
  uint8_t RESERVED_0[4];
  __IO uint32_t RXMGMASK;                          /**< Rx Mailboxes Global Mask Register, offset: 0x10 */
  __IO uint32_t RX14MASK;                          /**< Rx 14 Mask Register, offset: 0x14 */
  __IO uint32_t RX15MASK;                          /**< Rx 15 Mask Register, offset: 0x18 */
  __IO uint32_t ECR;                               /**< Error Counter, offset: 0x1C */
  __IO uint32_t ESR1;                              /**< Error and Status 1 Register, offset: 0x20 */
  uint8_t RESERVED_1[4];
  __IO uint32_t IMASK1;                            /**< Interrupt Masks 1 Register, offset: 0x28 */
  uint8_t RESERVED_2[4];
  __IO uint32_t IFLAG1;                            /**< Interrupt Flags 1 Register, offset: 0x30 */
  __IO uint32_t CTRL2;                             /**< Control 2 Register, offset: 0x34 */
  __I  uint32_t ESR2;                              /**< Error and Status 2 Register, offset: 0x38 */
  uint8_t RESERVED_3[8];
  __I  uint32_t CRCR;                              /**< CRC Register, offset: 0x44 */
  __IO uint32_t RXFGMASK;                          /**< Rx FIFO Global Mask Register, offset: 0x48 */
  __I  uint32_t RXFIR;                             /**< Rx FIFO Information Register, offset: 0x4C */
  __IO uint32_t CBT;                               /**< CAN Bit Timing Register, offset: 0x50 */
  uint8_t RESERVED_4[44];
  __IO uint32_t RAMn[FLEXCAN_RAMn_COUNT];          /**< Embedded RAM, array offset: 0x80, array step: 0x4 */
  uint8_t RESERVED_5[1536];
  __IO uint32_t RXIMR[FLEXCAN_RXIMR_COUNT];        /**< Rx Individual Mask Registers, array offset: 0x880, array step: 0x4 */
  uint8_t RESERVED_6[512];
  __IO uint32_t CTRL1_PN;                          /**< Pretended Networking Control 1 Register, offset: 0xB00 */
  __IO uint32_t CTRL2_PN;                          /**< Pretended Networking Control 2 Register, offset: 0xB04 */
  __IO uint32_t WU_MTC;                            /**< Pretended Networking Wake Up Match Register, offset: 0xB08 */
  __IO uint32_t FLT_ID1;                           /**< Pretended Networking ID Filter 1 Register, offset: 0xB0C */
  __IO uint32_t FLT_DLC;                           /**< Pretended Networking DLC Filter Register, offset: 0xB10 */
  __IO uint32_t PL1_LO;                            /**< Pretended Networking Payload Low Filter 1 Register, offset: 0xB14 */
  __IO uint32_t PL1_HI;                            /**< Pretended Networking Payload High Filter 1 Register, offset: 0xB18 */
  __IO uint32_t FLT_ID2_IDMASK;                    /**< Pretended Networking ID Filter 2 Register / ID Mask Register, offset: 0xB1C */
  __IO uint32_t PL2_PLMASK_LO;                     /**< Pretended Networking Payload Low Filter 2 Register / Payload Low Mask register, offset: 0xB20 */
  __IO uint32_t PL2_PLMASK_HI;                     /**< Pretended Networking Payload High Filter 2 low order bits / Payload High Mask register, offset: 0xB24 */
  uint8_t RESERVED_7[24];
  struct {                                         /* offset: 0xB40, array step: 0x10 */
    __I  uint32_t WMBn_CS;                           /**< Wake Up Message Buffer register for C/S, array offset: 0xB40, array step: 0x10 */
    __I  uint32_t WMBn_ID;                           /**< Wake Up Message Buffer Register for ID, array offset: 0xB44, array step: 0x10 */
    __I  uint32_t WMBn_D03;                          /**< Wake Up Message Buffer Register for Data 0-3, array offset: 0xB48, array step: 0x10 */
    __I  uint32_t WMBn_D47;                          /**< Wake Up Message Buffer Register Data 4-7, array offset: 0xB4C, array step: 0x10 */
  } WMB[FLEXCAN_WMB_COUNT];
  uint8_t RESERVED_8[128];
  __IO uint32_t FDCTRL;                            /**< CAN FD Control Register, offset: 0xC00 */
  __IO uint32_t FDCBT;                             /**< CAN FD Bit Timing Register, offset: 0xC04 */
  __I  uint32_t FDCRC;                             /**< CAN FD CRC Register, offset: 0xC08 */
} FLEXCAN_Type, *FLEXCAN_MemMapPtr;

/** Number of instances of the FLEXCAN module. */
#define FLEXCAN_INSTANCE_COUNT                   (3u)

/* FLEXCAN - Peripheral instance base addresses */
/** Peripheral FLEXCAN0 base address */
#define IP_FLEXCAN0_BASE                         (0x40024000u)
/** Peripheral FLEXCAN0 base pointer */
#define IP_FLEXCAN0                              ((FLEXCAN_Type *)IP_FLEXCAN0_BASE)
/** Peripheral FLEXCAN1 base address */
#define IP_FLEXCAN1_BASE                         (0x40025000u)
/** Peripheral FLEXCAN1 base pointer */
#define IP_FLEXCAN1                              ((FLEXCAN_Type *)IP_FLEXCAN1_BASE)
/** Peripheral FLEXCAN2 base address */
#define IP_FLEXCAN2_BASE                         (0x4002B000u)
/** Peripheral FLEXCAN2 base pointer */
#define IP_FLEXCAN2                              ((FLEXCAN_Type *)IP_FLEXCAN2_BASE)
/** Array initializer of FLEXCAN peripheral base addresses */
#define IP_FLEXCAN_BASE_ADDRS                    { IP_FLEXCAN0_BASE, IP_FLEXCAN1_BASE, IP_FLEXCAN2_BASE }
/** Array initializer of FLEXCAN peripheral base pointers */
#define IP_FLEXCAN_BASE_PTRS                     { IP_FLEXCAN0, IP_FLEXCAN1, IP_FLEXCAN2 }

/* ----------------------------------------------------------------------------
   -- FLEXCAN Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FLEXCAN_Register_Masks FLEXCAN Register Masks
 * @{
 */

/*! @name MCR - Module Configuration Register */
/*! @{ */

#define FLEXCAN_MCR_MAXMB_MASK                   (0x7FU)
#define FLEXCAN_MCR_MAXMB_SHIFT                  (0U)
#define FLEXCAN_MCR_MAXMB_WIDTH                  (7U)
#define FLEXCAN_MCR_MAXMB(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_MAXMB_SHIFT)) & FLEXCAN_MCR_MAXMB_MASK)

#define FLEXCAN_MCR_IDAM_MASK                    (0x300U)
#define FLEXCAN_MCR_IDAM_SHIFT                   (8U)
#define FLEXCAN_MCR_IDAM_WIDTH                   (2U)
#define FLEXCAN_MCR_IDAM(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_IDAM_SHIFT)) & FLEXCAN_MCR_IDAM_MASK)

#define FLEXCAN_MCR_FDEN_MASK                    (0x800U)
#define FLEXCAN_MCR_FDEN_SHIFT                   (11U)
#define FLEXCAN_MCR_FDEN_WIDTH                   (1U)
#define FLEXCAN_MCR_FDEN(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_FDEN_SHIFT)) & FLEXCAN_MCR_FDEN_MASK)

#define FLEXCAN_MCR_AEN_MASK                     (0x1000U)
#define FLEXCAN_MCR_AEN_SHIFT                    (12U)
#define FLEXCAN_MCR_AEN_WIDTH                    (1U)
#define FLEXCAN_MCR_AEN(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_AEN_SHIFT)) & FLEXCAN_MCR_AEN_MASK)

#define FLEXCAN_MCR_LPRIOEN_MASK                 (0x2000U)
#define FLEXCAN_MCR_LPRIOEN_SHIFT                (13U)
#define FLEXCAN_MCR_LPRIOEN_WIDTH                (1U)
#define FLEXCAN_MCR_LPRIOEN(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_LPRIOEN_SHIFT)) & FLEXCAN_MCR_LPRIOEN_MASK)

#define FLEXCAN_MCR_PNET_EN_MASK                 (0x4000U)
#define FLEXCAN_MCR_PNET_EN_SHIFT                (14U)
#define FLEXCAN_MCR_PNET_EN_WIDTH                (1U)
#define FLEXCAN_MCR_PNET_EN(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_PNET_EN_SHIFT)) & FLEXCAN_MCR_PNET_EN_MASK)

#define FLEXCAN_MCR_DMA_MASK                     (0x8000U)
#define FLEXCAN_MCR_DMA_SHIFT                    (15U)
#define FLEXCAN_MCR_DMA_WIDTH                    (1U)
#define FLEXCAN_MCR_DMA(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_DMA_SHIFT)) & FLEXCAN_MCR_DMA_MASK)

#define FLEXCAN_MCR_IRMQ_MASK                    (0x10000U)
#define FLEXCAN_MCR_IRMQ_SHIFT                   (16U)
#define FLEXCAN_MCR_IRMQ_WIDTH                   (1U)
#define FLEXCAN_MCR_IRMQ(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_IRMQ_SHIFT)) & FLEXCAN_MCR_IRMQ_MASK)

#define FLEXCAN_MCR_SRXDIS_MASK                  (0x20000U)
#define FLEXCAN_MCR_SRXDIS_SHIFT                 (17U)
#define FLEXCAN_MCR_SRXDIS_WIDTH                 (1U)
#define FLEXCAN_MCR_SRXDIS(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_SRXDIS_SHIFT)) & FLEXCAN_MCR_SRXDIS_MASK)

#define FLEXCAN_MCR_LPMACK_MASK                  (0x100000U)
#define FLEXCAN_MCR_LPMACK_SHIFT                 (20U)
#define FLEXCAN_MCR_LPMACK_WIDTH                 (1U)
#define FLEXCAN_MCR_LPMACK(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_LPMACK_SHIFT)) & FLEXCAN_MCR_LPMACK_MASK)

#define FLEXCAN_MCR_WRNEN_MASK                   (0x200000U)
#define FLEXCAN_MCR_WRNEN_SHIFT                  (21U)
#define FLEXCAN_MCR_WRNEN_WIDTH                  (1U)
#define FLEXCAN_MCR_WRNEN(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_WRNEN_SHIFT)) & FLEXCAN_MCR_WRNEN_MASK)

#define FLEXCAN_MCR_SUPV_MASK                    (0x800000U)
#define FLEXCAN_MCR_SUPV_SHIFT                   (23U)
#define FLEXCAN_MCR_SUPV_WIDTH                   (1U)
#define FLEXCAN_MCR_SUPV(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_SUPV_SHIFT)) & FLEXCAN_MCR_SUPV_MASK)

#define FLEXCAN_MCR_FRZACK_MASK                  (0x1000000U)
#define FLEXCAN_MCR_FRZACK_SHIFT                 (24U)
#define FLEXCAN_MCR_FRZACK_WIDTH                 (1U)
#define FLEXCAN_MCR_FRZACK(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_FRZACK_SHIFT)) & FLEXCAN_MCR_FRZACK_MASK)

#define FLEXCAN_MCR_SOFTRST_MASK                 (0x2000000U)
#define FLEXCAN_MCR_SOFTRST_SHIFT                (25U)
#define FLEXCAN_MCR_SOFTRST_WIDTH                (1U)
#define FLEXCAN_MCR_SOFTRST(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_SOFTRST_SHIFT)) & FLEXCAN_MCR_SOFTRST_MASK)

#define FLEXCAN_MCR_NOTRDY_MASK                  (0x8000000U)
#define FLEXCAN_MCR_NOTRDY_SHIFT                 (27U)
#define FLEXCAN_MCR_NOTRDY_WIDTH                 (1U)
#define FLEXCAN_MCR_NOTRDY(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_NOTRDY_SHIFT)) & FLEXCAN_MCR_NOTRDY_MASK)

#define FLEXCAN_MCR_HALT_MASK                    (0x10000000U)
#define FLEXCAN_MCR_HALT_SHIFT                   (28U)
#define FLEXCAN_MCR_HALT_WIDTH                   (1U)
#define FLEXCAN_MCR_HALT(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_HALT_SHIFT)) & FLEXCAN_MCR_HALT_MASK)

#define FLEXCAN_MCR_RFEN_MASK                    (0x20000000U)
#define FLEXCAN_MCR_RFEN_SHIFT                   (29U)
#define FLEXCAN_MCR_RFEN_WIDTH                   (1U)
#define FLEXCAN_MCR_RFEN(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_RFEN_SHIFT)) & FLEXCAN_MCR_RFEN_MASK)

#define FLEXCAN_MCR_FRZ_MASK                     (0x40000000U)
#define FLEXCAN_MCR_FRZ_SHIFT                    (30U)
#define FLEXCAN_MCR_FRZ_WIDTH                    (1U)
#define FLEXCAN_MCR_FRZ(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_FRZ_SHIFT)) & FLEXCAN_MCR_FRZ_MASK)

#define FLEXCAN_MCR_MDIS_MASK                    (0x80000000U)
#define FLEXCAN_MCR_MDIS_SHIFT                   (31U)
#define FLEXCAN_MCR_MDIS_WIDTH                   (1U)
#define FLEXCAN_MCR_MDIS(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_MCR_MDIS_SHIFT)) & FLEXCAN_MCR_MDIS_MASK)
/*! @} */

/*! @name CTRL1 - Control 1 Register */
/*! @{ */

#define FLEXCAN_CTRL1_PROPSEG_MASK               (0x7U)
#define FLEXCAN_CTRL1_PROPSEG_SHIFT              (0U)
#define FLEXCAN_CTRL1_PROPSEG_WIDTH              (3U)
#define FLEXCAN_CTRL1_PROPSEG(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PROPSEG_SHIFT)) & FLEXCAN_CTRL1_PROPSEG_MASK)

#define FLEXCAN_CTRL1_LOM_MASK                   (0x8U)
#define FLEXCAN_CTRL1_LOM_SHIFT                  (3U)
#define FLEXCAN_CTRL1_LOM_WIDTH                  (1U)
#define FLEXCAN_CTRL1_LOM(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_LOM_SHIFT)) & FLEXCAN_CTRL1_LOM_MASK)

#define FLEXCAN_CTRL1_LBUF_MASK                  (0x10U)
#define FLEXCAN_CTRL1_LBUF_SHIFT                 (4U)
#define FLEXCAN_CTRL1_LBUF_WIDTH                 (1U)
#define FLEXCAN_CTRL1_LBUF(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_LBUF_SHIFT)) & FLEXCAN_CTRL1_LBUF_MASK)

#define FLEXCAN_CTRL1_TSYN_MASK                  (0x20U)
#define FLEXCAN_CTRL1_TSYN_SHIFT                 (5U)
#define FLEXCAN_CTRL1_TSYN_WIDTH                 (1U)
#define FLEXCAN_CTRL1_TSYN(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_TSYN_SHIFT)) & FLEXCAN_CTRL1_TSYN_MASK)

#define FLEXCAN_CTRL1_BOFFREC_MASK               (0x40U)
#define FLEXCAN_CTRL1_BOFFREC_SHIFT              (6U)
#define FLEXCAN_CTRL1_BOFFREC_WIDTH              (1U)
#define FLEXCAN_CTRL1_BOFFREC(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_BOFFREC_SHIFT)) & FLEXCAN_CTRL1_BOFFREC_MASK)

#define FLEXCAN_CTRL1_SMP_MASK                   (0x80U)
#define FLEXCAN_CTRL1_SMP_SHIFT                  (7U)
#define FLEXCAN_CTRL1_SMP_WIDTH                  (1U)
#define FLEXCAN_CTRL1_SMP(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_SMP_SHIFT)) & FLEXCAN_CTRL1_SMP_MASK)

#define FLEXCAN_CTRL1_RWRNMSK_MASK               (0x400U)
#define FLEXCAN_CTRL1_RWRNMSK_SHIFT              (10U)
#define FLEXCAN_CTRL1_RWRNMSK_WIDTH              (1U)
#define FLEXCAN_CTRL1_RWRNMSK(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_RWRNMSK_SHIFT)) & FLEXCAN_CTRL1_RWRNMSK_MASK)

#define FLEXCAN_CTRL1_TWRNMSK_MASK               (0x800U)
#define FLEXCAN_CTRL1_TWRNMSK_SHIFT              (11U)
#define FLEXCAN_CTRL1_TWRNMSK_WIDTH              (1U)
#define FLEXCAN_CTRL1_TWRNMSK(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_TWRNMSK_SHIFT)) & FLEXCAN_CTRL1_TWRNMSK_MASK)

#define FLEXCAN_CTRL1_LPB_MASK                   (0x1000U)
#define FLEXCAN_CTRL1_LPB_SHIFT                  (12U)
#define FLEXCAN_CTRL1_LPB_WIDTH                  (1U)
#define FLEXCAN_CTRL1_LPB(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_LPB_SHIFT)) & FLEXCAN_CTRL1_LPB_MASK)

#define FLEXCAN_CTRL1_CLKSRC_MASK                (0x2000U)
#define FLEXCAN_CTRL1_CLKSRC_SHIFT               (13U)
#define FLEXCAN_CTRL1_CLKSRC_WIDTH               (1U)
#define FLEXCAN_CTRL1_CLKSRC(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_CLKSRC_SHIFT)) & FLEXCAN_CTRL1_CLKSRC_MASK)

#define FLEXCAN_CTRL1_ERRMSK_MASK                (0x4000U)
#define FLEXCAN_CTRL1_ERRMSK_SHIFT               (14U)
#define FLEXCAN_CTRL1_ERRMSK_WIDTH               (1U)
#define FLEXCAN_CTRL1_ERRMSK(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_ERRMSK_SHIFT)) & FLEXCAN_CTRL1_ERRMSK_MASK)

#define FLEXCAN_CTRL1_BOFFMSK_MASK               (0x8000U)
#define FLEXCAN_CTRL1_BOFFMSK_SHIFT              (15U)
#define FLEXCAN_CTRL1_BOFFMSK_WIDTH              (1U)
#define FLEXCAN_CTRL1_BOFFMSK(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_BOFFMSK_SHIFT)) & FLEXCAN_CTRL1_BOFFMSK_MASK)

#define FLEXCAN_CTRL1_PSEG2_MASK                 (0x70000U)
#define FLEXCAN_CTRL1_PSEG2_SHIFT                (16U)
#define FLEXCAN_CTRL1_PSEG2_WIDTH                (3U)
#define FLEXCAN_CTRL1_PSEG2(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PSEG2_SHIFT)) & FLEXCAN_CTRL1_PSEG2_MASK)

#define FLEXCAN_CTRL1_PSEG1_MASK                 (0x380000U)
#define FLEXCAN_CTRL1_PSEG1_SHIFT                (19U)
#define FLEXCAN_CTRL1_PSEG1_WIDTH                (3U)
#define FLEXCAN_CTRL1_PSEG1(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PSEG1_SHIFT)) & FLEXCAN_CTRL1_PSEG1_MASK)

#define FLEXCAN_CTRL1_RJW_MASK                   (0xC00000U)
#define FLEXCAN_CTRL1_RJW_SHIFT                  (22U)
#define FLEXCAN_CTRL1_RJW_WIDTH                  (2U)
#define FLEXCAN_CTRL1_RJW(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_RJW_SHIFT)) & FLEXCAN_CTRL1_RJW_MASK)

#define FLEXCAN_CTRL1_PRESDIV_MASK               (0xFF000000U)
#define FLEXCAN_CTRL1_PRESDIV_SHIFT              (24U)
#define FLEXCAN_CTRL1_PRESDIV_WIDTH              (8U)
#define FLEXCAN_CTRL1_PRESDIV(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PRESDIV_SHIFT)) & FLEXCAN_CTRL1_PRESDIV_MASK)
/*! @} */

/*! @name TIMER - Free Running Timer */
/*! @{ */

#define FLEXCAN_TIMER_TIMER_MASK                 (0xFFFFU)
#define FLEXCAN_TIMER_TIMER_SHIFT                (0U)
#define FLEXCAN_TIMER_TIMER_WIDTH                (16U)
#define FLEXCAN_TIMER_TIMER(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_TIMER_TIMER_SHIFT)) & FLEXCAN_TIMER_TIMER_MASK)
/*! @} */

/*! @name RXMGMASK - Rx Mailboxes Global Mask Register */
/*! @{ */

#define FLEXCAN_RXMGMASK_MG_MASK                 (0xFFFFFFFFU)
#define FLEXCAN_RXMGMASK_MG_SHIFT                (0U)
#define FLEXCAN_RXMGMASK_MG_WIDTH                (32U)
#define FLEXCAN_RXMGMASK_MG(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RXMGMASK_MG_SHIFT)) & FLEXCAN_RXMGMASK_MG_MASK)
/*! @} */

/*! @name RX14MASK - Rx 14 Mask Register */
/*! @{ */

#define FLEXCAN_RX14MASK_RX14M_MASK              (0xFFFFFFFFU)
#define FLEXCAN_RX14MASK_RX14M_SHIFT             (0U)
#define FLEXCAN_RX14MASK_RX14M_WIDTH             (32U)
#define FLEXCAN_RX14MASK_RX14M(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RX14MASK_RX14M_SHIFT)) & FLEXCAN_RX14MASK_RX14M_MASK)
/*! @} */

/*! @name RX15MASK - Rx 15 Mask Register */
/*! @{ */

#define FLEXCAN_RX15MASK_RX15M_MASK              (0xFFFFFFFFU)
#define FLEXCAN_RX15MASK_RX15M_SHIFT             (0U)
#define FLEXCAN_RX15MASK_RX15M_WIDTH             (32U)
#define FLEXCAN_RX15MASK_RX15M(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RX15MASK_RX15M_SHIFT)) & FLEXCAN_RX15MASK_RX15M_MASK)
/*! @} */

/*! @name ECR - Error Counter */
/*! @{ */

#define FLEXCAN_ECR_TXERRCNT_MASK                (0xFFU)
#define FLEXCAN_ECR_TXERRCNT_SHIFT               (0U)
#define FLEXCAN_ECR_TXERRCNT_WIDTH               (8U)
#define FLEXCAN_ECR_TXERRCNT(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ECR_TXERRCNT_SHIFT)) & FLEXCAN_ECR_TXERRCNT_MASK)

#define FLEXCAN_ECR_RXERRCNT_MASK                (0xFF00U)
#define FLEXCAN_ECR_RXERRCNT_SHIFT               (8U)
#define FLEXCAN_ECR_RXERRCNT_WIDTH               (8U)
#define FLEXCAN_ECR_RXERRCNT(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ECR_RXERRCNT_SHIFT)) & FLEXCAN_ECR_RXERRCNT_MASK)

#define FLEXCAN_ECR_TXERRCNT_FAST_MASK           (0xFF0000U)
#define FLEXCAN_ECR_TXERRCNT_FAST_SHIFT          (16U)
#define FLEXCAN_ECR_TXERRCNT_FAST_WIDTH          (8U)
#define FLEXCAN_ECR_TXERRCNT_FAST(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ECR_TXERRCNT_FAST_SHIFT)) & FLEXCAN_ECR_TXERRCNT_FAST_MASK)

#define FLEXCAN_ECR_RXERRCNT_FAST_MASK           (0xFF000000U)
#define FLEXCAN_ECR_RXERRCNT_FAST_SHIFT          (24U)
#define FLEXCAN_ECR_RXERRCNT_FAST_WIDTH          (8U)
#define FLEXCAN_ECR_RXERRCNT_FAST(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ECR_RXERRCNT_FAST_SHIFT)) & FLEXCAN_ECR_RXERRCNT_FAST_MASK)
/*! @} */

/*! @name ESR1 - Error and Status 1 Register */
/*! @{ */

#define FLEXCAN_ESR1_ERRINT_MASK                 (0x2U)
#define FLEXCAN_ESR1_ERRINT_SHIFT                (1U)
#define FLEXCAN_ESR1_ERRINT_WIDTH                (1U)
#define FLEXCAN_ESR1_ERRINT(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_ERRINT_SHIFT)) & FLEXCAN_ESR1_ERRINT_MASK)

#define FLEXCAN_ESR1_BOFFINT_MASK                (0x4U)
#define FLEXCAN_ESR1_BOFFINT_SHIFT               (2U)
#define FLEXCAN_ESR1_BOFFINT_WIDTH               (1U)
#define FLEXCAN_ESR1_BOFFINT(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BOFFINT_SHIFT)) & FLEXCAN_ESR1_BOFFINT_MASK)

#define FLEXCAN_ESR1_RX_MASK                     (0x8U)
#define FLEXCAN_ESR1_RX_SHIFT                    (3U)
#define FLEXCAN_ESR1_RX_WIDTH                    (1U)
#define FLEXCAN_ESR1_RX(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_RX_SHIFT)) & FLEXCAN_ESR1_RX_MASK)

#define FLEXCAN_ESR1_FLTCONF_MASK                (0x30U)
#define FLEXCAN_ESR1_FLTCONF_SHIFT               (4U)
#define FLEXCAN_ESR1_FLTCONF_WIDTH               (2U)
#define FLEXCAN_ESR1_FLTCONF(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_FLTCONF_SHIFT)) & FLEXCAN_ESR1_FLTCONF_MASK)

#define FLEXCAN_ESR1_TX_MASK                     (0x40U)
#define FLEXCAN_ESR1_TX_SHIFT                    (6U)
#define FLEXCAN_ESR1_TX_WIDTH                    (1U)
#define FLEXCAN_ESR1_TX(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_TX_SHIFT)) & FLEXCAN_ESR1_TX_MASK)

#define FLEXCAN_ESR1_IDLE_MASK                   (0x80U)
#define FLEXCAN_ESR1_IDLE_SHIFT                  (7U)
#define FLEXCAN_ESR1_IDLE_WIDTH                  (1U)
#define FLEXCAN_ESR1_IDLE(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_IDLE_SHIFT)) & FLEXCAN_ESR1_IDLE_MASK)

#define FLEXCAN_ESR1_RXWRN_MASK                  (0x100U)
#define FLEXCAN_ESR1_RXWRN_SHIFT                 (8U)
#define FLEXCAN_ESR1_RXWRN_WIDTH                 (1U)
#define FLEXCAN_ESR1_RXWRN(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_RXWRN_SHIFT)) & FLEXCAN_ESR1_RXWRN_MASK)

#define FLEXCAN_ESR1_TXWRN_MASK                  (0x200U)
#define FLEXCAN_ESR1_TXWRN_SHIFT                 (9U)
#define FLEXCAN_ESR1_TXWRN_WIDTH                 (1U)
#define FLEXCAN_ESR1_TXWRN(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_TXWRN_SHIFT)) & FLEXCAN_ESR1_TXWRN_MASK)

#define FLEXCAN_ESR1_STFERR_MASK                 (0x400U)
#define FLEXCAN_ESR1_STFERR_SHIFT                (10U)
#define FLEXCAN_ESR1_STFERR_WIDTH                (1U)
#define FLEXCAN_ESR1_STFERR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_STFERR_SHIFT)) & FLEXCAN_ESR1_STFERR_MASK)

#define FLEXCAN_ESR1_FRMERR_MASK                 (0x800U)
#define FLEXCAN_ESR1_FRMERR_SHIFT                (11U)
#define FLEXCAN_ESR1_FRMERR_WIDTH                (1U)
#define FLEXCAN_ESR1_FRMERR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_FRMERR_SHIFT)) & FLEXCAN_ESR1_FRMERR_MASK)

#define FLEXCAN_ESR1_CRCERR_MASK                 (0x1000U)
#define FLEXCAN_ESR1_CRCERR_SHIFT                (12U)
#define FLEXCAN_ESR1_CRCERR_WIDTH                (1U)
#define FLEXCAN_ESR1_CRCERR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_CRCERR_SHIFT)) & FLEXCAN_ESR1_CRCERR_MASK)

#define FLEXCAN_ESR1_ACKERR_MASK                 (0x2000U)
#define FLEXCAN_ESR1_ACKERR_SHIFT                (13U)
#define FLEXCAN_ESR1_ACKERR_WIDTH                (1U)
#define FLEXCAN_ESR1_ACKERR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_ACKERR_SHIFT)) & FLEXCAN_ESR1_ACKERR_MASK)

#define FLEXCAN_ESR1_BIT0ERR_MASK                (0x4000U)
#define FLEXCAN_ESR1_BIT0ERR_SHIFT               (14U)
#define FLEXCAN_ESR1_BIT0ERR_WIDTH               (1U)
#define FLEXCAN_ESR1_BIT0ERR(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BIT0ERR_SHIFT)) & FLEXCAN_ESR1_BIT0ERR_MASK)

#define FLEXCAN_ESR1_BIT1ERR_MASK                (0x8000U)
#define FLEXCAN_ESR1_BIT1ERR_SHIFT               (15U)
#define FLEXCAN_ESR1_BIT1ERR_WIDTH               (1U)
#define FLEXCAN_ESR1_BIT1ERR(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BIT1ERR_SHIFT)) & FLEXCAN_ESR1_BIT1ERR_MASK)

#define FLEXCAN_ESR1_RWRNINT_MASK                (0x10000U)
#define FLEXCAN_ESR1_RWRNINT_SHIFT               (16U)
#define FLEXCAN_ESR1_RWRNINT_WIDTH               (1U)
#define FLEXCAN_ESR1_RWRNINT(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_RWRNINT_SHIFT)) & FLEXCAN_ESR1_RWRNINT_MASK)

#define FLEXCAN_ESR1_TWRNINT_MASK                (0x20000U)
#define FLEXCAN_ESR1_TWRNINT_SHIFT               (17U)
#define FLEXCAN_ESR1_TWRNINT_WIDTH               (1U)
#define FLEXCAN_ESR1_TWRNINT(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_TWRNINT_SHIFT)) & FLEXCAN_ESR1_TWRNINT_MASK)

#define FLEXCAN_ESR1_SYNCH_MASK                  (0x40000U)
#define FLEXCAN_ESR1_SYNCH_SHIFT                 (18U)
#define FLEXCAN_ESR1_SYNCH_WIDTH                 (1U)
#define FLEXCAN_ESR1_SYNCH(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_SYNCH_SHIFT)) & FLEXCAN_ESR1_SYNCH_MASK)

#define FLEXCAN_ESR1_BOFFDONEINT_MASK            (0x80000U)
#define FLEXCAN_ESR1_BOFFDONEINT_SHIFT           (19U)
#define FLEXCAN_ESR1_BOFFDONEINT_WIDTH           (1U)
#define FLEXCAN_ESR1_BOFFDONEINT(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BOFFDONEINT_SHIFT)) & FLEXCAN_ESR1_BOFFDONEINT_MASK)

#define FLEXCAN_ESR1_ERRINT_FAST_MASK            (0x100000U)
#define FLEXCAN_ESR1_ERRINT_FAST_SHIFT           (20U)
#define FLEXCAN_ESR1_ERRINT_FAST_WIDTH           (1U)
#define FLEXCAN_ESR1_ERRINT_FAST(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_ERRINT_FAST_SHIFT)) & FLEXCAN_ESR1_ERRINT_FAST_MASK)

#define FLEXCAN_ESR1_ERROVR_MASK                 (0x200000U)
#define FLEXCAN_ESR1_ERROVR_SHIFT                (21U)
#define FLEXCAN_ESR1_ERROVR_WIDTH                (1U)
#define FLEXCAN_ESR1_ERROVR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_ERROVR_SHIFT)) & FLEXCAN_ESR1_ERROVR_MASK)

#define FLEXCAN_ESR1_STFERR_FAST_MASK            (0x4000000U)
#define FLEXCAN_ESR1_STFERR_FAST_SHIFT           (26U)
#define FLEXCAN_ESR1_STFERR_FAST_WIDTH           (1U)
#define FLEXCAN_ESR1_STFERR_FAST(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_STFERR_FAST_SHIFT)) & FLEXCAN_ESR1_STFERR_FAST_MASK)

#define FLEXCAN_ESR1_FRMERR_FAST_MASK            (0x8000000U)
#define FLEXCAN_ESR1_FRMERR_FAST_SHIFT           (27U)
#define FLEXCAN_ESR1_FRMERR_FAST_WIDTH           (1U)
#define FLEXCAN_ESR1_FRMERR_FAST(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_FRMERR_FAST_SHIFT)) & FLEXCAN_ESR1_FRMERR_FAST_MASK)

#define FLEXCAN_ESR1_CRCERR_FAST_MASK            (0x10000000U)
#define FLEXCAN_ESR1_CRCERR_FAST_SHIFT           (28U)
#define FLEXCAN_ESR1_CRCERR_FAST_WIDTH           (1U)
#define FLEXCAN_ESR1_CRCERR_FAST(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_CRCERR_FAST_SHIFT)) & FLEXCAN_ESR1_CRCERR_FAST_MASK)

#define FLEXCAN_ESR1_BIT0ERR_FAST_MASK           (0x40000000U)
#define FLEXCAN_ESR1_BIT0ERR_FAST_SHIFT          (30U)
#define FLEXCAN_ESR1_BIT0ERR_FAST_WIDTH          (1U)
#define FLEXCAN_ESR1_BIT0ERR_FAST(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BIT0ERR_FAST_SHIFT)) & FLEXCAN_ESR1_BIT0ERR_FAST_MASK)

#define FLEXCAN_ESR1_BIT1ERR_FAST_MASK           (0x80000000U)
#define FLEXCAN_ESR1_BIT1ERR_FAST_SHIFT          (31U)
#define FLEXCAN_ESR1_BIT1ERR_FAST_WIDTH          (1U)
#define FLEXCAN_ESR1_BIT1ERR_FAST(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR1_BIT1ERR_FAST_SHIFT)) & FLEXCAN_ESR1_BIT1ERR_FAST_MASK)
/*! @} */

/*! @name IMASK1 - Interrupt Masks 1 Register */
/*! @{ */

#define FLEXCAN_IMASK1_BUF31TO0M_MASK            (0xFFFFFFFFU)
#define FLEXCAN_IMASK1_BUF31TO0M_SHIFT           (0U)
#define FLEXCAN_IMASK1_BUF31TO0M_WIDTH           (32U)
#define FLEXCAN_IMASK1_BUF31TO0M(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IMASK1_BUF31TO0M_SHIFT)) & FLEXCAN_IMASK1_BUF31TO0M_MASK)
/*! @} */

/*! @name IFLAG1 - Interrupt Flags 1 Register */
/*! @{ */

#define FLEXCAN_IFLAG1_BUF0I_MASK                (0x1U)
#define FLEXCAN_IFLAG1_BUF0I_SHIFT               (0U)
#define FLEXCAN_IFLAG1_BUF0I_WIDTH               (1U)
#define FLEXCAN_IFLAG1_BUF0I(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF0I_SHIFT)) & FLEXCAN_IFLAG1_BUF0I_MASK)

#define FLEXCAN_IFLAG1_BUF4TO1I_MASK             (0x1EU)
#define FLEXCAN_IFLAG1_BUF4TO1I_SHIFT            (1U)
#define FLEXCAN_IFLAG1_BUF4TO1I_WIDTH            (4U)
#define FLEXCAN_IFLAG1_BUF4TO1I(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF4TO1I_SHIFT)) & FLEXCAN_IFLAG1_BUF4TO1I_MASK)

#define FLEXCAN_IFLAG1_BUF5I_MASK                (0x20U)
#define FLEXCAN_IFLAG1_BUF5I_SHIFT               (5U)
#define FLEXCAN_IFLAG1_BUF5I_WIDTH               (1U)
#define FLEXCAN_IFLAG1_BUF5I(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF5I_SHIFT)) & FLEXCAN_IFLAG1_BUF5I_MASK)

#define FLEXCAN_IFLAG1_BUF6I_MASK                (0x40U)
#define FLEXCAN_IFLAG1_BUF6I_SHIFT               (6U)
#define FLEXCAN_IFLAG1_BUF6I_WIDTH               (1U)
#define FLEXCAN_IFLAG1_BUF6I(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF6I_SHIFT)) & FLEXCAN_IFLAG1_BUF6I_MASK)

#define FLEXCAN_IFLAG1_BUF7I_MASK                (0x80U)
#define FLEXCAN_IFLAG1_BUF7I_SHIFT               (7U)
#define FLEXCAN_IFLAG1_BUF7I_WIDTH               (1U)
#define FLEXCAN_IFLAG1_BUF7I(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF7I_SHIFT)) & FLEXCAN_IFLAG1_BUF7I_MASK)

#define FLEXCAN_IFLAG1_BUF31TO8I_MASK            (0xFFFFFF00U)
#define FLEXCAN_IFLAG1_BUF31TO8I_SHIFT           (8U)
#define FLEXCAN_IFLAG1_BUF31TO8I_WIDTH           (24U)
#define FLEXCAN_IFLAG1_BUF31TO8I(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_IFLAG1_BUF31TO8I_SHIFT)) & FLEXCAN_IFLAG1_BUF31TO8I_MASK)
/*! @} */

/*! @name CTRL2 - Control 2 Register */
/*! @{ */

#define FLEXCAN_CTRL2_EDFLTDIS_MASK              (0x800U)
#define FLEXCAN_CTRL2_EDFLTDIS_SHIFT             (11U)
#define FLEXCAN_CTRL2_EDFLTDIS_WIDTH             (1U)
#define FLEXCAN_CTRL2_EDFLTDIS(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_EDFLTDIS_SHIFT)) & FLEXCAN_CTRL2_EDFLTDIS_MASK)

#define FLEXCAN_CTRL2_ISOCANFDEN_MASK            (0x1000U)
#define FLEXCAN_CTRL2_ISOCANFDEN_SHIFT           (12U)
#define FLEXCAN_CTRL2_ISOCANFDEN_WIDTH           (1U)
#define FLEXCAN_CTRL2_ISOCANFDEN(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_ISOCANFDEN_SHIFT)) & FLEXCAN_CTRL2_ISOCANFDEN_MASK)

#define FLEXCAN_CTRL2_PREXCEN_MASK               (0x4000U)
#define FLEXCAN_CTRL2_PREXCEN_SHIFT              (14U)
#define FLEXCAN_CTRL2_PREXCEN_WIDTH              (1U)
#define FLEXCAN_CTRL2_PREXCEN(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_PREXCEN_SHIFT)) & FLEXCAN_CTRL2_PREXCEN_MASK)

#define FLEXCAN_CTRL2_TIMER_SRC_MASK             (0x8000U)
#define FLEXCAN_CTRL2_TIMER_SRC_SHIFT            (15U)
#define FLEXCAN_CTRL2_TIMER_SRC_WIDTH            (1U)
#define FLEXCAN_CTRL2_TIMER_SRC(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_TIMER_SRC_SHIFT)) & FLEXCAN_CTRL2_TIMER_SRC_MASK)

#define FLEXCAN_CTRL2_EACEN_MASK                 (0x10000U)
#define FLEXCAN_CTRL2_EACEN_SHIFT                (16U)
#define FLEXCAN_CTRL2_EACEN_WIDTH                (1U)
#define FLEXCAN_CTRL2_EACEN(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_EACEN_SHIFT)) & FLEXCAN_CTRL2_EACEN_MASK)

#define FLEXCAN_CTRL2_RRS_MASK                   (0x20000U)
#define FLEXCAN_CTRL2_RRS_SHIFT                  (17U)
#define FLEXCAN_CTRL2_RRS_WIDTH                  (1U)
#define FLEXCAN_CTRL2_RRS(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_RRS_SHIFT)) & FLEXCAN_CTRL2_RRS_MASK)

#define FLEXCAN_CTRL2_MRP_MASK                   (0x40000U)
#define FLEXCAN_CTRL2_MRP_SHIFT                  (18U)
#define FLEXCAN_CTRL2_MRP_WIDTH                  (1U)
#define FLEXCAN_CTRL2_MRP(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_MRP_SHIFT)) & FLEXCAN_CTRL2_MRP_MASK)

#define FLEXCAN_CTRL2_TASD_MASK                  (0xF80000U)
#define FLEXCAN_CTRL2_TASD_SHIFT                 (19U)
#define FLEXCAN_CTRL2_TASD_WIDTH                 (5U)
#define FLEXCAN_CTRL2_TASD(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_TASD_SHIFT)) & FLEXCAN_CTRL2_TASD_MASK)

#define FLEXCAN_CTRL2_RFFN_MASK                  (0xF000000U)
#define FLEXCAN_CTRL2_RFFN_SHIFT                 (24U)
#define FLEXCAN_CTRL2_RFFN_WIDTH                 (4U)
#define FLEXCAN_CTRL2_RFFN(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_RFFN_SHIFT)) & FLEXCAN_CTRL2_RFFN_MASK)

#define FLEXCAN_CTRL2_BOFFDONEMSK_MASK           (0x40000000U)
#define FLEXCAN_CTRL2_BOFFDONEMSK_SHIFT          (30U)
#define FLEXCAN_CTRL2_BOFFDONEMSK_WIDTH          (1U)
#define FLEXCAN_CTRL2_BOFFDONEMSK(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_BOFFDONEMSK_SHIFT)) & FLEXCAN_CTRL2_BOFFDONEMSK_MASK)

#define FLEXCAN_CTRL2_ERRMSK_FAST_MASK           (0x80000000U)
#define FLEXCAN_CTRL2_ERRMSK_FAST_SHIFT          (31U)
#define FLEXCAN_CTRL2_ERRMSK_FAST_WIDTH          (1U)
#define FLEXCAN_CTRL2_ERRMSK_FAST(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_ERRMSK_FAST_SHIFT)) & FLEXCAN_CTRL2_ERRMSK_FAST_MASK)
/*! @} */

/*! @name ESR2 - Error and Status 2 Register */
/*! @{ */

#define FLEXCAN_ESR2_IMB_MASK                    (0x2000U)
#define FLEXCAN_ESR2_IMB_SHIFT                   (13U)
#define FLEXCAN_ESR2_IMB_WIDTH                   (1U)
#define FLEXCAN_ESR2_IMB(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR2_IMB_SHIFT)) & FLEXCAN_ESR2_IMB_MASK)

#define FLEXCAN_ESR2_VPS_MASK                    (0x4000U)
#define FLEXCAN_ESR2_VPS_SHIFT                   (14U)
#define FLEXCAN_ESR2_VPS_WIDTH                   (1U)
#define FLEXCAN_ESR2_VPS(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR2_VPS_SHIFT)) & FLEXCAN_ESR2_VPS_MASK)

#define FLEXCAN_ESR2_LPTM_MASK                   (0x7F0000U)
#define FLEXCAN_ESR2_LPTM_SHIFT                  (16U)
#define FLEXCAN_ESR2_LPTM_WIDTH                  (7U)
#define FLEXCAN_ESR2_LPTM(x)                     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_ESR2_LPTM_SHIFT)) & FLEXCAN_ESR2_LPTM_MASK)
/*! @} */

/*! @name CRCR - CRC Register */
/*! @{ */

#define FLEXCAN_CRCR_TXCRC_MASK                  (0x7FFFU)
#define FLEXCAN_CRCR_TXCRC_SHIFT                 (0U)
#define FLEXCAN_CRCR_TXCRC_WIDTH                 (15U)
#define FLEXCAN_CRCR_TXCRC(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CRCR_TXCRC_SHIFT)) & FLEXCAN_CRCR_TXCRC_MASK)

#define FLEXCAN_CRCR_MBCRC_MASK                  (0x7F0000U)
#define FLEXCAN_CRCR_MBCRC_SHIFT                 (16U)
#define FLEXCAN_CRCR_MBCRC_WIDTH                 (7U)
#define FLEXCAN_CRCR_MBCRC(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CRCR_MBCRC_SHIFT)) & FLEXCAN_CRCR_MBCRC_MASK)
/*! @} */

/*! @name RXFGMASK - Rx FIFO Global Mask Register */
/*! @{ */

#define FLEXCAN_RXFGMASK_FGM_MASK                (0xFFFFFFFFU)
#define FLEXCAN_RXFGMASK_FGM_SHIFT               (0U)
#define FLEXCAN_RXFGMASK_FGM_WIDTH               (32U)
#define FLEXCAN_RXFGMASK_FGM(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RXFGMASK_FGM_SHIFT)) & FLEXCAN_RXFGMASK_FGM_MASK)
/*! @} */

/*! @name RXFIR - Rx FIFO Information Register */
/*! @{ */

#define FLEXCAN_RXFIR_IDHIT_MASK                 (0x1FFU)
#define FLEXCAN_RXFIR_IDHIT_SHIFT                (0U)
#define FLEXCAN_RXFIR_IDHIT_WIDTH                (9U)
#define FLEXCAN_RXFIR_IDHIT(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RXFIR_IDHIT_SHIFT)) & FLEXCAN_RXFIR_IDHIT_MASK)
/*! @} */

/*! @name CBT - CAN Bit Timing Register */
/*! @{ */

#define FLEXCAN_CBT_EPSEG2_MASK                  (0x1FU)
#define FLEXCAN_CBT_EPSEG2_SHIFT                 (0U)
#define FLEXCAN_CBT_EPSEG2_WIDTH                 (5U)
#define FLEXCAN_CBT_EPSEG2(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_EPSEG2_SHIFT)) & FLEXCAN_CBT_EPSEG2_MASK)

#define FLEXCAN_CBT_EPSEG1_MASK                  (0x3E0U)
#define FLEXCAN_CBT_EPSEG1_SHIFT                 (5U)
#define FLEXCAN_CBT_EPSEG1_WIDTH                 (5U)
#define FLEXCAN_CBT_EPSEG1(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_EPSEG1_SHIFT)) & FLEXCAN_CBT_EPSEG1_MASK)

#define FLEXCAN_CBT_EPROPSEG_MASK                (0xFC00U)
#define FLEXCAN_CBT_EPROPSEG_SHIFT               (10U)
#define FLEXCAN_CBT_EPROPSEG_WIDTH               (6U)
#define FLEXCAN_CBT_EPROPSEG(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_EPROPSEG_SHIFT)) & FLEXCAN_CBT_EPROPSEG_MASK)

#define FLEXCAN_CBT_ERJW_MASK                    (0x1F0000U)
#define FLEXCAN_CBT_ERJW_SHIFT                   (16U)
#define FLEXCAN_CBT_ERJW_WIDTH                   (5U)
#define FLEXCAN_CBT_ERJW(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_ERJW_SHIFT)) & FLEXCAN_CBT_ERJW_MASK)

#define FLEXCAN_CBT_EPRESDIV_MASK                (0x7FE00000U)
#define FLEXCAN_CBT_EPRESDIV_SHIFT               (21U)
#define FLEXCAN_CBT_EPRESDIV_WIDTH               (10U)
#define FLEXCAN_CBT_EPRESDIV(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_EPRESDIV_SHIFT)) & FLEXCAN_CBT_EPRESDIV_MASK)

#define FLEXCAN_CBT_BTF_MASK                     (0x80000000U)
#define FLEXCAN_CBT_BTF_SHIFT                    (31U)
#define FLEXCAN_CBT_BTF_WIDTH                    (1U)
#define FLEXCAN_CBT_BTF(x)                       (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CBT_BTF_SHIFT)) & FLEXCAN_CBT_BTF_MASK)
/*! @} */

/*! @name RAMn - Embedded RAM */
/*! @{ */

#define FLEXCAN_RAMn_DATA_BYTE_3_MASK            (0xFFU)
#define FLEXCAN_RAMn_DATA_BYTE_3_SHIFT           (0U)
#define FLEXCAN_RAMn_DATA_BYTE_3_WIDTH           (8U)
#define FLEXCAN_RAMn_DATA_BYTE_3(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RAMn_DATA_BYTE_3_SHIFT)) & FLEXCAN_RAMn_DATA_BYTE_3_MASK)

#define FLEXCAN_RAMn_DATA_BYTE_2_MASK            (0xFF00U)
#define FLEXCAN_RAMn_DATA_BYTE_2_SHIFT           (8U)
#define FLEXCAN_RAMn_DATA_BYTE_2_WIDTH           (8U)
#define FLEXCAN_RAMn_DATA_BYTE_2(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RAMn_DATA_BYTE_2_SHIFT)) & FLEXCAN_RAMn_DATA_BYTE_2_MASK)

#define FLEXCAN_RAMn_DATA_BYTE_1_MASK            (0xFF0000U)
#define FLEXCAN_RAMn_DATA_BYTE_1_SHIFT           (16U)
#define FLEXCAN_RAMn_DATA_BYTE_1_WIDTH           (8U)
#define FLEXCAN_RAMn_DATA_BYTE_1(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RAMn_DATA_BYTE_1_SHIFT)) & FLEXCAN_RAMn_DATA_BYTE_1_MASK)

#define FLEXCAN_RAMn_DATA_BYTE_0_MASK            (0xFF000000U)
#define FLEXCAN_RAMn_DATA_BYTE_0_SHIFT           (24U)
#define FLEXCAN_RAMn_DATA_BYTE_0_WIDTH           (8U)
#define FLEXCAN_RAMn_DATA_BYTE_0(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RAMn_DATA_BYTE_0_SHIFT)) & FLEXCAN_RAMn_DATA_BYTE_0_MASK)
/*! @} */

/*! @name RXIMR - Rx Individual Mask Registers */
/*! @{ */

#define FLEXCAN_RXIMR_MI_MASK                    (0xFFFFFFFFU)
#define FLEXCAN_RXIMR_MI_SHIFT                   (0U)
#define FLEXCAN_RXIMR_MI_WIDTH                   (32U)
#define FLEXCAN_RXIMR_MI(x)                      (((uint32_t)(((uint32_t)(x)) << FLEXCAN_RXIMR_MI_SHIFT)) & FLEXCAN_RXIMR_MI_MASK)
/*! @} */

/*! @name CTRL1_PN - Pretended Networking Control 1 Register */
/*! @{ */

#define FLEXCAN_CTRL1_PN_FCS_MASK                (0x3U)
#define FLEXCAN_CTRL1_PN_FCS_SHIFT               (0U)
#define FLEXCAN_CTRL1_PN_FCS_WIDTH               (2U)
#define FLEXCAN_CTRL1_PN_FCS(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_FCS_SHIFT)) & FLEXCAN_CTRL1_PN_FCS_MASK)

#define FLEXCAN_CTRL1_PN_IDFS_MASK               (0xCU)
#define FLEXCAN_CTRL1_PN_IDFS_SHIFT              (2U)
#define FLEXCAN_CTRL1_PN_IDFS_WIDTH              (2U)
#define FLEXCAN_CTRL1_PN_IDFS(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_IDFS_SHIFT)) & FLEXCAN_CTRL1_PN_IDFS_MASK)

#define FLEXCAN_CTRL1_PN_PLFS_MASK               (0x30U)
#define FLEXCAN_CTRL1_PN_PLFS_SHIFT              (4U)
#define FLEXCAN_CTRL1_PN_PLFS_WIDTH              (2U)
#define FLEXCAN_CTRL1_PN_PLFS(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_PLFS_SHIFT)) & FLEXCAN_CTRL1_PN_PLFS_MASK)

#define FLEXCAN_CTRL1_PN_NMATCH_MASK             (0xFF00U)
#define FLEXCAN_CTRL1_PN_NMATCH_SHIFT            (8U)
#define FLEXCAN_CTRL1_PN_NMATCH_WIDTH            (8U)
#define FLEXCAN_CTRL1_PN_NMATCH(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_NMATCH_SHIFT)) & FLEXCAN_CTRL1_PN_NMATCH_MASK)

#define FLEXCAN_CTRL1_PN_WUMF_MSK_MASK           (0x10000U)
#define FLEXCAN_CTRL1_PN_WUMF_MSK_SHIFT          (16U)
#define FLEXCAN_CTRL1_PN_WUMF_MSK_WIDTH          (1U)
#define FLEXCAN_CTRL1_PN_WUMF_MSK(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_WUMF_MSK_SHIFT)) & FLEXCAN_CTRL1_PN_WUMF_MSK_MASK)

#define FLEXCAN_CTRL1_PN_WTOF_MSK_MASK           (0x20000U)
#define FLEXCAN_CTRL1_PN_WTOF_MSK_SHIFT          (17U)
#define FLEXCAN_CTRL1_PN_WTOF_MSK_WIDTH          (1U)
#define FLEXCAN_CTRL1_PN_WTOF_MSK(x)             (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL1_PN_WTOF_MSK_SHIFT)) & FLEXCAN_CTRL1_PN_WTOF_MSK_MASK)
/*! @} */

/*! @name CTRL2_PN - Pretended Networking Control 2 Register */
/*! @{ */

#define FLEXCAN_CTRL2_PN_MATCHTO_MASK            (0xFFFFU)
#define FLEXCAN_CTRL2_PN_MATCHTO_SHIFT           (0U)
#define FLEXCAN_CTRL2_PN_MATCHTO_WIDTH           (16U)
#define FLEXCAN_CTRL2_PN_MATCHTO(x)              (((uint32_t)(((uint32_t)(x)) << FLEXCAN_CTRL2_PN_MATCHTO_SHIFT)) & FLEXCAN_CTRL2_PN_MATCHTO_MASK)
/*! @} */

/*! @name WU_MTC - Pretended Networking Wake Up Match Register */
/*! @{ */

#define FLEXCAN_WU_MTC_MCOUNTER_MASK             (0xFF00U)
#define FLEXCAN_WU_MTC_MCOUNTER_SHIFT            (8U)
#define FLEXCAN_WU_MTC_MCOUNTER_WIDTH            (8U)
#define FLEXCAN_WU_MTC_MCOUNTER(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WU_MTC_MCOUNTER_SHIFT)) & FLEXCAN_WU_MTC_MCOUNTER_MASK)

#define FLEXCAN_WU_MTC_WUMF_MASK                 (0x10000U)
#define FLEXCAN_WU_MTC_WUMF_SHIFT                (16U)
#define FLEXCAN_WU_MTC_WUMF_WIDTH                (1U)
#define FLEXCAN_WU_MTC_WUMF(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WU_MTC_WUMF_SHIFT)) & FLEXCAN_WU_MTC_WUMF_MASK)

#define FLEXCAN_WU_MTC_WTOF_MASK                 (0x20000U)
#define FLEXCAN_WU_MTC_WTOF_SHIFT                (17U)
#define FLEXCAN_WU_MTC_WTOF_WIDTH                (1U)
#define FLEXCAN_WU_MTC_WTOF(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WU_MTC_WTOF_SHIFT)) & FLEXCAN_WU_MTC_WTOF_MASK)
/*! @} */

/*! @name FLT_ID1 - Pretended Networking ID Filter 1 Register */
/*! @{ */

#define FLEXCAN_FLT_ID1_FLT_ID1_MASK             (0x1FFFFFFFU)
#define FLEXCAN_FLT_ID1_FLT_ID1_SHIFT            (0U)
#define FLEXCAN_FLT_ID1_FLT_ID1_WIDTH            (29U)
#define FLEXCAN_FLT_ID1_FLT_ID1(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID1_FLT_ID1_SHIFT)) & FLEXCAN_FLT_ID1_FLT_ID1_MASK)

#define FLEXCAN_FLT_ID1_FLT_RTR_MASK             (0x20000000U)
#define FLEXCAN_FLT_ID1_FLT_RTR_SHIFT            (29U)
#define FLEXCAN_FLT_ID1_FLT_RTR_WIDTH            (1U)
#define FLEXCAN_FLT_ID1_FLT_RTR(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID1_FLT_RTR_SHIFT)) & FLEXCAN_FLT_ID1_FLT_RTR_MASK)

#define FLEXCAN_FLT_ID1_FLT_IDE_MASK             (0x40000000U)
#define FLEXCAN_FLT_ID1_FLT_IDE_SHIFT            (30U)
#define FLEXCAN_FLT_ID1_FLT_IDE_WIDTH            (1U)
#define FLEXCAN_FLT_ID1_FLT_IDE(x)               (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID1_FLT_IDE_SHIFT)) & FLEXCAN_FLT_ID1_FLT_IDE_MASK)
/*! @} */

/*! @name FLT_DLC - Pretended Networking DLC Filter Register */
/*! @{ */

#define FLEXCAN_FLT_DLC_FLT_DLC_HI_MASK          (0xFU)
#define FLEXCAN_FLT_DLC_FLT_DLC_HI_SHIFT         (0U)
#define FLEXCAN_FLT_DLC_FLT_DLC_HI_WIDTH         (4U)
#define FLEXCAN_FLT_DLC_FLT_DLC_HI(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_DLC_FLT_DLC_HI_SHIFT)) & FLEXCAN_FLT_DLC_FLT_DLC_HI_MASK)

#define FLEXCAN_FLT_DLC_FLT_DLC_LO_MASK          (0xF0000U)
#define FLEXCAN_FLT_DLC_FLT_DLC_LO_SHIFT         (16U)
#define FLEXCAN_FLT_DLC_FLT_DLC_LO_WIDTH         (4U)
#define FLEXCAN_FLT_DLC_FLT_DLC_LO(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_DLC_FLT_DLC_LO_SHIFT)) & FLEXCAN_FLT_DLC_FLT_DLC_LO_MASK)
/*! @} */

/*! @name PL1_LO - Pretended Networking Payload Low Filter 1 Register */
/*! @{ */

#define FLEXCAN_PL1_LO_Data_byte_3_MASK          (0xFFU)
#define FLEXCAN_PL1_LO_Data_byte_3_SHIFT         (0U)
#define FLEXCAN_PL1_LO_Data_byte_3_WIDTH         (8U)
#define FLEXCAN_PL1_LO_Data_byte_3(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_LO_Data_byte_3_SHIFT)) & FLEXCAN_PL1_LO_Data_byte_3_MASK)

#define FLEXCAN_PL1_LO_Data_byte_2_MASK          (0xFF00U)
#define FLEXCAN_PL1_LO_Data_byte_2_SHIFT         (8U)
#define FLEXCAN_PL1_LO_Data_byte_2_WIDTH         (8U)
#define FLEXCAN_PL1_LO_Data_byte_2(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_LO_Data_byte_2_SHIFT)) & FLEXCAN_PL1_LO_Data_byte_2_MASK)

#define FLEXCAN_PL1_LO_Data_byte_1_MASK          (0xFF0000U)
#define FLEXCAN_PL1_LO_Data_byte_1_SHIFT         (16U)
#define FLEXCAN_PL1_LO_Data_byte_1_WIDTH         (8U)
#define FLEXCAN_PL1_LO_Data_byte_1(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_LO_Data_byte_1_SHIFT)) & FLEXCAN_PL1_LO_Data_byte_1_MASK)

#define FLEXCAN_PL1_LO_Data_byte_0_MASK          (0xFF000000U)
#define FLEXCAN_PL1_LO_Data_byte_0_SHIFT         (24U)
#define FLEXCAN_PL1_LO_Data_byte_0_WIDTH         (8U)
#define FLEXCAN_PL1_LO_Data_byte_0(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_LO_Data_byte_0_SHIFT)) & FLEXCAN_PL1_LO_Data_byte_0_MASK)
/*! @} */

/*! @name PL1_HI - Pretended Networking Payload High Filter 1 Register */
/*! @{ */

#define FLEXCAN_PL1_HI_Data_byte_7_MASK          (0xFFU)
#define FLEXCAN_PL1_HI_Data_byte_7_SHIFT         (0U)
#define FLEXCAN_PL1_HI_Data_byte_7_WIDTH         (8U)
#define FLEXCAN_PL1_HI_Data_byte_7(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_HI_Data_byte_7_SHIFT)) & FLEXCAN_PL1_HI_Data_byte_7_MASK)

#define FLEXCAN_PL1_HI_Data_byte_6_MASK          (0xFF00U)
#define FLEXCAN_PL1_HI_Data_byte_6_SHIFT         (8U)
#define FLEXCAN_PL1_HI_Data_byte_6_WIDTH         (8U)
#define FLEXCAN_PL1_HI_Data_byte_6(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_HI_Data_byte_6_SHIFT)) & FLEXCAN_PL1_HI_Data_byte_6_MASK)

#define FLEXCAN_PL1_HI_Data_byte_5_MASK          (0xFF0000U)
#define FLEXCAN_PL1_HI_Data_byte_5_SHIFT         (16U)
#define FLEXCAN_PL1_HI_Data_byte_5_WIDTH         (8U)
#define FLEXCAN_PL1_HI_Data_byte_5(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_HI_Data_byte_5_SHIFT)) & FLEXCAN_PL1_HI_Data_byte_5_MASK)

#define FLEXCAN_PL1_HI_Data_byte_4_MASK          (0xFF000000U)
#define FLEXCAN_PL1_HI_Data_byte_4_SHIFT         (24U)
#define FLEXCAN_PL1_HI_Data_byte_4_WIDTH         (8U)
#define FLEXCAN_PL1_HI_Data_byte_4(x)            (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL1_HI_Data_byte_4_SHIFT)) & FLEXCAN_PL1_HI_Data_byte_4_MASK)
/*! @} */

/*! @name FLT_ID2_IDMASK - Pretended Networking ID Filter 2 Register / ID Mask Register */
/*! @{ */

#define FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK_MASK (0x1FFFFFFFU)
#define FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK_SHIFT (0U)
#define FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK_WIDTH (29U)
#define FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK(x) (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK_SHIFT)) & FLEXCAN_FLT_ID2_IDMASK_FLT_ID2_IDMASK_MASK)

#define FLEXCAN_FLT_ID2_IDMASK_RTR_MSK_MASK      (0x20000000U)
#define FLEXCAN_FLT_ID2_IDMASK_RTR_MSK_SHIFT     (29U)
#define FLEXCAN_FLT_ID2_IDMASK_RTR_MSK_WIDTH     (1U)
#define FLEXCAN_FLT_ID2_IDMASK_RTR_MSK(x)        (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID2_IDMASK_RTR_MSK_SHIFT)) & FLEXCAN_FLT_ID2_IDMASK_RTR_MSK_MASK)

#define FLEXCAN_FLT_ID2_IDMASK_IDE_MSK_MASK      (0x40000000U)
#define FLEXCAN_FLT_ID2_IDMASK_IDE_MSK_SHIFT     (30U)
#define FLEXCAN_FLT_ID2_IDMASK_IDE_MSK_WIDTH     (1U)
#define FLEXCAN_FLT_ID2_IDMASK_IDE_MSK(x)        (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FLT_ID2_IDMASK_IDE_MSK_SHIFT)) & FLEXCAN_FLT_ID2_IDMASK_IDE_MSK_MASK)
/*! @} */

/*! @name PL2_PLMASK_LO - Pretended Networking Payload Low Filter 2 Register / Payload Low Mask register */
/*! @{ */

#define FLEXCAN_PL2_PLMASK_LO_Data_byte_3_MASK   (0xFFU)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_3_SHIFT  (0U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_3_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_3(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_LO_Data_byte_3_SHIFT)) & FLEXCAN_PL2_PLMASK_LO_Data_byte_3_MASK)

#define FLEXCAN_PL2_PLMASK_LO_Data_byte_2_MASK   (0xFF00U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_2_SHIFT  (8U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_2_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_2(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_LO_Data_byte_2_SHIFT)) & FLEXCAN_PL2_PLMASK_LO_Data_byte_2_MASK)

#define FLEXCAN_PL2_PLMASK_LO_Data_byte_1_MASK   (0xFF0000U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_1_SHIFT  (16U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_1_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_1(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_LO_Data_byte_1_SHIFT)) & FLEXCAN_PL2_PLMASK_LO_Data_byte_1_MASK)

#define FLEXCAN_PL2_PLMASK_LO_Data_byte_0_MASK   (0xFF000000U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_0_SHIFT  (24U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_0_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_LO_Data_byte_0(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_LO_Data_byte_0_SHIFT)) & FLEXCAN_PL2_PLMASK_LO_Data_byte_0_MASK)
/*! @} */

/*! @name PL2_PLMASK_HI - Pretended Networking Payload High Filter 2 low order bits / Payload High Mask register */
/*! @{ */

#define FLEXCAN_PL2_PLMASK_HI_Data_byte_7_MASK   (0xFFU)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_7_SHIFT  (0U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_7_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_7(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_HI_Data_byte_7_SHIFT)) & FLEXCAN_PL2_PLMASK_HI_Data_byte_7_MASK)

#define FLEXCAN_PL2_PLMASK_HI_Data_byte_6_MASK   (0xFF00U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_6_SHIFT  (8U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_6_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_6(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_HI_Data_byte_6_SHIFT)) & FLEXCAN_PL2_PLMASK_HI_Data_byte_6_MASK)

#define FLEXCAN_PL2_PLMASK_HI_Data_byte_5_MASK   (0xFF0000U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_5_SHIFT  (16U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_5_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_5(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_HI_Data_byte_5_SHIFT)) & FLEXCAN_PL2_PLMASK_HI_Data_byte_5_MASK)

#define FLEXCAN_PL2_PLMASK_HI_Data_byte_4_MASK   (0xFF000000U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_4_SHIFT  (24U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_4_WIDTH  (8U)
#define FLEXCAN_PL2_PLMASK_HI_Data_byte_4(x)     (((uint32_t)(((uint32_t)(x)) << FLEXCAN_PL2_PLMASK_HI_Data_byte_4_SHIFT)) & FLEXCAN_PL2_PLMASK_HI_Data_byte_4_MASK)
/*! @} */

/*! @name WMBn_CS - Wake Up Message Buffer register for C/S */
/*! @{ */

#define FLEXCAN_WMBn_CS_DLC_MASK                 (0xF0000U)
#define FLEXCAN_WMBn_CS_DLC_SHIFT                (16U)
#define FLEXCAN_WMBn_CS_DLC_WIDTH                (4U)
#define FLEXCAN_WMBn_CS_DLC(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_CS_DLC_SHIFT)) & FLEXCAN_WMBn_CS_DLC_MASK)

#define FLEXCAN_WMBn_CS_RTR_MASK                 (0x100000U)
#define FLEXCAN_WMBn_CS_RTR_SHIFT                (20U)
#define FLEXCAN_WMBn_CS_RTR_WIDTH                (1U)
#define FLEXCAN_WMBn_CS_RTR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_CS_RTR_SHIFT)) & FLEXCAN_WMBn_CS_RTR_MASK)

#define FLEXCAN_WMBn_CS_IDE_MASK                 (0x200000U)
#define FLEXCAN_WMBn_CS_IDE_SHIFT                (21U)
#define FLEXCAN_WMBn_CS_IDE_WIDTH                (1U)
#define FLEXCAN_WMBn_CS_IDE(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_CS_IDE_SHIFT)) & FLEXCAN_WMBn_CS_IDE_MASK)

#define FLEXCAN_WMBn_CS_SRR_MASK                 (0x400000U)
#define FLEXCAN_WMBn_CS_SRR_SHIFT                (22U)
#define FLEXCAN_WMBn_CS_SRR_WIDTH                (1U)
#define FLEXCAN_WMBn_CS_SRR(x)                   (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_CS_SRR_SHIFT)) & FLEXCAN_WMBn_CS_SRR_MASK)
/*! @} */

/*! @name WMBn_ID - Wake Up Message Buffer Register for ID */
/*! @{ */

#define FLEXCAN_WMBn_ID_ID_MASK                  (0x1FFFFFFFU)
#define FLEXCAN_WMBn_ID_ID_SHIFT                 (0U)
#define FLEXCAN_WMBn_ID_ID_WIDTH                 (29U)
#define FLEXCAN_WMBn_ID_ID(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_ID_ID_SHIFT)) & FLEXCAN_WMBn_ID_ID_MASK)
/*! @} */

/*! @name WMBn_D03 - Wake Up Message Buffer Register for Data 0-3 */
/*! @{ */

#define FLEXCAN_WMBn_D03_Data_byte_3_MASK        (0xFFU)
#define FLEXCAN_WMBn_D03_Data_byte_3_SHIFT       (0U)
#define FLEXCAN_WMBn_D03_Data_byte_3_WIDTH       (8U)
#define FLEXCAN_WMBn_D03_Data_byte_3(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D03_Data_byte_3_SHIFT)) & FLEXCAN_WMBn_D03_Data_byte_3_MASK)

#define FLEXCAN_WMBn_D03_Data_byte_2_MASK        (0xFF00U)
#define FLEXCAN_WMBn_D03_Data_byte_2_SHIFT       (8U)
#define FLEXCAN_WMBn_D03_Data_byte_2_WIDTH       (8U)
#define FLEXCAN_WMBn_D03_Data_byte_2(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D03_Data_byte_2_SHIFT)) & FLEXCAN_WMBn_D03_Data_byte_2_MASK)

#define FLEXCAN_WMBn_D03_Data_byte_1_MASK        (0xFF0000U)
#define FLEXCAN_WMBn_D03_Data_byte_1_SHIFT       (16U)
#define FLEXCAN_WMBn_D03_Data_byte_1_WIDTH       (8U)
#define FLEXCAN_WMBn_D03_Data_byte_1(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D03_Data_byte_1_SHIFT)) & FLEXCAN_WMBn_D03_Data_byte_1_MASK)

#define FLEXCAN_WMBn_D03_Data_byte_0_MASK        (0xFF000000U)
#define FLEXCAN_WMBn_D03_Data_byte_0_SHIFT       (24U)
#define FLEXCAN_WMBn_D03_Data_byte_0_WIDTH       (8U)
#define FLEXCAN_WMBn_D03_Data_byte_0(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D03_Data_byte_0_SHIFT)) & FLEXCAN_WMBn_D03_Data_byte_0_MASK)
/*! @} */

/*! @name WMBn_D47 - Wake Up Message Buffer Register Data 4-7 */
/*! @{ */

#define FLEXCAN_WMBn_D47_Data_byte_7_MASK        (0xFFU)
#define FLEXCAN_WMBn_D47_Data_byte_7_SHIFT       (0U)
#define FLEXCAN_WMBn_D47_Data_byte_7_WIDTH       (8U)
#define FLEXCAN_WMBn_D47_Data_byte_7(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D47_Data_byte_7_SHIFT)) & FLEXCAN_WMBn_D47_Data_byte_7_MASK)

#define FLEXCAN_WMBn_D47_Data_byte_6_MASK        (0xFF00U)
#define FLEXCAN_WMBn_D47_Data_byte_6_SHIFT       (8U)
#define FLEXCAN_WMBn_D47_Data_byte_6_WIDTH       (8U)
#define FLEXCAN_WMBn_D47_Data_byte_6(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D47_Data_byte_6_SHIFT)) & FLEXCAN_WMBn_D47_Data_byte_6_MASK)

#define FLEXCAN_WMBn_D47_Data_byte_5_MASK        (0xFF0000U)
#define FLEXCAN_WMBn_D47_Data_byte_5_SHIFT       (16U)
#define FLEXCAN_WMBn_D47_Data_byte_5_WIDTH       (8U)
#define FLEXCAN_WMBn_D47_Data_byte_5(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D47_Data_byte_5_SHIFT)) & FLEXCAN_WMBn_D47_Data_byte_5_MASK)

#define FLEXCAN_WMBn_D47_Data_byte_4_MASK        (0xFF000000U)
#define FLEXCAN_WMBn_D47_Data_byte_4_SHIFT       (24U)
#define FLEXCAN_WMBn_D47_Data_byte_4_WIDTH       (8U)
#define FLEXCAN_WMBn_D47_Data_byte_4(x)          (((uint32_t)(((uint32_t)(x)) << FLEXCAN_WMBn_D47_Data_byte_4_SHIFT)) & FLEXCAN_WMBn_D47_Data_byte_4_MASK)
/*! @} */

/*! @name FDCTRL - CAN FD Control Register */
/*! @{ */

#define FLEXCAN_FDCTRL_TDCVAL_MASK               (0x3FU)
#define FLEXCAN_FDCTRL_TDCVAL_SHIFT              (0U)
#define FLEXCAN_FDCTRL_TDCVAL_WIDTH              (6U)
#define FLEXCAN_FDCTRL_TDCVAL(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_TDCVAL_SHIFT)) & FLEXCAN_FDCTRL_TDCVAL_MASK)

#define FLEXCAN_FDCTRL_TDCOFF_MASK               (0x1F00U)
#define FLEXCAN_FDCTRL_TDCOFF_SHIFT              (8U)
#define FLEXCAN_FDCTRL_TDCOFF_WIDTH              (5U)
#define FLEXCAN_FDCTRL_TDCOFF(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_TDCOFF_SHIFT)) & FLEXCAN_FDCTRL_TDCOFF_MASK)

#define FLEXCAN_FDCTRL_TDCFAIL_MASK              (0x4000U)
#define FLEXCAN_FDCTRL_TDCFAIL_SHIFT             (14U)
#define FLEXCAN_FDCTRL_TDCFAIL_WIDTH             (1U)
#define FLEXCAN_FDCTRL_TDCFAIL(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_TDCFAIL_SHIFT)) & FLEXCAN_FDCTRL_TDCFAIL_MASK)

#define FLEXCAN_FDCTRL_TDCEN_MASK                (0x8000U)
#define FLEXCAN_FDCTRL_TDCEN_SHIFT               (15U)
#define FLEXCAN_FDCTRL_TDCEN_WIDTH               (1U)
#define FLEXCAN_FDCTRL_TDCEN(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_TDCEN_SHIFT)) & FLEXCAN_FDCTRL_TDCEN_MASK)

#define FLEXCAN_FDCTRL_MBDSR0_MASK               (0x30000U)
#define FLEXCAN_FDCTRL_MBDSR0_SHIFT              (16U)
#define FLEXCAN_FDCTRL_MBDSR0_WIDTH              (2U)
#define FLEXCAN_FDCTRL_MBDSR0(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_MBDSR0_SHIFT)) & FLEXCAN_FDCTRL_MBDSR0_MASK)

#define FLEXCAN_FDCTRL_FDRATE_MASK               (0x80000000U)
#define FLEXCAN_FDCTRL_FDRATE_SHIFT              (31U)
#define FLEXCAN_FDCTRL_FDRATE_WIDTH              (1U)
#define FLEXCAN_FDCTRL_FDRATE(x)                 (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCTRL_FDRATE_SHIFT)) & FLEXCAN_FDCTRL_FDRATE_MASK)
/*! @} */

/*! @name FDCBT - CAN FD Bit Timing Register */
/*! @{ */

#define FLEXCAN_FDCBT_FPSEG2_MASK                (0x7U)
#define FLEXCAN_FDCBT_FPSEG2_SHIFT               (0U)
#define FLEXCAN_FDCBT_FPSEG2_WIDTH               (3U)
#define FLEXCAN_FDCBT_FPSEG2(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCBT_FPSEG2_SHIFT)) & FLEXCAN_FDCBT_FPSEG2_MASK)

#define FLEXCAN_FDCBT_FPSEG1_MASK                (0xE0U)
#define FLEXCAN_FDCBT_FPSEG1_SHIFT               (5U)
#define FLEXCAN_FDCBT_FPSEG1_WIDTH               (3U)
#define FLEXCAN_FDCBT_FPSEG1(x)                  (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCBT_FPSEG1_SHIFT)) & FLEXCAN_FDCBT_FPSEG1_MASK)

#define FLEXCAN_FDCBT_FPROPSEG_MASK              (0x7C00U)
#define FLEXCAN_FDCBT_FPROPSEG_SHIFT             (10U)
#define FLEXCAN_FDCBT_FPROPSEG_WIDTH             (5U)
#define FLEXCAN_FDCBT_FPROPSEG(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCBT_FPROPSEG_SHIFT)) & FLEXCAN_FDCBT_FPROPSEG_MASK)

#define FLEXCAN_FDCBT_FRJW_MASK                  (0x70000U)
#define FLEXCAN_FDCBT_FRJW_SHIFT                 (16U)
#define FLEXCAN_FDCBT_FRJW_WIDTH                 (3U)
#define FLEXCAN_FDCBT_FRJW(x)                    (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCBT_FRJW_SHIFT)) & FLEXCAN_FDCBT_FRJW_MASK)

#define FLEXCAN_FDCBT_FPRESDIV_MASK              (0x3FF00000U)
#define FLEXCAN_FDCBT_FPRESDIV_SHIFT             (20U)
#define FLEXCAN_FDCBT_FPRESDIV_WIDTH             (10U)
#define FLEXCAN_FDCBT_FPRESDIV(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCBT_FPRESDIV_SHIFT)) & FLEXCAN_FDCBT_FPRESDIV_MASK)
/*! @} */

/*! @name FDCRC - CAN FD CRC Register */
/*! @{ */

#define FLEXCAN_FDCRC_FD_TXCRC_MASK              (0x1FFFFFU)
#define FLEXCAN_FDCRC_FD_TXCRC_SHIFT             (0U)
#define FLEXCAN_FDCRC_FD_TXCRC_WIDTH             (21U)
#define FLEXCAN_FDCRC_FD_TXCRC(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCRC_FD_TXCRC_SHIFT)) & FLEXCAN_FDCRC_FD_TXCRC_MASK)

#define FLEXCAN_FDCRC_FD_MBCRC_MASK              (0x7F000000U)
#define FLEXCAN_FDCRC_FD_MBCRC_SHIFT             (24U)
#define FLEXCAN_FDCRC_FD_MBCRC_WIDTH             (7U)
#define FLEXCAN_FDCRC_FD_MBCRC(x)                (((uint32_t)(((uint32_t)(x)) << FLEXCAN_FDCRC_FD_MBCRC_SHIFT)) & FLEXCAN_FDCRC_FD_MBCRC_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group FLEXCAN_Register_Masks */

/*!
 * @}
 */ /* end of group FLEXCAN_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_FLEXCAN_H_) */

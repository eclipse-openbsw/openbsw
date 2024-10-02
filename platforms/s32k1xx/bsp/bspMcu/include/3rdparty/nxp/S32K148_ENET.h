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
 * @file S32K148_ENET.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_ENET
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
#if !defined(S32K148_ENET_H_)  /* Check if memory map has not been already included */
#define S32K148_ENET_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- ENET Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ENET_Peripheral_Access_Layer ENET Peripheral Access Layer
 * @{
 */

/** ENET - Size of Registers Arrays */
#define ENET_TC_COUNT                             4u

/** ENET - Register Layout Typedef */
typedef struct {
  uint8_t RESERVED_0[4];
  __IO uint32_t EIR;                               /**< Interrupt Event Register, offset: 0x4 */
  __IO uint32_t EIMR;                              /**< Interrupt Mask Register, offset: 0x8 */
  uint8_t RESERVED_1[4];
  __IO uint32_t RDAR;                              /**< Receive Descriptor Active Register, offset: 0x10 */
  __IO uint32_t TDAR;                              /**< Transmit Descriptor Active Register, offset: 0x14 */
  uint8_t RESERVED_2[12];
  __IO uint32_t ECR;                               /**< Ethernet Control Register, offset: 0x24 */
  uint8_t RESERVED_3[24];
  __IO uint32_t MMFR;                              /**< MII Management Frame Register, offset: 0x40 */
  __IO uint32_t MSCR;                              /**< MII Speed Control Register, offset: 0x44 */
  uint8_t RESERVED_4[28];
  __IO uint32_t MIBC;                              /**< MIB Control Register, offset: 0x64 */
  uint8_t RESERVED_5[28];
  __IO uint32_t RCR;                               /**< Receive Control Register, offset: 0x84 */
  uint8_t RESERVED_6[60];
  __IO uint32_t TCR;                               /**< Transmit Control Register, offset: 0xC4 */
  uint8_t RESERVED_7[28];
  __IO uint32_t PALR;                              /**< Physical Address Lower Register, offset: 0xE4 */
  __IO uint32_t PAUR;                              /**< Physical Address Upper Register, offset: 0xE8 */
  __IO uint32_t OPD;                               /**< Opcode/Pause Duration Register, offset: 0xEC */
  uint8_t RESERVED_8[40];
  __IO uint32_t IAUR;                              /**< Descriptor Individual Upper Address Register, offset: 0x118 */
  __IO uint32_t IALR;                              /**< Descriptor Individual Lower Address Register, offset: 0x11C */
  __IO uint32_t GAUR;                              /**< Descriptor Group Upper Address Register, offset: 0x120 */
  __IO uint32_t GALR;                              /**< Descriptor Group Lower Address Register, offset: 0x124 */
  uint8_t RESERVED_9[28];
  __IO uint32_t TFWR;                              /**< Transmit FIFO Watermark Register, offset: 0x144 */
  uint8_t RESERVED_10[56];
  __IO uint32_t RDSR;                              /**< Receive Descriptor Ring Start Register, offset: 0x180 */
  __IO uint32_t TDSR;                              /**< Transmit Buffer Descriptor Ring Start Register, offset: 0x184 */
  __IO uint32_t MRBR;                              /**< Maximum Receive Buffer Size Register, offset: 0x188 */
  uint8_t RESERVED_11[4];
  __IO uint32_t RSFL;                              /**< Receive FIFO Section Full Threshold, offset: 0x190 */
  __IO uint32_t RSEM;                              /**< Receive FIFO Section Empty Threshold, offset: 0x194 */
  __IO uint32_t RAEM;                              /**< Receive FIFO Almost Empty Threshold, offset: 0x198 */
  __IO uint32_t RAFL;                              /**< Receive FIFO Almost Full Threshold, offset: 0x19C */
  __IO uint32_t TSEM;                              /**< Transmit FIFO Section Empty Threshold, offset: 0x1A0 */
  __IO uint32_t TAEM;                              /**< Transmit FIFO Almost Empty Threshold, offset: 0x1A4 */
  __IO uint32_t TAFL;                              /**< Transmit FIFO Almost Full Threshold, offset: 0x1A8 */
  __IO uint32_t TIPG;                              /**< Transmit Inter-Packet Gap, offset: 0x1AC */
  __IO uint32_t FTRL;                              /**< Frame Truncation Length, offset: 0x1B0 */
  uint8_t RESERVED_12[12];
  __IO uint32_t TACC;                              /**< Transmit Accelerator Function Configuration, offset: 0x1C0 */
  __IO uint32_t RACC;                              /**< Receive Accelerator Function Configuration, offset: 0x1C4 */
  uint8_t RESERVED_13[56];
       uint32_t RMON_T_DROP;                       /**< Reserved Statistic Register, offset: 0x200 */
  __I  uint32_t RMON_T_PACKETS;                    /**< Tx Packet Count Statistic Register, offset: 0x204 */
  __I  uint32_t RMON_T_BC_PKT;                     /**< Tx Broadcast Packets Statistic Register, offset: 0x208 */
  __I  uint32_t RMON_T_MC_PKT;                     /**< Tx Multicast Packets Statistic Register, offset: 0x20C */
  __I  uint32_t RMON_T_CRC_ALIGN;                  /**< Tx Packets with CRC/Align Error Statistic Register, offset: 0x210 */
  __I  uint32_t RMON_T_UNDERSIZE;                  /**< Tx Packets Less Than Bytes and Good CRC Statistic Register, offset: 0x214 */
  __I  uint32_t RMON_T_OVERSIZE;                   /**< Tx Packets GT MAX_FL bytes and Good CRC Statistic Register, offset: 0x218 */
  __I  uint32_t RMON_T_FRAG;                       /**< Tx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x21C */
  __I  uint32_t RMON_T_JAB;                        /**< Tx Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register, offset: 0x220 */
  __I  uint32_t RMON_T_COL;                        /**< Tx Collision Count Statistic Register, offset: 0x224 */
  __I  uint32_t RMON_T_P64;                        /**< Tx 64-Byte Packets Statistic Register, offset: 0x228 */
  __I  uint32_t RMON_T_P65TO127;                   /**< Tx 65- to 127-byte Packets Statistic Register, offset: 0x22C */
  __I  uint32_t RMON_T_P128TO255;                  /**< Tx 128- to 255-byte Packets Statistic Register, offset: 0x230 */
  __I  uint32_t RMON_T_P256TO511;                  /**< Tx 256- to 511-byte Packets Statistic Register, offset: 0x234 */
  __I  uint32_t RMON_T_P512TO1023;                 /**< Tx 512- to 1023-byte Packets Statistic Register, offset: 0x238 */
  __I  uint32_t RMON_T_P1024TO2047;                /**< Tx 1024- to 2047-byte Packets Statistic Register, offset: 0x23C */
  __I  uint32_t RMON_T_P_GTE2048;                  /**< Tx Packets Greater Than 2048 Bytes Statistic Register, offset: 0x240 */
  __I  uint32_t RMON_T_OCTETS;                     /**< Tx Octets Statistic Register, offset: 0x244 */
       uint32_t IEEE_T_DROP;                       /**< Reserved Statistic Register, offset: 0x248 */
  __I  uint32_t IEEE_T_FRAME_OK;                   /**< Frames Transmitted OK Statistic Register, offset: 0x24C */
  __I  uint32_t IEEE_T_1COL;                       /**< Frames Transmitted with Single Collision Statistic Register, offset: 0x250 */
  __I  uint32_t IEEE_T_MCOL;                       /**< Frames Transmitted with Multiple Collisions Statistic Register, offset: 0x254 */
  __I  uint32_t IEEE_T_DEF;                        /**< Frames Transmitted after Deferral Delay Statistic Register, offset: 0x258 */
  __I  uint32_t IEEE_T_LCOL;                       /**< Frames Transmitted with Late Collision Statistic Register, offset: 0x25C */
  __I  uint32_t IEEE_T_EXCOL;                      /**< Frames Transmitted with Excessive Collisions Statistic Register, offset: 0x260 */
  __I  uint32_t IEEE_T_MACERR;                     /**< Frames Transmitted with Tx FIFO Underrun Statistic Register, offset: 0x264 */
  __I  uint32_t IEEE_T_CSERR;                      /**< Frames Transmitted with Carrier Sense Error Statistic Register, offset: 0x268 */
  __I  uint32_t IEEE_T_SQE;                        /**< Reserved Statistic Register, offset: 0x26C */
  __I  uint32_t IEEE_T_FDXFC;                      /**< Flow Control Pause Frames Transmitted Statistic Register, offset: 0x270 */
  __I  uint32_t IEEE_T_OCTETS_OK;                  /**< Octet Count for Frames Transmitted w/o Error Statistic Register, offset: 0x274 */
  uint8_t RESERVED_14[12];
  __I  uint32_t RMON_R_PACKETS;                    /**< Rx Packet Count Statistic Register, offset: 0x284 */
  __I  uint32_t RMON_R_BC_PKT;                     /**< Rx Broadcast Packets Statistic Register, offset: 0x288 */
  __I  uint32_t RMON_R_MC_PKT;                     /**< Rx Multicast Packets Statistic Register, offset: 0x28C */
  __I  uint32_t RMON_R_CRC_ALIGN;                  /**< Rx Packets with CRC/Align Error Statistic Register, offset: 0x290 */
  __I  uint32_t RMON_R_UNDERSIZE;                  /**< Rx Packets with Less Than 64 Bytes and Good CRC Statistic Register, offset: 0x294 */
  __I  uint32_t RMON_R_OVERSIZE;                   /**< Rx Packets Greater Than MAX_FL and Good CRC Statistic Register, offset: 0x298 */
  __I  uint32_t RMON_R_FRAG;                       /**< Rx Packets Less Than 64 Bytes and Bad CRC Statistic Register, offset: 0x29C */
  __I  uint32_t RMON_R_JAB;                        /**< Rx Packets Greater Than MAX_FL Bytes and Bad CRC Statistic Register, offset: 0x2A0 */
       uint32_t RMON_R_RESVD_0;                    /**< Reserved Statistic Register, offset: 0x2A4 */
  __I  uint32_t RMON_R_P64;                        /**< Rx 64-Byte Packets Statistic Register, offset: 0x2A8 */
  __I  uint32_t RMON_R_P65TO127;                   /**< Rx 65- to 127-Byte Packets Statistic Register, offset: 0x2AC */
  __I  uint32_t RMON_R_P128TO255;                  /**< Rx 128- to 255-Byte Packets Statistic Register, offset: 0x2B0 */
  __I  uint32_t RMON_R_P256TO511;                  /**< Rx 256- to 511-Byte Packets Statistic Register, offset: 0x2B4 */
  __I  uint32_t RMON_R_P512TO1023;                 /**< Rx 512- to 1023-Byte Packets Statistic Register, offset: 0x2B8 */
  __I  uint32_t RMON_R_P1024TO2047;                /**< Rx 1024- to 2047-Byte Packets Statistic Register, offset: 0x2BC */
  __I  uint32_t RMON_R_P_GTE2048;                  /**< Rx Packets Greater than 2048 Bytes Statistic Register, offset: 0x2C0 */
  __I  uint32_t RMON_R_OCTETS;                     /**< Rx Octets Statistic Register, offset: 0x2C4 */
  __I  uint32_t IEEE_R_DROP;                       /**< Frames not Counted Correctly Statistic Register, offset: 0x2C8 */
  __I  uint32_t IEEE_R_FRAME_OK;                   /**< Frames Received OK Statistic Register, offset: 0x2CC */
  __I  uint32_t IEEE_R_CRC;                        /**< Frames Received with CRC Error Statistic Register, offset: 0x2D0 */
  __I  uint32_t IEEE_R_ALIGN;                      /**< Frames Received with Alignment Error Statistic Register, offset: 0x2D4 */
  __I  uint32_t IEEE_R_MACERR;                     /**< Receive FIFO Overflow Count Statistic Register, offset: 0x2D8 */
  __I  uint32_t IEEE_R_FDXFC;                      /**< Flow Control Pause Frames Received Statistic Register, offset: 0x2DC */
  __I  uint32_t IEEE_R_OCTETS_OK;                  /**< Octet Count for Frames Received without Error Statistic Register, offset: 0x2E0 */
  uint8_t RESERVED_15[284];
  __IO uint32_t ATCR;                              /**< Adjustable Timer Control Register, offset: 0x400 */
  __IO uint32_t ATVR;                              /**< Timer Value Register, offset: 0x404 */
  __IO uint32_t ATOFF;                             /**< Timer Offset Register, offset: 0x408 */
  __IO uint32_t ATPER;                             /**< Timer Period Register, offset: 0x40C */
  __IO uint32_t ATCOR;                             /**< Timer Correction Register, offset: 0x410 */
  __IO uint32_t ATINC;                             /**< Time-Stamping Clock Period Register, offset: 0x414 */
  __I  uint32_t ATSTMP;                            /**< Timestamp of Last Transmitted Frame, offset: 0x418 */
  uint8_t RESERVED_16[488];
  __IO uint32_t TGSR;                              /**< Timer Global Status Register, offset: 0x604 */
  struct {                                         /* offset: 0x608, array step: 0x8 */
    __IO uint32_t TCSR;                              /**< Timer Control Status Register, array offset: 0x608, array step: 0x8 */
    __IO uint32_t TCCR;                              /**< Timer Compare Capture Register, array offset: 0x60C, array step: 0x8 */
  } TC[ENET_TC_COUNT];
} ENET_Type, *ENET_MemMapPtr;

/** Number of instances of the ENET module. */
#define ENET_INSTANCE_COUNT                      (1u)

/* ENET - Peripheral instance base addresses */
/** Peripheral ENET base address */
#define IP_ENET_BASE                             (0x40079000u)
/** Peripheral ENET base pointer */
#define IP_ENET                                  ((ENET_Type *)IP_ENET_BASE)
/** Array initializer of ENET peripheral base addresses */
#define IP_ENET_BASE_ADDRS                       { IP_ENET_BASE }
/** Array initializer of ENET peripheral base pointers */
#define IP_ENET_BASE_PTRS                        { IP_ENET }

/* ----------------------------------------------------------------------------
   -- ENET Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ENET_Register_Masks ENET Register Masks
 * @{
 */

/*! @name EIR - Interrupt Event Register */
/*! @{ */

#define ENET_EIR_TS_TIMER_MASK                   (0x8000U)
#define ENET_EIR_TS_TIMER_SHIFT                  (15U)
#define ENET_EIR_TS_TIMER_WIDTH                  (1U)
#define ENET_EIR_TS_TIMER(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_EIR_TS_TIMER_SHIFT)) & ENET_EIR_TS_TIMER_MASK)

#define ENET_EIR_TS_AVAIL_MASK                   (0x10000U)
#define ENET_EIR_TS_AVAIL_SHIFT                  (16U)
#define ENET_EIR_TS_AVAIL_WIDTH                  (1U)
#define ENET_EIR_TS_AVAIL(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_EIR_TS_AVAIL_SHIFT)) & ENET_EIR_TS_AVAIL_MASK)

#define ENET_EIR_WAKEUP_MASK                     (0x20000U)
#define ENET_EIR_WAKEUP_SHIFT                    (17U)
#define ENET_EIR_WAKEUP_WIDTH                    (1U)
#define ENET_EIR_WAKEUP(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_EIR_WAKEUP_SHIFT)) & ENET_EIR_WAKEUP_MASK)

#define ENET_EIR_PLR_MASK                        (0x40000U)
#define ENET_EIR_PLR_SHIFT                       (18U)
#define ENET_EIR_PLR_WIDTH                       (1U)
#define ENET_EIR_PLR(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_PLR_SHIFT)) & ENET_EIR_PLR_MASK)

#define ENET_EIR_UN_MASK                         (0x80000U)
#define ENET_EIR_UN_SHIFT                        (19U)
#define ENET_EIR_UN_WIDTH                        (1U)
#define ENET_EIR_UN(x)                           (((uint32_t)(((uint32_t)(x)) << ENET_EIR_UN_SHIFT)) & ENET_EIR_UN_MASK)

#define ENET_EIR_RL_MASK                         (0x100000U)
#define ENET_EIR_RL_SHIFT                        (20U)
#define ENET_EIR_RL_WIDTH                        (1U)
#define ENET_EIR_RL(x)                           (((uint32_t)(((uint32_t)(x)) << ENET_EIR_RL_SHIFT)) & ENET_EIR_RL_MASK)

#define ENET_EIR_LC_MASK                         (0x200000U)
#define ENET_EIR_LC_SHIFT                        (21U)
#define ENET_EIR_LC_WIDTH                        (1U)
#define ENET_EIR_LC(x)                           (((uint32_t)(((uint32_t)(x)) << ENET_EIR_LC_SHIFT)) & ENET_EIR_LC_MASK)

#define ENET_EIR_EBERR_MASK                      (0x400000U)
#define ENET_EIR_EBERR_SHIFT                     (22U)
#define ENET_EIR_EBERR_WIDTH                     (1U)
#define ENET_EIR_EBERR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_EIR_EBERR_SHIFT)) & ENET_EIR_EBERR_MASK)

#define ENET_EIR_MII_MASK                        (0x800000U)
#define ENET_EIR_MII_SHIFT                       (23U)
#define ENET_EIR_MII_WIDTH                       (1U)
#define ENET_EIR_MII(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_MII_SHIFT)) & ENET_EIR_MII_MASK)

#define ENET_EIR_RXB_MASK                        (0x1000000U)
#define ENET_EIR_RXB_SHIFT                       (24U)
#define ENET_EIR_RXB_WIDTH                       (1U)
#define ENET_EIR_RXB(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_RXB_SHIFT)) & ENET_EIR_RXB_MASK)

#define ENET_EIR_RXF_MASK                        (0x2000000U)
#define ENET_EIR_RXF_SHIFT                       (25U)
#define ENET_EIR_RXF_WIDTH                       (1U)
#define ENET_EIR_RXF(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_RXF_SHIFT)) & ENET_EIR_RXF_MASK)

#define ENET_EIR_TXB_MASK                        (0x4000000U)
#define ENET_EIR_TXB_SHIFT                       (26U)
#define ENET_EIR_TXB_WIDTH                       (1U)
#define ENET_EIR_TXB(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_TXB_SHIFT)) & ENET_EIR_TXB_MASK)

#define ENET_EIR_TXF_MASK                        (0x8000000U)
#define ENET_EIR_TXF_SHIFT                       (27U)
#define ENET_EIR_TXF_WIDTH                       (1U)
#define ENET_EIR_TXF(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_TXF_SHIFT)) & ENET_EIR_TXF_MASK)

#define ENET_EIR_GRA_MASK                        (0x10000000U)
#define ENET_EIR_GRA_SHIFT                       (28U)
#define ENET_EIR_GRA_WIDTH                       (1U)
#define ENET_EIR_GRA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIR_GRA_SHIFT)) & ENET_EIR_GRA_MASK)

#define ENET_EIR_BABT_MASK                       (0x20000000U)
#define ENET_EIR_BABT_SHIFT                      (29U)
#define ENET_EIR_BABT_WIDTH                      (1U)
#define ENET_EIR_BABT(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIR_BABT_SHIFT)) & ENET_EIR_BABT_MASK)

#define ENET_EIR_BABR_MASK                       (0x40000000U)
#define ENET_EIR_BABR_SHIFT                      (30U)
#define ENET_EIR_BABR_WIDTH                      (1U)
#define ENET_EIR_BABR(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIR_BABR_SHIFT)) & ENET_EIR_BABR_MASK)
/*! @} */

/*! @name EIMR - Interrupt Mask Register */
/*! @{ */

#define ENET_EIMR_TS_TIMER_MASK                  (0x8000U)
#define ENET_EIMR_TS_TIMER_SHIFT                 (15U)
#define ENET_EIMR_TS_TIMER_WIDTH                 (1U)
#define ENET_EIMR_TS_TIMER(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_TS_TIMER_SHIFT)) & ENET_EIMR_TS_TIMER_MASK)

#define ENET_EIMR_TS_AVAIL_MASK                  (0x10000U)
#define ENET_EIMR_TS_AVAIL_SHIFT                 (16U)
#define ENET_EIMR_TS_AVAIL_WIDTH                 (1U)
#define ENET_EIMR_TS_AVAIL(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_TS_AVAIL_SHIFT)) & ENET_EIMR_TS_AVAIL_MASK)

#define ENET_EIMR_WAKEUP_MASK                    (0x20000U)
#define ENET_EIMR_WAKEUP_SHIFT                   (17U)
#define ENET_EIMR_WAKEUP_WIDTH                   (1U)
#define ENET_EIMR_WAKEUP(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_WAKEUP_SHIFT)) & ENET_EIMR_WAKEUP_MASK)

#define ENET_EIMR_PLR_MASK                       (0x40000U)
#define ENET_EIMR_PLR_SHIFT                      (18U)
#define ENET_EIMR_PLR_WIDTH                      (1U)
#define ENET_EIMR_PLR(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_PLR_SHIFT)) & ENET_EIMR_PLR_MASK)

#define ENET_EIMR_UN_MASK                        (0x80000U)
#define ENET_EIMR_UN_SHIFT                       (19U)
#define ENET_EIMR_UN_WIDTH                       (1U)
#define ENET_EIMR_UN(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_UN_SHIFT)) & ENET_EIMR_UN_MASK)

#define ENET_EIMR_RL_MASK                        (0x100000U)
#define ENET_EIMR_RL_SHIFT                       (20U)
#define ENET_EIMR_RL_WIDTH                       (1U)
#define ENET_EIMR_RL(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_RL_SHIFT)) & ENET_EIMR_RL_MASK)

#define ENET_EIMR_LC_MASK                        (0x200000U)
#define ENET_EIMR_LC_SHIFT                       (21U)
#define ENET_EIMR_LC_WIDTH                       (1U)
#define ENET_EIMR_LC(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_LC_SHIFT)) & ENET_EIMR_LC_MASK)

#define ENET_EIMR_EBERR_MASK                     (0x400000U)
#define ENET_EIMR_EBERR_SHIFT                    (22U)
#define ENET_EIMR_EBERR_WIDTH                    (1U)
#define ENET_EIMR_EBERR(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_EBERR_SHIFT)) & ENET_EIMR_EBERR_MASK)

#define ENET_EIMR_MII_MASK                       (0x800000U)
#define ENET_EIMR_MII_SHIFT                      (23U)
#define ENET_EIMR_MII_WIDTH                      (1U)
#define ENET_EIMR_MII(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_MII_SHIFT)) & ENET_EIMR_MII_MASK)

#define ENET_EIMR_RXB_MASK                       (0x1000000U)
#define ENET_EIMR_RXB_SHIFT                      (24U)
#define ENET_EIMR_RXB_WIDTH                      (1U)
#define ENET_EIMR_RXB(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_RXB_SHIFT)) & ENET_EIMR_RXB_MASK)

#define ENET_EIMR_RXF_MASK                       (0x2000000U)
#define ENET_EIMR_RXF_SHIFT                      (25U)
#define ENET_EIMR_RXF_WIDTH                      (1U)
#define ENET_EIMR_RXF(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_RXF_SHIFT)) & ENET_EIMR_RXF_MASK)

#define ENET_EIMR_TXB_MASK                       (0x4000000U)
#define ENET_EIMR_TXB_SHIFT                      (26U)
#define ENET_EIMR_TXB_WIDTH                      (1U)
#define ENET_EIMR_TXB(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_TXB_SHIFT)) & ENET_EIMR_TXB_MASK)

#define ENET_EIMR_TXF_MASK                       (0x8000000U)
#define ENET_EIMR_TXF_SHIFT                      (27U)
#define ENET_EIMR_TXF_WIDTH                      (1U)
#define ENET_EIMR_TXF(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_TXF_SHIFT)) & ENET_EIMR_TXF_MASK)

#define ENET_EIMR_GRA_MASK                       (0x10000000U)
#define ENET_EIMR_GRA_SHIFT                      (28U)
#define ENET_EIMR_GRA_WIDTH                      (1U)
#define ENET_EIMR_GRA(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_GRA_SHIFT)) & ENET_EIMR_GRA_MASK)

#define ENET_EIMR_BABT_MASK                      (0x20000000U)
#define ENET_EIMR_BABT_SHIFT                     (29U)
#define ENET_EIMR_BABT_WIDTH                     (1U)
#define ENET_EIMR_BABT(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_BABT_SHIFT)) & ENET_EIMR_BABT_MASK)

#define ENET_EIMR_BABR_MASK                      (0x40000000U)
#define ENET_EIMR_BABR_SHIFT                     (30U)
#define ENET_EIMR_BABR_WIDTH                     (1U)
#define ENET_EIMR_BABR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_EIMR_BABR_SHIFT)) & ENET_EIMR_BABR_MASK)
/*! @} */

/*! @name RDAR - Receive Descriptor Active Register */
/*! @{ */

#define ENET_RDAR_RDAR_MASK                      (0x1000000U)
#define ENET_RDAR_RDAR_SHIFT                     (24U)
#define ENET_RDAR_RDAR_WIDTH                     (1U)
#define ENET_RDAR_RDAR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_RDAR_RDAR_SHIFT)) & ENET_RDAR_RDAR_MASK)
/*! @} */

/*! @name TDAR - Transmit Descriptor Active Register */
/*! @{ */

#define ENET_TDAR_TDAR_MASK                      (0x1000000U)
#define ENET_TDAR_TDAR_SHIFT                     (24U)
#define ENET_TDAR_TDAR_WIDTH                     (1U)
#define ENET_TDAR_TDAR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_TDAR_TDAR_SHIFT)) & ENET_TDAR_TDAR_MASK)
/*! @} */

/*! @name ECR - Ethernet Control Register */
/*! @{ */

#define ENET_ECR_RESET_MASK                      (0x1U)
#define ENET_ECR_RESET_SHIFT                     (0U)
#define ENET_ECR_RESET_WIDTH                     (1U)
#define ENET_ECR_RESET(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ECR_RESET_SHIFT)) & ENET_ECR_RESET_MASK)

#define ENET_ECR_ETHEREN_MASK                    (0x2U)
#define ENET_ECR_ETHEREN_SHIFT                   (1U)
#define ENET_ECR_ETHEREN_WIDTH                   (1U)
#define ENET_ECR_ETHEREN(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_ECR_ETHEREN_SHIFT)) & ENET_ECR_ETHEREN_MASK)

#define ENET_ECR_MAGICEN_MASK                    (0x4U)
#define ENET_ECR_MAGICEN_SHIFT                   (2U)
#define ENET_ECR_MAGICEN_WIDTH                   (1U)
#define ENET_ECR_MAGICEN(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_ECR_MAGICEN_SHIFT)) & ENET_ECR_MAGICEN_MASK)

#define ENET_ECR_SLEEP_MASK                      (0x8U)
#define ENET_ECR_SLEEP_SHIFT                     (3U)
#define ENET_ECR_SLEEP_WIDTH                     (1U)
#define ENET_ECR_SLEEP(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ECR_SLEEP_SHIFT)) & ENET_ECR_SLEEP_MASK)

#define ENET_ECR_EN1588_MASK                     (0x10U)
#define ENET_ECR_EN1588_SHIFT                    (4U)
#define ENET_ECR_EN1588_WIDTH                    (1U)
#define ENET_ECR_EN1588(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_ECR_EN1588_SHIFT)) & ENET_ECR_EN1588_MASK)

#define ENET_ECR_DBGEN_MASK                      (0x40U)
#define ENET_ECR_DBGEN_SHIFT                     (6U)
#define ENET_ECR_DBGEN_WIDTH                     (1U)
#define ENET_ECR_DBGEN(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ECR_DBGEN_SHIFT)) & ENET_ECR_DBGEN_MASK)

#define ENET_ECR_DBSWP_MASK                      (0x100U)
#define ENET_ECR_DBSWP_SHIFT                     (8U)
#define ENET_ECR_DBSWP_WIDTH                     (1U)
#define ENET_ECR_DBSWP(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ECR_DBSWP_SHIFT)) & ENET_ECR_DBSWP_MASK)
/*! @} */

/*! @name MMFR - MII Management Frame Register */
/*! @{ */

#define ENET_MMFR_DATA_MASK                      (0xFFFFU)
#define ENET_MMFR_DATA_SHIFT                     (0U)
#define ENET_MMFR_DATA_WIDTH                     (16U)
#define ENET_MMFR_DATA(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_DATA_SHIFT)) & ENET_MMFR_DATA_MASK)

#define ENET_MMFR_TA_MASK                        (0x30000U)
#define ENET_MMFR_TA_SHIFT                       (16U)
#define ENET_MMFR_TA_WIDTH                       (2U)
#define ENET_MMFR_TA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_TA_SHIFT)) & ENET_MMFR_TA_MASK)

#define ENET_MMFR_RA_MASK                        (0x7C0000U)
#define ENET_MMFR_RA_SHIFT                       (18U)
#define ENET_MMFR_RA_WIDTH                       (5U)
#define ENET_MMFR_RA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_RA_SHIFT)) & ENET_MMFR_RA_MASK)

#define ENET_MMFR_PA_MASK                        (0xF800000U)
#define ENET_MMFR_PA_SHIFT                       (23U)
#define ENET_MMFR_PA_WIDTH                       (5U)
#define ENET_MMFR_PA(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_PA_SHIFT)) & ENET_MMFR_PA_MASK)

#define ENET_MMFR_OP_MASK                        (0x30000000U)
#define ENET_MMFR_OP_SHIFT                       (28U)
#define ENET_MMFR_OP_WIDTH                       (2U)
#define ENET_MMFR_OP(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_OP_SHIFT)) & ENET_MMFR_OP_MASK)

#define ENET_MMFR_ST_MASK                        (0xC0000000U)
#define ENET_MMFR_ST_SHIFT                       (30U)
#define ENET_MMFR_ST_WIDTH                       (2U)
#define ENET_MMFR_ST(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_MMFR_ST_SHIFT)) & ENET_MMFR_ST_MASK)
/*! @} */

/*! @name MSCR - MII Speed Control Register */
/*! @{ */

#define ENET_MSCR_MII_SPEED_MASK                 (0x7EU)
#define ENET_MSCR_MII_SPEED_SHIFT                (1U)
#define ENET_MSCR_MII_SPEED_WIDTH                (6U)
#define ENET_MSCR_MII_SPEED(x)                   (((uint32_t)(((uint32_t)(x)) << ENET_MSCR_MII_SPEED_SHIFT)) & ENET_MSCR_MII_SPEED_MASK)

#define ENET_MSCR_DIS_PRE_MASK                   (0x80U)
#define ENET_MSCR_DIS_PRE_SHIFT                  (7U)
#define ENET_MSCR_DIS_PRE_WIDTH                  (1U)
#define ENET_MSCR_DIS_PRE(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_MSCR_DIS_PRE_SHIFT)) & ENET_MSCR_DIS_PRE_MASK)

#define ENET_MSCR_HOLDTIME_MASK                  (0x700U)
#define ENET_MSCR_HOLDTIME_SHIFT                 (8U)
#define ENET_MSCR_HOLDTIME_WIDTH                 (3U)
#define ENET_MSCR_HOLDTIME(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_MSCR_HOLDTIME_SHIFT)) & ENET_MSCR_HOLDTIME_MASK)
/*! @} */

/*! @name MIBC - MIB Control Register */
/*! @{ */

#define ENET_MIBC_MIB_CLEAR_MASK                 (0x20000000U)
#define ENET_MIBC_MIB_CLEAR_SHIFT                (29U)
#define ENET_MIBC_MIB_CLEAR_WIDTH                (1U)
#define ENET_MIBC_MIB_CLEAR(x)                   (((uint32_t)(((uint32_t)(x)) << ENET_MIBC_MIB_CLEAR_SHIFT)) & ENET_MIBC_MIB_CLEAR_MASK)

#define ENET_MIBC_MIB_IDLE_MASK                  (0x40000000U)
#define ENET_MIBC_MIB_IDLE_SHIFT                 (30U)
#define ENET_MIBC_MIB_IDLE_WIDTH                 (1U)
#define ENET_MIBC_MIB_IDLE(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_MIBC_MIB_IDLE_SHIFT)) & ENET_MIBC_MIB_IDLE_MASK)

#define ENET_MIBC_MIB_DIS_MASK                   (0x80000000U)
#define ENET_MIBC_MIB_DIS_SHIFT                  (31U)
#define ENET_MIBC_MIB_DIS_WIDTH                  (1U)
#define ENET_MIBC_MIB_DIS(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_MIBC_MIB_DIS_SHIFT)) & ENET_MIBC_MIB_DIS_MASK)
/*! @} */

/*! @name RCR - Receive Control Register */
/*! @{ */

#define ENET_RCR_LOOP_MASK                       (0x1U)
#define ENET_RCR_LOOP_SHIFT                      (0U)
#define ENET_RCR_LOOP_WIDTH                      (1U)
#define ENET_RCR_LOOP(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_RCR_LOOP_SHIFT)) & ENET_RCR_LOOP_MASK)

#define ENET_RCR_DRT_MASK                        (0x2U)
#define ENET_RCR_DRT_SHIFT                       (1U)
#define ENET_RCR_DRT_WIDTH                       (1U)
#define ENET_RCR_DRT(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_RCR_DRT_SHIFT)) & ENET_RCR_DRT_MASK)

#define ENET_RCR_MII_MODE_MASK                   (0x4U)
#define ENET_RCR_MII_MODE_SHIFT                  (2U)
#define ENET_RCR_MII_MODE_WIDTH                  (1U)
#define ENET_RCR_MII_MODE(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_RCR_MII_MODE_SHIFT)) & ENET_RCR_MII_MODE_MASK)

#define ENET_RCR_PROM_MASK                       (0x8U)
#define ENET_RCR_PROM_SHIFT                      (3U)
#define ENET_RCR_PROM_WIDTH                      (1U)
#define ENET_RCR_PROM(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_RCR_PROM_SHIFT)) & ENET_RCR_PROM_MASK)

#define ENET_RCR_BC_REJ_MASK                     (0x10U)
#define ENET_RCR_BC_REJ_SHIFT                    (4U)
#define ENET_RCR_BC_REJ_WIDTH                    (1U)
#define ENET_RCR_BC_REJ(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RCR_BC_REJ_SHIFT)) & ENET_RCR_BC_REJ_MASK)

#define ENET_RCR_FCE_MASK                        (0x20U)
#define ENET_RCR_FCE_SHIFT                       (5U)
#define ENET_RCR_FCE_WIDTH                       (1U)
#define ENET_RCR_FCE(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_RCR_FCE_SHIFT)) & ENET_RCR_FCE_MASK)

#define ENET_RCR_RMII_MODE_MASK                  (0x100U)
#define ENET_RCR_RMII_MODE_SHIFT                 (8U)
#define ENET_RCR_RMII_MODE_WIDTH                 (1U)
#define ENET_RCR_RMII_MODE(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_RCR_RMII_MODE_SHIFT)) & ENET_RCR_RMII_MODE_MASK)

#define ENET_RCR_RMII_10T_MASK                   (0x200U)
#define ENET_RCR_RMII_10T_SHIFT                  (9U)
#define ENET_RCR_RMII_10T_WIDTH                  (1U)
#define ENET_RCR_RMII_10T(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_RCR_RMII_10T_SHIFT)) & ENET_RCR_RMII_10T_MASK)

#define ENET_RCR_PADEN_MASK                      (0x1000U)
#define ENET_RCR_PADEN_SHIFT                     (12U)
#define ENET_RCR_PADEN_WIDTH                     (1U)
#define ENET_RCR_PADEN(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_RCR_PADEN_SHIFT)) & ENET_RCR_PADEN_MASK)

#define ENET_RCR_PAUFWD_MASK                     (0x2000U)
#define ENET_RCR_PAUFWD_SHIFT                    (13U)
#define ENET_RCR_PAUFWD_WIDTH                    (1U)
#define ENET_RCR_PAUFWD(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RCR_PAUFWD_SHIFT)) & ENET_RCR_PAUFWD_MASK)

#define ENET_RCR_CRCFWD_MASK                     (0x4000U)
#define ENET_RCR_CRCFWD_SHIFT                    (14U)
#define ENET_RCR_CRCFWD_WIDTH                    (1U)
#define ENET_RCR_CRCFWD(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RCR_CRCFWD_SHIFT)) & ENET_RCR_CRCFWD_MASK)

#define ENET_RCR_CFEN_MASK                       (0x8000U)
#define ENET_RCR_CFEN_SHIFT                      (15U)
#define ENET_RCR_CFEN_WIDTH                      (1U)
#define ENET_RCR_CFEN(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_RCR_CFEN_SHIFT)) & ENET_RCR_CFEN_MASK)

#define ENET_RCR_MAX_FL_MASK                     (0x3FFF0000U)
#define ENET_RCR_MAX_FL_SHIFT                    (16U)
#define ENET_RCR_MAX_FL_WIDTH                    (14U)
#define ENET_RCR_MAX_FL(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RCR_MAX_FL_SHIFT)) & ENET_RCR_MAX_FL_MASK)

#define ENET_RCR_NLC_MASK                        (0x40000000U)
#define ENET_RCR_NLC_SHIFT                       (30U)
#define ENET_RCR_NLC_WIDTH                       (1U)
#define ENET_RCR_NLC(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_RCR_NLC_SHIFT)) & ENET_RCR_NLC_MASK)

#define ENET_RCR_GRS_MASK                        (0x80000000U)
#define ENET_RCR_GRS_SHIFT                       (31U)
#define ENET_RCR_GRS_WIDTH                       (1U)
#define ENET_RCR_GRS(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_RCR_GRS_SHIFT)) & ENET_RCR_GRS_MASK)
/*! @} */

/*! @name TCR - Transmit Control Register */
/*! @{ */

#define ENET_TCR_GTS_MASK                        (0x1U)
#define ENET_TCR_GTS_SHIFT                       (0U)
#define ENET_TCR_GTS_WIDTH                       (1U)
#define ENET_TCR_GTS(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_TCR_GTS_SHIFT)) & ENET_TCR_GTS_MASK)

#define ENET_TCR_FDEN_MASK                       (0x4U)
#define ENET_TCR_FDEN_SHIFT                      (2U)
#define ENET_TCR_FDEN_WIDTH                      (1U)
#define ENET_TCR_FDEN(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TCR_FDEN_SHIFT)) & ENET_TCR_FDEN_MASK)

#define ENET_TCR_TFC_PAUSE_MASK                  (0x8U)
#define ENET_TCR_TFC_PAUSE_SHIFT                 (3U)
#define ENET_TCR_TFC_PAUSE_WIDTH                 (1U)
#define ENET_TCR_TFC_PAUSE(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_TCR_TFC_PAUSE_SHIFT)) & ENET_TCR_TFC_PAUSE_MASK)

#define ENET_TCR_RFC_PAUSE_MASK                  (0x10U)
#define ENET_TCR_RFC_PAUSE_SHIFT                 (4U)
#define ENET_TCR_RFC_PAUSE_WIDTH                 (1U)
#define ENET_TCR_RFC_PAUSE(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_TCR_RFC_PAUSE_SHIFT)) & ENET_TCR_RFC_PAUSE_MASK)

#define ENET_TCR_ADDSEL_MASK                     (0xE0U)
#define ENET_TCR_ADDSEL_SHIFT                    (5U)
#define ENET_TCR_ADDSEL_WIDTH                    (3U)
#define ENET_TCR_ADDSEL(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCR_ADDSEL_SHIFT)) & ENET_TCR_ADDSEL_MASK)

#define ENET_TCR_ADDINS_MASK                     (0x100U)
#define ENET_TCR_ADDINS_SHIFT                    (8U)
#define ENET_TCR_ADDINS_WIDTH                    (1U)
#define ENET_TCR_ADDINS(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCR_ADDINS_SHIFT)) & ENET_TCR_ADDINS_MASK)

#define ENET_TCR_CRCFWD_MASK                     (0x200U)
#define ENET_TCR_CRCFWD_SHIFT                    (9U)
#define ENET_TCR_CRCFWD_WIDTH                    (1U)
#define ENET_TCR_CRCFWD(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCR_CRCFWD_SHIFT)) & ENET_TCR_CRCFWD_MASK)
/*! @} */

/*! @name PALR - Physical Address Lower Register */
/*! @{ */

#define ENET_PALR_PADDR1_MASK                    (0xFFFFFFFFU)
#define ENET_PALR_PADDR1_SHIFT                   (0U)
#define ENET_PALR_PADDR1_WIDTH                   (32U)
#define ENET_PALR_PADDR1(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_PALR_PADDR1_SHIFT)) & ENET_PALR_PADDR1_MASK)
/*! @} */

/*! @name PAUR - Physical Address Upper Register */
/*! @{ */

#define ENET_PAUR_TYPE_MASK                      (0xFFFFU)
#define ENET_PAUR_TYPE_SHIFT                     (0U)
#define ENET_PAUR_TYPE_WIDTH                     (16U)
#define ENET_PAUR_TYPE(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_PAUR_TYPE_SHIFT)) & ENET_PAUR_TYPE_MASK)

#define ENET_PAUR_PADDR2_MASK                    (0xFFFF0000U)
#define ENET_PAUR_PADDR2_SHIFT                   (16U)
#define ENET_PAUR_PADDR2_WIDTH                   (16U)
#define ENET_PAUR_PADDR2(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_PAUR_PADDR2_SHIFT)) & ENET_PAUR_PADDR2_MASK)
/*! @} */

/*! @name OPD - Opcode/Pause Duration Register */
/*! @{ */

#define ENET_OPD_PAUSE_DUR_MASK                  (0xFFFFU)
#define ENET_OPD_PAUSE_DUR_SHIFT                 (0U)
#define ENET_OPD_PAUSE_DUR_WIDTH                 (16U)
#define ENET_OPD_PAUSE_DUR(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_OPD_PAUSE_DUR_SHIFT)) & ENET_OPD_PAUSE_DUR_MASK)

#define ENET_OPD_OPCODE_MASK                     (0xFFFF0000U)
#define ENET_OPD_OPCODE_SHIFT                    (16U)
#define ENET_OPD_OPCODE_WIDTH                    (16U)
#define ENET_OPD_OPCODE(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_OPD_OPCODE_SHIFT)) & ENET_OPD_OPCODE_MASK)
/*! @} */

/*! @name IAUR - Descriptor Individual Upper Address Register */
/*! @{ */

#define ENET_IAUR_IADDR1_MASK                    (0xFFFFFFFFU)
#define ENET_IAUR_IADDR1_SHIFT                   (0U)
#define ENET_IAUR_IADDR1_WIDTH                   (32U)
#define ENET_IAUR_IADDR1(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_IAUR_IADDR1_SHIFT)) & ENET_IAUR_IADDR1_MASK)
/*! @} */

/*! @name IALR - Descriptor Individual Lower Address Register */
/*! @{ */

#define ENET_IALR_IADDR2_MASK                    (0xFFFFFFFFU)
#define ENET_IALR_IADDR2_SHIFT                   (0U)
#define ENET_IALR_IADDR2_WIDTH                   (32U)
#define ENET_IALR_IADDR2(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_IALR_IADDR2_SHIFT)) & ENET_IALR_IADDR2_MASK)
/*! @} */

/*! @name GAUR - Descriptor Group Upper Address Register */
/*! @{ */

#define ENET_GAUR_GADDR1_MASK                    (0xFFFFFFFFU)
#define ENET_GAUR_GADDR1_SHIFT                   (0U)
#define ENET_GAUR_GADDR1_WIDTH                   (32U)
#define ENET_GAUR_GADDR1(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_GAUR_GADDR1_SHIFT)) & ENET_GAUR_GADDR1_MASK)
/*! @} */

/*! @name GALR - Descriptor Group Lower Address Register */
/*! @{ */

#define ENET_GALR_GADDR2_MASK                    (0xFFFFFFFFU)
#define ENET_GALR_GADDR2_SHIFT                   (0U)
#define ENET_GALR_GADDR2_WIDTH                   (32U)
#define ENET_GALR_GADDR2(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_GALR_GADDR2_SHIFT)) & ENET_GALR_GADDR2_MASK)
/*! @} */

/*! @name TFWR - Transmit FIFO Watermark Register */
/*! @{ */

#define ENET_TFWR_TFWR_MASK                      (0x3FU)
#define ENET_TFWR_TFWR_SHIFT                     (0U)
#define ENET_TFWR_TFWR_WIDTH                     (6U)
#define ENET_TFWR_TFWR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_TFWR_TFWR_SHIFT)) & ENET_TFWR_TFWR_MASK)

#define ENET_TFWR_STRFWD_MASK                    (0x100U)
#define ENET_TFWR_STRFWD_SHIFT                   (8U)
#define ENET_TFWR_STRFWD_WIDTH                   (1U)
#define ENET_TFWR_STRFWD(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_TFWR_STRFWD_SHIFT)) & ENET_TFWR_STRFWD_MASK)
/*! @} */

/*! @name RDSR - Receive Descriptor Ring Start Register */
/*! @{ */

#define ENET_RDSR_R_DES_START_MASK               (0xFFFFFFF8U)
#define ENET_RDSR_R_DES_START_SHIFT              (3U)
#define ENET_RDSR_R_DES_START_WIDTH              (29U)
#define ENET_RDSR_R_DES_START(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RDSR_R_DES_START_SHIFT)) & ENET_RDSR_R_DES_START_MASK)
/*! @} */

/*! @name TDSR - Transmit Buffer Descriptor Ring Start Register */
/*! @{ */

#define ENET_TDSR_X_DES_START_MASK               (0xFFFFFFF8U)
#define ENET_TDSR_X_DES_START_SHIFT              (3U)
#define ENET_TDSR_X_DES_START_WIDTH              (29U)
#define ENET_TDSR_X_DES_START(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_TDSR_X_DES_START_SHIFT)) & ENET_TDSR_X_DES_START_MASK)
/*! @} */

/*! @name MRBR - Maximum Receive Buffer Size Register */
/*! @{ */

#define ENET_MRBR_R_BUF_SIZE_MASK                (0x3FF0U)
#define ENET_MRBR_R_BUF_SIZE_SHIFT               (4U)
#define ENET_MRBR_R_BUF_SIZE_WIDTH               (10U)
#define ENET_MRBR_R_BUF_SIZE(x)                  (((uint32_t)(((uint32_t)(x)) << ENET_MRBR_R_BUF_SIZE_SHIFT)) & ENET_MRBR_R_BUF_SIZE_MASK)
/*! @} */

/*! @name RSFL - Receive FIFO Section Full Threshold */
/*! @{ */

#define ENET_RSFL_RX_SECTION_FULL_MASK           (0xFFU)
#define ENET_RSFL_RX_SECTION_FULL_SHIFT          (0U)
#define ENET_RSFL_RX_SECTION_FULL_WIDTH          (8U)
#define ENET_RSFL_RX_SECTION_FULL(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RSFL_RX_SECTION_FULL_SHIFT)) & ENET_RSFL_RX_SECTION_FULL_MASK)
/*! @} */

/*! @name RSEM - Receive FIFO Section Empty Threshold */
/*! @{ */

#define ENET_RSEM_RX_SECTION_EMPTY_MASK          (0xFFU)
#define ENET_RSEM_RX_SECTION_EMPTY_SHIFT         (0U)
#define ENET_RSEM_RX_SECTION_EMPTY_WIDTH         (8U)
#define ENET_RSEM_RX_SECTION_EMPTY(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RSEM_RX_SECTION_EMPTY_SHIFT)) & ENET_RSEM_RX_SECTION_EMPTY_MASK)

#define ENET_RSEM_STAT_SECTION_EMPTY_MASK        (0x1F0000U)
#define ENET_RSEM_STAT_SECTION_EMPTY_SHIFT       (16U)
#define ENET_RSEM_STAT_SECTION_EMPTY_WIDTH       (5U)
#define ENET_RSEM_STAT_SECTION_EMPTY(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RSEM_STAT_SECTION_EMPTY_SHIFT)) & ENET_RSEM_STAT_SECTION_EMPTY_MASK)
/*! @} */

/*! @name RAEM - Receive FIFO Almost Empty Threshold */
/*! @{ */

#define ENET_RAEM_RX_ALMOST_EMPTY_MASK           (0xFFU)
#define ENET_RAEM_RX_ALMOST_EMPTY_SHIFT          (0U)
#define ENET_RAEM_RX_ALMOST_EMPTY_WIDTH          (8U)
#define ENET_RAEM_RX_ALMOST_EMPTY(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RAEM_RX_ALMOST_EMPTY_SHIFT)) & ENET_RAEM_RX_ALMOST_EMPTY_MASK)
/*! @} */

/*! @name RAFL - Receive FIFO Almost Full Threshold */
/*! @{ */

#define ENET_RAFL_RX_ALMOST_FULL_MASK            (0xFFU)
#define ENET_RAFL_RX_ALMOST_FULL_SHIFT           (0U)
#define ENET_RAFL_RX_ALMOST_FULL_WIDTH           (8U)
#define ENET_RAFL_RX_ALMOST_FULL(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RAFL_RX_ALMOST_FULL_SHIFT)) & ENET_RAFL_RX_ALMOST_FULL_MASK)
/*! @} */

/*! @name TSEM - Transmit FIFO Section Empty Threshold */
/*! @{ */

#define ENET_TSEM_TX_SECTION_EMPTY_MASK          (0xFFU)
#define ENET_TSEM_TX_SECTION_EMPTY_SHIFT         (0U)
#define ENET_TSEM_TX_SECTION_EMPTY_WIDTH         (8U)
#define ENET_TSEM_TX_SECTION_EMPTY(x)            (((uint32_t)(((uint32_t)(x)) << ENET_TSEM_TX_SECTION_EMPTY_SHIFT)) & ENET_TSEM_TX_SECTION_EMPTY_MASK)
/*! @} */

/*! @name TAEM - Transmit FIFO Almost Empty Threshold */
/*! @{ */

#define ENET_TAEM_TX_ALMOST_EMPTY_MASK           (0xFFU)
#define ENET_TAEM_TX_ALMOST_EMPTY_SHIFT          (0U)
#define ENET_TAEM_TX_ALMOST_EMPTY_WIDTH          (8U)
#define ENET_TAEM_TX_ALMOST_EMPTY(x)             (((uint32_t)(((uint32_t)(x)) << ENET_TAEM_TX_ALMOST_EMPTY_SHIFT)) & ENET_TAEM_TX_ALMOST_EMPTY_MASK)
/*! @} */

/*! @name TAFL - Transmit FIFO Almost Full Threshold */
/*! @{ */

#define ENET_TAFL_TX_ALMOST_FULL_MASK            (0xFFU)
#define ENET_TAFL_TX_ALMOST_FULL_SHIFT           (0U)
#define ENET_TAFL_TX_ALMOST_FULL_WIDTH           (8U)
#define ENET_TAFL_TX_ALMOST_FULL(x)              (((uint32_t)(((uint32_t)(x)) << ENET_TAFL_TX_ALMOST_FULL_SHIFT)) & ENET_TAFL_TX_ALMOST_FULL_MASK)
/*! @} */

/*! @name TIPG - Transmit Inter-Packet Gap */
/*! @{ */

#define ENET_TIPG_IPG_MASK                       (0x1FU)
#define ENET_TIPG_IPG_SHIFT                      (0U)
#define ENET_TIPG_IPG_WIDTH                      (5U)
#define ENET_TIPG_IPG(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TIPG_IPG_SHIFT)) & ENET_TIPG_IPG_MASK)
/*! @} */

/*! @name FTRL - Frame Truncation Length */
/*! @{ */

#define ENET_FTRL_TRUNC_FL_MASK                  (0x3FFFU)
#define ENET_FTRL_TRUNC_FL_SHIFT                 (0U)
#define ENET_FTRL_TRUNC_FL_WIDTH                 (14U)
#define ENET_FTRL_TRUNC_FL(x)                    (((uint32_t)(((uint32_t)(x)) << ENET_FTRL_TRUNC_FL_SHIFT)) & ENET_FTRL_TRUNC_FL_MASK)
/*! @} */

/*! @name TACC - Transmit Accelerator Function Configuration */
/*! @{ */

#define ENET_TACC_SHIFT16_MASK                   (0x1U)
#define ENET_TACC_SHIFT16_SHIFT                  (0U)
#define ENET_TACC_SHIFT16_WIDTH                  (1U)
#define ENET_TACC_SHIFT16(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_TACC_SHIFT16_SHIFT)) & ENET_TACC_SHIFT16_MASK)

#define ENET_TACC_IPCHK_MASK                     (0x8U)
#define ENET_TACC_IPCHK_SHIFT                    (3U)
#define ENET_TACC_IPCHK_WIDTH                    (1U)
#define ENET_TACC_IPCHK(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TACC_IPCHK_SHIFT)) & ENET_TACC_IPCHK_MASK)

#define ENET_TACC_PROCHK_MASK                    (0x10U)
#define ENET_TACC_PROCHK_SHIFT                   (4U)
#define ENET_TACC_PROCHK_WIDTH                   (1U)
#define ENET_TACC_PROCHK(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_TACC_PROCHK_SHIFT)) & ENET_TACC_PROCHK_MASK)
/*! @} */

/*! @name RACC - Receive Accelerator Function Configuration */
/*! @{ */

#define ENET_RACC_PADREM_MASK                    (0x1U)
#define ENET_RACC_PADREM_SHIFT                   (0U)
#define ENET_RACC_PADREM_WIDTH                   (1U)
#define ENET_RACC_PADREM(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_RACC_PADREM_SHIFT)) & ENET_RACC_PADREM_MASK)

#define ENET_RACC_IPDIS_MASK                     (0x2U)
#define ENET_RACC_IPDIS_SHIFT                    (1U)
#define ENET_RACC_IPDIS_WIDTH                    (1U)
#define ENET_RACC_IPDIS(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_RACC_IPDIS_SHIFT)) & ENET_RACC_IPDIS_MASK)

#define ENET_RACC_PRODIS_MASK                    (0x4U)
#define ENET_RACC_PRODIS_SHIFT                   (2U)
#define ENET_RACC_PRODIS_WIDTH                   (1U)
#define ENET_RACC_PRODIS(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_RACC_PRODIS_SHIFT)) & ENET_RACC_PRODIS_MASK)

#define ENET_RACC_LINEDIS_MASK                   (0x40U)
#define ENET_RACC_LINEDIS_SHIFT                  (6U)
#define ENET_RACC_LINEDIS_WIDTH                  (1U)
#define ENET_RACC_LINEDIS(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_RACC_LINEDIS_SHIFT)) & ENET_RACC_LINEDIS_MASK)

#define ENET_RACC_SHIFT16_MASK                   (0x80U)
#define ENET_RACC_SHIFT16_SHIFT                  (7U)
#define ENET_RACC_SHIFT16_WIDTH                  (1U)
#define ENET_RACC_SHIFT16(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_RACC_SHIFT16_SHIFT)) & ENET_RACC_SHIFT16_MASK)
/*! @} */

/*! @name RMON_T_PACKETS - Tx Packet Count Statistic Register */
/*! @{ */

#define ENET_RMON_T_PACKETS_TXPKTS_MASK          (0xFFFFU)
#define ENET_RMON_T_PACKETS_TXPKTS_SHIFT         (0U)
#define ENET_RMON_T_PACKETS_TXPKTS_WIDTH         (16U)
#define ENET_RMON_T_PACKETS_TXPKTS(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_PACKETS_TXPKTS_SHIFT)) & ENET_RMON_T_PACKETS_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_BC_PKT - Tx Broadcast Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_BC_PKT_TXPKTS_MASK           (0xFFFFU)
#define ENET_RMON_T_BC_PKT_TXPKTS_SHIFT          (0U)
#define ENET_RMON_T_BC_PKT_TXPKTS_WIDTH          (16U)
#define ENET_RMON_T_BC_PKT_TXPKTS(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_BC_PKT_TXPKTS_SHIFT)) & ENET_RMON_T_BC_PKT_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_MC_PKT - Tx Multicast Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_MC_PKT_TXPKTS_MASK           (0xFFFFU)
#define ENET_RMON_T_MC_PKT_TXPKTS_SHIFT          (0U)
#define ENET_RMON_T_MC_PKT_TXPKTS_WIDTH          (16U)
#define ENET_RMON_T_MC_PKT_TXPKTS(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_MC_PKT_TXPKTS_SHIFT)) & ENET_RMON_T_MC_PKT_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_CRC_ALIGN - Tx Packets with CRC/Align Error Statistic Register */
/*! @{ */

#define ENET_RMON_T_CRC_ALIGN_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_CRC_ALIGN_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_CRC_ALIGN_TXPKTS_WIDTH       (16U)
#define ENET_RMON_T_CRC_ALIGN_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_CRC_ALIGN_TXPKTS_SHIFT)) & ENET_RMON_T_CRC_ALIGN_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_UNDERSIZE - Tx Packets Less Than Bytes and Good CRC Statistic Register */
/*! @{ */

#define ENET_RMON_T_UNDERSIZE_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_UNDERSIZE_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_UNDERSIZE_TXPKTS_WIDTH       (16U)
#define ENET_RMON_T_UNDERSIZE_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_UNDERSIZE_TXPKTS_SHIFT)) & ENET_RMON_T_UNDERSIZE_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_OVERSIZE - Tx Packets GT MAX_FL bytes and Good CRC Statistic Register */
/*! @{ */

#define ENET_RMON_T_OVERSIZE_TXPKTS_MASK         (0xFFFFU)
#define ENET_RMON_T_OVERSIZE_TXPKTS_SHIFT        (0U)
#define ENET_RMON_T_OVERSIZE_TXPKTS_WIDTH        (16U)
#define ENET_RMON_T_OVERSIZE_TXPKTS(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_OVERSIZE_TXPKTS_SHIFT)) & ENET_RMON_T_OVERSIZE_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_FRAG - Tx Packets Less Than 64 Bytes and Bad CRC Statistic Register */
/*! @{ */

#define ENET_RMON_T_FRAG_TXPKTS_MASK             (0xFFFFU)
#define ENET_RMON_T_FRAG_TXPKTS_SHIFT            (0U)
#define ENET_RMON_T_FRAG_TXPKTS_WIDTH            (16U)
#define ENET_RMON_T_FRAG_TXPKTS(x)               (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_FRAG_TXPKTS_SHIFT)) & ENET_RMON_T_FRAG_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_JAB - Tx Packets Greater Than MAX_FL bytes and Bad CRC Statistic Register */
/*! @{ */

#define ENET_RMON_T_JAB_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_JAB_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_JAB_TXPKTS_WIDTH             (16U)
#define ENET_RMON_T_JAB_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_JAB_TXPKTS_SHIFT)) & ENET_RMON_T_JAB_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_COL - Tx Collision Count Statistic Register */
/*! @{ */

#define ENET_RMON_T_COL_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_COL_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_COL_TXPKTS_WIDTH             (16U)
#define ENET_RMON_T_COL_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_COL_TXPKTS_SHIFT)) & ENET_RMON_T_COL_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P64 - Tx 64-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P64_TXPKTS_MASK              (0xFFFFU)
#define ENET_RMON_T_P64_TXPKTS_SHIFT             (0U)
#define ENET_RMON_T_P64_TXPKTS_WIDTH             (16U)
#define ENET_RMON_T_P64_TXPKTS(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P64_TXPKTS_SHIFT)) & ENET_RMON_T_P64_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P65TO127 - Tx 65- to 127-byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P65TO127_TXPKTS_MASK         (0xFFFFU)
#define ENET_RMON_T_P65TO127_TXPKTS_SHIFT        (0U)
#define ENET_RMON_T_P65TO127_TXPKTS_WIDTH        (16U)
#define ENET_RMON_T_P65TO127_TXPKTS(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P65TO127_TXPKTS_SHIFT)) & ENET_RMON_T_P65TO127_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P128TO255 - Tx 128- to 255-byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P128TO255_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P128TO255_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P128TO255_TXPKTS_WIDTH       (16U)
#define ENET_RMON_T_P128TO255_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P128TO255_TXPKTS_SHIFT)) & ENET_RMON_T_P128TO255_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P256TO511 - Tx 256- to 511-byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P256TO511_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P256TO511_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P256TO511_TXPKTS_WIDTH       (16U)
#define ENET_RMON_T_P256TO511_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P256TO511_TXPKTS_SHIFT)) & ENET_RMON_T_P256TO511_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P512TO1023 - Tx 512- to 1023-byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P512TO1023_TXPKTS_MASK       (0xFFFFU)
#define ENET_RMON_T_P512TO1023_TXPKTS_SHIFT      (0U)
#define ENET_RMON_T_P512TO1023_TXPKTS_WIDTH      (16U)
#define ENET_RMON_T_P512TO1023_TXPKTS(x)         (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P512TO1023_TXPKTS_SHIFT)) & ENET_RMON_T_P512TO1023_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P1024TO2047 - Tx 1024- to 2047-byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_T_P1024TO2047_TXPKTS_MASK      (0xFFFFU)
#define ENET_RMON_T_P1024TO2047_TXPKTS_SHIFT     (0U)
#define ENET_RMON_T_P1024TO2047_TXPKTS_WIDTH     (16U)
#define ENET_RMON_T_P1024TO2047_TXPKTS(x)        (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P1024TO2047_TXPKTS_SHIFT)) & ENET_RMON_T_P1024TO2047_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_P_GTE2048 - Tx Packets Greater Than 2048 Bytes Statistic Register */
/*! @{ */

#define ENET_RMON_T_P_GTE2048_TXPKTS_MASK        (0xFFFFU)
#define ENET_RMON_T_P_GTE2048_TXPKTS_SHIFT       (0U)
#define ENET_RMON_T_P_GTE2048_TXPKTS_WIDTH       (16U)
#define ENET_RMON_T_P_GTE2048_TXPKTS(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_P_GTE2048_TXPKTS_SHIFT)) & ENET_RMON_T_P_GTE2048_TXPKTS_MASK)
/*! @} */

/*! @name RMON_T_OCTETS - Tx Octets Statistic Register */
/*! @{ */

#define ENET_RMON_T_OCTETS_TXOCTS_MASK           (0xFFFFFFFFU)
#define ENET_RMON_T_OCTETS_TXOCTS_SHIFT          (0U)
#define ENET_RMON_T_OCTETS_TXOCTS_WIDTH          (32U)
#define ENET_RMON_T_OCTETS_TXOCTS(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_T_OCTETS_TXOCTS_SHIFT)) & ENET_RMON_T_OCTETS_TXOCTS_MASK)
/*! @} */

/*! @name IEEE_T_FRAME_OK - Frames Transmitted OK Statistic Register */
/*! @{ */

#define ENET_IEEE_T_FRAME_OK_COUNT_MASK          (0xFFFFU)
#define ENET_IEEE_T_FRAME_OK_COUNT_SHIFT         (0U)
#define ENET_IEEE_T_FRAME_OK_COUNT_WIDTH         (16U)
#define ENET_IEEE_T_FRAME_OK_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_FRAME_OK_COUNT_SHIFT)) & ENET_IEEE_T_FRAME_OK_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_1COL - Frames Transmitted with Single Collision Statistic Register */
/*! @{ */

#define ENET_IEEE_T_1COL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_1COL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_1COL_COUNT_WIDTH             (16U)
#define ENET_IEEE_T_1COL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_1COL_COUNT_SHIFT)) & ENET_IEEE_T_1COL_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_MCOL - Frames Transmitted with Multiple Collisions Statistic Register */
/*! @{ */

#define ENET_IEEE_T_MCOL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_MCOL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_MCOL_COUNT_WIDTH             (16U)
#define ENET_IEEE_T_MCOL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_MCOL_COUNT_SHIFT)) & ENET_IEEE_T_MCOL_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_DEF - Frames Transmitted after Deferral Delay Statistic Register */
/*! @{ */

#define ENET_IEEE_T_DEF_COUNT_MASK               (0xFFFFU)
#define ENET_IEEE_T_DEF_COUNT_SHIFT              (0U)
#define ENET_IEEE_T_DEF_COUNT_WIDTH              (16U)
#define ENET_IEEE_T_DEF_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_DEF_COUNT_SHIFT)) & ENET_IEEE_T_DEF_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_LCOL - Frames Transmitted with Late Collision Statistic Register */
/*! @{ */

#define ENET_IEEE_T_LCOL_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_T_LCOL_COUNT_SHIFT             (0U)
#define ENET_IEEE_T_LCOL_COUNT_WIDTH             (16U)
#define ENET_IEEE_T_LCOL_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_LCOL_COUNT_SHIFT)) & ENET_IEEE_T_LCOL_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_EXCOL - Frames Transmitted with Excessive Collisions Statistic Register */
/*! @{ */

#define ENET_IEEE_T_EXCOL_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_EXCOL_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_EXCOL_COUNT_WIDTH            (16U)
#define ENET_IEEE_T_EXCOL_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_EXCOL_COUNT_SHIFT)) & ENET_IEEE_T_EXCOL_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_MACERR - Frames Transmitted with Tx FIFO Underrun Statistic Register */
/*! @{ */

#define ENET_IEEE_T_MACERR_COUNT_MASK            (0xFFFFU)
#define ENET_IEEE_T_MACERR_COUNT_SHIFT           (0U)
#define ENET_IEEE_T_MACERR_COUNT_WIDTH           (16U)
#define ENET_IEEE_T_MACERR_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_MACERR_COUNT_SHIFT)) & ENET_IEEE_T_MACERR_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_CSERR - Frames Transmitted with Carrier Sense Error Statistic Register */
/*! @{ */

#define ENET_IEEE_T_CSERR_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_CSERR_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_CSERR_COUNT_WIDTH            (16U)
#define ENET_IEEE_T_CSERR_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_CSERR_COUNT_SHIFT)) & ENET_IEEE_T_CSERR_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_SQE - Reserved Statistic Register */
/*! @{ */

#define ENET_IEEE_T_SQE_COUNT_MASK               (0xFFFFU)
#define ENET_IEEE_T_SQE_COUNT_SHIFT              (0U)
#define ENET_IEEE_T_SQE_COUNT_WIDTH              (16U)
#define ENET_IEEE_T_SQE_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_SQE_COUNT_SHIFT)) & ENET_IEEE_T_SQE_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_FDXFC - Flow Control Pause Frames Transmitted Statistic Register */
/*! @{ */

#define ENET_IEEE_T_FDXFC_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_T_FDXFC_COUNT_SHIFT            (0U)
#define ENET_IEEE_T_FDXFC_COUNT_WIDTH            (16U)
#define ENET_IEEE_T_FDXFC_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_FDXFC_COUNT_SHIFT)) & ENET_IEEE_T_FDXFC_COUNT_MASK)
/*! @} */

/*! @name IEEE_T_OCTETS_OK - Octet Count for Frames Transmitted w/o Error Statistic Register */
/*! @{ */

#define ENET_IEEE_T_OCTETS_OK_COUNT_MASK         (0xFFFFFFFFU)
#define ENET_IEEE_T_OCTETS_OK_COUNT_SHIFT        (0U)
#define ENET_IEEE_T_OCTETS_OK_COUNT_WIDTH        (32U)
#define ENET_IEEE_T_OCTETS_OK_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_T_OCTETS_OK_COUNT_SHIFT)) & ENET_IEEE_T_OCTETS_OK_COUNT_MASK)
/*! @} */

/*! @name RMON_R_PACKETS - Rx Packet Count Statistic Register */
/*! @{ */

#define ENET_RMON_R_PACKETS_COUNT_MASK           (0xFFFFU)
#define ENET_RMON_R_PACKETS_COUNT_SHIFT          (0U)
#define ENET_RMON_R_PACKETS_COUNT_WIDTH          (16U)
#define ENET_RMON_R_PACKETS_COUNT(x)             (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_PACKETS_COUNT_SHIFT)) & ENET_RMON_R_PACKETS_COUNT_MASK)
/*! @} */

/*! @name RMON_R_BC_PKT - Rx Broadcast Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_BC_PKT_COUNT_MASK            (0xFFFFU)
#define ENET_RMON_R_BC_PKT_COUNT_SHIFT           (0U)
#define ENET_RMON_R_BC_PKT_COUNT_WIDTH           (16U)
#define ENET_RMON_R_BC_PKT_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_BC_PKT_COUNT_SHIFT)) & ENET_RMON_R_BC_PKT_COUNT_MASK)
/*! @} */

/*! @name RMON_R_MC_PKT - Rx Multicast Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_MC_PKT_COUNT_MASK            (0xFFFFU)
#define ENET_RMON_R_MC_PKT_COUNT_SHIFT           (0U)
#define ENET_RMON_R_MC_PKT_COUNT_WIDTH           (16U)
#define ENET_RMON_R_MC_PKT_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_MC_PKT_COUNT_SHIFT)) & ENET_RMON_R_MC_PKT_COUNT_MASK)
/*! @} */

/*! @name RMON_R_CRC_ALIGN - Rx Packets with CRC/Align Error Statistic Register */
/*! @{ */

#define ENET_RMON_R_CRC_ALIGN_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_CRC_ALIGN_COUNT_SHIFT        (0U)
#define ENET_RMON_R_CRC_ALIGN_COUNT_WIDTH        (16U)
#define ENET_RMON_R_CRC_ALIGN_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_CRC_ALIGN_COUNT_SHIFT)) & ENET_RMON_R_CRC_ALIGN_COUNT_MASK)
/*! @} */

/*! @name RMON_R_UNDERSIZE - Rx Packets with Less Than 64 Bytes and Good CRC Statistic Register */
/*! @{ */

#define ENET_RMON_R_UNDERSIZE_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_UNDERSIZE_COUNT_SHIFT        (0U)
#define ENET_RMON_R_UNDERSIZE_COUNT_WIDTH        (16U)
#define ENET_RMON_R_UNDERSIZE_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_UNDERSIZE_COUNT_SHIFT)) & ENET_RMON_R_UNDERSIZE_COUNT_MASK)
/*! @} */

/*! @name RMON_R_OVERSIZE - Rx Packets Greater Than MAX_FL and Good CRC Statistic Register */
/*! @{ */

#define ENET_RMON_R_OVERSIZE_COUNT_MASK          (0xFFFFU)
#define ENET_RMON_R_OVERSIZE_COUNT_SHIFT         (0U)
#define ENET_RMON_R_OVERSIZE_COUNT_WIDTH         (16U)
#define ENET_RMON_R_OVERSIZE_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_OVERSIZE_COUNT_SHIFT)) & ENET_RMON_R_OVERSIZE_COUNT_MASK)
/*! @} */

/*! @name RMON_R_FRAG - Rx Packets Less Than 64 Bytes and Bad CRC Statistic Register */
/*! @{ */

#define ENET_RMON_R_FRAG_COUNT_MASK              (0xFFFFU)
#define ENET_RMON_R_FRAG_COUNT_SHIFT             (0U)
#define ENET_RMON_R_FRAG_COUNT_WIDTH             (16U)
#define ENET_RMON_R_FRAG_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_FRAG_COUNT_SHIFT)) & ENET_RMON_R_FRAG_COUNT_MASK)
/*! @} */

/*! @name RMON_R_JAB - Rx Packets Greater Than MAX_FL Bytes and Bad CRC Statistic Register */
/*! @{ */

#define ENET_RMON_R_JAB_COUNT_MASK               (0xFFFFU)
#define ENET_RMON_R_JAB_COUNT_SHIFT              (0U)
#define ENET_RMON_R_JAB_COUNT_WIDTH              (16U)
#define ENET_RMON_R_JAB_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_JAB_COUNT_SHIFT)) & ENET_RMON_R_JAB_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P64 - Rx 64-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P64_COUNT_MASK               (0xFFFFU)
#define ENET_RMON_R_P64_COUNT_SHIFT              (0U)
#define ENET_RMON_R_P64_COUNT_WIDTH              (16U)
#define ENET_RMON_R_P64_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P64_COUNT_SHIFT)) & ENET_RMON_R_P64_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P65TO127 - Rx 65- to 127-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P65TO127_COUNT_MASK          (0xFFFFU)
#define ENET_RMON_R_P65TO127_COUNT_SHIFT         (0U)
#define ENET_RMON_R_P65TO127_COUNT_WIDTH         (16U)
#define ENET_RMON_R_P65TO127_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P65TO127_COUNT_SHIFT)) & ENET_RMON_R_P65TO127_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P128TO255 - Rx 128- to 255-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P128TO255_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P128TO255_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P128TO255_COUNT_WIDTH        (16U)
#define ENET_RMON_R_P128TO255_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P128TO255_COUNT_SHIFT)) & ENET_RMON_R_P128TO255_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P256TO511 - Rx 256- to 511-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P256TO511_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P256TO511_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P256TO511_COUNT_WIDTH        (16U)
#define ENET_RMON_R_P256TO511_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P256TO511_COUNT_SHIFT)) & ENET_RMON_R_P256TO511_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P512TO1023 - Rx 512- to 1023-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P512TO1023_COUNT_MASK        (0xFFFFU)
#define ENET_RMON_R_P512TO1023_COUNT_SHIFT       (0U)
#define ENET_RMON_R_P512TO1023_COUNT_WIDTH       (16U)
#define ENET_RMON_R_P512TO1023_COUNT(x)          (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P512TO1023_COUNT_SHIFT)) & ENET_RMON_R_P512TO1023_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P1024TO2047 - Rx 1024- to 2047-Byte Packets Statistic Register */
/*! @{ */

#define ENET_RMON_R_P1024TO2047_COUNT_MASK       (0xFFFFU)
#define ENET_RMON_R_P1024TO2047_COUNT_SHIFT      (0U)
#define ENET_RMON_R_P1024TO2047_COUNT_WIDTH      (16U)
#define ENET_RMON_R_P1024TO2047_COUNT(x)         (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P1024TO2047_COUNT_SHIFT)) & ENET_RMON_R_P1024TO2047_COUNT_MASK)
/*! @} */

/*! @name RMON_R_P_GTE2048 - Rx Packets Greater than 2048 Bytes Statistic Register */
/*! @{ */

#define ENET_RMON_R_P_GTE2048_COUNT_MASK         (0xFFFFU)
#define ENET_RMON_R_P_GTE2048_COUNT_SHIFT        (0U)
#define ENET_RMON_R_P_GTE2048_COUNT_WIDTH        (16U)
#define ENET_RMON_R_P_GTE2048_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_P_GTE2048_COUNT_SHIFT)) & ENET_RMON_R_P_GTE2048_COUNT_MASK)
/*! @} */

/*! @name RMON_R_OCTETS - Rx Octets Statistic Register */
/*! @{ */

#define ENET_RMON_R_OCTETS_COUNT_MASK            (0xFFFFFFFFU)
#define ENET_RMON_R_OCTETS_COUNT_SHIFT           (0U)
#define ENET_RMON_R_OCTETS_COUNT_WIDTH           (32U)
#define ENET_RMON_R_OCTETS_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_RMON_R_OCTETS_COUNT_SHIFT)) & ENET_RMON_R_OCTETS_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_DROP - Frames not Counted Correctly Statistic Register */
/*! @{ */

#define ENET_IEEE_R_DROP_COUNT_MASK              (0xFFFFU)
#define ENET_IEEE_R_DROP_COUNT_SHIFT             (0U)
#define ENET_IEEE_R_DROP_COUNT_WIDTH             (16U)
#define ENET_IEEE_R_DROP_COUNT(x)                (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_DROP_COUNT_SHIFT)) & ENET_IEEE_R_DROP_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_FRAME_OK - Frames Received OK Statistic Register */
/*! @{ */

#define ENET_IEEE_R_FRAME_OK_COUNT_MASK          (0xFFFFU)
#define ENET_IEEE_R_FRAME_OK_COUNT_SHIFT         (0U)
#define ENET_IEEE_R_FRAME_OK_COUNT_WIDTH         (16U)
#define ENET_IEEE_R_FRAME_OK_COUNT(x)            (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_FRAME_OK_COUNT_SHIFT)) & ENET_IEEE_R_FRAME_OK_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_CRC - Frames Received with CRC Error Statistic Register */
/*! @{ */

#define ENET_IEEE_R_CRC_COUNT_MASK               (0xFFFFU)
#define ENET_IEEE_R_CRC_COUNT_SHIFT              (0U)
#define ENET_IEEE_R_CRC_COUNT_WIDTH              (16U)
#define ENET_IEEE_R_CRC_COUNT(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_CRC_COUNT_SHIFT)) & ENET_IEEE_R_CRC_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_ALIGN - Frames Received with Alignment Error Statistic Register */
/*! @{ */

#define ENET_IEEE_R_ALIGN_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_R_ALIGN_COUNT_SHIFT            (0U)
#define ENET_IEEE_R_ALIGN_COUNT_WIDTH            (16U)
#define ENET_IEEE_R_ALIGN_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_ALIGN_COUNT_SHIFT)) & ENET_IEEE_R_ALIGN_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_MACERR - Receive FIFO Overflow Count Statistic Register */
/*! @{ */

#define ENET_IEEE_R_MACERR_COUNT_MASK            (0xFFFFU)
#define ENET_IEEE_R_MACERR_COUNT_SHIFT           (0U)
#define ENET_IEEE_R_MACERR_COUNT_WIDTH           (16U)
#define ENET_IEEE_R_MACERR_COUNT(x)              (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_MACERR_COUNT_SHIFT)) & ENET_IEEE_R_MACERR_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_FDXFC - Flow Control Pause Frames Received Statistic Register */
/*! @{ */

#define ENET_IEEE_R_FDXFC_COUNT_MASK             (0xFFFFU)
#define ENET_IEEE_R_FDXFC_COUNT_SHIFT            (0U)
#define ENET_IEEE_R_FDXFC_COUNT_WIDTH            (16U)
#define ENET_IEEE_R_FDXFC_COUNT(x)               (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_FDXFC_COUNT_SHIFT)) & ENET_IEEE_R_FDXFC_COUNT_MASK)
/*! @} */

/*! @name IEEE_R_OCTETS_OK - Octet Count for Frames Received without Error Statistic Register */
/*! @{ */

#define ENET_IEEE_R_OCTETS_OK_COUNT_MASK         (0xFFFFFFFFU)
#define ENET_IEEE_R_OCTETS_OK_COUNT_SHIFT        (0U)
#define ENET_IEEE_R_OCTETS_OK_COUNT_WIDTH        (32U)
#define ENET_IEEE_R_OCTETS_OK_COUNT(x)           (((uint32_t)(((uint32_t)(x)) << ENET_IEEE_R_OCTETS_OK_COUNT_SHIFT)) & ENET_IEEE_R_OCTETS_OK_COUNT_MASK)
/*! @} */

/*! @name ATCR - Adjustable Timer Control Register */
/*! @{ */

#define ENET_ATCR_EN_MASK                        (0x1U)
#define ENET_ATCR_EN_SHIFT                       (0U)
#define ENET_ATCR_EN_WIDTH                       (1U)
#define ENET_ATCR_EN(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_EN_SHIFT)) & ENET_ATCR_EN_MASK)

#define ENET_ATCR_OFFEN_MASK                     (0x4U)
#define ENET_ATCR_OFFEN_SHIFT                    (2U)
#define ENET_ATCR_OFFEN_WIDTH                    (1U)
#define ENET_ATCR_OFFEN(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_OFFEN_SHIFT)) & ENET_ATCR_OFFEN_MASK)

#define ENET_ATCR_OFFRST_MASK                    (0x8U)
#define ENET_ATCR_OFFRST_SHIFT                   (3U)
#define ENET_ATCR_OFFRST_WIDTH                   (1U)
#define ENET_ATCR_OFFRST(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_OFFRST_SHIFT)) & ENET_ATCR_OFFRST_MASK)

#define ENET_ATCR_PEREN_MASK                     (0x10U)
#define ENET_ATCR_PEREN_SHIFT                    (4U)
#define ENET_ATCR_PEREN_WIDTH                    (1U)
#define ENET_ATCR_PEREN(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_PEREN_SHIFT)) & ENET_ATCR_PEREN_MASK)

#define ENET_ATCR_PINPER_MASK                    (0x80U)
#define ENET_ATCR_PINPER_SHIFT                   (7U)
#define ENET_ATCR_PINPER_WIDTH                   (1U)
#define ENET_ATCR_PINPER(x)                      (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_PINPER_SHIFT)) & ENET_ATCR_PINPER_MASK)

#define ENET_ATCR_RESTART_MASK                   (0x200U)
#define ENET_ATCR_RESTART_SHIFT                  (9U)
#define ENET_ATCR_RESTART_WIDTH                  (1U)
#define ENET_ATCR_RESTART(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_RESTART_SHIFT)) & ENET_ATCR_RESTART_MASK)

#define ENET_ATCR_CAPTURE_MASK                   (0x800U)
#define ENET_ATCR_CAPTURE_SHIFT                  (11U)
#define ENET_ATCR_CAPTURE_WIDTH                  (1U)
#define ENET_ATCR_CAPTURE(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_CAPTURE_SHIFT)) & ENET_ATCR_CAPTURE_MASK)

#define ENET_ATCR_SLAVE_MASK                     (0x2000U)
#define ENET_ATCR_SLAVE_SHIFT                    (13U)
#define ENET_ATCR_SLAVE_WIDTH                    (1U)
#define ENET_ATCR_SLAVE(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_ATCR_SLAVE_SHIFT)) & ENET_ATCR_SLAVE_MASK)
/*! @} */

/*! @name ATVR - Timer Value Register */
/*! @{ */

#define ENET_ATVR_ATIME_MASK                     (0xFFFFFFFFU)
#define ENET_ATVR_ATIME_SHIFT                    (0U)
#define ENET_ATVR_ATIME_WIDTH                    (32U)
#define ENET_ATVR_ATIME(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_ATVR_ATIME_SHIFT)) & ENET_ATVR_ATIME_MASK)
/*! @} */

/*! @name ATOFF - Timer Offset Register */
/*! @{ */

#define ENET_ATOFF_OFFSET_MASK                   (0xFFFFFFFFU)
#define ENET_ATOFF_OFFSET_SHIFT                  (0U)
#define ENET_ATOFF_OFFSET_WIDTH                  (32U)
#define ENET_ATOFF_OFFSET(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_ATOFF_OFFSET_SHIFT)) & ENET_ATOFF_OFFSET_MASK)
/*! @} */

/*! @name ATPER - Timer Period Register */
/*! @{ */

#define ENET_ATPER_PERIOD_MASK                   (0xFFFFFFFFU)
#define ENET_ATPER_PERIOD_SHIFT                  (0U)
#define ENET_ATPER_PERIOD_WIDTH                  (32U)
#define ENET_ATPER_PERIOD(x)                     (((uint32_t)(((uint32_t)(x)) << ENET_ATPER_PERIOD_SHIFT)) & ENET_ATPER_PERIOD_MASK)
/*! @} */

/*! @name ATCOR - Timer Correction Register */
/*! @{ */

#define ENET_ATCOR_COR_MASK                      (0x7FFFFFFFU)
#define ENET_ATCOR_COR_SHIFT                     (0U)
#define ENET_ATCOR_COR_WIDTH                     (31U)
#define ENET_ATCOR_COR(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ATCOR_COR_SHIFT)) & ENET_ATCOR_COR_MASK)
/*! @} */

/*! @name ATINC - Time-Stamping Clock Period Register */
/*! @{ */

#define ENET_ATINC_INC_MASK                      (0x7FU)
#define ENET_ATINC_INC_SHIFT                     (0U)
#define ENET_ATINC_INC_WIDTH                     (7U)
#define ENET_ATINC_INC(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_ATINC_INC_SHIFT)) & ENET_ATINC_INC_MASK)

#define ENET_ATINC_INC_CORR_MASK                 (0x7F00U)
#define ENET_ATINC_INC_CORR_SHIFT                (8U)
#define ENET_ATINC_INC_CORR_WIDTH                (7U)
#define ENET_ATINC_INC_CORR(x)                   (((uint32_t)(((uint32_t)(x)) << ENET_ATINC_INC_CORR_SHIFT)) & ENET_ATINC_INC_CORR_MASK)
/*! @} */

/*! @name ATSTMP - Timestamp of Last Transmitted Frame */
/*! @{ */

#define ENET_ATSTMP_TIMESTAMP_MASK               (0xFFFFFFFFU)
#define ENET_ATSTMP_TIMESTAMP_SHIFT              (0U)
#define ENET_ATSTMP_TIMESTAMP_WIDTH              (32U)
#define ENET_ATSTMP_TIMESTAMP(x)                 (((uint32_t)(((uint32_t)(x)) << ENET_ATSTMP_TIMESTAMP_SHIFT)) & ENET_ATSTMP_TIMESTAMP_MASK)
/*! @} */

/*! @name TGSR - Timer Global Status Register */
/*! @{ */

#define ENET_TGSR_TF0_MASK                       (0x1U)
#define ENET_TGSR_TF0_SHIFT                      (0U)
#define ENET_TGSR_TF0_WIDTH                      (1U)
#define ENET_TGSR_TF0(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TGSR_TF0_SHIFT)) & ENET_TGSR_TF0_MASK)

#define ENET_TGSR_TF1_MASK                       (0x2U)
#define ENET_TGSR_TF1_SHIFT                      (1U)
#define ENET_TGSR_TF1_WIDTH                      (1U)
#define ENET_TGSR_TF1(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TGSR_TF1_SHIFT)) & ENET_TGSR_TF1_MASK)

#define ENET_TGSR_TF2_MASK                       (0x4U)
#define ENET_TGSR_TF2_SHIFT                      (2U)
#define ENET_TGSR_TF2_WIDTH                      (1U)
#define ENET_TGSR_TF2(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TGSR_TF2_SHIFT)) & ENET_TGSR_TF2_MASK)

#define ENET_TGSR_TF3_MASK                       (0x8U)
#define ENET_TGSR_TF3_SHIFT                      (3U)
#define ENET_TGSR_TF3_WIDTH                      (1U)
#define ENET_TGSR_TF3(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TGSR_TF3_SHIFT)) & ENET_TGSR_TF3_MASK)
/*! @} */

/*! @name TCSR - Timer Control Status Register */
/*! @{ */

#define ENET_TCSR_TDRE_MASK                      (0x1U)
#define ENET_TCSR_TDRE_SHIFT                     (0U)
#define ENET_TCSR_TDRE_WIDTH                     (1U)
#define ENET_TCSR_TDRE(x)                        (((uint32_t)(((uint32_t)(x)) << ENET_TCSR_TDRE_SHIFT)) & ENET_TCSR_TDRE_MASK)

#define ENET_TCSR_TMODE_MASK                     (0x3CU)
#define ENET_TCSR_TMODE_SHIFT                    (2U)
#define ENET_TCSR_TMODE_WIDTH                    (4U)
#define ENET_TCSR_TMODE(x)                       (((uint32_t)(((uint32_t)(x)) << ENET_TCSR_TMODE_SHIFT)) & ENET_TCSR_TMODE_MASK)

#define ENET_TCSR_TIE_MASK                       (0x40U)
#define ENET_TCSR_TIE_SHIFT                      (6U)
#define ENET_TCSR_TIE_WIDTH                      (1U)
#define ENET_TCSR_TIE(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TCSR_TIE_SHIFT)) & ENET_TCSR_TIE_MASK)

#define ENET_TCSR_TF_MASK                        (0x80U)
#define ENET_TCSR_TF_SHIFT                       (7U)
#define ENET_TCSR_TF_WIDTH                       (1U)
#define ENET_TCSR_TF(x)                          (((uint32_t)(((uint32_t)(x)) << ENET_TCSR_TF_SHIFT)) & ENET_TCSR_TF_MASK)
/*! @} */

/*! @name TCCR - Timer Compare Capture Register */
/*! @{ */

#define ENET_TCCR_TCC_MASK                       (0xFFFFFFFFU)
#define ENET_TCCR_TCC_SHIFT                      (0U)
#define ENET_TCCR_TCC_WIDTH                      (32U)
#define ENET_TCCR_TCC(x)                         (((uint32_t)(((uint32_t)(x)) << ENET_TCCR_TCC_SHIFT)) & ENET_TCCR_TCC_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group ENET_Register_Masks */

/*!
 * @}
 */ /* end of group ENET_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_ENET_H_) */

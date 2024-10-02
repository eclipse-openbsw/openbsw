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
 * @file S32K148_FTFC.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_FTFC
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
#if !defined(S32K148_FTFC_H_)  /* Check if memory map has not been already included */
#define S32K148_FTFC_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- FTFC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTFC_Peripheral_Access_Layer FTFC Peripheral Access Layer
 * @{
 */

/** FTFC - Size of Registers Arrays */
#define FTFC_FCCOB_COUNT                          12u
#define FTFC_FPROT_COUNT                          4u

/** FTFC - Register Layout Typedef */
typedef struct {
  __IO uint8_t FSTAT;                              /**< Flash Status Register, offset: 0x0 */
  __IO uint8_t FCNFG;                              /**< Flash Configuration Register, offset: 0x1 */
  __I  uint8_t FSEC;                               /**< Flash Security Register, offset: 0x2 */
  __I  uint8_t FOPT;                               /**< Flash Option Register, offset: 0x3 */
  __IO uint8_t FCCOB[FTFC_FCCOB_COUNT];            /**< Flash Common Command Object Registers, array offset: 0x4, array step: 0x1 */
  __IO uint8_t FPROT[FTFC_FPROT_COUNT];            /**< Program Flash Protection Registers, array offset: 0x10, array step: 0x1 */
  uint8_t RESERVED_0[2];
  __IO uint8_t FEPROT;                             /**< EEPROM Protection Register, offset: 0x16 */
  __IO uint8_t FDPROT;                             /**< Data Flash Protection Register, offset: 0x17 */
  uint8_t RESERVED_1[20];
  __I  uint8_t FCSESTAT;                           /**< Flash CSEc Status Register, offset: 0x2C */
  uint8_t RESERVED_2[1];
  __IO uint8_t FERSTAT;                            /**< Flash Error Status Register, offset: 0x2E */
  __IO uint8_t FERCNFG;                            /**< Flash Error Configuration Register, offset: 0x2F */
} FTFC_Type, *FTFC_MemMapPtr;

/** Number of instances of the FTFC module. */
#define FTFC_INSTANCE_COUNT                      (1u)

/* FTFC - Peripheral instance base addresses */
/** Peripheral FTFC base address */
#define IP_FTFC_BASE                             (0x40020000u)
/** Peripheral FTFC base pointer */
#define IP_FTFC                                  ((FTFC_Type *)IP_FTFC_BASE)
/** Array initializer of FTFC peripheral base addresses */
#define IP_FTFC_BASE_ADDRS                       { IP_FTFC_BASE }
/** Array initializer of FTFC peripheral base pointers */
#define IP_FTFC_BASE_PTRS                        { IP_FTFC }

/* ----------------------------------------------------------------------------
   -- FTFC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FTFC_Register_Masks FTFC Register Masks
 * @{
 */

/*! @name FSTAT - Flash Status Register */
/*! @{ */

#define FTFC_FSTAT_MGSTAT0_MASK                  (0x1U)
#define FTFC_FSTAT_MGSTAT0_SHIFT                 (0U)
#define FTFC_FSTAT_MGSTAT0_WIDTH                 (1U)
#define FTFC_FSTAT_MGSTAT0(x)                    (((uint8_t)(((uint8_t)(x)) << FTFC_FSTAT_MGSTAT0_SHIFT)) & FTFC_FSTAT_MGSTAT0_MASK)

#define FTFC_FSTAT_FPVIOL_MASK                   (0x10U)
#define FTFC_FSTAT_FPVIOL_SHIFT                  (4U)
#define FTFC_FSTAT_FPVIOL_WIDTH                  (1U)
#define FTFC_FSTAT_FPVIOL(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FSTAT_FPVIOL_SHIFT)) & FTFC_FSTAT_FPVIOL_MASK)

#define FTFC_FSTAT_ACCERR_MASK                   (0x20U)
#define FTFC_FSTAT_ACCERR_SHIFT                  (5U)
#define FTFC_FSTAT_ACCERR_WIDTH                  (1U)
#define FTFC_FSTAT_ACCERR(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FSTAT_ACCERR_SHIFT)) & FTFC_FSTAT_ACCERR_MASK)

#define FTFC_FSTAT_RDCOLERR_MASK                 (0x40U)
#define FTFC_FSTAT_RDCOLERR_SHIFT                (6U)
#define FTFC_FSTAT_RDCOLERR_WIDTH                (1U)
#define FTFC_FSTAT_RDCOLERR(x)                   (((uint8_t)(((uint8_t)(x)) << FTFC_FSTAT_RDCOLERR_SHIFT)) & FTFC_FSTAT_RDCOLERR_MASK)

#define FTFC_FSTAT_CCIF_MASK                     (0x80U)
#define FTFC_FSTAT_CCIF_SHIFT                    (7U)
#define FTFC_FSTAT_CCIF_WIDTH                    (1U)
#define FTFC_FSTAT_CCIF(x)                       (((uint8_t)(((uint8_t)(x)) << FTFC_FSTAT_CCIF_SHIFT)) & FTFC_FSTAT_CCIF_MASK)
/*! @} */

/*! @name FCNFG - Flash Configuration Register */
/*! @{ */

#define FTFC_FCNFG_EEERDY_MASK                   (0x1U)
#define FTFC_FCNFG_EEERDY_SHIFT                  (0U)
#define FTFC_FCNFG_EEERDY_WIDTH                  (1U)
#define FTFC_FCNFG_EEERDY(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_EEERDY_SHIFT)) & FTFC_FCNFG_EEERDY_MASK)

#define FTFC_FCNFG_RAMRDY_MASK                   (0x2U)
#define FTFC_FCNFG_RAMRDY_SHIFT                  (1U)
#define FTFC_FCNFG_RAMRDY_WIDTH                  (1U)
#define FTFC_FCNFG_RAMRDY(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_RAMRDY_SHIFT)) & FTFC_FCNFG_RAMRDY_MASK)

#define FTFC_FCNFG_ERSSUSP_MASK                  (0x10U)
#define FTFC_FCNFG_ERSSUSP_SHIFT                 (4U)
#define FTFC_FCNFG_ERSSUSP_WIDTH                 (1U)
#define FTFC_FCNFG_ERSSUSP(x)                    (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_ERSSUSP_SHIFT)) & FTFC_FCNFG_ERSSUSP_MASK)

#define FTFC_FCNFG_ERSAREQ_MASK                  (0x20U)
#define FTFC_FCNFG_ERSAREQ_SHIFT                 (5U)
#define FTFC_FCNFG_ERSAREQ_WIDTH                 (1U)
#define FTFC_FCNFG_ERSAREQ(x)                    (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_ERSAREQ_SHIFT)) & FTFC_FCNFG_ERSAREQ_MASK)

#define FTFC_FCNFG_RDCOLLIE_MASK                 (0x40U)
#define FTFC_FCNFG_RDCOLLIE_SHIFT                (6U)
#define FTFC_FCNFG_RDCOLLIE_WIDTH                (1U)
#define FTFC_FCNFG_RDCOLLIE(x)                   (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_RDCOLLIE_SHIFT)) & FTFC_FCNFG_RDCOLLIE_MASK)

#define FTFC_FCNFG_CCIE_MASK                     (0x80U)
#define FTFC_FCNFG_CCIE_SHIFT                    (7U)
#define FTFC_FCNFG_CCIE_WIDTH                    (1U)
#define FTFC_FCNFG_CCIE(x)                       (((uint8_t)(((uint8_t)(x)) << FTFC_FCNFG_CCIE_SHIFT)) & FTFC_FCNFG_CCIE_MASK)
/*! @} */

/*! @name FSEC - Flash Security Register */
/*! @{ */

#define FTFC_FSEC_SEC_MASK                       (0x3U)
#define FTFC_FSEC_SEC_SHIFT                      (0U)
#define FTFC_FSEC_SEC_WIDTH                      (2U)
#define FTFC_FSEC_SEC(x)                         (((uint8_t)(((uint8_t)(x)) << FTFC_FSEC_SEC_SHIFT)) & FTFC_FSEC_SEC_MASK)

#define FTFC_FSEC_FSLACC_MASK                    (0xCU)
#define FTFC_FSEC_FSLACC_SHIFT                   (2U)
#define FTFC_FSEC_FSLACC_WIDTH                   (2U)
#define FTFC_FSEC_FSLACC(x)                      (((uint8_t)(((uint8_t)(x)) << FTFC_FSEC_FSLACC_SHIFT)) & FTFC_FSEC_FSLACC_MASK)

#define FTFC_FSEC_MEEN_MASK                      (0x30U)
#define FTFC_FSEC_MEEN_SHIFT                     (4U)
#define FTFC_FSEC_MEEN_WIDTH                     (2U)
#define FTFC_FSEC_MEEN(x)                        (((uint8_t)(((uint8_t)(x)) << FTFC_FSEC_MEEN_SHIFT)) & FTFC_FSEC_MEEN_MASK)

#define FTFC_FSEC_KEYEN_MASK                     (0xC0U)
#define FTFC_FSEC_KEYEN_SHIFT                    (6U)
#define FTFC_FSEC_KEYEN_WIDTH                    (2U)
#define FTFC_FSEC_KEYEN(x)                       (((uint8_t)(((uint8_t)(x)) << FTFC_FSEC_KEYEN_SHIFT)) & FTFC_FSEC_KEYEN_MASK)
/*! @} */

/*! @name FOPT - Flash Option Register */
/*! @{ */

#define FTFC_FOPT_OPT_MASK                       (0xFFU)
#define FTFC_FOPT_OPT_SHIFT                      (0U)
#define FTFC_FOPT_OPT_WIDTH                      (8U)
#define FTFC_FOPT_OPT(x)                         (((uint8_t)(((uint8_t)(x)) << FTFC_FOPT_OPT_SHIFT)) & FTFC_FOPT_OPT_MASK)
/*! @} */

/*! @name FCCOB - Flash Common Command Object Registers */
/*! @{ */

#define FTFC_FCCOB_CCOBn_MASK                    (0xFFU)
#define FTFC_FCCOB_CCOBn_SHIFT                   (0U)
#define FTFC_FCCOB_CCOBn_WIDTH                   (8U)
#define FTFC_FCCOB_CCOBn(x)                      (((uint8_t)(((uint8_t)(x)) << FTFC_FCCOB_CCOBn_SHIFT)) & FTFC_FCCOB_CCOBn_MASK)
/*! @} */

/*! @name FPROT - Program Flash Protection Registers */
/*! @{ */

#define FTFC_FPROT_PROT_MASK                     (0xFFU)
#define FTFC_FPROT_PROT_SHIFT                    (0U)
#define FTFC_FPROT_PROT_WIDTH                    (8U)
#define FTFC_FPROT_PROT(x)                       (((uint8_t)(((uint8_t)(x)) << FTFC_FPROT_PROT_SHIFT)) & FTFC_FPROT_PROT_MASK)
/*! @} */

/*! @name FEPROT - EEPROM Protection Register */
/*! @{ */

#define FTFC_FEPROT_EPROT_MASK                   (0xFFU)
#define FTFC_FEPROT_EPROT_SHIFT                  (0U)
#define FTFC_FEPROT_EPROT_WIDTH                  (8U)
#define FTFC_FEPROT_EPROT(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FEPROT_EPROT_SHIFT)) & FTFC_FEPROT_EPROT_MASK)
/*! @} */

/*! @name FDPROT - Data Flash Protection Register */
/*! @{ */

#define FTFC_FDPROT_DPROT_MASK                   (0xFFU)
#define FTFC_FDPROT_DPROT_SHIFT                  (0U)
#define FTFC_FDPROT_DPROT_WIDTH                  (8U)
#define FTFC_FDPROT_DPROT(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FDPROT_DPROT_SHIFT)) & FTFC_FDPROT_DPROT_MASK)
/*! @} */

/*! @name FCSESTAT - Flash CSEc Status Register */
/*! @{ */

#define FTFC_FCSESTAT_BSY_MASK                   (0x1U)
#define FTFC_FCSESTAT_BSY_SHIFT                  (0U)
#define FTFC_FCSESTAT_BSY_WIDTH                  (1U)
#define FTFC_FCSESTAT_BSY(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_BSY_SHIFT)) & FTFC_FCSESTAT_BSY_MASK)

#define FTFC_FCSESTAT_SB_MASK                    (0x2U)
#define FTFC_FCSESTAT_SB_SHIFT                   (1U)
#define FTFC_FCSESTAT_SB_WIDTH                   (1U)
#define FTFC_FCSESTAT_SB(x)                      (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_SB_SHIFT)) & FTFC_FCSESTAT_SB_MASK)

#define FTFC_FCSESTAT_BIN_MASK                   (0x4U)
#define FTFC_FCSESTAT_BIN_SHIFT                  (2U)
#define FTFC_FCSESTAT_BIN_WIDTH                  (1U)
#define FTFC_FCSESTAT_BIN(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_BIN_SHIFT)) & FTFC_FCSESTAT_BIN_MASK)

#define FTFC_FCSESTAT_BFN_MASK                   (0x8U)
#define FTFC_FCSESTAT_BFN_SHIFT                  (3U)
#define FTFC_FCSESTAT_BFN_WIDTH                  (1U)
#define FTFC_FCSESTAT_BFN(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_BFN_SHIFT)) & FTFC_FCSESTAT_BFN_MASK)

#define FTFC_FCSESTAT_BOK_MASK                   (0x10U)
#define FTFC_FCSESTAT_BOK_SHIFT                  (4U)
#define FTFC_FCSESTAT_BOK_WIDTH                  (1U)
#define FTFC_FCSESTAT_BOK(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_BOK_SHIFT)) & FTFC_FCSESTAT_BOK_MASK)

#define FTFC_FCSESTAT_RIN_MASK                   (0x20U)
#define FTFC_FCSESTAT_RIN_SHIFT                  (5U)
#define FTFC_FCSESTAT_RIN_WIDTH                  (1U)
#define FTFC_FCSESTAT_RIN(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_RIN_SHIFT)) & FTFC_FCSESTAT_RIN_MASK)

#define FTFC_FCSESTAT_EDB_MASK                   (0x40U)
#define FTFC_FCSESTAT_EDB_SHIFT                  (6U)
#define FTFC_FCSESTAT_EDB_WIDTH                  (1U)
#define FTFC_FCSESTAT_EDB(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_EDB_SHIFT)) & FTFC_FCSESTAT_EDB_MASK)

#define FTFC_FCSESTAT_IDB_MASK                   (0x80U)
#define FTFC_FCSESTAT_IDB_SHIFT                  (7U)
#define FTFC_FCSESTAT_IDB_WIDTH                  (1U)
#define FTFC_FCSESTAT_IDB(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FCSESTAT_IDB_SHIFT)) & FTFC_FCSESTAT_IDB_MASK)
/*! @} */

/*! @name FERSTAT - Flash Error Status Register */
/*! @{ */

#define FTFC_FERSTAT_DFDIF_MASK                  (0x2U)
#define FTFC_FERSTAT_DFDIF_SHIFT                 (1U)
#define FTFC_FERSTAT_DFDIF_WIDTH                 (1U)
#define FTFC_FERSTAT_DFDIF(x)                    (((uint8_t)(((uint8_t)(x)) << FTFC_FERSTAT_DFDIF_SHIFT)) & FTFC_FERSTAT_DFDIF_MASK)
/*! @} */

/*! @name FERCNFG - Flash Error Configuration Register */
/*! @{ */

#define FTFC_FERCNFG_DFDIE_MASK                  (0x2U)
#define FTFC_FERCNFG_DFDIE_SHIFT                 (1U)
#define FTFC_FERCNFG_DFDIE_WIDTH                 (1U)
#define FTFC_FERCNFG_DFDIE(x)                    (((uint8_t)(((uint8_t)(x)) << FTFC_FERCNFG_DFDIE_SHIFT)) & FTFC_FERCNFG_DFDIE_MASK)

#define FTFC_FERCNFG_FDFD_MASK                   (0x20U)
#define FTFC_FERCNFG_FDFD_SHIFT                  (5U)
#define FTFC_FERCNFG_FDFD_WIDTH                  (1U)
#define FTFC_FERCNFG_FDFD(x)                     (((uint8_t)(((uint8_t)(x)) << FTFC_FERCNFG_FDFD_SHIFT)) & FTFC_FERCNFG_FDFD_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group FTFC_Register_Masks */

/*!
 * @}
 */ /* end of group FTFC_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_FTFC_H_) */

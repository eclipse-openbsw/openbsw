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
 * @file S32K148_ERM.h
 * @version 1.1
 * @date 2022-02-02
 * @brief Peripheral Access Layer for S32K148_ERM
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
#if !defined(S32K148_ERM_H_)  /* Check if memory map has not been already included */
#define S32K148_ERM_H_

#include "S32K148_COMMON.h"

/* ----------------------------------------------------------------------------
   -- ERM Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ERM_Peripheral_Access_Layer ERM Peripheral Access Layer
 * @{
 */

/** ERM - Size of Registers Arrays */
#define ERM_EARn_COUNT                            2u

/** ERM - Register Layout Typedef */
typedef struct {
  __IO uint32_t CR0;                               /**< ERM Configuration Register 0, offset: 0x0 */
  uint8_t RESERVED_0[12];
  __IO uint32_t SR0;                               /**< ERM Status Register 0, offset: 0x10 */
  uint8_t RESERVED_1[236];
  struct {                                         /* offset: 0x100, array step: 0x10 */
    __I  uint32_t EAR;                               /**< ERM Memory n Error Address Register, array offset: 0x100, array step: 0x10 */
    uint8_t RESERVED_0[12];
  } EARn[ERM_EARn_COUNT];
} ERM_Type, *ERM_MemMapPtr;

/** Number of instances of the ERM module. */
#define ERM_INSTANCE_COUNT                       (1u)

/* ERM - Peripheral instance base addresses */
/** Peripheral ERM base address */
#define IP_ERM_BASE                              (0x40018000u)
/** Peripheral ERM base pointer */
#define IP_ERM                                   ((ERM_Type *)IP_ERM_BASE)
/** Array initializer of ERM peripheral base addresses */
#define IP_ERM_BASE_ADDRS                        { IP_ERM_BASE }
/** Array initializer of ERM peripheral base pointers */
#define IP_ERM_BASE_PTRS                         { IP_ERM }

/* ----------------------------------------------------------------------------
   -- ERM Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup ERM_Register_Masks ERM Register Masks
 * @{
 */

/*! @name CR0 - ERM Configuration Register 0 */
/*! @{ */

#define ERM_CR0_ENCIE1_MASK                      (0x4000000U)
#define ERM_CR0_ENCIE1_SHIFT                     (26U)
#define ERM_CR0_ENCIE1_WIDTH                     (1U)
#define ERM_CR0_ENCIE1(x)                        (((uint32_t)(((uint32_t)(x)) << ERM_CR0_ENCIE1_SHIFT)) & ERM_CR0_ENCIE1_MASK)

#define ERM_CR0_ESCIE1_MASK                      (0x8000000U)
#define ERM_CR0_ESCIE1_SHIFT                     (27U)
#define ERM_CR0_ESCIE1_WIDTH                     (1U)
#define ERM_CR0_ESCIE1(x)                        (((uint32_t)(((uint32_t)(x)) << ERM_CR0_ESCIE1_SHIFT)) & ERM_CR0_ESCIE1_MASK)

#define ERM_CR0_ENCIE0_MASK                      (0x40000000U)
#define ERM_CR0_ENCIE0_SHIFT                     (30U)
#define ERM_CR0_ENCIE0_WIDTH                     (1U)
#define ERM_CR0_ENCIE0(x)                        (((uint32_t)(((uint32_t)(x)) << ERM_CR0_ENCIE0_SHIFT)) & ERM_CR0_ENCIE0_MASK)

#define ERM_CR0_ESCIE0_MASK                      (0x80000000U)
#define ERM_CR0_ESCIE0_SHIFT                     (31U)
#define ERM_CR0_ESCIE0_WIDTH                     (1U)
#define ERM_CR0_ESCIE0(x)                        (((uint32_t)(((uint32_t)(x)) << ERM_CR0_ESCIE0_SHIFT)) & ERM_CR0_ESCIE0_MASK)
/*! @} */

/*! @name SR0 - ERM Status Register 0 */
/*! @{ */

#define ERM_SR0_NCE1_MASK                        (0x4000000U)
#define ERM_SR0_NCE1_SHIFT                       (26U)
#define ERM_SR0_NCE1_WIDTH                       (1U)
#define ERM_SR0_NCE1(x)                          (((uint32_t)(((uint32_t)(x)) << ERM_SR0_NCE1_SHIFT)) & ERM_SR0_NCE1_MASK)

#define ERM_SR0_SBC1_MASK                        (0x8000000U)
#define ERM_SR0_SBC1_SHIFT                       (27U)
#define ERM_SR0_SBC1_WIDTH                       (1U)
#define ERM_SR0_SBC1(x)                          (((uint32_t)(((uint32_t)(x)) << ERM_SR0_SBC1_SHIFT)) & ERM_SR0_SBC1_MASK)

#define ERM_SR0_NCE0_MASK                        (0x40000000U)
#define ERM_SR0_NCE0_SHIFT                       (30U)
#define ERM_SR0_NCE0_WIDTH                       (1U)
#define ERM_SR0_NCE0(x)                          (((uint32_t)(((uint32_t)(x)) << ERM_SR0_NCE0_SHIFT)) & ERM_SR0_NCE0_MASK)

#define ERM_SR0_SBC0_MASK                        (0x80000000U)
#define ERM_SR0_SBC0_SHIFT                       (31U)
#define ERM_SR0_SBC0_WIDTH                       (1U)
#define ERM_SR0_SBC0(x)                          (((uint32_t)(((uint32_t)(x)) << ERM_SR0_SBC0_SHIFT)) & ERM_SR0_SBC0_MASK)
/*! @} */

/*! @name EAR - ERM Memory n Error Address Register */
/*! @{ */

#define ERM_EAR_EAR_MASK                         (0xFFFFFFFFU)
#define ERM_EAR_EAR_SHIFT                        (0U)
#define ERM_EAR_EAR_WIDTH                        (32U)
#define ERM_EAR_EAR(x)                           (((uint32_t)(((uint32_t)(x)) << ERM_EAR_EAR_SHIFT)) & ERM_EAR_EAR_MASK)
/*! @} */

/*!
 * @}
 */ /* end of group ERM_Register_Masks */

/*!
 * @}
 */ /* end of group ERM_Peripheral_Access_Layer */

#endif  /* #if !defined(S32K148_ERM_H_) */

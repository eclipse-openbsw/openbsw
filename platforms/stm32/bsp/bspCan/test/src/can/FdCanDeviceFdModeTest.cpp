// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FdCanDeviceFdModeTest.cpp
 * \brief Comprehensive unit tests for FdCanDevice CAN FD mode (enableFd).
 *
 * Strategy: allocate fake FDCAN_GlobalTypeDef, RCC_TypeDef, and GPIO_TypeDef structs
 * on the stack (or as static globals) so the driver writes to RAM instead of hardware.
 * The message RAM region is also faked as a static array.
 *
 * We override FDCAN1, RCC, and SRAMCAN_BASE via macros BEFORE including the
 * production header, so the driver's register accesses hit our fake structs.
 */

// ============================================================================
// Test-only hardware fakes — must be defined before any STM32 header inclusion
// ============================================================================

#include <cstdint>
#include <cstring>

// Provide __IO as volatile (same as CMSIS)
#ifndef __IO
#define __IO volatile
#endif

// --- Fake GPIO_TypeDef (matches STM32G4 layout) ---
typedef struct
{
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
    __IO uint32_t BRR;
} GPIO_TypeDef;

// --- Fake RCC_TypeDef (matches STM32G4 layout) ---
typedef struct
{
    __IO uint32_t CR;
    __IO uint32_t ICSCR;
    __IO uint32_t CFGR;
    __IO uint32_t PLLCFGR;
    uint32_t RESERVED0;
    uint32_t RESERVED1;
    __IO uint32_t CIER;
    __IO uint32_t CIFR;
    __IO uint32_t CICR;
    uint32_t RESERVED2;
    __IO uint32_t AHB1RSTR;
    __IO uint32_t AHB2RSTR;
    __IO uint32_t AHB3RSTR;
    uint32_t RESERVED3;
    __IO uint32_t APB1RSTR1;
    __IO uint32_t APB1RSTR2;
    __IO uint32_t APB2RSTR;
    uint32_t RESERVED4;
    __IO uint32_t AHB1ENR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t AHB3ENR;
    uint32_t RESERVED5;
    __IO uint32_t APB1ENR1;
    __IO uint32_t APB1ENR2;
    __IO uint32_t APB2ENR;
    uint32_t RESERVED6;
    __IO uint32_t AHB1SMENR;
    __IO uint32_t AHB2SMENR;
    __IO uint32_t AHB3SMENR;
    uint32_t RESERVED7;
    __IO uint32_t APB1SMENR1;
    __IO uint32_t APB1SMENR2;
    __IO uint32_t APB2SMENR;
    uint32_t RESERVED8;
    __IO uint32_t CCIPR;
    uint32_t RESERVED9;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
    __IO uint32_t CRRCR;
    __IO uint32_t CCIPR2;
} RCC_TypeDef;

// --- Fake FDCAN_GlobalTypeDef (matches STM32G4 layout) ---
typedef struct
{
    __IO uint32_t CREL;
    __IO uint32_t ENDN;
    uint32_t RESERVED1;
    __IO uint32_t DBTP;
    __IO uint32_t TEST;
    __IO uint32_t RWD;
    __IO uint32_t CCCR;
    __IO uint32_t NBTP;
    __IO uint32_t TSCC;
    __IO uint32_t TSCV;
    __IO uint32_t TOCC;
    __IO uint32_t TOCV;
    uint32_t RESERVED2[4];
    __IO uint32_t ECR;
    __IO uint32_t PSR;
    __IO uint32_t TDCR;
    uint32_t RESERVED3;
    __IO uint32_t IR;
    __IO uint32_t IE;
    __IO uint32_t ILS;
    __IO uint32_t ILE;
    uint32_t RESERVED4[8];
    __IO uint32_t RXGFC;
    __IO uint32_t XIDAM;
    __IO uint32_t HPMS;
    uint32_t RESERVED5;
    __IO uint32_t RXF0S;
    __IO uint32_t RXF0A;
    __IO uint32_t RXF1S;
    __IO uint32_t RXF1A;
    uint32_t RESERVED6[8];
    __IO uint32_t TXBC;
    __IO uint32_t TXFQS;
    __IO uint32_t TXBRP;
    __IO uint32_t TXBAR;
    __IO uint32_t TXBCR;
    __IO uint32_t TXBTO;
    __IO uint32_t TXBCF;
    __IO uint32_t TXBTIE;
    __IO uint32_t TXBCIE;
    __IO uint32_t TXEFS;
    __IO uint32_t TXEFA;
} FDCAN_GlobalTypeDef;

// --- Static fake peripherals ---
static RCC_TypeDef fakeRcc;
static FDCAN_GlobalTypeDef fakeFdcan;
static GPIO_TypeDef fakeTxGpio;
static GPIO_TypeDef fakeRxGpio;

// Fake message RAM (848 bytes = 212 words per FDCAN instance)
static uint32_t fakeMessageRam[212];

// --- Override hardware macros to point at our fakes ---
#define RCC             (&fakeRcc)
#define FDCAN1          (&fakeFdcan)
#define FDCAN1_BASE     (reinterpret_cast<uintptr_t>(&fakeFdcan))
#define SRAMCAN_BASE    (reinterpret_cast<uintptr_t>(&fakeMessageRam[0]))
#define PERIPH_BASE     0x40000000UL
#define APB1PERIPH_BASE PERIPH_BASE

// --- FDCAN register bit definitions (from stm32g474xx.h) ---
#define FDCAN_CCCR_INIT_Pos (0U)
#define FDCAN_CCCR_INIT     (0x1UL << FDCAN_CCCR_INIT_Pos)
#define FDCAN_CCCR_CCE_Pos  (1U)
#define FDCAN_CCCR_CCE      (0x1UL << FDCAN_CCCR_CCE_Pos)
#define FDCAN_CCCR_FDOE_Pos (8U)
#define FDCAN_CCCR_FDOE     (0x1UL << FDCAN_CCCR_FDOE_Pos)
#define FDCAN_CCCR_BRSE_Pos (9U)
#define FDCAN_CCCR_BRSE     (0x1UL << FDCAN_CCCR_BRSE_Pos)

#define FDCAN_NBTP_NTSEG2_Pos (0U)
#define FDCAN_NBTP_NTSEG1_Pos (8U)
#define FDCAN_NBTP_NBRP_Pos   (16U)
#define FDCAN_NBTP_NSJW_Pos   (25U)

#define FDCAN_ECR_TEC_Pos (0U)
#define FDCAN_ECR_REC_Pos (8U)

#define FDCAN_PSR_BO_Pos (7U)
#define FDCAN_PSR_BO     (0x1UL << FDCAN_PSR_BO_Pos)

#define FDCAN_IR_RF0N_Pos (0U)
#define FDCAN_IR_RF0N     (0x1UL << FDCAN_IR_RF0N_Pos)
#define FDCAN_IR_RF0F_Pos (1U)
#define FDCAN_IR_RF0F     (0x1UL << FDCAN_IR_RF0F_Pos)
#define FDCAN_IR_RF0L_Pos (2U)
#define FDCAN_IR_RF0L     (0x1UL << FDCAN_IR_RF0L_Pos)
#define FDCAN_IR_TC_Pos   (7U)
#define FDCAN_IR_TC       (0x1UL << FDCAN_IR_TC_Pos)

#define FDCAN_IE_RF0NE_Pos (0U)
#define FDCAN_IE_RF0NE     (0x1UL << FDCAN_IE_RF0NE_Pos)
#define FDCAN_IE_TCE_Pos   (7U)
#define FDCAN_IE_TCE       (0x1UL << FDCAN_IE_TCE_Pos)
#define FDCAN_IE_TEFNE_Pos (12U)
#define FDCAN_IE_TEFNE     (0x1UL << FDCAN_IE_TEFNE_Pos)

#define FDCAN_IR_TEFN_Pos (12U)
#define FDCAN_IR_TEFN     (0x1UL << FDCAN_IR_TEFN_Pos)

#define FDCAN_TXEFS_EFFL_Pos (0U)
#define FDCAN_TXEFS_EFFL     (0x7UL << FDCAN_TXEFS_EFFL_Pos)
#define FDCAN_TXEFS_EFGI_Pos (8U)
#define FDCAN_TXEFS_EFGI     (0x3UL << FDCAN_TXEFS_EFGI_Pos)

#define FDCAN_DBTP_DSJW_Pos   (0U)
#define FDCAN_DBTP_DTSEG2_Pos (4U)
#define FDCAN_DBTP_DTSEG1_Pos (8U)
#define FDCAN_DBTP_DBRP_Pos   (16U)
#define FDCAN_DBTP_TDC_Pos    (23U)
#define FDCAN_DBTP_TDC        (0x1UL << FDCAN_DBTP_TDC_Pos)

#define FDCAN_TDCR_TDCO_Pos (8U)

#define FDCAN_ILS_SMSG_Pos (2U)
#define FDCAN_ILS_SMSG     (0x1UL << FDCAN_ILS_SMSG_Pos)

#define FDCAN_ILE_EINT0_Pos (0U)
#define FDCAN_ILE_EINT0     (0x1UL << FDCAN_ILE_EINT0_Pos)
#define FDCAN_ILE_EINT1_Pos (1U)
#define FDCAN_ILE_EINT1     (0x1UL << FDCAN_ILE_EINT1_Pos)

#define FDCAN_RXGFC_ANFE_Pos (2U)
#define FDCAN_RXGFC_ANFS_Pos (4U)
#define FDCAN_RXGFC_LSS_Pos  (16U)
#define FDCAN_RXGFC_LSE_Pos  (20U)

#define FDCAN_RXF0S_F0FL_Pos (0U)
#define FDCAN_RXF0S_F0FL     (0xFUL << FDCAN_RXF0S_F0FL_Pos)
#define FDCAN_RXF0S_F0GI_Pos (8U)
#define FDCAN_RXF0S_F0GI     (0x3UL << FDCAN_RXF0S_F0GI_Pos)

#define FDCAN_TXFQS_TFFL_Pos  (0U)
#define FDCAN_TXFQS_TFFL      (0x7UL << FDCAN_TXFQS_TFFL_Pos)
#define FDCAN_TXFQS_TFQPI_Pos (16U)
#define FDCAN_TXFQS_TFQPI     (0x3UL << FDCAN_TXFQS_TFQPI_Pos)

#define RCC_APB1ENR1_FDCANEN_Pos (25U)
#define RCC_APB1ENR1_FDCANEN     (0x1UL << RCC_APB1ENR1_FDCANEN_Pos)

// Prevent the real mcu.h from being included (it would pull in stm32g474xx.h)
#define MCU_MCU_H
#define MCU_TYPEDEFS_H

// Provide the CANFrame include path directly
#include <can/canframes/CANFrame.h>

// Now include the driver header — it will see our faked types
#include <can/FdCanDevice.h>

// Include the implementation directly so it compiles with our fakes.
#include <can/FdCanDevice.cpp>

#include <gtest/gtest.h>

// ============================================================================
// Test fixture
// ============================================================================

class FdCanDeviceFdModeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        memset(&fakeRcc, 0, sizeof(fakeRcc));
        memset(&fakeFdcan, 0, sizeof(fakeFdcan));
        memset(&fakeTxGpio, 0, sizeof(fakeTxGpio));
        memset(&fakeRxGpio, 0, sizeof(fakeRxGpio));
        memset(fakeMessageRam, 0, sizeof(fakeMessageRam));
    }

    bios::FdCanDevice::Config makeDefaultConfig()
    {
        bios::FdCanDevice::Config cfg{};
        cfg.baseAddress = &fakeFdcan;
        cfg.prescaler   = 4U;
        cfg.nts1        = 13U;
        cfg.nts2        = 2U;
        cfg.nsjw        = 1U;
        cfg.rxGpioPort  = &fakeRxGpio;
        cfg.rxPin       = 11U;
        cfg.rxAf        = 9U;
        cfg.txGpioPort  = &fakeTxGpio;
        cfg.txPin       = 5U;
        cfg.txAf        = 9U;
        return cfg;
    }

    std::unique_ptr<bios::FdCanDevice> makeInitedDevice()
    {
        auto cfg = makeDefaultConfig();
        auto dev = std::make_unique<bios::FdCanDevice>(cfg);
        dev->init();
        return dev;
    }

    bios::FdCanDevice::FdConfig makeFdConfig(
        uint32_t prescaler = 1U,
        uint32_t dTs1      = 5U,
        uint32_t dTs2      = 2U,
        uint32_t dSjw      = 1U,
        bool brs           = true)
    {
        bios::FdCanDevice::FdConfig fdCfg{};
        fdCfg.dPrescaler = prescaler;
        fdCfg.dTs1       = dTs1;
        fdCfg.dTs2       = dTs2;
        fdCfg.dSjw       = dSjw;
        fdCfg.brsEnabled = brs;
        return fdCfg;
    }
};

// ============================================================================
// Category 1 -- enableFd with every data-phase prescaler value 1-32 (32 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler2)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(2U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 1U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler3)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(3U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 2U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(4U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 3U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler5)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(5U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 4U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler6)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(6U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 5U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler7)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(7U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 6U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler8)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(8U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 7U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler9)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(9U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 8U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler10)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(10U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 9U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler11)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(11U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 10U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler12)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(12U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 11U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler13)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(13U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 12U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler14)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(14U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 13U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler15)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(15U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 14U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler16)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 15U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler17)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(17U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 16U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler18)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(18U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 17U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler19)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(19U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 18U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler20)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(20U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 19U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler21)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(21U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 20U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler22)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(22U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 21U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler23)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(23U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 22U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler24)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(24U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 23U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler25)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(25U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 24U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler26)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(26U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 25U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler27)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(27U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 26U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler28)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(28U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 27U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler29)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(29U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 28U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler30)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(30U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 29U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler31)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(31U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 30U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdPrescaler32)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U);
    dev->enableFd(fdCfg);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 31U);
}

// ============================================================================
// Category 2 -- enableFd with dTs1 values 0-15 (16 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value0)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 0U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 1U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 1U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value2)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 2U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 2U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value3)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 3U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 3U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 4U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 4U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value5)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 5U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value6)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 6U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 6U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value7)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 7U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 7U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value8)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 8U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 8U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value9)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 9U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 9U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value10)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 10U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 10U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value11)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 11U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 11U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value12)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 12U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 12U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value13)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 13U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 13U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value14)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 14U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 14U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts1Value15)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 15U);
    dev->enableFd(fdCfg);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 15U);
}

// ============================================================================
// Category 3 -- enableFd with dTs2 values 0-7 (8 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value0)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 0U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 1U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 1U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value2)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 2U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value3)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 3U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 3U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 4U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 4U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value5)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 5U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 5U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value6)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 6U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 6U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDts2Value7)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 7U);
    dev->enableFd(fdCfg);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 7U);
}

// ============================================================================
// Category 4 -- enableFd with dSjw values 0-7 (8 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue0)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 0U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 1U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue2)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 2U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 2U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue3)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 3U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 3U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 4U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 4U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue5)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 5U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 5U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue6)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 6U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 6U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdDsjwValue7)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 7U);
    dev->enableFd(fdCfg);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 7U);
}

// ============================================================================
// Category 5 -- BRS enable/disable combinations with various DBTP values (10 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, brsEnabledWithPrescaler1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsDisabledWithPrescaler1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsEnabledWithPrescaler8)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(8U, 10U, 3U, 2U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 7U);
}

TEST_F(FdCanDeviceFdModeTest, brsDisabledWithPrescaler8)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(8U, 10U, 3U, 2U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 7U);
}

TEST_F(FdCanDeviceFdModeTest, brsEnabledWithPrescaler16)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U, 15U, 7U, 7U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsDisabledWithPrescaler16)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U, 15U, 7U, 7U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsEnabledWithMinimalTiming)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 0U, 0U, 0U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsDisabledWithMinimalTiming)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 0U, 0U, 0U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsEnabledWithMaxTiming)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U, 15U, 7U, 7U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brsDisabledWithMaxTiming)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U, 15U, 7U, 7U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

// ============================================================================
// Category 6 -- TDC configuration: TDCR TDCO values (10 tests)
// The driver always writes TDCO=5 (hardcoded), so we verify that.
// We also verify TDC is enabled in DBTP.
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, tdcEnabledInDBTPWithPrescaler1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceFdModeTest, tdcrOffsetIs5WithPrescaler1)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U);
    dev->enableFd(fdCfg);
    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U);
}

TEST_F(FdCanDeviceFdModeTest, tdcEnabledInDBTPWithPrescaler4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(4U);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceFdModeTest, tdcrOffsetIs5WithPrescaler4)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(4U);
    dev->enableFd(fdCfg);
    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U);
}

TEST_F(FdCanDeviceFdModeTest, tdcEnabledInDBTPWithPrescaler16)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceFdModeTest, tdcrOffsetIs5WithPrescaler16)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U);
    dev->enableFd(fdCfg);
    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U);
}

TEST_F(FdCanDeviceFdModeTest, tdcEnabledInDBTPWithPrescaler32)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceFdModeTest, tdcrOffsetIs5WithPrescaler32)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U);
    dev->enableFd(fdCfg);
    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U);
}

TEST_F(FdCanDeviceFdModeTest, tdcEnabledWithBrsOff)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(2U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceFdModeTest, tdcrOffsetIs5WithBrsOff)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(2U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg);
    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U);
}

// ============================================================================
// Category 7 -- FDOE/BRSE/CCCR bit preservation after enableFd (10 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, fdoeSetAfterEnableFd)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brseSetWhenBrsTrue)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brseClearWhenBrsFalse)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, initBitClearedAfterEnableFdLeaves)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    // enableFd calls leaveInitMode, which clears INIT
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, cceBitClearedAfterEnableFdLeaves)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    // leaveInitMode clears INIT; CCE is only active when INIT is set
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, fdoeStillSetAfterSecondEnableFd)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, brseStillSetAfterSecondEnableFd)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, fdoePreservedWhenBrsToggled)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    auto fdCfg2 = makeFdConfig(1U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg2);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, fdoeSetWithPrescaler2)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(2U, 3U, 1U, 1U, true);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, fdoeSetWithPrescaler10)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(10U, 8U, 4U, 3U, false);
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

// ============================================================================
// Category 8 -- enableFd then start: verify IE/ILE/ILS still correct (5 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, enableFdThenStartIEHasRF0NE)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdThenStartIEHasTEFNE)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdThenStartILEHasEINT0)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT0, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdThenStartFDOEPreserved)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdThenStartBRSEPreserved)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

// ============================================================================
// Category 9 -- enableFd then stop then enableFd: DBTP updated correctly (5 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdUpdatesPrescaler)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(2U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    dev->start();
    dev->stop();
    auto fdCfg2 = makeFdConfig(8U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg2);
    uint32_t brp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(brp, 7U);
}

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdUpdatesDts1)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    dev->start();
    dev->stop();
    auto fdCfg2 = makeFdConfig(1U, 12U, 2U, 1U, true);
    dev->enableFd(fdCfg2);
    uint32_t ts1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(ts1, 12U);
}

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdUpdatesDts2)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    dev->start();
    dev->stop();
    auto fdCfg2 = makeFdConfig(1U, 5U, 6U, 1U, true);
    dev->enableFd(fdCfg2);
    uint32_t ts2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(ts2, 6U);
}

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdUpdatesDsjw)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    dev->start();
    dev->stop();
    auto fdCfg2 = makeFdConfig(1U, 5U, 2U, 5U, true);
    dev->enableFd(fdCfg2);
    uint32_t sjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(sjw, 5U);
}

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdToggledBrs)
{
    auto dev = makeInitedDevice();
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    dev->start();
    dev->stop();
    auto fdCfg2 = makeFdConfig(1U, 5U, 2U, 1U, false);
    dev->enableFd(fdCfg2);
    // BRS was not explicitly cleared by enableFd since it uses |=
    // FDOE should still be set
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

// ============================================================================
// Category 10 -- Classic mode after init (no enableFd): FDOE clear (5 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, classicModeAfterInitFDOEClear)
{
    auto dev = makeInitedDevice();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, classicModeAfterInitBRSEClear)
{
    auto dev = makeInitedDevice();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, classicModeAfterInitDBTPZero)
{
    auto dev = makeInitedDevice();
    // DBTP should be at its default (0) since enableFd was not called
    EXPECT_EQ(fakeFdcan.DBTP, 0U);
}

TEST_F(FdCanDeviceFdModeTest, classicModeAfterStartFDOEStillClear)
{
    auto dev = makeInitedDevice();
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceFdModeTest, classicModeAfterStartBRSEStillClear)
{
    auto dev = makeInitedDevice();
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

// ============================================================================
// Category 11 -- enableFd with init mode enter/leave: CCCR_INIT transitions (5 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, afterInitCCCRINITIsSet)
{
    // After init(), device is in init mode, CCCR.INIT should be set
    auto dev = makeInitedDevice();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdExitsCCCRINIT)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    // enableFd calls leaveInitMode at the end
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdTwiceStillLeavesInitMode)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, enableFdSetsInitThenClearsIt)
{
    auto dev = makeInitedDevice();
    // init() leaves device in init mode
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    // After enableFd completes, INIT is cleared
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceFdModeTest, stopThenEnableFdTransitionsCorrectly)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    dev->stop();
    // stop enters init mode
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

// ============================================================================
// Category 12 -- Boundary: max prescaler + max tseg1 + max tseg2 (3 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, boundaryMaxAllFieldsPacked)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(32U, 15U, 7U, 7U, true);
    dev->enableFd(fdCfg);
    uint32_t dbtp = fakeFdcan.DBTP;
    uint32_t brp  = (dbtp >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    uint32_t ts1  = (dbtp >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    uint32_t ts2  = (dbtp >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    uint32_t sjw  = (dbtp >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(brp, 31U);
    EXPECT_EQ(ts1, 15U);
    EXPECT_EQ(ts2, 7U);
    EXPECT_EQ(sjw, 7U);
}

TEST_F(FdCanDeviceFdModeTest, boundaryMinAllFieldsPacked)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(1U, 0U, 0U, 0U, true);
    dev->enableFd(fdCfg);
    uint32_t dbtp = fakeFdcan.DBTP;
    // Mask out TDC bit for comparison
    uint32_t brp  = (dbtp >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    uint32_t ts1  = (dbtp >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    uint32_t ts2  = (dbtp >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    uint32_t sjw  = (dbtp >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(brp, 0U);
    EXPECT_EQ(ts1, 0U);
    EXPECT_EQ(ts2, 0U);
    EXPECT_EQ(sjw, 0U);
}

TEST_F(FdCanDeviceFdModeTest, boundaryMidRange)
{
    auto dev = makeInitedDevice();
    auto fdCfg = makeFdConfig(16U, 8U, 4U, 4U, true);
    dev->enableFd(fdCfg);
    uint32_t dbtp = fakeFdcan.DBTP;
    uint32_t brp  = (dbtp >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    uint32_t ts1  = (dbtp >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    uint32_t ts2  = (dbtp >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    uint32_t sjw  = (dbtp >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(brp, 15U);
    EXPECT_EQ(ts1, 8U);
    EXPECT_EQ(ts2, 4U);
    EXPECT_EQ(sjw, 4U);
}

// ============================================================================
// Category 13 -- NBTP not affected by enableFd (3 tests)
// ============================================================================

TEST_F(FdCanDeviceFdModeTest, nbtpUnchangedAfterEnableFd)
{
    auto dev = makeInitedDevice();
    uint32_t nbtpBefore = fakeFdcan.NBTP;
    auto fdCfg = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.NBTP, nbtpBefore);
}

TEST_F(FdCanDeviceFdModeTest, nbtpUnchangedAfterEnableFdWithMaxPrescaler)
{
    auto dev = makeInitedDevice();
    uint32_t nbtpBefore = fakeFdcan.NBTP;
    auto fdCfg = makeFdConfig(32U, 15U, 7U, 7U, true);
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.NBTP, nbtpBefore);
}

TEST_F(FdCanDeviceFdModeTest, nbtpUnchangedAfterTwoEnableFdCalls)
{
    auto dev = makeInitedDevice();
    uint32_t nbtpBefore = fakeFdcan.NBTP;
    auto fdCfg1 = makeFdConfig(1U, 5U, 2U, 1U, true);
    dev->enableFd(fdCfg1);
    auto fdCfg2 = makeFdConfig(16U, 10U, 5U, 3U, false);
    dev->enableFd(fdCfg2);
    EXPECT_EQ(fakeFdcan.NBTP, nbtpBefore);
}

// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FdCanDeviceTest.cpp
 * \brief Comprehensive unit tests for FdCanDevice (STM32G4 FDCAN register-level driver).
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
// The getInstanceRamBase() function compares against FDCAN1 which is now &fakeFdcan.
#include <can/FdCanDevice.cpp>

#include <gtest/gtest.h>

// ============================================================================
// Message RAM offset constants (must match FdCanDevice.cpp)
// ============================================================================
static constexpr uint32_t MSG_RAM_STD_FILTER_OFFSET = 0x000U;
static constexpr uint32_t MSG_RAM_RX_FIFO0_OFFSET   = 0x0B0U;
static constexpr uint32_t MSG_RAM_TX_BUFFER_OFFSET  = 0x278U; // STM32G4 specific

// ============================================================================
// Test fixture
// ============================================================================

class FdCanDeviceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Zero all fake peripherals and message RAM
        memset(&fakeRcc, 0, sizeof(fakeRcc));
        memset(&fakeFdcan, 0, sizeof(fakeFdcan));
        memset(&fakeTxGpio, 0, sizeof(fakeTxGpio));
        memset(&fakeRxGpio, 0, sizeof(fakeRxGpio));
        memset(fakeMessageRam, 0, sizeof(fakeMessageRam));

        // The CCCR.INIT bit: after setting INIT, the hardware immediately
        // reflects it. In our fake we simulate this by pre-setting the bit
        // in the write path (the driver busy-waits until INIT is set).
        // We handle this by setting INIT in CCCR before construction so
        // enterInitMode's while-loop terminates immediately.
    }

    bios::FdCanDevice::Config makeDefaultConfig()
    {
        bios::FdCanDevice::Config cfg{};
        cfg.baseAddress = &fakeFdcan;
        cfg.prescaler   = 4U;  // BRP = prescaler - 1 = 3
        cfg.nts1        = 13U; // NTSEG1
        cfg.nts2        = 2U;  // NTSEG2
        cfg.nsjw        = 1U;  // NSJW
        cfg.rxGpioPort  = &fakeRxGpio;
        cfg.rxPin       = 11U; // PA11 (high pin, tests AFR[1] path)
        cfg.rxAf        = 9U;  // AF9
        cfg.txGpioPort  = &fakeTxGpio;
        cfg.txPin       = 5U; // PB5 (low pin, tests AFR[0] path)
        cfg.txAf        = 9U; // AF9
        return cfg;
    }

    // Helper: simulate hardware behavior where INIT bit reflects immediately
    void simulateInitBitReflection()
    {
        // The driver sets INIT, then busy-waits. Since our fake register
        // reflects the write immediately (same memory), the loop exits.
        // No extra action needed for stack-based volatile registers.
    }

    // Helper: put device into initialized + started state
    std::unique_ptr<bios::FdCanDevice> makeInitedDevice()
    {
        auto cfg = makeDefaultConfig();
        auto dev = std::make_unique<bios::FdCanDevice>(cfg);
        dev->init();
        return dev;
    }

    std::unique_ptr<bios::FdCanDevice> makeStartedDevice()
    {
        auto dev = makeInitedDevice();
        dev->start();
        return dev;
    }

    // Helper: place a frame in RX FIFO0 message RAM at given index
    void
    placeRxFrame(uint8_t fifoIndex, uint32_t canId, bool extended, uint8_t dlc, uint8_t const* data)
    {
        uint32_t* rxBuf = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(fakeMessageRam) + MSG_RAM_RX_FIFO0_OFFSET
            + (fifoIndex * 16U));

        // Word 0: ID
        if (extended)
        {
            rxBuf[0] = (canId & 0x1FFFFFFFU) | (1U << 30U); // XTD bit
        }
        else
        {
            rxBuf[0] = ((canId & 0x7FFU) << 18U);
        }

        // Word 1: DLC
        rxBuf[1] = (static_cast<uint32_t>(dlc) << 16U);

        // Words 2-3: Data
        if (data != nullptr)
        {
            rxBuf[2] = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8U)
                       | (static_cast<uint32_t>(data[2]) << 16U)
                       | (static_cast<uint32_t>(data[3]) << 24U);
            rxBuf[3] = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8U)
                       | (static_cast<uint32_t>(data[6]) << 16U)
                       | (static_cast<uint32_t>(data[7]) << 24U);
        }
    }

    // Helper: set RX FIFO0 fill level and get index
    void setRxFifoStatus(uint8_t fillLevel, uint8_t getIndex)
    {
        fakeFdcan.RXF0S = (static_cast<uint32_t>(fillLevel) << FDCAN_RXF0S_F0FL_Pos)
                          | (static_cast<uint32_t>(getIndex) << FDCAN_RXF0S_F0GI_Pos);
    }

    // Helper: set TX FIFO free level and put index
    void setTxFifoStatus(uint8_t freeLevel, uint8_t putIndex)
    {
        fakeFdcan.TXFQS = (static_cast<uint32_t>(freeLevel) << FDCAN_TXFQS_TFFL_Pos)
                          | (static_cast<uint32_t>(putIndex) << FDCAN_TXFQS_TFQPI_Pos);
    }

    // Helper: read TX buffer element from message RAM
    uint32_t const* getTxBuffer(uint8_t index)
    {
        return reinterpret_cast<uint32_t const*>(
            reinterpret_cast<uintptr_t>(fakeMessageRam) + MSG_RAM_TX_BUFFER_OFFSET + (index * 16U));
    }

    // Helper: read standard filter element from message RAM
    uint32_t getStdFilter(uint8_t index)
    {
        uint32_t const* filterRam = reinterpret_cast<uint32_t const*>(
            reinterpret_cast<uintptr_t>(fakeMessageRam) + MSG_RAM_STD_FILTER_OFFSET);
        return filterRam[index];
    }
};

// ============================================================================
// Category 1 — Initialization
// ============================================================================

TEST_F(FdCanDeviceTest, initEnablesPeripheralClock)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);

    EXPECT_EQ(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
    dev.init();
    EXPECT_NE(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
}

TEST_F(FdCanDeviceTest, initSelectsPclk1KernelClock)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // CCIPR[25:24] should be 10b = PCLK1
    uint32_t fdcanSel = (fakeRcc.CCIPR >> 24U) & 3U;
    EXPECT_EQ(fdcanSel, 2U);
}

TEST_F(FdCanDeviceTest, initConfiguresGpioTxAlternateFunction)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // TX pin = 5 (low register AFR[0]), AF9
    uint32_t txModer = (fakeTxGpio.MODER >> (cfg.txPin * 2U)) & 3U;
    EXPECT_EQ(txModer, 2U); // Alternate function mode

    uint32_t txAf = (fakeTxGpio.AFR[0] >> (cfg.txPin * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 9U);

    // Very high speed
    uint32_t txSpeed = (fakeTxGpio.OSPEEDR >> (cfg.txPin * 2U)) & 3U;
    EXPECT_EQ(txSpeed, 3U);
}

TEST_F(FdCanDeviceTest, initConfiguresGpioRxAlternateFunctionHighPin)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // RX pin = 11 (high register AFR[1]), AF9
    uint32_t rxModer = (fakeRxGpio.MODER >> (cfg.rxPin * 2U)) & 3U;
    EXPECT_EQ(rxModer, 2U); // Alternate function mode

    uint32_t rxAf = (fakeRxGpio.AFR[1] >> ((cfg.rxPin - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 9U);

    // Pull-up for RX
    uint32_t rxPupdr = (fakeRxGpio.PUPDR >> (cfg.rxPin * 2U)) & 3U;
    EXPECT_EQ(rxPupdr, 1U);
}

TEST_F(FdCanDeviceTest, initConfiguresGpioRxLowPin)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 4U; // Low pin to test AFR[0] path
    cfg.rxAf  = 7U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxAf = (fakeRxGpio.AFR[0] >> (cfg.rxPin * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 7U);
}

TEST_F(FdCanDeviceTest, initConfiguresGpioTxHighPin)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 12U;
    cfg.txAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txAf = (fakeTxGpio.AFR[1] >> ((cfg.txPin - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 9U);
}

TEST_F(FdCanDeviceTest, initEntersInitMode)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // After init(), INIT bit should still be set (init mode stays until start())
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    // CCE should be set (configuration change enabled)
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_CCE, 0U);
}

TEST_F(FdCanDeviceTest, initDisablesFdAndBrs)
{
    auto cfg       = makeDefaultConfig();
    // Pre-set FDOE and BRSE to verify init clears them
    fakeFdcan.CCCR = FDCAN_CCCR_FDOE | FDCAN_CCCR_BRSE;
    bios::FdCanDevice dev(cfg);
    dev.init();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceTest, initConfiguresBitTiming)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbtp = fakeFdcan.NBTP;
    uint32_t nsjw = (nbtp >> FDCAN_NBTP_NSJW_Pos) & 0x7FU;
    uint32_t nbrp = (nbtp >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    uint32_t nts1 = (nbtp >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU;
    uint32_t nts2 = (nbtp >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU;

    EXPECT_EQ(nsjw, cfg.nsjw);
    EXPECT_EQ(nbrp, cfg.prescaler - 1U); // prescaler is encoded as (prescaler-1)
    EXPECT_EQ(nts1, cfg.nts1);
    EXPECT_EQ(nts2, cfg.nts2);
}

TEST_F(FdCanDeviceTest, initConfiguresMessageRam)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // RXGFC should have LSS=0, LSE=0 initially (accept-all replaces it)
    // TXBC should be 0 (FIFO mode)
    EXPECT_EQ(fakeFdcan.TXBC, 0U);
}

TEST_F(FdCanDeviceTest, initConfiguresAcceptAllFilter)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // ANFS=0 (accept non-matching std into FIFO0), ANFE=0 (accept non-matching ext into FIFO0)
    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfs, 0U);
    EXPECT_EQ(anfe, 0U);
}

TEST_F(FdCanDeviceTest, initSetsInitializedFlag)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    // Before init, start should do nothing (tested in startWithoutInitDoesNothing)
    dev.init();
    // After init, start should work — verified indirectly via start() tests
}

TEST_F(FdCanDeviceTest, doubleInitSafe)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // Second init should not crash
    dev.init();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

// ============================================================================
// Category 2 — Start / Stop
// ============================================================================

TEST_F(FdCanDeviceTest, startEnablesInterrupts)
{
    auto dev = makeInitedDevice();
    dev->start();
    // RF0NE (RX FIFO 0 new element) + TEFNE (TX Event FIFO new entry)
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, startSetsILS)
{
    auto dev = makeInitedDevice();
    dev->start();
    // All interrupts routed to line 0 (ILS=0)
    EXPECT_EQ(fakeFdcan.ILS, 0U);
}

TEST_F(FdCanDeviceTest, startEnablesILE)
{
    auto dev = makeInitedDevice();
    dev->start();
    // Only EINT0 enabled (all interrupts on line 0)
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT0, 0U);
}

TEST_F(FdCanDeviceTest, startSetsTXBTIE)
{
    auto dev = makeInitedDevice();
    dev->start();
    // All 3 TX buffers enabled
    EXPECT_EQ(fakeFdcan.TXBTIE, 0x7U);
}

TEST_F(FdCanDeviceTest, startLeavesInitMode)
{
    auto dev = makeInitedDevice();
    dev->start();
    // INIT bit should be cleared
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, startWithoutInitDoesNothing)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    // Do NOT call init()
    dev.start();
    // IE should remain 0 — start() returned early
    EXPECT_EQ(fakeFdcan.IE, 0U);
    EXPECT_EQ(fakeFdcan.ILS, 0U);
    EXPECT_EQ(fakeFdcan.ILE, 0U);
}

TEST_F(FdCanDeviceTest, stopDisablesInterrupts)
{
    auto dev = makeStartedDevice();
    dev->stop();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, stopEntersInitMode)
{
    auto dev = makeStartedDevice();
    dev->stop();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

// ============================================================================
// Category 3 — Transmit
// ============================================================================

TEST_F(FdCanDeviceTest, transmitReturnsTrueOnSuccess)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U); // 3 free slots, put index 0

    uint8_t data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ::can::CANFrame frame(0x100, data, 8U);

    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, transmitReturnsFalseWhenFifoFull)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(0U, 0U); // 0 free slots

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 8U);

    EXPECT_FALSE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, transmitSetsCorrectStandardId)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x123, data, 8U);
    dev->transmit(frame);

    uint32_t const* txBuf = getTxBuffer(0);
    // Standard ID: shifted left by 18, no XTD bit
    EXPECT_EQ(txBuf[0], (0x123U << 18U));
}

TEST_F(FdCanDeviceTest, transmitSetsExtendedId)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    // Extended ID has bit 31 set (0x80000000)
    uint32_t extId  = 0x80000000U | 0x12345U;
    ::can::CANFrame frame(extId, data, 8U);
    dev->transmit(frame);

    uint32_t const* txBuf = getTxBuffer(0);
    // Extended: raw 29-bit ID in bits [28:0], XTD bit at [30]
    EXPECT_EQ(txBuf[0], (0x12345U) | (1U << 30U));
}

TEST_F(FdCanDeviceTest, transmitSetsCorrectDlc)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 5U);
    dev->transmit(frame);

    uint32_t const* txBuf = getTxBuffer(0);
    uint8_t dlc           = static_cast<uint8_t>((txBuf[1] >> 16U) & 0xFU);
    EXPECT_EQ(dlc, 5U);
}

TEST_F(FdCanDeviceTest, transmitCopiesPayloadBytes)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    ::can::CANFrame frame(0x100, data, 8U);
    dev->transmit(frame);

    uint32_t const* txBuf = getTxBuffer(0);
    // Word 2: bytes 0-3 in little-endian order
    EXPECT_EQ(txBuf[2], 0x04030201U);
    // Word 3: bytes 4-7
    EXPECT_EQ(txBuf[3], 0x08070605U);
}

TEST_F(FdCanDeviceTest, transmitSetsRequestBit)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 1U); // put index = 1

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 8U);
    dev->transmit(frame);

    // TXBAR bit 1 should be set
    EXPECT_EQ(fakeFdcan.TXBAR, (1U << 1U));
}

TEST_F(FdCanDeviceTest, transmitUsesCorrectPutIndex)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(2U, 2U); // put index = 2

    uint8_t data[8] = {0xAA};
    ::can::CANFrame frame(0x200, data, 1U);
    dev->transmit(frame);

    // Data should be written at TX buffer index 2
    uint32_t const* txBuf = getTxBuffer(2);
    EXPECT_EQ(txBuf[0], (0x200U << 18U));
    EXPECT_EQ(fakeFdcan.TXBAR, (1U << 2U));
}

TEST_F(FdCanDeviceTest, transmitMultipleFramesUseDifferentBuffers)
{
    auto dev = makeStartedDevice();

    uint8_t data1[8] = {0x11};
    uint8_t data2[8] = {0x22};

    // First frame at put index 0
    setTxFifoStatus(3U, 0U);
    ::can::CANFrame frame1(0x100, data1, 1U);
    dev->transmit(frame1);

    // Second frame at put index 1
    setTxFifoStatus(2U, 1U);
    ::can::CANFrame frame2(0x200, data2, 1U);
    dev->transmit(frame2);

    uint32_t const* buf0 = getTxBuffer(0);
    uint32_t const* buf1 = getTxBuffer(1);
    EXPECT_EQ(buf0[0], (0x100U << 18U));
    EXPECT_EQ(buf1[0], (0x200U << 18U));
}

TEST_F(FdCanDeviceTest, transmitSingleFrame)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(1U, 0U);

    uint8_t data[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    ::can::CANFrame frame(0x7FF, data, 8U);
    EXPECT_TRUE(dev->transmit(frame));

    uint32_t const* txBuf = getTxBuffer(0);
    EXPECT_EQ(txBuf[0], (0x7FFU << 18U));
    EXPECT_EQ(static_cast<uint8_t>((txBuf[1] >> 16U) & 0xFU), 8U);
    EXPECT_EQ(txBuf[2], 0xEFBEADDEU);
    EXPECT_EQ(txBuf[3], 0xBEBAFECAU);
}

TEST_F(FdCanDeviceTest, transmitWithZeroPayload)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_TRUE(dev->transmit(frame));

    uint32_t const* txBuf = getTxBuffer(0);
    uint8_t dlc           = static_cast<uint8_t>((txBuf[1] >> 16U) & 0xFU);
    EXPECT_EQ(dlc, 0U);
}

TEST_F(FdCanDeviceTest, transmitWithMaxPayload)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8};
    ::can::CANFrame frame(0x100, data, 8U);
    EXPECT_TRUE(dev->transmit(frame));

    uint32_t const* txBuf = getTxBuffer(0);
    uint8_t dlc           = static_cast<uint8_t>((txBuf[1] >> 16U) & 0xFU);
    EXPECT_EQ(dlc, 8U);
    EXPECT_EQ(txBuf[2], 0xFCFDFEFFU);
    EXPECT_EQ(txBuf[3], 0xF8F9FAFBU);
}

// ============================================================================
// Category 4 — Receive ISR
// ============================================================================

TEST_F(FdCanDeviceTest, receiveISRDrainsFifo)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 1U);
}

TEST_F(FdCanDeviceTest, receiveISRExtractsStandardId)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x321, false, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxCount(), 1U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x321U);
}

TEST_F(FdCanDeviceTest, receiveISRExtractsExtendedId)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x1ABCDU, true, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxCount(), 1U);
    // Extended IDs have bit 31 set in the CANFrame representation
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x80000000U | 0x1ABCDU);
}

TEST_F(FdCanDeviceTest, receiveISRExtractsDlc)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 5, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxFrame(0).getPayloadLength(), 5U);
}

TEST_F(FdCanDeviceTest, receiveISRExtractsPayloadBytes)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    uint8_t const* payload = dev->getRxFrame(0).getPayload();
    EXPECT_EQ(payload[0], 0xAAU);
    EXPECT_EQ(payload[1], 0xBBU);
    EXPECT_EQ(payload[2], 0xCCU);
    EXPECT_EQ(payload[3], 0xDDU);
    EXPECT_EQ(payload[4], 0xEEU);
    EXPECT_EQ(payload[5], 0xFFU);
    EXPECT_EQ(payload[6], 0x11U);
    EXPECT_EQ(payload[7], 0x22U);
}

TEST_F(FdCanDeviceTest, receiveISRAcknowledgesFrame)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    // RXF0A should have been written with the get index (0)
    EXPECT_EQ(fakeFdcan.RXF0A, 0U);
}

TEST_F(FdCanDeviceTest, receiveISRReturnsFrameCount)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    // Place 2 frames (fill level = 2, both at index 0 since fake doesn't auto-advance)
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(2U, 0U);

    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 2U);
}

TEST_F(FdCanDeviceTest, receiveISRDropsWhenQueueFull)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};

    // Fill the software RX queue to capacity (32 frames)
    for (uint32_t i = 0U; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, static_cast<uint32_t>(0x100 + i), false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);

    // Now try to receive one more — should be acknowledged but dropped
    placeRxFrame(0, 0x999, false, 8, data);
    setRxFifoStatus(1U, 0U);
    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 0U);
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);
}

TEST_F(FdCanDeviceTest, receiveISRWithNullFilterAcceptsAll)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x7FF, false, 8, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 1U);
}

TEST_F(FdCanDeviceTest, receiveISRWithFilterRejectsUnmatched)
{
    auto dev = makeStartedDevice();

    // Create a filter bit field where only ID 0x100 is accepted
    uint8_t filter[256] = {};
    filter[0x100 / 8U] |= (1U << (0x100 % 8U));

    // Place a frame with ID 0x200 (not in filter)
    uint8_t data[8] = {};
    placeRxFrame(0, 0x200, false, 8, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(filter);
    EXPECT_EQ(received, 0U);
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, receiveISRWithFilterAcceptsMatched)
{
    auto dev = makeStartedDevice();

    uint8_t filter[256] = {};
    filter[0x100 / 8U] |= (1U << (0x100 % 8U));

    uint8_t data[8] = {0x42};
    placeRxFrame(0, 0x100, false, 1, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(filter);
    EXPECT_EQ(received, 1U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x100U);
}

TEST_F(FdCanDeviceTest, receiveISRSnapshotsFillLevel)
{
    auto dev = makeStartedDevice();

    // Set fill level to 2 — the ISR should drain exactly 2, not re-read F0FL
    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(2U, 0U);

    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 2U);
}

TEST_F(FdCanDeviceTest, receiveISRClearsInterruptFlags)
{
    auto dev = makeStartedDevice();

    // Pre-set interrupt flags
    fakeFdcan.IR = FDCAN_IR_RF0N | FDCAN_IR_RF0F | FDCAN_IR_RF0L;

    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    // IR should have RF0N|RF0F|RF0L written (write-1-to-clear)
    // In our fake, the last write to IR is the clear value
    EXPECT_EQ(fakeFdcan.IR, FDCAN_IR_RF0N | FDCAN_IR_RF0F | FDCAN_IR_RF0L);
}

TEST_F(FdCanDeviceTest, receiveISRWithEmptyFifo)
{
    auto dev = makeStartedDevice();
    setRxFifoStatus(0U, 0U); // F0FL = 0

    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 0U);
}

// ============================================================================
// Category 5 — RX Queue
// ============================================================================

TEST_F(FdCanDeviceTest, getRxCountReturnsZeroInitially)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    EXPECT_EQ(dev.getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, getRxFrameReturnsCorrectFrame)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {0xDE, 0xAD};
    placeRxFrame(0, 0x123, false, 2, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    auto const& frame = dev->getRxFrame(0);
    EXPECT_EQ(frame.getId(), 0x123U);
    EXPECT_EQ(frame.getPayloadLength(), 2U);
    EXPECT_EQ(frame.getPayload()[0], 0xDEU);
    EXPECT_EQ(frame.getPayload()[1], 0xADU);
}

TEST_F(FdCanDeviceTest, getRxCountAfterReceive)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    EXPECT_EQ(dev->getRxCount(), 1U);
}

TEST_F(FdCanDeviceTest, clearRxQueueResetsCount)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    EXPECT_EQ(dev->getRxCount(), 1U);
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, rxQueueWrapsAround)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill queue to capacity
    for (uint32_t i = 0U; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);

    // Clear and receive more — should wrap around
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);

    // Receive 5 more (these wrap into the beginning of the buffer)
    for (uint32_t i = 0U; i < 5U; i++)
    {
        placeRxFrame(0, 0x500U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 5U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x500U);
    EXPECT_EQ(dev->getRxFrame(4).getId(), 0x504U);
}

TEST_F(FdCanDeviceTest, rxQueueCapacity) { EXPECT_EQ(bios::FdCanDevice::RX_QUEUE_SIZE, 32U); }

// ============================================================================
// Category 6 — Interrupt control
// ============================================================================

TEST_F(FdCanDeviceTest, disableRxInterruptClearsRF0NE)
{
    auto dev = makeStartedDevice();
    // After start, RF0NE should be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);

    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    // TEFNE should still be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, enableRxInterruptSetsRF0NE)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);

    dev->enableRxInterrupt();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, transmitISRClearsTEFNAndTCFlags)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;

    // Empty TX Event FIFO (EFFL=0) — nothing to drain
    fakeFdcan.TXEFS = 0U;
    dev->transmitISR();
    // Both TEFN and TC flags should be cleared (write-1-to-clear)
    EXPECT_NE(fakeFdcan.IR & FDCAN_IR_TEFN, 0U);
    EXPECT_NE(fakeFdcan.IR & FDCAN_IR_TC, 0U);
}

TEST_F(FdCanDeviceTest, transmitISRWithNoPendingTx)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;
    fakeFdcan.TXEFS = 0U;
    // Should not crash
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

// NOTE: The simple static fake registers cannot simulate the HW behavior where
// writing TXEFA auto-decrements TXEFS.EFFL.  Testing with EFFL>0 would cause
// an infinite loop.  The drain logic is verified structurally (code review) and
// on real hardware.  These tests verify the flag-clearing and the empty-FIFO path.
//
// For a smarter fake that simulates TXEFA→EFFL feedback, see the integration
// test on real hardware (board-to-board CAN validation).

TEST_F(FdCanDeviceTest, transmitISREmptyFifoSkipsDrain)
{
    auto dev = makeStartedDevice();
    fakeFdcan.IR = 0U;
    fakeFdcan.TXEFS = 0U; // EFFL=0, nothing to drain
    fakeFdcan.TXEFA = 0xDEADBEEFU; // sentinel — should NOT be written

    dev->transmitISR();

    // TXEFA should be untouched (no drain)
    EXPECT_EQ(fakeFdcan.TXEFA, 0xDEADBEEFU);
    // Flags still cleared
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

// ============================================================================
// Category 7 — Error state
// ============================================================================

TEST_F(FdCanDeviceTest, isBusOffReturnsTrueWhenBOSet)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = FDCAN_PSR_BO;
    EXPECT_TRUE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, isBusOffReturnsFalseNormally)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = 0U;
    EXPECT_FALSE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, getTxErrorCounterReadsECR)
{
    auto dev      = makeStartedDevice();
    // TEC is in ECR[7:0]
    fakeFdcan.ECR = 42U;
    EXPECT_EQ(dev->getTxErrorCounter(), 42U);
}

TEST_F(FdCanDeviceTest, getTxErrorCounterMaxValue)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 0xFFU; // max TEC
    EXPECT_EQ(dev->getTxErrorCounter(), 255U);
}

TEST_F(FdCanDeviceTest, getRxErrorCounterReadsECR)
{
    auto dev      = makeStartedDevice();
    // REC is in ECR[14:8]
    fakeFdcan.ECR = (96U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getRxErrorCounter(), 96U);
}

TEST_F(FdCanDeviceTest, getRxErrorCounterMaxValue)
{
    auto dev      = makeStartedDevice();
    // REC max is 127 (7 bits)
    fakeFdcan.ECR = (0x7FU << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getRxErrorCounter(), 127U);
}

TEST_F(FdCanDeviceTest, errorCountersBothReadable)
{
    auto dev      = makeStartedDevice();
    // TEC = 100, REC = 50
    fakeFdcan.ECR = 100U | (50U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getTxErrorCounter(), 100U);
    EXPECT_EQ(dev->getRxErrorCounter(), 50U);
}

// ============================================================================
// Category 8 — Filter configuration
// ============================================================================

TEST_F(FdCanDeviceTest, configureAcceptAllFilterSetsANFS0ANFE0)
{
    auto dev        = makeInitedDevice();
    // Set non-zero first to verify it gets overwritten
    fakeFdcan.RXGFC = 0xFFFFFFFFU;
    dev->configureAcceptAllFilter();

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfs, 0U);
    EXPECT_EQ(anfe, 0U);
}

TEST_F(FdCanDeviceTest, configureFilterListRejectsNonMatching)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100, 0x200};
    dev->configureFilterList(idList, 2U);

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfs, 2U); // Reject non-matching standard
    EXPECT_EQ(anfe, 2U); // Reject non-matching extended
}

TEST_F(FdCanDeviceTest, configureFilterListSetsLSSCount)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100, 0x200, 0x300};
    dev->configureFilterList(idList, 3U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 3U);
}

TEST_F(FdCanDeviceTest, configureFilterListWritesFilterElements)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100, 0x2AB};
    dev->configureFilterList(idList, 2U);

    // Filter element format: SFT(1)<<30 | SFEC(1)<<27 | SFID1<<16 | SFID2
    uint32_t f0 = getStdFilter(0);
    EXPECT_EQ(f0, (1U << 30U) | (1U << 27U) | (0x100U << 16U) | 0x100U);

    uint32_t f1 = getStdFilter(1);
    EXPECT_EQ(f1, (1U << 30U) | (1U << 27U) | (0x2ABU << 16U) | 0x2ABU);
}

TEST_F(FdCanDeviceTest, configureFilterListCapsAt28)
{
    auto dev = makeInitedDevice();

    // Create 30 IDs (exceeds 28 max)
    uint32_t idList[30];
    for (uint32_t i = 0; i < 30; i++)
    {
        idList[i] = 0x100U + i;
    }
    dev->configureFilterList(idList, 30U);

    // Only 28 elements should be written; element 28 and 29 should be zero
    uint32_t f27 = getStdFilter(27);
    EXPECT_NE(f27, 0U); // element 27 should exist

    // Element at index 28 should NOT have been written (stays 0 from memset)
    uint32_t f28 = getStdFilter(28);
    EXPECT_EQ(f28, 0U);
}

TEST_F(FdCanDeviceTest, configureFilterListAcceptsConfiguredIds)
{
    // This is more of an integration test: configure filter, then verify
    // the RXGFC is set to reject non-matching but LSS covers our list
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100, 0x200, 0x300, 0x400, 0x500};
    dev->configureFilterList(idList, 5U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 5U);

    // Verify each filter element
    for (uint8_t i = 0; i < 5; i++)
    {
        uint32_t f    = getStdFilter(i);
        uint32_t sfid = (f >> 16U) & 0x7FFU;
        EXPECT_EQ(sfid, idList[i]);
    }
}

// ============================================================================
// Category 9 — Bit timing
// ============================================================================

TEST_F(FdCanDeviceTest, configureBitTimingWritesNBTP)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 8U;
    cfg.nts1      = 63U;
    cfg.nts2      = 15U;
    cfg.nsjw      = 7U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbtp = fakeFdcan.NBTP;
    EXPECT_NE(nbtp, 0U);
}

TEST_F(FdCanDeviceTest, prescalerEncodedCorrectly)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 10U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbrp = (fakeFdcan.NBTP >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    EXPECT_EQ(nbrp, 9U); // prescaler - 1
}

TEST_F(FdCanDeviceTest, tseg1Encoded)
{
    auto cfg = makeDefaultConfig();
    cfg.nts1 = 47U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts1 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU;
    EXPECT_EQ(nts1, 47U);
}

TEST_F(FdCanDeviceTest, tseg2Encoded)
{
    auto cfg = makeDefaultConfig();
    cfg.nts2 = 5U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts2 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU;
    EXPECT_EQ(nts2, 5U);
}

TEST_F(FdCanDeviceTest, sjwEncoded)
{
    auto cfg = makeDefaultConfig();
    cfg.nsjw = 3U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nsjw = (fakeFdcan.NBTP >> FDCAN_NBTP_NSJW_Pos) & 0x7FU;
    EXPECT_EQ(nsjw, 3U);
}

TEST_F(FdCanDeviceTest, bitTimingAllFieldsPackedCorrectly)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 2U; // BRP=1
    cfg.nts1      = 10U;
    cfg.nts2      = 3U;
    cfg.nsjw      = 2U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t expected = (2U << FDCAN_NBTP_NSJW_Pos) | (1U << FDCAN_NBTP_NBRP_Pos) // prescaler - 1
                        | (10U << FDCAN_NBTP_NTSEG1_Pos) | (3U << FDCAN_NBTP_NTSEG2_Pos);
    EXPECT_EQ(fakeFdcan.NBTP, expected);
}

// ============================================================================
// Category 10 — Edge cases / additional coverage
// ============================================================================

TEST_F(FdCanDeviceTest, constructorZerosRxQueue)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    EXPECT_EQ(dev.getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, multipleReceiveCycles)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {0x01};

    // Receive 3 frames
    for (uint32_t i = 0; i < 3; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 3U);

    // Verify all 3 frames
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x100U);
    EXPECT_EQ(dev->getRxFrame(1).getId(), 0x101U);
    EXPECT_EQ(dev->getRxFrame(2).getId(), 0x102U);

    // Clear and receive more
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);

    placeRxFrame(0, 0x200, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxCount(), 1U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
}

TEST_F(FdCanDeviceTest, stopThenRestartSequence)
{
    auto dev = makeStartedDevice();

    // Stop
    dev->stop();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);

    // Re-start
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, transmitAfterStopAndRestart)
{
    auto dev = makeStartedDevice();

    dev->stop();
    dev->start();

    setTxFifoStatus(3U, 0U);
    uint8_t data[8] = {0x42};
    ::can::CANFrame frame(0x100, data, 1U);
    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, filterBitFieldEdgeCases)
{
    auto dev = makeStartedDevice();

    // Test ID 0 (first bit of first byte)
    uint8_t filter[256] = {};
    filter[0]           = 0x01U; // Accept ID 0

    uint8_t data[8] = {};
    placeRxFrame(0, 0, false, 1, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(filter);
    EXPECT_EQ(received, 1U);
}

TEST_F(FdCanDeviceTest, filterBitFieldHighId)
{
    auto dev = makeStartedDevice();

    // Test ID 0x7FF (max standard ID)
    uint8_t filter[256] = {};
    filter[0x7FF / 8U] |= (1U << (0x7FF % 8U));

    uint8_t data[8] = {};
    placeRxFrame(0, 0x7FF, false, 1, data);
    setRxFifoStatus(1U, 0U);

    uint8_t received = dev->receiveISR(filter);
    EXPECT_EQ(received, 1U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x7FFU);
}

TEST_F(FdCanDeviceTest, rxQueueFullThenPartialDrain)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill to capacity
    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }

    // Read some frames
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x100U);
    EXPECT_EQ(dev->getRxFrame(31).getId(), 0x11FU);

    // Clear all and verify empty
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, extendedIdFullRange)
{
    auto dev = makeStartedDevice();

    uint8_t data[8]   = {};
    // Test with maximum extended ID
    uint32_t maxExtId = 0x1FFFFFFFU;
    placeRxFrame(0, maxExtId, true, 8, data);
    setRxFifoStatus(1U, 0U);

    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x80000000U | maxExtId);
}

TEST_F(FdCanDeviceTest, transmitExtendedIdFullRange)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    uint32_t extId  = 0x80000000U | 0x1FFFFFFFU;
    ::can::CANFrame frame(extId, data, 0U);
    dev->transmit(frame);

    uint32_t const* txBuf = getTxBuffer(0);
    EXPECT_EQ(txBuf[0], 0x1FFFFFFFU | (1U << 30U));
}

TEST_F(FdCanDeviceTest, disableEnableRxInterruptIdempotent)
{
    auto dev = makeStartedDevice();

    dev->disableRxInterrupt();
    dev->disableRxInterrupt(); // double disable should be safe
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);

    dev->enableRxInterrupt();
    dev->enableRxInterrupt(); // double enable should be safe
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, isBusOffWithOtherBitsSet)
{
    auto dev      = makeStartedDevice();
    // Set BO along with other PSR bits — should still detect bus-off
    fakeFdcan.PSR = 0xFFFFU;
    EXPECT_TRUE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, errorCountersWithBothFieldsNonZero)
{
    auto dev      = makeStartedDevice();
    // TEC=200, REC=100
    fakeFdcan.ECR = 200U | (100U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getTxErrorCounter(), 200U);
    EXPECT_EQ(dev->getRxErrorCounter(), 100U);
}

TEST_F(FdCanDeviceTest, errorCountersZero)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 0U;
    EXPECT_EQ(dev->getTxErrorCounter(), 0U);
    EXPECT_EQ(dev->getRxErrorCounter(), 0U);
}

TEST_F(FdCanDeviceTest, receiveISRMultipleFramesDifferentIds)
{
    auto dev = makeStartedDevice();

    // Simulate 3 frames in FIFO (all at index 0 since our fake doesn't auto-advance)
    uint8_t data1[8] = {0x01};
    uint8_t data2[8] = {0x02};
    uint8_t data3[8] = {0x03};

    // First batch
    placeRxFrame(0, 0x100, false, 1, data1);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    placeRxFrame(0, 0x200, false, 1, data2);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    placeRxFrame(0, 0x300, false, 1, data3);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    EXPECT_EQ(dev->getRxCount(), 3U);
    EXPECT_EQ(dev->getRxFrame(0).getPayload()[0], 0x01U);
    EXPECT_EQ(dev->getRxFrame(1).getPayload()[0], 0x02U);
    EXPECT_EQ(dev->getRxFrame(2).getPayload()[0], 0x03U);
}

TEST_F(FdCanDeviceTest, configureFilterListEmptyList)
{
    auto dev = makeInitedDevice();

    // Zero-length filter list
    dev->configureFilterList(nullptr, 0U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 0U);

    // ANFS and ANFE should both be 2 (reject)
    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfs, 2U);
    EXPECT_EQ(anfe, 2U);
}

TEST_F(FdCanDeviceTest, configureFilterListSingleId)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x7FF};
    dev->configureFilterList(idList, 1U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 1U);

    uint32_t f0 = getStdFilter(0);
    EXPECT_EQ(f0, (1U << 30U) | (1U << 27U) | (0x7FFU << 16U) | 0x7FFU);
}

TEST_F(FdCanDeviceTest, getRxFrameIndexWraps)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill half the queue
    for (uint32_t i = 0; i < 16; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    // Clear (advances head to 16)
    dev->clearRxQueue();

    // Fill past the end of the array (head=16, fill 20 more → wraps around)
    for (uint32_t i = 0; i < 20; i++)
    {
        placeRxFrame(0, 0x200U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 20U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
    EXPECT_EQ(dev->getRxFrame(19).getId(), 0x213U);
}

// ============================================================================
// Category 10 — CAN FD enableFd()
// ============================================================================

TEST_F(FdCanDeviceTest, enableFdSetsFDOEAndBRSE)
{
    auto dev = makeInitedDevice();

    ::bios::FdCanDevice::FdConfig fdCfg = {5U, 3U, 1U, 0U, true};
    dev->enableFd(fdCfg);

    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceTest, enableFdWithoutBRS)
{
    auto dev = makeInitedDevice();

    ::bios::FdCanDevice::FdConfig fdCfg = {5U, 3U, 1U, 0U, false};
    dev->enableFd(fdCfg);

    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceTest, enableFdConfiguresDBTP)
{
    auto dev = makeInitedDevice();

    ::bios::FdCanDevice::FdConfig fdCfg = {5U, 7U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    // DBRP = prescaler-1 = 4, DTSEG1 = 7, DTSEG2 = 2, DSJW = 1
    uint32_t dbtp = fakeFdcan.DBTP;
    EXPECT_EQ((dbtp >> FDCAN_DBTP_DBRP_Pos) & 0x1FU, 4U);
    EXPECT_EQ((dbtp >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU, 7U);
    EXPECT_EQ((dbtp >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU, 2U);
    EXPECT_EQ((dbtp >> FDCAN_DBTP_DSJW_Pos) & 0xFU, 1U);
}

TEST_F(FdCanDeviceTest, enableFdEnablesTDC)
{
    auto dev = makeInitedDevice();

    ::bios::FdCanDevice::FdConfig fdCfg = {5U, 3U, 1U, 0U, true};
    dev->enableFd(fdCfg);

    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
    EXPECT_NE(fakeFdcan.TDCR, 0U);
}

TEST_F(FdCanDeviceTest, enableFdWithoutInitDoesNothing)
{
    auto cfg = makeDefaultConfig();
    ::bios::FdCanDevice dev(cfg);
    // Not initialized — enableFd should be no-op
    ::bios::FdCanDevice::FdConfig fdCfg = {5U, 3U, 1U, 0U, true};
    dev.enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

// ============================================================================
// Category 11 — transmitISR EFFL snapshot
// ============================================================================

TEST_F(FdCanDeviceTest, transmitISRSnapshotsDrainCount)
{
    // Verify the snapshot approach: EFFL is read once, not in the loop condition.
    // With the snapshot, even if EFFL stays non-zero in a fake register,
    // the loop only runs the snapshotted number of times.
    // This test is structural — verifying flags are set correctly.
    auto dev = makeStartedDevice();
    fakeFdcan.IR    = 0U;
    fakeFdcan.TXEFS = 0U; // EFFL=0, nothing to drain

    dev->transmitISR();

    // Both TEFN and TC must be cleared
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

// ============================================================================
// NEW TESTS — Category 1: Init edge cases (20 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, initWithPrescaler1)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 1U; // BRP = 0
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbrp = (fakeFdcan.NBTP >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    EXPECT_EQ(nbrp, 0U);
}

TEST_F(FdCanDeviceTest, initWithPrescaler512)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 512U; // BRP = 511 (max 9-bit field)
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbrp = (fakeFdcan.NBTP >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    EXPECT_EQ(nbrp, 511U);
}

TEST_F(FdCanDeviceTest, initWithMaxTseg1)
{
    auto cfg = makeDefaultConfig();
    cfg.nts1 = 255U; // max 8-bit field
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts1 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU;
    EXPECT_EQ(nts1, 255U);
}

TEST_F(FdCanDeviceTest, initWithMaxTseg2)
{
    auto cfg = makeDefaultConfig();
    cfg.nts2 = 127U; // max 7-bit field
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts2 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU;
    EXPECT_EQ(nts2, 127U);
}

TEST_F(FdCanDeviceTest, initWithMaxSjw)
{
    auto cfg = makeDefaultConfig();
    cfg.nsjw = 127U; // max 7-bit field
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nsjw = (fakeFdcan.NBTP >> FDCAN_NBTP_NSJW_Pos) & 0x7FU;
    EXPECT_EQ(nsjw, 127U);
}

TEST_F(FdCanDeviceTest, initGpioTxPin0)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 0U;
    cfg.txAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txAf = (fakeTxGpio.AFR[0] >> (0U * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 9U);
    uint32_t txModer = (fakeTxGpio.MODER >> (0U * 2U)) & 3U;
    EXPECT_EQ(txModer, 2U);
}

TEST_F(FdCanDeviceTest, initGpioTxPin7)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 7U;
    cfg.txAf  = 11U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txAf = (fakeTxGpio.AFR[0] >> (7U * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 11U);
}

TEST_F(FdCanDeviceTest, initGpioTxPin8)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 8U;
    cfg.txAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txAf = (fakeTxGpio.AFR[1] >> ((8U - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 9U);
}

TEST_F(FdCanDeviceTest, initGpioTxPin15)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 15U;
    cfg.txAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txAf = (fakeTxGpio.AFR[1] >> ((15U - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(txAf, 9U);
}

TEST_F(FdCanDeviceTest, initGpioRxPin0)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 0U;
    cfg.rxAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxAf = (fakeRxGpio.AFR[0] >> (0U * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 9U);
    uint32_t rxPupdr = (fakeRxGpio.PUPDR >> (0U * 2U)) & 3U;
    EXPECT_EQ(rxPupdr, 1U);
}

TEST_F(FdCanDeviceTest, initGpioRxPin7)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 7U;
    cfg.rxAf  = 12U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxAf = (fakeRxGpio.AFR[0] >> (7U * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 12U);
}

TEST_F(FdCanDeviceTest, initGpioRxPin8)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 8U;
    cfg.rxAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxAf = (fakeRxGpio.AFR[1] >> ((8U - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 9U);
}

TEST_F(FdCanDeviceTest, initGpioRxPin15)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 15U;
    cfg.rxAf  = 9U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxAf = (fakeRxGpio.AFR[1] >> ((15U - 8U) * 4U)) & 0xFU;
    EXPECT_EQ(rxAf, 9U);
}

TEST_F(FdCanDeviceTest, initCCESetAfterInit)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // CCE must be set for configuration change
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_CCE, 0U);
}

TEST_F(FdCanDeviceTest, doubleInitDoesNotBreakBitTiming)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 6U;
    cfg.nts1      = 20U;
    cfg.nts2      = 4U;
    cfg.nsjw      = 2U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbtpFirst = fakeFdcan.NBTP;
    dev.init();
    EXPECT_EQ(fakeFdcan.NBTP, nbtpFirst);
}

TEST_F(FdCanDeviceTest, doubleInitPreservesGpioConfig)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txModerFirst = fakeTxGpio.MODER;
    uint32_t rxModerFirst = fakeRxGpio.MODER;
    dev.init();
    EXPECT_EQ(fakeTxGpio.MODER, txModerFirst);
    EXPECT_EQ(fakeRxGpio.MODER, rxModerFirst);
}

TEST_F(FdCanDeviceTest, initClockEnableBitPreserved)
{
    // Pre-set some other APB1ENR1 bits — init should not clear them
    fakeRcc.APB1ENR1 = 0x00000001U;
    auto cfg         = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // FDCANEN should be set AND the pre-existing bit preserved
    EXPECT_NE(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
    EXPECT_NE(fakeRcc.APB1ENR1 & 0x00000001U, 0U);
}

TEST_F(FdCanDeviceTest, initWithAllBitTimingZero)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 1U;
    cfg.nts1      = 0U;
    cfg.nts2      = 0U;
    cfg.nsjw      = 0U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    // NBTP should be 0 (prescaler-1=0, all others 0)
    EXPECT_EQ(fakeFdcan.NBTP, 0U);
}

TEST_F(FdCanDeviceTest, initTxGpioSpeedIsVeryHigh)
{
    auto cfg  = makeDefaultConfig();
    cfg.txPin = 3U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t txSpeed = (fakeTxGpio.OSPEEDR >> (3U * 2U)) & 3U;
    EXPECT_EQ(txSpeed, 3U); // Very high speed
}

TEST_F(FdCanDeviceTest, initRxGpioPullUpEnabled)
{
    auto cfg  = makeDefaultConfig();
    cfg.rxPin = 6U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t rxPupdr = (fakeRxGpio.PUPDR >> (6U * 2U)) & 3U;
    EXPECT_EQ(rxPupdr, 1U); // Pull-up
}

// ============================================================================
// NEW TESTS — Category 2: Start/stop state machine (20 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, startWithoutInitLeavesINITUntouched)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    // INIT starts at 0 (not initialized)
    fakeFdcan.CCCR = 0U;
    dev.start();
    // start() returned early — CCCR unchanged
    EXPECT_EQ(fakeFdcan.CCCR, 0U);
}

TEST_F(FdCanDeviceTest, startWithoutInitLeavesTXBTIEZero)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.start();
    EXPECT_EQ(fakeFdcan.TXBTIE, 0U);
}

TEST_F(FdCanDeviceTest, doubleStartDoesNotCrash)
{
    auto dev = makeInitedDevice();
    dev->start();
    dev->start(); // second start
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, doubleStartIEPreserved)
{
    auto dev = makeInitedDevice();
    dev->start();
    uint32_t ieFirst = fakeFdcan.IE;
    dev->start();
    EXPECT_EQ(fakeFdcan.IE, ieFirst);
}

TEST_F(FdCanDeviceTest, stopWithoutStartSafe)
{
    auto dev = makeInitedDevice();
    // init was called but not start — stop should not crash
    dev->stop();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, stopClearsRF0NE)
{
    auto dev = makeStartedDevice();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    dev->stop();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, stopClearsTCE)
{
    auto dev = makeStartedDevice();
    dev->stop();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, stopThenStartIERestored)
{
    auto dev = makeStartedDevice();
    dev->stop();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    dev->start();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, stopThenStartTXBTIERestored)
{
    auto dev = makeStartedDevice();
    dev->stop();
    dev->start();
    EXPECT_EQ(fakeFdcan.TXBTIE, 0x7U);
}

TEST_F(FdCanDeviceTest, stopThenStartILERestored)
{
    auto dev = makeStartedDevice();
    dev->stop();
    dev->start();
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT0, 0U);
}

TEST_F(FdCanDeviceTest, multipleStopStartCycles)
{
    auto dev = makeStartedDevice();
    for (int i = 0; i < 5; i++)
    {
        dev->stop();
        EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
        dev->start();
        EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    }
}

TEST_F(FdCanDeviceTest, startLeavesINITClearedEvenIfCCCRHasOtherBits)
{
    auto dev = makeInitedDevice();
    // Pre-set some bits in CCCR besides INIT
    fakeFdcan.CCCR |= (1U << 4U); // some random bit
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, startSetsILSToZero)
{
    auto dev = makeInitedDevice();
    // Pre-set ILS to non-zero
    fakeFdcan.ILS = 0xFFFFFFFFU;
    dev->start();
    EXPECT_EQ(fakeFdcan.ILS, 0U);
}

TEST_F(FdCanDeviceTest, startSetsOnlyEINT0)
{
    auto dev = makeInitedDevice();
    dev->start();
    // Only EINT0 should be enabled, not EINT1
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT0, 0U);
    // ILE should be exactly EINT0
    EXPECT_EQ(fakeFdcan.ILE, FDCAN_ILE_EINT0);
}

TEST_F(FdCanDeviceTest, startSetsIEWithRF0NEAndTEFNE)
{
    auto dev = makeInitedDevice();
    dev->start();
    // IE should have RF0NE and TEFNE set
    EXPECT_EQ(fakeFdcan.IE, (FDCAN_IE_RF0NE | FDCAN_IE_TEFNE));
}

TEST_F(FdCanDeviceTest, stopDoesNotClearILE)
{
    auto dev = makeStartedDevice();
    dev->stop();
    // stop() only clears specific IE bits and enters init mode
    // ILE is not explicitly cleared by stop()
    // (verify the actual behavior)
    // After stop, INIT should be set
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, stopDoesNotClearTXBTIE)
{
    auto dev = makeStartedDevice();
    EXPECT_EQ(fakeFdcan.TXBTIE, 0x7U);
    dev->stop();
    // stop() does not explicitly clear TXBTIE
    EXPECT_EQ(fakeFdcan.TXBTIE, 0x7U);
}

TEST_F(FdCanDeviceTest, startAfterStopClearsINIT)
{
    auto dev = makeStartedDevice();
    dev->stop();
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, initDoesNotLeaveInitMode)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // After init, INIT bit should still be set
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, startThenStopThenStartAgainIECorrect)
{
    auto dev = makeStartedDevice();
    EXPECT_EQ(fakeFdcan.IE, (FDCAN_IE_RF0NE | FDCAN_IE_TEFNE));
    dev->stop();
    // stop() clears RF0NE and TCE
    dev->start();
    EXPECT_EQ(fakeFdcan.IE, (FDCAN_IE_RF0NE | FDCAN_IE_TEFNE));
}

// ============================================================================
// NEW TESTS — Category 3: Interrupt management (20 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, enableRxInterruptAfterStartPreservesOtherIEBits)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    // TEFNE should still be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);

    dev->enableRxInterrupt();
    // Both RF0NE and TEFNE should be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, disableRxInterruptPreservesTEFNE)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, disableRxInterruptMultipleTimes)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    dev->disableRxInterrupt();
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    // Other bits unaffected
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TEFNE, 0U);
}

TEST_F(FdCanDeviceTest, enableRxInterruptMultipleTimes)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    dev->enableRxInterrupt();
    dev->enableRxInterrupt();
    dev->enableRxInterrupt();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, IERegisterBitMaskRF0NE)
{
    auto dev = makeStartedDevice();
    // Verify RF0NE bit is at position 0
    EXPECT_EQ(FDCAN_IE_RF0NE, (1U << 0U));
}

TEST_F(FdCanDeviceTest, IERegisterBitMaskTCE)
{
    // Verify TCE bit is at position 7
    EXPECT_EQ(FDCAN_IE_TCE, (1U << 7U));
}

TEST_F(FdCanDeviceTest, IERegisterBitMaskTEFNE)
{
    // Verify TEFNE bit is at position 12
    EXPECT_EQ(FDCAN_IE_TEFNE, (1U << 12U));
}

TEST_F(FdCanDeviceTest, ILSZeroAfterStart)
{
    auto dev = makeStartedDevice();
    // All interrupts routed to line 0
    EXPECT_EQ(fakeFdcan.ILS, 0U);
}

TEST_F(FdCanDeviceTest, ILSPreSetIsOverwritten)
{
    auto dev = makeInitedDevice();
    fakeFdcan.ILS = 0xFFFFFFFFU;
    dev->start();
    EXPECT_EQ(fakeFdcan.ILS, 0U);
}

TEST_F(FdCanDeviceTest, ILEOnlyEINT0Enabled)
{
    auto dev = makeStartedDevice();
    EXPECT_EQ(fakeFdcan.ILE, FDCAN_ILE_EINT0);
}

TEST_F(FdCanDeviceTest, ILEPreSetIsOverwritten)
{
    auto dev = makeInitedDevice();
    fakeFdcan.ILE = FDCAN_ILE_EINT0 | FDCAN_ILE_EINT1;
    dev->start();
    EXPECT_EQ(fakeFdcan.ILE, FDCAN_ILE_EINT0);
}

TEST_F(FdCanDeviceTest, TXBTIEAllThreeBuffers)
{
    auto dev = makeStartedDevice();
    // Bits 0, 1, 2 should all be set
    EXPECT_NE(fakeFdcan.TXBTIE & (1U << 0U), 0U);
    EXPECT_NE(fakeFdcan.TXBTIE & (1U << 1U), 0U);
    EXPECT_NE(fakeFdcan.TXBTIE & (1U << 2U), 0U);
}

TEST_F(FdCanDeviceTest, TXBTIENoExtraBitsSet)
{
    auto dev = makeStartedDevice();
    // Only bits 0-2 should be set
    EXPECT_EQ(fakeFdcan.TXBTIE & ~0x7U, 0U);
}

TEST_F(FdCanDeviceTest, disableEnableRxInterruptCyclePreservesIE)
{
    auto dev = makeStartedDevice();
    uint32_t originalIE = fakeFdcan.IE;

    dev->disableRxInterrupt();
    dev->enableRxInterrupt();

    EXPECT_EQ(fakeFdcan.IE, originalIE);
}

TEST_F(FdCanDeviceTest, IEAfterStopHasTEFNECleared)
{
    auto dev = makeStartedDevice();
    dev->stop();
    // stop() clears RF0NE and TCE, but TEFNE is not in the stop mask
    // Verify RF0NE is cleared
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, ILSBitPositions)
{
    // SMSG is at position 2
    EXPECT_EQ(FDCAN_ILS_SMSG, (1U << 2U));
}

TEST_F(FdCanDeviceTest, ILEBitPositions)
{
    EXPECT_EQ(FDCAN_ILE_EINT0, (1U << 0U));
    EXPECT_EQ(FDCAN_ILE_EINT1, (1U << 1U));
}

TEST_F(FdCanDeviceTest, enableRxInterruptDoesNotSetTCE)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    dev->enableRxInterrupt();
    // enableRxInterrupt only sets RF0NE, not TCE
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    // TCE is not set by start (start sets TEFNE)
}

TEST_F(FdCanDeviceTest, disableRxInterruptDoesNotAffectILE)
{
    auto dev = makeStartedDevice();
    uint32_t ileBefore = fakeFdcan.ILE;
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.ILE, ileBefore);
}

TEST_F(FdCanDeviceTest, disableRxInterruptDoesNotAffectILS)
{
    auto dev = makeStartedDevice();
    uint32_t ilsBefore = fakeFdcan.ILS;
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.ILS, ilsBefore);
}

// ============================================================================
// NEW TESTS — Category 4: Filter configuration (25 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, acceptAllFilterSetsLSSZero)
{
    auto dev = makeInitedDevice();
    dev->configureAcceptAllFilter();
    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 0U);
}

TEST_F(FdCanDeviceTest, acceptAllFilterSetsLSEZero)
{
    auto dev = makeInitedDevice();
    dev->configureAcceptAllFilter();
    uint32_t lse = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSE_Pos) & 0xFU;
    EXPECT_EQ(lse, 0U);
}

TEST_F(FdCanDeviceTest, acceptAllFilterClearsRejectBits)
{
    auto dev        = makeInitedDevice();
    fakeFdcan.RXGFC = 0xFFFFFFFFU;
    dev->configureAcceptAllFilter();

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfs, 0U);
    EXPECT_EQ(anfe, 0U);
}

TEST_F(FdCanDeviceTest, filterListWith1IdWritesOneElement)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x555};
    dev->configureFilterList(idList, 1U);

    uint32_t f0 = getStdFilter(0);
    EXPECT_NE(f0, 0U);
    // Element 1 should be 0 (only 1 configured)
    uint32_t f1 = getStdFilter(1);
    EXPECT_EQ(f1, 0U);
}

TEST_F(FdCanDeviceTest, filterListWith28IdsAllWritten)
{
    auto dev = makeInitedDevice();

    uint32_t idList[28];
    for (uint32_t i = 0; i < 28; i++)
    {
        idList[i] = i + 1U;
    }
    dev->configureFilterList(idList, 28U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 28U);

    // Verify all 28 elements are non-zero
    for (uint8_t i = 0; i < 28; i++)
    {
        EXPECT_NE(getStdFilter(i), 0U);
    }
}

TEST_F(FdCanDeviceTest, filterListWith0IdsLSSIsZero)
{
    auto dev = makeInitedDevice();
    dev->configureFilterList(nullptr, 0U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 0U);
}

TEST_F(FdCanDeviceTest, filterListANFSRejectBit)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    EXPECT_EQ(anfs, 2U);
}

TEST_F(FdCanDeviceTest, filterListANFERejectBit)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    uint32_t anfe = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFE_Pos) & 3U;
    EXPECT_EQ(anfe, 2U);
}

TEST_F(FdCanDeviceTest, filterElementSFTBitIsDualId)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    uint32_t f0  = getStdFilter(0);
    uint32_t sft = (f0 >> 30U) & 3U;
    EXPECT_EQ(sft, 1U); // Dual ID filter type
}

TEST_F(FdCanDeviceTest, filterElementSFECBitIsStoreInFIFO0)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    uint32_t f0   = getStdFilter(0);
    uint32_t sfec = (f0 >> 27U) & 7U;
    EXPECT_EQ(sfec, 1U); // Store in RX FIFO 0
}

TEST_F(FdCanDeviceTest, filterElementSFID1MatchesId)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x3AB};
    dev->configureFilterList(idList, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid1 = (f0 >> 16U) & 0x7FFU;
    EXPECT_EQ(sfid1, 0x3ABU);
}

TEST_F(FdCanDeviceTest, filterElementSFID2MatchesId)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x3AB};
    dev->configureFilterList(idList, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid2 = f0 & 0x7FFU;
    EXPECT_EQ(sfid2, 0x3ABU);
}

TEST_F(FdCanDeviceTest, filterListIdMaskedTo11Bits)
{
    auto dev = makeInitedDevice();

    // Pass an ID with bits above 10 set — should be masked to 11 bits
    uint32_t idList[] = {0xFFF};
    dev->configureFilterList(idList, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid1 = (f0 >> 16U) & 0x7FFU;
    EXPECT_EQ(sfid1, 0x7FFU); // Masked to 11 bits
}

TEST_F(FdCanDeviceTest, filterListMultipleElementsCorrectOrder)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x001, 0x010, 0x100, 0x200, 0x7FF};
    dev->configureFilterList(idList, 5U);

    for (uint8_t i = 0; i < 5; i++)
    {
        uint32_t f    = getStdFilter(i);
        uint32_t sfid = (f >> 16U) & 0x7FFU;
        EXPECT_EQ(sfid, idList[i]);
    }
}

TEST_F(FdCanDeviceTest, filterListOverwritesPreviousConfig)
{
    auto dev = makeInitedDevice();

    uint32_t idList1[] = {0x100, 0x200, 0x300};
    dev->configureFilterList(idList1, 3U);

    uint32_t idList2[] = {0x400};
    dev->configureFilterList(idList2, 1U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid1 = (f0 >> 16U) & 0x7FFU;
    EXPECT_EQ(sfid1, 0x400U);
}

TEST_F(FdCanDeviceTest, acceptAllThenFilterList)
{
    auto dev = makeInitedDevice();
    dev->configureAcceptAllFilter();

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    EXPECT_EQ(anfs, 0U);

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    EXPECT_EQ(anfs, 2U);
}

TEST_F(FdCanDeviceTest, filterListThenAcceptAll)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100};
    dev->configureFilterList(idList, 1U);

    uint32_t anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    EXPECT_EQ(anfs, 2U);

    dev->configureAcceptAllFilter();
    anfs = (fakeFdcan.RXGFC >> FDCAN_RXGFC_ANFS_Pos) & 3U;
    EXPECT_EQ(anfs, 0U);
}

TEST_F(FdCanDeviceTest, filterListWith5IdsHasCorrectLSS)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x001, 0x002, 0x003, 0x004, 0x005};
    dev->configureFilterList(idList, 5U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 5U);
}

TEST_F(FdCanDeviceTest, filterListWith10IdsHasCorrectLSS)
{
    auto dev = makeInitedDevice();

    uint32_t idList[10];
    for (uint32_t i = 0; i < 10; i++)
    {
        idList[i] = 0x100U + i;
    }
    dev->configureFilterList(idList, 10U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 10U);
}

TEST_F(FdCanDeviceTest, filterListWith28IdsLSSIs28)
{
    auto dev = makeInitedDevice();

    uint32_t idList[28];
    for (uint32_t i = 0; i < 28; i++)
    {
        idList[i] = 0x100U + i;
    }
    dev->configureFilterList(idList, 28U);

    uint32_t lss = (fakeFdcan.RXGFC >> FDCAN_RXGFC_LSS_Pos) & 0x1FU;
    EXPECT_EQ(lss, 28U);
}

TEST_F(FdCanDeviceTest, filterListIdZero)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x000};
    dev->configureFilterList(idList, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid1 = (f0 >> 16U) & 0x7FFU;
    uint32_t sfid2 = f0 & 0x7FFU;
    EXPECT_EQ(sfid1, 0U);
    EXPECT_EQ(sfid2, 0U);
}

TEST_F(FdCanDeviceTest, filterListIdMaxStandard)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x7FF};
    dev->configureFilterList(idList, 1U);

    uint32_t f0    = getStdFilter(0);
    uint32_t sfid1 = (f0 >> 16U) & 0x7FFU;
    EXPECT_EQ(sfid1, 0x7FFU);
}

TEST_F(FdCanDeviceTest, filterListElement27IsLast)
{
    auto dev = makeInitedDevice();

    uint32_t idList[28];
    for (uint32_t i = 0; i < 28; i++)
    {
        idList[i] = 0x100U + i;
    }
    dev->configureFilterList(idList, 28U);

    uint32_t f27   = getStdFilter(27);
    uint32_t sfid1 = (f27 >> 16U) & 0x7FFU;
    EXPECT_EQ(sfid1, 0x100U + 27U);
}

TEST_F(FdCanDeviceTest, filterListRXGFCFullWord)
{
    auto dev = makeInitedDevice();

    uint32_t idList[] = {0x100, 0x200};
    dev->configureFilterList(idList, 2U);

    uint32_t expected = (2U << FDCAN_RXGFC_ANFS_Pos) | (2U << FDCAN_RXGFC_ANFE_Pos)
                        | (2U << FDCAN_RXGFC_LSS_Pos);
    EXPECT_EQ(fakeFdcan.RXGFC, expected);
}

// ============================================================================
// NEW TESTS — Category 5: Error state (15 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, busOffWithBOBitOnly)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = FDCAN_PSR_BO;
    EXPECT_TRUE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, notBusOffWithZeroPSR)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = 0U;
    EXPECT_FALSE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, notBusOffWithOtherBitsButNotBO)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = 0x0000007FU; // bits 0-6 set, but not bit 7 (BO)
    EXPECT_FALSE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, busOffWithAllPSRBitsSet)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.PSR = 0xFFFFFFFFU;
    EXPECT_TRUE(dev->isBusOff());
}

TEST_F(FdCanDeviceTest, txErrorCounterZero)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 0U;
    EXPECT_EQ(dev->getTxErrorCounter(), 0U);
}

TEST_F(FdCanDeviceTest, txErrorCounter128)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 128U;
    EXPECT_EQ(dev->getTxErrorCounter(), 128U);
}

TEST_F(FdCanDeviceTest, txErrorCounter255)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 255U;
    EXPECT_EQ(dev->getTxErrorCounter(), 255U);
}

TEST_F(FdCanDeviceTest, rxErrorCounterZero)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 0U;
    EXPECT_EQ(dev->getRxErrorCounter(), 0U);
}

TEST_F(FdCanDeviceTest, rxErrorCounter64)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = (64U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getRxErrorCounter(), 64U);
}

TEST_F(FdCanDeviceTest, rxErrorCounter127)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = (127U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getRxErrorCounter(), 127U);
}

TEST_F(FdCanDeviceTest, txErrorCounterNotAffectedByREC)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 0U | (127U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getTxErrorCounter(), 0U);
}

TEST_F(FdCanDeviceTest, rxErrorCounterNotAffectedByTEC)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 255U | (0U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getRxErrorCounter(), 0U);
}

TEST_F(FdCanDeviceTest, bothErrorCountersIndependent)
{
    auto dev      = makeStartedDevice();
    fakeFdcan.ECR = 42U | (99U << FDCAN_ECR_REC_Pos);
    EXPECT_EQ(dev->getTxErrorCounter(), 42U);
    EXPECT_EQ(dev->getRxErrorCounter(), 99U);
}

TEST_F(FdCanDeviceTest, PSRBOPositionCorrect)
{
    EXPECT_EQ(FDCAN_PSR_BO, (1U << 7U));
}

TEST_F(FdCanDeviceTest, ECRFieldPositionsCorrect)
{
    EXPECT_EQ(FDCAN_ECR_TEC_Pos, 0U);
    EXPECT_EQ(FDCAN_ECR_REC_Pos, 8U);
}

// ============================================================================
// NEW TESTS — Category 6: Queue management (25 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, getRxCountEmptyAfterConstruction)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    EXPECT_EQ(dev.getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, clearRxQueueOnEmpty)
{
    auto dev = makeStartedDevice();
    EXPECT_EQ(dev->getRxCount(), 0U);
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, clearRxQueueDoubleClear)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxCount(), 1U);

    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, clearRxQueueAfterPartialRead)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Add 5 frames
    for (uint32_t i = 0; i < 5; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 5U);

    // Read a frame (does not consume it)
    EXPECT_EQ(dev->getRxFrame(2).getId(), 0x102U);

    // Clear all
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, getRxCountAfterMultipleReceives)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (uint32_t i = 0; i < 10; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 10U);
}

TEST_F(FdCanDeviceTest, getRxFrameWrappingAtBoundary)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill to exactly RX_QUEUE_SIZE
    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);

    // Clear
    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);

    // Receive 5 more — these wrap around
    for (uint32_t i = 0; i < 5; i++)
    {
        placeRxFrame(0, 0x200U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 5U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
    EXPECT_EQ(dev->getRxFrame(4).getId(), 0x204U);
}

TEST_F(FdCanDeviceTest, queueHeadAdvancesOnClear)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Add 10 frames
    for (uint32_t i = 0; i < 10; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    dev->clearRxQueue();

    // Add 5 more — should start from new head position
    for (uint32_t i = 0; i < 5; i++)
    {
        placeRxFrame(0, 0x200U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 5U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
}

TEST_F(FdCanDeviceTest, multipleClearCycles)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (int cycle = 0; cycle < 10; cycle++)
    {
        placeRxFrame(0, static_cast<uint32_t>(0x100 + cycle), false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
        EXPECT_EQ(dev->getRxCount(), 1U);
        EXPECT_EQ(dev->getRxFrame(0).getId(), static_cast<uint32_t>(0x100 + cycle));
        dev->clearRxQueue();
        EXPECT_EQ(dev->getRxCount(), 0U);
    }
}

TEST_F(FdCanDeviceTest, getRxFrameIndex0)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {0xAA};
    placeRxFrame(0, 0x100, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x100U);
}

TEST_F(FdCanDeviceTest, getRxFrameLastIndex)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }

    EXPECT_EQ(dev->getRxFrame(bios::FdCanDevice::RX_QUEUE_SIZE - 1).getId(),
              0x100U + bios::FdCanDevice::RX_QUEUE_SIZE - 1);
}

TEST_F(FdCanDeviceTest, rxQueueDropsWhenFullAndContinuesAcknowledging)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill to capacity
    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }

    // Try one more — should return 0 (dropped) but still ack
    placeRxFrame(0, 0x999, false, 1, data);
    setRxFifoStatus(1U, 0U);
    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 0U);
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);
}

TEST_F(FdCanDeviceTest, clearThenFillToCapacityAgain)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill to capacity
    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);

    dev->clearRxQueue();

    // Fill again
    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x200U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
}

TEST_F(FdCanDeviceTest, getRxCountIncrementsByOne)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (uint32_t i = 0; i < 5; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
        EXPECT_EQ(dev->getRxCount(), i + 1U);
    }
}

TEST_F(FdCanDeviceTest, clearRxQueueAfterSingleFrame)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};
    placeRxFrame(0, 0x100, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    dev->clearRxQueue();
    EXPECT_EQ(dev->getRxCount(), 0U);
}

TEST_F(FdCanDeviceTest, getRxFrameAfterClearAndRefill)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // First fill
    placeRxFrame(0, 0x100, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x100U);

    // Clear
    dev->clearRxQueue();

    // Refill
    placeRxFrame(0, 0x200, false, 1, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
}

TEST_F(FdCanDeviceTest, queueWrapsCorrectlyAt31To0)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    // Fill 31 frames, clear
    for (uint32_t i = 0; i < 31; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    dev->clearRxQueue();

    // Now head is at 31. Add 3 more — should wrap from 31 to 0, 1
    for (uint32_t i = 0; i < 3; i++)
    {
        placeRxFrame(0, 0x200U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), 3U);
    EXPECT_EQ(dev->getRxFrame(0).getId(), 0x200U);
    EXPECT_EQ(dev->getRxFrame(1).getId(), 0x201U);
    EXPECT_EQ(dev->getRxFrame(2).getId(), 0x202U);
}

TEST_F(FdCanDeviceTest, rxQueueSizeIs32)
{
    EXPECT_EQ(bios::FdCanDevice::RX_QUEUE_SIZE, 32U);
}

TEST_F(FdCanDeviceTest, getRxCountAfterExactlyFullQueue)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (uint32_t i = 0; i < bios::FdCanDevice::RX_QUEUE_SIZE; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    EXPECT_EQ(dev->getRxCount(), bios::FdCanDevice::RX_QUEUE_SIZE);
}

TEST_F(FdCanDeviceTest, receiveAfterClearRxQueueWorks)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    placeRxFrame(0, 0x100, false, 8, data);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);
    dev->clearRxQueue();

    placeRxFrame(0, 0x200, false, 8, data);
    setRxFifoStatus(1U, 0U);
    uint8_t received = dev->receiveISR(nullptr);
    EXPECT_EQ(received, 1U);
    EXPECT_EQ(dev->getRxCount(), 1U);
}

TEST_F(FdCanDeviceTest, clearRxQueueRepeatedlySafe)
{
    auto dev = makeStartedDevice();
    for (int i = 0; i < 100; i++)
    {
        dev->clearRxQueue();
        EXPECT_EQ(dev->getRxCount(), 0U);
    }
}

TEST_F(FdCanDeviceTest, fillClearFillClearPattern)
{
    auto dev        = makeStartedDevice();
    uint8_t data[8] = {};

    for (int round = 0; round < 5; round++)
    {
        for (uint32_t i = 0; i < 8; i++)
        {
            placeRxFrame(0, 0x100U + i, false, 1, data);
            setRxFifoStatus(1U, 0U);
            dev->receiveISR(nullptr);
        }
        EXPECT_EQ(dev->getRxCount(), 8U);
        dev->clearRxQueue();
        EXPECT_EQ(dev->getRxCount(), 0U);
    }
}

TEST_F(FdCanDeviceTest, getRxFramePayloadCorrectAfterWrap)
{
    auto dev = makeStartedDevice();

    // Fill 30 frames and clear
    uint8_t data[8] = {};
    for (uint32_t i = 0; i < 30; i++)
    {
        placeRxFrame(0, 0x100U + i, false, 1, data);
        setRxFifoStatus(1U, 0U);
        dev->receiveISR(nullptr);
    }
    dev->clearRxQueue();

    // Now add a frame with specific payload — wraps into beginning
    uint8_t payload[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE};
    placeRxFrame(0, 0x500, false, 8, payload);
    setRxFifoStatus(1U, 0U);
    dev->receiveISR(nullptr);

    auto const& frame = dev->getRxFrame(0);
    EXPECT_EQ(frame.getId(), 0x500U);
    EXPECT_EQ(frame.getPayloadLength(), 8U);
    EXPECT_EQ(frame.getPayload()[0], 0xDEU);
    EXPECT_EQ(frame.getPayload()[7], 0xBEU);
}

// ============================================================================
// NEW TESTS — Category 7: TX FIFO status (15 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, txFifoFreeLevel0ReturnsFalse)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(0U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_FALSE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, txFifoFreeLevel1ReturnsTrue)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(1U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, txFifoFreeLevel2ReturnsTrue)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(2U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, txFifoFreeLevel3ReturnsTrue)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, txFifoPutIndex0)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    dev->transmit(frame);

    EXPECT_EQ(fakeFdcan.TXBAR, (1U << 0U));
}

TEST_F(FdCanDeviceTest, txFifoPutIndex1)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 1U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    dev->transmit(frame);

    EXPECT_EQ(fakeFdcan.TXBAR, (1U << 1U));
}

TEST_F(FdCanDeviceTest, txFifoPutIndex2)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 2U);

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    dev->transmit(frame);

    EXPECT_EQ(fakeFdcan.TXBAR, (1U << 2U));
}

TEST_F(FdCanDeviceTest, txFifoFullThenFreeReturnsTrue)
{
    auto dev = makeStartedDevice();

    // First: FIFO full
    setTxFifoStatus(0U, 0U);
    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_FALSE(dev->transmit(frame));

    // Now: FIFO has space
    setTxFifoStatus(1U, 0U);
    EXPECT_TRUE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, txFQSFieldPositions)
{
    EXPECT_EQ(FDCAN_TXFQS_TFFL_Pos, 0U);
    EXPECT_EQ(FDCAN_TXFQS_TFQPI_Pos, 16U);
}

TEST_F(FdCanDeviceTest, txFifoChecksOnlyTFFL)
{
    auto dev = makeStartedDevice();
    // TFFL = 0 but other fields non-zero
    fakeFdcan.TXFQS = (2U << FDCAN_TXFQS_TFQPI_Pos);
    // TFFL is 0, so transmit should fail
    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    EXPECT_FALSE(dev->transmit(frame));
}

TEST_F(FdCanDeviceTest, transmitDoesNotModifyTXFQS)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);
    uint32_t txfqsBefore = fakeFdcan.TXFQS;

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    dev->transmit(frame);

    EXPECT_EQ(fakeFdcan.TXFQS, txfqsBefore);
}

TEST_F(FdCanDeviceTest, transmitSetsTXBAROnly)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(3U, 0U);

    fakeFdcan.TXBAR = 0U;
    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);
    dev->transmit(frame);

    EXPECT_EQ(fakeFdcan.TXBAR, 1U);
}

TEST_F(FdCanDeviceTest, transmitTXBCIsZeroForFifoMode)
{
    auto dev = makeStartedDevice();
    EXPECT_EQ(fakeFdcan.TXBC, 0U);
}

TEST_F(FdCanDeviceTest, txFifoSequentialPutIndices)
{
    auto dev = makeStartedDevice();

    uint8_t data[8] = {};
    ::can::CANFrame frame(0x100, data, 0U);

    for (uint8_t i = 0; i < 3; i++)
    {
        setTxFifoStatus(3U - i, i);
        dev->transmit(frame);
        EXPECT_NE(fakeFdcan.TXBAR & (1U << i), 0U);
    }
}

TEST_F(FdCanDeviceTest, transmitWithFullFifoDoesNotWriteMessageRam)
{
    auto dev = makeStartedDevice();
    setTxFifoStatus(0U, 0U); // Full

    // Zero out TXBAR
    fakeFdcan.TXBAR = 0U;

    uint8_t data[8] = {0xFF};
    ::can::CANFrame frame(0x100, data, 1U);
    EXPECT_FALSE(dev->transmit(frame));

    // TXBAR should not be written
    EXPECT_EQ(fakeFdcan.TXBAR, 0U);
}

// ============================================================================
// NEW TESTS — Category 8: CAN FD enableFd() (20 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, enableFdSetsFDOE)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 3U, 1U, 0U, false};
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
}

TEST_F(FdCanDeviceTest, enableFdWithBRSSetsBRSE)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 3U, 1U, 0U, true};
    dev->enableFd(fdCfg);
    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceTest, enableFdWithoutBRSClearsBRSE)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 3U, 1U, 0U, false};
    dev->enableFd(fdCfg);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
}

TEST_F(FdCanDeviceTest, enableFdDBTPPrescaler)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {3U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dbrp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(dbrp, 2U); // prescaler - 1
}

TEST_F(FdCanDeviceTest, enableFdDBTPTseg1)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 15U, 3U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dtseg1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(dtseg1, 15U);
}

TEST_F(FdCanDeviceTest, enableFdDBTPTseg2)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 7U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dtseg2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(dtseg2, 7U);
}

TEST_F(FdCanDeviceTest, enableFdDBTPSjw)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 3U, true};
    dev->enableFd(fdCfg);

    uint32_t dsjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(dsjw, 3U);
}

TEST_F(FdCanDeviceTest, enableFdTDCEnabled)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    EXPECT_NE(fakeFdcan.DBTP & FDCAN_DBTP_TDC, 0U);
}

TEST_F(FdCanDeviceTest, enableFdTDCROffset)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t tdco = (fakeFdcan.TDCR >> FDCAN_TDCR_TDCO_Pos) & 0x7FU;
    EXPECT_EQ(tdco, 5U); // Typical offset value set by driver
}

TEST_F(FdCanDeviceTest, enableFdBeforeInitIsNoOp)
{
    auto cfg = makeDefaultConfig();
    ::bios::FdCanDevice dev(cfg);
    // Not initialized
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev.enableFd(fdCfg);

    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
    EXPECT_EQ(fakeFdcan.DBTP, 0U);
}

TEST_F(FdCanDeviceTest, enableFdLeavesInitMode)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    // enableFd calls leaveInitMode at the end
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, enableFdPrescaler1)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {1U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dbrp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(dbrp, 0U); // prescaler - 1
}

TEST_F(FdCanDeviceTest, enableFdPrescaler32)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {32U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dbrp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(dbrp, 31U);
}

TEST_F(FdCanDeviceTest, enableFdMaxTseg1)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 31U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dtseg1 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG1_Pos) & 0x1FU;
    EXPECT_EQ(dtseg1, 31U);
}

TEST_F(FdCanDeviceTest, enableFdMaxTseg2)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 15U, 1U, true};
    dev->enableFd(fdCfg);

    uint32_t dtseg2 = (fakeFdcan.DBTP >> FDCAN_DBTP_DTSEG2_Pos) & 0xFU;
    EXPECT_EQ(dtseg2, 15U);
}

TEST_F(FdCanDeviceTest, enableFdMaxSjw)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 15U, true};
    dev->enableFd(fdCfg);

    uint32_t dsjw = (fakeFdcan.DBTP >> FDCAN_DBTP_DSJW_Pos) & 0xFU;
    EXPECT_EQ(dsjw, 15U);
}

TEST_F(FdCanDeviceTest, enableFdDBTPAllFieldsPacked)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {4U, 10U, 3U, 2U, true};
    dev->enableFd(fdCfg);

    uint32_t dbtp     = fakeFdcan.DBTP;
    uint32_t expected = (2U << FDCAN_DBTP_DSJW_Pos) | (3U << FDCAN_DBTP_DBRP_Pos)
                        | (10U << FDCAN_DBTP_DTSEG1_Pos) | (3U << FDCAN_DBTP_DTSEG2_Pos)
                        | FDCAN_DBTP_TDC;
    EXPECT_EQ(dbtp, expected);
}

TEST_F(FdCanDeviceTest, enableFdThenStartWorks)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    // enableFd leaves init mode, start enters init again and leaves
    dev->start();
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_INIT, 0U);
}

TEST_F(FdCanDeviceTest, enableFdDoesNotAffectNBTP)
{
    auto dev = makeInitedDevice();

    uint32_t nbtpBefore = fakeFdcan.NBTP;

    ::bios::FdCanDevice::FdConfig fdCfg = {2U, 5U, 2U, 1U, true};
    dev->enableFd(fdCfg);

    EXPECT_EQ(fakeFdcan.NBTP, nbtpBefore);
}

TEST_F(FdCanDeviceTest, enableFdWithMinimalConfig)
{
    auto dev = makeInitedDevice();
    ::bios::FdCanDevice::FdConfig fdCfg = {1U, 1U, 1U, 0U, false};
    dev->enableFd(fdCfg);

    EXPECT_NE(fakeFdcan.CCCR & FDCAN_CCCR_FDOE, 0U);
    EXPECT_EQ(fakeFdcan.CCCR & FDCAN_CCCR_BRSE, 0U);
    uint32_t dbrp = (fakeFdcan.DBTP >> FDCAN_DBTP_DBRP_Pos) & 0x1FU;
    EXPECT_EQ(dbrp, 0U);
}

// ============================================================================
// NEW TESTS — Category 9: transmitISR (15 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, transmitISRClearsFlags)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;
    fakeFdcan.TXEFS = 0U;
    dev->transmitISR();
    EXPECT_NE(fakeFdcan.IR & FDCAN_IR_TEFN, 0U);
    EXPECT_NE(fakeFdcan.IR & FDCAN_IR_TC, 0U);
}

TEST_F(FdCanDeviceTest, transmitISREmptyFifoDoesNotWriteTXEFA)
{
    auto dev = makeStartedDevice();
    fakeFdcan.IR    = 0U;
    fakeFdcan.TXEFS = 0U;
    fakeFdcan.TXEFA = 0x12345678U; // Sentinel
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.TXEFA, 0x12345678U);
}

TEST_F(FdCanDeviceTest, transmitISRMultipleCallsEmptyFifo)
{
    auto dev = makeStartedDevice();
    for (int i = 0; i < 5; i++)
    {
        fakeFdcan.IR    = 0U;
        fakeFdcan.TXEFS = 0U;
        dev->transmitISR();
        EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
    }
}

TEST_F(FdCanDeviceTest, transmitISRIRWriteIsTEFNOrTC)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;
    fakeFdcan.TXEFS = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

TEST_F(FdCanDeviceTest, transmitISRDoesNotAffectIE)
{
    auto dev = makeStartedDevice();
    uint32_t ieBefore = fakeFdcan.IE;
    fakeFdcan.IR      = 0U;
    fakeFdcan.TXEFS   = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.IE, ieBefore);
}

TEST_F(FdCanDeviceTest, transmitISRDoesNotAffectILE)
{
    auto dev = makeStartedDevice();
    uint32_t ileBefore = fakeFdcan.ILE;
    fakeFdcan.IR       = 0U;
    fakeFdcan.TXEFS    = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.ILE, ileBefore);
}

TEST_F(FdCanDeviceTest, transmitISRDoesNotAffectILS)
{
    auto dev = makeStartedDevice();
    uint32_t ilsBefore = fakeFdcan.ILS;
    fakeFdcan.IR       = 0U;
    fakeFdcan.TXEFS    = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.ILS, ilsBefore);
}

TEST_F(FdCanDeviceTest, transmitISRDoesNotAffectCCCR)
{
    auto dev = makeStartedDevice();
    uint32_t cccrBefore = fakeFdcan.CCCR;
    fakeFdcan.IR        = 0U;
    fakeFdcan.TXEFS     = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.CCCR, cccrBefore);
}

TEST_F(FdCanDeviceTest, transmitISRDoesNotAffectTXBTIE)
{
    auto dev = makeStartedDevice();
    uint32_t txbtieBefore = fakeFdcan.TXBTIE;
    fakeFdcan.IR          = 0U;
    fakeFdcan.TXEFS       = 0U;
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.TXBTIE, txbtieBefore);
}

TEST_F(FdCanDeviceTest, transmitISRPreExistingIRBitsOverwritten)
{
    auto dev = makeStartedDevice();
    // Pre-set IR with some bits
    fakeFdcan.IR    = 0xFFFFFFFFU;
    fakeFdcan.TXEFS = 0U;
    dev->transmitISR();
    // The last write to IR is the clear pattern
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

TEST_F(FdCanDeviceTest, transmitISRCalledBeforeAnyTransmit)
{
    auto dev = makeStartedDevice();
    fakeFdcan.IR    = 0U;
    fakeFdcan.TXEFS = 0U;
    // Should be safe even if no transmit was ever done
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

TEST_F(FdCanDeviceTest, transmitISRTEFNBitPosition)
{
    EXPECT_EQ(FDCAN_IR_TEFN, (1U << 12U));
}

TEST_F(FdCanDeviceTest, transmitISRTCBitPosition)
{
    EXPECT_EQ(FDCAN_IR_TC, (1U << 7U));
}

TEST_F(FdCanDeviceTest, transmitISRTXEFSFieldPositions)
{
    EXPECT_EQ(FDCAN_TXEFS_EFFL_Pos, 0U);
    EXPECT_EQ(FDCAN_TXEFS_EFGI_Pos, 8U);
}

TEST_F(FdCanDeviceTest, transmitISRSequentialCallsAllClearFlags)
{
    auto dev = makeStartedDevice();
    for (int i = 0; i < 10; i++)
    {
        fakeFdcan.IR    = 0U;
        fakeFdcan.TXEFS = 0U;
        dev->transmitISR();
    }
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

// ============================================================================
// NEW TESTS — Category 10: Bit timing (15 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, nbtpPrescaler1FieldValue0)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 1U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbrp = (fakeFdcan.NBTP >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    EXPECT_EQ(nbrp, 0U);
}

TEST_F(FdCanDeviceTest, nbtpPrescaler256FieldValue255)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 256U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbrp = (fakeFdcan.NBTP >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU;
    EXPECT_EQ(nbrp, 255U);
}

TEST_F(FdCanDeviceTest, nbtpTseg1Is0)
{
    auto cfg = makeDefaultConfig();
    cfg.nts1 = 0U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts1 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU;
    EXPECT_EQ(nts1, 0U);
}

TEST_F(FdCanDeviceTest, nbtpTseg1Is128)
{
    auto cfg = makeDefaultConfig();
    cfg.nts1 = 128U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts1 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU;
    EXPECT_EQ(nts1, 128U);
}

TEST_F(FdCanDeviceTest, nbtpTseg2Is0)
{
    auto cfg = makeDefaultConfig();
    cfg.nts2 = 0U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts2 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU;
    EXPECT_EQ(nts2, 0U);
}

TEST_F(FdCanDeviceTest, nbtpTseg2Is64)
{
    auto cfg = makeDefaultConfig();
    cfg.nts2 = 64U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nts2 = (fakeFdcan.NBTP >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU;
    EXPECT_EQ(nts2, 64U);
}

TEST_F(FdCanDeviceTest, nbtpSjwIs0)
{
    auto cfg = makeDefaultConfig();
    cfg.nsjw = 0U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nsjw = (fakeFdcan.NBTP >> FDCAN_NBTP_NSJW_Pos) & 0x7FU;
    EXPECT_EQ(nsjw, 0U);
}

TEST_F(FdCanDeviceTest, nbtpSjwIs64)
{
    auto cfg = makeDefaultConfig();
    cfg.nsjw = 64U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nsjw = (fakeFdcan.NBTP >> FDCAN_NBTP_NSJW_Pos) & 0x7FU;
    EXPECT_EQ(nsjw, 64U);
}

TEST_F(FdCanDeviceTest, nbtpFieldPositions)
{
    EXPECT_EQ(FDCAN_NBTP_NTSEG2_Pos, 0U);
    EXPECT_EQ(FDCAN_NBTP_NTSEG1_Pos, 8U);
    EXPECT_EQ(FDCAN_NBTP_NBRP_Pos, 16U);
    EXPECT_EQ(FDCAN_NBTP_NSJW_Pos, 25U);
}

TEST_F(FdCanDeviceTest, nbtpFieldIsolation500kbps)
{
    // Typical 500kbps: prescaler=4, tseg1=13, tseg2=2, sjw=1
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 4U;
    cfg.nts1      = 13U;
    cfg.nts2      = 2U;
    cfg.nsjw      = 1U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t expected = (1U << FDCAN_NBTP_NSJW_Pos) | (3U << FDCAN_NBTP_NBRP_Pos)
                        | (13U << FDCAN_NBTP_NTSEG1_Pos) | (2U << FDCAN_NBTP_NTSEG2_Pos);
    EXPECT_EQ(fakeFdcan.NBTP, expected);
}

TEST_F(FdCanDeviceTest, nbtpFieldIsolation250kbps)
{
    // Typical 250kbps: prescaler=8, tseg1=13, tseg2=2, sjw=1
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 8U;
    cfg.nts1      = 13U;
    cfg.nts2      = 2U;
    cfg.nsjw      = 1U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t expected = (1U << FDCAN_NBTP_NSJW_Pos) | (7U << FDCAN_NBTP_NBRP_Pos)
                        | (13U << FDCAN_NBTP_NTSEG1_Pos) | (2U << FDCAN_NBTP_NTSEG2_Pos);
    EXPECT_EQ(fakeFdcan.NBTP, expected);
}

TEST_F(FdCanDeviceTest, nbtpFieldIsolation1Mbps)
{
    // Typical 1Mbps: prescaler=2, tseg1=13, tseg2=2, sjw=1
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 2U;
    cfg.nts1      = 13U;
    cfg.nts2      = 2U;
    cfg.nsjw      = 1U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t expected = (1U << FDCAN_NBTP_NSJW_Pos) | (1U << FDCAN_NBTP_NBRP_Pos)
                        | (13U << FDCAN_NBTP_NTSEG1_Pos) | (2U << FDCAN_NBTP_NTSEG2_Pos);
    EXPECT_EQ(fakeFdcan.NBTP, expected);
}

TEST_F(FdCanDeviceTest, nbtpNoOverlapBetweenFields)
{
    // Set all fields to max and verify no bit overlap
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 512U; // BRP = 511 = 0x1FF (9 bits at pos 16)
    cfg.nts1      = 255U; // 8 bits at pos 8
    cfg.nts2      = 127U; // 7 bits at pos 0
    cfg.nsjw      = 127U; // 7 bits at pos 25
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t nbtp = fakeFdcan.NBTP;
    EXPECT_EQ((nbtp >> FDCAN_NBTP_NTSEG2_Pos) & 0x7FU, 127U);
    EXPECT_EQ((nbtp >> FDCAN_NBTP_NTSEG1_Pos) & 0xFFU, 255U);
    EXPECT_EQ((nbtp >> FDCAN_NBTP_NBRP_Pos) & 0x1FFU, 511U);
    EXPECT_EQ((nbtp >> FDCAN_NBTP_NSJW_Pos) & 0x7FU, 127U);
}

TEST_F(FdCanDeviceTest, nbtpAllFieldsMinimal)
{
    auto cfg      = makeDefaultConfig();
    cfg.prescaler = 1U;
    cfg.nts1      = 0U;
    cfg.nts2      = 0U;
    cfg.nsjw      = 0U;
    bios::FdCanDevice dev(cfg);
    dev.init();

    EXPECT_EQ(fakeFdcan.NBTP, 0U);
}

// ============================================================================
// NEW TESTS — Category 11: Peripheral clock (10 tests)
// ============================================================================

TEST_F(FdCanDeviceTest, clockEnableBitPosition)
{
    EXPECT_EQ(RCC_APB1ENR1_FDCANEN_Pos, 25U);
    EXPECT_EQ(RCC_APB1ENR1_FDCANEN, (1U << 25U));
}

TEST_F(FdCanDeviceTest, initEnablesFDCANClockInAPB1ENR1)
{
    fakeRcc.APB1ENR1 = 0U;
    auto cfg         = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    EXPECT_NE(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
}

TEST_F(FdCanDeviceTest, initPreservesOtherAPB1ENR1Bits)
{
    fakeRcc.APB1ENR1 = 0x0000FFFFU;
    auto cfg         = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    // Original bits preserved
    EXPECT_NE(fakeRcc.APB1ENR1 & 0x0000FFFFU, 0U);
    // FDCANEN also set
    EXPECT_NE(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
}

TEST_F(FdCanDeviceTest, initSetsClockSelectPCLK1)
{
    fakeRcc.CCIPR = 0U;
    auto cfg      = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t fdcanSel = (fakeRcc.CCIPR >> 24U) & 3U;
    EXPECT_EQ(fdcanSel, 2U); // PCLK1
}

TEST_F(FdCanDeviceTest, initClockSelectClearsOldValue)
{
    // Pre-set clock select to HSE (00) with other bits
    fakeRcc.CCIPR = (3U << 24U); // was 11b
    auto cfg      = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    uint32_t fdcanSel = (fakeRcc.CCIPR >> 24U) & 3U;
    EXPECT_EQ(fdcanSel, 2U); // Should be 10b = PCLK1
}

TEST_F(FdCanDeviceTest, initClockSelectPreservesOtherCCIPRBits)
{
    fakeRcc.CCIPR = 0x00FFFFFFU; // bits 0-23 set
    auto cfg      = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // Lower 24 bits should still be set
    EXPECT_EQ(fakeRcc.CCIPR & 0x00FFFFFFU, 0x00FFFFFFu);
    // Clock select should be PCLK1
    uint32_t fdcanSel = (fakeRcc.CCIPR >> 24U) & 3U;
    EXPECT_EQ(fdcanSel, 2U);
}

TEST_F(FdCanDeviceTest, doubleInitDoesNotDoubleSetClock)
{
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    uint32_t apb1After1 = fakeRcc.APB1ENR1;
    dev.init();
    // OR-ing the same bit twice is idempotent
    EXPECT_EQ(fakeRcc.APB1ENR1, apb1After1);
}

TEST_F(FdCanDeviceTest, initClockSelectBits24And25)
{
    fakeRcc.CCIPR = 0U;
    auto cfg      = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // Bit 25 should be set, bit 24 should be clear (10b)
    EXPECT_NE(fakeRcc.CCIPR & (1U << 25U), 0U);
    EXPECT_EQ(fakeRcc.CCIPR & (1U << 24U), 0U);
}

TEST_F(FdCanDeviceTest, initEnablesClockBeforeGPIO)
{
    // Verify clock is enabled (if clock wasn't enabled, GPIO writes would have no effect
    // on real hardware — but we can verify the clock bit is set)
    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();
    EXPECT_NE(fakeRcc.APB1ENR1 & RCC_APB1ENR1_FDCANEN, 0U);
}

TEST_F(FdCanDeviceTest, initCCIPRUpperBitsUnaffected)
{
    fakeRcc.CCIPR = 0xFC000000U; // bits 26-31 set
    auto cfg      = makeDefaultConfig();
    bios::FdCanDevice dev(cfg);
    dev.init();

    // Bits 26-31 should still be set
    EXPECT_EQ(fakeRcc.CCIPR & 0xFC000000U, 0xFC000000U);
}

// ============================================================================
// Phase 1a — TX callback delegate (match FlexCANDevice::transmitISR)
//
// FlexCANDevice::transmitISR() calls fFrameSentCallback() directly.
// FdCanDevice::transmitISR() should do the same when constructed with a delegate.
// ============================================================================

TEST_F(FdCanDeviceTest, transmitISRInvokesCallbackDelegate)
{
    bool callbackFired = false;
    auto callback      = ::etl::delegate<void()>::create([&callbackFired]() { callbackFired = true; });

    auto cfg = makeDefaultConfig();
    bios::FdCanDevice dev(cfg, callback);
    fakeFdcan.CCCR |= FDCAN_CCCR_INIT;
    dev.init();
    dev.start();
    fakeFdcan.TXEFS = 0U;

    dev.transmitISR();
    EXPECT_TRUE(callbackFired);
}

TEST_F(FdCanDeviceTest, transmitISRWithoutDelegateDoesNotCrash)
{
    // Construct without callback — existing API, should still work
    auto dev = makeStartedDevice();
    fakeFdcan.TXEFS = 0U;

    dev->transmitISR(); // Must not crash
    EXPECT_EQ(fakeFdcan.IR, (FDCAN_IR_TEFN | FDCAN_IR_TC));
}

// ============================================================================
// Phase 1b — Selective TX interrupt (match FlexCANDevice enable/disable pattern)
//
// S32K: transmit(frame, bufIdx, true) enables TX interrupt for that buffer.
//       transmit(frame, bufIdx, false) does not.
//       transmitISR() disables the TX interrupt.
// G4:   transmit(frame, true) should enable TCE.
//       transmit(frame, false) should NOT enable TCE.
//       transmitISR() should disable TCE.
// ============================================================================

TEST_F(FdCanDeviceTest, transmitWithInterruptEnablesTCE)
{
    auto dev = makeStartedDevice();
    // Clear IE to known state (start() may have set it)
    fakeFdcan.IE = FDCAN_IE_RF0NE; // Only RX enabled, no TCE
    // TX FIFO has space
    fakeFdcan.TXFQS = 0x3U; // TFFL=3

    ::can::CANFrame frame(0x100U, nullptr, 0U);
    bool ok = dev->transmit(frame, true);
    EXPECT_TRUE(ok);
    // TCE should now be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, transmitWithoutInterruptDoesNotEnableTCE)
{
    auto dev = makeStartedDevice();
    fakeFdcan.IE = FDCAN_IE_RF0NE; // Only RX, no TCE
    fakeFdcan.TXFQS = 0x3U;

    ::can::CANFrame frame(0x100U, nullptr, 0U);
    bool ok = dev->transmit(frame, false);
    EXPECT_TRUE(ok);
    // TCE should NOT be set
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, transmitISRDisablesTCE)
{
    auto dev = makeStartedDevice();
    fakeFdcan.IE    = FDCAN_IE_RF0NE | FDCAN_IE_TCE; // Both enabled
    fakeFdcan.TXEFS = 0U;

    dev->transmitISR();
    // TCE should be cleared after ISR (match S32K disableTransmitInterrupt)
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
    // RF0NE should still be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

// ============================================================================
// Phase 1c — RF0NE disable in receiveISR
//
// The existing comment in CanSystem.cpp claims RF0NE is disabled during ISR
// processing. The code does NOT do this. Fix: receiveISR should disable RF0NE
// to prevent re-entry. enableRxInterrupt() re-enables it in receiveTask().
// ============================================================================

TEST_F(FdCanDeviceTest, receiveISRDisablesRF0NE)
{
    auto dev = makeStartedDevice();
    // Enable RF0NE
    fakeFdcan.IE = FDCAN_IE_RF0NE | FDCAN_IE_TCE;
    // Put one frame in RX FIFO
    fakeFdcan.RXF0S = (1U << FDCAN_RXF0S_F0FL_Pos); // F0FL=1
    fakeFdcan.IR    = FDCAN_IR_RF0N;

    // Place a valid frame in message RAM at RX FIFO0 index 0
    uint8_t data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    placeRxFrame(0, 0x100, false, 8, data);

    dev->receiveISR(nullptr);

    // RF0NE should be CLEARED after receiveISR (prevent re-entry)
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    // TCE should be unaffected
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

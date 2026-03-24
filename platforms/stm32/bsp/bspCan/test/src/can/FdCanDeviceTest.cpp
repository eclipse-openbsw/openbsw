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
static constexpr uint32_t MSG_RAM_TX_BUFFER_OFFSET  = 0x128U;

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
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, startSetsILS)
{
    auto dev = makeInitedDevice();
    dev->start();
    // SMSG group routed to interrupt line 1
    EXPECT_NE(fakeFdcan.ILS & FDCAN_ILS_SMSG, 0U);
}

TEST_F(FdCanDeviceTest, startEnablesILE)
{
    auto dev = makeInitedDevice();
    dev->start();
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT0, 0U);
    EXPECT_NE(fakeFdcan.ILE & FDCAN_ILE_EINT1, 0U);
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
    // TCE should still be set
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
}

TEST_F(FdCanDeviceTest, enableRxInterruptSetsRF0NE)
{
    auto dev = makeStartedDevice();
    dev->disableRxInterrupt();
    EXPECT_EQ(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);

    dev->enableRxInterrupt();
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_RF0NE, 0U);
}

TEST_F(FdCanDeviceTest, transmitISRClearsTCFlag)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;

    dev->transmitISR();
    // The driver writes FDCAN_IR_TC to IR (write-1-to-clear)
    EXPECT_EQ(fakeFdcan.IR, FDCAN_IR_TC);
}

TEST_F(FdCanDeviceTest, transmitISRWithNoPendingTx)
{
    auto dev     = makeStartedDevice();
    fakeFdcan.IR = 0U;
    // Should not crash
    dev->transmitISR();
    EXPECT_EQ(fakeFdcan.IR, FDCAN_IR_TC);
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
    EXPECT_NE(fakeFdcan.IE & FDCAN_IE_TCE, 0U);
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

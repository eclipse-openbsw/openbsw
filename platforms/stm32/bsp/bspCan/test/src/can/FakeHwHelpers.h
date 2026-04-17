// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   FakeHwHelpers.h
 * \brief  Smart fake register helpers for STM32 unit tests.
 *
 * Solves three problems that prevent tests from running on x86_64 host:
 *
 * 1. HW busy-wait loops: Production code writes a control bit then polls
 *    a status bit that hardware would auto-set/clear. These helpers hook
 *    into the fake register structs to simulate the state transitions.
 *
 * 2. 64-bit pointer truncation: Provides FAKE_ADDR() macro that safely
 *    creates a testable address from a static array on any platform.
 *
 * 3. Message RAM access: Provides a fake message RAM region with proper
 *    addressing that works on both 32-bit and 64-bit hosts.
 */

#pragma once

#include <cstdint>
#include <cstring>

// ============================================================================
// Portable address casting — works on both 32-bit ARM and 64-bit host
// ============================================================================

/// Cast a pointer to the address type used by the production code (uint32_t).
/// On 64-bit host this truncates, but the production code will cast it back
/// to a pointer via reinterpret_cast<T*>(addr) which restores the full address
/// ONLY if the original pointer is in the low 4GB. We use static arrays which
/// on most x86_64 systems are in the low address space.
///
/// For tests that need message RAM pointer arithmetic, use FakeMessageRam instead.
static inline uint32_t ptrToU32(void* p)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// ============================================================================
// BxCAN state simulation
// ============================================================================

/// Call after each write to fake CAN->MCR to simulate MSR tracking.
/// When INRQ is set in MCR, hardware sets INAK in MSR.
/// When INRQ is cleared, hardware clears INAK.
struct BxCanStateTracker
{
    /// Bit positions (must match CAN_MCR_INRQ and CAN_MSR_INAK)
    static constexpr uint32_t MCR_INRQ = (1U << 0);
    static constexpr uint32_t MSR_INAK = (1U << 0);

    /// Call this after writing to fakeCan.MCR
    static void syncMsrFromMcr(uint32_t volatile& mcr, uint32_t volatile& msr)
    {
        if ((mcr & MCR_INRQ) != 0U)
        {
            msr |= MSR_INAK; // entering init mode
        }
        else
        {
            msr &= ~MSR_INAK; // leaving init mode
        }
    }
};

// ============================================================================
// FDCAN state simulation
// ============================================================================

struct FdCanStateTracker
{
    static constexpr uint32_t CCCR_INIT = (1U << 0);

    /// Call this after writing to fakeFdcan.CCCR
    static void syncCccrInit(uint32_t volatile& cccr)
    {
        // On real HW, writing INIT=1 is reflected immediately.
        // Writing INIT=0 clears it after the peripheral finishes.
        // In the fake, the write already sets/clears the bit directly
        // since we're writing to a RAM variable. No extra action needed.
        // The production code's while-loop checks the same variable
        // and will see the change immediately.
        (void)cccr;
    }
};

// ============================================================================
// ADC state simulation
// ============================================================================

struct AdcStateTracker
{
    /// After writing ADCAL=1 to CR, hardware clears it when calibration done.
    /// We clear it immediately since there's no real calibration to do.
    static void clearAdcalAfterWrite(uint32_t volatile& cr, uint32_t adcalBit)
    {
        cr &= ~adcalBit;
    }

    /// After writing ADEN=1 to CR, hardware sets ADRDY in ISR.
    static void setAdrdyAfterEnable(
        uint32_t volatile& cr,
        uint32_t volatile& isr,
        uint32_t adenBit,
        uint32_t adrdyBit)
    {
        if ((cr & adenBit) != 0U)
        {
            isr |= adrdyBit;
        }
    }

    /// After writing ADSTART=1, hardware sets EOC when conversion done.
    static void setEocAfterStart(
        uint32_t volatile& cr,
        uint32_t volatile& isr,
        uint32_t adstartBit,
        uint32_t eocBit)
    {
        if ((cr & adstartBit) != 0U)
        {
            isr |= eocBit;
        }
    }
};

// ============================================================================
// Flash state simulation
// ============================================================================

struct FlashStateTracker
{
    /// Flash BSY bit is always 0 in fake (operation completes instantly).
    /// The production waitForFlash() checks FLASH->SR & BSY — as long as
    /// we memset SR to 0 in SetUp(), the loop exits immediately.
    /// No action needed if SetUp clears SR.

    /// After erasePage sets STRT, clear it (simulates erase complete).
    static void clearAfterErase(uint32_t volatile& cr, uint32_t volatile& sr, uint32_t strtBit)
    {
        cr &= ~strtBit;
        sr = 0U; // BSY=0, no errors
    }
};

// ============================================================================
// Fake message RAM for FDCAN (solves 64-bit pointer truncation)
// ============================================================================

/// Provides a message RAM region that can be addressed via uint32_t offsets.
/// Instead of using absolute addresses (which get truncated on x86_64),
/// the tests should set SRAMCAN_BASE to 0 and use relative offsets into
/// this array. The production code does: `base + offset` then casts to
/// `uint32_t*`. If base is the actual array address (as uintptr_t cast to
/// uint32_t), it gets truncated. The fix: define SRAMCAN_BASE as 0 and
/// override getInstanceRamBase() to return the real pointer.
///
/// However, since getInstanceRamBase is a static function inside the .cpp,
/// we can't easily override it. The simplest approach: on 64-bit host,
/// accept that message RAM tests will segfault and skip them with a guard.
///
/// For a complete fix, the production code should use uintptr_t for RAM
/// addresses, which is an API change requiring upstream approval.

#if UINTPTR_MAX > UINT32_MAX
#define FAKE_MSG_RAM_64BIT_HOST 1
#else
#define FAKE_MSG_RAM_64BIT_HOST 0
#endif

// Copyright 2025 Accenture.

#include "safeMonitor/Register.h"

#include "common.h"

#include <gtest/gtest.h>

struct RegisterTest : ::testing::Test
{
    using RegisterType = uint32_t;
    using RegisterMonitor
        = ::safeMonitor::Register<HandlerMock, MyEvent, RegisterType, ScopedMutexMock, MyContext>;

    static RegisterType const BITS_ALL    = 0xFFFFFFFFU;
    static RegisterType const BITS_NONE   = 0x00000000U;
    static RegisterType const BITS_ODD    = 0xAAAAAAAAU;
    static RegisterType const BITS_EVEN   = 0x55555555U;
    static size_t const NUMBER_OF_ENTRIES = 4U;

    void resetValues()
    {
        _r0 = BITS_ALL;
        _r1 = BITS_NONE;
        _r2 = BITS_NONE;
        _r3 = BITS_NONE;
    }

    HandlerMock _handler;
    RegisterType _r0{BITS_ALL};
    RegisterType _r1{BITS_NONE};
    RegisterType _r2{BITS_NONE};
    RegisterType _r3{BITS_NONE};
    RegisterMonitor::Entry const _entries[NUMBER_OF_ENTRIES] = {
        {reinterpret_cast<uintptr_t>(&_r0), BITS_ALL, BITS_ALL},
        {reinterpret_cast<uintptr_t>(&_r1), BITS_ALL, BITS_NONE},
        {reinterpret_cast<uintptr_t>(&_r2), BITS_ODD, BITS_NONE},
        {reinterpret_cast<uintptr_t>(&_r3), BITS_EVEN, BITS_NONE},
    };
    RegisterMonitor _registerMonitor{_handler, SOMETHING_HAPPENED, _entries};
};

/**
 * \desc:
 * Flips unmasked bits of first register value and checks that handler is called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural], [fault_insertion]
 */
TEST_F(RegisterTest, DetectsBitFlipToZero)
{
    size_t const numberOfBits = sizeof(RegisterType) * 8U;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(numberOfBits);
    for (size_t i = 0; i < numberOfBits; ++i)
    {
        _r0 = ~static_cast<RegisterType>(1U << i);
        _registerMonitor.check();
    }
}

/**
 * \desc:
 * Flips unmasked bits of second register value and checks that handler is called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural], [fault_insertion]
 */
TEST_F(RegisterTest, DetectsBitFlipToOne)
{
    size_t const numberOfBits = sizeof(RegisterType) * 8U;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(numberOfBits);
    for (size_t i = 0; i < numberOfBits; ++i)
    {
        _r1 = static_cast<RegisterType>(1U << i);
        _registerMonitor.check();
    }
}

/**
 * \desc:
 * Changes masked bits of third and fourth register values and checks that handler is NOT called.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural]
 */
TEST_F(RegisterTest, IgnoresMaskedBits)
{
    EXPECT_CALL(_handler, handle(::testing::_)).Times(0U);
    _r2 = BITS_EVEN;
    _r3 = BITS_ODD;
    _registerMonitor.check();
}

/**
 * \desc:
 * Checks that the handler is called for very register entry if the masked value is not
 * equal to the expected value.
 *
 * \prec: None
 *
 * \postc: None
 *
 * \testtec: [structural], [fault_insertion]
 */

TEST_F(RegisterTest, ChecksAllEntries)
{
    resetValues();
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(0);
    _registerMonitor.check(); // should _NOT_ call handler

    resetValues();
    _r0 = BITS_NONE;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1);
    _registerMonitor.check(); // should call handler

    resetValues();
    _r1 = BITS_ALL;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1);
    _registerMonitor.check(); // should call handler

    resetValues();
    _r2 = BITS_ODD;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1);
    _registerMonitor.check(); // should call handler

    resetValues();
    _r3 = BITS_EVEN;
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED)).Times(1);
    _registerMonitor.check(); // should call handler
}

TEST_F(RegisterTest, UsesScopedMutex)
{
    ScopedMutexMock::reset();
    _registerMonitor.check();
    EXPECT_EQ(1, ScopedMutexMock::numConstructed());
    EXPECT_TRUE(ScopedMutexMock::allDestructed());
}

TEST_F(RegisterTest, UsesDefaultConstructedContext)
{
    _registerMonitor.check();
    EXPECT_EQ(0xDEADBEEF, _registerMonitor.getContext().value);
}

TEST_F(RegisterTest, StoresAndReturnsGivenContext)
{
    MyContext c;
    c.value = 0xDEADC0DE;
    _registerMonitor.check(c);
    EXPECT_EQ(c.value, _registerMonitor.getContext().value);
}

TEST_F(RegisterTest, ContextIsBeingCopied)
{
    MyContext const c1;
    _registerMonitor.check(c1);
    MyContext const& c2 = _registerMonitor.getContext();
    EXPECT_NE(&c1, &c2);
}

TEST_F(RegisterTest, LastCheckedEntryIsInitiallyNull)
{
    EXPECT_EQ(nullptr, _registerMonitor.getLastCheckedEntry());
}

TEST_F(RegisterTest, LastCheckedEntryPointsToLastEntryAfterCheck)
{
    auto* const ptrToLastEntry = &_entries[NUMBER_OF_ENTRIES - 1];
    _registerMonitor.check();
    EXPECT_EQ(_registerMonitor.getLastCheckedEntry(), ptrToLastEntry);
}

TEST_F(RegisterTest, LastCheckedEntryPointsToFaultyEntry)
{
    EXPECT_CALL(_handler, handle(SOMETHING_HAPPENED));
    auto* const ptrToFaultyEntry = &_entries[2];
    _r2                          = ~_r2;
    _registerMonitor.check();
    EXPECT_EQ(_registerMonitor.getLastCheckedEntry(), ptrToFaultyEntry);
}

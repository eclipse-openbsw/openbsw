// Copyright 2024 Accenture.

#pragma once

#include <etl/singleton_base.h>

#include <gmock/gmock.h>

namespace testing
{
/**
 * SystemTimerMock
 *
 * \section SystemTimerMock_example Usage example
 * \code{.cpp}
 * TEST(example, using_mock)
 * {
 *     ::testing SystemTimerMock systemTimer;
 *     EXPECT_CALL(systemTimer, getSystemTimeUs32Bit())
 *         .WillOnce(Return(10U));
 *
 *     EXPECT_THAT(10U, getSystemTimeUs32Bit());
 * }
 * \endcode
 */

class SystemTimerMock : public ::etl::singleton_base<SystemTimerMock>
{
public:
    SystemTimerMock() : ::etl::singleton_base<SystemTimerMock>(*this) {}

    /**
     *  \see sysDelayUs(uint32_t delay)
     */
    MOCK_METHOD(void, sysDelayUs, (uint32_t delay));

    /**
     * \see getSystemTicks(void)
     */
    MOCK_METHOD(uint64_t, getSystemTicks, ());

    /*
     * \see getSystemTimeUs32Bit(void)
     */
    MOCK_METHOD(uint32_t, getSystemTimeUs32Bit, ());

    /*
     * \see getSystemTimeMs32Bit(void)
     */
    MOCK_METHOD(uint32_t, getSystemTimeMs32Bit, ());

    /**
     * \see getSystemTicks32Bit(void)
     */
    MOCK_METHOD(uint32_t, getSystemTicks32Bit, ());

    /**
     * \see getSystemTimeNs(void)
     */
    MOCK_METHOD(uint64_t, getSystemTimeNs, ());

    /**
     * \see getSystemTimeUs(void)
     */
    MOCK_METHOD(uint64_t, getSystemTimeUs, ());

    /**
     * \see getSystemTimeMs(void)
     */
    MOCK_METHOD(uint64_t, getSystemTimeMs, ());

    /**
     * \see systemTicksToTimeUs(uint64_t ticks)
     */
    MOCK_METHOD(uint64_t, systemTicksToTimeUs, (uint64_t ticks));

    /**
     * \see systemTicksToTimeNs(uint64_t ticks)
     */
    MOCK_METHOD(uint64_t, systemTicksToTimeNs, (uint64_t ticks));

    /**
     * \see initSystemTimer()
     */
    MOCK_METHOD(void, initSystemTimer, ());
};

} // namespace testing

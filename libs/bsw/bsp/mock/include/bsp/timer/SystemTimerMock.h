// Copyright 2024 Accenture.

#pragma once

#include <estd/singleton.h>

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

class SystemTimerMock : public ::estd::singleton<SystemTimerMock>
{
public:
    SystemTimerMock() : ::estd::singleton<SystemTimerMock>(*this) {}

    /**
     *  \see sysDelayUs(uint32_t delay)
     */
    MOCK_METHOD1(sysDelayUs, void(uint32_t delay));

    /**
     * \see getSystemTicks(void)
     */
    MOCK_METHOD0(getSystemTicks, uint64_t());

    /*
     * \see getSystemTimeUs32Bit(void)
     */
    MOCK_METHOD0(getSystemTimeUs32Bit, uint32_t());

    /*
     * \see getSystemTimeMs32Bit(void)
     */
    MOCK_METHOD0(getSystemTimeMs32Bit, uint32_t());

    /**
     * \see getSystemTicks32Bit(void)
     */
    MOCK_METHOD0(getSystemTicks32Bit, uint32_t());

    /**
     * \see getSystemTimeNs(void)
     */
    MOCK_METHOD0(getSystemTimeNs, uint64_t());

    /**
     * \see getSystemTimeUs(void)
     */
    MOCK_METHOD0(getSystemTimeUs, uint64_t());

    /**
     * \see getSystemTimeMs(void)
     */
    MOCK_METHOD0(getSystemTimeMs, uint64_t());

    /**
     * \see systemTicksToTimeUs(uint64_t ticks)
     */
    MOCK_METHOD1(systemTicksToTimeUs, uint64_t(uint64_t ticks));

    /**
     * \see systemTicksToTimeNs(uint64_t ticks)
     */
    MOCK_METHOD1(systemTicksToTimeNs, uint64_t(uint64_t ticks));

    /**
     * \see initSystemTimer()
     */
    MOCK_METHOD0(initSystemTimer, void());
};

} // namespace testing


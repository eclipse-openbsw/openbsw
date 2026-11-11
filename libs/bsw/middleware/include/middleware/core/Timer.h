/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/type_traits.h>

namespace middleware::core
{

/**
 * \brief This Timer class can be used to countdown timespans or ticks.
 *
 * The Timer is "time" agnostic, which means it can be used for counting down
 * different timespans (ms, seconds, ...) or just ticks. The functionality is
 * determined by the initialisation value and the use of the Timer::decrement()
 * method. To countdown tick, just call the Timer::decrement() method when
 * needed. To countdown times, set the initial value accordingly to the planned
 * frequency and call Timer::decrement() at a regular frequency.
 *
 * The Timer has three states:
 * - Running: The initially set timespan is not expired.
 * - Expired: The initially set timespan has expired ( @see Timer::isExpired() ).
 * - Reset: The timer is in reset state ( @see Timer::isReset() ).
 *          It can be put into the running state by (re)setting to a new, to be
 *          counted down, timespan/ticks via Timer::setCounter().
 *
 * For compatibility, the maximum value of T maps to the reset state and never
 * expires when decremented. Finite countdowns range from zero (already expired)
 * to the maximum value of T minus one.
 */
template<typename T>
class Timer
{
    static_assert(etl::is_unsigned_v<T>, "T must be an unsigned type.");

public:
    constexpr Timer() = default;

    explicit constexpr Timer(T const ticks)
    : counter_(static_cast<T>(static_cast<T>(ticks) + EXPIRED_TIME))
    {}

    Timer(Timer const&)                = default;
    Timer(Timer&&) noexcept            = default;
    Timer& operator=(Timer const&)     = default;
    Timer& operator=(Timer&&) noexcept = default;
    ~Timer()                           = default;

    /**
     * \brief Tick down the actual counter value.
     */
    inline void decrement()
    {
        if (counter_ > EXPIRED_TIME)
        {
            --counter_;
        }
    }

    inline constexpr bool isReset() const { return counter_ == RESET_TIME; }

    inline constexpr bool isExpired() const { return counter_ == EXPIRED_TIME; }

    inline constexpr bool isRunning() const { return counter_ > EXPIRED_TIME; }

    inline void setCounter(T const time)
    {
        counter_ = static_cast<T>(static_cast<T>(time) + EXPIRED_TIME);
    }

    inline T getCounter() const { return static_cast<T>(static_cast<T>(counter_) - EXPIRED_TIME); }

    inline void reset() { counter_ = RESET_TIME; }

    inline void expire() { counter_ = EXPIRED_TIME; }

private:
    static constexpr T RESET_TIME{static_cast<T>(0U)};
    static constexpr T EXPIRED_TIME{static_cast<T>(1U)};
    static constexpr T DECREMENT_STEP{static_cast<T>(1U)};
    T counter_{RESET_TIME};
};

/**
 * \brief Fixed-period countdown wrapper around Timer.
 */
template<typename T, T t>
class Ticker
{
public:
    constexpr Ticker() : timer_(t) {}

    Ticker(Ticker const&)                = default;
    Ticker(Ticker&&) noexcept            = default;
    Ticker& operator=(Ticker const&)     = default;
    Ticker& operator=(Ticker&&) noexcept = default;
    ~Ticker()                            = default;

    inline constexpr bool isExpired() const { return timer_.isExpired(); }

    inline constexpr bool isReset() const { return timer_.isReset(); }

    inline constexpr bool isRunning() const { return timer_.isRunning(); }

    inline void expire() { timer_.expire(); }

    inline void reset() { timer_.reset(); }

    inline constexpr T elapsed() const
    {
        if (timer_.isReset())
        {
            return static_cast<T>(0U);
        }
        return static_cast<T>(static_cast<T>(MAX_COUNT) - timer_.getCounter());
    }

    inline void restart() { timer_.setCounter(MAX_COUNT); }

    [[maybe_unused]] inline bool tick()
    {
        timer_.decrement();
        return !timer_.isExpired();
    }

private:
    static constexpr T MAX_COUNT{t};
    Timer<T> timer_;
};

} // namespace middleware::core

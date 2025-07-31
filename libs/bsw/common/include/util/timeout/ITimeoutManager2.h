// Copyright 2024 Accenture.

/**
 * Contains interface ITimeoutManager2.
 * \file ITimeoutManager2.h
 */
#pragma once

#include <etl/uncopyable.h>

#include <platform/estdint.h>

namespace common
{
class AbstractTimeout;

/**
 * Interface of timeoutmanager class.
 *
 *
 */
class ITimeoutManager2 : public ::etl::uncopyable
{
public:
    ITimeoutManager2() = default;

    /**
     * Errorcodes used by ITimeoutManager2.
     */
    enum ErrorCode
    {
        /** everything OK */
        TIMEOUT_OK,
        /** a given timeout was already in use */
        TIMEOUT_ALREADY_SET,
        /** a given parameter is invalid */
        TIMEOUT_INVALID_VALUE
    };

    /**
     * Initializes TimeoutManager.
     */
    virtual void init() = 0;

    /**
     * Shuts down TimeoutManager.
     */
    virtual void shutdown() = 0;

    /**
     * Sets a timeout.
     * \param timeout AbstractTimeout whose expired method gets called once
     * the given time is up
     * \param time time in ms when the timeout shall expire
     * \param cyclic flag indicating if the timeou shall be cyclic
     *
     * \return ErrorCode
     *          - TIMEOUT_OK: timeout has been successfully set
     *          - TIMEOUT_ALREADY_SET: the given timeout was already in use
     *          - TIMEOUT_INVALID_VALUE: time == 0
     */
    virtual ErrorCode set(AbstractTimeout& timeout, uint32_t time, bool cyclic) = 0;

    /**
     * Cancels a given timeout.
     * \param timeout AbstractTimeout to be canceled
     *
     * \warning
     * Because of task priorities and preemption it might be possible that
     * the expired callback is called although the client is currently in a
     * state where it tries to cancel the timeout. Therefore users should
     * protect the expired callback and the code that calls cancel on the timeout
     * with a critical section.
     */
    virtual void cancel(AbstractTimeout& timeout) = 0;
};

} // namespace common

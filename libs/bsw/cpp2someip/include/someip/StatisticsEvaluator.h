/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/QueryManager.h"
#include "someip/Statistics.h"

#include <async/Types.h>
#include <async/util/Call.h>

#include <cstdint>

namespace someip
{
class IServiceRegistry;
class QueryManager;
} // namespace someip

namespace someip
{
class StatisticsEvaluator
{
public:
    StatisticsEvaluator(
        ::async::ContextType const ethernetContext,
        IServiceRegistry& serviceRegistry,
        QueryManager& queryManager);

    virtual ~StatisticsEvaluator() = default;

    void init();
    void shutdown();

    void reset();

    void cyclic();

    uint32_t getNumReceivedMessagesPerSecond() const;
    uint32_t getNumSentMessagesPerSecond() const;
    uint32_t getNumReceivedSdMessagesPerSecond() const;
    uint32_t getNumSentSdMessagesPerSecond() const;

    uint32_t getNumSentTrainsPerSecond() const;

    uint32_t getNumTrainsSentBecauseFullPerSecond() const;

    uint32_t getNumTrainsSentBecauseNoFreeTrainPerSecond() const;

    uint32_t getNumTrainsSentBecauseDuplicatePduPerSecond() const;

    uint32_t getNumTrainsSentBecauseTimeoutPerSecond() const;

private:
    static uint32_t safeDivide(uint32_t numerator, uint32_t denominator);

    ::async::ContextType const _ethernetContext;
    ::async::Function _cyclicFunction;
    ::async::TimeoutType _cyclicTimeout;
    IServiceRegistry& _serviceRegistry;
    QueryManager& _queryManager;
    uint32_t _secondsExpired;
};

/*
 * inline implementation
 */
inline uint32_t StatisticsEvaluator::getNumReceivedMessagesPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::PDU_RX), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumSentMessagesPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::PDU_TX), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumReceivedSdMessagesPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::SD_FRAME_RX), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumSentSdMessagesPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::SD_FRAME_TX), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumSentTrainsPerSecond() const
{
    return safeDivide(Statistics::getNumSentTrains(), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumTrainsSentBecauseFullPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::TRAIN_FULL), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumTrainsSentBecauseNoFreeTrainPerSecond() const
{
    return safeDivide(
        Statistics::getCounter(Statistics::Counter::TRAIN_NOT_AVAILABLE), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumTrainsSentBecauseDuplicatePduPerSecond() const
{
    return safeDivide(
        Statistics::getCounter(Statistics::Counter::TRAIN_DUPLICATE_PDU), _secondsExpired);
}

inline uint32_t StatisticsEvaluator::getNumTrainsSentBecauseTimeoutPerSecond() const
{
    return safeDivide(Statistics::getCounter(Statistics::Counter::TRAIN_TIMEOUT), _secondsExpired);
}

// static
inline uint32_t
StatisticsEvaluator::safeDivide(uint32_t const numerator, uint32_t const denominator)
{
    if (denominator > 0U)
    {
        return numerator / denominator;
    }

    return 0U;
}

} // namespace someip

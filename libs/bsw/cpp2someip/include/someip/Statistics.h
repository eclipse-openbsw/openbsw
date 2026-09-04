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

#include <cstdint>

namespace someip
{
class Statistics
{
public:
    enum class Counter : uint16_t
    {
        PDU_TX,
        PDU_RX,
        FRAME_RX,
        TRAIN_FULL,
        TRAIN_NOT_AVAILABLE,
        TRAIN_DUPLICATE_PDU,
        TRAIN_TIMEOUT,
        SD_FRAME_TX,
        SD_FRAME_RX,
        SD_MESSAGE_RX,
        SD_REBOOT,
        SD_FIND_RX,
        SD_OFFER_RX,
        SD_FIND_EVENTGROUP_RX,
        SD_PUBLISH_RX,
        SD_SUBSCRIBE_RX,
        SD_SUBSCRIBE_ACK_RX,
        SD_SUBSCRIBE_NACK_RX,
        SD_UNKNOWN_RX,
        SD_MALFORMED_MESSAGE_RX,
        RPC_WRONG_PROTOCOL_VERSION_RX,
        RPC_WRONG_INTERFACE_VERSION_RX,
        RPC_UNKNOWN_SERVICE_RX,
        RPC_UNKNOWN_METHOD_RX,
        RPC_MALFORMED_MESSAGE_RX,
        COUNTER_ARRAY_SIZE
    };

    static void reset();

    static void incCounter(Counter counter);
    static uint32_t getCounter(Counter counter);
    static void resetCounter(Counter counter);

    static uint32_t getNumSentTrains();

    static uint32_t numIncomingSubscriptions;
    static uint32_t maxNumIncomingSubscriptions;
    static uint32_t numRemoteServices;
    static uint32_t maxNumRemoteServices;
    static uint32_t numProvidedServices;
    static uint32_t maxNumProvidedServices;
    static uint32_t numQueries;
    static uint32_t maxNumQueries;

private:
    static uint32_t counters[static_cast<uint16_t>(Counter::COUNTER_ARRAY_SIZE)];
};

// static
inline void Statistics::incCounter(Counter const counter)
{
    if (static_cast<uint16_t>(counter) < static_cast<uint16_t>(Counter::COUNTER_ARRAY_SIZE) - 1U)
    {
        ++counters[static_cast<uint16_t>(counter)];
    }
}

// static
inline uint32_t Statistics::getCounter(Counter const counter)
{
    return counters[static_cast<uint16_t>(counter)];
}

inline void Statistics::resetCounter(Counter const counter)
{
    counters[static_cast<uint16_t>(counter)] = 0U;
}

// static
inline uint32_t Statistics::getNumSentTrains()
{
    return counters[static_cast<uint16_t>(Counter::TRAIN_FULL)]
           + counters[static_cast<uint16_t>(Counter::TRAIN_NOT_AVAILABLE)]
           + counters[static_cast<uint16_t>(Counter::TRAIN_DUPLICATE_PDU)]
           + counters[static_cast<uint16_t>(Counter::TRAIN_TIMEOUT)];
}

} // namespace someip

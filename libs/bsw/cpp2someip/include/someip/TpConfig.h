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

#include "someip/SomeIpConstants.h"
#include "someip/TpReceiver.h"
#include "someip/TpSender.h"
#include <cstdint>

namespace someip
{
/**
 * The TP configuration.
 */
class TpConfig
{
public:
    TpConfig(TpConfig const&)            = delete;
    TpConfig& operator=(TpConfig const&) = delete;

    ::etl::span<TpSender*> tpSenders;
    ::etl::span<TpReceiver*> tpReceivers;

protected:
    TpConfig(::etl::span<TpSender*> const senders, ::etl::span<TpReceiver*> const receivers)
    : tpSenders(senders), tpReceivers(receivers)
    {}

    ~TpConfig() = default;
};

namespace internal
{
/**
 * Internal TP resources.
 */
template<uint8_t NumTpStreams, size_t TpBufferSize>
class TpResources : public TpConfig
{
public:
    TpResources()
    : TpConfig(
        ::etl::span<TpSender*>(&_senderPtr, 1),
        ::etl::span<TpReceiver*>(_receiverList, NumTpStreams))
    , _senderPtr(&_sender)
    , _receiverArray()
    {
        for (size_t i = 0; i < NumTpStreams; ++i)
        {
            _receiverList[i] = &(_receiverArray[i]);
        }
    }

private:
    ::someip::declare::TpSender<UDP_PACKET_MAX_SIZE> _sender;
    TpSender* _senderPtr;

    ::someip::declare::TpReceiver<TpBufferSize> _receiverArray[NumTpStreams];
    TpReceiver* _receiverList[NumTpStreams];
};

template<size_t TpBufferSize>
class TpResources<0U, TpBufferSize> : public TpConfig
{
public:
    TpResources() : TpConfig({}, {}) {}
};
} // namespace internal
} // namespace someip

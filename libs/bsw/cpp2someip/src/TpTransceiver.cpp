/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpTransceiver.h"

#include "bsp/timer/SystemTimer.h"
#include "someip/logger.h"

#include <util/timeout/ITimeoutManager2.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::common::ITimeoutManager2;
using ::util::logger::SOMEIP;

TpTransceiver::TpTransceiver(
    ::async::ContextType const ethernetContext,
    ::etl::span<TpSender*>& senders,
    ::etl::span<TpReceiver*>& receivers)
: _ethernetContext(ethernetContext)
, _cyclicFunction(::async::Function::CallType::create<TpTransceiver, &TpTransceiver::cyclic>(*this))
, _cyclicTimeout()
, _senders(senders)
, _receivers(receivers)
, _busy(false)
{}

void TpTransceiver::stop()
{
    if (_busy)
    {
        _cyclicTimeout.cancel();
        _busy = false;
    }

    for (TpReceiver* const receiver : _receivers)
    {
        if ((receiver != nullptr) && (receiver->isActive()))
        {
            receiver->stop();
        }
    }
}

// virtual
bool TpTransceiver::sendTpMessage(NetworkChannel& channel, SomeIpMessage const& message) const
{
    if (_senders.size() == 0U)
    {
        ERROR_LOG(SOMEIP, "TpTransceiver::sendTpMessage(): no sender available!");
        return false;
    }

    TpSender::TpResult const result = _senders.at(0U)->send(channel, message); // sync !

    return (TpSender::TpResult::TP_OK == result);
}

// virtual
void TpTransceiver::receiveTpMessage(
    NetworkChannel& channel, SomeIpMessage const& message, ITpListener& listener)
{
    TpReceiver* receiver = nullptr;
    TpReceiver* idle     = nullptr;

    for (TpReceiver* const item : _receivers)
    {
        if (item != nullptr)
        {
            if (item->isMatching(channel, message))
            {
                receiver = item;
                break;
            }
            if ((idle == nullptr) && (!item->isActive()))
            {
                idle = item;
            }
        }
    }

    if ((receiver == nullptr) && (idle != nullptr))
    {
        receiver = idle;
    }

    if (receiver == nullptr)
    {
        ERROR_LOG(SOMEIP, "TpTransceiver::receiveTpMessage(): no receiver available!");
        return;
    }

    if (!receiver->isActive())
    {
        receiver->start(channel, message, listener);
    }

    uint32_t const time = static_cast<uint32_t>(getSystemTimeMs32Bit());

    TpReceiver::TpResult const result = receiver->receive(channel, message, time); // async !

    if (TpReceiver::TpResult::TP_PENDING == result)
    {
        if (!_busy)
        {
            async::schedule(
                _ethernetContext,
                _cyclicFunction,
                _cyclicTimeout,
                TP_UPDATE_CYCLE,
                ::async::TimeUnit::MILLISECONDS);

            _busy = true;
        }
    }
    else
    {
        receiver->stop();

        if (_busy)
        {
            bool busy = false;

            for (TpReceiver* const item : _receivers)
            {
                if ((item != nullptr) && (item->isActive()))
                {
                    busy = true;
                    break;
                }
            }

            if (!busy)
            {
                _cyclicTimeout.cancel();
                _busy = false;
            }
        }
    }
}

void TpTransceiver::cyclic()
{
    uint32_t const time = static_cast<uint32_t>(getSystemTimeMs32Bit());

    bool busy = false;

    for (TpReceiver* const receiver : _receivers)
    {
        if (receiver != nullptr)
        {
            if (receiver->isExpired(time))
            {
                WARN_LOG(SOMEIP, "TpTransceiver::expired(): tp-receiver[%p] timeout!", receiver);

                receiver->stop();
            }
            else if (receiver->isActive())
            {
                busy = true;
            }
            else
            {
                // nothing to do
            }
        }
    }

    _busy = busy;

    if (_busy)
    {
        async::schedule(
            _ethernetContext,
            _cyclicFunction,
            _cyclicTimeout,
            TP_UPDATE_CYCLE,
            ::async::TimeUnit::MILLISECONDS);
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

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

#include "someip/ITpTransceiver.h"
#include "someip/TpReceiver.h"
#include "someip/TpSender.h"

#include <async/Types.h>
#include <async/util/Call.h>

#include <etl/vector.h>

namespace someip
{
/**
 * Implementation of a TP transceiver.
 *
 * Note: Implements a synchronous send and asynchronous receive of TP messages.
 */
class TpTransceiver : public ITpTransceiver
{
public:
    TpTransceiver(
        ::async::ContextType const ethernetContext,
        ::etl::span<TpSender*>& senders,
        ::etl::span<TpReceiver*>& receivers);

    void stop();

    /** see: ITpTransceiver::sendTpMessage */
    bool sendTpMessage(NetworkChannel& channel, SomeIpMessage const& message) const override;

    /** see: ITpTransceiver::receiveTpMessage */
    void receiveTpMessage(
        NetworkChannel& channel, SomeIpMessage const& message, ITpListener& listener) override;

    void cyclic();

private:
    ::async::ContextType const _ethernetContext;
    ::async::Function _cyclicFunction;
    ::async::TimeoutType _cyclicTimeout;

    ::etl::span<TpSender*>& _senders;
    ::etl::span<TpReceiver*>& _receivers;

    bool _busy;
};

} // namespace someip

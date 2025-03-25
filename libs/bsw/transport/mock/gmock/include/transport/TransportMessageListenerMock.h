// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "transport/ITransportMessageListener.h"

#include <gmock/gmock.h>

namespace transport
{
class TransportMessageListenerMock : public ITransportMessageListener
{
public:
    MOCK_METHOD3(
        messageReceived,
        ReceiveResult(uint8_t, TransportMessage&, ITransportMessageProcessedListener*));
};

} // namespace transport


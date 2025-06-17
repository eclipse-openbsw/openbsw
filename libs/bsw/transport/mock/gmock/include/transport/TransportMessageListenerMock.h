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
    MOCK_METHOD(
        ReceiveResult,
        messageReceived,
        (uint8_t, TransportMessage&, ITransportMessageProcessedListener*),
        (override));
};

} // namespace transport

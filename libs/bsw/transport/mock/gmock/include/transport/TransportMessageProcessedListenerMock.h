// Copyright 2024 Accenture.

#pragma once

#include "transport/ITransportMessageProcessedListener.h"

#include <gmock/gmock.h>

namespace transport
{
class TransportMessageProcessedListenerMock : public ITransportMessageProcessedListener
{
public:
    TransportMessageProcessedListenerMock() {}

    virtual ~TransportMessageProcessedListenerMock() {}

    MOCK_METHOD(
        void,
        transportMessageProcessed,
        (TransportMessage & transportMessage, ProcessingResult result));
};

} // namespace transport

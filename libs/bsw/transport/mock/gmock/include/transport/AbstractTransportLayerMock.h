// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#pragma once

#include "transport/AbstractTransportLayer.h"

#include <gmock/gmock.h>

namespace transport
{
class AbstractTransportLayerMock : public AbstractTransportLayer
{
public:
    AbstractTransportLayerMock(uint8_t busId) : AbstractTransportLayer(busId) {}

    virtual ~AbstractTransportLayerMock() {}

    using AbstractTransportLayer::shutdownCompleteDummy;

    MOCK_METHOD(ErrorCode, init, (), (override));
    MOCK_METHOD(bool, shutdown, (ShutdownDelegate), (override));
    MOCK_METHOD(
        ErrorCode, send, (TransportMessage&, ITransportMessageProcessedListener*), (override));

    ITransportMessageProvidingListener& getProvidingListenerHelper_impl()
    {
        return getProvidingListenerHelper();
    }
};

} // namespace transport

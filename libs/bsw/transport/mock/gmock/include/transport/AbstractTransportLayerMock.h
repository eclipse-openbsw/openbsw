/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

    MOCK_METHOD(ErrorCode, init, ());
    MOCK_METHOD(bool, shutdown, (ShutdownDelegate));
    MOCK_METHOD(ErrorCode, send, (TransportMessage&, ITransportMessageProcessedListener*));

    ITransportMessageProvidingListener& getProvidingListenerHelper_impl()
    {
        return fProvidingListenerHelper;
    }
};

} // namespace transport

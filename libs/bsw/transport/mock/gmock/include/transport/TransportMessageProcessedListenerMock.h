/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

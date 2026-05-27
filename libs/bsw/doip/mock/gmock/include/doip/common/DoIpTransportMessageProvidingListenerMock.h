/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/IDoIpTransportMessageProvidingListener.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Mock for DoIP specific transport message providing listener.
 */
class DoIpTransportMessageProvidingListenerMock : public IDoIpTransportMessageProvidingListener
{
public:
    MOCK_METHOD6(
        getTransportMessage,
        GetResult(
            uint8_t sourceBusId,
            uint16_t sourceId,
            uint16_t targetId,
            uint16_t size,
            ::etl::span<uint8_t const> const& peek,
            ::transport::TransportMessage*& transportMessage));
    MOCK_METHOD1(releaseTransportMessage, void(::transport::TransportMessage& transportMessage));
    MOCK_METHOD3(
        messageReceived,
        ReceiveResult(
            uint8_t sourceBusId,
            ::transport::TransportMessage& transportMessage,
            ::transport::ITransportMessageProcessedListener* notificationListener));
};

} // namespace doip

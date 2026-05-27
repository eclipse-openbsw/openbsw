/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTransportMessageProvidingListenerAdapter.h"

namespace doip
{
::transport::ITransportMessageProvider::ErrorCode
DoIpTransportMessageProvidingListenerAdapter::getTransportMessage(
    uint8_t const sourceBusId,
    uint16_t const sourceId,
    uint16_t const targetId,
    uint16_t const size,
    ::etl::span<uint8_t const> const& peek,
    ::transport::TransportMessage*& transportMessage)
{
    return _transportMessageProvidingListener
        .getTransportMessage(sourceBusId, sourceId, targetId, size, peek, transportMessage)
        .getResult();
}

void DoIpTransportMessageProvidingListenerAdapter::releaseTransportMessage(
    ::transport::TransportMessage& transportMessage)
{
    _transportMessageProvidingListener.releaseTransportMessage(transportMessage);
}

::transport::ITransportMessageListener::ReceiveResult
DoIpTransportMessageProvidingListenerAdapter::messageReceived(
    uint8_t const sourceBusId,
    ::transport::TransportMessage& transportMessage,
    ::transport::ITransportMessageProcessedListener* const notificationListener)
{
    return _transportMessageProvidingListener
        .messageReceived(sourceBusId, transportMessage, notificationListener)
        .getResult();
}

void DoIpTransportMessageProvidingListenerAdapter::dump() {}

} // namespace doip

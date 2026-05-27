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

#include <transport/ITransportMessageProvidingListener.h>

namespace doip
{
/**
 * This simple adapter implements a generic transport message providing listener based
 * on a DoIP specific message providing listener.
 *
 * Whenever a specialized DoIP specific transport message provider is required there will
 * still be some transport layers that only accept a generic transport message provider.
 * This adapter implements such a generic transport message provider by mapping all calls
 * to the more specific DoIP transport message provider.
 */
class DoIpTransportMessageProvidingListenerAdapter
: public ::transport::ITransportMessageProvidingListener
{
public:
    /**
     * Constructor.
     * \param transportMessageProvidingListener reference to DoIP specific message providing
     * listener
     */
    explicit DoIpTransportMessageProvidingListenerAdapter(
        IDoIpTransportMessageProvidingListener& transportMessageProvidingListener);

    /**
     * \see ITransportMessageProvidingListener::getTransportMessage()
     */
    ErrorCode getTransportMessage(
        uint8_t sourceBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        ::transport::TransportMessage*& transportMessage) override;

    /**
     * \see ITransportMessageProvidingListener::releaseTransportMessage()
     */
    void releaseTransportMessage(::transport::TransportMessage& transportMessage) override;

    /**
     * \see ITransportMessageProvidingListener::dump()
     */
    void dump() override;

    /**
     * \see ITransportMessageProvidingListener::messageReceived()
     */
    ReceiveResult messageReceived(
        uint8_t sourceBusId,
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* notificationListener) override;

private:
    IDoIpTransportMessageProvidingListener& _transportMessageProvidingListener;
};

/**
 * Inline implementations.
 */
inline DoIpTransportMessageProvidingListenerAdapter::DoIpTransportMessageProvidingListenerAdapter(
    IDoIpTransportMessageProvidingListener& transportMessageProvidingListener)
: _transportMessageProvidingListener(transportMessageProvidingListener)
{}

} // namespace doip

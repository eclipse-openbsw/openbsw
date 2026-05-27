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

#include "doip/common/DoIpResult.h"

#include <common/busid/BusId.h>
#include <transport/ITransportMessageProvidingListener.h>

#include <etl/span.h>

namespace doip
{
/**
 * Interface for a DoIp transport message providing listener. This interface is a DoIP extension
 * of the corresponding transport interface which allows to return also the DoIP specific
 * diagnostic NACK codes in case of errors.
 */
class IDoIpTransportMessageProvidingListener
{
public:
    using GetResult     = DoIpResult<::transport::ITransportMessageProvider::ErrorCode>;
    using ReceiveResult = DoIpResult<::transport::ITransportMessageListener::ReceiveResult>;

    /**
     * returns a TransportMessage for a given sourceBusId and targetId
     * \param sourceBusId       id of bus message is received from
     * \param sourceId          id of TransportMessage's source
     * \param targetId          id of TransportMessage's target
     * \param size              size of the requested TransportMessage
     * \param peek              span to the payload of the underlying data buffer
     * \param transportMessage  a pointer to TransportMessage (nullptr if no
     *          message was available) is written to this pointer.
     * \return
     *      result object holding both the transport layer error code and (if not TPMSG_OK) the
     *      corresponding nack code to report
     * \note
     * The TransportMessage must be released by a call to releaseTransportMessage
     * before it can be returned again by this method.
     */
    virtual GetResult getTransportMessage(
        uint8_t sourceBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        ::transport::TransportMessage*& transportMessage)
        = 0;

    /**
     * callback when message processing is done
     * \param transportMessage TransportMessage to which was received
     */
    virtual void releaseTransportMessage(::transport::TransportMessage& transportMessage) = 0;

    /**
     * callback when a TransportMessage has been received completely
     * \param sourceBusId            id of bus message is received from
     * \param transportMessage       TransportMessage that was received
     * \param notificationListener   listener that wants to be notified when
     * the transportMessage has been processed. May be NULL which implies that
     * no notification is required.
     * \return  the immediate result of receiving the message
     */
    virtual ReceiveResult messageReceived(
        uint8_t sourceBusId,
        ::transport::TransportMessage& transportMessage,
        ::transport::ITransportMessageProcessedListener* notificationListener)
        = 0;

protected:
    ~IDoIpTransportMessageProvidingListener() = default;
    IDoIpTransportMessageProvidingListener& operator=(IDoIpTransportMessageProvidingListener const&)
        = delete;
};

} // namespace doip

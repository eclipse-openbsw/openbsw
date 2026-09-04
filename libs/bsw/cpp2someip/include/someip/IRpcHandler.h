/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/SomeIpMessage.h"

#include <ip/IPEndpoint.h>

#include <cstdint>

namespace someip
{
class IEventReceiver;
class NetworkChannel;

class IRpcHandler
{
protected:
    IRpcHandler() = default;

public:
    enum class ErrorCode : uint8_t
    {
        RPC_HANDLER_OK,
        RPC_HANDLER_ERROR,
        RPC_HANDLER_UNKNOWN_SERVICE,
        RPC_HANDLER_UNKNOWN_METHOD,
        RPC_HANDLER_WRONG_INTERFACE_VERSION,
        RPC_HANDLER_MALFORMED_MESSAGE,
        RPC_HANDLER_WRONG_MESSAGE_TYPE,
        RPC_HANDLER_WRONG_MESSAGE_TYPE_REQUEST_RESPONSE,
        RPC_HANDLER_WRONG_MESSAGE_TYPE_FIRE_AND_FORGET,
        RPC_HANDLER_NOT_RESPONSIBLE
    };

    IRpcHandler(IRpcHandler const&)            = delete;
    IRpcHandler& operator=(IRpcHandler const&) = delete;

    virtual ~IRpcHandler() = default;

    /**
     * Pure virtual function that is responsible for handling RPC messages.
     */
    virtual IRpcHandler::ErrorCode
    handleMessage(NetworkChannel const& channel, SomeIpMessage const& message)
        = 0;

    /**
     * Pure virtual function that is responsible for handling requests.
     */
    virtual ErrorCode handleRequest(
        SomeIpMessage const& message,
        ::ip::IPEndpoint const& sourceAddress,
        uint16_t localPort,
        uint8_t proto)
        = 0;

    /**
     * Pure virtual function that is responsible for handling responses.
     */
    virtual ErrorCode handleResponse(
        SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress, uint16_t localPort)
        = 0;

    /**
     * Pure virtual function that is responsible for handling notifications.
     */
    virtual void handleNotification(
        SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress, uint16_t localPort)
        = 0;

    /**
     * Pure virtual function that is responsible for handling errors.
     */
    virtual ErrorCode
    handleError(SomeIpMessage const& message, ::ip::IPEndpoint const& sourceAddress)
        = 0;

    /**
     * Pure virtual function that sets EventReceiver.
     */
    virtual void setEventReceiver(IEventReceiver& eventReceiver) = 0;

    /**
     * Pure virtual function that removes EventReceiver.
     */
    virtual void removeEventReceiver() = 0;
};

} // namespace someip

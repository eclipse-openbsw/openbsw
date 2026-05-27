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

#include "doip/server/DoIpServerConnectionHandler.h"
#include "doip/server/DoIpServerTransportMessageHandler.h"

namespace doip
{
class DoIpServerConnectionHandlerCallback;
class DoIpServerTransportLayerParameters;
class IDoIpTransportMessageProvidingListener;

/**
 * Class that holds common configuration data for transport connections.
 */
class DoIpServerTransportConnectionConfig
{
public:
    /**
     * Constructor.
     * \param busId logical bus
     * \param logicalEntityAddress address of represented entity
     * \param context asynchronous execution context
     * \param messageProvidingListener reference to message providing listener interface
     * \param messageProcessedListener reference to message processed listener
     * \param sendJobBlockPool reference to block pool for send jobs
     * \param parameters reference to transport layer parameters
     */
    DoIpServerTransportConnectionConfig(
        uint8_t busId,
        uint16_t logicalEntityAddress,
        ::async::ContextType context,
        IDoIpTransportMessageProvidingListener& messageProvidingListener,
        ::transport::ITransportMessageProcessedListener& messageProcessedListener,
        DoIpServerTransportLayerParameters const& parameters);

    /**
     * Get the corresponding bus id.
     * \return bus id
     */
    uint8_t getBusId() const;

    /**
     * Get the address of the represented logical entity.
     * \return logical entity address
     */
    uint16_t getLogicalEntityAddress() const;

    /**
     * Get the asynchronous execution context.
     * \return asynchronous execution context
     */
    ::async::ContextType getContext() const;

    /**
     * Get the message provider and listener.
     * \return reference to message provider listener interface
     */
    IDoIpTransportMessageProvidingListener& getMessageProvidingListener() const;

    /**
     * Get a message processed listener.
     * \return reference to message listener interface
     */
    ::transport::ITransportMessageProcessedListener& getMessageProcessedListener() const;

    /**
     * Get the parameters of the transport layer.
     * \return reference to the parameters
     */
    DoIpServerTransportLayerParameters const& getParameters() const;

private:
    IDoIpTransportMessageProvidingListener& _messageProvidingListener;
    ::transport::ITransportMessageProcessedListener& _messageProcessedListener;
    DoIpServerTransportLayerParameters const& _parameters;
    uint16_t _logicalEntityAddress;
    uint8_t _busId;
    ::async::ContextType const _context;
};

/**
 * Inline implementations.
 */
inline DoIpServerTransportConnectionConfig::DoIpServerTransportConnectionConfig(
    uint8_t const busId,
    uint16_t const logicalEntityAddress,
    ::async::ContextType const context,
    IDoIpTransportMessageProvidingListener& messageProvidingListener,
    ::transport::ITransportMessageProcessedListener& messageProcessedListener,
    DoIpServerTransportLayerParameters const& parameters)
: _messageProvidingListener(messageProvidingListener)
, _messageProcessedListener(messageProcessedListener)
, _parameters(parameters)
, _logicalEntityAddress(logicalEntityAddress)
, _busId(busId)
, _context(context)
{}

inline uint8_t DoIpServerTransportConnectionConfig::getBusId() const { return _busId; }

inline uint16_t DoIpServerTransportConnectionConfig::getLogicalEntityAddress() const
{
    return _logicalEntityAddress;
}

inline ::async::ContextType DoIpServerTransportConnectionConfig::getContext() const
{
    return _context;
}

// listener is managed from the outside
inline IDoIpTransportMessageProvidingListener&
DoIpServerTransportConnectionConfig::getMessageProvidingListener() const
{
    return _messageProvidingListener;
}

// listener is managed from the outside
inline ::transport::ITransportMessageProcessedListener&
DoIpServerTransportConnectionConfig::getMessageProcessedListener() const
{
    return _messageProcessedListener;
}

inline DoIpServerTransportLayerParameters const&
DoIpServerTransportConnectionConfig::getParameters() const
{
    return _parameters;
}

} // namespace doip

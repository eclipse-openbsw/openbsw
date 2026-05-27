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

#include "doip/common/DoIpConstants.h"
#include "doip/server/DoIpServerVehicleIdentificationRequest.h"

#include <async/Types.h>
#include <ip/NetworkInterfaceConfig.h>

#include <etl/ipool.h>
#include <etl/span.h>

namespace doip
{
class IDoIpServerVehicleAnnouncementParameterProvider;
class IDoIpServerEntityStatusCallback;
class IDoIpServerVehicleIdentificationCallback;

/**
 * Implements a vehicle identification server on a single UDP socket.
 */
class DoIpServerVehicleIdentificationConfig
{
public:
    /**
     * Object pool type for pending requests to a vehicle identification socket.
     */
    using RequestPool = ::etl::ipool;

    /**
     * Constructor.
     * \param protocolVersion doip protocol version used for all communication
     * \param context asynchronous execution context
     * \param vehicleIdentificationCallback reference to interface that delivers vehicle
     * identification data
     * \param entityStatusCallback reference to interface that provides information about
     * entity status
     * \param serverConnectionInfo reference to
     * \param networkInterfaceConfigRegistry reference to registry for getting access to network
     * configuration data
     * \param logicalEntityAddress logical address of the represented entity (device)
     * \param parameterProvider reference to service parameters provider
     * \param requestPool request pool
     */
    DoIpServerVehicleIdentificationConfig(
        DoIpConstants::ProtocolVersion protocolVersion,
        ::async::ContextType context,
        IDoIpServerVehicleIdentificationCallback& vehicleIdentificationCallback,
        IDoIpServerEntityStatusCallback& entityStatusCallback,
        ::ip::NetworkInterfaceConfigRegistry& networkInterfaceConfigRegistry,
        uint16_t logicalEntityAddress,
        IDoIpServerVehicleAnnouncementParameterProvider const& parameterProvider,
        RequestPool& requestPool);

    /**
     * Get the protocol version.
     * \return protocol version
     */
    DoIpConstants::ProtocolVersion getProtocolVersion() const;

    /**
     * Get the asynchronous execution context.
     * \return asynchronous execution context
     */
    ::async::ContextType getContext() const;

    /**
     * Get the vehicle identification callback interface.
     * \return reference to callback interface
     */
    IDoIpServerVehicleIdentificationCallback& getVehicleIdentificationCallback() const;

    /**
     * Get the entity status callback interface.
     * \return reference to callback interface
     */
    IDoIpServerEntityStatusCallback& getEntityStatusCallback() const;

    /**
     * Get the vehicle identification callback interface.
     * \return reference to callback interface
     */
    ::ip::NetworkInterfaceConfigRegistry& getNetworkInterfaceConfigRegistry() const;

    /**
     * Get the address of the represented logical entity.
     * \return address
     */
    uint16_t getLogicalEntityAddress() const;

    /**
     * Get the parameters for the vehicle identification service.
     * \return reference to parameter provider
     */
    IDoIpServerVehicleAnnouncementParameterProvider const& getParameters() const;

    /**
     * Get the request pool.
     * \return reference to the request pool
     */
    RequestPool& getRequestPool() const;

    /**
     * Get write buffer.
     * \return reference to write buffer
     */
    ::etl::span<uint8_t> getWriteBuffer();

private:
    IDoIpServerVehicleIdentificationCallback& _vehicleIdentificationCallback;
    IDoIpServerEntityStatusCallback& _entityStatusCallback;
    ::ip::NetworkInterfaceConfigRegistry& _networkInterfaceConfigRegistry;
    IDoIpServerVehicleAnnouncementParameterProvider const& _parameterProvider;
    RequestPool& _requestPool;
    uint16_t _logicalEntityAddress;
    DoIpConstants::ProtocolVersion _protocolVersion;
    ::async::ContextType const _context;
    uint8_t _writeBuffer[40];
};

/**
 * Implementations.
 */
inline DoIpServerVehicleIdentificationConfig::DoIpServerVehicleIdentificationConfig(
    DoIpConstants::ProtocolVersion const protocolVersion,
    ::async::ContextType const context,
    IDoIpServerVehicleIdentificationCallback& vehicleIdentificationCallback,
    IDoIpServerEntityStatusCallback& entityStatusCallback,
    ::ip::NetworkInterfaceConfigRegistry& networkInterfaceConfigRegistry,
    uint16_t const logicalEntityAddress,
    IDoIpServerVehicleAnnouncementParameterProvider const& parameterProvider,
    RequestPool& requestPool)
: _vehicleIdentificationCallback(vehicleIdentificationCallback)
, _entityStatusCallback(entityStatusCallback)
, _networkInterfaceConfigRegistry(networkInterfaceConfigRegistry)
, _parameterProvider(parameterProvider)
, _requestPool(requestPool)
, _logicalEntityAddress(logicalEntityAddress)
, _protocolVersion(protocolVersion)
, _context(context)
, _writeBuffer()
{}

inline DoIpConstants::ProtocolVersion
DoIpServerVehicleIdentificationConfig::getProtocolVersion() const
{
    return _protocolVersion;
}

inline ::async::ContextType DoIpServerVehicleIdentificationConfig::getContext() const
{
    return _context;
}

// config is just a container
inline IDoIpServerVehicleIdentificationCallback&
DoIpServerVehicleIdentificationConfig::getVehicleIdentificationCallback() const
{
    return _vehicleIdentificationCallback;
}

// config is just a container
inline IDoIpServerEntityStatusCallback&
DoIpServerVehicleIdentificationConfig::getEntityStatusCallback() const
{
    return _entityStatusCallback;
}

// config is just a container
inline ::ip::NetworkInterfaceConfigRegistry&
DoIpServerVehicleIdentificationConfig::getNetworkInterfaceConfigRegistry() const
{
    return _networkInterfaceConfigRegistry;
}

inline uint16_t DoIpServerVehicleIdentificationConfig::getLogicalEntityAddress() const
{
    return _logicalEntityAddress;
}

inline IDoIpServerVehicleAnnouncementParameterProvider const&
DoIpServerVehicleIdentificationConfig::getParameters() const
{
    return _parameterProvider;
}

// config is just a container
inline DoIpServerVehicleIdentificationConfig::RequestPool&
DoIpServerVehicleIdentificationConfig::getRequestPool() const
{
    return _requestPool;
}

inline ::etl::span<uint8_t> DoIpServerVehicleIdentificationConfig::getWriteBuffer()
{
    return ::etl::span<uint8_t>(_writeBuffer);
}

} // namespace doip

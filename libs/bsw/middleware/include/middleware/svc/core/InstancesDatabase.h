/********************************************************************************
 * Copyright (c) 2025 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "IClusterConnection.h"
#include "middleware/svc/core/types.h"

#include <etl/span.h>

namespace middleware::svc::core
{

/**
 * Interface for instance database management.
 * This interface provides access to the database of service instances, including
 * both skeleton and proxy cluster connections, as well as the registered instance IDs.
 * Implementations of this interface maintain the mappings between service instances and
 * their corresponding cluster connections.
 */
class IInstanceDatabase
{
public:
    /** Returns the ID of the cluster this database belongs to (the source cluster of its
     * connections). */
    virtual uint8_t getClusterId() const = 0;

    /** Returns the span of cluster connections a skeleton of this cluster has to subscribe to. */
    virtual ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const = 0;

    /**
     * Returns the connection from this cluster to the cluster hosting \p instanceId, or nullptr
     * if the instance cannot be reached by a proxy of this cluster.
     */
    virtual IClusterConnection* getProxyConnection(uint16_t instanceId) const = 0;

    /** Returns the span of instance IDs provided by this cluster. */
    virtual ::etl::span<uint16_t const> getOwnInstanceIdsRange() const = 0;

    IInstanceDatabase& operator=(IInstanceDatabase const&) = delete;
    IInstanceDatabase& operator=(IInstanceDatabase&&)      = delete;

protected:
    ~IInstanceDatabase() = default;
};

} // namespace middleware::svc::core

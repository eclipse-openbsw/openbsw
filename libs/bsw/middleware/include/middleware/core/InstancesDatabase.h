// Copyright 2025 BMW AG

#pragma once

#include "IClusterConnection.h"
#include "middleware/core/types.h"

#include <etl/span.h>

namespace middleware
{
namespace core
{

/**
 * Interface for instance database management.
 * This interface provides access to the database of service instances, including
 * both skeleton and proxy cluster connections, as well as the registered instance IDs.
 * Implementations of this interface maintain the mappings between service instances and
 * their corresponding cluster connections.
 */
struct IInstanceDatabase
{
    /** Returns the span of skeleton cluster connections. */
    virtual etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const = 0;

    /** Returns the span of proxy cluster connections. */
    virtual etl::span<IClusterConnection* const> getProxyConnectionsRange() const = 0;

    /** Returns the span of registered instance IDs. */
    virtual etl::span<uint16_t const> getInstanceIdsRange() const = 0;

    IInstanceDatabase& operator=(IInstanceDatabase const&) = delete;
    IInstanceDatabase& operator=(IInstanceDatabase&&)      = delete;

protected:
    ~IInstanceDatabase() = default;
};

} // namespace core
} // namespace middleware

// Copyright 2025 BMW AG

#pragma once

#include <etl/span.h>

#include "IClusterConnection.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

/**
 * \brief Interface for instance database management.
 * \details This interface provides access to the database of service instances, including
 * both skeleton and proxy cluster connections, as well as the registered instance IDs.
 * Implementations of this interface maintain the mappings between service instances and
 * their corresponding cluster connections.
 */
struct IInstanceDatabase
{
    /**
     * \brief Get the range of skeleton cluster connections.
     * \details Returns a span containing all skeleton cluster connections registered in the
     * database. This allows iteration over all available skeleton connections.
     *
     * \return span of const pointers to IClusterConnection objects for skeletons
     */
    virtual etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const = 0;

    /**
     * \brief Get the range of proxy cluster connections.
     * \details Returns a span containing all proxy cluster connections registered in the
     * database. This allows iteration over all available proxy connections.
     *
     * \return span of const pointers to IClusterConnection objects for proxies
     */
    virtual etl::span<IClusterConnection* const> getProxyConnectionsRange() const = 0;

    /**
     * \brief Get the range of registered instance IDs.
     * \details Returns a span containing all service instance IDs registered in the database.
     * This provides access to all active service instances.
     *
     * \return span of const uint16_t instance IDs
     */
    virtual etl::span<uint16_t const> getInstanceIdsRange() const = 0;

    IInstanceDatabase& operator=(IInstanceDatabase const&) = delete;
    IInstanceDatabase& operator=(IInstanceDatabase&&)      = delete;
};

} // namespace core
} // namespace middleware

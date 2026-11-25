/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/array.h>
#include <etl/span.h>

#include "middleware/svc/core/IClusterConnection.h"
#include "middleware/svc/core/InstancesDatabase.h"
#include "middleware/svc/core/types.h"

using namespace middleware::svc::core;

class ClusterConnection : public IClusterConnection
{
public:
    uint8_t getSourceClusterId() const override { return static_cast<uint8_t>(1U); }

    uint8_t getTargetClusterId() const override { return static_cast<uint8_t>(2U); }

    HRESULT subscribe(ProxyBase&, uint16_t const) override { return HRESULT::Ok; }

    HRESULT subscribe(SkeletonBase&, uint16_t const) override { return HRESULT::Ok; }

    void unsubscribe(ProxyBase&, uint16_t const) override {}

    void unsubscribe(SkeletonBase&, uint16_t const) override {}

    HRESULT sendMessage(Message const&) const override { return HRESULT::Ok; }

    void processMessage(Message const&) const override {}

    size_t registeredTransceiversCount(uint16_t const) const override { return 1U; }

    HRESULT dispatchMessage(Message const&) const override { return HRESULT::Ok; }
};

class BadClusterConnection : public IClusterConnection
{
public:
    uint8_t getSourceClusterId() const override { return static_cast<uint8_t>(1U); }

    uint8_t getTargetClusterId() const override { return static_cast<uint8_t>(2U); }

    HRESULT subscribe(ProxyBase&, uint16_t const) override { return HRESULT::NotRegistered; }

    HRESULT subscribe(SkeletonBase&, uint16_t const) override { return HRESULT::NotRegistered; }

    void unsubscribe(ProxyBase&, uint16_t const) override {}

    void unsubscribe(SkeletonBase&, uint16_t const) override {}

    HRESULT sendMessage(Message const&) const override { return HRESULT::NotRegistered; }

    void processMessage(Message const&) const override {}

    size_t registeredTransceiversCount(uint16_t const) const override { return 0U; }

    HRESULT dispatchMessage(Message const&) const override
    {
        return HRESULT::CannotAllocatePayload;
    }
};

inline ClusterConnection clusterConnection;
inline BadClusterConnection badClusterConnection;

class InstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr InstancesDatabase() = default;

    uint8_t getClusterId() const override { return static_cast<uint8_t>(1U); }

    ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return ::etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    IClusterConnection* getProxyConnection(uint16_t const instanceId) const override
    {
        return (instanceId == _instanceIds[0U]) ? fProxyConnection : nullptr;
    }

    ::etl::span<::uint16_t const> getOwnInstanceIdsRange() const override
    {
        return ::etl::span<uint16_t const>(_instanceIds);
    }

private:
    ::etl::array<uint16_t const, 1U> const _instanceIds = {{1}};
    IClusterConnection* const fProxyConnection          = &clusterConnection;
    ::etl::array<IClusterConnection* const, 2U> const fSkeletonConnections
        = {{&clusterConnection, nullptr}};
};

class BadInstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr BadInstancesDatabase() = default;

    uint8_t getClusterId() const override { return static_cast<uint8_t>(1U); }

    ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return ::etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    IClusterConnection* getProxyConnection(uint16_t const instanceId) const override
    {
        return (instanceId == _instanceIds[0U]) ? fProxyConnection : nullptr;
    }

    ::etl::span<::uint16_t const> getOwnInstanceIdsRange() const override
    {
        return ::etl::span<uint16_t const>(_instanceIds);
    }

private:
    ::etl::array<uint16_t const, 1U> const _instanceIds = {{1}};
    IClusterConnection* const fProxyConnection          = &badClusterConnection;
    ::etl::array<IClusterConnection* const, 2U> const fSkeletonConnections
        = {{&badClusterConnection, nullptr}};
};

class EmptyInstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr EmptyInstancesDatabase() = default;

    uint8_t getClusterId() const override { return static_cast<uint8_t>(1U); }

    ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return ::etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    IClusterConnection* getProxyConnection(uint16_t const instanceId) const override
    {
        return (instanceId == _instanceIds[0U]) ? fProxyConnection : nullptr;
    }

    ::etl::span<::uint16_t const> getOwnInstanceIdsRange() const override
    {
        return ::etl::span<uint16_t const>(_instanceIds);
    }

private:
    ::etl::array<uint16_t const, 1U> const _instanceIds = {{1}};
    IClusterConnection* const fProxyConnection          = nullptr;
    ::etl::array<IClusterConnection* const, 0U> const fSkeletonConnections{};
};

constexpr InstancesDatabase instancesDatabase;
constexpr EmptyInstancesDatabase emptyInstancesDatabase;
constexpr BadInstancesDatabase badInstancesDatabase;

/** Connection with configurable source and target, used to tell connections apart. */
class RoutedClusterConnection : public ClusterConnection
{
public:
    RoutedClusterConnection(uint8_t const source, uint8_t const target)
    : _source(source), _target(target)
    {}

    uint8_t getSourceClusterId() const override { return _source; }

    uint8_t getTargetClusterId() const override { return _target; }

private:
    uint8_t _source;
    uint8_t _target;
};

struct ProxyRoute
{
    uint16_t instanceId;
    IClusterConnection* connection;
};

/** Database of a single cluster, as generated for a service deployed in several clusters. */
class ClusterInstancesDatabase : public ::IInstanceDatabase
{
public:
    ClusterInstancesDatabase(
        uint8_t const clusterId,
        ::etl::span<uint16_t const> const ownInstanceIds,
        ::etl::span<IClusterConnection* const> const skeletonConnections,
        ::etl::span<ProxyRoute const> const proxyRoutes)
    : _clusterId(clusterId)
    , _ownInstanceIds(ownInstanceIds)
    , _skeletonConnections(skeletonConnections)
    , _proxyRoutes(proxyRoutes)
    {}

    uint8_t getClusterId() const override { return _clusterId; }

    ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return _skeletonConnections;
    }

    IClusterConnection* getProxyConnection(uint16_t const instanceId) const override
    {
        for (auto const& route : _proxyRoutes)
        {
            if (route.instanceId == instanceId)
            {
                return route.connection;
            }
        }
        return nullptr;
    }

    ::etl::span<uint16_t const> getOwnInstanceIdsRange() const override { return _ownInstanceIds; }

private:
    uint8_t _clusterId;
    ::etl::span<uint16_t const> _ownInstanceIds;
    ::etl::span<IClusterConnection* const> _skeletonConnections;
    ::etl::span<ProxyRoute const> _proxyRoutes;
};

constexpr ::etl::array<::IInstanceDatabase const* const, 1U> INSTANCESDATABASE{&instancesDatabase};
constexpr ::etl::array<::IInstanceDatabase const* const, 1U> EMPTYINSTANCESDATABASE{
    &emptyInstancesDatabase};
constexpr ::etl::array<::IInstanceDatabase const* const, 1U> BADINSTANCESDATABASE{
    &badInstancesDatabase};

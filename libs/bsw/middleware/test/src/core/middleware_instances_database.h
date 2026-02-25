#pragma once

#include <etl/array.h>
#include <etl/span.h>

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/types.h"

using namespace middleware::core;

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

class InstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr InstancesDatabase() = default;

    etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    etl::span<IClusterConnection* const> getProxyConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fProxyConnections);
    }

    etl::span<::uint16_t const> getInstanceIdsRange() const override
    {
        return etl::span<uint16_t const>(instanceIds_);
    }

private:
    etl::array<uint16_t const, 1U> const instanceIds_ = {{1}};
    ClusterConnection clustConn_;
    etl::array<IClusterConnection* const, 2U> const fProxyConnections    = {{&clustConn_, nullptr}};
    etl::array<IClusterConnection* const, 2U> const fSkeletonConnections = {{&clustConn_, nullptr}};
};

class BadInstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr BadInstancesDatabase() = default;

    etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    etl::span<IClusterConnection* const> getProxyConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fProxyConnections);
    }

    etl::span<::uint16_t const> getInstanceIdsRange() const override
    {
        return etl::span<uint16_t const>(instanceIds_);
    }

private:
    etl::array<uint16_t const, 1U> const instanceIds_ = {{1}};
    BadClusterConnection clustConn_;
    etl::array<IClusterConnection* const, 2U> const fProxyConnections    = {{&clustConn_, nullptr}};
    etl::array<IClusterConnection* const, 2U> const fSkeletonConnections = {{&clustConn_, nullptr}};
};

class EmptyInstancesDatabase : public ::IInstanceDatabase
{
public:
    constexpr EmptyInstancesDatabase() = default;

    etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fSkeletonConnections);
    }

    etl::span<IClusterConnection* const> getProxyConnectionsRange() const override
    {
        return etl::span<IClusterConnection* const>(fProxyConnections);
    }

    etl::span<::uint16_t const> getInstanceIdsRange() const override
    {
        return etl::span<uint16_t const>(instanceIds_);
    }

private:
    etl::array<uint16_t const, 1U> const instanceIds_ = {{1}};
    ClusterConnection clustConn_;
    etl::array<IClusterConnection* const, 0U> const fProxyConnections{};
    etl::array<IClusterConnection* const, 0U> const fSkeletonConnections{};
};

constexpr InstancesDatabase _InstancesDatabase;
constexpr EmptyInstancesDatabase _EmptyInstancesDatabase;
constexpr BadInstancesDatabase _BadInstancesDatabase;

constexpr etl::array<::IInstanceDatabase const* const, 1U> INSTANCESDATABASE{&_InstancesDatabase};
constexpr etl::array<::IInstanceDatabase const* const, 1U> EMPTYINSTANCESDATABASE{
    &_EmptyInstancesDatabase};
constexpr etl::array<::IInstanceDatabase const* const, 1U> BADINSTANCESDATABASE{
    &_BadInstancesDatabase};

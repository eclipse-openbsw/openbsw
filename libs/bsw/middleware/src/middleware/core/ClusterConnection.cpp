// Copyright 2025 BMW AG

#include "middleware/core/ClusterConnection.h"

#include "middleware/core/IClusterConnectionConfigurationBase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{

ClusterConnectionNoTimeoutProxyOnly::ClusterConnectionNoTimeoutProxyOnly(
    IClusterConnectionConfigurationProxyOnly& configuration)
: Base(configuration)
{}

HRESULT
ClusterConnectionNoTimeoutProxyOnly::subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationProxyOnly&>(Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnectionNoTimeoutProxyOnly::unsubscribe(ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationProxyOnly&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

ClusterConnectionNoTimeoutSkeletonOnly::ClusterConnectionNoTimeoutSkeletonOnly(
    IClusterConnectionConfigurationSkeletonOnly& configuration)
: Base(configuration)
{}

HRESULT ClusterConnectionNoTimeoutSkeletonOnly::subscribe(
    SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationSkeletonOnly&>(Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionNoTimeoutSkeletonOnly::unsubscribe(
    SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationSkeletonOnly&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

ClusterConnectionNoTimeoutBidirectional::ClusterConnectionNoTimeoutBidirectional(
    IClusterConnectionConfigurationBidirectional& configuration)
: Base(configuration)
{}

HRESULT ClusterConnectionNoTimeoutBidirectional::subscribe(
    ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

HRESULT ClusterConnectionNoTimeoutBidirectional::subscribe(
    SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionNoTimeoutBidirectional::unsubscribe(
    ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

void ClusterConnectionNoTimeoutBidirectional::unsubscribe(
    SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectional&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

ClusterConnectionBidirectionalWithTimeout::ClusterConnectionBidirectionalWithTimeout(
    IClusterConnectionConfigurationBidirectionalWithTimeout& configuration)
: Base(configuration)
{}

HRESULT ClusterConnectionBidirectionalWithTimeout::subscribe(
    ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectionalWithTimeout&>(
               Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnectionBidirectionalWithTimeout::unsubscribe(
    ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectionalWithTimeout&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

HRESULT ClusterConnectionBidirectionalWithTimeout::subscribe(
    SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationBidirectionalWithTimeout&>(
               Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionBidirectionalWithTimeout::unsubscribe(
    SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationBidirectionalWithTimeout&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

ClusterConnectionProxyOnlyWithTimeout::ClusterConnectionProxyOnlyWithTimeout(
    IClusterConnectionConfigurationProxyOnlyWithTimeout& configuration)
: Base(configuration)
{}

HRESULT
ClusterConnectionProxyOnlyWithTimeout::subscribe(ProxyBase& proxy, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationProxyOnlyWithTimeout&>(
               Base::getConfiguration())
        .subscribe(proxy, serviceInstanceId);
}

void ClusterConnectionProxyOnlyWithTimeout::unsubscribe(ProxyBase& proxy, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationProxyOnlyWithTimeout&>(Base::getConfiguration())
        .unsubscribe(proxy, serviceId);
}

ClusterConnectionSkeletonOnlyWithTimeout::ClusterConnectionSkeletonOnlyWithTimeout(
    IClusterConnectionConfigurationSkeletonOnlyWithTimeout& configuration)
: Base(configuration)
{}

HRESULT ClusterConnectionSkeletonOnlyWithTimeout::subscribe(
    SkeletonBase& skeleton, uint16_t const serviceInstanceId)
{
    return static_cast<IClusterConnectionConfigurationSkeletonOnlyWithTimeout&>(
               Base::getConfiguration())
        .subscribe(skeleton, serviceInstanceId);
}

void ClusterConnectionSkeletonOnlyWithTimeout::unsubscribe(
    SkeletonBase& skeleton, uint16_t const serviceId)
{
    static_cast<IClusterConnectionConfigurationSkeletonOnlyWithTimeout&>(Base::getConfiguration())
        .unsubscribe(skeleton, serviceId);
}

} // namespace core
} // namespace middleware

/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/UdpConfig.h"

#include "someip/logger.h"

#include <etl/span.h>

// Logger API uses printf-style varargs for fixed diagnostic messages in this module.
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace someip
{
using ::util::logger::SOMEIP;

UdpProxyConfig::UdpProxyConfig(
    ::etl::span<UdpProxy> proxies,
    ::etl::ivector<UdpProxy*>& serverProxies,
    ::etl::ivector<uint16_t>& ports)
: _proxies(proxies), _serverProxies(serverProxies), _ports(ports)
{}

void UdpProxyConfig::setListener(INetworkListener& listener) const
{
    for (UdpProxy& proxy : _proxies)
    {
        proxy.setListener(listener);
    }
}

size_t UdpProxyConfig::getSize() const { return _proxies.size(); }

UdpProxy* UdpProxyConfig::getProxy(size_t const pos) const { return &(_proxies[pos]); }

UdpProxy* UdpProxyConfig::nextProxy() const
{
    for (UdpProxy& proxy : _proxies)
    {
        if (!proxy.isOpen())
        {
            return &proxy;
        }
    }

    return nullptr;
}

UdpProxy* UdpProxyConfig::getOpenProxy(uint16_t const localPort) const
{
    for (UdpProxy& proxy : _proxies)
    {
        if (proxy.isOpen())
        {
            auto const portResult = proxy.getLocalPort();
            if (portResult.has_value() && (portResult.value() == localPort))
            {
                return &proxy;
            }
        }
    }

    return nullptr;
}

bool UdpProxyConfig::initPort(uint16_t const port) const
{
    if (!_ports.full())
    {
        _ports.push_back(port);
        return true;
    }

    return false;
}

::etl::expected<uint16_t, PortError> UdpProxyConfig::getPort(size_t const pos) const
{
    if (_ports.size() > pos)
    {
        uint16_t const port = _ports[pos];
        if (port == port::INVALID)
        {
            return ::etl::unexpected<PortError>(PortError::NOT_INITIALIZED);
        }
        return port;
    }
    return ::etl::unexpected<PortError>(PortError::OUT_OF_RANGE);
}

void UdpProxyConfig::addToServers(UdpProxy& proxy)
{
    if (!_serverProxies.full())
    {
        proxy.incRefCounter();
        _serverProxies.push_back(&proxy);
    }
}

void UdpProxyConfig::shutdownPort(uint16_t const port)
{
    UdpProxy* const proxy = getOpenProxy(port);
    if (nullptr != proxy)
    {
        releaseServer(*proxy);
        proxy->close();
    }
}

void UdpProxyConfig::releaseServer(UdpProxy& proxy)
{
    etl::ivector<someip::UdpProxy*, void>::iterator it
        = etl::find(_serverProxies.begin(), _serverProxies.end(), &proxy);
    if (it != _serverProxies.end())
    {
        proxy.decRefCounter();
        (void)_serverProxies.erase(it);
    }
}

void UdpProxyConfig::close()
{
    for (size_t i = 0U; i < _proxies.size(); ++i)
    {
        UdpProxy& proxy = _proxies[i];
        releaseServer(proxy);
        if (proxy.isOpen())
        {
            INFO_LOG(SOMEIP, "Network: close proxy[%d]", i);
            proxy.close();
        }
    }
}

} // namespace someip

// NOLINTEND(cppcoreguidelines-pro-type-vararg)

/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/NetworkConfig.h"

namespace someip
{
NetworkConfig::NetworkConfig(
    ::ip::IPAddress const& multicastIp,
    ::ip::IPAddress const& localIp,
    uint8_t const subnetId,
    UdpConfig& udpConfig,
    TcpConfig& tcpConfig,
    TpConfig& tpConfig)
: _multicastIp(multicastIp)
, _localIp(localIp)
, _udpConfig(udpConfig)
, _tcpConfig(tcpConfig)
, _tpConfig(tpConfig)
, _subnetId(subnetId)
, _useMagicCookie(false)
{}

} // namespace someip

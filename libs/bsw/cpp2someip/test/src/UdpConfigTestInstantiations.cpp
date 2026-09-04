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

#include <udp/socket/AbstractDatagramSocketMock.h>

// Explicit template instantiations for types used in tests
namespace someip
{
namespace internal
{
template class UdpProxyResources<::udp::AbstractDatagramSocketMock, 0>;
template class UdpProxyResources<::udp::AbstractDatagramSocketMock, 1>;
} // namespace internal
} // namespace someip

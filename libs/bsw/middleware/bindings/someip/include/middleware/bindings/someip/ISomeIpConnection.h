/********************************************************************************
 * Copyright (c) 2026 KPIT Technologies Ltd.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "middleware/core/IClusterConnectionConfigurationBase.h"

namespace middleware::bindings::someip
{

namespace internal
{
class IServiceEndpoint;
} // namespace internal

/**
 * The configuration of a middleware cluster connection whose services travel over
 * SOME/IP. The endpoint of each service adds itself when it is constructed.
 */
class ISomeIpConnection : public ::middleware::core::IClusterConnectionConfiguration
{
public:
    /** Adds the endpoint of one service. Called by the endpoint's constructor. */
    virtual void add(internal::IServiceEndpoint& endpoint) = 0;
};

} // namespace middleware::bindings::someip

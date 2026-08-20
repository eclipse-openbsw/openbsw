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

#include "features/communication/dummy_serviceCommon.h"
#include "features/communication/dummy_serviceProxy.h"
#include "features/communication/dummy_serviceSkeleton.h"

namespace middleware::docs::user_examples
{
using ClusterId   = ::middleware::core::ClusterId;
using HRESULT     = ::middleware::core::HRESULT;
using FutureState = ::middleware::core::Future::State;
} // namespace middleware::docs::user_examples

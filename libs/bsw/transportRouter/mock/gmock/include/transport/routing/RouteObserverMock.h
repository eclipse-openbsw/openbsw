/********************************************************************************
 * Copyright (c) 2026 Jefferson Nascimento
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// AI disclosure: this file was largely generated with an AI assistant and was reviewed and
// tested by the contributor. Assisted-by: Anthropic Claude Opus 5.5

#pragma once

#include "transport/routing/IRouteObserver.h"

#include <gmock/gmock.h>

namespace transport
{
class RouteObserverMock : public IRouteObserver
{
public:
    MOCK_METHOD(void, routeResponded, (size_t routeIndex), (override));
    MOCK_METHOD(void, routeTimedOut, (size_t routeIndex), (override));
};

} // namespace transport

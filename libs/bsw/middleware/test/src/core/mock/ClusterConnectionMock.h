/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace test
{

class ClusterConnectionMock : public IClusterConnection
{
public:
    MOCK_METHOD(uint8_t, getSourceClusterId, (), (const, override));
    MOCK_METHOD(uint8_t, getTargetClusterId, (), (const, override));
    MOCK_METHOD(HRESULT, subscribe, (ProxyBase&, uint16_t const), (override));
    MOCK_METHOD(HRESULT, subscribe, (SkeletonBase&, uint16_t const), (override));
    MOCK_METHOD(void, unsubscribe, (ProxyBase&, uint16_t const), (override));
    MOCK_METHOD(void, unsubscribe, (SkeletonBase&, uint16_t const), (override));
    MOCK_METHOD(HRESULT, sendMessage, (Message const&), (const, override));
    MOCK_METHOD(void, processMessage, (Message const&), (const, override));
    MOCK_METHOD(size_t, registeredTransceiversCount, (uint16_t const), (const, override));
    MOCK_METHOD(HRESULT, dispatchMessage, (Message const&), (const, override));
};

} // namespace test
} // namespace core
} // namespace middleware

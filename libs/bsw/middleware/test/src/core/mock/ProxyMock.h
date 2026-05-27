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

#include "ClusterConnectionMock.h"
#include "middleware/core/ProxyBase.h"
#include "middleware/core/types.h"

namespace middleware
{
namespace core
{
namespace test
{

class ProxyMock : public ProxyBase
{
public:
    ProxyMock() : ProxyBase() { setConnection(&_dummyConnection); }

    MOCK_METHOD(bool, isInitialized, (), (const, override));
    MOCK_METHOD(HRESULT, sendMessage, (Message&), (const, override));
    MOCK_METHOD(HRESULT, onNewMessageReceived, (Message const&), (override));
    MOCK_METHOD(uint16_t, getServiceId, (), (const, override));

    void setConnection(IClusterConnection* cc) { _connection = cc; }

    ::testing::NiceMock<ClusterConnectionMock> _dummyConnection{};

    template<typename EventAttribute>
    void setEvent(EventAttribute const& att, Message const& msg)
    {
        att.setEvent_(msg);
    }
};

} // namespace test
} // namespace core
} // namespace middleware

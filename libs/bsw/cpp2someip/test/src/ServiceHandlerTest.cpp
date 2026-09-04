/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/ServiceHandler.h"

#include "someip/CallDoneClosureMock.h"
#include "someip/SomeIpSerializableMock.h"

#include <etl/pool.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

using namespace ::someip;

struct ServiceHandlerTest
: ::testing::Test
, ServiceHandler
{
    ServiceHandlerTest() : ServiceHandler(_pool) {}

    ::someip::MethodDetail const* getMethodDetail(uint16_t /* methodId */) const override
    {
        return nullptr;
    }

    void callMethod(
        uint16_t,
        ::someip::ISomeIpSerializable const*,
        ::someip::ISomeIpSerializable*,
        CallDoneClosure&) override
    {}

    ::someip::ISomeIpSerializable* getResponse(uint16_t) override { return nullptr; }

    ::someip::ISomeIpSerializable* getRequest(uint16_t) override { return nullptr; }

    bool notifyInitialEvents(
        uint16_t const /* serviceId */,
        uint16_t const /* instanceId */,
        uint8_t const /* majorVersion */,
        uint16_t const /* eventGroup */,
        ::ip::IPAddress const& /* ipAddress */,
        uint16_t const /* port */,
        uint8_t const /* proto */) override
    {
        return true;
    }

    static uint8_t const NUMBER_OF_CALLBACKS = 5U;

private:
    ::etl::pool<ServiceHandler::RpcCallback, NUMBER_OF_CALLBACKS> _pool;
};

TEST_F(ServiceHandlerTest, IsSerializableValid)
{
    StrictMock<CallDoneClosureMock> callDone;

    EXPECT_CALL(callDone, invoke(someip::RPC_INVALID_PAYLOAD)).Times(1);
    EXPECT_FALSE(isSerializableValid(nullptr, callDone));

    StrictMock<SomeIpSerializableMock> serializable;
    EXPECT_TRUE(isSerializableValid(&serializable, callDone));
}

/**
 * Make sure isRequestResponseValid() does not return true if request or response is nullptr.
 */
TEST_F(ServiceHandlerTest, test_isRequestResponseValid)
{
    StrictMock<SomeIpSerializableMock> request, response;

    StrictMock<CallDoneClosureMock> callDone;

    EXPECT_CALL(callDone, invoke(::someip::RPC_INVALID_PAYLOAD)).Times(3);

    EXPECT_FALSE(isRequestResponseValid(nullptr, nullptr, callDone));
    EXPECT_FALSE(isRequestResponseValid(&request, nullptr, callDone));
    EXPECT_FALSE(isRequestResponseValid(nullptr, &response, callDone));
    EXPECT_TRUE(isRequestResponseValid(&request, &response, callDone));
}

TEST_F(ServiceHandlerTest, ReleaseCallback)
{
    ServiceHandler::RpcCallback* cbList[NUMBER_OF_CALLBACKS];
    for (size_t i = 0U; i < NUMBER_OF_CALLBACKS; i++)
    {
        cbList[i] = &getCallback();
    }
    EXPECT_FALSE(hasAvailableCallback());

    releaseCallback(*cbList[NUMBER_OF_CALLBACKS - 1U]);
    EXPECT_TRUE(hasAvailableCallback());
}

} // anonymous namespace

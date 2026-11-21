/********************************************************************************
 * Copyright (c) 2025, 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <cstddef>
#include <cstdint>

#include <etl/array.h>
#include <etl/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "middleware/core/IClusterConnection.h"
#include "middleware/core/InstancesDatabase.h"
#include "middleware/core/Message.h"
#include "middleware/core/MessagePayloadBuilder.h"
#include "middleware/core/SkeletonAttribute.h"
#include "middleware/core/SkeletonBase.h"
#include "middleware/core/types.h"

namespace middleware::core::test
{

namespace internal
{
constexpr uint16_t service_id = 0x10U;
constexpr uint16_t event_id   = 0U;
constexpr uint16_t instanceId = 1U;
} // namespace internal

using ArgType = uint8_t;

class SkeletonAttributeClusterConnectionMock : public IClusterConnection
{
public:
    static constexpr uint8_t SOURCE_CLUSTER_ID = 1U;

    uint8_t getSourceClusterId() const override { return SOURCE_CLUSTER_ID; }

    uint8_t getTargetClusterId() const override { return 0U; }

    HRESULT subscribe(ProxyBase& /* proxyBase */, uint16_t /* instanceId */) override
    {
        return HRESULT::NotImplemented;
    }

    MOCK_METHOD(HRESULT, subscribe, (SkeletonBase&, uint16_t), (override));

    void unsubscribe(ProxyBase& /* proxyBase */, uint16_t /* instanceId */) override {}

    void unsubscribe(SkeletonBase& /* skeletonBase */, uint16_t /* instanceId */) override {}

    MOCK_METHOD(HRESULT, sendMessage, (Message const&), (const, override));

    void processMessage(Message const& /* message */) const override {}

    size_t registeredTransceiversCount(uint16_t /* instanceId */) const override { return 0U; }

    HRESULT dispatchMessage(Message const& /* message */) const override
    {
        return HRESULT::NotImplemented;
    }
};

class InstanceDatabase : public IInstanceDatabase
{
public:
    explicit InstanceDatabase(IClusterConnection& connection) : _connections{{&connection}} {}

    ::etl::span<IClusterConnection* const> getSkeletonConnectionsRange() const override
    {
        return _connections;
    }

    ::etl::span<IClusterConnection* const> getProxyConnectionsRange() const override { return {}; }

    ::etl::span<uint16_t const> getInstanceIdsRange() const override { return _instanceIds; }

private:
    ::etl::array<IClusterConnection*, 1U> const _connections;
    ::etl::array<uint16_t, 1U> const _instanceIds{{1U}};
};

class DerivedSkeleton : public ::middleware::core::SkeletonBase
{
public:
    [[nodiscard]] uint16_t getServiceId() const override { return internal::service_id; }

    HRESULT onNewMessageReceived(Message const&) override { return HRESULT::NotImplemented; }

    [[nodiscard]] uint32_t getProcessId() const override { return 0U; }

    DerivedSkeleton() : SkeletonBase(), attribute(*this) {}

    HRESULT init(uint16_t instanceId, ::etl::span<IInstanceDatabase const* const> const& database)
    {
        return initFromInstancesDatabase(instanceId, database);
    }

    SkeletonAttribute<ArgType, internal::event_id, true> attribute;
};

class SkeletonAttributeTest : public ::testing::Test
{
public:
    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(SkeletonAttributeTest, SendPreservesCurrentValueOnFailure)
{
    DerivedSkeleton skeleton;
    ArgType const payload = 0xFFU;

    skeleton.attribute.set(payload);
    EXPECT_EQ(skeleton.attribute.send(), HRESULT::NotRegistered);
    EXPECT_EQ(skeleton.attribute.get(), payload);
}

TEST_F(SkeletonAttributeTest, SendBroadcastsCurrentValueSuccessfully)
{
    ::testing::StrictMock<SkeletonAttributeClusterConnectionMock> connection;
    InstanceDatabase const database(connection);
    ::etl::array<IInstanceDatabase const*, 1U> const databases{{&database}};
    DerivedSkeleton skeleton;
    ArgType const payload = 0xFFU;

    EXPECT_CALL(connection, subscribe(::testing::Ref(skeleton), internal::instanceId))
        .WillOnce(::testing::Return(HRESULT::Ok));
    ASSERT_EQ(skeleton.init(internal::instanceId, databases), HRESULT::Ok);
    ASSERT_TRUE(skeleton.isInitialized());

    EXPECT_CALL(connection, sendMessage(::testing::_))
        .WillOnce(
            [&](Message const& msg)
            {
                EXPECT_TRUE(msg.isEvent());
                EXPECT_EQ(msg.getHeader().serviceId, internal::service_id);
                EXPECT_EQ(msg.getHeader().memberId, internal::event_id);
                EXPECT_EQ(
                    msg.getHeader().srcClusterId,
                    SkeletonAttributeClusterConnectionMock::SOURCE_CLUSTER_ID);
                EXPECT_EQ(MessagePayloadBuilder::readPayload<ArgType>(msg), payload);
                return HRESULT::Ok;
            });

    skeleton.attribute.set(payload);
    EXPECT_EQ(skeleton.attribute.send(), HRESULT::Ok);
    EXPECT_EQ(skeleton.attribute.get(), payload);
}

} // namespace middleware::core::test

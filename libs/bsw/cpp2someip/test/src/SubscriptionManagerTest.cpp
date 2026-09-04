/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SubscriptionManager.h"

#include "someip/SomeIpConstants.h"
#include "someip/SubscribedEventGroup.h"

#include <ip/IPAddress.h>

#include <cstdint>

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::ip;

struct SubscriptionManagerTest : ::testing::Test
{
    static constexpr uint16_t NUM_SUBSCRIPTIONS = 16U;

    declare::SubscriptionManager<NUM_SUBSCRIPTIONS> _subscriptionManager;
};

uint16_t const SubscriptionManagerTest::NUM_SUBSCRIPTIONS;

TEST_F(SubscriptionManagerTest, testAddForOneEventgroup)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress1              = 0xC0000201;
    uint32_t ipAddress2              = 0xC0000202;
    uint16_t port                    = 0x8899;

    IPEndpoint endpoint1(make_ip4(ipAddress1), port);
    IPEndpoint endpoint2(make_ip4(ipAddress2), port);

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints);
    EXPECT_EQ(2U, endpoints->size());

    // Note: etl::intrusive_forward_list::push_front() reverses insertion order
    // endpoint2 was added last, so it appears first
    SubscriptionEndpointList::iterator itr = endpoints->begin();
    EXPECT_EQ(ttl, itr->ttl);
    EXPECT_EQ(endpoint2, *itr);
    ++itr;
    EXPECT_EQ(ttl, itr->ttl);
    EXPECT_EQ(endpoint1, *itr);
}

/**
 * Test number of subscriptions of SubscriptionManager. Make sure number is increased by adding a
 * subscription.
 */
TEST_F(SubscriptionManagerTest, test_number_of_subscriptions)
{
    EXPECT_EQ(0U, _subscriptionManager.getCurrentNumberOfSubscriptions());
    EXPECT_EQ(NUM_SUBSCRIPTIONS, _subscriptionManager.getMaximumNumberOfSubscriptions());

    uint16_t const serviceId    = 0x1234;
    uint8_t const majorVersion  = 0x01;
    uint16_t const instanceId   = 0x01;
    uint16_t const eventgroupId = 0x01;
    uint32_t const ttl          = 0x03;
    uint32_t const ipAddress    = 0xC0000201;
    uint16_t const port         = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    EXPECT_EQ(1U, _subscriptionManager.getCurrentNumberOfSubscriptions());
    EXPECT_EQ(NUM_SUBSCRIPTIONS, _subscriptionManager.getMaximumNumberOfSubscriptions());
}

/**
 * Make sure SubscriptionManager detects on calling addSubscription() if a subscription is already
 * added.
 */
TEST_F(SubscriptionManagerTest, test_subscription_already_added)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000201;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_ALREADY_SUBSCRIBED, result);
}

TEST_F(SubscriptionManagerTest, testAddManyForOneEventgroupMultipleTimes)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroupId = 0x01;
    uint32_t ipAddress               = 0xC0000201;
    ttl::type ttl                    = 0x03;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    for (size_t i = 0U; i < NUM_SUBSCRIPTIONS; ++i)
    {
        result = _subscriptionManager.addSubscription(
            serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress + i), port);
        EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
    }

    SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints);
    EXPECT_TRUE(NUM_SUBSCRIPTIONS == endpoints->size());

    // Note: etl::intrusive_forward_list::push_front() reverses insertion order
    // Items added as ipAddress+0, ipAddress+1, ... ipAddress+(NUM_SUBSCRIPTIONS-1)
    // But appear in reverse: ipAddress+(NUM_SUBSCRIPTIONS-1), ... ipAddress+1, ipAddress+0
    size_t i = 0U;
    for (SubscriptionEndpointList::iterator itr = endpoints->begin(); itr != endpoints->end();
         ++itr)
    {
        IPEndpoint endpoint(make_ip4(ipAddress + (NUM_SUBSCRIPTIONS - 1U - i)), port);
        EXPECT_EQ(ttl, itr->ttl);
        EXPECT_EQ(endpoint, *itr);
        ++i;
    }

    // this subscription is one too many
    result = _subscriptionManager.addSubscription(
        serviceId,
        majorVersion,
        instanceId,
        eventgroupId,
        ttl,
        make_ip4(ipAddress + NUM_SUBSCRIPTIONS),
        port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR, result);

    // now we let all subscriptions expire
    _subscriptionManager.updateTTLs(ttl + 1U);
    SubscriptionEndpointList* endpoints2 = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_FALSE(endpoints2);

    // ... and add new subscriptions again
    for (size_t i = 0U; i < NUM_SUBSCRIPTIONS; ++i)
    {
        result = _subscriptionManager.addSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroupId,
            ttl,
            make_ip4(ipAddress + NUM_SUBSCRIPTIONS + i),
            port);
        EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
    }

    SubscriptionEndpointList* endpoints3 = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints3);
    EXPECT_TRUE(NUM_SUBSCRIPTIONS == endpoints3->size());

    // Note: etl::intrusive_forward_list::push_front() reverses insertion order
    // Items added as ipAddress+NUM_SUBSCRIPTIONS+0, ...
    // ipAddress+NUM_SUBSCRIPTIONS+(NUM_SUBSCRIPTIONS-1) But appear in reverse order
    i = 0U;
    for (SubscriptionEndpointList::iterator itr = endpoints3->begin(); itr != endpoints3->end();
         ++itr)
    {
        IPEndpoint endpoint(
            make_ip4(ipAddress + NUM_SUBSCRIPTIONS + (NUM_SUBSCRIPTIONS - 1U - i)), port);
        EXPECT_EQ(ttl, itr->ttl);
        EXPECT_EQ(endpoint, *itr);
        ++i;
    }
}

TEST_F(SubscriptionManagerTest, testAddForMultipleEventgroups)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    uint16_t instanceId1             = 0x01;
    uint16_t instanceId2             = 0x02;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress1              = 0xC0000201;
    uint32_t ipAddress2              = 0xC0000202;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup1(serviceId, majorVersion, instanceId1, eventgroupId);
    SubscriptionEndpointList* endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_TRUE(endpoints1);
    EXPECT_EQ(1U, endpoints1->size());

    SubscribedEventGroup eventgroup2(serviceId, majorVersion, instanceId2, eventgroupId);
    SubscriptionEndpointList* endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_TRUE(endpoints2);
    EXPECT_EQ(1U, endpoints2->size());

    SubscriptionEndpointList::iterator itr = endpoints1->begin();
    EXPECT_EQ(ttl, itr->ttl);
    EXPECT_EQ(IPEndpoint(make_ip4(ipAddress1), port), *itr);

    itr = endpoints2->begin();
    EXPECT_EQ(ttl, itr->ttl);
    EXPECT_EQ(IPEndpoint(make_ip4(ipAddress2), port), *itr);
}

TEST_F(SubscriptionManagerTest, testAddForTooManySubscriptions)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x00;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000200;
    uint16_t port                    = 0x8800;

    SubscriptionManager::InternalSubscribeResult result;

    for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
    {
        result = _subscriptionManager.addSubscription(
            serviceId,
            majorVersion,
            instanceId + i,
            eventgroupId,
            ttl,
            make_ip4(ipAddress + i),
            port + i);
        EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
    }

    for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
    {
        SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId + i, eventgroupId);
        SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
        EXPECT_TRUE(endpoints);
        EXPECT_EQ(1U, endpoints->size());
    }

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR, result);
}

TEST_F(SubscriptionManagerTest, testAddForTooManySubscriptions2)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x00;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000200;
    uint16_t port                    = 0x8800;

    SubscriptionManager::InternalSubscribeResult result;

    for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
    {
        result = _subscriptionManager.addSubscription(
            serviceId,
            majorVersion,
            instanceId,
            eventgroupId,
            ttl,
            make_ip4(ipAddress + i),
            port + i);
        EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
    }

    SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints);
    EXPECT_EQ(NUM_SUBSCRIPTIONS, endpoints->size());

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_ERROR, result);
}

/**
 * Test removing subscription from SubscriptionManager.
 */
TEST_F(SubscriptionManagerTest, test_removeSubscription)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress1              = 0xC0000201;
    uint32_t ipAddress2              = 0xC0000202;
    uint16_t port                    = 0x8899;

    _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress1), port);
    _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress2), port);

    SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints);
    EXPECT_EQ(2U, endpoints->size());

    _subscriptionManager.removeSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, make_ip4(ipAddress1), port);
    EXPECT_EQ(1U, endpoints->size());

    SubscriptionEndpointList::iterator itr = endpoints->begin();
    EXPECT_EQ(ttl, itr->ttl);
    EXPECT_EQ(IPEndpoint(make_ip4(ipAddress2), port), *itr);

    _subscriptionManager.removeSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, make_ip4(ipAddress2), port);

    EXPECT_FALSE(_subscriptionManager.getSubscriptions(eventgroup));
}

TEST_F(SubscriptionManagerTest, testRemoveSubscriptionsForEventgroup)
{
    uint16_t const serviceId    = 0x1234;
    uint8_t const majorVersion  = 0x01;
    uint16_t const instanceId1  = 0x01;
    uint16_t const instanceId2  = 0x02;
    uint16_t const eventgroupId = 0x01;
    uint32_t const ttl          = 0x03;
    uint32_t const ipAddress1   = 0xC0000201;
    uint32_t const ipAddress2   = 0xC0000202;
    uint16_t const port         = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    // instanceId1, ipAddress1
    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    // instanceId1, ipAddress2
    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    // instanceId2, ipAddress1
    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    // instanceId2, ipAddress2
    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup1(serviceId, majorVersion, instanceId1, eventgroupId);
    SubscriptionEndpointList* endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_TRUE(endpoints1);
    EXPECT_EQ(2U, endpoints1->size());

    SubscribedEventGroup eventgroup2(serviceId, majorVersion, instanceId2, eventgroupId);
    SubscriptionEndpointList* endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_TRUE(endpoints2);
    EXPECT_EQ(2U, endpoints2->size());

    // remove subscriptions for eventgroup1
    _subscriptionManager.removeSubscriptions(serviceId, majorVersion, instanceId1, eventgroupId);

    endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_FALSE(endpoints1);
    endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_TRUE(endpoints2);
    EXPECT_EQ(2U, endpoints2->size());
}

TEST_F(SubscriptionManagerTest, testRemoveSubscriptions)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    uint16_t instanceId1             = 0x01;
    uint16_t instanceId2             = 0x02;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000201;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup1(serviceId, majorVersion, instanceId1, eventgroupId);
    SubscriptionEndpointList* endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_TRUE(endpoints1);
    EXPECT_EQ(1U, endpoints1->size());

    SubscribedEventGroup eventgroup2(serviceId, majorVersion, instanceId2, eventgroupId);
    SubscriptionEndpointList* endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_TRUE(endpoints2);
    EXPECT_EQ(1U, endpoints2->size());

    _subscriptionManager.removeSubscriptions(make_ip4(ipAddress));

    endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_FALSE(endpoints1);
    endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_FALSE(endpoints2);
}

TEST_F(SubscriptionManagerTest, testRemoveSubscriptionsMixedIPs)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    uint16_t instanceId1             = 0x01;
    uint16_t instanceId2             = 0x02;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress1              = 0xC0000201;
    uint32_t ipAddress2              = 0xC0000202;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId1, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress1), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId2, eventgroupId, ttl, make_ip4(ipAddress2), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup1(serviceId, majorVersion, instanceId1, eventgroupId);
    SubscriptionEndpointList* endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_TRUE(endpoints1);
    EXPECT_EQ(2U, endpoints1->size());

    SubscribedEventGroup eventgroup2(serviceId, majorVersion, instanceId2, eventgroupId);
    SubscriptionEndpointList* endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_TRUE(endpoints2);
    EXPECT_EQ(2U, endpoints2->size());

    _subscriptionManager.removeSubscriptions(make_ip4(ipAddress1));

    endpoints1 = _subscriptionManager.getSubscriptions(eventgroup1);
    EXPECT_EQ(1U, endpoints1->size());
    SubscriptionEndpointList::iterator itr1 = endpoints1->begin();
    EXPECT_EQ(ipAddress2, ip4_to_u32(itr1->getAddress()));

    endpoints2 = _subscriptionManager.getSubscriptions(eventgroup2);
    EXPECT_EQ(1U, endpoints2->size());
    SubscriptionEndpointList::iterator itr2 = endpoints1->begin();
    EXPECT_EQ(ipAddress2, ip4_to_u32(itr2->getAddress()));
}

TEST_F(SubscriptionManagerTest, testRemoveSubscriptionsRemovesEventgroupIfEmpty)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x01;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000201;
    uint16_t port                    = 0x8899;

    SubscriptionManager::InternalSubscribeResult result;

    result = _subscriptionManager.addSubscription(
        serviceId, majorVersion, instanceId, eventgroupId, ttl, make_ip4(ipAddress), port);
    EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);

    SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId, eventgroupId);
    SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_TRUE(endpoints);
    EXPECT_EQ(1U, endpoints->size());

    _subscriptionManager.removeSubscriptions(make_ip4(ipAddress));

    endpoints = _subscriptionManager.getSubscriptions(eventgroup);
    EXPECT_FALSE(endpoints);

    // check if empty eventgroup was removed, too
    for (size_t i = 1U; i < NUM_SUBSCRIPTIONS + 1U; ++i)
    {
        result = _subscriptionManager.addSubscription(
            serviceId, majorVersion, instanceId + i, eventgroupId, ttl, make_ip4(ipAddress), port);
        EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
    }
}

TEST_F(SubscriptionManagerTest, testRemoveSubscriptionsOnStop)
{
    service_id::type serviceId       = 0x1234;
    major_version::type majorVersion = 0x01;
    instance_id::type instanceId     = 0x00;
    eventgroup_id::type eventgroupId = 0x01;
    ttl::type ttl                    = 0x03;
    uint32_t ipAddress               = 0xC0000200;
    uint16_t port                    = 0x8800;

    SubscriptionManager::InternalSubscribeResult result;

    for (size_t loops = 0U; loops < 2U; ++loops)
    {
        for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
        {
            result = _subscriptionManager.addSubscription(
                serviceId,
                majorVersion,
                instanceId + i,
                eventgroupId,
                ttl,
                make_ip4(ipAddress + i),
                port + i);
            EXPECT_EQ(SubscriptionManager::InternalSubscribeResult::INTERNAL_SUBSCRIBE_OK, result);
        }

        for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
        {
            SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId + i, eventgroupId);
            SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
            EXPECT_TRUE(endpoints);
            EXPECT_EQ(1U, endpoints->size());
        }

        _subscriptionManager.stop();

        for (size_t i = 1U; i <= NUM_SUBSCRIPTIONS; ++i)
        {
            SubscribedEventGroup eventgroup(serviceId, majorVersion, instanceId + i, eventgroupId);
            SubscriptionEndpointList* endpoints = _subscriptionManager.getSubscriptions(eventgroup);
            EXPECT_FALSE(endpoints);
        }
    }
}

} // anonymous namespace

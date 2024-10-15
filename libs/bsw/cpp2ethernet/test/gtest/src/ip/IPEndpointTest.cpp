// Copyright 2025 Accenture.

#include "ip/IPEndpoint.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::ip;
using namespace ::testing;

TEST(IPEndpointTest, ConstructorTests)
{
    // default constructor
    IPEndpoint e;
    ASSERT_FALSE(e.isSet());
    ASSERT_EQ(0U, ip4_to_u32(e.getAddress()));
    ASSERT_EQ(0U, e.getPort());

    // normal constructor
    IPAddress a = make_ip4(0x12345678);
    IPEndpoint other(a, 20U);
    ASSERT_TRUE(other.isSet());
    ASSERT_EQ(20U, other.getPort());
    ASSERT_EQ(a, other.getAddress());

    // assignment operator
    e = other;
    ASSERT_TRUE(e.isSet());
    ASSERT_EQ(20U, e.getPort());
    ASSERT_EQ(a, e.getAddress());
    ASSERT_EQ(e, other);

    // copy constructor
    IPEndpoint tmp(e);
    ASSERT_EQ(20U, tmp.getPort());
    ASSERT_EQ(a, tmp.getAddress());

    // port is different
    tmp.setPort(10U);
    ASSERT_FALSE(e == tmp);
    ASSERT_TRUE(e != tmp);
    ASSERT_FALSE(e < tmp);

    // ports are the same
    e.setPort(10U);

    // address is now different
    tmp.setAddress(make_ip4(0x12345679));
    ASSERT_FALSE(e == tmp);
    ASSERT_TRUE(e < tmp);

    // port is not set
    IPEndpoint tmp2;
    tmp2.setAddress(make_ip4(0x12345678));
    ASSERT_FALSE(e < tmp2);

    // clear
    e.clear();
    ASSERT_FALSE(e.isSet());
    ASSERT_EQ(0U, ip4_to_u32(e.getAddress()));
    ASSERT_EQ(0U, e.getPort());
}

TEST(IPEndpointTest, FieldsAreCorrectlySet)
{
    IPEndpoint e;
    ASSERT_FALSE(e.isSet());

    IPAddress a = make_ip4(0x12345678);
    e.setAddress(a);
    ASSERT_FALSE(e.isSet());

    e.setPort(0);
    ASSERT_TRUE(e.isSet());

    e.clear();
    ASSERT_FALSE(e.isSet());

    e.setPort(1);
    ASSERT_FALSE(e.isSet());
    e.setAddress(a);
    ASSERT_TRUE(e.isSet());
}

} // anonymous namespace

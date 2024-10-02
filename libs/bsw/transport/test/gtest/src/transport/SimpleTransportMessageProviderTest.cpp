// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "transport/SimpleTransportMessageProvider.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace ::transport;

namespace
{
TEST(SimpleTransportMessageProvider, GetTransportMessage)
{
    uint8_t const bus = 0;
    declare::TransportMessageProviderConfiguration<10, 10> config;

    SimpleTransportMessageProvider provider(config);

    TransportMessage* msg = 0;
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_OK,
        provider.getTransportMessage(bus, 10, 10, 1, {}, msg));

    msg = 0;
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE,
        provider.getTransportMessage(bus, 10, 10, 1000, {}, msg));
}

TEST(SimpleTransportMessageProvider, GetTransportMessageNoneAvailable)
{
    uint8_t const bus = 0;
    declare::TransportMessageProviderConfiguration<1, 10> config;

    SimpleTransportMessageProvider provider(config);

    TransportMessage* msg = 0;
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_OK,
        provider.getTransportMessage(bus, 10, 10, 1, {}, msg));

    TransportMessage* msg1 = 0;
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
        provider.getTransportMessage(bus, 10, 10, 1, {}, msg1));

    provider.releaseTransportMessage(*msg);
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_OK,
        provider.getTransportMessage(bus, 10, 10, 1, {}, msg1));
}

TEST(SimpleTransportMessageProvider, Dump)
{
    declare::TransportMessageProviderConfiguration<1, 10> config;

    SimpleTransportMessageProvider provider(config);
    ASSERT_NO_THROW(provider.dump());
}

} // anonymous namespace

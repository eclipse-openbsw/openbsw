// Copyright 2024 Accenture.

/**
 * Contains
 * \file
 * \ingroup
 */

#include "transport/DestituteTransportMessageProvider.h"
#include "transport/TransportMessage.h"

#include <gtest/gtest.h>

using namespace ::testing;
using namespace transport;

namespace
{
TEST(DestituteTransportMessageProvider, GetTransportMessage)
{
    DestituteTransportMessageProvider provider;
    TransportMessage* msg = 0L;
    ASSERT_EQ(
        ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
        provider.getTransportMessage(0, 10, 10, 1, {}, msg));
    ASSERT_EQ(0L, msg);
}

TEST(DestituteTransportMessageProvider, ReleaseTransportMessage)
{
    DestituteTransportMessageProvider provider;
    TransportMessage msg;
    ASSERT_NO_THROW(provider.releaseTransportMessage(msg));
}

TEST(DestituteTransportMessageProvider, Dump)
{
    DestituteTransportMessageProvider provider;
    ASSERT_NO_THROW(provider.dump());
}

} // anonymous namespace

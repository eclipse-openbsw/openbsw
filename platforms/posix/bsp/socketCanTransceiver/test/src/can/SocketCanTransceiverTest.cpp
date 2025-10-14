// Copyright 2024 Accenture.

#include "can/SocketCanTransceiver.h"

#include <gtest/gtest.h>

namespace
{

using namespace ::testing;

/**
 * \desc
 * Verifies that a newly constructed SocketCanTransceiver is in state CLOSED.
 */
TEST(SocketCanTransceiverTest, transceiver_creation)
{
    ::can::SocketCanTransceiver::DeviceConfig config{"vcan0", {}};
    ::can::SocketCanTransceiver transceiver{config};
    EXPECT_EQ(transceiver.getState(), ::can::ICanTransceiver::State::CLOSED);
}

} // namespace

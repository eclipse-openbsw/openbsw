/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

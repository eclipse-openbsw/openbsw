/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/NetworkChannel.h"
#include "someip/SomeIpMessage.h"

#include <cstdint>

namespace someip
{
class SomeIpMessage;

class ITpListener;

/**
 * Interface for a TP transceiver.
 */
class ITpTransceiver
{
public:
    virtual ~ITpTransceiver() = default;

    static uint8_t const TP_MESSAGE_TYPE_BIT_MASK = 32U;

    static uint32_t const TP_RECEIVE_TIMEOUT = 100U;
    static uint32_t const TP_UPDATE_CYCLE    = TP_RECEIVE_TIMEOUT / 4U;

    // INTERFACE1_START

    /**
     * Function that tells if outgoing message is TP message.
     */
    static bool isOutgoingTpMessage(uint8_t proto, uint32_t length);

    /**
     * Function that tells if incoming message is TP message.
     */
    static bool isIncomingTpMessage(uint8_t proto, SomeIpMessage::MessageType type);

    // INTERFACE1_END

    struct TpHeader
    {
        uint32_t payloadOffset;
        bool hasMoreSegments;
    };

    static size_t const TP_HEADER_LENGTH = 4U;

    // INTERFACE2_START

    /**
     * Function that that serializes TP header.
     */
    static uint32_t serializeTpHeader(TpHeader const& input);

    /**
     * Function that that parses TP header.
     */
    static void parseTpHeader(uint32_t input, TpHeader& output);

    /**
     * Pure virtual function that handles sending TP message if possible.
     */
    virtual bool sendTpMessage(NetworkChannel& channel, SomeIpMessage const& message) const = 0;

    /**
     * Pure virtual function that handles receiving TP message if possible.
     */
    virtual void
    receiveTpMessage(NetworkChannel& channel, SomeIpMessage const& message, ITpListener& listener)
        = 0;

    // INTERFACE2_END
};

} // namespace someip

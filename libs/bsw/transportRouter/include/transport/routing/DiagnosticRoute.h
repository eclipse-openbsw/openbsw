/********************************************************************************
 * Copyright (c) 2026 Jefferson Nascimento
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

// AI disclosure: this file was largely generated with an AI assistant and was reviewed and
// tested by the contributor. Assisted-by: Anthropic Claude Opus 5.5

#pragma once

#include <etl/span.h>

#include <cstddef>
#include <cstdint>

namespace transport
{
/**
 * A diagnostic node reachable through the gateway.
 *
 * The TransportRouter forwards a request whose target address equals logicalAddress to the
 * transport layer registered for busId and expects the node's response within p2Ms (or
 * p2StarMs after each response pending, NRC 0x78).
 */
struct DiagnosticRoute
{
    /// Logical address of the node (target address of requests, source address of responses).
    uint16_t logicalAddress;
    /// Bus whose transport layer reaches the node.
    uint8_t busId;
    /// Time to the start of the node's response.
    uint16_t p2Ms;
    /// Time to the next response after a response pending (NRC 0x78).
    uint16_t p2StarMs;
    /// Largest request routed to the node; larger requests are rejected.
    uint16_t maxLength;
    /// Name used in log output; may be nullptr.
    char const* name;
};

/**
 * Static configuration of a TransportRouter.
 */
struct TransportRouterConfiguration
{
    /// Logical address of the gateway's own diagnostic server.
    uint16_t localAddress;
    /// Bus of the gateway's own diagnostic server (e.g. SELFDIAG).
    uint8_t localBusId;
    /// Functional address: requests go to the own server and to every route bus.
    uint16_t functionalAddress;
    /// Source address the gateway uses towards nodes (the gateway acts as their tester).
    uint16_t gatewayTesterAddress;
    /// External testers: source addresses in [testerAddressMin, testerAddressMax].
    uint16_t testerAddressMin;
    uint16_t testerAddressMax;
    /// How long responses to a functional request are forwarded.
    uint32_t functionalWindowMs;
    /// Budget to deliver a request to a node and to receive a segmented response; at least the
    /// longest transfer on the slowest (e.g. paced) bus.
    /// TransportRouter::DEFAULT_TRANSFER_TIMEOUT_MS suits unpaced classic CAN.
    uint32_t transferTimeoutMs;
    /// Largest functional request (one single frame on classic CAN: 7).
    uint16_t maxFunctionalLength;
    /// Routing table; it is checked by TransportRouter::validate().
    ::etl::span<DiagnosticRoute const> routes;
};

} // namespace transport

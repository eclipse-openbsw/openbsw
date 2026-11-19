..
   *******************************************************************************
   Copyright (c) 2026 Jefferson Nascimento

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

..
   AI disclosure: this file was largely generated with an AI assistant and was reviewed by
   the contributor. Assisted-by: Anthropic Claude Opus 5.5

transportRouter
===============

Overview
--------

``TransportRouter`` is a diagnostic gateway router. It routes UDS messages by
logical address between external testers (for example DoIP), the gateway's own
diagnostic server and diagnostic nodes reached through other transport layers
(for example DoCAN). It implements ``ITransportMessageProvidingListener`` and
plugs into the existing transport layers in the same way as
``TransportRouterSimple``.

``TransportRouterSimple`` forwards every request to the local diagnostic server
and every reply to the bus that sent the last request. ``TransportRouter`` adds
what a gateway needs:

- a routing table of ``DiagnosticRoute`` entries (logical address, bus, P2, P2*,
  maximum length), checked by ``validate()``
- address-based routing: requests to the local address, to a route's address or
  to the functional address
- one outstanding request per route, with P2/P2* supervision and pass-through of
  response pending (NRC 0x78)
- restoration of the original tester address on responses, so several testers on
  several buses can share the gateway
- functional fan-out: one copy per route bus, and forwarding of the nodes'
  responses within a configurable window
- release of a tester's pending requests when its connection closes
- saturating statistics and an ``IRouteObserver`` for route outcomes, e.g. a
  lost-communication monitor

The router never interprets or generates diagnostic payloads for a node.

Routing rules
-------------

Messages are classified by address, not by bus:

=================================  ==============================================
Message                            Routing
=================================  ==============================================
Tester → local address             Local bus (own diagnostic server)
Tester → route address             Route's bus, source replaced by the gateway
                                   tester address; the route becomes busy
Tester → functional address        Local bus, and one copy per route bus
Route node → gateway tester        The tester of the pending (or functional)
                                   request, on that tester's bus
Local server → tester              The tester's bus, learned from its requests
=================================  ==============================================

``getTransportMessage()`` reports routing errors with
``ITransportMessageProvider::ErrorCode``. The DoIP server maps them to diagnostic
message NACKs:

==============================  =====================================  ========
Situation                       Error code                             DoIP
==============================  =====================================  ========
Unknown target address          ``TPMSG_INVALID_TGT_ADDRESS``          0x03
Larger than the route maximum   ``TPMSG_SIZE_TOO_LARGE``               0x04
Request outstanding on route    ``TPMSG_NO_MSG_AVAILABLE``             0x05
No free buffer                  ``TPMSG_NO_MSG_AVAILABLE``             0x05
==============================  =====================================  ========

Timing
------

``cyclic()`` must be called periodically, for example every 10 ms. A route waits
for its node:

- ``transferTimeoutMs`` (configuration) for the transport layer to confirm the request
- ``p2Ms`` after the confirmation for the start of the response
- ``p2StarMs`` after each response pending
- at least ``transferTimeoutMs`` (or ``p2StarMs`` if longer) once a segmented
  response has started

``DEFAULT_TRANSFER_TIMEOUT_MS`` (2 s) suits classic CAN without pacing. A slower or
paced bus needs a budget for its longest transfer, for example several 4095-byte
transfers that share a paced CAN bus.

When a deadline expires the route becomes idle again, the timeout is counted and
the observer is notified. The tester side keeps its own P2 client timing; the
router does not send a response on behalf of a node.

Buffers
-------

The router owns ``NUM_BUFFERS`` full-size (4095 bytes) and ``NUM_SMALL_BUFFERS``
small (8 bytes) message buffers. Requests to the local and functional addresses
always get a full-size buffer, because the UDS server builds its response in the
request buffer. All memory is allocated statically.

Usage
-----

.. code-block:: cpp

    DiagnosticRoute const ROUTES[] = {
        {0x1020U, ::busid::CAN_0, 150U, 5100U, 4095U, "rear"},
        {0x1030U, ::busid::CAN_0, 150U, 5100U, 4095U, "front"},
    };

    TransportRouterConfiguration const CONFIG{
        0x1010U,           // own diagnostic server
        ::busid::SELFDIAG,
        0xE400U,           // functional address
        0x0E10U,           // gateway tester address towards the nodes
        0x0E00U, 0x0EFFU,  // external testers
        150U,              // functional response window [ms]
        TransportRouter::DEFAULT_TRANSFER_TIMEOUT_MS,
        7U,                // largest functional request
        ROUTES};

    TransportRouterStatistics statistics;
    TransportRouter router(CONFIG, statistics, TransportRouter::NowMsType::create<&nowMs>());

    size_t badRoute;
    if (router.validate(badRoute) != TransportRouter::ValidationError::NONE) { /* stop */ }
    router.init();
    // register the DoIP, DoCAN and UDS transport layers with addTransportLayer()
    // call router.cyclic() every 10 ms

The DoCAN layer must map each route to the node's CAN identifiers with
``transportSourceId`` = route address and ``transportTargetId`` = gateway tester
address, and the functional address to the functional CAN identifier.

Unit tests
----------

``test/`` contains the unit tests (``transportRouterTest``). For tests of code
that implements or uses ``IRouteObserver``, the module provides
``transport::RouteObserverMock`` in ``mock/gmock/include``: CMake target
``transportRouterMock``, Bazel target ``transport_router_mock``.

..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Routing detailed design
=======================

Each routing message uses an 8-byte header followed by its payload:

* 4 bytes: message ID (big endian)
* 4 bytes: payload length in bytes (big endian)
* N bytes: payload

``RxAdapter`` or ``PduTransportRxAdapter`` extracts PDU(s) from a payload according to its
``RxAdapterTable`` and implements the ``IReader`` interface. The ``RxAdapterTable`` determines
which PDUs a payload contains with the message ID and the payload length. For each PDU,
it defines a length and an offset in bytes, i.e., the position of the PDU in the payload.

.. _rxAdapterTable:

.. literalinclude:: ../include/routing/RxAdapterTable.h
   :start-after: RX_ADAPTER_TABLE_BEGIN
   :end-before: RX_ADAPTER_TABLE_END
   :language: cpp

``Router`` reads PDUs from channels using ``IReader`` interfaces and writes them to channels using
``IWriter`` interfaces. Its ``PduRoutingTable`` specifies to which channel(s) a PDU should be
forwarded as well as the outgoing message ID.

.. _pduRoutingTable:

.. literalinclude:: ../include/routing/PduRoutingTable.h
   :start-after: PDU_ROUTING_TABLE_BEGIN
   :end-before: PDU_ROUTING_TABLE_END
   :language: c++


``LegacyTxAdapter`` or ``PduTransportTxAdapter`` creates outgoing messages out of PDUs according to its
``TxAdapterTable`` and implements the ``IWriter`` interface. For each message, the ``TxAdapterTable``
determines the payload length and the PDU offset within it.

.. _txAdapterTable:

.. literalinclude:: ../include/routing/TxAdapterTable.h
   :start-after: TX_ADAPTER_TABLE_BEGIN
   :end-before: TX_ADAPTER_TABLE_END
   :language: cpp

Routing example
---------------

The diagram below shows a concrete routing path through the tables. A message arriving on
``PDU_TRANSPORT0`` with ID 18 is routed to ``CAN0`` (outgoing ID 1), which in turn routes to
``PDU_TRANSPORT1`` (outgoing ID 4096), which loops back to ``PDU_TRANSPORT0`` (outgoing ID 256).
This illustrates how the routing module chains multi-hop routes across CAN and PDU transport
channels using only ID-based table lookups.

The diagram is generated with:

.. code-block:: console

   cd tools
   python -m blob.routing visualize --channel PDU_TRANSPORT0 --id 18 \
       libs/bsw/routing/test/routing.jsonl

.. graphviz:: resources/routing_example.dot



System context
--------------

The building-block diagram in the module overview shows the internal components. From an
architectural point of view it is equally important to see the module in its **system context** —
the external actors it depends on and the boundaries it must not cross:

.. uml::
   :align: center

   left to right direction

   [Configuration blob\n(in flash)] as blob
   [CAN / FlexRay\ndrivers] as bus
   [Network stack\n(UDP sockets)] as net
   [Periodic scheduler\n(async context)] as sched
   [Error handler\n(integrator callback)] as err

   package "routing" {
     [Integration] as integ
   }

   blob --> integ : parsed tables (read-only)
   bus <--> integ : PDUs via SPSC MemoryQueue\n(driver context)
   net <--> integ : datagrams\n(network context)
   sched --> integ : route()/send/timeout\n(periodic context)
   integ --> err : status codes

This context view makes the two critical boundaries explicit: the **flash boundary** (the blob is
read-only and never copied) and the **context boundary** between the bus driver context and the
routing/network context, which is crossed only through single-producer/single-consumer queues.

Error handling and reliability
------------------------------

The module is *fail-operational at the PDU granularity*: a malformed or undeliverable PDU is
dropped and reported, but never corrupts the pipeline or blocks other PDUs. Errors are surfaced
through the integrator-supplied ``ErrorHandler`` callback rather than through exceptions or return
codes on the hot path. The reported status codes are:

.. literalinclude:: ../include/routing/ErrorHandler.h
   :start-after: STATUS_CODES_BEGIN
   :end-before: STATUS_CODES_END
   :language: cpp

Key reliability properties:

* **Bounded, non-blocking degradation.** When an RX allocation fails (queue full), the incoming
  datagram/frame is discarded and counted; the error is reported only on the first failure of a run
  to avoid log flooding. Statistics counters (``failedMemAllocPdus``, ``invalidIpAddressPdus``,
  ``timeoutSends``, ``flushes``, …) are exposed for health monitoring.
* **Source filtering.** UDP receivers optionally validate the source IP against a configured
  allow-list before accepting a datagram.
* **No silent blocking on TX.** The legacy CAN TX path performs no retry; a failed bus write is
  dropped rather than stalling the router.
* **Configuration integrity** is a precondition supplied by the ``blob`` module when the integrator
  passes a span returned by ``blob::load()``. The routing loaders then perform their own
  table-structure checks before adding channels and routing entries.

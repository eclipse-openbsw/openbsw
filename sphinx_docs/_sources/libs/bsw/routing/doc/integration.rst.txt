..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Routing Integration
===================

Integration
-----------

For projects using FlexRay, CAN, and PDU transport, the ``Integration`` class provides
a ready-to-go implementation of the instantiation of the routing components.

The ``Integration`` class uses the ``blob`` module to access configurations, which can be loaded
from memory using the ``load`` functions.

Getting started
+++++++++++++++

1. Generate routing-related blob artifacts with the blob generator tool.
2. Provide an ``::etl::span<uint8_t const>`` over the generated blob bytes and validate it with
   ``blob::load()``. Treat an empty returned span as a configuration error and do not continue
   routing initialization with it.
3. Pass the validated span to ``PduTransportIntegration::init()`` and ``Integration::init()``.
4. Provide bus and socket endpoints (CAN/FlexRay readers and writers, UDP sockets) to these ``init()``
   calls.
5. Call ``PduTransportIntegration::activate()`` once the network stack is up. This binds and joins
   the UDP sockets and enables the RX/TX direction of each PDU transport channel according to its
   configured mode and VLAN. Until ``activate()`` has been called, no UDP data is received or
   transmitted, even though ``init()`` has already built all tables and adapters.
6. In the periodic task, call ``checkTransmissionTimeouts()`` and ``sendUdpFrames()`` on
   ``PduTransportIntegration`` and ``route()`` on ``Integration`` (see :ref:`routingLifecycle`).
7. On shutdown, call ``PduTransportIntegration::disable()`` to close the sockets and tear down the
   receivers/senders.

.. note::

   The socket API is **not** thread-safe. ``activate()``, ``disable()``, ``checkTransmissionTimeouts()``,
   ``sendUdpFrames()`` and ``route()`` must all be driven from the same task/context that owns the
   network stack. See :ref:`routingConcurrency` for the full concurrency model.

Here is the structure of this class, with regard to the ``Routing`` components (note that
``CHANNELS = PDU_TRANSPORT_CHANNELS + CAN_CHANNELS + FLEXRAY_CHANNELS``):

.. uml::
   :align: center

    namespace io {
       interface IWriter {}
       interface IReader {}
       hide members
   }
   namespace routing {
       class Integration<MAX_NUM_PDU_TRANSPORT_CHANNELS, NUM_CAN_CHANNELS, NUM_FLEXRAY_CHANNELS, MAX_PDU_TRANSPORT_CHANNEL_ELEMENT_SIZE, MAX_CAN_CHANNEL_ELEMENT_SIZE, MAX_FLEXRAY_CHANNEL_ELEMENT_SIZE> {}
        class Router<N> {}
        class RxAdapter {}
        class TxAdapter {}


        Integration "1" *-- "1" Router
        Integration "1" *-- "CHANNELS" RxAdapter
        Integration "1" *-- "CHANNELS" TxAdapter
        Integration --> blob.Blob : "uses"

        Router "1" --> "CHANNELS" io.IWriter
        Router "1" --> "CHANNELS" io.IReader

        RxAdapter --|> io.IReader
        TxAdapter --|> io.IWriter

        hide members
   }
   namespace blob{
        class Blob

        hide members
   }


PDU Transport Integration
-------------------------

The ``PduTransportIntegration`` class takes care of instantiating all the components related to the UDP-based PDU
transport communication, except for the sockets. The sockets are provided at initialization and their type has
to derive from ``AbstractDatagramSocket``. ``PduTransportIntegration`` separates frame creation from
socket transmission:

* ``PduTransportBufferedWriter`` batches outgoing PDU transport messages into frame-sized queue
  entries
* ``checkTransmissionTimeouts()`` evaluates whether buffered frames must be committed to TX queues because
  of configured timeouts
* ``sendUdpFrames()`` begins the actual UDP transmission by reading frames from the TX queue and sending
  up to 10 frames per channel per call through sockets

As a result, data already written to TX queues may still not have been sent out. Data is written to the TX queue when
the next message would no longer fit into the current frame buffer or the configured transmission timeout expires.
The timeout starts with the first committed message in an empty frame buffer, not with allocation.

The structure of this class is the following:

.. uml::
   :align: center

    namespace udp {
       interface AbstractDatagramSocket {}
       hide members
   }
   namespace io {
       class MemoryQueueWriter {}
       class MemoryQueueReader {}

       MemoryQueueWriter --> MemoryQueue
       MemoryQueueReader --> MemoryQueue

       namespace udp {
           class Receiver{}
           class Sender{}
       }
       hide members
   }
   namespace routing {
       class PduTransportIntegration<MAX_NUM_CHANNELS, MAX_ELEMENT_SIZE> {}

        PduTransportIntegration -r-> blob.Blob : "uses"
        PduTransportIntegration -r-> PduTransportConfig : "uses"

        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueue
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueueReader
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueueWriter
        PduTransportIntegration "1" *-- "PDU_TRANSPORT_CHANNELS" io.udp.Receiver
        PduTransportIntegration "1" *-- "PDU_TRANSPORT_CHANNELS" io.udp.Sender
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" udp.AbstractDatagramSocket

        hide members
   }
   namespace blob{
        class Blob
        hide members
   }

If we look at the way these components are interconnected and connected with the ``Integration`` class, we get:

.. uml::
   :align: center

   component Integration {
     () IReader as ir1
     () IWriter as iw1

     [Router]
     [RxAdapter]
     [TxAdapter]
   }

   component PduTransportIntegration {
     () IReader as ir4
     () IWriter as iw4

     [Receiver]
     [MemoryQueue] as mq1
     [MemoryQueue] as mq2
     [Sender]
     () AbstractDatagramSocket as isock
     () AbstractDatagramSocket as osock
   }

   () IReader as ir3
   () IWriter as iw3

   [UdpSocket] as iudp
   [UdpSocket] as oudp

   [Router] -left-( ir1
   [Router] -right-( iw1
   [RxAdapter] -right- ir1
   [TxAdapter] -left- iw1
   [RxAdapter] -down-( ir3
   [TxAdapter] -down-( iw3

   mq1 -up- ir3
   mq2 -up- iw3

   mq1 -left- iw4
   mq2 -right- ir4
   [Receiver] -right-( iw4
   [Sender] -left-( ir4

   [Receiver] -down-( isock
   [Sender] -down-( osock

   iudp -up- isock
   oudp -up- osock

.. _routingLifecycle:

Lifecycle and workflows
-----------------------

The routing components are plain objects with explicit ``init``/``activate``/``disable`` phases;
they do not own a task and never allocate. The integrating system owns the lifecycle and schedules
the runtime calls.

Initialization
    *Entry points:* ``PduTransportIntegration::init()``, ``Integration::init()``.

    Parse the already validated blob span, discover the configured channels, build the RX/TX adapter
    tables and the routing table, and connect the readers/writers. No socket or bus I/O happens yet.
    ``init()`` is idempotent: a second call on an already-initialized instance is ignored.

Startup
    *Entry points:* ``PduTransportIntegration::activate()``, plus registering the bus listeners and
    starting the periodic task.

    Bind and join the UDP sockets, enable the RX and/or TX direction of every PDU transport
    channel whose configured VLAN matches, and start driving the runtime loop. ``activate()``
    clears the RX/TX queues and resets the buffered writers, so it is safe to call again to
    re-activate after a ``disable()``.

Runtime
    *Entry points:* ``checkTransmissionTimeouts()``, ``sendUdpFrames()``, ``route()``.

    Executed at the cadence chosen by the integrating system. See below.

Shutdown / de-initialization
    *Entry points:* ``PduTransportIntegration::disable()``, plus deregistering the bus listeners and
    cancelling the task.

    Disconnect and close all sockets and destroy the receivers/senders. The parsed tables remain
    valid, so the instance can be re-activated with ``activate()`` without re-running ``init()``.

The runtime step performs three actions, in order:

* ``checkTransmissionTimeouts()`` flushes any buffered TX frame whose transmission timeout has
  expired or is within the implementation's early-flush margin, bounding TX latency.
* ``sendUdpFrames()`` drains the TX queues to the sockets, sending at most a fixed number of frames
  per channel per call (a compile-time constant, currently 10, in ``PduTransportIntegration``).
* ``route()`` moves PDUs from the RX side to the TX side of the router (see
  :ref:`routingThroughput`).

.. _routingConcurrency:

Threading and concurrency
-------------------------

The routing pipeline is designed around **single-threaded execution of the router and the socket
I/O**, with lock-free hand-off queues bridging any other context that produces or consumes PDUs.

* **Router / PDU transport context.** ``route()``, ``checkTransmissionTimeouts()``,
  ``sendUdpFrames()``, ``activate()`` and ``disable()`` must run in one and the same context — the
  one that owns the network (socket) stack. The socket API is not thread-safe, so this constraint is
  mandatory, not merely advisory.
* **UDP reception** happens as a data-listener callback from the network stack (same context) and is
  written into a per-channel RX ``MemoryQueue``.
* **Legacy bus reception** (CAN/FlexRay) typically happens in a *different* driver context. The
  driver's frame callback writes the PDU into a single-producer/single-consumer (SPSC)
  ``MemoryQueue``; the router reads from it in its own context. This queue is the only
  synchronization primitive needed — it is lock-free and wait-free for one producer and one
  consumer, so no mutex is taken on the hot path.

Consequently the only cross-context sharing is through ``MemoryQueue`` instances, each restricted to
exactly one producer and one consumer. Do not call any router or PDU-transport method concurrently
from two contexts.

.. _routingThroughput:

Performance and efficiency
--------------------------

The module targets constrained, hard-real-time ECUs. The most important characteristics for an
integrator to size and tune the pipeline are:

* **Static, allocation-free.** All buffers are fixed-capacity ``etl`` arrays/vectors sized at
  compile time from the template parameters; there is no heap use at init or runtime. The memory
  footprint is deterministic and given by the formulas in :doc:`interface`.
* **Span-based, bounded copying.** Configuration data is read in place from the validated blob span.
  Routed payload bytes are copied when adapters extract PDUs and when writers emit output PDUs or
  cross queue, bus, or socket boundaries.
* **One PDU per** ``route()`` **call, round-robin.** ``route()`` inspects the channels in a
  rotating order and returns after forwarding a *single* PDU (or after one full sweep finds
  nothing). This guarantees fairness across channels and a bounded, predictable per-call cost, but
  it also means the routed throughput equals the ``route()`` call rate. To raise throughput, call
  ``route()`` in a loop until it returns ``false`` (drain), or increase the task frequency —
  trading CPU headroom for latency.
* **Frame batching on TX.** ``PduTransportBufferedWriter`` packs multiple PDUs into one Ethernet
  frame up to the configured channel element size. This amortizes the per-frame IP/UDP overhead. A
  frame is committed when the next PDU no longer fits **or** the per-channel transmission timeout
  expires; the timeout therefore bounds the added latency of batching. Set the timeout to ``0`` to
  disable batching latency at the cost of protocol overhead.
* **Bounded socket work.** ``sendUdpFrames()`` sends at most a fixed number of frames per channel
  per call — a compile-time constant (currently 10) in ``PduTransportIntegration``, not a runtime-
  or blob-configurable value. This keeps the worst-case execution time of a single runtime step
  bounded even under burst load; backlog is drained over subsequent calls. Changing the cap means
  editing that constant.
* **Blob read in place.** The configuration blob is span-based and allocation-free (see the
  ``blob`` module design), so the routing tables need not be copied into routing-owned RAM before
  parsing.

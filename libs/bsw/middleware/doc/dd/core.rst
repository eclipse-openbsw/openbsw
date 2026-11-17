..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Core
====

The core software unit of the middleware provides the key algorithms needed for the communication between applications.

Middleware Message
------------------

Middleware is a service-oriented message passing system. The ``Message`` class is its fundamental unit of communication.
This class consists of a header and a payload.
The header has the following information:

* Service ID - identifies the service to which the message belongs.
* Member ID - identifies the specific member function of the service that is being invoked.
* Request ID - identifies the specific request being made.
* Instance ID - identifies the instance of the service (multiple instances may exist).
* Source cluster ID - identifies the cluster from which the message originated.
* Destination Cluster ID - identifies the cluster to which the message is being sent.
* Address ID - identifies the recipient of the message within the destination cluster.

The middleware services' nodes will be scattered across different clusters in the ECU.
This means that messages can travel between clusters. Each ``Message`` must carry information about its origin and destination.
Additionally, a message may have several possible recipients. Each recipient needs a unique identifier after system initialization.
To this end, the header contains the source cluster ID, target cluster ID, and address ID fields.

Finally, the payload contains the actual data being transmitted. The message
capacity is defined by ``Message::MAX_PAYLOAD_SIZE`` as
``Message::MAX_MESSAGE_SIZE - sizeof(Message::Header) - sizeof(uint32_t)``.
With the current 64-byte message layout, this capacity is 48 bytes.
Payloads that fit and are trivially copyable are stored in the message's
internal buffer. Larger or non-trivial payloads are copy-constructed into
middleware-managed external storage, and the message records an external
payload offset and the ``hasExternalPayload`` flag. ``Message`` also supports
an ``ErrorState`` payload for error responses.

Middleware PDU
--------------

A PDU (Protocol Data Unit) is a fixed-size block of bytes that carries signals
between clusters. Unlike a ``Message``, a PDU is not queued or delivered: it is
a single shared-memory slot that always holds the most recent value. A writer
overwrites the slot, and any reader can sample it at any time. The size of each
PDU is defined in the deployment model, and the generated code places one
``MiddlewarePdu`` per PDU in shared memory.
The PDUs are protected with a sequence lock like algorithm. The difference to
a traditional sequence lock is that there is no writer-side locking. For this,
a Writer Contract should be followed.

Only one writer may exist per PDU. A PDU is only owned by one application, so
it makes no sense to have more than a single owner. The sequence lock
uses only a core lock on the writer side to avoid live-locks. However, a
second writer on a different core would corrupt the payload, as well as the
sequence counting. Violating this contract leads to undefined behavior.

Sequence Lock
-------------

``MiddlewarePdu`` is a sequence-locked buffer made of an atomic sequence
counter and the non-atomic payload.

* ``store()`` increments the counter to an odd value, copies the payload, then
  increments it again to an even value. An odd counter means a write is in
  progress.
* ``load()`` reads the counter, retries while it is odd, copies the payload,
  and reads the counter again. If both reads match, the copy is consistent.
  Otherwise a write overlapped the copy and the reader retries.

Readers never block the writer, and the writer never waits for readers. The
trade-off is that a reader may spin while a write is in progress.
``load()`` returns the sequence number of the snapshot it read, which
can be used to detect whether the snapshot differs from the previous one.

Platform Obligations
--------------------

The sequence lock relies on two platform-provided functions in
``middleware::concurrency``. If either is missing, linking fails.

* ``atomicThreadFence()`` shall provide acquire/release ordering of the
  payload accesses against the sequence counter, as seen by all cores that
  access the PDU. It behaves like
  ``std::atomic_thread_fence(std::memory_order_acq_rel)``. On hardware that
  reorders loads or stores between cores (e.g. ARM, TriCore) this requires a
  hardware barrier. A compiler barrier is sufficient only where the hardware
  already guarantees this order (single core, x86). Release and acquire
  accesses on the sequence counter do not order the non-atomic payload
  accesses, so ``store()`` and ``load()`` use this fence to order the payload
  against the counter.
* ``spinLockOptimization()`` is called on every retry of ``load()``. It
  lets a platform insert a CPU hint such as ``yield`` (ARM) or ``pause`` (x86)
  into that loop, as well as a ``taskYield()`` (FreeRTOS). It may be empty,
  although it is not recommended to leave the CPU on the loop 100% of
  the time.

The qualifier interface used by the generated PDU reader is described in
:doc:`../integration/integrating_middleware_into_ecu`.

PDU Broker
----------

The PDU broker connects the PDUs in shared memory to a bus. It is implemented by
``PduBrokerConnection`` and has two directions.

* **Transmit:** a PDU writer stores its PDU with ``commit()``, and its
  transmission strategies push the PDU ID to a FIFO queue in shared memory.
  ``processTransmissions()`` drains this queue in the order the IDs were queued,
  so the broker never has to search shared memory for pending PDUs. For
  each ID, the broker copies a consistent snapshot of the PDU into a local buffer
  and passes it to ``IPduTxSink::submit()``.
* **Receive:** the bus driver calls ``onReceive()`` with the ID and payload of a
  received PDU. The broker stores the payload in shared memory, which increments
  the sequence counter of the PDU.

The broker hands the PDUs over in commit order and removes each ID from the
queue as soon as the PDU is handed over. IDs that are not PDU IDs are
discarded. The sink owns everything after the hand-off: its backlog, retries,
acknowledgements and what it drops, and it makes drops observable.
``IPduTxSink::submit()`` returns nothing: the broker continues with the next
PDU, so a slow or failing destination never blocks the PDUs behind it, and the
shared queue holds no pending transmissions. A sink that serves
independent destinations, such as CAN transmit buffers selected by ID, keeps a
backlog per destination.

The broker reaches the PDUs through a table of function pointers generated from
the deployment (``getPduAccessTable()``). PDU IDs are defined in the deployment,
are unique and need not be sequential, so the generator sorts the table by ID
and the broker finds a PDU with a binary search. This keeps
``PduBrokerConnection`` independent of the number and size of the PDUs. The table
holds no shared-memory addresses, so it is valid in every process.

Thread Safety
+++++++++++++

The broker is the single consumer of the PDU ID queue. ``onReceive()`` and
``processTransmissions()`` must be called from the same context. A PDU that
the broker receives from the bus must not have a writer, because there must be
exactly one writer per PDU (see `Middleware PDU`_).

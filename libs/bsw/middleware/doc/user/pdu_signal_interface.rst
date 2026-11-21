.. *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
  *******************************************************************************

PDU Signal Interface
====================

Overview
--------

The middleware PDU signal interface maps typed signals to and from a PDU byte
payload. A deployment defines each PDU's payload size and signal
bit layout.

The code generator also creates a typed reader that snapshots the
shared-memory PDU, runs the configured qualifier on the snapshot,
and returns all decoded signals together.

PDU Configuration
-----------------

Each configured signal specifies its C++ type and bit
length. Only ``BigEndian`` byte order and ``MSB-first`` bit ordering
is supported at the moment.

``pdu_id`` is an optional, unique deployment value (``uint32_t``) under which a
PDU is announced to the PDU broker. A PDU without ``pdu_id`` is not
transmitted: it cannot have a ``transmission``.

``outbound_pdu_queue_size`` is required when at least one PDU has a ``pdu_id`` and
must be between 1 and 255.

.. code-block:: yaml

   outbound_pdu_queue_size: 10

   pdus:
     - name: SamplePdu
       pdu_id: 256
       size: 2
       namespace: example::signal
       headers:
         common: example/signal/SamplePduCommon.h
         reader: example/signal/SamplePduReader.h
         writer: example/signal/SamplePduWriter.h
       signals:
         - name: checksum
           type: uint8_t
           bit_length: 8
         - name: value
           type: uint8_t
           bit_length: 8

Supported datatypes
-------------------

The following signal kinds are supported:

.. list-table::
   :header-rows: 1
   :widths: 18 32 50

   * - Kind
     - ``type``
     - Generated member
   * - Unsigned integer
     - ``uint8_t``, ``uint16_t``, ``uint32_t``, ``uint64_t``
     - A member of that type. ``bit_length`` must not exceed the width of the
       type.
   * - Boolean
     - ``bool``
     - A single-bit flag (``bit_length: 1``).
   * - Byte/word array
     - ``array`` plus ``element_type``
     - ``etl::array<element_type, bit_length / element width>``. The element order of the PDU
       is preserved; elements are not byte-swapped.
   * - Enum
     - Name of an entry in the PDU's ``enums``
     - A scoped ``enum class`` with the configured underlying type.
   * - Bitset
     - Name of an entry in the PDU's ``bitsets``
     - A generated class with one ``test<Name>()`` accessor per named bit.

Signed integers and floating-point types are not supported.

Generation rejects signals that exceed the payload, arrays with partial
elements, booleans wider than one bit, and unknown named types. Enum and bitset
signal widths must fit their unsigned underlying type and contain every
declared value or bit. Reader and common headers are required; mock generation
also requires ``headers.reader_mock``.

Enum signals are converted with a plain ``static_cast``. The raw value is not
checked against the declared enumerators, so a received value that is not
listed still yields an ``enum class`` object holding that number.

Phy translation of signals
--------------------------

An unsigned integer signal can carry an optional ``phy_translation`` that maps
the raw value to a physical value::

   physical = raw * scaling_factor + offset

.. code-block:: yaml

   signals:
     - name: temperature
       type: uint16_t
       bit_length: 12
       phy_translation:
         offset: -40.0
         scaling_factor: 0.1

For every such signal the generated PDU struct gets a method
``<signalName>ToPhysical()`` that returns a ``double``:

.. code-block:: cpp

   auto const sample = reader.read();
   if (qualifier.getQualification() == Qualifier::Qualification::Ok)
   {
       double const celsius = sample.temperatureToPhysical();
   }

The generated writer also provides ``write<SignalName>FromPhysical(double)``.
It applies the inverse conversion:

.. code-block:: cpp

   writer.writeTemperatureFromPhysical(25.0);
   writer.commit();

The conversion is ``raw = (physical - offset) / scaling_factor``. Results below
zero saturate to zero; results above the signal's wire range saturate to its
maximum value, including infinite results. NaN leaves the staged value
unchanged and does not request a transmission. In-range fractional results
are truncated, not rounded; floating-point arithmetic can affect values near
integer boundaries. The ordinary raw setter stages the result without
committing it. Call ``commit()`` separately to store the payload. Both
coefficients must be finite, and ``scaling_factor`` must be nonzero.

The signal member itself always keeps the raw value. The reader-side
``ToPhysical()`` conversion is computed on demand and applies no range check,
so qualify the PDU before converting. ``phy_translation`` is only available
for unsigned integer signals, not for arrays, enums or bitsets.

Signal Bit Mapping
------------------

``bit_length`` is the number of bits in the signal.
Lengths are compile-time inputs to the generated read and write operations.

In a big-endian pdu, the most significant byte comes first as well as the
signal bits inside each byte, i.e., bits go from the most-significant to the
least-significant bit within each byte, then continue at the next byte.

For example, a 16-bit signal reads bytes ``AD BE`` as ``0xADBE``
in big-endian mode. A byte-array signal preserves the PDU's byte sequence;
it is not converted to an integer or byte-swapped.

Generated Reader and Qualification
----------------------------------

Each PDU has a generated reader named ``<PduName>Reader``. Its constructor
takes the application's ``middleware::core::IQualifier*``. Its ``read()``
method returns a snapshot of all decoded signals. Check the qualification
before using any returned signal value:

.. code-block:: cpp

   Qualifier qualifier;
   SamplePduReader reader{&qualifier};

   auto const result = reader.read();
   if (qualifier.getQualification() != Qualifier::Qualification::Ok)
   {
       return;
   }
   use(result.value);

Unit testing with the reader mock
---------------------------------

When a PDU is generated with ``generation_mode: mock`` (or the generator runs
with ``--generation-mode mock``), ``<PduName>Reader`` forwards ``read()`` to a
GMock-based ``<PduName>ReaderMock``. The mock lives in the same namespace as
the reader and registers itself as the active mock on construction:

.. code-block:: cpp

   NiceMock<SamplePduReaderMock> readerMock{};
   SamplePduReader reader{&qualifier};

   readerMock.ON_CALL_READ_RETURNS(SamplePduReaderMock::ReadResult{expected});
   readerMock.EXPECT_READ();

   auto const result = reader.read();

If any service uses mock generation, the deployment is interface-only: all
services and PDUs must use mock generation. Mixing normal services or PDU
readers/writers with mock services is rejected, because their transport
infrastructure is not generated. Scheduled writers cannot use mock generation.
``--generation-mode mock`` supplies defaults; it does not override explicit
per-service or per-PDU modes. Mock ``Config.h`` only forward-declares the
transport memory layout and does not provide shared-memory storage.

Generated Writer
----------------

A typed writer is generated when ``headers.writer`` is configured for the PDU.
It stages signal values and stores them in shared memory when ``commit()``
is called. Use ``write()`` to replace all staged values, or the generated
per-signal methods to update individual values while leaving the others
unchanged. The writer constructor takes the initial signal values; the default
constructor stages every signal with all bits set (for example ``0xFF`` for an
8-bit signal), like the reset state of the shared PDU.

Neither constructor writes shared memory. Initial values remain staged
until ``commit()`` serializes them; a cluster tick does not commit them.
For a scheduled writer, call ``commit()`` after construction and before
the first cluster tick if the initial values must be available for the
first notification. An initially active cyclic strategy queues the PDU
ID on that tick even without a commit. The broker reads the shared
payload, not the writer's staged values: this is the reset payload if
nothing has committed it yet, or the last committed payload otherwise.

.. code-block:: cpp

   example::signal::SamplePduWriter writer{
     example::signal::SamplePdu{0U, 0U}};
   writer.writeChecksum(0x12U);
   writer.writeValue(0x34U);
   writer.commit();

``commit()`` only stores the payload in shared memory; announcing the PDU to the
broker is controlled separately by ``transmission``. A writer cannot be copied or
moved, and only one writer may exist for each PDU.

Automatic Transmission
----------------------

Configure ``transmission`` (which requires a ``pdu_id`` and a
``writer_cluster``) to announce the PDU to the broker according to the configured
strategies. The announcement carries only the ``pdu_id``, so call ``commit()``
after staging values and before the cluster's next tick. ``writer_cluster`` names
the cluster that drives the writer: ``process<Cluster>Cluster()``, called once per
millisecond, runs its strategies.

``triggerPduEvent()``, ``setActiveCondition(bool)`` and ``trigger_on_change``
setters only request a send; the ID is queued on a cluster tick when the strategy
permits it. If the outbound queue is full, the ID is dropped and counted in the
queue statistics.

.. code-block:: yaml

   writer_cluster: Cluster0
   transmission:
     modeFalse:
       strategies:
         - cyclic: {ms: 100}
         - event: {debounce_ms: 20, repetitions: 2, repetition_cycle: 50}
     modeTrue:
       strategies: []

``modeFalse`` is active initially, and ``setActiveCondition(bool)`` switches
between ``modeFalse`` and ``modeTrue``.

* **Cyclic**: Queues the PDU ID on the first update, on mode entry, and every ``ms`` ticks.
* **Event**: Records ``triggerPduEvent()`` (or a ``trigger_on_change`` signal
  change), including during debounce. A cyclic tick queues the ID once
  ``debounce_ms`` has elapsed since the previous initial event notification.
  Multiple pending events coalesce into one notification of the latest committed
  payload. This notification starts a new sequence of ``repetitions`` additional
  sends, spaced by ``repetition_cycle`` ticks. Repetitions do not restart debounce.
  A new event and a due repetition on the same tick produce one notification and
  restart the repetition sequence. Both repetition fields must be zero or both
  positive.
* **Empty list**: Disables automatic ID notifications.

Changing modes cancels the previous mode's pending events and repetitions,
including any pending cyclic-mode-entry notification. An unchanged mode condition
does not cancel pending work. Inactive modes' debounce timers remain frozen.

The durations ``ms``, ``debounce_ms`` and ``repetition_cycle`` are counted in
writer ticks, which are milliseconds when the cluster calls ``cyclic()`` once
per millisecond.

``ms`` must be positive. ``debounce_ms`` may be zero; ``repetition_cycle`` may
be zero only when ``repetitions`` is also zero. Finite durations end at 65534
ticks. For compatibility, 65535 is a non-expiring timer value: a cyclic
strategy still sends initially and on mode entry, but has no periodic sends
afterward;
an event strategy with this debounce sends its first event but blocks later
events in that mode; a repetition cycle of 65535 produces no repetitions.

Strategies execute in configured order on every tick, even if an earlier
strategy requests a send. Their send requests are combined into one PDU ID
notification for that tick, so all strategy timers continue to advance.

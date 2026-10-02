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

.. code-block:: yaml

   pdus:
     - name: SamplePdu
       size: 2
       namespace: example::signal
       headers:
         common: example/signal/SamplePduCommon.h
         reader: example/signal/SamplePduReader.h
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

The signal member itself always keeps the raw value. The conversion is computed
on demand and applies no range check, so qualify the PDU before converting.
``phy_translation`` is only available for unsigned integer signals, not for
arrays, enums or bitsets.

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

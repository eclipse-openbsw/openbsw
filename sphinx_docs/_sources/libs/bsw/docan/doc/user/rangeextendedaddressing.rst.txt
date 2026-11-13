..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _docan_range_extended_addressing:

Range-Based Extended Addressing
===============================

Overview
--------

``DoCanRangeExtendedAddressingFilter`` is a variant of
``DoCanExtendedAddressingFilter`` for the special case where a *contiguous*
range of CAN identifiers maps arithmetically onto a *contiguous* range of
transport (``N_TA``) addresses:

.. code-block:: text

   transportId = canId - baseCanId + baseTransportId

The Extended Addressing format itself (AE byte, source/target semantics,
functional rules) is defined by **ISO 15765-2**; see
:ref:`docan_extended_addressing`. This page documents only the range variant.

Instead of one lookup-table entry per participant, the block is described by
three values (base CAN id, base transport address, count), giving a constant,
table-free footprint and O(1) translation. The filter implements the same
interfaces as the table-based Extended filter (``IDoCanAddressConverter`` plus a
CAN acceptance filter) and is the converter for a dedicated *range-extended*
transport layer, one of the four aggregated by
``DoCanMultiAddressingTransportLayer``. It replaces the table-based filter only
for the layer that owns it; the other layers are untouched.

.. list-table:: Table vs range Extended filter
   :header-rows: 1
   :widths: 30 35 35

   * - Property
     - ``DoCanExtendedAddressingFilter`` (table)
     - ``DoCanRangeExtendedAddressingFilter`` (range)
   * - Address layout
     - Arbitrary / sparse
     - Contiguous, uniformly mapped
   * - Storage
     - One entry per node
     - Three scalars (base id, base N_TA, count)
   * - CAN acceptance
     - ``BitFieldFilter``
     - ``IntervalFilter`` (single interval)
   * - Lookup cost
     - Search over the table
     - Arithmetic (add / subtract)
   * - Best fit
     - A few non-adjacent ECUs
     - A dense block of ECUs

Address Translation
-------------------

CAN id and transport address are derived from one another via the base offsets,
with a bounds check against ``count``:

* ``canIdToTransportId``: ``baseTransportId + (canId - baseCanId)``, valid for
  ``baseCanId <= canId < baseCanId + count``.
* ``transportIdToCanId``: ``baseCanId + (transportId - baseTransportId)``, valid
  for ``baseTransportId <= transportId < baseTransportId + count``.

The mapping is symmetric (id and address differ by a constant offset). CAN
acceptance uses ``IntervalFilter``, initialised in ``init()`` from the same
``count`` as ``[baseCanId, baseCanId + count - 1]`` - so the accepted set is by
construction identical to the translatable set and the two cannot drift apart.

Functional Addressing
---------------------

Functional targets may be flagged via the ``functionalAddresses`` slice passed
to ``init()``; handling follows ISO 15765-2 (single-frame only, functional
target reports an **invalid transmission address** so ``DoCanReceiver`` rejects a
multi-frame functional request).

.. note::

   Extended Addressing on 11-bit ids has no legislative broadcast id; for
   physical point-to-point use the functional slice is left empty. The Normal
   Fixed functional-broadcast remapping in
   ``DoCanMultiAddressingTransportLayer`` applies only to the Normal Fixed layer.

Configuration Constraints
-------------------------

``init()`` asserts (start-up traps on violation):

* ``count`` is non-zero.
* Range fits an 11-bit base id:
  ``baseCanId + (count - 1) <= CanId::MAX_RAW_BASE_ID``.
* Range fits one AE byte: ``baseTransportId + (count - 1) <= 0xFF``.

.. _docan_range_extended_layer_integration:

Layer Integration
-----------------

There is no composite converter. The range filter is the
``IDoCanAddressConverter`` of a dedicated range-extended layer, bound as the
**third** of four layers. The ``init()`` signature is unchanged from the
standalone filter:

.. code-block:: cpp

   // Demo block: CAN ids 0x600..0x60F <-> transport ids 0x20..0x2F.
   uint32_t const RANGE_EXTENDED_BASE_CAN_ID       = 0x600U;
   uint16_t const RANGE_EXTENDED_BASE_TRANSPORT_ID = 0x020U;
   uint16_t const RANGE_EXTENDED_RANGE_COUNT       = 0x010U; // 16 nodes

   _rangeExtendedAddressingFilter.init(
       static_cast<DataLinkLayerType::AddressType>(RANGE_EXTENDED_BASE_CAN_ID),
       RANGE_EXTENDED_BASE_TRANSPORT_ID,
       RANGE_EXTENDED_RANGE_COUNT,
       ::etl::span<uint16_t const>{}, // no functional targets on this range
       _rangeExtendedCodec);

   _multiAddressingTransportLayer.bind(
       _normalAddressingLayer,
       _extendedAddressingLayer,
       _rangeExtendedAddressingLayer, // backed by _rangeExtendedAddressingFilter
       _normalFixedAddressingLayer);

No static ``AddressEntryType[]`` table is required for this layer.

.. note::

   Type aliases and member names follow the project's ``DoCanSystem.h`` /
   ``DoCanSystem.cpp`` conventions; the ``init()`` arguments and the ``bind()``
   slot order are the fixed parts.

Frame Routing
-------------

**Reception (RX).** The layer's own ``IntervalFilter``
(``[baseCanId, baseCanId + count - 1]``) is the acceptance gate; any accepted id
is guaranteed to translate (shared ``count``).

**Transmission (TX).** ``DoCanMultiAddressingTransportLayer::send`` selects the
layer by tester id via ``getTransportLayerForTesterId``, a ``switch`` matching a
**single** ``RANGE_EXTENDED_ADDRESSING_TESTER_ID``. An unmatched tester id
yields ``TP_SEND_FAIL`` rather than a silent mis-route.

.. important::

   RX and TX are **asymmetric on purpose**. RX accepts the whole block (any of
   the ``count`` nodes can send here); TX reaches this layer only for the one
   configured ``RANGE_EXTENDED_ADDRESSING_TESTER_ID``. This matches an
   **ECU-side** role (receive from many, reply to one tester). A
   **tester/gateway-side** role originating frames to every node would need a
   range check in ``getTransportLayerForTesterId`` instead of one constant; the
   "dense block" best-fit above describes the *reception* side.

.. important::

   Two invariants keep the multilayer split unambiguous:

   * **Disjoint RX ranges.** The acceptance interval must not overlap any other
     layer's accepted CAN ids on the same bus.
   * **Distinct tester ids.** Each mode uses a distinct tester id, since
     ``getTransportLayerForTesterId`` switches on the target id.

   The acceptance interval is derived from ``count`` inside ``init()``, so it
   cannot be configured wider than the translatable range.

Disjointness
------------

Per-layer stateless routing requires the range-extended CAN ids to be disjoint
from all other layers on the same bus. The integrator asserts this at start-up,
for example:

.. code-block:: text

   Normal      : 0x7DF / 0x7E0 / 0x7E8, plus diag pair 0x2A / 0xF0
   Extended    : table filter's id set
   RangeExt    : [0x600, 0x60F]
   NormalFixed : 29-bit ids (isBase == false)

Disjointness is enforced by the integrator's start-up assertion, not guaranteed
by the layout itself.

Limitations
-----------

* Only a **single contiguous linear range** is supported; sparse maps require
  the table-based ``DoCanExtendedAddressingFilter``.
* The CAN-id and transport-id offsets must be **equal** (symmetric mapping);
  asymmetric request/response layouts are not expressible.
* ``count`` is bounded by both the 11-bit base id space and the 8-bit AE byte.

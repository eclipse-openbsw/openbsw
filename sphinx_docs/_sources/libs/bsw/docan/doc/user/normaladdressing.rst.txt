..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _docan_normal_addressing:

Normal Addressing
=================

Overview
--------

Normal Addressing is an ISO 15765-2 addressing format in which the CAN
identifier alone represents the network address; no addressing information is
stored in the payload.

For the definition of the addressing format and its semantics, refer to
**ISO 15765-2**. This page documents only what is specific to the OpenBSW
implementation.

Implementation:

* ``DoCanNormalAddressing``
* ``DoCanNormalAddressingFilter``

.. note::

   Examples use the project configuration (``NORMAL_ADDRESSING_ADDRESSES``): the
   diagnostic pair ``0x2A`` / ``0xF0``, the legislative ISO 15765-4 pair
   ``0x7E0`` / ``0x7E8``, and the legislative OBD functional request id
   ``0x7DF``. ECU address ``0x2A`` (``LOGICAL_ADDRESS``).

Address Entry Model
-------------------

Each entry defines a bidirectional channel (request and response) and has
**six** fields:

.. code-block:: cpp

   struct AddressEntry
   {
       uint32_t _canReceptionId;      // CAN id this entry receives on
       uint32_t _canTransmissionId;   // CAN id used for the response
       uint16_t _transportSourceId;   // transport source (sender)
       uint16_t _transportTargetId;   // transport target (this node)
       uint8_t  _receptionCodecIdx;   // index into the codec span (RX)
       uint8_t  _transmissionCodecIdx;// index into the codec span (TX)
   };

There is **no** ``_singleFrameOnly`` field; multi-frame functional requests are
rejected via an invalid transmission id (see :ref:`na-functional`).

Example (legislative physical pair): ``RX 0x7E0 / TX 0x7E8``, source
``NORMAL_ADDRESSING_TESTER_ID``, target ``0x2A``. One entry defines both the
request (``0x7E0``: tester -> ``0x2A``) and response (``0x7E8``: ``0x2A`` ->
tester) paths.

CAN Identifier Support
----------------------

Both 11-bit (base) and 29-bit (extended) reception ids are supported and may
coexist in one table. ``init()`` separates them:

* **Base (11-bit)** ids are registered with ``BitFieldFilter::add()``.
* **Extended (29-bit)** ids are collected into ``_extendedEntries`` and matched
  by lookup rather than the bit field.

The project table is currently all 11-bit, but mixing 29-bit entries is
supported.

Address Representation and Format
---------------------------------

The data-link address is the CAN identifier itself: reception decoding returns
``canId``; transmission encoding writes ``canId = transmissionAddress``. No
payload bytes are reserved for addressing (protocol data begins at byte 0).
Reported format: ``DoCanAddressFormat::Normal``. The frame codec is selected per
direction via the entry's ``_receptionCodecIdx`` / ``_transmissionCodecIdx``.

Initialization
--------------

``init()`` accepts:

* ``addressEntries`` - the address table (non-owning span).
* ``codecEntries`` - span of frame codecs indexed by the entry codec indices.

Start-up assertions:

* Address table is non-empty.
* Reception CAN ids are **strictly increasing** (unique) - required for the
  ``etl::lower_bound`` reception lookup.
* Extended (29-bit) reception ids are valid.

.. _na-functional:

Functional Addressing
---------------------

A functional (broadcast) request is an ordinary entry whose **transmission id is
invalid**. Project configuration:

.. code-block:: text

   RX CAN ID = 0x7DF (OBD functional request)
   TX CAN ID = INVALID_ADDRESS
   Source    = NORMAL_ADDRESSING_TESTER_ID
   Target    = FUNCTIONAL_ALL_ISO14229

Per ISO 15765-2 a functional request is single-frame only. Because this entry
reports ``INVALID_ADDRESS`` as the transmission id, ``DoCanReceiver`` rejects a
multi-frame functional request. Each ECU replies physically via its own entry
(``0x7E8`` for ``0x2A``).

Reception / Transmission Mapping
--------------------------------

* ``getReceptionParameters()`` looks up the RX CAN id via
  ``findEntryTypeByReceptionAddress`` (``etl::lower_bound`` over base and
  extended entries), then returns the transport pair
  ``(_transportSourceId, _transportTargetId)``, transmission id
  ``_canTransmissionId`` (possibly ``INVALID_ADDRESS``), and reception codec.
* ``getTransmissionParameters()`` **reverses** the requested transport pair
  (``sourceId = pair.target``, ``targetId = pair.source``), linearly finds the
  matching entry, and builds the data-link pair
  ``(_canReceptionId, _canTransmissionId)`` with the transmission codec. Returns
  ``nullptr`` if no pair matches.

Example: requested pair source ``0x2A`` / target ``NORMAL_ADDRESSING_TESTER_ID``
reverses to source ``NORMAL_ADDRESSING_TESTER_ID`` / target ``0x2A``, matching
``RX 0x7E0 / TX 0x7E8`` -> ``TX CAN ID = 0x7E8``.

Filter and Layer Integration
----------------------------

``DoCanNormalAddressingFilter`` implements ``IDoCanAddressConverter`` and
``can::BitFieldFilter``. ``match()`` dispatches by id kind: base (11-bit) ids via
``BitFieldFilter::match()``, extended (29-bit) ids via
``findEntryTypeByReceptionAddress`` over ``_extendedEntries``.

There is no composite converter. The filter is the ``IDoCanAddressConverter`` of
a dedicated Normal transport layer, bound as the **first** of the four layers of
``DoCanMultiAddressingTransportLayer``:

.. code-block:: cpp

   _multiAddressingTransportLayer.bind(
       normalAddressingLayer,        // backed by _normalAddressingFilter
       extendedAddressingLayer,
       rangeExtendedAddressingLayer,
       normalFixedAddressingLayer);

On transmission, routing is by tester id: both
``NORMAL_ADDRESSING_TESTER_ID`` and ``NORMAL_ADDRESSING_DIAG_TESTER_ID`` map to
the normal layer. ``formatDataLinkAddress()`` prints the raw CAN id as
``0x%08x`` (e.g. ``0x000007e0``).

Implementation Invariants
-------------------------

* Address table is non-empty; reception CAN ids strictly increasing (unique).
* Extended (29-bit) reception ids are valid.
* Data-link address equals the CAN identifier; no payload bytes reserved.
* Reception and transmission mappings remain reversible; transport pairs unique.
* A functional entry uses an invalid transmission id (no per-entry single-frame
  flag; entries have exactly six fields).
* Base ids admitted via ``BitFieldFilter``; extended ids via the extended-entry
  lookup.
* Address format stays ``DoCanAddressFormat::Normal``.
* The filter is bound as the Normal layer of
  ``DoCanMultiAddressingTransportLayer``.

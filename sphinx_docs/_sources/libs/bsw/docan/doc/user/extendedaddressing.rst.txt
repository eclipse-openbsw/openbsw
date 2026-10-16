..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _docan_extended_addressing:

Extended Addressing
===================

Overview
--------

Extended Addressing is an ISO 15765-2 addressing format in which the network
address information is split between the 11-bit CAN identifier and an Address
Extension (AE) byte carried in payload byte 0.

For the definition of the addressing format, the source/target semantics, the
frame structure and the functional-addressing rules, refer to **ISO 15765-2**.
This page documents only what is specific to the OpenBSW implementation.

Implementation:

* ``DoCanExtendedAddressing``
* ``DoCanExtendedAddressingFilter`` (table-based)

.. note::

   All examples use the project configuration: tester transport id ``0xF4``
   (``EXTENDED_ADDRESSING_TESTER_ID``), ECU ``0x2A`` (``LOGICAL_ADDRESS``),
   request CAN id ``0x600``, response CAN id ``0x601``.

Data Link Address Representation
--------------------------------

To use a single internal address type, the CAN identifier and Address Extension
are combined into one packed data-link address (AE in the high 8 bits, CAN id in
the low 11 bits):

.. code-block:: cpp

   pack(canId, addressExtension)   // build
   canIdOf(address)                // decompose
   addressExtensionOf(address)     // decompose

Reported address format: ``DoCanAddressFormat::Extended``.

Codec Preset
------------

.. code-block:: cpp

   DoCanFrameCodecConfigPresets::EA_PADDED_CLASSIC
   {
      {8U, 8U}, {8U, 8U}, {8U, 8U}, {8U, 8U}, // SF / FF / CF / FC
      0xCCU,    // filler
      1U        // offset: byte 0 reserved for the Address Extension (N_TA)
   }

The offset remains ``1U`` (protocol data begins at payload byte 1).

Address Entries
---------------

The filter is configured with a list of ``{ canId, transportId }`` entries, each
associating a CAN identifier with the transport address of the node that
*transmits* on it. Entries must be **ascending by CAN id (duplicates allowed)**;
``init()`` asserts this ordering.

Project configuration (``EXTENDED_ADDRESSING_ADDRESSES``):

.. code-block:: cpp

   {
      { 0x600U, EXTENDED_ADDRESSING_TESTER_ID },   // tester 0xF4 -> request  0x600
      { 0x601U, LOGICAL_ADDRESS },                 // ECU 0x2A    -> response 0x601
      { 0x601U, FUNCTIONAL_ALL_ISO14229 }          // functional entry
   }

The functional entry deliberately reuses ``0x601`` (the ECU's real response id)
so any Flow Control for a multi-frame functional exchange is emitted on the
correct CAN id. The physical and functional entries on ``0x601`` are
disambiguated by the Address Extension byte, not the CAN id. Functional addresses
are passed to ``init()`` as a non-owning span
(``EXTENDED_ADDRESSING_FUNCTIONAL_ADDRESSES = { FUNCTIONAL_ALL_ISO14229 }``).

Functional handling follows ISO 15765-2: single-frame only. A functional target
causes ``getReceptionParameters()`` to report ``INVALID_ADDRESS``, so
``DoCanReceiver`` rejects a multi-frame functional request.

Reception / Transmission Mapping
--------------------------------

* ``decodeReceptionAddress()`` returns ``pack(canId, payload[0])``.
* ``getReceptionParameters()`` resolves source by CAN id and target by AE, then
  builds the transport pair ``(sourceEntry.transportId, targetId)``.
* ``encodeTransmissionAddress()`` sets ``canId = canIdOf(address)`` and
  ``payload[0] = addressExtensionOf(address)``.
* ``getTransmissionParameters()`` looks up source/target entries by transport id;
  returns ``nullptr`` if either is not configured.

Example (request, tester ``0xF4`` -> ECU ``0x2A``):

.. code-block:: text

   0xF4 -> 0x600 (source entry), 0x2A -> 0x601 (target entry)
   TX = pack(0x601, 0xF4)   RX = pack(0x600, 0x2A)

Filter and Layer Integration
----------------------------

``DoCanExtendedAddressingFilter`` implements ``IDoCanAddressConverter`` and
``can::BitFieldFilter``. Each configured CAN id is registered via
``BitFieldFilter::add(canId)`` in ``init()``; frame admission is delegated to
``BitFieldFilter::match()``.

There is no composite converter. The filter is the ``IDoCanAddressConverter`` of
a dedicated Extended transport layer aggregated by
``DoCanMultiAddressingTransportLayer``:

.. code-block:: cpp

   _multiAddressingTransportLayer.bind(
       normalAddressingLayer,
       extendedAddressingLayer,      // backed by _extendedAddressingFilter
       rangeExtendedAddressingLayer,
       normalFixedAddressingLayer);

On transmission, routing to this layer is by tester id
(``EXTENDED_ADDRESSING_TESTER_ID``). ``formatDataLinkAddress()`` prints
``0x<canId>/0x<ae>`` in lowercase (e.g. ``0x601/0xf4``).

``init()`` arguments:

* ``addressEntries`` - the ``{canId, transportId}`` table (non-empty, ascending).
* ``functionalAddresses`` - non-owning span (may be empty).
* ``codec`` - ``EA_PADDED_CLASSIC``.

Both spans are non-owning views; the backing arrays must outlive the filter and
are therefore file-local statics.

Implementation Invariants
-------------------------

* Address Extension occupies payload byte 0; protocol data begins at byte 1.
* ``EA_PADDED_CLASSIC`` keeps offset ``1U``.
* Packed addresses carry both CAN id and Address Extension.
* Address entries stay ascending by CAN id (duplicates allowed).
* Reception and transmission mappings remain reversible.
* Filter admission stays delegated to ``BitFieldFilter``.
* Address format stays ``DoCanAddressFormat::Extended``.
* The filter is bound as the Extended layer of
  ``DoCanMultiAddressingTransportLayer``.

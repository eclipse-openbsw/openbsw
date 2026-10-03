..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _docan_normal_fixed_addressing:

Normal Fixed Addressing
=======================

Overview
--------

Normal Fixed Addressing is an ISO 15765-2 addressing format where the complete
network addressing information is encoded in the 29-bit extended CAN identifier;
no payload byte is reserved for addressing. It supports physical (1:1) and
functional (1:n) addressing.

For the definition of the addressing format, the CAN-id field layout and the
functional-addressing rules, refer to **ISO 15765-2**. This page documents only
what is specific to the OpenBSW implementation.

Implementation:

* ``DoCanNormalFixedAddressing``
* ``DoCanNormalFixedAddressingFilter``

.. note::

   All examples are from the **ECU (OpenBSW node)** perspective (receive
   request, transmit response). In this project the tester address is ``0xF3``
   and the ECU (server) address is ``0x2A``.

CAN Identifier Bases and Packing
--------------------------------

* Physical base: ``0x18DA0000UL`` (target byte = destination ECU).
* Functional base: ``0x18DB0000UL`` (target byte = configured group address).

Layout: ``base | (targetAddress << 8) | sourceAddress``. Helpers
``addressingIdBaseOf(canId)``, ``targetAddressOf(canId)``,
``sourceAddressOf(canId)`` extract the fields (all address bytes fit ``0x00``
-``0xFF``).

Reported format: ``DoCanAddressFormat::NormalFixed``. No payload byte is used
for addressing (protocol data starts at ``payload[0]``). Codec preset:
``DoCanFrameCodecConfigPresets::PADDED_CLASSIC``, offset ``0U``.

Physical and Functional Examples
--------------------------------

Physical (tester ``0xF3`` <-> ECU ``0x2A``), multi-frame allowed:

.. code-block:: text

   Request  CAN ID: 0x18DA2AF3  (0x18DA | target 0x2A | source 0xF3)
   Response CAN ID: 0x18DAF32A  (0x18DA | target 0xF3 | source 0x2A)

Functional (group ``0x33``), single-frame only:

.. code-block:: text

   Request  CAN ID: 0x18DB33F3  (0x18DB | group 0x33 | source 0xF3)
   Response CAN ID: 0x18DAF32A  (physical response from ECU 0x2A)

Each responding ECU uses **its own physical address** as the response source;
the functional group address is never used as the source.

Source (Tester) Allow-List
--------------------------

By default the filter is gateway-generic: any source (tester) address is
accepted. Optionally, an allow-list of source bytes may be passed to ``init()``
(see :ref:`nf-init-parameters`). When non-empty, only requests whose source is
listed are accepted (physical and functional alike); every other source is
silently dropped at reception (``getReceptionParameters()`` returns ``nullptr``)
- no response, no routing to any other scheme. When empty, accept-all is
preserved.

This lets a node expose exactly one 29-bit tester identity (here ``0xF3``) while
other tester identities are served only by their 11-bit schemes.

.. code-block:: text

   Allow-list: { 0xF3 }
   RX 0x18DA2AF3 (src 0xF3)   -> accepted, response 0x18DAF32A
   RX 0x18DA2AF1 (src 0xF1)   -> dropped (nullptr)
   RX 0x18DB33F3 (src 0xF3)   -> accepted, response 0x18DAF32A
   RX 0x18DB33F1 (src 0xF1)   -> dropped (nullptr)

The allow-list is a **reception-only** policy; ``getTransmissionParameters()``
does not consult it. It is passed as a caller-owned const view
(``etl::span``) - no dynamic memory, no runtime state, decision computed per
call.

Reception Rules
---------------

``decodeReceptionAddress()`` returns the received CAN id directly.
``getReceptionParameters()`` resolves it in this order (allow-list checked
**before** addressing-mode validation, matching the implementation):

#. Extract base, target/group, source.
#. Apply the optional source allow-list.
#. Validate addressing mode: physical requires base ``0x18DA`` and target **not**
   a group address; functional requires base ``0x18DB`` and target **is** a
   group address; any other base is rejected.
#. Build the transport pair and physical response address.
#. Return codec and single-frame restriction.

.. code-block:: text

   Physical   RX 0x18DA2AF3 -> response 0x18DAF32A, singleFrameOnly=false
   Functional RX 0x18DB33F3 -> response 0x18DAF32A, singleFrameOnly=true

Transmission Rules
------------------

``encodeTransmissionAddress()`` writes the packed address to the CAN id.
``getTransmissionParameters()`` validates source/target and builds the physical
TX/RX ids. Transmission is **always physical** - functional requests are
single-frame only and never use this multi-frame / flow-control path.

.. code-block:: text

   ECU 0x2A responding to tester 0xF3:
   TX = 0x18DAF32A (response)   RX = 0x18DA2AF3 (matching request)

.. _nf-init-parameters:

Initialization Parameters
-------------------------

``init()`` accepts:

* ``functionalAddresses`` - valid group addresses for functional addressing.
* ``allowedTesters`` - optional source (tester) byte allow-list; empty = accept
  any source (gateway-generic), non-empty = only listed sources (physical and
  functional).
* ``codec`` - frame codec.

The filter itself does **not** remap the functional transport address; see
:ref:`nf-functional-remap`.

.. _nf-functional-remap:

Functional Address Remapping (Transport Layer)
----------------------------------------------

Remapping is done one level up, by ``NormalFixedFunctionalAddressRemapper``
inside ``DoCanMultiAddressingTransportLayer``, which wraps the Normal Fixed
layer's provider and listener:

* On reception, a functional target equal to
  ``NORMAL_FIXED_ADDRESSING_FUNCTIONAL_ADDRESS`` is remapped to
  ``TransportConfiguration::FUNCTIONAL_ALL_ISO14229`` before upstream delivery;
  any other target is passed through unchanged.
* The same mapping applies when a transport message is provisioned, so upstream
  consistently sees the ISO 14229 functional address.

This affects only the transport-layer address reported upstream. It must **not**
change the wire-level response CAN id: each ECU still replies physically using
its own physical address as source.

.. code-block:: text

   Functional RX 0x18DB33F3 -> upstream target = FUNCTIONAL_ALL_ISO14229
                            -> wire response    = 0x18DAF32A (physical)

CAN Filtering
-------------

The filter registers the physical and functional ranges:

.. code-block:: cpp

   MaskFilter::add(physicalBase,   physicalBase   | 0xFFFFU)  // 0x18DA0000..0x18DAFFFF
   MaskFilter::add(functionalBase, functionalBase | 0xFFFFU)  // 0x18DB0000..0x18DBFFFF

``match()`` delegates to ``MaskFilter::match()``; detailed validation (group
address, source allow-list) happens later in ``getReceptionParameters()``. The
mask filter accepts any target/source byte; the source allow-list, **when
configured**, is the authoritative source gate. With an empty allow-list,
base/group validation is the only acceptance gate.

``formatDataLinkAddress()`` prints ``0x<target>/0x<source>`` (e.g. ``0x2a/0xf3``,
``0x33/0xf3``).

Implementation Invariants
-------------------------

* Uses 29-bit extended CAN ids; physical base ``0x18DA``, functional base
  ``0x18DB``; no payload byte reserved for addressing.
* Physical requests must not target group addresses; functional requests must.
* Functional requests are single-frame only; responses are physical, using the
  responding ECU's physical address as source (never the group address).
* Empty allow-list = accept any source; non-empty = only listed sources
  (physical and functional). The allow-list gates reception only, never
  transmission.
* Addressing decisions are computed per call, not from cached state.
* Functional remapping is limited to the transport-layer boundary and to
  upstream reporting only (wire response unchanged).
* ``DoCanAddressFormat::NormalFixed`` is reported for this mode.

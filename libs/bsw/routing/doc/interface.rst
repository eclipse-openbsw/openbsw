..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Routing Interface
=================

Router
------

The Router reads PDUs from readers, and writes them to the appropriate writers.

It is configured using a :ref:`routing table<pduRoutingTable>` which describes where the PDUs should
be routed and with which message ID. This table is described in the detailed design, and can be
produced from a high-level jsonl description using the generation tools.

Properties
++++++++++

Given N the number of known PDUs, and M the number of channels, the memory usage range, in bytes, is
given by the following formula:

.. math::

   [4 * (N + 1), 4 * (N + 1) + 5 * M * N]

where the lower bound corresponds to PDUs with no destinations, and the upper bound
corresponds to every PDU being routed to every channel.

For example, for 10 known PDUs and 3 channels, the memory usage will be in the range
:math:`[44, 194]` bytes.

Template Parameters
+++++++++++++++++++

.. literalinclude:: ../include/routing/Router.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

Public API
++++++++++

.. literalinclude:: ../include/routing/Router.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

RxAdapter
---------

The RxAdapter is an adapter that extracts PDUs from messages in the provided
IReader, exposing them with the IReader interface.
It counts the PDUs and bytes extracted from the messages.

Its configuration is also achieved using an appropriate :ref:`table<rxAdapterTable>`.

Properties
++++++++++

Given N the number of known PDUs, M the number of channels, L the number of messages with
explicit configured lengths, and P the number of known messages, the total memory usage of all
adapters, in bytes, is given by the following formula:

.. math::

   8 * M + 8 * P + 4 * L + 8 * N

For example, for 10 known PDUs, 3 channels, 5 known messages, and 2 configured message lengths,
the memory usage will be 152 bytes.

Template Parameters
+++++++++++++++++++

.. literalinclude:: ../include/routing/PduTransportRxAdapter.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

.. literalinclude:: ../include/routing/LegacyRxAdapter.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

Public API
++++++++++

.. literalinclude:: ../include/routing/RxAdapter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

.. literalinclude:: ../include/routing/PduTransportRxAdapter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

.. literalinclude:: ../include/routing/LegacyRxAdapter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

PduTransportTxAdapter
---------------------

The PduTransportTxAdapter is an adapter that forwards PDUs to an IWriter, exposing an IWriter interface
itself.
It collects statistics about the size and number of PDUs forwarded.

Public API
++++++++++
.. literalinclude:: ../include/routing/PduTransportTxAdapter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

LegacyTxAdapter
---------------

The LegacyTxAdapter is an adapter that forwards PDUs to an IWriter, exposing an IWriter
interface itself. It is meant to be used for legacy buses, like CAN or FlexRay, where the incoming
PDU needs to be sent in a specific position in an outgoing PDU.

Its configuration is achieved using an appropriate :ref:`table<txAdapterTable>`.

Public API
++++++++++
.. literalinclude:: ../include/routing/LegacyTxAdapter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :language: cpp
    :dedent: 4

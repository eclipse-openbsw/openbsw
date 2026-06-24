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



..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Configuration
=============

A blob is a contiguous, self-describing binary region. All multi-byte fields are stored in
big-endian byte order. The region begins with a fixed-size header, followed by the blob data,
which is itself a sequence of configurations.

Blob layout
-----------

.. list-table::
   :header-rows: 1
   :widths: 15 15 70

   * - Field
     - Size
     - Description
   * - ``version``
     - 4 bytes
     - Blob format version. Currently ``0x00000001``.
   * - ``magic``
     - 4 bytes
     - Magic number identifying a blob. Always ``0xDEADBEEF``.
   * - ``size``
     - 4 bytes
     - Size, in bytes, of the blob data that follows the header.
   * - ``data``
     - ``size``
     - Concatenated configurations (see below).

The header is consumed by ``checkHeader()`` (see :ref:`blob` APIs), which verifies the version and
magic number and records the declared data size. The ``Blob`` constructor additionally checks that
the actual memory region matches ``HEADER_SIZE + size``.

Configuration layout
--------------------

Each configuration inside the blob data has its own header followed by a payload. The ``type`` field
identifies how the payload should be interpreted.

.. list-table::
   :header-rows: 1
   :widths: 15 15 70

   * - Field
     - Size
     - Description
   * - ``type``
     - 4 bytes
     - Configuration type identifier (see *Configuration types* below).
   * - ``reserved``
     - 8 bytes
     - Reserved space, i.e., configuration-specific data that is not interpreted by the blob module.
   * - ``size``
     - 4 bytes
     - Size, in bytes, of the configuration ``data`` (including padding and the trailing CRC).
   * - ``data``
     - ``size``
     - Configuration payload, optionally padded, ending with a 4-byte CRC.

The last four bytes of every configuration ``data`` hold a big-endian CRC computed over the full
configuration entry bytes except these final CRC bytes. In other words, the CRC input starts at
``type`` and includes ``reserved``, ``size``, and the payload bytes. The CRC is verified by
``checkCrc()``, which re-computes a 32-bit CRC using ``etl::crc32`` (standard CRC-32). The payload
is padded with ``0xFF`` bytes so that its length is a multiple of four before the CRC is appended.

Configuration types
-------------------

The supported configuration types are defined by the ``ConfigType`` enum:

.. list-table::
   :header-rows: 1
   :widths: 25 20 55

   * - Type
     - Value
     - Description
   * - ``ROUTING``
     - ``0x00``
     - PDU routing table.
   * - ``RX_ADAPTER``
     - ``0x01``
     - RX adapter table.
   * - ``TX_ADAPTER``
     - ``0x02``
     - TX adapter table.
   * - ``CHANNEL``
     - ``0xCC``
     - Channel configuration.
   * - ``CHANNEL_NAMES``
     - ``0xDD``
     - Channel name configuration.
   * - ``META``
     - ``0xFE``
     - Metaconfiguration: a list of descriptive strings about other configurations.
   * - ``UNKNOWN``
     - ``0xFFFFFFFF``
     - Sentinel for an unrecognized configuration type.

A byte-level example of a complete blob containing a ``CHANNEL_NAMES`` and a ``ROUTING``
configuration is given in :doc:`examples`.
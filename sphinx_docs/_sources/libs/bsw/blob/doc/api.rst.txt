..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

APIs
====

.. sourceinclude:: include/blob/Blob.h
    :start-after: [Load]
    :end-before: [Load]

Before performing any operation on the blob data, it is important to validate this data.
This is enabled by ``checkHeader()``. Called during blob loading and initialization,
``checkHeader()`` checks the version and magic number of the blob, and assigns the blob's size.
This size is later checked in the constructor of ``Blob``.

.. sourceinclude:: include/blob/Header.h
    :start-after: [HeaderCheck]
    :end-before: [HeaderCheck]

The ``checkCrc()``, also called during blob loading, uses ``etl::crc32`` (standard CRC-32).
It re-computes the CRC on the configuration's data and compares it against the previously
computed CRC bytes at the end of the configuration's data.

.. sourceinclude:: include/blob/util.h
    :start-after: [CrcCheck]
    :end-before: [CrcCheck]
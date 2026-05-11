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
This is enabled by the ``checkHeader()``. Called during blob loading and initialization,
the ``checkHeader()`` checks version and magic number of the blob, and assigns blob’s size.
This size is later checked in the constructor of ``Blob``.

.. sourceinclude:: include/blob/Header.h
    :start-after: [HeaderCheck]
    :end-before: [HeaderCheck]

The ``checkCrc()``, also called during blob loading, uses the ``crc`` utility module, see
:ref:`util_crc`. It re-computes CRC on Config's data and compares against previously computed
CRC bytes at the end of Config's data.

.. sourceinclude:: include/blob/util.h
    :start-after: [CrcCheck]
    :end-before: [CrcCheck]
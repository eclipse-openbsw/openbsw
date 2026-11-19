..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _io_BufferedWriter:

io::BufferedWriter
==================

The ``BufferedWriter`` is a helper class implementing :ref:`io_IWriter` allowing a buffered
transfer of smaller slices using an :ref:`io_IWriter` which provides these buffers. Using a
``BufferedWriter`` only makes sense, if the chunks of data being transferred via this channel
are much smaller compared to the maximum allocation size of this channel.
A typical use case would be transmission of multiple 8-byte CAN frames via UDP packets of e.g.
1400 bytes.

Properties
----------

* Internally the ``BufferedWriter`` manages slices of ``destination.maxSize()`` bytes and
  provides subslices of it to the user.
* Call to ``flush()`` required to commit currently buffered data.
* **Memory consumption**: ``sizeof(::io::IWriter&) + sizeof(::etl::span<uint8_t>) + sizeof(size_t)``

Public API
----------

The public API of ``BufferedWriter`` consists of a constructor and the inherited ``IWriter`` API:

.. sourceinclude:: include/io/BufferedWriter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

Usage example
-------------

Here is an example:

.. sourceinclude:: examples/BufferedWriterExample.cpp
    :start-after: EXAMPLE_BEGIN BufferedWriter
    :end-before: EXAMPLE_END BufferedWriter
    :emphasize-lines: 45
    :linenos:

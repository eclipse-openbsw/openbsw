.. _io_ForwardingReader:

io::ForwardingReader
====================

The ``ForwardingReader`` is a helper class implementing :ref:`io_IReader`, allowing to read
data from a source instance of a reader while forwarding a copy of the data to a destination
:ref:`io_IWriter`. This is useful to fork a input data stream e.g. to a debug writer while
consuming it. In contrast to :ref:`io_SplitWriter`, which splits a data stream at the writer
side, the ``ForwardingReader`` allows forking a data stream at the reader side of a channel.

Properties
----------

* **Memory consumption**: ``sizeof(IReader&) + sizeof(IWriter&) + (sizeof(::etl::span<uint8_t>) * 2)``

Public API
----------

The public API of ``ForwardingReader`` consists of a constructor and the inherited :ref:`io_IReader`
API:

.. sourceinclude:: include/io/ForwardingReader.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

Usage Example
-------------

The following example shows a simplified usage of ``ForwardingReader``:

.. sourceinclude:: examples/ForwardingReaderExample.cpp
    :start-after: EXAMPLE_BEGIN ForwardingReader
    :end-before: EXAMPLE_END ForwardingReader
    :emphasize-lines: 20
    :linenos:

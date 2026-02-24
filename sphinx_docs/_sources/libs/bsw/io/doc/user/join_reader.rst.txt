.. _io_JoinReader:

io::JoinReader
==============

The ``JoinReader`` is an adapter class implementing :ref:`io_IReader`. It takes multiple readers
and provides one unified interface to them. This is useful for connection e.g. multiple input
channels with one output channel.

Properties
----------

* **Memory consumption**: ``sizeof(::etl::span<::io::IReader*, N>) + sizeof(size_t) + 1``

Template Parameters
-------------------

``JoinReader`` is a class template with the following parameters:

.. sourceinclude:: include/io/JoinReader.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

Public API
----------

The public API of ``JoinReader`` consists of a constructor and the inherited :ref:`io_IReader` API:

.. sourceinclude:: include/io/JoinReader.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

Usage Example
-------------

The following example shows a simplified usage of ``JoinReader``:

.. sourceinclude:: examples/JoinReaderExample.cpp
    :start-after: EXAMPLE_BEGIN JoinReader
    :end-before: EXAMPLE_END JoinReader
    :linenos:


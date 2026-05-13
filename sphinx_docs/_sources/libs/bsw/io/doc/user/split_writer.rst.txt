.. _io_SplitWriter:

io::SplitWriter
===============

The ``SplitWriter`` is an adapter class implementing :ref:`io_IWriter`. It takes multiple writers
and provides one unified interface to them writing a copy of the committed data to each of them.
This is useful for connection e.g. one input channel to multiple output channels.

Properties
----------

* **Memory consumption**: ``sizeof(::etl::span<::io::IWriter*, N>) + sizeof(::etl::span<uint8_t>) + sizeof(size_t)``

Template Parameters
-------------------

``SplitWriter`` is a class template with the following parameters:

.. sourceinclude:: include/io/SplitWriter.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

Public API
----------

The public API of ``SplitWriter`` consists of a constructor and the inherited :ref:`io_IWriter` API:

.. sourceinclude:: include/io/SplitWriter.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

Usage Example
-------------

The following example shows a simplified usage of ``SplitWriter``:

.. sourceinclude:: examples/SplitWriterExample.cpp
    :start-after: EXAMPLE_BEGIN SplitWriter
    :end-before: EXAMPLE_END SplitWriter
    :linenos:

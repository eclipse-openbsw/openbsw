.. _io_VariantQueue:

io::VariantQueue
================

Description
-----------

The idea of a "variant queue" here is to encode multiple frame types with attached
dynamically-sized payloads and transmit them via :ref:`io_MemoryQueue`. This allows
more flexibility and better memory utilization than typed ``spsc::Queue`` + fixed-size frames.

``VariantQueue`` only implements serialization/deserialization, not a different queue type.
Underneath, :ref:`io_MemoryQueue` is used.

Encoding looks like this:

.. table::
    :align: left
    :widths: auto

    ======== ================ ================
    uint8    N bytes          M bytes
    type id  frame/header     payload
    ======== ================ ================

.. note::

    Structs used as headers need to be POD. Unless unaligned read/write is acceptable on the
    target platform, they should also have 1-byte alignment requirement. Helper structs:
    ``etl::unaligned_type<T, etl::endian::big>``, ``etl::unaligned_type<T, etl::endian::little>``
    can be used for serializing integers.  Convenience typedefs are: ``etl::be_uint32_t``,
    ``etl::le_uint32_t`` etc. (defined in ``etl/unaligned_type.h``).

Example
-------

This is how you declare a ``VariantQueue`` which can encode types ``A`` and ``B``:

.. sourceinclude:: examples/VariantQueueExample.cpp
   :start-after: EXAMPLE_START declare
   :end-before: EXAMPLE_END declare

To write to the queue, you have several options (with or without payload, passing the payload
directly or filling it in when memory is already allocated from the queue) - see different overloads
of the ``::io::variant_q::write()`` function.

When the queue is full and an element cannot be written, ``write()`` will return false.

.. sourceinclude:: examples/VariantQueueExample.cpp
   :start-after: EXAMPLE_START write
   :end-before: EXAMPLE_END write
   :dedent: 4

To read from a queue and dispatch elements of different types you need to implement a visitor,
similar to when you use etl::variant. You can choose to read header structs
only:


.. sourceinclude:: examples/VariantQueueExample.cpp
   :start-after: EXAMPLE_START read_no_payload
   :end-before: EXAMPLE_END read_no_payload
   :dedent: 4

Or read them with payload:

.. sourceinclude:: examples/VariantQueueExample.cpp
   :start-after: EXAMPLE_START read_with_payload
   :end-before: EXAMPLE_END read_with_payload
   :dedent: 4

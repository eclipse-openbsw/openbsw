.. _io_iwriter:

io::IWriter
===========

The interface ``IWriter`` is an abstraction to write chunks of bytes with variable size to
a communication channel.

Public API
----------

.. sourceinclude:: include/io/IWriter.h
   :start-after: PUBLICAPI_START
   :end-before: PUBLICAPI_END
   :dedent: 4

Usage of API
------------

The interface ``IWriter`` provides a two step API. The user first needs to ``allocate`` a
etl::span of bytes. After a successful allocation, the slice can be filled with the
data to be transferred to the reader side of the stream. Calling ``commit`` makes the data available
to the :ref:`io_IReader` connected to the same channel. It also makes the allocated data invalid
- **it must not be modified after committing!**

.. uml::
    :scale: 100%

    actor SW
    SW -> IWriter : auto d = allocate(size)
    alt d.size() > 0 case
        SW -> SW  : fill d
        SW -> IWriter : commit()
    end

Example
+++++++

The following example shows, how the allocated memory can be used to directly emplace the data
which shall be transferred into it.

.. sourceinclude:: examples/MemoryQueueExample.cpp
   :start-after: EXAMPLE_BEGIN IWriter
   :end-before: EXAMPLE_END IWriter

Multiple Producers
------------------

The two step nature of the interface makes it by default only be usable for a single producer.
Multiple producers using the same writer in parallel lead to race conditions because calling
``allocate()`` would return the same memory to multiple users. A wrapper implementation which
turns the two step API into a single call API makes multiple producers possible, however with
slightly different semantics because data you want to write needs to be present before the call
and cannot be put into the allocated memory on the fly.

Example
+++++++

The following example shows, how with the help of a locking mechanism, the ``IWriter`` interface
can be used to work with multiple producers.

.. sourceinclude:: examples/MemoryQueueExample.cpp
   :start-after: EXAMPLE_BEGIN IWriter_MPSC
   :end-before: EXAMPLE_END IWriter_MPSC

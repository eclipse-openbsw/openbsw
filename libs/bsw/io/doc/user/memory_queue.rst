.. _io_MemoryQueue:

io::MemoryQueue
===============

A ``MemoryQueue`` is a lock free, single producer single consumer queue intended for communication
between software components that share access to the same memory. It provides much better average
case memory usage for use cases where the data elements transported through the queue vary in size.
It's nested classes :ref:`io_MemoryQueue::Writer` and :ref:`io_MemoryQueue::Reader` provide access
to the queue.

Properties
----------

* Lock free, single producer single consumer queue which provides implementations of
  :ref:`io_IWriter` and :ref:`io_IReader` through the adapters :ref:`io_MemoryQueueWriter`
  and :ref:`io_MemoryQueueReader`.
* Any successful allocation will always return a contiguous region of memory.
* A ``MemoryQueue`` is full, if not ``MAX_ELEMENT_SIZE`` bytes can be allocated.
* An allocation can provide between ``1`` and ``MAX_ELEMENT_SIZE`` bytes and will consume
  an extra ``sizeof(SIZE_TYPE)`` bytes to store the allocation size.
* **Memory consumption**: ~ ``CAPACITY + 3 * sizeof(size_t)``

Differences to Other Queues
---------------------------

A typical lock free, single producer single consumer (SPSC) queue class template would provide a
type like ``template<class T, size_t N>class Queue;`` where ``T`` is the type of elements the queue
can handle and ``N`` is the number of elements the ``Queue`` holds. As an example let's look at a
queue of CAN-FD frames, that consist of a 32bit *id* and up to 64 bytes of *payload* so a total of
**68 bytes per frame** at most.

A ``Queue<CanFdFrame, 100>`` would use about 6800 bytes of RAM. Now, the 64 bytes maximum payload
length are a worst case, i.e. it can happen and will happen but it's not the average case. If the
average frame only had 12 bytes of payload a full queue could still only hold 100 frames and only
use 1600 bytes from the total of 6800 bytes. This is a actual **payload rate of roughly 24 percent**
- or in other words **76 percent of RAM are wasted** in the average case.

If we compare this to a ``MemoryQueue<6800, 68>`` this queue can hold at most
``6800 / (68 + 2) == 97`` full frames but in the average case it can hold ``6800 / (16 + 2) == 377``
frames with 12 bytes of payload and not waste any memory.

The ``MemoryQueue`` provides chunks of bytes represented as ``::etl::span<uint8_t>`` and the user
has to take care about serializing and deserializing the transferred data.

Memory Placement / Cache
------------------------

Single-Core
+++++++++++

The ``MemoryQueue`` can be used for interaction between tasks on the same core. In this case it just
needs to be placed in a memory region both tasks can access. As they are on the same core, no
caching effects can occur.

Multi-Core
++++++++++

If the ``MemoryQueue`` is used for interaction between two cores, it needs to be placed in
**non-cached RAM** to avoid caching-effects on the other end of the queue, if the data cache is
enabled on at least one of the using cores.

It has to be ensured that all binaries accessing the queue use the same physical memory location for
the queue, but only one core must initialize the memory and call the constructor.

It is recommended to construct the queue on the main core before all other cores are started. This
prevents accessing the queue too early by design:

.. uml::
    :scale: 100%

    participant core0 order 0
    participant MemoryQueue order 1
    participant core1 order 2

    note over core0, core1: Access to MemoryQueue not allowed yet

    core0 -> MemoryQueue: Placement new

    note over core0, core1: Access allowed from core0

    core0 -> core1: Start core
    core1 -> MemoryQueue: Reinterpret memory region

    note over core0, core1: Access allowed from both cores

    core0 -> MemoryQueue: Access
    core1 -> MemoryQueue: Access

Instantiation
-------------

``MemoryQueue`` is a **class template** with the following parameters:

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: TPARAMS_BEGIN
    :end-before: TPARAMS_END
    :language: none

It can only be **constructed** using it's default constructor.

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: PUBLIC_API_BEGIN
    :end-before: PUBLIC_API_END
    :dedent: 4

Access Through Generic Interfaces
---------------------------------

The generic way of accessing a ``MemoryQueue`` is using the adapter classes
:ref:`io_MemoryQueueReader` and :ref:`io_MemoryQueueWriter`.
They are based on the :ref:`io_IReader` and :ref:`io_IWriter` interfaces with **virtual**
functions, which means that the exact type of the queue does not have to be known.

For intercore communication an instance of :ref:`io_MemoryQueue` is placed into non-cached shared
memory. Two software components placed on different cores can communicate via the queue using
the interfaces :ref:`io_IReader` and :ref:`io_IWriter` by creating instances of
:ref:`io_MemoryQueueReader` and :ref:`io_MemoryQueueWriter`.

.. uml::
    :scale: 100%

    top to bottom direction

    frame core0 {
        actor SWC1
        interface IReader as IReaderC0
        interface IWriter as IWriterC0
        agent MemoryQueueReader as MQRC0
        agent MemoryQueueWriter as MQWC0
    }
    cloud SharedMemory {
        queue MemoryQueue1 as MQ1
        queue MemoryQueue2 as MQ2
    }
    note bottom of SharedMemory : non-cached
    frame core1 {
        actor SWC2
        interface IReader as IReaderC1
        interface IWriter as IWriterC1
        agent MemoryQueueReader as MQRC1
        agent MemoryQueueWriter as MQWC1
    }

    core0 -[hidden] SharedMemory

    MQ2 - MQWC0
    MQ1 - MQWC1
    MQ1 - MQRC0
    MQ2 - MQRC1

    SWC1 -d-( IReaderC0
    MQRC0 -u- IReaderC0
    SWC1 -u-( IWriterC0
    MQWC0 -- IWriterC0

    SWC2 -d-( IReaderC1
    MQRC1 -u- IReaderC1
    SWC2 -u-( IWriterC1
    MQWC1 -- IWriterC1

.. _io_MemoryQueueWriter:

io::MemoryQueueWriter
+++++++++++++++++++++

``MemoryQueueWriter`` is an implementation of :ref:`io_IWriter` providing write access to a
``MemoryQueue``. It is a class template with the queue type as parameter:

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: TPARAMS_MQW_BEGIN
    :end-before: TPARAMS_MQW_END
    :language: none

The public API of ``MemoryQueueWriter`` is the same as for :ref:`io_IWriter`:

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: PUBLIC_API_MQW_BEGIN
    :end-before: PUBLIC_API_MQW_END
    :dedent: 4

Statistics
~~~~~~~~~~

``MemoryQueueWriter``` provides an `minAvailable()` and `resetMinAvailable()` as a way to gather
statistics about the queue's usage.

`minAvailable()` returns the smallest amount of bytes which were available during a call to
`allocate()`.

`resetMinAvailable()` resets this value to the current `available()` value.

Together, these can be used in projects to adapt the queue sizes based on runtime data.

.. _io_MemoryQueueReader:

io::MemoryQueueReader
+++++++++++++++++++++

``MemoryQueueReader`` is an implementation of :ref:`io_IReader` providing read access to a
``MemoryQueue``. It is a class template with the queue type as parameter:

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: TPARAMS_MQR_BEGIN
    :end-before: TPARAMS_MQR_END
    :language: none

The public API of ``MemoryQueueReader`` is the same as for :ref:`io_IReader`

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: PUBLIC_API_MQR_BEGIN
    :end-before: PUBLIC_API_MQR_END
    :dedent: 4

Usage Example
+++++++++++++

The above example using ``MemoryQueueWriter`` and ``MemoryQueueReader`` would look like this:

.. sourceinclude:: examples/MemoryQueueExample.cpp
    :start-after: EXAMPLE_BEGIN IWriterIReader
    :end-before: EXAMPLE_END IWriterIReader
    :emphasize-lines: 7
    :linenos:

And the setup code could look like this:

.. sourceinclude:: examples/MemoryQueueExample.cpp
    :start-after: EXAMPLE_BEGIN IWriterIReader2
    :end-before: EXAMPLE_END IWriterIReader2
    :emphasize-lines: 2,3,6,7,8,9,18
    :linenos:

Access Through Nested Classes
-----------------------------

If performance matters, the ``MemoryQueue`` can also be accessed directly using it's
**nested classes** :ref:`io_MemoryQueue::Reader` and :ref:`io_MemoryQueue::Writer`. This
avoids the virtual functions from the interfaces mentioned above, but the exact type of the queue
has to be known.

For intercore communication an instance of :ref:`io_MemoryQueue` is placed into non-cached shared
memory. Two software components placed on different cores can communicate via the queue by creating
instances of :ref:`io_MemoryQueue::Reader` and :ref:`io_MemoryQueue::Writer` using the non-virtual
API.

.. uml::
    :scale: 100%

    top to bottom direction

    frame core0 {
        actor SWC1
        agent "MemoryQueue::Reader" as MQRC0
        agent "MemoryQueue::Writer" as MQWC0
    }
    cloud SharedMemory {
        queue MemoryQueue1 as MQ1
        queue MemoryQueue2 as MQ2
    }
    note bottom of SharedMemory : non-cached
    frame core1 {
        actor SWC2
        agent "MemoryQueue::Reader" as MQRC1
        agent "MemoryQueue::Writer" as MQWC1
    }

    core0 -[hidden] SharedMemory

    MQ2 - MQWC0
    MQ1 - MQWC1
    MQ1 - MQRC0
    MQ2 - MQRC1

    SWC1 -d- MQRC0
    SWC1 -u- MQWC0

    SWC2 -d- MQRC1
    SWC2 -u- MQWC1

.. _io_MemoryQueue::Writer:

io::MemoryQueue::Writer
+++++++++++++++++++++++

``MemoryQueue::Writer`` provides an API without virtual functions to write data into a
``MemoryQueue``.

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: PUBLIC_API_WRITER_BEGIN
    :end-before: PUBLIC_API_WRITER_END
    :dedent: 8

.. _io_MemoryQueue::Reader:

io::MemoryQueue::Reader
+++++++++++++++++++++++

``MemoryQueue::Reader`` provides an API without virtual functions to read data from a
``MemoryQueue``.

.. sourceinclude:: include/io/MemoryQueue.h
    :start-after: PUBLIC_API_READER_BEGIN
    :end-before: PUBLIC_API_READER_END
    :dedent: 8

Usage Example
+++++++++++++

As ``MemoryQueue`` is a class template, user code might also become templated code as shown in the
following example:

.. sourceinclude:: examples/MemoryQueueExample.cpp
    :start-after: EXAMPLE_BEGIN WriterReader
    :end-before: EXAMPLE_END WriterReader
    :emphasize-lines: 7,8
    :linenos:

The code instantiating reader and writer could look like this:

.. sourceinclude:: examples/MemoryQueueExample.cpp
    :start-after: EXAMPLE_BEGIN WriterReader2
    :end-before: EXAMPLE_END WriterReader2
    :emphasize-lines: 2,3,6,7,8,9,18
    :linenos:

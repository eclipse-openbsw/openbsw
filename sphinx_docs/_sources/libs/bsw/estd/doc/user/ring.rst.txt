.. _estd_ring:

ring
====

Overview
--------


The ``estd::ring`` is a container similar to circular queue,that holds number of
elements ordered in a FIFO (first-in-first-out) sequence. Elements are inserted
at the back using a push operation and are deleted from the front using pop
operation. |br| |br| slice is used to manage the underlying memory, which is
allocated for holding control variables and the remaining is dedicated to the
ring buffer. If the provided memory is insufficient, it returns nullptr.
Additionally, an assertion is triggered when the index passed to the at()
function is out of the bounds. |br| |br| Attempting to push an element when the
ring is at maximum capacity will lead to overwriting the oldest element in the
ring.

Usage
-----
The usage constraints and guidelines apply for ring:

  .. list-table::
    :widths: 30 70

    *  - Constraints
       - Creation of ring with particular size should be within the maximum size
         limit. |br| |br| Returns nullptr if attempt to create is made.
    *  - Usage guidance
       - Size of ring should be checked before accessing the elements. |br| |br|
         Accessing outside the bounds of ring using ``at()`` function will
         result in assertion.

Example
-------

``ring`` is a container similar to circular queue.The operations are performed
based on FIFO (First In First Out) principle.

Usage example to create and perform operation on ring:

.. literalinclude:: ../../examples/ring.cpp
   :start-after: EXAMPLE_RING_CONSTRUCTION_AND_OPERATION_START
   :end-before: EXAMPLE_RING_CONSTRUCTION_AND_OPERATION_END
   :language: c++
   :dedent: 4

The following example shows that pushing an element into the full ring will
overwrite the oldest element in the ring:

.. literalinclude:: ../../examples/ring.cpp
   :start-after: EXAMPLE_RING_PUSH_BACK_START
   :end-before: EXAMPLE_RING_PUSH_BACK_END
   :language: c++
   :dedent: 4


.. _estd_tiny_ring:

tiny_ring
=========

Overview
--------

The ``estd::tiny_ring`` is a linear data structure that follows 'first-in,
first-out' (FIFO) principle but in a circular manner. The class is specifically
tailored for small ring buffers, limiting the maximum size to 255 elements.

Static assertion is thrown if attempt to create tiny_ring with size zero and
with size greater than 256.

Usage
-----

The usage constraints and guidelines apply for tiny_ring:

    .. list-table::
       :widths: 30 70

       *  - Constraints
          - tiny_ring of size greater than 255 cannot be created. |br| |br|
       *  - Usage guidance
          - Size of tiny_ring should be checked before accessing the elements. |br| |br|
            Accessing outside the bounds of tiny_ring using ``data()`` function will result in
            assertion.

Example
-------

The tiny_ring class is specifically tailored for small ring buffers.The
operations are performed based on FIFO (First In First Out) principle.

The example shows the creation of tiny_ring and usage of various functions in
tiny_ring:

.. literalinclude:: ../../examples/tiny_ring.cpp
   :start-after: EXAMPLE_TINY_RING_CONSTRUCTION_AND_OPERATION_START
   :end-before: EXAMPLE_TINY_RING_CONSTRUCTION_AND_OPERATION_END
   :language: c++
   :dedent: 4

The following example shows that pushing an element into the full tiny_ring will
overwrite the oldest element in the tiny_ring:

.. literalinclude:: ../../examples/tiny_ring.cpp
   :start-after: EXAMPLE_TINY_RING_PUSH_BACK_START
   :end-before: EXAMPLE_TINY_RING_PUSH_BACK_END
   :language: c++
   :dedent: 4

The example shows the use of data function:

.. literalinclude:: ../../examples/tiny_ring.cpp
   :start-after: EXAMPLE_TINY_RING_DATA_FUNCTION_START
   :end-before: EXAMPLE_TINY_RING_DATA_FUNCTION_END
   :language: c++
   :dedent: 4
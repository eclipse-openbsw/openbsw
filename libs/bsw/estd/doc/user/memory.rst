.. _estd_memory:

memory
======

Overview
--------

The ``memory`` provides utility functions which operate on any container that can be handled by
slice. It consists of around 20 utility functions which makes our work easier to work on slices.
``set()``, ``copy()``, ``move()``, ``take()``, ``split()``, ``emplace()``, ``make()`` etc are some
of the utility functions.

Usage Context
-------------
.. list-table::
    :widths: 25 75

    * - Preconditions
      - Make sure to include the appropriate library header for memory.
    * - Assumptions of usage
      - The data used can be converted to slice.
    * - Recommended usage guidance
      - The size of the underlying container of a slice should be greater than zero.

Usage Example
-------------
The code snippet below provides the example for the usage of memory:

Following are the examples for the operations on slices:

.. literalinclude:: ../../examples/memory.cpp
   :start-after: EXAMPLE_START
   :end-before: EXAMPLE_END
   :language: c++

Following is the example for comparison operations:

.. literalinclude:: ../../examples/memory.cpp
   :start-after: COMPARISON_EXAMPLE_START
   :end-before: COMPARISON_EXAMPLE_END
   :language: c++


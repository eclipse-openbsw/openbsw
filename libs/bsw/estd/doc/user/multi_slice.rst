.. _estd_multi_slice:

multi_slice
===========

Overview
--------

``estd::multi_slice`` provides pointers to multiple same-sized contiguous sequences of
elements (e.g. slice, array etc.) of different types. It basically helps to combine two or more
different-type but same-sized slices into a single structure.

:ref:`estd_slice` can only refer to a contiguous sequence of elements
(e.g. :ref:`estd_array`, :ref:`estd_vector`, C-array, etc.). :ref:`estd_slice` represents
a view of a contiguous sequence of elements while providing a safe interface
to work with said elements.
``multi_slice`` can only refer to contiguous sequences of elements.

Usage
-----

.. list-table::
  :widths: 30 70

  *  - Constraints
     - ``multi_slice`` can take a maximum of 8 arguments (i.e. Containers). |br| |br|
       Creation with more than eight arguments/containers will result in a compilation error.
  *  - Usage guidance
     - The size of all containers passed to the ``multi_slice`` should be same. |br| |br|
       The size of the resulting ``multi_slice`` will be zero if an attempt to create it
       with containers of different sizes is made.

Example
-------

The following example shows the construction of ``multi_slice`` and various operations that can be
performed on it.

.. literalinclude:: ../../examples/multi_slice.cpp
   :start-after: EXAMPLE_MULTI_SLICE_CONSTRUCTION_AND_OPERATIONS_START
   :end-before: EXAMPLE_MULTI_SLICE_CONSTRUCTION_AND_OPERATIONS_END
   :language: c++





.. _estd_ordered_vector:

estd::ordered_vector
====================

Overview
--------

The ``estd::ordered_vector`` is a fixed-capacity associative container that
represents a sorted multiset of objects of type Key. Sorting is done using the
key comparison function Compare and the default order is ascending order.

Unlike ``std::multiset``, search operations have logarithmic complexity. Removal
and insertion operations have linear complexity in the distance to the end of
the ordered_vector.

Removal and insertion operations cause a memmove of the elements located behind
the target position. This invalidates all the references, pointers, and
iterators referring to the moved elements.

Declare namespace allocates memory whereas ::estd::ordered_vector<type> is a
handler class. The ordered vector from declare namespace has functionalities to
construct, whereas, ordered vector under estd namespace has other functions like
insert, find_or_insert, contains, remove, clear, etc.

Usage
-----

The usage guidelines apply for ordered_vector:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Size of ordered_vector should be checked before accessing the elements. |br| |br|
           Accessing outside the bounds of ordered_vector using ``at()`` and
           ``operator[]()`` function will result in assertion.

Example
-------

The examples for the above functions are given below.

.. literalinclude:: ../../examples/ordered_vector.cpp
   :start-after: EXAMPLE_ORDERED_VECTOR_START
   :end-before: EXAMPLE_ORDERED_VECTOR_END
   :language: c++
   :dedent: 4
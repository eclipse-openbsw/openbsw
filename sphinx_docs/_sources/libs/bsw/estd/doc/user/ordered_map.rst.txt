.. _estd_ordered_map:

estd::ordered_map
=================

Overview
--------

The ``estd::ordered_map`` is a fixed-capacity associative container that contains
key-value pairs with unique keys. Keys are sorted using the comparison function
compare and the default order is ascending order.

Unlike ``std::map``, search operations have logarithmic complexity. Removal
and insertion operations have linear complexity in the distance to the end of
the ordered_map, including ``operator[]`` if the target key is not already
present.

The functions ``insert``, ``emplace``, ``erase`` and ``operator[]`` cause a
memmove of the elements located behind the target position. This invalidates all
the references, pointers, and iterators referring to the moved elements.

Usage
-----

The usage guidelines apply for ordered_map:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Size of ordered_map should be checked before accessing the elements. |br| |br|
           Accessing outside the bounds of ordered_map using ``at()`` and ``operator[]()`` function
           will result in assertion.

Example
-------

The following example shows the declaration of ordered_map:

.. literalinclude:: ../../examples/ordered_map.cpp
   :start-after: EXAMPLE_ORDERED_MAP_DECLARATION_START
   :end-before: EXAMPLE_ORDERED_MAP_DECLARATION_END
   :language: c++
   :dedent: 4

The examples in the section below shows the usage of member functions:

.. literalinclude:: ../../examples/ordered_map.cpp
   :start-after: EXAMPLE_ORDERED_MAP_MEMBER_FUNCTION_START
   :end-before: EXAMPLE_ORDERED_MAP_MEMBER_FUNCTION_END
   :language: c++
   :dedent: 4

The next examples shows usage of various operations provided by the class
ordered_map:

.. literalinclude:: ../../examples/ordered_map.cpp
   :start-after: EXAMPLE_ORDERED_MAP_OPERATIONS_START
   :end-before: EXAMPLE_ORDERED_MAP_OPERATIONS_END
   :language: c++
   :dedent: 4
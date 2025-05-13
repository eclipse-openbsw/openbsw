.. _estd_bitset:

bitset
======

Introduction
------------

The ``estd::bitset`` is a fixed-size representation of bits. Single bit can be referred to via
indices. Index access begins with the least significant bit. |br| |br| A bitset is an array of
bools that uses less space than an array or vector of bools because each boolean value in bitset is
optimized to occupy only one bit of space rather than one separate byte. |br| |br| In contrast to
``std::bitset`` this class doesn't throw a ``std::out_of_range`` exception but calls
``estd_assert`` when the index passed to the set(), reset(), test(), and flip() functions is
out of bounds.

Usage
-----

The usage guidelines apply for bitset.

.. list-table::
   :widths: 30 70

   * - Usage guidance
     - The initial value for the bitset only considers the least significant bits of given size.
       Other bits are ignored. |br| |br|
       If the given initial value is a positive decimal number then only the whole number part is
       considered. |br| |br|
       For modifiers operator&=(), operator|=(), and operator^=() are only
       defined for bitsets of the same size ``Size``.

Example
-------

The following example shows the accessing of bitset element.

.. literalinclude:: ../../examples/bitset.cpp
   :start-after: EXAMPLE_START element_access
   :end-before: EXAMPLE_END element_access
   :language: c++
   :dedent: 4

The following example shows the functionality of bitset operations.

.. literalinclude:: ../../examples/bitset.cpp
   :start-after: EXAMPLE_START operations
   :end-before: EXAMPLE_END operations
   :language: c++
   :dedent: 4

.. _estd_iterator:

Iterator
========

Overview
--------

``estd::iterator`` provides a set of iterator tags and a template struct for
iterator traits. These components are essential for working with iterators in
C++. The iterator tags categorize iterators into different types, such as input
iterators, output iterators, forward iterators, bidirectional iterators, and
random-access iterators. The iterator traits struct provides a uniform interface
for accessing properties of iterators. This code enables the estd namespace to
provide iterator functionality that is compatible with both C++98 and C++11
standards.

``estd::iterator`` has a helper function that works with iterators, uses
``std::advance`` function which is available in C++17. The ``::estd::next()``
function is responsible for progressing an iterator either forward or backward
by a certain number of places.

This ``estd::iterator`` encapsulates a common operation of advancing an iterator
in a generic and reusable way, contributing to the flexibility and generic
programming capabilities of C++.

Usage
-----

The usage constrains and guidelines apply for iterator:

   .. list-table::
      :widths: 30 70

      * - Constrains
        - The input parameters should be valid within the range of elements you are working with.
      * - Usage guidance
        - Ensure that the iterator passed to the function is valid and points to a valid range of
          elements. |br| |br|
          Specify the appropriate number of positions to advance the iterator to avoid going beyond
          the valid range of the container.

Example
-------

Here is an example demonstrating the usage of ``next()`` functions:

.. literalinclude:: ../../examples/iterator.cpp
   :start-after: EXAMPLE_START iterator
   :end-before: EXAMPLE_END iterator
   :language: c++






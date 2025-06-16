.. _estd_tuple:

Overview
========

``tuple`` provides a set of utilities for working with C++11 standard tuples, backporting some
of the features from C++14 to ensure compatibility with earlier versions of the language.
It includes various helper functions and templates.

Usage
-----

.. list-table::
   :widths: 30 70

  * - Constraints
    - The `estd::get` function relies on unique types within the tuple. If a tuple contains multiple elements of the same type, only the first occurrence will be accessed correctly. |br| |br|
    - The `for_each` function requires the function `f` passed to it to be compatible with all elements of the tuple.
  * - Usage Guidance
    - Ensure that the types within the tuple are unique when using `estd::get` to avoid ambiguity.|br| |br|
    - When using `for_each`, the function `f`` should be able to accept all types in the tuple as arguments. For example, if the tuple contains both int and `std::string`, `f` should be able to process both types. |br| |br|

Example
-------

The following example demonstrates retrieving tuple elements.

.. literalinclude:: ../../examples/tuple.cpp
   :start-after: EXAMPLE_GET_CONSTRAINT_START
   :end-before: EXAMPLE_GET_CONSTRAINT_END
   :language: c++

This example shows applying a function to each tuple element.

.. literalinclude:: ../../examples/tuple.cpp
   :start-after: EXAMPLE_FOR_EACH_CONSTRAINT_START
   :end-before: EXAMPLE_FOR_EACH_CONSTRAINT_END
   :language: c++

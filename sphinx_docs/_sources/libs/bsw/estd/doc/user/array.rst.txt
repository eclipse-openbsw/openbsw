.. _estd_array:

estd::array
===========

Overview
--------

Provides a fixed-size container with contiguous storage. It is designed to offer
the C++11 ``std::array`` feature in an C++98 environment and to offer a ``estd``
compliant version of ``std::array`` in C++11 or later projects. In contrast to
``std::array`` this class doesn't throw a ``std::out_of_range`` exception but
calls ``estd_assert`` when the index passed to the ``at()`` function is out of
bounds.

It is an aggregate type with the same semantics as a struct holding a C-style
array ``T[N]`` as its only non-static data member. Unlike a C-style array, it
doesn't decay to ``T*`` automatically.

Usage Context
-------------

An array is merely a wrapper class for a raw C++ array. It is an aggregate type
and can thus be aggregate initialized.

The usage guidelines apply for array:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Size of array should be checked before accessing the elements. |br| |br|
           Accessing outside the bounds of array using ``at()`` function will result in assertion.

Example
+++++++

The following example shows a small function summing up the elements of an
array:

.. literalinclude:: ../../examples/array.cpp
   :start-after: EXAMPLE_START array
   :end-before: EXAMPLE_END array
   :language: c++

The next piece of code shows an example how this function is called:

.. literalinclude:: ../../examples/array.cpp
   :start-after: EXAMPLE_START array_init
   :end-before: EXAMPLE_END array_init
   :language: c++
   :dedent: 4

Implicitly Declared Methods for Aggregate Type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``array<T, N>`` is an aggregate type and thus provides the following implicitly
declared methods:

* Constructor
   .. code-block:: cpp

      template<class T, size_t N> array<T, N>::array({...})

   Initializes the array according to the rules of aggregate initialization
   see: `<http://en.cppreference.com/w/cpp/language/aggregate_initialization>`_.

* Destructor
   .. code-block:: cpp

      template<class T, size_t N> array<T, N>::~array()

   Destroys every element of the array.

* Assignment:
   .. code-block:: cpp

      template<class T, size_t N> array<T, N>& array<T, N>::operator=(const array<T, N>&)

   Overwrites every element of the array with the corresponding element of
   another array.

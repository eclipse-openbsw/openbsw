.. _estd_slice:

slice
=====

Introduction
------------

The slice represents a view of a contiguous sequence of elements while providing a friendly
container-like interface to work with said elements. This means that slice doesn't own the data it
refers to. slice internally stores information about the beginning of a container (pointer to the
first element of a sequence) and the number of elements to refer to (size), which means that the
data must live longer than the slice itself.

**Origin:** slice is almost identical to `std::span <https://en.cppreference.com/w/cpp/container/span>`_,
which was introduced in C++20 and before that it was available in C++14 with help of
`Guideline Support Library <https://github.com/microsoft/GSL>`_ ``gsl::span``.

``estd::slice`` is your go-to type when you want to refer to containers in your API.

Usage
-----

The usage guidelines apply for slice.

   .. list-table::
      :widths: 30 70

      *  - Constraints
         - Static slice having size greater than the container, cannot be created. |br| |br|
           Compilation error will be thrown if attempt to create is made.
      *  - Usage guidance
         - Size of the slice should be checked before accessing the elements. |br| |br|
           Accessing outside the bounds of slice using ``at<>()`` will result in compilation
           error. |br| and using ``at()``, ``operator[]()`` function will result in an assertion.
           |br| |br| |br|
           Size of the slice should be checked before trying to create subslice from a slice of size
           'Size'. |br| |br|
           Calling 'subslice<Limit>()' with Limit > Size and 'offset<Offset>() with Offset >= Size
           will result in compilation error. |br|
           Calling  subslice(limit) with limit > Size and offset(position) with position >= Size
           will return zero length slice.

Example
-------

Example below shows how to construct static estd::slice from C array. Here the size of slice
is known at compile-time :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd static construction from C array
    :end-before: EXAMPLE_END estd static construction from C array
    :language: c++
    :dedent: 4

Example below shows how to construct variable length estd::slice from C array. Here the size of
the slice is variable :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd dynamic construction from C array
    :end-before: EXAMPLE_END estd dynamic construction from C array
    :language: c++
    :dedent: 4

The next example shows constructing slice from :ref:`estd_array` :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd construction from an estd array
    :end-before: EXAMPLE_END estd construction from an estd array
    :language: c++
    :dedent: 4

The next example shows constructing dynamic slice from :ref:`estd_vector` :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd dynamic construction from estd vector
    :end-before: EXAMPLE_END estd dynamic construction from estd vector
    :language: c++
    :dedent: 4

Below is an example of how slice can be constructed from a pointer to an array.

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd construction from pointer
    :end-before: EXAMPLE_END estd construction from pointer
    :language: c++
    :dedent: 4

Note that it's not possible to construct a slice with size greater than a data
it refers to. The following code won't compile.

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd array too small
    :end-before: EXAMPLE_END estd array too small
    :language: c++
    :dedent: 4

Example below shows how to construct  estd::slice from non-Member functions like make_str(),
make_slice(), make_static_slice() :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd make_slice
    :end-before: EXAMPLE_END estd make_slice
    :language: c++

The below example shows the ways to iterate through the slice :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd iterate
    :end-before: EXAMPLE_END estd iterate
    :language: c++

The below example shows how to access the data of slice with 'at<Index>()' function :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd access_at
    :end-before: EXAMPLE_END estd access_at
    :language: c++

The next example shows creating a subslice from another slice (subview) :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd subviews
    :end-before: EXAMPLE_END estd subviews
    :language: c++

The next example shows creating a subslice from another slice with a compile time size
(static subview) :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd subviews_static
    :end-before: EXAMPLE_END estd subviews_static
    :language: c++

The next example shows remapping the slice to another area of the container with advance(), trim()
function :

.. sourceinclude:: examples/slice.cpp
    :start-after: EXAMPLE_BEGIN estd modify
    :end-before: EXAMPLE_END estd modify
    :language: c++

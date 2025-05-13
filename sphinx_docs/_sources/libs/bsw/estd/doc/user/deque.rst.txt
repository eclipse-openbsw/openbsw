.. _estd_deque:

estd::deque
===========

Overview
--------

A fixed-size container that holds a specific number of elements which can be
added or removed from both ends.

Unlike the STL deque, ``estd::deque`` is fully stack allocated and will
therefore not grow or reallocate memory.

It allows for fast insertion and deletion at both its beginning and its end. In
addition, insertion and deletion at either end of a deque never invalidates
pointers or references to the rest of the elements.

Attempts to exceed the fixed capacity of the deque will result in a failed
``estd_assert``.

The container can be used with noncopyable or non default constructible types by
using ``.emplace_front()``, ``.emplace_back()`` together with the
``estd::constructor<T>`` API.

Declare namespace allocates memory whereas estd::deque<T, size_type> is a
handler class.

Usage
-----

The usage guidelines apply for deque:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Size of deque should be checked before accessing the elements. |br| |br|
           Accessing outside the bounds of deque using ``at()`` and ``operator[]()`` function
           will result in assertion.

Example
-------

The following are the examples of declaring deque:

.. literalinclude:: ../../examples/deque.cpp
   :start-after: EXAMPLE_START declare
   :end-before: EXAMPLE_END declare
   :language: c++
   :dedent: 4

The following are the usage examples of iterators in deque:

.. literalinclude:: ../../examples/deque.cpp
   :start-after: EXAMPLE_START iterate
   :end-before: EXAMPLE_END iterate
   :language: c++

The following are the usage examples of capacity in deque:

.. literalinclude:: ../../examples/deque.cpp
   :start-after: EXAMPLE_START capacity
   :end-before: EXAMPLE_END capacity
   :language: c++
   :dedent: 4

The following are the usage examples of modifying operations in deque:

.. literalinclude:: ../../examples/deque.cpp
   :start-after: EXAMPLE_START modify
   :end-before: EXAMPLE_END modify
   :language: c++
   :dedent: 4

The following are the usage examples of comparison operations in deque:

.. literalinclude:: ../../examples/deque.cpp
   :start-after: EXAMPLE_START compare
   :end-before: EXAMPLE_END compare
   :language: c++
   :dedent: 4

Similarly, the other available operators can be used as per the usage example to
compare elements of deque.

.. _estd_priority_queue:

priority_queue
==============

Overview
--------

``estd::priority_queue`` is a type of queue container that arranges elements
based on their priority. Elements with higher priority are retrieved prior to
those with lower priority.

The ``estd::priority_queue`` operations e.g. ``push()``, ``pop()`` use
``std::push_heap()`` & ``std::pop_heap()`` to maintain the priority.

The priority of ``estd::priority_queue`` is defined by the user-provided
``Comparator`` supplied to maintain the ordering, which defaults to
``std::less<T>`` and would cause the greatest element to appear as the top.

``estd::priority_queue`` uses an encapsulated object of ``estd::vector``
(sequential container class) as its underlying container, providing a specific
set of member functions to access and modify its elements.

Syntax
++++++

.. code:: cpp

    ::estd::declare::priority_queue<T, N> pqueue_name;

Parameters
++++++++++

.. csv-table::

    "``T``", "Type of elements of priority_queue."
    "``N``", "Size of the priority_queue."

Usage
-----

Below mentioned are the usage constraints and guidelines applicable for
prority_queue.

    .. list-table::
       :widths: 30 70

       *  - Constraints
          - The priority_queue has a fixed max_size which is given at the time of declaration. This
            memory is allocated in stack. |br| |br|
            Exceeding the maximum size will trigger an assertion.
       *  - Usage guidance
          - Size of priority_queue should be checked before accessing the elements. |br| |br|
            Accessing elements from empty priority_queue using ``top()`` function
            will result in assertion.

Example
-------

The below example shows the construction of priority_queue & its operations.

.. literalinclude:: ../../examples/priority_queue.cpp
   :start-after: EXAMPLE_START
   :end-before: EXAMPLE_END
   :language: c++
.. _estd_stack:

stack
=====

Overview
--------

``estd::stack`` provides the functionality of a ``stack``, specifically a LIFO
(last-in, first-out) data structure.
|br| |br|
The class template acts as a wrapper to the underlying container. Only stack specific functions
(push and pop) are provided. ``stack`` appends and removes the elements to/from the back of the
underlying container. The back of the underlaying container can be considered as the
``top of the stack``.
|br| |br|
The ``stack`` uses an encapsulated object of :ref:`vector <estd_vector>` (by default) as its
underlying container.
|br| |br|
The 'declare' pattern in ``estd::declare::stack`` is to declare a statically-sized ``stack`` object
of size N which stores maximum size of ``stack`` at runtime, increasing RAM usage.

Usage
-----

The usage constraints and guidelines apply for ``stack``.

.. list-table::
   :widths: 30 70

   * - Constraints
     - ``stack`` has a fixed max_size which is given at the time of declaration. |br| |br|
       Expanding further than the max_size is not possible results in a runtime error.
   * - Usage guidance
     - ``estd::declare::stack`` must be used to create a stack, ``estd::stack``
       can only refer to an existing stack objects. |br| |br|
       The size should be checked before push or pop operations to avoid runtime error.

Example
-------

The following example shows various ways to construct a ``stack``.

.. literalinclude:: ../../examples/stack.cpp
   :start-after: EXAMPLE_STACK_CONSTRUCT_START
   :end-before: EXAMPLE_STACK_CONSTRUCT_END
   :language: c++
   :dedent: 4

The next piece of code shows an example of accessing elements from a ``stack``.

.. literalinclude:: ../../examples/stack.cpp
   :start-after: EXAMPLE_STACK_ACCESS_START
   :end-before: EXAMPLE_STACK_ACCESS_END
   :language: c++
   :dedent: 4

The next example shows various capacity operations performed on a ``stack``.

.. literalinclude:: ../../examples/stack.cpp
   :start-after: EXAMPLE_STACK_CAPACITY_START
   :end-before: EXAMPLE_STACK_CAPACITY_END
   :language: c++
   :dedent: 4

The example shows various modifying operations performed on a ``stack``.

.. literalinclude:: ../../examples/stack.cpp
   :start-after: EXAMPLE_STACK_MODIFIERS_START
   :end-before: EXAMPLE_STACK_MODIFIERS_END
   :language: c++
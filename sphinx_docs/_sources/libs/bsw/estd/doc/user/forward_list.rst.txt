.. _estd_forward_list:

estd::forward_list
==================

Overview
--------

``forward_list<T>`` provides a intrusive singly linked list. Intrusive means,
that through inheriting from a common base class. Each node provides the
necessary next pointer already and no wrapper node type with heap allocation is
required.

Usage
-----

The next example shows a simple example of the usage of a forward_list:

Example
+++++++

.. literalinclude:: ../../examples/forward_list.cpp
   :start-after: EXAMPLE_START:forward_list
   :end-before: EXAMPLE_END:forward_list
   :language: c++

``forward_list`` is also very useful in embedded systems that cannot use dynamic
memory allocation.

.. literalinclude:: ../../examples/forward_list.cpp
   :start-after: EXAMPLE_START:forward_list_vs_vector_1
   :end-before: EXAMPLE_END:forward_list_vs_vector_1
   :language: c++

In contrast to that, consider the example based on ``forward_list``. The pattern
is, that the interface inherits from ``forward_list_node`` which makes
maintaining lists of that interface easy.

.. literalinclude:: ../../examples/forward_list.cpp
   :start-after: EXAMPLE_START:forward_list_vs_vector_2
   :end-before: EXAMPLE_END:forward_list_vs_vector_2
   :language: c++

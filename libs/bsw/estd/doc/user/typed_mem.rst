.. _estd_typed_mem:

estd::typed_mem<T>
==================

Overview
--------

``typed_mem<T>`` is wrapper type for delayed creation of objects. If an application
uses global variables with static storage duration that are spread over multiple
compilation units, the initialization order is not guaranteed. If dependencies
between objects of these multiple compilation units exist, this can lead to
a application crash.

``typed_mem<T>`` allows to allocate memory with static storage duration be allows
the construction of the objects at runtime.

Example
-------

.. literalinclude:: ../../examples/typed_mem.cpp
   :start-after: EXAMPLE_BEGIN:typed_mem_lifecycle
   :end-before: EXAMPLE_END:typed_mem_lifecycle
   :language: c++

.. _estd_object_pool:

estd::object_pool
=================

Overview
--------

An ``::estd::object_pool`` is a memory manager for objects of the same type.
It allows user code to ``acquire`` or ``allocate`` objects from the pool and to
``release`` them when they are no longer needed. The pool can be configured to
provide a fixed number of elements. The ``::estd::object_pool`` cannot be
instantiated but is designed to be the user interface for client code.

The object_pool class is designed to store a pool of objects. Declare
object_pool class is used to restrict the number of object_pools which can be
created.It is a custom allocator for objects of a given type and keeps track of
the items which have been acquired and released. This requires a few extra bytes
of storage. The object_pool needs 1 bit for each element which requires 1 byte
for every 8 elements.

Usage
-----

The usage guidelines apply for object_pool:

   .. list-table::
      :widths: 30 70

      *  - Usage guidance
         - Size of the object pool should have a positive integer value.


Example
-------

Declaration and usage of object pool with allocate()

.. literalinclude:: ../../examples/object_pool.cpp
   :start-after: EXAMPLE_START construction
   :end-before: EXAMPLE_END construction
   :language: c++
   :dedent: 4

Declaration and usage of object pool with acquire()

.. literalinclude:: ../../examples/object_pool.cpp
   :start-after: EXAMPLE_START Usage
   :end-before: EXAMPLE_END Usage
   :language: c++
   :dedent: 4

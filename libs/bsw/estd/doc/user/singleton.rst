.. _estd_singleton:

singleton
=========

Overview
--------

The ``singleton`` template class ensures that only one instance of a class is created. It uses a
static pointer, initialized to nullptr and to check if an instance exists. The `instance` method
returns a reference to the single instance, ensuring controlled access. This design supports
without default constructors and provides a safe way to implement the singleton pattern.

Usage
-----

.. list-table::
  :widths: 30 70

  * - Preconditions
    - Make sure to include the appropriate library header for singleton.
  * - Assumptions
    - The constructor of the class must pass its pointer to the base class singleton in order to instantiate it.
  * - Usage guidance
    - Once instantiated, always use the instance() method to access the object.


Example
-------

The following example shows the usage of singleton using structure:

.. literalinclude:: ../../examples/singleton.cpp
   :start-after: EXAMPLE_SINGLETON_STRUCTURE_START
   :end-before: EXAMPLE_SINGLETON_STRUCTURE_END
   :language: c++
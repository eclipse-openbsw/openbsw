.. _estd_constructor:

constructor
===========

Overview
--------

A template class that provides a portable way to construct objects from memory
that is owned by a fixed-size container. This class makes it possible to add
objects that do not have default constructors. It is used for all of the emplace
methods.

..note::

  This is an outdated artifact and no longer necessary.

Usage
-----

- Constructors are used in containers to initialize objects in pre-allocated
  memory. This ensures that each object is properly set up within the allocated
  space.

- Use ```construct``` to call the constructor of given class.

- Constructors ensures an object starts in a valid state by initializing member variables and
  performing necessary setup.

Example
-------

An example for usage of constructor using a container is shown below:

.. literalinclude:: ../../examples/constructor.cpp
   :start-after: EXAMPLE_BEGIN emplaceExample
   :end-before: EXAMPLE_END emplaceExample
   :language: c++